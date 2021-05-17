#pragma once
#include <iostream>
#include <string>
#include <thread>
#include <regex>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <opencv2/opencv.hpp>

namespace reflock
{

    struct Mutex : std::mutex
    {
        Mutex() = default;
        Mutex(Mutex const &) noexcept : std::mutex() {}
        bool operator==(Mutex const &other) noexcept { return this == &other; }
    };

    struct Cond : std::condition_variable
    {
        Cond() = default;
        Cond(Cond const &) noexcept : std::condition_variable() {}
        bool operator==(Cond const &other) noexcept { return this == &other; }
    };

    enum reflock_type
    {
        sender,
        receiver
    };

    template <typename T>
    class RefLockManager
    {
    public:
        RefLockManager(){
        };
        RefLockManager(T &value):value_(value){
        };
        ~RefLockManager()
        {
            cond_.notify_all();
        }
        bool connect(const std::string &id, reflock_type ref_type)
        {
            if (ref_type == receiver)
            {
                return true;
            }
            return false;
        }
        bool try_set(const std::string &id, T &value)
        {
            if (mtx_.try_lock())
            {
                value_ = value;
                cond_.notify_one();
                mtx_.unlock();
                return true;
            }
            return false;
        }

        bool set(const std::string &id, T &value)
        {
            // find mtx
            std::unique_lock<std::mutex> lock(mtx_);
            value_ = value;
            cond_.notify_one();
            lock.unlock();
            return true;
        }

        bool get(const std::string &id, T &value)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cond_.wait(lock);
            value = value_;
            return true;
        }

        bool try_get(const std::string &id, T &value)
        {
            std::lock_guard<std::mutex> lock(mtx_);
            value = value_;
            return true;
        }

        bool disconnect(const std::string id)
        {
            value_ = T();
            cond_.notify_one();
            return true;
        }

    private:
        std::mutex mtx_;
        std::condition_variable cond_;
        T value_;
        T init;
    };
    
}; // namespace reflock

extern reflock::RefLockManager<cv::Mat> global_reflock;
extern reflock::RefLockManager<std::pair<cv::Mat, unsigned long>>  cambrion_reflock;