#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QVBoxLayout>

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

private:
    void setupUI();
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

    // GPU related elements - now directly managed lists of widgets
    QLabel *m_gpuSectionTitle; // To manage visibility
    QGridLayout *m_gpuLayout; // Layout for GPU widgets
    QList<GpuDisplayWidgets> m_gpuDisplayWidgets;


    QLabel *m_memoryLabel;
    QProgressBar *m_memoryBar;
    QLabel *m_memoryDetailLabel;

    QLabel *m_diskLabel;
    QProgressBar *m_diskBar;
    QLabel *m_diskDetailLabel;

    QLabel *m_networkSentLabel;
    QLabel *m_networkRecvLabel;
    QLabel *m_networkSpeedLabel;

    QLabel *m_statusLabel;
    QLabel *m_lastUpdateLabel;

    qint64 m_prevNetworkSent;
    qint64 m_prevNetworkRecv;
    QDateTime m_lastMetricTimestamp;
};

#endif // DASHBOARDWIDGET_H
