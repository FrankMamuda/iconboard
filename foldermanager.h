/*
 * Copyright (C) 2017 Zvaigznu Planetarijs
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 */

#pragma once

//
// includes
//
#include "singleton.h"
#include "folderview.h"
#include "desktopicon.h"

//
// classes
//
class FolderView;
class DesktopWidget;

/**
 * @brief The FolderManager class
 */
class FolderManager : public QObject {
    Q_OBJECT

public:
    ~FolderManager();
    static FolderManager *instance() { return Singleton<FolderManager>::instance( FolderManager::createInstance ); }
    int count() const;
    int iconCount() const;
    FolderView *at( int index ) const;
    DesktopIcon *iconAt( int index ) const;
#ifdef Q_OS_WIN
    DesktopWidget *desktop;
#endif
    QList<FolderView*> previews;

public slots:
    void add( FolderView *folderView );
    void add( DesktopIcon *desktopIcon );
    void remove( FolderView *folderView );
    void remove( DesktopIcon *desktopIcon );
    void shutdown();

private:
    FolderManager( QObject *parent = nullptr );
    static FolderManager *createInstance() { return new FolderManager(); }
    QList<FolderView*> list;
    QList<DesktopIcon*> iconList;
};
