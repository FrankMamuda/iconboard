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
#include "screenmapper.h"
#include "ui_screenmapper.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>

/**
 * @brief Widget::Widget
 * @param parent
 */
Widget::Widget( QWidget *parent ) : QWidget( parent ), ui( new Ui::Widget ) {
    this->ui->setupUi( this );
}

/**
 * @brief Widget::~Widget
 */
Widget::~Widget() {
    delete this->ui;
}

/**
 * @brief Widget::paintEvent
 * @param event
 */
void Widget::paintEvent( QPaintEvent *event ) {
    QPainter painter( this );
    QRect rect;

    // get whole geometry
    rect = QApplication::primaryScreen()->virtualGeometry();

    // make a pixmap of screen geometries
    QPixmap pixmap( rect.width(), rect.height());
    pixmap.fill( Qt::transparent );
    {
        QPainter screenPainter( &pixmap );
        int alpha = 64;

        // translate
        screenPainter.translate( -rect.topLeft());
        screenPainter.fillRect( rect, QBrush( QColor::fromRgb( 0, 255, 0, 255 )));

        // go each individual screen
        foreach ( QScreen *screen, QApplication::screens()) {
            int index;
            QTextOption textOption;
            QFont font;

            // fill screen rect
            screenPainter.fillRect( screen->geometry(), QColor::fromRgb( 0, 0, 0, alpha ));
            alpha += 64;

            if ( alpha > 255 )
                alpha = 64;

            // paint screen index
            index = QApplication::screens().indexOf( screen ) + 1;
            textOption.setAlignment( Qt::AlignCenter );
            screenPainter.setPen( QColor::fromRgb( 255, 255, 255, 255 ));
            font.setPointSize( screen->geometry().height() / 2 );
            screenPainter.setFont( font );
            screenPainter.drawText( screen->geometry(), QString( "%1" ).arg( index ), textOption );
            qDebug() << screen->name();
        }
    }

    // resize and paint pixmap
    pixmap = pixmap.scaled( this->size() * 2, Qt::KeepAspectRatio, Qt::FastTransformation ).scaled( this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation );
    painter.drawPixmap( 0, 0, pixmap.width(), pixmap.height(), pixmap );

    //this->resize( pixmap.size());

    // paint widget as is
    QWidget::paintEvent( event );
}

