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
    QDir dir;

    dir.setPath( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));
    if ( dir.exists()) {
        FolderView *folderView;

#ifdef Q_OS_WIN
        folderView = new FolderView(( QWidget* )FolderManager::instance()->desktop, dir.absolutePath());
#else
        folderView = new FolderView( nullptr, dir.absolutePath());
#endif
        folderView->show();
        folderView->resetStyleSheet();
        folderView->sort();
        this->ui->widgetList->reset();
        FolderManager::instance()->add( folderView );
    }
}

/**
 * @brief WidgetList::on_actionRemove_triggered
 */
void WidgetList::on_actionRemove_triggered() {
    QMessageBox msgBox;
    FolderView *folderView;
    int state;

    folderView = FolderManager::instance()->at( this->ui->widgetList->currentIndex().row());
    if ( folderView == nullptr )
        return;

    // display warning
    msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( folderView->title()));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        folderView->hide();
        FolderManager::instance()->remove( folderView );
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
    FolderView *folderView;

    // TODO: set checkable

    folderView = FolderManager::instance()->at( this->ui->widgetList->currentIndex().row());
    if ( folderView == nullptr )
        return;

    // toggle visibility
    if ( folderView->isVisible())
        folderView->hide();
    else
        folderView->show();
}

/**
 * @brief WidgetList::on_actionMap_triggered
 */
void WidgetList::on_actionMap_triggered() {
    ScreenMapper mapperDialog;
    FolderView *folderView;

    folderView = FolderManager::instance()->at( this->ui->widgetList->currentIndex().row());
    if ( folderView == nullptr )
        return;

    mapperDialog.setWidgetRect( folderView->geometry());
    mapperDialog.exec();

}

/**
 * @brief WidgetList::on_buttonClose_clicked
 */
void WidgetList::on_buttonClose_clicked() {
    this->hide();
}

