#-------------------------------------------------
#
# Project created by QtCreator 2010-11-25T01:50:58
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = detect
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += "/usr/include/opencv"
LIBS += -L/usr/lib \
    -lcv \
    -lhighgui
