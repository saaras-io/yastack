#include "common/api/os_sys_calls_impl.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdint>
#include "ff_api.h"
#include <envoy/network/address.h>

namespace Envoy {
namespace Api {

SysCallIntResult OsSysCallsImpl::bind(int64_t sockfd, const sockaddr* addr, socklen_t addrlen) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(sockfd))) {
        CLEAR_FP_SOCKET(sockfd);
        rc = ff_bind(sockfd, (const linux_sockaddr*) addr, addrlen);
    } else {
        rc = ::bind(sockfd, addr, addrlen);
    }
#else
        rc = ::bind(sockfd, addr, addrlen);
#endif
    return {rc, errno};

}

SysCallIntResult OsSysCallsImpl::ioctl(int64_t sockfd, unsigned long int request, void* argp) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(sockfd))) {
        CLEAR_FP_SOCKET(sockfd);
        rc = ff_ioctl(sockfd, request, argp);
    } else {
        rc = ::ioctl(sockfd, request, argp);
    }
#else
        rc = ::ioctl(sockfd, request, argp);
#endif
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::open(const std::string& full_path, int flags, int mode) {
    int rc = 0;
    rc = ::open(full_path.c_str(), flags, mode);
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::close(int64_t fd) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(fd))) {
        CLEAR_FP_SOCKET(fd);
        rc = ff_close(fd);
    } else {
        rc = ::close(fd);
    }
#else
        rc = ::close(fd);
#endif
    return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::write(int64_t fd, const void* buffer, size_t num_bytes) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(fd))) {
        CLEAR_FP_SOCKET(fd);
        rc = ff_write(fd, buffer, num_bytes);
    } else {
        rc = ::write(fd, buffer, num_bytes);
    }
#else
        rc = return ::write(fd, buffer, num_bytes);
#endif
    return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::writev(int64_t fd, const iovec* iovec, int num_iovec) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(fd))) {
        CLEAR_FP_SOCKET(fd);
        rc = ff_writev(fd, iovec, num_iovec);
    } else {
        rc = ::writev(fd, iovec, num_iovec);
    }
#else
        rc = ::writev(fd, iovec, num_iovec);
#endif
    return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::readv(int64_t fd, const iovec* iovec, int num_iovec) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(fd))) {
        CLEAR_FP_SOCKET(fd);
        rc = ff_readv(fd, iovec, num_iovec);
    } else {
        rc = ::readv(fd, iovec, num_iovec);
    }
#else
        rc = ::readv(fd, iovec, num_iovec);
#endif
    return {rc, errno};
}

SysCallSizeResult OsSysCallsImpl::recv(int64_t socket, void* buffer, size_t length, int flags) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(socket))) {
        CLEAR_FP_SOCKET(socket);
        rc = ff_recv(socket, buffer, length, flags);
    } else {
        rc = ::recv(socket, buffer, length, flags);
    }
#else
        rc = ::recv(socket, buffer, length, flags);
#endif
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::shmOpen(const char* name, int oflag, mode_t mode) {
    int rc = 0;
    rc = ::shm_open(name, oflag, mode);
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::shmUnlink(const char* name) {
    int rc = 0;
    rc = ::shm_unlink(name);
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::ftruncate(int64_t fd, off_t length) {
    int rc = 0;
    rc = ::ftruncate(fd, length);
    return {rc, errno};
}

SysCallPtrResult OsSysCallsImpl::mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    void * rc = ::mmap(addr, length, prot, flags, fd, offset);
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::stat(const char* pathname, struct stat* buf) {
    int rc = 0;
    rc = ::stat(pathname, buf);
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::setsockopt(int64_t sockfd, int level, int optname, const void* optval,
                               socklen_t optlen) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(sockfd))) {
        CLEAR_FP_SOCKET(sockfd);
        rc = ff_setsockopt(sockfd, level, optname, optval, optlen);
    } else {
        rc = ::setsockopt(sockfd, level, optname, optval, optlen);
    }
#else
        rc = ::setsockopt(sockfd, level, optname, optval, optlen);
#endif
    return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::getsockopt(int64_t sockfd, int level, int optname, void* optval,
                               socklen_t* optlen) {
    int rc = 0;
#if defined __HAS_FF__
    if ((IS_FP_SOCKET(sockfd))) {
        CLEAR_FP_SOCKET(sockfd);
        rc = ff_getsockopt(sockfd, level, optname, optval, optlen);
    } else {
        rc = ::getsockopt(sockfd, level, optname, optval, optlen);
    }
#else
        rc = ::getsockopt(sockfd, level, optname, optval, optlen);
#endif
        return {rc, errno};
}

SysCallIntResult OsSysCallsImpl::socket(int domain, int type, int protocol) {
  const int rc = ::socket(domain, type, protocol);
  return {rc, errno};
}

} // namespace Api
} // namespace Envoy
