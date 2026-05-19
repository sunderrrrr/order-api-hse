#pragma once

#include "oatpp/web/server/HttpConnectionHandler.hpp"
#include "oatpp/network/tcp/server/ConnectionProvider.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/core/macro/component.hpp"
#include "db/Database.hpp"

#include <cstdlib>
#include <memory>
#include <string>

/**
 * @brief Регистрирует все зависимости приложения в DI-контейнере oatpp
 *
 * Настройки берутся из переменных окружения:
 *   DB_HOST, DB_PORT, DB_NAME, DB_USER, DB_PASS, SERVER_PORT
 */
class AppComponent {
public:
    /// @brief JSON-маппер для сериализации DTO
    OATPP_CREATE_COMPONENT(
        std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper)
    ([] {
        return oatpp::parser::json::mapping::ObjectMapper::createShared();
    }());

    /// @brief TCP-провайдер: слушает входящие соединения на заданном порту
    OATPP_CREATE_COMPONENT(
        std::shared_ptr<oatpp::network::ServerConnectionProvider>,
        serverConnectionProvider)
    ([] {
        const char* p = std::getenv("SERVER_PORT");
        v_uint16 port = p ? static_cast<v_uint16>(std::stoi(p)) : 8080;
        return oatpp::network::tcp::server::ConnectionProvider::createShared(
            {"0.0.0.0", port, oatpp::network::Address::IP_4});
    }());

    /// @brief HTTP-роутер — сюда регистрируются контроллеры
    OATPP_CREATE_COMPONENT(
        std::shared_ptr<oatpp::web::server::HttpRouter>, httpRouter)
    ([] {
        return oatpp::web::server::HttpRouter::createShared();
    }());

    /// @brief Обработчик HTTP-соединений
    OATPP_CREATE_COMPONENT(
        std::shared_ptr<oatpp::web::server::HttpConnectionHandler>,
        serverConnectionHandler)
    ([] {
        OATPP_COMPONENT(std::shared_ptr<oatpp::web::server::HttpRouter>, router);
        return oatpp::web::server::HttpConnectionHandler::createShared(router);
    }());

    /// @brief Соединение с PostgreSQL (параметры из env)
    OATPP_CREATE_COMPONENT(std::shared_ptr<Database>, database)([] {
        auto h = std::string(std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost");
        auto p = std::string(std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "5432");
        auto n = std::string(std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "orders");
        auto u = std::string(std::getenv("DB_USER") ? std::getenv("DB_USER") : "postgres");
        auto w = std::string(std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "");

        std::string conn =
            "host=" + h + " port=" + p +
            " dbname=" + n + " user=" + u + " password=" + w;

        return std::make_shared<Database>(conn);
    }());
};