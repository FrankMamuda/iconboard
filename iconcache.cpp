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
#ifdef Q_OS_WIN
#include <QtWin>
#include <QDir>
#include <commctrl.h>
#include <commoncontrols.h>
#include <shellapi.h>
#include <Winuser.h>
#include <QPainter>
#endif

/**
 * @brief IconCache::IconCache
 * @param parent
 */
IconCache::IconCache( QObject *parent ) : QObject( parent ) {
}

/**
 * @brief IconCache::icon
 * @param name
 * @param scale
 * @param themeName
 * @return
 */
QIcon IconCache::icon( const QString &iconName, int scale, const QString theme ) {
    QIcon icon;
    QString alias;
    QString themeName( theme );

    // revert to default if empty
    if ( themeName.isEmpty())
        themeName = IconIndex::instance()->defaultTheme();

    // return empty icon on invalid themes
    if ( !QString::compare( themeName, "system" ))
        return QIcon();

    // make unique icons for different sizes
    alias = QString( "%1_%2_%3" ).arg( iconName ).arg( themeName ).arg( scale );

    // retrieve icon from index cache if it is not available in internal cache
    if ( !this->cache.contains( alias )) {
        icon = IndexCache::instance()->icon( iconName, scale, themeName );

        // handle missing icons
        if ( icon.isNull()) {
            icon = IndexCache::instance()->icon( "application-x-zerosize", scale, themeName );
            if ( icon.isNull())
                return QIcon();
        }

        // add icon to cache
        this->cache[alias] = icon;
    }

    // retrieve icon from internal cache
    icon = this->cache[alias];
    return icon;
}

/**
 * @brief IconCache::thumbnail
 * @param path
 * @param scale
 * @param ok
 * @return
 */
QIcon IconCache::thumbnail( const QString &path, int scale, bool &ok ) {
    QRect rect;
    QPixmap pixmap;

    ok = false;

    if ( !pixmap.load( path ))
        return pixmap;

    if ( pixmap.isNull() && !pixmap.width())
        return pixmap;

    if ( pixmap.height() > scale || pixmap.width() > scale ) {
        if ( pixmap.width() > pixmap.height())
            rect = QRect( pixmap.width() / 2 - pixmap.height() / 2, 0, pixmap.height(), pixmap.height());
        else if ( pixmap.width() < pixmap.height())
            rect = QRect( 0, pixmap.height() / 2 - pixmap.width() / 2, pixmap.width(), pixmap.width());

        pixmap = pixmap.copy( rect );

        if ( pixmap.width() >= scale * 2.0f )
            pixmap = pixmap.scaled( scale * 2.0f, scale * 2.0f, Qt::IgnoreAspectRatio, Qt::FastTransformation );

        pixmap = pixmap.scaled( scale, scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }

    ok = true;
    return QIcon( pixmap );
}

/**
 * @brief IconCache::extractIcon
 * @param path
 * @return
 */
QIcon IconCache::extractIcon( const QString &path, bool &ok, bool jumbo ) {
#ifdef Q_OS_WIN
    SHFILEINFO shellInfo;
    QPixmap pixmap;
    QImage image;
    int index;
    int y, k;

    ok = false;

    // only accept executables
    if ( !path.endsWith( ".exe" ))
        return QIcon();

    memset( &shellInfo, 0, sizeof( SHFILEINFO ));

#ifdef Q_CC_MSVC
    // skip this check on msvc - it always fails
    SHGetFileInfo( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( path ).utf16()), 0, &shellInfo, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION | SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON );
#else
    if ( SUCCEEDED( SHGetFileInfo( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( path ).utf16()), 0, &shellInfo, sizeof( SHFILEINFO ), SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_ICONLOCATION | SHGFI_USEFILEATTRIBUTES | SHGFI_LARGEICON ))) {
#endif
        if ( shellInfo.hIcon ) {
            if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA ) {
                IImageList *imageList = nullptr;
                index = 0x2;

                if ( jumbo )
                    index = 0x4;

                if ( SUCCEEDED( SHGetImageList( index, { 0x46eb5926, 0x582e, 0x4017, { 0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50 }}, reinterpret_cast<void **>( &imageList )))) {
                    HICON hIcon;

                    if ( SUCCEEDED( imageList->GetIcon( shellInfo.iIcon, ILD_TRANSPARENT, &hIcon ))) {
                        pixmap = QtWin::fromHICON( hIcon );
                        DestroyIcon( hIcon );

                        if ( pixmap.isNull())
                            return QIcon();

                        if ( !pixmap.isNull() && pixmap.width()) {
                            if ( !jumbo )
                                ok = true;

                            return QIcon( pixmap );
                        }
                    }
                }
            }
            pixmap = QtWin::fromHICON( shellInfo.hIcon );
            DestroyIcon( shellInfo.hIcon );

            if ( !pixmap.isNull() && pixmap.width()) {
                ok = true;
                return QIcon( pixmap );
            }
        }
#ifndef Q_CC_MSVC
    }
#endif
#else
    Q_UNUSED( path )
    Q_UNUSED( ok )
    Q_UNUSED( jumbo )
#endif
    return QIcon();
}

/**
 * @brief IconCache::addSymlinkLabel
 * @param icon
 * @param originalSize
 * @return
 */
QIcon IconCache::addSymlinkLabel( const QIcon &icon, int originalSize, const QString theme ) {
    QPixmap base, overlay;
    QIcon overlayIcon( ":/icons/link" );
    const float factor = 4.0f;
    float overlaySize = originalSize / factor;
    Q_UNUSED( theme )
    QSize actualSize;

    // abort if disabled
    if ( Variable::instance()->isDisabled( "ui_displaySymlinkIcon" ))
        return icon;

    // limit shortcut arrow size
    if ( overlaySize > 24.0f )
        overlaySize = 24.0f;
    else if ( overlaySize < 8.0f )
        overlaySize = 8.0f;

    // get base pixmap (the icon)
    base = icon.pixmap( originalSize, originalSize );
    actualSize = icon.actualSize( QSize( originalSize, originalSize ));

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
 * @brief IconCache::iconForFilename
 * @return
 */
QIcon IconCache::iconForFilename( const QString &fileName, int iconSize ) {
    QMimeDatabase db;
    QString iconName;
    QIcon icon;
    QFileInfo info( fileName );
    bool ok;
    QString filePath( fileName );

    if ( info.isDir())
        return IconCache::instance()->icon( "inode-directory", iconSize );

    if ( info.isSymLink()) {
        filePath = info.symLinkTarget();

        QFileInfo target( info.symLinkTarget());
        if ( target.isDir())
            return IconCache::instance()->icon( "inode-directory", iconSize );
    }

    // get mimetype by matching content
    iconName = db.mimeTypeForFile( filePath, QMimeDatabase::MatchContent ).iconName();

    // extract icons only from executables
    if ( iconName.contains( "x-ms-dos-executable" )) {
        // extract jumbo first
        icon = IconCache::instance()->extractIcon( filePath, ok, true );
        if ( !ok )
            icon = IconCache::instance()->extractIcon( filePath, ok );
    } else if ( iconName.startsWith( "image-" )) {
        bool ok;
        icon = IconCache::instance()->thumbnail( filePath, iconSize, ok );
    }

    if ( icon.isNull())
        icon = IconCache::instance()->icon( iconName, iconSize );

    if ( info.isSymLink())
        icon = IconCache::instance()->addSymlinkLabel( icon, iconSize );

    return icon;
}
