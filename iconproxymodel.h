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

/**
 * @brief The IconProxyModel class
 */
class IconProxyModel : public QIdentityProxyModel {
    Q_OBJECT

public:
    explicit IconProxyModel( QObject *parent = nullptr );
    ~IconProxyModel();
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    static QIcon iconForFilename( const QString & fileName );

signals:
    void iconFound( const QString &fileName, const QIcon &icon, const QPersistentModelIndex &index ) const;

private slots:
    void updateModel( const QString &fileName, const QIcon &icon, const QPersistentModelIndex &index ) {
        if ( icon.isNull())
            return;

        emit this->dataChanged( index, index );
        this->iconTable[fileName] = icon;
    }

private:
    QHash<QString, QIcon> iconTable;
};
