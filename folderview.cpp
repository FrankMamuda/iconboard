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
#include <QFileDialog>
#include <QDebug>
#include <QInputDialog>
#include <QDesktopServices>
#include <QPainter>
#include <QMenu>
#include <QScreen>
#include <QStorageInfo>
#include "folderview.h"
#include "folderdelegate.h"
#include "traywidget.h"
#include "iconproxymodel.h"
#include "stylesheetdialog.h"
#include "iconcache.h"
#include "iconindex.h"
#ifdef Q_OS_WIN
#include <shlobj.h>
#endif

//
// defines
//
#define PROXY_MODEL

/**
 * @brief FolderView::FolderView
 * @param parent
 * @param rootPath
 */
#ifdef Q_OS_WIN
FolderView::FolderView( QWidget *parent, const QString &rootPath, HWND windowParent, TrayWidget *trayParent ) : QWidget( parent ), ui( new Ui::FolderView )/*, proxyModel( new IconProxyModel())*/, model( new FileSystemModel()), gesture( NoGesture ), currentGrabArea( NoArea ), trayWidget( trayParent ) {
#else
FolderView::FolderView( QWidget *parent, const QString &rootPath, TrayWidget *trayParent ) : QWidget( parent ), ui( new Ui::FolderView )/*, proxyModel( new IconProxyModel())*/, model( new FileSystemModel()), gesture( NoGesture ), currentGrabArea( NoArea ), trayWidget( trayParent ) {
#endif
    QDir dir( rootPath );
    QFile styleSheet;

    // set up UI
    this->ui->setupUi( this );

    // set up listView and its model
    this->ui->view->setViewMode( QListView::IconMode );
    //this->model->setResolveSymlinks( false );
#ifdef PROXY_MODEL
    this->proxyModel = new IconProxyModel( this );
    this->proxyModel->setSourceModel( this->model );
    this->ui->view->setModel( this->proxyModel );
    this->ui->view->setRootIndex( this->proxyModel->mapFromSource( this->model->setRootPath( rootPath )));
#else
    this->ui->view->setModel( this->model );
    this->ui->view->setRootIndex( this->model->setRootPath( rootPath )));
#endif
    this->ui->view->setDragDropMode( QAbstractItemView::NoDragDrop );
    this->ui->view->setAttribute( Qt::WA_TransparentForMouseEvents, true );

    this->ui->view->setDragEnabled(true);
    this->ui->view->setAcceptDrops(true);
    this->ui->view->setDropIndicatorShown(true);


    // set up view delegate
    this->delegate = new FolderDelegate( this->ui->view );
    this->ui->view->setItemDelegate( this->delegate );

    // set title
    if ( dir.isRoot())
        this->ui->title->setText( QStorageInfo( dir ).displayName());
    else
        this->ui->title->setText( dir.dirName());

    // set default stylesheet
    styleSheet.setFileName( ":/stylesheets/stylesheet.qss" );
    if ( styleSheet.open( QFile::ReadOnly )) {
        this->defaultStyleSheet = styleSheet.readAll().constData();
        styleSheet.close();
    }

    // get window handles and set as child window
#ifdef Q_OS_WIN
    this->setupFrame( windowParent );
#else
    this->setupFrame();
    this->setWindowFlags( this->windowFlags() | Qt::WindowStaysOnBottomHint );
#endif
}

/**
 * @brief FolderView::~FolderView
 */
FolderView::~FolderView() {
#ifdef PROXY_MODEL
    delete this->proxyModel;
#endif
    delete this->delegate;
    delete this->model;
    delete this->ui;
}

/**
 * @brief FolderView::currentStyleSheet
 * @return
 */
QString FolderView::currentStyleSheet() const {
    if ( this->customStyleSheet().isEmpty())
        return this->styleSheet();

    return this->customStyleSheet();
}

/**
 * @brief FolderView::iconSize
 * @return
 */
int FolderView::iconSize() const {
    int size;

    size = this->ui->view->iconSize().width();
    if ( !size )
        return Ui::DefaultIconSize;

    return size;
}

/**
 * @brief FolderView::displayContextMenu
 * @param point
 */
void FolderView::displayContextMenu( const QPoint &point ) {
    QMenu menu, *appearanceMenu, *styleMenu;
    QAction *actionListMode, *actionReadOnly;

    menu.addAction( IconCache::instance()->icon( "inode-directory", 16, IconIndex::instance()->defaultTheme()), this->tr( "Change directory" ), this, SLOT( changeDirectory()));
    menu.addAction( IconCache::instance()->icon( "edit-rename", 16, IconIndex::instance()->defaultTheme()), this->tr( "Rename view" ), this, SLOT( renameView()));
    menu.addAction( IconCache::instance()->icon( "view-close", 16, IconIndex::instance()->defaultTheme()), this->tr( "Hide" ), this, SLOT( hide()));
    menu.addSeparator();
    appearanceMenu = menu.addMenu( IconCache::instance()->icon( "color-picker", 16, IconIndex::instance()->defaultTheme()), this->tr( "Appearance" ));
    styleMenu = appearanceMenu->addMenu( this->tr( "Style" ));
    styleMenu->addAction( IconCache::instance()->icon( "document-edit", 16, IconIndex::instance()->defaultTheme()), this->tr( "Custom stylesheet" ), this, SLOT( editStylesheet()));
    appearanceMenu->addAction( IconCache::instance()->icon( "transform-scale", 16, IconIndex::instance()->defaultTheme()), this->tr( "Set icon size" ), this, SLOT( setIconSize()));
    actionListMode = appearanceMenu->addAction( IconCache::instance()->icon( "view-list-details", 16, IconIndex::instance()->defaultTheme()), this->tr( "List mode" ), this, SLOT( toggleViewMode()));
    actionListMode->setCheckable( true );

    if ( this->viewMode() == QListView::ListMode )
        actionListMode->setChecked( true );
    else
        actionListMode->setChecked( false );

    menu.addSeparator();
    actionReadOnly = menu.addAction( this->tr( "Read only" ), this, SLOT( toggleAccessMode()));
    actionReadOnly->setCheckable( true );
    actionReadOnly->setDisabled( true );

    if ( this->model->isReadOnly())
        actionReadOnly->setChecked( true );
    else
        actionReadOnly->setChecked( false );

    menu.exec( this->mapToGlobal( point ));
}

/**
 * @brief FolderView::setCustomTitle
 * @param title
 */
void FolderView::setCustomTitle( const QString &title ) {
    if ( title.isEmpty())
        return;

    this->m_customTitle = title;
    this->ui->title->setText( this->customTitle());
}

/**
 * @brief FolderView::setCustomStyleSheet
 * @param stylesheet
 */
void FolderView::setCustomStyleSheet( const QString &stylesheet ) {
    this->m_customStyleSheet = stylesheet;

    if ( stylesheet.isEmpty())
        this->setDefaultStyleSheet();
    else {
        this->setStyleSheet( stylesheet );

        // this has to be done to properly reset view
        this->delegate->clearCache();
        this->ui->view->setItemDelegate( nullptr );
        this->ui->view->setItemDelegate( this->delegate );
    }
}

/**
 * @brief FolderView::setDefaultStyleSheet
 */
void FolderView::setDefaultStyleSheet() {
    this->setStyleSheet( this->defaultStyleSheet );

    // this has to be done to properly reset view
    this->delegate->clearCache();
    this->ui->view->setItemDelegate( nullptr );
    this->ui->view->setItemDelegate( this->delegate );
}

/**
 * @brief FolderView::setIconSize
 */
void FolderView::setIconSize() {
    int size;
    bool ok;

    size = QInputDialog::getInt( this->parentWidget(), this->tr( "Set icon size" ), this->tr( "Size:" ), this->iconSize(), 0, 256, 16, &ok );
    if ( ok ) {
        this->setIconSize( size );

        // this has to be done to properly reset view
        this->delegate->clearCache();
        this->ui->view->setItemDelegate( nullptr );
        this->ui->view->setItemDelegate( this->delegate );

#ifdef PROXY_MODEL
        this->proxyModel->clearIconCache();
#endif
    }
}

/**
 * @brief FolderView::changeDirectory
 */
void FolderView::changeDirectory() {
    QDir dir;

    dir.setPath( QFileDialog::getExistingDirectory( this->parentWidget(), this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));

    if ( dir.exists()) {
        this->model->setRootPath( dir.absolutePath());
#ifdef PROXY_MODEL
        this->ui->view->setRootIndex( this->proxyModel->mapFromSource( this->model->index( dir.absolutePath())));
#else
        this->ui->view->setRootIndex( this->model->index( dir.absolutePath()));
#endif

        // this has to be done to properly reset view
        this->delegate->clearCache();
        this->ui->view->setItemDelegate( nullptr );
        this->ui->view->setItemDelegate( this->delegate );
    }
}

/**
 * @brief FolderView::renameView
 */
void FolderView::renameView() {
    QString title;
    bool ok;

    title = QInputDialog::getText( this->parentWidget(), this->tr( "Rename view" ), this->tr( "Title:" ), QLineEdit::Normal, this->model->rootDirectory().dirName(), &ok );
    if ( ok && !title.isEmpty())
        this->setCustomTitle( title );
}

/**
 * @brief FolderView::editStylesheet
 */
void FolderView::editStylesheet() {
    StyleSheetDialog dialog( this, this->currentStyleSheet());
    int result;

    result = dialog.exec();
    if ( result == QDialog::Accepted )
        this->setCustomStyleSheet( dialog.customStyleSheet());
}

/**
 * @brief FolderView::toggleViewMode
 */
void FolderView::toggleViewMode() {
    if ( this->viewMode() == QListView::IconMode )
        this->setViewMode( QListView::ListMode );
    else
        this->setViewMode( QListView::IconMode );
}

/**
 * @brief FolderView::toggleAccessMode
 */
void FolderView::toggleAccessMode() {
    this->setReadOnly( !this->isReadOnly());
}

/**
 * @brief FolderView::setReadOnly
 */
void FolderView::setReadOnly( bool enable ) {
    this->model->setReadOnly( enable );

    if ( enable )
        this->ui->view->setDragDropMode( QListView::DropOnly );
    else
        this->ui->view->setDragDropMode( QListView::NoDragDrop );

}

/**
 * @brief FolderView::isReadOnly
 * @return
 */
bool FolderView::isReadOnly() const {
    return this->model->isReadOnly();
}

/**
 * @brief FolderView::paintEvent
 * @param event
 */
void FolderView::paintEvent( QPaintEvent *event ) {
    QPainter painter( this );

    // clear background for transparency
    painter.setCompositionMode( QPainter::CompositionMode_Clear );
    painter.fillRect( this->rect(), QColor( 0, 0, 0, 0 ));
    painter.setCompositionMode( QPainter::CompositionMode_SourceOver );

    // grab area debugging
#if 0
    int y;
    for ( y = 0; y < Frame::MouseGrabAreas; y++ ) {
        painter.fillRect( this->grabAreas[y], QBrush( QColor::fromRgb( 255, 0, 0, 255 )));
    }
#endif

    // paint as usual
    QWidget::paintEvent( event );
}

/**
 * @brief FolderView::makeGrabAreas
 */
void FolderView::makeGrabAreas() {
    this->grabAreas[TopLeft] = QRect( 0, 0, Frame::BorderWidth, Frame::BorderWidth );
    this->grabAreas[Top] = QRect( Frame::BorderWidth, 0, this->width() - Frame::BorderWidth * 2, Frame::BorderWidth );
    this->grabAreas[TopRight] = QRect( this->width() - Frame::BorderWidth, 0, Frame::BorderWidth, Frame::BorderWidth );
    this->grabAreas[Right] = QRect( this->width() - Frame::BorderWidth, Frame::BorderWidth, Frame::BorderWidth, this->height() - Frame::BorderWidth * 2 );
    this->grabAreas[BottomRight] = QRect( this->width() - Frame::BorderWidth, this->height() - Frame::BorderWidth, Frame::BorderWidth, Frame::BorderWidth );
    this->grabAreas[Bottom] = QRect( Frame::BorderWidth, this->height() - Frame::BorderWidth, this->width() - Frame::BorderWidth * 2, Frame::BorderWidth );
    this->grabAreas[BottomLeft] = QRect( 0, this->height() - Frame::BorderWidth, Frame::BorderWidth, Frame::BorderWidth );
    this->grabAreas[Left] = QRect( 0, Frame::BorderWidth, Frame::BorderWidth, this->height() - Frame::BorderWidth * 2 );
}

/**
 * @brief FolderView::eventFilter
 * @param object
 * @param event
 * @return
 */
bool FolderView::eventFilter( QObject *object, QEvent *event ) {
    QMouseEvent *mouseEvent;
    int y;

    // filter mouse events
    if ( event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
         event->type() == QEvent::MouseMove || event->type() == QEvent::HoverMove ||
         event->type() == QEvent::Leave || event->type() == QEvent::Resize ||
         event->type() == QEvent::Move || event->type() == QEvent::Enter ) {

        // get mouse event
        mouseEvent = static_cast<QMouseEvent*>( event );

        // test mouse origin if needed
        if ( this->gesture == NoGesture && !this->isMaximized()) {
            this->currentGrabArea = NoArea;
            for ( y = 0; y < Frame::MouseGrabAreas; y++ ) {
                if ( this->grabAreas[y].contains( mouseEvent->pos())) {
                    this->currentGrabArea = static_cast<Areas>( y );
                    break;
                }
            }
        }

        // ignore listView mouse events when cursor is on the edges
        if ( this->currentGrabArea == NoArea )
            this->ui->view->setAttribute( Qt::WA_TransparentForMouseEvents, false );
        else
            this->ui->view->setAttribute( Qt::WA_TransparentForMouseEvents, true );

        // change cursor shape if needed
        if ( this->gesture == NoGesture ) {
            Qt::CursorShape shape = Qt::ArrowCursor;

            if ( this->currentGrabArea == Top || this->currentGrabArea == Bottom )
                shape = Qt::SizeVerCursor;
            else if ( this->currentGrabArea == Left || this->currentGrabArea == Right )
                shape = Qt::SizeHorCursor;
            else if ( this->currentGrabArea == TopLeft || this->currentGrabArea == BottomRight )
                shape = Qt::SizeFDiagCursor;
            else if ( this->currentGrabArea == TopRight || this->currentGrabArea == BottomLeft )
                shape = Qt::SizeBDiagCursor;
            else if ( this->currentGrabArea == NoArea )
                shape = Qt::ArrowCursor;

            if ( this->cursor().shape() != shape )
                this->setCursor( QCursor( shape ));
        }

        // handle events
        switch ( event->type()) {
        case QEvent::MouseButtonPress:
            if ( mouseEvent->button() == Qt::LeftButton ) {
                // store mouse position
                this->mousePos = mouseEvent->globalPos();

                // no drag when maximized
                if ( this->isMaximized())
                    return true;

                // determine gesture
                if ( this->currentGrabArea == NoArea )
                    this->gesture = Drag;
                else
                    this->gesture = Resize;

                return true;
            }
            break;

        case QEvent::MouseButtonRelease:
            this->gesture = NoGesture;

            if ( mouseEvent->button() == Qt::RightButton )
                this->displayContextMenu( mouseEvent->pos());

            break;

        case QEvent::Enter:
        {
#ifdef Q_OS_WIN
            // do winapi magic
            DWORD curentThread, foregroundThread;

            curentThread = GetCurrentThreadId();
            foregroundThread = GetWindowThreadProcessId( GetForegroundWindow(), NULL );

            // steal input thread form foreground
            AttachThreadInput( foregroundThread, curentThread, TRUE );
            SetForegroundWindow(( HWND )this->winId());
            AttachThreadInput( foregroundThread, curentThread, FALSE );
#else
            // TODO: steal focus in X11 and other environments
#endif
        }
            break;

        case QEvent::Leave:
            this->gesture = NoGesture;
            break;

        case QEvent::MouseMove:
            if ( this->gesture == Drag ) {
                QPoint offset, newPos, screenOffset;

                //
                // NOTE: code refactored for use in mutiple monitor systems
                //

                // get offset from previous mouse position
                offset = mouseEvent->globalPos() - this->mousePos;

                // get screen offset
                screenOffset = QApplication::primaryScreen()->availableVirtualGeometry().topLeft();

                // get new position offsetting it by screen and mouse offsets
                newPos = this->pos() - screenOffset + offset;

                // use winapi to move the window (avoiding buggy Qt setGeometry)
#ifdef Q_OS_WIN
                MoveWindow(( HWND )this->winId(), newPos.x(), newPos.y(), this->width(), this->height(), true );
#else
                this->move( newPos.x(), newPos.y());
#endif
                // update last mouse position
                this->mousePos = mouseEvent->globalPos();

                return true;
            } else if ( this->gesture == Resize ) {
                QRect updatedGeometry;
                QPoint screenOffset, updatedMouse;

                //
                // NOTE: code refactored for use in mutiple monitor systems
                //

                // get screen offset
                screenOffset = QApplication::primaryScreen()->availableVirtualGeometry().topLeft();

                // offset geometry and mouse position
                updatedGeometry = this->geometry();
                updatedGeometry.translate( -screenOffset );
                updatedMouse = this->mousePos - screenOffset;

                // determine grab point
                switch ( this->currentGrabArea ) {
                case TopLeft:
                    updatedGeometry.setTopLeft( updatedMouse );
                    break;

                case Top:
                    updatedGeometry.setTop( updatedMouse.y());
                    break;

                case TopRight:
                    updatedGeometry.setTopRight( updatedMouse );
                    break;

                case Right:
                    updatedGeometry.setRight( updatedMouse.x());
                    break;

                case BottomRight:
                    updatedGeometry.setBottomRight( updatedMouse );
                    break;

                case Bottom:
                    updatedGeometry.setBottom( updatedMouse.y());
                    break;

                case BottomLeft:
                    updatedGeometry.setBottomLeft( updatedMouse );
                    break;

                case Left:
                    updatedGeometry.setLeft( updatedMouse.x());
                    break;

                default:
                    break;
                }

                // respect minimum width
                if ( updatedGeometry.width() < this->minimumWidth())
                    updatedGeometry.setLeft( this->geometry().x() - screenOffset.x());

                // respect minimum height
                if ( updatedGeometry.height() < this->minimumHeight())
                    updatedGeometry.setTop( this->geometry().y() - screenOffset.y());

                // use winapi to resize the window (avoiding buggy Qt setGeometry)
#ifdef Q_OS_WIN
                MoveWindow(( HWND )this->winId(), updatedGeometry.x(), updatedGeometry.y(), updatedGeometry.width(), updatedGeometry.height(), true );
#else
                this->setGeometry( updatedGeometry );
#endif

                // update last mouse position
                this->mousePos = mouseEvent->globalPos();

                // return success
                return true;
            }
            break;

        case QEvent::Resize:
            this->makeGrabAreas();
            break;

        case QEvent::Move:
            break;

        default:
            break;
        }
    }

    // other events are handled normally
    return QWidget::eventFilter( object, event );
}

/**
 * @brief FolderView::setupFrame
 */
#ifdef Q_OS_WIN
void FolderView::setupFrame( HWND windowParent ) {
#else
void FolderView::setupFrame() {
#endif
    // enable mouse tracking
    this->setMouseTracking( true );

    // filter events
    this->installEventFilter( this );

    // make this a child window of desktop shell
#ifdef Q_OS_WIN
    SetParent(( HWND )this->winId(), windowParent );
#endif

    // set appropriate window flags
    this->setWindowFlags( this->windowFlags() | Qt::FramelessWindowHint );
    this->setAttribute( Qt::WA_TranslucentBackground );
    this->setAttribute( Qt::WA_NoSystemBackground );
    this->setAttribute( Qt::WA_Hover );

    // make mouse grab areas
    this->makeGrabAreas();
}

/**
 * @brief FolderView::on_view_clicked
 * @param index
 */
void FolderView::on_view_clicked( const QModelIndex &index ) {
    if ( !index.isValid())
        return;

#ifdef PROXY_MODEL
    QDesktopServices::openUrl( QUrl::fromLocalFile( this->model->data( this->proxyModel->mapToSource( index ), QFileSystemModel::FilePathRole ).toString()));
#else
    QDesktopServices::openUrl( QUrl::fromLocalFile( this->model->data( index, QFileSystemModel::FilePathRole ).toString()));
#endif
    this->ui->view->clearSelection();
}

/**
 * @brief FolderView::openShellContextMenuForObject
 * @param path
 * @param pos
 * @param parentWindow
 * @return
 */
#ifdef Q_OS_WIN
void FolderView::openShellContextMenuForObject( const std::wstring &path, QPoint pos, HWND parentWindow ) {
    ITEMIDLIST *itemIdList;
    IShellFolder *shellFolder;
    LPCITEMIDLIST idChild;
    HRESULT result;
    IContextMenu *contextMenu;
    HMENU popupMenu;

    result = SHParseDisplayName( path.c_str(), 0, &itemIdList, 0, 0 );
    if ( !SUCCEEDED( result ) || itemIdList == nullptr )
        return;

    result = SHBindToParent( itemIdList, IID_IShellFolder, reinterpret_cast<void**>( &shellFolder ), &idChild );
    if ( !SUCCEEDED( result ) || shellFolder == nullptr )
        return;

    if ( !SUCCEEDED( shellFolder->GetUIObjectOf( parentWindow, 1, reinterpret_cast<const ITEMIDLIST **>( &idChild ), IID_IContextMenu, 0, reinterpret_cast<void**>( &contextMenu ))))
        return;

    popupMenu = CreatePopupMenu();
    if ( popupMenu == nullptr )
        return;

    if ( SUCCEEDED( contextMenu->QueryContextMenu( popupMenu, 0, 1, 0x7FFF, CMF_NORMAL ))) {
        int command;

        command = TrackPopupMenuEx( popupMenu, TPM_RETURNCMD, pos.x(), pos.y(), parentWindow, NULL );
        if ( command > 0 ) {
            CMINVOKECOMMANDINFOEX info;

            memset( &info, 0, sizeof( CMINVOKECOMMANDINFOEX ));

            info.cbSize = sizeof( info );
            info.fMask = CMIC_MASK_UNICODE;
            info.hwnd = parentWindow;
            info.lpVerb  = MAKEINTRESOURCEA( command - 1 );
            info.lpVerbW = MAKEINTRESOURCEW( command - 1 );
            info.nShow = SW_SHOWNORMAL;

            contextMenu->InvokeCommand( reinterpret_cast<LPCMINVOKECOMMANDINFO>( &info ));
        }
    }
    DestroyMenu( popupMenu );
}
#endif

/**
 * @brief FolderView::on_view_customContextMenuRequested
 * @param pos
 */
void FolderView::on_view_customContextMenuRequested( const QPoint &pos ) {
    QModelIndex index;

    index = this->ui->view->indexAt( pos );
    if ( !index.isValid())
        return;

#ifdef Q_OS_WIN
    FolderView::openShellContextMenuForObject( reinterpret_cast<const wchar_t *>( QDir::toNativeSeparators( this->model->data( this->proxyModel->mapToSource( index ), QFileSystemModel::FilePathRole ).toString()).utf16()), QCursor::pos(), ( HWND )this->winId());
    this->ui->view->selectionModel()->clear();
#else
    // TODO: context menu in other environments
#endif
}

/**
 * @brief FolderView::makeContextMenu
 */
/*void FolderView::makeContextMenu()
{

}*/

/**
 * @brief FileSystemModel::flags
 * @param index
 * @return
 */
/*Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const {
    if ( !index.isValid())
        return Qt::NoItemFlags;

    return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled );
}*/
