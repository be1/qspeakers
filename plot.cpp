#include "plot.h"
#include <QtCharts/QtCharts>

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
using namespace QtCharts;
#endif

Plot::Plot(QWidget *parent) :
    QChartView(parent),
    xmin(10.0),
    xmax(1000.0),
    pointerLabel(nullptr),
    vLine(nullptr),
    vLabel(nullptr)
{
    initializeChart();
    initializeCurve();
    initializeScales();
}

Plot::Plot(QString title, QWidget *parent) :
    QChartView(parent),
    xmin(10.0),
    xmax(1000.0),
    pointerLabel(nullptr),
    vLine(nullptr),
    vLabel(nullptr)
{
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
    delete vLabel;
    delete pointerLabel;
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
    //curve->setUseOpenGL(true);
    chart->addSeries(curve);
}


void Plot::draw3dbVLine()
{
    double x = curveXfromY(-3.0);
    double y1 = 10.0, y2 = -40.0;

    QPointF value1(x, y1);
    QPointF point1 = chart->mapToPosition(value1);
    QPointF value2 = QPoint(x, y2);
    QPointF point2 = chart->mapToPosition(value2);
    QLineF line (point1, point2);
    if (vLine) {
        this->scene()->removeItem(vLine);
        delete vLine;
        vLine = nullptr;
    }
    vLine = this->scene()->addLine(line, QPen(Qt::DashLine));

    if (vLabel) {
        this->scene()->removeItem(vLabel);
        delete vLabel;
        vLabel = nullptr;
    }
    QString label = QString(tr("%1 dB at %2 Hz")).arg(-3.0).arg(round(x));
    vLabel = this->scene()->addSimpleText(label);
    QPointF pos = chart->mapToPosition(value1);
    vLabel->setPos(pos);
}

void Plot::resizeEvent(QResizeEvent *event)
{
    QChartView::resizeEvent(event);
    draw3dbVLine();
}

void Plot::mousePressEvent(QMouseEvent *event)
{
    if (!pointerLabel) {
        QPoint pos = event->pos();
        QPointF val = chart->mapToValue(pos);
        double y = curveYfromX(val.x());
        QString label = QString(tr("%1 dB at %2 Hz")).arg(round(y)).arg(round(val.x()));
        pointerLabel = this->scene()->addSimpleText(label);
        pointerLabel->setPos(pos.x() - pointerLabel->boundingRect().width(), pos.y() - pointerLabel->boundingRect().height());
        event->accept();
    }
}

void Plot::mouseMoveEvent(QMouseEvent *event)
{
    if (pointerLabel) {
        QPoint pos = event->pos();
        QPointF val = chart->mapToValue(pos);
        double y = curveYfromX(val.x());
        QString label = QString(tr("%1 dB at %2 Hz")).arg(round(y)).arg(round(val.x()));
        pointerLabel->setText(label);
        pointerLabel->setPos(pos.x() - pointerLabel->boundingRect().width(), pos.y() - pointerLabel->boundingRect().height());
        event->accept();
    }
}

void Plot::mouseReleaseEvent(QMouseEvent *event)
{
    if (pointerLabel) {
        this->scene()->removeItem(pointerLabel);
        delete pointerLabel;
        pointerLabel = nullptr;
        event->accept();
    }
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
