#ifndef DTITLEBAR_H
#define DTITLEBAR_H

#include <QWidget>
#include <QMenu>

#include <dobject.h>
#include "dwidget_global.h"

DWIDGET_BEGIN_NAMESPACE

class DTitlebarPrivate;
class DMenu;

///
/// \brief The DTitlebar class is an universal title bar on the top of windows.
///
class LIBDTKWIDGETSHARED_EXPORT DTitlebar : public QWidget , public DObject
{
    Q_OBJECT
public:
    explicit DTitlebar(QWidget *parent = 0);

#ifndef QT_NO_MENU
    Q_DECL_DEPRECATED DMenu *menu() const;
    QMenu *getMenu() const;
#endif
    QWidget *customWidget() const;

#ifndef QT_NO_MENU
    Q_DECL_DEPRECATED_X("Plase use void setMenu(QMenu *)") void setMenu(DMenu *);
    void setMenu(QMenu *menu);
#endif
    void setCustomWidget(QWidget *, bool fixCenterPos = false);
    void setCustomWidget(QWidget *, Qt::AlignmentFlag flag = Qt::AlignCenter, bool fixCenterPos = false);
    void setWindowFlags(Qt::WindowFlags type);
    int buttonAreaWidth() const;
    bool separatorVisible() const;

    void setVisible(bool visible) Q_DECL_OVERRIDE;

    void resize(int width, int height);
    void resize(const QSize &);
signals:
    Q_DECL_DEPRECATED void minimumClicked();
    Q_DECL_DEPRECATED void maximumClicked();
    Q_DECL_DEPRECATED void restoreClicked();
    Q_DECL_DEPRECATED void closeClicked();
    void optionClicked();
    void doubleClicked();
    void mousePressed(Qt::MouseButtons buttons);
    void mouseMoving(Qt::MouseButton botton);

#ifdef Q_OS_WIN
    void mousePosPressed(Qt::MouseButtons buttons, QPoint pos);
    void mousePosMoving(Qt::MouseButton botton, QPoint pos);
#endif

public slots:
    void setFixedHeight(int h);
    void setSeparatorVisible(bool visible);
    void setTitle(const QString &title);
    void setIcon(const QPixmap &icon);
    Q_DECL_DEPRECATED void setWindowState(Qt::WindowState windowState);
    /// Maximized/Minumized
    void toggleWindowState();

private slots:
#ifndef QT_NO_MENU
    void showMenu();
#endif

protected:
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    D_DECLARE_PRIVATE(DTitlebar)
    D_PRIVATE_SLOT(void _q_toggleWindowState())
    D_PRIVATE_SLOT(void _q_showMinimized())
};

DWIDGET_END_NAMESPACE

#endif // DTITLEBAR_H
