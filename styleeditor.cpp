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
#include "folderview.h"
#include "iconproxymodel.h"
#include "iconcache.h"
#include "styleeditor.h"
#include "ui_styleeditor.h"
#include <QComboBox>

/**
 * @brief StyleEditor::StyleEditor
 * @param parent
 */
StyleEditor::StyleEditor( QWidget *parent, Modes mode, const QString &styleSheet ) : QDialog( parent ), ui( new Ui::StyleEditor ), model( new StyleDemoModel( this )), toolBar( new QToolBar( this )), m_mode( mode ) {
    // set up view
    this->ui->setupUi( this );

    // set up virtual folder widget
    this->ui->view->setViewMode( QListView::IconMode );
    this->ui->view->setModel( this->model );
    this->delegate = new FolderDelegate( this->ui->view );
    this->ui->view->setItemDelegate( this->delegate );

    // set up tab widget
    if ( this->ui->tabWidget->count() != 2 )
        return;

    this->ui->tabWidget->setTabText( 0, this->tr( "Preview" ));
    this->ui->tabWidget->setTabIcon( 0, IconCache::instance()->icon( "view-preview", 16 ));
    this->ui->tabWidget->setTabText( 1, this->tr( "Editor" ));
    this->ui->tabWidget->setTabIcon( 1, IconCache::instance()->icon( "color-picker", 16 ));

    // set window icon
    this->setWindowIcon( IconCache::instance()->icon( "color-picker" ));

    // set up toolBar
    if ( this->mode() == Full ) {
        this->toolBar->setFloatable( false );
        this->toolBar->setMovable( false );
        this->toolBar->setIconSize( QSize( 16, 16 ));
        this->toolBar->setAllowedAreas( Qt::TopToolBarArea );
        this->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
        this->toolBar->setStyleSheet( "QToolBar { border-bottom: 1px solid gray; }" );
        this->ui->centralLayout->setMenuBar( this->toolBar );

        // add toolbar buttons
        this->toolBar->addAction( IconCache::instance()->icon( "document-save", 16 ), this->tr( "Save" ));
        this->toolBar->addAction( IconCache::instance()->icon( "document-save-as", 16 ), this->tr( "Save as" ));
        this->toolBar->addAction( IconCache::instance()->icon( "edit-rename", 16 ), this->tr( "Rename" ));
        this->toolBar->addAction( IconCache::instance()->icon( "document-revert", 16 ), this->tr( "Revert" ));
        this->toolBar->addAction( IconCache::instance()->icon( "edit-delete", 16 ), this->tr( "Remove" ));
        this->toolBar->addWidget( new QLabel( this->tr( "Style:" )));
        this->toolBar->addWidget( new QComboBox());
    } else if ( this->mode() == Custom ) {
        this->ui->styleSheetEditor->setPlainText( styleSheet );
    }

    // do this after
    this->ui->view->setStyleSheet( this->ui->styleSheetEditor->toPlainText());
    this->ui->title->setStyleSheet( this->ui->styleSheetEditor->toPlainText());
    this->ui->title->setAutoFillBackground( true );
}

/**
 * @brief StyleEditor::~StyleEditor
 */
StyleEditor::~StyleEditor() {
    delete this->delegate;
    delete this->model;
    delete this->toolBar;
    delete ui;
}

/**
 * @brief StyleDemoModel::data
 * @param role
 * @return
 */
QVariant StyleDemoModel::data( const QModelIndex &index, int role ) const {
    if ( role == Qt::DisplayRole )
        return QString( "Folder %1" ).arg( index.row() + 1 );

    if ( role == Qt::DecorationRole )
        return IconCache::instance()->icon( "inode-directory", 48 );

    return QVariant();
}

/**
 * @brief StyleEditor::on_tabWidget_currentChanged
 * @param index
 */
void StyleEditor::on_tabWidget_currentChanged( int index ) {
    if ( index == 0 ) {
       this->ui->view->setStyleSheet( this->customStyleSheet());
       this->ui->title->setStyleSheet( this->customStyleSheet());
    }
}

/**
 * @brief StyleEditor::customStyleSheet
 * @return
 */
QString StyleEditor::customStyleSheet() const {
    return this->ui->styleSheetEditor->toPlainText();
}

