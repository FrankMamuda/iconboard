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

#pragma once

//
// includes
//
#include <QStyledItemDelegate>
#include <QListView>
#include <QHash>

/**
 * @brief The ListItem class
 */
class ListItem {
public:
    QStringList lines;
    QList<int> lineWidths;
    int textHeight;
};
Q_DECLARE_METATYPE( ListItem )

/**
 * @brief The FolderDelegate class
 */
// TODO: allow changing these in stylesheet via props
namespace FolderDelegateNamespace {
static const int MarginTop = 4;
static const int MarginSide = 24;
static const int TextLines = 3;
static const int MarginText = 4;
}

/**
 * @brief The FolderDelegate class
 */
class FolderDelegate : public QStyledItemDelegate {
public:
    FolderDelegate( QListView *parent );
    ~FolderDelegate() {}

public slots:
    void clearCache() { this->cache.clear(); }

protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
    ListItem textItemForIndex( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    mutable QHash<QModelIndex, ListItem> cache;
};
