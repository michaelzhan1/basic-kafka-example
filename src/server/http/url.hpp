#pragma once

#include <string>
#include <unordered_map>
#include <vector>

std::string normalize_path(const std::string& path);

std::string get_query_string(const std::string& url);

std::unordered_map<std::string, std::vector<std::string>> parse_query_string(const std::string& query_string);
