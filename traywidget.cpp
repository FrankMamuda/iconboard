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
#include <QMenu>
#include <QMessageBox>
#include <QFileDialog>
#include <QDataStream>
#include <QTextStream>
#include <QDomDocument>
#include <QDir>
#include <QScreen>
#include <QDebug>
#include <QTextDocument>
#include <QXmlStreamWriter>
#include <QBuffer>
#include "traywidget.h"
#include "ui_traywidget.h"
#include "widgetmodel.h"
#include "folderview.h"
#include "variable.h"
#include "settings.h"
#include "iconcache.h"
#include "screenmapper.h"

/**
 * @brief TrayWidget::TrayWidget
 * @param parent
 */
TrayWidget::TrayWidget( QWidget *parent ) : QMainWindow( parent, Qt::Tool ), ui( new Ui::TrayWidget ), tray( new QSystemTrayIcon( QIcon( ":/icons/launcher_96" ))), model( new WidgetModel( this, this )), desktop( new QDesktopWidget()) {
    // init ui
    this->ui->setupUi( this );

    // set up icon cache
#ifdef Q_OS_WIN
    QDir iconDir( QDir::currentPath() + "/icons" );
#else
    QDir iconDir( "/usr/share/icons" );
#endif
    QIcon::setThemeSearchPaths( QStringList( iconDir.absolutePath()));
    QIcon::setThemeName( Ui::lightIconTheme );

    // show tray icon
    this->tray->show();

    // init model
    this->ui->widgetList->setModel( this->model );

    // set up desktop widget
    this->getWindowHandles();
    this->wallpaper = desktop->grab();
    //SetParent(( HWND )this->desktop->winId(), this->worker );
    //this->readXML();

    // connect tray icon
    this->connect( this->tray, SIGNAL( activated( QSystemTrayIcon::ActivationReason )), this, SLOT( trayIconActivated( QSystemTrayIcon::ActivationReason )));
    this->connect( qApp, SIGNAL( aboutToQuit()), this, SLOT( writeConfiguration()));

    // set up icons
    this->ui->actionAdd->setIcon( IconCache::instance()->icon( "list-add" ));
    this->ui->actionRemove->setIcon( IconCache::instance()->icon( "list-remove" ));
    this->ui->actionMap->setIcon( IconCache::instance()->icon( "view-grid" ));
    this->ui->actionShow->setIcon( IconCache::instance()->icon( "visibility" ));
}

/**
 * @brief TrayWidget::~TrayWidget
 */
TrayWidget::~TrayWidget() {
    this->disconnect( this->tray, SIGNAL( activated( QSystemTrayIcon::ActivationReason )));
    //delete this->cache;
    delete this->desktop;
    delete this->model;
    delete this->tray;
    delete this->ui;
}

/**
 * @brief TrayWidget::trayIconActivated
 */
void TrayWidget::trayIconActivated( QSystemTrayIcon::ActivationReason reason ) {
    switch ( reason ) {
    case QSystemTrayIcon::Trigger:
        if ( this->isHidden())
            this->show();
        else
            this->hide();
        break;

    case QSystemTrayIcon::Context:
        this->showContextMenu();
        break;

    default:
        break;
    }
}

/**
 * @brief TrayWidget::showContextMenu
 */
void TrayWidget::showContextMenu() {
    QMenu menu;
    menu.addAction( this->tr( "Settings" ), this, SLOT( showSettingsDialog()));
    menu.addSeparator();
    menu.addAction( this->tr( "Exit" ), qApp, SLOT( quit()));
    menu.exec( QCursor::pos());
}

/**
 * @brief TrayWidget::readXML
 */
void TrayWidget::readXML() {
    QString path;
    QDomDocument document;
    QDomNode node, childNode;
    QDomElement element, childElement;

#ifdef QT_DEBUG
    path = QDir::homePath() + "/.iconBoardDebug/configuration.xml";
#else
    path = QDir::homePath() + "/.iconBoard/configuration.xml";
#endif

    // load xml file
    QFile xmlFile( path );

    if ( !xmlFile.exists() || !xmlFile.open( QFile::ReadOnly | QFile::Text )) {
        qDebug() << this->tr( "TrayWidget::readXML: no configuration file found" );
        return;
    }

    document.setContent( &xmlFile );
    node = document.documentElement().firstChild();

    while ( !node.isNull()) {
        element = node.toElement();

        if ( !element.isNull()) {
            if ( !QString::compare( element.tagName(), "variable" )) {
                QString key;
                QVariant value;

                childNode = element.firstChild();
                key = element.attribute( "key" );

                if ( element.hasAttribute( "binary" )) {
                    QByteArray array;
                    array = QByteArray::fromBase64( element.attribute( "binary" ).toUtf8().constData());
                    QBuffer buffer( &array );
                    buffer.open( QIODevice::ReadOnly );
                    QDataStream in( &buffer );
                    in >> value;
                } else
                    value = element.attribute( "value" );

                if ( Variable::instance()->contains( key ))
                    Variable::instance()->setValue( key, value, true );
            } else if ( !QString::compare( element.tagName(), "widget" )) {
                FolderView *widget;
                QString text, styleSheet;
                bool isVisible = true;
                bool readOnly = true;
                QRect widgetGeometry;

                childNode = element.firstChild();
                widget = new FolderView( this->desktop, element.attribute( "rootPath" ), this->worker, this );
                widgetGeometry = widget->geometry();

                styleSheet = element.attribute( "stylesheet" );

                while ( !childNode.isNull()) {
                    childElement = childNode.toElement();
                    text = childElement.text();

                    if ( !childElement.isNull()) {
                        if ( !QString::compare( childElement.tagName(), "geometry" )) {
                            widgetGeometry = QRect(
                                        childElement.attribute( "x" ).toInt(),
                                        childElement.attribute( "y" ).toInt(),
                                        childElement.attribute( "width" ).toInt(),
                                        childElement.attribute( "height" ).toInt());
                        } else if ( !QString::compare( childElement.tagName(), "title" )) {
                            widget->setCustomTitle( text );
                        } else if ( !QString::compare( childElement.tagName(), "visible" )) {
                            isVisible = static_cast<bool>( text.toInt());
                        } else if ( !QString::compare( childElement.tagName(), "viewMode" )) {
                            widget->setViewMode( static_cast<QListView::ViewMode>( text.toInt()));
                        } else if ( !QString::compare( childElement.tagName(), "readOnly" ))
                            readOnly = static_cast<bool>( text.toInt());
                        else if ( !QString::compare( childElement.tagName(), "iconSize" ))
                            widget->setIconSize( text.toInt());
                    }

                    childNode = childNode.nextSibling();
                }

                if ( isVisible )
                    widget->show();

                {
                    QPoint screenOffset;

                    //
                    // NOTE: code refactored for use in mutiple monitor systems
                    //       this still makes Qt complain about invalid geometry, but that does not matter at all
                    //

                    // get screen offset
                    screenOffset = QApplication::primaryScreen()->availableVirtualGeometry().topLeft();

                    // offset geometry and mouse position
                    widgetGeometry.translate( -screenOffset );

                    // use winapi to resize the window (avoiding buggy Qt setGeometry)
                    MoveWindow(( HWND )widget->winId(), widgetGeometry.x(), widgetGeometry.y(), widgetGeometry.width(), widgetGeometry.height(), true );
                }

                widget->setReadOnly( readOnly );

                if ( !styleSheet.isEmpty())
                    widget->setCustomStyleSheet( styleSheet );

                this->widgetList << widget;
            }
        }
        node = node.nextSibling();
    }

    this->ui->widgetList->reset();
    document.clear();
    xmlFile.close();
}

/**
 * @brief TrayWidget::writeConfiguration
 */
void TrayWidget::writeConfiguration() {
    QString path;
#ifdef QT_DEBUG
    QDir configDir( QDir::homePath() + "/.iconBoardDebug/" );
#else
    QDir configDir( QDir::homePath() + "/.iconBoard/" );
#endif

    if ( !configDir.exists())
        configDir.mkpath( configDir.absolutePath());

    path = configDir.absolutePath() + "/configuration.xml";

    // load xml file
    QFile xmlFile( path );
    if ( !xmlFile.open( QFile::WriteOnly | QFile::Text | QFile::Truncate )) {
        qDebug() << "TrayWidget::writeConfiguration: error - could not open configuration file" << path;
        return;
    }

    QXmlStreamWriter stream( &xmlFile );
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement( "configuration" );
    stream.writeAttribute( "version", "2" );

    // write out variables
    foreach ( VariableEntry var, Variable::instance()->list ) {
        stream.writeEmptyElement( "variable" );
        stream.writeAttribute( "key", var.key());

        if ( !var.value().canConvert<QString>()) {
            QByteArray array;
            QBuffer buffer(&array);

            buffer.open( QIODevice::WriteOnly );
            QDataStream out( &buffer );

            out << var.value();
            buffer.close();

            stream.writeAttribute( "binary", QString( array.toBase64()));
        } else {
            stream.writeAttribute( "value", var.value().toString());
        }
    }

    // write out widgets
    foreach ( FolderView *widget, this->widgetList ) {
        stream.writeStartElement( "widget" );
        stream.writeAttribute( "rootPath", widget->rootPath());

        // stylesheet (for better readability store as attribute)
        stream.writeAttribute( "stylesheet", widget->customStyleSheet().replace( "\r", "" ));

        // geometry
        stream.writeEmptyElement( "geometry" );
        stream.writeAttribute( "x", QString::number( widget->pos().x()));
        stream.writeAttribute( "y", QString::number( widget->pos().y()));
        stream.writeAttribute( "width", QString::number( widget->width()));
        stream.writeAttribute( "height", QString::number( widget->height()));

        // title
        if ( !widget->customTitle().isEmpty())
            stream.writeTextElement( "title", widget->customTitle());

        // visibility
        if ( !widget->isVisible())
            stream.writeTextElement( "visible", QString::number( 0 ));

        // viewMode
        stream.writeTextElement( "viewMode", QString::number( static_cast<int>( widget->viewMode())));

        // access mode
        if ( !widget->isReadOnly())
            stream.writeTextElement( "readOnly", QString::number( 0 ));

        // icon size
        stream.writeTextElement( "iconSize", QString::number( widget->iconSize()));

        // end widget element
        stream.writeEndElement();
    }

    /*
    stream2.writeStartElement( "variable" );
    stream2.writeAttribute( "name", "cvarSmth" );
    stream2.writeEndElement();

    CREATION:
                            #name           #defaultValue
    Variable::add( "ui_displaySymlinkIcon", true );

    STORAGE:
    <variable name="ui_displaySymlinkIcon" value="true">

    ACCESS:
    Variable::value( "ui_displaySymlinkIcon" ).toBool();
      searches for variable:
        not found - false
        found - value

    add flags in future such as archive, temporary, read only, etc.
*/

    // end config element
    stream.writeEndElement();

    // end document
    stream.writeEndDocument();

    // close file
    xmlFile.close();
}

/**
 * @brief FolderView::getWindowHandles
 */
void TrayWidget::getWindowHandles() {
    HWND desktop, progman, shell = nullptr, worker = nullptr;

    // get desktop window
    desktop = GetDesktopWindow();
    if ( desktop == nullptr )
        return;

    // get progman
    progman = FindWindowEx( desktop, 0, L"Progman", L"Program Manager" );
    if ( progman == nullptr )
        return;

    // get first worker and shell
    SendMessageTimeout( progman, 0x052C, 0, 0, SMTO_NORMAL, 3000, NULL);
    while( shell == nullptr ) {
        worker = FindWindowEx( desktop, worker, L"WorkerW", 0 );
        if ( worker != nullptr )
            shell = FindWindowEx( worker, 0, L"SHELLDLL_DefView", 0 );
        else
            break;
    }

    // store worker handle
    this->worker = worker;
}

/**
 * @brief TrayWidget::on_widgetList_doubleClicked
 * @param index
 */
void TrayWidget::on_widgetList_doubleClicked( const QModelIndex & ) {
    this->on_actionShow_triggered();
}
/**
 * @brief TrayWidget::showSettingsDialog
 */
void TrayWidget::showSettingsDialog() {
    Settings settingsDialog;

    settingsDialog.exec();
   // settingsDialog->show();
}

/**
 * @brief TrayWidget::on_actionAdd_triggered
 */
void TrayWidget::on_actionAdd_triggered() {
    QDir dir;

    dir.setPath( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));
    if ( dir.exists()) {
        this->widgetList << new FolderView( this->desktop, dir.absolutePath(), this->worker, this );
        this->widgetList.last()->show();
        this->ui->widgetList->reset();
    }
}

/**
 * @brief TrayWidget::on_actionRemove_triggered
 */
void TrayWidget::on_actionRemove_triggered() {
    QMessageBox msgBox;
    FolderView *widget;
    int row, state;

    row = this->ui->widgetList->currentIndex().row();
    if ( row < 0 || row >= this->widgetList.count())
        return;

    // get widget ptr
    widget = this->widgetList.at( row );

    // display warning
    msgBox.setText( this->tr( "Do you really want to remove \"%1\"?" ).arg( widget->title()));
    msgBox.setStandardButtons( QMessageBox::Yes | QMessageBox::No );
    msgBox.setDefaultButton( QMessageBox::Yes );
    msgBox.setIcon( QMessageBox::Warning );
    state = msgBox.exec();

    // check options
    switch ( state ) {
    case QMessageBox::Yes:
        widget->hide();
        this->widgetList.removeOne( widget );
        this->ui->widgetList->reset();
        //delete widget;
        break;

    case QMessageBox::No:
    default:
        return;
    }
}

/**
 * @brief TrayWidget::on_actionShow_triggered
 */
void TrayWidget::on_actionShow_triggered() {
    FolderView *widget;
    int row;

    // TODO: set checkable

    row = this->ui->widgetList->currentIndex().row();
    if ( row < 0 || row >= this->widgetList.count())
        return;

    // get widget ptr
    widget = this->widgetList.at( row );

    // toggle visibility
    if ( widget->isVisible())
        widget->hide();
    else
        widget->show();
}

/**
 * @brief TrayWidget::on_actionMap_triggered
 */
void TrayWidget::on_actionMap_triggered() {
    ScreenMapper mapperDialog;
    mapperDialog.exec();
}

/**
 * @brief TrayWidget::on_buttonClose_clicked
 */
void TrayWidget::on_buttonClose_clicked() {
    this->hide();
}
