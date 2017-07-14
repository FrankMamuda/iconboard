#
# Copyright (C) 2017 Zvaigznu Planetarijs
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see http://www.gnu.org/licenses/.
#

QT       += core gui xml concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = IconBoard
TEMPLATE = app

SOURCES += main.cpp\
    folderview.cpp \
    folderdelegate.cpp \
    widgetmodel.cpp \
    traywidget.cpp \
    filestream.cpp \
    iconindex.cpp \
    iconcache.cpp \
    indexcache.cpp \
    iconproxymodel.cpp \
    stylesheetdialog.cpp

HEADERS  += \
    folderview.h \
    folderdelegate.h \
    widgetmodel.h \
    traywidget.h \
    iconcache.h \
    filestream.h \
    iconindex.h \
    callonce.h \
    singleton.h \
    indexcache.h \
    iconproxymodel.h \
    stylesheetdialog.h

FORMS    += \
    folderview.ui \
    traywidget.ui \
    stylesheetdialog.ui

RESOURCES += \
    resources.qrc

win32:RC_FILE = icon.rc
win32:QT += winextras
win32:LIBS += -lgdi32 -luser32
