#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "http/http.hpp"
#include "http/url.hpp"

void get_logs(int client_socket, const HTTPRequest& req) {
    std::string query_string = get_query_string(req.path);
    std::unordered_map<std::string, std::vector<std::string>> query_params =
        parse_query_string(query_string);

    int start;
    if (query_params.find("start") != query_params.end()) {
        start = std::stoi(query_params["start"][0]);
    } else {
        nlohmann::json res = nlohmann::json{
            {"code", 400}, {"message", "Missing 'start' parameter"}};
        HTTPHandler::send_response(client_socket, res.dump(),
                                   HTTPStatus::BAD_REQUEST,
                                   HTTPContentType::JSON);
        return;
    }

    int end=-1;
    if (query_params.find("end") != query_params.end()) {
        end = std::stoi(query_params["end"][0]);
    }

    HTTPHandler::send_response(client_socket, "Logs from " + std::to_string(start) + " to " + std::to_string(end),
                               HTTPStatus::OK, HTTPContentType::TEXT_PLAIN);
}