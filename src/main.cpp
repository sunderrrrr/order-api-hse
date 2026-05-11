#include <iostream>
#include <cstdlib>
#include "db/Database.hpp"

int main() {
    try {
        auto host = std::string(std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost");
        auto port = std::string(std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "5432");
        auto name = std::string(std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "orders");
        auto user = std::string(std::getenv("DB_USER") ? std::getenv("DB_USER") : "postgres");
        auto pass = std::string(std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "");
        if (pass == "") {
            std::cerr << "DB pass is empty" << std::endl;
        }
        std::string conn =
            "host=" + host + " port=" + port +
            " dbname=" + name + " user=" + user + " password=" + pass;

        Database db(conn);
        db.initSchema();
        std::cout << "DB connected, schema ready" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}