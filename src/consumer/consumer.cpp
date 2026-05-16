#include <librdkafka/rdkafkacpp.h>

#include <csignal>
#include <iostream>
#include <string>

static bool run = true;
static void sigterm(int) { run = false; }

int main() {
    std::string brokers = "kafka:9092";
    std::string topic_name = "hello-world";
    std::string group_id = "my-consumer-group";

    // signal handlers for graceful shutdown
    std::signal(SIGINT, sigterm);
    std::signal(SIGTERM, sigterm);

    std::string errstr;
    RdKafka::Conf* conf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);

    // bootstrap servers
    if (conf->set("bootstrap.servers", brokers, errstr) !=
        RdKafka::Conf::CONF_OK) {
        std::cerr << "Failed to set bootstrap.servers: " << errstr << std::endl;
        return 1;
    }

    // consumer group id
    if (conf->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK) {
        std::cerr << "Failed to set group.id: " << errstr << std::endl;
        return 1;
    }

    // offset reset policy: start from earliest if no committed offset
    if (conf->set("auto.offset.reset", "earliest", errstr) !=
        RdKafka::Conf::CONF_OK) {
        std::cerr << "Failed to set auto.offset.reset: " << errstr << std::endl;
        return 1;
    }

    // create consumer
    RdKafka::KafkaConsumer* consumer =
        RdKafka::KafkaConsumer::create(conf, errstr);
    if (!consumer) {
        std::cerr << "Failed to create consumer: " << errstr << std::endl;
        return 1;
    };
    delete conf;

    std::cout << "Created consumer " << consumer->name() << std::endl;

    // subscribe to topic
    std::vector<std::string> topics = {topic_name};
    RdKafka::ErrorCode err = consumer->subscribe(topics);
    if (err != RdKafka::ERR_NO_ERROR) {
        std::cerr << "Failed to subscribe to topics: " << RdKafka::err2str(err)
                  << std::endl;
        delete consumer;
        return 1;
    }

    std::cout << "Subscribed to topic: " << topic_name << std::endl;

    // consume messages
    while (run) {
        RdKafka::Message* msg = consumer->consume(1000);  // timeout in ms

        switch (msg->err()) {
            case RdKafka::ERR_NO_ERROR:
                std::cout << "Received message on partition "
                          << msg->partition() << " at offset " << msg->offset()
                          << ":\n"
                          << static_cast<const char*>(msg->payload())
                          << std::endl;
                break;
            case RdKafka::ERR__TIMED_OUT:
                // no message received within timeout, continue
                break;
            default:
                std::cerr << "Error consuming message: " << msg->errstr()
                          << std::endl;
                break;
        }
        delete msg;
    }

    // cleanup
    std::cout << "Closing consumer..." << std::endl;
    consumer->close();
    delete consumer;
    return 0;
}