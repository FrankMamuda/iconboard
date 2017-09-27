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
ProxyModel::ProxyModel( QObject *parent ) : QSortFilterProxyModel( parent ), m_stopping( false ) {
    this->view = qobject_cast<FolderView*>( parent );
    this->connect( this, SIGNAL( iconFound( QString, QIcon, QModelIndex )), this, SLOT( updateModel( QString, QIcon, QModelIndex )));
}

/**
 * @brief ProxyModel::~ProxyModel
 */
ProxyModel::~ProxyModel() {
    this->disconnect( this, SIGNAL( iconFound( QString, QIcon, QModelIndex )));
}

/**
 * @brief ProxyModel::waitForThreads
 */
void ProxyModel::waitForThreads() {
    // stop receiving any pending updates
    this->blockSignals( true );

    // signal all active threads to stop
    this->stop();

    // wait for threads to finish computation and empty the thread pool
    foreach ( QFuture<void> future, this->threadPool )
        future.waitForFinished();
    this->threadPool.clear();

    // allow updates and new threads
    this->blockSignals( false );
    this->reset();
}

/**
 * @brief ProxyModel::data
 * @param index
 * @param role
 * @return
 */
QVariant ProxyModel::data( const QModelIndex &index, int role ) const {
    QString fileName;
    int iconSize;

    if ( this->isStopping())
        return QVariant();

    if ( role == QFileSystemModel::FileIconRole ) {
        fileName = index.data( QFileSystemModel::FilePathRole ).toString();
        iconSize = this->view->iconSize();

        if ( this->cache.contains( fileName ))
            return this->cache[fileName];

        // run fetcher
        QFuture<void> future = QtConcurrent::run( [ this, fileName, index, iconSize ] {
            QIcon icon;

            if ( this->isStopping())
                return;

            QFileInfo info( fileName );
            if ( !info.isReadable())
                return;

            icon = IconCache::instance()->iconForFilename( fileName, iconSize );
            if ( !icon.isNull() || this->isStopping())
                emit this->iconFound( fileName, icon, index );
        } );
        this->threadPool.append( future );
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
