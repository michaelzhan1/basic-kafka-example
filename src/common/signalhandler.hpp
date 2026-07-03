#pragma once

#include <atomic>
#include <csignal>

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