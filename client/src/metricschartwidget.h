#ifndef METRICSCHARTWIDGET_H
#define METRICSCHARTWIDGET_H

#include <QWidget>
#include <QDateTime>
#include <QJsonArray>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QComboBox>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QLabel>
#include <QTimer> // Added for QTimer
#include "animatedbutton.h" // Added for AnimatedButton

#include <QTimer> // Added for QTimer
#include "animatedbutton.h" // Added for AnimatedButton

QT_BEGIN_NAMESPACE
class QChart;
class QChartView;
QT_END_NAMESPACE

/**
 * @brief Widget for displaying historical metrics as time-series charts
 * 
 * This widget uses Qt Charts to visualize historical system metrics
 * including CPU, Memory, and Disk usage over time.
 */
class MetricsChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetricsChartWidget(QWidget *parent = nullptr);
    ~MetricsChartWidget();

public slots:
    void updateHistoryData(const QJsonObject &response);
    void clearChart();

signals:
    void requestHistory(const QDateTime &startTime, const QDateTime &endTime);

public slots:
    void refreshChart();

private slots:
    void onTimeRangeChanged();
    void onMetricTypeChanged(int index);
    void onRefreshClicked();

private:
    void setupUI();
    void setupChart();
    void updateChart(const QJsonArray &metrics);
    QDateTime getStartTimeForRange(int rangeIndex) const;
    QString getMetricName(int type) const;
    
    enum MetricType {
        CPU_PERCENT,
        MEMORY_PERCENT,
        DISK_PERCENT,
        NETWORK_SENT,
        NETWORK_RECV
    };
    
    QChart *m_chart;
    QChartView *m_chartView;
    QComboBox *m_metricTypeCombo;
    QComboBox *m_timeRangeCombo;
    QPushButton *m_refreshButton;
    QLabel *m_statusLabel;
    
    QTimer *m_refreshTimer;
    AnimatedButton *m_startButton;
    AnimatedButton *m_stopButton;
    bool m_isLiveUpdating;

    MetricType m_currentMetricType;
    QDateTimeAxis *m_axisX;
    QValueAxis *m_axisY;
    QDateTime m_currentEndTime;

private slots:
    void onStartClicked();
    void onStopClicked();
    void onAutoRefresh();
};

#endif // METRICSCHARTWIDGET_H
