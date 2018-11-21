#pragma once

#include <cstdint>
#include <functional>
#include <list>
#include <vector>

#include "envoy/common/time.h"
#include "envoy/event/deferred_deletable.h"
#include "envoy/event/dispatcher.h"
#include "envoy/network/connection_handler.h"

#include "common/common/logger.h"
#include "common/common/thread.h"
#include "common/event/libevent.h"

namespace Envoy {
namespace Event {

/**
 * libevent implementation of Event::Dispatcher.
 */
class DispatcherImpl : Logger::Loggable<Logger::Id::main>, public Dispatcher {
public:



  ///////////// BEGIN: Retain old code /////////////////
  // These function calls were modified from the original to take an additional argument. These are how the original ones look without the fake argument
  DispatcherImpl(TimeSystem& time_source, int fake_unused_argument);
  DispatcherImpl(TimeSystem& time_source, Buffer::WatermarkFactoryPtr&& factory, int fake_unused_argument);
  ///////////// END: Retain old code /////////////////


  DispatcherImpl(TimeSystem& time_source);
  DispatcherImpl(TimeSystem& time_source, Buffer::WatermarkFactoryPtr&& factory);
  ~DispatcherImpl();

  /**
   * @return event_base& the libevent base.
   */
  event_base& base() { return *base_; }
  event_base& base_host() { return *base_host_; }

  // Event::Dispatcher
  TimeSystem& timeSystem() override { return time_system_; }
  void clearDeferredDeleteList() override;
  Network::ConnectionPtr
  createServerConnection(Network::ConnectionSocketPtr&& socket,
                         Network::TransportSocketPtr&& transport_socket) override;
  Network::ClientConnectionPtr
  createClientConnection(Network::Address::InstanceConstSharedPtr address,
                         Network::Address::InstanceConstSharedPtr source_address,
                         Network::TransportSocketPtr&& transport_socket,
                         const Network::ConnectionSocket::OptionsSharedPtr& options) override;
  Network::DnsResolverSharedPtr createDnsResolver(
      const std::vector<Network::Address::InstanceConstSharedPtr>& resolvers) override;
  FileEventPtr createFileEvent(int64_t fd, FileReadyCb cb, FileTriggerType trigger,
                               uint32_t events) override;
  Filesystem::WatcherPtr createFilesystemWatcher() override;
  Network::ListenerPtr createListener(Network::Socket& socket, Network::ListenerCallbacks& cb,
                                      bool bind_to_port,
                                      bool hand_off_restored_destination_connections) override;
  TimerPtr createTimer(TimerCb cb) override;
  void deferredDelete(DeferredDeletablePtr&& to_delete) override;
  void exit() override;
  SignalEventPtr listenForSignal(int signal_num, SignalCb cb) override;
  void post(std::function<void()> callback) override;
  void run(RunType type) override;
  void run2(RunType type) override;
  Buffer::WatermarkFactory& getWatermarkFactory() override { return *buffer_factory_; }

private:
  void runPostCallbacks();

  // Validate that an operation is thread safe, i.e. it's invoked on the same thread that the
  // dispatcher run loop is executing on. We allow run_tid_ == 0 for tests where we don't invoke
  // run().
  bool isThreadSafe() const {
    return run_tid_ == 0 || run_tid_ == Thread::Thread::currentThreadId();
  }

  TimeSystem& time_system_;
  Thread::ThreadId run_tid_{};
  Buffer::WatermarkFactoryPtr buffer_factory_;
  Libevent::BasePtr base_;
  // base for collecting and processing events on host system
  Libevent::BasePtr base_host_;
  SchedulerPtr scheduler_;
  TimerPtr deferred_delete_timer_;
  TimerPtr post_timer_;
  std::vector<DeferredDeletablePtr> to_delete_1_;
  std::vector<DeferredDeletablePtr> to_delete_2_;
  std::vector<DeferredDeletablePtr>* current_to_delete_;
  Thread::MutexBasicLockable post_lock_;
  std::list<std::function<void()>> post_callbacks_ GUARDED_BY(post_lock_);
  bool deferred_deleting_{};
};

} // namespace Event
} // namespace Envoy
