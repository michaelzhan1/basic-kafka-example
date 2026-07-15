#pragma once

#include <functional>
#include <string>
#include <unordered_map>

#include "http/http.hpp"

class Router {
   public:
    using HandlerFunc = std::function<void(int)>;

    void add_route(const std::string& path, HandlerFunc handler);
    void handle(const std::string& path, int client_socket);

   private:
    std::unordered_map<std::string, HandlerFunc> routes;
};