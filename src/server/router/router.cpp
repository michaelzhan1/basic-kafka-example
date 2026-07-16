#include "router.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "http/http.hpp"
#include "util.hpp"

void Router::add_route(const std::string& path, HandlerFunc handler) {
    std::string trimmed_path = normalize_path(path);
    std::cout << "Adding route: " << path << " (normalized: \""
              << trimmed_path << "\")" << std::endl;
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
