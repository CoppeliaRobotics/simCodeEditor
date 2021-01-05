include(config.pri)

TARGET = simExtCodeEditor
TEMPLATE = lib

DEFINES -= UNICODE
CONFIG += shared plugin
QT += core gui widgets xml printsupport

*-msvc* {
    QMAKE_CXXFLAGS += -O2
    QMAKE_CXXFLAGS += -W3
}
*-g++* {
    QMAKE_CXXFLAGS += -O3
    QMAKE_CXXFLAGS += -Wall
    QMAKE_CXXFLAGS += -Wno-unused-parameter
    QMAKE_CXXFLAGS += -Wno-strict-aliasing
    QMAKE_CXXFLAGS += -Wno-empty-body
    QMAKE_CXXFLAGS += -Wno-write-strings

    QMAKE_CXXFLAGS += -Wno-unused-but-set-variable
    QMAKE_CXXFLAGS += -Wno-unused-local-typedefs
    QMAKE_CXXFLAGS += -Wno-narrowing

    QMAKE_CFLAGS += -O3
    QMAKE_CFLAGS += -Wall
    QMAKE_CFLAGS += -Wno-strict-aliasing
    QMAKE_CFLAGS += -Wno-unused-parameter
    QMAKE_CFLAGS += -Wno-unused-but-set-variable
    QMAKE_CFLAGS += -Wno-unused-local-typedefs
}

INCLUDEPATH += "../include"

INCLUDEPATH += $$BOOST_INCLUDEPATH
INCLUDEPATH += $$QSCINTILLA_INCLUDEPATH
LIBS += $$QSCINTILLA_LIBS

win32 {
    DEFINES += WIN_SIM
}

macx {
    DEFINES += MAC_SIM
}

unix:!macx {
    DEFINES += LIN_SIM
}

SOURCES += \
    tinyxml2/tinyxml2.cpp \
    plugin.cpp \
    debug.cpp \
    UI.cpp \
    SIM.cpp \
    ../common/simLib.cpp \
    common.cpp \
    dialog.cpp \
    editor.cpp \
    searchandreplacepanel.cpp \
    toolbar.cpp \
    statusbar.cpp

HEADERS +=\
    tinyxml2/tinyxml2.h \
    plugin.h \
    debug.h \
    UI.h \
    SIM.h \
    ../include/simLib.h \
    common.h \
    dialog.h \
    editor.h \
    searchandreplacepanel.h \
    toolbar.h \
    statusbar.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
