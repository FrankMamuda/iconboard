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
#include <QDir>
#include <QDomDocument>
#include <QPixmap>
#include <QIcon>
#include "indexcache.h"
#include "iconindex.h"
#include "main.h"

/**
 * @brief IndexCache::IndexCache
 * @param parent
 */
IndexCache::IndexCache( QObject *parent ) : QObject( parent ), m_valid( false ), m_badEntries( 0 ) {
    QDir directory;

    // set default path
#ifdef QT_DEBUG
    this->setPath( QDir::home().absolutePath() + "/.iconBoardDebug/cache" );
#else
    this->setPath( QDir::home().absolutePath() + "/.iconBoard/cache" );
#endif
    directory.setPath( this->path());

    // check if cache dir exists
    if ( !directory.exists()) {
        qInfo() << this->tr( "creating non-existant cache dir" );
        directory.mkpath( this->path());

        // additional failsafe
        if ( !directory.exists()) {
            qFatal( this->tr( "unable to create icon cache dir" ).toUtf8().constData());
            return;
        }
    }

    // set up index file
    this->indexFile.setFilename( this->path() + "/" + IndexCacheNamespace::IndexFilename );
    if ( !this->indexFile.open()) {
        qFatal( this->tr( "index file non-writable" ).toUtf8().constData());
        return;
    } else {
        if ( !this->indexFile.size())
            this->indexFile << IndexCacheNamespace::Version;
    }

    // read data
    if ( !this->read()) {
        qFatal( this->tr( "failed to read cache" ).toUtf8().constData());
        return;
    }

    this->setValid( true );
}

/**
 * @brief IndexCache::read
 * @return
 */
bool IndexCache::read() {
    quint8 version;
    Entry entry;

    // read index file version
    this->indexFile.toStart();
    this->indexFile >> version;

    // check version
    if ( version != IndexCacheNamespace::Version ) {
        qCritical() << this->tr( "version mismatch for index file" );
        return false;
    }

    // read indexes
    while ( !this->indexFile.atEnd()) {
        this->indexFile >> entry;

        QFileInfo info( entry.fileName );
        if ( info.exists())
            this->index[entry.alias] = entry;
        else
            this->m_badEntries++;
    }

    // report
    qInfo() << this->tr( "found %1 entries in index file" ).arg( this->index.count());

    // return success
    return true;
}

/**
 * @brief IndexCache::write
 * @param iconName
 * @param themeName
 * @param iconScale
 * @param fileName
 * @return
 */
bool IndexCache::write( const QString &iconName, int iconScale, const QString &theme, const QString &fileName ) {
    QString alias;

    // failsafe
    if ( !this->isValid())
        return false;

    // ignore placeholder icons
    if ( !QString::compare( "application-x-zerosize", iconName ) || fileName.isEmpty())
        return false;

    // check hash
    if ( iconName.isEmpty() || iconScale < 0 ) {
        qWarning() << this->tr( "invalid iconName or scale" );
        return false;
    }

    // generate alias
    alias = QString( "%1_%2_%3" ).arg( iconName ).arg( theme ).arg( iconScale );

    // check for duplicates
    if ( this->contains( alias ))
        return true;

    // create new entry
    Entry entry( alias, fileName );
    this->indexFile.seek( FileStream::End );
    this->indexFile << entry;

    // add new enty to list
    this->index[entry.alias] = entry;

    // flush to disk immediately
    this->indexFile.sync();

    // return success
    return true;
}

/**
 * @brief IndexCache::shutdown
 */
void IndexCache::shutdown() {
    // here we assume our icon cache is corrupt
    if ( this->badEntries() >= 10 ) {
        qInfo() << this->tr( "clearing corrupt index cache" );
        this->indexFile.resize( 0 );
        this->indexFile << IndexCacheNamespace::Version;
        this->m_badEntries = 0;
    }

    // set subsystem as inactive and close the index file
    this->setValid( false );
    this->indexFile.close();
}

/**
 * @brief IndexCache::matchList
 * @param name
 * @return
 */
MatchList IndexCache::matchList( const QString &iconName, const QString &theme ) const {
    QSet<QString> iconIndex;
    int recursionLevel = 2;
    bool ok;
    Match iconMatch;
    MatchList matchList;

    // get icon index
    iconIndex = IconIndex::instance()->iconIndex( iconName, theme );

    // don't bother if no matches
    if ( iconIndex.isEmpty())
        return matchList;

    // go through all iconIndex
    foreach ( const QString &path, iconIndex ) {
        iconMatch = this->readIconFile( path, ok, recursionLevel );
        if ( ok )
            matchList << iconMatch;
    }

    return matchList;
}

/**
 * @brief IndexCache::readIconFile
 * @param fileName
 * @return
 */
Match IndexCache::readIconFile( const QString &fileName, bool &ok, int recursionLevel ) const {
    QString buffer;
    QFile file;
    bool binary = false;
    Match iconMatch( fileName );
    int y;

    // bad icon by default
    ok = false;

    // avoid recursions
    recursionLevel--;
    if ( recursionLevel < 0 )
        return iconMatch;

    // attempt to read icon file
    file.setFileName( fileName );
    if ( !file.open( QFile::ReadOnly ))
        return iconMatch;

    // convert to text in case it is an svg or a symbolic link
    buffer = QString( file.readAll().constData());
    file.close();

    // test if file is binary
    for ( y = 0; y < qMin( 128, buffer.length()); y++ ) {
        if ( buffer.at( y ).unicode() > 127 )  {
            binary = true;
            break;
        }
    }

    // handle non-binary files (symlinks or svg)
    if ( !binary ) {
        // test if svg
        if ( buffer.startsWith( "<svg" ) || buffer.startsWith( "<?xml" )) {
            iconMatch.scale = this->parseSVG( buffer );
        } else {
            QFileInfo info( file );
            QDir dir;
            QString link;
            int pos = 0, numCdUps = 0;

            // construct filename from symlink
            dir.setPath( info.absolutePath());
            link = buffer;
            while (( pos = buffer.indexOf( "../", pos )) != -1 ) {
                link = buffer.mid( pos + 3, buffer.length() - pos - 3 );
                dir.cdUp();
                pos++;
                numCdUps++;
            }
            link = dir.absolutePath() + "/" + link;

            // recursively read symlink target
            iconMatch.fileName = link;
            return this->readIconFile( link, ok, recursionLevel );
        }
    } else {
        QPixmap pixmap;

        // get size directly from pixmap
        if ( pixmap.load( fileName ))
            iconMatch.scale = pixmap.width();
    }

    // clean up
    buffer.clear();

    // check scale
    if ( iconMatch.scale > 0 )
        ok = true;

    // return icon match
    return iconMatch;
}

/**
 * @brief IndexCache::parseSVG
 * @param buffer
 * @return
 */
int IndexCache::parseSVG( const QString &buffer ) const {
    QDomDocument doc;
    QDomNodeList svgNodes;
    int width = 0, height = 0, scale = 0;

    // parse svg as an XML document
    doc.setContent( buffer );

    // get root node
    svgNodes = doc.elementsByTagName( "svg" );
    if ( svgNodes.size()) {
        QDomElement element;

        // convert node to element
        element = svgNodes.at( 0 ).toElement();

        // get width directly
        if ( element.hasAttribute( "width" ))
            width = element.attribute( "width" ).toInt();

        // get height directly
        if ( element.hasAttribute( "height" ))
            height = element.attribute( "height" ).toInt();

        // extract width and height from viewBox (override)
        if ( element.hasAttribute( "viewBox" )) {
            QStringList parms;

            parms = element.attribute( "viewBox" ).split( " " );
            if ( parms.count() == 4 ) {
                width = parms.at( 2 ).toInt();
                height = parms.at( 3 ).toInt();
            }
        }
    }

    // done parsing
    doc.clear();

    // store size
    if ( width && height )
        scale = width;

    return scale;
}

/**
 * @brief IndexCache::bestMatch
 * @param name
 * @param scale
 * @param themeName
 * @return
 */
Match IndexCache::bestMatch( const QString &iconName, int scale, const QString &theme ) const {
    int y = 0, bestIndex = 0, bestScale = 0;
    MatchList matchList;

    // get icon match list
    matchList = this->matchList( iconName, theme );
    if ( matchList.isEmpty())
        return Match();

    // go through all matches
    if ( scale == 0 ) {
        foreach ( const Match &match, matchList ) {
            if ( match.scale > bestScale ) {
                bestScale = match.scale;
                bestIndex = y;
            }

            y++;
        }
    } else {
        foreach ( const Match &match, matchList ) {
            if ( match.scale == scale ) {
                bestIndex = y;
                bestScale = match.scale;
                break;
            } else if ( match.scale > bestScale ) {
                bestScale = match.scale;
                bestIndex = y;
            }

            y++;
        }
    }

    return matchList.at( bestIndex );
}

/**
 * @brief IconCache::findIcon some icon themes from have folders full of symlinks to avoid duplicate icons
 * unfortunately QIcon does not handle these symlinks (plain text files). Moreover, since we cannot reliably
 * get a filename from QIcon::fromTheme, we must manually find the best matching icon here while resolving all
 * symlinks
 * @param name
 * @return
 */
QIcon IndexCache::icon( const QString &iconName, int scale, const QString &theme ) {
    Match match;
    QString alias;

    // check if icon is already cache
    alias = QString( "%1_%2_%3" ).arg( iconName ).arg( theme ).arg( scale );
    if ( this->contains( alias ))
        return QIcon( this->indexEntry( alias ).fileName );

    // get best match
    match = this->bestMatch( iconName, scale, theme );

    // write out to cache
    if ( match.scale >= 0 ) {
        // NOTE: write operation should not be called from multiple threads
        this->write( iconName, scale, theme, match.fileName );
        return QIcon( match.fileName );
    }

    return QIcon();
}
