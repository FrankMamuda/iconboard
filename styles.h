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
#include "singleton.h"

/**
 * @brief The BuiltInStyles namespace
 */
namespace BuiltInStyles {
const static QStringList Styles = QStringList() << "dark" << "light";
}

/**
 * @brief The StyleEntry class
 */
class StyleEntry {
public:
    StyleEntry( const QString &name = QString::null, const QString &styleSheet = QString::null, bool builtIn = true ) : m_name( name ), m_styleSheet( styleSheet ), m_builtIn( builtIn ) {}
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

/**
 * @brief The StyleManager class
 */
class StyleManager : public QObject {
    Q_OBJECT

public:
    StyleManager();
    static StyleManager *createInstance() { return new StyleManager(); }
    static StyleManager *instance() { return Singleton<StyleManager>::instance( StyleManager::createInstance ); }
    bool contains( const QString &name ) const { return this->list.contains( name ); }
    QMap<QString, StyleEntry> list;

public slots:
    void add( const QString &name, const QString &styleSheet, bool builtIn = true );
    void remove( const QString &name );
    void rename( const QString &oldName, const QString &newName );
};
