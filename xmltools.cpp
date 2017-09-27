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
#include "xmltools.h"
#include "variable.h"
#include "traywidget.h"
#include "folderview.h"
#include "themes.h"
#include <QBuffer>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QXmlStreamWriter>
#include <QScreen>

/**
 * @brief XMLTools::XMLTools
 * @param parent
 */
XMLTools::XMLTools( QObject *parent ) : QObject( parent ) {
}

/**
 * @brief XMLTools::writeConfiguration
 * @param mode
 */
void XMLTools::writeConfiguration( Modes mode, QObject *object ) {
    QString path, savedData, newData;

#ifdef QT_DEBUG
    QDir configDir( QDir::homePath() + "/.iconBoardDebug/" );
#else
    QDir configDir( QDir::homePath() + "/.iconBoard/" );
#endif

    if ( !configDir.exists())
        configDir.mkpath( configDir.absolutePath());

    // switch mode
    switch ( mode ) {
    case Variables:
        path = configDir.absolutePath() + "/" + XMLFiles::Variables;
        break;

    case Widgets:
        path = configDir.absolutePath() + "/" + XMLFiles::Widgets;
        if ( object == nullptr )
            return;
        break;

    case Themes:
        path = configDir.absolutePath() + "/" + XMLFiles::Themes;
        break;

    case NoMode:
    default:
        qDebug() << "XMLTools::writeConfiguration: error - invalid mode";
        return;
    }

    // read xml file and create buffer
    QFile xmlFile( path );
    QBuffer xmlBuffer;
    xmlBuffer.open( QBuffer::WriteOnly | QBuffer::Text | QBuffer::Truncate );

    // create stream
    QXmlStreamWriter stream( &xmlBuffer );
    stream.setAutoFormatting( true );
    stream.writeStartDocument();
    stream.writeStartElement( "configuration" );
    stream.writeAttribute( "version", "3" );

    // switch mode
    switch ( mode ) {
    case Variables:
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
        break;

    case Widgets:
    {
        TrayWidget *trayWidget;

        // get tray widget
        trayWidget = qobject_cast<TrayWidget *>( object );
        if ( trayWidget == nullptr )
            return;

        foreach ( FolderView *widget, trayWidget->widgetList ) {
            stream.writeStartElement( "widget" );
            stream.writeAttribute( "rootPath", widget->rootPath());

            // styleSheet (for better readability store as attribute)
            stream.writeAttribute( "styleSheet", widget->customStyleSheet().replace( "\r", "" ));

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

            // sortOrder
            stream.writeTextElement( "sortOrder", QString::number( static_cast<int>( widget->sortOrder())));

            // caseSensitive
            stream.writeTextElement( "caseSensitive", QString::number( static_cast<int>( widget->isCaseSensitive())));

            // dirsFirst
            stream.writeTextElement( "dirsFirst", QString::number( static_cast<int>( widget->directoriesFirst())));

            // access mode
            if ( !widget->isReadOnly())
                stream.writeTextElement( "readOnly", QString::number( 0 ));

            // icon size
            stream.writeTextElement( "iconSize", QString::number( widget->iconSize()));

            // end widget element
            stream.writeEndElement();
        }

    }
        break;

    case Themes:
        // begin theme element
        foreach ( Theme *theme, Themes::instance()->list ) {
            // skip built in themes
            if ( theme->builtIn())
                continue;

            // begin theme element
            stream.writeStartElement( "theme" );

            // styleSheet (for better readability store as attribute)
            stream.writeAttribute( "name", theme->name());
            stream.writeAttribute( "styleSheet", theme->styleSheet().replace( "\r", "" ));

            // end theme element
            stream.writeEndElement();
        }
        break;

    case NoMode:
    default:
        qDebug() << "XMLTools::writeConfiguration: error - invalid mode";
        return;
    }

    // end config element
    stream.writeEndElement();

    // end document
    stream.writeEndDocument();

    // close buffer
    xmlBuffer.close();

    // read existing config from file
    if ( xmlFile.open( QFile::ReadOnly | QIODevice::Text )) {
        savedData = xmlFile.readAll();
        xmlFile.close();
    }

    // read new config from buffer
    if ( xmlBuffer.open( QFile::ReadOnly | QIODevice::Text )) {
        newData = xmlBuffer.readAll();
        xmlBuffer.close();
    }

    // compare data
    if ( !QString::compare( savedData, newData )) {
        //qDebug() << "XMLTools::writeConfiguration: data identical, aboring save" << mode;
    } else {
        // write out as binary (not QIODevice::Text) to avoid CR line endings
        if ( !xmlFile.open( QFile::WriteOnly | QFile::Truncate )) {
            qDebug() << "XMLTools::writeConfiguration: error - could not open configuration file" << path;
            return;
        }
        xmlFile.write( newData.toUtf8().replace( "\r", "" ));
    }

    // close file
    xmlFile.close();
}

/**
 * @brief XMLTools::readConfiguration
 * @param mode
 * @param object
 */
void XMLTools::readConfiguration( Modes mode, QObject *object ) {
    QString path;
    QDomDocument document;
    QDomNode node, childNode;
    QDomElement element, childElement;
    TrayWidget *trayWidget;

#ifdef QT_DEBUG
    QDir configDir( QDir::homePath() + "/.iconBoardDebug/" );
#else
    QDir configDir( QDir::homePath() + "/.iconBoard/" );
#endif

    if ( !configDir.exists())
        configDir.mkpath( configDir.absolutePath());

    // switch mode
    switch ( mode ) {
    case Variables:
        path = configDir.absolutePath() + "/" + XMLFiles::Variables;
        break;

    case Widgets:
        path = configDir.absolutePath() + "/" + XMLFiles::Widgets;
        if ( object == nullptr )
            return;

        // get tray widget
        trayWidget = qobject_cast<TrayWidget *>( object );
        if ( trayWidget == nullptr )
            return;
        break;

    case Themes:
        path = configDir.absolutePath() + "/" + XMLFiles::Themes;
        break;

    case NoMode:
    default:
        qDebug() << "XMLTools::readConfiguration: error - invalid mode";
        return;
    }

    // load xml file
    QFile xmlFile( path );

    if ( !xmlFile.exists() || !xmlFile.open( QFile::ReadOnly | QFile::Text )) {
        qDebug() << this->tr( "XMLTools::readConfiguration: no configuration file found" );
        return;
    }

    document.setContent( &xmlFile );
    node = document.documentElement().firstChild();

    while ( !node.isNull()) {
        element = node.toElement();

        if ( !element.isNull()) {
            if ( !QString::compare( element.tagName(), "widget" ) && mode == Widgets ) {
                FolderView *widget;
                QString text, styleSheet;
                bool isVisible = true;
                bool readOnly = true;
                QRect widgetGeometry;
                QPoint screenOffset;
                Qt::SortOrder sortOrder = Qt::AscendingOrder;
                bool dirsFirst = true;
                bool caseSensitive = false;

                childNode = element.firstChild();
#ifdef Q_OS_WIN
                widget = new FolderView( nullptr, element.attribute( "rootPath" ), trayWidget->worker, trayWidget );
#else
                widget = new FolderView( nullptr, element.attribute( "rootPath" ), trayWidget );
#endif
                widgetGeometry = widget->geometry();

                styleSheet = element.attribute( "styleSheet" );

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
                        } else if ( !QString::compare( childElement.tagName(), "readOnly" )) {
                            readOnly = static_cast<bool>( text.toInt());
                        } else if ( !QString::compare( childElement.tagName(), "sortOrder" )) {
                            sortOrder = static_cast<Qt::SortOrder>( text.toInt());
                        } else if ( !QString::compare( childElement.tagName(), "dirsFirst" )) {
                            dirsFirst = static_cast<bool>( text.toInt());
                        } else if ( !QString::compare( childElement.tagName(), "caseSensitive" )) {
                            caseSensitive = static_cast<bool>( text.toInt());
                        } else if ( !QString::compare( childElement.tagName(), "iconSize" )) {
                            widget->setIconSize( text.toInt());
                        }
                    }

                    childNode = childNode.nextSibling();
                }

                if ( isVisible )
                    widget->show();

                //
                // NOTE: code refactored for use in mutiple monitor systems
                //       this still makes Qt complain about invalid geometry, but that does not matter at all
                //

                // get screen offset
                screenOffset = QApplication::primaryScreen()->availableVirtualGeometry().topLeft();

                // offset geometry and mouse position
                widgetGeometry.translate( -screenOffset );

                // use winapi to resize the window (avoiding buggy Qt setGeometry)
#ifdef Q_OS_WIN
                MoveWindow(( HWND )widget->winId(), widgetGeometry.x(), widgetGeometry.y(), widgetGeometry.width(), widgetGeometry.height(), true );
#else
                widget->setGeometry( widgetGeometry );
#endif

                widget->setCaseSensitive( caseSensitive );
                widget->setDirectoriesFirst( dirsFirst );
                widget->setReadOnly( readOnly );
                widget->setSortOrder( sortOrder );
                widget->setCustomStyleSheet( styleSheet );
                widget->sort();
                trayWidget->widgetList << widget;
            } else if ( !QString::compare( element.tagName(), "variable" ) && mode == Variables ) {
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
            } else if ( !QString::compare( element.tagName(), "theme" ) && mode == Themes ) {
                QString name, styleSheet;

                childNode = element.firstChild();
                name = element.attribute( "name" );
                styleSheet = element.attribute( "styleSheet" );

                if ( !Themes::instance()->contains( name ) && !name.isNull() && !styleSheet.isNull())
                    Themes::instance()->add( name, styleSheet, false );
            }
        }
        node = node.nextSibling();
    }

    document.clear();
    xmlFile.close();
}
