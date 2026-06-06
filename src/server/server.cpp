#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <poll.h>

#include <csignal>
#include <iostream>
#include <string>

static bool run = true;
static void sigterm(int) { run = false; }

int main() {
    std::signal(SIGINT, sigterm);
    std::signal(SIGTERM, sigterm);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // set socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // set address config
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // set up polling
    struct pollfd pfd;
    pfd.fd = server_fd;
    pfd.events = POLLIN;

    // start server (let up to 3 clients wait in the queue)
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_fd);
        return 1;
    }
    listen(server_fd, 3); 

    std::cout << "Server is listening on port 8080. Press Ctrl+C to stop." << std::endl;

    while (run) {
        int poll_ret = poll(&pfd, 1, 500); // wait for 0.5 seconds
        if (poll_ret < 0) {
            if (errno == EINTR) {
                continue; // interrupted by signal, check run flag again
            }
            std::cerr << "Poll error" << std::endl;
            break;
        } else if (poll_ret == 0) {
            continue; // timeout, check run flag again
        }

        int new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        // copy request
        char buffer[30000] = {0};
        read(new_socket, buffer, 30000);

        // log request
        std::cout << "Received request:\n" << buffer << std::endl;

        // built response
        std::string html = "<html><body><h1>Hello, World!</h1></body></html>";
        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: " +
            std::to_string(html.size()) +
            "\r\n"
            "\r\n" +
            html;

        // send response
        send(new_socket, response.c_str(), response.size(), 0);
        close(new_socket);
    }

    close(server_fd);
    std::cout << "Server closed gracefully" << std::endl;

    return 0;
}