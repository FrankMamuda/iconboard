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
#include <QMessageBox>
#include <QFileDialog>
#include <QDir>
#include <QScreen>
#include <QDebug>
#include "traywidget.h"
#include "ui_traywidget.h"
#include "widgetmodel.h"
#include "folderview.h"
#include "variable.h"
#include "settings.h"
#include "iconcache.h"
#include "screenmapper.h"
#include "xmltools.h"
#include "iconindex.h"
#include "about.h"
#include "themeeditor.h"

/**
 * @brief TrayWidget::TrayWidget
 * @param parent
 */
TrayWidget::TrayWidget( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::TrayWidget ), tray( new QSystemTrayIcon( QIcon( ":/icons/launcher_96" ))), model( new WidgetModel( this, this )), menu( new QMenu( this )) {
    // init ui
    this->ui->setupUi( this );
}

#ifdef Q_OS_WIN
#ifndef QT_DEBUG
void CALLBACK handleWinEvent( HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD );
#endif
#endif

/**
 * @brief TrayWidget::initialize
 */
void TrayWidget::initialize() {

    // set hook
#ifdef Q_OS_WIN
#ifdef QT_DEBUG
    // no need for unnecessary hooks in testing environment
    this->hook = nullptr;
#else
    this->hook = SetWinEventHook( EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr,
                                  handleWinEvent,
                                  0, 0, 0 );
#endif
#endif

    // initialize desktop widget
#ifdef Q_OS_WIN
    this->desktop = new DesktopWidget;
#endif

    // show tray icon
    this->tray->show();

    // init model
    this->ui->widgetList->setModel( this->model );

    // connect tray icon
    this->connect( this->tray, SIGNAL( activated( QSystemTrayIcon::ActivationReason )), this, SLOT( trayIconActivated( QSystemTrayIcon::ActivationReason )));

    // set up icons
    this->ui->actionAdd->setIcon( IconCache::instance()->icon( "list-add", 16 ));
    this->ui->actionRemove->setIcon( IconCache::instance()->icon( "list-remove", 16 ));
    this->ui->actionMap->setIcon( IconCache::instance()->icon( "view-grid", 16 ));
    this->ui->actionShow->setIcon( IconCache::instance()->icon( "visibility", 16 ));
    this->ui->buttonClose->setIcon( IconCache::instance()->icon( "dialog-close", 16 ));

    // reload on changed virtual geometry
    this->connect( qApp->primaryScreen(), SIGNAL( virtualGeometryChanged( QRect )), this, SLOT( reload()));

    // setup context menu
    QAction *actionSettings, *actionAbout, *actionTheme;
    this->menu->addAction( IconCache::instance()->icon( "view-list-icons", 16 ), this->tr( "Widget list" ), this, SLOT( show()));
    actionSettings = this->menu->addAction( IconCache::instance()->icon( "configure", 16 ), this->tr( "Settings" ));
    actionSettings->connect( actionSettings, &QAction::triggered, this, [ actionSettings, this ]() {
        Settings settingsDialog( this );
        settingsDialog.exec();
    });
    actionTheme = this->menu->addAction( IconCache::instance()->icon( "color-picker", 16 ), this->tr( "Theme editor" ));
    actionTheme->connect( actionTheme, &QAction::triggered, this, [ actionTheme, this ]() {
        ThemeEditor themeDialog;
        themeDialog.exec();
    });

    this->menu->addSeparator();
    actionAbout = this->menu->addAction( IconCache::instance()->icon( "help-about", 16 ), this->tr( "About" ));
    actionAbout->connect( actionAbout, &QAction::triggered, this, [ actionAbout, this ]() {
        About aboutDialog;
        aboutDialog.exec();
    });

    // exit lambda
    this->connect( this->menu->addAction( IconCache::instance()->icon( "application-exit", 16 ), this->tr( "Exit" )), &QAction::triggered, [this]() {
        this->writeConfiguration();

        foreach ( FolderView *fw, this->widgetList )
            delete fw;

#ifdef Q_OS_WIN
        if ( this->hook != nullptr ) {
            UnhookWinEvent( this->hook );
            this->hook = nullptr;
        }
#endif

        qInfo() << "exit call received";

        qApp->quit();
    } );

    this->tray->setContextMenu( this->menu );

    // read configuration
    this->readConfiguration();

    // bind iconTheme variable, to index new themes
    Variable::instance()->bind( "ui_iconTheme", this, SLOT( iconThemeChanged( QVariant )));

#ifndef QT_DEBUG
    // not available in release
    this->ui->toolBar->removeAction( this->ui->actionMap );
#endif

    // save settings every 60 seconds
    this->startTimer( 5000 );
}

/**
 * @brief TrayWidget::~TrayWidget
 */
TrayWidget::~TrayWidget() {
#ifdef Q_OS_WIN
    if ( this->hook != nullptr ) {
        UnhookWinEvent( this->hook );
        this->hook = nullptr;
    }
#endif

    this->disconnect( this->tray, SIGNAL( activated( QSystemTrayIcon::ActivationReason )));

#ifdef Q_OS_WIN
    delete this->desktop;
#endif
    delete this->model;
    delete this->menu;
    delete this->tray;
    delete this->ui;
}

/**
 * @brief TrayWidget::trayIconActivated
 */
void TrayWidget::trayIconActivated( QSystemTrayIcon::ActivationReason reason ) {
    switch ( reason ) {
    case QSystemTrayIcon::Trigger:
        if ( this->isHidden())
            this->show();
        else
            this->hide();
        break;

    case QSystemTrayIcon::Context:
        this->tray->contextMenu()->exec( QCursor::pos());
        break;

    default:
        break;
    }
}

/**
 * @brief TrayWidget::on_widgetList_doubleClicked
 * @param index
 */
void TrayWidget::on_widgetList_doubleClicked( const QModelIndex & ) {
    this->on_actionShow_triggered();
}

/**
 * @brief TrayWidget::on_actionAdd_triggered
 */
void TrayWidget::on_actionAdd_triggered() {
    QDir dir;

    dir.setPath( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));
    if ( dir.exists()) {
#ifdef Q_OS_WIN
        this->widgetList << new FolderView( this->desktop, dir.absolutePath());
#else
        this->widgetList << new FolderView( nullptr, dir.absolutePath());
#endif
        this->widgetList.last()->show();
        this->widgetList.last()->resetStyleSheet();
        this->widgetList.last()->sort();
        this->ui->widgetList->reset();
    }
}

/**
 * @brief TrayWidget::on_actionRemove_triggered
 */
void TrayWidget::on_actionRemove_triggered() {
    QMessageBox msgBox;
    FolderView *widget;
    int row, state;

    row = this->ui->widgetList->currentIndex().row();
    if ( row < 0 || row >= this->widgetList.count())
        return;

    // get widget ptr
    widget = this->widgetList.at( row );

    // display warning
    msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( widget->title()));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        widget->hide();
        this->widgetList.removeOne( widget );
        this->ui->widgetList->reset();
        //delete widget;
        break;

    case QMessageBox::No:
    default:
        return;
    }
}

/**
 * @brief TrayWidget::on_actionShow_triggered
 */
void TrayWidget::on_actionShow_triggered() {
    FolderView *widget;
    int row;

    // TODO: set checkable

    row = this->ui->widgetList->currentIndex().row();
    if ( row < 0 || row >= this->widgetList.count())
        return;

    // get widget ptr
    widget = this->widgetList.at( row );

    // toggle visibility
    if ( widget->isVisible())
        widget->hide();
    else
        widget->show();
}

/**
 * @brief TrayWidget::on_actionMap_triggered
 */
void TrayWidget::on_actionMap_triggered() {
    ScreenMapper mapperDialog;

    FolderView *widget;
    int row;

    row = this->ui->widgetList->currentIndex().row();
    if ( row < 0 || row >= this->widgetList.count())
        return;

    // get widget ptr
    widget = this->widgetList.at( row );

    mapperDialog.setWidgetRect( widget->geometry());
    mapperDialog.exec();

}

/**
 * @brief TrayWidget::on_buttonClose_clicked
 */
void TrayWidget::on_buttonClose_clicked() {
    this->hide();
}

/**
 * @brief TrayWidget::reload basically reloads all widgets
 */
void TrayWidget::reload() {
    // announce
    qInfo() << "reloading configuration";

    // save existing widget list
    this->writeConfiguration();

    // close all widgets
    foreach ( FolderView *widget, this->widgetList )
        widget->close();

    // clear widget list
    this->widgetList.clear();

    // clear icon cache
    IconCache::instance()->clearCache();

    // reload widget list
    this->readConfiguration();
}

/**
 * @brief TrayWidget::iconThemeChanged
 * @param value
 */
void TrayWidget::iconThemeChanged( QVariant value ) {
    QString themeName;

    themeName = value.toString();
    if ( themeName.isEmpty())
        return;

    if ( QString::compare( themeName, IconIndex::instance()->defaultTheme()) || QString::compare( themeName, "system" )) {
        IconIndex::instance()->build( themeName );
        IconIndex::instance()->setDefaultTheme( themeName );
        this->reload();
    }
}

/**
 * @brief TrayWidget::timerEvent
 * @param event
 */
void TrayWidget::timerEvent( QTimerEvent * ) {
    this->writeConfiguration();
}

/**
 * @brief TrayWidget::readConfiguration
 */
void TrayWidget::readConfiguration() {
    XMLTools::instance()->readConfiguration( XMLTools::Widgets );
    this->ui->widgetList->reset();
}

/**
 * @brief TrayWidget::writeConfiguration
 */
void TrayWidget::writeConfiguration() {
    XMLTools::instance()->writeConfiguration( XMLTools::Widgets );
    XMLTools::instance()->writeConfiguration( XMLTools::Variables );
    XMLTools::instance()->writeConfiguration( XMLTools::Themes );
}

#ifdef Q_OS_WIN
/**
 * @brief DesktopWidget::DesktopWidget
 * @param parent
 */
DesktopWidget::DesktopWidget(QWidget *parent) : QWidget( parent ), nativeEventIgnored( false ) {
    SetWindowLong( reinterpret_cast<HWND>( this->winId()), GWL_EXSTYLE, ( GetWindowLong( reinterpret_cast<HWND>( this->winId()), GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | ~WS_EX_APPWINDOW ));
}

/**
 * @brief DesktopWidget::lowerWindow
 */
void DesktopWidget::lowerWindow() {
    if ( !this->nativeEventIgnored ) {
        this->nativeEventIgnored = true;
        SetWindowPos( reinterpret_cast<HWND>( this->winId()), HWND_BOTTOM, 0,0,0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
        this->nativeEventIgnored = false;
    }
}

/**
 * @brief DesktopWidget::nativeEvent
 * @param eventType
 * @param message
 * @param result
 * @return
 */
bool DesktopWidget::nativeEvent( const QByteArray &eventType, void *message, long *result ) {
    MSG *msg;

    msg = static_cast<MSG*>( message );
    if ( msg->message == WM_WINDOWPOSCHANGED )
        this->lowerWindow();

    return QWidget::nativeEvent( eventType, message, result );
}

/**
 * @brief handleWinEvent this basically fixes the wrong z-order after using "Show Desktop" button
 * @param event
 * @param hwnd
 */
#ifndef QT_DEBUG
void CALLBACK handleWinEvent( HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD ) {
    if ( event == EVENT_SYSTEM_FOREGROUND ) {
        foreach ( FolderView *widget, TrayWidget::instance()->widgetList ) {
            if ( reinterpret_cast<HWND>( widget->winId()) == hwnd )
                TrayWidget::instance()->desktop->lowerWindow();
        }

        if ( reinterpret_cast<HWND>( TrayWidget::instance()->desktop->winId()))
            TrayWidget::instance()->desktop->lowerWindow();
    }
}
#endif
#endif
