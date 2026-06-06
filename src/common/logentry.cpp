#include "logentry.hpp"

#include <chrono>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

namespace {
bool is_valid_status(int status_value) {
    return status_value >= static_cast<int>(Status::OK) &&
           status_value <= static_cast<int>(Status::ERROR);
}
}  // namespace

// Constructors

LogEntry::LogEntry(const std::string& worker_id, Status status)
    : worker_id(worker_id),
      status(status),
      timestamp(std::chrono::system_clock::now()) {}

LogEntry::LogEntry(const Json& j) {
    if (!j.contains("worker_id") || !j.contains("status") ||
        !j.contains("timestamp")) {
        throw std::invalid_argument(
            "LogEntry JSON must contain worker_id, status, and timestamp");
    }

    worker_id = j.at("worker_id").get<std::string>();
    int status_value = j.at("status").get<int>();
    if (!is_valid_status(status_value)) {
        throw std::invalid_argument("LogEntry status value is out of range");
    }
    status = static_cast<Status>(status_value);

    auto timestamp_ms = j.at("timestamp").get<long long>();
    timestamp = Timestamp(std::chrono::milliseconds(timestamp_ms));
}

// Methods

LogEntry::Json LogEntry::to_json() const {
    return Json{
        {"worker_id", worker_id},
        {"status", static_cast<int>(status)},
        {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                          timestamp.time_since_epoch())
                          .count()}};
}

// Getters

const std::string& LogEntry::get_worker_id() const { return worker_id; }

Status LogEntry::get_status() const { return status; }

LogEntry::Timestamp LogEntry::get_timestamp() const { return timestamp; }
