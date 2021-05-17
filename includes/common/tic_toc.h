#pragma once

#include <ctime>
#include <cstdlib>
#include <chrono>
#include <iostream>
#include <string>
// #include <glog/logging.h>
class TicToc
{
public:
    TicToc()
    {
        tic();
    }

    void tic()
    {
        start = std::chrono::system_clock::now();
    }

    double toc()
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        return elapsed_seconds.count() * 1000;
    }

    double restart_toc()
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        start = std::chrono::system_clock::now();
        return elapsed_seconds.count() * 1000;
    }

    void record_time()
    {
        double time_interval_ = restart_toc();
        // LOG(INFO) << "Interval time: " << time_interval_ << "ms";
    }

    void toc_print(std::string str)
    {
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
        std::cout << str << " spend: " << elapsed_seconds.count() * 1000 << " ms" << std::endl;
        start = std::chrono::system_clock::now();
    }

    void log(std::string str)
    {
        double time_interval_ = restart_toc();
        // LOG(INFO) << str << ": " << time_interval_ << " ms";
    }

private:
    std::chrono::time_point<std::chrono::system_clock> start, end;
};
