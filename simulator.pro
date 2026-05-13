QT += core gui widgets

CONFIG += c++17

TARGET = simulator
TEMPLATE = app

# -----------------------------------------------------------------------
# Your existing logic files — untouched
# -----------------------------------------------------------------------
SOURCES += \
    graph.cpp \
    HashMap.cpp \
    LinkedList.cpp \
    Service.cpp \
    ServiceManager.cpp \

# -----------------------------------------------------------------------
# Qt UI layer
# -----------------------------------------------------------------------
SOURCES += \
    main.cpp \
    MainWindow.cpp \
    ServiceNode.cpp \
    EdgeLine.cpp \
    AddServiceDialog.cpp \
    AddEdgeDialog.cpp \

HEADERS += \
    graph.h \
    HashMap.h \
    LinkedList.h \
    Queue.h \
    Service.h \
    ServiceManager.h \
    MainWindow.h \
    ServiceNode.h \
    EdgeLine.h \
    AddServiceDialog.h \
    AddEdgeDialog.h \
