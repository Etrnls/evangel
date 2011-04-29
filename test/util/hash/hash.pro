TEMPLATE = app

DESTDIR = ../../../bin/test
TARGET = hash

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

HEADERS += testhash.h \
           ../../../core/util/hash/crc32.h \
           ../../../core/util/hash/hash.h

SOURCES += testhash.cpp \
           ../../../core/util/hash/crc32.cpp \
           ../../../core/util/hash/hash.cpp
