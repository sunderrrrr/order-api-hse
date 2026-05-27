#pragma once

#include "oatpp/web/server/api/ApiController.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

#include "dto/OrderDto.hpp"
#include "db/Database.hpp"

#include <memory>

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
    OrderController(
        std::shared_ptr<Database> db,
        std::shared_ptr<oatpp::data::mapping::ObjectMapper> objectMapper)
        : oatpp::web::server::api::ApiController(objectMapper)
        , db_(db)
    {}

    /**
     * @brief Фабричный метод — создаёт контроллер через DI oatpp
     * @param db объект базы данных
     * @return shared_ptr на контроллер
     */
    static std::shared_ptr<OrderController> createShared(
        std::shared_ptr<Database> db,
        OATPP_COMPONENT(std::shared_ptr<oatpp::data::mapping::ObjectMapper>,
                        objectMapper))
    {
        return std::make_shared<OrderController>(db, objectMapper);
    }
    /**
     * @brief Создать новый заказ
     */
    ENDPOINT("POST", "/orders", createOrder,
             BODY_DTO(Object<OrderDto>, body))
    {
        try {
            if (!body->title || body->title->length() == 0) {
                throw std::invalid_argument("Field 'title' is required");
            }

            std::string desc = (body->description)
                ? body->description->c_str() : "";

            Order o = db_->createOrder(body->title->c_str(), desc);
            Object<OrderDto> dto = orderToDto(o);
            auto response = createResponse(
                Status::CODE_201,
                getDefaultObjectMapper()->writeToString(dto));
            response->putHeader("Content-Type", "application/json");
            return response;

        } catch (const std::invalid_argument& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            auto response = createResponse(
                Status::CODE_400,
                getDefaultObjectMapper()->writeToString(msg));
            response->putHeader("Content-Type", "application/json");
            return response;

        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            auto response = createResponse(
                Status::CODE_500,
                getDefaultObjectMapper()->writeToString(msg));
            response->putHeader("Content-Type", "application/json");
            return response;
        }
    }

    /**
     * @brief Получить все заказы
     *
     * Возвращает JSON-массив всех заказов в поле "orders".
     */
    ENDPOINT("GET", "/orders", getOrders) {
        try {
            auto orders = db_->getAllOrders();

            auto list = OrderListDto::createShared();
            list->orders = {};
            for (const auto& o : orders) {
                list->orders->push_back(orderToDto(o));
            }
            return createDtoResponse(Status::CODE_200, list);

        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            return createDtoResponse(Status::CODE_500, msg);
        }
    }
    /**
     * @brief Получить заказ по ID
     *
     * Возвращает 404 если заказ не существует.
     */
    ENDPOINT("GET", "/orders/{id}", getOrderById,
            PATH(Int32, id))
    {
        try {
            Order o = db_->getOrderById(id);
            return createDtoResponse(Status::CODE_200, orderToDto(o));

        } catch (const std::runtime_error& e) {
            auto msg = MessageDto::createShared();
            msg->message = e.what();
            return createDtoResponse(Status::CODE_404, msg);
        } catch (const std::exception& e) {
            auto msg = MessageDto::createShared();
            msg->message = std::string("Internal error: ") + e.what();
            return createDtoResponse(Status::CODE_500, msg);
        }
    }
private:
    std::shared_ptr<Database> db_; ///< Объект для работы с БД

    /**
     * @brief Конвертирует Order в OrderDto
     * @param o внутренняя модель заказа
     * @return oatpp Object<OrderDto> (не shared_ptr!)
     */
    static Object<OrderDto> orderToDto(const Order& o) {
        auto dto         = OrderDto::createShared();
        dto->id          = o.id;
        dto->title       = o.title.c_str();
        dto->description = o.description.c_str();
        dto->status      = statusToStr(o.status).c_str(); 
        dto->created_at  = o.created_at.c_str();
        return dto;  
    }
};

#include OATPP_CODEGEN_END(ApiController)