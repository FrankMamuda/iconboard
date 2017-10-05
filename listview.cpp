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
#include "listview.h"
#include <QMimeData>

/**
 * @brief ListView::ListView
 */
ListView::ListView( QWidget *parent ) : QListView( parent ) {
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

/**
 * @brief ListView::setReadOnly
 * @param enable
 */
void ListView::setReadOnly( bool enable ) {
    enable = !enable;

    qDebug() << "  LW" << !enable;

    this->setDragEnabled( enable );
    this->viewport()->setAcceptDrops( enable );
    this->setDropIndicatorShown( enable );
    this->setDragDropMode( enable ? QAbstractItemView::DragDrop : QAbstractItemView::NoDragDrop );
    this->setAcceptDrops( enable );
    this->setDefaultDropAction( enable ? Qt::MoveAction : Qt::IgnoreAction );
}

/**
 * @brief ListView::dropEvent
 * @param event
 */
void ListView::dropEvent( QDropEvent *event ) {
    qDebug() << "drop" << event->mimeData()->text();
    QListView::dropEvent( event );
}
