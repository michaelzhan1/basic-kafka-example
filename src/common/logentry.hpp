#pragma once

#include <chrono>
#include <nlohmann/json.hpp>
#include <string>

#include "status.hpp"

class LogEntry {
   public:
    using Timestamp = std::chrono::system_clock::time_point;
    using Json = nlohmann::json;

    // constructors
    LogEntry(const std::string& worker_id, Status status);
    explicit LogEntry(const Json& j);

    // methods
    Json to_json() const;

    // getters
    const std::string& get_worker_id() const;
    Status get_status() const;
    Timestamp get_timestamp() const;

   private:
    // members
    std::string worker_id;
    Status status;
    Timestamp timestamp;
};
