cmake_minimum_required(VERSION 2.8.11)
project(v_repExtCodeEditor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

find_package(Boost REQUIRED)
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Xml REQUIRED)

set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CoppeliaRobotics-modules)
find_package(VREP 3.5.0 REQUIRED)

set(QSCINTILLA_DIR_DEFAULT NOTFOUND)
if(NOT "$ENV{QSCINTILLA_DIR}" STREQUAL "")
    set(QSCINTILLA_DIR_DEFAULT "$ENV{QSCINTILLA_DIR}")
else()
    # arbitrary guess:
    set(QSCINTILLA_DIR_DEFAULT ${CMAKE_CURRENT_SOURCE_DIR}/../../QScintilla-commercial-2.10.2)
endif()
set(QSCINTILLA_DIR ${QSCINTILLA_DIR_DEFAULT} CACHE STRING "Path to the QScintilla directory")

include_directories(${VREP_INCLUDE})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${Boost_INCLUDE_DIRS})
if(QSCINTILLA_DIR)
    include_directories(${QSCINTILLA_DIR}/Qt4Qt5)
    include_directories(${QSCINTILLA_DIR}/include)
    set(QSCINTILLA_LIB ${QSCINTILLA_DIR}/Qt4Qt5/${CMAKE_SHARED_LIBRARY_PREFIX}qscintilla2_qt5${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
    message(FATAL_ERROR "Please provide path to QScintilla via -DQSCINTILLA_DIR=...")
endif()

#vrep_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simExtCustomUI.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    scintillaDlg.cpp
    plugin.cpp
    UI.cpp
    SIM.cpp
    QtUtils.cpp
    debug.cpp
    tinyxml2/tinyxml2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/external/v_repPlusPlus/Plugin.cpp
    #${CMAKE_CURRENT_BINARY_DIR}/generated/stubs.cpp
    ${VREP_EXPORTED_SOURCES}
)

set(LIBRARIES
    ${Boost_LIBRARIES}
    ${VREP_LIBRARIES}
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Xml
    ${QSCINTILLA_LIB}
)

add_library(v_repExtCodeEditor SHARED ${SOURCES})
target_link_libraries(v_repExtCodeEditor ${LIBRARIES})

install(TARGETS v_repExtCodeEditor DESTINATION ${VREP_ROOT})

