/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#ifndef DSTYLE_H
#define DSTYLE_H

#include <dtkwidget_global.h>
#include <DPalette>

#include <QCommonStyle>

QT_BEGIN_NAMESPACE
class QTextLayout;
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE

class DViewItemAction;
class DStyle : public QCommonStyle
{
    Q_OBJECT

public:
    enum PrimitiveElement {
        PE_ItemBackground = QStyle::PE_CustomBase + 1,          //列表项的背景色
        PE_CustomBase = QStyle::PE_CustomBase + 0xf00000
    };

    enum PixelMetric {
        PM_FocusBorderWidth = QStyle::PM_CustomBase + 1,        //控件焦点状态的边框宽度
        PM_FocusBorderSpacing,                                  //控件内容和border之间的间隔
        PM_FrameRadius,                                         //控件的圆角大小
        PM_ShadowRadius,                                        //控件阴影效果的半径
        PM_ShadowHOffset,                                       //阴影在水平方向的偏移
        PM_ShadowVOffset,                                       //阴影在竖直方向的偏移
        PM_FrameMargins,                                        //控件的margins区域，控件内容 = 控件大小 - FrameMargins
        PM_CustomBase = QStyle::PM_CustomBase + 0xf00000
    };

    enum StyleState {
        SS_NormalState = 0x00000000,
        SS_HoverState = 0x00000001,
        SS_PressState = 0x00000002,
        SS_StateCustomBase = 0x000000f0,

        StyleState_Mask = 0x000000ff,
        SS_CheckedFlag = 0x00000100,
        SS_SelectedFlag = 0x00000200,
        SS_FocusFlag = 0x00000400,
        SS_FlagCustomBase = 0xf00000
    };
    Q_DECLARE_FLAGS(StateFlags, StyleState)

    static QColor adjustColor(const QColor &base,
                              qint8 hueFloat = 0, qint8 saturationFloat = 0, qint8 lightnessFloat = 0,
                              qint8 redFloat = 0, qint8 greenFloat = 0, qint8 blueFloat = 0, qint8 alphaFloat = 0);
    static QColor blendColor(const QColor &substrate, const QColor &superstratum);

    DStyle();

    inline void drawPrimitive(DStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = nullptr) const
    { proxy()->drawPrimitive(static_cast<QStyle::PrimitiveElement>(pe), opt, p, w); }
    inline int pixelMetric(DStyle::PixelMetric m, const QStyleOption *opt = nullptr, const QWidget *widget = nullptr) const
    { return proxy()->pixelMetric(static_cast<QStyle::PixelMetric>(m), opt, widget); }

    int pixelMetric(QStyle::PixelMetric m, const QStyleOption *opt, const QWidget *widget) const override;

    // 获取一个加工后的画笔
    QBrush generatedBrush(const QStyleOption *option, const QBrush &base,
                          QPalette::ColorGroup cg = QPalette::Normal,
                          QPalette::ColorRole role = QPalette::NoRole) const;
    QBrush generatedBrush(StyleState state, const QStyleOption *option, const QBrush &base,
                          QPalette::ColorGroup cg = QPalette::Normal,
                          QPalette::ColorRole role = QPalette::NoRole) const;
    virtual QBrush generatedBrush(StateFlags flags, const QBrush &base,
                                  QPalette::ColorGroup cg = QPalette::Normal,
                                  QPalette::ColorRole role = QPalette::NoRole,
                                  const QStyleOption *option = nullptr) const;

    QBrush generatedBrush(const QStyleOption *option, const QBrush &base,
                          DPalette::ColorGroup cg = DPalette::Normal,
                          DPalette::ColorType type = DPalette::ItemBackground) const;
    QBrush generatedBrush(StyleState state, const QStyleOption *option, const QBrush &base,
                          DPalette::ColorGroup cg = DPalette::Normal,
                          DPalette::ColorType type = DPalette::ItemBackground) const;
    virtual QBrush generatedBrush(StateFlags flags, const QBrush &base,
                                  DPalette::ColorGroup cg = DPalette::Normal,
                                  DPalette::ColorType role = DPalette::ItemBackground,
                                  const QStyleOption *option = nullptr) const;

    using QCommonStyle::drawPrimitive;
    using QCommonStyle::pixelMetric;

#if QT_CONFIG(itemviews)
    static QSizeF viewItemTextLayout(QTextLayout &textLayout, int lineWidth);
    static QSize viewItemSize(const QStyle *style, const QStyleOptionViewItem *option, int role);
    static void viewItemLayout(const QStyle *style, const QStyleOptionViewItem *opt, QRect *pixmapRect,
                               QRect *textRect, QRect *checkRect, bool sizehint);
    virtual void viewItemLayout(const QStyleOptionViewItem *opt, QRect *pixmapRect,
                                QRect *textRect, QRect *checkRect, bool sizehint) const;

    static QRect viewItemDrawText(const QStyle *style, QPainter *p, const QStyleOptionViewItem *option, const QRect &rect);
    virtual QRect viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const;
#endif
};

DWIDGET_END_NAMESPACE

#endif // DSTYLE_H
