#include "database.hpp"

#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <vector>

#include "logentry.hpp"

void insert_log(pqxx::connection& db_conn, const LogEntry& log_entry) {
    try {
        pqxx::work tx(db_conn);

        // status formatting
        std::string status_str = static_cast<std::string>(to_string(log_entry.get_status()));

        // handle time formatting
        auto tp = log_entry.get_timestamp();
        auto time_t_val = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::gmtime(&time_t_val), "%Y-%m-%d %H:%M:%S");

        std::string query =
            "INSERT INTO logs (worker_id, status, ts) "
            "VALUES ($1, $2, $3)";
        tx.exec_params(query, log_entry.get_worker_id(),
                       status_str,
                       ss.str());
        tx.commit();
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }

    std::cout << "Inserted log entry for worker " << log_entry.get_worker_id()
              << " with status " << to_string(log_entry.get_status())
              << " at timestamp " << log_entry.get_timestamp().time_since_epoch().count()
              << std::endl;
}

std::vector<LogEntry> query_log(pqxx::connection& db_conn,
                                const std::string& start_timestamp,
                                const std::string& end_timestamp) {
    if (start_timestamp.empty()) {
        throw std::invalid_argument("Start timestamp cannot be empty");
    }

    std::vector<LogEntry> logs;
    try {
        pqxx::work tx(db_conn);

        std::string query;
        pqxx::result res;
        if (end_timestamp.empty()) {
            query =
                "SELECT worker_id, status, ts FROM logs "
                "WHERE ts >= $1::timestamptz "
                "ORDER BY ts ASC";
            res = tx.exec_params(query, start_timestamp);
        } else {
            query =
                "SELECT worker_id, status, ts FROM logs "
                "WHERE ts >= $1::timestamptz AND ts <= $2::timestamptz "
                "ORDER BY ts ASC";
            res = tx.exec_params(query, start_timestamp, end_timestamp);
        }

        for (const auto& row : res) {
            std::string worker_id = row["worker_id"].as<std::string>();
            std::string status_str = row["status"].as<std::string>();
            std::string ts_str = row["ts"].as<std::string>();

            // Convert status string back to Status enum
            Status status;
            if (status_str == "OK") {
                status = Status::OK;
            } else if (status_str == "WARN") {
                status = Status::WARN;
            } else if (status_str == "ERROR") {
                status = Status::ERROR;
            } else {
                throw std::runtime_error("Unknown status: " + status_str);
            }

            logs.emplace_back(worker_id, status);
        }
    } catch (const std::exception& e) {
        std::cerr << "Database error: " << e.what() << std::endl;
    }

    return logs;
}