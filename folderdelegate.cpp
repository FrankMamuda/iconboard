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
#include <QApplication>
#include <QPainter>
#include "folderdelegate.h"
#include <QDebug>

/**
 * @brief FolderDelegate::FolderDelegate
 * @param parent
 */
FolderDelegate::FolderDelegate( QListView *parent ) {
    // set parent
    this->setParent( qobject_cast<QObject*>( parent ));
}

/**
 * @brief FolderDelegate::sizeHint
 * @param option
 * @param index
 * @return
 */
QSize FolderDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    ListItem item;
    QListView *view;
    QSize size;
    QStyleOptionViewItem customOption;
    QString text;

    // get parent listView
    view = qobject_cast<QListView*>( this->parent());
    if ( view == nullptr )
        return QStyledItemDelegate::sizeHint( option, index );

    // calculate proper size for multi-line text
    size = QStyledItemDelegate::sizeHint( option, index );

    // get display text
    text = view->model()->data( index, Qt::DisplayRole ).toString();

    // only icon mode has custom placement
    if ( view->viewMode() == QListView::IconMode ) {
        customOption = option;
        customOption.rect.setWidth( option.decorationSize.width() + FolderDelegateNamespace::MarginSide * 2 );

        if ( this->cache.contains( text )) {
            item = this->cache[text];
        } else {
            item = this->textItemForIndex( customOption, index );
            this->cache[text] = item;
        }

        size.setWidth( customOption.rect.width());
        size.setHeight( FolderDelegateNamespace::MarginTop + option.decorationSize.height() + item.lines.count() * item.textHeight );
    }

    return size;
}

/**
 * @brief textItemForIndex
 * @param text
 */
ListItem FolderDelegate::textItemForIndex( const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    QString text, line[FolderDelegateNamespace::TextLines];
    int y, textHeight, numLines = 0;
    QListView *view;
    ListItem item;

    // get parent listView
    view = qobject_cast<QListView*>( this->parent());
    if ( view == nullptr )
        return item;

    // get display text and height
    text = view->model()->data( index, Qt::DisplayRole ).toString();
    textHeight = option.fontMetrics.height();

    // split text into max 3 lines
    while ( FolderDelegateNamespace::TextLines - numLines ) {
        for ( y = 0; y < text.length(); y++ ) {
            if ( option.fontMetrics.width( text.left( y + 1 )) > option.rect.width() - FolderDelegateNamespace::MarginText * 2 )
                break;
        }

        if ( y > 0 ) {
            if ( numLines < FolderDelegateNamespace::TextLines - 1 ) {
                line[numLines] = text.left( y );
                text = text.mid( y, text.length() - y );
            } else
                line[numLines] = option.fontMetrics.elidedText( text, Qt::ElideRight, option.rect.width() - FolderDelegateNamespace::MarginText * 2 );
        } else
            break;

        numLines++;
    }

    // store data as a new display item
    for ( y = 0; y < numLines; y++ ) {
        item.lines << line[y];
        item.lineWidths << option.fontMetrics.width( line[y] ) + 1;
    }

    item.textHeight = textHeight;
    return item;
}

/**
 * @brief FolderDelegate::paint
 * @param painter
 * @param option
 * @param index
 */
void FolderDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const {
    int width, height, offset;
    QListView *view;
    QBrush hilightBrush;
    QString text;

    // get parent listView
    view = qobject_cast<QListView*>( this->parent());
    if ( view == nullptr )
        return;

    // save painter state & get hilight brush
    painter->save();
    hilightBrush = option.palette.highlight();
    painter->setPen( Qt::NoPen );

    // mouseOver/hover item
    if ( option.state & QStyle::State_MouseOver || option.state & QStyle::State_Selected ) {
        hilightBrush.setColor( QColor::fromRgbF( hilightBrush.color().redF(), hilightBrush.color().greenF(), hilightBrush.color().blueF(), 0.25 ));
        painter->fillRect( option.rect, hilightBrush );
    }

    // get display text
    text = view->model()->data( index, Qt::DisplayRole ).toString();

    // restore painter state
    painter->restore();

    // handle icon view
    if ( view->viewMode() == QListView::IconMode ) {
        ListItem item;
        QRect rect;
        QIcon icon;
        int y;
        QTextOption to;

        // get pixmap and its dimensions
        icon = qvariant_cast<QIcon>( view->model()->data( index, Qt::DecorationRole ));
        rect = option.rect;
        rect.setY( rect.y() + FolderDelegateNamespace::MarginTop );
        width = option.decorationSize.width();
        height = option.decorationSize.height();

        // properly position pixmap
        if ( width < rect.width()) {
            offset = rect.width() - width;
            rect.setX( rect.x() + offset / 2 );
            rect.setWidth( width );
        }

        // draw pixmap
        rect.setHeight( height );
        painter->drawPixmap( rect, icon.pixmap( rect.size()));

        // split text into multiple lines
        if ( cache.contains( text )) {
            item = cache[text];
        } else {
            item = this->textItemForIndex( option, index );
            cache[text] = item;
        }

        //item = this->textItemForIndex( option, index );
        to.setAlignment( Qt::AlignHCenter );

        // init text rectangle
        rect = option.rect;
        rect.setY( FolderDelegateNamespace::MarginTop + rect.y() + height - item.textHeight );

        // display multi-line text
        for ( y = 0; y < item.lines.count(); y++ ) {
            rect.setX( rect.x() + ( rect.width() - item.lineWidths.at( y )) / 2 );
            rect.setY( rect.y() + item.textHeight );
            rect.setHeight( item.textHeight );
            rect.setWidth( item.lineWidths.at( y ));
            painter->drawText( rect, item.lines.at( y ), to );
        }
    } else {
        QStyleOptionViewItem optionNoSelection;
        QStyle::State state;

        // remove hover/selection flags
        state = option.state;
        state = state & ( ~QStyle::State_MouseOver );
        state = state & ( ~QStyle::State_Selected );
        state = state & ( ~QStyle::State_HasFocus );
        state = state & ( ~QStyle::State_Active );

        optionNoSelection = option;
        optionNoSelection.state = state;

        // paint it exactly the same as before, yet ignoring selections
        QStyledItemDelegate::paint( painter, optionNoSelection, index );
    }
}
