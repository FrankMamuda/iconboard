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
#include "screenmapper.h"
#include "ui_screenmapper.h"

/**
 * @brief Widget::Widget
 * @param parent
 */
ScreenMapper::ScreenMapper( QWidget *parent ) : QDialog( parent ), ui( new Ui::ScreenMapper ) {
    int y;

    this->ui->setupUi( this );

    // go each individual screen
    for ( y = 0; y < QApplication::screens().count(); y++ ) {
        QScreen *screen;

        screen = QApplication::screens().at( y );
        this->ui->comboScreens->addItem( QString( "%1: \"%2\"" ).arg( y + 1 ).arg( screen->name().remove( "\\" ).remove( "." )));
    }
}

/**
 * @brief Widget::~Widget
 */
ScreenMapper::~ScreenMapper() {
    delete this->ui;
}
