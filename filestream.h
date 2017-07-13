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
#include <QFile>
#include <QDataStream>

/**
 * @brief The FileStream class
 */
class FileStream : public QDataStream {
public:
    enum Origin {
        Set = 0,
        Start,
        End
    };
    FileStream( const QString &filename ) { this->m_file.setFileName( filename ); }
    FileStream() {}
    void setFilename( const QString &filename ) { this->m_file.setFileName( filename ); }
    bool open();
    bool isOpen() { return this->m_file.isWritable(); }
    void close();
    bool seek( Origin origin, qint64 position = 0 );
    bool toStart() { return this->seek( Start ); }
    bool toEnd() { return this->seek( End ); }
    bool setPos( qint64 position ) { return this->seek( Set, position ); }
    qint64 size() const { return this->m_file.size(); }
    void resize( qint64 size ) { this->m_file.resize( size ); }
    void clear() { this->resize( 0 ); }
    void sync() { this->m_file.flush(); }

private:
    QFile m_file;
};
