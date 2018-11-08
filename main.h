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
#include <QTimer>
#include <QVariant>

//
// classes
//
class WidgetList;
class TrayIcon;

/**
 * @brief The Main class
 */
class Main final : public QObject {
    Q_OBJECT
    Q_PROPERTY( bool initialized READ hasInitialized WRITE setInitialized )

public:
    ~Main() {}
    static Main *instance() { static Main *instance( new Main()); return instance; }
    bool hasInitialized() const { return this->m_initialized; }
    static void messageFilter( QtMsgType type, const QMessageLogContext &, const QString &msg );
    bool reloadScheduled() const { return this->m_reloadScheduled; }
    static QSize currentResolution();
    static QSize targetResolution();

protected:
    void timerEvent( QTimerEvent * ) { this->writeConfiguration(); }

public slots:
    void readConfiguration();
    void writeConfiguration( bool force = false );
    void reload();
    void scheduleReload();
    void shutdown();

private slots:
    void setInitialized( bool init = true ) { this->m_initialized = init; }
    void iconThemeChanged( QVariant value );

private:
    Main( QObject *parent = nullptr );
    bool m_initialized;
    WidgetList *widgetList;
    TrayIcon *tray;
    bool m_reloadScheduled;
    QTimer timer;
};

/**
 * @brief The GarbageMan class
 */
class GarbageMan final {
public:
    /**
     * @brief instance
     * @return
     */
    static GarbageMan *instance() { static GarbageMan *instance( new GarbageMan()); return instance; }
    GarbageMan( const GarbageMan & ) = delete;
    ~GarbageMan() = default;

    /**
     * @brief add adds pointers (singletons) to garbage collection list
     * @param object
     */
    void add( QObject *object ) {
        if ( !this->garbage.contains( object ))
            this->garbage << object;
    }

    /**
     * @brief clear deletes poiners in reverse order
     */
    void clear() {
        std::reverse( this->garbage.begin(), this->garbage.end());
        foreach ( QObject *object, this->garbage ) {
            if ( object != nullptr ) {
                delete object;
                object = nullptr;
            }
        }
        this->garbage.clear();
    }

private:
    explicit GarbageMan() = default;
    QList<QObject*> garbage;
};
