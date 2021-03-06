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
#include <QMap>
#include <QObject>

/**
 * @brief The XMLFiles namespace
 */
namespace XMLFiles {
const static QString Variables( "configuration.xml" );
const static QString Widgets( "widgets.xml" );
const static QString Themes( "themes.xml" );
}

/**
 * @brief The XMLTools class
 */
class XMLTools final : public QObject {
    Q_OBJECT
    Q_ENUMS( Modes )

public:
    ~XMLTools() {}
    static XMLTools *instance() { static XMLTools *instance( new XMLTools()); return instance; }
    enum Modes {
        NoMode = -1,
        Variables,
        Widgets,
        Themes
    };
    void write( Modes mode, bool force = false );
    void read( Modes mode );

public slots:
    void saveOnLock( const QVariant &value );

private:
    XMLTools( QObject *parent = nullptr );
};
