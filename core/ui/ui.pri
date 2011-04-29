DEPENDPATH += $$PWD $$PWD/telnet $$PWD/web

HEADERS += $$PWD/uimanager.h \
           $$PWD/uiserver.h \
           $$PWD/telnet/telnetserver.h \
           $$PWD/telnet/telnetsocket.h \
           $$PWD/web/webserver.h \
           $$PWD/web/webserviceauthentication.h \
           $$PWD/web/webservicestatic.h

SOURCES += $$PWD/uimanager.cpp \
           $$PWD/telnet/telnetserver.cpp \
           $$PWD/telnet/telnetsocket.cpp \
           $$PWD/web/webserver.cpp \
           $$PWD/web/webserviceauthentication.cpp \
           $$PWD/web/webservicestatic.cpp

RESOURCES += $$PWD/web/static/static.qrc
