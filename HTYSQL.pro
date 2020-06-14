#-------------------------------------------------
#
# Project created by QtCreator 2020-06-12T11:13:07
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYSQL
TEMPLATE = app

CONFIG += c++11

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    dialogaddconnection.cpp

HEADERS += \
        mainwindow.h \
    dialogaddconnection.h

FORMS += \
        mainwindow.ui \
    dialogaddconnection.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
