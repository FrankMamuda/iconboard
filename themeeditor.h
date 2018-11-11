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

#pragma once

//
// includes
//
#include "iconcache.h"
#include "folderdelegate.h"
#include "ui_themeeditor.h"
#include "themes.h"
#include <QAbstractListModel>
#include <QComboBox>
#include <QDialog>
#include <QToolBar>

/**
 * The Ui namespace
 */
namespace Ui {
class ThemeEditor;
}

/**
 * @brief The ThemeDemoModel class
 */
class ThemeDemoModel : public QAbstractListModel {
    Q_OBJECT

public:
    ThemeDemoModel( QObject *parent ) : QAbstractListModel( parent ) {}
    int rowCount( const QModelIndex & = QModelIndex()) const override { return 3; }
    QVariant data( const QModelIndex &, int role ) const;
};

/**
 * @brief The ThemeEditor class
 */
class ThemeEditor : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "Theme editing dialog" )
    Q_ENUMS( Modes )

public:
    enum Modes {
        NoMode = -1,
        Full,
        Custom
    };
    explicit ThemeEditor( QWidget *parent = nullptr, Modes mode = Full, const QString &styleSheet = QString() );
    ~ThemeEditor();
    Modes mode() const { return this->m_mode; }
    QString currentStyleSheet() const { return this->ui->styleSheetEditor->toPlainText(); }
    Theme *baseTheme() const { return this->m_baseTheme; }

private slots:
    void on_tabWidget_currentChanged( int index ) { if ( index == 0 ) this->setWidgetStyleSheet(); }
    void setWidgetStyleSheet() { this->ui->view->setStyleSheet( this->currentStyleSheet()); this->ui->title->setStyleSheet( this->currentStyleSheet()); this->delegate->clearCache(); this->ui->view->setSpacing( this->ui->view->spacing()); }
    void setEditorStyleSheet( const QString &styleSheet ) { this->ui->styleSheetEditor->setPlainText( styleSheet ); this->setWidgetStyleSheet(); }
    void setBaseTheme( Theme *theme ) { this->m_baseTheme = theme; this->setEditorStyleSheet( this->baseTheme()->styleSheet()); }
    void save();
    void saveAs();
    void saveChangesPrompt();
    void populateThemes( const QString &name = QString() );

private:
    Ui::ThemeEditor *ui;
    ThemeDemoModel *model;
    QToolBar *toolBar;
    FolderDelegate *delegate;
    Modes m_mode;
    QLabel *label;
    QComboBox *themeSelector;
    Theme *m_baseTheme;
};
