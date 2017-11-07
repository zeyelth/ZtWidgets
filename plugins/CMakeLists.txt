find_package(Qt5 ${MIN_QT_VERSION}
    REQUIRED COMPONENTS
    Designer
)

# If this is not explicitly included we get the extremely helpful error "Error: Undefined interface"
include_directories(${Qt5Designer_INCLUDE_DIRS})

set(ZtWidgetsPlugin_SOURCES
    src/colorpickerplugin.cpp
    src/slidereditplugin.cpp
    src/ztwidgetsplugincollection.cpp
)

set(ZtWidgetsPlugin_HEADERS
    src/colorpickerplugin.h
    src/slidereditplugin.h
    src/ztwidgetsplugincollection.h
)

qt5_wrap_cpp(ZtWidgetsPlugin_HEADER_MOC
    ${ZtWidgetsPlugin_HEADERS}
)

add_library(ZtWidgetsPlugin MODULE ${ZtWidgetsPlugin_SOURCES} ${ZtWidgetsPlugin_HEADER_MOC})

target_include_directories(ZtWidgetsPlugin PRIVATE ${ZtWidgets_INCLUDE})

target_link_libraries(ZtWidgetsPlugin ZtWidgets)

install(TARGETS ZtWidgetsPlugin DESTINATION ${CMAKE_INSTALL_PREFIX})
