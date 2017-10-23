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
#include "singleton.h"
#include <QVariant>

//
// classes
//
class WidgetList;
class TrayIcon;

/**
 * @brief The Main class
 */
class Main : public QObject {
    Q_OBJECT
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )

public:
    ~Main() {}
    static Main *instance() { return Singleton<Main>::instance( Main::createInstance ); }
    bool hasInitialized() const { return this->m_initialized; }
    static void messageFilter( QtMsgType type, const QMessageLogContext &, const QString &msg );

protected:
    void timerEvent( QTimerEvent * ) { this->writeConfiguration(); }

public slots:
    void readConfiguration();
    void writeConfiguration();
    void reload();
    void shutdown();

private slots:
    void setInitialized( bool init = true ) { this->m_initialized = init; }
    void iconThemeChanged( QVariant value );

private:
    Main( QObject *parent = nullptr );
    static Main *createInstance() { return new Main(); }
    bool m_initialized;
    WidgetList *widgetList;
    TrayIcon *tray;
};
