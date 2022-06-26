#include "job_manager.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <mutex>

using vm::job_manager::JobManager;

void create_random_order_tasks(JobManager &tl, const std::vector<int>& t_list)
{
    std::vector<std::chrono::seconds> time_points{t_list.begin(), t_list.end()};

    auto rng = std::default_random_engine {};
    std::shuffle(std::begin(time_points), std::end(time_points), rng);

    const auto start{std::chrono::steady_clock::now()};

    for(auto tp: time_points){
        std::thread([&tl, &tp, &start](){
            tl.QueueJob(start + tp, [](){
                return;
            });
        }).detach();
    }
}

int main()
{
    JobManager scheduler;

    create_random_order_tasks(scheduler, {10, 20, 25, 30});

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    scheduler.Start();
    create_random_order_tasks(scheduler, {-10, -20, 45});

    std::this_thread::sleep_for(std::chrono::seconds(45));
    scheduler.End();

    return 0;
}