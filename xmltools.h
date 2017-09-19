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

/**
 * @brief The XMLFiles namespace
 */
namespace XMLFiles {
const static QString Variables( "configuration.xml" );
const static QString Widgets( "widgets.xml" );
}

/**
 * @brief The XMLTools class
 */
class XMLTools : public QObject {
    Q_OBJECT
    Q_ENUMS( Modes )

public:
    XMLTools( QObject *parent = 0 );
    static XMLTools *createInstance() { return new XMLTools(); }
    static XMLTools *instance() { return Singleton<XMLTools>::instance( XMLTools::createInstance ); }
    enum Modes {
        NoMode = -1,
        Variables,
        Widgets
    };
    void writeConfiguration( Modes mode, QObject *object = nullptr );
    void readConfiguration( Modes mode, QObject *object = nullptr );
};
