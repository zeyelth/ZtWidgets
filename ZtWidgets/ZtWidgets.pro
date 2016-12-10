isEmpty(ZTWIDGETS_INSTALL_PATH) {
    error(ZTWIDGETS_INSTALL_PATH not set!)
}

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -Werror

TEMPLATE = lib

TARGET = $$qtLibraryTarget($$TARGET)

INCLUDEPATH += include

DEFINES += ZTWIDGETS_LIB

CONFIG(debug, debug|release) {
    target.path = $$ZTWIDGETS_INSTALL_PATH/debug
} else {
    target.path = $$ZTWIDGETS_INSTALL_PATH/release
}

message($$TARGET will be installed in $$target.path)
INSTALLS += target

SOURCES += src/colorpicker.cpp \
    src/colorpickerpopup.cpp \
    src/colorhexedit.cpp \
    src/colordisplay.cpp \
    src/huesaturationwheel.cpp \
    src/slideredit.cpp

HEADERS += include/ZtWidgets/colorpicker.h \
    include/ZtWidgets/ztwidgets_global.h \
    include/ZtWidgets/slideredit.h \
    src/colorpickerpopup_p.h \
    src/colorhexedit_p.h \
    src/colordisplay_p.h \
    src/huesaturationwheel_p.h
