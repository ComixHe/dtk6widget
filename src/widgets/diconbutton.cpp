/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
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
#include "diconbutton.h"
#include "dstyleoption.h"
#include "dobject_p.h"
#include "dstyle.h"

#include <private/qabstractbutton_p.h>

DWIDGET_BEGIN_NAMESPACE

class DIconButtonPrivate : public DCORE_NAMESPACE::DObjectPrivate
{
public:
    DIconButtonPrivate(DIconButton *qq)
        : DObjectPrivate(qq)
    {
        // 默认调整大小，否则可能会导致按钮显示后为 QWidget 的默认大小
        qq->resize(qq->iconSize());
    }

    bool flat = false;
    int iconType = -1;

    D_DECLARE_PUBLIC(DIconButton)
};

DIconButton::DIconButton(QWidget *parent)
    : QAbstractButton(parent)
    , DObject(*new DIconButtonPrivate(this))
{

}

DIconButton::DIconButton(QStyle::StandardPixmap iconType, QWidget *parent)
    : DIconButton(static_cast<DStyle::StandardPixmap>(iconType), parent)
{

}

DIconButton::DIconButton(DStyle::StandardPixmap iconType, QWidget *parent)
    : DIconButton(parent)
{
    d_func()->iconType = iconType;
}

DIconButton::~DIconButton()
{

}

void DIconButton::setIcon(const QIcon &icon)
{
    D_D(DIconButton);

    d->iconType = -1;
    QAbstractButton::setIcon(icon);
}

void DIconButton::setIcon(QStyle::StandardPixmap iconType)
{
    D_D(DIconButton);

    d->iconType = iconType;
    setIcon(style()->standardIcon(iconType, nullptr, this));
}

void DIconButton::setIcon(DStyle::StandardPixmap iconType)
{
    D_D(DIconButton);

    d->iconType = iconType;
    setIcon(DStyleHelper(style()).standardIcon(iconType, nullptr, this));
}

QSize DIconButton::sizeHint() const
{
    QAbstractButtonPrivate *bp = static_cast<QAbstractButtonPrivate*>(QAbstractButton::d_ptr.data());

    if (bp->sizeHint.isValid()) {
        return bp->sizeHint;
    }

    DStyleOptionButton opt;
    initStyleOption(&opt);

    opt.rect.setSize(opt.iconSize);
    bp->sizeHint = DStyleHelper(style()).sizeFromContents(DStyle::CT_IconButton, &opt, opt.iconSize, this).expandedTo(QApplication::globalStrut());
    int size = qMax(bp->sizeHint.width(), bp->sizeHint.height());
    bp->sizeHint.setHeight(size);
    bp->sizeHint.setWidth(size);

    return bp->sizeHint;
}

QSize DIconButton::minimumSizeHint() const
{
    return sizeHint();
}

QSize DIconButton::iconSize() const
{
    QAbstractButtonPrivate *bp = static_cast<QAbstractButtonPrivate*>(QAbstractButton::d_ptr.data());

    if (bp->iconSize.isValid()) {
        return bp->iconSize;
    }

    DStyleHelper dstyle(style());
    int size = dstyle.pixelMetric(DStyle::PM_IconButtonIconSize, nullptr, this);

    if (Q_LIKELY(size > 0)) {
        return QSize(size, size);
    }

    return QAbstractButton::iconSize();
}

bool DIconButton::isFlat() const
{
    D_DC(DIconButton);

    return d->flat;
}

void DIconButton::setFlat(bool flat)
{
    D_D(DIconButton);

    d->flat = flat;

    if (d->flat == flat)
        return;

    QAbstractButtonPrivate *bp = static_cast<QAbstractButtonPrivate*>(QAbstractButton::d_ptr.data());
    bp->sizeHint = QSize();

    update();
    updateGeometry();
}

void DIconButton::initStyleOption(DStyleOptionButton *option) const
{
    if (!option)
        return;

    D_DC(DIconButton);

    option->initFrom(this);
    option->init(this);

    if (d->flat)
        option->features |= QStyleOptionButton::Flat;

    if (isChecked())
        option->state |= QStyle::State_On;

    if (isDown())
        option->state |= QStyle::State_Sunken;

    if (!d->flat && !isDown())
        option->state |= QStyle::State_Raised;

    option->text = text();
    option->icon = icon();
    option->iconSize = iconSize();
}

void DIconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    DStylePainter p(this);
    DStyleOptionButton opt;
    initStyleOption(&opt);
    p.drawControl(DStyle::CE_IconButton, opt);
}

bool DIconButton::event(QEvent *e)
{
    if (e->type() == QEvent::Polish) {
        D_DC(DIconButton);

        if (d->iconType > 0) {
            if (d->iconType > static_cast<int>(QStyle::SP_CustomBase)) {
                DStyleHelper dstyle(style());
                setIcon(dstyle.standardIcon(static_cast<DStyle::StandardPixmap>(d->iconType), nullptr, this));
            } else {
                setIcon(style()->standardIcon(static_cast<QStyle::StandardPixmap>(d->iconType), nullptr, this));
            }
        }
    }

    return QAbstractButton::event(e);
}

DWIDGET_END_NAMESPACE
