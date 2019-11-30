#ifndef DREGIONMONITOR_H
#define DREGIONMONITOR_H

#include "dobject.h"
#include "dtkwidget_global.h"

#include <QObject>

DWIDGET_BEGIN_NAMESPACE

class DRegionMonitorPrivate;
class D_DECL_DEPRECATED_X("Use libdtkgui") DRegionMonitor : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DRegionMonitor)
    Q_DISABLE_COPY(DRegionMonitor)
    Q_PROPERTY(CoordinateType coordinateType READ coordinateType WRITE setCoordinateType NOTIFY coordinateTypeChanged)

public:
    explicit DRegionMonitor(QObject *parent = nullptr);

    enum WatchedFlags {
        Button_Left     = 1,
        Button_Right    = 3,
    };

    enum CoordinateType {
        ScaleRatio,
        Original
    };
    Q_ENUM(CoordinateType)

    bool registered() const;
    QRegion watchedRegion() const;
    CoordinateType coordinateType() const;

Q_SIGNALS:
    void buttonPress(const QPoint &p, const int flag) const;
    void buttonRelease(const QPoint &p, const int flag) const;
    void cursorMove(const QPoint &p) const;
    void keyPress(const QString &keyname) const;
    void keyRelease(const QString &keyname) const;
    void coordinateTypeChanged(CoordinateType type) const;

public Q_SLOTS:
    void registerRegion();
    inline void registerRegion(const QRegion &region) { setWatchedRegion(region); registerRegion(); }
    void unregisterRegion();
    void setWatchedRegion(const QRegion &region);
    void setCoordinateType(CoordinateType type);

private:
    Q_PRIVATE_SLOT(d_func(), void _q_ButtonPress(const int, const int, const int, const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_ButtonRelease(const int, const int, const int, const QString&))
    Q_PRIVATE_SLOT(d_func(), void _q_CursorMove(const int, const int, const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_KeyPress(const QString &, const int, const int, const QString &))
    Q_PRIVATE_SLOT(d_func(), void _q_KeyRelease(const QString &, const int, const int, const QString &))
};

DWIDGET_END_NAMESPACE

#endif // DREGIONMONITOR_H
