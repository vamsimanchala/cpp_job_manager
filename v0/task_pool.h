#pragma once

#include "time_point_task.h"

#include <thread>
#include <memory>
#include <vector>
#include <atomic>
#include <set>

namespace vm
{
namespace job_manager
{
//
// TaskPool Class to contain the threads and the list of tasks
//
class TaskPool
{
public:
  //
  // @brief: Constructor to build the task_list
  // @param: num_threads is the number of threads available in the Pool to complete the Jobs
  //
  TaskPool(int num_threads);

  ~TaskPool();

  //
  // @brief: AddJob will add the tasks to the list
  //    based on the DataStructure used to hiold the tasks, the teask might be ordered
  // @param: time_point: const-reference to steady_time::time_point type
  // @param: task: const-reference to function<void()> type
  //
  void AddJob(const Task::time_point_t &time_to_run, const Task::task_t &function);

  //
  // @brief: AddJob will add the tasks to the list
  //    based on the DataStructure used to hiold the tasks, the task might be ordered
  // @param: time_point: r-value-reference to steady_time::time_point type
  // @param: task: r-value-reference to function<void()> type
  //
  void AddJob(Task::time_point_t &&time_to_run, Task::task_t &&function);

  // 
  // @brief: StartProcessingJobs to start the reserved number of threads in the pool
  //
  void StartProcessingJobs();

  //
  // @brief: EndProcessing to end the Processing of Jobs
  //
  void EndProcessing();

private:
  void WorkerThreadFunction();

  std::mutex mutex_; // Mutex for exclusive access of the task_set
  std::condition_variable cv_; // Conditin_Variable for signaling the threads 
  std::set<TimePointTask> task_list_; // List of Jobs in a ThreadSafeOrderedList
  std::vector<std::thread> worker_threads_; // Vector of threads
  size_t num_threads_{0}; // Number of threads in the Pool to complete the Jobs
  bool stop_flag_{false}; // Used to stop the threads
};
} // namespace job_manager
} // namespace vm
