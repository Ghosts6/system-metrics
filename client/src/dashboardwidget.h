#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QProgressBar>
#include <QJsonObject>

/**
 * @brief Dashboard widget providing an overview of system metrics
 * 
 * Displays key metrics in a compact, easy-to-read format with visual indicators.
 */
class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);

public slots:
    void updateMetrics(const QJsonObject &metrics);

private:
    void setupUI();
    QString formatBytes(qint64 bytes);
    QString formatPercent(double percent);
    QString formatUptime(qint64 seconds); // New helper
    QString formatNetworkSpeed(double bytesPerSecond); // New helper
    
    // System info
    QLabel *m_hostnameLabel;
    QLabel *m_platformLabel;
    QLabel *m_uptimeLabel;
    
    // CPU
    QLabel *m_cpuLabel;
    QProgressBar *m_cpuBar;
    QLabel *m_cpuDetailLabel;
    
    // Memory
    QLabel *m_memoryLabel;
    QProgressBar *m_memoryBar;
    QLabel *m_memoryDetailLabel;
    
    // Disk
    QLabel *m_diskLabel;
    QProgressBar *m_diskBar;
    QLabel *m_diskDetailLabel;
    
    // Network
    QLabel *m_networkSentLabel;
    QLabel *m_networkRecvLabel;
    QLabel *m_networkSpeedLabel;

    // GPU
    QLabel *m_gpuLabel;
    QProgressBar *m_gpuBar;
    QLabel *m_gpuDetailLabel;
    
    // Variables for network speed calculation and uptime
    qint64 m_prevNetworkSent;
    qint64 m_prevNetworkRecv;
    QDateTime m_lastMetricTimestamp;
    
    // Status indicators
    QLabel *m_statusLabel;
    QLabel *m_lastUpdateLabel;
};

#endif // DASHBOARDWIDGET_H
