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
#include "desktopicon.h"
#include <QDebug>

/**
 * @brief IconSettings::IconSettings
 * @param parent
 */
IconSettings::IconSettings( QWidget *parent ) : QDialog(parent), ui( new Ui::IconSettings ) {
    this->ui->setupUi( this );
    this->connect( this->ui->iconSizeSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setIconSize( value );
    } );
    this->connect( this->ui->previewSizeSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setPreviewIconSize( value );
    } );
    this->connect( this->ui->rowCountSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setRows( value );
    } );
    this->connect( this->ui->columnCountSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setColumns( value );
    } );
    this->connect( this->ui->paddingSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setPadding( value );
    } );
    this->connect( this->ui->textEdit, &QLineEdit::textChanged, [ this ]( const QString &text ) {
        this->ui->iconWidgetDemo->setTitle( text );
    } );
    this->connect( this->ui->widthSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setTextWidth( static_cast<qreal>( value ) / 100.0 );
    } );
    this->connect( this->ui->iconShapeCombo, static_cast< void( QComboBox::* )( int )>( &QComboBox::activated ), [ this ]( int index ) {
        this->ui->iconWidgetDemo->setShape( static_cast<DesktopIcon::Shapes>( index ));
    } );
    // TODO: color
    this->connect( this->ui->titleCheckBox, &QCheckBox::toggled, [ this ]( bool enable ) {
        this->ui->iconWidgetDemo->setTitleVisible( enable );
    } );
    this->connect( this->ui->hoverCheckBox, &QCheckBox::toggled, [ this ]( bool enable ) {
        this->ui->iconWidgetDemo->setHoverPreview( enable );
    } );
}

/**
 * @brief IconSettings::~IconSettings
 */
IconSettings::~IconSettings() {
    delete this->ui;
}

/**
 * @brief IconSettings::setIcon
 * @param icon
 */
void IconSettings::setIcon( DesktopIcon *icon ) {
    // TODO: block signals?
    this->ui->iconSizeSlider->setValue( icon->iconSize());
    this->ui->previewSizeSlider->setValue( icon->previewIconSize());
    this->ui->rowCountSlider->setValue( icon->rows());
    this->ui->columnCountSlider->setValue( icon->columns());
    this->ui->paddingSlider->setValue( icon->padding());
    this->ui->textEdit->setText( icon->title());
    this->ui->widthSlider->setValue( static_cast<int>( icon->textWidth() * 100 ));
    this->ui->iconShapeCombo->setCurrentIndex( static_cast<int>( icon->shape()));
    // TODO: background
    this->ui->titleCheckBox->setChecked( icon->isTitleVisible());
    this->ui->titleCheckBox->setChecked( icon->hoverPreview());
    //this->ui->iconWidgetDemo->adjustFrame();
}
