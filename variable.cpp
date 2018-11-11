/*
 * Copyright (C) 2013-2018 Factory #12
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
#include "main.h"
#include "variable.h"
#include "widget.h"
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDebug>
#include <QLineEdit>
#include <QTimeEdit>
#include <QAction>

/**
 * @brief Variable::Variable
 */
Variable::Variable() {
    // update widgets on variable change
    this->connect( this, &Variable::valueChanged, [ this ]( const QString &key ) {
        foreach ( Widget *widget, this->boundVariables.values( key )) {
            QVariant var( this->value<QVariant>( key ));

            if ( widget->value() != var )
                widget->setValue( var );
        }
    } );

    // update variable and sibling widgets on widget change
    this->connect( this, &Variable::widgetChanged, [ this ]( const QString &key, Widget *widget, const QVariant &value ) {
        foreach ( Widget *boundWidget, this->boundVariables.values( key )) {
            if ( boundWidget == widget ) {
                this->setValue( key, value );
            } else {
                boundWidget->setValue( value );
            }
        }
    } );
}

/**
 * @brief Variable::~Variable
 */
Variable::~Variable() {
    this->disconnect( this, SIGNAL( widgetChanged( QString, Widget*, QVariant )));
    this->disconnect( this, SIGNAL( valueChanged( QString )));
}

/**
 * @brief Variable::bind
 * @param key
 * @param receiver
 * @param method
 */
void Variable::bind( const QString &key, const QObject *receiver, const char *method ) {
    QPair<QObject*, int> slot;

    if ( key.isEmpty())
        return;

    // check if method is a slot
    const int code = (( static_cast<int>( *method ) - '0' ) & 0x3 );
    if ( code != 1 )
        return;

    // get method name
    ++method;

    // create an object/method pair
    slot.first = const_cast<QObject*>( receiver );
    slot.second = receiver->metaObject()->indexOfSlot( QMetaObject::normalizedSignature( qPrintable( method )));

    // add pair to slotList
    this->slotList[key] = slot;
}

/**
 * @brief Variable::bind
 * @param key
 * @param object
 * @return
 */
QString Variable::bind( const QString &key, QObject *object ) {
    Widget *boundWidget( new Widget( object ));

    boundWidget->setValue( this->value<QVariant>( key ));
    this->connect( boundWidget, &Widget::changed, [ this, key, boundWidget ]( const QVariant &value ) { emit this->widgetChanged( key, boundWidget, value ); } );
    this->boundVariables.insert( key, boundWidget );

    return key;
}

/**
 * @brief Variable::unbind
 * @param key
 * @param object
 */
void Variable::unbind( const QString &key, QObject *object ) {
    if ( this->boundVariables.contains( key )) {
        QList<Widget*> widgetList( this->boundVariables.values( key ));

        if ( object == nullptr ) {
            qDeleteAll( widgetList );
            this->boundVariables.remove( key );
            return;
        }

        foreach ( Widget *compare, this->boundVariables.values( key )) {
            if ( compare->widget == object ) {
                this->boundVariables.remove( key, compare );
                delete compare;
            }
        }
    }
}
