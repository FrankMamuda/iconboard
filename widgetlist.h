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
#include "about.h"
#include "settings.h"
#include "themeeditor.h"
#include <QMainWindow>
#include <QMenu>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

//
// classes
//
class FolderView;
class WidgetModel;
class IconCache;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class WidgetList;
}

/**
 * @brief The WidgetList class
 */
class WidgetList : public QMainWindow {
    Q_OBJECT
    Q_CLASSINFO( "description", "System tray activated folder widget manager" )

public:
    explicit WidgetList( QWidget *parent = 0 );
    ~WidgetList();

public slots:
    void reset();
    void showSettingsDialog() { this->settingsDialog->exec(); }
    void showThemeDialog() { this->themeDialog->exec(); }
    void showAboutDialog() { this->aboutDialog->exec(); }

private slots:
    void on_widgetList_doubleClicked( const QModelIndex &index );
    void on_actionAdd_triggered();
    void on_actionRemove_triggered();
    void on_actionShow_triggered();
    void on_actionMap_triggered();
    void on_buttonClose_clicked();
    void iconThemeChanged( QVariant value );

private:
    Ui::WidgetList *ui;
    WidgetModel *model;
    QMenu *menu;
    Settings *settingsDialog;
    ThemeEditor *themeDialog;
    About *aboutDialog;
#ifdef Q_OS_WIN
    HWINEVENTHOOK hook;
#endif
};
