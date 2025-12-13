# Multi-Axis Graph Implementation Details

## Overview

The multi-axis chart widget now supports displaying multiple independent Y-axes on the same chart, each with its own scale, labels, and series assignments. This allows plotting data with vastly different value ranges (e.g., -1 to 1, -100 to 100, -1M to 1M) on the same visualization.

## Architecture

### Data Structures

#### YAxisInfo Structure
```cpp
struct YAxisInfo {
    int axisId;                    // Unique axis identifier
    QString label;                 // Axis label (e.g., "Line 1 (Largest Scale)")
    double min, max;               // Axis range
    QColor color;                  // Axis line color
    Qt::AlignmentFlag side;        // Qt::AlignLeft or Qt::AlignRight
    QStringList seriesNames;       // Series assigned to this axis
    bool autoScale;                // Enable automatic scaling
};
```

#### Enhanced DataSeries Structure
```cpp
struct DataSeries {
    QString name;                  // Series name
    QList<QPointF> points;        // Data points
    QColor color;                  // Line color
    QPen pen;                      // Drawing pen
    bool visible;                  // Visibility flag
    int yAxisId;                   // Y-axis assignment (NEW)
};
```

### Member Variables

```cpp
QMap<int, YAxisInfo> m_yAxes;     // All Y-axes (old: m_yMin, m_yMax)
int m_nextAxisId;                 // Counter for generating axis IDs
bool m_multiAxisEnabled;          // Enable/disable multi-axis mode
bool m_autoGroupSeries;           // Enable automatic series grouping
int m_maxYAxes;                   // Maximum number of Y-axes (default: 4)
double m_groupingThreshold;       // Range similarity threshold
int m_axisSpacing;                // Spacing between axes (default: 60px)
```

## Rendering Pipeline

### 1. Chart Drawing Order

```cpp
void XYChartWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBackground(painter);    // Clear background
    drawGrid(painter);          // Draw grid lines
    drawAxes(painter);          // Draw all Y-axes (MULTI-AXIS SUPPORT)
    drawSeries(painter);        // Draw data series
    drawLegend(painter);        // Draw legend
    drawCrosshair(painter);     // Draw crosshair
}
```

### 2. Axis Rendering: drawAxes()

**New multi-axis logic:**

```cpp
void XYChartWidget::drawAxes(QPainter &painter)
{
    // Draw X-axis (horizontal)
    painter.drawLine(m_leftMargin, height() - m_bottomMargin,
                     width() - m_rightMargin, height() - m_bottomMargin);

    // Draw all Y-axes
    if (m_multiAxisEnabled) {
        // Loop through all axes and render each one
        for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
            drawYAxis(painter, it.value());  // Render individual axis
        }
    } else {
        // Single-axis mode (backward compatible)
        painter.drawLine(m_leftMargin, m_topMargin,
                         m_leftMargin, height() - m_bottomMargin);
    }

    // Draw axis labels (handled in drawYAxis() for multi-axis)
    // ...
}
```

### 3. Individual Axis Rendering: drawYAxis()

**Algorithm for positioning multiple axes:**

```cpp
void XYChartWidget::drawYAxis(QPainter &painter, const YAxisInfo &axis)
{
    // Step 1: Calculate position based on axis side
    int xPos = m_leftMargin;
    int axisWidth = 50;  // Pixels per axis column

    if (axis.side == Qt::AlignRight) {
        // Right-side axes positioned right of plot
        xPos = width() - m_rightMargin;
    } else {
        // Left-side axes stack leftward from m_leftMargin
        // Count how many left-side axes come before this one
        int leftCount = 0;
        for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
            if (it.value().axisId == axis.axisId) break;
            if (it.value().side == Qt::AlignLeft) leftCount++;
        }
        // Position: m_leftMargin - (count+1)*width
        xPos = m_leftMargin - (leftCount + 1) * axisWidth;
    }

    // Step 2: Draw axis line with color
    painter.setPen(QPen(axis.color, 2));
    painter.drawLine(xPos, m_topMargin, xPos, height() - m_bottomMargin);

    // Step 3: Draw axis labels and ticks
    painter.setPen(axis.color);

    // Axis name (rotated)
    painter.save();
    painter.translate(xPos - 25, m_topMargin - 5);
    painter.rotate(-90);
    painter.drawText(0, 0, 30, 15, Qt::AlignCenter, axis.label);
    painter.restore();

    // Tick marks and value labels
    for (int i = 0; i <= 10; ++i) {
        double value = axis.min + (axis.max - axis.min) * i / 10.0;
        int y = height() - m_bottomMargin -
                (height() - m_topMargin - m_bottomMargin) * i / 10.0;

        // Draw tick mark
        painter.drawLine(xPos - 3, y, xPos + 3, y);

        // Draw value label
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
```

### 4. Series Rendering with Axis-Specific Mapping

```cpp
void XYChartWidget::drawSeries(QPainter &painter)
{
    for (const auto &series : m_series) {
        if (!series.visible || series.points.size() < 2) continue;

        painter.setPen(series.pen);

        // Get the Y-axis this series is assigned to
        int axisId = series.yAxisId;

        // Draw lines connecting points
        for (int i = 0; i < series.points.size() - 1; ++i) {
            // Transform data coordinates using axis-specific mapping
            QPointF p1 = mapToWidget(series.points[i], axisId);
            QPointF p2 = mapToWidget(series.points[i + 1], axisId);
            painter.drawLine(p1, p2);
        }

        // Draw point markers
        painter.setBrush(series.color);
        for (const QPointF &point : series.points) {
            QPointF widgetPoint = mapToWidget(point, axisId);
            painter.drawEllipse(widgetPoint, 3, 3);
        }
    }
}
```

## Coordinate Transformation

### mapToWidget(point, axisId)

Converts data coordinates to widget (screen) coordinates using axis-specific Y range:

```cpp
QPointF XYChartWidget::mapToWidget(const QPointF &dataPoint, int axisId) const
{
    // Plot area dimensions
    int plotWidth = width() - m_leftMargin - m_rightMargin;
    int plotHeight = height() - m_topMargin - m_bottomMargin;

    // X coordinate (same for all axes)
    double xRange = m_xMax - m_xMin;
    if (xRange < 1e-10) xRange = 1.0;
    double x = m_leftMargin + (dataPoint.x() - m_xMin) / xRange * plotWidth;

    // Y coordinate (axis-specific)
    double yRange = 10.0;
    double yMin = 0.0, yMax = 10.0;

    if (m_yAxes.contains(axisId)) {
        yMin = m_yAxes[axisId].min;      // Use axis-specific minimum
        yMax = m_yAxes[axisId].max;      // Use axis-specific maximum
        yRange = yMax - yMin;
    }

    if (yRange < 1e-10) yRange = 1.0;

    // Transform: data Y → widget Y (inverted because Y increases downward)
    double y = height() - m_bottomMargin -
               (dataPoint.y() - yMin) / yRange * plotHeight;

    return QPointF(x, y);
}
```

### Example Transformation

For **Line 1 (blue)** with **axis 0** (range [-1.2, 1.2]):
- Data point: (5.0, 0.5)
- Y range: 1.2 - (-1.2) = 2.4
- Normalized Y: (0.5 - (-1.2)) / 2.4 = 1.7 / 2.4 ≈ 0.708 (about 70% up)
- Widget Y: plotBottom - 0.708 × plotHeight

For **Line 2 (red)** with **axis 1** (range [-150, 150]):
- Data point: (5.0, 75.0)
- Y range: 150 - (-150) = 300
- Normalized Y: (75 - (-150)) / 300 = 225 / 300 = 0.75 (75% up)
- Widget Y: plotBottom - 0.75 × plotHeight

For **Line 3 (green)** with **axis 2** (range [-1M, 1M]):
- Data point: (5.0, 500000.0)
- Y range: 1M - (-1M) = 2M
- Normalized Y: (500000 - (-1000000)) / 2000000 = 1500000 / 2000000 = 0.75 (75% up)
- Widget Y: plotBottom - 0.75 × plotHeight

## Layout and Margins

### Automatic Margin Calculation

```cpp
void XYChartWidget::updateMargins()
{
    if (!m_multiAxisEnabled) {
        return;  // Use default margins
    }

    // Count axes on each side
    int leftCount = countAxesOnSide(Qt::AlignLeft);
    int rightCount = countAxesOnSide(Qt::AlignRight);

    // Allocate 60 pixels per axis + 60 pixel base
    m_leftMargin = 60 + qMax(0, leftCount - 1) * m_axisSpacing;
    m_rightMargin = 20 + rightCount * m_axisSpacing;
}
```

### Visual Layout Example

For 3 left-side axes and 0 right-side axes:

```
Screen coordinates:
0                                    width
|                                      |
[Axis2][Axis1][Axis0]|=========Plot Area=========|
  50px  50px   60px   |                            | margin
         ↑            ↑
     stacked     m_leftMargin = 60 + 2*60 = 180
     outward
```

## Usage Example

### Setting Up Multi-Axis Chart

```cpp
// Create chart widget
XYChartWidget *chart = new XYChartWidget(this);
chart->setTitle("Multi-Axis Chart");

// Enable multi-axis mode
chart->setMultiAxisEnabled(true);

// Create three Y-axes
chart->createYAxis(0, "Line 1 (Largest Scale)", Qt::AlignLeft);
chart->createYAxis(1, "Line 2 (Medium Scale)", Qt::AlignRight);
chart->createYAxis(2, "Line 3 (Smallest Scale)", Qt::AlignLeft);

// Set axis ranges
chart->setAxisRange(0, -1.2, 1.2);        // Line 1 range
chart->setAxisRange(1, -150, 150);        // Line 2 range
chart->setAxisRange(2, -1000000, 1000000); // Line 3 range

// Add series
chart->addSeries("Line 1 (small)", Qt::blue);
chart->addSeries("Line 2 (medium)", Qt::red);
chart->addSeries("Line 3 (large)", Qt::green);

// Assign series to axes
chart->assignSeriesToAxis("Line 1 (small)", 0);
chart->assignSeriesToAxis("Line 2 (medium)", 1);
chart->assignSeriesToAxis("Line 3 (large)", 2);

// Add data points
for (int i = 0; i < 100; ++i) {
    double t = i * 0.1;
    chart->addPoint("Line 1 (small)", t, qSin(t * 0.1));
    chart->addPoint("Line 2 (medium)", t, 100 * qSin(t * 0.2));
    chart->addPoint("Line 3 (large)", t, 1000000 * qSin(t * 0.15));
}
```

## Performance Considerations

- **Axis Rendering**: O(n) where n = number of axes (typically 3-4)
- **Coordinate Mapping**: O(1) per point (lookup in QMap is O(log n))
- **Series Rendering**: O(m) where m = total data points
- **Overall**: Negligible performance impact compared to data rendering

## Future Enhancements

1. **Full Auto-Grouping**: Implement clustering algorithm to automatically group series by value range
2. **Right-Side Axes**: Complete support for right-side axis positioning
3. **Axis Scaling**: UI controls to manually adjust axis ranges
4. **Crosshair Multi-Axis**: Show Y values from all axes at cursor position
5. **Export Formats**: Support PNG, JPG, SVG, PDF export with proper scaling
6. **Scrolling**: Enable horizontal/vertical panning for large datasets

## Known Limitations (v1.0)

- Auto-grouping currently assigns all series to axis 0
- Crosshair Y-value display only shows axis 0 value
- Right-side axis positioning is allocated but not fully rendered
- No UI for adjusting axis ranges post-creation

---

**Implementation Status**: ✅ Complete for basic multi-axis support
**Tested with**: 3 Y-axes with vastly different ranges
**Backward Compatible**: Yes - defaults to single-axis mode
