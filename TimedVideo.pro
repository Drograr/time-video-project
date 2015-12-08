#-------------------------------------------------
#
# Project created by QtCreator 2015-12-08T16:40:38
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TimedVideo
TEMPLATE = app


SOURCES += main.cpp\
        tvmainwindow.cpp \
    timedvideo.cpp

HEADERS  += tvmainwindow.h

FORMS    += tvmainwindow.ui

CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-1.0
