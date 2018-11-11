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
#include <QDialog>
#include <QVariant>

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
    Q_CLASSINFO( "description", "Settings dialog" )

public:
    explicit Settings( QWidget *parent = nullptr );
    ~Settings();

private slots:
    void on_closeButton_clicked();
    void runOnStartupValueChanged( QVariant value );
    void lockToResolutionValueChanged( QVariant value );
    void setResolutionToolTip();

private:
    Ui::Settings *ui;
};
