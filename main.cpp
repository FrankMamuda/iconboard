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
#include <QScreen>
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
 *      very hard to reproduce, very rare bug (might be Qt related)
 *    i18n
 *    cleanup
 *    GitHub page
 *    release beta
 *    win32: for some reason some icons fail to get shell icon on reload
 *
 *  [NOT URGENT]
 *  [to be implemented/fixed in future versions]
 *    root folder ('My Computer') folder support
 *    custom alignment
 *    free placement, free scaling
 *    whole desktop replacement option
 *    multi column list
 *    custom per-item icons
 *    thumbnail loading as an option
 *    folderView spacing, other props
 *    macOS issues
 *    sorting issues on network folders (dirsFirst not working)
 *    complete documentation
 *    QWindow::requestActivate: requestActivate() FolderViewWindow
 *    first run dialog
 *    option to download icon packs
 *    option not to upscale small icons (center them instead)
 *    custom icon dir (set in settings)
 *    minor issues with z-order (windows)
 *    handle shortcut/folder drops on tray icon or widget list to add widget
 *    fix flickering with batched resize (currently listview set to singlepass)
 *    folder previews in popups
 *    fix no icons by default on linux (add some build in basic file and folder)
 *    alias (custom naming) of items
 *    basic support for .desktop files (use for drag and drop instead of lnk)
 *    semi-merger with filemanager project (enable build configuration to build either or)
 *    linux segfault on icon theme change (older Qt versions)
 *    horizontal centering in QListView
 *    allow QIcon::fromTheme on unix (instead of manual loading)
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

// default message handler
static const QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER = qInstallMessageHandler( 0 );

/**
 * @brief messageFilter
 * @param type
 * @param context
 * @param msg
 */
void Main::messageFilter( QtMsgType type, const QMessageLogContext &context, const QString &msg ) {
    QFile logFile;
    QString output( msg );

    // display message as is
    (*QT_DEFAULT_MESSAGE_HANDLER)( type, context, msg );

    // quit app on fatal errors
    if ( type == QtFatalMsg ) {
        Main::instance()->shutdown();
        exit( 0 );
    }

    // log to file, when not in debug mode
    if ( qgetenv( "QTDIR" ).isEmpty()) {
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
}

/**
 * @brief Main::currentResolution
 * @return
 */
QSize Main::currentResolution() {
    return qApp->primaryScreen()->size();
}

/**
 * @brief Main::targetResolution
 * @return
 */
QSize Main::targetResolution() {
    QSize targetResolution( Variable::instance()->value<QSize>( "app_targetResolution" ));

    if ( targetResolution.isEmpty() || !targetResolution.isValid())
        return Main::instance()->currentResolution();

    return targetResolution;
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
    Variable::instance()->add( "app_lockToResolution", false );
    Variable::instance()->add( "app_targetResolution", "" );
    Variable::instance()->add( "app_lock", false );
    XMLTools::instance()->read( XMLTools::Variables );
    XMLTools::instance()->read( XMLTools::Themes );
    Variable::instance()->bind( "app_lock", XMLTools::instance(), SLOT( saveOnLock( QVariant )));

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
Main::Main( QObject *parent ) : QObject( parent ), m_initialized( false ), m_reloadScheduled( false ) {
    // announce
#ifdef QT_DEBUG
    qInfo() << this->tr( "initializing" );
#endif

    // save settings every 60 seconds
    this->startTimer( 60 * 1000 );

    // set initialized
    this->setInitialized();

    this->widgetList = new WidgetList();
    this->tray = new TrayIcon( this->widgetList );

    // reload on changed virtual geometry
    this->connect( qApp->primaryScreen(), SIGNAL( virtualGeometryChanged( QRect )), this, SLOT( scheduleReload()));
    this->connect( qApp, SIGNAL( screenAdded( QScreen* )), this, SLOT( scheduleReload()));
    this->connect( qApp, SIGNAL( screenRemoved( QScreen* )), this, SLOT( scheduleReload()));
    this->connect( &this->timer, SIGNAL( timeout()), this, SLOT( reload()));
    this->timer.setInterval( 1000 );

    // bind iconTheme variable, to index new themes
    Variable::instance()->bind( "ui_iconTheme", this, SLOT( iconThemeChanged( QVariant )));
}

/**
 * @brief Main::iconThemeChanged
 * @param value
 */
void Main::iconThemeChanged( QVariant value ) {
    QString themeName;

    themeName = value.toString();
    if ( themeName.isEmpty())
        return;

    if ( QString::compare( themeName, IconIndex::instance()->defaultTheme()) || QString::compare( themeName, "system" )) {
        IconIndex::instance()->build( themeName );
        IconIndex::instance()->setDefaultTheme( themeName );

        Main::instance()->reload();
    }
}

/**
 * @brief Main::readConfiguration
 */
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
void Main::writeConfiguration( bool force ) {
    if ( !this->hasInitialized())
        return;

    if ( this->reloadScheduled())
        return;

    XMLTools::instance()->write( XMLTools::Widgets, force );
    XMLTools::instance()->write( XMLTools::Variables, force );
    XMLTools::instance()->write( XMLTools::Themes, force );
}

/**
 * @brief Main::reload
 */
void Main::reload() {
    // stop reload timer
    this->m_reloadScheduled = false;
    this->timer.stop();

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
 * @brief Main::scheduleReload
 */
void Main::scheduleReload() {
    this->m_reloadScheduled = true;
    this->timer.stop();
    this->timer.start();
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
