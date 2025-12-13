QT += core gui widgets

CONFIG += c++11

TARGET = example
TEMPLATE = app

SOURCES += \
    xy_chart_widget.cpp \
    example.cpp

HEADERS += \
    xy_chart_widget.h

# Cài đặt build
DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui
