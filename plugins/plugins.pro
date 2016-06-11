isEmpty(ZTWIDGETS_INSTALL_PATH) {
    error(ZTWIDGETS_INSTALL_PATH not set!)
}

QT += uiplugin

QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS += -Werror

CONFIG += plugin

TEMPLATE = lib

TARGET = ZtWidgetsPlugin

INCLUDEPATH += ../ZtWidgets/include

CONF = release
CONFIG(debug, debug|release) {
    CONF = debug
}

LIBS += $$ZTWIDGETS_INSTALL_PATH/$$CONF/libZtWidgets.so
PRE_TARGETDEPS += $$ZTWIDGETS_INSTALL_PATH/$$CONF/libZtWidgets.so

# allow command line override of install path
# Qt Creator/Designer should be started with QT_PLUGIN_PATH set to the same thing if overridden
isEmpty(QT_INSTALL_PLUGINS) {
    QT_INSTALL_PLUGINS = $$[QT_INSTALL_PLUGINS]
}

target.path = $$QT_INSTALL_PLUGINS/designer
message($$TARGET will be installed in $$target.path)

# this works well enough for development, but for proper installs you might want to consider building ZtWidgets as part of the plugin
QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/\''
ztwidgets_copy.path = $$target.path
ztwidgets_copy.files = $$ZTWIDGETS_INSTALL_PATH/$$CONF/libZtWidgets.*

INSTALLS += target ztwidgets_copy

SOURCES += src/colorpickerplugin.cpp

HEADERS += src/colorpickerplugin.h
