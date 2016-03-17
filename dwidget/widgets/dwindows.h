#ifndef DWINDOW_H
#define DWINDOW_H

#include <QWidget>
#include <dobject.h>
#include <libdui_global.h>

#include "dwidget.h"

DWIDGET_BEGIN_NAMESPACE

class DWindowPrivate;

class DWindow : public DWidget
{
    Q_OBJECT
public:
    explicit DWindow(DWidget *parent = 0);
    ~DWindow();

    Qt::WindowFlags windowFlags();
    void setWindowFlags(Qt::WindowFlags type);

    virtual void setTitle(const QString &);
    virtual void setIcon(const QPixmap &icon);
    virtual void setTitleFixedHeight(int h);

    virtual void setContentLayout(QLayout *);
    virtual void setContentWidget(QWidget *);

    virtual void showEvent(QShowEvent *);
    virtual void paintEvent(QPaintEvent *);
private:
    void setShadowHits();

    D_DECLARE_PRIVATE(DWindow)
    Q_DISABLE_COPY(DWindow)
};

DWIDGET_END_NAMESPACE

#endif // DWINDOW_H
