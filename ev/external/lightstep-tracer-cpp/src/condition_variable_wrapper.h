#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace lightstep {
// ConditionVariableWrapper is used to support testing. It allows for a custom
// condition variable implementation and artificial clock so that tests can be
// run without blocking timeout.
class ConditionVariableWrapper {
 public:
  virtual ~ConditionVariableWrapper() = default;

  virtual std::chrono::steady_clock::time_point Now() const = 0;

  virtual bool WaitFor(std::unique_lock<std::mutex>& lock,
                       const std::chrono::steady_clock::duration& duration,
                       std::function<bool()> predicate) = 0;

  virtual bool WaitUntil(std::unique_lock<std::mutex>& lock,
                         const std::chrono::steady_clock::time_point& timeout,
                         std::function<bool()> predicate) = 0;

  virtual void NotifyAll() = 0;
};

class StandardConditionVariableWrapper : public ConditionVariableWrapper {
 public:
  std::chrono::steady_clock::time_point Now() const override {
    return std::chrono::steady_clock::now();
  }

  bool WaitFor(std::unique_lock<std::mutex>& lock,
               const std::chrono::steady_clock::duration& duration,
               std::function<bool()> predicate) override {
    return condition_variable_.wait_for(lock, duration, predicate);
  }

  bool WaitUntil(std::unique_lock<std::mutex>& lock,
                 const std::chrono::steady_clock::time_point& timeout,
                 std::function<bool()> predicate) override {
    return condition_variable_.wait_until(lock, timeout, predicate);
  }

  void NotifyAll() override { condition_variable_.notify_all(); }

 private:
  std::condition_variable condition_variable_;
};
}  // namespace lightstep
