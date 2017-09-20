/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2011 ~ 2017 Deepin, Inc.
 *               2011 ~ 2017 Wang Yong
 *
 * Author:     Wang Yong <wangyong@deepin.com>
 * Maintainer: Wang Yong <wangyong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */ 

#ifndef DSIMPLELISTVIEW_H
#define DSIMPLELISTVIEW_H

#include "dsimplelistitem.h"
#include <dtkwidget_global.h>
#include <QImage>
#include <QTimer>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE

typedef bool (* SortAlgorithm) (const DSimpleListItem *item1, const DSimpleListItem *item2, bool descendingSort);
typedef bool (* SearchAlgorithm) (const DSimpleListItem *item, QString searchContent);

class DSimpleListView : public QWidget
{
    Q_OBJECT
    
public:
    DSimpleListView(QWidget *parent = 0);
    ~DSimpleListView();

    // DSimpleListView interfaces.
    
    /* 
     * Set row height of DSimpleListView.
     * 
     * @height the height of row
     */
    void setRowHeight(int height);

    /*
     * Set column titles, widths and height.
     * If you want some column use expand space, please set width with -1
     * Only allowed one -1 to set in width list.
     * 
     * @titles a list to contains titles
     * @widths the width of column, list length same as titles list
     * @height height of titlebar, don't display titlebar if set height with 0
     */
    void setColumnTitleInfo(QList<QString> titles, QList<int> widths, int height);
    
    /*
     * Set column hide flags.
     * At least have false in hide flags list, and hide flags count must same as titles list.
     *
     * @toggleHideFlags the hide flags to control column wether toggle show/hide.
     * @alwaysVisibleColumn the column index that column is always visible, default is -1, mean no column can always visible.
     */
    void setColumnHideFlags(QList<bool> toggleHideFlags, int alwaysVisibleColumn=-1);
    
    /*
     * Set column sorting algorithms.
     * Note SortAlgorithm function type must be 'static', otherwise function pointer can't match type.
     * 
     * @algorithms a list of SortAlgorithm, SortAlgorithm is function pointer, it's type is: 'bool (*) (const DSimpleListItem *item1, const DSimpleListItem *item2, bool descendingSort)'
     * @sortColumn default sort column, -1 mean don't sort any column default
     * @descendingSort whether sort column descending, default is false
     */
    void setColumnSortingAlgorithms(QList<SortAlgorithm> *algorithms, int sortColumn=-1, bool descendingSort=false);
    
    /*
     * Set search algorithm to filter match items.
     * 
     * @algorithm the search algorithm, it's type is: 'bool (*) (const DSimpleListItem *item, QString searchContent)'
     */
    void setSearchAlgorithm(SearchAlgorithm algorithm);
    
    /*
     * Set radius to clip listview.
     * 
     * @radius the radius of clip area, default is 8 pixel.
     */
    void setClipRadius(int radius);
    
    /* 
     * Add DSimpleListItem list to ListView.
     * If user has click title to sort, sort items after add items to list. 
     * 
     * @items List of LiteItem*
     */
    void addItems(QList<DSimpleListItem*> items);
    
    /* 
     * Clear items from DSimpleListView.
     */
    void clearItems();
    
    /*
     * Add DSimpleListItem list to mark selected effect in ListView.
     * 
     * @items List of DSimpleListItem* to mark selected
     * @recordLastSelection record last selection item to make selected operation continuously, default is true
     */
    void addSelections(QList<DSimpleListItem*> items, bool recordLastSelection=true);
    
    /*
     * Clear selection items from DSimpleListView.
     * 
     * @clearLastSelection clear last selection item if option is true, default is true
     */
    void clearSelections(bool clearLastSelection=true);
    
    /*
     * Refresh all items in DSimpleListView.
     * This function is different that addItems is: it will clear items first before add new items.
     * This function will keep selection status and scroll offset when add items.
     * 
     * @items List of DSimpleListItem* to add
     */
    void refreshItems(QList<DSimpleListItem*> items);
    
    /*
     * Search
     */
    void search(QString searchContent);
    
    // DSimpleListView operations.
    void selectAllItems();
    void selectFirstItem();
    void selectLastItem();
    void selectNextItem();
    void selectPrevItem();
    
    void shiftSelectPageDown();
    void shiftSelectPageUp();
    void shiftSelectToEnd();
    void shiftSelectToHome();
    void shiftSelectToNext();
    void shiftSelectToPrev();
    
    void scrollPageDown();
    void scrollPageUp();
    
    void ctrlScrollPageDown();
    void ctrlScrollPageUp();
    void ctrlScrollToEnd();
    void ctrlScrollToHome();
    
protected:
    virtual void leaveEvent(QEvent * event);
    
    QImage arrowDownDarkHoverImage;
    QImage arrowDownDarkNormalImage;
    QImage arrowDownDarkPressImage;
    QImage arrowDownHoverImage;
    QImage arrowDownLightHoverImage;
    QImage arrowDownLightNormalImage;
    QImage arrowDownLightPressImage;
    QImage arrowDownNormalImage;
    QImage arrowDownPressImage;
    QImage arrowUpDarkHoverImage;
    QImage arrowUpDarkNormalImage;
    QImage arrowUpDarkPressImage;
    QImage arrowUpHoverImage;
    QImage arrowUpLightHoverImage;
    QImage arrowUpLightNormalImage;
    QImage arrowUpLightPressImage;
    QImage arrowUpNormalImage;
    QImage arrowUpPressImage;
    QString backgroundColor = "#ffffff";
    QString frameColor = "#000000";
    QString scrollbarColor = "#ffffff";
    QString searchColor = "#000000";
    QString titleAreaColor = "#ffffff";
    QString titleColor = "#000000";
    QString titleLineColor = "#000000";
    double backgroundOpacity = 0.03;
    double frameOpacity = 0.1;
    double titleAreaOpacity = 0.02;
    int titleSize = 10;
    qreal scrollbarFrameHoverOpacity = 0;
    qreal scrollbarFrameNormalOpacity = 0;
    qreal scrollbarFramePressOpacity = 0;
    qreal scrollbarHoverOpacity = 0.7;
    qreal scrollbarNormalOpacity = 0.5;
    qreal scrollbarPressOpacity = 0.8;
                           
signals:
    void rightClickItems(QPoint pos, QList<DSimpleListItem*> items);
    void changeColumnVisible(int index, bool visible, QList<bool> columnVisibles);
    void changeSortingStatus(int index, bool sortingOrder);
    void changeHoverItem(QPoint pos, DSimpleListItem* item, int columnIndex);
    
private slots:
    void hideScrollbar();
    
private:
    bool eventFilter(QObject *, QEvent *event);
    void keyPressEvent(QKeyEvent *keyEvent);
    void mouseMoveEvent(QMouseEvent *mouseEvent);
    void mousePressEvent(QMouseEvent *mouseEvent);
    void mouseReleaseEvent(QMouseEvent *mouseEvent);
    void paintEvent(QPaintEvent *);
    void paintScrollbar(QPainter *painter);
    void selectNextItemWithOffset(int scrollOffset);
    void selectPrevItemWithOffset(int scrollOffset);
    void shiftSelectItemsWithBound(int selectionStartIndex, int selectionEndIndex);
    void shiftSelectNextItemWithOffset(int scrollOffset);
    void shiftSelectPrevItemWithOffset(int scrollOffset);
    void wheelEvent(QWheelEvent *event);
                        
    QList<DSimpleListItem*> getSearchItems(QList<DSimpleListItem*> items);
    QList<int> getRenderWidths();
    bool isMouseAtScrollArea(int x);
    bool isMouseAtTitleArea(int y);
    int adjustRenderOffset(int offset);
    int getBottomRenderOffset();
    int getItemsTotalHeight();
    int getScrollAreaHeight();
    int getScrollbarHeight();
    int getScrollbarY();
    int getTopRenderOffset();
    void sortItemsByColumn(int column, bool descendingSort);
    void startScrollbarHideTimer();
    
    DSimpleListItem *lastHoverItem;
    DSimpleListItem *lastSelectItem;
    QList<DSimpleListItem*> *listItems;
    QList<DSimpleListItem*> *renderItems;
    QList<DSimpleListItem*> *selectionItems;
    QList<QString> columnTitles;
    QList<SortAlgorithm> *sortingAlgorithms;
    QList<bool> *sortingOrderes;
    QList<bool> columnVisibles;
    QList<int> columnWidths;
    QString searchContent;
    QTimer *hideScrollbarTimer;
    SearchAlgorithm searchAlgorithm;
    bool defaultSortingOrder;
    bool mouseAtScrollArea;
    bool mouseDragScrollbar;
    int alwaysVisibleColumn;
    int clipRadius;
    int defaultSortingColumn;
    int hideScrollbarDuration;
    int lastHoverColumnIndex;
    int oldRenderOffset;
    int renderOffset;
    int rowHeight;
    int scrollDistance;
    int scrollStartY;
    int scrollUnit;
    int scrollbarDefaultWidth;
    int scrollbarDragWidth;
    int scrollbarMinHeight;
    int scrollbarPadding;
    int titleArrowPadding;
    int titleHeight;
    int titleHoverColumn;
    int titlePadding;
    int titlePressColumn;
};

DWIDGET_END_NAMESPACE

#endif
