cmake_minimum_required(VERSION 3.16.3)
project(simCodeEditor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

set(Qt Qt5 CACHE STRING "Qt version to use (e.g. Qt5)")
set_property(CACHE Qt PROPERTY STRINGS Qt5 Qt6)  #

find_package(${Qt} COMPONENTS Core Gui Widgets PrintSupport Xml Network REQUIRED)

if(NOT COPPELIASIM_INCLUDE_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(COPPELIASIM_INCLUDE_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/include)
    else()
        set(COPPELIASIM_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${COPPELIASIM_INCLUDE_DIR}/cmake)
find_package(CoppeliaSim 4.5.1.7 REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
add_definitions(-DQT_COMPIL)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simCodeEditor.lua)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES
    dialog.cpp
    editor.cpp
    toolbar.cpp
    snippets.cpp
    statusbar.cpp
    searchandreplacepanel.cpp
    plugin.cpp
    UI.cpp
    SIM.cpp
    common.cpp
)

set(LIBRARIES
    ${Qt}::Core
    ${Qt}::Gui
    ${Qt}::Widgets
    ${Qt}::PrintSupport
    ${Qt}::Xml
    ${Qt}::Network
)

coppeliasim_add_plugin(simCodeEditor SOURCES ${SOURCES})
target_link_libraries(simCodeEditor PRIVATE ${LIBRARIES})
coppeliasim_add_resource_directory(snippets)
