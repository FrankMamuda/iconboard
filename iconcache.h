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
#include <QIcon>
#include "singleton.h"

/**
 * @brief The IconCache class
 */
class IconCache : public QObject {
    Q_OBJECT

public:
    IconCache( QObject *parent = 0 );
    static IconCache *createInstance() { return new IconCache(); }
    static IconCache *instance() { return Singleton<IconCache>::instance( IconCache::createInstance ); }
    QIcon icon( const QString &iconName, int scale = 0, const QString theme = QString::null );
    QIcon thumbnail( const QString &path, int scale, bool &ok );

private:
    QHash<QString, QIcon> cache;
};

#if 0
#pragma once

//
// includes
//
#include <QHash>
#include <QDir>
#include <QIcon>
#include <QWriteLocker>
#include "filestream.h"
#include <QMutexLocker>

/**
 * @brief The Cache namespace
 */
namespace Cache {
    static const quint8 Version = 1;
    static const QString IndexFilename( "icons.index" );
}

/**
 * @brief The Entry struct
 */
struct Entry {
    Entry( const QString &a = QString::null, const QString &f = QString::null ) : alias( a ), fileName( f ) {}
    QString alias;
    QString fileName;
};
Q_DECLARE_METATYPE( Entry )

// read/write operators
inline static QDataStream &operator<<( QDataStream &out, const Entry &e ) { out << e.alias << e.fileName; return out; }
inline static QDataStream &operator>>( QDataStream &in, Entry &e ) { in >> e.alias >> e.fileName; return in; }

/**
 * @brief The Match struct
 */
struct Match {
    Match( const QString &f = QString::null, int s = 0 ) : fileName( f ), scale( s ) {}
    QString fileName;
    int scale;
};
Q_DECLARE_METATYPE( Match )

/**
 * @brief MatchList
 */
typedef QList<Match> MatchList;
Q_DECLARE_METATYPE( MatchList )

/**
 * @brief The IconCache class
 */
class IconCache : public QObject {
    Q_OBJECT
    Q_DISABLE_COPY( IconCache )

public:
    IconCache( const QString &path );
    ~IconCache() { this->shutdown(); }
    QIcon icon( const QString &name, int scale = 0, const QString themeName = QString::null );
    void buildIndex( const QString &themeName );
    int parseSVG( const QString &buffer );
    Match readIconFile( const QString &buffer, bool &ok, int recursionLevel = 2 );
    MatchList getIconMatchList( const QString &name, const QString &themeName );
    QIcon findIcon( const QString &name, int scale = 0, const QString &themeName = QString::null );
    QString defaultTheme() const { QMutexLocker( &this->m_mutex ); return this->m_defaultTheme; }

private slots:
    void setValid( bool valid ) { QMutexLocker( &this->m_mutex ); this->m_valid = valid; }
    void shutdown();

private:
    QHash<QString, QIcon> iconCache;
    QHash<QString, QStringList> index;
    QString m_defaultTheme;
    QString path() const { QMutexLocker( &this->m_mutex ); return this->m_path; }
    bool isValid() const { QMutexLocker( &this->m_mutex ); return this->m_valid; }
    bool write( const QString &iconName, const QString &themeName, int iconScale, const QString &fileName );
    bool contains( const QString &alias ) const { QMutexLocker( &this->m_mutex ); return this->indexCache.contains( alias ); }
    bool read();
    FileStream indexFile;
    QString m_path;
    QHash<QString, Entry> indexCache;
    bool m_valid;
    QDir cacheDir;
    mutable QMutex m_mutex;
    mutable QReadWriteLock m_lock;
    mutable QReadWriteLock m_readLock;
};
#endif
