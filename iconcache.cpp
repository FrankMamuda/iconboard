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
    QString cachedFile( this->fileNameForHash( hashForFile( fileName ), scale ));
    if ( !cachedFile.isEmpty()) {
        if ( cache.load( cachedFile ))
            return cache;
    }

    const HRESULT hrFileInfo = SHGetFileInfo( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( fileName ).utf16()), 0, &fileInfo, sizeof( SHFILEINFO ), flags );

    // for some reason this always fails on msvc
#ifndef Q_CC_MSVC
    if ( SUCCEEDED( hrFileInfo ))
#else
    Q_UNUSED( hrFileInfo )
#endif
    {
        if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA && fileInfo.hIcon ) {
            auto pixmapFromImageList = [ fileInfo ]( int index, QPixmap &pixmap ) {
                IImageList *imageList = nullptr;
                HICON hIcon = 0;

                if ( SUCCEEDED( SHGetImageList( index, IID_PPV_ARGS( &imageList )))) {
                    if ( SUCCEEDED( imageList->GetIcon( fileInfo.iIcon, ILD_TRANSPARENT, &hIcon ))) {
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
    QSize actualSize;

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
 * @brief ProxyModel::getDriveIconName
 * @param info
 * @return
 */
#ifdef Q_OS_WIN
QString IconCache::getDriveIconName( const QString &path ) const {
    UINT type;

    type = GetDriveType( reinterpret_cast<const wchar_t *>( path.utf16()));
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
    QFileInfo info( fileName );
    QFileInfo target( info.symLinkTarget());
    QString iconName, absolutePath( info.isSymLink() ? target.absoluteFilePath() : info.absoluteFilePath());
    QMimeDatabase db;
    QIcon icon;
    bool isDir = info.isDir() || target.isDir();

    // get mimetype by matching content
    iconName = isDir ? "inode-directory" : db.mimeTypeForFile( absolutePath, QMimeDatabase::MatchContent ).iconName();

#ifdef Q_OS_WIN
    // initialize COM (needed for SHGetFileInfo in a threaded environment)
    const HRESULT hrCoInit = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if ( FAILED( hrCoInit ))
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
    if ( icon.isNull())
        icon = this->icon( iconName, scale );
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

/**
 * @brief IconCache::preLoadWindowsIcons since win32 does not have icons by default, we load these from system resources
 */
#ifdef Q_OS_WIN
void IconCache::preLoadWindowsIcons() {
    // icon struct
    typedef struct WindowsIcon_s {
        const char *name;
        int index;
        int scale;
    } WindowsIcon_t;

    // icons in imageres.dll
    WindowsIcon_t imageresDllIcons[] = {
        // tray menu
        { "view-list-icons", 148, 16 }, // "Widget List"
        { "configure", 68, 16 }, // "Settings"
        { "color-picker", 70, 16 }, // "Theme editor"
        { "help-about", 81, 16 }, // "About"
        { "application-exit", 98, 16 }, // "Exit"

        // settings, about, widget list, etc.
        { "dialog-close", 98, 16 }, // close icon

        // style editor
        { "document-save", 28, 16 }, // "Save"
        { "document-save-as", 29, 16 }, // "Save as"
        { "edit-rename", 94, 16 }, // "Rename"
        { "document-revert", 86, 16 }, // "Remove"
        { "edit-delete", 89, 16 }, // "Delete"
        { "view-preview", 168, 16 }, // "Preview" tab
        { "inode-directory", 4, 48 }, // Folder icon

        // widget list
        { "list-remove", 89, 16 }, // "Remove"
        { "visibility", 4, 16 }, // "Show/Hide"

        // folderView context
        { "view-close", 98, 16 }, // "Hide"
        { "document-edit", 70, 16 }, // "Custom stylesheet"
        { "inode-directory", 4, 16 }, // "Change directory"
    };

    // icons in shell32.dll
    WindowsIcon_t shellDllIcons[] = {
        // widget list
        { "list-add", 319, 16 }, // "Add"
    };

    // icon extractor lambda
    auto pixmapFromDll = [=]( const wchar_t *dllName, const WindowsIcon_t list[], int count ) {
        int y;
        HMODULE hMod;

        // open library
        hMod = GetModuleHandle( dllName );
        if ( hMod == NULL)
            hMod = LoadLibrary( dllName );

        if ( hMod == NULL )
            return;

        // go through icon list
        for ( y = 0; y < count; y++ ) {
            QPixmap pixmap;
            QString alias;

            // get icons
            pixmap = QtWin::fromHICON( reinterpret_cast<HICON>( LoadImage( hMod, MAKEINTRESOURCE( list[y].index ), IMAGE_ICON, list[y].scale, list[y].scale, LR_DEFAULTCOLOR | LR_SHARED )));
            if ( !pixmap.isNull()) {
                alias = QString( "%1_%2_%3" ).arg( list[y].name ).arg( IconIndex::instance()->defaultTheme()).arg( list[y].scale );
                this->add( alias, QIcon( pixmap ));
            }
        }

        // close libarary
        FreeLibrary( hMod );
    };

    // load icons from system libraries
    pixmapFromDll( L"imageres.dll", imageresDllIcons, sizeof( imageresDllIcons ) / sizeof( WindowsIcon_t ));
    pixmapFromDll( L"shell32.dll", shellDllIcons, sizeof( shellDllIcons ) / sizeof( WindowsIcon_t ) );
}
#endif
