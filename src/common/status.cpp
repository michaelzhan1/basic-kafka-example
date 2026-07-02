#include "status.hpp"

#include <random>

Status get_random_status() {
    double random_value = static_cast<double>(rand()) / RAND_MAX;
    Status status;
    if (random_value < 0.7) {
        status = Status::OK;
    } else if (random_value < 0.9) {
        status = Status::WARN;
    } else {
        status = Status::ERROR;
    }

    return status;
}