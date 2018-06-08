# location of boost headers:
    BOOST_INCLUDEPATH = "c:/local/boost_1_62_0"    # (e.g. Windows)
    #BOOST_INCLUDEPATH = "/usr/local/include"    # (e.g. MacOS)

# qscintilla location:
    QSCINTILLA_DIR = "d:/QScintilla-commercial-2.7.2"    # (e.g. Windows)
    #QSCINTILLA_DIR = "../../QScintilla-commercial-2.7.2"    # (e.g. Windows-MSYS2)
    #QSCINTILLA_DIR = "../../QScintilla-commercial-2.7.2"    # (e.g. Ubuntu)
    #QSCINTILLA_DIR = "../../QScintilla-commercial-2.7.2"    # (e.g. MacOS)

# qscintilla headers:
    QSCINTILLA_INCLUDEPATH = "$${QSCINTILLA_DIR}/include" "$${QSCINTILLA_DIR}/Qt4Qt5"

# qscintilla libraries to link:
    QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/release/release/qscintilla2.lib"    # (e.g. Windows)
    #QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/release/release/libqscintilla2.dll.a"    # (e.g. Windows-MSYS2)    
    #QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/release/libqscintilla2.so"    # (e.g. Ubuntu)
    #QSCINTILLA_LIBS = "$${QSCINTILLA_DIR}/release/libqscintilla2.9.0.2.dylib"    # (e.g. MacOS)

# Make sure if a config.pri is found one level above, that it will be used instead of this one:
    exists(../config.pri) { include(../config.pri) }
