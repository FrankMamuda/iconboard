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
#include "stylesheetdialog.h"
#include "ui_stylesheetdialog.h"

/**
 * @brief StyleSheetDialog::StyleSheetDialog
 * @param parent
 */
StyleSheetDialog::StyleSheetDialog( QWidget *parent, const QString &styleSheet ) : QDialog( parent ), ui( new Ui::StyleSheetDialog ) {
    this->ui->setupUi( this );
    this->ui->editor->setPlainText( styleSheet );
}

/**
 * @brief StyleSheetDialog::~StyleSheetDialog
 */
StyleSheetDialog::~StyleSheetDialog() {
    delete this->ui;
}

/**
 * @brief StyleSheetDialog::customStyleSheet
 * @return
 */
QString StyleSheetDialog::customStyleSheet() const {
    return this->ui->editor->toPlainText();
}
