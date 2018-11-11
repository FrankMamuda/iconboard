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
#include <QColorDialog>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

/**
 * @brief IconSettings::IconSettings
 * @param parent
 */
IconSettings::IconSettings( QWidget *parent ) : QDialog(parent), ui( new Ui::IconSettings ) {
    this->ui->setupUi( this );

    this->connect( this->ui->iconSizeSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setIconSize( value );
        this->ui->iconSize->setText( QString::number( value ));
    } );
    this->connect( this->ui->previewSizeSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setPreviewIconSize( value );
        this->ui->previewSize->setText( QString::number( value ));
    } );
    this->connect( this->ui->rowCountSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setRows( value );
        this->ui->rowCount->setText( QString::number( value ));
    } );
    this->connect( this->ui->columnCountSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setColumns( value );
        this->ui->columnCount->setText( QString::number( value ));
    } );
    this->connect( this->ui->paddingSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setPadding( static_cast<qreal>( value ) / 100.0 );
        this->ui->paddingValue->setText( QString::number( value ) + "%" );
    } );
    this->connect( this->ui->textEdit, &QLineEdit::textChanged, [ this ]( const QString &text ) {
        this->ui->iconWidgetDemo->setTitle( text );
    } );
    this->connect( this->ui->widthSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setTextWidth( static_cast<qreal>( value ) / 100.0 );
        this->ui->widthValue->setText( QString::number( value ) + "%" );
    } );
    this->connect( this->ui->iconShapeCombo, static_cast< void( QComboBox::* )( int )>( &QComboBox::activated ), [ this ]( int index ) {
        this->ui->iconWidgetDemo->setShape( static_cast<DesktopIcon::Shapes>( index ));
    } );
    this->connect( this->ui->backgroundButton, &QToolButton::clicked, [ this ]() {
        QColor colour( QColorDialog::getColor( this->ui->iconWidgetDemo->background(), this ));
        if ( !colour.isValid())
            return;

        this->setBackgroundColour( colour );
    } );
    this->connect( this->ui->titleCheckBox, &QCheckBox::toggled, [ this ]( bool enable ) {
        this->ui->iconWidgetDemo->setTitleVisible( enable );
    } );
    this->connect( this->ui->hoverCheckBox, &QCheckBox::toggled, [ this ]( bool enable ) {
        this->ui->iconWidgetDemo->setHoverPreview( enable );
    } );
    this->connect( this->ui->hOffsetSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setHOffset( static_cast<qreal>( value ) / 100.0 );
        this->ui->hOffset->setText( QString::number( value ) + "%" );
    } );
    this->connect( this->ui->vOffsetSlider, &QSlider::valueChanged, [ this ]( int value ) {
        this->ui->iconWidgetDemo->setVOffset( static_cast<qreal>( value ) / 100.0 );
        this->ui->vOffset->setText( QString::number( value ) + "%" );
    } );
    this->connect( this->ui->iconButton, &QPushButton::clicked, [ this ]() {
        QString fileName( QFileDialog::getOpenFileName( this, this->tr( "Select file" ), "", this->tr( "Icons (*.png)" )));
        QFileInfo info( fileName );
        if ( info.exists())
            this->ui->iconWidgetDemo->setCustomIcon( info.absoluteFilePath());
    } );
    this->connect( this->ui->resetButton, &QPushButton::clicked, [ this ]() {
        this->ui->iconWidgetDemo->setCustomIcon( QString() );
    } );

    // NOTE: for now
    //this->ui->hoverCheckBox->hide();
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
    this->ui->paddingSlider->setValue( static_cast<int>( icon->padding() * 100.0 ));
    this->ui->textEdit->setText( icon->title());
    this->ui->widthSlider->setValue( static_cast<int>( icon->textWidth() * 100.0 ));
    this->ui->iconShapeCombo->setCurrentIndex( static_cast<int>( icon->shape()));
    this->setBackgroundColour( icon->background());
    this->ui->titleCheckBox->setChecked( icon->isTitleVisible());
    this->ui->hoverCheckBox->setChecked( icon->hoverPreview());
    this->ui->hOffsetSlider->setValue( static_cast<int>( icon->hOffset() * 100.0 ));
    this->ui->vOffsetSlider->setValue( static_cast<int>( icon->vOffset() * 100.0 ));

    this->ui->iconWidgetDemo->setTarget( icon->target());
    this->ui->iconWidgetDemo->setIcon();

    const QFileInfo info( icon->target());
    if ( !info.isDir())
        this->ui->previewGroup->setDisabled( true );

    // setup text labels here
    this->ui->iconSize->setText( QString::number( icon->iconSize()));
    this->ui->previewSize->setText( QString::number( icon->previewIconSize()));
    this->ui->rowCount->setText( QString::number( icon->rows()));
    this->ui->columnCount->setText( QString::number( icon->columns()));
    this->ui->paddingValue->setText( QString::number( static_cast<int>( icon->padding() * 100.0 )) + "%" );
    this->ui->widthValue->setText( QString::number( static_cast<int>( icon->textWidth() * 100.0 )) + "%" );
    this->ui->hOffset->setText( QString::number( static_cast<int>( icon->hOffset() * 100.0 )) + "%" );
    this->ui->vOffset->setText( QString::number( static_cast<int>( icon->vOffset() * 100.0 )) + "%" );
    this->ui->iconWidgetDemo->setCustomIcon( icon->customIcon());
    this->ui->iconWidgetDemo->setShape( icon->shape());

    // finally set icon
    this->icon = icon;
}

/**
 * @brief IconSettings::setBackgroundColour
 * @param colour
 */
void IconSettings::setBackgroundColour( const QColor &colour ) {
    QPixmap icon( 16, 16 );
    icon.fill( Qt::transparent );

    QPainter painter( &icon );
    painter.setRenderHint( QPainter::Antialiasing );
    painter.setBrush( colour );
    painter.setPen( colour );
    painter.drawEllipse( 1, 1, 14, 14 );

    this->ui->backgroundButton->setIcon( QIcon( icon ));
    this->ui->iconWidgetDemo->setBackground( colour );
}

/**
 * @brief IconSettings::accept
 */
void IconSettings::accept() {
    this->icon->setIconSize( this->ui->iconSizeSlider->value());
    this->icon->setPreviewIconSize( this->ui->previewSizeSlider->value());
    this->icon->setRows( this->ui->rowCountSlider->value());
    this->icon->setColumns( this->ui->columnCountSlider->value());
    this->icon->setPadding( static_cast<qreal>( this->ui->paddingSlider->value() / 100.0 ));
    this->icon->setTitle( this->ui->textEdit->text());
    this->icon->setTextWidth( static_cast<qreal>( this->ui->widthSlider->value()) / 100.0 );
    this->icon->setShape( static_cast<DesktopIcon::Shapes>( this->ui->iconShapeCombo->currentIndex()));
    this->icon->setBackground( this->ui->iconWidgetDemo->background());
    this->icon->setTitleVisible( this->ui->titleCheckBox->isChecked());
    this->icon->setHoverPreview( this->ui->hoverCheckBox->isChecked());
    this->icon->setHOffset( static_cast<qreal>( this->ui->hOffsetSlider->value()) / 100.0 );
    this->icon->setVOffset( static_cast<qreal>( this->ui->vOffsetSlider->value()) / 100.0 );
    this->icon->setCustomIcon( this->ui->iconWidgetDemo->customIcon());

    QDialog::accept();
}
