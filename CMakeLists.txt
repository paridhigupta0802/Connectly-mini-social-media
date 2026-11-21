cmake_minimum_required(VERSION 3.16)

project(Connectly LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

set(MYSQL_ROOT
    "C:/MySQL/mysql-connector-c++-26.7.0-winx64"
    CACHE PATH "MySQL Connector/C++ installation directory"
)

qt_add_executable(Connectly

    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    auth.cpp
    SocialGraph.cpp
    suggestion.cpp
    queue.cpp
    chat.cpp

    auth.h
    SocialGraph.h
    suggestion.h
    queue.h
    chat.h
)

target_include_directories(Connectly PRIVATE

    "${CMAKE_CURRENT_SOURCE_DIR}"

    "${MYSQL_ROOT}/include"
    "${MYSQL_ROOT}/include/jdbc"
)

target_link_directories(Connectly PRIVATE
    "${MYSQL_ROOT}/lib64/vs14"
)

target_link_libraries(Connectly PRIVATE
    Qt6::Widgets
    mysqlcppconn
)

add_custom_command(TARGET Connectly POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_if_different

    "${MYSQL_ROOT}/lib64/mysqlcppconn-10-vs14.dll"

    "$<TARGET_FILE_DIR:Connectly>"
)
