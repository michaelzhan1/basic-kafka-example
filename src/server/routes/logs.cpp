#include "routes/logs.hpp"

#include <chrono>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "database.hpp"
#include "http/http.hpp"
#include "http/url.hpp"
#include "logentry.hpp"

namespace {
std::string unix_time_to_timestamp(const std::string& unix_time) {
    std::time_t time_value = static_cast<std::time_t>(std::stoll(unix_time));
    std::tm utc_time = *std::gmtime(&time_value);

    std::ostringstream output;
    output << std::put_time(&utc_time, "%Y-%m-%d %H:%M:%S");
    return output.str();
}
}  // namespace

void get_logs(int client_socket, const HTTPRequest& req) {
    std::string query_string = get_query_string(req.path);
    std::unordered_map<std::string, std::vector<std::string>> query_params =
        parse_query_string(query_string);

    std::string start_timestamp;
    if (query_params.find("start") != query_params.end()) {
        start_timestamp = unix_time_to_timestamp(query_params["start"][0]);
    } else {
        nlohmann::json res = nlohmann::json{
            {"code", 400}, {"message", "Missing 'start' parameter"}};
        HTTPHandler::send_response(client_socket, res.dump(),
                                   HTTPStatus::BAD_REQUEST,
                                   HTTPContentType::JSON);
        return;
    }

    std::string end_timestamp;
    if (query_params.find("end") != query_params.end()) {
        end_timestamp = unix_time_to_timestamp(query_params["end"][0]);
    }

    std::unique_ptr<pqxx::connection> db_conn;
    try {
        db_conn = std::make_unique<pqxx::connection>(
            "postgresql://postgres:password@pgbouncer:6432/logs_db");
    } catch (const std::exception& e) {
        nlohmann::json res = nlohmann::json{
            {"code", 500}, {"message", "Database connection error"}};

        HTTPHandler::send_response(client_socket, res.dump(),
                                   HTTPStatus::INTERNAL_SERVER_ERROR,
                                   HTTPContentType::JSON);
        return;
    }
    std::vector<LogEntry> logs =
        query_log(*db_conn, start_timestamp, end_timestamp);

    nlohmann::json res = nlohmann::json::array();
    for (const auto& log : logs) {
        res.push_back(log.to_json());
    }
    HTTPHandler::send_response(client_socket, res.dump(), HTTPStatus::OK,
                               HTTPContentType::JSON);
}