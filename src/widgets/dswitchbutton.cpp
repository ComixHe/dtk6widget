/*
 * Copyright (C) 2015 ~ 2017 Deepin Technology Co., Ltd.
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

#include "dswitchbutton.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QIcon>
#include "dthememanager.h"
#include "private/dswitchbutton_p.h"

DWIDGET_BEGIN_NAMESPACE

/*!
 * \class DSwitchButton
 * \brief The DSwitchButton class provides switch like widget.
 *
 * User can put the switch on/off the turn some feature on/off.
 *
 * It's inspired by UISwitch of Apple,
 * see https://developer.apple.com/documentation/uikit/uiswitch.
 */

/*!
 * \brief DSwitchButton::DSwitchButton constructs an instance of DSwitchButton.
 * \param parent is passed to QFrame constructor.
 */
DSwitchButton::DSwitchButton(QWidget *parent) :
    QFrame(parent),
    DObject(*new DSwitchButtonPrivate(this))
{
    setObjectName("DSwitchButton");

    setMaximumSize(36, 20);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    DThemeManager::registerWidget(this);

    D_D(DSwitchButton);

    d->init();

    connect(d->animation, &QVariantAnimation::valueChanged, [&]() {
        this->update();
    });
}

/*!
 * \property DSwitchButton::checked
 * \brief This property holds whether the switch is on or off.
 *
 * True if the switch is on, otherwise false.
 */
bool DSwitchButton::checked() const
{
    D_DC(DSwitchButton);

    return d->checked;
}

QSize DSwitchButton::sizeHint() const
{
    return maximumSize();
}

QColor DSwitchButton::disabledBackground() const
{
    D_DC(DSwitchButton);
    return d->disabledBackground;
}

QColor DSwitchButton::enabledBackground() const
{
    D_DC(DSwitchButton);
    return d->enabledBackground;
}

void DSwitchButton::setChecked(bool arg)
{
    D_D(DSwitchButton);

    if (d->checked != arg) {
        d->checked = arg;

        if (arg) {
            d->animation->setStartValue(d->animationStartValue);
            d->animation->setEndValue(d->animationEndValue);
        } else {
            d->animation->setStartValue(d->animationEndValue);
            d->animation->setEndValue(d->animationStartValue);
        }
        d->animation->start();

        Q_EMIT checkedChanged(arg);
    }
}

void DSwitchButton::setEnabledBackground(QColor enabledBackground)
{
    D_D(DSwitchButton);
    d->enabledBackground = enabledBackground;
}

void DSwitchButton::setDisabledBackground(QColor disabledBackground)
{
    D_D(DSwitchButton);
    d->disabledBackground = disabledBackground;
}

void DSwitchButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    D_D(DSwitchButton);

    QColor frontground = Qt::white;
    QColor background = d->enabledBackground;

    if (d->checked) {
        background = d->checkedBackground;
    }

    if (!isEnabled()) {
        background.setAlpha(255 * 55 / 100);
        frontground.setAlpha(255 * 55 / 100);
    }

    QPainter p(this);
    p.setRenderHints(QPainter::Antialiasing);

    QPainterPath path;
    path.addRoundedRect(rect(), height() / 2.0, height() / 2.0);
    path.closeSubpath();

    p.setClipPath(path);

    double indicatorX = 0;

    if (d->animation->state() == QVariantAnimation::Stopped) {
        if (!d->checked) {
            indicatorX = d->animationStartValue;
        } else {
            indicatorX = d->animationEndValue;
        }
    } else {
        indicatorX = d->animation->currentValue().toDouble();
    }

    int moveWidth = width() - height();
    int border = 1 * 1;
    int btSize = height() - border * 2;
    QRectF btRect(moveWidth * indicatorX + border, border, btSize, btSize);
    QPainterPath btPath;
    btPath.addRoundedRect(btRect, btSize / 2.0, btSize / 2.0);
    btPath.closeSubpath();

    p.fillPath(path, background);
    p.fillPath(btPath, frontground);
}

void DSwitchButton::mousePressEvent(QMouseEvent *e)
{
    D_D(DSwitchButton);

    if (e->button() == Qt::LeftButton) {
        setChecked(!d->checked);
        e->accept();
    }
}

DSwitchButtonPrivate::DSwitchButtonPrivate(DSwitchButton *qq)
    : DObjectPrivate(qq)
{
    init();
}

DSwitchButtonPrivate::~DSwitchButtonPrivate()
{
    animation->deleteLater();
}

void DSwitchButtonPrivate::init()
{
    checked = false;
    animation = new QVariantAnimation;
    animationStartValue = 0;
    animationEndValue = 1;
}

DWIDGET_END_NAMESPACE
