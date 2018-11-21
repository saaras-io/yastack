#include <functional>

#include "common/common/lock_guard.h"
#include "common/common/thread.h"
#include "common/event/dispatcher_impl.h"

#include "test/mocks/common.h"
#include "test/test_common/test_time.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::InSequence;

namespace Envoy {
namespace Event {

class TestDeferredDeletable : public DeferredDeletable {
public:
  TestDeferredDeletable(std::function<void()> on_destroy) : on_destroy_(on_destroy) {}
  ~TestDeferredDeletable() { on_destroy_(); }

private:
  std::function<void()> on_destroy_;
};

TEST(DeferredDeleteTest, DeferredDelete) {
  InSequence s;
  DangerousDeprecatedTestTime test_time;
  DispatcherImpl dispatcher(test_time.timeSystem());
  ReadyWatcher watcher1;

  dispatcher.deferredDelete(
      DeferredDeletablePtr{new TestDeferredDeletable([&]() -> void { watcher1.ready(); })});

  // The first one will get deleted inline.
  EXPECT_CALL(watcher1, ready());
  dispatcher.clearDeferredDeleteList();

  // This one does a nested deferred delete. We should need two clear calls to actually get
  // rid of it with the vector swapping. We also test that inline clear() call does nothing.
  ReadyWatcher watcher2;
  ReadyWatcher watcher3;
  dispatcher.deferredDelete(DeferredDeletablePtr{new TestDeferredDeletable([&]() -> void {
    watcher2.ready();
    dispatcher.deferredDelete(
        DeferredDeletablePtr{new TestDeferredDeletable([&]() -> void { watcher3.ready(); })});
    dispatcher.clearDeferredDeleteList();
  })});

  EXPECT_CALL(watcher2, ready());
  dispatcher.clearDeferredDeleteList();

  EXPECT_CALL(watcher3, ready());
  dispatcher.clearDeferredDeleteList();
}

class DispatcherImplTest : public ::testing::Test {
protected:
  DispatcherImplTest()
      : dispatcher_(std::make_unique<DispatcherImpl>(test_time_.timeSystem())),
        work_finished_(false) {
    dispatcher_thread_ = std::make_unique<Thread::Thread>([this]() {
      // Must create a keepalive timer to keep the dispatcher from exiting.
      std::chrono::milliseconds time_interval(500);
      keepalive_timer_ = dispatcher_->createTimer(
          [this, time_interval]() { keepalive_timer_->enableTimer(time_interval); });
      keepalive_timer_->enableTimer(time_interval);

      dispatcher_->run(Dispatcher::RunType::Block);
    });
  }

  ~DispatcherImplTest() override {
    dispatcher_->exit();
    dispatcher_thread_->join();
  }

  DangerousDeprecatedTestTime test_time_;

  std::unique_ptr<Thread::Thread> dispatcher_thread_;
  DispatcherPtr dispatcher_;
  Thread::MutexBasicLockable mu_;
  Thread::CondVar cv_;

  bool work_finished_;
  TimerPtr keepalive_timer_;
};

//TEST_F(DispatcherImplTest, Post) {
//    dispatcher_->post([this]() {
//            {
//
//            Thread::LockGuard lock(mu_);
//
//            work_finished_ = true;
//            }
//            cv_.notifyOne();
//            });
//
//    Thread::LockGuard lock(mu_);
//    while (!work_finished_) {
//        cv_.wait(mu_);
//    }
//}
//
////// Ensure that there is no deadlock related to calling a posted callback, or
//// destructing a closure when finished calling it.
//TEST_F(DispatcherImplTest, RunPostCallbacksLocking) {
//  class PostOnDestruct {
//  public:
//    PostOnDestruct(Dispatcher& dispatcher) : dispatcher_(dispatcher) {}
//    ~PostOnDestruct() {
//      dispatcher_.post([]() {});
//    }
//    void method() {}
//    Dispatcher& dispatcher_;
//  };
//
//  {
//    // Block dispatcher first to ensure that both posted events below are handled
//    // by a single call to runPostCallbacks().
//    //
//    // This also ensures that the post_lock_ is not held while callbacks are called,
//    // or else this would deadlock.
//    Thread::LockGuard lock(mu_);
//    dispatcher_->post([this]() { Thread::LockGuard lock(mu_); });
//
//    auto post_on_destruct = std::make_shared<PostOnDestruct>(*dispatcher_);
//    dispatcher_->post([=]() { post_on_destruct->method(); });
//    dispatcher_->post([this]() {
//      {
//        Thread::LockGuard lock(mu_);
//        work_finished_ = true;
//      }
//      cv_.notifyOne();
//    });
//  }
//
//  Thread::LockGuard lock(mu_);
//  while (!work_finished_) {
//    cv_.wait(mu_);
//  }
//}


// This test case will not work.
// The timer creation results in evtimer_assign().
// However to satisfy this, the event_base must be polled.
// Since we are in a dpdk environment and this test doesn't
// run the poll loop, the timer will not be serviced.
//
// Right now this test just hangs (as expected)
//
// In real scenarios, when we are polling the event loop,
// the timer event is also serviced, hence it will fire.
TEST_F(DispatcherImplTest, Timer) {
    TimerPtr timer;
    dispatcher_->post([this, &timer]() {
            {

            std::cout << "dispatcher_->post acquire lock...";
            std::cout << std::endl;
            //Thread::LockGuard lock(mu_);
            timer = dispatcher_->createTimer([this]() {
                    {
                    Thread::LockGuard lock(mu_);

                    std::cout << "dispatcher_->post lock acquired...";
                    std::cout << std::endl;
                    work_finished_ = true;
                    }
                    cv_.notifyOne();
                    });
            }
            cv_.notifyOne();
            });

    std::cout << "acquire lock...";
    std::cout << std::endl;
    Thread::LockGuard lock(mu_);
    while (timer == nullptr) {

        std::cout << "timer ! nullptr ...";
        std::cout << std::endl;
        cv_.wait(mu_);
    }
    timer->enableTimer(std::chrono::milliseconds(50));

    while (!work_finished_) {
        cv_.wait(mu_);
    }
}

} // namespace Event
} // namespace Envoy
