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
//
//
#include "iconsettings.h"
#include "ui_iconsettings.h"

/**
 * @brief IconSettings::IconSettings
 * @param parent
 */
IconSettings::IconSettings( QWidget *parent ) : QDialog(parent), ui( new Ui::IconSettings ) {
    this->ui->setupUi( this );
    this->ui->iconWidgetDemo->setIconSize( 128 );
    this->ui->iconSizeSlider->setValue( 128 );

    this->connect( this->ui->iconSizeSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setIconSize( value );
        //this->ui->frame->setFixedSize( this->ui->iconWidgetDemo->size());
    } );
}

/**
 * @brief IconSettings::~IconSettings
 */
IconSettings::~IconSettings() {
    delete this->ui;
}
