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
#include "widgetlist.h"
#include "iconindex.h"
#include "iconcache.h"
#include "indexcache.h"
#include "variable.h"
#include "proxymodel.h"
#include "application.h"
#include "xmltools.h"
#include "themes.h"
#include "widgetlist.h"
#include "foldermanager.h"
#include "main.h"
#include "trayicon.h"

/*
 * TODO/FIXME list:
 *
 *  [URGENT]
 *  [checklist for first public release]
 *    fix random segfaults on open
 *    i18n
 *    cleanup
 *    GitHub page
 *    release beta
 *    linux segfault on icon theme change
 *      might be Qt version related, not sure
 *      possible solution would be requiring a restart, though this needs a new
 *        flag in Variable class
 *
 *  [NOT URGENT]
 *  [to be implemented in future versions]
 *    root folder ('My Computer') folder support
 *    custom alignment
 *    free placement, free scaling
 *    whole desktop replacement option
 *    multi column list
 *    custom per-item icons
 *    disk-caching of extracted icons and thumbnails
 *    drive names (from shortcuts)
 *    thumbnail loading as an option
 *    folderView spacing, other props
 *    macOS issues
 *    sorting issues on network folders (dirsFirst not working)
 *    complete documentation
 *    QWindow::requestActivate: requestActivate() FolderViewWindow
 *    first run dialog
 *    option to download icon packs
 *    option not to upscale small icons
 *    custom icon dir (set in settings)
 *    minor issues with z-order (windows)
 *    handle shortcut/folder drops on tray icon or widget list to add widget
 *    move hook from Widgetlist to Desktopwidget
 *    move iconThemeChanged to IconIndex
 *    fix flickering with batched resize (currently listview set to singlepass)
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
 * @brief messageFilter
 * @param type
 * @param context
 * @param msg
 */
void Main::messageFilter( QtMsgType type, const QMessageLogContext &, const QString &msg ) {
    QFile logFile;
    QString output( msg );

    switch ( type ) {
    case QtDebugMsg:
        output.prepend( QObject::tr( "Debug: " ));
        break;

    case QtWarningMsg:
        output.prepend( QObject::tr( "Warning: " ));
        break;

    case QtCriticalMsg:
        output.prepend( QObject::tr( "Critical: " ));
        break;

    case QtFatalMsg:
        output.prepend( QObject::tr( "Fatal: " ));
        Main::instance()->shutdown();
        break;

    case QtInfoMsg:
        output.prepend( QObject::tr( "Info: " ));
        break;
    };

    // open log file and write out
    logFile.setFileName( QDir::currentPath() + "/" + "log" );

    if ( logFile.size() > 1024 * 1024 )
        logFile.open( QIODevice::WriteOnly | QIODevice::Truncate );
    else
        logFile.open( QIODevice::WriteOnly | QIODevice::Append );

    QTextStream steam( &logFile );
    steam << output << endl;
    logFile.close();
}

/**
 * @brief qMain
 * @param argc
 * @param argv
 * @return
 */
int main( int argc, char *argv[] ) {
    QString themeName;

    // set console output pattern
    qSetMessagePattern( "%{if-category}%{category}: %{endif}%{function}: %{message}" );

    // log to file in non-qtcreator environment
    if ( qgetenv( "QTDIR" ).isEmpty())
        qInstallMessageHandler( Main::messageFilter );

    // app
    Application app( argc, argv );

    // fix runOnStartup in wrong dir bug
    QDir::setCurrent( qApp->applicationDirPath());

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
    XMLTools::instance()->read( XMLTools::Variables );
    XMLTools::instance()->read( XMLTools::Themes );

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
#ifdef Q_OS_WIN
    else {
        IconCache::instance()->preLoadWindowsIcons();
    }
#endif

    // read config
    Main::instance()->readConfiguration();

    return app.exec();
}

/**
 * @brief Main::readConfiguration
 */
Main::Main( QObject *parent ) : QObject( parent ), m_initialized( false ) {
    // announce
#ifdef QT_DEBUG
    qInfo() << "initializing";
#endif

    // save settings every 60 seconds
    this->startTimer( 60 * 1000 );

    // set initialized
    this->setInitialized();

    this->widgetList = new WidgetList();
    this->tray = new TrayIcon( this->widgetList );
}

void Main::readConfiguration() {
    XMLTools::instance()->read( XMLTools::Widgets );
    this->widgetList->reset();

    // all done
    if ( !FolderManager::instance()->count())
        this->widgetList->show();
}

/**
 * @brief Main::writeConfiguration
 */
void Main::writeConfiguration() {
    if ( !this->hasInitialized())
        return;

    XMLTools::instance()->write( XMLTools::Widgets );
    XMLTools::instance()->write( XMLTools::Variables );
    XMLTools::instance()->write( XMLTools::Themes );
}

/**
 * @brief Main::reload
 */
void Main::reload() {
    // announce
    qInfo() << "reloading configuration";

    // save existing widget list
    this->writeConfiguration();

    // close all widgets
    FolderManager::instance()->shutdown();

    // clear icon cache
    IconCache::instance()->shutdown();

    // reload widget list
    this->readConfiguration();
}

/**
 * @brief Main::shutdown
 */
void Main::shutdown() {
    // abort doing shutdown routine twice
    if ( !this->hasInitialized())
        return;

    // announce
    qInfo() << "exit call received";

    // write out config
    this->writeConfiguration();

    // set not initialized
    this->setInitialized( false );

    // close all subsystems
    IndexCache::instance()->shutdown();
    IconCache::instance()->shutdown();
    IconIndex::instance()->shutdown();
    Themes::instance()->shutdown();
    FolderManager::instance()->shutdown();
    delete this->widgetList;

    // close all windows
    qApp->closeAllWindows();
    qApp->quit();
}
