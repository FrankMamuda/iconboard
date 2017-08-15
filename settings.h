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
#include <QSignalMapper>
#include <QDialog>
#include "variable.h"

/**
 * @brief The Ui namespace
 */
namespace Ui {
class Settings;
}

/**
 * @brief The Settings class
 */
class Settings : public QDialog {
    Q_OBJECT

public:
    explicit Settings( QWidget *parent = 0 );
    ~Settings();
    void bind( const QString &key, QWidget *widget );
    void setValue( const QString &key, bool internal = false );

private slots:
    void internalValueChanged( const QString &key ) { this->setValue( key, true ); }
    void externalValueChanged( const QString &key ) { this->setValue( key, false ); }
    void on_closeButton_clicked();

private:
    Ui::Settings *ui;
    QMap<QString, QWidget*>boundVariables;
    QSignalMapper *signalMapper;
};
