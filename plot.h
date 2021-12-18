#ifndef PLOT_H
#define PLOT_H

#include <QtCharts/QtCharts>

class Plot : public QChartView
{
    Q_OBJECT

public:
    Plot(QWidget *parent = 0);
    Plot(QString title, QWidget *parent = 0);
    ~Plot();
    void appendPointF(const QPointF& pointf);
    void plot(const QList<double> x, QList<double>y);
    void clear(void);
    void setUseOpenGL(bool enable = true);

    void draw3dbVLine();
    double getXmin() const;
    double getXmax() const;
    QLineSeries* series() const;

protected:
    void initializeScales();
    void initializeChart(const QString& title = nullptr);
    void initializeCurve();
    void resizeEvent(QResizeEvent *event);
    bool viewportEvent(QEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    double curveXfromY(double y);
    double curveYfromX(double x);
    void drawVLine(QPoint pos);
private:
    QLineSeries *curve;
    QChart *chart;
    double xmin;
    double xmax;
    QGraphicsSimpleTextItem *pointerLabel;
    QGraphicsLineItem *vLine;
    QGraphicsLineItem *vLine3db;
    QGraphicsSimpleTextItem *vLabel3db;
};

#endif // PLOT_H
