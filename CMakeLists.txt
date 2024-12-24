cmake_minimum_required(VERSION 3.10)
project(OpenGL_ZBuffer_Project)

# Устанавливаем стандарт C++
set(CMAKE_CXX_STANDARD 17)

# Подключаем OpenGL и FreeGLUT
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

# Подключаем GLEW и GLFW
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)
pkg_check_modules(GLEW REQUIRED glew)

# Указываем директории для include файлов
include_directories(${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIR} ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
link_directories(${GLFW_LIBRARY_DIRS} ${GLEW_LIBRARY_DIRS})

# Указываем исходный файл main.cpp
add_executable(main src/main.cpp)

# Линковка с необходимыми библиотеками
target_link_libraries(main ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES} ${GLFW_LIBRARIES} dl pthread)
