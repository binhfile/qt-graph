#include "xy_chart_widget.h"
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QPainter>
#include <QFileDialog>
#include <QDateTime>
#include <QDebug>
#include <QTimer>
#include <QtMath>

class TestSaveGraphWindow : public QMainWindow
{
    Q_OBJECT

public:
    TestSaveGraphWindow(QWidget *parent = nullptr) : QMainWindow(parent), m_time(0.0), m_pointCount(0)
    {
        setWindowTitle("Test: Save Graph to Image File");
        resize(1200, 800);

        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // Create chart widget
        m_chartWidget = new XYChartWidget(this);
        m_chartWidget->setTitle("Test Graph - Multi-Axis Sine Waves");
        m_chartWidget->setXLabel("Time (s)");
        m_chartWidget->setYLabel("Value");

        // Enable multi-axis mode
        m_chartWidget->setMultiAxisEnabled(true);

        // Create three Y-axes with different scales
        m_chartWidget->createYAxis(0, "Line 1 (Largest Scale)", Qt::AlignLeft);
        m_chartWidget->createYAxis(1, "Line 2 (Medium Scale)", Qt::AlignRight);
        m_chartWidget->createYAxis(2, "Line 3 (Smallest Scale)", Qt::AlignLeft);

        // Set ranges for each axis
        m_chartWidget->setAxisRange(0, -1.2, 1.2);
        m_chartWidget->setAxisRange(1, -150, 150);
        m_chartWidget->setAxisRange(2, -1000000, 1000000);

        // Add three series with different Y value ranges
        m_chartWidget->addSeries("Line 1 (small)", Qt::blue);
        m_chartWidget->addSeries("Line 2 (medium)", Qt::red);
        m_chartWidget->addSeries("Line 3 (large)", Qt::green);

        // Assign each series to its corresponding axis
        m_chartWidget->assignSeriesToAxis("Line 1 (small)", 0);
        m_chartWidget->assignSeriesToAxis("Line 2 (medium)", 1);
        m_chartWidget->assignSeriesToAxis("Line 3 (large)", 2);

        // Set max points
        m_chartWidget->setMaxPointsPerSeries(500);

        mainLayout->addWidget(m_chartWidget);

        // Control buttons and status
        QVBoxLayout *controlLayout = new QVBoxLayout();

        QLabel *statusLabel = new QLabel("Status: Ready", this);
        m_statusLabel = statusLabel;
        controlLayout->addWidget(statusLabel);

        QVBoxLayout *buttonLayout = new QVBoxLayout();

        QPushButton *generateBtn = new QPushButton("Generate Test Data", this);
        QPushButton *saveBtn = new QPushButton("Save Graph to Image", this);
        QPushButton *clearBtn = new QPushButton("Clear", this);
        QPushButton *exitBtn = new QPushButton("Exit", this);

        buttonLayout->addWidget(generateBtn);
        buttonLayout->addWidget(saveBtn);
        buttonLayout->addWidget(clearBtn);
        buttonLayout->addWidget(exitBtn);

        controlLayout->addLayout(buttonLayout);
        mainLayout->addLayout(controlLayout);

        setCentralWidget(centralWidget);

        // Connect buttons
        connect(generateBtn, &QPushButton::clicked, this, &TestSaveGraphWindow::generateTestData);
        connect(saveBtn, &QPushButton::clicked, this, &TestSaveGraphWindow::saveGraphToImage);
        connect(clearBtn, &QPushButton::clicked, this, &TestSaveGraphWindow::clearData);
        connect(exitBtn, &QPushButton::clicked, this, &QMainWindow::close);

        // Timer for data generation
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &TestSaveGraphWindow::updateData);
    }

private slots:
    void generateTestData()
    {
        m_statusLabel->setText("Status: Generating test data...");
        m_time = 0.0;
        m_pointCount = 0;
        m_chartWidget->clearAllSeries();

        // Generate 400 points
        for (int i = 0; i < 400; i++) {
            double line1Value = qSin(m_time * 0.1);
            double line2Value = 100.0 * qSin(m_time * 0.2);
            double line3Value = 1000000.0 * qSin(m_time * 0.15);

            m_chartWidget->addPoint("Line 1 (small)", m_time, line1Value);
            m_chartWidget->addPoint("Line 2 (medium)", m_time, line2Value);
            m_chartWidget->addPoint("Line 3 (large)", m_time, line3Value);

            m_time += 0.1;
            m_pointCount++;
        }

        m_statusLabel->setText(QString("Status: Generated %1 points per series").arg(m_pointCount));
    }

    void updateData()
    {
        double line1Value = qSin(m_time * 0.1);
        double line2Value = 100.0 * qSin(m_time * 0.2);
        double line3Value = 1000000.0 * qSin(m_time * 0.15);

        m_chartWidget->addPoint("Line 1 (small)", m_time, line1Value);
        m_chartWidget->addPoint("Line 2 (medium)", m_time, line2Value);
        m_chartWidget->addPoint("Line 3 (large)", m_time, line3Value);

        m_time += 0.1;
        m_pointCount++;
    }

    void saveGraphToImage()
    {
        m_statusLabel->setText("Status: Saving graph to image...");

        // Create a pixmap of the chart widget
        QPixmap pixmap(m_chartWidget->size());
        pixmap.fill(Qt::white);

        // Render the widget to the pixmap
        QPainter painter(&pixmap);
        m_chartWidget->render(&painter);
        painter.end();

        // Generate filename with timestamp
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
        QString filename = QString("test_graph_%1.png").arg(timestamp);
        QString fullPath = "C:/Users/ngoth/Downloads/files/" + filename;

        // Save the pixmap
        bool success = pixmap.save(fullPath, "PNG");

        if (success) {
            m_statusLabel->setText(QString("Status: Graph saved successfully to %1").arg(filename));
            qDebug() << "Graph saved to:" << fullPath;
        } else {
            m_statusLabel->setText(QString("Status: FAILED to save graph!"));
            qDebug() << "Failed to save graph to:" << fullPath;
        }
    }

    void clearData()
    {
        m_chartWidget->clearAllSeries();
        m_time = 0.0;
        m_pointCount = 0;
        m_timer->stop();
        m_statusLabel->setText("Status: Cleared");
    }

private:
    XYChartWidget *m_chartWidget;
    QLabel *m_statusLabel;
    QTimer *m_timer;
    double m_time;
    int m_pointCount;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    TestSaveGraphWindow window;
    window.show();

    return app.exec();
}

#include "test_save_graph.moc"
