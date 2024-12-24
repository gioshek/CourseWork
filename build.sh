#!/bin/bash

# Удаление старой папки сборки
rm -rf build

# Создание новой папки для сборки
mkdir build

# Переход в папку сборки
cd build

# Генерация файлов сборки с CMake
cmake ..

# Компиляция проекта
cmake --build .

# Запуск программы, если сборка успешна
if [ $? -eq 0 ]; then
    echo "Build successful. Running the program..."
    ./main
else
    echo "Build failed."
fi
