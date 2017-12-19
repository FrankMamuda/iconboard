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
#include "iconsettings.h"
#include "variable.h"
#include <QPainter>
#include <QFileInfo>
#include <QDebug>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QRgb>
#include <QMenu>

// TODO: allow larger rect for longer text?

/**
 * @brief DesktopIcon::DesktopIcon
 * @param parent
 */
DesktopIcon::DesktopIcon( QWidget *parent, const QString &target, qreal padding, int iconSize, const QString &customIcon ) : QWidget( parent ),
    m_target( target ), m_iconSize( iconSize ), m_previewIconSize( Icon::IconSize ),
    m_rows( Icon::RowCount ), m_columns( Icon::ColumnCount ),
    m_move( false ), m_padding( padding ), preview( nullptr ), m_textWidth( 1.5 ),
    m_shape( Circle ),
    m_background( Qt::white ),
    m_titleVisible( true ),
    m_hoverPreview( false ),
    m_hOffset( 0 ), m_vOffset( 0 ),
    m_customIcon( customIcon )
{
    // set up timer
    this->timer.setInterval( 200 );
    this->connect( &this->timer, &QTimer::timeout, [ this ]() {
        if ( !Variable::instance()->isEnabled( "app_lock" )) {
            if ( this->geometry().contains( QCursor::pos())) {
                this->m_move = true;
                this->setCursor( QCursor( Qt::ClosedHandCursor ));
                this->repaint();
            }
            this->timer.stop();
        }
    });

    // set object name for styling
    this->setObjectName( "DesktopIcon" );

    // set icon from target file or folder
    this->setCustomIcon( this->customIcon());
    if ( this->icon().isNull())
        qWarning() << this->tr( "invalid icon" );

    // adjust frame size
    this->adjustFrame();
}

/**
 * @brief DesktopIcon::~DesktopIcon
 */
DesktopIcon::~DesktopIcon() {
    if ( this->preview != nullptr )
        delete this->preview;
}

/**
 * @brief DesktopIcon::setIconSize
 * @param size
 */
void DesktopIcon::adjustFrame() {
    QFontMetrics fm( this->font());
    this->setFixedSize( static_cast<int>( this->iconSize() * this->textWidth()), this->iconSize() + fm.height());
}

/**
 * @brief DesktopIcon::setupPreview
 */
void DesktopIcon::setupPreview() {
    this->preview->setIconSize( this->previewIconSize());
    this->preview->setupPreviewMode( this->rows(), this->columns());
    this->preview->show();
    this->preview->sort();
}

/**
 * @brief DesktopIcon::loadIcon
 */
void DesktopIcon::setIcon() {
    int scale = static_cast<int>( this->iconSize() - this->padded() * 2 );

    if ( this->customIcon().isEmpty()) {
        QFileInfo info( this->target());
        QIcon icon;

        if ( !info.exists()) {
            icon = IconCache::instance()->icon( "messagebox_warning", scale );
            this->setTitle( this->tr( "Desktop icon" ));
        } else {
            this->setTitle( info.fileName());
#ifdef Q_OS_WIN
            if ( info.isDir())
                icon = IconCache::instance()->extractPixmap( info.absoluteFilePath(), scale );
            else
#endif
                icon = IconCache::instance()->iconForFilename( info.absoluteFilePath(), scale, true );
        }

        this->m_icon = icon;
    } else {
        this->m_icon = QIcon( this->customIcon());
    }
}

/**
 * @brief DesktopIcon::setCustomIcon
 * @param icon
 */
void DesktopIcon::setCustomIcon( const QString &icon ) {
    this->m_customIcon = icon;
    this->setIcon();
}

/**
 * @brief DesktopIcon::setupFrame
 */
void DesktopIcon::setupFrame() {
    // set up frame
    this->setAutoFillBackground( true );
    this->setMouseTracking( true );
    this->setWindowFlags( this->windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Window );
    this->setAttribute( Qt::WA_TranslucentBackground );
    this->setAttribute( Qt::WA_NoSystemBackground );
    this->setAttribute( Qt::WA_Hover );

    // filter events
    this->installEventFilter( this );

    // set up preview
    QFileInfo info( this->target());
    if ( this->preview == nullptr ) {
        this->preview = new FolderView( this, info.absoluteFilePath(), FolderView::Preview );
        this->setupPreview();
        this->preview->hide();
    }
}

/**
 * @brief DesktopIcon::paintEvent
 * @param event
 */
void DesktopIcon::paintEvent( QPaintEvent *event ) {
    QColor white( 255, 255, 255, 196 );
    QPixmap pixmap( this->iconSize(), this->iconSize());
    QPainter painter( this );
    qreal width = this->iconSize() * this->textWidth();
    qreal offset = ( width - this->iconSize()) / 2.0;
    int hOfs = static_cast<int>( this->iconSize() * this->hOffset());
    int vOfs = static_cast<int>( this->iconSize() * this->vOffset());

    pixmap.fill( Qt::transparent );
    {
        QPainter thumb( &pixmap );

        painter.setRenderHint( QPainter::Antialiasing );
        thumb.setRenderHint( QPainter::Antialiasing );
        painter.setPen( this->background());
        painter.setBrush( this->background());

        switch ( this->shape()) {
        case Circle:
            painter.drawEllipse( QPointF( width / 2.0, this->iconSize() / 2.0 ), this->iconSize() / 2.0 - 1.0, this->iconSize() / 2.0 - 1.0 );
            break;

        case Square:
            painter.fillRect( QRectF( offset, 0, this->iconSize() - 2.0, this->iconSize() - 2.0 ), QBrush( this->background()));
            break;

        case Rounded:
        {
            QPainterPath path;
            path.addRoundedRect( QRectF( offset, 0, this->iconSize() - 2, this->iconSize() - 2 ), this->iconSize() * 0.125, this->iconSize() * 0.125 );
            painter.fillPath( path, QBrush( this->background()));
        }
            break;

        case NoShape:
        case Plain:
            break;
        }

        thumb.drawEllipse( QPoint( this->iconSize() / 2, this->iconSize() / 2 ), this->iconSize() / 2 - 1, this->iconSize() / 2 - 1 );

        // draw folder icon
        if ( !this->icon().isNull()) {
            int pd = static_cast<int>( this->padded());
            int scale = this->iconSize() - pd * 2;

            QPixmap pm( this->icon().pixmap( QSize( scale, scale ), this->m_move ? QIcon::Disabled : QIcon::Normal ));
            painter.drawPixmap( static_cast<int>( pd + offset ) + hOfs, pd + vOfs, scale, scale, pm );
            thumb.drawPixmap( pd + hOfs, pd + vOfs, scale, scale, pm );
            this->thumbnail = pixmap.scaled( 48, 48 );
        }
    }

    // draw text
    QFontMetrics fm( this->font());
    if ( !this->title().isEmpty() && this->isTitleVisible()) {
        QRectF textRect( 0, this->iconSize(), width, fm.height());
        QString displayText( fm.elidedText( this->title(), Qt::ElideRight, static_cast<int>( textRect.width())));

        painter.setPen( Qt::black );
        painter.drawText( textRect.adjusted( 0, 2, 2, 0 ), Qt::AlignCenter, displayText );
        painter.setPen( Qt::white );
        painter.drawText( textRect, Qt::AlignCenter, displayText );
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
    // filter mouse events
    if ( event->type() == QEvent::MouseButtonPress ||
         event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove ||
         event->type() == QEvent::Enter || event->type() == QEvent::Leave
         ) {
        QMouseEvent *mouseEvent( static_cast<QMouseEvent*>( event ));

        // handle events
        switch ( event->type()) {
        case QEvent::MouseButtonPress:
            if ( mouseEvent->button() == Qt::LeftButton )
                this->timer.start();
            else
                this->timer.stop();

            return true;

        case QEvent::MouseButtonRelease:
            this->timer.stop();

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
                if ( mouseEvent->button() == Qt::RightButton && !Variable::instance()->isEnabled( "app_lock" )) {
                    QMenu menu;
                    menu.addAction( IconCache::instance()->icon( "configure", 16 ), this->tr( "Configure" ), [ this ]() {
                        IconSettings iconSettings;
                        iconSettings.setIcon( this );
                        iconSettings.exec();
                    } );
                    menu.exec( QCursor::pos());

                    return false;
                }

                if ( mouseEvent->button() != Qt::LeftButton )
                    return false;

                // open either folder preview or file
                QFileInfo info( this->target());
                if ( info.isDir()) {
                    this->setupPreview();
                } else {
                    // open file directly
                    QDesktopServices::openUrl( QUrl::fromLocalFile( info.absoluteFilePath()));
                }
            }
            return true;

        case QEvent::Enter:
            this->setCursor( QCursor( Qt::PointingHandCursor ));

            if ( this->hoverPreview() && Variable::instance()->isEnabled( "app_lock" )) {
                QFileInfo info( this->target());
                if ( info.isDir())
                    this->setupPreview();
            }
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
