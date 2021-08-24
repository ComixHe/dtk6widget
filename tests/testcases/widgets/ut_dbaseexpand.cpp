/*
* Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co.,Ltd.
*
* Author:     Ye ShanShan <yeshanshan@uniontech.com>
*
* Maintainer: Ye ShanShan <yeshanshan@uniontech.com>>
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

#include "dbaseexpand.h"
DWIDGET_USE_NAMESPACE
class ut_DBaseExpand : public testing::Test
{
protected:
    void SetUp() override
    {
        target = new DBaseExpand();
    }
    void TearDown() override
    {
        if (target) {
            delete target;
            target = nullptr;
        }
    }
    DBaseExpand *target = nullptr;
};

TEST_F(ut_DBaseExpand, setContent)
{
    auto widget = new QWidget();
    target->setContent(widget);
    ASSERT_EQ(target->getContent(), widget);
};

TEST_F(ut_DBaseExpand, setExpand)
{
    target->setExpand(true);
    ASSERT_EQ(target->expand(), true);
};
