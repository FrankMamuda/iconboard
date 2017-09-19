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
#include <QFileSystemModel>
#include <QMimeDatabase>
#include <QtConcurrent>
#include "folderview.h"
#include "iconproxymodel.h"
#include "iconcache.h"

/**
 * @brief IconProxyModel::IconProxyModel
 * @param parent
 */
IconProxyModel::IconProxyModel( QObject *parent ) : QIdentityProxyModel( parent ) {
    this->view = qobject_cast<FolderView*>( parent );
    this->connect( this, SIGNAL( iconFound( QString, QIcon, QPersistentModelIndex )), this, SLOT( updateModel( QString, QIcon, QPersistentModelIndex )));
}

/**
 * @brief IconProxyModel::~IconProxyModel
 */
IconProxyModel::~IconProxyModel() {
    this->disconnect( this, SIGNAL( iconFound( QString, QIcon, QPersistentModelIndex )));
}

/**
 * @brief IconProxyModel::data
 * @param index
 * @param role
 * @return
 */
QVariant IconProxyModel::data( const QModelIndex &index, int role ) const {
    QString fileName;
    QPersistentModelIndex persistentIndex( index );
    int iconSize;

    if ( role == QFileSystemModel::FileIconRole ) {
        fileName = index.data( QFileSystemModel::FilePathRole ).toString();
        iconSize = this->view->iconSize();

        if ( this->iconTable.contains( fileName ))
            return this->iconTable[fileName];

        QtConcurrent::run( [ this, fileName, persistentIndex, iconSize ] {
            QIcon icon;

            icon = IconCache::instance()->iconForFilename( fileName, iconSize );
            if ( !icon.isNull())
                emit this->iconFound( fileName, icon, persistentIndex );
        } );
    } else if ( role == Qt::DisplayRole ) {
        fileName = index.data( QFileSystemModel::FilePathRole ).toString();
        if ( fileName.endsWith( ".lnk" ))
            return fileName.remove( ".lnk" ).split( "/" ).last();

        return QIdentityProxyModel::data( index, Qt::DisplayRole ).toString();
    }

    return QIdentityProxyModel::data( index, role );
}
