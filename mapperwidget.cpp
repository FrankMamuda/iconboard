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
#include <QScreen>
#include "mapperwidget.h"
#include "screenmapper.h"

/**
 * @brief Widget::paintEvent
 * @param event
 */
void MapperWidget::paintEvent( QPaintEvent *event ) {
    QPainter painter( this );
    const QRect rect( QApplication::primaryScreen()->virtualGeometry());

    // make a pixmap of screen geometries
    QPixmap pixmap( rect.width(), rect.height());
    pixmap.fill( Qt::transparent );
    {
        QPainter screenPainter( &pixmap );
        int alpha = 64;
        QFont font;
        QPen pen;

        // translate
        screenPainter.translate( -rect.topLeft());
        screenPainter.fillRect( rect, QBrush( QColor::fromRgb( 0, 255, 0, 255 )));

        // go each individual screen
        foreach ( QScreen *screen, QApplication::screens()) {
            // fill screen rect
            screenPainter.fillRect( screen->geometry(), QColor::fromRgb( 0, 0, 0, alpha ));
            alpha += 64;

            if ( alpha > 255 )
                alpha = 64;

            // paint screen index
            const int index = QApplication::screens().indexOf( screen ) + 1;
            screenPainter.setPen( QColor::fromRgb( 255, 255, 255, 255 ));
            font.setPointSize( screen->geometry().height() / 2 );
            screenPainter.setFont( font );
            screenPainter.drawText( screen->geometry(), QString( "%1" ).arg( index ), { Qt::AlignCenter } );
        }

        // paint widget rect
        screenPainter.fillRect( this->screenMapper()->widgetRect, QBrush( QColor::fromRgb( 0, 0, 0, 128 ), Qt::SolidPattern ));
        font.setPointSize( this->screenMapper()->widgetRect.height() / 2 );
        screenPainter.setFont( font );
        pen = screenPainter.pen();
        pen.setWidth( 4 );
        screenPainter.setPen( pen );
        screenPainter.drawRect( this->screenMapper()->widgetRect );
        screenPainter.drawText( this->screenMapper()->widgetRect, "W", { Qt::AlignCenter } );
    }

    // resize and paint pixmap
    pixmap = pixmap.scaled( this->size() * 2, Qt::KeepAspectRatio, Qt::FastTransformation ).scaled( this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    painter.drawPixmap( this->width() / 2 - pixmap.width() / 2, this->height() / 2 - pixmap.height() / 2, pixmap.width(), pixmap.height(), pixmap );

    // paint widget as is
    QWidget::paintEvent( event );
}

/**
 * @brief MapperWidget::screenMapper
 * @return
 */
MapperWidget::MapperWidget(QWidget *parent) : QWidget( parent ) { }

/**
 * @brief MapperWidget::screenMapper
 * @return
 */
ScreenMapper *MapperWidget::screenMapper() {
    return qobject_cast<ScreenMapper*>( this->parentWidget());
}
