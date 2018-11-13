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
#include <QDebug>
#include <QCheckBox>
#include <QSpinBox>
#include <QComboBox>
#include <QSettings>
#include <QScreen>
#include "settings.h"
#include "ui_settings.h"
#include "iconindex.h"
#include "iconcache.h"
#include "main.h"
#include "variable.h"

/**
 * @brief Settings::Settings
 * @param parent
 */
Settings::Settings( QWidget *parent ) : QDialog( parent ), ui( new Ui::Settings ) {
    const QDir dir( IconIndex::instance()->path());

    // set up ui
    this->ui->setupUi( this );
    this->ui->closeButton->setIcon( IconCache::instance()->icon( "dialog-close", ":/icons/close", 16 ));

    // find all icon dirs
    this->ui->iconTheme->addItem( this->tr( "System default" ), "system" );
    foreach ( const QString &path, dir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot )) {
        const QFile file( IconIndex::instance()->path() + "/" + path + "/" + "index.theme" );
        if ( !file.exists())
            continue;

        this->ui->iconTheme->addItem( path, path );
    }

    // first bind vars and set initial values
    Variable::instance()->bind( "ui_displaySymlinkIcon", this->ui->displaySymlinkIcon );
    Variable::instance()->bind( "ui_iconTheme", this->ui->iconTheme );
    Variable::instance()->bind( "app_lockToResolution", this->ui->lockToResolution );
    Variable::instance()->bind( "app_lockToResolution", this, SLOT( lockToResolutionValueChanged( QVariant )));
    this->setResolutionToolTip();

#ifdef Q_OS_WIN
    Variable::instance()->bind( "app_runOnStartup", this->ui->runOnStartup );

    // bind runOnStarup variable, to write out settings value
    Variable::instance()->bind( "app_runOnStartup", this, SLOT( runOnStartupValueChanged( QVariant )));
#else
    // hide runOnStartup option on non-win32 systems
    this->ui->runOnStartup->hide();
    this->resize( this->width(), 0 );
#endif
}

/**
 * @brief Settings::~Settings
 */
Settings::~Settings() {
    Variable::instance()->unbind( "ui_displaySymlinkIcon", this->ui->displaySymlinkIcon );
    Variable::instance()->unbind( "ui_iconTheme", this->ui->iconTheme );
    Variable::instance()->unbind( "app_lockToResolution", this->ui->lockToResolution );

#ifdef Q_OS_WIN
    Variable::instance()->unbind( "app_runOnStartup", this->ui->runOnStartup );
#endif

    delete this->ui;
}

/**
 * @brief Settings::on_closeButton_clicked
 */
void Settings::on_closeButton_clicked() {
    this->accept();
}

/**
 * @brief Settings::runOnStartupValueChanged
 * @param value
 */
void Settings::runOnStartupValueChanged( QVariant value ) {
#ifdef QT_DEBUG
    qDebug() << "runOnStartupValueChanged changed to" << value.toBool();
#else
    QSettings settings( "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat );

    if ( value.toBool())
        settings.setValue( "IconBoardApp", QCoreApplication::applicationFilePath().replace( '/', '\\' ));
    else
        settings.remove( "IconBoardApp" );
#endif
}

/**
 * @brief Settings::lockToResolutionValueChanged
 * @param value
 */
void Settings::lockToResolutionValueChanged( QVariant value ) {
    Variable::instance()->setValue<bool>( "app_lockToResolution", value.toBool());
    Variable::instance()->setValue<QSize>( "app_targetResolution", Main::instance()->currentResolution());
    this->setResolutionToolTip();
}

/**
 * @brief Settings::setResolutionToolTip
 */
void Settings::setResolutionToolTip() {
    bool enable = Variable::instance()->isEnabled( "app_lockToResolution" );
    QSize resolution( Variable::instance()->value<QSize>( "app_targetResolution" ));
    this->ui->lockToResolution->setToolTip( enable ? this->tr( "Currently locked to %1x%2" ).arg( resolution.width()).arg( resolution.height()) : this->tr( "Resolution lock not set\n" ));
}
