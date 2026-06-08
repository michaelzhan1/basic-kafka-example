#pragma once

#include <string>

std::string receive_request(int socket);

bool send_response(int socket, const std::string& response);