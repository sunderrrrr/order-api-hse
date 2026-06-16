#pragma once
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include OATPP_CODEGEN_BEGIN(DTO)

/**
 * @brief DTO для передачи данных заказа клиенту/от клиента
 */
class OrderDto : public oatpp::DTO {
    DTO_INIT(OrderDto, DTO)

    /// @brief ID заказа
    DTO_FIELD(Int32, id);

    /// @brief Название заказа
    DTO_FIELD(String, title);

    /// @brief Описание заказа
    DTO_FIELD(String, description);

    /// @brief Статус заказа
    DTO_FIELD(String, status);

    /// @brief Дата создания
    DTO_FIELD(String, created_at);
};

/**
 * @brief DTO для ответа со списком заказов
 */
class OrderListDto : public oatpp::DTO {
    DTO_INIT(OrderListDto, DTO)

    /// @brief Список заказов
    DTO_FIELD(List<Object<OrderDto>>, orders);
};

/**
 * @brief DTO для сообщений об ошибках/успехе
 */
class MessageDto : public oatpp::DTO {
    DTO_INIT(MessageDto, DTO)

    /// @brief Текст сообщения
    DTO_FIELD(String, message);
};

#include OATPP_CODEGEN_END(DTO)