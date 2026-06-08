#include "http.hpp"

#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <string>

std::string receive_request(int socket) {
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

bool send_response(int socket, const std::string& response) {
    ssize_t total_sent = 0;
    ssize_t response_length = response.size();

    while (total_sent < response_length) {
        ssize_t bytes_sent =
            send(socket, response.c_str() + total_sent, response_length - total_sent, 0);

        if (bytes_sent < 0) {
            perror("Send error");
            return false;
        }

        total_sent += bytes_sent;
    }

    return true;
}