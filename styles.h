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

/**
 * @brief The Style class
 */
class Style : public QObject {
   Q_OBJECT
   Q_PROPERTY( QString name READ name WRITE setName )
   Q_PROPERTY( QString styleSheet READ styleSheet WRITE setStyleSheet )

public:
    QString name() const { return this->m_name; }
    QString styleSheet() const { return this->m_styleSheet; }

public slots:
    void setStyleSheet( const QString &value ) { this->m_styleSheet = value; }
    void setName( const QString &value ) { this->m_name = value; }

private:
    QString m_name;
    QString m_styleSheet;
};

