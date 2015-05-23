#-------------------------------------------------
#
# Project created by QtCreator 2015-05-21T14:25:27
#
#-------------------------------------------------

QT       += core gui sql network
CONFIG +=c++11
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OTC_Platform520
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    accessredis.cpp \
    addclientdialog.cpp \
    base_function.cpp \
    ClientService.cpp \
    errmsgbase.cpp \
    orderplacedialog.cpp \
    otc_constants.cpp \
    otc_types.cpp \
    RPCServer.cpp \
    trademanager.cpp \
    tradetypes.cpp \
    valuation_class.cpp

HEADERS  += mainwindow.h \
    accessredis.h \
    addclientdialog.h \
    base_function.h \
    ClientService.h \
    errmsgbase.h \
    hiredis.h \
    orderplacedialog.h \
    otc_constants.h \
    otc_types.h \
    RPCServer.h \
    trademanager.h \
    tradetypes.h \
    valuation_class.h \
    RPCAsyncServer.h

FORMS    += mainwindow.ui \
    addclientdialog.ui \
    orderplacedialog.ui

LIBS += -lhiredis\
        -lthrift\
        -lthriftnb \
        -lthriftqt5 \
        -levent
