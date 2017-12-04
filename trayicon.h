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
#include <QAction>
#include <QSystemTrayIcon>

/**
 * @brief The TrayIcon class
 */
class TrayIcon : public QSystemTrayIcon {
    Q_OBJECT

public:
    enum Actions {
        Widgets = 0,
        Settings,
        About,
        Themes,
        Lock
    };

    explicit TrayIcon( QObject *parent = nullptr );
    ~TrayIcon();

private slots:
    void iconActivated( QSystemTrayIcon::ActivationReason reason );

private:
    QMap<Actions, QAction*> actionMap;
};
