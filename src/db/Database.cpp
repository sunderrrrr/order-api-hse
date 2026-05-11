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