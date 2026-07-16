#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <string>
#include <syncstream>

#include "http/http.hpp"
#include "router/router.hpp"
#include "routes/favicon.hpp"
#include "routes/index.hpp"
#include "routes/logs.hpp"
#include "signalhandler.hpp"
#include "threadpool.hpp"

class Server {
   private:
    int port_;
    int server_fd_;
    Router& router_;

   public:
    explicit Server(int port, Router& r)
        : port_(port), server_fd_(-1), router_(r) {}
    ~Server() {
        if (server_fd_ != -1) {
            close(server_fd_);
        }
    }

    void start() {
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            perror("Socket creation failed");
            return;
        }

        // set socket options
        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        // set address config
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);

        // start server (let up to 128 clients wait in the queue)
        if (bind(server_fd_, (sockaddr*)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            close(server_fd_);
            return;
        }
        listen(server_fd_, 128);

        std::cout << "Server is listening on port " << port_
                  << ". Press Ctrl+C to stop." << std::endl;
    }

    int accept_connection() { return accept(server_fd_, nullptr, nullptr); }

    int get_fd() const { return server_fd_; }

    void handle_client(int client_socket) {
        // read request
        std::string request = HTTPHandler::receive_request(client_socket);
        if (request.empty()) {
            std::cerr << "Failed to receive request or request timed out - "
                         "empty request"
                      << std::endl;
            close(client_socket);
            return;
        }

        // parse request
        HTTPRequest http_request = HTTPHandler::parse_request(request);

        // handle request
        router_.handle(client_socket, http_request);

        // log request
        std::osyncstream(std::cout)
            << "Received request:\n"
            << request.substr(0, 100) << "..." << std::endl;

        std::osyncstream(std::cout)
            << "Parsed request: method=" << http_request.method
            << ", path=" << http_request.path << std::endl;

        close(client_socket);
    }
};

int main() {
    SignalHandler::setup();

    Router router;
    router.add_route("/", get_index);
    router.add_route("/favicon.ico", get_favicon);
    router.add_route("/logs", get_logs);

    try {
        Server server(8080, router);
        server.start();

        // set up polling
        struct pollfd pfd;
        pfd.fd = server.get_fd();
        pfd.events = POLLIN;

        // set up workers to handle clients
        ThreadPool thread_pool(4);  // 4 worker threads

        while (SignalHandler::running()) {
            int poll_ret = poll(&pfd, 1, 500);  // wait for 0.5 seconds
            if (poll_ret < 0) {
                if (errno == EINTR) {
                    continue;  // interrupted by signal, check run flag again
                }
                std::cerr << "Poll error" << std::endl;
                break;
            } else if (poll_ret == 0) {
                continue;  // timeout, check run flag again
            }

            int new_socket = server.accept_connection();
            if (new_socket < 0) {
                continue;  // error accepting connection, try again
            }

            thread_pool.enqueue(
                [new_socket, &server]() { server.handle_client(new_socket); });
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    std::cout << "Server closed gracefully" << std::endl;

    return 0;
}