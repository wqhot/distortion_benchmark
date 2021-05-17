#pragma once

#include <functional>
#include <iostream>
#include <atomic>
#include <vector>
#include <common/timer.h>
#include <pipelines/run_flag.h>

template <typename T>
class Timer_zy : public Timer
{
public:
    void start_test_zy(int interval, std::function<bool(T *)> func, T *item)
    {
        // 已经开始的任务
        if (_expired == false)
            return;

        // 开启定时器任务
        _expired = false;
        std::thread([this, interval, func, item]() {
            // std::cout << "timer " << std::this_thread::get_id() << std::endl;
            while (!_try_to_expire)
            {
                // sleep
                std::this_thread::sleep_for(std::chrono::milliseconds(interval));
                bool exit_flag = !func(item);
                if (exit_flag)
                {
                    _try_to_expire = true;
                    // g_thread_manager.try_to_expire();
                    std::cout << "over" << std::endl;
                    global_run_flag = false;
                }
            }

            {
                // 更新状态量
                std::lock_guard<std::mutex> locker(_mutex);
                _expired = true;
                _expired_cond.notify_one();
            }
        }).detach();
    }
};