#include "common/network/listen_socket_impl.h"

#include <sys/socket.h>
#include <sys/types.h>

#include <string>

#include "envoy/common/exception.h"

#include "common/common/assert.h"
#include "common/common/fmt.h"
#include "common/network/address_impl.h"
#include "common/network/utility.h"
#include "envoy/api/v2/lds.pb.h"

#if defined (__HAS_FF__)
#include "ff_api.h"
#endif

namespace Envoy {
namespace Network {

void ListenSocketImpl::doBind() {
  const Api::SysCallIntResult result = local_address_->bind(fd_);
  if (result.rc_ == -1) {
    close();
    throw EnvoyException(
        fmt::format("cannot bind '{}': {}", local_address_->asString(), strerror(result.errno_)));
  }
  if (local_address_->type() == Address::Type::Ip && local_address_->ip()->port() == 0) {
    // If the port we bind is zero, then the OS will pick a free port for us (assuming there are
    // any), and we need to find out the port number that the OS picked.
    local_address_ = Address::addressFromFd(fd_);
  }
}

// TODO: Set options on ff socket appropriately 
void ListenSocketImpl::setListenSocketOptions(const Network::Socket::OptionsSharedPtr& options) {
  if (!Network::Socket::applyOptions(options, *this,
                                     envoy::api::v2::core::SocketOption::STATE_PREBIND)) {
    throw EnvoyException("ListenSocket: Setting socket options failed");
  }
}

TcpListenSocket::TcpListenSocket(const Address::InstanceConstSharedPtr& address,
                                 const Network::Socket::OptionsSharedPtr& options,
                                 bool bind_to_port)
    : ListenSocketImpl(address->socket(Address::SocketType::Stream), address) {
        RELEASE_ASSERT(fd_ != -1, "");

  // TODO(htuch): This might benefit from moving to SocketOptionImpl.
        int on = 1;
        int rc;
        if (likely(IS_FP_SOCKET(fd_))) {
            CLEAR_FP_SOCKET(fd_);
            rc = ff_setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
            SET_FP_SOCKET(fd_);
        } else {
            rc = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        }
        RELEASE_ASSERT(rc != -1, "");

        // TODO: Set options on ff socket appropriately 
        //setListenSocketOptions(options);

        if (bind_to_port) {
            doBind();
        }
}

TcpListenSocket::TcpListenSocket(int64_t fd, const Address::InstanceConstSharedPtr& address,
                                 const Network::Socket::OptionsSharedPtr& options)
    : ListenSocketImpl(fd, address) {
  setListenSocketOptions(options);
}

UdsListenSocket::UdsListenSocket(const Address::InstanceConstSharedPtr& address)
    : ListenSocketImpl(address->socket(Address::SocketType::Stream), address) {
  RELEASE_ASSERT(fd_ != -1, "");
  doBind();
}

UdsListenSocket::UdsListenSocket(int fd, const Address::InstanceConstSharedPtr& address)
    : ListenSocketImpl(fd, address) {}

/* TODO: Remove this signature if not needed. Provider is now a part of socket */
TcpListenSocket::TcpListenSocket(const Address::InstanceConstSharedPtr& address, bool bind_to_port, const Network::Socket::OptionsSharedPtr& options, uint8_t provider)
    : ListenSocketImpl(address->socket(provider, Address::SocketType::Stream), address) {
  RELEASE_ASSERT(fd_ != -1, "");

  int on = 1;
  int rc;

#if defined (__HAS_FF__)
  
  // The address->socket() call above that takes the provider set the FP flag depending on the provider
  // TODO 4-21-2018 - revist after support for host based sockets
  if (IS_FP_SOCKET(fd_)) {
    rc = ff_setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  } else {
    rc = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  }

#else
    
  rc = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));  
#endif
  RELEASE_ASSERT(rc != -1, "");

  if (bind_to_port) {
    doBind();
  }
}

} // namespace Network
} // namespace Envoy
