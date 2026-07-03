#include <librdkafka/rdkafkacpp.h>

#include <csignal>
#include <iostream>
#include <nlohmann/json.hpp>
#include <pqxx/pqxx>
#include <string>
#include <thread>

#include "database.hpp"
#include "logentry.hpp"
#include "signalhandler.hpp"

int main() {
    SignalHandler::setup();
    
    std::string brokers = "kafka:9092";
    std::string topic_name = "hello-world";
    std::string group_id = "my-consumer-group";

    // configuration object
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

    // offset reset policy: only read new messages
    if (conf->set("auto.offset.reset", "latest", errstr) !=
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

    // set up database connection
    std::unique_ptr<pqxx::connection> db_conn;
    try {
        db_conn = std::make_unique<pqxx::connection>(
            "postgresql://postgres:password@pgbouncer:6432/logs_db");
    } catch (const std::exception& e) {
        std::cerr << "Database connection error: " << e.what() << std::endl;
        consumer->close();
        delete consumer;
        return 1;
    }

    while (SignalHandler::running()) {
        if (!db_conn) {
            try {
                std::cout << "Attempting to reconnect to database..."
                          << std::endl;
                db_conn = std::make_unique<pqxx::connection>(
                    "postgresql://postgres:password@pgbouncer:6432/logs_db");
            } catch (const std::exception& e) {
                std::cerr << "Reconnection failed: " << e.what()
                          << ". Retrying in 5s..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(5));
                continue;  // Skip the rest of the loop and try again
            }
        }

        RdKafka::Message* msg = consumer->consume(1000);  // 1000 ms

        switch (msg->err()) {
            case RdKafka::ERR_NO_ERROR: {
                // receive message
                std::string payload(static_cast<const char*>(msg->payload()),
                                    msg->len());
                std::cout << "Received message: " << payload << std::endl;

                // parse message
                nlohmann::json payload_json;
                try {
                    payload_json = nlohmann::json::parse(payload);
                } catch (const nlohmann::json::parse_error& e) {
                    std::cerr << "JSON parse error: " << e.what()
                              << " Payload: " << payload << std::endl;
                    break;  // skip this message
                }

                // insert into database
                try {
                    insert_log(*db_conn, LogEntry(payload_json));
                } catch (const pqxx::broken_connection& e) {
                    std::cerr << "Database connection lost: " << e.what()
                              << std::endl;
                    db_conn.reset();
                } catch (const std::exception& e) {
                    std::cerr << "Insert failed: " << e.what() << std::endl;
                }
                break;
            }
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