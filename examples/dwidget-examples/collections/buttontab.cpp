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

#include "buttontab.h"

#include <QDebug>

DWIDGET_USE_NAMESPACE

ButtonTab::ButtonTab(QWidget *parent) : QLabel(parent)
{
    setStyleSheet("ButtonTab{background-color: #252627;}");

    DArrowButton * arrowButton = new DArrowButton(this);
    arrowButton->setArrowDirection(DArrowButton::ArrowDown);
    arrowButton->move(5, 5);

    DWindowMinButton * minButton = new DWindowMinButton(this);
    minButton->move(30, 5);

    DWindowMaxButton * maxButton = new DWindowMaxButton(this);
    maxButton->move(50, 5);

    DWindowCloseButton * closeButton = new DWindowCloseButton(this);
    closeButton->move(90, 5);

    DWindowOptionButton * optionButton = new DWindowOptionButton(this);
    optionButton->move(110, 5);

    //////////////////////////////////////////////////////////////--DTextButton

    DImageButton *imageButton = new DImageButton(":/images/button.png", ":/images/buttonHover.png", ":/images/buttonPress.png", this);
    imageButton->move(5, 100);
    imageButton->setChecked(true);

    DImageButton *imageButton2 = new DImageButton(this);
    imageButton2->setNormalPic(":/images/buttonHover.png");
    imageButton2->move(35, 100);

    DImageButton *checkableImageButton = new DImageButton(":/images/button.png", ":/images/buttonHover.png", ":/images/buttonPress.png", ":/images/buttonChecked.png", this);
    checkableImageButton->move(85, 100);
    connect(checkableImageButton, SIGNAL(clicked()), this, SLOT(buttonClickTest()));

    DSwitchButton *switchButton = new DSwitchButton(this);
    switchButton->move(85, 200);
}

void ButtonTab::buttonClickTest()
{
    qDebug() << "clicked";
}
