#-------------------------------------------------
#
# Project created by QtCreator 2015-04-17T08:17:44
#
#-------------------------------------------------

QT       += core gui network
CONFIG += openssl-linked
RC_FILE = win-icon.rc


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Updater
TEMPLATE = app

TRANSLATIONS = updater_en.ts updater_fr.ts

SOURCES += main.cpp \
    updater.cpp

HEADERS  += \
    updater.h

