#-------------------------------------------------
#
# Project created by QtCreator 2018-04-09T18:23:23
#
#-------------------------------------------------

QT       += core gui multimedia av avwidgets
CONFIG += qxt
QXT = core widgets

LIBS +=-lavcodec -lavutil
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScreenRecord
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

RESOURCES += \
    res.qrc

FORMS += \
    maindlg.ui \
    trandlg.ui

HEADERS += \
    audatacapthread.h \
    audataringbuffer.h \
    auencthread.h \
    maindlg.h \
    packetringbuffer.h \
    screencapencthread.h \
    timestamp.h \
    trandlg.h \
    videoencthread.h \
    videoringbuffer.h \
    videosavetemp.h \
    writerthread.h

SOURCES += \
    audatacapthread.cpp \
    audataringbuffer.cpp \
    auencthread.cpp \
    main.cpp \
    maindlg.cpp \
    packetringbuffer.cpp \
    screencapencthread.cpp \
    timestamp.cpp \
    trandlg.cpp \
    videoencthread.cpp \
    videoringbuffer.cpp \
    videosavetemp.cpp \
    writerthread.cpp


