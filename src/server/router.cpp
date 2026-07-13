#include "router.hpp"

#include <string>

#include "http.hpp"

void Router::add_route(const std::string& path, HandlerFunc handler) {
    routes[path] = handler;
}

void Router::handle(const std::string& path, int client_socket) {
    if (routes.find(path) != routes.end()) {
        routes[path](client_socket);
    } else {
        HTTPHandler::send_404(client_socket);
    }
}