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
// TODO: allow changing these in styleSheet via props
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
    Q_DISABLE_COPY( FolderDelegate )
    Q_PROPERTY( int textLineCount READ textLineCount WRITE setTextLineCount )
    Q_PROPERTY( bool selectionVisible READ isSelectionVisible WRITE setSelectionVisible )

public:
    explicit FolderDelegate( QListView *parent = nullptr );
    ~FolderDelegate() {}
    int textLineCount() const { return this->m_textLineCount; }
    bool isSelectionVisible() const { return this->m_selectionVisible; }
    int topMargin() const { return this->m_textLineCount; }
    int sideMargin() const { return this->m_sideMargin; }
    int textMargin() const { return this->m_textMargin; }

public slots:
    void clearCache() { this->cache.clear(); }
    void setTextLineCount( int count = FolderDelegateNamespace::TextLines ) { this->m_textLineCount = count; }
    void setSelectionVisible( bool enable ) { this->m_selectionVisible = enable; }
    void setTopMargin( int margin ) { this->m_topMargin = margin; }
    void setSideMargin( int margin ) { this->m_sideMargin = margin; }
    void setTextMargin( int margin ) { this->m_textMargin = margin; }

protected:
    void paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const;

private:
    ListItem textItemForIndex( const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    mutable QHash<QString, ListItem> cache;
    int m_textLineCount;
    bool m_selectionVisible;
    int m_topMargin;
    int m_sideMargin;
    int m_textMargin;
};
