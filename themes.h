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
#include <QMap>
#include <QObject>

/**
 * @brief The BuiltInThemes namespace
 */
namespace BuiltInThemes {
const static QStringList Names = QStringList() << "dark" << "light";
}

/**
 * @brief The Theme class
 */
class Theme {
public:
    explicit Theme( const QString &name = QString(), const QString &styleSheet = QString(), bool builtIn = true ) : m_name( name ), m_styleSheet( styleSheet ), m_builtIn( builtIn ) {}
    QString name() const { return this->m_name; }
    QString styleSheet() const { return this->m_styleSheet; }
    bool builtIn() const { return this->m_builtIn; }
    void setStyleSheet( const QString &value ) { this->m_styleSheet = value; }
    void setName( const QString &value ) { this->m_name = value; }
    void setBuiltIn( bool value ) { this->m_builtIn = value; }

private:
    QString m_name;
    QString m_styleSheet;
    bool m_builtIn;
};
Q_DECLARE_METATYPE( Theme* )

/**
 * @brief The Theme class
 */
class Themes final : public QObject {
    Q_OBJECT

public:
    ~Themes() {}
    static Themes *instance() { static Themes *instance( new Themes()); return instance; }
    bool contains( const QString &name ) const;
    QList<Theme*> list;
    Theme *find( const QString &name ) const;

public slots:
    void add( const QString &name, const QString &styleSheet, bool builtIn = true );
    void remove( const QString &name );
    void shutdown() { qDeleteAll( this->list ); this->list.clear(); }

private:
    Themes( QObject *parent = nullptr );
};
