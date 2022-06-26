#include "job_manager.h"

namespace vm
{
namespace job_manager
{
/* class constructor; creates a pool of 4 threads that start waiting
* for jobs to be ran. See description of QueueJob for details.
*/
JobManager::JobManager() : task_pool_(std::make_unique<TaskPool>(4)){};

/* Queues a job and its corresponding execution time in a list. The
* job manager will run the job when the system time reaches
* 'time_to_run'.
*
* Jobs can be queued from multiple threads and out of order. For example,
* it's possible that thread A queues a job that needs to run tomorrow
* at 10:10 am and later thread B queues a job that needs to run tomorrow
* at 9 am.
*
* Jobs with 'time_to_run' values in the past should run immediately. 
*
* Once the job is executed, it is removed from the list to limit
* memory consumption.
*
* In order to avoid execution delays the JobManager uses a thread
* pool of 4 threads to run jobs. This allows up to 4 jobs to run
* in parallel.
*
* For best use of system resources threads should wait on
* synchronization objects when they're not running a job.
*
* INPUT PARAMETERS
* time_to_run: absolute time since epoch when the job needs to run.
* job: function object that should be called to run the job.
*/
void JobManager::QueueJob(std::chrono::steady_clock::time_point time_to_run,
              std::function<void(void)> job) const
{
  task_pool_->AddJob(std::move(time_to_run), std::move(job));
}

/*
* Start the JobManager
*/
void JobManager::Start() const
{
  task_pool_->StartProcessingJobs();
}

/*
* End the JobManager
*/
void JobManager::End() const
{
  task_pool_->EndProcessing();
}
} // namespace job_manager
} // namespace vm
