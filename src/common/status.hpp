#pragma once

#include <string_view>

enum class Status { OK, WARN, ERROR };

constexpr std::string_view to_string(Status status) {
    switch (status) {
        case Status::OK:
            return "OK";
        case Status::WARN:
            return "WARN";
        case Status::ERROR:
            return "ERROR";
    }
    return "UNKNOWN";
}

Status get_random_status();