cmake_minimum_required(VERSION 3.12)
project(simExtCodeEditor)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_MACOSX_RPATH 1)

find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Python3 REQUIRED)

if(NOT LIBPLUGIN_DIR)
    if(DEFINED ENV{COPPELIASIM_ROOT_DIR})
        set(LIBPLUGIN_DIR $ENV{COPPELIASIM_ROOT_DIR}/programming/libPlugin)
    else()
        set(LIBPLUGIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../libPlugin)
    endif()
endif()
list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules
    ${LIBPLUGIN_DIR}/cmake)
find_package(CoppeliaSim 4.1.0.0 REQUIRED)

set(QSCINTILLA_DIR_DEFAULT NOTFOUND)
if(NOT "$ENV{QSCINTILLA_DIR}" STREQUAL "")
    set(QSCINTILLA_DIR_DEFAULT "$ENV{QSCINTILLA_DIR}")
else()
    # arbitrary guess:
    set(QSCINTILLA_DIR_DEFAULT ${CMAKE_CURRENT_SOURCE_DIR}/QScintilla_commercial-2.10.8)
endif()
set(QSCINTILLA_DIR ${QSCINTILLA_DIR_DEFAULT} CACHE STRING "Path to the QScintilla directory")
if(NOT EXISTS "${QSCINTILLA_DIR}")
    message(FATAL_ERROR "The specified QSCINTILLA_DIR (${QSCINTILLA_DIR}) does not exist")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/generated)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
if(QSCINTILLA_DIR)
    include_directories(${QSCINTILLA_DIR}/Qt4Qt5)
    include_directories(${QSCINTILLA_DIR}/include)
    if(NOT QSCINTILLA_LIBRARY)
        if(NOT QSCINTILLA_BUILD_DIR)
            set(QSCINTILLA_BUILD_DIR ${QSCINTILLA_DIR}/Qt4Qt5/)
        endif()
        if(CMAKE_BUILD_TYPE MATCHES [Dd][Ee][Bb][Uu][Gg])
            set(QSCINTILLA_LIBRARY_SUFFIX _debug)
        endif()
        set(QSCINTILLA_LIBRARY ${QSCINTILLA_BUILD_DIR}/${CMAKE_SHARED_LIBRARY_PREFIX}qscintilla2_qt5${QSCINTILLA_LIBRARY_SUFFIX}${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
else()
    message(FATAL_ERROR "Please provide path to QScintilla via -DQSCINTILLA_DIR=...")
endif()
add_definitions(-DQT_COMPIL)

coppeliasim_generate_stubs(${CMAKE_CURRENT_BINARY_DIR}/generated XML_FILE ${CMAKE_CURRENT_SOURCE_DIR}/callbacks.xml LUA_FILE ${CMAKE_CURRENT_SOURCE_DIR}/simExtCodeEditor.lua)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/generated)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/generated/api_index.cpp
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/api_index.py
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/api_index.py
        --input ${COPPELIASIM_ROOT_DIR}/helpFiles
        --output ${CMAKE_CURRENT_BINARY_DIR}/generated/api_index.cpp
)
add_custom_target(generate_api_index DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generated/api_index.cpp)

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

if(QSCINTILLA_BUILD_DIR)
    if(WIN32)
        file(TO_CMAKE_PATH ${QSCINTILLA_BUILD_DIR} QSCINTILLA_BUILD_DIR)
        list(APPEND SOURCES
            ${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintilla.cpp
            ${QSCINTILLA_BUILD_DIR}/release/moc_qsciscintillabase.cpp
        )
    endif()
endif()

set(LIBRARIES
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::PrintSupport
    Qt5::Xml
    ${QSCINTILLA_LIBRARY}
)

coppeliasim_add_plugin(simExtCodeEditor SOURCES ${SOURCES})
add_dependencies(simExtCodeEditor generate_api_index)
target_link_libraries(simExtCodeEditor ${LIBRARIES})

if(APPLE)
    get_filename_component(QSCINTILLA_LIB_NAME ${QSCINTILLA_LIBRARY} NAME)
    add_custom_command(TARGET simExtCodeEditor POST_BUILD COMMAND ${CMAKE_INSTALL_NAME_TOOL} -change ${QSCINTILLA_LIB_NAME} @executable_path/../Frameworks/${QSCINTILLA_LIB_NAME} $<TARGET_FILE:simExtCodeEditor>)
endif()
