#pragma once
#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <map>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
namespace thread
{
class CancelableTask
{
public:
    CancelableTask(std::function<void()> task, std::chrono::system_clock::time_point runtime)
        : runtime(runtime)
        , task(task)
    {
    }

    ~CancelableTask() {}
    void operator()() { task(); }
    auto operator<=>(const CancelableTask& other) const
    {
        // 首先比较值
        if (auto cmp = runtime <=> other.runtime; cmp != 0) { return cmp; }
        return this <=> &other;
    }

public:
    std::atomic<bool>                           isActive = true;
    const std::chrono::system_clock::time_point runtime;

private:
    std::function<void()> task;
};

class ThreadPoolImpl
{
    struct CancelableTaskCompare
    {
        bool operator()(const std::shared_ptr<CancelableTask>& l, const std::shared_ptr<CancelableTask>& r)
            const
        {
            if (!l) { return !!r; }
            if (!r) { return false; }
            return *l < *r;
        }
    };

public:
    std::vector<std::thread>                                                 workers;
    std::queue<std::function<void()>>                                        tasks;
    std::map<std::shared_ptr<CancelableTask>, size_t, CancelableTaskCompare> delayTasks;
    std::mutex                                                               mutex;
    std::condition_variable                                                  condition;
    bool                                                                     stop { false };
    ThreadPoolImpl(size_t nThreads);
    ~ThreadPoolImpl();
};

class ThreadPool
{

public:
    explicit ThreadPool(size_t nThreads = std::thread::hardware_concurrency());
    ~ThreadPool();

    void resize(size_t nThreads = std::thread::hardware_concurrency());
    void destroy();

    template<class F, class... Args>
    decltype(auto) addTask(F&& f, Args&&... args)
    {
        auto task = std::make_shared<std::packaged_task<std::invoke_result_t<F, Args...>()>>(
            [f = std::forward<F>(f), args...] { return f(args...); }
        );
        auto res = task->get_future();
        addTaskImpl([task] { (*task)(); });
        return res;
    }

    template<class F, class... Args>
    decltype(auto) addDelayTask(const std::chrono::system_clock::duration& duration, F&& f, Args&&... args)
    {
        auto futureTime = std::chrono::system_clock::now() + duration;
        auto task       = std::make_shared<std::packaged_task<std::invoke_result_t<F, Args...>()>>(
            [f = std::forward<F>(f), args...] { return f(args...); }
        );
        auto cancelableTask = std::make_shared<CancelableTask>([task]() { (*task)(); }, futureTime);
        addDelayTaskImpl(cancelableTask);
        return std::make_tuple(cancelableTask, task->get_future());
    }

private:
    std::unique_ptr<ThreadPoolImpl> impl;

    void addTaskImpl(std::function<void()> task);
    void addDelayTaskImpl(std::shared_ptr<CancelableTask> task);
};
} // namespace thread
