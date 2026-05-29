#!/bin/bash

./build.sh

if [ $? -ne 0 ]; then
    echo "Ошибка: сборка не удалась"
    exit 1
fi

echo "Сборка завершена успешно"

cd build || exit 1

if [ ! -f "./order_api" ]; then
    echo "Ошибка: файл order_api не найден в папке build"
    exit 1
fi

echo "Запуск..."

DB_HOST=localhost \
DB_PORT=5432 \
DB_NAME=orders \
DB_USER=postgres \
DB_PASS=secret \
./order_api