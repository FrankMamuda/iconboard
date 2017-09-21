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
#include <QAbstractListModel>
#include <QDialog>
#include <QToolBar>

/**
 * The Ui namespace
 */
namespace Ui {
class StyleEditor;
}

/**
 * @brief The StyleDemoModel class
 */
class StyleDemoModel : public QAbstractListModel {
    Q_OBJECT

public:
    StyleDemoModel( QObject *parent ) : QAbstractListModel( parent ) {}
    int rowCount( const QModelIndex & = QModelIndex()) const override { return 3; }
    QVariant data( const QModelIndex &, int role ) const;
};

/**
 * @brief The StyleEditor class
 */
class StyleEditor : public QDialog {
    Q_OBJECT
    Q_ENUMS( Modes )

public:
    enum Modes {
        NoMode = -1,
        Full,
        Custom
    };
    explicit StyleEditor( QWidget *parent = 0, Modes mode = Full, const QString &styleSheet = QString::null );
    ~StyleEditor();
    Modes mode() const { return this->m_mode; }
    QString customStyleSheet() const;

private slots:
    void on_tabWidget_currentChanged( int index );

private:
    Ui::StyleEditor *ui;
    StyleDemoModel *model;
    QToolBar *toolBar;
    FolderDelegate *delegate;
    Modes m_mode;
};
