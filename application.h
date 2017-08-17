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
#include <QApplication>
#include <QSharedMemory>

/**
 * @brief The Application class
 */
class Application : public QApplication {
    Q_OBJECT

public:
#ifdef QT_DEBUG
    Application( int &argc, char **argv ) : QApplication( argc, argv ), memory( new QSharedMemory( "iconboardApp", this )) {}
#else
    Application( int &argc, char **argv ) : QApplication( argc, argv ), memory( new QSharedMemory( "iconboardDebugApp", this )) {}
#endif
    ~Application() { if ( this->memory->isAttached()) this->memory->detach(); }

public slots:
    bool lock() {
        if ( this->memory->attach( QSharedMemory::ReadOnly )) {
            this->memory->detach();
            return false;
        }

        if ( this->memory->create( 1 ))
            return true;

        return false;
    }

private:
    QSharedMemory *memory;
};
