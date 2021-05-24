#-------------------------------------------------
#
# Project created by QtCreator 2019-02-18T10:49:40
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
DESTDIR  = ../houses

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
        main.cpp \
        mainwindow.cpp \
    datamodel_cc.cpp

HEADERS += \
        mainwindow.h \
    connection.h \
    datamodel_cc.h

FORMS += \
        mainwindow.ui

DISTFILES +=

RESOURCES += \
    resources.qrc

RC_ICONS = $$PWD/ico/0234.ico
