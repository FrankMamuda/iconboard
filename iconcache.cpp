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
#include <Winuser.h>
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
        this->add( alias, icon );
    }

    // retrieve icon from internal cache
    icon = this->cache[alias];
    return icon;
}

/**
 * @brief IconCache::thumbnail
 * @param path
 * @param scale
 * @return
 */
QIcon IconCache::thumbnail( const QString &path, int scale ) {
    QRect rect;
    QIcon icon;
    QPixmap pixmap;

    if ( !pixmap.load( path ))
        return icon;

    if ( pixmap.isNull() && !pixmap.width())
        return icon;

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

        if ( pixmap.width() >= scale * 2.0f )
            pixmap = pixmap.scaled( scale * 2.0f, scale * 2.0f, Qt::IgnoreAspectRatio, Qt::FastTransformation );

        pixmap = pixmap.scaled( scale, scale, Qt::IgnoreAspectRatio, Qt::SmoothTransformation );
    }

    if ( !pixmap.isNull())
        icon = QIcon( pixmap );

    return icon;
}

/**
 * @brief extractPixmap
 * @param fileName
 * @return
 */
#ifdef Q_OS_WIN
QPixmap IconCache::extractPixmap( const QString &fileName ) {
    SHFILEINFO fileInfo;
    QPixmap pixmap;
    QImage image;
    QFileInfo info( fileName );
    int flags = SHGFI_ICON | SHGFI_SYSICONINDEX | SHGFI_LARGEICON;
    int y, k;
    bool ok;

    // initialize COM (needed for SHGetFileInfo in a threaded environment)
    const HRESULT hrCoInit = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if ( FAILED( hrCoInit ))
        return pixmap;

    memset( &fileInfo, 0, sizeof( SHFILEINFO ));

    if ( !info.isDir())
        flags |= SHGFI_USEFILEATTRIBUTES;

    const HRESULT hrFileInfo = SHGetFileInfo( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( fileName ).utf16()), 0, &fileInfo, sizeof( SHFILEINFO ), flags );

    if ( SUCCEEDED( hrFileInfo )) {
        if ( QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA && fileInfo.hIcon ) {
            auto pixmapFromImageList = [ fileInfo ]( int index, QPixmap &pixmap ) {
                IImageList *imageList = nullptr;
                HICON hIcon = 0;

                if ( SUCCEEDED( SHGetImageList( index, { 0x46eb5926, 0x582e, 0x4017, { 0x9f, 0xdf, 0xe8, 0x99, 0x8d, 0xaa, 0x9, 0x50 }}, reinterpret_cast<void **>( &imageList )))) {
                    if ( SUCCEEDED( imageList->GetIcon( fileInfo.iIcon, ILD_TRANSPARENT, &hIcon ))) {
                        pixmap = QtWin::fromHICON( hIcon );
                        DestroyIcon( hIcon );
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
                        if ( image.pixelColor( y, k ).alphaF() > 0.0f )
                            ok = true;
                    }
                }
            }

            // then try to get the large icon
            if ( pixmap.isNull() || !ok ) {
                pixmapFromImageList( 0x2, pixmap );
            }
        }

        // if everything fails, get icon the old way
        if ( pixmap.isNull() && fileInfo.hIcon ) {
            pixmap = QtWin::fromHICON( fileInfo.hIcon );
            DestroyIcon( fileInfo.hIcon );
        }
    }

    // uninitialize COM
    CoUninitialize();

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
    float overlaySize = originalSize / factor;
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
    QFileInfo info( fileName );
    QFileInfo target( info.symLinkTarget());
    QString iconName, absolutePath( info.isSymLink() ? target.absoluteFilePath() : info.absoluteFilePath());
    QMimeDatabase db;
    QIcon icon;

    // get mimetype by matching content
    iconName = info.isDir() ? "inode-directory" : db.mimeTypeForFile( absolutePath, QMimeDatabase::MatchContent ).iconName();

    // generate thumbnail for images
    if ( iconName.startsWith( "image-" ))
        icon = this->thumbnail( absolutePath, iconSize );
#ifdef Q_OS_WIN
    // get icon from executables
    if ( iconName.startsWith( "application-x-ms-dos-executable" ) && !info.isSymLink())
        icon = QIcon( this->extractPixmap( absolutePath ));
    // get icon from win32 shortcuts
    if ( icon.isNull() && info.isSymLink())
        icon = QIcon( this->extractPixmap( fileName ));
#endif
    // get icon for mimetype
    if ( icon.isNull())
        icon = this->icon( iconName, iconSize );
#ifdef Q_OS_WIN
    // if mimetype icon fails (no custom icon theme, for example), get win32 shell icon
    if ( icon.isNull()) {
        QString alias;
        icon = QIcon( this->extractPixmap( absolutePath ));
        alias = QString( "%1_%2_%3" ).arg( iconName ).arg( IconIndex::instance()->defaultTheme()).arg( iconSize );

        // store shell icon in cache, to avoid unnecessary extractions
        if ( !icon.isNull() && !this->cache.contains( alias ))
            this->add( alias, icon );
    }
#endif

    // add symlink label if required
    if ( info.isSymLink())
        icon = this->addSymlinkLabel( icon, iconSize );

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
