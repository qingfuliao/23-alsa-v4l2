TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    AudioRender.cpp

HEADERS += \
    AudioRender.h \
    RingBuffer.h
