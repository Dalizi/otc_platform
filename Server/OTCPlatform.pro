#-------------------------------------------------
#
# Project created by QtCreator 2015-01-30T10:47:31
#
#-------------------------------------------------

QT       += core gui sql network
CONFIG += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OTCPlatform
TEMPLATE = app


DEFINES += NOMINMAX

SOURCES += main.cpp\
        mainwindow.cpp \
    tradetypes.cpp \
    trademanager.cpp \
    addclientdialog.cpp \
    orderplacedialog.cpp \
    accessredis.cpp \
    base_function.cpp \
    valuation_class.cpp \
    ClientService.cpp \
    otc_constants.cpp \
    otc_types.cpp \
    RPCServer.cpp \
    errmsgbase.cpp

HEADERS  += mainwindow.h \
    tradetypes.h \
    trademanager.h \
    addclientdialog.h \
    orderplacedialog.h \
    accessredis.h \
    base_function.h \
    valuation_class.h \
    ClientService.h \
    otc_constants.h \
    otc_types.h \
    RPCServer.h \
    errmsgbase.h \
    RPCAsyncServer.h

FORMS    += mainwindow.ui \
    addclientdialog.ui \
    orderplacedialog.ui

#INCLUDEPATH += ~/Downloads/redis-master/ \
#            ~/Downloads/thrift-0.9.2/lib/cpp/src \

DEPENDPATH += /usr/local/lib



unix|win32: LIBS += -lthrift

unix|win32: LIBS += -lthriftnb

unix|win32: LIBS += -lhiredis

unix|win32: LIBS += -lthriftqt5
