#pragma once

#include <chrono>
#include <functional>
#include <thread>
#include <future>
#include <iostream>

namespace vm
{
namespace job_manager
{
//
// Base class to hold Task types
//
class Task {
public:
  using task_t = std::function<void(void)>; // public typedef to alias the function object
  using clock_t = std::chrono::steady_clock;
  using time_point_t = std::chrono::time_point<clock_t>;

  // Constructor to copy the task
  Task(const task_t& task);

  // Constructor to move the r-value reference task
  Task(task_t &&task);

  // Executes the task function.
  virtual void operator()(void);

protected:
  task_t task_;
};

// 
// TimePointTask: This Task type has a time_point attribute that can be used to perform 
// time_point based scheduling of these tasks. 
//
class TimePointTask: public Task
{
public:
  //
  // @brief: Constructor to contruct the TimePointTask using copies of time_point and task
  // @param: time_point: const-reference to steady_time::time_point type
  // @param: task: const-reference to function<void()> type
  // 
  TimePointTask(const time_point_t &time_point, const task_t &task);

  //
  // @brief: Constructor to contruct the TimePointTask using r-value-refs of time_point and task
  // @param: time_point: r-value-reference to steady_time::time_point type
  // @param: task: r-value-reference to function<void()> type
  // 
  TimePointTask(const time_point_t &time_point, task_t &&task);

  // 
  // @brief: Operator() overload to executes the task function.
  //
  virtual void operator()(void);

  // 
  // @brief: Operator<= overload to compare two time_point tasks.
  //
  bool operator<= (const TimePointTask& rhs) const;

  // 
  // @brief: Operator< overload to compare two time_point tasks.
  //
  bool operator< (const TimePointTask& rhs) const;

  // 
  // @brief: get to_run_at_
  //
  inline time_point_t GetRunTimePoint() const {
    return to_run_at_;
  };

private:
  //
  // @brief: This is a very neat way to workaround the blocking nature of the 
  //    fire-and-forget async future. This is truely asynchronous and runs 
  //
  void call_async_task();

  time_point_t received_at_; // steady Time
  time_point_t to_run_at_; // steady Time
};
} // namespace job_manager
} // namespace vm
