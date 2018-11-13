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
#include "iconcache.h"
#include "iconindex.h"
#include "indexcache.h"
#include "variable.h"
#include <QPainter>
#include <QMimeDatabase>
#include <QDebug>
#ifdef Q_OS_WIN
#include <QtWin>
#include <QDir>
#include <commctrl.h>
#include <commoncontrols.h>
#include <shellapi.h>
#include <winuser.h>
#endif
#include "main.h"

/**
 * @brief IconCache::IconCache
 * @param parent
 */
IconCache::IconCache( QObject *parent ) : QObject( parent ) {
    // announce
#ifdef QT_DEBUG
    qInfo() << this->tr( "initializing" );
#endif

    // add to garbage collector
    GarbageMan::instance()->add( this );
}

/**
 * @brief IconCache::icon
 * @param name
 * @param scale
 * @param themeName
 * @return
 */
QIcon IconCache::icon( const QString &iconName, int scale, const QString theme, const QString &fallback ) {
    QIcon icon;
    QString themeName( theme );

    // revert to default if empty
    if ( themeName.isEmpty())
        themeName = IconIndex::instance()->defaultTheme();

    // return empty icon on invalid themes
    if ( !QString::compare( themeName, "system" ))
        return QIcon();

    // make unique icons for different sizes
    const QString alias( QString( "%1_%2_%3" ).arg( iconName ).arg( themeName ).arg( scale ));

    // retrieve icon from index cache if it is not available in internal cache
    if ( !this->cache.contains( alias )) {
        icon = IndexCache::instance()->icon( iconName, scale, themeName );

        // handle missing icons
        if ( icon.isNull()) {
            /* here we read fallback icons from either cache or actual files */
            const QString cachedName( IndexCache::instance()->path() + "/" + alias + ".png" );
            const QPixmap pixmap( cachedName );

            // first check cache, then try the actual file
            if ( pixmap.isNull() && !fallback.isEmpty()) {
                // write out icons with known sizes
                if ( scale > 0 ) {
                    QPixmap fallbackIcon( QIcon( fallback ).pixmap( scale, scale ));
                    fallbackIcon.save( cachedName );
                    icon = QIcon( fallbackIcon );
                } else {
                    icon = QIcon( fallback );
                }
            } else {
                icon = QIcon( pixmap );
            }

            if ( icon.isNull()) {
                icon = IndexCache::instance()->icon( "application-x-zerosize", scale, themeName );
                if ( icon.isNull())
                    return QIcon();
            }
        }

        // add icon to cache
        this->add( alias, icon );
    }

    // retrieve icon from internal cache
    icon = this->cache[alias];
    return icon;
}

/**
 * @brief IconCache::fastDownscale
 * @param pixmap
 * @return
 */
QPixmap IconCache::fastDownscale( const QPixmap &pixmap, int scale ) const {
    QPixmap downScaled( pixmap );

    if ( pixmap.isNull() || scale <= 0 )
        return QPixmap();

    if ( downScaled.width() >= scale * 2.0f )
        downScaled = downScaled.scaled( static_cast<int>( scale * 2.0f ), static_cast<int>( scale * 2.0f ), Qt::IgnoreAspectRatio, Qt::FastTransformation );

    return downScaled.scaled( scale, scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
}

/**
 * @brief IconCache::thumbnail
 * @param path
 * @param scale
 * @return
 */
QIcon IconCache::thumbnail( const QString &fileName, int scale, bool upscale ) {
    QRect rect;
    QIcon icon;
    QPixmap pixmap, cache;

    // thumbnail cache
    QString cachedFile( this->fileNameForHash( hashForFile( fileName ), scale ));
    if ( !cachedFile.isEmpty()) {
        if ( cache.load( cachedFile ))
            return QIcon( cache );
    }

    if ( !pixmap.load( fileName ))
        return icon;

    if ( pixmap.isNull() && !pixmap.width())
        return icon;

    if ( upscale && pixmap.width() < scale )
        pixmap = pixmap.scaledToWidth( scale, Qt::SmoothTransformation );

    if ( pixmap.height() < scale || pixmap.width() < scale ) {
        QPixmap result( scale, scale );
        result.fill( Qt::transparent );
        {
            QPainter painter( &result );
            painter.drawPixmap( scale / 2 - pixmap.width() / 2, scale / 2 - pixmap.height() / 2, pixmap );
        }
        pixmap = result;
    } else if ( pixmap.height() > scale || pixmap.width() > scale ) {
        if ( pixmap.width() > pixmap.height())
            rect = QRect( pixmap.width() / 2 - pixmap.height() / 2, 0, pixmap.height(), pixmap.height());
        else if ( pixmap.width() < pixmap.height())
            rect = QRect( 0, pixmap.height() / 2 - pixmap.width() / 2, pixmap.width(), pixmap.width());

        pixmap = pixmap.copy( rect );
        pixmap = this->fastDownscale( pixmap, scale );
    }

    if ( !pixmap.isNull()) {
        icon = QIcon( pixmap );

        if ( !cachedFile.isEmpty())
            pixmap.save( cachedFile );
    }

    return icon;
}

/**
 * @brief checksum
 * @param data
 * @param len
 * @return
 */
quint32 IconCache::checksum( const char* data, size_t len ) const {
    const quint32 m = 0x5bd1e995, r = 24;
    quint32 h = 0, w;
    const char *l = data + len;

    while ( data + 4 <= l ) {
        w = *( reinterpret_cast<const quint32*>( data ));
        data += 4;
        h += w;
        h *= m;
        h ^= ( h >> 16 );
    }

    switch ( l - data ) {
    case 3:
        h += static_cast<quint32>( data[2] << 16 );

    case 2:
        h += static_cast<quint32>( data[1] << 8 );

    case 1:
        h += static_cast<quint32>( data[0] );
        h *= m;
        h ^= ( h >> r );
        break;
    }
    return h;
}

/**
 * @brief IconCache::hashForFile
 * @param fileName
 * @return
 */
quint32 IconCache::hashForFile( const QString &fileName ) const {
    QFile file( fileName );
    const qint64 maxSize = 10485760;
    quint32 hash = 0;

    if ( file.open( QFile::ReadOnly )) {
        // read the first 10MB and assume files are identical
        if ( file.size() >= maxSize )
            hash = IconCache::instance()->checksum( file.read( maxSize ).constData(), maxSize );
        else
            hash = IconCache::instance()->checksum( file.readAll().constData(), static_cast<size_t>( file.size()));

        file.close();
    }

    return hash;
}

/**
 * @brief IconCache::fileNameForHash
 * @param hash
 * @return
 */
QString IconCache::fileNameForHash( quint32 hash, int scale ) const {
    QString suffix;

    if ( hash == 0 )
        return "";

    if ( scale > 0 )
        suffix = QString( "_%1" ).arg( scale );

    return IndexCache::instance()->path() + "/" + QString::number( hash ) + suffix + ".png";
}

#ifdef Q_OS_WIN

/**
 * @brief IconCache::extractPixmap
 * @param fileName
 * @return
 */
QPixmap IconCache::extractPixmap( const QString &fileName, int scale ) {
    SHFILEINFO fileInfo;
    QPixmap pixmap, cache;
    QImage image;
    QFileInfo info( fileName );
    int flags = SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_LARGEICON;
    int y, k;
    bool ok = false;

    memset( &fileInfo, 0, sizeof( SHFILEINFO ));

    if ( !info.isDir())
        flags |= SHGFI_USEFILEATTRIBUTES;

    // win32 icon cache
    const QMimeDatabase db;
    const QMimeType mime( db.mimeTypeForFile( fileName, QMimeDatabase::MatchContent ));
    const QString cachedFile(
                !QString::compare( mime.iconName(), "application-x-ms-dos-executable" ) || info.isSymLink() ?
                    this->fileNameForHash( hashForFile( fileName ), scale ) :
                    IndexCache::instance()->path() + "/" + mime.iconName() + "_" + QString::number( scale ) + ".png"
                    );

    if ( !cachedFile.isEmpty()) {
        if ( cache.load( cachedFile ))
            return cache;
    }

    const int hrFileInfo = static_cast<const int>( SHGetFileInfo( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( fileName ).utf16()), 0, &fileInfo, sizeof( SHFILEINFO ), static_cast<UINT>( flags )));

    // for some reason this always fails on msvc
#ifndef Q_CC_MSVC
    if ( static_cast<const int>( hrFileInfo ) >= 0 )
#else
    Q_UNUSED( hrFileInfo )
#endif
    {
        if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA && fileInfo.hIcon ) {
            auto pixmapFromImageList = [ fileInfo ]( int index, QPixmap &pixmap ) {
                IImageList *imageList = nullptr;
                HICON hIcon = 0;

                if ( static_cast<int>( SHGetImageList( index, IID_PPV_ARGS( &imageList ))) >= 0 ) {
                    if ( static_cast<int>( imageList->GetIcon( fileInfo.iIcon, ILD_TRANSPARENT, &hIcon )) >=0 ) {
                        pixmap = QtWin::fromHICON( hIcon );
                        DestroyIcon( hIcon );
                        imageList->Release();
                    }
                }
            };

            // first try to get the jumbo icon
            pixmapFromImageList( 0x4, pixmap );

            // test if most of the image is blank
            // (invalid jumbo with 48x48 on top left)
            if ( pixmap.width() >= 64 && pixmap.height() >= 64 ) {
                image = pixmap.toImage();
                for ( y = 64; y < pixmap.width(); y++ ) {
                    for ( k = 64; k < pixmap.height(); k++ ) {
                        if ( image.pixelColor( y, k ).alphaF() > 0.0 )
                            ok = true;
                    }
                }
            }

            // check if icon is really a small, but centered one
            if ( ok ) {
                enum Sides { Top = 0, Bottom, Right, Left };
                auto checkSide = [ image ]( Sides side, int scale ) {
                    if ( image.width() != 256 || image.height() != 256 )
                        return false;

                    if ( scale <= 8 || scale >= 128 )
                        return false;

                    int hOffset = 0, vOffset = 0;
                    int hBound = image.width();
                    int vBound = image.height();

                    switch ( side ) {
                    case Top:
                        vBound = scale;
                        break;

                    case Bottom:
                        vOffset = 256 - scale;
                        break;

                    case Left:
                        hBound = scale;
                        break;

                    case Right:
                        hOffset = 256 - scale;
                        break;
                    }

                    bool blank = true;
                    for ( int x = hOffset; x < hBound; x++ ) {
                        for ( int y = vOffset; y < vBound; y++ ) {
                            if ( image.pixelColor( x, y ).alphaF() > 0.0 )
                                blank = false;
                        }
                    }
                    return blank;
                };

                const int cropScale = 64;
                if ( checkSide( Top, cropScale ) && checkSide( Bottom, cropScale ) && checkSide( Left, cropScale ) && checkSide( Right, cropScale ))
                    ok = false;
            }

            // then try to get the large icon
            if ( pixmap.isNull() || !ok )
                pixmapFromImageList( 0x2, pixmap );
        }

        // if everything fails, get icon the old way
        if ( pixmap.isNull() && fileInfo.hIcon ) {
            pixmap = QtWin::fromHICON( fileInfo.hIcon );
            DestroyIcon( fileInfo.hIcon );
        }
    }

    // save icon in cache folder as plain PNG for faster reads
    pixmap = this->fastDownscale( pixmap, scale );
    if ( !cachedFile.isEmpty())
        pixmap.save( cachedFile );

    return pixmap;
}
#endif

/**
 * @brief IconCache::addSymlinkLabel
 * @param icon
 * @param originalSize
 * @return
 */
QIcon IconCache::addSymlinkLabel( const QIcon &icon, int originalSize ) {
    QPixmap base, overlay;
    QIcon overlayIcon( ":/icons/link" );
    const float factor = 4.0f;
    int overlaySize = static_cast<int>( originalSize / factor );

    // abort if disabled
    if ( Variable::instance()->isDisabled( "ui_displaySymlinkIcon" ))
        return icon;

    // limit shortcut arrow size
    if ( overlaySize > 24 )
        overlaySize = 24;
    else if ( overlaySize < 8 )
        overlaySize = 8;

    // get base pixmap (the icon)
    base = icon.pixmap( originalSize, originalSize );
    const QSize actualSize( icon.actualSize( QSize( originalSize, originalSize )));

    // get overlay arrow (TODO: allow custom arrows)
    overlay = overlayIcon.pixmap( overlaySize, overlaySize );

    // superimpose arrow over base pixmap
    QPixmap result( originalSize, originalSize );
    result.fill( Qt::transparent );
    {
        QPainter painter( &result );
        painter.drawPixmap( originalSize / 2 - actualSize.width() / 2, originalSize / 2 - actualSize.height() / 2, base );
        painter.drawPixmap( 0, originalSize - overlaySize, overlaySize, overlaySize, overlay );
    }

    // return overlay icon
    return QIcon( result );
}


/**
 * @brief ProxyModel::getDriveIconName
 * @param info
 * @return
 */
#ifdef Q_OS_WIN
QString IconCache::getDriveIconName( const QString &path ) const {
    const UINT type = GetDriveType( reinterpret_cast<const wchar_t *>( path.utf16()));

    switch ( type ) {
    case DRIVE_REMOVABLE:
        return "drive-removable-media";

    case DRIVE_REMOTE:
        return "network-workgroup";

    case DRIVE_CDROM:
        return "media-optical";

    case DRIVE_RAMDISK:
        return "media-flash";

    case DRIVE_FIXED:
        return "drive-harddisk";

    case DRIVE_UNKNOWN:
    case DRIVE_NO_ROOT_DIR:
    default:
        return "drive-removable-media";
    }
}
#endif

/**
 * @brief IconCache::iconForFilename
 * @return
 */
QIcon IconCache::iconForFilename( const QString &fileName, int scale, bool upscale ) {
    const QFileInfo info( fileName );
    const QFileInfo target( info.symLinkTarget());
    QString iconName;
    const QString absolutePath( info.isSymLink() ? target.absoluteFilePath() : info.absoluteFilePath());
    const QMimeDatabase db;
    QIcon icon;
    const bool isDir = info.isDir() || target.isDir();

    // get mimetype by matching content
    iconName = isDir ? "inode-directory" : db.mimeTypeForFile( absolutePath, QMimeDatabase::MatchContent ).iconName();

#ifdef Q_OS_WIN
    // initialize COM (needed for SHGetFileInfo in a threaded environment)
    const int hrCoInit = static_cast<const int>( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ));
    if ( hrCoInit < 0 )
        return icon;

    if ( info.isRoot() || target.isRoot())
        iconName = IconCache::instance()->getDriveIconName( absolutePath );
#endif

    // generate thumbnail for images
    if ( !isDir ) {
        if ( iconName.startsWith( "image-" ))
            icon = this->thumbnail( absolutePath, scale, upscale );
#ifdef Q_OS_WIN
        // get icon from executables
        if ( iconName.startsWith( "application-x-ms-dos-executable" ) && !info.isSymLink())
            icon = QIcon( this->extractPixmap( absolutePath, scale ));
        // get icon from win32 shortcuts
        if ( icon.isNull() && info.isSymLink())
            icon = QIcon( this->extractPixmap( fileName, scale ));
        // get icon from appref-ms files
        if ( icon.isNull() && fileName.endsWith( ".appref-ms" ))
            icon = QIcon( this->extractPixmap( fileName, scale ));
#endif
    }

    // get icon for mimetype
    if ( icon.isNull()) {
        if ( isDir )
            icon = this->icon( iconName, ":/icons/folder_scalable", scale );
        else
            icon = this->icon( iconName, scale );
    }
#ifdef Q_OS_WIN
    // if mimetype icon fails (no custom icon theme, for example), get win32 shell icon
    if ( icon.isNull()) {
        QString alias;
        icon = QIcon( this->extractPixmap( absolutePath, scale ));
        alias = QString( "%1_%2_%3" ).arg( iconName ).arg( IconIndex::instance()->defaultTheme()).arg( scale );

        // store shell icon in cache, to avoid unnecessary extractions
        if ( !icon.isNull() && !this->cache.contains( alias ))
            this->add( alias, icon );
    }

    // add symlink label if required
    if ( info.isSymLink() || fileName.endsWith( ".appref-ms" ))
        icon = this->addSymlinkLabel( icon, scale );

    // uninitialize COM
    CoUninitialize();
#endif

    // return the icon
    return icon;
}
