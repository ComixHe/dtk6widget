#include "dwaterprogress.h"

#include <QtMath>
#include <QTimer>
#include <QPainter>

#include <DObjectPrivate>

#include <DSvgRenderer>

DCORE_USE_NAMESPACE

DWIDGET_BEGIN_NAMESPACE


struct Pop {
    Pop(double s, double xs, double ys):
        size(s), xSpeed(xs), ySpeed(ys) {}

    double size;
    double xSpeed;
    double ySpeed;
    double xOffset;
    double yOffset;
};

class DWaterProgressPrivate: public DTK_CORE_NAMESPACE::DObjectPrivate
{
public:
    DWaterProgressPrivate(DWaterProgress *parent): DObjectPrivate(parent)
    {
        pops.append(Pop(7, -1.8, 0.6));
        pops.append(Pop(8, 1.2, 1.0));
        pops.append(Pop(11, 0.8, 1.6));
    }

    void resizePixmap(QSize sz);
    void initUI();
    void setValue(int v);
    void paint(QPainter *p);

    QImage waterFrontImage;
    QImage waterBackImage;
    QString progressText;
    QTimer *timer               = Q_NULLPTR;
    QList<Pop> pops;

    int     interval            = 33;
    int     value;

    double  frontXOffset        = 0;
    double  backXOffset         = 0;

    D_DECLARE_PUBLIC(DWaterProgress)
};

DWaterProgress::DWaterProgress(QWidget *parent) :
    QWidget(parent), DObject(*new DWaterProgressPrivate(this))
{
    D_D(DWaterProgress);
    d->initUI();
}

DWaterProgress::~DWaterProgress()
{

}

int DWaterProgress::value() const
{
    D_DC(DWaterProgress);
    return d->value;
}

void DWaterProgress::start()
{
    D_DC(DWaterProgress);
    d->timer->start();
}

void DWaterProgress::stop()
{
    D_DC(DWaterProgress);
    d->timer->stop();
}

void DWaterProgress::setValue(int v)
{
    D_D(DWaterProgress);
    if (d->value == v) {
        return;
    }
    d->setValue(v);
    Q_EMIT valueChanged();
}

void DWaterProgress::paintEvent(QPaintEvent *)
{
    D_D(DWaterProgress);
    QPainter p(this);
    d->paint(&p);
}

void DWaterProgressPrivate::resizePixmap(QSize sz)
{
    // resize water;
    auto waterWidth = 500  * sz.width() / 100;
    auto waterHeight = 110  * sz.height() / 100;
    auto waterSize = QSizeF(waterWidth, waterHeight).toSize();

    if (waterFrontImage.size() != waterSize) {
        DSvgRenderer renderer(QString(":/images/light/images/water_front.svg"));
        QImage image(waterWidth, waterHeight, QImage::Format_ARGB32);
        image.fill(Qt::transparent);  // partly transparent red-ish background
        QPainter waterPainter(&image);
        renderer.render(&waterPainter);
        waterFrontImage = image;
    }
    if (waterBackImage.size() != waterSize) {
        DSvgRenderer renderer(QString(":/images/light/images/water_back.svg"));
        QImage image(waterWidth, waterHeight, QImage::Format_ARGB32);
        image.fill(Qt::transparent);  // partly transparent red-ish background
        QPainter waterPainter(&image);
        renderer.render(&waterPainter);
        waterBackImage = image;
    }
}

void DWaterProgressPrivate::initUI()
{
    D_Q(DWaterProgress);
    q->setMinimumSize(100, 100);

    value = 0;

    timer = new QTimer(q);
    timer->setInterval(interval);
    resizePixmap(q->size());
    frontXOffset = q->width();
    backXOffset = 0;

    q->connect(timer, &QTimer::timeout, q, [ = ] {
        // interval can not be zero, and limit to 1
        interval = (interval < 1) ? 1 : interval;

        // move 60% per second
        double frontXDeta = 40.0 / (1000.0 / interval);
        // move 90% per second
        double backXDeta = 60.0 / (1000.0 / interval);
        frontXOffset -= frontXDeta * q->width() / 100;
        backXOffset += backXDeta * q->width() / 100;

        if (frontXOffset > q->width())
        {
            frontXOffset = q->width();
        }
        if (frontXOffset < - (waterFrontImage.width() - q->width()))
        {
            frontXOffset = q->width();
        }

        if (backXOffset > waterBackImage.width())
        {
            backXOffset = 0;
        }

        // update pop
        // move 25% per second default
        double speed = 25 / (1000.0 / interval) /** 100 / q->height()*/;
        for (auto &pop : pops)
        {
            // yOffset 0 ~ 100;
            pop.yOffset += speed * pop.ySpeed;
            if (pop.yOffset < 0) {
            }
            if (pop.yOffset > value) {
                pop.yOffset = 0;
            }
            pop.xOffset = qSin((pop.yOffset / 100) * 2 * 3.14) * 18 * pop.xSpeed + 50;
        }
        q->update();
    });
}

void DWaterProgressPrivate::setValue(int v)
{
    value = v;
    progressText = QString("%1%").arg(v);
}

void DWaterProgressPrivate::paint(QPainter *p)
{
    D_Q(DWaterProgress);
    p->setRenderHint(QPainter::Antialiasing);

    auto rect = q->rect();
    auto sz = q->size();
    resizePixmap(sz);

    int yOffset = rect.topLeft().y() + (100 - value - 10)  * sz.height() / 100;

    // draw water
    QImage waterImage = QImage(sz, QImage::Format_ARGB32_Premultiplied);
    QPainter waterPinter(&waterImage);
    waterPinter.setCompositionMode(QPainter::CompositionMode_Source);
    waterPinter.fillRect(waterImage.rect(), QColor(26, 27, 27, 255 / 10));
    waterPinter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    waterPinter.drawImage(static_cast<int>(backXOffset), yOffset, waterBackImage);
    waterPinter.drawImage(static_cast<int>(backXOffset) - waterBackImage.width(), yOffset, waterBackImage);
    waterPinter.drawImage(static_cast<int>(frontXOffset), yOffset, waterFrontImage);
    waterPinter.drawImage(static_cast<int>(frontXOffset) - waterFrontImage.width(), yOffset, waterFrontImage);

    //drwa pop
    if (value > 30) {
        for (auto &pop : pops) {
            QPainterPath popPath;
            popPath.addEllipse(pop.xOffset * sz.width() / 100, (100 - pop.yOffset) * sz.height() / 100,
                               pop.size * sz.width() / 100, pop.size * sz.height() / 100);
            waterPinter.fillPath(popPath, QColor(77, 208, 255));
        }
    }
    auto font = waterPinter.font();
    font.setPixelSize(sz.height() * 20 / 100);
    waterPinter.setFont(font);
    waterPinter.setPen(Qt::white);
    waterPinter.drawText(rect, Qt::AlignCenter, progressText);
    waterPinter.end();

    QPixmap maskPixmap(sz);
    maskPixmap.fill(Qt::transparent);
    QPainterPath path;
    path.addEllipse(QRectF(0, 0, sz.width(), sz.height()));
    QPainter maskPainter(&maskPixmap);
    maskPainter.setRenderHint(QPainter::HighQualityAntialiasing);
    maskPainter.setPen(QPen(Qt::white, 1));
    maskPainter.fillPath(path, QBrush(Qt::white));

    QPainter::CompositionMode mode = QPainter::CompositionMode_SourceIn;
    QImage contentImage = QImage(sz, QImage::Format_ARGB32_Premultiplied);
    QPainter contentPainter(&contentImage);
    contentPainter.setCompositionMode(QPainter::CompositionMode_Source);
    contentPainter.fillRect(contentImage.rect(), Qt::transparent);
    contentPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    contentPainter.drawImage(0, 0, maskPixmap.toImage());
    contentPainter.setCompositionMode(mode);
    contentPainter.drawImage(0, 0, waterImage);
    contentPainter.setCompositionMode(QPainter::CompositionMode_DestinationOver);
    contentPainter.end();

    p->drawImage(0, 0, contentImage);

}

DWIDGET_END_NAMESPACE
