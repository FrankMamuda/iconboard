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

/**
 * @brief The IconCache class
 */
class IconCache final : public QObject {
    Q_OBJECT

public:
    static IconCache *instance() { static IconCache *instance( new IconCache()); return instance; }
    ~IconCache() {}
    QIcon icon( const QString &iconName, int scale = 0, const QString theme = QString::null );
    QIcon thumbnail( const QString &fileName, int scale, bool upscale = false );
    QIcon addSymlinkLabel( const QIcon &icon, int originalSize );
    QIcon iconForFilename( const QString &fileName, int scale, bool upscale = false );
#ifdef Q_OS_WIN
    QPixmap extractPixmap( const QString &fileName, int scale );
    void preLoadWindowsIcons();
    QString getDriveIconName( const QString &path ) const;
#endif
    quint32 checksum( const char *data, size_t len ) const;
    quint32 hashForFile( const QString &fileName ) const;
    QString fileNameForHash( quint32 hash, int scale = 0 ) const;
    QPixmap fastDownscale( const QPixmap &pixmap, int scale ) const;

private slots:
    void add( const QString &fileName, const QIcon &icon ) { this->cache[fileName] = icon; }

public slots:
    void shutdown() { this->cache.clear(); }

private:
    IconCache( QObject *parent = nullptr );
    QHash<QString, QIcon> cache;
};
