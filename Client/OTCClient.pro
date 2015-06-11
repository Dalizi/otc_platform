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

INCLUDEPATH += C:\Users\mystconqueror\Downloads\boost_1_58_0\boost_1_58_0



win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/x64/release/ -llibthrift
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/x64/debug/ -llibthrift

INCLUDEPATH += $$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/src
DEPENDPATH += $$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/x64/Debug

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/x64/release/ -llibthriftnb
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/x64/debug/ -llibthriftnb

INCLUDEPATH += $$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/src
DEPENDPATH += $$PWD/../../Downloads/thrift-0.9.2/thrift-0.9.2/lib/cpp/x64/Debug
