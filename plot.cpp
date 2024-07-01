#include "plot.h"
#include <QtCharts/QtCharts>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
using namespace QtCharts;
#endif

Plot::Plot(QWidget *parent) :
    QChartView(parent),
    xmin(10.0),
    xmax(1000.0),
    vLine(nullptr),
    vLine3db(nullptr),
    vLabel3db(nullptr)
{
    setCursor(Qt::CrossCursor);
    initializeChart();
    initializeCurve();
    initializeScales();
}

Plot::Plot(QString title, QWidget *parent) :
    QChartView(parent),
    xmin(10.0),
    xmax(1000.0),
    vLine(nullptr),
    vLine3db(nullptr),
    vLabel3db(nullptr)
{
    setCursor(Qt::CrossCursor);
    initializeChart(title);
    initializeCurve();
    initializeScales();
}

Plot::~Plot()
{
    curve->clear();
    delete curve;
    delete chart;
    delete vLine;
    delete vLine3db;
    delete vLabel3db;
}

void Plot::appendPointF(const QPointF &pointf)
{
    curve->append(pointf);
}

void Plot::plot(const QList<double> x, QList<double> y)
{
    QList<QPointF> samples;
    for (int i = 0; i < x.size(); i++) {
        QPointF p(x[i], y[i]);
        curve->append(p);
    }
}

void Plot::clear()
{
    curve->clear();
}

void Plot::setUseOpenGL(bool enable)
{
    curve->setUseOpenGL(enable);
}

void Plot::initializeCurve() {
    curve = new QLineSeries(chart);
    chart->addSeries(curve);
}


void Plot::draw3dbVLine()
{
    double x = curveXfromY(-3.0);
    double y1 = 10.0, y2 = -40.0;

    QPointF value1(x, y1);
    QPointF point1 = chart->mapToPosition(value1);
    QPointF value2 = QPointF(x, y2);
    QPointF point2 = chart->mapToPosition(value2);
    QLineF line (point1, point2);
    if (vLine3db) {
        this->scene()->removeItem(vLine3db);
        delete vLine3db;
    }
    vLine3db = this->scene()->addLine(line, QPen(Qt::DashLine));

    if (vLabel3db) {
        this->scene()->removeItem(vLabel3db);
        delete vLabel3db;
    }
    QString label = QString(tr("%1 dB at %2 Hz")).arg(-3.0).arg(x);
    vLabel3db = this->scene()->addSimpleText(label);
    QPointF pos = chart->mapToPosition(value1);
    vLabel3db->setPos(pos);
}

void Plot::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);
    draw3dbVLine();
}

bool Plot::viewportEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::ToolTip:
            return false;
        default:
            break;
    }

    return QChartView::viewportEvent(event);
}

void Plot::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() != Qt::LeftButton)
        return;

    QPoint pos = event->pos();
    QPointF val = chart->mapToValue(pos);
    double y = curveYfromX(val.x());
    QString label = QString(tr("%1 dB at %2 Hz")).arg(y).arg(val.x());

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QToolTip::showText(event->globalPos(), label, this, QRect(), 10000);
#else
    QToolTip::showText(event->globalPosition().toPoint(), label, this, QRect(), 10000);
#endif

    drawVLine(pos);

    event->accept();
}

void Plot::mousePressEvent(QMouseEvent *event)
{

    if (event->buttons() != Qt::LeftButton)
        return;

    QPoint pos = event->pos();

    QPointF val = chart->mapToValue(pos);
    double y = curveYfromX(val.x());
    QString label = QString(tr("%1 dB at %2 Hz")).arg(round(y)).arg(round(val.x()));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QToolTip::showText(event->globalPos(), label, this, QRect(), 10000);
#else
    QToolTip::showText(event->globalPosition().toPoint(), label, this, QRect(), 10000);
#endif
    drawVLine(pos);

    event->accept();
}

void Plot::mouseReleaseEvent(QMouseEvent *event)
{
    if (vLine) {
        this->scene()->removeItem(vLine);
        delete vLine;
        vLine = nullptr;
    }

    event->accept();
}

double Plot::curveXfromY(double y)
{
    double x = 1.0;
    QList<QPointF> list = curve->points();
    for (int i = 0; i < list.size() - 1; i++) {
        QPointF p1 = list.at(i);
        QPointF p2 = list.at(i+1);
        if (p1.y() <= y && p2.y() > y) {
            x = p1.x();
            break;
        }
    }

    return x;
}

double Plot::curveYfromX(double x)
{

    double y = 0.0;
    QList<QPointF> list = curve->points();
    for (int i = 0; i < list.size() - 1; i++) {
        QPointF p1 = list.at(i);
        QPointF p2 = list.at(i+1);
        if (p1.x() <= x && p2.x() > x) {
            y = p1.y();
            break;
        }
    }

    return y;
}

void Plot::drawVLine(QPoint pos)
{
    QPointF val = chart->mapToValue(pos);

    /* redraw dashed vertical line */
    double x = val.x();
    QValueAxis* yaxis0 = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical, curve).at(0));
    double ymin = yaxis0->min();
    double ymax = yaxis0->max();
    QPointF value1(x, ymin);
    QPointF point1 = chart->mapToPosition(value1);
    QPointF value2(x, ymax);
    QPointF point2 = chart->mapToPosition(value2);
    QLineF line (point1, point2);
    if (vLine) {
        this->scene()->removeItem(vLine);
        delete vLine;
    }
    vLine = this->scene()->addLine(line, QPen(Qt::DashLine));
}

double Plot::getXmax() const
{
    return xmax;
}

QLineSeries* Plot::series() const
{
   return curve;
}

double Plot::getXmin() const
{
    return xmin;
}

void Plot::initializeChart(const QString& title)
{
    chart = new QChart();
    chart->setTitle(title);
    chart->legend()->hide();
    this->setChart(chart);
    this->setRenderHint(QPainter::Antialiasing);
}

void Plot::initializeScales()
{
    QLogValueAxis *XAxis = new QLogValueAxis();
    XAxis->setBase(10.0);
    XAxis->setLabelFormat("%g");
    XAxis->setTitleText(tr("Frequency [Hz]"));
    XAxis->setMax(xmax);
    XAxis->setMin(xmin);
    XAxis->setMinorTickCount(8);
    chart->addAxis(XAxis, Qt::AlignBottom);
    curve->attachAxis(XAxis);

    QValueAxis *YAxis = new QValueAxis();
    YAxis->setTitleText(tr("Sound pressure [dB]"));
    YAxis->setLabelFormat("%g");
    YAxis->setMax(10);
    YAxis->setMin(-40);
    YAxis->setTickCount(6);
    YAxis->setMinorTickCount(1);
    chart->addAxis(YAxis, Qt::AlignLeft);
    curve->attachAxis(YAxis);
}
