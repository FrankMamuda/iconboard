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
#include <QVariant>
#include <QString>
#include <QMetaMethod>

/**
 * @brief The VariableEntry class
 */
class VariableEntry {
public:
    VariableEntry( const QString &key = QString::null, const QVariant &defaultValue = QVariant()) : m_key( key ), m_value( defaultValue ), m_defaultValue( defaultValue ) {}
    QString key() const { return this->m_key; }
    QVariant value() const { return this->m_value; }
    QVariant defaultValue() const { return this->m_defaultValue; }
    void setValue( const QVariant &value ) { m_value = value; }

private:
    QString m_key;
    QVariant m_value;
    QVariant m_defaultValue;
};

/**
 * @brief The Variable class
 */
class Variable final : public QObject {
    Q_OBJECT

public:
    ~Variable() {}
    static Variable *instance() { static Variable *instance( new Variable()); return instance; }
    bool contains( const QString &key ) const { return this->list.contains( key ); }

    template<typename T>
    T value( const QString &key, bool defaultValue = false ) { if ( defaultValue ) return qvariant_cast<T>( this->list[key].defaultValue()); return qvariant_cast<T>( this->list[key].value()); }
    int integer( const QString &key, bool defaultValue = false ) { return Variable::instance()->value<int>( key, defaultValue ); }
    bool isEnabled( const QString &key, bool defaultValue = false ) { return Variable::instance()->value<bool>( key, defaultValue ); }
    bool isDisabled( const QString &key, bool defaultValue = false ) { return !Variable::instance()->isEnabled( key, defaultValue ); }
    QString string( const QString &key, bool defaultValue = false ) { return Variable::instance()->value<QString>( key, defaultValue ); }

    template<typename T>
    void updateConnections( const QString &key, const T &value ) {
        if ( Variable::instance()->slotList.contains( key )) {
            QPair<QObject*, int> slot;

            slot = Variable::instance()->slotList[key];
            slot.first->metaObject()->method( slot.second ).invoke( slot.first, Qt::QueuedConnection, Q_ARG( QVariant, value ));
        }
    }

    template<typename T>
    void setValue( const QString &key, const T &value, bool initial = false ) {
        if ( initial ) {
            // initial read from configuration file
            Variable::instance()->list[key].setValue( value );
        } else {
            QVariant currentValue;

            if ( !Variable::instance()->contains( key ))
                return;

            currentValue = Variable::instance()->list[key].value();

            // any subsequent value changes emit a valueChanged signal
            if ( value != currentValue ) {
                Variable::instance()->list[key].setValue( value );
                emit valueChanged( key );
                Variable::instance()->updateConnections( key, value );
            }
        }
    }
    void setInteger( const QString &key, int value ) { Variable::instance()->setValue<int>( key, value ); }
    void enable( const QString &key ) { Variable::instance()->setValue<bool>( key, true ); }
    void disable( const QString &key ) { Variable::instance()->setValue<bool>( key, false ); }
    void setString( const QString &key, const QString &string ) { Variable::instance()->setValue<QString>( key, string ); }

    template<typename T>
    void add( const QString &key, const T &value ) { if ( !Variable::instance()->list.contains( key ) && !key.isEmpty()) Variable::instance()->list[key] = VariableEntry( key, QVariant( value )); }
    void reset( const QString &key ) { if ( Variable::instance()->contains( key )) Variable::instance()->setValue<QVariant>( key, Variable::instance()->value<QVariant>( key, true )); }

    QMap<QString, VariableEntry> list;
    QMap<QString, QPair<QObject*, int> > slotList;
    void bind( const QString &key, const QObject *receiver, const char *method );

signals:
    void valueChanged( const QString &key );

private:
    explicit Variable() {}
};
