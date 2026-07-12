#pragma once

#include <string>

enum HTTPContentType { TEXT_HTML = 0, TEXT_PLAIN = 1, APPLICATION_JSON = 2 };

enum HTTPStatus {
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500
};

class HTTPHandler {
   public:
    static std::string receive_request(int socket);

    static bool send_response(int socket, const std::string& response,
                              HTTPStatus status, HTTPContentType content_type);
};
