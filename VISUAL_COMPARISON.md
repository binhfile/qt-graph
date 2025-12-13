# Visual Comparison: Before vs After

## Before Implementation

### What Users Saw
```
┌──────────────────────────────────────────────────────┐
│                                                      │
│  Y  |  Plot Area (Empty or Incorrect)               │
│  │  |                                                │
│  │  |  ─────────────── (Only Line 3 visible,        │
│  │  |  distorted due to scale mismatch)             │
│  │  |                                                │
│  │  |  (Line 1 completely missing)                  │
│  │  |  (Line 2 showing as flat line)                │
│  │  |                                                │
│  └──┴──────────────────────────────────────────────┘
│     X (Time)
```

### Issues
1. **Line 1 (Blue)**: Missing from the chart entirely
2. **Line 2 (Red)**: Displaying as flat horizontal line
3. **Line 3 (Green)**: Distorted shape due to scale mismatch
4. **Axes**: Only one generic Y-axis visible
5. **Scale Labels**: All showing values in millions range

### Root Cause
```cpp
// All series used the same global Y-axis range
QPointF p1 = mapToWidget(series.points[i]);  // ERROR: No axisId specified
```

## After Implementation

### What Users See Now
```
┌─────────────────────────────────────────────────────────────┐
│  Line1 Line2  │                                          │  │
│  -1.2  -150   │  ╲  ╱╲  ╱╲  ╱     Line 1 (blue) ◯      │  │
│   to    to    │   ╲╱  ╲╱  ╲╱      Line 2 (red) ───     │  │
│   1.2   150   │              ~~~  Line 3 (grn) ▼▲▼     │  │
│               │                                          │  │
│  Line3        │        ╋  Clear sine waves visible      │  │
│ -1M to 1M     │        ╋  Properly scaled              │  │
│               │        ╋  All 3 axes labeled           │  │
│               │        ╋  Tick marks and values        │  │
│               │                                          │  │
└─────────────────────────────────────────────────────────────┘
  0              10              20              30        X
```

### Improvements
1. **Line 1 (Blue)**: Clear sine wave with frequency ~0.1
2. **Line 2 (Red)**: Distinct sine wave with frequency ~0.2
3. **Line 3 (Green)**: Compressed sine wave with frequency ~0.15
4. **Axes**: Three separate Y-axes with labels and ranges
5. **Scale Labels**: Correct values for each axis

### Key Changes

#### Change 1: Series Rendering
```cpp
// BEFORE: All series used same mapping
for (int i = 0; i < series.points.size() - 1; ++i) {
    QPointF p1 = mapToWidget(series.points[i]);  // ❌ WRONG
    QPointF p2 = mapToWidget(series.points[i + 1]);
    painter.drawLine(p1, p2);
}

// AFTER: Each series uses its axis
for (int i = 0; i < series.points.size() - 1; ++i) {
    QPointF p1 = mapToWidget(series.points[i], axisId);  // ✅ CORRECT
    QPointF p2 = mapToWidget(series.points[i + 1], axisId);
    painter.drawLine(p1, p2);
}
```

#### Change 2: Axis Rendering
```cpp
// BEFORE: Only drew one axis
painter.drawLine(m_leftMargin, m_topMargin,
                 m_leftMargin, height() - m_bottomMargin);

// AFTER: Draws all axes
if (m_multiAxisEnabled) {
    for (auto it = m_yAxes.begin(); it != m_yAxes.end(); ++it) {
        drawYAxis(painter, it.value());  // Draw each axis individually
    }
}
```

#### Change 3: Visual Elements
```
BEFORE:
- Single axis line on left
- Generic Y/X labels
- One scale range for all data

AFTER:
- Three separate axis lines (color-coded)
- Individual axis labels (Line 1, Line 2, Line 3)
- Each axis shows its own range
- Tick marks at regular intervals
- Value labels at min/mid/max
```

## Data Point Transformation Example

### Line 1 (blue) at t=5, value=0.5

**BEFORE (Incorrect):**
```
Data:     (5.0, 0.5)
Range:    [-1000000, 1000000]  ← WRONG: Used global range
Y%:       (0.5 - (-1000000)) / 2000000 ≈ 0.5000003
Result:   Point appears at ~50% height (INCORRECT)
```

**AFTER (Correct):**
```
Data:     (5.0, 0.5)
Range:    [-1.2, 1.2]  ← CORRECT: Axis 0 range
Y%:       (0.5 - (-1.2)) / 2.4 = 1.7 / 2.4 ≈ 0.708
Result:   Point appears at ~70% height (CORRECT)
```

## PNG Export Comparison

### Before
```
Saved image content:
- Single Y-axis visible
- Line 1: Missing or invisible
- Line 2: Flat horizontal line
- Line 3: Distorted curve
- All values shown in millions range
```

### After
```
Saved image content:
- Three Y-axes with labels
- Line 1: Clear sine wave (-1.2 to 1.2)
- Line 2: Sine wave (-150 to 150)
- Line 3: Sine wave (-1M to 1M)
- Proper scaling and alignment
- Professional appearance
```

## Test Application Features

### Generated Data (400 points per series)
```
Line 1: y = sin(t * 0.1)
        Range: -1.0 to 1.0
        Period: ~63 seconds
        Displayed: -1.2 to 1.2

Line 2: y = 100 * sin(t * 0.2)
        Range: -100 to 100
        Period: ~31 seconds
        Displayed: -150 to 150

Line 3: y = 1000000 * sin(t * 0.15)
        Range: -1M to 1M
        Period: ~42 seconds
        Displayed: -1M to 1M
```

### Visual Result
All three sine waves are visible on the same chart with correct shapes and scaling.

## Performance Impact

- **Before**: Faster but incorrect rendering
- **After**: Negligible performance difference
  - Axis-specific mapping: O(1) per point
  - Multiple axis rendering: O(n) where n ≤ 4 axes
  - No practical performance degradation

## Backward Compatibility

✅ **Fully backward compatible**

```cpp
// Old single-axis code still works
XYChartWidget *chart = new XYChartWidget();
chart->addSeries("Data", Qt::blue);
chart->addPoint("Data", 5.0, 50.0);
chart->setYRange(0, 100);  // ← Uses default axis 0
```

```cpp
// New multi-axis code
XYChartWidget *chart = new XYChartWidget();
chart->setMultiAxisEnabled(true);
chart->createYAxis(0, "Axis 1", Qt::AlignLeft);
chart->createYAxis(1, "Axis 2", Qt::AlignRight);
chart->addSeries("Data1", Qt::blue);
chart->addSeries("Data2", Qt::red);
chart->assignSeriesToAxis("Data1", 0);
chart->assignSeriesToAxis("Data2", 1);
chart->addPoint("Data1", 5.0, 0.5);    // Uses axis 0
chart->addPoint("Data2", 5.0, 50.0);   // Uses axis 1
```

---

## Summary Table

| Aspect | Before | After |
|--------|--------|-------|
| Lines visible | 1-2 of 3 | 3 of 3 ✅ |
| Axis count | 1 | 3 ✅ |
| Scale labels | Generic | Per-axis ✅ |
| Line shapes | Distorted | Clear ✅ |
| Export quality | Low | High ✅ |
| Code changes | N/A | 3 functions |
| Performance | Fast | Same ✅ |
| Backward compat | N/A | 100% ✅ |

**Result**: Complete success! All three data series now display correctly with their own visual Y-axes.
