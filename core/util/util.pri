DEPENDPATH += $$PWD $$PWD/bitmap $$PWD/hash $$PWD/numeric

HEADERS += $$PWD/endian.h \
           $$PWD/exithandler.h \
           $$PWD/humanreadable.h \
           $$PWD/settings.h \
           $$PWD/block.h \
           $$PWD/bitmap/segmentset.h \
           $$PWD/bitmap/bitmap.h \
           $$PWD/cipher/rc4.h \
           $$PWD/hash/crc32.h \
           $$PWD/hash/hash.h \
           $$PWD/numeric/random.h \
           $$PWD/numeric/integer.h \
           $$PWD/numeric/uint160.h


SOURCES += $$PWD/exithandler.cpp \
           $$PWD/humanreadable.cpp \
           $$PWD/settings.cpp \
           $$PWD/bitmap/segmentset.cpp \
           $$PWD/bitmap/bitmap.cpp \
           $$PWD/cipher/rc4.cpp \
           $$PWD/hash/crc32.cpp \
           $$PWD/hash/hash.cpp \
           $$PWD/numeric/integer.cpp \
           $$PWD/numeric/uint160.cpp
