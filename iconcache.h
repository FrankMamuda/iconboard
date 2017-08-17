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
    QIcon extractIcon( const QString &path, bool &ok, bool jumbo = false );
    QIcon addSymlinkLabel( const QIcon &icon, int originalSize, const QString theme = QString::null );

private:
    QHash<QString, QIcon> cache;
};
