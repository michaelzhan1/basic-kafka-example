#include "routes.hpp"

#include <string>

#include "http.hpp"

void Routes::index_route(int client_socket) {
    std::string html =
        "<html><body><h1>Welcome to the Home Page</h1></body></html>";
    HTTPHandler::send_response(client_socket, html, HTTPStatus::OK,
                               HTTPContentType::TEXT_HTML);
}