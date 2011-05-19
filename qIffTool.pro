#-------------------------------------------------
#
# Project created by QtCreator 2011-05-19T19:39:10
#
#-------------------------------------------------

QT       += core gui

TARGET = qIffTool
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    MemoryMappedFile.cpp \
    IffIlbm.cpp \
    IffContainer.cpp \
    Iff8svx.cpp

HEADERS  += mainwindow.h \
    MemoryMappedFile.h \
    IffIlbm.h \
    IffContainer.h \
    Iff8svx.h

FORMS    += mainwindow.ui
