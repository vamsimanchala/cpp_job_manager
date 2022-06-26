
#include "task_pool.h"

namespace vm
{
namespace job_manager
{
//
// @brief: Constructor to build the task_list
// @param: num_threads is the number of threads available in the Pool to complete the Jobs
//
TaskPool::TaskPool(int num_threads) 
  : num_threads_(num_threads)
{
  worker_threads_.reserve(num_threads_);
}

TaskPool::~TaskPool(){
  EndProcessing();
}

//
// @brief: AddJob will add the tasks to the list
//    based on the DataStructure used to hiold the tasks, the teask might be ordered
// @param: time_point: const-reference to steady_time::time_point type
// @param: task: const-reference to function<void()> type
//
void TaskPool::AddJob(const Task::time_point_t &time_to_run, const Task::task_t &function)
{
  std::unique_lock<std::mutex> lock(mutex_);
  const bool notify = task_list_.empty() 
    || (time_to_run < task_list_.begin()->GetRunTimePoint())
    || (Task::clock_t::now() >= time_to_run); 
  task_list_.insert(TimePointTask(time_to_run, function));

  if(notify){
    lock.unlock();
    cv_.notify_one();
  }
}

//
// @brief: AddJob will add the tasks to the list
//    based on the DataStructure used to hiold the tasks, the task might be ordered
// @param: time_point: r-value-reference to steady_time::time_point type
// @param: task: r-value-reference to function<void()> type
//
void TaskPool::AddJob(Task::time_point_t &&time_to_run, Task::task_t &&function)
{
  std::unique_lock<std::mutex> lock(mutex_);
  const bool notify = task_list_.empty() 
    || (time_to_run < task_list_.begin()->GetRunTimePoint())
    || (Task::clock_t::now() >= time_to_run);
  task_list_.insert(TimePointTask(std::move(time_to_run), std::move(function)));

  if(notify){
    lock.unlock();
    cv_.notify_one();
  }
}

// 
// @brief: StartProcessingJobs to start the reserved number of threads in the pool
//
void TaskPool::StartProcessingJobs()
{
  std::lock_guard<std::mutex> lock(mutex_);
  for (size_t i = 0; i < num_threads_; i++)
  {
    worker_threads_.push_back(std::thread(&TaskPool::WorkerThreadFunction, this));
  }
}

//
// @brief: EndProcessing to end the Processing of Jobs
//
void TaskPool::EndProcessing(){
  std::unique_lock<std::mutex> lock(mutex_);
  stop_flag_ = true;
  lock.unlock();
  cv_.notify_one();
  task_list_.clear();
  for (size_t i = 0; i < num_threads_; i++)
  {
    if(worker_threads_[i].joinable()){
      worker_threads_[i].join();
    }
  }
}

void TaskPool::WorkerThreadFunction()
{
  while (!stop_flag_)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    // Wait and unblock the mutex for other threads if the task_list_is 
    // empty.
    cv_.wait(lock, [&](){
      return !task_list_.empty();
    });

    // Wait until you find a Task that needs to be Executed immediately
    const Task::time_point_t earliest_time_point = task_list_.begin()->GetRunTimePoint();
    cv_.wait_until(lock, earliest_time_point, [&](){
      return (!task_list_.empty() && (Task::clock_t::now() >= task_list_.begin()->GetRunTimePoint()))
        || (task_list_.begin()->GetRunTimePoint() < earliest_time_point);
    });

    while(!task_list_.empty() && Task::clock_t::now() >= task_list_.begin()->GetRunTimePoint()) {
      TimePointTask task = std::move(task_list_.extract(task_list_.begin()).value()); // From C++17
      task();
    }
  }
}
} // namespace job_manager
} // namespace vm
