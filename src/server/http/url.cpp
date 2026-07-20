#include "http/url.hpp"

#include <sstream>
#include <string>

std::string normalize_path(const std::string& path) {
    std::stringstream ss(path);
    std::string root_path;

    if (std::getline(ss, root_path, '?')) {
        if (root_path[root_path.size() - 1] == '/') {
            root_path.pop_back();  // remove trailing slash
        }
        return root_path;
    }

    return path;
}

std::string get_query_string(const std::string& url) {
    std::stringstream ss(url);
    std::string query_string;

    if (std::getline(ss, query_string, '?')) {
        if (std::getline(ss, query_string)) {
            return query_string;
        }
    }

    return "";
}

std::unordered_map<std::string, std::vector<std::string>> parse_query_string(const std::string& query_string) {
    std::unordered_map<std::string, std::vector<std::string>> query_params;
    std::stringstream ss(query_string);
    std::string param;

    while (std::getline(ss, param, '&')) {
        size_t pos = param.find('=');
        if (pos != std::string::npos) {
            std::string key = param.substr(0, pos);
            std::string value = param.substr(pos + 1);
            query_params[key].push_back(value);
        } else {
            query_params[param].push_back("");  // Handle parameters without values
        }
    }

    return query_params;
}
