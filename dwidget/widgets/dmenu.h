#ifndef DMENU_H
#define DMENU_H

#include <DObject>
#include <DAction>
#include <libdui_global.h>

#include <QObject>

DWIDGET_BEGIN_NAMESPACE
class DMenuPrivate;
class DMenuItem;

class LIBDTKWIDGETSHARED_EXPORT DMenu:  public QObject, public DObject
{
    Q_OBJECT

public:
    DMenu(QObject *parent = nullptr);

    void attatch(QWidget *);

    DAction *addAction(const QString & text);
    DAction *addAction(const QIcon & icon, const QString & text);

    DAction *addMenu(DMenu *menu);
    DMenu *addMenu(const QString & title);
    DMenu *addMenu(const QIcon & icon, const QString & title);
    DAction *addSeparator();

    DAction *actionAt(const QString &text);

    void exec();
    void exec(const QPoint & p, QAction * action = 0);
    void show(const QPoint &pos);

Q_SIGNALS:
    void triggered(DAction * action);

private:
    friend class DActionPrivate;

    Q_PRIVATE_SLOT(d_func(), void _q_onMenuUnregistered())
    Q_PRIVATE_SLOT(d_func(), void _q_onItemInvoked(const QString &, bool))
    D_DECLARE_PRIVATE(DMenu)
};

DWIDGET_END_NAMESPACE

#endif // DMENU_H
