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
 * TODO/FIXME list:
 *
 *  [URGENT]
 *  [checklist for first public release]
 *    non-read only folders with drag & drop
 *    fix random segfaults on open
 *    i18n
 *    cleanup
 *    GitHub page
 *    linux segfault on icon theme change
 *      might be Qt version related, not sure
 *      possible solution would be requiring a restart, though this needs a new
 *        flag in Variable class
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
 *    macOS issues
 *    sorting issues on network folders (dirsFirst not working)
 *    complete documentation
 *    QWindow::requestActivate: requestActivate() FolderViewWindow
 *
 *  [CLEANUP]
 *    proper Q_PROPERTY implementation in classes
 *    allocs/deallocs
 *    includes
 *    private/public members
 *    static/const functions and mutables
 *
 *  [macOS]
 *    z-order issue (disappears on desktop)
 *    icon should not be visible in dock
 *    resize/move issues
 *    application bundle icon
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

    // set console output pattern
    qSetMessagePattern( "%{if-category}%{category}: %{endif}%{function}: %{message}" );

    // fixes auto close behaviour on linux
    QApplication::setQuitOnLastWindowClosed( false );

    // request trivial fileInfo early
    // might fix random "ShGetFileInfoBackground() timed out" bug
#ifdef Q_OS_WIN
#if ( QT_VERSION <= QT_VERSION_CHECK( 5, 8, 0 ))
    QFileInfo( QCoreApplication::applicationFilePath());
#endif
#endif

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
