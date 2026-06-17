#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

#include "controller/OrderController.hpp"
#include "db/IDatabase.hpp"
#include "dto/OrderDto.hpp"
#include "model/Order.hpp"
#include "oatpp/parser/json/mapping/ObjectMapper.hpp"

/**
 * @brief Мок-класс для тестирования без реальной БД
 */
class MockDB : public IDatabase {
   private:
    std::vector<Order> orders_;
    int nextId_ = 1;

   public:
    void initSchema() override {}

    Order createOrder(const std::string& title, const std::string& description) override {
        if (title.empty()) {
            throw std::invalid_argument("Title cannot be empty");
        }

        Order o;
        o.id = nextId_++;
        o.title = title;
        o.description = description;
        o.status = OrderStatus::PENDING;
        o.created_at = "17-06-2026 12:00:00";
        orders_.push_back(o);
        return o;
    }

    std::vector<Order> getAllOrders() override { return orders_; }

    Order getOrderById(int id) override {
        for (const auto& o : orders_) {
            if (o.id == id) {
                return o;
            }
        }
        throw std::runtime_error("Order not found: " + std::to_string(id));
    }

    std::vector<Order> getOrdersByStatus(OrderStatus status) override {
        std::vector<Order> result;
        for (const auto& o : orders_) {
            if (o.status == status) {
                result.push_back(o);
            }
        }
        return result;
    }

    std::vector<Order> searchOrders(const std::string& keyword) override {
        std::vector<Order> result;
        std::string kw = keyword;
        std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);

        for (const auto& o : orders_) {
            std::string title = o.title;
            std::string desc = o.description;
            std::transform(title.begin(), title.end(), title.begin(), ::tolower);
            std::transform(desc.begin(), desc.end(), desc.begin(), ::tolower);

            if (title.find(kw) != std::string::npos || desc.find(kw) != std::string::npos) {
                result.push_back(o);
            }
        }
        return result;
    }

    std::vector<Order> getOrdersByDate(const std::string& date) override {
        std::vector<Order> result;
        for (const auto& o : orders_) {
            if (o.created_at.substr(0, 10) == date) {
                result.push_back(o);
            }
        }
        return result;
    }

    void updateOrderStatus(int id, OrderStatus status) override {
        for (auto& o : orders_) {
            if (o.id == id) {
                o.status = status;
                return;
            }
        }
        throw std::runtime_error("Order not found: " + std::to_string(id));
    }

    void deleteOrder(int id) override {
        for (auto it = orders_.begin(); it != orders_.end(); ++it) {
            if (it->id == id) {
                orders_.erase(it);
                return;
            }
        }
        throw std::runtime_error("Order not found: " + std::to_string(id));
    }

    void clearForTesting() override {
        orders_.clear();
        nextId_ = 1;
    }
};

/**
 * @brief Создает ObjectMapper для тестов
 */
std::shared_ptr<oatpp::data::mapping::ObjectMapper> createObjectMapper() {
    return std::static_pointer_cast<oatpp::data::mapping::ObjectMapper>(
        oatpp::parser::json::mapping::ObjectMapper::createShared());
}

/**
 * @brief Тесты контроллера
 */
TEST_SUITE("OrderController") {
    TEST_SUITE("createOrder") {
        TEST_CASE("создает заказ с валидными title и description") {
            auto mockDb = std::make_shared<MockDB>();
            auto objectMapper = createObjectMapper();
            auto controller = OrderController::createShared(mockDb, objectMapper);

            auto dto = OrderDto::createShared();
            dto->title = "test123";
            dto->description = "description123";

            auto response = controller->createOrder(dto);

            CHECK(response->getStatus().code == 201);

            auto orders = mockDb->getAllOrders();
            CHECK(orders.size() == 1);
            CHECK(orders[0].title == "test123");
            CHECK(orders[0].description == "description123");
            CHECK(orders[0].status == OrderStatus::PENDING);
        }

        TEST_CASE("возвращает 400 при пустом title") {
            auto mockDb = std::make_shared<MockDB>();
            auto objectMapper = createObjectMapper();
            auto controller = OrderController::createShared(mockDb, objectMapper);

            auto dto = OrderDto::createShared();
            dto->title = "";
            dto->description = "description123";

            auto response = controller->createOrder(dto);

            CHECK(response->getStatus().code == 400);
            CHECK(mockDb->getAllOrders().empty());
        }
    }

    TEST_SUITE("getOrderById") {
        TEST_CASE("возвращает заказ по существующему ID") {
            auto mockDb = std::make_shared<MockDB>();
            auto objectMapper = createObjectMapper();

            Order created = mockDb->createOrder("test123", "desc");
            auto controller = OrderController::createShared(mockDb, objectMapper);

            auto response = controller->getOrderById(created.id);

            CHECK(response->getStatus().code == 200);
        }

        TEST_CASE("возвращает 404 при несуществующем ID") {
            auto mockDb = std::make_shared<MockDB>();
            auto objectMapper = createObjectMapper();
            auto controller = OrderController::createShared(mockDb, objectMapper);

            auto response = controller->getOrderById(999);

            CHECK(response->getStatus().code == 404);
        }
    }

    TEST_SUITE("updateStatus") {
        TEST_CASE("обновляет статус на валидное значение") {
            auto mockDb = std::make_shared<MockDB>();
            auto objectMapper = createObjectMapper();

            Order created = mockDb->createOrder("test123", "desc");
            CHECK(created.status == OrderStatus::PENDING);

            auto dto = OrderDto::createShared();
            dto->status = "done";

            auto controller = OrderController::createShared(mockDb, objectMapper);
            auto response = controller->updateStatus(created.id, dto);

            CHECK(response->getStatus().code == 200);

            Order updated = mockDb->getOrderById(created.id);
            CHECK(updated.status == OrderStatus::DONE);
        }

        TEST_CASE("возвращает 404 при обновлении статуса несуществующего заказа") {
            auto mockDb = std::make_shared<MockDB>();
            auto objectMapper = createObjectMapper();

            auto dto = OrderDto::createShared();
            dto->status = "done";

            auto controller = OrderController::createShared(mockDb, objectMapper);
            auto response = controller->updateStatus(999, dto);

            CHECK(response->getStatus().code == 404);
        }

        TEST_CASE("возвращает 400 при невалидном статусе") {
            auto mockDb = std::make_shared<MockDB>();
            auto objectMapper = createObjectMapper();

            Order created = mockDb->createOrder("test123", "desc");

            auto dto = OrderDto::createShared();
            dto->status = "invalid_status";

            auto controller = OrderController::createShared(mockDb, objectMapper);
            auto response = controller->updateStatus(created.id, dto);

            CHECK(response->getStatus().code == 400);

            Order unchanged = mockDb->getOrderById(created.id);
            CHECK(unchanged.status == OrderStatus::PENDING);
        }
    }
}