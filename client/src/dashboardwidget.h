#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QGridLayout>

struct GpuDisplayWidgets {
    QLabel *nameLabel;
    QProgressBar *utilBar;
    QLabel *detailLabel;
};

class DashboardWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DashboardWidget(QWidget *parent = nullptr);

public slots:
    void updateMetrics(const QJsonObject &metrics);
    void clearMetrics();

private:
    void setupUI();
    void updateSectionPositions();
    QString extractCpuModel(const QString &cpuBrand);
    QString formatBytes(qint64 bytes);
    QString formatPercent(double percent);
    QString formatUptime(qint64 seconds);
    QString formatNetworkSpeed(double bytesPerSecond);

    QLabel *m_hostnameLabel;
    QLabel *m_platformLabel;
    QLabel *m_uptimeLabel;
    
    QLabel *m_cpuLabel;
    QProgressBar *m_cpuBar;
    QLabel *m_cpuDetailLabel;
    
    // GPU related elements
    QLabel *m_gpuSectionTitle;
    QList<GpuDisplayWidgets> m_gpuDisplayWidgets;
    int m_gpuStartRow; // Track where GPU widgets start
    
    // Section titles
    QLabel *m_cpuTitle;
    QLabel *m_memoryTitle;
    QLabel *m_diskTitle;
    QLabel *m_networkTitle;
    
    QLabel *m_memoryLabel;
    QProgressBar *m_memoryBar;
    QLabel *m_memoryDetailLabel;
    
    QLabel *m_diskLabel;
    QProgressBar *m_diskBar;
    QLabel *m_diskDetailLabel;
    
    QLabel *m_networkSentLabel;
    QLabel *m_networkRecvLabel;
    
    QLabel *m_statusLabel;
    QLabel *m_lastUpdateLabel;
    
    qint64 m_prevNetworkSent;
    qint64 m_prevNetworkRecv;
    QDateTime m_lastMetricTimestamp;
    
    QGridLayout *m_metricsLayout;
    int m_currentRow; // Track current row position
};

#endif // DASHBOARDWIDGET_H