# Code Reference: Multi-Axis Implementation

## Quick Navigation

### Core Functions in xy_chart_widget.cpp

| Function | Location | Purpose |
|----------|----------|---------|
| `drawAxes()` | Lines 702-762 | Main axis rendering dispatcher |
| `drawYAxis()` | Lines 611-666 | Individual Y-axis renderer |
| `drawSeries()` | Lines 764-785 | Series rendering with axis mapping |
| `mapToWidget()` | Lines 545-568 | Axis-specific coordinate mapping |
| `createYAxis()` | Lines 305-326 | Create and register Y-axes |
| `assignSeriesToAxis()` | Lines 328-348 | Assign series to axes |
| `setAxisRange()` | Lines 350-360 | Set axis value range |
| `calculateAutoScaleForAxis()` | Lines 868-915 | Auto-scale individual axis |
| `updateMargins()` | Lines 993-1004 | Auto-adjust plot margins |
| `countAxesOnSide()` | Lines 983-991 | Count axes on left/right |

### Header File (xy_chart_widget.h)

#### Structures (Around line 40-60)
```cpp
struct YAxisInfo {
    int axisId;
    QString label;
    double min, max;
    QColor color;
    Qt::AlignmentFlag side;
    QStringList seriesNames;
    bool autoScale;
};

struct DataSeries {
    // ... existing members ...
    int yAxisId;  // NEW: Line 57
};
```

#### Member Variables (Around line 90-110)
```cpp
QMap<int, YAxisInfo> m_yAxes;        // Line 91
int m_nextAxisId;                    // Line 92
bool m_multiAxisEnabled;             // Line 93
bool m_autoGroupSeries;              // Line 94
int m_maxYAxes;                      // Line 95
double m_groupingThreshold;          // Line 96
int m_axisSpacing;                   // Line 97
```

#### Public Methods (Around line 140-160)
```cpp
void setMultiAxisEnabled(bool enabled);           // Line 141
void createYAxis(int axisId, const QString &label, // Line 142
                 Qt::AlignmentFlag side);
void assignSeriesToAxis(const QString &seriesName, // Line 144
                       int axisId);
int getAxisForSeries(const QString &seriesName);  // Line 146
void setAxisRange(int axisId, double min,         // Line 147
                  double max);
void setAxisAutoScale(int axisId, bool autoScale); // Line 149
QPointF mapToWidget(const QPointF &dataPoint,     // Line 151
                   int axisId) const;
QPointF mapToData(const QPointF &widgetPoint,     // Line 152
                 int axisId) const;
```

## Key Code Snippets

### 1. Enable Multi-Axis Mode
```cpp
// xy_chart_widget.cpp, line 289
void XYChartWidget::setMultiAxisEnabled(bool enabled)
{
    m_multiAxisEnabled = enabled;
    update();
}
```

### 2. Create Y-Axis
```cpp
// xy_chart_widget.cpp, lines 305-326
void XYChartWidget::createYAxis(int axisId, const QString &label, Qt::AlignmentFlag side)
{
    if (m_yAxes.contains(axisId)) {
        return;
    }

    YAxisInfo axis;
    axis.axisId = axisId;
    axis.label = label;
    axis.side = side;
    axis.color = getNextColor();
    axis.autoScale = true;
    axis.min = 0.0;
    axis.max = 10.0;

    m_yAxes[axisId] = axis;
    if (axisId >= m_nextAxisId) {
        m_nextAxisId = axisId + 1;
    }
    updateMargins();  // ← Auto-adjust plot area
    update();
}
```

### 3. Assign Series to Axis
```cpp
// xy_chart_widget.cpp, lines 328-348
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

    // Assign to new axis
    m_series[seriesName].yAxisId = axisId;
    if (m_yAxes.contains(axisId)) {
        if (!m_yAxes[axisId].seriesNames.contains(seriesName)) {
            m_yAxes[axisId].seriesNames.append(seriesName);
        }
    }

    calculateAutoScaleForAxis(axisId);
    update();
}
```

### 4. Render All Axes
```cpp
// xy_chart_widget.cpp, lines 710-715
if (m_multiAxisEnabled) {
    // Draw each axis in the multi-axis setup
    for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
        drawYAxis(painter, it.value());  // ← Render each axis
    }
}
```

### 5. Draw Individual Axis
```cpp
// xy_chart_widget.cpp, lines 611-666
void XYChartWidget::drawYAxis(QPainter &painter, const YAxisInfo &axis)
{
    // Calculate position based on side
    int xPos = m_leftMargin;
    int axisWidth = 50;

    if (axis.side == Qt::AlignRight) {
        xPos = width() - m_rightMargin;
    } else {
        // Count axes before this one on left side
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

    // Draw axis labels
    if (m_showAxisLabels) {
        // ... tick marks and value labels ...
    }
}
```

### 6. Render Series with Axis Mapping
```cpp
// xy_chart_widget.cpp, lines 764-785
void XYChartWidget::drawSeries(QPainter &painter)
{
    for (const auto &series : m_series) {
        if (!series.visible || series.points.size() < 2) continue;

        painter.setPen(series.pen);

        // Get the Y-axis this series is assigned to
        int axisId = series.yAxisId;  // ← Key change

        // Draw lines using axis-specific mapping
        for (int i = 0; i < series.points.size() - 1; ++i) {
            QPointF p1 = mapToWidget(series.points[i], axisId);  // ← Axis-specific
            QPointF p2 = mapToWidget(series.points[i + 1], axisId);
            painter.drawLine(p1, p2);
        }

        // Draw point markers
        painter.setBrush(series.color);
        for (const QPointF &point : series.points) {
            QPointF widgetPoint = mapToWidget(point, axisId);  // ← Axis-specific
            painter.drawEllipse(widgetPoint, 3, 3);
        }
    }
}
```

### 7. Coordinate Mapping
```cpp
// xy_chart_widget.cpp, lines 545-568
QPointF XYChartWidget::mapToWidget(const QPointF &dataPoint, int axisId) const
{
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
        yMin = m_yAxes[axisId].min;      // ← Axis-specific min
        yMax = m_yAxes[axisId].max;      // ← Axis-specific max
        yRange = yMax - yMin;
    }

    if (yRange < 1e-10) yRange = 1.0;

    // Transform to widget coordinates
    double y = height() - m_bottomMargin -
               (dataPoint.y() - yMin) / yRange * plotHeight;

    return QPointF(x, y);
}
```

### 8. Auto-Scale Individual Axis
```cpp
// xy_chart_widget.cpp, lines 868-915
void XYChartWidget::calculateAutoScaleForAxis(int axisId)
{
    if (!m_yAxes.contains(axisId)) {
        return;
    }

    YAxisInfo &axis = m_yAxes[axisId];
    if (!axis.autoScale) {
        return;
    }

    // Find min/max from series assigned to this axis
    double minVal = std::numeric_limits<double>::max();
    double maxVal = std::numeric_limits<double>::lowest();

    for (const QString &seriesName : axis.seriesNames) {
        if (!m_series.contains(seriesName)) continue;
        const DataSeries &series = m_series[seriesName];

        for (const QPointF &point : series.points) {
            minVal = qMin(minVal, point.y());
            maxVal = qMax(maxVal, point.y());
        }
    }

    // Set axis range with 10% padding
    if (minVal <= maxVal) {
        double padding = (maxVal - minVal) * 0.1;
        axis.min = minVal - padding;
        axis.max = maxVal + padding;
    }
}
```

### 9. Adjust Margins
```cpp
// xy_chart_widget.cpp, lines 993-1004
void XYChartWidget::updateMargins()
{
    if (!m_multiAxisEnabled) {
        return;  // Keep default margins
    }

    int leftCount = countAxesOnSide(Qt::AlignLeft);
    int rightCount = countAxesOnSide(Qt::AlignRight);

    // Base margin + spacing per axis
    m_leftMargin = 60 + qMax(0, leftCount - 1) * m_axisSpacing;
    m_rightMargin = 20 + rightCount * m_axisSpacing;
}
```

## Application Integration

### example.cpp (Main Demo)
```cpp
// Lines 32-40: Multi-axis setup
m_chartWidget->setMultiAxisEnabled(true);
m_chartWidget->createYAxis(0, "Line 1 (Largest Scale)", Qt::AlignLeft);
m_chartWidget->createYAxis(1, "Line 2 (Medium Scale)", Qt::AlignRight);
m_chartWidget->createYAxis(2, "Line 3 (Smallest Scale)", Qt::AlignLeft);

// Lines 43-50: Set ranges
m_chartWidget->setAxisRange(0, -1.2, 1.2);
m_chartWidget->setAxisRange(1, -150, 150);
m_chartWidget->setAxisRange(2, -1000000, 1000000);

// Lines 53-60: Assign series
m_chartWidget->assignSeriesToAxis("Line 1 (small)", 0);
m_chartWidget->assignSeriesToAxis("Line 2 (medium)", 1);
m_chartWidget->assignSeriesToAxis("Line 3 (large)", 2);
```

### test_save_graph.cpp (Test Application)
```cpp
// Lines 35-55: Complete multi-axis setup
m_chartWidget->setMultiAxisEnabled(true);
m_chartWidget->createYAxis(0, "Line 1 (Largest Scale)", Qt::AlignLeft);
m_chartWidget->createYAxis(1, "Line 2 (Medium Scale)", Qt::AlignRight);
m_chartWidget->createYAxis(2, "Line 3 (Smallest Scale)", Qt::AlignLeft);

m_chartWidget->setAxisRange(0, -1.2, 1.2);
m_chartWidget->setAxisRange(1, -150, 150);
m_chartWidget->setAxisRange(2, -1000000, 1000000);

m_chartWidget->addSeries("Line 1 (small)", Qt::blue);
m_chartWidget->addSeries("Line 2 (medium)", Qt::red);
m_chartWidget->addSeries("Line 3 (large)", Qt::green);

m_chartWidget->assignSeriesToAxis("Line 1 (small)", 0);
m_chartWidget->assignSeriesToAxis("Line 2 (medium)", 1);
m_chartWidget->assignSeriesToAxis("Line 3 (large)", 2);
```

## Compilation Flags

The project compiles with these settings:
- **Compiler**: MSVC 2022 BuildTools
- **C++ Standard**: Qt 5.12.12 compatible
- **Optimization**: Release build (O2)
- **Warnings**: /W3 (Level 3, with minor unused variable warnings)

## Testing Checklist

- [x] Multi-axis creation
- [x] Series assignment to axes
- [x] Axis-specific coordinate mapping
- [x] Visual axis rendering
- [x] Auto-margin calculation
- [x] PNG export with all axes
- [x] Backward compatibility
- [x] Build without critical errors

---

For more details, see:
- `MULTI_AXIS_IMPLEMENTATION.md` - Technical deep dive
- `FIX_SUMMARY.md` - Implementation summary
- `VISUAL_COMPARISON.md` - Before/after comparison
