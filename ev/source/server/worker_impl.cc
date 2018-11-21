#include "server/worker_impl.h"

#include <functional>

#include "envoy/event/dispatcher.h"
#include "envoy/event/timer.h"
#include "envoy/server/configuration.h"
#include "envoy/thread_local/thread_local.h"

#include "common/common/thread.h"

#include "server/connection_handler_impl.h"

/* We define NO_THREAD to run this logic in context of main thread. */
/* Define NO_THREAD to run non-multithreaded version */
#if defined(__HAS_FF__)
#define NO_THREAD
#endif

namespace Envoy {
namespace Server {

    /* createWorker for multi-threaded version */
WorkerPtr ProdWorkerFactory::createWorker() {
#ifndef NO_THREAD
  Event::DispatcherPtr dispatcher(api_.allocateDispatcher(time_system_));
  return WorkerPtr{new WorkerImpl(
      tls_, hooks_, std::move(dispatcher),
      Network::ConnectionHandlerPtr{new ConnectionHandlerImpl(ENVOY_LOGGER(), *dispatcher)})};
#endif
}

/* createWorker(main_thread_dispatcher) for single process zero thread version */
WorkerPtr ProdWorkerFactory::createWorker(Envoy::Event::Dispatcher& dispatcher) {
  return WorkerPtr{new WorkerImpl(
      tls_, hooks_, dispatcher,
      Network::ConnectionHandlerPtr{new ConnectionHandlerImpl(ENVOY_LOGGER(), dispatcher)})};
}

WorkerImpl::WorkerImpl(ThreadLocal::Instance& tls, TestHooks& hooks,
#ifndef NO_THREAD
                       Event::DispatcherPtr&& dispatcher,
#else
                       Event::Dispatcher& dispatcher,
#endif
                       Network::ConnectionHandlerPtr handler)
    : tls_(tls), hooks_(hooks), 
#ifndef NO_THREAD
    dispatcher_(std::move(dispatcher)), 
#else
    dispatcher_(dispatcher), 
#endif
    handler_(std::move(handler)) {
#ifndef NO_THREAD
  tls_.registerThread(*dispatcher_, false);
#endif
}

void WorkerImpl::addListener(Network::ListenerConfig& listener, AddListenerCompletion completion) {
  // All listener additions happen via post. However, we must deal with the case where the listener
  // can not be created on the worker. There is a race condition where 2 processes can successfully
  // bind to an address, but then fail to listen() with EADDRINUSE. During initial startup, we want
  // to surface this.
#ifndef NO_THREAD
  dispatcher_->post([this, &listener, completion]() -> void {
#endif
    try {
      handler_->addListener(listener);
      hooks_.onWorkerListenerAdded();
      completion(true);
    } catch (const Network::CreateListenerException& e) {
      completion(false);
    }
#ifndef NO_THREAD
  });
#endif
}

uint64_t WorkerImpl::numConnections() {
  uint64_t ret = 0;
  if (handler_) {
    ret = handler_->numConnections();
  }
  return ret;
}

void WorkerImpl::removeListener(Network::ListenerConfig& listener,
                                std::function<void()> completion) {
#ifndef NO_THREAD
  ASSERT(thread_);
#endif
  const uint64_t listener_tag = listener.listenerTag();
#ifndef NO_THREAD
  dispatcher_->post([this, listener_tag, completion]() -> void {
    handler_->removeListeners(listener_tag);
    completion();
    hooks_.onWorkerListenerRemoved();
  });
#else
    handler_->removeListeners(listener_tag);
    completion();
    hooks_.onWorkerListenerRemoved();
#endif
}

void WorkerImpl::start(GuardDog& guard_dog) {
#ifndef NO_THREAD
  ASSERT(!thread_);
  thread_.reset(new Thread::Thread([this, &guard_dog]() -> void { threadRoutine(guard_dog); }));
#else
  threadRoutine(guard_dog);
#endif
}

void WorkerImpl::stop() {
  // It's possible for the server to cleanly shut down while cluster initialization during startup
  // is happening, so we might not yet have a thread.

#ifndef NO_THREAD
  if (thread_) {
    dispatcher_->exit();
    thread_->join();
  }
#endif
}

void WorkerImpl::stopListener(Network::ListenerConfig& listener) {
#ifndef NO_THREAD
  ASSERT(thread_);
#endif
  const uint64_t listener_tag = listener.listenerTag();
#ifndef NO_THREAD
  dispatcher_->post([this, listener_tag]() -> void { handler_->stopListeners(listener_tag); });
#endif
  handler_->stopListeners(listener_tag); 
}

void WorkerImpl::stopListeners() {
#ifndef NO_THREAD
  ASSERT(thread_);
  dispatcher_->post([this]() -> void { handler_->stopListeners(); });
#else
  handler_->stopListeners(); 
#endif
}

void WorkerImpl::threadRoutine(GuardDog& guard_dog) {
#ifndef NO_THREAD
  ENVOY_LOG(debug, "worker entering dispatch loop");
  auto watchdog = guard_dog.createWatchDog(Thread::Thread::currentThreadId());
  watchdog->startWatchdog(*dispatcher_);
#endif

#ifndef NO_THREAD
  dispatcher_->run(Event::Dispatcher::RunType::Block);
  ENVOY_LOG(debug, "worker exited dispatch loop");
  guard_dog.stopWatching(watchdog);

  // We must close all active connections before we actually exit the thread. This prevents any
  // destructors from running on the main thread which might reference thread locals. Destroying
  // the handler does this as well as destroying the dispatcher which purges the delayed deletion
  // list.
  handler_.reset();
  tls_.shutdownThread();
  watchdog.reset();
#endif
}

} // namespace Server
} // namespace Envoy
