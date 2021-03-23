/**
 * @file thread_pool.h
 * @author wsz (3033129201@qq.com)
 * @brief 
 * @date 2021-03-23:17:38:26
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef WSZ_TASK_POOL
#define WSZ_TASK_POOL

#include <queue>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <condition_variable>
namespace wsz
{
    /**
     * @brief 线程池
     * 
     */
    class TaskPool
    {
    public:
        /**
         * @brief 任务
         * 
         */
        using Task = std::function<void()>;
        /**
         * @brief Construct a new Thread Pool object
         * 
         */
        TaskPool() = default;
        /**
         * @brief Destroy the Thread Pool object
         * 
         */
        ~TaskPool()
        {
            if (!stop_)
                Stop();
        }
        /**
         * @brief 线程池开始
         * 
         * @param num 
         */
        void Start(int num)
        {
            stop_ = false;
            pool_size_ = num;
            for (size_t i = 0; i < pool_size_; ++i)
            {
                task_pool_.emplace_back(&TaskPool::threadProcess, this);
            }
        }
        /**
         * @brief 添加一个任务
         * 
         * @param task 
         */
        void AddTask(Task &&task)
        {
            {
                std::unique_lock<std::mutex> locker(safe_lock_);
                safe_condition_.wait(locker, [this] { return safe_qeque_.size() <= 100 || stop_; });
                if (stop_)
                    return;
                safe_qeque_.emplace(std::forward<Task>(task));
            }
            safe_condition_.notify_one();
        }
        /**
         * @brief 停止所有线程任务
         * 
         */
        void Stop()
        {
            stop_ = true;
            safe_condition_.notify_all();
            for (auto &th : task_pool_)
            {
                th.join();
            }
            {
                decltype(safe_qeque_) tmp;
                std::swap(safe_qeque_, tmp);
            }
            task_pool_.clear();
            pool_size_ = 0;
        }

    private:
        /**
         * @brief 线程函数
         * 
         */
        void threadProcess()
        {
            Task task;
            while (true)
            {
                {
                    std::unique_lock<std::mutex> locker(safe_lock_);
                    safe_condition_.wait(locker, [this] { return !safe_qeque_.empty() || stop_; });
                    if (stop_)
                        break;
                    task = std::move(safe_qeque_.front());
                    safe_qeque_.pop();
                }
                task();
                safe_condition_.notify_one();
            }
        }

    private:
        // 停止标志
        bool stop_;
        // 任务池数量
        int pool_size_;
        // 安全锁
        std::mutex safe_lock_;
        // 安全因子
        std::condition_variable safe_condition_;
        // 安全队列
        std::queue<Task> safe_qeque_;
        // 任务池
        std::vector<std::thread> task_pool_;
    };
} // namespace wsz

#endif /* WSZ_TASK_POOL */
