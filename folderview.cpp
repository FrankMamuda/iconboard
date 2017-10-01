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
#include "proxymodel.h"
#include "ThemeEditor.h"
#include "iconcache.h"
#include "iconindex.h"
#include "themes.h"
#include "variable.h"
#ifdef Q_OS_WIN
#include <shlobj.h>
#endif

/**
 * @brief FolderView::FolderView
 * @param parent
 * @param rootPath
 */
FolderView::FolderView( QWidget *parent, const QString &rootPath,
                        TrayWidget *trayParent ) : QWidget( parent ), ui( new Ui::FolderView ), model( new FileSystemModel()), gesture( NoGesture ), currentGrabArea( NoArea ),
    trayWidget( trayParent ), m_sortOrder( Qt::AscendingOrder ),
    m_dirsFirst( true ), m_caseSensitive( false )
{
    QDir dir( rootPath );
    QFile styleSheet;

    // set up UI
    this->ui->setupUi( this );

    // set up listView and its model
    this->ui->view->setViewMode( QListView::IconMode );
    this->proxyModel = new ProxyModel( this );
    this->proxyModel->setSourceModel( this->model );
    this->ui->view->setModel( this->proxyModel );
    this->ui->view->setRootIndex( this->proxyModel->mapFromSource( this->model->setRootPath( rootPath )));
    this->ui->view->setDragDropMode( QAbstractItemView::NoDragDrop );
    this->ui->view->setAttribute( Qt::WA_TransparentForMouseEvents, true );

    // enable drag & drop
    this->ui->view->setDragEnabled( true );
    this->ui->view->setAcceptDrops( true );
    this->ui->view->setDropIndicatorShown( true );

    // set up view delegate
    this->delegate = new FolderDelegate( this->ui->view );
    this->ui->view->setItemDelegate( this->delegate );

    // set title
    this->ui->title->setAutoFillBackground( true );
    if ( dir.isRoot())
        this->ui->title->setText( QStorageInfo( dir ).displayName());
    else
        this->ui->title->setText( dir.dirName());

    // set default styleSheet
    styleSheet.setFileName( ":/styleSheets/dark.qss" );
    if ( styleSheet.open( QFile::ReadOnly )) {
        this->defaultStyleSheet = styleSheet.readAll().constData();
        styleSheet.close();
    }

    // remove frame
    this->setupFrame();

    // connect variable
    Variable::instance()->bind( "ui_displaySymlinkIcon", this, SLOT( displaySymlinkLabelsChanged()));
}

/**
 * @brief FolderView::displaySymlinkLabelsChanged
 */
void FolderView::displaySymlinkLabelsChanged() {
    this->proxyModel->clearCache();
    this->update();
}

/**
 * @brief FolderView::~FolderView
 */
FolderView::~FolderView() {
    //delete this->menu;
    this->proxyModel->waitForThreads();
    delete this->proxyModel;
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
    QMenu menu;
    QAction *actionReadOnly;

    // change directory lambda
    this->connect( menu.addAction( IconCache::instance()->icon( "inode-directory", 16 ), this->tr( "Change directory" )), &QAction::triggered, [this]() {
        QDir dir;

        dir.setPath( QFileDialog::getExistingDirectory( this->parentWidget(), this->tr( "Select directory" ), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks ));

        if ( dir.exists()) {
            this->proxyModel->waitForThreads();
            this->ui->view->setRootIndex( this->proxyModel->mapFromSource( this->model->setRootPath( dir.absolutePath())));
            this->ui->title->setText( dir.dirName());
            this->sort();
        }
    } );

    // rename view lambda
    this->connect( menu.addAction( IconCache::instance()->icon( "edit-rename", 16 ), this->tr( "Rename view" )), &QAction::triggered, [this]() {
        QString title;
        bool ok;

        title = QInputDialog::getText( this->parentWidget(), this->tr( "Rename view" ), this->tr( "Title:" ), QLineEdit::Normal, this->model->rootDirectory().dirName(), &ok );
        if ( ok && !title.isEmpty())
            this->setCustomTitle( title );
    } );

    // close view
    menu.addAction( IconCache::instance()->icon( "view-close", 16 ), this->tr( "Hide" ), this, SLOT( hide()));

    // add separator
    menu.addSeparator();

    //
    // begin APPEARANCE menu
    //
    {
        QMenu *appearanceMenu;
        QAction *actionListMode;

        // create appearance menu
        appearanceMenu = menu.addMenu( IconCache::instance()->icon( "color-picker", 16 ), this->tr( "Appearance" ));

        // icon size
        appearanceMenu->addAction( IconCache::instance()->icon( "transform-scale", 16 ), this->tr( "Set icon size" ), this, SLOT( setIconSize()));

        //
        // begin THEME menu
        //
        {
            QMenu *themeMenu;

            // add theme menu
            themeMenu = appearanceMenu->addMenu( this->tr( "Theme" ));

            // custom styleSheet lambda
            this->connect( themeMenu->addAction( IconCache::instance()->icon( "document-edit", 16 ), this->tr( "Custom stylesheet" )), &QAction::triggered, [this]() {
                ThemeEditor dialog( this, ThemeEditor::Custom, this->currentStyleSheet());
                int result;

                result = dialog.exec();
                if ( result == QDialog::Accepted )
                    this->setCustomStyleSheet( dialog.currentStyleSheet());
            } );

            // add builtin/predefined theme chooser
            foreach ( Theme *theme, Themes::instance()->list ) {
                // connect via lambda
                this->connect( themeMenu->addAction( theme->name()), &QAction::triggered, [this, theme]() {
                    this->setCustomStyleSheet( theme->styleSheet(), true );
                } );
            }
        }

        // end THEME menu

        // view mode lambda
        actionListMode = appearanceMenu->addAction( IconCache::instance()->icon( "view-list-details", 16 ), this->tr( "List mode" ));
        actionListMode->setCheckable( true );
        actionListMode->setChecked( this->viewMode() == QListView::ListMode );
        this->connect( actionListMode, &QAction::triggered, [this]() {
            this->setViewMode( this->viewMode() == QListView::IconMode ? QListView::ListMode : QListView::IconMode );
        } );
    }
    // end APPEARANCE menu

    // add separator
    menu.addSeparator();

    //
    // begin SORT menu
    //
    {
        QMenu *sortMenu;
        QAction *actionSortOrder, *actionDirsFirst, *actionCaseSensitive;

        // sort menu
        sortMenu = menu.addMenu( IconCache::instance()->icon( "format-list-ordered", 16 ), this->tr( "Sort" ));

        // sort order lambda
        actionSortOrder = sortMenu->addAction( IconCache::instance()->icon( "view-sort-ascending", 16 ), this->tr( "Ascending order" ));
        actionSortOrder->setCheckable( true );
        actionSortOrder->setChecked( this->sortOrder() == Qt::AscendingOrder );
        this->connect( actionSortOrder, &QAction::triggered, [this]() {
            this->proxyModel->waitForThreads();
            this->setSortOrder( this->sortOrder() == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder );
            this->sort();
        } );

        // directories first sort
        actionDirsFirst = sortMenu->addAction( this->tr( "Directories first" ));
        actionDirsFirst->setCheckable( true );
        actionDirsFirst->setChecked( this->directoriesFirst());
        this->connect( actionDirsFirst, &QAction::triggered, [this]() {
            this->proxyModel->waitForThreads();
            this->setDirectoriesFirst( !this->directoriesFirst());
            this->sort();
        } );

        // case sensitive sort
        actionCaseSensitive = sortMenu->addAction( this->tr( "Case sensitive" ));
        actionCaseSensitive->setCheckable( true );
        this->connect( actionCaseSensitive, &QAction::triggered, [this]() {
            this->proxyModel->waitForThreads();
            this->setCaseSensitive( !this->isCaseSensitive());
            this->sort();
        } );
    }

    // add separator
    menu.addSeparator();

    // read only lambda
    actionReadOnly = menu.addAction( this->tr( "Read only" ));
    actionReadOnly->setCheckable( true );
    actionReadOnly->setChecked( this->model->isReadOnly());
    actionReadOnly->setDisabled( true );
    this->connect( actionReadOnly, &QAction::triggered, [this]() {
        this->setReadOnly( !this->isReadOnly());
    } );

    // show menu
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
 * @param styleSheet
 */
void FolderView::setCustomStyleSheet( const QString &styleSheet, bool force ) {
    this->m_customStyleSheet = styleSheet;

    if ( styleSheet.isEmpty() && !force )
        this->setDefaultStyleSheet();
    else {
        this->setStyleSheet( styleSheet );

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

        this->proxyModel->clearCache();
    }
}

/**
 * @brief FolderView::setReadOnly
 */
void FolderView::setReadOnly( bool enable ) {
    this->model->setReadOnly( enable );
    this->ui->view->setDragDropMode( enable ? QListView::DropOnly : QListView::NoDragDrop );
}

/**
 * @brief FolderView::setSortOrder
 * @param order
 */
void FolderView::setSortOrder( Qt::SortOrder order ) {
    this->m_sortOrder = order;
}

/**
 * @brief FolderView::sort
 * @param order
 */
void FolderView::sort() {
    this->proxyModel->sort( 0 );

    // FIXME: for some reason we have to reset model
    this->ui->view->setModel( nullptr );
    this->ui->view->setModel( this->proxyModel );
    this->proxyModel->setSourceModel( this->model );
    this->ui->view->setRootIndex( this->proxyModel->mapFromSource( this->model->setRootPath( this->rootPath())));

    // this has to be done to properly reset view
    this->delegate->clearCache();
    this->ui->view->setItemDelegate( nullptr );
    this->ui->view->setItemDelegate( this->delegate );
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
                QPoint offset;

                // get offset from previous mouse position
                offset = mouseEvent->globalPos() - this->mousePos;
                this->move( this->x() + offset.x(), this->y() + offset.y());

                // update last mouse position
                this->mousePos = mouseEvent->globalPos();

                return true;
            } else if ( this->gesture == Resize ) {
                QRect updatedGeometry;
                QPoint screenOffset, updatedMouse;

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
                this->setGeometry( updatedGeometry );

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
void FolderView::setupFrame() {
    // enable mouse tracking
    this->setMouseTracking( true );

    // filter events
    this->installEventFilter( this );

    // set appropriate window flags
    this->setWindowFlags( this->windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnBottomHint | Qt::Tool | Qt::WindowDoesNotAcceptFocus );
    this->setAttribute( Qt::WA_TranslucentBackground );
    this->setAttribute( Qt::WA_NoSystemBackground );
    this->setAttribute( Qt::WA_Hover );
    this->setAttribute( Qt::WA_ShowWithoutActivating );
    this->setFocusPolicy( Qt::NoFocus );

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

    QDesktopServices::openUrl( QUrl::fromLocalFile( this->model->data( this->proxyModel->mapToSource( index ), QFileSystemModel::FilePathRole ).toString()));
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
 * @brief FileSystemModel::flags
 * @param index
 * @return
 */
/*Qt::ItemFlags FileSystemModel::flags(const QModelIndex &index) const {
    if ( !index.isValid())
        return Qt::NoItemFlags;

    return ( Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled );
}*/
