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
#include <QDebug>
#include "traywidget.h"
#include "iconindex.h"
#include "iconcache.h"
#include "indexcache.h"
#include "variable.h"
#include "proxymodel.h"
#include "application.h"
#include "xmltools.h"
#include "themes.h"

/*
 * TODO list:
 * [DONE]
 *    hidden widgets
 *    custom styleSheet support
 *    xml safe strings in config
 *    "open with" dialog
 *    bugfix for right click on item and hilight rect
 *    list mode
 *    custom icon size
 *    extract icons from symlinks
 *    QFileSystemWatcher - no need
 *    geometry fix for multiple monitors
 *    icon extraction issues on MSVC builds
 *    centred icons (if smaller than icon size)
 *    settings dialog with custom variables
 *    remove hilight in list mode
 *    weird QObject::moveToThread fix
 *    focusless scrolling (mouseOver scrolling)
 *    single instance
 *    fixed custom styleSheet font issues
 *    remove QListView border
 *    start-on-boot option
 *    fix tray context menu for linux
 *    'about' dialog
 *    predefined themes
 *    theme editor
 *    custom sorting
 *    custom hilight/selection color - use selection-background-color: in qss
 *    periodic (timed settings save)
 *    thread safe exit (wait for QtConcurrent threads to end)
 *    performace issues (using isReadable() to avoid timeouts)
 *    fix z-order change after "ToggleDesktop" on windows
 *    detect widget off screen
 *    styleSheet issues for closed widgets
 *    remove screenMapper from release builds

 * [REVERTED]
 *    weird QPersistentIndex corruption bugfix
 *      using QModelIndex instead QPersistentIndex is a bad idea - segfaults
 *      might also be related to "index from wrong model passed to mapFromSource" bug
 *    reverted back to QPersistentIndex - yet to see any corruption
 *      very random, hard to reproduce
 *
 *  [URGENT]
 *  [checklist for first public release]
 *    non-read only folders with drag & drop
 *    fix random segfaults on open
 *    i18n
 *    cleanup
 *    GitHub page
 *
 *  [NOT URGENT]
 *  [to be implemented in future versions]
 *    lock to specific resolution
 *    lock to desktop screen
 *    root folder ('My Computer') folder support
 *    pseudo-folders (with drag-drop support)
 *    custom alignment
 *    free placement, free scaling
 *    whole desktop replacement option
 *    multi column list
 *    custom per-item icons
 *    extract shell icons from dirs on symlinks
 *    disk-caching of extracted icons and thumbnails
 *    drive names (from shortcuts)
 *    extract icons from links themselves not their targets
 *    thumbnail loading as an option
 *    folderView spacing, other props
 *
 *  [CLEANUP]
 *    proper Q_PROPERTY implementation in classes
 *    allocs/deallocs
 *    includes
 *    private/public members
 *    static/const functions and mutables
 */

/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QString themeName;
    Application app( argc, argv );

    // fixes auto close behaviour on linux
    QApplication::setQuitOnLastWindowClosed( false );

    // create an instance of app
    if ( !app.lock())
        return EXIT_FAILURE;

    // register metatypes
    qRegisterMetaType<Entry>( "Entry" );
    qRegisterMetaType<Match>( "Match" );
    qRegisterMetaType<MatchList>( "MatchList" );
    qRegisterMetaType<Theme*>( "Theme*" );
    qRegisterMetaType<ProxyIcon>( "ProxyIcon" );

    // add default variables
    Variable::instance()->add( "ui_displaySymlinkIcon", true );
    Variable::instance()->add( "app_runOnStartup", false );
    Variable::instance()->add( "ui_iconTheme", "system" );
    XMLTools::instance()->readConfiguration( XMLTools::Variables );
    XMLTools::instance()->readConfiguration( XMLTools::Themes );

    // request a trivial icon early to avoid QObject::moveToThread bug
#ifdef Q_OS_LINUX
    QIcon i;
    Q_UNUSED( i )
#endif
    IconCache::instance()->iconForFilename( QDir::currentPath(), 0 );

    // setup icons
    themeName = Variable::instance()->string( "ui_iconTheme" );
    if ( !themeName.isEmpty() && QString::compare( "system", themeName )) {
        QFile file( IconIndex::instance()->path() + "/" + themeName + "/" + "index.theme" );
        if ( file.exists()) {
            IconIndex::instance()->build( themeName );
            IconIndex::instance()->setDefaultTheme( themeName );
        }
    }

    // display tray widget
    TrayWidget::instance()->initialize();

    return app.exec();
}
