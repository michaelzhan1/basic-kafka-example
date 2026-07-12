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

#include "http.hpp"
#include "signalhandler.hpp"
#include "threadpool.hpp"

/**
 * Class representing the HTTP server. It abstracts out setup and accepting and
 * handling connections.
 */
class Server {
   private:
    int port_;
    int server_fd_;

   public:
    /**
     * Constructor
     */
    explicit Server(int port) : port_(port), server_fd_(-1) {}

    /**
     * Destructor
     */
    ~Server() {
        if (server_fd_ != -1) {
            close(server_fd_);
        }
    }

    /**
     * Setup
     */
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

    /**
     * Accept a new connection
     */
    int accept_connection() { return accept(server_fd_, nullptr, nullptr); }

    int get_fd() const { return server_fd_; }

    static void handle_client(int client_socket) {
        // read request
        std::string request = HTTPHandler::receive_request(client_socket);

        if (!request.empty()) {
            // log request
            std::osyncstream(std::cout)
                << "Received request:\n"
                << request.substr(0, 100) << "..." << std::endl;

            // build response
            std::string html =
                "<html><body><h1>Hello, World!</h1></body></html>";

            // send response
            HTTPHandler::send_response(client_socket, html, HTTPStatus::OK,
                          HTTPContentType::TEXT_HTML);
        }

        close(client_socket);
    }
};

int main() {
    SignalHandler::setup();

    try {
        Server server(8080);
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
                [new_socket]() { Server::handle_client(new_socket); });
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    std::cout << "Server closed gracefully" << std::endl;

    return 0;
}