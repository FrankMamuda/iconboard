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
#include <QObject>
#include <QMap>
#include <QSet>
#include <QDir>
#include "singleton.h"

/**
 * @brief The IconIndexNamespace namespace
 */
namespace IconIndexNamespace {
const static QStringList Extensions( QStringList() << "png" << "svg" );
}

/**
 * @brief The IconIndex class
 */
class IconIndex : public QObject {
    Q_OBJECT

public:
    explicit IconIndex( QObject *parent = 0 );
    static IconIndex *createInstance() { return new IconIndex(); }
    static IconIndex *instance() { return Singleton<IconIndex>::instance( IconIndex::createInstance ); }
    QString path() const { return m_path; }
    QString defaultTheme() const { return m_defaultTheme; }
    bool build( const QString &themeName = QString::null );
    QSet<QString> iconIndex( const QString &iconName, const QString &theme = QString::null ) const;
    void setDefaultTheme( const QString &theme ) { this->m_defaultTheme = theme; }

private slots:
    void setPath( const QString &path ) { this->m_path = path; }

private:
    static IconIndex *m_instance;
    QString m_path;
    QString m_defaultTheme;
    QMap<QString, QSet<QString> > index;
};
