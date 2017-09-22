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
 * @brief ProxyModel::ProxyModel
 * @param parent
 */
ProxyModel::ProxyModel( QObject *parent ) : QSortFilterProxyModel( parent ) {
    this->view = qobject_cast<FolderView*>( parent );
    this->connect( this, SIGNAL( iconFound( QString, QIcon, QPersistentModelIndex )), this, SLOT( updateModel( QString, QIcon, QPersistentModelIndex )));
}

/**
 * @brief ProxyModel::~ProxyModel
 */
ProxyModel::~ProxyModel() {
    this->disconnect( this, SIGNAL( iconFound( QString, QIcon, QPersistentModelIndex )));
}

/**
 * @brief ProxyModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ProxyModel::data( const QModelIndex &index, int role ) const {
    QString fileName;
    QPersistentModelIndex persistentIndex( index );
    int iconSize;

    if ( role == QFileSystemModel::FileIconRole ) {
        fileName = index.data( QFileSystemModel::FilePathRole ).toString();
        iconSize = this->view->iconSize();

        if ( this->cache.contains( fileName ))
            return this->cache[fileName];

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

        return QSortFilterProxyModel::data( index, Qt::DisplayRole ).toString();
    }

    return QSortFilterProxyModel::data( index, role );
}

/**
 * @brief ProxyModel::lessThan
 * @param left
 * @param right
 * @return
 */
bool ProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const {
    bool compare;
    QFileInfo leftInfo, rightInfo;

    if ( this->sortColumn() == 0 ) {
        FileSystemModel *fileSystemModel;

        fileSystemModel = qobject_cast<FileSystemModel*>( this->sourceModel());
        if ( fileSystemModel == nullptr || this->view == nullptr ) {
            return QSortFilterProxyModel::lessThan( left, right );
        }

        compare = this->view->sortOrder() == Qt::AscendingOrder ? true : false;
        leftInfo  = fileSystemModel->fileInfo( left );
        rightInfo = fileSystemModel->fileInfo( right );
        //qDebug() << "sort" << leftInfo.fileName() << rightInfo.fileName() << compare << this->view->isCaseSensitive() << this->

        if ( this->view->directoriesFirst()) {
            if ( !leftInfo.isDir() && rightInfo.isDir())
                return !compare;

            if ( leftInfo.isDir() && !rightInfo.isDir())
                return compare;
        }

        bool value;
        if ( this->view->isCaseSensitive()) {
            value = QString::localeAwareCompare( leftInfo.fileName(), rightInfo.fileName());
            return compare ? value : !value;
        }

        value = QString::localeAwareCompare( leftInfo.fileName().toLower(), rightInfo.fileName().toLower());
        return compare ? value : !value;
    }

    return QSortFilterProxyModel::lessThan( left, right );
}
