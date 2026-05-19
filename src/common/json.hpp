#pragma once

#include <string>

#include "status.hpp"

std::string generate_json_message(const std::string& worker_id, Status status);