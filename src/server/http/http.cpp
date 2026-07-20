#include "http.hpp"

#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

#include "util.hpp"

namespace {
std::string http_status_to_string(HTTPStatus status) {
    switch (status) {
        case OK:
            return "200 OK";
        case BAD_REQUEST:
            return "400 Bad Request";
        case NOT_FOUND:
            return "404 Not Found";
        case INTERNAL_SERVER_ERROR:
            return "500 Internal Server Error";
        default:
            return "500 Internal Server Error";  // default to internal server
                                                 // error
    }
}

std::string http_content_type_to_string(HTTPContentType content_type) {
    switch (content_type) {
        case TEXT_HTML:
            return "text/html";
        case TEXT_PLAIN:
            return "text/plain";
        case JSON:
            return "application/json";
        case IMAGE_ICON:
            return "image/x-icon";
        default:
            return "text/plain";  // default to plain text
    }
}
}  // namespace

std::string HTTPHandler::receive_request(int socket) {
    std::string request;
    char buffer[4096];

    // set 5 second timeout for receiving data
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    while (true) {
        ssize_t bytes_read = read(socket, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                std::cerr << "Request timed out" << std::endl;
            } else {
                perror("Read error");
            }
            return "";
        } else if (bytes_read == 0) {
            // End of stream
            break;
        }

        buffer[bytes_read] = '\0';  // null-terminate the buffer
        request.append(buffer, bytes_read);

        if (request.find("\r\n\r\n") != std::string::npos) {
            // End of HTTP headers
            break;
        }
    }

    return request;
}

HTTPRequest HTTPHandler::parse_request(const std::string& request) {
    std::istringstream request_stream(request);
    std::string line;

    if (std::getline(request_stream, line)) {
        std::istringstream line_stream(line);
        HTTPRequest http_request;

        if (line_stream >> http_request.method >> http_request.path) {
            return http_request;
        }
    }

    return HTTPRequest{};  // return empty request on failure
}

bool HTTPHandler::send_response(int socket, const std::string& msg,
                                HTTPStatus status,
                                HTTPContentType content_type) {
    std::vector<char> data(msg.begin(), msg.end());
    return send_response(socket, data, status, content_type);
}

bool HTTPHandler::send_response(int socket, const std::vector<char>& msg,
                                HTTPStatus status,
                                HTTPContentType content_type) {
    std::string headers = "HTTP/1.1 " + http_status_to_string(status) + "\r\n";
    headers +=
        "Content-Type: " + http_content_type_to_string(content_type) + "\r\n";
    headers += "Content-Length: " + std::to_string(msg.size()) + "\r\n";
    headers += "Access-Control-Allow-Origin: *\r\n";  // CORS header
    headers += "\r\n";  // end of headers

    std::vector<char> packet(headers.begin(), headers.end());
    packet.insert(packet.end(), msg.begin(), msg.end());

    ssize_t total_sent = 0;
    ssize_t packet_length = packet.size();
    while (total_sent < packet_length) {
        ssize_t bytes_sent = send(socket, packet.data() + total_sent,
                                  packet_length - total_sent, 0);

        if (bytes_sent < 0) {
            perror("Send error");
            return false;
        }

        total_sent += bytes_sent;
    }

    return true;
}

bool HTTPHandler::send_404(int socket) {
    std::string html = "<html><body><h1>404 Not Found</h1></body></html>";
    return send_response(socket, html, HTTPStatus::NOT_FOUND,
                         HTTPContentType::TEXT_HTML);
}
