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
#include <QDebug>
#include <QFile>
#include <QRegularExpression>
#include <QElapsedTimer>
#include "iconindex.h"

/**
 * @brief IconIndex::IconIndex
 * @param parent
 */
IconIndex::IconIndex( QObject *parent ) : QObject( parent ) {
#ifdef Q_OS_WIN
    this->setPath( QDir::currentPath() + "/" + "icons" );
#else
    // NOTE: not sure about macOS (can we use a custom dir from Settings dialog)
    this->setPath( "/usr/share/icons" );
#endif
}

/**
 * @brief IconIndex::build builds custom icon directory index
 * performace-wise it takes only a few msec, so this should not be a bottleneck
 * @param theme
 */
bool IconIndex::build( const QString &theme ) {
    QString buffer, themePath;
    QFile indexFile;
    QRegularExpression re;
    QRegularExpressionMatch match;
    QSet<QString> directories;
    QElapsedTimer timer;

    // performance counters
    timer.start();

    // reject empty theme names
    if ( theme.isEmpty()) {
        qDebug() << this->tr( "IconIndex::build: empty theme name provided" );
        return false;
    }

    // reject system theme
    if ( !QString::compare( theme, "system" )) {
        qDebug() << this->tr( "IconIndex::build: system theme is not to be built" );
        return false;
    }

    // set default theme if none is set
    if ( this->defaultTheme().isEmpty())
        this->setDefaultTheme( theme );

    // create theme path
    themePath = QString( "%1/%2/" ).arg( this->path()).arg( theme );

    // open theme index file
    indexFile.setFileName( themePath + "index.theme" );
    if ( !indexFile.open( QFile::ReadOnly )) {
        qDebug() << this->tr( "IconIndex::build: index file \"%1\" not found" ).arg( indexFile.fileName());
        return false;
    }

    // read & close index file
    buffer = QString( indexFile.readAll().constData());
    indexFile.close();

    // set pattern & extract icon directories
    re.setPattern( "Directories=(.+)\\s" );
    match = re.match( buffer );
    if ( match.hasMatch())
        directories = match.captured( 1 ).remove( "\r" ).split( "," ).toSet();

    // abort on no directories
    if ( !directories.count())
        return false;

    // store directories in index
    this->index[theme] = directories;

    // performance counters
#ifdef QT_DEBUG
    qDebug() << this->tr( "IconIndex::build: \"%1\" index built in %2 msec" ).arg( theme ).arg( timer.elapsed());
#endif

    return true;
}

/**
 * @brief IconIndex::iconIndex this could be called from multiple threads, although since it does
 * not modify internal structure, I can assume this is thread safe?
 * @param iconName
 * @return
 */
QSet<QString> IconIndex::iconIndex( const QString &iconName, const QString &theme ) const {
    QSet<QString> paths;

    // generate icon paths
    foreach ( const QString &directory, this->index[theme] ) {
        foreach ( const QString &extension, IconIndexNamespace::Extensions )
            paths << QString( "%1/%2/%3/%4.%5" ).arg( this->path()).arg( theme ).arg( directory ).arg( iconName ).arg( extension );
    }
    return paths;
}
