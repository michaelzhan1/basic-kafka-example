#include <iostream>
#include <string>
#include <librdkafka/rdkafkacpp.h>

int main() {
    std::string brokers = "kafka:9092";
    std::string topic_name = "hello-world";
    std::string message_str = "Hello, Kafka!";

    // configuration object
    std::string errstr;
    RdKafka::Conf *conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    // set bootstrap servers
    if (conf->set("bootstrap.servers", brokers, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << "Failed to set bootstrap.servers: " << errstr << std::endl;
        return 1;
    }

    // make producer
    RdKafka::Producer *producer = RdKafka::Producer::create(conf, errstr);
    if (!producer) {
        std::cerr << "Failed to create producer: " << errstr << std::endl;
        return 1;
    }
    delete conf;

    std::cout << "Created producer " << producer->name() << std::endl;

    // produce message
    RdKafka::ErrorCode resp = producer->produce(
        topic_name,
        RdKafka::Topic::PARTITION_UA, // unassigned, let kafka choose
        RdKafka::Producer::RK_MSG_COPY, // copy payload
        const_cast<char *>(message_str.c_str()), // message payload
        message_str.size(), // payload size
        nullptr, 0, // optional key and its size
        0, // timestamp (0 defaults to current time)
        nullptr // message opaque, not used here
    );

    if (resp != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to produce message: " << RdKafka::err2str(resp) << std::endl;
    } else {
        std::cout << "Produced message: " << message_str << std::endl;
    }

    // flush producer
    std::cout << "Flushing producer..." << std::endl;
    producer->flush(10000); // wait for max 10 seconds

    // clean up
    delete producer;
    return 0;
}