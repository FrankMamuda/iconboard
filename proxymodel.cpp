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
#include "filesystemmodel.h"

/**
 * @brief ProxyModel::ProxyModel
 * @param parent
 */
ProxyModel::ProxyModel( QObject *parent ) : QSortFilterProxyModel( parent ), m_stopping( false ), threadPool( new QThreadPool( this )) {
    this->view = qobject_cast<FolderView*>( parent );
#ifdef ALT_PROXY_MODE
    this->connect( this, SIGNAL( iconFound( QString, QIcon )), this, SLOT( updateModel( QString, QIcon )));
#else
    this->connect( this, SIGNAL( iconFound( QString, QIcon, QPersistentModelIndex )), this, SLOT( updateModel( QString, QIcon, QPersistentModelIndex )));
#endif
}

/**
 * @brief ProxyModel::~ProxyModel
 */
ProxyModel::~ProxyModel() {
    this->waitForThreads();
    delete this->threadPool;

#ifdef ALT_PROXY_MODE
    this->disconnect( this, SIGNAL( iconFound( QString, QIcon )));
#else
    this->disconnect( this, SIGNAL( iconFound( QString, QIcon, QPersistentModelIndex )));
#endif
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
    this->threadPool->waitForDone();

    // allow updates and new threads
    this->blockSignals( false );
    this->reset();
}

/**
 * @brief ProxyModel::updateModel
 * @param fileName
 * @param icon
 * @param index
 */
#ifdef ALT_PROXY_MODE
void ProxyModel::updateModel( const QString &fileName, const QIcon &icon ) {
    int y;
    QModelIndex index;
    bool found = false;
#else
void ProxyModel::updateModel( const QString &fileName, const QIcon &icon, const QPersistentModelIndex &index ) {
#endif

    if ( icon.isNull())
        return;

#ifdef ALT_PROXY_MODE
    //
    // this is a little more inefficient, however we don't have to deal with QPersistentModelIndex
    // that is prone to corruption
    //
    for ( y = 0; y < this->rowCount( this->view->rootIndex()); y++ ) {
        index = this->index( y, 0, this->view->rootIndex());
        if ( !index.isValid())
            continue;

        if ( !QString::compare( index.data( QFileSystemModel::FilePathRole ).toString(), fileName )) {
            found = true;
            break;
        }
    }

    if ( !found )
        return;
#endif

    if ( !index.isValid())
        return;

    // NOTE: random segfault here (might be fixed by using QPersistentModelIndex)
    emit this->dataChanged( index, index );
    this->cache[fileName] = icon;
    this->queue.removeOne( fileName );
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

        // add to queue and avoid duplicates
        if ( this->queue.contains( fileName ))
            return QSortFilterProxyModel::data( index, role );
        else
            this->queue.append( fileName );

        // run fetcher
#ifdef ALT_PROXY_MODE
        QtConcurrent::run( this->threadPool, [ this, fileName, iconSize ] {
#else
        QPersistentModelIndex persistentIndex( index );
        QtConcurrent::run( this->threadPool, [ this, fileName, persistentIndex, iconSize ] {
#endif
            QIcon icon;

            if ( this->isStopping())
                return;

            /*
            QFileInfo info( fileName );
            if ( !info.isReadable())
                return;*/

            icon = IconCache::instance()->iconForFilename( fileName, iconSize );
            if ( !icon.isNull() || this->isStopping()) {
#ifdef ALT_PROXY_MODE
                emit this->iconFound( fileName, icon );
#else
                emit this->iconFound( fileName, icon, persistentIndex );
#endif
            }
        } );
    } else if ( role == Qt::DisplayRole ) {
#ifdef Q_OS_WIN
        // TODO: make this a static function (duplicate in IconCache)
        QRegularExpression re( "^([A-Z]):\\/$" );
        QRegularExpressionMatch match;

        fileName = index.data( QFileSystemModel::FilePathRole ).toString();

        match = re.match( fileName );
        if ( match.hasMatch())
            return match.captured( 1 ) + this->tr( " drive" );

        if ( fileName.endsWith( ".lnk" )) {
            return fileName.remove( ".lnk" ).split( "/" ).last();
        } else if ( fileName.endsWith( ".appref-ms" )) {
            return fileName.remove( ".appref-ms" ).split( "/" ).last();
        }
#endif
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
