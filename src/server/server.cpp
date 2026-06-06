#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <iostream>

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // set socket options
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // set address config
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // start server (let up to 3 clients wait in the queue)
    if (bind(server_fd, (sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(server_fd);
        return 1;
    }
    listen(server_fd, 3);

    std::cout << "Server is listening on port 8080" << std::endl;

    while (true) {
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
            "Content-Length: " + std::to_string(html.size()) + "\r\n"
            "\r\n" + html;
        
        // send response
        send(new_socket, response.c_str(), response.size(), 0);
        close(new_socket);
    }

    close(server_fd);

    return 0;
}