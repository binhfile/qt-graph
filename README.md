# XY Chart Widget - Qt Widget vẽ biểu đồ XY

Widget Qt để vẽ biểu đồ đường XY với hỗ trợ nhiều tập dữ liệu và cập nhật realtime.

## Tính năng

✨ **Tính năng chính:**
- Vẽ nhiều tập dữ liệu (series) trên cùng một biểu đồ
- Cập nhật dữ liệu realtime
- Tự động scale hoặc scale thủ công
- Hiển thị lưới và nhãn trục tùy chỉnh
- Chú thích (legend) tự động
- Giới hạn số điểm để quản lý bộ nhớ
- Tương tác chuột (click vào điểm)
- Tùy chỉnh màu sắc và độ dày đường

## Cấu trúc file

```
xy_chart_widget.h      - File header của class
xy_chart_widget.cpp    - Implementation
example.cpp            - Ví dụ sử dụng với cập nhật realtime
CMakeLists.txt         - File build cho CMake
xy_chart_widget.pro    - File project cho qmake
```

## Build

### Sử dụng CMake:

```bash
mkdir build
cd build
cmake ..
make
./example
```

### Sử dụng qmake:

```bash
qmake
make
./bin/example
```

## Cách sử dụng

### 1. Khởi tạo widget

```cpp
#include "xy_chart_widget.h"

XYChartWidget *chart = new XYChartWidget(parent);
chart->setTitle("Biểu đồ của tôi");
chart->setXLabel("Thời gian (s)");
chart->setYLabel("Giá trị");
```

### 2. Thêm series dữ liệu

```cpp
// Thêm series mới với màu tùy chỉnh
chart->addSeries("Dữ liệu 1", Qt::blue);
chart->addSeries("Dữ liệu 2", Qt::red);
chart->addSeries("Dữ liệu 3", Qt::green);
```

### 3. Thêm dữ liệu

```cpp
// Thêm một điểm
chart->addPoint("Dữ liệu 1", x, y);

// Thêm nhiều điểm
QVector<QPointF> points;
for (double x = 0; x < 10; x += 0.1) {
    points.append(QPointF(x, sin(x)));
}
chart->addPoints("Dữ liệu 1", points);

// Hoặc cập nhật toàn bộ dữ liệu
chart->setSeriesData("Dữ liệu 1", points);
```

### 4. Cập nhật Realtime

```cpp
// Giới hạn số điểm tối đa (để tránh tràn bộ nhớ)
chart->setMaxPointsPerSeries(200);

// Trong slot cập nhật (ví dụ từ QTimer)
void updateData() {
    double newValue = calculateValue();
    chart->addPoint("Dữ liệu 1", currentTime, newValue);
    currentTime += 0.1;
}

// Kết nối với timer
QTimer *timer = new QTimer(this);
connect(timer, &QTimer::timeout, this, &MyClass::updateData);
timer->start(50); // Cập nhật mỗi 50ms
```

### 5. Tùy chỉnh hiển thị

```cpp
// Bật/tắt auto scale
chart->setAutoScale(true);

// Hoặc đặt range thủ công
chart->setXRange(0, 100);
chart->setYRange(-1, 1);

// Hiển thị/ẩn lưới
chart->setGridVisible(true);

// Tùy chỉnh màu sắc và độ dày
chart->setSeriesColor("Dữ liệu 1", Qt::darkBlue);
chart->setSeriesPenWidth("Dữ liệu 1", 3);

// Ẩn/hiện một series
chart->setSeriesVisible("Dữ liệu 1", false);
```

### 6. Xóa dữ liệu

```cpp
// Xóa dữ liệu của một series
chart->clearSeries("Dữ liệu 1");

// Xóa tất cả dữ liệu
chart->clearAllSeries();

// Xóa series hoàn toàn
chart->removeSeries("Dữ liệu 1");
```

### 7. Xử lý sự kiện click

```cpp
connect(chart, &XYChartWidget::seriesClicked, 
        this, [](const QString &name, const QPointF &point) {
    qDebug() << "Clicked on" << name << "at" << point;
});
```

## API Reference

### Quản lý Series

| Phương thức | Mô tả |
|------------|-------|
| `addSeries(name, color)` | Thêm series mới |
| `removeSeries(name)` | Xóa series |
| `setSeriesVisible(name, visible)` | Hiện/ẩn series |
| `setSeriesColor(name, color)` | Đặt màu |
| `setSeriesPenWidth(name, width)` | Đặt độ dày |

### Thêm dữ liệu

| Phương thức | Mô tả |
|------------|-------|
| `addPoint(name, x, y)` | Thêm 1 điểm |
| `addPoints(name, points)` | Thêm nhiều điểm |
| `setSeriesData(name, points)` | Cập nhật toàn bộ |

### Xóa dữ liệu

| Phương thức | Mô tả |
|------------|-------|
| `clearSeries(name)` | Xóa dữ liệu series |
| `clearAllSeries()` | Xóa tất cả |

### Cài đặt hiển thị

| Phương thức | Mô tả |
|------------|-------|
| `setXRange(min, max)` | Đặt phạm vi X |
| `setYRange(min, max)` | Đặt phạm vi Y |
| `setAutoScale(enable)` | Bật auto scale |
| `setMaxPointsPerSeries(max)` | Giới hạn điểm |
| `setGridVisible(visible)` | Hiện lưới |
| `setAxisLabelsVisible(visible)` | Hiện nhãn |
| `setTitle(title)` | Đặt tiêu đề |
| `setXLabel(label)` | Nhãn trục X |
| `setYLabel(label)` | Nhãn trục Y |

### Lấy thông tin

| Phương thức | Mô tả |
|------------|-------|
| `getSeriesNames()` | Danh sách tên series |
| `getSeriesCount()` | Số lượng series |

### Signals

| Signal | Mô tả |
|--------|-------|
| `seriesClicked(name, point)` | Khi click vào điểm |

## Ví dụ nâng cao

### Vẽ nhiều đường sin/cos realtime

```cpp
QTimer *timer = new QTimer(this);
double time = 0.0;

connect(timer, &QTimer::timeout, [&]() {
    chart->addPoint("Sin", time, sin(time));
    chart->addPoint("Cos", time, cos(time));
    chart->addPoint("Tan", time, tan(time) / 10.0); // Scale down
    time += 0.1;
});

chart->setMaxPointsPerSeries(300);
timer->start(50);
```

### Vẽ dữ liệu từ sensor

```cpp
// Giả sử có hàm đọc sensor
void onSensorData(double temperature, double humidity) {
    static double timestamp = 0.0;
    
    chart->addPoint("Temperature", timestamp, temperature);
    chart->addPoint("Humidity", timestamp, humidity);
    
    timestamp += 1.0; // Tăng 1 giây
}
```

## Yêu cầu

- Qt 5.x hoặc Qt 6.x
- C++11 trở lên
- Modules: QtCore, QtWidgets

## License

MIT License - Tự do sử dụng cho mục đích cá nhân và thương mại.

## Tác giả

Created with Claude AI Assistant
