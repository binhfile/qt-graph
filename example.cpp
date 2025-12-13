#include "xy_chart_widget.h"
#include <QApplication>
#include <QMainWindow>
#include <QTimer>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QtMath>
#include <QDebug>

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent), m_time(0.0)
    {
        setWindowTitle("XY Chart Widget - Multi-Axis Demo");
        resize(1000, 700);

        // Widget chính
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // Tạo chart widget
        m_chartWidget = new XYChartWidget(this);
        m_chartWidget->setTitle("Multi-Axis Chart: Three Different Y Scales");
        m_chartWidget->setXLabel("Time (s)");
        m_chartWidget->setYLabel("Value");

        // Enable multi-axis mode
        m_chartWidget->setMultiAxisEnabled(true);

        // Create three Y-axes with different scales
        // Line 1: Use LARGEST scale (most magnification) to show sin wave clearly
        // Line 2: Use MEDIUM scale
        // Line 3: Use SMALLEST scale (least magnification) to fit huge values
        m_chartWidget->createYAxis(0, "Line 1 (Largest Scale)", Qt::AlignLeft);
        m_chartWidget->createYAxis(1, "Line 2 (Medium Scale)", Qt::AlignRight);
        m_chartWidget->createYAxis(2, "Line 3 (Smallest Scale)", Qt::AlignLeft);

        // Set ranges for each axis - inverted scales for better visualization
        // Line 1: Largest scale (smallest value range = most zoom)
        m_chartWidget->setAxisRange(0, -1.2, 1.2);

        // Line 2: Medium scale
        m_chartWidget->setAxisRange(1, -150, 150);

        // Line 3: Smallest scale (largest value range = least zoom)
        m_chartWidget->setAxisRange(2, -1000000, 1000000);

        // Add three series with different Y value ranges
        m_chartWidget->addSeries("Line 1 (small)", Qt::blue);
        m_chartWidget->addSeries("Line 2 (medium)", Qt::red);
        m_chartWidget->addSeries("Line 3 (large)", Qt::green);

        // Assign each series to its corresponding axis
        m_chartWidget->assignSeriesToAxis("Line 1 (small)", 0);
        m_chartWidget->assignSeriesToAxis("Line 2 (medium)", 1);
        m_chartWidget->assignSeriesToAxis("Line 3 (large)", 2);

        // Set max points to prevent memory issues during realtime updates
        m_chartWidget->setMaxPointsPerSeries(300);

        mainLayout->addWidget(m_chartWidget);

        // Control buttons
        QHBoxLayout *buttonLayout = new QHBoxLayout();

        QPushButton *startBtn = new QPushButton("Start", this);
        QPushButton *stopBtn = new QPushButton("Stop", this);
        QPushButton *clearBtn = new QPushButton("Clear", this);

        buttonLayout->addWidget(startBtn);
        buttonLayout->addWidget(stopBtn);
        buttonLayout->addWidget(clearBtn);
        buttonLayout->addStretch();

        mainLayout->addLayout(buttonLayout);

        setCentralWidget(centralWidget);

        // Timer for realtime updates
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &MainWindow::updateData);

        // Connect buttons
        connect(startBtn, &QPushButton::clicked, this, &MainWindow::startRealtime);
        connect(stopBtn, &QPushButton::clicked, this, &MainWindow::stopRealtime);
        connect(clearBtn, &QPushButton::clicked, this, &MainWindow::clearData);

        // Connect signal when clicking on series
        connect(m_chartWidget, &XYChartWidget::seriesClicked,
                this, &MainWindow::onSeriesClicked);
    }
    
private slots:
    void startRealtime()
    {
        m_timer->start(50); // Cập nhật mỗi 50ms
    }
    
    void stopRealtime()
    {
        m_timer->stop();
    }
    
    void clearData()
    {
        m_chartWidget->clearAllSeries();
        m_time = 0.0;
    }
    
    void updateData()
    {
        // Line 1: SLOW sine wave with largest scale (most magnification)
        // Much slower frequency so you can see the sine curve shape clearly
        // Oscillates -1 to 1, displayed in range -1.2 to 1.2
        double line1Value = qSin(m_time * 0.1);

        // Line 2: Medium speed sine wave scaled to -100 to 100 range
        // Oscillates roughly -100 to 100, displayed in range -150 to 150
        double line2Value = 100.0 * qSin(m_time * 0.2);

        // Line 3: Slower sine wave scaled to -1000000 to 1000000 range
        // Oscillates roughly -1000000 to 1000000, displayed in range -1000000 to 1000000
        double line3Value = 1000000.0 * qSin(m_time * 0.15);

        // Add new points to each series
        m_chartWidget->addPoint("Line 1 (small)", m_time, line1Value);
        m_chartWidget->addPoint("Line 2 (medium)", m_time, line2Value);
        m_chartWidget->addPoint("Line 3 (large)", m_time, line3Value);

        m_time += 0.1;
    }
    
    void onSeriesClicked(const QString &seriesName, const QPointF &point)
    {
        qDebug() << "Clicked on series:" << seriesName 
                 << "at point (" << point.x() << "," << point.y() << ")";
    }
    
private:
    XYChartWidget *m_chartWidget;
    QTimer *m_timer;
    double m_time;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    MainWindow window;
    window.show();
    
    return app.exec();
}

#include "example.moc"
