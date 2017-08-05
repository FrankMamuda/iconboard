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
#include "traywidget.h"
#include "ui_traywidget.h"
#include "widgetmodel.h"
#include "folderview.h"

/**
 * @brief TrayWidget::TrayWidget
 * @param parent
 */
TrayWidget::TrayWidget( QWidget *parent ) : QWidget( parent, Qt::Tool ), ui( new Ui::TrayWidget ), tray( new QSystemTrayIcon( QIcon( ":/icons/launcher_96" ))), model( new WidgetModel( this, this )), desktop( new QDesktopWidget()) {
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
    SetParent(( HWND )this->desktop->winId(), this->worker );
    this->readXML();

    // connect tray icon
    this->connect( this->tray, SIGNAL( activated( QSystemTrayIcon::ActivationReason )), this, SLOT( trayIconActivated( QSystemTrayIcon::ActivationReason )));
    this->connect( qApp, SIGNAL( aboutToQuit()), this, SLOT( writeConfiguration()));
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
    menu.addAction( "Exit", qApp, SLOT( quit()));
    menu.exec( QCursor::pos());
}

/**
 * @brief TrayWidget::on_buttonAdd_clicked
 */
void TrayWidget::on_buttonAdd_clicked() {
    QDir dir;

    dir.setPath( QFileDialog::getExistingDirectory( this, this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));
    if ( dir.exists()) {
        this->widgetList << new FolderView( this->desktop, dir.absolutePath(), this->worker, this );
        this->widgetList.last()->show();
        this->ui->widgetList->reset();
    }
}

/**
 * @brief TrayWidget::on_buttonRemove_clicked
 */
void TrayWidget::on_buttonRemove_clicked() {
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
 * @brief decodeXML
 * @param buffer
 * @return
 */
QString TrayWidget::decodeXML( const QString &buffer ) {
    QTextDocument text;
    text.setHtml( buffer );
    return text.toPlainText();
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
        qDebug() << "error";
        return;
    }

    document.setContent( &xmlFile );
    node = document.documentElement().firstChild();

    while ( !node.isNull()) {
        element = node.toElement();

        if ( !element.isNull()) {
            if ( !QString::compare( element.tagName(), "widget" )) {
                FolderView *widget;
                QString text, styleSheet;
                QPoint pos;
                QSize size;
                bool isVisible = true;

                childNode = element.firstChild();
                widget = new FolderView( this->desktop, element.attribute( "rootPath" ), this->worker, this );

                pos = widget->pos();
                size = widget->size();

                while ( !childNode.isNull()) {
                    childElement = childNode.toElement();
                    text = TrayWidget::decodeXML( childElement.text());

                    if ( !childElement.isNull()) {
                        if ( !QString::compare( childElement.tagName(), "x" ))
                            pos.setX( text.toInt());
                        else if ( !QString::compare( childElement.tagName(), "y" ))
                            pos.setY( text.toInt());
                        else if ( !QString::compare( childElement.tagName(), "width" ))
                            size.setWidth( text.toInt());
                        else if ( !QString::compare( childElement.tagName(), "height" ))
                            size.setHeight( text.toInt());
                        else if ( !QString::compare( childElement.tagName(), "title" ))
                            widget->setCustomTitle( text );
                        else if ( !QString::compare( childElement.tagName(), "visible" ))
                            isVisible = static_cast<bool>( text.toInt());
                        else if ( !QString::compare( childElement.tagName(), "stylesheet" ))
                            styleSheet = text;
                        else if ( !QString::compare( childElement.tagName(), "listMode" )) {
                            if ( text.toInt() == 1 )
                                widget->setViewMode( QListView::ListMode );
                        } else if ( !QString::compare( childElement.tagName(), "iconSize" ))
                            widget->setIconSize( text.toInt());
                    }

                    childNode = childNode.nextSibling();
                }

                if ( isVisible )
                    widget->show();

                {
                    QRect updatedGeometry;
                    QPoint screenOffset;

                    //
                    // NOTE: code refactored for use in mutiple monitor systems
                    //       this still makes Qt complain about invalid geometry, but that does not matter at all
                    //

                    // get screen offset
                    screenOffset = QApplication::primaryScreen()->availableVirtualGeometry().topLeft();

                    // offset geometry and mouse position
                    updatedGeometry = QRect( pos.x(), pos.y(), size.width(), size.height());
                    updatedGeometry.translate( -screenOffset );

                    // use winapi to resize the window (avoiding buggy Qt setGeometry)
                    MoveWindow(( HWND )widget->winId(), updatedGeometry.x(), updatedGeometry.y(), updatedGeometry.width(), updatedGeometry.height(), true );
                }

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

    QTextStream stream( &xmlFile );
    stream << "<configuration>\n";
    foreach ( FolderView *widget, this->widgetList ) {
        stream << QString( "  <widget rootPath=\"%1\">\n" ).arg( widget->rootPath().toHtmlEscaped());
        stream << QString( "    <x>%1</x>\n" ).arg( widget->pos().x());
        stream << QString( "    <y>%1</y>\n" ).arg( widget->pos().y());
        stream << QString( "    <width>%1</width>\n" ).arg( widget->width());
        stream << QString( "    <height>%1</height>\n" ).arg( widget->height());

        if ( !widget->customTitle().isEmpty())
            stream << QString( "    <title>%1</title>\n" ).arg( widget->customTitle().toHtmlEscaped());

        if ( !widget->isVisible())
            stream << QString( "    <visible>0</visible>\n" );

        if ( !widget->customStyleSheet().isEmpty())
            stream << QString( "    <stylesheet>%1</stylesheet>\n" ).arg( widget->customStyleSheet().toHtmlEscaped());

        if ( widget->viewMode() == QListView::ListMode )
            stream << QString( "    <listMode>1</listMode>\n" );

        stream << QString( "    <iconSize>%1</iconSize>\n" ).arg( widget->iconSize());

        stream << "  </widget>\n";
    }
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
 * @brief TrayWidget::on_buttonVisibility_clicked
 */
void TrayWidget::on_buttonVisibility_clicked() {
    FolderView *widget;
    int row;

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
 * @brief TrayWidget::on_widgetList_doubleClicked
 * @param index
 */
void TrayWidget::on_widgetList_doubleClicked( const QModelIndex & ) {
    this->on_buttonVisibility_clicked();
}
