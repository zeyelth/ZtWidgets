project(ZtWidgets)

message(STATUS "Using CMake version: ${CMAKE_VERSION}")

cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

set(ZTWIDGETS_VERSION 0.0.5)
set(ZTWIDGETS_SOVERSION "${ZTWIDGETS_VERSION}")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

set(MIN_QT_VERSION 5.15.0)
set(MIN_PYQT_VERSION 5.15.0)
set(MIN_SIP_VERSION 4.19.0)
set(MIN_PYTHON_VERSION 3.8)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")

# Include source and binary directory for builds, so generated include files can be found
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 ${MIN_QT_VERSION}
    REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

add_definitions(
    -DQT_NO_SIGNALS_SLOTS_KEYWORDS
)

set(ZtWidgets_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/ZtWidgets")
set(ZtWidgets_INCLUDE "${ZtWidgets_ROOT}/include")

add_subdirectory(ZtWidgets)
add_subdirectory(plugins)
add_subdirectory(sip)
add_subdirectory(examples)
