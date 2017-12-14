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
#include <QDir>
#include <QFileDialog>
#include "folderview.h"
#include "foldermanager.h"
#include "screenmapper.h"
#include "widgetlist.h"
#include "iconindex.h"
#include "widgetmodel.h"
#include "variable.h"
#include "iconcache.h"
#include "main.h"
#include "ui_widgetlist.h"

/**
 * @brief WidgetList::WidgetList
 * @param parent
 */
WidgetList::WidgetList( QWidget *parent ) : QMainWindow( parent ), ui( new Ui::WidgetList ), model( new WidgetModel( this, this )),
    settingsDialog( new Settings( this )),
    themeDialog( new ThemeEditor( this )),
    aboutDialog( new About( this )) {
    // init ui
    this->ui->setupUi( this );

    // init model
    this->ui->widgetList->setModel( this->model );

    // set up icons
    this->ui->actionAdd->setIcon( IconCache::instance()->icon( "list-add", 16 ));
    this->ui->actionRemove->setIcon( IconCache::instance()->icon( "list-remove", 16 ));
    this->ui->actionMap->setIcon( IconCache::instance()->icon( "view-grid", 16 ));
    this->ui->actionShow->setIcon( IconCache::instance()->icon( "visibility", 16 ));
    this->ui->buttonClose->setIcon( IconCache::instance()->icon( "dialog-close", 16 ));

#ifndef QT_DEBUG
    // not available in release
    this->ui->toolBar->removeAction( this->ui->actionMap );
#endif
}

/**
 * @brief WidgetList::reset
 */
void WidgetList::reset() {
    this->ui->widgetList->reset();
}

/**
 * @brief WidgetList::~WidgetList
 */
WidgetList::~WidgetList() {
    // delete dialogs
    delete this->settingsDialog;
    delete this->aboutDialog;
    delete this->themeDialog;

    // delete model and the ui
    delete this->model;
    delete this->ui;
}

/**
 * @brief WidgetList::on_widgetList_doubleClicked
 * @param index
 */
void WidgetList::on_widgetList_doubleClicked( const QModelIndex & ) {
    this->on_actionShow_triggered();
}

/**
 * @brief WidgetList::on_actionAdd_triggered
 */
void WidgetList::on_actionAdd_triggered() {
    QMenu menu, *subMenu;
#ifdef Q_OS_WIN
    QWidget *widget = reinterpret_cast<QWidget*>( FolderManager::instance()->desktop );
#else
    QWidget *widget = nullptr;
#endif

    // add folder widget lambda
    menu.addAction( IconCache::instance()->icon( "inode-folder", 16 ), this->tr( "Folder" ), [ this, widget ]() {
        QDir dir( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));
        if ( dir.exists()) {
            FolderView *folderView;
            folderView = new FolderView( widget, dir.absolutePath());
            folderView->show();
            folderView->resetStyleSheet();
            folderView->sort();
            this->ui->widgetList->reset();
            FolderManager::instance()->add( folderView );
        }
    } );

    // subMenu icon->folder target
    //             ->file target
    subMenu = menu.addMenu( this->tr( "Icon" ));

    // addIcon lambda
    auto addIcon = [ this, widget ]( const QString &path ) {
        DesktopIcon *desktopIcon;
        desktopIcon = new DesktopIcon( widget, path );
        desktopIcon->show();
        FolderManager::instance()->add( desktopIcon );
    };

    // add icon widget lambda with FILE as its targer
    subMenu->addAction( IconCache::instance()->icon( "application-x-zerosize", 16 ), this->tr( "File" ), [ this, addIcon ]() {
        QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Select file or directory" ), "" ));
        QFileInfo info( fileName );
        if ( info.exists())
            addIcon( info.absoluteFilePath());

    } );

    // add icon widget lambda with FOLDER as its targer
    subMenu->addAction( IconCache::instance()->icon( "inode-folder", 16 ), this->tr( "Folder" ), [ this, addIcon ]() {
        QDir dir( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));
        if ( dir.exists())
            addIcon( dir.absolutePath());
    } );

    menu.exec( QCursor::pos());

    this->ui->widgetList->reset();
}

/**
 * @brief WidgetList::on_actionRemove_triggered
 */
void WidgetList::on_actionRemove_triggered() {
    QMessageBox msgBox;
    FolderView *folderView;
    DesktopIcon *desktopIcon;
    int state, index;

    index = this->ui->widgetList->currentIndex().row();

    // display warning
    msgBox.setText( this->tr( "Do you really want to remove this widget?" ));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        if ( index >= FolderManager::instance()->count()) {
            desktopIcon = FolderManager::instance()->iconAt( index - FolderManager::instance()->count());
            desktopIcon->hide();
            FolderManager::instance()->remove( desktopIcon );
        } else {
            folderView = FolderManager::instance()->at( index );
            folderView->hide();
            FolderManager::instance()->remove( folderView );
        }
        this->ui->widgetList->reset();
        break;

    case QMessageBox::No:
    default:
        return;
    }
}

/**
 * @brief WidgetList::on_actionShow_triggered
 */
void WidgetList::on_actionShow_triggered() {
    QWidget *widget;
    int index;

    index = this->ui->widgetList->currentIndex().row();

    // TODO: set checkable
    if ( index >= FolderManager::instance()->count())
        widget = qobject_cast<QWidget*>( FolderManager::instance()->iconAt( index - FolderManager::instance()->count()));
    else
        widget = qobject_cast<QWidget*>( FolderManager::instance()->at( index ));

    if ( widget == nullptr )
        return;

    // toggle visibility
    if ( widget->isVisible())
        widget->hide();
    else
        widget->show();
}

/**
 * @brief WidgetList::on_actionMap_triggered
 */
void WidgetList::on_actionMap_triggered() {
    ScreenMapper mapperDialog;
    QWidget *widget;
    int index;

    index = this->ui->widgetList->currentIndex().row();

    // TODO: set checkable
    if ( index >= FolderManager::instance()->count())
        widget = qobject_cast<QWidget*>( FolderManager::instance()->iconAt( index - FolderManager::instance()->count()));
    else
        widget = qobject_cast<QWidget*>( FolderManager::instance()->at( index ));

    if ( widget == nullptr )
        return;

    mapperDialog.setWidgetRect( widget->geometry());
    mapperDialog.exec();
}

/**
 * @brief WidgetList::on_buttonClose_clicked
 */
void WidgetList::on_buttonClose_clicked() {
    this->hide();
}
