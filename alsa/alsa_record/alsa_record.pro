TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    AudioCapture.cpp \
    AACEncoder.cpp

HEADERS += \
    AudioCapture.h \
    RingBuffer.h \
    AACEncoder.h \
    ../third_lib/alsa/include/alsa/asoundlib.h \
    ../third_lib/alsa/include/alsa/pcm.h
