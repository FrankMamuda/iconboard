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
#include "styleeditor.h"
#include "ui_styleeditor.h"

/**
 * @brief StyleEditor::StyleEditor
 * @param parent
 */
StyleEditor::StyleEditor( QWidget *parent ) : QDialog( parent ), ui( new Ui::StyleEditor ) {
    this->ui->setupUi( this );

    this->ui->listView->setViewMode( QListView::IconMode );
    QFileSystemModel *model = new FileSystemModel();
    IconProxyModel *proxyModel = new IconProxyModel( this );
    proxyModel->setSourceModel( model );
    this->ui->listView->setModel( proxyModel );
    this->ui->listView->setRootIndex( proxyModel->mapFromSource( model->setRootPath( QDir::homePath())));
}

/**
 * @brief StyleEditor::~StyleEditor
 */
StyleEditor::~StyleEditor() {
    delete ui;
}
