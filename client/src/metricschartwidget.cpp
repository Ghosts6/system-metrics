#include "metricschartwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <QPainter>
#include <QBrush>
#include <QColor>
#include <limits>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QAbstractSeries>
#include "animatedbutton.h"

MetricsChartWidget::MetricsChartWidget(QWidget *parent)
    : QWidget(parent)
    , m_chart(nullptr)
    , m_chartView(nullptr)
    , m_currentMetricType(CPU_PERCENT)
    , m_axisX(nullptr)
    , m_axisY(nullptr)
    , m_refreshTimer(new QTimer(this))
    , m_startButton(nullptr)
    , m_stopButton(nullptr)
    , m_isLiveUpdating(true)
{
    connect(m_refreshTimer, &QTimer::timeout, this, &MetricsChartWidget::onAutoRefresh);
    m_refreshTimer->start(5000); // 5 seconds interval
    
    setupUI();
    setupChart();
}

MetricsChartWidget::~MetricsChartWidget()
{
}

void MetricsChartWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("Historical Metrics", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #14a085; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    // Controls layout
    QHBoxLayout *controlsLayout = new QHBoxLayout();
    controlsLayout->setSpacing(10);

    QLabel *metricLabel = new QLabel("Metric:", this);
    metricLabel->setStyleSheet("color: #e0e0e0; font-size: 12px;");
    m_metricTypeCombo = new QComboBox(this);
    m_metricTypeCombo->addItem("CPU Usage %", CPU_PERCENT);
    m_metricTypeCombo->addItem("Memory Usage %", MEMORY_PERCENT);
    m_metricTypeCombo->addItem("Disk Usage %", DISK_PERCENT);
    m_metricTypeCombo->addItem("Network Sent", NETWORK_SENT);
    m_metricTypeCombo->addItem("Network Received", NETWORK_RECV);
    m_metricTypeCombo->setStyleSheet(
        "QComboBox { border: 1px solid #3d3d3d; border-radius: 4px; padding: 5px; background-color: #2d2d2d; color: #e0e0e0; }"
        "QComboBox QAbstractItemView { border: 1px solid #3d3d3d; background-color: #2d2d2d; selection-background-color: #0d7377; color: #e0e0e0; }"
    );
    connect(m_metricTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MetricsChartWidget::onMetricTypeChanged);

    QLabel *rangeLabel = new QLabel("Time Range:", this);
    rangeLabel->setStyleSheet("color: #e0e0e0; font-size: 12px;");
    m_timeRangeCombo = new QComboBox(this);
    m_timeRangeCombo->addItem("Last Hour", 0);
    m_timeRangeCombo->addItem("Last 6 Hours", 1);
    m_timeRangeCombo->addItem("Last 24 Hours", 2);
    m_timeRangeCombo->addItem("Last 7 Days", 3);
    m_timeRangeCombo->setCurrentIndex(2); // Default to "Last 24 Hours"
    m_timeRangeCombo->setStyleSheet(
        "QComboBox { border: 1px solid #3d3d3d; border-radius: 4px; padding: 5px; background-color: #2d2d2d; color: #e0e0e0; }"
        "QComboBox QAbstractItemView { border: 1px solid #3d3d3d; background-color: #2d2d2d; selection-background-color: #0d7377; color: #e0e0e0; }"
    );
    connect(m_timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MetricsChartWidget::onTimeRangeChanged);

    m_stopButton = new AnimatedButton("Stop Live Update", this);
    connect(m_stopButton, &QPushButton::clicked, this, &MetricsChartWidget::onStopClicked);

    m_startButton = new AnimatedButton("Start Live Update", this);
    connect(m_startButton, &QPushButton::clicked, this, &MetricsChartWidget::onStartClicked);
    m_startButton->hide(); // Hidden by default as live update is on

    controlsLayout->addWidget(metricLabel);
    controlsLayout->addWidget(m_metricTypeCombo);
    controlsLayout->addWidget(rangeLabel);
    controlsLayout->addWidget(m_timeRangeCombo);
    controlsLayout->addWidget(m_stopButton);
    controlsLayout->addWidget(m_startButton);
    controlsLayout->addStretch();

    mainLayout->addLayout(controlsLayout);

    // Chart view
    m_chartView = new QChartView(this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView);

    m_statusLabel = new QLabel("No data loaded", this);
    m_statusLabel->setStyleSheet("color: #b0b0b0; font-size: 11px; padding: 5px;");
    mainLayout->addWidget(m_statusLabel);

    // Initial load with default time range, triggered by initial live update or explicit start
    if (m_isLiveUpdating) {
        onRefreshClicked();
    }
}

void MetricsChartWidget::setupChart()
{
    m_chart = new QChart();
    m_chart->setTheme(QChart::ChartThemeDark);
    m_chart->setBackgroundBrush(QBrush(QColor(26, 26, 26)));
    m_chart->setTitleBrush(QBrush(QColor(224, 224, 224)));
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);
    m_chart->legend()->setLabelColor(QColor(224, 224, 224));

    // Setup axes
    m_axisX = new QDateTimeAxis();
    m_axisX->setFormat("HH:mm");
    m_axisX->setTitleText("Time");
    m_axisX->setLabelsColor(QColor(224, 224, 224));
    m_axisX->setTitleBrush(QBrush(QColor(224, 224, 224)));
    m_axisX->setGridLineColor(QColor(60, 60, 60)); // Softer grid lines
    m_axisX->setGridLinePen(QPen(QColor(60, 60, 60), 0.5, Qt::SolidLine)); // Thinner, solid grid lines
    m_chart->addAxis(m_axisX, Qt::AlignBottom);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Value");
    m_axisY->setLabelsColor(QColor(224, 224, 224));
    m_axisY->setTitleBrush(QBrush(QColor(20, 160, 133))); // #14a085 - same as main title
    QFont yAxisTitleFont = m_axisY->titleFont();
    yAxisTitleFont.setPointSize(10); // Slightly smaller than main widget title
    m_axisY->setTitleFont(yAxisTitleFont);
    m_axisY->setGridLineColor(QColor(60, 60, 60)); // Softer grid lines
    m_axisY->setGridLinePen(QPen(QColor(60, 60, 60), 0.5, Qt::SolidLine)); // Thinner, solid grid lines
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_chartView->setChart(m_chart);
}

void MetricsChartWidget::onTimeRangeChanged()
{
    if (m_isLiveUpdating) {
        onRefreshClicked();
    }
}

QDateTime MetricsChartWidget::getStartTimeForRange(int rangeIndex) const
{
    QDateTime now = QDateTime::currentDateTime();
    switch (rangeIndex) {
        case 0: return now.addSecs(-3600);      // Last Hour
        case 1: return now.addSecs(-21600);     // Last 6 Hours
        case 2: return now.addDays(-1);         // Last 24 Hours
        case 3: return now.addDays(-7);         // Last 7 Days
        default: return now.addSecs(-3600);
    }
}

void MetricsChartWidget::onMetricTypeChanged(int index)
{
    m_currentMetricType = static_cast<MetricType>(m_metricTypeCombo->itemData(index).toInt());
    if (m_isLiveUpdating) {
        onRefreshClicked();
    }
}

void MetricsChartWidget::onRefreshClicked()
{
    int rangeIndex = m_timeRangeCombo->currentIndex();
    QDateTime endTime = QDateTime::currentDateTime();
    QDateTime startTime = getStartTimeForRange(rangeIndex);
    m_currentEndTime = endTime; // Store the current end time
    emit requestHistory(startTime, endTime);
}

void MetricsChartWidget::refreshChart()
{
    onRefreshClicked();
}

void MetricsChartWidget::onStartClicked()
{
    m_refreshTimer->start(5000); // Resume with 5 seconds interval
    m_isLiveUpdating = true;
    m_startButton->hide();
    m_stopButton->show();
    onRefreshClicked(); // Immediately refresh
}

void MetricsChartWidget::onStopClicked()
{
    m_refreshTimer->stop();
    m_isLiveUpdating = false;
    m_startButton->show();
    m_stopButton->hide();
}

void MetricsChartWidget::onAutoRefresh()
{
    if (m_isLiveUpdating) {
        onRefreshClicked();
    }
}

void MetricsChartWidget::updateHistoryData(const QJsonObject &response)
{
    if (!response.contains("items")) {
        m_statusLabel->setText("No data in response");
        clearChart();
        return;
    }

    QJsonArray items = response["items"].toArray();
    if (items.isEmpty()) {
        int total = response.value("total").toInt(0);
        if (total == 0) {
            m_statusLabel->setText("No metrics found for selected time range. Try a different time range or ensure metrics are being collected.");
        } else {
            m_statusLabel->setText(QString("No metrics in this page (Total: %1)").arg(total));
        }
        clearChart();
        return;
    }

    updateChart(items);

    int total = response.value("total").toInt(0);
    m_statusLabel->setText(QString("Loaded %1 of %2 data points").arg(items.size()).arg(total));
}

QString MetricsChartWidget::getMetricName(int type) const
{
    switch (static_cast<MetricType>(type)) {
        case CPU_PERCENT: return "CPU Usage";
        case MEMORY_PERCENT: return "Memory Usage";
        case DISK_PERCENT: return "Disk Usage";
        case NETWORK_SENT: return "Network Sent";
        case NETWORK_RECV: return "Network Received";
        default: return "Unknown";
    }
}

void MetricsChartWidget::updateChart(const QJsonArray &metrics)
{
    // Remove existing series
    QList<QAbstractSeries*> seriesList = m_chart->series();
    for (QAbstractSeries *series : seriesList) {
        m_chart->removeSeries(series);
        delete series;
    }

    if (metrics.isEmpty()) {
        return;
    }

    const int GAP_THRESHOLD_SECONDS = 300; // 5 minutes

    QLineSeries *currentSeries = nullptr;
    QDateTime lastTimestamp;

    QDateTime minTime = QDateTime::currentDateTime();
    QDateTime maxTime = QDateTime::fromSecsSinceEpoch(0);
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::min();


    for (const QJsonValue &value : metrics) {
        QJsonObject metric = value.toObject();
        
        QString timestampStr = metric["timestamp"].toString();
        QDateTime currentTimestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
        
        if (!currentTimestamp.isValid()) {
            continue;
        }

        // Check for a gap
        if (currentSeries && lastTimestamp.isValid() &&
            currentTimestamp.toSecsSinceEpoch() - lastTimestamp.toSecsSinceEpoch() > GAP_THRESHOLD_SECONDS)
        {
            // Gap detected, finalize current series and start a new one
            if (currentSeries->count() > 0) {
                m_chart->addSeries(currentSeries);
                currentSeries->attachAxis(m_axisX);
                currentSeries->attachAxis(m_axisY);
            } else {
                delete currentSeries; // Delete empty series if only one point before gap
            }
            currentSeries = nullptr; // Reset for new series
        }

        if (!currentSeries) {
            currentSeries = new QLineSeries();
            currentSeries->setName(getMetricName(m_currentMetricType));
            currentSeries->setColor(QColor(20, 160, 133)); // #14a085
            QPen seriesPen = currentSeries->pen();
            seriesPen.setWidth(2); // Make the line thicker
            currentSeries->setPen(seriesPen);
        }

        double dataValue = 0.0;
        switch (m_currentMetricType) {
            case CPU_PERCENT:
                dataValue = metric["cpu_percent"].toDouble();
                break;
            case MEMORY_PERCENT:
                dataValue = metric["memory_percent"].toDouble();
                break;
            case DISK_PERCENT:
                dataValue = metric["disk_percent"].toDouble();
                break;
            case NETWORK_SENT:
                dataValue = metric["network_bytes_sent"].toDouble() / (1024.0 * 1024.0); // Convert to MB
                break;
            case NETWORK_RECV:
                dataValue = metric["network_bytes_recv"].toDouble() / (1024.0 * 1024.0); // Convert to MB
                break;
        }

        currentSeries->append(currentTimestamp.toMSecsSinceEpoch(), dataValue);

        if (currentTimestamp < minTime) minTime = currentTimestamp;
        if (currentTimestamp > maxTime) maxTime = currentTimestamp; // Keep track of overall max time from data for y-axis scaling
        if (dataValue < minValue) minValue = dataValue;
        if (dataValue > maxValue) maxValue = dataValue;
        
        lastTimestamp = currentTimestamp;
    }

    // Add the last series if it exists and has points
    if (currentSeries && currentSeries->count() > 0) {
        m_chart->addSeries(currentSeries);
        currentSeries->attachAxis(m_axisX);
        currentSeries->attachAxis(m_axisY);
    } else if (currentSeries) {
        delete currentSeries; // Delete if the last series was empty
    }


    // Update axes
    // Use m_currentEndTime for the max range to ensure the axis covers the entire requested period
    m_axisX->setRange(minTime, m_currentEndTime);

    
    // Add some padding to Y axis
    double padding = (maxValue - minValue) * 0.1;
    if (padding == 0) padding = maxValue * 0.1;
    m_axisY->setRange(qMax(0.0, minValue - padding), maxValue + padding);
    
    // Update Y axis title based on metric type
    QString yTitle = getMetricName(m_currentMetricType);
    if (m_currentMetricType == NETWORK_SENT || m_currentMetricType == NETWORK_RECV) {
        yTitle += " (MB)";
    } else {
        yTitle += " (%)";
    }
    m_axisY->setTitleText(yTitle);

    // Update chart title
    QString timeRangeText = m_timeRangeCombo->currentText();
    m_chart->setTitle(getMetricName(m_currentMetricType) + " Over Time (" + timeRangeText + ")");
}

void MetricsChartWidget::clearChart()
{
    QList<QAbstractSeries*> seriesList = m_chart->series();
    for (QAbstractSeries *series : seriesList) {
        m_chart->removeSeries(series);
        delete series;
    }
}
