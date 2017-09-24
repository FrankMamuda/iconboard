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
#include <QIcon>
#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>

//
// classes
//
class FolderView;

/**
 * @brief The ProxyIdentityModel class
 */
class ProxyIdentityModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit ProxyIdentityModel( QObject *parent = nullptr );
    ~ProxyIdentityModel();
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    void clearCache() { this->cache.clear(); }

signals:
    void iconFound( const QString &fileName, const QIcon &icon, const QModelIndex &index ) const;

private slots:
    void updateModel( const QString &fileName, const QIcon &icon, const QModelIndex &index ) {
        if ( icon.isNull())
            return;

        emit this->dataChanged( index, index );
        this->cache[fileName] = icon;
    }

protected:
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

private:
    QHash<QString, QIcon> cache;
    FolderView *view;
};


/**
 * @brief The ProxySortModel class
 */
class ProxySortModel : public QSortFilterProxyModel {
    Q_OBJECT

public:
    explicit ProxySortModel( QObject *parent = nullptr );

protected:
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

private:
    FolderView *view;
};
