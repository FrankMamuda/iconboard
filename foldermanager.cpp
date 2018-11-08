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

//
// includes
//
#include "foldermanager.h"
#include "folderview.h"
#include "main.h"
#ifdef Q_OS_WIN
#include "desktopwidget.h"
#endif
#include <QDebug>

/**
 * @brief FolderManager::FolderManager
 * @param parent
 */
FolderManager::FolderManager( QObject *parent ) : QObject( parent )
#ifdef Q_OS_WIN
  , desktop( new DesktopWidget )
#endif
{
    // announce
#ifdef QT_DEBUG
    qInfo() << this->tr( "initializing" );
#endif

    // add to garbage collector
    GarbageMan::instance()->add( this );
}

/**
 * @brief FolderManager::~FolderManager
 */
FolderManager::~FolderManager() {
#ifdef Q_OS_WIN
    delete this->desktop;
#endif
}

/**
 * @brief FolderManager::count
 * @return
 */
int FolderManager::count() const {
    return this->list.count();
}

/**
 * @brief FolderManager::iconCount
 * @return
 */
int FolderManager::iconCount() const {
    return this->iconList.count();
}

/**
 * @brief FolderManager::at
 * @param index
 * @return
 */
FolderView *FolderManager::at( int index ) const {
    if ( index < 0 || index >= this->count())
        return nullptr;

    return this->list.at( index );
}

/**
 * @brief FolderManager::iconAt
 * @param index
 * @return
 */
DesktopIcon *FolderManager::iconAt( int index ) const {
    if ( index < 0 || index >= this->iconCount())
        return nullptr;

    return this->iconList.at( index );
}

/**
 * @brief FolderManager::add
 * @param folderView
 */
void FolderManager::add( FolderView *folderView ) {
    this->list << folderView;
}

/**
 * @brief FolderManager::add
 * @param desktopIcon
 */
void FolderManager::add( DesktopIcon *desktopIcon ) {
    this->iconList << desktopIcon;
}

/**
 * @brief FolderManager::remove
 * @param folderView
 */
void FolderManager::remove( FolderView *folderView ) {
    this->list.removeOne( folderView );
    delete folderView;
}

/**
 * @brief FolderManager::remove
 * @param desktopIcon
 */
void FolderManager::remove( DesktopIcon *desktopIcon ) {
    this->iconList.removeOne( desktopIcon );
    delete desktopIcon;
}

/**
 * @brief FolderManager::shutdown
 */
void FolderManager::shutdown() {
    qDeleteAll( this->list );
    this->list.clear();
    qDeleteAll( this->iconList );
    this->iconList.clear();
}
