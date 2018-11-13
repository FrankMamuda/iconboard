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
#include <QApplication>
#include <QDialog>
#include "ui_about.h"
#include "iconcache.h"

//
// namespaces
//
namespace Ui {
class About;
}

/**
 * @brief The About class
 */
class About : public QDialog {
    Q_OBJECT
    Q_CLASSINFO( "description", "About dialog" )

public:
    explicit About( QWidget *parent = nullptr ) : QDialog( parent ), ui( new Ui::About ) {
        this->ui->setupUi( this );
        this->ui->aboutQt->setIcon( IconCache::instance()->icon( "help-about", ":/icons/info", 16 ));
        this->ui->exitButton->setIcon( IconCache::instance()->icon( "dialog-close", ":/icons/close", 16 ));
    }
    ~About() { delete this->ui; }

private:
    Ui::About *ui;

private slots:
    void on_aboutQt_clicked() { QApplication::aboutQt(); }
};
