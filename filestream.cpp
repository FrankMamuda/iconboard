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
#include "filestream.h"

/**
 * @brief FileStream::open
 * @return
 */
bool FileStream::open() {
    if ( this->m_file.fileName().isEmpty())
        return false;

    if ( this->m_file.open( QFile::ReadWrite )) {
        this->setDevice( &this->m_file );
        return true;
    }
    return false;
}

/**
 * @brief FileStream::close
 */
void FileStream::close() {
    this->unsetDevice();
    this->m_file.close();
}

/**
 * @brief FileStream::seek
 * @param origin
 * @param position
 * @return
 */
bool FileStream::seek( FileStream::Origin origin, qint64 position ) {
    if ( !this->isOpen() || position > this->m_file.size())
        return false;

    if ( this->m_file.pos() < position ) {
        if ( this->skipRawData( position - this->m_file.pos()) != -1 )
            return true;
        else
            return false;
    }

    switch ( origin ) {
    case Start:
        return this->device()->seek( 0 );

    case End:
        return this->device()->seek( this->m_file.size());

    case Set:
        return this->device()->seek( position );
    }
}
