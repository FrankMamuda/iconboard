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
#include <QtConcurrent>
#include "folderview.h"
#include "proxymodel.h"
#include "iconcache.h"
#include <QDebug>

/**
 * @brief ProxyIdentityModel::ProxyIdentityModel
 * @param parent
 */
ProxyIdentityModel::ProxyIdentityModel( QObject *parent ) : QIdentityProxyModel( parent ) {
    this->view = qobject_cast<FolderView*>( parent );
    this->connect( this, SIGNAL( iconFound( QString, QIcon, QModelIndex )), this, SLOT( updateModel( QString, QIcon, QModelIndex )));
}

/**
 * @brief ProxyIdentityModel::~ProxyIdentityModel
 */
ProxyIdentityModel::~ProxyIdentityModel() {
    this->disconnect( this, SIGNAL( iconFound( QString, QIcon, QModelIndex )));
}

/**
 * @brief ProxyIdentityModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ProxyIdentityModel::data( const QModelIndex &index, int role ) const {
    QString fileName;
    int iconSize;

    if ( role == QFileSystemModel::FileIconRole ) {
        fileName = index.data( QFileSystemModel::FilePathRole ).toString();
        iconSize = this->view->iconSize();

        if ( this->cache.contains( fileName ))
            return this->cache[fileName];

        QtConcurrent::run( [ this, fileName, index, iconSize ] {
            QIcon icon;

            icon = IconCache::instance()->iconForFilename( fileName, iconSize );
            if ( !icon.isNull())
                emit this->iconFound( fileName, icon, index );
        } );
    } else if ( role == Qt::DisplayRole ) {
        fileName = index.data( QFileSystemModel::FilePathRole ).toString();
        if ( fileName.endsWith( ".lnk" ))
            return fileName.remove( ".lnk" ).split( "/" ).last();

        return QIdentityProxyModel::data( index, Qt::DisplayRole ).toString();
    }

    return QIdentityProxyModel::data( index, role );
}

/**
 * @brief ProxySortModel::ProxySortModel
 * @param parent
 */
ProxySortModel::ProxySortModel( QObject *parent ) : QSortFilterProxyModel( parent ) {
    this->view = qobject_cast<FolderView*>( parent );
}

/**
 * @brief ProxySortModel::lessThan
 * @param left
 * @param right
 * @return
 */
bool ProxySortModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const {
    if ( this->sortColumn() == 0 ) {
        QFileInfo leftInfo, rightInfo;
        FileSystemModel *fileSystemModel;
        bool compare, value;

        fileSystemModel = qobject_cast<FileSystemModel*>( this->sourceModel());
        if ( fileSystemModel == nullptr || this->view == nullptr )
            return QSortFilterProxyModel::lessThan( left, right );

        compare = this->view->sortOrder() == Qt::AscendingOrder ? true : false;
        leftInfo  = fileSystemModel->fileInfo( left );
        rightInfo = fileSystemModel->fileInfo( right );

        if ( this->view->directoriesFirst()) {
            if ( !leftInfo.isDir() && rightInfo.isDir())
                return false;

            if ( leftInfo.isDir() && !rightInfo.isDir())
                return true;
        }

        if ( this->view->isCaseSensitive()) {
            value = leftInfo.fileName() < rightInfo.fileName();
            return compare ? value : !value;
        }

        value = leftInfo.fileName().toLower() < rightInfo.fileName().toLower();
        return compare ? value : !value;
    }

    return QSortFilterProxyModel::lessThan( left, right );
}
