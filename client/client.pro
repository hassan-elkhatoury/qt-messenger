QT += core gui widgets network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtMessengerClient
TEMPLATE = app

CONFIG += c++17

SOURCES += \
    main.cpp \
    loginwindow.cpp \
    registerwindow.cpp \
    mainwindow.cpp \
    chatbubble.cpp \
    contactlistitem.cpp \
    networkclient.cpp \
    utils.cpp

HEADERS += \
    loginwindow.h \
    registerwindow.h \
    mainwindow.h \
    chatbubble.h \
    contactlistitem.h \
    networkclient.h \
    utils.h

RESOURCES += \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    resources/dark.qss \
    resources/light.qss