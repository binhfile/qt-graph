#include "xy_chart_widget.h"
#include <QDebug>
#include <QFontMetrics>
#include <QPaintEvent>
#include <QPainter>
#include <QtMath>
#include <limits>

XYChartWidget::XYChartWidget(QWidget *parent)
    : QWidget(parent), m_showGrid(true), m_showAxisLabels(true),
      m_autoScale(true), m_xMin(0.0), m_xMax(10.0), m_yMin(0.0), m_yMax(10.0),
      m_maxPointsPerSeries(-1), m_leftMargin(10), m_rightMargin(10),
      m_topMargin(40), m_bottomMargin(50), m_colorIndex(0),
      m_crosshairVisible(true), m_crosshairPos(-1, -1), m_nextAxisId(1),
      m_multiAxisEnabled(false), m_autoGroupSeries(false), m_maxYAxes(4),
      m_groupingThreshold(0.3), m_axisSpacing(40), m_darkModeEnabled(false) {
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

XYChartWidget::~XYChartWidget() {}

void XYChartWidget::addSeries(const QString &seriesName, const QColor &color) {
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

void XYChartWidget::addPoint(const QString &seriesName, double x, double y) {
  addPoint(seriesName, QPointF(x, y));
}

void XYChartWidget::addPoint(const QString &seriesName, const QPointF &point) {
  if (!m_series.contains(seriesName)) {
    addSeries(seriesName);
  }

  m_series[seriesName].points.append(point);

  // Limit number of points if set
  if (m_maxPointsPerSeries > 0 &&
      m_series[seriesName].points.size() > m_maxPointsPerSeries) {
    m_series[seriesName].points.removeFirst();
  }

  // Auto-scale: Update both global and axis-specific scales
  if (m_autoScale) {
    calculateAutoScale();
  }

  // Also update axis-specific auto-scale if in multi-axis mode
  if (m_multiAxisEnabled) {
    int axisId = m_series[seriesName].yAxisId;
    calculateAutoScaleForAxis(axisId);
  }

  update();
}

void XYChartWidget::addPoints(const QString &seriesName,
                              const QVector<QPointF> &points) {
  if (!m_series.contains(seriesName)) {
    addSeries(seriesName);
  }

  m_series[seriesName].points.append(points);

  // Limit number of points
  if (m_maxPointsPerSeries > 0 &&
      m_series[seriesName].points.size() > m_maxPointsPerSeries) {
    int excess = m_series[seriesName].points.size() - m_maxPointsPerSeries;
    m_series[seriesName].points.remove(0, excess);
  }

  // Auto-scale: Update both global and axis-specific scales
  if (m_autoScale) {
    calculateAutoScale();
  }

  // Also update axis-specific auto-scale if in multi-axis mode
  if (m_multiAxisEnabled) {
    int axisId = m_series[seriesName].yAxisId;
    calculateAutoScaleForAxis(axisId);
  }

  update();
}

void XYChartWidget::setSeriesData(const QString &seriesName,
                                  const QVector<QPointF> &points) {
  if (!m_series.contains(seriesName)) {
    addSeries(seriesName);
  }

  m_series[seriesName].points = points;

  // Limit number of points
  if (m_maxPointsPerSeries > 0 &&
      m_series[seriesName].points.size() > m_maxPointsPerSeries) {
    int excess = m_series[seriesName].points.size() - m_maxPointsPerSeries;
    m_series[seriesName].points.remove(0, excess);
  }

  // Auto-scale: Update both global and axis-specific scales
  if (m_autoScale) {
    calculateAutoScale();
  }

  // Also update axis-specific auto-scale if in multi-axis mode
  if (m_multiAxisEnabled) {
    int axisId = m_series[seriesName].yAxisId;
    calculateAutoScaleForAxis(axisId);
  }

  update();
}

void XYChartWidget::clearSeries(const QString &seriesName) {
  if (m_series.contains(seriesName)) {
    m_series[seriesName].points.clear();
    update();
  }
}

void XYChartWidget::clearAllSeries() {
  for (auto &series : m_series) {
    series.points.clear();
  }
  update();
}

void XYChartWidget::removeSeries(const QString &seriesName) {
  m_series.remove(seriesName);
  update();
}

void XYChartWidget::setSeriesVisible(const QString &seriesName, bool visible) {
  if (m_series.contains(seriesName)) {
    m_series[seriesName].visible = visible;
    update();
  }
}

void XYChartWidget::setSeriesColor(const QString &seriesName,
                                   const QColor &color) {
  if (m_series.contains(seriesName)) {
    m_series[seriesName].color = color;
    m_series[seriesName].pen.setColor(color);
    update();
  }
}

void XYChartWidget::setSeriesPenWidth(const QString &seriesName, int width) {
  if (m_series.contains(seriesName)) {
    m_series[seriesName].pen.setWidth(width);
    update();
  }
}

void XYChartWidget::setXRange(double min, double max) {
  m_xMin = min;
  m_xMax = max;
  m_autoScale = false;
  update();
}

void XYChartWidget::setYRange(double min, double max) {
  m_yMin = min;
  m_yMax = max;
  m_autoScale = false;
  update();
}

void XYChartWidget::setAutoScale(bool enable) {
  m_autoScale = enable;
  if (m_autoScale) {
    calculateAutoScale();
  }
  update();
}

void XYChartWidget::setMaxPointsPerSeries(int maxPoints) {
  m_maxPointsPerSeries = maxPoints;
}

void XYChartWidget::setGridVisible(bool visible) {
  m_showGrid = visible;
  update();
}

void XYChartWidget::setAxisLabelsVisible(bool visible) {
  m_showAxisLabels = visible;
  update();
}

void XYChartWidget::setTitle(const QString &title) {
  m_title = title;
  update();
}

void XYChartWidget::setXLabel(const QString &label) {
  m_xLabel = label;
  update();
}

void XYChartWidget::setYLabel(const QString &label) {
  m_yLabel = label;
  update();
}

QStringList XYChartWidget::getSeriesNames() const { return m_series.keys(); }

int XYChartWidget::getSeriesCount() const { return m_series.size(); }

void XYChartWidget::setCrosshairVisible(bool visible) {
  m_crosshairVisible = visible;
  update();
}

bool XYChartWidget::isCrosshairVisible() const { return m_crosshairVisible; }

void XYChartWidget::setDarkModeEnabled(bool enabled) {
  m_darkModeEnabled = enabled;
  update();
}

bool XYChartWidget::isDarkModeEnabled() const { return m_darkModeEnabled; }

void XYChartWidget::setMultiAxisEnabled(bool enabled) {
  m_multiAxisEnabled = enabled;
  if (enabled && m_autoGroupSeries) {
    autoGroupSeriesToAxes();
  }
  update();
}

bool XYChartWidget::isMultiAxisEnabled() const { return m_multiAxisEnabled; }

void XYChartWidget::setAutoGroupSeries(bool enabled) {
  m_autoGroupSeries = enabled;
  if (m_multiAxisEnabled && enabled) {
    autoGroupSeriesToAxes();
  }
  update();
}

bool XYChartWidget::isAutoGroupEnabled() const { return m_autoGroupSeries; }

void XYChartWidget::createYAxis(int axisId, const QString &label,
                                Qt::AlignmentFlag side) {
  if (m_yAxes.contains(axisId)) {
    return;
  }

  YAxisInfo axis;
  axis.axisId = axisId;
  axis.label = label;
  axis.side = side;
  axis.color = Qt::black; // Will be set when series is assigned
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

void XYChartWidget::assignSeriesToAxis(const QString &seriesName, int axisId) {
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

int XYChartWidget::getAxisForSeries(const QString &seriesName) const {
  if (!m_series.contains(seriesName)) {
    return 0;
  }
  return m_series[seriesName].yAxisId;
}

void XYChartWidget::setAxisRange(int axisId, double min, double max) {
  if (!m_yAxes.contains(axisId)) {
    return;
  }

  m_yAxes[axisId].min = min;
  m_yAxes[axisId].max = max;
  m_yAxes[axisId].autoScale = false;
  update();
}

void XYChartWidget::setAxisAutoScale(int axisId, bool enabled) {
  if (!m_yAxes.contains(axisId)) {
    return;
  }

  m_yAxes[axisId].autoScale = enabled;
  if (enabled) {
    calculateAutoScaleForAxis(axisId);
  }
  update();
}

void XYChartWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);

  QPainter painter(this);
  // Optimization: Only enable antialiasing for lines and curves, not for grid
  painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
  painter.setRenderHint(QPainter::HighQualityAntialiasing, false);

  // Draw background based on dark mode
  QColor bgColor = m_darkModeEnabled ? QColor(30, 30, 30) : Qt::white;
  QColor textColor = m_darkModeEnabled ? Qt::white : Qt::black;
  painter.fillRect(rect(), bgColor);

  // Draw title with cached font
  if (!m_title.isEmpty()) {
    painter.setPen(textColor);
    QFont titleFont = painter.font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    painter.setFont(titleFont);
    painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, m_title);
  }

  // Draw grid and axes
  if (m_showGrid) {
    drawGrid(painter);
  }
  drawAxes(painter);

  // Draw data series
  drawSeries(painter);

  // Draw legend
  if (!m_series.isEmpty()) {
    drawLegend(painter);
  }

  // Draw crosshair
  if (m_crosshairVisible) {
    drawCrosshair(painter);
  }
}

void XYChartWidget::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);
  update();
}

void XYChartWidget::mousePressEvent(QMouseEvent *event) {
  QPointF dataPoint = mapToData(event->pos());

  // Find nearest series (optimization: limit search for large datasets)
  double minDist = std::numeric_limits<double>::max();
  QString nearestSeries;
  QPointF nearestPoint;
  const double distanceThreshold = 10.0; // 10 pixel threshold
  const int maxPointsToCheck =
      2000; // Optimization: skip detailed search for huge datasets

  for (auto it = m_series.begin(); it != m_series.end(); ++it) {
    if (!it->visible || it->points.isEmpty())
      continue;

    // Optimization: For very large datasets, sample points instead of checking
    // all
    const int pointCount = it->points.size();
    const int checkStep =
        (pointCount > maxPointsToCheck) ? (pointCount / maxPointsToCheck) : 1;

    for (int i = 0; i < pointCount; i += checkStep) {
      const QPointF &point = it->points[i];
      QPointF widgetPoint = mapToWidget(point);
      double dist = QLineF(widgetPoint, event->pos()).length();
      if (dist < minDist && dist < distanceThreshold) {
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

void XYChartWidget::mouseMoveEvent(QMouseEvent *event) {
  // Optimization: Only update if crosshair position actually changed
  if (m_crosshairPos != event->pos()) {
    m_crosshairPos = event->pos();
    update();
  }
  QWidget::mouseMoveEvent(event);
}

void XYChartWidget::leaveEvent(QEvent *event) {
  m_crosshairPos = QPoint(-1, -1);
  update();
  QWidget::leaveEvent(event);
}

void XYChartWidget::calculateAutoScale() {
  if (m_series.isEmpty())
    return;

  bool firstPoint = true;

  for (const auto &series : m_series) {
    if (!series.visible || series.points.isEmpty())
      continue;

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

  if (xRange < 1e-10)
    xRange = 1.0;
  if (yRange < 1e-10)
    yRange = 1.0;

  m_xMin -= xRange * 0.05;
  m_xMax += xRange * 0.05;
  m_yMin -= yRange * 0.05;
  m_yMax += yRange * 0.05;
}

QPointF XYChartWidget::mapToWidget(const QPointF &dataPoint) const {
  int plotWidth = width() - m_leftMargin - m_rightMargin;
  int plotHeight = height() - m_topMargin - m_bottomMargin;

  double xRange = m_xMax - m_xMin;
  double yRange = m_yMax - m_yMin;

  if (xRange < 1e-10)
    xRange = 1.0;
  if (yRange < 1e-10)
    yRange = 1.0;

  double x = m_leftMargin + (dataPoint.x() - m_xMin) / xRange * plotWidth;
  double y = height() - m_bottomMargin -
             (dataPoint.y() - m_yMin) / yRange * plotHeight;

  return QPointF(x, y);
}

QPointF XYChartWidget::mapToData(const QPointF &widgetPoint) const {
  int plotWidth = width() - m_leftMargin - m_rightMargin;
  int plotHeight = height() - m_topMargin - m_bottomMargin;

  double xRange = m_xMax - m_xMin;
  double yRange = m_yMax - m_yMin;

  double x = m_xMin + (widgetPoint.x() - m_leftMargin) / plotWidth * xRange;
  double y = m_yMin + (height() - m_bottomMargin - widgetPoint.y()) /
                          plotHeight * yRange;

  return QPointF(x, y);
}

QPointF XYChartWidget::mapToWidget(const QPointF &dataPoint, int axisId) const {
  int plotWidth = width() - m_leftMargin - m_rightMargin;
  int plotHeight = height() - m_topMargin - m_bottomMargin;

  double xRange = m_xMax - m_xMin;
  if (xRange < 1e-10)
    xRange = 1.0;
  double x = m_leftMargin + (dataPoint.x() - m_xMin) / xRange * plotWidth;

  // Use axis-specific Y range
  double yRange = 10.0;
  double yMin = 0.0, yMax = 10.0;

  if (m_yAxes.contains(axisId)) {
    yMin = m_yAxes[axisId].min;
    yMax = m_yAxes[axisId].max;
    yRange = yMax - yMin;
  }

  if (yRange < 1e-10)
    yRange = 1.0;
  double y =
      height() - m_bottomMargin - (dataPoint.y() - yMin) / yRange * plotHeight;

  return QPointF(x, y);
}

QPointF XYChartWidget::mapToData(const QPointF &widgetPoint, int axisId) const {
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

  double y = yMin + (height() - m_bottomMargin - widgetPoint.y()) / plotHeight *
                        yRange;

  return QPointF(x, y);
}

void XYChartWidget::drawGrid(QPainter &painter) {
  // Grid color based on dark mode
  QColor gridColor =
      m_darkModeEnabled ? QColor(60, 60, 60) : QColor(220, 220, 220);
  QPen gridPen(gridColor, 1);
  gridPen.setCapStyle(Qt::FlatCap); // Optimization: Use flat caps
  painter.setPen(gridPen);

  int plotWidth = width() - m_leftMargin - m_rightMargin;
  int plotHeight = height() - m_topMargin - m_bottomMargin;
  int plotRight = width() - m_rightMargin;
  int plotBottom = height() - m_bottomMargin;

  // Optimization: Pre-calculate divisor
  const int gridDivisions = 10;
  const int xStep = plotWidth / gridDivisions;
  const int yStep = plotHeight / gridDivisions;

  // Vertical grid lines - optimized iteration
  for (int i = 0; i <= gridDivisions; ++i) {
    int x = m_leftMargin + (xStep * i);
    painter.drawLine(x, m_topMargin, x, plotBottom);
  }

  // Horizontal grid lines - optimized iteration
  for (int i = 0; i <= gridDivisions; ++i) {
    int y = m_topMargin + (yStep * i);
    painter.drawLine(m_leftMargin, y, plotRight, y);
  }
}

void XYChartWidget::drawYAxis(QPainter &painter, const YAxisInfo &axis) {
  // Calculate position for this axis based on which side and order
  int xPos = m_leftMargin;
  int axisSpacing = m_axisSpacing; // Use consistent spacing from member
                                   // variable (40px between axes)
  int axisWidth =
      40; // Width allocated for axis label text (reduced for tight layout)
  int textWidth = 50;      // Width needed for value labels on right side
  int minEdgePadding = 35; // Minimum space from window edge to prevent text
                           // cutoff (reduced with tight margins)

  if (axis.side == Qt::AlignRight) {
    // Right side axes - stack them from left to right (away from plot area)
    int rightCount = 0;
    for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
      if (it.value().axisId == axis.axisId)
        break;
      if (it.value().side == Qt::AlignRight)
        rightCount++;
    }
    // Position: starting at right margin boundary, moving further right for
    // additional axes
    xPos = width() - m_rightMargin - (rightCount * axisSpacing);
    // Ensure axis and its labels don't go beyond right edge of window
    int maxXPos = width() - minEdgePadding;
    if (xPos + textWidth > maxXPos) {
      xPos = maxXPos - textWidth;
    }
  } else {
    // Left side axes - stack them from right to left (away from plot area)
    int leftCount = 0;
    for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
      if (it.value().axisId == axis.axisId)
        break;
      if (it.value().side == Qt::AlignLeft)
        leftCount++;
    }
    // Position: starting at left margin boundary, moving further left for
    // additional axes
    xPos = m_leftMargin + (leftCount * axisSpacing);
    // Ensure axis and its labels don't go beyond left edge of window
    int minXPos = minEdgePadding;
    if (xPos - axisWidth < minXPos) {
      xPos = minXPos + axisWidth;
    }
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
      int y = height() - m_bottomMargin -
              (height() - m_topMargin - m_bottomMargin) * i / 10;

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

void XYChartWidget::drawCrosshairTextBox(QPainter &painter, QPoint pos,
                                         const QStringList &lines) {
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

  if (boxX + boxWidth > plotRight)
    boxX = pos.x() - boxWidth - 10;
  if (boxY < plotTop)
    boxY = pos.y() + 10;

  // Tooltip colors based on dark mode
  QColor bgColor =
      m_darkModeEnabled ? QColor(50, 50, 50, 220) : QColor(255, 255, 200, 220);
  QColor textColor = m_darkModeEnabled ? Qt::white : Qt::black;
  QColor borderColor = m_darkModeEnabled ? QColor(100, 100, 100) : Qt::black;

  painter.fillRect(boxX, boxY, boxWidth, totalHeight, bgColor);
  painter.setPen(borderColor);
  painter.drawRect(boxX, boxY, boxWidth, totalHeight);

  painter.setPen(textColor);
  for (int i = 0; i < lines.size(); ++i) {
    painter.drawText(boxX + 4, boxY + 2 + i * lineHeight + fm.ascent(),
                     lines[i]);
  }
}

void XYChartWidget::drawAxes(QPainter &painter) {
  // Axis color based on dark mode
  QColor axisColor = m_darkModeEnabled ? Qt::white : Qt::black;
  QColor textColor = m_darkModeEnabled ? Qt::white : Qt::black;

  painter.setPen(QPen(axisColor, 2));

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
    painter.drawLine(m_leftMargin, m_topMargin, m_leftMargin,
                     height() - m_bottomMargin);
  }

  if (m_showAxisLabels) {
    painter.setPen(textColor);
    // Optimization: Create font once instead of modifying painter font
    // repeatedly
    QFont labelFont = painter.font();
    labelFont.setPointSize(8);
    painter.setFont(labelFont);

    // Optimization: Pre-calculate axis dimensions
    const int plotWidth = width() - m_leftMargin - m_rightMargin;
    const int plotHeight = height() - m_topMargin - m_bottomMargin;
    const double xRange = m_xMax - m_xMin;
    const double yRange = m_yMax - m_yMin;
    const double xStep = xRange / 10.0;
    const double yStep = yRange / 10.0;

    // X-axis labels - optimized calculations
    for (int i = 0; i <= 10; ++i) {
      double value = m_xMin + (xStep * i);
      int x = m_leftMargin + (plotWidth * i / 10);
      painter.drawText(x - 30, height() - m_bottomMargin + 20, 60, 20,
                       Qt::AlignCenter, QString::number(value, 'f', 1));
    }

    // Y-axis labels (only for single axis mode)
    if (!m_multiAxisEnabled) {
      for (int i = 0; i <= 10; ++i) {
        double value = m_yMin + (yStep * i);
        int y = height() - m_bottomMargin - (plotHeight * i / 10);
        painter.drawText(5, y - 10, m_leftMargin - 10, 20,
                         Qt::AlignRight | Qt::AlignVCenter,
                         QString::number(value, 'f', 1));
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
      painter.translate(15, (height() - m_topMargin - m_bottomMargin) / 2 +
                                m_topMargin);
      painter.rotate(-90);
      painter.drawText(-100, 0, 200, 20, Qt::AlignCenter, m_yLabel);
      painter.restore();
    }
  }
}

void XYChartWidget::drawSeries(QPainter &painter) {
  for (const auto &series : m_series) {
    if (!series.visible || series.points.size() < 2)
      continue;

    painter.setPen(series.pen);

    // Use axis-specific mapping based on the series' yAxisId
    int axisId = series.yAxisId;

    // Optimization: Draw lines as a polyline instead of individual segments
    QVector<QPointF> widgetPoints;
    widgetPoints.reserve(series.points.size());

    for (int i = 0; i < series.points.size(); ++i) {
      widgetPoints.append(mapToWidget(series.points[i], axisId));
    }

    // Draw polyline in one operation
    if (widgetPoints.size() >= 2) {
      for (int i = 0; i < widgetPoints.size() - 1; ++i) {
        painter.drawLine(widgetPoints[i], widgetPoints[i + 1]);
      }
    }

    // Optimization: Skip point drawing for datasets with many points (> 500)
    // as individual points become imperceptible and expensive to draw
    if (series.points.size() <= 500) {
      painter.setBrush(series.color);
      for (const QPointF &widgetPoint : widgetPoints) {
        painter.drawEllipse(widgetPoint, 3, 3);
      }
    }
  }
}

void XYChartWidget::drawLegend(QPainter &painter) {
  int legendX = width() - m_rightMargin - 200;
  int legendY = m_topMargin + 10;
  int lineHeight = 20;

  // Legend colors based on dark mode
  QColor bgColor =
      m_darkModeEnabled ? QColor(50, 50, 50, 200) : QColor(255, 255, 255, 200);
  QColor textColor = m_darkModeEnabled ? Qt::white : Qt::black;
  QColor borderColor = m_darkModeEnabled ? QColor(100, 100, 100) : Qt::black;

  painter.setPen(borderColor);
  painter.setBrush(bgColor);

  int visibleCount = 0;
  for (const auto &series : m_series) {
    if (series.visible)
      visibleCount++;
  }

  if (visibleCount == 0)
    return;

  painter.drawRect(legendX - 5, legendY - 5, 200,
                   visibleCount * lineHeight + 5);

  int i = 0;
  for (const auto &series : m_series) {
    if (!series.visible)
      continue;

    // Draw colored line
    painter.setPen(series.pen);
    painter.drawLine(legendX, legendY + i * lineHeight + 10, legendX + 30,
                     legendY + i * lineHeight + 10);

    // Draw name
    painter.setPen(textColor);
    painter.drawText(legendX + 35, legendY + i * lineHeight, 150, lineHeight,
                     Qt::AlignVCenter, series.name);
    i++;
  }
}

void XYChartWidget::drawCrosshair(QPainter &painter) {
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

  // Convert widget coordinates to data coordinates using mapToData
  // (use axis 0 for X since X is same for all axes, but we need a valid axisId)
  QPointF dataPoint =
      mapToData(QPointF(m_crosshairPos.x(), m_crosshairPos.y()), 0);
  double xData = dataPoint.x();
  lines.append(QString("X: %1").arg(xData, 0, 'f', 2));

  // Add Y values from each visible series (on its own axis)
  for (const auto &series : m_series) {
    if (!series.visible)
      continue;

    int axisId = series.yAxisId;
    // Convert Y to data coordinate using axis-specific range
    QPointF seriesDataPoint =
        mapToData(QPointF(m_crosshairPos.x(), m_crosshairPos.y()), axisId);

    // Only show the Y value for this axis
    QString yText =
        QString("%1: %2").arg(series.name).arg(seriesDataPoint.y(), 0, 'f', 2);
    lines.append(yText);
  }

  // Draw multi-line coordinate text box using helper function
  drawCrosshairTextBox(painter, m_crosshairPos, lines);
}

void XYChartWidget::autoGroupSeriesToAxes() {
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

void XYChartWidget::calculateAutoScaleForAxis(int axisId) {
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

int XYChartWidget::calculateAxisPosition(const YAxisInfo &axis) const {
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

int XYChartWidget::countAxesOnSide(Qt::AlignmentFlag side) const {
  int count = 0;
  for (auto it = m_yAxes.constBegin(); it != m_yAxes.constEnd(); ++it) {
    if (it.value().side == side) {
      count++;
    }
  }
  return count;
}

void XYChartWidget::updateMargins() {
  if (!m_multiAxisEnabled) {
    return;
  }

  int leftCount = countAxesOnSide(Qt::AlignLeft);
  int rightCount = countAxesOnSide(Qt::AlignRight);

  // Increase margins to ensure axis labels aren't cut off
  // Base margins provide space for single axis, then add spacing for each
  // additional axis Use consistent spacing value from member variable
  // (m_axisSpacing)
  int baseLeftMargin =
      35; // Base space for left side (minimal, axes close to border)
  int baseRightMargin = 60; // Base space for right side

  // Calculate total margin needed: base + spacing for each additional axis
  // For left: axes stack to the left, so margin grows leftward
  m_leftMargin = baseLeftMargin + leftCount * m_axisSpacing;
  // For right: axes stack to the right, so margin grows rightward
  m_rightMargin = baseRightMargin + rightCount * m_axisSpacing;
}

QColor XYChartWidget::getNextColor() {
  static const QColor colors[] = {Qt::blue,       Qt::red,     Qt::green,
                                  Qt::magenta,    Qt::cyan,    Qt::darkYellow,
                                  Qt::darkBlue,   Qt::darkRed, Qt::darkGreen,
                                  Qt::darkMagenta};

  return colors[m_colorIndex++ % 10];
}
