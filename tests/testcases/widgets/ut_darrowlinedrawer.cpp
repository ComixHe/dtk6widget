/*
 * Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     Wang Peng <wangpenga@uniontech.com>
 *
 * Maintainer: Wang Peng <wangpenga@uniontech.com>
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

#include <gtest/gtest.h>
#include <QTest>
#include <QDebug>

#include "darrowlinedrawer.h"
#include "ddrawer.h"
#include "private/ddrawer_p.h"

#include <QLabel>

DWIDGET_USE_NAMESPACE

class ut_DArrowLineDrawer : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;
    QWidget *widget = nullptr;
    DArrowLineDrawer *drawer = nullptr;
};

void ut_DArrowLineDrawer::SetUp()
{
    widget = new QWidget;
    widget->resize(300, 200);
    drawer = new DArrowLineDrawer(widget);
    drawer->resize(100, 30);
}

void ut_DArrowLineDrawer::TearDown()
{
    widget->deleteLater();
}

TEST_F(ut_DArrowLineDrawer, testDArrowLineDrawerSetTitle)
{
    const QString title("xxxxxxxxxxxx");
    drawer->setTitle(title);
    // TODO
}

TEST_F(ut_DArrowLineDrawer, testDArrowLineDrawerSetExpand)
{
    bool expand = false;
    drawer->setExpand(expand);
    expand = true;
    drawer->setExpand(expand);
    // TODO
}
