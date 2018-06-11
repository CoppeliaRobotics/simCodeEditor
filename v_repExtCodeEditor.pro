include(config.pri)

TARGET = v_repExtCodeEditor
TEMPLATE = lib

DEFINES -= UNICODE
CONFIG += shared
QT += widgets xml

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
    DEFINES += WIN_VREP
}

macx {
    DEFINES += MAC_VREP
}

unix:!macx {
    DEFINES += LIN_VREP
}

SOURCES += \
    tinyxml2/tinyxml2.cpp \
    plugin.cpp \
    debug.cpp \
    scintillaDlg.cpp \
    UI.cpp \
    SIM.cpp \
    QtUtils.cpp \
    ../common/v_repLib.cpp \

HEADERS +=\
    tinyxml2/tinyxml2.h \
    plugin.h \
    codeEditor.h \
    debug.h \
    scintillaDlg.h \
    UI.h \
    SIM.h \
    QtUtils.h \
    ../include/v_repLib.h \

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}
