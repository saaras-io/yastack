#include "testing_condition_variable_wrapper.h"
#include <algorithm>
#include <exception>
#include <iostream>
#include <thread>

namespace lightstep {
//------------------------------------------------------------------------------
// NotifyAllEvent::Process
//------------------------------------------------------------------------------
void TestingConditionVariableWrapper::NotifyAllEvent::Process(
    TestingConditionVariableWrapper& condition_variable) {
  auto events = condition_variable.events_;
  condition_variable.events_.clear();
  for (auto event : events) {
    event->Notify();
  }
  delete this;
}

//------------------------------------------------------------------------------
// Destructor
//------------------------------------------------------------------------------
TestingConditionVariableWrapper::~TestingConditionVariableWrapper() {
  if (!events_.empty()) {
    std::cerr << "Event list must be empty upon destruction\n";
    std::terminate();
  }
}

//------------------------------------------------------------------------------
// Now
//------------------------------------------------------------------------------
std::chrono::steady_clock::time_point TestingConditionVariableWrapper::Now()
    const {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  return now_;
}

//------------------------------------------------------------------------------
// set_now
//------------------------------------------------------------------------------
void TestingConditionVariableWrapper::set_now(
    const std::chrono::steady_clock::time_point& time_point) {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  now_ = std::max(now_, time_point);
}

//------------------------------------------------------------------------------
// WaitFor
//------------------------------------------------------------------------------
bool TestingConditionVariableWrapper::WaitFor(
    std::unique_lock<std::mutex>& lock,
    const std::chrono::steady_clock::duration& duration,
    std::function<bool()> predicate) {
  auto timeout = Now() + duration;
  return WaitUntil(lock, timeout, predicate);
}

//------------------------------------------------------------------------------
// WaitUntil
//------------------------------------------------------------------------------
bool TestingConditionVariableWrapper::WaitUntil(
    std::unique_lock<std::mutex>& lock,
    const std::chrono::steady_clock::time_point& timeout,
    std::function<bool()> predicate) {
  WaitEvent event{timeout};
  if (predicate()) {
    return true;
  }
  while (1) {
    lock.unlock();
    AddEvent(&event);
    while (!event.DecrementTicker()) {
      std::this_thread::yield();
    }
    lock.lock();
    if (timeout <= Now()) {
      return predicate();
    }
    if (predicate()) {
      return true;
    }
  }
}

//------------------------------------------------------------------------------
// NotifyAll
//------------------------------------------------------------------------------
void TestingConditionVariableWrapper::NotifyAll() {
  auto event = new NotifyAllEvent{Now()};
  if (block_notify_all_) {
    AddEvent(event);
  } else {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    event->Process(*this);
  }
}

//------------------------------------------------------------------------------
// WaitTillNextEvent
//------------------------------------------------------------------------------
void TestingConditionVariableWrapper::WaitTillNextEvent() {
  while (1) {
    {
      std::lock_guard<std::mutex> lock_guard{mutex_};
      if (!events_.empty()) {
        return;
      }
    }
    std::this_thread::yield();
  }
}

//------------------------------------------------------------------------------
// Step
//------------------------------------------------------------------------------
void TestingConditionVariableWrapper::Step() {
  while (1) {
    {
      std::lock_guard<std::mutex> lock_guard{mutex_};
      auto iter = std::min_element(events_.begin(), events_.end(),
                                   [](Event* lhs, Event* rhs) {
                                     return lhs->timeout() < rhs->timeout();
                                   });
      if (iter != events_.end()) {
        auto event = *iter;
        now_ = std::max(event->timeout(), now_);
        events_.erase(iter);
        event->Process(*this);
        break;
      }
    }
    std::this_thread::yield();
  }
}

//------------------------------------------------------------------------------
// AddEvent
//------------------------------------------------------------------------------
void TestingConditionVariableWrapper::AddEvent(Event* event) {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  events_.push_back(event);
}

//------------------------------------------------------------------------------
// top_event
//------------------------------------------------------------------------------
auto TestingConditionVariableWrapper::next_event() const -> const Event* {
  std::lock_guard<std::mutex> lock_guard{mutex_};
  auto iter = std::min_element(
      events_.begin(), events_.end(),
      [](Event* lhs, Event* rhs) { return lhs->timeout() < rhs->timeout(); });
  if (iter == events_.end()) {
    return nullptr;
  }
  return *iter;
}
}  // namespace lightstep
