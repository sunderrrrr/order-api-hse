# Order API

REST API для управления заказами. CRUD операции. База данных PostgreSQL. Фреймворк oat++

Выполнил: Илья Коротаев БИБ251

## Стек

- C++17
- oat++ 1.3.0
- PostgreSQL 15
- libpq
- CMake 3.15
- Conan 2.0
- Docker
- Doxygen

## Установка

Клонировать репозиторий:

```
git clone <ссылка>
cd order-api-hse
```

Собрать через Docker (рекомендуется):

```
docker-compose up --build
```

После сборки API доступно на http://localhost:8080

Собрать локально без Docker:

```
conan install . --output-folder=build --build=missing -s build_type=Release -s compiler.libcxx=libstdc++11
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/Release/order_api
```
Или запустите (для NIX систем)
```
./build.sh
```
## Переменные окружения

| Переменная | Описание | Пример |
|------------|----------|--------------|
| DB_HOST | Хост базы данных | localhost |
| DB_PORT | Порт базы данных | 5432 |
| DB_NAME | Имя базы данных | orders |
| DB_USER | Пользователь базы данных | postgres |
| DB_PASS | Пароль базы данных | secret |
| SERVER_PORT | Порт сервера | 8080 |

## Запуск тестов после сборки проекта

```
./build/tests
```

## Генерация документации

```
cd docs
doxygen Doxyfile
```

Документация будет в папке docs/html/

## Структура проекта

```
src/
  controller/   # обработчики HTTP запросов
  db/           # работа с PostgreSQL
  dto/          # объекты передачи данных
  model/        # модели данных
test/           # тесты
docs/           # сгенерированная документация
Dockerfile      # контейнер приложения
docker-compose.yml # оркестрация контейнеров
CMakeLists.txt  # сборка проекта
conanfile.txt   # зависимости
```

## Эндпоинты

| Метод | Путь | Описание |
|-------|------|----------|
| POST | /orders | Создание заказа |
| GET | /orders | Список заказов с фильтрацией |
| GET | /orders/{id} | Получение заказа по id |
| PATCH | /orders/{id}/status | Обновление статуса |
| DELETE | /orders/{id} | Удаление заказа |

## Фильтрация GET /orders

- status - pending, processing, done, cancelled
- keyword - поиск по тексту в title и description
- date - фильтр по дате в формате DD-MM-YYYY

## Пример запроса

POST /orders

```
{
  "title": "Заказ 1",
  "description": "Описание заказа"
}
```

Ответ:

```
{
  "id": 1,
  "title": "Заказ 1",
  "description": "Описание заказа",
  "status": "pending",
  "created_at": "17-06-2026 12:00:00"
}
```