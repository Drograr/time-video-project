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
    tvgsrecorder.cpp

HEADERS  += tvmainwindow.h \
    tvgsrecorder.h

FORMS    += tvmainwindow.ui

CONFIG += link_pkgconfig
PKGCONFIG += gstreamer-video-1.0 #gstreamer-1.0
