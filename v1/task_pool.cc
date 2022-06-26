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
  : task_list_(std::make_unique<ThreadSafeOrderedList<TimePointTask>>()),
    num_threads_(num_threads)
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
  task_list_->insert(TimePointTask(time_to_run, function));
}

//
// @brief: AddJob will add the tasks to the list
//    based on the DataStructure used to hiold the tasks, the teask might be ordered
// @param: time_point: r-value-reference to steady_time::time_point type
// @param: task: r-value-reference to function<void()> type
//
void TaskPool::AddJob(Task::time_point_t &&time_to_run, Task::task_t &&function)
{
  task_list_->insert(TimePointTask(std::move(time_to_run), std::move(function)));
}

// 
// @brief: StartProcessingJobs to start the reserved number of threads in the pool
//
void TaskPool::StartProcessingJobs()
{
  for (size_t i = 0; i < num_threads_; i++)
  {
    worker_threads_.push_back(std::thread(&TaskPool::WorkerThreadFunction, this));
  }
}

//
// @brief: EndProcessing to end the Processing of Jobs
//
void TaskPool::EndProcessing(){
  stop_flag_ = true;
  for (size_t i = 0; i < num_threads_; i++)
  {
    if(worker_threads_[i].joinable()){
      worker_threads_[i].join();
    }
  }
}

void TaskPool::WorkerThreadFunction()
{
  while (true && !stop_flag_.load())
  {
    // Will simply pop the Job and execte the task if the task is a valid pointer
    // The Tasks are sorted in the ascending order and the processing of the Jobs is being done 
    // by using a fire-and-forget paradigm to asynchronously process the Jobs.
    
    // This method will ensure that the Worker_threads of the Pool are not doing the majority of the work.
    // Instead are only getting the First element on the ThreadSafeOrderedList and firing the async processing
    
    // This will also reduces(eliminates) the chance of another thread pushing to the ThreadSafeOrderedList
    // while one thread is trying to pop the first element.
    // Because, the data-structure is designed in a way that only one thread gets exclusive access, to the first element 
    // of the list, to pop. And once a thread gets access, only operation it does is to pop.

    // Only drawback with this approach is that the tasks that are meant to be process at a later time in the future,
    // but were added to the Pool early will sleep in a potentially separate thread(because OS effectively handles the spawning and
    // management of the async threds). 

    // This drawback can be eleiminated by allowing the threads to be able to peak the front of the list and sleep if the next 
    // Job is farther in time. And pop, only if any of the tasks are ready(in time_point) to be processes.
    // This approach might have its own drawbacks as well, in terms of peaking a lot. Some additional synchonization 
    // techniques can be used to notify the Pool if a Job with an early(any other) start data has been added to the List    
    std::unique_ptr<TimePointTask> task = std::move(task_list_->pop());
    if (task)
    {
      (*task)();
    }
  }
}
} // namespace job_manager
} // namespace vm
