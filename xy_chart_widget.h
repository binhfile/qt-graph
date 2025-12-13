#ifndef XY_CHART_WIDGET_H
#define XY_CHART_WIDGET_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QPen>
#include <QString>
#include <QMap>

struct YAxisInfo {
    int axisId;
    QString label;
    double min;
    double max;
    QColor color;
    Qt::AlignmentFlag side;
    QStringList seriesNames;
    bool autoScale;

    YAxisInfo() : axisId(0), min(0.0), max(10.0),
                  color(Qt::black), side(Qt::AlignLeft), autoScale(true) {}
};

struct DataSeries {
    QString name;
    QVector<QPointF> points;
    QColor color;
    QPen pen;
    bool visible;
    int yAxisId;

    DataSeries() : visible(true), yAxisId(0) {
        pen.setWidth(2);
    }
};

class XYChartWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit XYChartWidget(QWidget *parent = nullptr);
    ~XYChartWidget();
    
    // Thêm một series dữ liệu mới
    void addSeries(const QString &seriesName, const QColor &color = Qt::blue);
    
    // Thêm điểm vào series
    void addPoint(const QString &seriesName, double x, double y);
    void addPoint(const QString &seriesName, const QPointF &point);
    
    // Thêm nhiều điểm cùng lúc
    void addPoints(const QString &seriesName, const QVector<QPointF> &points);
    
    // Cập nhật toàn bộ dữ liệu của series
    void setSeriesData(const QString &seriesName, const QVector<QPointF> &points);
    
    // Xóa dữ liệu
    void clearSeries(const QString &seriesName);
    void clearAllSeries();
    
    // Xóa series
    void removeSeries(const QString &seriesName);
    
    // Hiển thị/ẩn series
    void setSeriesVisible(const QString &seriesName, bool visible);
    
    // Cài đặt màu sắc và độ dày đường
    void setSeriesColor(const QString &seriesName, const QColor &color);
    void setSeriesPenWidth(const QString &seriesName, int width);
    
    // Cài đặt giới hạn trục
    void setXRange(double min, double max);
    void setYRange(double min, double max);
    void setAutoScale(bool enable);
    
    // Cài đặt số lượng điểm tối đa (để giới hạn bộ nhớ cho realtime)
    void setMaxPointsPerSeries(int maxPoints);
    
    // Cài đặt lưới và nhãn
    void setGridVisible(bool visible);
    void setAxisLabelsVisible(bool visible);
    void setTitle(const QString &title);
    void setXLabel(const QString &label);
    void setYLabel(const QString &label);
    
    // Lấy thông tin
    QStringList getSeriesNames() const;
    int getSeriesCount() const;

    // Crosshair settings
    void setCrosshairVisible(bool visible);
    bool isCrosshairVisible() const;

    // Multi-axis support
    void setMultiAxisEnabled(bool enabled);
    bool isMultiAxisEnabled() const;
    void setAutoGroupSeries(bool enabled);
    bool isAutoGroupEnabled() const;
    void createYAxis(int axisId, const QString &label, Qt::AlignmentFlag side);
    void assignSeriesToAxis(const QString &seriesName, int axisId);
    int getAxisForSeries(const QString &seriesName) const;
    void setAxisRange(int axisId, double min, double max);
    void setAxisAutoScale(int axisId, bool enabled);

signals:
    void seriesClicked(const QString &seriesName, const QPointF &point);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    
private:
    // Dữ liệu
    QMap<QString, DataSeries> m_series;
    
    // Cài đặt hiển thị
    QString m_title;
    QString m_xLabel;
    QString m_yLabel;
    bool m_showGrid;
    bool m_showAxisLabels;
    bool m_autoScale;
    
    // Giới hạn trục
    double m_xMin, m_xMax;
    double m_yMin, m_yMax;
    
    // Giới hạn điểm
    int m_maxPointsPerSeries;
    
    // Margin cho vẽ
    int m_leftMargin;
    int m_rightMargin;
    int m_topMargin;
    int m_bottomMargin;
    
    // Hàm helper
    void calculateAutoScale();
    void calculateAutoScaleForAxis(int axisId);
    QPointF mapToWidget(const QPointF &dataPoint) const;
    QPointF mapToData(const QPointF &widgetPoint) const;
    QPointF mapToWidget(const QPointF &dataPoint, int axisId) const;
    QPointF mapToData(const QPointF &widgetPoint, int axisId) const;
    void drawGrid(QPainter &painter);
    void drawAxes(QPainter &painter);
    void drawYAxis(QPainter &painter, const YAxisInfo &axis);
    void drawSeries(QPainter &painter);
    void drawLegend(QPainter &painter);
    void drawCrosshair(QPainter &painter);
    void drawCrosshairTextBox(QPainter &painter, QPoint pos, const QStringList &lines);
    QColor getNextColor();
    void autoGroupSeriesToAxes();
    int calculateAxisPosition(const YAxisInfo &axis) const;
    int countAxesOnSide(Qt::AlignmentFlag side) const;
    void updateMargins();

    int m_colorIndex;

    // Multi-axis
    QMap<int, YAxisInfo> m_yAxes;
    int m_nextAxisId;
    bool m_multiAxisEnabled;
    bool m_autoGroupSeries;
    int m_maxYAxes;
    double m_groupingThreshold;
    int m_axisSpacing;

    // Crosshair
    bool m_crosshairVisible;
    QPoint m_crosshairPos;
};

#endif // XY_CHART_WIDGET_H
