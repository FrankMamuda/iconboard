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
#include <QTimer>

/**
 * @brief The Icon namespace
 */
namespace Icon {
    const static int RowCount = 3;
    const static int ColumnCount = 3;
    const static int IconSize = 48;
    const static qreal Padding = 0.125;
};

//
// classes
//
class FolderView;

/**
 * @brief The DesktopIcon class
 */
class DesktopIcon : public QWidget {
    Q_OBJECT
    Q_PROPERTY( QString target READ target WRITE setTarget )
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( int previewIconSize READ previewIconSize WRITE setPreviewIconSize )
    Q_PROPERTY( int rows READ rows WRITE setRows )
    Q_PROPERTY( int columns READ columns WRITE setColumns )
    Q_PROPERTY( qreal padding READ padding WRITE setPadding )
    Q_PROPERTY( QString title READ title WRITE setTitle )
    Q_PROPERTY( qreal textWidth READ textWidth WRITE setTextWidth )
    Q_PROPERTY( Shapes shape READ shape WRITE setShape )
    Q_PROPERTY( QColor background READ background WRITE setBackground )
    Q_PROPERTY( bool titleVisible READ isTitleVisible WRITE setTitleVisible )
    Q_PROPERTY( bool hoverPreview READ hoverPreview WRITE setHoverPreview )
    Q_PROPERTY( qreal hOffset READ hOffset WRITE setHOffset )
    Q_PROPERTY( qreal vOffset READ vOffset WRITE setVOffset )
    Q_PROPERTY( QString customIcon READ customIcon WRITE setCustomIcon )

public:
    // modes
    enum Shapes {
        NoShape = -1,
        Circle,
        Square,
        Rounded,
        Plain
    };
    Q_ENUMS( Shapes )

    explicit DesktopIcon( QWidget *parent = nullptr, const QString &target = QString(), qreal padding = Icon::Padding, int iconSize = Icon::IconSize, const QString &customIcon = QString() );
    ~DesktopIcon();
    QString target() const { return this->m_target; }
    QIcon icon() const { return this->m_icon; }
    int iconSize() const { return this->m_iconSize; }
    int previewIconSize() const { return this->m_previewIconSize; }
    int rows() const { return this->m_rows; }
    int columns() const { return this->m_columns; }
    qreal padding() const { return this->m_padding; }
    qreal padded() const { return this->iconSize() * this->padding(); }
    QString title() const { return this->m_title; }
    qreal textWidth() const { return this->m_textWidth; }
    Shapes shape() const { return this->m_shape; }
    QColor background() const { return this->m_background; }
    bool isTitleVisible() const { return this->m_titleVisible; }
    bool hoverPreview() const { return this->m_hoverPreview; }
    qreal hOffset() const { return this->m_hOffset; }
    qreal vOffset() const { return this->m_vOffset; }
    QString customIcon() const { return this->m_customIcon; }

    // thumbnail for widget list
    QPixmap thumbnail;

public slots:
    void setTarget( const QString &target ) { this->m_target = target; }
    void setIcon();
    void setIconSize( int size ) { this->m_iconSize = size; this->setIcon(); this->adjustFrame(); }
    void setPreviewIconSize( int size ) { this->m_previewIconSize = size; }
    void setRows( int rows ) { this->m_rows = rows; }
    void setColumns( int columns ) { this->m_columns = columns; }
    void setPadding( qreal padding ) { this->m_padding = padding; this->setIcon(); this->repaint(); }
    void setTitle( const QString &title ) { this->m_title = title; this->repaint(); }
    void setTextWidth( qreal fraction ) { fraction = qMin( fraction, 2.0 );  fraction = qMax( fraction, 0.5 ); this->m_textWidth = fraction; this->adjustFrame(); }
    void setShape( Shapes shape ) { this->m_shape = shape; this->repaint(); }
    void setBackground( const QColor &color ) { this->m_background = color; this->repaint(); }
    void setTitleVisible( bool enable ) { this->m_titleVisible = enable; this->repaint(); }
    void setHoverPreview( bool enable ) { this->m_hoverPreview = enable; }
    void setHOffset( qreal offset ) { this->m_hOffset = offset; this->repaint(); }
    void setVOffset( qreal offset ) { this->m_vOffset = offset; this->repaint(); }
    void setCustomIcon( const QString &icon );

    void setupFrame();
    void adjustFrame();
    void setupPreview();

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
    qreal m_padding;
    FolderView *preview;
    QString m_title;
    QTimer timer;
    qreal m_textWidth;
    Shapes m_shape;
    QColor m_background;
    bool m_titleVisible;
    bool m_hoverPreview;
    qreal m_hOffset;
    qreal m_vOffset;
    QString m_customIcon;
};
