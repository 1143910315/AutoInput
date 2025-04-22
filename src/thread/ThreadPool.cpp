#include "thread/ThreadPool.h"
thread::ThreadPoolImpl::ThreadPoolImpl(size_t nThreads)
{
    for (size_t i = 1; i <= nThreads; ++i)
    {
        workers.emplace_back([threadId = i, this] {
            while (true)
            {
                std::unique_lock lock { mutex };

                auto checkLoop = [this, threadId]() {
                    if (!tasks.empty() || stop) { return false; }
                    if (delayTasks.empty()) { return true; }
                    else
                    {
                        auto now = std::chrono::system_clock::now();
                        for (auto& [cancelableTask, id] : delayTasks)
                        {
                            if (now < cancelableTask->runtime) { return true; }
                            if (id == 0 && cancelableTask->isActive.load()) { return false; }
                        }
                        return true;
                    }
                };
                while (checkLoop())
                {
                    bool longWait = true;
                    auto it       = delayTasks.begin();
                    while (it != delayTasks.end())
                    {
                        auto& [cancelableTask, id] = *it;
                        if (id == 0)
                        {
                            if (!cancelableTask->isActive.load())
                            {
                                it = delayTasks.erase(it);
                                continue;
                            }
                            longWait = false;
                            id       = threadId;
                            condition.wait_until(lock, cancelableTask->runtime, [this, threadId]() {
                                if (!tasks.empty() || stop) { return true; }
                                for (auto& [cancelableTask, id] : delayTasks)
                                {
                                    if (id == 0) { return true; }
                                    if (id == threadId) { return !cancelableTask->isActive.load(); }
                                }
                                return false;
                            });
                            id = 0;
                            break;
                        }
                        it++;
                    }
                    if (longWait)
                    {
                        condition.wait(lock, [this]() {
                            if (!tasks.empty() || stop) { return true; }
                            for (auto& [cancelableTask, id] : delayTasks)
                            {
                                if (id == 0) { return true; }
                            }
                            return false;
                        });
                    }
                }
                if (stop && tasks.empty())
                {
                    if (delayTasks.empty()) { return; }
                    else
                    {
                        auto now = std::chrono::system_clock::now();
                        for (auto& [cancelableTask, id] : delayTasks)
                        {
                            if (now < cancelableTask->runtime) { return; }
                            if (id == 0) { break; }
                        }
                    }
                }
                if (tasks.empty())
                {
                    for (auto& [cancelableTask, id] : delayTasks)
                    {
                        if (id == 0) { break; }
                    }
                    for (auto it = delayTasks.begin(); it != delayTasks.end(); ++it)
                    {
                        if (it->second == 0)
                        {
                            bool                            expected = true;
                            std::shared_ptr<CancelableTask> task     = it->first;
                            delayTasks.erase(it);
                            lock.unlock();
                            if (task->isActive.compare_exchange_strong(expected, false)) { (*task)(); }
                            break;
                        }
                    }
                }
                else
                {
                    std::function<void()> task = std::move(tasks.front());
                    tasks.pop();
                    lock.unlock();
                    task();
                }
            }
        });
    }
}
thread::ThreadPoolImpl::~ThreadPoolImpl()
{
    stop = true;
    condition.notify_all();
    for (auto& worker : workers)
    {
        if (worker.joinable()) worker.join();
    }
}
thread::ThreadPool::ThreadPool(size_t nThreads)
    : impl(std::make_unique<ThreadPoolImpl>(nThreads)) {};
thread::ThreadPool::~ThreadPool() {};
void thread::ThreadPool::resize(size_t nThreads)
{
    impl = std::make_unique<ThreadPoolImpl>(nThreads);
}
void thread::ThreadPool::destroy() { impl.reset(); }
void thread::ThreadPool::addTaskImpl(std::function<void()> task)
{
    {
        std::lock_guard lock { impl->mutex };
        impl->tasks.emplace(std::move(task));
    }
    impl->condition.notify_one();
}
void thread::ThreadPool::addDelayTaskImpl(std::shared_ptr<CancelableTask> task)
{
    {
        std::lock_guard lock { impl->mutex };
        impl->delayTasks.emplace(std::move(task), 0);
    }
    impl->condition.notify_one();
}