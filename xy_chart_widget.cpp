#include "xy_chart_widget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <QtMath>
#include <QDebug>
#include <limits>

XYChartWidget::XYChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_showGrid(true)
    , m_showAxisLabels(true)
    , m_autoScale(true)
    , m_xMin(0.0)
    , m_xMax(10.0)
    , m_yMin(0.0)
    , m_yMax(10.0)
    , m_maxPointsPerSeries(-1)
    , m_leftMargin(60)
    , m_rightMargin(20)
    , m_topMargin(40)
    , m_bottomMargin(50)
    , m_colorIndex(0)
    , m_crosshairVisible(true)
    , m_crosshairPos(-1, -1)
    , m_nextAxisId(1)
    , m_multiAxisEnabled(false)
    , m_autoGroupSeries(false)
    , m_maxYAxes(4)
    , m_groupingThreshold(0.3)
    , m_axisSpacing(60)
{
    setMinimumSize(400, 300);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setMouseTracking(true);

    // Create default axis 0
    YAxisInfo defaultAxis;
    defaultAxis.axisId = 0;
    defaultAxis.min = m_yMin;
    defaultAxis.max = m_yMax;
    defaultAxis.color = Qt::black;
    defaultAxis.side = Qt::AlignLeft;
    defaultAxis.autoScale = true;
    m_yAxes[0] = defaultAxis;
}

XYChartWidget::~XYChartWidget()
{
}

void XYChartWidget::addSeries(const QString &seriesName, const QColor &color)
{
    if (m_series.contains(seriesName)) {
        qWarning() << "Series" << seriesName << "already exists!";
        return;
    }

    DataSeries series;
    series.name = seriesName;
    series.yAxisId = 0;
    series.color = color.isValid() ? color : getNextColor();
    series.pen.setColor(series.color);
    series.pen.setWidth(2);

    m_series[seriesName] = series;

    // Add to default axis
    if (m_yAxes.contains(0)) {
        if (!m_yAxes[0].seriesNames.contains(seriesName)) {
            m_yAxes[0].seriesNames.append(seriesName);
        }
    }

    update();
}

void XYChartWidget::addPoint(const QString &seriesName, double x, double y)
{
    addPoint(seriesName, QPointF(x, y));
}

void XYChartWidget::addPoint(const QString &seriesName, const QPointF &point)
{
    if (!m_series.contains(seriesName)) {
        addSeries(seriesName);
    }
    
    m_series[seriesName].points.append(point);
    
    // Giới hạn số lượng điểm nếu được thiết lập
    if (m_maxPointsPerSeries > 0 && 
        m_series[seriesName].points.size() > m_maxPointsPerSeries) {
        m_series[seriesName].points.removeFirst();
    }
    
    if (m_autoScale) {
        calculateAutoScale();
    }
    
    update();
}

void XYChartWidget::addPoints(const QString &seriesName, const QVector<QPointF> &points)
{
    if (!m_series.contains(seriesName)) {
        addSeries(seriesName);
    }
    
    m_series[seriesName].points.append(points);
    
    // Giới hạn số lượng điểm
    if (m_maxPointsPerSeries > 0 && 
        m_series[seriesName].points.size() > m_maxPointsPerSeries) {
        int excess = m_series[seriesName].points.size() - m_maxPointsPerSeries;
        m_series[seriesName].points.remove(0, excess);
    }
    
    if (m_autoScale) {
        calculateAutoScale();
    }
    
    update();
}

void XYChartWidget::setSeriesData(const QString &seriesName, const QVector<QPointF> &points)
{
    if (!m_series.contains(seriesName)) {
        addSeries(seriesName);
    }
    
    m_series[seriesName].points = points;
    
    // Giới hạn số lượng điểm
    if (m_maxPointsPerSeries > 0 && 
        m_series[seriesName].points.size() > m_maxPointsPerSeries) {
        int excess = m_series[seriesName].points.size() - m_maxPointsPerSeries;
        m_series[seriesName].points.remove(0, excess);
    }
    
    if (m_autoScale) {
        calculateAutoScale();
    }
    
    update();
}

void XYChartWidget::clearSeries(const QString &seriesName)
{
    if (m_series.contains(seriesName)) {
        m_series[seriesName].points.clear();
        update();
    }
}

void XYChartWidget::clearAllSeries()
{
    for (auto &series : m_series) {
        series.points.clear();
    }
    update();
}

void XYChartWidget::removeSeries(const QString &seriesName)
{
    m_series.remove(seriesName);
    update();
}

void XYChartWidget::setSeriesVisible(const QString &seriesName, bool visible)
{
    if (m_series.contains(seriesName)) {
        m_series[seriesName].visible = visible;
        update();
    }
}

void XYChartWidget::setSeriesColor(const QString &seriesName, const QColor &color)
{
    if (m_series.contains(seriesName)) {
        m_series[seriesName].color = color;
        m_series[seriesName].pen.setColor(color);
        update();
    }
}

void XYChartWidget::setSeriesPenWidth(const QString &seriesName, int width)
{
    if (m_series.contains(seriesName)) {
        m_series[seriesName].pen.setWidth(width);
        update();
    }
}

void XYChartWidget::setXRange(double min, double max)
{
    m_xMin = min;
    m_xMax = max;
    m_autoScale = false;
    update();
}

void XYChartWidget::setYRange(double min, double max)
{
    m_yMin = min;
    m_yMax = max;
    m_autoScale = false;
    update();
}

void XYChartWidget::setAutoScale(bool enable)
{
    m_autoScale = enable;
    if (m_autoScale) {
        calculateAutoScale();
    }
    update();
}

void XYChartWidget::setMaxPointsPerSeries(int maxPoints)
{
    m_maxPointsPerSeries = maxPoints;
}

void XYChartWidget::setGridVisible(bool visible)
{
    m_showGrid = visible;
    update();
}

void XYChartWidget::setAxisLabelsVisible(bool visible)
{
    m_showAxisLabels = visible;
    update();
}

void XYChartWidget::setTitle(const QString &title)
{
    m_title = title;
    update();
}

void XYChartWidget::setXLabel(const QString &label)
{
    m_xLabel = label;
    update();
}

void XYChartWidget::setYLabel(const QString &label)
{
    m_yLabel = label;
    update();
}

QStringList XYChartWidget::getSeriesNames() const
{
    return m_series.keys();
}

int XYChartWidget::getSeriesCount() const
{
    return m_series.size();
}

void XYChartWidget::setCrosshairVisible(bool visible)
{
    m_crosshairVisible = visible;
    update();
}

bool XYChartWidget::isCrosshairVisible() const
{
    return m_crosshairVisible;
}

void XYChartWidget::setMultiAxisEnabled(bool enabled)
{
    m_multiAxisEnabled = enabled;
    if (enabled && m_autoGroupSeries) {
        autoGroupSeriesToAxes();
    }
    update();
}

bool XYChartWidget::isMultiAxisEnabled() const
{
    return m_multiAxisEnabled;
}

void XYChartWidget::setAutoGroupSeries(bool enabled)
{
    m_autoGroupSeries = enabled;
    if (m_multiAxisEnabled && enabled) {
        autoGroupSeriesToAxes();
    }
    update();
}

bool XYChartWidget::isAutoGroupEnabled() const
{
    return m_autoGroupSeries;
}

void XYChartWidget::createYAxis(int axisId, const QString &label, Qt::AlignmentFlag side)
{
    if (m_yAxes.contains(axisId)) {
        return;
    }

    YAxisInfo axis;
    axis.axisId = axisId;
    axis.label = label;
    axis.side = side;
    axis.color = Qt::black;  // Will be set when series is assigned
    axis.autoScale = true;
    axis.min = 0.0;
    axis.max = 10.0;

    m_yAxes[axisId] = axis;
    if (axisId >= m_nextAxisId) {
        m_nextAxisId = axisId + 1;
    }
    updateMargins();
    update();
}

void XYChartWidget::assignSeriesToAxis(const QString &seriesName, int axisId)
{
    if (!m_series.contains(seriesName)) {
        return;
    }

    int oldAxisId = m_series[seriesName].yAxisId;

    // Remove from old axis
    if (m_yAxes.contains(oldAxisId)) {
        m_yAxes[oldAxisId].seriesNames.removeAll(seriesName);
    }

    // Add to new axis
    m_series[seriesName].yAxisId = axisId;
    if (m_yAxes.contains(axisId)) {
        if (!m_yAxes[axisId].seriesNames.contains(seriesName)) {
            m_yAxes[axisId].seriesNames.append(seriesName);
        }

        // Set axis color to match the series color
        m_yAxes[axisId].color = m_series[seriesName].color;

        if (m_yAxes[axisId].autoScale) {
            calculateAutoScaleForAxis(axisId);
        }
    }

    update();
}

int XYChartWidget::getAxisForSeries(const QString &seriesName) const
{
    if (!m_series.contains(seriesName)) {
        return 0;
    }
    return m_series[seriesName].yAxisId;
}

void XYChartWidget::setAxisRange(int axisId, double min, double max)
{
    if (!m_yAxes.contains(axisId)) {
        return;
    }

    m_yAxes[axisId].min = min;
    m_yAxes[axisId].max = max;
    m_yAxes[axisId].autoScale = false;
    update();
}

void XYChartWidget::setAxisAutoScale(int axisId, bool enabled)
{
    if (!m_yAxes.contains(axisId)) {
        return;
    }

    m_yAxes[axisId].autoScale = enabled;
    if (enabled) {
        calculateAutoScaleForAxis(axisId);
    }
    update();
}

void XYChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Vẽ nền
    painter.fillRect(rect(), Qt::white);
    
    // Vẽ tiêu đề
    if (!m_title.isEmpty()) {
        painter.setPen(Qt::black);
        QFont titleFont = painter.font();
        titleFont.setPointSize(12);
        titleFont.setBold(true);
        painter.setFont(titleFont);
        painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, m_title);
    }
    
    // Vẽ lưới và trục
    if (m_showGrid) {
        drawGrid(painter);
    }
    drawAxes(painter);
    
    // Vẽ dữ liệu
    drawSeries(painter);

    // Vẽ chú thích
    if (!m_series.isEmpty()) {
        drawLegend(painter);
    }

    // Vẽ crosshair
    if (m_crosshairVisible) {
        drawCrosshair(painter);
    }
}

void XYChartWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    update();
}

void XYChartWidget::mousePressEvent(QMouseEvent *event)
{
    QPointF dataPoint = mapToData(event->pos());
    
    // Tìm series gần nhất
    double minDist = std::numeric_limits<double>::max();
    QString nearestSeries;
    QPointF nearestPoint;
    
    for (auto it = m_series.begin(); it != m_series.end(); ++it) {
        if (!it->visible || it->points.isEmpty()) continue;
        
        for (const QPointF &point : it->points) {
            QPointF widgetPoint = mapToWidget(point);
            double dist = QLineF(widgetPoint, event->pos()).length();
            if (dist < minDist && dist < 10.0) { // 10 pixel threshold
                minDist = dist;
                nearestSeries = it->name;
                nearestPoint = point;
            }
        }
    }
    
    if (!nearestSeries.isEmpty()) {
        emit seriesClicked(nearestSeries, nearestPoint);
    }

    QWidget::mousePressEvent(event);
}

void XYChartWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_crosshairPos = event->pos();
    update();
    QWidget::mouseMoveEvent(event);
}

void XYChartWidget::leaveEvent(QEvent *event)
{
    m_crosshairPos = QPoint(-1, -1);
    update();
    QWidget::leaveEvent(event);
}

void XYChartWidget::calculateAutoScale()
{
    if (m_series.isEmpty()) return;
    
    bool firstPoint = true;
    
    for (const auto &series : m_series) {
        if (!series.visible || series.points.isEmpty()) continue;
        
        for (const QPointF &point : series.points) {
            if (firstPoint) {
                m_xMin = m_xMax = point.x();
                m_yMin = m_yMax = point.y();
                firstPoint = false;
            } else {
                m_xMin = qMin(m_xMin, point.x());
                m_xMax = qMax(m_xMax, point.x());
                m_yMin = qMin(m_yMin, point.y());
                m_yMax = qMax(m_yMax, point.y());
            }
        }
    }
    
    // Thêm margin 5%
    double xRange = m_xMax - m_xMin;
    double yRange = m_yMax - m_yMin;
    
    if (xRange < 1e-10) xRange = 1.0;
    if (yRange < 1e-10) yRange = 1.0;
    
    m_xMin -= xRange * 0.05;
    m_xMax += xRange * 0.05;
    m_yMin -= yRange * 0.05;
    m_yMax += yRange * 0.05;
}

QPointF XYChartWidget::mapToWidget(const QPointF &dataPoint) const
{
    int plotWidth = width() - m_leftMargin - m_rightMargin;
    int plotHeight = height() - m_topMargin - m_bottomMargin;
    
    double xRange = m_xMax - m_xMin;
    double yRange = m_yMax - m_yMin;
    
    if (xRange < 1e-10) xRange = 1.0;
    if (yRange < 1e-10) yRange = 1.0;
    
    double x = m_leftMargin + (dataPoint.x() - m_xMin) / xRange * plotWidth;
    double y = height() - m_bottomMargin - (dataPoint.y() - m_yMin) / yRange * plotHeight;
    
    return QPointF(x, y);
}

QPointF XYChartWidget::mapToData(const QPointF &widgetPoint) const
{
    int plotWidth = width() - m_leftMargin - m_rightMargin;
    int plotHeight = height() - m_topMargin - m_bottomMargin;

    double xRange = m_xMax - m_xMin;
    double yRange = m_yMax - m_yMin;

    double x = m_xMin + (widgetPoint.x() - m_leftMargin) / plotWidth * xRange;
    double y = m_yMin + (height() - m_bottomMargin - widgetPoint.y()) / plotHeight * yRange;

    return QPointF(x, y);
}

QPointF XYChartWidget::mapToWidget(const QPointF &dataPoint, int axisId) const
{
    int plotWidth = width() - m_leftMargin - m_rightMargin;
    int plotHeight = height() - m_topMargin - m_bottomMargin;

    double xRange = m_xMax - m_xMin;
    if (xRange < 1e-10) xRange = 1.0;
    double x = m_leftMargin + (dataPoint.x() - m_xMin) / xRange * plotWidth;

    // Use axis-specific Y range
    double yRange = 10.0;
    double yMin = 0.0, yMax = 10.0;

    if (m_yAxes.contains(axisId)) {
        yMin = m_yAxes[axisId].min;
        yMax = m_yAxes[axisId].max;
        yRange = yMax - yMin;
    }

    if (yRange < 1e-10) yRange = 1.0;
    double y = height() - m_bottomMargin - (dataPoint.y() - yMin) / yRange * plotHeight;

    return QPointF(x, y);
}

QPointF XYChartWidget::mapToData(const QPointF &widgetPoint, int axisId) const
{
    int plotWidth = width() - m_leftMargin - m_rightMargin;
    int plotHeight = height() - m_topMargin - m_bottomMargin;

    double xRange = m_xMax - m_xMin;
    double x = m_xMin + (widgetPoint.x() - m_leftMargin) / plotWidth * xRange;

    // Use axis-specific Y range
    double yMin = 0.0, yMax = 10.0;
    if (m_yAxes.contains(axisId)) {
        yMin = m_yAxes[axisId].min;
        yMax = m_yAxes[axisId].max;
    }
    double yRange = yMax - yMin;

    double y = yMin + (height() - m_bottomMargin - widgetPoint.y()) / plotHeight * yRange;

    return QPointF(x, y);
}

void XYChartWidget::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(QColor(220, 220, 220), 1));
    
    int plotWidth = width() - m_leftMargin - m_rightMargin;
    int plotHeight = height() - m_topMargin - m_bottomMargin;
    
    // Lưới dọc
    for (int i = 0; i <= 10; ++i) {
        int x = m_leftMargin + plotWidth * i / 10;
        painter.drawLine(x, m_topMargin, x, height() - m_bottomMargin);
    }
    
    // Lưới ngang
    for (int i = 0; i <= 10; ++i) {
        int y = m_topMargin + plotHeight * i / 10;
        painter.drawLine(m_leftMargin, y, width() - m_rightMargin, y);
    }
}

void XYChartWidget::drawYAxis(QPainter &painter, const YAxisInfo &axis)
{
    // Calculate position for this axis based on which side and order
    int xPos = m_leftMargin;
    int axisWidth = 50;  // Width allocated for each axis label column

    if (axis.side == Qt::AlignRight) {
        // Right side axes
        xPos = width() - m_rightMargin;
    } else {
        // Left side axes - stack them from right to left
        int leftCount = 0;
        for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
            if (it.value().axisId == axis.axisId) break;
            if (it.value().side == Qt::AlignLeft) leftCount++;
        }
        xPos = m_leftMargin - (leftCount + 1) * axisWidth;
    }

    // Draw axis line
    painter.setPen(QPen(axis.color, 2));
    painter.drawLine(xPos, m_topMargin, xPos, height() - m_bottomMargin);

    if (m_showAxisLabels) {
        painter.setPen(axis.color);
        QFont labelFont = painter.font();
        labelFont.setPointSize(7);
        painter.setFont(labelFont);

        // Draw axis label (name of the axis)
        painter.save();
        painter.translate(xPos - 25, m_topMargin - 5);
        painter.rotate(-90);
        painter.drawText(0, 0, 30, 15, Qt::AlignCenter, axis.label);
        painter.restore();

        // Draw tick marks and value labels
        for (int i = 0; i <= 10; ++i) {
            double value = axis.min + (axis.max - axis.min) * i / 10;
            int y = height() - m_bottomMargin - (height() - m_topMargin - m_bottomMargin) * i / 10;

            // Tick mark
            painter.drawLine(xPos - 3, y, xPos + 3, y);

            // Value label
            QString valueStr = QString::number(value, 'f', 1);
            if (axis.side == Qt::AlignRight) {
                painter.drawText(xPos + 5, y - 10, 50, 20,
                               Qt::AlignLeft | Qt::AlignVCenter, valueStr);
            } else {
                painter.drawText(xPos - axisWidth + 5, y - 10, axisWidth - 10, 20,
                               Qt::AlignRight | Qt::AlignVCenter, valueStr);
            }
        }
    }
}

void XYChartWidget::drawCrosshairTextBox(QPainter &painter, QPoint pos, const QStringList &lines)
{
    QFont font = painter.font();
    font.setPointSize(8);
    QFontMetrics fm(font);

    int maxWidth = 0;
    for (const QString &line : lines) {
        maxWidth = qMax(maxWidth, fm.width(line));
    }

    int lineHeight = fm.height();
    int totalHeight = lines.size() * lineHeight + 4;
    int boxWidth = maxWidth + 8;

    int plotLeft = m_leftMargin;
    int plotRight = width() - m_rightMargin;
    int plotTop = m_topMargin;

    int boxX = pos.x() + 10;
    int boxY = pos.y() - 10 - totalHeight;

    if (boxX + boxWidth > plotRight) boxX = pos.x() - boxWidth - 10;
    if (boxY < plotTop) boxY = pos.y() + 10;

    painter.fillRect(boxX, boxY, boxWidth, totalHeight, QColor(255, 255, 200, 220));
    painter.setPen(Qt::black);
    painter.drawRect(boxX, boxY, boxWidth, totalHeight);

    for (int i = 0; i < lines.size(); ++i) {
        painter.drawText(boxX + 4, boxY + 2 + i * lineHeight + fm.ascent(), lines[i]);
    }
}

void XYChartWidget::drawAxes(QPainter &painter)
{
    painter.setPen(QPen(Qt::black, 2));

    // Draw X-axis
    painter.drawLine(m_leftMargin, height() - m_bottomMargin,
                     width() - m_rightMargin, height() - m_bottomMargin);

    // Draw all Y-axes
    if (m_multiAxisEnabled) {
        // Draw each axis in the multi-axis setup
        for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
            drawYAxis(painter, it.value());
        }
    } else {
        // Single axis mode - draw default axis on left
        painter.drawLine(m_leftMargin, m_topMargin,
                         m_leftMargin, height() - m_bottomMargin);
    }

    if (m_showAxisLabels) {
        painter.setPen(Qt::black);
        QFont labelFont = painter.font();
        labelFont.setPointSize(8);
        painter.setFont(labelFont);

        // X-axis labels
        for (int i = 0; i <= 10; ++i) {
            double value = m_xMin + (m_xMax - m_xMin) * i / 10;
            int x = m_leftMargin + (width() - m_leftMargin - m_rightMargin) * i / 10;
            painter.drawText(x - 30, height() - m_bottomMargin + 20, 60, 20,
                           Qt::AlignCenter, QString::number(value, 'f', 1));
        }

        // Y-axis labels (only for single axis mode)
        if (!m_multiAxisEnabled) {
            for (int i = 0; i <= 10; ++i) {
                double value = m_yMin + (m_yMax - m_yMin) * i / 10;
                int y = height() - m_bottomMargin - (height() - m_topMargin - m_bottomMargin) * i / 10;
                painter.drawText(5, y - 10, m_leftMargin - 10, 20,
                               Qt::AlignRight | Qt::AlignVCenter, QString::number(value, 'f', 1));
            }
        }

        // X-axis title
        if (!m_xLabel.isEmpty()) {
            painter.drawText(m_leftMargin, height() - 15,
                           width() - m_leftMargin - m_rightMargin, 15,
                           Qt::AlignCenter, m_xLabel);
        }

        // Y-axis title (only for single axis mode)
        if (!m_multiAxisEnabled && !m_yLabel.isEmpty()) {
            painter.save();
            painter.translate(15, (height() - m_topMargin - m_bottomMargin) / 2 + m_topMargin);
            painter.rotate(-90);
            painter.drawText(-100, 0, 200, 20, Qt::AlignCenter, m_yLabel);
            painter.restore();
        }
    }
}

void XYChartWidget::drawSeries(QPainter &painter)
{
    for (const auto &series : m_series) {
        if (!series.visible || series.points.size() < 2) continue;

        painter.setPen(series.pen);

        // Use axis-specific mapping based on the series' yAxisId
        int axisId = series.yAxisId;

        for (int i = 0; i < series.points.size() - 1; ++i) {
            QPointF p1 = mapToWidget(series.points[i], axisId);
            QPointF p2 = mapToWidget(series.points[i + 1], axisId);
            painter.drawLine(p1, p2);
        }

        // Vẽ các điểm
        painter.setBrush(series.color);
        for (const QPointF &point : series.points) {
            QPointF widgetPoint = mapToWidget(point, axisId);
            painter.drawEllipse(widgetPoint, 3, 3);
        }
    }
}

void XYChartWidget::drawLegend(QPainter &painter)
{
    int legendX = width() - m_rightMargin - 150;
    int legendY = m_topMargin + 10;
    int lineHeight = 20;
    
    painter.setPen(Qt::black);
    painter.setBrush(QColor(255, 255, 255, 200));
    
    int visibleCount = 0;
    for (const auto &series : m_series) {
        if (series.visible) visibleCount++;
    }
    
    if (visibleCount == 0) return;
    
    painter.drawRect(legendX - 5, legendY - 5, 145, visibleCount * lineHeight + 5);
    
    int i = 0;
    for (const auto &series : m_series) {
        if (!series.visible) continue;
        
        // Vẽ đường màu
        painter.setPen(series.pen);
        painter.drawLine(legendX, legendY + i * lineHeight + 10,
                        legendX + 30, legendY + i * lineHeight + 10);
        
        // Vẽ tên
        painter.setPen(Qt::black);
        painter.drawText(legendX + 35, legendY + i * lineHeight,
                        100, lineHeight, Qt::AlignVCenter, series.name);
        i++;
    }
}

void XYChartWidget::drawCrosshair(QPainter &painter)
{
    if (m_crosshairPos.x() < 0 || m_crosshairPos.y() < 0) {
        return;
    }

    int plotLeft = m_leftMargin;
    int plotRight = width() - m_rightMargin;
    int plotTop = m_topMargin;
    int plotBottom = height() - m_bottomMargin;

    // Only draw crosshair if inside plot area
    if (m_crosshairPos.x() < plotLeft || m_crosshairPos.x() > plotRight ||
        m_crosshairPos.y() < plotTop || m_crosshairPos.y() > plotBottom) {
        return;
    }

    // Draw vertical line
    painter.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    painter.drawLine(m_crosshairPos.x(), plotTop, m_crosshairPos.x(), plotBottom);

    // Draw horizontal line
    painter.drawLine(plotLeft, m_crosshairPos.y(), plotRight, m_crosshairPos.y());

    // Draw center point
    painter.setPen(QPen(Qt::gray, 2));
    painter.drawPoint(m_crosshairPos);

    // Build list of text lines for display
    QStringList lines;

    // Convert X to data coordinate (same for all)
    int plotWidth = plotRight - plotLeft;
    double xRange = m_xMax - m_xMin;
    if (xRange < 1e-10) xRange = 1.0;
    double xData = m_xMin + (m_crosshairPos.x() - plotLeft) / plotWidth * xRange;
    lines.append(QString("X: %1").arg(xData, 0, 'f', 2));

    // Add Y values from each visible series (on its own axis)
    for (const auto &series : m_series) {
        if (!series.visible) continue;

        int axisId = series.yAxisId;
        // Convert Y to data coordinate using axis-specific range
        QPointF dataPoint = mapToData(QPointF(m_crosshairPos.x(), m_crosshairPos.y()), axisId);

        // Only show the Y value for this axis
        QString yText = QString("%1: %2").arg(series.name).arg(dataPoint.y(), 0, 'f', 2);
        lines.append(yText);
    }

    // Draw multi-line coordinate text box using helper function
    drawCrosshairTextBox(painter, m_crosshairPos, lines);
}

void XYChartWidget::autoGroupSeriesToAxes()
{
    if (!m_multiAxisEnabled || m_series.isEmpty()) {
        return;
    }

    // Clear all axis assignments (except axis 0)
    for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
        it.value().seriesNames.clear();
    }

    // If auto-grouping disabled or only one series, assign all to axis 0
    if (!m_autoGroupSeries || m_series.size() <= 1) {
        for (auto it = m_series.begin(); it != m_series.end(); ++it) {
            it.value().yAxisId = 0;
            m_yAxes[0].seriesNames.append(it.key());
        }
        calculateAutoScaleForAxis(0);
        return;
    }

    // Simple grouping: use first axis for all series
    // (Full clustering algorithm can be added later)
    for (auto it = m_series.begin(); it != m_series.end(); ++it) {
        it.value().yAxisId = 0;
        m_yAxes[0].seriesNames.append(it.key());
    }

    calculateAutoScaleForAxis(0);
}

void XYChartWidget::calculateAutoScaleForAxis(int axisId)
{
    if (!m_yAxes.contains(axisId)) {
        return;
    }

    YAxisInfo &axis = m_yAxes[axisId];
    if (!axis.autoScale) {
        return;
    }

    if (axis.seriesNames.isEmpty()) {
        axis.min = 0.0;
        axis.max = 10.0;
        return;
    }

    bool firstPoint = true;
    double minVal = 0.0, maxVal = 10.0;

    for (const QString &seriesName : axis.seriesNames) {
        if (!m_series.contains(seriesName)) {
            continue;
        }

        const DataSeries &series = m_series[seriesName];
        if (!series.visible || series.points.isEmpty()) {
            continue;
        }

        for (const QPointF &point : series.points) {
            if (firstPoint) {
                minVal = maxVal = point.y();
                firstPoint = false;
            } else {
                minVal = qMin(minVal, point.y());
                maxVal = qMax(maxVal, point.y());
            }
        }
    }

    // Add 5% margin
    double range = maxVal - minVal;
    if (range < 1e-10) {
        range = 1.0;
    }

    axis.min = minVal - range * 0.05;
    axis.max = maxVal + range * 0.05;
}

int XYChartWidget::calculateAxisPosition(const YAxisInfo &axis) const
{
    if (axis.side == Qt::AlignLeft) {
        int leftCount = countAxesOnSide(Qt::AlignLeft);
        int axisIndex = 0;
        for (auto it = m_yAxes.constBegin(); it != m_yAxes.constEnd(); ++it) {
            if (it.value().side == Qt::AlignLeft) {
                if (it.value().axisId == axis.axisId) {
                    break;
                }
                axisIndex++;
            }
        }
        return m_leftMargin - (axisIndex + 1) * m_axisSpacing;
    } else {
        return width() - m_rightMargin;
    }
}

int XYChartWidget::countAxesOnSide(Qt::AlignmentFlag side) const
{
    int count = 0;
    for (auto it = m_yAxes.constBegin(); it != m_yAxes.constEnd(); ++it) {
        if (it.value().side == side) {
            count++;
        }
    }
    return count;
}

void XYChartWidget::updateMargins()
{
    if (!m_multiAxisEnabled) {
        return;
    }

    int leftCount = countAxesOnSide(Qt::AlignLeft);
    int rightCount = countAxesOnSide(Qt::AlignRight);

    m_leftMargin = 60 + qMax(0, leftCount - 1) * m_axisSpacing;
    m_rightMargin = 20 + rightCount * m_axisSpacing;
}

QColor XYChartWidget::getNextColor()
{
    static const QColor colors[] = {
        Qt::blue, Qt::red, Qt::green, Qt::magenta,
        Qt::cyan, Qt::darkYellow, Qt::darkBlue, Qt::darkRed,
        Qt::darkGreen, Qt::darkMagenta
    };

    return colors[m_colorIndex++ % 10];
}
