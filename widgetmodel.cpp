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
    return FolderManager::instance()->count() + FolderManager::instance()->iconCount();
}

/**
 * @brief WidgetModel::data
 * @param index
 * @param role
 * @return
 */
QVariant WidgetModel::data( const QModelIndex &index, int role ) const {    
    //
    // TODO/FIXME: everything here is inefficient and needs an urgent rewrite
    //             with the use of QtConcurrent multithreading
    //

    if ( !index.isValid() || index.row() < 0 )
        return QVariant();

    switch ( role ) {
    case Qt::DecorationRole:
    {
        QPixmap pixmap;

        if ( index.row() >= FolderManager::instance()->count())
            pixmap = FolderManager::instance()->iconAt( index.row() - FolderManager::instance()->count())->thumbnail;
        else {
            FolderManager::instance()->at( index.row())->makeThumbnail();
            pixmap = FolderManager::instance()->at( index.row())->thumbnail;
        }

        if ( pixmap.isNull())
            pixmap = IconCache::instance()->icon( "application-x-zerosize", 24 ).pixmap( 24, 24 );

        if ( !pixmap.isNull())
            return pixmap.scaled( 24, 24, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }
        break;

    case Qt::FontRole:
    {
        QFont font;
        font.setBold( true );
        bool isVisible;

        if ( index.row() >= FolderManager::instance()->count())
            isVisible = FolderManager::instance()->iconAt( index.row() - FolderManager::instance()->count())->isVisible();
        else
            isVisible = FolderManager::instance()->at( index.row())->isVisible();

        return isVisible ? font : QFont();
    }

    case Qt::DisplayRole:
        if ( index.row() >= FolderManager::instance()->count())
            return FolderManager::instance()->iconAt( index.row() - FolderManager::instance()->count())->title() + " (icon)";

        return FolderManager::instance()->at( index.row())->title() + " (folder)";
    }

    return QVariant();
}
