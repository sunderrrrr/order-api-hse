#include <doctest/doctest.h>
#include "model/Order.hpp"
#include "db/Database.hpp"
#include <memory>
#include <stdexcept>
#include <vector>
#include <cctype>
#include <algorithm>
#include <string>

/**
 * @brief Мок-класс Database для тестирования без реальной БД
 */
class MockDatabase : public Database {
public:
    /**
     * @brief Конструктор мок-объекта (не открывает соединение с БД)
     */
    MockDatabase() : Database() {}
    
    std::vector<Order> mockOrders;
    std::vector<std::string> callLog;
    bool throwOnNextCall = false;
    std::string throwMessage;
    
    void clearForTesting() override {
        callLog.push_back("clearForTesting");
        mockOrders.clear();
    }
    
    void initSchema() override {
        callLog.push_back("initSchema");
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
    }
    
    Order createOrder(const std::string& title, const std::string& description) override {
        callLog.push_back("createOrder:" + title + ":" + description);
        
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        
        if (title.empty()) {
            throw std::invalid_argument("Title cannot be empty");
        }
        
        Order testOrder;
        testOrder.id = static_cast<int>(mockOrders.size()) + 1;
        testOrder.title = title;
        testOrder.description = description;
        testOrder.status = OrderStatus::PENDING;
        testOrder.created_at = "19-05-2026 11:11:11";
        mockOrders.push_back(testOrder);
        return testOrder;
    }
    
    std::vector<Order> getAllOrders() override {
        callLog.push_back("getAllOrders");
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        return mockOrders;
    }
    
    Order getOrderById(int id) override {
        callLog.push_back("getOrderById:" + std::to_string(id));
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        
        for (const auto& testOrder : mockOrders) {
            if (testOrder.id == id) {
                return testOrder;
            }
        }
        throw std::runtime_error("Order not found: " + std::to_string(id));
    }
    
    void deleteOrder(int id) override {
        callLog.push_back("deleteOrder:" + std::to_string(id));
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        
        for (auto it = mockOrders.begin(); it != mockOrders.end(); ++it) {
            if (it->id == id) {
                mockOrders.erase(it);
                return;
            }
        }
        throw std::runtime_error("Order not found: " + std::to_string(id));
    }
    
    void updateOrderStatus(int id, OrderStatus status) override {
        callLog.push_back("updateOrderStatus:" + std::to_string(id) + ":" + statusToStr(status));
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        
        for (auto& testOrder : mockOrders) {
            if (testOrder.id == id) {
                testOrder.status = status;
                return;
            }
        }
        throw std::runtime_error("Order not found: " + std::to_string(id));
    }
    
    std::vector<Order> getOrdersByStatus(OrderStatus status) override {
        callLog.push_back("getOrdersByStatus:" + statusToStr(status));
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        
        std::vector<Order> result;
        for (const auto& testOrder : mockOrders) {
            if (testOrder.status == status) {
                result.push_back(testOrder);
            }
        }
        return result;
    }
    
    std::vector<Order> searchOrders(const std::string& keyword) override {
        callLog.push_back("searchOrders:" + keyword);
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        
        std::vector<Order> result;
        std::string keywordLower = keyword;
        for (auto& c : keywordLower) c = static_cast<char>(std::tolower(c));
        
        for (const auto& testOrder : mockOrders) {
            std::string titleLower = testOrder.title;
            std::string descLower = testOrder.description;
            for (auto& c : titleLower) c = static_cast<char>(std::tolower(c));
            for (auto& c : descLower) c = static_cast<char>(std::tolower(c));
            
            if (titleLower.find(keywordLower) != std::string::npos ||
                descLower.find(keywordLower) != std::string::npos) {
                result.push_back(testOrder);
            }
        }
        return result;
    }
    
    std::vector<Order> getOrdersByDate(const std::string& date) override {
        callLog.push_back("getOrdersByDate:" + date);
        if (throwOnNextCall) {
            throwOnNextCall = false;
            throw std::runtime_error(throwMessage);
        }
        
        std::vector<Order> result;
        for (const auto& testOrder : mockOrders) {
            if (testOrder.created_at.substr(0, 10) == date) {
                result.push_back(testOrder);
            }
        }
        return result;
    }
};

/**
 * @brief Мок-класс для тестирования API контроллера
 */
class MockApiHandler {
private:
    std::shared_ptr<MockDatabase> testDb;
    
public:
    /**
     * @brief Структура ответа API
     */
    struct Response {
        int statusCode;
        std::string body;
        std::string error;
    };
    
    MockApiHandler() : testDb(std::make_shared<MockDatabase>()) {}
    
    Response createOrder(const std::string& title, const std::string& description) {
        try {
            if (title.empty()) {
                return {400, "", "Field 'title' is required"};
            }
            Order testOrder = testDb->createOrder(title, description);
            return {201, "Order created with id: " + std::to_string(testOrder.id), ""};
        } catch (const std::invalid_argument& e) {
            return {400, "", e.what()};
        } catch (const std::runtime_error& e) {
            return {500, "", "Internal error: " + std::string(e.what())};
        } catch (const std::exception& e) {
            return {500, "", "Internal error: " + std::string(e.what())};
        }
    }
    
    Response getOrders(const std::string& status, const std::string& keyword, const std::string& date) {
        try {
            std::vector<Order> testOrders;
            
            if (!status.empty()) {
                testOrders = testDb->getOrdersByStatus(statusFromStr(status));
            } else if (!keyword.empty()) {
                testOrders = testDb->searchOrders(keyword);
            } else if (!date.empty()) {
                testOrders = testDb->getOrdersByDate(date);
            } else {
                testOrders = testDb->getAllOrders();
            }
            
            return {200, "Found " + std::to_string(testOrders.size()) + " orders", ""};
        } catch (const std::invalid_argument& e) {
            return {400, "", e.what()};
        } catch (const std::runtime_error& e) {
            return {500, "", "Internal error: " + std::string(e.what())};
        } catch (const std::exception& e) {
            return {500, "", "Internal error: " + std::string(e.what())};
        }
    }
    
    Response getOrderById(int id) {
        try {
            Order testOrder = testDb->getOrderById(id);
            return {200, "Order: " + testOrder.title, ""};
        } catch (const std::runtime_error& e) {
            return {404, "", e.what()};
        } catch (const std::exception& e) {
            return {500, "", "Internal error: " + std::string(e.what())};
        }
    }
    
    Response deleteOrder(int id) {
        try {
            testDb->deleteOrder(id);
            return {200, "Order deleted", ""};
        } catch (const std::runtime_error& e) {
            return {404, "", e.what()};
        } catch (const std::exception& e) {
            return {500, "", "Internal error: " + std::string(e.what())};
        }
    }
    
    Response updateStatus(int id, const std::string& status) {
        try {
            if (status.empty()) {
                return {400, "", "Field 'status' is required"};
            }
            OrderStatus testStatus = statusFromStr(status);
            testDb->updateOrderStatus(id, testStatus);
            return {200, "Status updated", ""};
        } catch (const std::invalid_argument& e) {
            return {400, "", e.what()};
        } catch (const std::runtime_error& e) {
            return {404, "", e.what()};
        } catch (const std::exception& e) {
            return {500, "", "Internal error: " + std::string(e.what())};
        }
    }
    
    std::shared_ptr<MockDatabase> getDb() { return testDb; }
};

TEST_SUITE("MockDatabase - базовые операции") {

    TEST_CASE("createOrder добавляет заказ в мок-хранилище") {
        MockDatabase testDb;
        Order testOrder = testDb.createOrder("test123", "test123");
        
        CHECK(testOrder.id == 1);
        CHECK(testOrder.title == "test123");
        CHECK(testOrder.description == "test123");
        CHECK(testOrder.status == OrderStatus::PENDING);
        CHECK(testDb.getAllOrders().size() == 1);
    }

    TEST_CASE("getAllOrders возвращает все созданные заказы") {
        MockDatabase testDb;
        testDb.createOrder("test001", "test001");
        testDb.createOrder("test002", "test002");
        testDb.createOrder("test003", "test003");
        
        std::vector<Order> testOrders = testDb.getAllOrders();
        CHECK(testOrders.size() == 3);
    }

    TEST_CASE("getOrderById возвращает правильный заказ") {
        MockDatabase testDb;
        Order testFirst = testDb.createOrder("first", "test123");
        Order testSecond = testDb.createOrder("second", "test123");
        
        Order testFetched = testDb.getOrderById(testSecond.id);
        CHECK(testFetched.title == "second");
    }

    TEST_CASE("deleteOrder удаляет заказ") {
        MockDatabase testDb;
        Order testToDelete = testDb.createOrder("test123", "test123");
        CHECK(testDb.getAllOrders().size() == 1);
        
        testDb.deleteOrder(testToDelete.id);
        CHECK(testDb.getAllOrders().empty());
    }

    TEST_CASE("updateOrderStatus изменяет статус") {
        MockDatabase testDb;
        Order testOrder = testDb.createOrder("test123", "test123");
        
        testDb.updateOrderStatus(testOrder.id, OrderStatus::DONE);
        Order testUpdated = testDb.getOrderById(testOrder.id);
        CHECK(testUpdated.status == OrderStatus::DONE);
    }
}

TEST_SUITE("MockDatabase - крайние случаи") {

    TEST_CASE("getOrderById с несуществующим id бросает исключение") {
        MockDatabase testDb;
        CHECK_THROWS_AS(testDb.getOrderById(999), std::runtime_error);
    }

    TEST_CASE("deleteOrder с несуществующим id бросает исключение") {
        MockDatabase testDb;
        CHECK_THROWS_AS(testDb.deleteOrder(999), std::runtime_error);
    }

    TEST_CASE("updateOrderStatus с несуществующим id бросает исключение") {
        MockDatabase testDb;
        CHECK_THROWS_AS(testDb.updateOrderStatus(999, OrderStatus::DONE), std::runtime_error);
    }

    TEST_CASE("createOrder с пустым title бросает исключение") {
        MockDatabase testDb;
        CHECK_THROWS_AS(testDb.createOrder("", "test123"), std::invalid_argument);
    }
}

TEST_SUITE("MockDatabase - поиск и фильтрация") {

    TEST_CASE("searchOrders находит по заголовку") {
        MockDatabase testDb;
        testDb.createOrder("searchable123", "test123");
        testDb.createOrder("other", "test123");
        
        std::vector<Order> testResults = testDb.searchOrders("searchable");
        CHECK(testResults.size() == 1);
        CHECK(testResults[0].title == "searchable123");
    }

    TEST_CASE("searchOrders находит по описанию") {
        MockDatabase testDb;
        testDb.createOrder("test001", "specialKeyword");
        testDb.createOrder("test002", "test123");
        
        std::vector<Order> testResults = testDb.searchOrders("Keyword");
        CHECK(testResults.size() == 1);
    }

    TEST_CASE("searchOrders без совпадений возвращает пустой вектор") {
        MockDatabase testDb;
        testDb.createOrder("test001", "test123");
        
        std::vector<Order> testResults = testDb.searchOrders("nonexistent");
        CHECK(testResults.empty());
    }

    TEST_CASE("getOrdersByStatus фильтрует по статусу") {
        MockDatabase testDb;
        Order test1 = testDb.createOrder("test001", "");
        Order test2 = testDb.createOrder("test002", "");
        testDb.updateOrderStatus(test2.id, OrderStatus::DONE);
        
        std::vector<Order> testPending = testDb.getOrdersByStatus(OrderStatus::PENDING);
        CHECK(testPending.size() == 1);
        CHECK(testPending[0].id == test1.id);
        
        std::vector<Order> testDone = testDb.getOrdersByStatus(OrderStatus::DONE);
        CHECK(testDone.size() == 1);
        CHECK(testDone[0].id == test2.id);
    }
}

TEST_SUITE("MockDatabase - логирование вызовов") {

    TEST_CASE("каждый метод логирует свой вызов") {
        MockDatabase testDb;
        
        testDb.initSchema();
        testDb.createOrder("test123", "test123");
        testDb.getAllOrders();
        testDb.getOrderById(1);
        testDb.updateOrderStatus(1, OrderStatus::DONE);
        testDb.getOrdersByStatus(OrderStatus::DONE);
        testDb.searchOrders("test123");
        testDb.getOrdersByDate("19-05-2026");
        testDb.deleteOrder(1);
        testDb.clearForTesting();
        
        CHECK(testDb.callLog.size() == 10);
        CHECK(testDb.callLog[0] == "initSchema");
        CHECK(testDb.callLog[1] == "createOrder:test123:test123");
        CHECK(testDb.callLog[2] == "getAllOrders");
        CHECK(testDb.callLog[3] == "getOrderById:1");
        CHECK(testDb.callLog[4] == "updateOrderStatus:1:done");
        CHECK(testDb.callLog[5] == "getOrdersByStatus:done");
        CHECK(testDb.callLog[6] == "searchOrders:test123");
        CHECK(testDb.callLog[7] == "getOrdersByDate:19-05-2026");
        CHECK(testDb.callLog[8] == "deleteOrder:1");
        CHECK(testDb.callLog[9] == "clearForTesting");
    }
}

TEST_SUITE("MockApiHandler - createOrder") {

    TEST_CASE("создание заказа с валидными данными возвращает 201") {
        MockApiHandler testApi;
        MockApiHandler::Response testResp = testApi.createOrder("test123", "test123");
        
        CHECK(testResp.statusCode == 201);
        CHECK(testResp.error.empty());
        CHECK(testResp.body.find("Order created") != std::string::npos);
    }

    TEST_CASE("создание заказа с пустым заголовком возвращает 400") {
        MockApiHandler testApi;
        MockApiHandler::Response testResp = testApi.createOrder("", "test123");
        
        CHECK(testResp.statusCode == 400);
        CHECK(testResp.error == "Field 'title' is required");
    }

    TEST_CASE("создание заказа при ошибке БД возвращает 500") {
        MockApiHandler testApi;
        testApi.getDb()->throwOnNextCall = true;
        testApi.getDb()->throwMessage = "DB connection lost";
        
        MockApiHandler::Response testResp = testApi.createOrder("test123", "test123");
        CHECK(testResp.statusCode == 500);
        CHECK(testResp.error.find("Internal error") != std::string::npos);
        CHECK(testResp.error.find("DB connection lost") != std::string::npos);
    }
}

TEST_SUITE("MockApiHandler - getOrders") {

    TEST_CASE("получение всех заказов без параметров") {
        MockApiHandler testApi;
        testApi.getDb()->createOrder("test001", "");
        testApi.getDb()->createOrder("test002", "");
        
        MockApiHandler::Response testResp = testApi.getOrders("", "", "");
        CHECK(testResp.statusCode == 200);
        CHECK(testResp.body == "Found 2 orders");
    }

    TEST_CASE("фильтрация по статусу") {
        MockApiHandler testApi;
        Order testOrder = testApi.getDb()->createOrder("test123", "");
        testApi.getDb()->updateOrderStatus(testOrder.id, OrderStatus::DONE);
        
        MockApiHandler::Response testResp = testApi.getOrders("done", "", "");
        CHECK(testResp.statusCode == 200);
        CHECK(testResp.body == "Found 1 orders");
    }

    TEST_CASE("поиск по ключевому слову") {
        MockApiHandler testApi;
        testApi.getDb()->createOrder("searchable123", "");
        testApi.getDb()->createOrder("other", "");
        
        MockApiHandler::Response testResp = testApi.getOrders("", "searchable", "");
        CHECK(testResp.statusCode == 200);
        CHECK(testResp.body == "Found 1 orders");
    }

    TEST_CASE("невалидный статус возвращает 400") {
        MockApiHandler testApi;
        MockApiHandler::Response testResp = testApi.getOrders("invalid_status", "", "");
        CHECK(testResp.statusCode == 400);
        CHECK(testResp.error.find("Unknown status") != std::string::npos);
    }
}

TEST_SUITE("MockApiHandler - getOrderById") {

    TEST_CASE("существующий заказ возвращает 200") {
        MockApiHandler testApi;
        Order testOrder = testApi.getDb()->createOrder("test123", "");
        
        MockApiHandler::Response testResp = testApi.getOrderById(testOrder.id);
        CHECK(testResp.statusCode == 200);
        CHECK(testResp.body == "Order: test123");
    }

    TEST_CASE("несуществующий заказ возвращает 404") {
        MockApiHandler testApi;
        MockApiHandler::Response testResp = testApi.getOrderById(999);
        CHECK(testResp.statusCode == 404);
        CHECK(testResp.error.find("Order not found") != std::string::npos);
    }
}

TEST_SUITE("MockApiHandler - deleteOrder") {

    TEST_CASE("удаление существующего заказа возвращает 200") {
        MockApiHandler testApi;
        Order testOrder = testApi.getDb()->createOrder("test123", "");
        
        MockApiHandler::Response testResp = testApi.deleteOrder(testOrder.id);
        CHECK(testResp.statusCode == 200);
        CHECK(testResp.body == "Order deleted");
        CHECK(testApi.getDb()->getAllOrders().empty());
    }

    TEST_CASE("удаление несуществующего заказа возвращает 404") {
        MockApiHandler testApi;
        MockApiHandler::Response testResp = testApi.deleteOrder(999);
        CHECK(testResp.statusCode == 404);
        CHECK(testResp.error.find("Order not found") != std::string::npos);
    }
}

TEST_SUITE("MockApiHandler - updateStatus") {

    TEST_CASE("обновление статуса валидным значением возвращает 200") {
        MockApiHandler testApi;
        Order testOrder = testApi.getDb()->createOrder("test123", "");
        
        MockApiHandler::Response testResp = testApi.updateStatus(testOrder.id, "done");
        CHECK(testResp.statusCode == 200);
        CHECK(testResp.body == "Status updated");
        
        Order testUpdated = testApi.getDb()->getOrderById(testOrder.id);
        CHECK(testUpdated.status == OrderStatus::DONE);
    }

    TEST_CASE("пустой статус возвращает 400") {
        MockApiHandler testApi;
        Order testOrder = testApi.getDb()->createOrder("test123", "");
        
        MockApiHandler::Response testResp = testApi.updateStatus(testOrder.id, "");
        CHECK(testResp.statusCode == 400);
        CHECK(testResp.error == "Field 'status' is required");
    }

    TEST_CASE("невалидный статус возвращает 400") {
        MockApiHandler testApi;
        Order testOrder = testApi.getDb()->createOrder("test123", "");
        
        MockApiHandler::Response testResp = testApi.updateStatus(testOrder.id, "invalid");
        CHECK(testResp.statusCode == 400);
        CHECK(testResp.error.find("Unknown status") != std::string::npos);
    }

    TEST_CASE("обновление статуса несуществующего заказа возвращает 404") {
        MockApiHandler testApi;
        MockApiHandler::Response testResp = testApi.updateStatus(999, "done");
        CHECK(testResp.statusCode == 404);
    }
}

TEST_SUITE("MockApiHandler - интеграционные сценарии") {

    TEST_CASE("полный цикл жизни заказа через API") {
        MockApiHandler testApi;
        MockApiHandler::Response testCreate = testApi.createOrder("testLifecycle", "test123");
        CHECK(testCreate.statusCode == 201);
        
        std::string idStr = testCreate.body.substr(testCreate.body.find("id: ") + 4);
        int testId = std::stoi(idStr);
        
        MockApiHandler::Response testGet = testApi.getOrderById(testId);
        CHECK(testGet.statusCode == 200);
        
        MockApiHandler::Response testUpdate = testApi.updateStatus(testId, "processing");
        CHECK(testUpdate.statusCode == 200);
        
        MockApiHandler::Response testDelete = testApi.deleteOrder(testId);
        CHECK(testDelete.statusCode == 200);
        
        MockApiHandler::Response testGetAfter = testApi.getOrderById(testId);
        CHECK(testGetAfter.statusCode == 404);
    }

    TEST_CASE("создание нескольких заказов и фильтрация") {
        MockApiHandler testApi;
        testApi.createOrder("alpha", "test123");
        testApi.createOrder("beta", "test123");
        testApi.createOrder("gamma", "test123");
        
        MockApiHandler::Response testAll = testApi.getOrders("", "", "");
        CHECK(testAll.body == "Found 3 orders");
        
        MockApiHandler::Response testSearch = testApi.getOrders("", "beta", "");
        CHECK(testSearch.body == "Found 1 orders");
    }
}