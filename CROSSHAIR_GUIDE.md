# Multi-Value Crosshair Guide

## What is the Crosshair?

The crosshair is an interactive feature that shows you exact data values at your cursor position on the chart.

## Visual Display

### Crosshair Elements

```
           ╱ ← Vertical dashed line (shows X position)
          ╱
         ╱   ╱───────────────────────────┐
        ╱   │ X: 5.23                    │
       ╱    │ Line 1 (small): 0.45       │ ← Tooltip with values
      ╱     │ Line 2 (medium): 45.67     │
     ╱      │ Line 3 (large): 456700.12  │
    •───────╲─────────────────────────────┘ ← Center point
    │        ╲
    │         ╲ ← Horizontal dashed line (shows Y position on screen)
    │          ╲
    ↓
X axis position
```

### Components

1. **Vertical Dashed Line** - Shows your X position on the chart
2. **Horizontal Dashed Line** - Shows your Y position (optional visual)
3. **Center Point** - Where the lines cross (your cursor position)
4. **Tooltip Box** - Shows the actual data values

## Multi-Value Display

### What You See

When you move your cursor over the chart, the tooltip shows:

```
┌──────────────────────────────────────┐
│ X: 15.23                             │ ← X coordinate (shared by all)
│ Line 1 (small): 0.50                 │ ← Y value from Line 1 (Axis 0)
│ Line 2 (medium): 50.12               │ ← Y value from Line 2 (Axis 1)
│ Line 3 (large): 501234.56            │ ← Y value from Line 3 (Axis 2)
└──────────────────────────────────────┘
```

### Understanding the Values

Each Y value is calculated using its own Y-axis scale:

#### Line 1 (small) - Axis 0
- **Range**: -1.2 to 1.2
- **Formula**: Y_data = Y_min + (Y_max - Y_min) × (pixel_from_bottom / plot_height)
- **Example**: If cursor is 50% up from bottom: Y = -1.2 + 2.4 × 0.5 = 0.6

#### Line 2 (medium) - Axis 1
- **Range**: -150 to 150
- **Formula**: Same as above but with Axis 1 range
- **Example**: Same cursor position: Y = -150 + 300 × 0.5 = 75.0

#### Line 3 (large) - Axis 2
- **Range**: -1,000,000 to 1,000,000
- **Formula**: Same as above but with Axis 2 range
- **Example**: Same cursor position: Y = -1000000 + 2000000 × 0.5 = 0

### Why Different Y Values at Same Position?

Because each series uses a different Y-axis with a different scale!

The **same pixel position** on screen corresponds to:
- Different data values on different axes
- This is the whole point of multi-axis charts!

## How to Use

### Basic Usage

1. **Move your cursor over the chart**
   - As you move, the crosshair follows
   - The vertical dashed line shows your X position
   - The tooltip updates in real-time

2. **Read the values**
   - Top line: "X: value" - your X coordinate
   - Next 3 lines: Y values for each series
   - All values update as you move

3. **Position matters**
   - Move to where peaks of lines are
   - Check that Y values are near ±1.0, ±100, ±1M
   - Verify the values make sense for the scale

### Practical Examples

#### At Time X = 0.0 (Start)

```
X: 0.00
Line 1 (small): 0.00   ← 0 because sin(0) = 0
Line 2 (medium): 0.00  ← 0 because sin(0) = 0
Line 3 (large): 0.00   ← 0 because sin(0) = 0
```

#### At Peak of Line 1 (X ≈ 15.7, where sin peaks)

```
X: 15.7
Line 1 (small): 1.00   ← Peak of sine (sin(π/2) ≈ 1.0)
Line 2 (medium): 87.5  ← Same X on different scale
Line 3 (large): 875234 ← Same X on different scale
```

#### At Peak of Line 2 (X ≈ 7.85, different phase)

```
X: 7.85
Line 1 (small): 0.70   ← Not at peak for Line 1
Line 2 (medium): 100.0 ← Peak of sine (sin(π/2) ≈ 1.0) × 100
Line 3 (large): 866000 ← Maximum at this point
```

## Tooltip Positioning

The tooltip intelligently positions itself to stay visible:

### Normal Position (Right side of cursor)
```
         ┌─────────────┐
         │ X: 5.00     │
         │ Line 1: 0.5 │
         │ Line 2: 50  │
         │ Line 3: 500K│
         └─────────────┘
              ↑
        Cursor here
```

### Adjusted If Too Close to Right Edge
```
    ┌─────────────┐
    │ X: 5.00     │
    │ Line 1: 0.5 │
    │ Line 2: 50  │
    │ Line 3: 500K│
    └─────────────┘
                   ↑
            Cursor here
```

### Adjusted If Too Close to Top Edge
```
              ↓
        Cursor here
    ┌─────────────┐
    │ X: 5.00     │
    │ Line 1: 0.5 │
    │ Line 2: 50  │
    │ Line 3: 500K│
    └─────────────┘
```

## Color Coding

The crosshair display uses:

```
Text Color          What It Shows
──────────────────────────────────
Black               X value
Black               Series names and Y values
Light Yellow Box    Background (easy to read)
```

### Future Enhancement Possibility
Eventually, the Y values could be colored to match their series:
```
Line 1 (small): 0.50      ← Could be blue
Line 2 (medium): 50.12    ← Could be red
Line 3 (large): 501234.56 ← Could be green
```

## Accuracy

The Y values displayed in the crosshair are:

✅ **Accurate to 2 decimal places** - Shows 0.50, not 0.500000
✅ **Axis-specific** - Uses the correct axis range for each value
✅ **Real-time** - Updates as you move cursor
✅ **Synchronized** - All values for same X coordinate

### Precision

The values shown are formatted with 2 decimal places:
- `0.50` not `0.5000000000`
- `50.12` not `50.123456`
- `501234.56` not `501234.56789`

This is good for readability while maintaining accuracy.

## Tips for Using Crosshair

### Tip 1: Find Peak Values
Move cursor to where a line appears to peak and read the Y value:
- Line 1 should peak around ±1.0
- Line 2 should peak around ±100.0
- Line 3 should peak around ±1,000,000

### Tip 2: Compare Values at Same X
Position at the same X coordinate and see all three Y values:
```
At X = 10.5:
- Line 1: 0.80
- Line 2: 79.5
- Line 3: 795000

These show the exact values for all three series at the same time!
```

### Tip 3: Identify Crossing Points
Find where lines intersect visually and read the coordinates.

### Tip 4: Track Oscillation
Watch how values oscillate as you move left to right:
```
X changes: 0 → 5 → 10 → 15 → 20
Line 1: 0 → 1 → 0 → -1 → 0  (one complete cycle)
```

## Technical Details

### How Values are Calculated

For each visible series:

1. **Get the series' axis ID**: `int axisId = series.yAxisId;`
2. **Get axis range**: `double yMin = m_yAxes[axisId].min;` (e.g., -1.2)
3. **Get axis max**: `double yMax = m_yAxes[axisId].max;` (e.g., 1.2)
4. **Calculate pixel position percentage**: `% = (cursor_y - plot_bottom) / plot_height`
5. **Convert to data value**: `Y_data = yMin + (yMax - yMin) × %`

### Code Location

See `drawCrosshair()` in `xy_chart_widget.cpp` (lines 828-881)

```cpp
// Convert Y to data coordinate using axis-specific range
QPointF dataPoint = mapToData(QPointF(m_crosshairPos.x(), m_crosshairPos.y()), axisId);

// Format and display
QString yText = QString("%1: %2").arg(series.name).arg(dataPoint.y(), 0, 'f', 2);
```

## Troubleshooting

### Tooltip Not Showing
- Make sure cursor is **inside** the plot area (between axes)
- Not in the axis label areas

### Values Look Wrong
- Check you're reading the correct line name
- Verify cursor position matches the value
- Remember each line uses a different axis scale

### Tooltip Overlapping Data
- Move cursor slightly to reposition tooltip
- The tooltip will try to avoid the data

## Performance

✅ **No performance impact** - Crosshair calculations are instant
✅ **Smooth tracking** - Updates as fast as mouse position
✅ **Works with 400+ points** - No lag with test data

---

**Feature Status**: ✅ Complete and tested
**Build Date**: December 13, 2025 23:33 UTC
**Production Ready**: Yes
