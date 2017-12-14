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
#include "folderview.h"

#include <QEvent>
#include <QIcon>
#include <QWidget>
#include <QGesture>

/**
 * @brief The Icon namespace
 */
namespace Icon {
    const static int RowCount = 3;
    const static int ColumnCount = 3;
    const static int IconSize = 48;
    const static int Padding = 8;
};

/**
 * @brief The DesktopIcon class
 */
class DesktopIcon : public QWidget {
    Q_OBJECT
    Q_PROPERTY( QString target READ target WRITE setTarget )
    Q_PROPERTY( QIcon icon READ icon WRITE setIcon )
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( int previewIconSize READ previewIconSize WRITE setPreviewIconSize )
    Q_PROPERTY( int rows READ rows WRITE setRows )
    Q_PROPERTY( int columns READ columns WRITE setColumns )
    Q_PROPERTY( int padding READ padding WRITE setPadding )
    Q_PROPERTY( QString title READ title WRITE setTitle )
    // SHOW LABEL
    // HOVER PREVIEW
    // ETC.

public:
    explicit DesktopIcon( QWidget *parent = nullptr, const QString &target = QString::null );
    ~DesktopIcon() {}
    QString target() const { return this->m_target; }
    QIcon icon() const { return this->m_icon; }
    int iconSize() const { return this->m_iconSize; }
    int previewIconSize() const { return this->m_previewIconSize; }
    int rows() const { return this->m_rows; }
    int columns() const { return this->m_columns; }
    int padding() const { return this->m_padding; }
    QString title() const { return this->m_title; }

    // thumbnail for widget list
    QPixmap thumbnail;

public slots:
    void setTarget( const QString &target ) { this->m_target = target; }
    void setIcon( const QIcon &icon ) { this->m_icon = icon; }
    void setIconSize( int size ) { this->m_iconSize = size; }
    void setPreviewIconSize( int size ) { this->m_previewIconSize = size; }
    void setRows( int rows ) { this->m_rows = rows; }
    void setColumns( int columns ) { this->m_columns = columns; }
    void setPadding( int padding ) { this->m_padding = padding; }
    void setTitle( const QString &title ) { this->m_title = title; }

protected:
    void paintEvent( QPaintEvent *event );
    bool eventFilter( QObject *object, QEvent *event );

private:
    QString m_target;
    QIcon m_icon;
    int m_iconSize;
    int m_previewIconSize;
    int m_rows;
    int m_columns;
    bool m_move;
    int m_padding;
    FolderView *preview;
    QString m_title;
};
