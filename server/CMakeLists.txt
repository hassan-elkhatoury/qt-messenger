cmake_minimum_required(VERSION 3.14)

find_package(Qt6 COMPONENTS Core Network Sql REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Network Sql REQUIRED)
endif()

set(PROJECT_SOURCES
    main.cpp
    server.cpp
    server.h
    database.cpp
    database.h
    user.cpp
    user.h
    message.cpp
    message.h
)

add_executable(QtMessengerServer ${PROJECT_SOURCES})

target_link_libraries(QtMessengerServer PRIVATE
    Qt::Core
    Qt::Network
    Qt::Sql
)