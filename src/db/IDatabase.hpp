#pragma once

#include <string>
#include <vector>

#include "model/Order.hpp"

/**
 * @brief Интерфейс для работы с базой данных
 */
class IDatabase {
   public:
    virtual ~IDatabase() = default;
    virtual void initSchema() = 0;

    virtual Order createOrder(const std::string& title, const std::string& description) = 0;

    virtual std::vector<Order> getAllOrders() = 0;

    virtual Order getOrderById(int id) = 0;

    virtual std::vector<Order> getOrdersByStatus(OrderStatus status) = 0;

    virtual std::vector<Order> searchOrders(const std::string& keyword) = 0;

    virtual std::vector<Order> getOrdersByDate(const std::string& date) = 0;

    virtual void updateOrderStatus(int id, OrderStatus status) = 0;

    virtual void deleteOrder(int id) = 0;

    virtual void clearForTesting() = 0;
};