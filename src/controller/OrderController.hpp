#pragma once

#include <memory>

#include "db/IDatabase.hpp"
#include "dto/OrderDto.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

/**
 * @brief Контроллер для управления заказами
 */
class OrderController : public oatpp::web::server::api::ApiController {
   public:
    /**
     * @brief Конструктор контроллера
     * @param db объект базы данных
     * @param objectMapper JSON-маппер
     */
    OrderController(std::shared_ptr<IDatabase> db,
                    std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper), db_(db) {}

    /**
     * @brief Фабричный метод — создаёт контроллер через DI oatpp
     * @param db объект базы данных
     * @return shared_ptr на контроллер
     */
    static std::shared_ptr<OrderController> createShared(
        std::shared_ptr<IDatabase> db,
        OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>, objectMapper)) {
        return std::make_shared<OrderController>(db, objectMapper);
    }

    /**
     * @brief OPTIONS запрос для /orders
     */
    ENDPOINT("OPTIONS", "/orders", optionsOrders) {
        auto response = createResponse(Status::CODE_200, nullptr);
        response->putHeader("Access-Control-Allow-Origin", "*");
        response->putHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        response->putHeader("Access-Control-Allow-Headers", "Content-Type");
        response->putHeader("Access-Control-Max-Age", "86400");
        return response;
    }

    /**
     * @brief OPTIONS запрос для /orders/{id}
     */
    ENDPOINT("OPTIONS", "/orders/{id}", optionsOrderById) {
        auto response = createResponse(Status::CODE_200, nullptr);
        response->putHeader("Access-Control-Allow-Origin", "*");
        response->putHeader("Access-Control-Allow-Methods", "GET, DELETE, PATCH, OPTIONS");
        response->putHeader("Access-Control-Allow-Headers", "Content-Type");
        response->putHeader("Access-Control-Max-Age", "86400");
        return response;
    }

    ENDPOINT("OPTIONS", "/orders/{id}/status", optionsOrderStatus) {
        auto response = createResponse(Status::CODE_200, nullptr);
        response->putHeader("Access-Control-Allow-Origin", "*");
        response->putHeader("Access-Control-Allow-Methods", "PATCH, OPTIONS");
        response->putHeader("Access-Control-Allow-Headers", "Content-Type");
        response->putHeader("Access-Control-Max-Age", "86400");
        return response;
    }

    /**
     * @brief Создать новый заказ
     */
    ENDPOINT("POST", "/orders", createOrder, BODY_DTO(Object<OrderDto>, body)) {
        try {
            if (!body->title || body->title->length() == 0) {
                throw std::invalid_argument("Field 'title' is required");
            }

            std::string desc = (body->description) ? body->description->c_str() : "";

            Order o = db_->createOrder(body->title->c_str(), desc);
            Object<OrderDto> dto = orderToDto(o);
            auto response =
                createResponse(Status::CODE_201, getDefaultObjectMapper()->writeToString(dto));
            response->putHeader("Content-Type", "application/json");
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;

        } catch (const std::invalid_argument& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            auto response =
                createResponse(Status::CODE_400, getDefaultObjectMapper()->writeToString(msg));
            response->putHeader("Content-Type", "application/json");
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;

        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            auto response =
                createResponse(Status::CODE_500, getDefaultObjectMapper()->writeToString(msg));
            response->putHeader("Content-Type", "application/json");
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        }
    }

    /**
     * @brief Получить заказы с опциональной фильтрацией
     */
    ENDPOINT("GET", "/orders", getOrders, QUERY(String, status, "status", ""),
             QUERY(String, keyword, "keyword", ""), QUERY(String, date, "date", "")) {
        try {
            std::vector<Order> orders;

            if (status && status->length() > 0) {
                orders = db_->getOrdersByStatus(statusFromStr(status->c_str()));
            } else if (keyword && keyword->length() > 0) {
                orders = db_->searchOrders(keyword->c_str());
            } else if (date && date->length() > 0) {
                orders = db_->getOrdersByDate(date->c_str());
            } else {
                orders = db_->getAllOrders();
            }

            auto list = OrderListDto::createShared();
            list->orders = {};
            for (const auto& o : orders) {
                list->orders->push_back(orderToDto(o));
            }
            auto response = createDtoResponse(Status::CODE_200, list);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;

        } catch (const std::invalid_argument& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            auto response = createDtoResponse(Status::CODE_400, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            auto response = createDtoResponse(Status::CODE_500, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        }
    }

    /**
     * @brief Получить заказ по ID
     */
    ENDPOINT("GET", "/orders/{id}", getOrderById, PATH(Int32, id)) {
        try {
            Order o = db_->getOrderById(id);
            auto response = createDtoResponse(Status::CODE_200, orderToDto(o));
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;

        } catch (const std::runtime_error& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            auto response = createDtoResponse(Status::CODE_404, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            auto response = createDtoResponse(Status::CODE_500, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        }
    }

    /**
     * @brief Удалить заказ по ID
     */
    ENDPOINT("DELETE", "/orders/{id}", deleteOrder, PATH(Int32, id)) {
        try {
            db_->deleteOrder(id);
            auto msg = MessageDto::createShared();
            msg->message = "Order deleted";
            auto response = createDtoResponse(Status::CODE_200, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;

        } catch (const std::runtime_error& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            auto response = createDtoResponse(Status::CODE_404, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            auto response = createDtoResponse(Status::CODE_500, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        }
    }

    /**
     * @brief Изменить статус заказа
     *
     * Тело: {"status": "done"}
     * Допустимые значения: pending, processing, done, cancelled
     */
    ENDPOINT("PATCH", "/orders/{id}/status", updateStatus, PATH(Int32, id),
             BODY_DTO(Object<OrderDto>, body)) {
        try {
            if (!body->status || body->status->length() == 0) {
                throw std::invalid_argument("Field 'status' is required");
            }
            OrderStatus s = statusFromStr(body->status->c_str());
            db_->updateOrderStatus(id, s);

            auto msg = MessageDto::createShared();
            msg->message = "Status updated";
            auto response = createDtoResponse(Status::CODE_200, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;

        } catch (const std::invalid_argument& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            auto response = createDtoResponse(Status::CODE_400, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        } catch (const std::runtime_error& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            auto response = createDtoResponse(Status::CODE_404, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            auto response = createDtoResponse(Status::CODE_500, msg);
            response->putHeader("Access-Control-Allow-Origin", "*");
            return response;
        }
    }

   private:
    std::shared_ptr<IDatabase> db_;  ///< Интерфейс для работы с БД

    /**
     * @brief Конвертирует Order в OrderDto
     * @param o внутренняя модель заказа
     * @return oatpp Object<OrderDto>
     */
    static Object<OrderDto> orderToDto(const Order& o) {
        auto dto = OrderDto::createShared();
        dto->id = o.id;
        dto->title = o.title.c_str();
        dto->description = o.description.c_str();
        dto->status = statusToStr(o.status).c_str();
        dto->created_at = o.created_at.c_str();
        return dto;
    }
};

#include OATPP_CODEGEN_END(ApiController)