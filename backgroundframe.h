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
#include <QPainter>
#include <QFrame>

/**
 * @brief The BackgroundFrame class
 */
class BackgroundFrame : public QFrame {
    Q_OBJECT

public:
    BackgroundFrame( QWidget *parent ) : QFrame( parent ) {}

protected:
    void paintEvent( QPaintEvent *event ) {
        QPainter painter( this );
        QBrush brush( Qt::black, Qt::FDiagPattern );
        painter.fillRect( this->rect(), brush );
        QFrame::paintEvent( event );
    }
};
