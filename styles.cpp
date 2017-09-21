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
#include "styles.h"
#include <QFile>

/**
 * @brief StyleManager::StyleManager
 */
StyleManager::StyleManager() {
    // add blank stylesheet
    this->add( "system", "", true );

    // add dark and light stylesheets (built-in)
    foreach ( const QString style, BuiltInStyles::Styles ) {
        QFile styleSheet;

        styleSheet.setFileName( QString( ":/stylesheets/%1.qss" ).arg( style ));
        if ( styleSheet.open( QFile::ReadOnly )) {
            this->add( style, styleSheet.readAll().constData(), true );
            styleSheet.close();
        }
    }
}

/**
 * @brief StyleManager::add
 * @param name
 * @param styleSheet
 * @param builtIn
 */
void StyleManager::add( const QString &name, const QString &styleSheet, bool builtIn ) {
    this->list[name] = StyleEntry( name, styleSheet, builtIn );
}

/**
 * @brief StyleManager::remove
 * @param name
 */
void StyleManager::remove( const QString &name ) {
    if ( this->contains( name ))
        this->list.remove( name );
}

/**
 * @brief StyleManagerStyleManager::rename
 * @param oldName
 * @param newName
 */
void StyleManager::rename( const QString &oldName, const QString &newName ) {
    StyleEntry entry;

    if ( !this->contains( oldName ))
        return;

    entry = this->list[oldName];
    this->list.remove( oldName );
    if ( entry.builtIn())
        return;

    entry.setName( newName );
    this->list[newName] = entry;
}

