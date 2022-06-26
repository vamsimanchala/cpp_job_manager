#include "job_manager.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <random>
#include <algorithm>
#include <mutex>

using vm::job_manager::JobManager;

std::vector<std::chrono::seconds> time_points{
    std::chrono::seconds(10), 
    std::chrono::seconds(15), 
    std::chrono::seconds(37), 
    std::chrono::seconds(9)};

void create_random_order_tasks(JobManager &tl)
{
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

    create_random_order_tasks(scheduler);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    scheduler.Start();

    std::this_thread::sleep_for(std::chrono::seconds(25));
    scheduler.End();

    return 0;
}