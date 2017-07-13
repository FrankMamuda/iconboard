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
#include <QAbstractListModel>

//
// clases
//
class TrayWidget;

/**
 * @brief The WidgetModel class
 */
class WidgetModel : public QAbstractListModel {
    Q_OBJECT

public:
    WidgetModel( QObject *parent, TrayWidget *widget ) : QAbstractListModel( parent ), parentWidget( widget ) {}
    int rowCount( const QModelIndex & = QModelIndex()) const override;
    QVariant data( const QModelIndex &index, int role ) const;

private:
    TrayWidget *parentWidget;
};
