cmake_minimum_required(VERSION 3.24)
project(C_GUI_Test C)

set(CMAKE_C_STANDARD 11)
set(SOURCE_FILES main.c)


find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk4)

include_directories(${GTK_INCLUDE_DIRS})
link_directories(${GTK_LIBRARY_DIRS})

find_package(json-c CONFIG)
find_package( CURL REQUIRED )

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} PRIVATE json-c::json-c ${GTK_LIBRARIES} curl CURL::libcurl )