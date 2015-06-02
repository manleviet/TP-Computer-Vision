#-------------------------------------------------
#
# Project created by QtCreator 2010-11-28T11:20:20
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = detectColor
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp

INCLUDEPATH += "/usr/include/opencv"
LIBS += -L/usr/lib \
    -lcv \
    -lhighgui
