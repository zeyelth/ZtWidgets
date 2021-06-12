add_definitions(-DZTWIDGETS_LIB)


set(ZtWidgets_SOURCES
    src/colorpicker.cpp
    src/colorpickerpopup.cpp
    src/colorhexedit.cpp
    src/colordisplay.cpp
    src/color_utils.cpp
    src/huesaturationwheel.cpp
    src/slideredit.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(ZtWidgets_HEADERS
    include/ZtWidgets/colorpicker.h
    include/ZtWidgets/slideredit.h
    include/ZtWidgets/ztwidgets_global.h
    src/colordisplay_p.h
    src/colorhexedit_p.h
    src/colorpickerpopup_p.h
    src/color_utils_p.h
    src/huesaturationwheel_p.h
)

qt5_wrap_cpp(ZtWidgets_HEADER_MOC
    ${ZtWidgets_HEADERS}
)

# It's not necessary to include the headers for compilation, but if they're not included the Qt Creator editor won't resolve definitions properly
add_library(ZtWidgets SHARED ${ZtWidgets_SOURCES} ${ZtWidgets_HEADER_MOC} ${ZtWidgets_HEADERS})

target_link_libraries(ZtWidgets Qt5::Widgets)

set_target_properties(ZtWidgets
    PROPERTIES VERSION ${ZTWIDGETS_VERSION} SOVERSION ${ZTWIDGETS_SOVERSION})

install(TARGETS ZtWidgets DESTINATION ${CMAKE_INSTALL_PREFIX})
