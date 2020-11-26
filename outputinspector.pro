#-------------------------------------------------
#
# Project created by QtCreator 2017-03-25T20:10:00
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = outputinspector
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        settings.cpp

HEADERS  += mainwindow.h \
    settings.h

FORMS    += mainwindow.ui
#Windows (R) application icon:
RC_ICONS = outputinspector.ico
#Mac OS (R) application icon:
ICON =

DISTFILES += \
    outputinspector.ico
