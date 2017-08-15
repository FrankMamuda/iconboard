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
#include "variable.h"
#include "iconproxymodel.h"

/*
 * TODO list:
 *  [DONE] hidden widgets
 *  [DONE] custom stylesheet support
 *  [DONE] xml safe strings in config
 *  [DONE] "open with" dialog
 *  [DONE] bugfix for right click on item and hilight rect
 *  [DONE] list mode
 *  [DONE] custom icon size
 *  [DONE] extract icons from symlinks
 *  [DONE] QFileSystemWatcher - no need
 *  [DONE] geometry fix for multiple monitors
 *  [DONE] icon extraction issues on MSVC builds
 *  [DONE] centred icons (if smaller than icon size)
 *  [DONE] settings dialog with custom variables
 *  [DONE?] weird QPersistentIndex corruption bugfix
 *  [DONE?] weird QObject::moveToThread fix
 *  lock to specific resolution
 *  lock to desktop
 *  periodic (timed settings save)
 *  root folder ('My Computer') folder support
 *  pseudo-folders (with drag-drop support)
 *  non-read only folders
 *  backup configuration
 *  single instance
 *  custom sorting
 *  custom alignment
 *  remove hilight in list mode
 *  custom hilight/selection color
 *  dir itertor batched loading
 *  free grid placement
 *  focusless scrolling (mouseOver scrolling)
 *  multi column list
 *  start-on-boot option
 *  custom per-item icons
 *  performace issues with large directories
 *  extract shell icons from dirs on symlinks
 *  caching of extracted icons and thumbnails
 *  drive names (from shortcuts)
 *  setting custom stylesheet changes font for some reason
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

    // request a trivial icon early to avoid QObject::moveToThread bug
    IconProxyModel::iconForFilename( QDir::currentPath(), 0 );

    // display tray widget
    TrayWidget trayWidget;

    // add vars
    Variable::instance()->add( "ui_displaySymlinkIcon", true );
    Variable::instance()->add( "universalInteger", 42 );

    QIcon overlayIcon( ":/icons/link" );
    Variable::instance()->add( "ui_overlayIconTest", overlayIcon );

    // read config
    trayWidget.readXML();

    return app.exec();
}
