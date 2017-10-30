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

//
// includes
//
#include "themes.h"
#include <QFile>
#include <QDebug>

/**
 * @brief Themes::Themes
 */
Themes::Themes( QObject *parent ) : QObject( parent ) {
    // announce
#ifdef QT_DEBUG
    qInfo() << this->tr( "initializing" );
#endif

    // add blank styleSheet
    this->add( "system", "", true );

    // add dark and light styleSheets (built-in)
    foreach ( const QString name, BuiltInThemes::Names ) {
        QFile styleSheet;

        styleSheet.setFileName( QString( ":/styleSheets/%1.qss" ).arg( name ));
        if ( styleSheet.open( QFile::ReadOnly )) {
            this->add( name, styleSheet.readAll().constData(), true );
            styleSheet.close();
        }
    }
}

/**
 * @brief Themes::contains
 * @param name
 * @return
 */
bool Themes::contains( const QString &name ) const {
    return ( this->find( name ) != nullptr );
}

/**
 * @brief Themes::find
 * @param name
 * @return
 */
Theme *Themes::find( const QString &name ) const {
    foreach ( Theme *theme, this->list ) {
        if ( !QString::compare( name, theme->name()))
            return theme;
    }
    return nullptr;
}

/**
 * @brief Themes::add
 * @param name
 * @param styleSheet
 * @param builtIn
 */
void Themes::add( const QString &name, const QString &styleSheet, bool builtIn ) {
    this->list << new Theme( name, styleSheet, builtIn );
}

/**
 * @brief Themes::remove
 * @param name
 */
void Themes::remove( const QString &name ) {
    Theme *entry;

    entry = this->find( name );
    if ( entry == nullptr )
        return;

    this->list.removeAll( entry );
    delete entry;
}

