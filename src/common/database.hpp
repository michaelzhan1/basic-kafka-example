#pragma once

#include <pqxx/pqxx>

#include "logentry.hpp"

void insert_log(pqxx::connection& db_conn, const LogEntry& log_entry);