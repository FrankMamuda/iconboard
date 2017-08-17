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
#include <QDebug>
#include <QCheckBox>
#include <QSpinBox>
#include <QSettings>
#include "settings.h"
#include "ui_settings.h"

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings( QWidget *parent ) : QDialog( parent ), ui( new Ui::Settings ), signalMapper( new QSignalMapper( this )) {
    this->ui->setupUi( this );

    // connect for updates
    this->connect( Variable::instance(), SIGNAL( valueChanged( QString )), this, SLOT( externalValueChanged( QString )));
    this->connect( this->signalMapper, SIGNAL( mapped( QString )), this, SLOT( internalValueChanged( QString )));

    // first bind vars and set initial values
    this->bind( "ui_displaySymlinkIcon", this->ui->displaySymlinkIcon );
    this->bind( "app_runOnStartup", this->ui->runOnStartup );

    // bind runOnStarup variable, to write out settings value
    Variable::instance()->bind( "app_runOnStartup", this, SLOT( runOnStartupValueChanged( QVariant )));
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings(){
    this->disconnect( Variable::instance(), SIGNAL( valueChanged( QString )));
    this->disconnect( this->signalMapper, SIGNAL( mapped( QString )));
    delete this->ui;
}

/**
 * @brief Settings::bind
 * @param variable
 * @param widget
 */
void Settings::bind( const QString &key, QWidget *widget ) {
    this->boundVariables[key] = widget;
    this->setValue( key, false );

    // determine widget type
    // TODO: expand to other widget types in future
    if ( !QString::compare( widget->metaObject()->className(), "QCheckBox" )) {
        QCheckBox *checkBox;
        checkBox = qobject_cast<QCheckBox*>( widget );

        // connect for further updates
        if ( checkBox != nullptr ) {
            this->connect( checkBox, SIGNAL( stateChanged( int )), this->signalMapper, SLOT( map()));
            signalMapper->setMapping( checkBox, key );
        }
    } else if ( !QString::compare( widget->metaObject()->className(), "QSpinBox" )) {
        QSpinBox *spinBox;
        spinBox = qobject_cast<QSpinBox*>( widget );

        // connect for further updates
        if ( spinBox != nullptr ) {
            this->connect( spinBox, SIGNAL( valueChanged( int )), this->signalMapper, SLOT( map()));
            signalMapper->setMapping( spinBox, key );
        }
    } else {
        qDebug() << "Settings::bind: unsupported container" << widget->metaObject()->className();
    }
}

/**
 * @brief Settings::setValue
 * @param key
 */
void Settings::setValue( const QString &key, bool internal ) {
    QWidget *widget;

    if ( !this->boundVariables.contains( key ))
        return;

    // get widget and block it's signals
    widget = this->boundVariables[key];
    widget->blockSignals( true );

    // determine widget type
    if ( !QString::compare( widget->metaObject()->className(), "QCheckBox" )) {
        QCheckBox *checkBox;

        checkBox = qobject_cast<QCheckBox*>( widget );

        if ( checkBox != nullptr ) {
            if ( internal )
                Variable::instance()->setValue<bool>( key, checkBox->isChecked(), true );
            else
                checkBox->setChecked( Variable::instance()->isEnabled( key ));
        }
    } else if ( !QString::compare( widget->metaObject()->className(), "QSpinBox" )) {
        QSpinBox *spinBox;

        spinBox = qobject_cast<QSpinBox*>( widget );

        if ( spinBox != nullptr ) {
            if ( internal )
                Variable::instance()->setValue<int>( key, spinBox->value(), true );
            else
                spinBox->setValue( Variable::instance()->integer( key ));
        }
    } else {
        qDebug() << "Settings::setValue: unsupported container" << widget->metaObject()->className();
    }

    // force update
    if ( internal )
        Variable::instance()->updateConnections( key, Variable::instance()->value<QVariant>( key ));

    // unblock signals
    widget->blockSignals( false );
}

/**
 * @brief Settings::on_closeButton_clicked
 */
void Settings::on_closeButton_clicked() {
    this->close();
}

/**
 * @brief Settings::runOnStartupValueChanged
 * @param value
 */
void Settings::runOnStartupValueChanged( QVariant value ) {
#ifdef QT_DEBUG
    qDebug() << "runOnStartupValueChanged change to" << value.toBool();
#else
    QSettings settings( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat );

    if ( value.toBool())
        settings.setValue( "IconBoardApp", QCoreApplication::applicationFilePath().replace( '/', '\\' ));
    else
        settings.remove( "IconBoardApp" );
#endif
}
