QT += core gui widgets

CONFIG += c++11

TARGET = test_save_graph
TEMPLATE = app

SOURCES += \
    xy_chart_widget.cpp \
    test_save_graph.cpp

HEADERS += \
    xy_chart_widget.h

# Build settings
DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
RCC_DIR = build/rcc
UI_DIR = build/ui
