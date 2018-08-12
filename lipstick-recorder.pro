TEMPLATE = app
TARGET = lipstick-recorder
VERSION = 0.0.1

target.path += /usr/bin
INSTALLS += target

CONFIG += wayland-scanner link_pkgconfig c++11
QT += gui-private network
PKGCONFIG += wayland-client
WAYLANDCLIENTSOURCES += protocol/lipstick-recorder.xml

SOURCES += \
    src/main.cpp \
    src/recorder.cpp \
    src/screenprovider.cpp

HEADERS += \
    src/recorder.h \
    src/screenprovider.h

OTHER_FILES += \
    rpm/lipstick-recorder.spec
