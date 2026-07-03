#pragma once

#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

#include "threadpool.hpp"

template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        [func = std::forward<F>(f), ...args = std::forward<Args>(args)]() mutable {
            return std::invoke(std::move(func), std::move(args)...);
        }
    );

    std::future<return_type> res = task->get_future();
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        if (stop) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}
