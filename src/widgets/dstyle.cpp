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
#include "dstyle.h"

#include <QStyleOption>
#include <QTextLayout>
#include <QTextLine>
#include <QPixmapCache>

#include <qmath.h>
#include <private/qfixed_p.h>
#include <private/qtextengine_p.h>

QT_BEGIN_NAMESPACE
//extern Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
extern Q_WIDGETS_EXPORT void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

DWIDGET_BEGIN_NAMESPACE

inline static int adjustColorValue(int base, qint8 increment, int max = 255)
{
    return increment > 0 ? (max - base) * increment / 100.0 + base
                         : base * (1 + increment / 100.0);
}

QColor DStyle::adjustColor(const QColor &base,
                           qint8 hueFloat, qint8 saturationFloat, qint8 lightnessFloat,
                           qint8 redFloat, qint8 greenFloat, qint8 blueFloat, qint8 alphaFloat)
{
    // 按HSL格式调整
    int H, S, L, A;
    base.getHsl(&H, &S, &L, &A);

    H = H > 0 ? adjustColorValue(H, hueFloat, 359) : H;
    S = adjustColorValue(S, saturationFloat);
    L = adjustColorValue(L, lightnessFloat);
    A = adjustColorValue(A, alphaFloat);

    QColor new_color = QColor::fromHsl(H, S, L, A);

    // 按RGB格式调整
    int R, G, B;
    new_color.getRgb(&R, &G, &B);

    R = adjustColorValue(R, redFloat);
    G = adjustColorValue(G, greenFloat);
    B = adjustColorValue(B, blueFloat);

    new_color.setRgb(R, G, B, A);

    return new_color;
}

QColor DStyle::blendColor(const QColor &substrate, const QColor &superstratum)
{
    QColor c2 = superstratum.toRgb();

    if (c2.alpha() >= 255)
        return c2;

    QColor c1 = substrate.toRgb();
    qreal c1_weight = 1 - c2.alphaF();

    int r = c1_weight * c1.red() + c2.alphaF() * c2.red();
    int g = c1_weight * c1.green() + c2.alphaF() * c2.green();
    int b = c1_weight * c1.blue() + c2.alphaF() * c2.blue();

    return QColor(r, g, b, c1.alpha());
}

namespace DDrawUtils {
static QImage dropShadow(const QPixmap &px, qreal radius, const QColor &color)
{
    if (px.isNull())
        return QImage();

    QImage tmp(px.size() + QSize(radius * 2, radius * 2), QImage::Format_ARGB32_Premultiplied);
    tmp.fill(0);
    QPainter tmpPainter(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
    tmpPainter.drawPixmap(QPoint(radius, radius), px);
    tmpPainter.end();

    // blur the alpha channel
    QImage blurred(tmp.size(), QImage::Format_ARGB32_Premultiplied);
    blurred.fill(0);
    QPainter blurPainter(&blurred);
    qt_blurImage(&blurPainter, tmp, radius, false, true);
    blurPainter.end();

    if (color == QColor(Qt::black))
        return blurred;

    tmp = blurred;

    // blacken the image...
    tmpPainter.begin(&tmp);
    tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tmpPainter.fillRect(tmp.rect(), color);
    tmpPainter.end();

    return tmp;
}

static QList<QRect> sudokuByRect(const QRect &rect, QMargins borders)
{
    QList<QRect> list;

//    qreal border_width = borders.left() + borders.right();

//    if ( border_width > rect.width()) {
//        borders.setLeft(borders.left() / border_width * rect.width());
//        borders.setRight(rect.width() - borders.left());
//    }

//    qreal border_height = borders.top() + borders.bottom();

//    if (border_height > rect.height()) {
//        borders.setTop(borders.top()/ border_height * rect.height());
//        borders.setBottom(rect.height() - borders.top());
//    }

    const QRect &contentsRect = rect - borders;

    list << QRect(0, 0, borders.left(), borders.top());
    list << QRect(list.at(0).topRight(), QSize(contentsRect.width(), borders.top())).translated(1, 0);
    list << QRect(list.at(1).topRight(), QSize(borders.right(), borders.top())).translated(1, 0);
    list << QRect(list.at(0).bottomLeft(), QSize(borders.left(), contentsRect.height())).translated(0, 1);
    list << contentsRect;
    list << QRect(contentsRect.topRight(), QSize(borders.right(), contentsRect.height())).translated(1, 0);
    list << QRect(list.at(3).bottomLeft(), QSize(borders.left(), borders.bottom())).translated(0, 1);
    list << QRect(contentsRect.bottomLeft(), QSize(contentsRect.width(), borders.bottom())).translated(0, 1);
    list << QRect(contentsRect.bottomRight(), QSize(borders.left(), borders.bottom())).translated(1, 1);

    return list;
}

static QImage borderImage(const QPixmap &px, const QMargins &borders, const QSize &size, QImage::Format format)
{
    QImage image(size, format);
    QPainter pa(&image);

    const QList<QRect> sudoku_src = sudokuByRect(px.rect(), borders);
    const QList<QRect> sudoku_tar = sudokuByRect(QRect(QPoint(0, 0), size), borders);

    pa.setCompositionMode(QPainter::CompositionMode_Source);  //设置组合模式

    for (int i = 0; i < 9; ++i) {
        pa.drawPixmap(sudoku_tar[i], px, sudoku_src[i]);
    }

    pa.end();

    return image;
}

void drawShadow(QPainter *pa, const QRect &rect, qreal xRadius, qreal yRadius, const QColor &sc, qreal radius, const QPoint &offset)
{
    QPixmap shadow;
    qreal scale = pa->paintEngine()->paintDevice()->devicePixelRatioF();
    QRect shadow_rect = rect;

    shadow_rect.setTopLeft(shadow_rect.topLeft() + offset);

    xRadius *= scale;
    yRadius *= scale;
    radius *= scale;

    const QString &key = QString("dtk-shadow-%1x%2-%3-%4").arg(xRadius).arg(yRadius).arg(sc.name()).arg(radius);

    if (!QPixmapCache::find(key, shadow)) {
        QImage shadow_base(QSize(xRadius * 3, yRadius * 3), QImage::Format_ARGB32_Premultiplied);
        shadow_base.fill(0);
        QPainter pa(&shadow_base);

        pa.setBrush(sc);
        pa.setPen(Qt::NoPen);
        pa.drawRoundedRect(shadow_base.rect(), xRadius, yRadius);
        pa.end();

        shadow_base = dropShadow(QPixmap::fromImage(shadow_base), radius, sc);
        shadow = QPixmap::fromImage(shadow_base);
        QPixmapCache::insert(key, shadow);
    }

    const QMargins margins(xRadius + radius, yRadius + radius, xRadius + radius, yRadius + radius);
    QImage new_shadow = borderImage(shadow, margins, shadow_rect.size() * scale, QImage::Format_ARGB32_Premultiplied);
//    QPainter pa_shadow(&new_shadow);
//    pa_shadow.setCompositionMode(QPainter::CompositionMode_Clear);
//    pa_shadow.setPen(Qt::NoPen);
//    pa_shadow.setBrush(Qt::transparent);
//    pa_shadow.setRenderHint(QPainter::Antialiasing);
//    pa_shadow.drawRoundedRect((new_shadow.rect() - QMargins(radius, radius, radius, radius)).translated(-offset), xRadius, yRadius);
//    pa_shadow.end();
    new_shadow.setDevicePixelRatio(scale);
    pa->drawImage(shadow_rect.topLeft(), new_shadow);
}

void drawShadow(QPainter *pa, const QRect &rect, const QPainterPath &path, const QColor &sc, int radius, const QPoint &offset)
{
    QPixmap shadow;
    qreal scale = pa->paintEngine()->paintDevice()->devicePixelRatioF();
    QRect shadow_rect = rect;

    shadow_rect.setTopLeft(rect.topLeft() + offset);
    radius *= scale;

    QImage shadow_base(shadow_rect.size() * scale, QImage::Format_ARGB32_Premultiplied);
    shadow_base.fill(0);
    shadow_base.setDevicePixelRatio(scale);

    QPainter paTmp(&shadow_base);
    paTmp.setBrush(sc);
    paTmp.setPen(Qt::NoPen);
    paTmp.drawPath(path);
    paTmp.end();
    shadow_base = dropShadow(QPixmap::fromImage(shadow_base), radius, sc);
    shadow = QPixmap::fromImage(shadow_base);
    shadow.setDevicePixelRatio(scale);

    pa->drawPixmap(shadow_rect, shadow);
}

void drawFork(QPainter *pa, const QRectF &rect, const QColor &color, int width)
{
    QPen pen;
    pen.setWidth(width);
    pen.setColor(color);

    pa->setRenderHint(QPainter::Antialiasing, true);
    pa->setPen(pen);
    pa->setBrush(Qt::NoBrush);
    pa->drawLine(rect.topLeft(), rect.bottomRight());
    pa->drawLine(rect.bottomLeft(), rect.topRight());
}

void drawRoundedRect(QPainter *pa, const QRect &rect, qreal xRadius, qreal yRadius, Corners corners, Qt::SizeMode mode)
{
    QRectF r = rect.normalized();

    if (r.isNull())
        return;

    if (mode == Qt::AbsoluteSize) {
        qreal w = r.width() / 2;
        qreal h = r.height() / 2;

        if (w == 0) {
            xRadius = 0;
        } else {
            xRadius = 100 * qMin(xRadius, w) / w;
        }
        if (h == 0) {
            yRadius = 0;
        } else {
            yRadius = 100 * qMin(yRadius, h) / h;
        }
    } else {
        if (xRadius > 100)                          // fix ranges
            xRadius = 100;

        if (yRadius > 100)
            yRadius = 100;
    }

    if (xRadius <= 0 || yRadius <= 0) {             // add normal rectangle
        pa->drawRect(r);
        return;
    }

    QPainterPath path;
    qreal x = r.x();
    qreal y = r.y();
    qreal w = r.width();
    qreal h = r.height();
    qreal rxx2 = w * xRadius / 100;
    qreal ryy2 = h * yRadius / 100;

    path.arcMoveTo(x, y, rxx2, ryy2, 180);

    if (corners & TopLeftCorner) {
        path.arcTo(x, y, rxx2, ryy2, 180, -90);
    } else {
        path.lineTo(r.topLeft());
    }

    if (corners & TopRightCorner) {
        path.arcTo(x + w - rxx2, y, rxx2, ryy2, 90, -90);
    } else {
        path.lineTo(r.topRight());
    }

    if (corners & BottomRightCorner) {
        path.arcTo(x + w - rxx2, y + h - ryy2, rxx2, ryy2, 0, -90);
    } else {
        path.lineTo(r.bottomRight());
    }

    if (corners & BottomLeftCorner) {
        path.arcTo(x, y + h - ryy2, rxx2, ryy2, 270, -90);
    } else {
        path.lineTo(r.bottomLeft());
    }

    path.closeSubpath();
    pa->drawPath(path);
}

void drawMark(QPainter *pa, const QRectF &rect, const QColor &boxInside, const QColor &boxOutside, const int penWidth, const int outLineLeng)
{
    QPen pen(boxInside);
    pen.setWidth(penWidth);
    pa->setPen(pen);
    pen.setJoinStyle(Qt::RoundJoin);
    pa->setRenderHint(QPainter::Antialiasing, true);

    pa->drawLine(rect.x(), rect.center().y(), rect.center().x(), rect.bottom());
    pa->drawLine(rect.center().x(), rect.bottom(), rect.right(), rect.y());

    if (outLineLeng == 0)
        return;

    double xWide = (rect.width() / 2.0);
    int yHigh = rect.height();
    double length = sqrt(pow(xWide, 2) + pow(yHigh, 2));
    double x = rect.right() + (outLineLeng / length) * xWide;
    double y = rect.y() - (outLineLeng / length) * yHigh;

    pen.setColor(boxOutside);
    pa->setPen(pen);
    pa->drawLine(QPointF(rect.topRight()), QPointF(x, y));
}

void drawBorder(QPainter *pa, const QRectF &rect, const QBrush &brush, int borderWidth, int radius)
{
    pa->setPen(QPen(brush, borderWidth, Qt::SolidLine));
    pa->setBrush(Qt::NoBrush);
    pa->setRenderHint(QPainter::Antialiasing);
    pa->drawRoundedRect(rect.adjusted(1, 1, -1, -1), radius, radius) ;
}

void drawArrow(QPainter *pa, const QRectF &rect, const QColor &color, Qt::ArrowType arrow, int width)
{
    QPen pen;
    pen.setWidth(width);
    pen.setColor(color);
    pa->setRenderHint(QPainter::Antialiasing, true);
    pa->setPen(pen);
    pa->setBrush(Qt::NoBrush);
    const QPointF center = rect.center();

    switch (arrow) {
    case Qt::UpArrow: {
        pa->drawLine(QPointF(center.x(), rect.y()), rect.bottomLeft());
        pa->drawLine(QPointF(center.x(), rect.y()), rect.bottomRight());
        break;
    }
    case Qt::LeftArrow: {
        pa->drawLine(QPointF(rect.x(), center.y()), rect.bottomRight());
        pa->drawLine(QPointF(rect.x(), center.y()), rect.topRight());
        break;
    }
    case Qt::DownArrow: {
        pa->drawLine(QPointF(center.x(), rect.bottom()), rect.topLeft());
        pa->drawLine(QPointF(center.x(), rect.bottom()), rect.topRight());
        break;
    }
    case Qt::RightArrow: {
        pa->drawLine(QPointF(rect.right(), center.y()), rect.topLeft());
        pa->drawLine(QPointF(rect.right(), center.y()), rect.bottomLeft());
        break;
    }
    default:
        break;
    }

}

void drawPlus(QPainter *painter, const QRectF &rect, const QColor &color, qreal width)
{
    qreal centerX = rect.center().x() ;
    qreal centerY = rect.center().y() ;
    QPen pen = color;
    pen.setWidthF(width);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QPointF(rect.x(), centerY), QPointF(rect.right(), centerY));
    painter->drawLine(QPointF(centerX, rect.y()), QPointF(centerX, rect.bottom()));
}

void drawSubtract(QPainter *painter, const QRectF &rect, const QColor &color, qreal width)
{
    qreal centerY = rect.center().y() ;
    QPen pen = color;
    pen.setWidthF(width);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(QPointF(rect.left(), centerY), QPointF(rect.right(), centerY));
}
}

DStyle::DStyle()
{

}

void DStyle::drawPrimitive(const QStyle *style, DStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w)
{
    DStyleHelper dstyle(style);

    switch (pe) {
    case PE_ItemBackground: {
        if (const DStyleOptionBackgroundGroup *vopt = qstyleoption_cast<const DStyleOptionBackgroundGroup *>(opt)) {
            QColor color = vopt->dpalette.color(DPalette::ItemBackground);

            if (dstyle.dstyle()) {
                color = dstyle.dstyle()->generatedBrush(vopt, color, vopt->dpalette.currentColorGroup(), DPalette::ItemBackground).color();
            }

            if (color.alpha() == 0) {
                return;
            }

            int frame_radius = dstyle.pixelMetric(PM_FrameRadius, opt, w);
            p->setBrush(color);
            p->setPen(Qt::NoPen);
            p->setRenderHint(QPainter::Antialiasing);

            if (vopt->directions != Qt::Horizontal && vopt->directions != Qt::Vertical) {
                p->drawRoundedRect(vopt->rect, frame_radius, frame_radius);
                break;
            }

            switch (vopt->position) {
            case DStyleOptionBackgroundGroup::OnlyOne:
                p->drawRoundedRect(vopt->rect, frame_radius, frame_radius);
                break;
            case DStyleOptionBackgroundGroup::Beginning: {
                if (vopt->directions == Qt::Horizontal) {
                    DDrawUtils::drawRoundedRect(p, vopt->rect, frame_radius, frame_radius,
                                                DDrawUtils::TopLeftCorner | DDrawUtils::BottomLeftCorner);
                } else {
                    DDrawUtils::drawRoundedRect(p, vopt->rect, frame_radius, frame_radius,
                                                DDrawUtils::TopLeftCorner | DDrawUtils::TopRightCorner);
                }

                break;
            }
            case DStyleOptionBackgroundGroup::End:
                if (vopt->directions == Qt::Horizontal) {
                    DDrawUtils::drawRoundedRect(p, vopt->rect, frame_radius, frame_radius,
                                                DDrawUtils::TopRightCorner | DDrawUtils::BottomRightCorner);
                } else {
                    DDrawUtils::drawRoundedRect(p, vopt->rect, frame_radius, frame_radius,
                                                DDrawUtils::BottomLeftCorner | DDrawUtils::BottomRightCorner);
                }

                break;
            case DStyleOptionBackgroundGroup::Middle:
                p->setRenderHint(QPainter::Antialiasing, false);
                p->drawRect(vopt->rect);
                break;
            default:
                break;
            }

            return;
        }
        break;
    }
    case PE_IconButtonPanel: {
        DStyleHelper dstyle(style);
        p->fillRect(opt->rect, dstyle.getColor(opt, QPalette::Background));
        break;
    }
    case PE_IconButtonIcon: {
        if (const DStyleOptionButton *btn = qstyleoption_cast<const DStyleOptionButton*>(opt)) {
            DStyleHelper dstyle(style);
            DStyleOptionIcon icon_option;

            icon_option.QStyleOption::operator =(*opt);
            icon_option.icon = btn->icon;
            icon_option.iconSize = btn->iconSize;
            icon_option.dpalette = btn->dpalette;

            dstyle.drawPrimitive(PE_Icon, &icon_option, p, w);
        }
        break;
    }
    case PE_Icon: {
        if (const DStyleOptionIcon *icon_opt = qstyleoption_cast<const DStyleOptionIcon*>(opt)) {
            QIcon::Mode mode = QIcon::Normal;
            QIcon::State state = QIcon::Off;

            if (!(opt->state & QStyle::State_Enabled)) {
                mode = QIcon::Disabled;
            } else if (opt->state & QStyle::State_Selected) {
                mode = QIcon::Selected;
            } else if (opt->state & QStyle::State_MouseOver) {
                mode = QIcon::Active;
            }

            if (opt->state & QStyle::State_On) {
                state = QIcon::On;
            }

            icon_opt->icon.actualSize(w->window()->windowHandle(), icon_opt->iconSize, mode, state);
            icon_opt->icon.paint(p, opt->rect, Qt::AlignCenter, mode, state);
        }
        break;
    }
    default:
        break;
    }
}

void DStyle::drawControl(const QStyle *style, DStyle::ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w)
{
    switch (ce) {
    case CE_FloatingButton: {
        if (const DStyleOptionButton *btn = qstyleoption_cast<const DStyleOptionButton *>(opt)) {
            DStyleHelper dstyle(style);
            dstyle.drawControl(CE_FloatingButtonBevel, opt, p, w);
            QStyleOptionButton subopt = *btn;
            subopt.rect = style->subElementRect(SE_PushButtonContents, opt, w);

            if (!(btn->features & QStyleOptionButton::Flat))
                subopt.palette.setBrush(QPalette::ButtonText, subopt.palette.highlightedText());

            style->drawControl(CE_PushButtonLabel, &subopt, p, w);

            if (btn->state.testFlag(QStyle::State_HasFocus)) {
                int border_width = dstyle.pixelMetric(PM_FocusBorderWidth, opt, w);
                QColor color = opt->palette.color(QPalette::Highlight);

                if (const DStyle *dstyle = qobject_cast<const DStyle*>(style)) {
                    color = dstyle->generatedBrush(opt, color, opt->palette.currentColorGroup(), QPalette::Highlight).color();
                }

                p->setPen(QPen(color, border_width, Qt::SolidLine));
                p->setBrush(Qt::NoBrush);
                p->setRenderHint(QPainter::Antialiasing);
                p->drawEllipse(QRectF(opt->rect).adjusted(1, 1, -1, -1));
            }

            return;
        }
        break;
    }
    case CE_FloatingButtonBevel: {
        DStyleHelper dstyle(style);
        int frame_margins = dstyle.pixelMetric(PM_FrameMargins, opt, w);
        const QMargins margins(frame_margins, frame_margins, frame_margins, frame_margins);
        QRect shadow_rect = opt->rect + margins;
        const QRect content_rect = opt->rect - margins;
        QColor color = opt->palette.highlight().color();

        if (const DStyle *dstyle = qobject_cast<const DStyle*>(style)) {
            color = dstyle->generatedBrush(opt, color, opt->palette.currentColorGroup(), QPalette::Highlight).color();
        }

        qreal frame_radius = content_rect.width() / 2.0;
        int shadow_radius = dstyle.pixelMetric(PM_ShadowRadius, opt, w);
        int shadow_xoffset = dstyle.pixelMetric(PM_ShadowHOffset, opt, w);
        int shadow_yoffset = dstyle.pixelMetric(PM_ShadowVOffset, opt, w);

        shadow_rect.setTopLeft(shadow_rect.topLeft() + QPoint(shadow_xoffset, shadow_yoffset));
        shadow_rect.setWidth(qMin(shadow_rect.width(), shadow_rect.height()));
        shadow_rect.setHeight(qMin(shadow_rect.width(), shadow_rect.height()));
        shadow_rect.moveCenter(opt->rect.center() + QPoint(shadow_xoffset / 2.0, shadow_yoffset / 2.0));

        DDrawUtils::drawShadow(p, shadow_rect, frame_radius, frame_radius,
                               DStyle::adjustColor(color, 0, 0, +30), shadow_radius, QPoint(0, 0));

        p->setPen(Qt::NoPen);
        p->setBrush(color);
        p->setRenderHint(QPainter::Antialiasing);
        p->drawEllipse(content_rect);
        break;
    }
    case CE_IconButton: {
        if (const DStyleOptionButton *btn = qstyleoption_cast<const DStyleOptionButton *>(opt)) {
            DStyleHelper dstyle(style);

            if (!(btn->features & DStyleOptionButton::Flat)) {
                dstyle.drawPrimitive(PE_IconButtonPanel, opt, p, w);
            }

            DStyleOptionButton new_opt = *btn;
            new_opt.rect = dstyle.subElementRect(SE_IconButtonIcon, opt, w);
            dstyle.drawPrimitive(PE_IconButtonIcon, opt, p, w);
        }
        break;
    }
    default:
        break;
    }
}

int DStyle::pixelMetric(const QStyle *style, DStyle::PixelMetric m, const QStyleOption *opt, const QWidget *widget)
{
    DStyleHelper dstyle(style);

    switch (m) {
    case PM_FocusBorderWidth: Q_FALLTHROUGH()
    case PM_FocusBorderSpacing:
        return 2;
    case PM_FrameRadius:
        return 8;
    case PM_ShadowRadius:
        return 6;
    case PM_ShadowHOffset:
        return 0;
    case PM_ShadowVOffset:
        return 2;
    case PM_FrameMargins: {
        int shadow_radius = dstyle.pixelMetric(PM_ShadowRadius, opt, widget);
        int shadow_xoffset = dstyle.pixelMetric(PM_ShadowHOffset, opt, widget);
        int shadow_yoffset = dstyle.pixelMetric(PM_ShadowVOffset, opt, widget);

        int border_width = pixelMetric(style, PM_FocusBorderWidth, opt, widget);
        int border_spacing = pixelMetric(style, PM_FocusBorderSpacing, opt, widget);

        int margins = qMax((shadow_radius + qMax(shadow_xoffset, shadow_yoffset)) / 2, border_width + border_spacing);

        return margins;
    }
    case PM_IconButtonIconSize:
        return 32;
    default:
        break;
    }

    return -1;
}

QRect DStyle::subElementRect(const QStyle *style, DStyle::SubElement r, const QStyleOption *opt, const QWidget *widget)
{
    Q_UNUSED(style)
    Q_UNUSED(widget)

    switch (r) {
    case SE_IconButtonIcon:
        return opt->rect;
    default:
        break;
    }

    return QRect();
}

QSize DStyle::sizeFromContents(const QStyle *style, DStyle::ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget)
{
    Q_UNUSED(style)
    Q_UNUSED(widget)

    switch (ct) {
    case CT_IconButton:
        if (const DStyleOptionButton *btn = qstyleoption_cast<const DStyleOptionButton*>(opt)) {
            return contentsSize.expandedTo(btn->iconSize);
        }
        Q_FALLTHROUGH()
    default:
        break;
    }

    return contentsSize;
}

QIcon DStyle::standardIcon(const QStyle *style, DStyle::StandardPixmap st, const QStyleOption *opt, const QWidget *widget)
{
    Q_UNUSED(style)
    Q_UNUSED(st)
    Q_UNUSED(opt)
    Q_UNUSED(widget);

    return QIcon();
}

static DStyle::StyleState getState(const QStyleOption *option)
{
    DStyle::StyleState state = DStyle::SS_NormalState;

    if (option->state.testFlag(DStyle::State_Sunken)) {
        state = DStyle::SS_PressState;
    } else if (option->state.testFlag(DStyle::State_MouseOver)) {
        state = DStyle::SS_HoverState;
    }

    return state;
}

static DStyle::StateFlags getFlags(const QStyleOption *option)
{
    DStyle::StateFlags flags = 0;

    if (option->state.testFlag(DStyle::State_On)) {
        flags |= DStyle::SS_CheckedFlag;
    }

    if (option->state.testFlag(DStyle::State_Selected)) {
        flags |= DStyle::SS_SelectedFlag;
    }

    if (option->state.testFlag(DStyle::State_HasFocus)) {
        flags |= DStyle::SS_FocusFlag;
    }

    return flags;
}

void DStyle::drawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    if (Q_UNLIKELY(pe < QStyle::PE_CustomBase)) {
        return QCommonStyle::drawPrimitive(pe, opt, p, w);
    }

    drawPrimitive(this, static_cast<PrimitiveElement>(pe), opt, p, w);
}

void DStyle::drawControl(QStyle::ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *w) const
{
    if (Q_UNLIKELY(ce < QStyle::CE_CustomBase)) {
        return QCommonStyle::drawControl(ce, opt, p, w);
    }

    drawControl(this, static_cast<ControlElement>(ce), opt, p, w);
}

int DStyle::pixelMetric(QStyle::PixelMetric m, const QStyleOption *opt, const QWidget *widget) const
{
    if (Q_UNLIKELY(m < QStyle::PM_CustomBase)) {
        return QCommonStyle::pixelMetric(m, opt, widget);
    }

    return pixelMetric(this, static_cast<PixelMetric>(m), opt, widget);
}

QRect DStyle::subElementRect(QStyle::SubElement r, const QStyleOption *opt, const QWidget *widget) const
{
    if (Q_UNLIKELY(r < QStyle::SE_CustomBase)) {
        return QCommonStyle::subElementRect(r, opt, widget);
    }

    return subElementRect(this, static_cast<DStyle::SubElement>(r), opt, widget);
}

QSize DStyle::sizeFromContents(QStyle::ContentsType ct, const QStyleOption *opt, const QSize &contentsSize, const QWidget *widget) const
{
    if (Q_UNLIKELY(ct < QStyle::CT_CustomBase)) {
        return QCommonStyle::sizeFromContents(ct, opt, contentsSize, widget);
    }

    return sizeFromContents(this, static_cast<DStyle::ContentsType>(ct), opt, contentsSize, widget);
}

QIcon DStyle::standardIcon(QStyle::StandardPixmap st, const QStyleOption *opt, const QWidget *widget) const
{
    if (Q_UNLIKELY(st < QStyle::SP_CustomBase)) {
        return QCommonStyle::standardIcon(st, opt, widget);
    }

    return standardIcon(this, static_cast<DStyle::StandardPixmap>(st), opt, widget);
}

QBrush DStyle::generatedBrush(const QStyleOption *option, const QBrush &base, QPalette::ColorGroup cg, QPalette::ColorRole role) const
{
    return generatedBrush(getState(option), option, base, cg, role);
}

QBrush DStyle::generatedBrush(DStyle::StyleState state, const QStyleOption *option, const QBrush &base, QPalette::ColorGroup cg, QPalette::ColorRole role) const
{
    if (auto proxy = qobject_cast<const DStyle*>(this->proxy())) {
        return proxy->generatedBrush(getFlags(option) | state, base, cg, role, option);
    }

    return generatedBrush(getFlags(option) | state, base, cg, role, option);
}

QBrush DStyle::generatedBrush(StateFlags flags, const QBrush &base, QPalette::ColorGroup cg, QPalette::ColorRole role, const QStyleOption *option) const
{
    Q_UNUSED(flags)
    Q_UNUSED(base)
    Q_UNUSED(cg)
    Q_UNUSED(role)
    Q_UNUSED(option)

    return base;
}

QBrush DStyle::generatedBrush(const QStyleOption *option, const QBrush &base, QPalette::ColorGroup cg, DPalette::ColorType type) const
{
    return generatedBrush(getState(option), option, base, cg, type);
}

QBrush DStyle::generatedBrush(DStyle::StyleState state, const QStyleOption *option, const QBrush &base, QPalette::ColorGroup cg, DPalette::ColorType type) const
{
    if (auto proxy = qobject_cast<const DStyle*>(this->proxy())) {
        return proxy->generatedBrush(getFlags(option) | state, base, cg, type, option);
    }

    return generatedBrush(getFlags(option) | state, base, cg, type, option);
}

QBrush DStyle::generatedBrush(StateFlags flags, const QBrush &base, QPalette::ColorGroup cg, DPalette::ColorType role, const QStyleOption *option) const
{
    Q_UNUSED(flags)
    Q_UNUSED(cg)
    Q_UNUSED(role)
    Q_UNUSED(option)

    return base;
}

#if QT_CONFIG(itemviews)
QSizeF DStyle::viewItemTextLayout(QTextLayout &textLayout, int lineWidth)
{
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout.beginLayout();
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    textLayout.endLayout();
    return QSizeF(widthUsed, height);
}

QSize DStyle::viewItemSize(const QStyle *style, const QStyleOptionViewItem *option, int role)
{
    const QWidget *widget = option->widget;
    switch (role) {
    case Qt::CheckStateRole:
        if (option->features & QStyleOptionViewItem::HasCheckIndicator)
            return QSize(style->pixelMetric(QStyle::PM_IndicatorWidth, option, widget),
                         style->pixelMetric(QStyle::PM_IndicatorHeight, option, widget));
        break;
    case Qt::DisplayRole:
        if (option->features & QStyleOptionViewItem::HasDisplay) {
            QTextOption textOption;
            textOption.setWrapMode(QTextOption::WordWrap);
            QTextLayout textLayout(option->text, option->font);
            textLayout.setTextOption(textOption);
            const bool wrapText = option->features & QStyleOptionViewItem::WrapText;
            const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, option, widget) + 1;
            QRect bounds = option->rect;
            switch (option->decorationPosition) {
            case QStyleOptionViewItem::Left:
            case QStyleOptionViewItem::Right: {
                if (wrapText && bounds.isValid()) {
                    int width = bounds.width() - 2 * textMargin;
                    if (option->features & QStyleOptionViewItem::HasDecoration)
                        width -= option->decorationSize.width() + 2 * textMargin;
                    bounds.setWidth(width);
                } else
                    bounds.setWidth(QFIXED_MAX);
                break;
            }
            case QStyleOptionViewItem::Top:
            case QStyleOptionViewItem::Bottom:
                if (wrapText)
                    bounds.setWidth(bounds.isValid() ? bounds.width() - 2 * textMargin : option->decorationSize.width());
                else
                    bounds.setWidth(QFIXED_MAX);
                break;
            default:
                break;
            }

            if (wrapText && option->features & QStyleOptionViewItem::HasCheckIndicator)
                bounds.setWidth(bounds.width() - style->pixelMetric(QStyle::PM_IndicatorWidth) - 2 * textMargin);

            const int lineWidth = bounds.width();
            const QSizeF size = viewItemTextLayout(textLayout, lineWidth);
            return QSize(qCeil(size.width()) + 2 * textMargin, qCeil(size.height()));
        }
        break;
    case Qt::DecorationRole:
        if (option->features & QStyleOptionViewItem::HasDecoration) {
            return option->decorationSize;
        }
        break;
    default:
        break;
    }

    return QSize(0, 0);
}

void DStyle::viewItemLayout(const QStyle *style, const QStyleOptionViewItem *opt,  QRect *pixmapRect, QRect *textRect, QRect *checkRect, bool sizehint)
{
    Q_ASSERT(checkRect && pixmapRect && textRect);
    *pixmapRect = QRect(QPoint(0, 0), viewItemSize(style, opt, Qt::DecorationRole));
    *textRect = QRect(QPoint(0, 0), viewItemSize(style, opt, Qt::DisplayRole));
    *checkRect = QRect(QPoint(0, 0), viewItemSize(style, opt, Qt::CheckStateRole));

    const QWidget *widget = opt->widget;
    const bool hasCheck = checkRect->isValid();
    const bool hasPixmap = pixmapRect->isValid();
    const bool hasText = textRect->isValid();
    const bool hasMargin = (hasText | hasPixmap | hasCheck);
    const int frameHMargin = hasMargin ? style->pixelMetric(QStyle::PM_FocusFrameHMargin, opt, widget) + 1 : 0;
    const int textMargin = hasText ? frameHMargin : 0;
    const int pixmapMargin = hasPixmap ? frameHMargin : 0;
    const int checkMargin = hasCheck ? frameHMargin : 0;
    const int x = opt->rect.left();
    const int y = opt->rect.top();
    int w, h;

    if (textRect->height() == 0 && (!hasPixmap || !sizehint)) {
        //if there is no text, we still want to have a decent height for the item sizeHint and the editor size
        textRect->setHeight(opt->fontMetrics.height());
    }

    QSize pm(0, 0);
    if (hasPixmap) {
        pm = pixmapRect->size();
        pm.rwidth() += 2 * pixmapMargin;
    }
    if (sizehint) {
        h = qMax(checkRect->height(), qMax(textRect->height(), pm.height()));
        if (opt->decorationPosition == QStyleOptionViewItem::Left
            || opt->decorationPosition == QStyleOptionViewItem::Right) {
            w = textRect->width() + pm.width();
        } else {
            w = qMax(textRect->width(), pm.width());
        }

        int cw = 0;
        QRect check;
        if (hasCheck) {
            cw = checkRect->width() + 2 * checkMargin;
            if (sizehint) w += cw;
            // 选择标识框优先放置在右边
            if (opt->direction == Qt::RightToLeft) {
                check.setRect(x, y, cw, h);
            } else {
                check.setRect(x + w - cw, y, cw, h);
            }
        }

        QRect display;
        QRect decoration;
        switch (opt->decorationPosition) {
        case QStyleOptionViewItem::Top: {
            if (hasPixmap)
                pm.setHeight(pm.height() + pixmapMargin); // add space
            h = sizehint ? textRect->height() : h - pm.height();

            if (opt->direction == Qt::RightToLeft) {
                decoration.setRect(x, y, w - cw, pm.height());
                display.setRect(x, y + pm.height(), w - cw, h);
            } else {
                decoration.setRect(x + cw, y, w - cw, pm.height());
                display.setRect(x + cw, y + pm.height(), w - cw, h);
            }
            break; }
        case QStyleOptionViewItem::Bottom: {
            if (hasText)
                textRect->setHeight(textRect->height() + textMargin); // add space
            h = sizehint ? textRect->height() + pm.height() : h;

            if (opt->direction == Qt::RightToLeft) {
                display.setRect(x, y, w - cw, textRect->height());
                decoration.setRect(x, y + textRect->height(), w - cw, h - textRect->height());
            } else {
                display.setRect(x + cw, y, w - cw, textRect->height());
                decoration.setRect(x + cw, y + textRect->height(), w - cw, h - textRect->height());
            }
            break; }
        case QStyleOptionViewItem::Left: {
            if (opt->direction == Qt::LeftToRight) {
                decoration.setRect(x + cw, y, pm.width(), h);
                display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
            } else {
                display.setRect(x, y, w - pm.width() - cw, h);
                decoration.setRect(display.right() + 1, y, pm.width(), h);
            }
            break; }
        case QStyleOptionViewItem::Right: {
            if (opt->direction == Qt::LeftToRight) {
                display.setRect(x + cw, y, w - pm.width() - cw, h);
                decoration.setRect(display.right() + 1, y, pm.width(), h);
            } else {
                decoration.setRect(x, y, pm.width(), h);
                display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
            }
            break; }
        default:
            qWarning("doLayout: decoration position is invalid");
            decoration = *pixmapRect;
            break;
        }

        *checkRect = check;
        *pixmapRect = decoration;
        *textRect = display;
    } else {
        w = opt->rect.width();
        h = opt->rect.height();

        *pixmapRect = QStyle::alignedRect(opt->direction, opt->decorationAlignment,
                                          pixmapRect->size(), opt->rect.adjusted(pixmapMargin, pixmapMargin, -pixmapMargin, -pixmapMargin));

        QRect display = opt->rect;

        switch (opt->decorationPosition) {
        case QStyleOptionViewItem::Top: {
            int residue_height = opt->rect.bottom() - pixmapRect->bottom();

            // 空间不够时，要改变图标的位置来腾出空间
            if (textRect->height() > residue_height) {
                pixmapRect->moveTop(qMax(0, pixmapRect->top() - textRect->height() + residue_height));
            }

            display.setTop(pixmapRect->bottom() + pixmapMargin);
            break;
        }
        case QStyleOptionViewItem::Bottom: {
            int residue_height = pixmapRect->top() - opt->rect.top();

            // 空间不够时，要改变图标的位置来腾出空间
            if (textRect->height() > residue_height) {
                pixmapRect->moveBottom(qMin(opt->rect.bottom(), pixmapRect->bottom() + textRect->height() - residue_height));
            }

            display.setBottom(pixmapRect->top() - pixmapMargin);
            break;
        }
        case QStyleOptionViewItem::Left:
        case QStyleOptionViewItem::Right:
            if (opt->decorationPosition == QStyleOptionViewItem::Left
                    && opt->direction == Qt::LeftToRight) {
                int residue_width = pixmapRect->left() - opt->rect.left();

                // 空间不够时，要改变图标的位置来腾出空间
                if (textRect->width() > residue_width) {
                    pixmapRect->moveLeft(qMax(opt->rect.left(), pixmapRect->left() - textRect->width() + residue_width));
                }

                display.setLeft(pixmapRect->right() + pixmapMargin);
            } else {
                int residue_width = opt->rect.right() - pixmapRect->left();

                // 空间不够时，要改变图标的位置来腾出空间
                if (textRect->width() > residue_width) {
                    pixmapRect->moveRight(qMin(opt->rect.right(), pixmapRect->right() + textRect->width() - residue_width));
                }

                display.setRight(pixmapRect->left() - pixmapMargin);
            }
        default:
            break;
        }

        display.adjust(checkMargin, 0, -checkMargin, 0);
        *checkRect = QStyle::alignedRect(opt->direction, Qt::AlignRight | Qt::AlignVCenter, checkRect->size(), display);

        // 减去margins
        display.adjust(textMargin, textMargin, -textMargin, -textMargin);
//        *textRect = QStyle::alignedRect(opt->direction, opt->displayAlignment, textRect->size(), display);

        if (opt->features & QStyleOptionViewItem::HasCheckIndicator)
            display.setRight(checkRect->left() - checkMargin - textMargin);

        *textRect = display;
    }
}

void DStyle::viewItemLayout(const QStyleOptionViewItem *opt, QRect *pixmapRect, QRect *textRect, QRect *checkRect, bool sizehint) const
{
    viewItemLayout(this, opt, pixmapRect, textRect, checkRect, sizehint);
}

QRect DStyle::viewItemDrawText(const QStyle *style, QPainter *p, const QStyleOptionViewItem *option, const QRect &rect)
{
    const QWidget *widget = option->widget;
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const bool wrapText = option->features & QStyleOptionViewItem::WrapText;
    QTextOption textOption;
    textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    textOption.setTextDirection(option->direction);
    textOption.setAlignment(QStyle::visualAlignment(option->direction, option->displayAlignment));
    QTextLayout textLayout(option->text, option->font);
    textLayout.setTextOption(textOption);

    viewItemTextLayout(textLayout, textRect.width());

    QString elidedText;
    qreal height = 0;
    qreal width = 0;
    int elidedIndex = -1;
    const int lineCount = textLayout.lineCount();
    for (int j = 0; j < lineCount; ++j) {
        const QTextLine line = textLayout.lineAt(j);
        if (j + 1 <= lineCount - 1) {
            const QTextLine nextLine = textLayout.lineAt(j + 1);
            if ((nextLine.y() + nextLine.height()) > textRect.height()) {
                int start = line.textStart();
                int length = line.textLength() + nextLine.textLength();
                const QStackTextEngine engine(textLayout.text().mid(start, length), option->font);
                elidedText = engine.elidedText(option->textElideMode, textRect.width());
                height += line.height();
                width = textRect.width();
                elidedIndex = j;
                break;
            }
        }
        if (line.naturalTextWidth() > textRect.width()) {
            int start = line.textStart();
            int length = line.textLength();
            const QStackTextEngine engine(textLayout.text().mid(start, length), option->font);
            elidedText = engine.elidedText(option->textElideMode, textRect.width());
            height += line.height();
            width = textRect.width();
            elidedIndex = j;
            break;
        }
        width = qMax<qreal>(width, line.width());
        height += line.height();
    }

    const QRect layoutRect = QStyle::alignedRect(option->direction, option->displayAlignment,
                                                 QSize(int(width), int(height)), textRect);
    const QPointF position = layoutRect.topLeft();
    for (int i = 0; i < lineCount; ++i) {
        const QTextLine line = textLayout.lineAt(i);
        if (i == elidedIndex) {
            qreal x = position.x() + line.x();
            qreal y = position.y() + line.y() + line.ascent();
            p->save();
            p->setFont(option->font);
            p->drawText(QPointF(x, y), elidedText);
            p->restore();
            break;
        }
        line.draw(p, position);
    }

    return layoutRect;
}

QRect DStyle::viewItemDrawText(QPainter *p, const QStyleOptionViewItem *option, const QRect &rect) const
{
    return viewItemDrawText(this, p, option, rect);
}
#endif

DWIDGET_END_NAMESPACE
