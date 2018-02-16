#-------------------------------------------------
#
# Project created by QtCreator 2017-12-17T18:46:59
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QuintetPlayer
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


SOURCES += \
        src/main.cpp \
        src/mainwindow.cpp \
    HCADecoder/clHCA.cpp \
    src/utils.cpp \
    src/HCAStreamChannel.cpp \
    HCADecoder/HCADecodeService.cpp

HEADERS += \
        src/mainwindow.h \
    HCADecoder/clHCA.h \
    src/utils.h \
    src/HCAStreamChannel.h \
    HCADecoder/HCADecodeService.h \
    HCADecoder/Semaphore.h \
    bass/bass.h \
    bass/bassmix.h

FORMS += \
        mainwindow.ui

unix|win32: LIBS += -L$$PWD/bass/ -lbass -lbassmix

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.

TRANSLATIONS = QuintetPlayer_jp.ts
