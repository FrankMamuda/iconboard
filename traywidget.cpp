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
#include "styleeditor.h"

/**
 * @brief TrayWidget::TrayWidget
 * @param parent
 */
TrayWidget::TrayWidget( QWidget *parent ) : QMainWindow( parent/*, Qt::Tool*/ ), ui( new Ui::TrayWidget ), tray( new QSystemTrayIcon( QIcon( ":/icons/launcher_96" ))), model( new WidgetModel( this, this )), menu( new QMenu( this )) {
    // init ui
    this->ui->setupUi( this );

    // show tray icon
    this->tray->show();

    // init model
    this->ui->widgetList->setModel( this->model );

    // set up desktop widget
#ifdef Q_OS_WIN
    this->getWindowHandles();
#endif

    // connect tray icon
    this->connect( this->tray, SIGNAL( activated( QSystemTrayIcon::ActivationReason )), this, SLOT( trayIconActivated( QSystemTrayIcon::ActivationReason )));
    this->connect( qApp, SIGNAL( aboutToQuit()), this, SLOT( writeConfiguration()));

    // set up icons
    this->ui->actionAdd->setIcon( IconCache::instance()->icon( "list-add", 16 ));
    this->ui->actionRemove->setIcon( IconCache::instance()->icon( "list-remove", 16 ));
    this->ui->actionMap->setIcon( IconCache::instance()->icon( "view-grid", 16 ));
    this->ui->actionShow->setIcon( IconCache::instance()->icon( "visibility", 16 ));

    // reload on changed virtual geometry
    this->connect( qApp->primaryScreen(), SIGNAL( virtualGeometryChanged( QRect )), this, SLOT( reload()));

    // setup context menu
    QAction *actionSettings, *actionAbout, *actionStyle;
    this->menu->addAction( IconCache::instance()->icon( "view-list-icons", 16 ), this->tr( "Widget list" ), this, SLOT( show()));
    actionSettings = this->menu->addAction( IconCache::instance()->icon( "configure", 16 ), this->tr( "Settings" ));
    actionSettings->connect( actionSettings, &QAction::triggered, this, [ actionSettings, this ]() {
        Settings settingsDialog;
        settingsDialog.exec();
    });
#ifdef QT_DEBUG
    actionStyle = this->menu->addAction( this->tr( "Style editor" ));
    actionStyle->connect( actionStyle, &QAction::triggered, this, [ actionStyle, this ]() {
        StyleEditor styleDialog;
        styleDialog.exec();
    });
#endif
    this->menu->addSeparator();
    actionAbout = this->menu->addAction( IconCache::instance()->icon( "help-about", 16 ), this->tr( "About" ));
    actionAbout->connect( actionAbout, &QAction::triggered, this, [ actionAbout, this ]() {
        About aboutDialog;
        aboutDialog.exec();
    });
    this->menu->addAction( IconCache::instance()->icon( "application-exit", 16 ), this->tr( "Exit" ), qApp, SLOT( quit()));
    this->tray->setContextMenu( this->menu );

    // read configuration
    this->readConfiguration();

    // bind iconTheme variable, to index new themes
    Variable::instance()->bind( "ui_iconTheme", this, SLOT( iconThemeChanged( QVariant )));

#ifndef QT_DEBUG
    // not currently available
    this->ui->actionMap->setEnabled( false );
#endif
}

/**
 * @brief TrayWidget::~TrayWidget
 */
TrayWidget::~TrayWidget() {
    this->disconnect( this->tray, SIGNAL( activated( QSystemTrayIcon::ActivationReason )));
    //delete this->cache;
    //delete this->desktop;
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
 * @brief FolderView::getWindowHandles
 */
#ifdef Q_OS_WIN
void TrayWidget::getWindowHandles() {
    HWND desktop, progman, shell = nullptr, worker = nullptr;

    // get desktop window
    desktop = GetDesktopWindow();
    if ( desktop == nullptr )
        return;

    // get progman
    progman = FindWindowEx( desktop, 0, L"Progman", L"Program Manager" );
    if ( progman == nullptr )
        return;

    // get first worker and shell
    SendMessageTimeout( progman, 0x052C, 0, 0, SMTO_NORMAL, 3000, NULL );
    while( shell == nullptr ) {
        worker = FindWindowEx( desktop, worker, L"WorkerW", 0 );
        if ( worker != nullptr )
            shell = FindWindowEx( worker, 0, L"SHELLDLL_DefView", 0 );
        else
            break;
    }

    // store worker handle
    this->worker = worker;
}
#endif

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
        this->widgetList << new FolderView( nullptr, dir.absolutePath(), this->worker, this );
#else
        this->widgetList << new FolderView( nullptr, dir.absolutePath(), this );
#endif
        this->widgetList.last()->show();
        this->widgetList.last()->setDefaultStyleSheet();
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

    qDebug() << "set widget rect" << widget->geometry();
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
    qDebug() << "TrayWidget::reload: reloading configuration";

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
 * @brief TrayWidget::readConfiguration
 */
void TrayWidget::readConfiguration() {
    XMLTools::instance()->readConfiguration( XMLTools::Widgets, this );
    this->ui->widgetList->reset();
}

/**
 * @brief TrayWidget::writeConfiguration
 */
void TrayWidget::writeConfiguration() {
    XMLTools::instance()->writeConfiguration( XMLTools::Widgets, this );
    XMLTools::instance()->writeConfiguration( XMLTools::Variables );

#ifdef QT_DEBUG
    XMLTools::instance()->writeConfiguration( XMLTools::Styles );
#endif
}
