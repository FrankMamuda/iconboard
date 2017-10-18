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
#include <QWidget>
#include <windows.h>

/**
 * @brief The DesktopWidget class
 */
class DesktopWidget : public QWidget {
    Q_OBJECT
    Q_CLASSINFO( "description", "FolderView parent widget that manages proper z-order" )

public:
    DesktopWidget( QWidget *parent = nullptr ) : QWidget( parent ), nativeEventIgnored( false ) {
        SetWindowLong( reinterpret_cast<HWND>( this->winId()), GWL_EXSTYLE, ( GetWindowLong( reinterpret_cast<HWND>( this->winId()), GWL_EXSTYLE) | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW | ~WS_EX_APPWINDOW ));
    }

public slots:
    void lower() {
        if ( !this->nativeEventIgnored ) {
            this->nativeEventIgnored = true;
            SetWindowPos( reinterpret_cast<HWND>( this->winId()), HWND_BOTTOM, 0,0,0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE );
            this->nativeEventIgnored = false;
        }
    }

protected:
    bool nativeEvent( const QByteArray &eventType, void *message, long *result ) {
        MSG *msg;

        msg = static_cast<MSG*>( message );
        if ( msg->message == WM_WINDOWPOSCHANGED )
            this->lower();

        return QWidget::nativeEvent( eventType, message, result );
    }

private:
    bool nativeEventIgnored;
};
