DEPENDPATH += $$PWD $$PWD/util $$PWD/algorithm $$PWD/peer $$PWD/tracker $$PWD/dht

HEADERS += $$PWD/btmetasource.h \
           $$PWD/btmagnetsource.h \
           $$PWD/bttransfer.h \
           $$PWD/btmagnettransfer.h \
           $$PWD/btclient.h \
           $$PWD/btserver.h \
           $$PWD/util/bcodec.h \
           $$PWD/util/metainfo.h \
           $$PWD/algorithm/choker.h \
           $$PWD/algorithm/leechingchoker.h \
           $$PWD/algorithm/seedingchoker.h \
           $$PWD/algorithm/blockselector.h \
           $$PWD/algorithm/lrfselector.h \
           $$PWD/peer/peerid.h \
           $$PWD/peer/peer.h \
           $$PWD/peer/peerwiresocket.h \
           $$PWD/peer/peermanager.h \
           $$PWD/tracker/trackerclientsocket.h \
           $$PWD/tracker/trackerclient.h \
           $$PWD/tracker/trackerclientmanager.h \
           $$PWD/tracker/http/httptrackerclientsocket.h \
           $$PWD/tracker/http/httptrackerclient.h \
           $$PWD/tracker/udp/udptrackerclientsocket.h \
           $$PWD/tracker/udp/udptrackerclient.h \
           $$PWD/dht/node.h \
           $$PWD/dht/kbucket.h

SOURCES += $$PWD/btmetasource.cpp \
           $$PWD/btmagnetsource.cpp \
           $$PWD/bttransfer.cpp \
           $$PWD/btmagnettransfer.cpp \
           $$PWD/btclient.cpp \
           $$PWD/btserver.cpp \
           $$PWD/util/bcodec.cpp \
           $$PWD/util/metainfo.cpp \
           $$PWD/algorithm/leechingchoker.cpp \
           $$PWD/algorithm/seedingchoker.cpp \
           $$PWD/algorithm/blockselector.cpp \
           $$PWD/algorithm/lrfselector.cpp \
           $$PWD/peer/peerid.cpp \
           $$PWD/peer/peer.cpp \
           $$PWD/peer/peerwiresocket.cpp \
           $$PWD/peer/peermanager.cpp \
           $$PWD/tracker/trackerclient.cpp \
           $$PWD/tracker/trackerclientmanager.cpp \
           $$PWD/tracker/http/httptrackerclientsocket.cpp \
           $$PWD/tracker/http/httptrackerclient.cpp \
           $$PWD/tracker/udp/udptrackerclientsocket.cpp \
           $$PWD/tracker/udp/udptrackerclient.cpp \
           $$PWD/dht/node.cpp \
           $$PWD/dht/kbucket.cpp

