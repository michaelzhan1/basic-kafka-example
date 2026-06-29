#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

class ThreadPool {
   public:
    // constructors
    explicit ThreadPool(size_t num_threads);
    ~ThreadPool();

    // methods
    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<std::invoke_result_t<F, Args...>>;

   private:
    // members
    std::vector<std::jthread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};

#include "threadpool.tpp"
