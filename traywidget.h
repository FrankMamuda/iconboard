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
#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QDesktopWidget>
#include <windows.h>

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
class TrayWidget;
static const QString lightIconTheme = "breeze";
static const QString darkIconTheme = "breeze-dark";
}

/**
 * @brief The TrayWidget class
 */
class TrayWidget : public QMainWindow {
    Q_OBJECT

public:
    explicit TrayWidget( QWidget *parent = 0 );
    ~TrayWidget();
    QList<FolderView*> widgetList;
    QPixmap wallpaper;
    void readXML();

private slots:
    void trayIconActivated( QSystemTrayIcon::ActivationReason reason );
    void showContextMenu();
    void writeConfiguration();
    void getWindowHandles();
    void on_widgetList_doubleClicked( const QModelIndex &index );
    void showSettingsDialog();
    void on_actionAdd_triggered();
    void on_actionRemove_triggered();
    void on_actionShow_triggered();
    void on_actionMap_triggered();
    void on_buttonClose_clicked();

private:
    Ui::TrayWidget *ui;
    QSystemTrayIcon *tray;
    WidgetModel *model;
    QDesktopWidget *desktop;
    HWND worker;
};
