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
#include "iconcache.h"
#include "iconindex.h"
#include "stylesheetdialog.h"
#include "ui_stylesheetdialog.h"

#include <QComboBox>
#include <QLabel>

/**
 * @brief StyleSheetDialog::StyleSheetDialog
 * @param parent
 */
StyleSheetDialog::StyleSheetDialog( QWidget *parent, const QString &styleSheet ) : QDialog( parent ), ui( new Ui::StyleSheetDialog ), toolBar( new QToolBar( this )) {
    this->ui->setupUi( this );
    this->ui->editor->setPlainText( styleSheet );

    // set up toolbar
#if 0
    this->toolBar->setFloatable( false );
    this->toolBar->setMovable( false );
    this->toolBar->setIconSize( QSize( 16, 16 ));
    this->toolBar->setAllowedAreas( Qt::TopToolBarArea );
    this->toolBar->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    this->ui->verticalLayout->setMenuBar( this->toolBar );

    // add buttons
    this->toolBar->addAction( IconCache::instance()->icon( "document-save", 16, IconIndex::instance()->defaultTheme()), this->tr( "Save" ));
    this->toolBar->addWidget( new QLabel( "Base style" ));
    this->toolBar->addWidget( new QComboBox());
#endif
}

/**
 * @brief StyleSheetDialog::~StyleSheetDialog
 */
StyleSheetDialog::~StyleSheetDialog() {
    //delete this->toolBar;
    delete this->ui;
}

/**
 * @brief StyleSheetDialog::customStyleSheet
 * @return
 */
QString StyleSheetDialog::customStyleSheet() const {
    return this->ui->editor->toPlainText();
}
