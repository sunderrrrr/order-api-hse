#pragma once

#include <libpq-fe.h>

#include <string>
#include <vector>

#include "db/IDatabase.hpp"
#include "model/Order.hpp"

/**
 * @brief Класс для работы с базой данных
 *
 * Инкапсулирует все SQL-запросы
 * Соединение открывается один раз в конструкторе
 */
class Database : public IDatabase {
   public:
    /**
     * @brief Конструктор - открывает соединение с БД
     * @param connStr Строка подключения PostgreSQL
     * @throws std::runtime_error Если подключение не удалось
     */
    explicit Database(const std::string& connStr);

    /**
     * @brief Виртуальный деструктор для корректного наследования
     */
    virtual ~Database();

    /**
     * @brief Создаёт таблицу orders по миграциям если её нет
     * @throws std::runtime_error при ошибке SQL
     */
    void initSchema() override;

    /**
     * @brief Добавление нового заказа
     * @param title название
     * @param description описание
     * @return Созданный объект класса Order
     * @throws std::invalid_argument Если title пустой
     * @throws std::runtime_error при ошибке вставки
     */
    Order createOrder(const std::string& title, const std::string& description) override;

    /**
     * @brief Возвращает все заказы
     * @return Вектор всех заказов в БД
     */
    std::vector<Order> getAllOrders() override;

    /**
     * @brief Возвращает заказ по ID
     * @param id Идентификатор заказа
     * @return Найденный по ID объект класса Order
     * @throws std::runtime_error Если заказ не найден
     */
    Order getOrderById(int id) override;

    /**
     * @brief Возвращает заказы с заданным статусом
     * @param status статус для фильтрации
     * @return std::vector<Order> Вектор заказов
     */
    std::vector<Order> getOrdersByStatus(OrderStatus status) override;

    /**
     * @brief Ищет заказы по ключевому слову в title или description
     * @param keyword слово для поиска (без учёта регистра)
     * @return std::vector<Order> Вектор подходящих заказов
     */
    std::vector<Order> searchOrders(const std::string& keyword) override;

    /**
     * @brief Возвращает заказы, созданные в указанную дату
     * @param date Дата в формате "ДД-ММ-ГГГГ"
     * @return std::vector<Order> Вектор заказов
     * @throws std::runtime_error При неверном формате даты
     */
    std::vector<Order> getOrdersByDate(const std::string& date) override;

    /**
     * @brief Обновляет статус заказа
     * @param id Идентификатор заказа
     * @param status Новый статус
     * @throws std::runtime_error если заказ не найден
     */
    void updateOrderStatus(int id, OrderStatus status) override;

    /**
     * @brief Удаляет заказ по ID
     * @param id Идентификатор заказа
     * @throws std::runtime_error если заказ не найден
     */
    void deleteOrder(int id) override;

    /**
     * @brief Очищает БД для тестов
     */
    void clearForTesting() override;

   protected:
    /**
     * @brief Защищенный конструктор для мок-объектов (не открывает соединение с БД)
     */
    Database() : conn_(nullptr) {}

    PGconn* conn_;  ///< Соединение с PostgreSQL

    /**
     * @brief Вспомогательный метод для парса строки результата в объект Order
     */
    Order rowToOrder(PGresult* res, int row);

    /**
     * @brief Проверяет статус результата
     * @throws std::runtime_error Если статус не совпадает
     */
    void checkResult(PGresult* res, ExecStatusType expectedStatus);
};