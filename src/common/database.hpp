#pragma once

#include <pqxx/pqxx>
#include <string>

#include "logentry.hpp"

void insert_log(pqxx::connection& db_conn, const LogEntry& log_entry);
std::vector<LogEntry> query_log(pqxx::connection& db_conn,
								const std::string& start_timestamp,
								const std::string& end_timestamp);