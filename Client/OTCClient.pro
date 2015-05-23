#-------------------------------------------------
#
# Project created by QtCreator 2015-03-04T15:22:11
#
#-------------------------------------------------

QT       += core gui

CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OTCClient
TEMPLATE = app

DEFINES += NOMINMAX


SOURCES += main.cpp\
        mainwindow.cpp \
    ClientService.cpp \
    otc_constants.cpp \
    otc_types.cpp \
    logindialog.cpp \
    orderplacedialog.cpp

HEADERS  += mainwindow.h \
    ClientService.h \
    otc_constants.h \
    otc_types.h \
    logindialog.h \
    orderplacedialog.h

FORMS    += mainwindow.ui \
    logindialog.ui \
    orderplacedialog.ui

LIBS += -lhiredis
LIBS += -lthrift
LIBS += -lthriftnb
