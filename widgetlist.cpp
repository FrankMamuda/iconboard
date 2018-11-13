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
#include <QItemSelectionModel>

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
    this->ui->actionAdd->setIcon( IconCache::instance()->icon( "list-add", ":/icons/add", 16 ));
    this->ui->actionRemove->setIcon( IconCache::instance()->icon( "list-remove", ":/icons/remove", 16 ));
    this->ui->actionMap->setIcon( IconCache::instance()->icon( "view-grid", ":/icons/find", 16 ));
    this->ui->actionShow->setIcon( IconCache::instance()->icon( "visibility", ":/icons/visibility", 16 ));
    this->ui->buttonClose->setIcon( IconCache::instance()->icon( "dialog-close", ":/icons/close", 16 ));

#ifndef QT_DEBUG
    // not available in release
    this->ui->toolBar->removeAction( this->ui->actionMap );
#endif

    // setup hide/show button
    this->ui->actionShow->setCheckable( true );
    this->ui->actionShow->setChecked( false );
    this->connect( this->ui->widgetList->selectionModel(), &QItemSelectionModel::currentChanged, [ this ]( const QModelIndex &current, const QModelIndex & ) {
        const int index = current.row();
        bool isVisible;
        const QWidget *widget = ( index >= FolderManager::instance()->count()) ?
                    qobject_cast<QWidget*>( FolderManager::instance()->iconAt( index - FolderManager::instance()->count())) :
                    qobject_cast<QWidget*>( FolderManager::instance()->at( index ));

        isVisible = ( widget == nullptr ) ? false : widget->isVisible();

        this->ui->actionShow->setChecked( isVisible );
        this->ui->actionMap->setEnabled( isVisible );
        this->ui->actionShow->setText( isVisible ? this->tr( "Hide" ) : this->tr( "Show" ));
    } );
    //if ( this->model->rowCount())
    //    this->ui->widgetList->selectionModel()->select( this->model->index( 0, 0 ), QItemSelectionModel::Select );
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

    // add folder widget lambda
    menu.addAction( IconCache::instance()->icon( "inode-folder", 16 ), this->tr( "Folder widget" ), [ this ]() {
        const QDir dir( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));

        if ( dir.exists()) {
            FolderView *folderView( new FolderView( nullptr, dir.absolutePath()));

            if ( folderView == nullptr )
                return;

            folderView->show();
            folderView->resetStyleSheet();
            folderView->sort();
            this->ui->widgetList->reset();

            FolderManager::instance()->add( folderView );
        }
    } );

    // subMenu icon->folder target
    //             ->file target
    subMenu = menu.addMenu( this->tr( "Icon widget" ));

    // addIcon lambda
    auto addIcon = []( const QString &path ) {
        DesktopIcon *desktopIcon( new DesktopIcon( nullptr, path ));

        desktopIcon->setupFrame();
        desktopIcon->show();

        FolderManager::instance()->add( desktopIcon );
    };

    // add icon widget lambda with FILE as its targer
    subMenu->addAction( IconCache::instance()->icon( "application-x-zerosize", 16 ), this->tr( "File target" ), [ this, addIcon ]() {
        const QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Select file" ), "" ));
        const QFileInfo info( fileName );

        if ( info.exists())
            addIcon( info.absoluteFilePath());

    } );

    // add icon widget lambda with FOLDER as its targer
    subMenu->addAction( IconCache::instance()->icon( "inode-folder", 16 ), this->tr( "Folder target" ), [ this, addIcon ]() {
        QDir dir( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));

        // FIXME: adds on cancel anyway

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
    const int index = this->ui->widgetList->currentIndex().row();

    // display warning
    msgBox.setText( this->tr( "Do you really want to remove this widget?" ));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );

    // check options
    switch ( msgBox.exec()) {
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
    const int index = this->ui->widgetList->currentIndex().row();
    QWidget *widget = ( index >= FolderManager::instance()->count()) ?
                qobject_cast<QWidget*>( FolderManager::instance()->iconAt( index - FolderManager::instance()->count())) :
                qobject_cast<QWidget*>( FolderManager::instance()->at( index ));

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
    const int index = this->ui->widgetList->currentIndex().row();
    const QWidget *widget = ( index >= FolderManager::instance()->count()) ?
                qobject_cast<QWidget*>( FolderManager::instance()->iconAt( index - FolderManager::instance()->count())) :
                qobject_cast<QWidget*>( FolderManager::instance()->at( index ));

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
