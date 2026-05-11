#pragma once

#include <string>
#include <vector>
#include <libpq-fe.h>
#include "model/Order.hpp"

/**
 * @brief Класс для работы с базой данных
 *
 * Инкапсулирует все SQL-запросы
 * Соединение открывается один раз в конструкторе
 */
class Database {
public:
    /**
     * @brief Конструктор - открывает соединение с БД
     * @param connStr Строка подключения PostgreSQL
     *        (пример "host=localhost dbname=orders user=postgres password=pass")
     * @throws std::runtime_error Если подключение не удалось
     */
    explicit Database(const std::string& connStr);

    /// @brief Деструктор - закрывает соединение
    ~Database();

    /// @brief Создаёт таблицу orders по миграциям если её нет
    /// @throws std::runtime_error при ошибке SQL
    void initSchema();

    /**
     * @brief Добавление нового заказа
     * @param title название
     * @param description описание
     * @return Созданный объект класса Order с заполненным id и created_at
     * @throws std::runtime_error при ошибке вставки
     */
    Order createOrder(const std::string& title, const std::string& description);

    /**
     * @brief Возвращает все заказы
     * @return Вектор всех заказов в БД
     */
    std::vector<Order> getAllOrders();

    /**
     * @brief Возвращает заказ по ID
     * @param id Идентификатор заказа
     * @return Найденный по ID объект класса Order
     * @throws std::runtime_error Если заказ не найден
     */
    Order getOrderById(int id);

    /**
     * @brief Возвращает заказы с заданным статусом
     * @param status статус для фильтрации
     * @return std::vector<Order> Вектор заказов
     */
    std::vector<Order> getOrdersByStatus(OrderStatus status);

    /**
     * @brief Ищет заказы по ключевому слову в title или description
     * @param keyword слово для поиска (без учёта регистра)
     * @return std::vector<Order> Вектор подходящих заказов
     */
    std::vector<Order> searchOrders(const std::string& keyword);

    /**
     * @brief Возвращает заказы, созданные в указанную дату
     * @param date Дата в формате "ДД-ММ-ГГГГ"
     * @return std::vector<Order> Вектор заказов
     */
    std::vector<Order> getOrdersByDate(const std::string& date);

    /**
     * @brief Обновляет статус заказа
     * @param id Идентификатор заказа
     * @param status Новый статус
     * @throws std::runtime_error если заказ не найден
     */
    void updateOrderStatus(int id, OrderStatus status);

    /**
     * @brief Удаляет заказ по ID
     * @param id Идентификатор заказа
     * @throws std::runtime_error если заказ не найден
     */
    void deleteOrder(int id);

private:
    PGconn* conn_; ///< Соединение с PostgreSQL

    /**
     * @brief Вспомогательный метод для парса строки результата в объект Order
     * @param res Результат запроса libpq
     * @param row Номер строки
     * @return Order
     */
    Order rowToOrder(PGresult* res, int row);

    /**
     * @brief Проверяет статус результата, бросает исключение при ошибке
     * @param res Результат запроса
     * @param expectedStatus Ожидаемый статус (PGRES_TUPLES_OK и т.п.)
     * @throws std::runtime_error Если статус не совпадает
     */
    void checkResult(PGresult* res, ExecStatusType expectedStatus);
};