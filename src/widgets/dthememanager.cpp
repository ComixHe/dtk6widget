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

#include <QFile>
#include <QWidget>
#include <QStyle>
#include <QStyleFactory>

#include <QDebug>

#include <private/qstylesheetstyle_p.h>

#include "dthememanager.h"
#include "dapplication.h"

DWIDGET_BEGIN_NAMESPACE

class DThemeManagerPrivate : public DThemeManager
{

};

Q_GLOBAL_STATIC(DThemeManagerPrivate, DThemeManagerStatic)


/*!
 * \class DThemeManager
 * \brief The DThemeManager class provides helper to manage theme files of widgets.
 *
 * The DThemeManager is singleton class, DThemeManager::instance is used to
 * get an instance of DThemeManager.
 *
 * DThemeManager can be used to get the current application theme, find the
 * corresponding theme file of a widget.
 */

DThemeManager *DThemeManager::instance()
{
    return DThemeManagerStatic;
}

/*!
 * \brief DThemeManager::theme
 * \return the theme name currently be used by this application, the possible
 * value are "light" and "dark".
 */
QString DThemeManager::theme() const
{
    return m_theme;
}

QString DThemeManager::theme(const QWidget *widget, QWidget **baseWidget) const
{
    QString theme;

    if (baseWidget)
        *baseWidget = nullptr;

    do {
        theme = widget->property("_d_dtk_theme").toString();
        if (!theme.isEmpty()) {
            if (baseWidget)
                *baseWidget = const_cast<QWidget*>(widget);

            break;
        }
        widget = widget->isWindow() ? 0 : widget->parentWidget();
    } while (widget);

    return theme.isEmpty() ? m_theme : theme;
}

/*!
 * \brief DThemeManager::setTheme sets the application theme.
 * \param theme is the theme name to be set.
 */
void DThemeManager::setTheme(const QString theme)
{
    if (m_theme != theme) {
        QStyle *style = Q_NULLPTR;

        // TODO: remove this shit in the future.
        // It's just a trick to make all DApplications use dde qt5 styles,
        // if dlight or ddark style is set to default style of dde, those
        // ugly code will no longer needed.
        if (theme == "light") {
            style = QStyleFactory::create("dlight");
            m_theme = theme;
        } else if (theme == "dark") {
            style = QStyleFactory::create("ddark");
            m_theme = theme;
        } else if (theme == "semilight") {
            style = QStyleFactory::create("dsemilight");
            m_theme = "light";
        } else if (theme == "semidark") {
            style = QStyleFactory::create("dsemidark");
            m_theme = "dark";
        }

        if (style) {
            qApp->setStyle(style);
        }

        Q_EMIT themeChanged(m_theme);
    }
}

static QString getObjectClassName(const QObject *obj)
{
    const QString &type_name = QString::fromLocal8Bit(obj->metaObject()->className());
    const QStringList &widget_type_list = type_name.split("::");

    return widget_type_list.isEmpty() ? type_name : widget_type_list.last();
}

static void emitThemeChanged(DThemeManager *manager, QWidget *widget, const QString &theme)
{
    Q_EMIT manager->widgetThemeChanged(widget, theme);

    for (QObject *child : widget->children()) {
        if (QWidget *cw = qobject_cast<QWidget*>(child)) {
            if (cw->property("_d_dtk_theme").isValid())
                continue;

            emitThemeChanged(manager, cw, theme);
        }
    }
}

static void setStyle(QWidget *widget, QStyle *style)
{
    widget->setStyle(style);

    for (QObject *child : widget->children()) {
        if (QWidget *cw = qobject_cast<QWidget*>(child)) {
            if (cw->property("_d_dtk_theme").isValid())
                continue;

            setStyle(cw, style);
        }
    }
}

static void inseritStyle(QWidget *widget, const QWidget *baseWidget)
{
    if (widget == baseWidget)
        return;

    QStyle *base_style = baseWidget ? baseWidget->style() : qApp->style();

    if (base_style->inherits("QStyleSheetStyle")) {
        base_style = static_cast<QStyleSheetStyle*>(base_style)->base;
    }

    widget->setStyle(base_style);
}

/*!
 * \brief DThemeManager::setTheme sets theme on a widget.
 * \param widget is the target widget.
 * \param theme is the name of theme to be set.
 */
void DThemeManager::setTheme(QWidget *widget, const QString theme)
{
    Q_ASSERT(widget);

    if (theme.isEmpty()) {
        QString old_theme = this->theme(widget);

        widget->setProperty("_d_dtk_theme", QVariant());

        QWidget *baseWidget = nullptr;

        if (this->theme(widget, &baseWidget) != old_theme) {
            emitThemeChanged(this, widget, this->theme(widget));
        }

        inseritStyle(widget, baseWidget);

        return;
    }

    const QString &old_theme = this->theme(widget);
    QStyle *style = Q_NULLPTR;

    // TODO: remove this shit in the future.
    // It's just a trick to make all DApplications use dde qt5 styles,
    // if dlight or ddark style is set to default style of dde, those
    // ugly code will no longer needed.
    if (theme == "light") {
        style = QStyleFactory::create("dlight");
        widget->setProperty("_d_dtk_theme", theme);
    } else if (theme == "dark") {
        style = QStyleFactory::create("ddark");
        widget->setProperty("_d_dtk_theme", theme);
    } else if (theme == "semilight") {
        style = QStyleFactory::create("dsemilight");
        widget->setProperty("_d_dtk_theme", "light");
    } else if (theme == "semidark") {
        style = QStyleFactory::create("dsemidark");
        widget->setProperty("_d_dtk_theme", "dark");
    }

    if (style) {
        setStyle(widget, style);
    }

    if (old_theme != theme)
        emitThemeChanged(this, widget, theme);
}

/*!
 * \brief DThemeManager::getQssForWidget searches for the theme file of one class in a specific theme.
 * \param className is the name of the class.
 * \param theme is the name of the theme to be applied.
 * \return The content of the theme file.
 *
 * \note The class name should have no namespace prefixes.
 */
QString DThemeManager::getQssForWidget(const QString className, const QString &theme) const
{
    QString qss;

    QString themeName = theme.isEmpty() ? m_theme : theme;
    QString themeURL = QString(":/%1/%2.theme").arg(themeName).arg(className);
    QFile themeFile(themeURL);

    if (themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qss = themeFile.readAll();
        themeFile.close();
    } /*else {
        qWarning() << "open qss file failed" << themeURL << themeFile.errorString();
    }*/

    return qss;
}

/*!
 * \brief DThemeManager::getQssForWidget This function is overloaded.
 * \param className is the name of the target class.
 * \param widget is the widget from which to read the theme name.
 * \return The content of the theme file.
 *
 * \note The class name should have no namespace prefixes.
 */
QString DThemeManager::getQssForWidget(const QString className, const QWidget *widget) const
{
    Q_ASSERT(widget);

    return getQssForWidget(className, theme(widget));
}

/*!
 * \brief DThemeManager::getQssForWidget This function is overloaded.
 * \param widget is the target widget.
 * \return The content of the theme file to be applied on the widget.
 */
QString DThemeManager::getQssForWidget(const QWidget *widget) const
{
    return getQssForWidget(getObjectClassName(widget), widget);
}

/*!
    \since 2.0
    \relates DThemeManager
    Find QSS file on the qrc by getQssForWidget, className is form metaObject
 */
void DThemeManager::registerWidget(QWidget *widget, QStringList propertys)
{
    auto meta = widget->metaObject();
    Q_ASSERT(meta);

    auto className = meta->className();
    auto dtm = DThemeManager::instance();
    auto qss = dtm->getQssForWidget(className, widget);
    widget->setStyleSheet(widget->styleSheet() + qss);

    connect(dtm, &DThemeManager::themeChanged,
    widget, [widget, dtm, className](QString) {
        widget->setStyleSheet(dtm->getQssForWidget(className, widget));
    });

    for (auto property : propertys) {
        auto propertyIndex = meta->indexOfProperty(property.toLatin1().data());
        if (-1 == propertyIndex) {
            continue;
        }

        connect(widget, meta->property(propertyIndex).notifySignal(),
                dtm, dtm->metaObject()->method(dtm->metaObject()->indexOfMethod("updateQss()")));
    }
}

/*!
 * \brief DThemeManager::updateQss This slot repolishes the theme applied on
 * the sender widget.
 *
 * This slot is triggered when the property of a registered widget changes,
 * it's useful because Qt has no support of repolishing the style if some
 * custom properties used in the QSS file has changed.
 */
void DThemeManager::updateQss()
{
    QWidget *w = qobject_cast<QWidget *>(sender());
    if (w) {
        w->setStyleSheet(w->styleSheet());
    }
}

DThemeManager::DThemeManager()
    : QObject()
{
    setTheme("light");
}

static void updateWidgetTheme(DThemeManager *manager, QWidget *widget, QWidget *baseWidget, const QString &theme)
{
    inseritStyle(widget, baseWidget);

    Q_EMIT manager->widgetThemeChanged(widget, theme);

    for (QObject *child : widget->children()) {
        if (QWidget *cw = qobject_cast<QWidget*>(child)) {
            if (widget->property("_d_dtk_theme").isValid())
                return;

            updateWidgetTheme(manager, cw, baseWidget, theme);
        }
    }
}

void DThemeManager::updateThemeOnParentChanged(QWidget *widget)
{
    if (widget->property("_d_dtk_theme").isValid())
        return;

    QWidget *base_widget = nullptr;
    const QString &theme = this->theme(widget, &base_widget);

    if (!base_widget)
        return;

    updateWidgetTheme(this, widget, base_widget, theme);
}


DWIDGET_END_NAMESPACE
