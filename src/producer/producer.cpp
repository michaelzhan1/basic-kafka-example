#include <librdkafka/rdkafkacpp.h>
#include <unistd.h>

#include <chrono>
#include <condition_variable>
#include <csignal>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

#include "logentry.hpp"
#include "status.hpp"

std::condition_variable shutdown_cv;
std::mutex shutdown_mtx;
bool signal_received = false;

void handle_signal(int) {
    {
        std::lock_guard<std::mutex> lock(shutdown_mtx);
        signal_received = true;
    }
    shutdown_cv.notify_all();
}

std::string get_self_hostname() {
    char hostname[1024];
    gethostname(hostname, 1024);
    return std::string(hostname);
}

void producer_worker_job(std::stop_token stop_token,
                         RdKafka::Producer* shared_producer, std::string topic,
                         std::string hostname, int thread_id) {
    std::string worker_id = hostname + "-worker-" + std::to_string(thread_id);
    std::cout << "Starting producer worker: " << worker_id << std::endl;

    while (!stop_token.stop_requested()) {
        double random_value = static_cast<double>(rand()) / RAND_MAX;
        Status status;
        if (random_value < 0.7) {
            status = Status::OK;
        } else if (random_value < 0.9) {
            status = Status::WARN;
        } else {
            status = Status::ERROR;
        }

        LogEntry log(worker_id, status);
        std::string msg = log.to_json().dump();

        shared_producer->produce(
            topic,
            RdKafka::Topic::PARTITION_UA,    // unassigned, let kafka choose
            RdKafka::Producer::RK_MSG_COPY,  // copy payload
            const_cast<char*>(msg.c_str()),  // message payload
            msg.size(),                      // payload size
            nullptr, 0,                      // optional key and its size
            0,       // timestamp (0 defaults to current time)
            nullptr  // message opaque, not used here
        );

        std::cout << "[" << worker_id << "] Produced message: " << msg
                  << std::endl;

        std::unique_lock<std::mutex> lock(shutdown_mtx);
        if (shutdown_cv.wait_for(
                lock, std::chrono::milliseconds(1500 + thread_id * 100),
                [] { return signal_received; })) {
            break;  // exit loop if signal received
        }
    }
}

int main() {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    std::string brokers = "kafka:9092";
    std::string topic_name = "hello-world";
    std::string hostname = get_self_hostname();

    // configuration object
    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    // set bootstrap servers
    if (conf->set("bootstrap.servers", brokers, errstr) !=
        RdKafka::Conf::CONF_OK) {
        std::cerr << "Failed to set bootstrap.servers: " << errstr << std::endl;
        return 1;
    }

    // make producer
    RdKafka::Producer* producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        return 1;
    }
    delete conf;

    std::cout << "Created producer " << producer->name() << std::endl;

    int num_workers = 4;
    std::vector<std::jthread> worker_threads;
    for (int i = 0; i < num_workers; ++i) {
        worker_threads.emplace_back(producer_worker_job, producer, topic_name,
                                    hostname, i);
    }

    while (true) {
        {
            std::lock_guard<std::mutex> lock(shutdown_mtx);
            if (signal_received) {
                break;
            }
        }
        producer->poll(1000);  // poll for delivery reports and events
    }

    for (auto& worker : worker_threads) {
        worker.request_stop();
    }

    worker_threads.clear();  // wait for all threads to finish

    // flush producer
    std::cout << "Flushing producer..." << std::endl;
    producer->flush(10000);  // wait for max 10 seconds

    // clean up
    delete producer;
    return 0;
}