#include <pqxx/pqxx>

void insert_log(pqxx::connection& c, const std::string& log_message);