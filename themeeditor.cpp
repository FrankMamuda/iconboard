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
#include "proxymodel.h"
#include "iconcache.h"
#include "themeeditor.h"
#include "ui_themeeditor.h"
#include "themes.h"
#include "themeeditor.h"
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QVariant>

/**
 * @brief ThemeEditor::ThemeEditor
 * @param parent
 */
ThemeEditor::ThemeEditor( QWidget *parent, Modes mode, const QString &styleSheet ) : QDialog( parent ), ui( new Ui::ThemeEditor ),
    model( new ThemeDemoModel( this )), toolBar( new QToolBar( this )), m_mode( mode ),
    label( new QLabel()), themeSelector( new QComboBox()) {
    // set up view
    this->ui->setupUi( this );
    this->ui->title->setAutoFillBackground( true );

    // something must be wrong
    if ( Themes::instance()->list.isEmpty() || this->ui->stackedWidget->count() != 2 )
        return;

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

        //
        // add toolbar buttons
        //

        // save
        this->connect( this->toolBar->addAction( IconCache::instance()->icon( "document-save", 16 ), this->tr( "Save" )), SIGNAL( triggered( bool )), this, SLOT( save()));

        // save as
        this->connect( this->toolBar->addAction( IconCache::instance()->icon( "document-save-as", 16 ), this->tr( "Save as" )), SIGNAL( triggered( bool )), this, SLOT( saveAs()));

        // rename
        this->connect( this->toolBar->addAction( IconCache::instance()->icon( "edit-rename", 16 ), this->tr( "Rename" )), &QAction::triggered, [this]() {
            QString name;

            if ( this->baseTheme()->builtIn()) {
                QMessageBox::information( this, this->tr( "Cannot rename theme" ), this->tr( "Cannot rename built-in theme \"%1\"" ).arg( this->baseTheme()->name()));
                return;
            }

            name = this->baseTheme()->name();
            this->saveAs();
            Themes::instance()->remove( name );
            this->populateThemes( this->baseTheme()->name());
        } );

        // revert
        this->connect( this->toolBar->addAction( IconCache::instance()->icon( "document-revert", 16 ), this->tr( "Revert" )), &QAction::triggered, [this]() {
            this->setEditorStyleSheet( this->baseTheme()->styleSheet());
        } );

        // remove
        this->connect( this->toolBar->addAction( IconCache::instance()->icon( "edit-delete", 16 ), this->tr( "Remove" )), &QAction::triggered, [this]() {
            if ( this->baseTheme()->builtIn()) {
                QMessageBox::information( this, this->tr( "Cannot remove theme" ), this->tr( "Cannot remove built-in theme \"%1\"" ).arg( this->baseTheme()->name()));
                return;
            }

            if ( QMessageBox::question( this, this->tr( "Remove theme" ),
                                        this->tr( "Are you sure you want to delete \"%1\" theme?" ).arg( this->baseTheme()->name()),
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::Yes ) == QMessageBox::Yes ) {
                Themes::instance()->remove( this->baseTheme()->name());
                this->populateThemes();
            }
        } );

        // add theme selector
        this->label->setText( this->tr( "Current theme:" ));
        this->toolBar->addWidget( this->label );
        this->toolBar->addWidget( this->themeSelector );

        // fill combo box
        this->populateThemes();

        // connect comboBox for updates
        this->connect( this->themeSelector, static_cast< void( QComboBox::* )( int )>( &QComboBox::activated ), [=]( int index ) {
            if ( index == -1 )
                return;

            if ( QString::compare( this->currentStyleSheet().simplified(), this->baseTheme()->styleSheet().simplified()))
                this->saveChangesPrompt();

            this->setBaseTheme( this->themeSelector->currentData().value<Theme*>());
        } );

        // enable close button
        this->ui->stackedWidget->setCurrentIndex( 1 );
        this->ui->stackedWidget->setFixedHeight( this->ui->closeButton->height());
    } else if ( this->mode() == Custom ) {
        // set custom theme
        this->setBaseTheme( new Theme( "custom", styleSheet, false ));

        // enable button box
        this->ui->stackedWidget->setCurrentIndex( 0 );
        this->ui->stackedWidget->setFixedHeight( this->ui->buttonBox->height());
    }
}

/**
 * @brief ThemeEditor::~ThemeEditor
 */
ThemeEditor::~ThemeEditor() {
    if ( this->mode() == Custom )
        delete this->m_baseTheme;

    delete this->label;
    delete this->themeSelector;
    delete this->delegate;
    delete this->model;
    delete this->toolBar;
    delete ui;
}

/**
 * @brief ThemeEditor::populateThemes
 */
void ThemeEditor::populateThemes( const QString &name ) {
    int y = 0, currentIndex = -1;

    // empty combobox
    this->themeSelector->clear();

    // add themes to comboBox
    foreach ( Theme *theme, Themes::instance()->list ) {
        this->themeSelector->addItem( theme->name(), QVariant::fromValue( theme ));

        if ( !QString::compare( theme->name(), name )) {
            currentIndex = y;
            this->setBaseTheme( theme );
        }

        y++;
    }

    // set first styleSheet in editor
    if ( name.isEmpty())
        this->setBaseTheme( Themes::instance()->list.first());
    else {
        if ( currentIndex != -1 && currentIndex < this->themeSelector->count())
             this->themeSelector->setCurrentIndex( currentIndex );
    }
}

/**
 * @brief ThemeEditor::saveChangesPrompt
 * @return
 */
void ThemeEditor::saveChangesPrompt() {
    if ( QMessageBox::question( this, this->tr( "StyleSheet has been changed" ),
                                this->tr( "Do you want to save changes to the \"%1\" theme?" ).arg( this->baseTheme()->name()),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::Yes ) == QMessageBox::Yes ) {
        this->save();
    }
}

/**
 * @brief ThemeEditor::save
 */
void ThemeEditor::save() {
    if ( !this->baseTheme()->builtIn()) {
        this->baseTheme()->setStyleSheet( this->currentStyleSheet());
    } else {
        QMessageBox::information( this, this->tr( "Cannot save theme" ), this->tr( "Cannot overwrite built-in theme \"%1\"\nTry a different name" ).arg( this->baseTheme()->name()));
        this->saveAs();
    }
}

/**
 * @brief ThemeEditor::saveAs
 */
void ThemeEditor::saveAs() {
    QString name;
    name = QInputDialog::getText( this->parentWidget(), this->tr( "Save theme" ), this->tr( "Name:" ), QLineEdit::Normal );

    if ( name.isEmpty()) {
        QMessageBox::warning( this, this->tr( "Cannot save theme" ), this->tr( "Empty theme name" ));
        return;
    }

    if ( Themes::instance()->contains( name )) {
        QMessageBox::warning( this, this->tr( "Cannot save theme" ), this->tr( "Theme \"%1\" already exists\nTry a different name" ).arg( name ));
        return;
    }

    Themes::instance()->add( name, this->currentStyleSheet(), false );
    this->populateThemes( name );
}

/**
 * @brief ThemeDemoModel::data
 * @param role
 * @return
 */
QVariant ThemeDemoModel::data( const QModelIndex &index, int role ) const {
    if ( role == Qt::DisplayRole )
        return QString( "Folder %1" ).arg( index.row() + 1 );

    if ( role == Qt::DecorationRole )
        return IconCache::instance()->icon( "inode-directory", 48 );

    return QVariant();
}
