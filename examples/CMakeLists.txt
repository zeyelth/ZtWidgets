set(ZtWidgetsExample_SOURCES
    main.cpp
    mainwindow.cpp
)

set(ZtWidgetsExample_HEADERS
    mainwindow.h
)

qt5_wrap_cpp(ZtWidgetsExample_HEADER_MOC
    ${ZtWidgetsExample_HEADERS}
)

qt5_wrap_ui(ZtWidgetsExample_SOURCES
    forms/mainwindow.ui
)

add_executable(ZtWidgetsExample ${ZtWidgetsExample_SOURCES} ${ZtWidgetsExample_HEADER_MOC})

target_include_directories(ZtWidgetsExample PRIVATE ${ZtWidgets_INCLUDE})

target_link_libraries(ZtWidgetsExample ZtWidgets)

install(TARGETS ZtWidgetsExample DESTINATION ${CMAKE_INSTALL_PREFIX})
