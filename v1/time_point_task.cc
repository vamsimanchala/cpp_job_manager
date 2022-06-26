#include "time_point_task.h"

namespace vm
{
namespace job_manager
{
//
// Base class to hold Task types
//

// Constructor to copy the task
Task::Task(const task_t& task): task_(task)
{
}

// Constructor to move the r-value reference task
Task::Task(task_t &&task) : task_(std::move(task))
{
}

// Executes the task function.
void Task::operator()(void)
{
  if (task_)
  {
    task_();
  }
}


//
// @brief: Constructor to contruct the TimePointTask using copies of time_point and task
// @param: time_point: const-reference to steady_time::time_point type
// @param: task: const-reference to function<void()> type
// 
TimePointTask::TimePointTask(const time_point_t &time_point, const task_t &task)
    : received_at_(clock_t::now()),
      to_run_at_(time_point),
      Task(task) {}

//
// @brief: Constructor to contruct the TimePointTask using r-value-refs of time_point and task
// @param: time_point: r-value-reference to steady_time::time_point type
// @param: task: r-value-reference to function<void()> type
// 
TimePointTask::TimePointTask(const time_point_t &time_point, task_t &&task)
    : received_at_(clock_t::now()),
      to_run_at_(time_point),
      Task(std::move(task)) {}

// 
// @brief: Operator() overload to executes the task function.
//
void TimePointTask::operator()(void)
{
  if (task_)
  {
    // This is using asynchronous fire-and-forget paradigm 
    // as Tasks need not return a future/value after completion of the tasks.
    // This perticularly helps in unblocking the worker limited number of threads
    // if the operations performed in these functions/tasks are computationally intense.
    call_async_task();
  }
}

// 
// @brief: Operator<= overload to compare two time_point tasks.
//
bool TimePointTask::operator<= (const TimePointTask& rhs) const {
  return to_run_at_ <= rhs.to_run_at_;
}

// 
// @brief: Operator< overload to compare two time_point tasks.
//
bool TimePointTask::operator< (const TimePointTask& rhs) const {
  return to_run_at_ < rhs.to_run_at_;
}

//
// @brief: This is a very neat way to workaround the blocking nature of the 
//    fire-and-forget async future. This is truely asynchronous and runs 
//
void TimePointTask::call_async_task(){
  auto fut_ptr = std::make_shared<std::future<void>>();
  *fut_ptr = std::async(std::launch::async, [fut_ptr, this](){
    const auto now = std::chrono::steady_clock::now();
    if (to_run_at_ > now){
      std::this_thread::sleep_until(to_run_at_);
    }
    std::cout << "Executed at: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(to_run_at_ - received_at_).count()
              << "ms from the Start of the program!" 
              << std::endl;

    task_();
  });
}
} // namespace job_manager
} // namespace vm
