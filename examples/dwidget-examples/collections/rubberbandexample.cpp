/*
 * Copyright (C) 2020 ~ 2020 Deepin Technology Co., Ltd.
 *
 * Author:     sunkang <sunkang@uniontech.com>
 *
 * Maintainer: sunkang <sunkang@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMenu>
#include <QScrollArea>
#include <QDebug>

#include <DDialog>
#include <DStyle>
#include <DPushButton>
#include <DMessageBox>
#include <DWarningButton>
#include <DSuggestButton>
#include <DFrame>

#include "rubberbandexample.h"

DWIDGET_USE_NAMESPACE

RubberBandExampleWindow::RubberBandExampleWindow(QWidget *parent)
    : PageWindowInterface(parent)
{
    addExampleWindow(new DRubberBandExample(this));
}

DRubberBandExample::DRubberBandExample(QWidget *parent)
    : ExampleWindowInterface(parent)
{
}

QString DRubberBandExample::getTitleName() const
{
    return "DRubberBand";
}

QString DRubberBandExample::getDescriptionInfo() const
{
    return QString();
}

int DRubberBandExample::getFixedHeight() const
{
    return 300;
}
