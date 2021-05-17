#pragma once
#include <opencv2/core.hpp>
#include <queue>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>

typedef std::pair<cv::Mat, double> basic_item;
namespace thread_queue
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

    template <class T>
    class ThreadQueue
    {
    public:
        typedef std::queue<T> basic_queue;
        int add_queue()
        {
            int queue_id = m_queues.size();
            m_queues.push_back(basic_queue());
            m_try_to_expire.push_back(false);
            return queue_id;
        }
        int insert_queue(int id, T &insert_item)
        {
            if (id < 0)
            {
                return -1;
            }
            std::unique_lock<std::mutex> lock(m_mutexs[id]);
            while (m_queues[id].size() > 10)
            {
                m_queues[id].pop();
                // std::cout << "img[" << id << "] drop!" << std::endl;
            }
            m_queues[id].push(std::move(insert_item));
            m_conds[id].notify_all();
            lock.unlock();
            return 0;
        }
        bool get_queue(int id, T &get_item)
        {
            if (id < 0)
            {
                return false;
            }
            // std::cout << "lock " << id << " try get" << std::endl;
            std::unique_lock<std::mutex> lock(m_mutexs[id]);
            // std::cout << "lock " << id << "get" << std::endl;
            if (m_queues[id].empty())
            {
                m_conds[id].wait(lock);
            }
            bool ret = !m_queues[id].empty();
            if (!m_queues[id].empty())
            {
                get_item = std::move(m_queues[id].front());
                m_queues[id].pop();
            }
            lock.unlock();
            // std::cout << "lock " << id << " release" << std::endl;
            return ret;
        }
        bool try_get_queue(int id, T &get_item)
        {
            if (id < 0)
            {
                return false;
            }
            std::unique_lock<std::mutex> lock(m_mutexs[id]);
            if (m_queues[id].empty())
            {
                return false;
            }
            if (!m_queues[id].empty())
            {
                get_item = std::move(m_queues[id].front());
                m_queues[id].pop();
            }
            lock.unlock();
            return true;
        }
        bool get_queue_without_pop(int id, T &get_item)
        {
            if (id < 0)
            {
                return false;
            }
            std::unique_lock<std::mutex> lock(m_mutexs[id]);
            // std::cout << "lock " << id << "get" << std::endl;
            if (m_queues[id].empty())
            {
                m_conds[id].wait(lock);
            }
            bool ret = !m_queues[id].empty();
            if (!m_queues[id].empty())
            {
                get_item = m_queues[id].front();
            }
            lock.unlock();
            // std::cout << "lock " << id << " release" << std::endl;
            return ret;
        }
        bool insert_queue_with_pop(int id, T &insert_item)
        {
            if (id < 0)
            {
                return false;
            }
            std::unique_lock<std::mutex> lock(m_mutexs[id]);
            while (m_queues[id].size() > 0)
            {
                m_queues[id].pop();
            }
            m_queues[id].push(std::move(insert_item));
            m_conds[id].notify_all();
            lock.unlock();
            return 0;
        }
        void wait_for_insert(int id)
        {
            if (id < 0)
            {
                return;
            }
            std::unique_lock<std::mutex> lock(m_insert_mutexs[id]);
            m_insert_conds[id].wait(lock);
            lock.unlock();
        }
        void notify_to_insert(int id)
        {
            if (id < 0)
            {
                return;
            }
            m_insert_conds[id].notify_all();
        }
        int get_queue_size(int id)
        {
            if (id < 0)
            {
                return -1;
            }
            std::unique_lock<std::mutex> lock(m_mutexs[id]);
            int length = 0;
            if (m_queues[id].empty())
            {
                m_conds[id].wait(lock);
            }
            if (!m_queues[id].empty())
            {
                length = m_queues[id].size();
            }
            lock.unlock();
            return length;
        }
        std::mutex g_mutex;
        std::condition_variable g_cond;
        inline bool get_try_to_expire(int id)
        {
            if (id < 0)
            {
                return false;
            }
            return m_try_to_expire[id];
        }
        inline void try_to_expire()
        {
            for (size_t i = 0; i < m_try_to_expire.size(); i++)
            {
                m_try_to_expire[i] = true;
            }
            for (size_t i = 0; i < m_try_to_expire.size(); i++)
            {
                m_conds[i].notify_all();
            }
            for (size_t i = 0; i < m_try_to_expire.size(); i++)
            {
                m_insert_conds[i].notify_all();
            }
            should_exit = true;
        }
        bool should_exit;
        ThreadQueue()
        {
            m_queues.clear();
            m_try_to_expire.clear();
            should_exit = false;
        }

    private:
        std::vector<basic_queue> m_queues;
        std::mutex m_mutexs[10];
        std::condition_variable m_conds[10];
        std::mutex m_insert_mutexs[10];
        std::condition_variable m_insert_conds[10];
        std::vector<bool> m_try_to_expire;
    };
}; // namespace thread_queue
