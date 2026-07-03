#include <librdkafka/rdkafkacpp.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <syncstream>

#include "logentry.hpp"
#include "signalhandler.hpp"
#include "status.hpp"
#include "threadpool.hpp"
#include "util.hpp"

std::string get_self_hostname() {
    char hostname[1024];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        return "unknown";
    }
    return std::string(hostname);
}

void start_service(ThreadPool& pool, RdKafka::Producer* producer,
                   const std::string& topic, const std::string& service_id) {
    auto work_loop = [producer, topic, service_id, &pool]() -> void {
        Status status = get_random_status();
        LogEntry log(service_id, status);

        std::string msg = log.to_json().dump();
        RdKafka::ErrorCode err = producer->produce(
            topic,
            RdKafka::Topic::PARTITION_UA,    // unassigned, let kafka choose
            RdKafka::Producer::RK_MSG_COPY,  // copy payload
            const_cast<char*>(msg.data()),   // message payload
            msg.size(),                      // payload size
            nullptr, 0,                      // optional key and its size
            0,       // timestamp (0 defaults to current time)
            nullptr  // message opaque, not used here
        );
        if (err != RdKafka::ERR_NO_ERROR) {
            std::osyncstream(std::cerr)
                << "Failed to produce message: " << RdKafka::err2str(err)
                << '\n';
        }

        std::osyncstream(std::cout) << "Produced message: " << msg << std::endl;

        random_sleep(
            500,
            2000);  // sleep for a random duration between 0.5 and 2 seconds

        if (SignalHandler::running()) {
            pool.enqueue(
                start_service, std::ref(pool), producer, topic,
                service_id);  // re-enqueue the task for continuous operation
        }
    };

    pool.enqueue(work_loop);
}

int main() {
    SignalHandler::setup();

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

    // thread pool
    ThreadPool thread_pool(4);
    std::cout << "Starting producer workers..." << std::endl;

    for (int i = 0; i < 4; i++) {
        start_service(thread_pool, producer, topic_name,
                      hostname + "-worker-" + std::to_string(i));
    }

    while (SignalHandler::running()) {
        producer->poll(1000);  // poll for delivery reports and events
    }

    // flush producer
    std::cout << "Flushing producer..." << std::endl;
    producer->flush(10000);  // wait for max 10 seconds

    // clean up
    delete producer;
    return 0;
}