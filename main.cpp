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
#include <QApplication>
#include <QDebug>
#include "traywidget.h"
#include "indexcache.h"
#include "iconindex.h"
#include "iconcache.h"

/*
 * TODO list:
 *  lock to specific resolution
 *  lock to desktop
 *  periodic (timed settings save)
 *  [DONE] hidden widgets
 *  root folder ('My Computer') folder support
 *  pseudo-folders (with drag-drop support)
 *  non-read only folders
 *  [DONE] custom stylesheet support
 *  [DONE] xml safe strings in config
 *  backup configuration
 *  single instance
 *  custom sorting
 *  custom alignment
 *  remove hilight in list mode
 *  custom hilight/selection color
 *  [DONE] custom icon size
 *  [DONE] extract icons from symlinks
 *  dir itertor batched loading
 *  [DONE] QFileSystemWatcher - no need
 *  free grid placement
 *  "open with" dialog
 *  bugfix for right click on item and hilight rect
 *  [DONE] list mode
 *  focusless scrolling (mouseOver scrolling)
 *  multi column list
 *  weird QPersistentIndex corruption bugfix
 *  weird QObject::moveToThread fix
 *  start-on-boot option
 *  custom per-item icons
 *  performace issues with large directories
 *  extract shell icons from dirs on symlinks
 *  [DONE] geometry fix for multiple monitors
 *  [DONE] icon extraction issues on MSVC builds
 *  caching of extracted icons and thumbnails
 *  [DONE] centred icons (if smaller than icon size)
 */

/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QApplication app( argc, argv );

    // register metatypes
    qRegisterMetaType<Entry>( "Entry" );
    qRegisterMetaType<Match>( "Match" );
    qRegisterMetaType<MatchList>( "MatchList" );

    // setup icons
    IconIndex::instance()->build( "breeze" );
    IconIndex::instance()->build( "breeze-dark" );

    // display tray widget
    TrayWidget a;
    Q_UNUSED( a );

    return app.exec();
}
