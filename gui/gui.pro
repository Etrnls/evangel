TEMPLATE = app

DESTDIR = ../bin
TARGET = evangel_gui

OBJECTS_DIR = ../build/gui/obj
MOC_DIR = ../build/gui/moc
RCC_DIR = ../build/gui/rcc

CONFIG += warn_on thread
CONFIG -= exceptions stl

win32 {
	CONFIG += console
}

QT += network xml

HEADERS += mainwindow.h

SOURCES += main.cpp \
           mainwindow.cpp
