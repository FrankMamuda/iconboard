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
#include "desktopwidget.h"
#include "foldermanager.h"

#ifndef QT_DEBUG
/**
 * @brief handleWinEvent
 * @param event
 * @param hwnd
 */
void CALLBACK handleWinEvent( HWINEVENTHOOK, DWORD event, HWND hwnd, LONG, LONG, DWORD, DWORD ) {
    if ( event == EVENT_SYSTEM_FOREGROUND ) {
        int y;
        for ( y = 0; y < FolderManager::instance()->count(); y++ ) {
            FolderView *widget = FolderManager::instance()->at( y );
            if ( reinterpret_cast<HWND>( widget->winId()) == hwnd )
                FolderManager::instance()->desktop->lower();
        }

        for ( y = 0; y < FolderManager::instance()->iconCount(); y++ ) {
            DesktopIcon *icon = FolderManager::instance()->iconAt( y );
            if ( reinterpret_cast<HWND>( icon->winId()) == hwnd )
                FolderManager::instance()->desktop->lower();
        }

        if ( reinterpret_cast<HWND>( FolderManager::instance()->desktop->winId()))
            FolderManager::instance()->desktop->lower();
    }
}
#endif

/**
 * @brief DesktopWidget::DesktopWidget
 * @param parent
 */
DesktopWidget::DesktopWidget(QWidget *parent) : QWidget( parent ), nativeEventIgnored( false ) {
    // set hook
#ifdef QT_DEBUG
    // no need for unnecessary hooks in testing environment
    this->hook = nullptr;
#else
    this->hook = SetWinEventHook( EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, nullptr,
                                  handleWinEvent,
                                  0, 0, 0 );
#endif

    SetWindowLong( reinterpret_cast<HWND>( this->winId()), GWL_EXSTYLE, ( GetWindowLong( reinterpret_cast<HWND>( this->winId()), GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | ~WS_EX_APPWINDOW ));
}
