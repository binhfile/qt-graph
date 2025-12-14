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
        setWindowTitle("Test: Auto-Save Graph to Image File");
        resize(1200, 800);

        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // Create chart widget
        m_chartWidget = new XYChartWidget(this);
        m_chartWidget->setTitle("Test Graph - Multi-Axis Sine Waves");
        m_chartWidget->setXLabel("Time (s)");
        m_chartWidget->setYLabel("Value");
        m_chartWidget->setDarkModeEnabled(false);

        // Enable multi-axis mode for auto-scaling each series independently
        m_chartWidget->setMultiAxisEnabled(true);

        // Create four Y-axes with different scales
        m_chartWidget->createYAxis(0, "Line 1 (small)", Qt::AlignLeft);
        m_chartWidget->createYAxis(1, "Line 2 (medium)", Qt::AlignRight);
        m_chartWidget->createYAxis(2, "Line 3 (large)", Qt::AlignLeft);
        m_chartWidget->createYAxis(3, "Line 4 (huge)", Qt::AlignRight);

        // Enable auto-scale for each axis to see all data properly
        m_chartWidget->setAxisAutoScale(0, true);
        m_chartWidget->setAxisAutoScale(1, true);
        m_chartWidget->setAxisAutoScale(2, true);
        m_chartWidget->setAxisAutoScale(3, true);

        // Add four series with different Y value ranges
        m_chartWidget->addSeries("Line 1 (small)", Qt::blue);
        m_chartWidget->addSeries("Line 2 (medium)", Qt::red);
        m_chartWidget->addSeries("Line 3 (large)", Qt::green);
        m_chartWidget->addSeries("Line 4 (huge)", Qt::magenta);

        // Assign each series to its corresponding axis
        m_chartWidget->assignSeriesToAxis("Line 1 (small)", 0);
        m_chartWidget->assignSeriesToAxis("Line 2 (medium)", 1);
        m_chartWidget->assignSeriesToAxis("Line 3 (large)", 2);
        m_chartWidget->assignSeriesToAxis("Line 4 (huge)", 3);

        // Set max points
        m_chartWidget->setMaxPointsPerSeries(500);

        mainLayout->addWidget(m_chartWidget);

        // Status label
        QVBoxLayout *controlLayout = new QVBoxLayout();

        QLabel *statusLabel = new QLabel("Status: Initializing auto-generation...", this);
        m_statusLabel = statusLabel;
        controlLayout->addWidget(statusLabel);

        mainLayout->addLayout(controlLayout);

        setCentralWidget(centralWidget);

        // Timer for data generation
        m_timer = new QTimer(this);
        connect(m_timer, &QTimer::timeout, this, &TestSaveGraphWindow::updateData);

        // Auto-start the generation process after a short delay to allow UI to render
        QTimer::singleShot(500, this, &TestSaveGraphWindow::autoGenerateAndSave);
    }

private slots:
    void autoGenerateAndSave()
    {
        m_statusLabel->setText("Status: Auto-generating test data...");
        generateTestData();

        // Save after generation completes
        QTimer::singleShot(1000, this, &TestSaveGraphWindow::saveGraphToImage);
    }

    void generateTestData()
    {
        m_statusLabel->setText("Status: Generating test data...");
        m_time = 0.0;
        m_pointCount = 0;
        m_chartWidget->clearAllSeries();

        // Generate 400 points
        for (int i = 0; i < 400; i++) {
            // Line 1: Small sine wave (-1 to +1)
            double line1Value = qSin(m_time * 0.1);

            // Line 2: Medium sine wave (-100 to +100)
            double line2Value = 100.0 * qSin(m_time * 0.2);

            // Line 3: Large sine wave (-1M to +1M)
            double line3Value = 1000000.0 * qSin(m_time * 0.15);

            // Line 4: Huge cosine wave (-1B to +1B) - uses cosine instead of sine
            // Cosine frequency is different: 0.08, making it phase-shifted from others
            double line4Value = 1000000000.0 * qCos(m_time * 0.08 + 1.0);

            m_chartWidget->addPoint("Line 1 (small)", m_time, line1Value);
            m_chartWidget->addPoint("Line 2 (medium)", m_time, line2Value);
            m_chartWidget->addPoint("Line 3 (large)", m_time, line3Value);
            m_chartWidget->addPoint("Line 4 (huge)", m_time, line4Value);

            m_time += 0.1;
            m_pointCount++;
        }

        m_statusLabel->setText(QString("Status: Generated %1 points per series").arg(m_pointCount));
    }

    void updateData()
    {
        // Line 1: Small sine wave (-1 to +1)
        double line1Value = qSin(m_time * 0.1);

        // Line 2: Medium sine wave (-100 to +100)
        double line2Value = 100.0 * qSin(m_time * 0.2);

        // Line 3: Large sine wave (-1M to +1M)
        double line3Value = 1000000.0 * qSin(m_time * 0.15);

        // Line 4: Huge cosine wave (-1B to +1B)
        double line4Value = 1000000000.0 * qCos(m_time * 0.08 + 1.0);

        m_chartWidget->addPoint("Line 1 (small)", m_time, line1Value);
        m_chartWidget->addPoint("Line 2 (medium)", m_time, line2Value);
        m_chartWidget->addPoint("Line 3 (large)", m_time, line3Value);
        m_chartWidget->addPoint("Line 4 (huge)", m_time, line4Value);

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

        // Auto-exit after saving
        QTimer::singleShot(1000, qApp, &QApplication::quit);
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
