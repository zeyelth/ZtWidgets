include(SIPMacros)

find_package(PythonInterp ${MIN_PYTHON_VERSION})
find_package(SIP ${MIN_SIP_VERSION})
find_package(PyQt5 ${MIN_PYQT_VERSION})

include_directories(${ZtWidgets_INCLUDE} ${SIP_INCLUDE_DIR})

# dummy target so IDEs like Qt Creator picks them up properly
file(GLOB_RECURSE FLIST "./*.sip")
add_custom_target(sip_extrafiles SOURCES ${FLIST})

set(SIP_INCLUDES
    ${PYQT5_SIP_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(SIP_CONCAT_PARTS 1)

# We need to set WS_X11 or some defines in Qt won't be set properly, resulting in helpful errors like:
# "sip: Q_PID is undefined"
set(SIP_TAGS WS_X11 ${PYQT5_VERSION_TAG})

set(PYTHON_SITE_PACKAGES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/PyQt5/)

set(ZtWidgets_SIP_SOURCE
    ZtWidgets/colorpicker.sip
    ZtWidgets/slideredit.sip
)

set(SIP_EXTRA_FILES_DEPEND ${ZtWidgets_SIP_SOURCE})

add_sip_python_module(ZtWidgets.colorpicker ZtWidgets/colorpicker.sip ZtWidgets)
add_sip_python_module(ZtWidgets.slideredit ZtWidgets/colorpicker.sip ZtWidgets)

# install __init__.py so the ZtWidgets directory becomes a ready to use Python module
install(FILES ZtWidgets/__init__.py DESTINATION ${PYTHON_SITE_PACKAGES_INSTALL_DIR}/ZtWidgets/)
