#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "../http/http.hpp"

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

    // send response
    HTTPHandler::send_response(client_socket, buffer, HTTPStatus::OK, HTTPContentType::IMAGE_ICON);
}