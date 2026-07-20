#include "router.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "http/http.hpp"
#include "http/url.hpp"
#include "util.hpp"

void Router::add_route(const std::string& path, HandlerFunc handler) {
    std::string trimmed_path = normalize_path(path);
    std::cout << "Adding route: " << path << " (normalized: \"" << trimmed_path
              << "\")" << std::endl;
    routes[trimmed_path] = handler;
}

void Router::handle(int client_socket, const HTTPRequest& req) {
    std::string trimmed_path = normalize_path(req.path);
    if (routes.find(trimmed_path) != routes.end()) {
        routes[trimmed_path](client_socket, req);
    } else {
        HTTPHandler::send_404(client_socket);
    }
}
