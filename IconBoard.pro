#
# Copyright (C) 2017-2018 Zvaigznu Planetarijs
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

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    main.cpp \
    desktopicon.cpp \
    filesystemmodel.cpp \
    filestream.cpp \
    folderdelegate.cpp \
    foldermanager.cpp \
    folderview.cpp \
    iconcache.cpp \
    iconindex.cpp \
    iconsettings.cpp \
    indexcache.cpp \
    listview.cpp \
    mapperwidget.cpp \
    proxymodel.cpp \
    screenmapper.cpp \
    settings.cpp \
    themeeditor.cpp \
    themes.cpp \
    trayicon.cpp \
    variable.cpp \
    widgetlist.cpp \
    widgetmodel.cpp \
    xmltools.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

FORMS += \
    about.ui \
    folderview.ui \
    iconsettings.ui \
    screenmapper.ui \
    settings.ui \
    themeeditor.ui \
    widgetlist.ui

HEADERS += \
    about.h \
    application.h \
    backgroundframe.h \
    desktopicon.h \
    filesystemmodel.h \
    filestream.h \
    folderdelegate.h \
    foldermanager.h \
    folderview.h \
    iconcache.h \
    iconindex.h \
    iconsettings.h \
    indexcache.h \
    listview.h \
    main.h \
    mapperwidget.h \
    proxymodel.h \
    screenmapper.h \
    settings.h \
    themeeditor.h \
    themes.h \
    trayicon.h \
    variable.h \
    widgetlist.h \
    widgetmodel.h \
    xmltools.h \
    widget.h

win32:SOURCES +=
win32:HEADERS +=
win32:RC_FILE = icon.rc
win32:QT += winextras
win32:LIBS += -lgdi32 -luser32 -luuid -lole32

