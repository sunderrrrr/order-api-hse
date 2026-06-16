#include <iostream>

#include "AppComponent.hpp"
#include "controller/OrderController.hpp"
#include "oatpp/core/base/Environment.hpp"
#include "oatpp/network/Server.hpp"

int main() {
    oatpp::base::Environment::init();

    try {
        AppComponent components;

        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
        OATPP_COMPONENT(std::shared_ptr<Database>, database);
        OATPP_COMPONENT(std::shared_ptr<oatpp::network::ServerConnectionProvider>, provider);
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpConnectionHandler>, handler);

        database->initSchema();
        auto controller = OrderController::createShared(database);
        router->addController(controller);
        oatpp::network::Server server(provider, handler);

        const char* p = std::getenv("SERVER_PORT");
        std::cout << "Order API on port " << (p ? p : "8080") << std::endl;

        server.run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        oatpp::base::Environment::destroy();
        return 1;
    }

    oatpp::base::Environment::destroy();
    return 0;
}