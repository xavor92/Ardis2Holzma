#-------------------------------------------------
#
# Project created by QtCreator 2014-09-26T10:28:23
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Ardis2Holzma
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    editmatdb.cpp

HEADERS  += mainwindow.h \
    editmatdb.h

FORMS    += mainwindow.ui \
    editmatdb.ui

QMAKE_CXXFLAGS += -Wall
