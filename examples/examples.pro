isEmpty(ZTWIDGETS_INSTALL_PATH) {
    error(ZTWIDGETS_INSTALL_PATH not set!)
}

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -Werror

TARGET = ZtExamples
TEMPLATE = app

INCLUDEPATH += ../ZtWidgets/include

CONF = release
CONFIG(debug, debug|release) {
    CONF = debug
}

target.path = $$ZTWIDGETS_INSTALL_PATH/$$CONF
LIBS += $$target.path/libZtWidgets.so
PRE_TARGETDEPS += $$target.path/libZtWidgets.so

QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/\''

message($$TARGET will be installed in $$target.path)
INSTALLS += target

SOURCES += main.cpp \
    mainwindow.cpp

HEADERS += mainwindow.h

FORMS += mainwindow.ui
