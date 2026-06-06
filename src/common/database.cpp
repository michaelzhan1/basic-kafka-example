#include "database.hpp"

#include <iostream>
#include <pqxx/pqxx>
#include <string>

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