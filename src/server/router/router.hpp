#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "http/http.hpp"

class Router {
   public:
    using HandlerFunc = std::function<void(int, const HTTPRequest&)>;

    void add_route(const std::string& path, HandlerFunc handler);
    void handle(int client_socket, const HTTPRequest& req);

   private:
    std::unordered_map<std::string, HandlerFunc> routes;
};
