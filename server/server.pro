QT += core network sql
QT -= gui

TARGET = QtMessengerServer
TEMPLATE = app

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += \
    main.cpp \
    server.cpp \
    database.cpp \
    user.cpp \
    message.cpp

HEADERS += \
    server.h \
    database.h \
    user.h \
    message.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target