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
#include <QDebug>
#include "folderview.h"
#include "iconcache.h"
#include "listview.h"
#include <QFileSystemModel>
#include <QMenu>
#include <QMimeData>

/**
 * @brief ListView::ListView
 */
ListView::ListView( QWidget *parent ) : QListView( parent ) {
    this->setSelectionMode( QAbstractItemView::SingleSelection );
    this->setSelectionRectVisible( false );
    this->proxyStyle = new ProxyStyle( this->style());
    this->setStyle( this->proxyStyle );
}

/**
 * @brief ListView::~ListView
 */
ListView::~ListView() {
    // FIXME: deletion causes a segfault
    //delete this->proxyStyle;
}

/**
 * @brief ListView::setReadOnly
 * @param enable
 */
void ListView::setReadOnly( bool enable ) {
    enable = !enable;

    this->setDragEnabled( enable );
    this->viewport()->setAcceptDrops( enable );
    this->setDropIndicatorShown( enable );
    this->setDragDropMode( enable ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop );
    this->setAcceptDrops( enable );
    this->setDefaultDropAction( Qt::IgnoreAction );
}

/**
 * @brief ListView::dropEvent
 * @param event
 */
void ListView::dropEvent( QDropEvent *event ) {
    QMenu menu;

    // abort if same source
    // TODO: allow dropping on internal folders
    if ( event->source() == qobject_cast<QObject*>( this ))
        return;

    // ignore action by default
    event->setDropAction( Qt::IgnoreAction );

    // copy lambda
    this->connect( menu.addAction( IconCache::instance()->icon( "edit-copy", 16 ), this->tr( "Copy" )), &QAction::triggered, [event]() {
        event->setDropAction( Qt::CopyAction );
    } );

    // move lambda
    this->connect( menu.addAction( this->tr( "Move" )), &QAction::triggered, [event]() {
        event->setDropAction( Qt::MoveAction );
    } );

    // link lambda
    this->connect( menu.addAction( IconCache::instance()->icon( "insert-link", 16 ), this->tr( "Link" )), &QAction::triggered, [this, event]() {
        QString target;
        FolderView *folderView;
        QModelIndex index;

        // get parent widget
        folderView = qobject_cast<FolderView*>( this->parentWidget());
        if ( folderView == nullptr ) {
            qDebug() << "bad parent";
            return;
        }

        // get parent widget root path
        target = folderView->rootPath();

        // get item under cursor
        index = this->indexAt( event->pos());

        // get drop target full path
        if ( index.isValid())
            target = index.data( QFileSystemModel::FilePathRole ).toString();
        else
            target = folderView->rootPath();

        // check of dropped on file (not a directory!)
        if ( !QDir( target ).exists())
            return;

        // go through dropEvent fileNames
        if ( event->mimeData()->hasUrls()) {
            foreach ( QUrl url, event->mimeData()->urls()) {
                QString filePath( url.toLocalFile());
                QString link( target + "/" + QFileInfo( filePath ).fileName());

#ifdef Q_OS_WIN
                if ( !link.endsWith( ".lnk" ))
                    link.append( ".lnk" );

#endif
                QFile::link( filePath, link );
            }
        }
    } );
    menu.exec( QCursor::pos());

    // proceed with the intended action
    QListView::dropEvent( event );
}
