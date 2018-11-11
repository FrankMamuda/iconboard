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

#pragma once

//
// includes
//
#include <QSharedPointer>
#include <QVariant>

/**
 * @brief The Var class
 */
class Var final {
public:
    enum class Flag {
        NoFlags  = 0x0,
        ReadOnly = 0x1,
        NoSave   = 0x2,
        Hidden   = 0x4
    };
    Q_DECLARE_FLAGS( Flags, Flag )

    explicit Var( const QString &key = QString(), const QVariant &defaultValue = QVariant(), Flags flags = Flag::NoFlags ) : m_key( key ), m_value( defaultValue ), m_defaultValue( defaultValue ), m_flags( flags ) {}
    virtual ~Var() = default;
    QString key() const { return this->m_key; }
    Flags flags() const { return this->m_flags; }
    virtual QVariant value() const { return this->m_value; }
    QVariant defaultValue() const { return this->m_defaultValue; }
    virtual void setValue( const QVariant &value ) { m_value = value; }
    Var& operator = ( const Var & ) = default;
    Var( const Var& ) = default;
    virtual QSharedPointer<Var> copy() const { return QSharedPointer<Var>( new Var( *this )); }

private:
    QString m_key;
    QVariant m_value;
    QVariant m_defaultValue;
    Flags m_flags;
};

// declare flags
Q_DECLARE_OPERATORS_FOR_FLAGS( Var::Flags )
