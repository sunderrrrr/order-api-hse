#include "Database.hpp"
#include <stdexcept>

Database::Database(const std::string& connStr) {
    conn_ = PQconnectdb(connStr.c_str());
    if (PQstatus(conn_) != CONNECTION_OK) {
        std::string err = PQerrorMessage(conn_);
        PQfinish(conn_);
        throw std::runtime_error("DB connection failed: " + err);
    }
}

Database::~Database() {
    if (conn_) {
        PQfinish(conn_);
    }
}

void Database::clearForTesting() {
    PGresult* res = PQexec(conn_, "TRUNCATE TABLE orders RESTART IDENTITY");
    checkResult(res, PGRES_COMMAND_OK);
    PQclear(res);
}

void Database::checkResult(PGresult* res, ExecStatusType expected) {
    if (PQresultStatus(res) != expected) {
        std::string err = PQresultErrorMessage(res);
        PQclear(res);
        throw std::runtime_error("SQL error: " + err);
    }
}

Order Database::rowToOrder(PGresult* res, int row) {
    Order o;
    o.id          = std::stoi(PQgetvalue(res, row, 0));
    o.title       = PQgetvalue(res, row, 1);
    o.description = PQgetvalue(res, row, 2);
    o.status      = statusFromStr(PQgetvalue(res, row, 3));
    o.created_at  = PQgetvalue(res, row, 4);
    return o;
}

void Database::initSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS orders (
            id          SERIAL PRIMARY KEY,
            title       TEXT NOT NULL,
            description TEXT NOT NULL DEFAULT '',
            status      TEXT NOT NULL DEFAULT 'pending',
            created_at  TIMESTAMP NOT NULL DEFAULT NOW()
        );
    )";
    PGresult* res = PQexec(conn_, sql);
    checkResult(res, PGRES_COMMAND_OK);
    PQclear(res);
}

Order Database::createOrder(const std::string& title,
                             const std::string& description) {
    if (title.empty()) {
        throw std::invalid_argument("Title cannot be empty");
    }
    
    const char* sql =
        "INSERT INTO orders (title, description) "
        "VALUES ($1, $2) "
        "RETURNING id, title, description, status, "
        "TO_CHAR(created_at, 'DD-MM-YYYY HH24:MI:SS')";

    const char* params[2] = {title.c_str(), description.c_str()};
    PGresult* res = PQexecParams(conn_, sql, 2,
                                 nullptr, params, nullptr, nullptr, 0);
    checkResult(res, PGRES_TUPLES_OK);

    Order o = rowToOrder(res, 0);
    PQclear(res);
    return o;
}

std::vector<Order> Database::getAllOrders() {
    const char* sql =
        "SELECT id, title, description, status, "
        "TO_CHAR(created_at, 'DD-MM-YYYY HH24:MI:SS') "
        "FROM orders ORDER BY id";

    PGresult* res = PQexec(conn_, sql);
    checkResult(res, PGRES_TUPLES_OK);

    std::vector<Order> orders;
    int rows = PQntuples(res);
    for (int i = 0; i < rows; ++i) {
        orders.push_back(rowToOrder(res, i));
    }
    PQclear(res);
    return orders;
}

Order Database::getOrderById(int id) {
    std::string idStr = std::to_string(id);
    const char* sql =
        "SELECT id, title, description, status, "
        "TO_CHAR(created_at, 'DD-MM-YYYY HH24:MI:SS') "
        "FROM orders WHERE id = $1";

    const char* params[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn_, sql, 1,
                                 nullptr, params, nullptr, nullptr, 0);
    checkResult(res, PGRES_TUPLES_OK);

    if (PQntuples(res) == 0) {
        PQclear(res);
        throw std::runtime_error("Order not found: " + idStr);
    }

    Order o = rowToOrder(res, 0);
    PQclear(res);
    return o;
}

void Database::deleteOrder(int id) {
    std::string idStr = std::to_string(id);
    const char* sql = "DELETE FROM orders WHERE id = $1 RETURNING id";

    const char* params[1] = {idStr.c_str()};
    PGresult* res = PQexecParams(conn_, sql, 1,
                                 nullptr, params, nullptr, nullptr, 0);
    checkResult(res, PGRES_TUPLES_OK);

    if (PQntuples(res) == 0) {
        PQclear(res);
        throw std::runtime_error("Order not found: " + idStr);
    }
    PQclear(res);
}