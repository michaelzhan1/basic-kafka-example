#include "util.hpp"

#include <chrono>
#include <random>
#include <thread>

void random_sleep(int min_ms, int max_ms) {
    int sleep_duration = min_ms + (std::rand() % (max_ms - min_ms + 1));
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_duration));
}
