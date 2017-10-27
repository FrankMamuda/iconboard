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
#include <QFileInfo>
#include <QFutureWatcher>
#include <QIcon>
#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>
#include <QThreadPool>

//
// classes
//
class FolderView;

//
// defines
//
#define ALT_PROXY_MODE

/**
 * @brief The ProxyIcon struct
 */
struct ProxyIcon {
    explicit ProxyIcon( const QString &f = QString::null, const QIcon &i = QIcon(), const QModelIndex &n = QModelIndex()) : fileName( f ), icon( i ), index( n ) {}
    QString fileName;
    QIcon icon;
    QModelIndex index;
};
Q_DECLARE_METATYPE( ProxyIcon )

/**
 * @brief The ProxyModel class
 */
class ProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
    Q_DISABLE_COPY( ProxyModel )

public:
    explicit ProxyModel( QObject *parent = nullptr );
    ~ProxyModel();
    QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;
    bool isStopping() const { QMutexLocker( &this->m_mutex ); return m_stopping; }

    Qt::ItemFlags flags( const QModelIndex &index ) const {
        if ( !index.isValid())
            return Qt::NoItemFlags;

        return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled  | Qt::ItemIsDragEnabled );
    }

    Qt::DropActions supportedDropActions() const {
        return Qt::CopyAction | Qt::MoveAction;
    }

public slots:
    void clearCache() { this->cache.clear(); }
    void waitForThreads();
    void stop() { this->m_stopping = true; }
    void reset() { this->m_stopping = false; }

signals:
#ifdef ALT_PROXY_MODE
    void iconFound( const QString &fileName, const QIcon &icon ) const;
#else
    void iconFound( const QString &fileName, const QIcon &icon, const QPersistentModelIndex &index ) const;
#endif

private slots:
#ifdef ALT_PROXY_MODE
    void updateModel(const QString &fileName, const QIcon &icon );
#else
    void updateModel( const QString &fileName, const QIcon &icon, const QPersistentModelIndex &index );
#endif

protected:
    bool lessThan( const QModelIndex &left, const QModelIndex &right ) const;

private:
    QHash<QString, QIcon> cache;
    mutable QStringList queue;
    FolderView *view;
    mutable QMutex m_mutex;
    bool m_stopping;
    QThreadPool *threadPool;
};
