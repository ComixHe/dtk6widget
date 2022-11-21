// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIMAGEVIEWER_H
#define DIMAGEVIEWER_H

#include <dtkwidget_global.h>
#include <DGraphicsView>
#include <DObject>

DWIDGET_BEGIN_NAMESPACE

class DImageViewerPrivate;
class LIBDTKWIDGETSHARED_EXPORT DImageViewer : public DGraphicsView, public DCORE_NAMESPACE::DObject
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(qreal scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY scaleFactorChanged)
    Q_PROPERTY(qreal rotateAngle READ rotateAngle NOTIFY rotateAngleChanged) 

public:
    explicit DImageViewer(QWidget *parent = nullptr);
    explicit DImageViewer(const QImage &image, QWidget *parent = nullptr);
    explicit DImageViewer(const QString &fileName, QWidget *parent = nullptr);
    ~DImageViewer() Q_DECL_OVERRIDE;

    QImage image() const;
    void setImage(const QImage &image);
    QString fileName() const;
    void setFileName(const QString &fileName);

    qreal scaleFactor() const;
    void setScaleFactor(qreal factor);

    void autoFitImage();
    void fitToWidget();
    void fitNormalSize();
    void rotateClockwise();
    void rotateCounterclockwise();
    qreal rotateAngle() const;
    void resetRotateAngle();
    void clear();

    void centerOn(qreal x, qreal y);

Q_SIGNALS:
    void imageChanged(const QImage &image);
    void fileNameChanged(const QString &fileName);
    void scaleFactorChanged(qreal scaleFactor);
    void rotateAngleChanged(qreal angle);
    void transformChanged();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    void timerEvent(QTimerEvent *event) Q_DECL_OVERRIDE;
    void drawBackground(QPainter *painter, const QRectF &rect) Q_DECL_OVERRIDE;
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(DImageViewer)
    D_DECLARE_PRIVATE(DImageViewer)

    D_PRIVATE_SLOT(void _q_imageLoadFinished())
    D_PRIVATE_SLOT(void _q_pinchAnimeFinished())
};

DWIDGET_END_NAMESPACE

#endif // DIMAGEVIEWER_H
