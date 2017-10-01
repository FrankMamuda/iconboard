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
#include <QScreen>
#include "traywidget.h"
#include "indexcache.h"
#include "iconindex.h"
#include "iconcache.h"
#include "variable.h"
#include "proxymodel.h"
#include "application.h"
#include "xmltools.h"
#include "themes.h"

/*
 * TODO list:
 *  [DONE] hidden widgets
 *  [DONE] custom styleSheet support
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
 *  [DONE] remove hilight in list mode
 *  [DONE] weird QObject::moveToThread fix
 *  [DONE] focusless scrolling (mouseOver scrolling)
 *  [DONE] single instance
 *  [DONE] fixed custom styleSheet font issues
 *  [DONE] remove QListView border
 *  [DONE] start-on-boot option
 *  [DONE] fix tray context menu for linux
 *  [DONE] 'about' dialog
 *  [DONE] predefined themes
 *  [DONE] theme editor
 *  [DONE] custom sorting
 *  [DONE] custom hilight/selection color - use selection-background-color: in qss
 *  [DONE] weird QPersistentIndex corruption bugfix - reverted to QModelIndex
 *  [DONE] periodic (timed settings save)
 *  [DONE] thread safe exit (wait for QtConcurrent threads to end)
 *  [DONE] performace issues (using isReadable() to avoid timeouts)
 *  lock to specific resolution
 *  lock to desktop screen
 *  root folder ('My Computer') folder support
 *  pseudo-folders (with drag-drop support)
 *  non-read only folders
 *  backup configuration
 *  custom alignment
 *  free placement, free scaling
 *  whole desktop replacement option
 *  multi column list
 *  custom per-item icons
 *  extract shell icons from dirs on symlinks
 *  caching of extracted icons and thumbnails
 *  drive names (from shortcuts)
 *  extract icons from links themselves not their targets
 *  thumbnail caching as an option
 *  fix ASSERT: "!"QSortFilterProxyModel: index from wrong model passed to mapFromSource"" in file itemmodels\qsortfilterproxymodel.cpp
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
    TrayWidget trayWidget;
    Q_UNUSED( trayWidget )

    return app.exec();
}
