#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <iostream>
#include <string>

#include "http.hpp"

/**
 * Class to handle SIGINT and SIGTERM signals for graceful shutdown of the
 * server.
 */
class SignalHandler {
   private:
    static inline std::atomic<bool> run{true};
    static void sigterm(int) { run = false; }

   public:
    static void setup() {
        std::signal(SIGINT, sigterm);
        std::signal(SIGTERM, sigterm);
    }

    static bool running() { return run.load(); }
};

class Server {
   private:
    int port_;
    int server_fd_;

   public:
    explicit Server(int port) : port_(port), server_fd_(-1) {}
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

        // start server (let up to 3 clients wait in the queue)
        if (bind(server_fd_, (sockaddr*)&address, sizeof(address)) < 0) {
            perror("Bind failed");
            close(server_fd_);
            return;
        }
        listen(server_fd_, 3);

        std::cout << "Server is listening on port " << port_
                  << ". Press Ctrl+C to stop." << std::endl;
    }

    int accept_connection() { return accept(server_fd_, nullptr, nullptr); }

    int get_fd() const { return server_fd_; }
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

            // read request
            std::string request = receive_request(new_socket);

            if (!request.empty()) {
                // log request
                std::cout << "Received request:\n"
                          << request.substr(0, 100) << "..." << std::endl;

                // build response
                std::string html =
                    "<html><body><h1>Hello, World!</h1></body></html>";
                std::string response =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: " +
                    std::to_string(html.size()) +
                    "\r\n"
                    "\r\n" +
                    html;

                // send response
                if (send_response(new_socket, response)) {
                    std::cout << "Response sent successfully" << std::endl;
                } else {
                    std::cerr << "Failed to send response" << std::endl;
                }
            }

            close(new_socket);
        }
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
    }

    std::cout << "Server closed gracefully" << std::endl;

    return 0;
}