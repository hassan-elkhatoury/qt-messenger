cmake_minimum_required(VERSION 3.14)

find_package(Qt6 COMPONENTS Core Gui Widgets Network Sql REQUIRED)
if (NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Gui Widgets Network Sql REQUIRED)
endif()

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    loginwindow.cpp
    loginwindow.h
    registerwindow.cpp
    registerwindow.h
    chatwindow.cpp
    chatwindow.h
    chatbubble.cpp
    chatbubble.h
    contactlistitem.cpp
    contactlistitem.h
    networkclient.cpp
    networkclient.h
    utils.cpp
    utils.h
    resources.qrc
)

add_executable(QtMessengerClient ${PROJECT_SOURCES})

target_link_libraries(QtMessengerClient PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::Network
    Qt::Sql
)