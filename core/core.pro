TEMPLATE = app

DESTDIR = ../bin
TARGET = evangel

OBJECTS_DIR = ../build/core/obj
MOC_DIR = ../build/core/moc
RCC_DIR = ../build/core/rcc

CONFIG += warn_on thread qxt
CONFIG -= exceptions stl

win32 {
	CONFIG += console
}

QT -= gui
QT += network xml
QXT += web

DEFINES += QT_NO_CAST_FROM_BYTEARRAY \
	QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

linux-g++ {
	QMAKE_CXXFLAGS += -std=c++0x
} else:win32-g++ {
	QMAKE_CXXFLAGS += -std=gnu++0x
}
QMAKE_CXXFLAGS += -pg
QMAKE_LFLAGS += -pg

include(util/util.pri)
include(future/future.pri)
include(disk/disk.pri)
include(network/network.pri)
include(ui/ui.pri)
include(protocol/protocol.pri)

INCLUDEPATH = $$PWD

HEADERS += singleton.h log.h console.h \
           abstractsource.h abstracttransfer.h \
           abstracttask.h filetask.h \
           taskmanager.h

SOURCES += main.cpp log.cpp console.cpp \
           abstractsource.cpp \
           abstracttask.cpp filetask.cpp \
           taskmanager.cpp

