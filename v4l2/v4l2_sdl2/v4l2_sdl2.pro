TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    v4l2.c

HEADERS += \
    v4l2.h \
    videodev2.h
