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
#include <QAction>
#include <QCheckBox>
#include <QWidget>
#include <QLineEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QLoggingCategory>

/**
 * @brief The Widget_ namespace
 */
namespace Widget_ {
    const static QLoggingCategory Debug( "widget" );
}

//
// classes
//
class Variable;

/**
 * @brief The Widget class
 */
class Widget final : public QObject {
    Q_OBJECT
    Q_ENUMS( Types )
    friend class Variable;

public:
    enum class Types {
        NoType = -1,
        CheckBox,
        Action,
        LineEdit,
        TimeEdit,
        SpinBox,
        ComboBox
    };

    /**
     * @brief type
     * @return
     */
    Types type() const { return this->m_type; }

    /**
     * @brief name
     * @return
     */
    QString name() const { return ( this->widget != nullptr ) ? this->widget->objectName() : QString(); }

    /**
     * @brief Widget
     * @param w
     */
    Widget( QObject *w ) : m_type( Types::NoType ), widget( w ) {
        // determine widget type
        if ( !QString::compare( widget->metaObject()->className(), "QCheckBox" )) {
            this->connection = this->connect( qobject_cast<QCheckBox*>( widget ), SIGNAL( stateChanged( int )), this, SLOT( valueChanged()));
            this->m_type = Types::CheckBox;
        } else if ( !QString::compare( widget->metaObject()->className(), "QAction" )) {
            this->connection = this->connect( qobject_cast<QAction*>( widget ), SIGNAL( triggered( bool )), this, SLOT( valueChanged()));
            this->m_type = Types::Action;
        } else if ( !QString::compare( widget->metaObject()->className(), "QLineEdit" )) {
            this->connection = this->connect( qobject_cast<QLineEdit*>( widget ), SIGNAL( textChanged( QString )), this, SLOT( valueChanged()));
            this->m_type = Types::LineEdit;
        } else if ( !QString::compare( widget->metaObject()->className(), "QTimeEdit" )) {
            this->connection = this->connect( qobject_cast<QTimeEdit*>( widget ), SIGNAL( timeChanged( QTime )), this, SLOT( valueChanged()));
            this->m_type = Types::TimeEdit;
        } else if ( !QString::compare( widget->metaObject()->className(), "QSpinBox" )) {
            this->connection = this->connect( qobject_cast<QSpinBox*>( widget ), SIGNAL( valueChanged( int )), this, SLOT( valueChanged()));
            this->m_type = Types::SpinBox;
        } else if ( !QString::compare( widget->metaObject()->className(), "QComboBox" )) {
            this->connection = this->connect( qobject_cast<QComboBox*>( widget ), SIGNAL( currentIndexChanged( int )), this, SLOT( valueChanged()));
            this->m_type = Types::ComboBox;
        } else {
            qCWarning( Widget_::Debug ) << this->tr( "unsupported container \"%1\"" ).arg( widget->metaObject()->className());
        }
    }

    /**
     * @brief ~Widget
     */
    ~Widget() { this->disconnect( this->connection ); }

    /**
     * @brief value
     * @return
     */
    QVariant value() const {
        if ( this->widget == nullptr )
            return QVariant();

        switch ( this->type()) {
        case Types::CheckBox:
            return qobject_cast<QCheckBox*>( this->widget )->isChecked();

        case Types::Action:
            return qobject_cast<QAction*>( this->widget )->isChecked();

        case Types::LineEdit:
            return qobject_cast<QLineEdit*>( this->widget )->text();

        case Types::TimeEdit:
            return qobject_cast<QTimeEdit*>( this->widget )->time();

        case Types::SpinBox:
            return qobject_cast<QSpinBox*>( this->widget )->value();

        case Types::ComboBox:
        {
            QComboBox *comboBox( qobject_cast<QComboBox*>( this->widget ));
            QAbstractItemModel *model( comboBox->model());

            if ( model != nullptr )
                return model->data( model->index( comboBox->currentIndex(), 0 ), Qt::UserRole );
        }
            break;

        case Types::NoType:
            break;
        }

        return QVariant();
    }

public slots:
    void setValue( const QVariant &value ) {
        if ( this->widget == nullptr )
            return;

        this->blockSignals( true );

        switch ( this->type()) {
        case Types::CheckBox:
            qobject_cast<QCheckBox*>( this->widget )->setChecked( static_cast<bool>( value.toInt()));
            break;

        case Types::Action:
            qobject_cast<QAction*>( this->widget )->setChecked( static_cast<bool>( value.toInt()));
            break;

        case Types::LineEdit:
            qobject_cast<QLineEdit*>( this->widget )->setText( value.toString());
            break;

        case Types::TimeEdit:
            qobject_cast<QTimeEdit*>( this->widget )->setTime( value.toTime());
            break;

        case Types::SpinBox:
            qobject_cast<QSpinBox*>( this->widget )->setValue( value.toInt());
            break;

        case Types::ComboBox:
        {
            QComboBox *comboBox( qobject_cast<QComboBox*>( this->widget ));
            QAbstractItemModel *model( comboBox->model());
            int y;

            if ( model != nullptr ) {
                for ( y = 0; y < model->rowCount(); y++ ) {
                    if ( model->data( model->index( y, 0 ), Qt::UserRole ) == value ) {
                        comboBox->setCurrentIndex( y );
                        break;
                    }
                }
            }
        }
            break;

        case Types::NoType:
            break;
        }

        this->blockSignals( false );
    }

private slots:
    void valueChanged() { emit this->changed( this->value()); }

signals:
    void changed( const QVariant &variant );

private:
    Types m_type;
    QObject *widget;
    QMetaObject::Connection connection;
};

// declare enums
Q_DECLARE_METATYPE( Widget::Types )
