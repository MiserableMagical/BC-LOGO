QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 0.9.0

SOURCES += \
    editor.cpp \
    eval.cpp \
    highlight.cpp \
    lexer.cpp \
    main.cpp \
    mainwindow.cpp \
    mylistener.cpp \
    painter_single.cpp \
    parser.cpp \
    parser_control.cpp \
    parser_single.cpp

HEADERS += \
    _cursor.h \
    editor.h \
    lexer.h \
    mainwindow.h \
    mylistener.h \
    paint.h

FORMS += \
    mainwindow.ui

RC_ICONS = icon3.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    cursor.qrc
