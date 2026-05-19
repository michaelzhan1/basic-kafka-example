#include "json.hpp"

#include <chrono>
#include <format>
#include <string>

std::string generate_json_message(const std::string& worker_id, Status status) {
    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);

    return "{\"worker_id\": \"" + worker_id + "\", \"status\": \"" +
           static_cast<std::string>(to_string(status)) +
           "\", \"timestamp\": \"" + timestamp + "\"}";
}