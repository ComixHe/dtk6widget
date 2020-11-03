#include "dprintpreviewwidget.h"
#include "private/dprintpreviewwidget_p.h"
#include <QVBoxLayout>
#include <private/qprinter_p.h>
#include <QPicture>
#include <QFileInfo>

#define FIRST_PAGE 1
#define FIRST_INDEX 0

DWIDGET_BEGIN_NAMESPACE

DPrintPreviewWidgetPrivate::DPrintPreviewWidgetPrivate(DPrintPreviewWidget *qq)
    : DFramePrivate(qq)
    , imposition(DPrintPreviewWidget::One)
    , order(DPrintPreviewWidget::L2R_T2B)
    , refreshMode(DPrintPreviewWidgetPrivate::RefreshImmediately)
{
}

void DPrintPreviewWidgetPrivate::init()
{
    Q_Q(DPrintPreviewWidget);

    graphicsView = new GraphicsView;
    graphicsView->setInteractive(true);
    graphicsView->setDragMode(QGraphicsView::NoDrag);
    graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    graphicsView->setLineWidth(0);

    scene = new QGraphicsScene(graphicsView);
    q->themeTypeChanged(DGuiApplicationHelper::instance()->themeType());
    graphicsView->setScene(scene);

    background = new QGraphicsRectItem();
    background->setZValue(-1);
    scene->addItem(background);
    waterMark = new WaterMark();
    scene->addItem(waterMark);
    waterMark->setZValue(0);

    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->addWidget(graphicsView);

    colorMode = previewPrinter->colorMode();
}

void DPrintPreviewWidgetPrivate::populateScene()
{
    QSize paperSize = previewPrinter->pageLayout().fullRectPixels(previewPrinter->resolution()).size();
    QRect pageRect = previewPrinter->pageLayout().paintRectPixels(previewPrinter->resolution());

    QRectF rect(pageRect);
    background->setRect(rect);
    background->setBrush(Qt::white);

    int page = 1;
    //todo 多页显示接口添加
    for (int i = 0; i < pictures.size(); i++) {
        PageItem *item = new PageItem(page++, pictures[i], paperSize, pageRect);
        item->setVisible(false);
        scene->addItem(item);
        pages.append(item);
    }

    if (!pages.isEmpty()) {
        if (currentPageNumber == 0)
            currentPageNumber = FIRST_PAGE;
        setCurrentPage(currentPageNumber);
    }
}

void DPrintPreviewWidgetPrivate::generatePreview()
{
    if (refreshMode == RefreshDelay)
        return;

    for (auto *page : qAsConst(pages))
        scene->removeItem(page);
    qDeleteAll(pages);
    pages.clear();

    Q_Q(DPrintPreviewWidget);
    previewPrinter->setPreviewMode(true);
    Q_EMIT q->paintRequested(previewPrinter);
    previewPrinter->setPreviewMode(false);
    pictures = previewPrinter->getPrinterPages();
    setPageRangeAll();
    populateScene();
    impositionPages();
    scene->setSceneRect(scene->itemsBoundingRect());
    fitView();
}

void DPrintPreviewWidgetPrivate::fitView()
{
    QRectF target = scene->sceneRect();
    graphicsView->fitInView(target, Qt::KeepAspectRatio);
    graphicsView->resetScale();
}

void DPrintPreviewWidgetPrivate::print(bool printAsPicture)
{
    QRect pageRect = previewPrinter->pageRect();
    QSize paperSize = previewPrinter->pageLayout().fullRectPixels(previewPrinter->resolution()).size();
    QMargins pageMargins = previewPrinter->pageLayout().marginsPixels(previewPrinter->resolution());
    QImage savedImages(paperSize, QImage::Format_ARGB32);
    QString outPutFileName = previewPrinter->outputFileName();
    QString suffix = QFileInfo(outPutFileName).suffix();
    bool isJpegImage = !suffix.compare(QLatin1String("jpeg"), Qt::CaseInsensitive);
    QPainter painter;

    if (printAsPicture) {
        painter.begin(&savedImages);
        painter.setClipRect(0, 0, paperSize.width(), paperSize.height());
    } else {
        painter.begin(previewPrinter);
        painter.setClipRect(0, 0, pageRect.width(), pageRect.height());
    }

    savedImages.fill(Qt::white);
    painter.scale(scale, scale);
    QPointF leftTopPoint;
    double lt_x, lt_y;
    if (scale >= 1.0) {
        lt_x = printAsPicture ? pageMargins.left() : 0.0;
        lt_y = printAsPicture ? pageMargins.top() : 0.0;
    } else {
        lt_x = printAsPicture ? (paperSize.width() * (1.0 - scale) / (2.0 * scale) + pageMargins.left())
                              : (pageRect.width() * (1.0 - scale) / (2.0 * scale));
        lt_y = printAsPicture ? (paperSize.height() * (1.0 - scale) / (2.0 * scale) + pageMargins.top())
                              : (pageRect.height() * (1.0 - scale) / (2.0 * scale));
    }

    leftTopPoint.setX(lt_x);
    leftTopPoint.setY(lt_y);

    QVector<int> pageVector;
    if (pageRangeMode == DPrintPreviewWidget::CurrentPage)
        pageVector.append(pageRange.at(currentPageNumber - 1));
    else {
        pageVector = pageRange;
    }
    for (int i = 0; i < pageVector.size(); i++) {
        if (0 != i && !printAsPicture)
            previewPrinter->newPage();

        painter.save();
        //todo scale,black and white,watermarking,……
        painter.drawPicture(leftTopPoint, *(pictures[pageVector.at(i) - 1]));
        painter.restore();

        if (printAsPicture) {
            // write image
            QString stres = outPutFileName.right(suffix.length() + 1);
            QString tmpString = QString(outPutFileName).remove(stres) + QString("(%1)").arg(QString::number(i + 1)) + stres;

            savedImages.save(tmpString, isJpegImage ? "JPEG" : "PNG");
            savedImages.fill(Qt::white);
        }
    }

    painter.end();
}

void DPrintPreviewWidgetPrivate::setPageRangeAll()
{
    int size = pictures.count();
    size = targetPage(size);
    pageRange.clear();
    for (int i = FIRST_PAGE; i <= size; i++) {
        pageRange.append(i);
    }
    Q_Q(DPrintPreviewWidget);
    Q_EMIT q->totalPages(size);

    reviewChanged = false;
}

int DPrintPreviewWidgetPrivate::pagesCount()
{
    return pageRange.size();
}

void DPrintPreviewWidgetPrivate::setCurrentPage(int page)
{
    int pageCount = pagesCount();
    if (page < FIRST_PAGE)
        return;
    if (page > pageCount)
        page = pageCount;
    int pageNumber = index2page(page - 1);
    if (pageNumber < 0)
        return;
    int lastPage = index2page(currentPageNumber - 1);
    if (lastPage > -1) {
        if (lastPage > pages.size())
            pages.back()->setVisible(false);
        else
            pages.at(lastPage - 1)->setVisible(false);
    }
    currentPageNumber = page;
    if (pageNumber > pages.size())
        return;
    pages.at(pageNumber - 1)->setVisible(true);

    Q_Q(DPrintPreviewWidget);
    Q_EMIT q->currentPageChanged(page);
}

int DPrintPreviewWidgetPrivate::targetPage(int page)
{
    int mod = 0;
    switch (imposition) {
    case DPrintPreviewWidget::Imposition::One:
        break;
    case DPrintPreviewWidget::Imposition::OneRowTwoCol:
        mod = page % 2;
        break;
    case DPrintPreviewWidget::Imposition::TwoRowTwoCol:
        mod = page % 4;
        break;
    case DPrintPreviewWidget::Imposition::TwoRowThreeCol:
        mod = page % 6;
        break;
    case DPrintPreviewWidget::Imposition::ThreeRowThreeCol:
        mod = page % 9;
        break;
    case DPrintPreviewWidget::Imposition::FourRowFourCol:
        mod = page % 16;
        break;
    default:
        break;
    }
    if (mod)
        page += 1;
    return page;
}

int DPrintPreviewWidgetPrivate::index2page(int index)
{
    if (index < 0 || index >= pageRange.size())
        return -1;
    return pageRange.at(index);
}

int DPrintPreviewWidgetPrivate::page2index(int page)
{
    return pageRange.indexOf(page);
}

void DPrintPreviewWidgetPrivate::impositionPages()
{
    QSize paperSize = previewPrinter->pageLayout().fullRectPixels(previewPrinter->resolution()).size();
    QRect pageRect = previewPrinter->pageLayout().paintRectPixels(previewPrinter->resolution());

    switch (imposition) {
    case DPrintPreviewWidget::Imposition::One:
    {
        for (int i = 0, page = 1; i < pageRange.count(); i++) {
            PageItem *item = new PageItem(page++, pictures[i], paperSize, pageRect);
            item->setVisible(false);
            scene->addItem(item);
            pages.append(item);
        }
        if (!pages.isEmpty()) {
            if (currentPageNumber == 0)
                currentPageNumber = FIRST_PAGE;
            setCurrentPage(currentPageNumber);
        }
    }
        break;
    case DPrintPreviewWidget::Imposition::OneRowTwoCol:
        break;
    case DPrintPreviewWidget::Imposition::TwoRowTwoCol:
        break;
    case DPrintPreviewWidget::Imposition::TwoRowThreeCol:
        break;
    case DPrintPreviewWidget::Imposition::ThreeRowThreeCol:
        break;
    case DPrintPreviewWidget::Imposition::FourRowFourCol:
        break;
    default:
        break;
    }
}

DPrintPreviewWidget::DPrintPreviewWidget(DPrinter *printer, QWidget *parent)
    : DFrame(*new DPrintPreviewWidgetPrivate(this))
{
    Q_D(DPrintPreviewWidget);
    d->previewPrinter = printer;
    d->init();
}

void DPrintPreviewWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    if (visible) {
        updatePreview();
    }
}

void DPrintPreviewWidget::setPageRangeALL()
{
    Q_D(DPrintPreviewWidget);
    d->setPageRangeAll();
    if (!d->pageRange.isEmpty())
        d->setCurrentPage(FIRST_PAGE);
}

void DPrintPreviewWidget::setReGenerate(bool generate)
{
    reviewChange(generate);
}

void DPrintPreviewWidget::setPageRangeMode(PageRange mode)
{
    Q_D(DPrintPreviewWidget);
    d->pageRangeMode = mode;
}

DPrintPreviewWidget::PageRange DPrintPreviewWidget::pageRangeMode()
{
    Q_D(DPrintPreviewWidget);
    return d->pageRangeMode;
}

void DPrintPreviewWidget::reviewChange(bool generate)
{
    Q_D(DPrintPreviewWidget);
    d->reviewChanged = generate;
    d->impositionPages();
}

void DPrintPreviewWidget::setPageRange(const QVector<int> &rangePages)
{
    Q_D(DPrintPreviewWidget);
    int currentPage = d->index2page(d->currentPageNumber - 1);
    if (currentPage > 0) {
        d->pages.at(currentPage - 1)->setVisible(false);
    }
    d->pageRange = rangePages;
    Q_EMIT pagesCountChanged(d->pagesCount());
    d->setCurrentPage(d->currentPageNumber);
}

void DPrintPreviewWidget::setPageRange(int from, int to)
{
    Q_D(DPrintPreviewWidget);
    if (from > to)
        return;
    int currentPage = d->index2page(d->currentPageNumber - 1);
    if (currentPage > 0) {
        d->pages.at(currentPage - 1)->setVisible(false);
    }
    d->pageRange.clear();
    for (int i = from; i <= to; i++)
        d->pageRange.append(i);
    Q_EMIT pagesCountChanged(d->pagesCount());
    d->setCurrentPage(d->currentPageNumber);
}

int DPrintPreviewWidget::pagesCount()
{
    Q_D(DPrintPreviewWidget);
    return d->pagesCount();
}

int DPrintPreviewWidget::currentPage()
{
    Q_D(DPrintPreviewWidget);
    return d->index2page(d->currentPageNumber - 1);
}

bool DPrintPreviewWidget::turnPageAble()
{
    Q_D(DPrintPreviewWidget);
    return pagesCount() > 1;
}

void DPrintPreviewWidget::setColorMode(const QPrinter::ColorMode &colorMode)
{
    Q_D(DPrintPreviewWidget);

    d->colorMode = colorMode;
    d->previewPrinter->setColorMode(colorMode);
    int page = d->index2page(d->currentPageNumber - 1);
    if (page > 0) {
        d->pages.at(page - 1)->update();
        d->graphicsView->resetScale(false);
    }
}

void DPrintPreviewWidget::setOrientation(const QPrinter::Orientation &pageOrientation)
{
    Q_D(DPrintPreviewWidget);

    d->previewPrinter->setOrientation(pageOrientation);
    reviewChange(true);
    d->generatePreview();
}

DPrinter::ColorMode DPrintPreviewWidget::getColorMode()
{
    Q_D(DPrintPreviewWidget);
    return d->colorMode;
}

void DPrintPreviewWidget::setScale(qreal scale)
{
    Q_D(DPrintPreviewWidget);
    d->scale = scale;
}

qreal DPrintPreviewWidget::getScale() const
{
    D_DC(DPrintPreviewWidget);
    return d->scale;
}

void DPrintPreviewWidget::updateView()
{
    Q_D(DPrintPreviewWidget);
    if (d->currentPageNumber < 0 || d->currentPageNumber > d->pages.count() || d->pages.empty())
        return;
    d->pages.at(d->currentPageNumber - 1)->update();
    d->graphicsView->resetScale(false);
}

void DPrintPreviewWidget::refreshBegin()
{
    Q_D(DPrintPreviewWidget);

    d->refreshMode = DPrintPreviewWidgetPrivate::RefreshDelay;
}

void DPrintPreviewWidget::refreshEnd()
{
    Q_D(DPrintPreviewWidget);

    d->refreshMode = DPrintPreviewWidgetPrivate::RefreshImmediately;
    updatePreview();
}

void DPrintPreviewWidget::setWaterMarkType(int type)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setType(static_cast<WaterMark::Type>(type));
}

void DPrintPreviewWidget::setWaterMargImage(const QImage &image)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setImage(image);
}

void DPrintPreviewWidget::setWaterMarkRotate(qreal rotate)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setRotation(rotate);
}

void DPrintPreviewWidget::setWaterMarkScale(qreal scale)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setImageScale(scale);
}

void DPrintPreviewWidget::setWaterMarkOpacity(qreal opacity)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setOpacity(opacity);
}

void DPrintPreviewWidget::setConfidentialWaterMark()
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setText(qApp->translate("DPrintPreviewWidget", "Confidential"));
}

void DPrintPreviewWidget::setDraftWaterMark()
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setText(qApp->translate("DPrintPreviewWidget", "Draft"));
}

void DPrintPreviewWidget::setSampleWaterMark()
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setText(qApp->translate("DPrintPreviewWidget", "Sample"));
}

void DPrintPreviewWidget::setCustomWaterMark(const QString &text)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setText(text);
}

void DPrintPreviewWidget::setWaterMarkFont(const QFont &font)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setFont(font);
}

void DPrintPreviewWidget::setWaterMarkColor(const QColor &color)
{
    Q_D(DPrintPreviewWidget);

    d->waterMark->setColor(color);
}

void DPrintPreviewWidget::setImposition(Imposition im)
{
    Q_D(DPrintPreviewWidget);
    d->imposition = im;
    d->impositionPages();
}

void DPrintPreviewWidget::setOrder(Order order)
{
    Q_D(DPrintPreviewWidget);
    d->order = order;
    d->impositionPages();
}

void DPrintPreviewWidget::updatePreview()
{
    Q_D(DPrintPreviewWidget);
    reviewChange(true);
    d->generatePreview();
    d->graphicsView->updateGeometry();
}

void DPrintPreviewWidget::turnFront()
{
    Q_D(DPrintPreviewWidget);
    if (d->currentPageNumber < 2)
        return;
    setCurrentPage(d->currentPageNumber - 1);
}

void DPrintPreviewWidget::turnBack()
{
    Q_D(DPrintPreviewWidget);
    if (d->currentPageNumber >= d->pagesCount())
        return;
    setCurrentPage(d->currentPageNumber + 1);
}

void DPrintPreviewWidget::turnBegin()
{
    Q_D(DPrintPreviewWidget);
    if (d->pageRange.isEmpty())
        return;
    setCurrentPage(FIRST_PAGE);
}

void DPrintPreviewWidget::turnEnd()
{
    Q_D(DPrintPreviewWidget);
    if (d->pageRange.isEmpty())
        return;
    setCurrentPage(d->pageRange.size());
}

void DPrintPreviewWidget::setCurrentPage(int page)
{
    Q_D(DPrintPreviewWidget);
    d->setCurrentPage(page);
}

void DPrintPreviewWidget::print(bool isSavedPicture)
{
    Q_D(DPrintPreviewWidget);
    d->print(isSavedPicture);
}

void DPrintPreviewWidget::themeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    Q_D(DPrintPreviewWidget);
    if (DGuiApplicationHelper::DarkType == themeType)
        d->scene->setBackgroundBrush(QColor(0, 0, 0, 3));
    else
        d->scene->setBackgroundBrush(QColor(255, 255, 255, 5));
}

DPrinter::DPrinter(QPrinter::PrinterMode mode)
    : QPrinter(mode)
{
}

void DPrinter::setPreviewMode(bool isPreview)
{
    d_ptr->setPreviewMode(isPreview);
}

QList<const QPicture *> DPrinter::getPrinterPages()
{
    return d_ptr->previewPages();
}

void PageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    QRectF paperRect(0, 0, paperSize.width(), paperSize.height());

    painter->setClipRect(paperRect & option->exposedRect);
    //painter->fillRect(paperRect, Qt::white);

    if (!pagePicture)
        return;

    content->setRect(QRectF(pageRect.topLeft(), pageRect.size()));
    content->update();
}

void ContentItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    Q_UNUSED(widget);
    painter->setClipRect(brect & item->exposedRect);

    DPrintPreviewWidget *pwidget = qobject_cast<DPrintPreviewWidget *>(scene()->parent()->parent());
    qreal scale = pwidget->getScale();
    painter->scale(scale, scale);

    QPointF leftTopPoint;
    if (scale >= 1.0) {
        leftTopPoint = QPointF(0, 0);
    } else {
        leftTopPoint.setX(((pageRect.width() * (1.0 - scale) / 2.0)) / scale);
        leftTopPoint.setY(((pageRect.height() * (1.0 - scale) / 2.0)) / scale);
    }

    if (pwidget && (pwidget->getColorMode() == QPrinter::GrayScale)) {
        // 图像灰度处理
        painter->drawPicture(leftTopPoint, grayscalePaint(*pagePicture));
    } else if (pwidget && (pwidget->getColorMode() == QPrinter::Color)) {
        painter->drawPicture(leftTopPoint, *pagePicture);
    }
}

QPicture ContentItem::grayscalePaint(const QPicture &picture)
{
    QImage image(pageRect.size(), QImage::Format_ARGB32);
    QPainter imageP;

    image.fill(Qt::transparent);
    imageP.begin(&image);
    imageP.drawPicture(0, 0, picture);
    imageP.end();

    image = imageGrayscale(&image);

    QPicture temp;
    QPainter tempP;

    tempP.begin(&temp);
    tempP.drawImage(0, 0, image);
    tempP.end();

    return temp;
}

QImage ContentItem::imageGrayscale(const QImage *origin)
{
    int w = origin->width();
    int h = origin->height();
    QImage iGray(w, h, QImage::Format_ARGB32);

    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            QRgb pixel = origin->pixel(i, j);
            int gray = qGray(pixel);
            QColor color(gray, gray, gray, qAlpha(pixel));
            iGray.setPixel(i, j, color.rgba());
        }
    }

    return iGray;
}

void WaterMark::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget)
{
    Q_UNUSED(item);
    Q_UNUSED(widget);
    switch (type) {
    case Type::None:
        return;
    case Type::Text: {
    }
        return;
    case Type::Image: {
        QPolygonF brectPol = mapFromScene(brectPolygon);
        QPolygonF twopol = mapFromScene(twoPolygon);
        QPainterPath path;
        path.addPolygon(twopol);
        path.addPolygon(brectPol);
        path.addPolygon(twopol);
        painter->setClipPath(path, Qt::IntersectClip);
        QImage img = sourceImage.scaledToWidth(sourceImage.width() * imageScale);
        QSize size = img.size() / img.devicePixelRatio();
        int imgWidth = size.width();
        int imgHeight = size.height();
        int space = qMin(imgWidth, imgHeight);
        if (layout == Center) {
            QPointF leftTop(brect.center().x() - imgWidth / 2.0, brect.center().y() - imgHeight / 2.0);
            painter->drawImage(leftTop, img);
            return;
        }

        // targetRectf,需要绘制水印的范围，取高宽两倍于brect
        QRectF targetRectf = QRectF(brect.x() - brect.width() / 2, brect.y() - brect.height() / 2, brect.width() * 2, brect.height() * 2);
        QPointF leftTop = targetRectf.topLeft();
        qreal colStart = leftTop.x();
        for (int row = 0; targetRectf.contains(leftTop);) {
            leftTop += QPointF(row % 2 * space, 0);
            for (int col = 0; targetRectf.contains(leftTop); col++) {
                painter->drawImage(leftTop, img);
                leftTop += QPointF(imgWidth + space, 0);
            }
            row++;
            leftTop += QPointF(0, space + imgHeight);
            leftTop.setX(colStart);
        }
    }
    }
}

GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    scaleRatio = 1.0;

    scaleResetButton = new DIconButton(this);
    scaleResetButton->setFixedSize(36, 36);
    scaleResetButton->setIcon(QIcon::fromTheme("print_previewscale"));
    scaleResetButton->setIconSize(QSize(18, 18));
    scaleResetButton->setVisible(false);

    onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());

    connect(scaleResetButton, &DIconButton::clicked, this, [this]() {
        resetScale(false);
    });
}

void GraphicsView::resetScale(bool autoReset)
{
    if (!autoReset) {
        this->scale(1.0 / scaleRatio,
                    1.0 / scaleRatio);
    }

    scaleRatio = 1.0;
    scaleResetButton->setVisible(false);
}

void GraphicsView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() & Qt::LeftButton)
        this->setDragMode(QGraphicsView::ScrollHandDrag);

    QGraphicsView::mousePressEvent(e);
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() & Qt::LeftButton)
        this->setDragMode(QGraphicsView::NoDrag);

    QGraphicsView::mouseReleaseEvent(e);
}

void GraphicsView::wheelEvent(QWheelEvent *e)
{
    if (0 > e->angleDelta().y()) {
        if (scaleRatio * 100 > 10) {
            scale(PREVIEW_NARROW_RATIO, PREVIEW_NARROW_RATIO);
            scaleRatio *= PREVIEW_NARROW_RATIO;
            scaleResetButton->setVisible(true);
        }
    } else {
        if (scaleRatio * 100 < 200) {
            scale(PREVIEW_ENLARGE_RATIO, PREVIEW_ENLARGE_RATIO);
            scaleRatio *= PREVIEW_ENLARGE_RATIO;
            scaleResetButton->setVisible(true);
        }
    }

    if (qFuzzyCompare(scaleRatio, 1))
        scaleResetButton->setVisible(false);
}

void GraphicsView::resizeEvent(QResizeEvent *e)
{
    QGraphicsView::resizeEvent(e);
    scaleResetButton->move(this->width() - scaleResetButton->width() - PREVIEW_SCALEBUTTON_MARGIN,
                           PREVIEW_SCALEBUTTON_MARGIN);
    this->fitInView(scene()->sceneRect(), Qt::KeepAspectRatio);
    this->resetScale();
    Q_EMIT resized();
}

void GraphicsView::showEvent(QShowEvent *e)
{
    QGraphicsView::showEvent(e);
    scaleResetButton->move(this->width() - scaleResetButton->width() - PREVIEW_SCALEBUTTON_MARGIN,
                           PREVIEW_SCALEBUTTON_MARGIN);
    Q_EMIT resized();
}

void GraphicsView::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::PaletteChange:
        onThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());
        break;

    default:
        QGraphicsView::changeEvent(e);
    }
}

void GraphicsView::onThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    QPalette btnPalette = scaleResetButton->palette();
    if (themeType == DGuiApplicationHelper::LightType) {
        btnPalette.setColor(QPalette::Light, QColor(247, 247, 247, qRound(0.7 * 255)));
        btnPalette.setColor(QPalette::Dark, QColor(247, 247, 247, qRound(0.7 * 255)));
    } else if (themeType == DGuiApplicationHelper::DarkType) {
        btnPalette.setColor(QPalette::Light, QColor(32, 32, 32, qRound(0.5 * 255)));
        btnPalette.setColor(QPalette::Dark, QColor(32, 32, 32, qRound(0.5 * 255)));
    }

    scaleResetButton->setPalette(btnPalette);
}

DWIDGET_END_NAMESPACE
