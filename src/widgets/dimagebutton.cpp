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

#include "dimagebutton.h"
#include "dconstants.h"
#include "dthememanager.h"

#include <QMouseEvent>
#include <QEvent>
#include <QIcon>
#include <QApplication>

DWIDGET_BEGIN_NAMESPACE

DImageButton::DImageButton(QWidget *parent)
    : QLabel(parent)
{
    D_THEME_INIT_WIDGET(DImageButton);
    updateIcon();
}

DImageButton::DImageButton(const QString &normalPic, const QString &hoverPic, const QString &pressPic, QWidget *parent)
    : QLabel(parent)
{
    D_THEME_INIT_WIDGET(DImageButton);

    if (!normalPic.isEmpty())
        m_normalPic = normalPic;
    if (!hoverPic.isEmpty())
        m_hoverPic = hoverPic;
    if (!pressPic.isEmpty())
        m_pressPic = pressPic;

    setCheckable(false);

    updateIcon();
}

DImageButton::DImageButton(const QString &normalPic, const QString &hoverPic,
                           const QString &pressPic, const QString &checkedPic, QWidget *parent)
    : QLabel(parent)
{
    D_THEME_INIT_WIDGET(DImageButton);

    if (!normalPic.isEmpty())
        m_normalPic = normalPic;
    if (!hoverPic.isEmpty())
        m_hoverPic = hoverPic;
    if (!pressPic.isEmpty())
        m_pressPic = pressPic;
    if (!checkedPic.isEmpty())
        m_checkedPic = checkedPic;

    setCheckable(true);

    updateIcon();
}

DImageButton::~DImageButton()
{
}

void DImageButton::enterEvent(QEvent *event)
{
    setCursor(Qt::PointingHandCursor);

    if (!m_isChecked){
        setState(Hover);
    }

    event->accept();
    //QLabel::enterEvent(event);
}

void DImageButton::leaveEvent(QEvent *event)
{
    if (!m_isChecked){
        setState(Normal);
    }

    event->accept();
    //QLabel::leaveEvent(event);
}

void DImageButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
        return;

    setState(Press);

    event->accept();
    //QLabel::mousePressEvent(event);
}

void DImageButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (!rect().contains(event->pos()))
        return;

    if (m_isCheckable){
        m_isChecked = !m_isChecked;
        if (m_isChecked){
            setState(Checked);
        } else {
            setState(Normal);
        }
    } else {
        setState(Hover);
    }

    event->accept();
    //QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
        Q_EMIT clicked();
}

void DImageButton::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isCheckable && !rect().contains(event->pos())) {
        setState(Normal);
    }
}

void DImageButton::updateIcon()
{
    switch (m_state) {
    case Hover:     if (!m_hoverPic.isEmpty()) setPixmap(loadPixmap(m_hoverPic));      break;
    case Press:     if (!m_pressPic.isEmpty()) setPixmap(loadPixmap(m_pressPic));      break;
    case Checked:   if (!m_checkedPic.isEmpty()) setPixmap(loadPixmap(m_checkedPic));  break;
    default:        if (!m_normalPic.isEmpty()) setPixmap(loadPixmap(m_normalPic));    break;
    }

    setAlignment(Qt::AlignCenter);

    Q_EMIT stateChanged();
}

void DImageButton::setState(DImageButton::State state)
{
    if (m_state == state)
        return;

    m_state = state;

    updateIcon();
}

QPixmap DImageButton::loadPixmap(const QString &path)
{
    qreal ratio = 1.0;

    const qreal devicePixelRatio = qApp->devicePixelRatio();

    QPixmap pixmap;

    if (devicePixelRatio > ratio) {
        pixmap.load(qt_findAtNxFile(path, devicePixelRatio, &ratio));

        pixmap = pixmap.scaled(devicePixelRatio / ratio * pixmap.width(),
                               devicePixelRatio / ratio * pixmap.height(),
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        pixmap.setDevicePixelRatio(devicePixelRatio);
    } else {
        pixmap.load(path);
    }

    return pixmap;
}

void DImageButton::setCheckable(bool flag)
{
    m_isCheckable = flag;

    if (!m_isCheckable){
        setState(Normal);
    }
}

void DImageButton::setChecked(bool flag)
{
    if (m_isCheckable == false){
        return;
    }

    m_isChecked = flag;
    if (m_isChecked){
        setState(Checked);
    } else {
        setState(Normal);
    }
}

bool DImageButton::isChecked()
{
    return m_isChecked;
}

bool DImageButton::isCheckable()
{
    return m_isCheckable;
}

void DImageButton::setNormalPic(const QString &normalPicPixmap)
{
    m_normalPic = normalPicPixmap;
    updateIcon();
}

void DImageButton::setHoverPic(const QString &hoverPicPixmap)
{
    m_hoverPic = hoverPicPixmap;
    updateIcon();
}

void DImageButton::setPressPic(const QString &pressPicPixmap)
{
    m_pressPic = pressPicPixmap;
    updateIcon();
}

void DImageButton::setCheckedPic(const QString &checkedPicPixmap)
{
    m_checkedPic = checkedPicPixmap;
    updateIcon();
}

DImageButton::State DImageButton::getState() const
{
    return m_state;
}

DWIDGET_END_NAMESPACE
