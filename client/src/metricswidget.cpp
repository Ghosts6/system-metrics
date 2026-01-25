#include "metricswidget.h"
#include <QDateTime>
#include <QLocale>
#include <QStyle>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QPainter>

MetricsWidget::MetricsWidget(QWidget *parent)
    : QWidget(parent)
    , m_detailChartView(nullptr)
    , m_currentChart(nullptr) // Initialize m_currentChart
    , m_chartComboBox(nullptr)
    , m_cpuSeries(nullptr)
    , m_memorySeries(nullptr)
    , m_diskSeries(nullptr)
    , m_networkSentSeries(nullptr)
    , m_networkRecvSeries(nullptr)
    , m_cpuAxisY(nullptr)
    , m_memoryAxisY(nullptr)
    , m_diskAxisY(nullptr)
    , m_networkAxisY(nullptr)
    , m_networkAxisX(nullptr)
    , m_cpuAxisX(nullptr)
    , m_memoryAxisX(nullptr)
    , m_diskAxisX(nullptr)
{
    setupCharts();  // Create charts first
    setupUI();      // Then add them to UI
}

MetricsWidget::~MetricsWidget()
{
    for (QChart *chart : m_charts.values()) {
        delete chart;
    }
    m_charts.clear();
}

void MetricsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("Live System Metrics", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #14a085; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    QGridLayout *summaryLayout = new QGridLayout();
    summaryLayout->setSpacing(15);
    summaryLayout->setColumnStretch(1, 1);
    
    m_hostnameLabel = new QLabel("Hostname: -", this);
    m_hostnameLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    m_platformLabel = new QLabel("Platform: -", this);
    m_platformLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    m_timestampLabel = new QLabel("Last Update: -", this);
    m_timestampLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    
    summaryLayout->addWidget(m_hostnameLabel, 0, 0);
    summaryLayout->addWidget(m_platformLabel, 0, 1);
    summaryLayout->addWidget(m_timestampLabel, 0, 2);

    // CPU Section
    m_cpuLabel = new QLabel("CPU: 0%", this);
    m_cpuLabel->setStyleSheet("color: #e0e0e0; font-size: 13px; font-weight: bold;");
    m_cpuBar = new QProgressBar(this);
    m_cpuBar->setRange(0, 100);
    m_cpuBar->setValue(0);
    m_cpuBar->setTextVisible(true);
    m_cpuBar->setFormat("%p%");
    m_cpuBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; height: 28px; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );

    summaryLayout->addWidget(m_cpuLabel, 1, 0);
    summaryLayout->addWidget(m_cpuBar, 1, 1, 1, 2); // Span 2 columns

    // Memory Section
    m_memoryLabel = new QLabel("Memory: 0 / 0", this);
    m_memoryLabel->setStyleSheet("color: #e0e0e0; font-size: 13px; font-weight: bold;");
    m_memoryBar = new QProgressBar(this);
    m_memoryBar->setRange(0, 100);
    m_memoryBar->setValue(0);
    m_memoryBar->setTextVisible(true);
    m_memoryBar->setFormat("%p%");
    m_memoryBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; height: 28px; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );

    summaryLayout->addWidget(m_memoryLabel, 2, 0);
    summaryLayout->addWidget(m_memoryBar, 2, 1, 1, 2); // Span 2 columns

    // Disk Section
    m_diskLabel = new QLabel("Disk: 0 / 0", this);
    m_diskLabel->setStyleSheet("color: #e0e0e0; font-size: 13px; font-weight: bold;");
    m_diskBar = new QProgressBar(this);
    m_diskBar->setRange(0, 100);
    m_diskBar->setValue(0);
    m_diskBar->setTextVisible(true);
    m_diskBar->setFormat("%p%");
    m_diskBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; height: 28px; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );

    summaryLayout->addWidget(m_diskLabel, 3, 0);
    summaryLayout->addWidget(m_diskBar, 3, 1, 1, 2); // Span 2 columns

    // Network Section
    m_networkLabel = new QLabel("Network: Sent 0, Received 0", this);
    m_networkLabel->setStyleSheet("color: #e0e0e0; font-size: 13px; font-weight: bold;");
    summaryLayout->addWidget(m_networkLabel, 4, 0, 1, 3); // Span 3 columns

    mainLayout->addLayout(summaryLayout);

    // Separator line
    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #3d3d3d;");
    mainLayout->addWidget(line);

    // Chart selection and detailed view
    QHBoxLayout *chartSelectionLayout = new QHBoxLayout();
    QLabel *chartSelectLabel = new QLabel("Select Chart:", this);
    chartSelectLabel->setStyleSheet("color: #e0e0e0; font-size: 13px; font-weight: bold;");
    
    m_chartComboBox = new QComboBox(this);
    m_chartComboBox->addItem("CPU Usage");
    m_chartComboBox->addItem("Memory Usage");
    m_chartComboBox->addItem("Disk Usage");
    m_chartComboBox->addItem("Network Activity");
    m_chartComboBox->setStyleSheet(
        "QComboBox { border: 1px solid #3d3d3d; border-radius: 4px; padding: 5px; background-color: #2d2d2d; color: #e0e0e0; }"
        "QComboBox::drop-down { border: 0px; }"
        "QComboBox::down-arrow { image: url(:/icons/down-arrow.png); width: 12px; height: 12px; }"
        "QComboBox QAbstractItemView { border: 1px solid #3d3d3d; background-color: #2d2d2d; selection-background-color: #0d7377; color: #e0e0e0; }"
    );
    m_chartComboBox->setCurrentIndex(0); // Default to CPU

    chartSelectionLayout->addWidget(chartSelectLabel);
    chartSelectionLayout->addWidget(m_chartComboBox);
    chartSelectionLayout->addStretch();
    mainLayout->addLayout(chartSelectionLayout);

    m_detailChartView = new QChartView(this);
    m_detailChartView->setRenderHint(QPainter::Antialiasing);
    m_detailChartView->setStyleSheet("border: 1px solid #3d3d3d; border-radius: 5px; margin: 5px;");
    m_detailChartView->setMinimumHeight(300); // Make it larger

    mainLayout->addWidget(m_detailChartView);
    QObject::connect(m_chartComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                     this, &MetricsWidget::onChartSelected);

    // Set initial chart
    if (!m_charts.isEmpty() && m_detailChartView) {
        onChartSelected(0);
    }

    mainLayout->addStretch();
}

void MetricsWidget::onChartSelected(int index)
{
    if (!m_chartComboBox || !m_detailChartView || m_charts.isEmpty()) {
        return;
    }
    
    QString chartName = m_chartComboBox->itemText(index);
    if (m_charts.contains(chartName)) {
        m_currentChart = m_charts.value(chartName);
        if (m_currentChart) {
            m_detailChartView->setChart(m_currentChart);
        }
    }
}

void MetricsWidget::setupCharts()
{
    // Common setup for all detailed charts
    auto setupCommonChart = [&](QChart *chart, const QString &title, QLineSeries *series, QValueAxis *axisY, QValueAxis *axisX) {
        chart->setTheme(QChart::ChartThemeDark);
        chart->setBackgroundBrush(QBrush(QColor(26, 26, 26)));
        chart->setTitle(title);
        chart->setTitleBrush(QBrush(QColor(224, 224, 224)));
        chart->legend()->setVisible(false);
        chart->addSeries(series);

        // Y-axis
        axisY->setLabelsColor(QColor(224, 224, 224));
        axisY->setTitleBrush(QBrush(QColor(224, 224, 224)));
        axisY->setGridLineColor(QColor(61, 61, 61));
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);

        // X-axis (time based for detailed charts)
        axisX->setRange(0, MAX_DATA_POINTS -1); // Adjusted for 0-indexed appending
        axisX->setLabelsColor(QColor(224, 224, 224));
        axisX->setTitleBrush(QBrush(QColor(224, 224, 224)));
        axisX->setGridLineColor(QColor(61, 61, 61));
        axisX->setTickCount(MAX_DATA_POINTS / 5); // Show fewer ticks on X-axis
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);
    };

    // CPU Chart
    QChart *cpuChart = new QChart();
    m_cpuSeries = new QLineSeries();
    m_cpuSeries->setColor(QColor(20, 160, 133)); // #14a085
    m_cpuSeries->setPen(QPen(QColor(20, 160, 133), 2));
    m_cpuAxisY = new QValueAxis();
    m_cpuAxisY->setRange(0, 100);
    m_cpuAxisY->setTitleText("%");
    m_cpuAxisX = new QValueAxis();
    setupCommonChart(cpuChart, "CPU Usage (%)", m_cpuSeries, m_cpuAxisY, m_cpuAxisX);
    m_charts.insert("CPU Usage", cpuChart);

    // Memory Chart
    QChart *memoryChart = new QChart();
    m_memorySeries = new QLineSeries();
    m_memorySeries->setColor(QColor(20, 160, 133));
    m_memorySeries->setPen(QPen(QColor(20, 160, 133), 2));
    m_memoryAxisY = new QValueAxis();
    m_memoryAxisY->setRange(0, 100);
    m_memoryAxisY->setTitleText("%");
    m_memoryAxisX = new QValueAxis();
    setupCommonChart(memoryChart, "Memory Usage (%)", m_memorySeries, m_memoryAxisY, m_memoryAxisX);
    m_charts.insert("Memory Usage", memoryChart);

    // Disk Chart
    QChart *diskChart = new QChart();
    m_diskSeries = new QLineSeries();
    m_diskSeries->setColor(QColor(20, 160, 133));
    m_diskSeries->setPen(QPen(QColor(20, 160, 133), 2));
    m_diskAxisY = new QValueAxis();
    m_diskAxisY->setRange(0, 100);
    m_diskAxisY->setTitleText("%");
    m_diskAxisX = new QValueAxis();
    setupCommonChart(diskChart, "Disk Usage (%)", m_diskSeries, m_diskAxisY, m_diskAxisX);
    m_charts.insert("Disk Usage", diskChart);

    // Network Chart
    QChart *networkChart = new QChart();
    networkChart->setTheme(QChart::ChartThemeDark);
    networkChart->setBackgroundBrush(QBrush(QColor(26, 26, 26)));
    networkChart->setTitle("Network Activity");
    networkChart->setTitleBrush(QBrush(QColor(224, 224, 224)));
    networkChart->legend()->setVisible(true);
    networkChart->legend()->setAlignment(Qt::AlignBottom);
    networkChart->legend()->setLabelColor(QColor(224, 224, 224));
    
    m_networkSentSeries = new QLineSeries();
    m_networkSentSeries->setName("Sent");
    m_networkSentSeries->setColor(QColor(20, 160, 133));
    m_networkSentSeries->setPen(QPen(QColor(20, 160, 133), 2));
    networkChart->addSeries(m_networkSentSeries);
    
    m_networkRecvSeries = new QLineSeries();
    m_networkRecvSeries->setName("Received");
    m_networkRecvSeries->setColor(QColor(13, 115, 119)); // #0d7377
    m_networkRecvSeries->setPen(QPen(QColor(13, 115, 119), 2));
    networkChart->addSeries(m_networkRecvSeries);
    
    m_networkAxisY = new QValueAxis();
    m_networkAxisY->setRange(0, 100);
    m_networkAxisY->setTitleText("MB");
    m_networkAxisY->setLabelsColor(QColor(224, 224, 224));
    m_networkAxisY->setTitleBrush(QBrush(QColor(224, 224, 224)));
    m_networkAxisY->setGridLineColor(QColor(61, 61, 61));
    networkChart->addAxis(m_networkAxisY, Qt::AlignLeft);
    m_networkSentSeries->attachAxis(m_networkAxisY);
    m_networkRecvSeries->attachAxis(m_networkAxisY);
    
    m_networkAxisX = new QValueAxis();
    m_networkAxisX->setRange(0, MAX_DATA_POINTS - 1);
    m_networkAxisX->setLabelsColor(QColor(224, 224, 224));
    m_networkAxisX->setTitleBrush(QBrush(QColor(224, 224, 224)));
    m_networkAxisX->setGridLineColor(QColor(61, 61, 61));
    m_networkAxisX->setTickCount(MAX_DATA_POINTS / 5);
    networkChart->addAxis(m_networkAxisX, Qt::AlignBottom);
    m_networkSentSeries->attachAxis(m_networkAxisX);
    m_networkRecvSeries->attachAxis(m_networkAxisX);
    m_charts.insert("Network Activity", networkChart);
}

void MetricsWidget::updateChart(QLineSeries *series, QValueAxis *axisY, QValueAxis *axisX, QQueue<double> &history, double value, double maxValue)
{
    // Add new value to history
    history.enqueue(value);
    
    // Remove old values if exceeding max points
    while (history.size() > MAX_DATA_POINTS) {
        history.dequeue();
    }
    
    // Update series
    series->clear();
    for (int i = 0; i < history.size(); ++i) {
        series->append(static_cast<double>(i), history.at(i)); // Use i as x-coordinate
    }
    
    // Update Y axis range with some padding
    double minVal = 0;
    double maxVal = maxValue;
    if (!history.isEmpty()) {
        double dataMax = *std::max_element(history.begin(), history.end());
        maxVal = qMax(maxValue, dataMax * 1.1);
    }
    axisY->setRange(minVal, maxVal);

    // Update X axis range (always show MAX_DATA_POINTS interval)
    axisX->setRange(0, MAX_DATA_POINTS - 1);
}

void MetricsWidget::updateNetworkChart(double sentMB, double recvMB)
{
    m_networkSentHistory.enqueue(sentMB);
    m_networkRecvHistory.enqueue(recvMB);
    
    while (m_networkSentHistory.size() > MAX_DATA_POINTS) {
        m_networkSentHistory.dequeue();
    }
    while (m_networkRecvHistory.size() > MAX_DATA_POINTS) {
        m_networkRecvHistory.dequeue();
    }
    
    m_networkSentSeries->clear();
    m_networkRecvSeries->clear();
    
    for (int i = 0; i < m_networkSentHistory.size(); ++i) {
        m_networkSentSeries->append(i, m_networkSentHistory.at(i));
        m_networkRecvSeries->append(i, m_networkRecvHistory.at(i));
    }
    
    // Update Y axis
    double maxVal = 0;
    for (int i = 0; i < m_networkSentHistory.size(); ++i) {
        maxVal = qMax(maxVal, qMax(m_networkSentHistory.at(i), m_networkRecvHistory.at(i)));
    }
    m_networkAxisY->setRange(0, qMax(10.0, maxVal * 1.1)); // Ensure a minimum range

    // Dynamically update Y-axis title based on max value
    QString yTitle = "Network Activity ";
    if (maxVal >= 1024) { // If maxVal is in GB range
        yTitle += "(GB)";
    } else if (maxVal >= 1) { // If maxVal is in MB range
        yTitle += "(MB)";
    } else { // If maxVal is in KB range or less
        yTitle += "(KB)";
    }
    m_networkAxisY->setTitleText(yTitle);
}

void MetricsWidget::updateMetrics(const QJsonObject &metrics)
{
    if (metrics.contains("hostname")) {
        m_hostnameLabel->setText(QString("Hostname: %1").arg(metrics["hostname"].toString()));
    }
    if (metrics.contains("platform")) {
        m_platformLabel->setText(QString("Platform: %1").arg(metrics["platform"].toString()));
    }
    if (metrics.contains("timestamp")) {
        QString timestamp = metrics["timestamp"].toString();
        m_timestampLabel->setText(QString("Last Update: %1").arg(timestamp));
    }

    if (metrics.contains("cpu_percent")) {
        double cpuPercent = metrics["cpu_percent"].toDouble();
        m_cpuLabel->setText(QString("CPU: %1% (%2 cores)")
                           .arg(cpuPercent, 0, 'f', 1)
                           .arg(metrics["cpu_count"].toInt()));
        m_cpuBar->setValue(static_cast<int>(cpuPercent));
        updateChart(m_cpuSeries, m_cpuAxisY, m_cpuAxisX, m_cpuHistory, cpuPercent, 100.0);
    }

    if (metrics.contains("memory_total")) {
        qint64 total = static_cast<qint64>(metrics["memory_total"].toDouble());
        qint64 used = static_cast<qint64>(metrics["memory_used"].toDouble());
        double percent = metrics["memory_percent"].toDouble();
        
        m_memoryLabel->setText(QString("Memory: %1 / %2")
                              .arg(formatBytes(used))
                              .arg(formatBytes(total)));
        m_memoryBar->setValue(static_cast<int>(percent));
        updateChart(m_memorySeries, m_memoryAxisY, m_memoryAxisX, m_memoryHistory, percent, 100.0);
    }

    if (metrics.contains("disk_total")) {
        qint64 total = static_cast<qint64>(metrics["disk_total"].toDouble());
        qint64 used = static_cast<qint64>(metrics["disk_used"].toDouble());
        double percent = metrics["disk_percent"].toDouble();
        
        m_diskLabel->setText(QString("Disk: %1 / %2")
                            .arg(formatBytes(used))
                            .arg(formatBytes(total)));
        m_diskBar->setValue(static_cast<int>(percent));
        updateChart(m_diskSeries, m_diskAxisY, m_diskAxisX, m_diskHistory, percent, 100.0);
    }

    if (metrics.contains("network_bytes_sent") && metrics.contains("network_bytes_recv")) {
        qint64 sent = static_cast<qint64>(metrics["network_bytes_sent"].toDouble());
        qint64 recv = static_cast<qint64>(metrics["network_bytes_recv"].toDouble());
        
        double sentMB = sent / (1024.0 * 1024.0);
        double recvMB = recv / (1024.0 * 1024.0);
        
        m_networkLabel->setText(QString("Network: Sent %1, Received %2")
                               .arg(formatBytes(sent))
                               .arg(formatBytes(recv)));
        updateNetworkChart(sentMB, recvMB);
    }
}

QString MetricsWidget::formatBytes(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    const qint64 TB = GB * 1024;

    if (bytes >= TB) {
        return QString("%1 TB").arg(bytes / static_cast<double>(TB), 0, 'f', 2);
    } else if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / static_cast<double>(GB), 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / static_cast<double>(MB), 0, 'f', 2);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / static_cast<double>(KB), 0, 'f', 2);
    } else {
        return QString("%1 B").arg(bytes);
    }
}
