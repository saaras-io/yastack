#pragma once

#include <functional>
#include <memory>

#include "envoy/api/api.h"
#include "envoy/network/connection_handler.h"
#include "envoy/server/guarddog.h"
#include "envoy/server/listener_manager.h"
#include "envoy/server/worker.h"
#include "envoy/thread_local/thread_local.h"

#include "common/common/logger.h"
#include "common/common/thread.h"

#include "server/test_hooks.h"

#if defined(__HAS_FF__)
#define NO_THREAD
#endif

namespace Envoy {
namespace Server {

class ProdWorkerFactory : public WorkerFactory, Logger::Loggable<Logger::Id::main> {
public:
  ProdWorkerFactory(ThreadLocal::Instance& tls, Api::Api& api, TestHooks& hooks,
                    Event::TimeSystem& time_system)
      : tls_(tls), api_(api), hooks_(hooks), time_system_(time_system) {}

  // Server::WorkerFactory
  WorkerPtr createWorker() override;
  WorkerPtr createWorker(Envoy::Event::Dispatcher& dispatcher) override;

private:
  ThreadLocal::Instance& tls_;
  Api::Api& api_;
  TestHooks& hooks_;
  Event::TimeSystem& time_system_;
};

/**
 * A server threaded worker that wraps up a worker thread, event loop, etc.
 */
class WorkerImpl : public Worker, Logger::Loggable<Logger::Id::main> {
public:
  WorkerImpl(ThreadLocal::Instance& tls, TestHooks& hooks, 
#ifndef NO_THREAD
          Event::DispatcherPtr&& dispatcher,
#else
          Event::Dispatcher& dispatcher,
#endif
          Network::ConnectionHandlerPtr handler);

  // Server::Worker
  void addListener(Network::ListenerConfig& listener, AddListenerCompletion completion) override;
  uint64_t numConnections() override;
  void removeListener(Network::ListenerConfig& listener, std::function<void()> completion) override;
  void start(GuardDog& guard_dog) override;
  void stop() override;
  void stopListener(Network::ListenerConfig& listener) override;
  void stopListeners() override;

private:
  void threadRoutine(GuardDog& guard_dog);

  ThreadLocal::Instance& tls_;
  TestHooks& hooks_;
#ifndef NO_THREAD
  Event::DispatcherPtr dispatcher_;
#else
  Event::Dispatcher& dispatcher_;
#endif
  Network::ConnectionHandlerPtr handler_;
  Thread::ThreadPtr thread_;
};

} // namespace Server
} // namespace Envoy
