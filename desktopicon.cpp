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
#include "desktopicon.h"
#include "folderview.h"
#include "iconcache.h"
#include <QPainter>
#include <QFileInfo>
#include <QDebug>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QTapAndHoldGesture>
#include <QRgb>
#include <QMenu>

/**
 * @brief DesktopIcon::DesktopIcon
 * @param parent
 */
DesktopIcon::DesktopIcon( QWidget *parent, const QString &target ) : QWidget( parent ),
    m_target( target ), m_iconSize( Icon::IconSize ), m_previewIconSize( Icon::IconSize ),
    m_rows( Icon::RowCount ), m_columns( Icon::ColumnCount ),
    m_move( false ), m_padding( Icon::Padding ), preview( nullptr ) {
    // set object name for styling
    this->setObjectName( "DesktopIcon" );

    // TODO: display warning icon if anything fails
    if ( this->target().isEmpty()) {
        qCritical() << this->tr( "empty target" );
        this->close();
    }

    QFileInfo info( this->target());
    if ( !info.exists()) {
        qCritical() << this->tr( "invalid target" );
        this->close();
    } else {
        this->setTitle( info.fileName());
        this->preview = new FolderView( this, info.absoluteFilePath(), FolderView::Preview );
    }

    // set icon from target file or folder
    this->setIcon( IconCache::instance()->iconForFilename( info.absoluteFilePath(), this->iconSize()));
    if ( this->icon().isNull()) {
        qCritical() << this->tr( "invalid icon" );
        this->close();
    }

    // resize to icon size for now
    // TODO: add padding, margins, shadow, shape and other features later
    QFontMetrics fm( this->font());
    this->setFixedSize( this->iconSize(), this->iconSize() + fm.height());

    // set up frame
    this->setAutoFillBackground( true );
    this->setMouseTracking( true );
    this->setWindowFlags( this->windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Window );
    this->setAttribute( Qt::WA_TranslucentBackground );
    this->setAttribute( Qt::WA_NoSystemBackground );
    this->setAttribute( Qt::WA_Hover );

    // detect move event
    this->grabGesture( Qt::TapAndHoldGesture );
    QTapAndHoldGesture::setTimeout( 300 );

    // filter events
    this->installEventFilter( this );

    this->setRows( 3 );
    this->setColumns( 4 );
}

/**
 * @brief DesktopIcon::paintEvent
 * @param event
 */
void DesktopIcon::paintEvent( QPaintEvent *event ) {
    QColor white( 255, 255, 255, 196 );
    QPixmap pixmap( this->iconSize(), this->iconSize());
    QPainter painter( this );

    pixmap.fill( Qt::transparent );
    {
        QPainter thumb( &pixmap );

        painter.setRenderHint( QPainter::Antialiasing );
        thumb.setRenderHint( QPainter::Antialiasing );
        painter.setPen( white );
        painter.setBrush( white );
        painter.drawEllipse( QPoint( this->iconSize() / 2, this->iconSize() / 2 ), this->iconSize() / 2 - 1, this->iconSize() / 2 - 1 );
        thumb.drawEllipse( QPoint( this->iconSize() / 2, this->iconSize() / 2 ), this->iconSize() / 2 - 1, this->iconSize() / 2 - 1 );

        // draw folder icon
        if ( !this->icon().isNull()) {
            int scale = this->iconSize() - this->padding() * 2;
            painter.drawPixmap( this->padding(), this->padding(), scale, scale, this->icon().pixmap( QSize( this->iconSize() - this->padding() * 2, this->iconSize() - this->padding() * 2 ), this->m_move ? QIcon::Disabled : QIcon::Normal ));
            thumb.drawPixmap( this->padding(), this->padding(), scale, scale, this->icon().pixmap( QSize( this->iconSize() - this->padding() * 2, this->iconSize() - this->padding() * 2 ), this->m_move ? QIcon::Disabled : QIcon::Normal ));
            this->thumbnail = pixmap.scaled( 48, 48 );
        }
    }

    // draw text
    QFontMetrics fm( this->font());
    if ( !this->target().isEmpty()) {
        QRect textRect( 0, this->iconSize(), this->iconSize(), fm.height());

        painter.setPen( Qt::black );
        painter.drawText( textRect.adjusted( 0, 2, 2, 0 ), Qt::AlignCenter, this->title());
        painter.setPen( Qt::white );
        painter.drawText( textRect, Qt::AlignCenter, this->title());
    }

    // paint parent
    QWidget::paintEvent( event );
}

/**
 * @brief FolderView::eventFilter
 * @param object
 * @param event
 * @return
 */
bool DesktopIcon::eventFilter( QObject *object, QEvent *event ) {
    // handle tap-and hold (move) event
    if ( event->type() == QEvent::Gesture ) {
        if ( QGesture *gesture = static_cast<QGestureEvent*>( event )->gesture( Qt::TapAndHoldGesture )) {
            if ( gesture->state() == Qt::GestureStarted ) {
                this->m_move = true;
                this->setCursor( QCursor( Qt::ClosedHandCursor ));
                this->repaint();
                return true;
            }
        }
        return false;
    }

    // TODO: CHANGE CURSOR SHAPE ON ENTER

    // filter mouse events
    if ( event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove ||
         event->type() == QEvent::Enter || event->type() == QEvent::Leave
         ) {
        // handle events
        switch ( event->type()) {
        case QEvent::MouseButtonRelease:
            if ( this->m_move ) {
                // disable move gesture and repaint icon
                this->m_move = false;

                if ( this->geometry().contains( QCursor::pos()))
                    this->setCursor( QCursor( Qt::PointingHandCursor ));
                else
                    this->setCursor( QCursor( Qt::ArrowCursor ));

                this->repaint();
            } else {
                // abort on leave
                if ( !this->geometry().contains( QCursor::pos()))
                    return false;

                // context menu
                QMouseEvent *mouseEvent( static_cast<QMouseEvent*>( event ));
                if ( mouseEvent->button() == Qt::RightButton ) {
#ifdef QT_DEBUG
                    QMenu menu;
                    menu.addAction( IconCache::instance()->icon( "configure", 16 ), this->tr( "Configure" ), [](){ qDebug() << "unimplemented"; } );
                    menu.exec( QCursor::pos());
#endif
                    return false;
                }

                if ( mouseEvent->button() != Qt::LeftButton )
                    return false;

                // open either folder preview or file
                QFileInfo info( this->target());
                if ( info.isDir()) {
                    // show a new preview
                    this->preview->setIconSize( this->previewIconSize());
                    this->preview->show();
                    this->preview->sort();
                    this->preview->setupPreviewMode( this->rows(), this->columns());
                } else {
                    // open file directly
                    QDesktopServices::openUrl( QUrl::fromLocalFile( info.absoluteFilePath()));
                }
            }
            return true;

        case QEvent::Enter:
            this->setCursor( QCursor( Qt::PointingHandCursor ));
            break;

        case QEvent::Leave:
            this->setCursor( QCursor( Qt::ArrowCursor ));
            break;

        case QEvent::MouseMove:
            // handle move gesture
            if ( this->m_move ) {
                this->setGeometry( QCursor::pos().x() - this->width() / 2,
                                   QCursor::pos().y() - this->height() / 2,
                                   this->width(),
                                   this->height());
            }
            return true;

        default:
            break;
        }
    }

    // other events are handled normally
    return QWidget::eventFilter( object, event );
}