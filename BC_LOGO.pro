QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 1.0.0

SOURCES += \
    core/source.cpp \
    core/diagnostic.cpp \
    core/lexer.cpp \
    core/ast.cpp \
    core/builtins.cpp \
    core/parser.cpp \
    core/interpreter.cpp \
    core/engine.cpp \
    editor.cpp \
    highlight.cpp \
    main.cpp \
    mainwindow.cpp \
    mylistener.cpp \
    painter_single.cpp \
    qtconsole.cpp \
    qtplotter.cpp

HEADERS += \
    _cursor.h \
    core/source.h \
    core/span.h \
    core/token.h \
    core/lexer.h \
    core/diagnostic.h \
    core/diagnostic_codes.h \
    core/ast.h \
    core/builtins.h \
    core/procedure.h \
    core/parser.h \
    core/value.h \
    core/environment.h \
    core/runtime.h \
    core/interpreter.h \
    core/engine.h \
    editor.h \
    mainwindow.h \
    mylistener.h \
    paint.h \
    qtconsole.h \
    qtplotter.h

FORMS += \
    mainwindow.ui

RC_ICONS = icon3.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    cursor.qrc
