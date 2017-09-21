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
#include <QFileSystemModel>
#include <QMouseEvent>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include "ui_folderview.h"

//
// classes
//
class FolderDelegate;
class TrayWidget;
class IconProxyModel;

/**
 * @brief The Ui namespace
 */
namespace Ui {
class FolderView;
const static int DefaultIconSize = 48;
}

/**
 * @brief The Frame namespace
 */
namespace Frame {
static const int BorderWidth = 4;
static const int MouseGrabAreas = 8;
}

/**
 * @brief The FileSystemModel class
 */
class FileSystemModel : public QFileSystemModel {
    Q_OBJECT

public:
    explicit FileSystemModel( QObject *parent = 0, const QString &path = QDir::currentPath()) : QFileSystemModel( parent ) {
        this->setRootPath( path );
        //..this->setReadOnly( true ); // TODO: add as menu (RO by default)
    }

public slots:
    void softReset() { emit this->dataChanged( this->index( 0, 0 ), this->index( this->rowCount() - 1, this->columnCount() - 1 )); }
    void reset() { this->beginResetModel(); this->resetInternalData(); this->endResetModel(); }
};

/**
 * @brief The FolderView class
 */
class FolderView : public QWidget {
    Q_OBJECT
    Q_PROPERTY( QString title READ title )
    Q_PROPERTY( QString rootPath READ rootPath )
    Q_PROPERTY( QString customTitle READ customTitle WRITE setCustomTitle )
    Q_PROPERTY( QString customStyleSheet READ customStyleSheet WRITE setCustomStyleSheet )
    Q_PROPERTY( Qt::SortOrder sortOrder READ sortOrder WRITE setSortOrder )

public:
#ifdef Q_OS_WIN
    explicit FolderView( QWidget *parent, const QString &rootPath, HWND windowParent, TrayWidget *trayParent );
#else
    explicit FolderView( QWidget *parent, const QString &rootPath, TrayWidget *trayParent );
#endif
    ~FolderView();
    QString title() const { if ( !this->customTitle().isNull()) return this->customTitle(); return this->ui->title->text(); }
    QString rootPath() const { return this->model->rootPath(); }
    QString currentStyleSheet() const;
    QString customTitle() const { return this->m_customTitle; }
    QString customStyleSheet() const { return this->m_customStyleSheet; }
    QListView::ViewMode viewMode() const { return this->ui->view->viewMode(); }
    int iconSize() const;
#ifdef Q_OS_WIN
    static void openShellContextMenuForObject( const std::wstring &path, QPoint pos, HWND parentWindow );
#endif
    bool isReadOnly() const;
    Qt::SortOrder sortOrder() const { return this->m_sortOrder; }

    // grab areas for frameless resizing
    enum Areas {
        NoArea = -1,
        TopLeft,
        Top,
        TopRight,
        Right,
        BottomRight,
        Bottom,
        BottomLeft,
        Left
    };
    Q_ENUMS( Areas )

    // frameless gestures
    enum Gestures {
        NoGesture = -1,
        Drag,
        Resize
    };
    Q_ENUMS( Gestures )

public slots:
    void displayContextMenu( const QPoint &point );
    void setCustomTitle( const QString &title );
    void setCustomStyleSheet( const QString &stylesheet, bool force = false );
    void setDefaultStyleSheet();
    void setViewMode( QListView::ViewMode viewMode ) { this->ui->view->setViewMode( viewMode ); }
    void setIconSize( int size ) { this->ui->view->setIconSize( QSize( size, size )); }
    void setIconSize();
    void setReadOnly( bool enable = true );
    void setSortOrder( Qt::SortOrder order );

protected:
    void paintEvent( QPaintEvent *event );
    bool eventFilter( QObject *object, QEvent *event );

private slots:
    void changeDirectory();
    void renameView();
#ifdef Q_OS_WIN
    void setupFrame( HWND windowParent );
#else
    void setupFrame();
#endif
    void makeGrabAreas();
    void on_view_clicked( const QModelIndex &index );
    void editStylesheet();
    void toggleViewMode();
    void toggleAccessMode();
    void on_view_customContextMenuRequested( const QPoint &pos );
    //void makeContextMenu();
    void toggleSortOrder();

private:
    Ui::FolderView *ui;
    IconProxyModel *proxyModel;
    FileSystemModel *model;
    FolderDelegate *delegate;
    QPoint mousePos;
    Gestures gesture;
    Areas currentGrabArea;
    QRect grabAreas[Frame::MouseGrabAreas];
    TrayWidget *trayWidget;
    QString m_customTitle;
    QString m_customStyleSheet;
    QString defaultStyleSheet;
    Qt::SortOrder m_sortOrder;
    //QMenu menu;
};
