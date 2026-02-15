#ifndef METRICSWIDGET_H
#define METRICSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QJsonObject>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QQueue>
#include <QComboBox>
#include <QMap>

QT_BEGIN_NAMESPACE
class QChart;
QT_END_NAMESPACE

class MetricsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetricsWidget(QWidget *parent = nullptr);
    ~MetricsWidget();

public slots:
    void updateMetrics(const QJsonObject &metrics);
    void clearMetrics();

private:
    QLabel *m_cpuLabel;
    QProgressBar *m_cpuBar;
    QLabel *m_gpuLabel;
    QProgressBar *m_gpuBar;
    QLabel *m_memoryLabel;
    QProgressBar *m_memoryBar;
    QLabel *m_diskLabel;
    QProgressBar *m_diskBar;
    QLabel *m_networkLabel;
    QLabel *m_hostnameLabel;
    QLabel *m_platformLabel;
    QLabel *m_timestampLabel;

    QChartView *m_detailChartView; // To display the currently selected detailed chart
    QMap<QString, QChart*> m_charts; // Map to hold different detailed charts
    QChart *m_currentChart; // Pointer to the currently displayed chart
    QComboBox *m_chartComboBox; // Chart selection combo box

    // Keep series and axes for now, will be updated in next steps
    QLineSeries *m_cpuSeries;
    QLineSeries *m_gpuSeries;
    QLineSeries *m_memorySeries;
    QLineSeries *m_diskSeries;
    QLineSeries *m_networkSentSeries;
    QLineSeries *m_networkRecvSeries;
    
    QValueAxis *m_cpuAxisY;
    QValueAxis *m_gpuAxisY;
    QValueAxis *m_memoryAxisY;
    QValueAxis *m_diskAxisY;
    QValueAxis *m_networkAxisY;
    QValueAxis *m_networkAxisX;

    QValueAxis *m_cpuAxisX;
    QValueAxis *m_gpuAxisX;
    QValueAxis *m_memoryAxisX;
    QValueAxis *m_diskAxisX; // Still needed for network chart's second axis

    // Data history (keep last 60 points)
    static const int MAX_DATA_POINTS = 60;
    QQueue<double> m_cpuHistory;
    QQueue<double> m_gpuHistory;
    QQueue<double> m_memoryHistory;
    QQueue<double> m_diskHistory;
    QQueue<double> m_networkSentHistory;
    QQueue<double> m_networkRecvHistory;

    void setupUI();
    void setupCharts();
    void updateChart(QLineSeries *series, QValueAxis *axisY, QValueAxis *axisX, QQueue<double> &history, double value, double maxValue = 100.0);
    void updateNetworkChart(double sentMB, double recvMB);
    QString formatBytes(qint64 bytes);

private slots:
    void onChartSelected(int index);
};

#endif
