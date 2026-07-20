#pragma once

#include <iostream>
#include <string>

#include "http/http.hpp"

void get_index(int client_socket, const HTTPRequest&);
