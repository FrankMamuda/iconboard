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

/**
 * @brief The Ui namespace
 */
namespace Ui {
class IconSettings;
}

//
// classes
//
class DesktopIcon;

/**
 * @brief The IconSettings class
 */
class IconSettings : public QDialog {
    Q_OBJECT

public:
    explicit IconSettings( QWidget *parent = 0 );
    ~IconSettings();
    void setIcon( DesktopIcon *icon );
    void setBackgroundColour( const QColor &colour );

public slots:
    void accept();

private:
    Ui::IconSettings *ui;
    DesktopIcon *icon;
};
