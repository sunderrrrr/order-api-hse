#pragma once

#include <stdexcept>
#include <string>

/// @brief Статусы, в которых может находиться заказ
enum class OrderStatus {
    PENDING,     ///< Ожидает обработки
    PROCESSING,  ///< В работе
    DONE,        ///< Выполнен
    CANCELLED    ///< Отменён
};

/**
 * @brief Преобразует строку в OrderStatus
 * @param s Строка (например, "pending")
 * @return Соответствующий OrderStatus
 * @throws std::invalid_argument Если строка не распознана
 */
inline OrderStatus statusFromStr(const std::string& s) {
    if (s == "pending") return OrderStatus::PENDING;
    if (s == "processing") return OrderStatus::PROCESSING;
    if (s == "done") return OrderStatus::DONE;
    if (s == "cancelled") return OrderStatus::CANCELLED;
    throw std::invalid_argument("Unknown status: " + s);
}

/**
 * @brief Преобразует OrderStatus в строку
 * @param status Статус заказа
 * @return Статус в формате строки
 */
inline std::string statusToStr(OrderStatus status) {
    switch (status) {
        case OrderStatus::PENDING:
            return "pending";
        case OrderStatus::PROCESSING:
            return "processing";
        case OrderStatus::DONE:
            return "done";
        case OrderStatus::CANCELLED:
            return "cancelled";
    }
    return "unknown";
}

/**
 * @brief Внутренняя модель заказа
 */
struct Order {
    int id;                   ///< Уникальный идентификатор
    std::string title;        ///< Название заказа
    std::string description;  ///< Описание заказа
    OrderStatus status;       ///< Текущий статус
    std::string created_at;   ///< Дата создания
};