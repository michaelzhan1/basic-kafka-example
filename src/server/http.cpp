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