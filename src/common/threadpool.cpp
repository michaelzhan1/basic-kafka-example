#include "threadpool.hpp"

#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <utility>

ThreadPool::ThreadPool(size_t num_threads) {
    if (num_threads < 1) {
        throw std::invalid_argument("ThreadPool must have at least one thread");
    }

    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    condition.wait(lock, [this] {
                        return stop || !tasks.empty();
                    });
                    if (stop && tasks.empty()) {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        stop = true;
    }
    condition.notify_all();
}