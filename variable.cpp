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
#include "variable.h"

/**
 * @brief Variable::connect
 * @param key
 * @param receiver
 * @param method
 */
void Variable::bind( const QString &key, const QObject *receiver, const char *method ) {
   QPair<QObject*, int> slot;
   int code;

    if ( key.isEmpty())
        return;

    // check if method is a slot
    code = ((( int )( *method ) - '0' ) & 0x3 );
    if ( code != 1 )
        return;

    // get method name
    ++method;

    // create an object/method pair
    slot.first = const_cast<QObject*>( receiver );
    slot.second = receiver->metaObject()->indexOfSlot( QMetaObject::normalizedSignature( qPrintable( method )));

    // add pair to slotList
    Variable::instance()->slotList[key] = slot;
}
