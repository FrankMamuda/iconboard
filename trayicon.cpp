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
#include "trayicon.h"
#include "iconcache.h"
#include "main.h"
#include "themes.h"
#include "settings.h"
#include "about.h"
#include "variable.h"
#include "themeeditor.h"
#include "widgetlist.h"
#include <QMenu>
#include <QIcon>
#include <QDebug>

/**
 * @brief TrayIcon::TrayIcon
 * @param parent
 */
TrayIcon::TrayIcon( QObject *parent ) : QSystemTrayIcon( parent ) {
    WidgetList *parentWidget;

    // setup context menu
    this->setContextMenu( new QMenu());

    // get parent widget list
    parentWidget = qobject_cast<WidgetList *>( this->parent());
    if ( parentWidget == nullptr ) {
        qCritical() << this->tr( "invalid parent widget" );
        return;
    }

    // show widget list action
    this->actionMap[Widgets] = this->contextMenu()->addAction( IconCache::instance()->icon( "view-list-icons", ":/icons/widgets", 16 ), this->tr( "Widget list" ), parentWidget, SLOT( show()));

    // show settings dialog action
    this->actionMap[Settings] = this->contextMenu()->addAction( IconCache::instance()->icon( "configure", ":/icons/settings", 16 ), this->tr( "Settings" ));
    this->connect( this->actionMap[Settings], SIGNAL( triggered( bool )), parentWidget, SLOT( showSettingsDialog()));

    // show theme editor dialog action
    this->actionMap[Themes] = this->contextMenu()->addAction( IconCache::instance()->icon( "color-picker", ":/icons/themes", 16 ), this->tr( "Theme editor" ));
    this->connect( this->actionMap[Themes], SIGNAL( triggered( bool )), parentWidget, SLOT( showThemeDialog()));

    this->contextMenu()->addSeparator();

    // lock widgets
    this->actionMap[Lock] = this->contextMenu()->addAction( "" );
    this->connect( this->actionMap[Lock], &QAction::triggered, []() {
        Variable::instance()->setValue<bool>( "app_lock", !Variable::instance()->isEnabled( "app_lock" ));
    } );

    this->contextMenu()->addSeparator();

    // show about dialog action
    this->actionMap[About] = this->contextMenu()->addAction( IconCache::instance()->icon( "help-about", ":/icons/info", 16 ), this->tr( "About" ));
    this->connect( this->actionMap[About], SIGNAL( triggered( bool )), parentWidget, SLOT( showAboutDialog()));

    // exit action
    this->connect( this->contextMenu()->addAction( IconCache::instance()->icon( "application-exit", ":/icons/close", 16 ), this->tr( "Exit" )), &QAction::triggered, this, []() { Main::instance()->shutdown(); });

#ifdef QT_DEBUG
    // set another icon to distinguish release from debug build
    this->setIcon( QIcon( ":/icons/launcher_debug_96" ));
#else
    this->setIcon( QIcon( ":/icons/launcher_96" ));
#endif

    // connect and show tray icon
    this->connect( this, SIGNAL( activated( QSystemTrayIcon::ActivationReason )), this, SLOT( iconActivated( QSystemTrayIcon::ActivationReason )));
    this->show();
}

/**
 * @brief TrayIcon::~TrayIcon
 */
TrayIcon::~TrayIcon() {
    // safely dispose of the menu
    qDeleteAll( this->contextMenu()->actions());
    this->contextMenu()->clear();

    // hide tray icon
    this->disconnect( this, SIGNAL( activated( QSystemTrayIcon::ActivationReason )));
    this->hide();
}

/**
 * @brief TrayIcon::iconActivated
 */
void TrayIcon::iconActivated( QSystemTrayIcon::ActivationReason reason ) {
    WidgetList *parentWidget;

    // get parent widget list
    parentWidget = qobject_cast<WidgetList *>( this->parent());
    if ( parentWidget == nullptr ) {
        qCritical() << this->tr( "invalid parent widget" );
        return;
    }

    switch ( reason ) {
    case QSystemTrayIcon::Trigger:
        if ( Variable::instance()->isDisabled( "app_lock" )) {
            if ( parentWidget->isHidden())
                parentWidget->show();
            else
                parentWidget->hide();
            break;
        }
        break;

    case QSystemTrayIcon::Context:
        this->actionMap[Widgets]->setEnabled( Variable::instance()->isDisabled( "app_lock" ));
        this->actionMap[Settings]->setEnabled( Variable::instance()->isDisabled( "app_lock" ));
        this->actionMap[Themes]->setEnabled( Variable::instance()->isDisabled( "app_lock" ));

        if ( Variable::instance()->isEnabled( "app_lock" )) {
            this->actionMap[Lock]->setIcon( IconCache::instance()->icon( "object-unlocked", ":/icons/unlock", 16 ));
            this->actionMap[Lock]->setText( this->tr( "Unlock widgets" ));
        } else {
            this->actionMap[Lock]->setIcon( IconCache::instance()->icon( "object-locked", ":/icons/lock", 16 ));
            this->actionMap[Lock]->setText( this->tr( "Lock widgets" ));
        }

        this->contextMenu()->exec( QCursor::pos());
        break;

    case Unknown:
    case DoubleClick:
    case MiddleClick:
        break;
    }
}
