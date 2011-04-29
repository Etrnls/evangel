TEMPLATE = app

DESTDIR = ../../../bin/test
TARGET = bt

OBJECTS_DIR = ../../../build/test/obj
MOC_DIR = ../../../build/test/moc

CONFIG += warn_on qtestlib
CONFIG -= exceptions stl

win32 {
	CONFIG += console
}

QT -= gui

DEFINES += UNIT_TEST

linux-g++ {
	QMAKE_CXXFLAGS += -std=c++0x
} else:win32-g++ {
	QMAKE_CXXFLAGS += -std=gnu++0x
}

INCLUDEPATH += ../../../core

HEADERS += util/testbcodec.h \
           ../../../core/protocol/bt/util/bcodec.h

SOURCES += util/testbcodec.cpp \
           ../../../core/protocol/bt/util/bcodec.cpp
