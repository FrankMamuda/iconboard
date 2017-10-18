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
#include "widgetmodel.h"
#include "widgetlist.h"
#include "folderview.h"
#include "foldermanager.h"

/**
 * @brief WidgetModel::rowCount
 * @return
 */
int WidgetModel::rowCount( const QModelIndex & ) const {
    return FolderManager::instance()->count();
}

/**
 * @brief WidgetModel::data
 * @param index
 * @param role
 * @return
 */
QVariant WidgetModel::data( const QModelIndex &index, int role ) const {
    if ( !index.isValid() || index.row() < 0 )
        return QVariant();

    switch ( role ) {
    case Qt::DecorationRole:
    {
        QPixmap pixmap;
        pixmap = FolderManager::instance()->at( index.row())->grab();

        // TODO: USE ASPECT RATIO OF SCREEN
        // and QtConcurrent
        if ( !pixmap.isNull())
            return pixmap.scaled( 40, 30 );
    }

    case Qt::DisplayRole:
        return FolderManager::instance()->at( index.row())->title();

    default:
        return QVariant();
    }
}
