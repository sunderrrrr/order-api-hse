#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "model/Order.hpp"
#include "db/Database.hpp"
#include <cstdlib>

TEST_SUITE("statusFromStr") {

    TEST_CASE("все четыре валидных статуса парсятся корректно") {
        CHECK(statusFromStr("pending")    == OrderStatus::PENDING);
        CHECK(statusFromStr("processing") == OrderStatus::PROCESSING);
        CHECK(statusFromStr("done")       == OrderStatus::DONE);
        CHECK(statusFromStr("cancelled")  == OrderStatus::CANCELLED);
    }

    TEST_CASE("неизвестная строка бросает исключение") {
        CHECK_THROWS_AS(statusFromStr("shipped"),   std::invalid_argument);
        CHECK_THROWS_AS(statusFromStr("finished"),  std::invalid_argument);
        CHECK_THROWS_AS(statusFromStr("in_progress"), std::invalid_argument);
    }

    TEST_CASE("пустая строка бросает исключения") {
        CHECK_THROWS_AS(statusFromStr(""), std::invalid_argument);
    }

    TEST_CASE("учет регистра статуса") {
        CHECK_THROWS_AS(statusFromStr("PENDING"),    std::invalid_argument);
        CHECK_THROWS_AS(statusFromStr("Done"),       std::invalid_argument);
        CHECK_THROWS_AS(statusFromStr("CANCELLED"),  std::invalid_argument);
        CHECK_THROWS_AS(statusFromStr("Processing"), std::invalid_argument);
    }

    TEST_CASE("пробелы вокруг строки не принимаются") {
        CHECK_THROWS_AS(statusFromStr(" pending"), std::invalid_argument);
        CHECK_THROWS_AS(statusFromStr("done "),   std::invalid_argument);
        CHECK_THROWS_AS(statusFromStr(" done "),  std::invalid_argument);
    }

    TEST_CASE("сообщение исключения содержит переданную строку") {
        try {
            statusFromStr("bad_status");
            FAIL("должно было бросить исключение");
        } catch (const std::invalid_argument& e) {
            CHECK(std::string(e.what()).find("bad_status") != std::string::npos);
        }
    }
}

TEST_SUITE("statusToStr") {

    TEST_CASE("все статусы дают правильные строки") {
        CHECK(statusToStr(OrderStatus::PENDING)    == "pending");
        CHECK(statusToStr(OrderStatus::PROCESSING) == "processing");
        CHECK(statusToStr(OrderStatus::DONE)       == "done");
        CHECK(statusToStr(OrderStatus::CANCELLED)  == "cancelled");
    }

    TEST_CASE("результат никогда не пустой") {
        CHECK_FALSE(statusToStr(OrderStatus::PENDING).empty());
        CHECK_FALSE(statusToStr(OrderStatus::PROCESSING).empty());
        CHECK_FALSE(statusToStr(OrderStatus::DONE).empty());
        CHECK_FALSE(statusToStr(OrderStatus::CANCELLED).empty());
    }
}

TEST_SUITE("Структура заказа") {

    TEST_CASE("поля можно записать и прочитать") {
        Order o;
        o.id          = 42;
        o.title       = "Testtttt";
        o.description = "qwerty1234";
        o.status      = OrderStatus::PENDING;
        o.created_at  = "19-05-2026 11:11:11";

        CHECK(o.id          == 42);
        CHECK(o.title       == "Testtttt");
        CHECK(o.description == "qwerty1234");
        CHECK(o.status      == OrderStatus::PENDING);
        CHECK(o.created_at  == "19-05-2026 11:11:11");
    }

    TEST_CASE("статус можно обновить") {
        Order o;
        o.status = OrderStatus::PENDING;
        CHECK(o.status == OrderStatus::PENDING);

        o.status = OrderStatus::DONE;
        CHECK(o.status == OrderStatus::DONE);
        CHECK(o.status != OrderStatus::PENDING);
    }

    TEST_CASE("описание может быть пустым") {
        Order o;
        o.description = "";
        CHECK(o.description.empty());
    }
}

// бд тесты
static std::string testConnStr() {
    std::string h = std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "localhost";
    std::string p = std::getenv("DB_PORT") ? std::getenv("DB_PORT") : "5432";
    std::string n = std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "orders";
    std::string u = std::getenv("DB_USER") ? std::getenv("DB_USER") : "postgres";
    std::string w = std::getenv("DB_PASS") ? std::getenv("DB_PASS") : "secret";
    return "host=" + h + " port=" + p +
           " dbname=" + n + " user=" + u + " password=" + w;
}

TEST_SUITE("Подключение к бд") {

    TEST_CASE("подключение с верными данными не бросает исключение") {
        CHECK_NOTHROW(Database db(testConnStr()));
    }

    TEST_CASE("подключение с неверными данными бросает runtime_error") {
        CHECK_THROWS_AS(
            Database db("host=localhost dbname=nonexistent user=nobody password=bad"),
            std::runtime_error
        );
    }

    TEST_CASE("сообщение об ошибке подключения не пустое") {
        try {
            Database db("host=localhost dbname=bad user=bad password=bad");
            FAIL("должно было бросить");
        } catch (const std::runtime_error& e) {
            CHECK_FALSE(std::string(e.what()).empty());
        }
    }
}

TEST_SUITE("Инит схемы бд") {

    TEST_CASE("initSchema выполняется без исключений") {
        Database db(testConnStr());
        CHECK_NOTHROW(db.initSchema());
    }

    TEST_CASE("повторный вызов initSchema не бросает исключение") {
        Database db(testConnStr());
        db.initSchema();
        CHECK_NOTHROW(db.initSchema());
    }
}
// тесты апи
struct CleanDb {
    Database db;

    CleanDb() : db(testConnStr()) {
        db.initSchema();
        db.clearForTesting();
    }
};

TEST_SUITE("createOrder: позитивные сценарии") {

    TEST_CASE_FIXTURE(CleanDb, "создаёт заказ с test123 и abc") {
        Order o = db.createOrder("test123", "abc");

        CHECK(o.id > 0);
        CHECK(o.title == "test123");
        CHECK(o.description == "abc");
    }

    TEST_CASE_FIXTURE(CleanDb, "новый заказ имеет статус PENDING") {
        Order o = db.createOrder("123123", "desc");
        CHECK(o.status == OrderStatus::PENDING);
    }

    TEST_CASE_FIXTURE(CleanDb, "created_at заполняется автоматически") {
        Order o = db.createOrder("tesstttt", "");
        CHECK_FALSE(o.created_at.empty());
        CHECK(o.created_at.size() >= 10);
    }

    TEST_CASE_FIXTURE(CleanDb, "id автоинкрементится для aaa bbb ccc") {
        Order o1 = db.createOrder("aaa", "");
        Order o2 = db.createOrder("bbb", "");
        Order o3 = db.createOrder("ccc", "");

        CHECK(o2.id > o1.id);
        CHECK(o3.id > o2.id);
    }

    TEST_CASE_FIXTURE(CleanDb, "description может быть пустым для test456") {
        Order o = db.createOrder("test456", "");
        CHECK(o.description.empty());
    }

    TEST_CASE_FIXTURE(CleanDb, "юникод тест123 сохраняется корректно") {
        Order o = db.createOrder("тест123", "описание");
        CHECK(o.title == "тест123");
    }

    TEST_CASE_FIXTURE(CleanDb, "спецсимволы Order'; DROP TABLE orders; -- не ломают запрос") {
        Order o = db.createOrder("Order'; DROP TABLE orders; --", "");
        CHECK(o.id > 0);
        CHECK(o.title == "Order'; DROP TABLE orders; --");
    }
}

TEST_SUITE("createOrder: негативные сценарии") {

    TEST_CASE_FIXTURE(CleanDb, "пустой title бросает invalid_argument") {
        CHECK_THROWS_AS(db.createOrder("", "desc"), std::invalid_argument);
    }

    TEST_CASE_FIXTURE(CleanDb, "после ошибки с пустым title БД остаётся рабочей") {
        CHECK_THROWS(db.createOrder("", ""));
        CHECK_NOTHROW(db.createOrder("normal123", "desc"));
    }
}

TEST_SUITE("createOrder: валидация входных данных") {

    TEST_CASE("пустая строка не проходит валидацию") {
        std::string title = "";
        bool valid = !title.empty();
        CHECK_FALSE(valid);
    }

    TEST_CASE("test789 123456 и пробелы проходят валидацию") {
        CHECK(!std::string("test789").empty());
        CHECK(!std::string("123456").empty());
        CHECK(!std::string("  ").empty());
    }
}