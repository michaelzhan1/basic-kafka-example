#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "../http.hpp"

void get_favicon(int client_socket) {
    std::string path = "./src/server/assets/favicon.ico";

    std::ifstream file(path, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        HTTPHandler::send_404(client_socket);
        return;
    }

    std::streamsize file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(file_size);
    file.read(buffer.data(), file_size);

    // 3. Construct the HTTP Headers
    std::string response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: image/x-icon\r\n"
        "Content-Length: " +
        std::to_string(file_size) +
        "\r\n"
        "Connection: close\r\n\r\n";

    // 4. Send Header then Binary Data
    send(client_socket, response.c_str(), response.size(), 0);
    send(client_socket, buffer.data(), file_size, 0);
}