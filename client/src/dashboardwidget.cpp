#include "dashboardwidget.h"
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QJsonObject>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
    , m_prevNetworkSent(0)
    , m_prevNetworkRecv(0)
    , m_lastMetricTimestamp(QDateTime())
{
    setupUI();
}

void DashboardWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("System Dashboard", this);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #14a085; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    // System info section
    QHBoxLayout *systemInfoLayout = new QHBoxLayout();
    m_hostnameLabel = new QLabel("Hostname: -", this);
    m_hostnameLabel->setStyleSheet("color: #b0b0b0; font-size: 12px; padding: 5px;");
    m_platformLabel = new QLabel("Platform: -", this);
    m_platformLabel->setStyleSheet("color: #b0b0b0; font-size: 12px; padding: 5px;");
    m_uptimeLabel = new QLabel("Uptime: -", this);
    m_uptimeLabel->setStyleSheet("color: #b0b0b0; font-size: 12px; padding: 5px;");
    
    systemInfoLayout->addWidget(m_hostnameLabel);
    systemInfoLayout->addWidget(m_platformLabel);
    systemInfoLayout->addWidget(m_uptimeLabel);
    systemInfoLayout->addStretch();
    
    mainLayout->addLayout(systemInfoLayout);

    // Metrics grid
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(20);
    gridLayout->setColumnStretch(1, 2);

    // CPU section
    QLabel *cpuTitle = new QLabel("CPU Usage", this);
    cpuTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    gridLayout->addWidget(cpuTitle, 0, 0, 1, 2);
    
    m_cpuLabel = new QLabel("0%", this);
    m_cpuLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");
    gridLayout->addWidget(m_cpuLabel, 1, 0);
    
    m_cpuBar = new QProgressBar(this);
    m_cpuBar->setRange(0, 100);
    m_cpuBar->setValue(0);
    m_cpuBar->setTextVisible(true);
    m_cpuBar->setFormat("%p%");
    m_cpuBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; height: 35px; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );
    gridLayout->addWidget(m_cpuBar, 1, 1);
    
    m_cpuDetailLabel = new QLabel("Cores: -", this);
    m_cpuDetailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    gridLayout->addWidget(m_cpuDetailLabel, 2, 0, 1, 2);

    // GPU section title (will be hidden if no GPUs, initially placed but hidden)
    m_gpuSectionTitle = new QLabel("GPU Usage", this);
    m_gpuSectionTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    gridLayout->addWidget(m_gpuSectionTitle, 3, 0, 1, 2);
    m_gpuSectionTitle->hide(); // Initially hidden

    m_gpuLayout = new QGridLayout();
    m_gpuLayout->setSpacing(10);
    m_gpuLayout->setColumnStretch(0, 1);
    m_gpuLayout->setColumnStretch(1, 2);
    gridLayout->addLayout(m_gpuLayout, 4, 0, 1, 2);

    // Memory section (starts after GPU section, which is dynamically sized)
    QLabel *memoryTitle = new QLabel("Memory Usage", this);
    memoryTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    gridLayout->addWidget(memoryTitle, 5, 0, 1, 2);
    
    m_memoryLabel = new QLabel("0%", this);
    m_memoryLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");
    gridLayout->addWidget(m_memoryLabel, 6, 0);
    
    m_memoryBar = new QProgressBar(this);
    m_memoryBar->setRange(0, 100);
    m_memoryBar->setValue(0);
    m_memoryBar->setTextVisible(true);
    m_memoryBar->setFormat("%p%");
    m_memoryBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; height: 35px; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );
    gridLayout->addWidget(m_memoryBar, 6, 1);
    
    m_memoryDetailLabel = new QLabel("Used: - / Total: -", this);
    m_memoryDetailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    gridLayout->addWidget(m_memoryDetailLabel, 7, 0, 1, 2);

    // Disk section (rows adjusted)
    QLabel *diskTitle = new QLabel("Disk Usage", this);
    diskTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    gridLayout->addWidget(diskTitle, 8, 0, 1, 2);
    
    m_diskLabel = new QLabel("0%", this);
    m_diskLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");
    gridLayout->addWidget(m_diskLabel, 9, 0);
    
    m_diskBar = new QProgressBar(this);
    m_diskBar->setRange(0, 100);
    m_diskBar->setValue(0);
    m_diskBar->setTextVisible(true);
    m_diskBar->setFormat("%p%");
    m_diskBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; height: 35px; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );
    gridLayout->addWidget(m_diskBar, 9, 1);
    
    m_diskDetailLabel = new QLabel("Used: - / Total: -", this);
    m_diskDetailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    gridLayout->addWidget(m_diskDetailLabel, 10, 0, 1, 2);

    // Network section (rows adjusted)
    QLabel *networkTitle = new QLabel("Network Activity", this);
    networkTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    gridLayout->addWidget(networkTitle, 11, 0, 1, 2);
    
    m_networkSentLabel = new QLabel("Sent: 0 B", this);
    m_networkSentLabel->setStyleSheet("color: #e0e0e0; font-size: 14px; padding: 5px;");
    gridLayout->addWidget(m_networkSentLabel, 12, 0);
    
    m_networkRecvLabel = new QLabel("Received: 0 B", this);
    m_networkRecvLabel->setStyleSheet("color: #e0e0e0; font-size: 14px; padding: 5px;");
    gridLayout->addWidget(m_networkRecvLabel, 12, 1);
    
    m_networkSpeedLabel = new QLabel("Speed: -", this);
    m_networkSpeedLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    gridLayout->addWidget(m_networkSpeedLabel, 13, 0, 1, 2);

    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();

    // Status bar
    QHBoxLayout *statusLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("● Status: Disconnected", this);
    m_statusLabel->setStyleSheet("color: #ff6b6b; font-size: 12px; font-weight: bold; padding: 5px;");
    m_lastUpdateLabel = new QLabel("Last Update: Never", this);
    m_lastUpdateLabel->setStyleSheet("color: #b0b0b0; font-size: 11px; padding: 5px;");
    
    statusLayout->addWidget(m_statusLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(m_lastUpdateLabel);
    
    mainLayout->addLayout(statusLayout);
}

void DashboardWidget::updateMetrics(const QJsonObject &metrics)
{
    // System info
    if (metrics.contains("hostname")) {
        m_hostnameLabel->setText(QString("Hostname: %1").arg(metrics["hostname"].toString()));
    }
    if (metrics.contains("platform")) {
        m_platformLabel->setText(QString("Platform: %1").arg(metrics["platform"].toString()));
    }
    
    // CPU
    if (metrics.contains("cpu_percent")) {
        double cpuPercent = metrics["cpu_percent"].toDouble();
        m_cpuLabel->setText(formatPercent(cpuPercent));
        m_cpuBar->setValue(static_cast<int>(cpuPercent));
        
        if (metrics.contains("cpu_count")) {
            m_cpuDetailLabel->setText(QString("Cores: %1").arg(metrics["cpu_count"].toInt()));
        }
    }

    // Memory
    if (metrics.contains("memory_percent")) {
        double memPercent = metrics["memory_percent"].toDouble();
        m_memoryLabel->setText(formatPercent(memPercent));
        m_memoryBar->setValue(static_cast<int>(memPercent));
        
        if (metrics.contains("memory_total") && metrics.contains("memory_used")) {
            qint64 total = static_cast<qint64>(metrics["memory_total"].toDouble());
            qint64 used = static_cast<qint64>(metrics["memory_used"].toDouble());
            m_memoryDetailLabel->setText(
                QString("Used: %1 / Total: %2")
                .arg(formatBytes(used))
                .arg(formatBytes(total))
            );
        }
    }

    // Disk
    if (metrics.contains("disk_percent")) {
        double diskPercent = metrics["disk_percent"].toDouble();
        m_diskLabel->setText(formatPercent(diskPercent));
        m_diskBar->setValue(static_cast<int>(diskPercent));
        
        if (metrics.contains("disk_total") && metrics.contains("disk_used")) {
            qint64 total = static_cast<qint64>(metrics["disk_total"].toDouble());
            qint64 used = static_cast<qint64>(metrics["disk_used"].toDouble());
            m_diskDetailLabel->setText(
                QString("Used: %1 / Total: %2")
                .arg(formatBytes(used))
                .arg(formatBytes(total))
            );
        }
    }

    // Network
    if (metrics.contains("network_bytes_sent")) {
        qint64 sent = static_cast<qint64>(metrics["network_bytes_sent"].toDouble());
        m_networkSentLabel->setText(QString("Sent: %1").arg(formatBytes(sent)));
    }
    if (metrics.contains("network_bytes_recv")) {
        qint64 recv = static_cast<qint64>(metrics["network_bytes_recv"].toDouble());
        m_networkRecvLabel->setText(QString("Received: %1").arg(formatBytes(recv)));
    }
    
    // Update status
    m_statusLabel->setText("● Status: Connected");
    m_statusLabel->setStyleSheet("color: #14a085; font-size: 12px; font-weight: bold; padding: 5px;");
    
    if (metrics.contains("timestamp")) {
        QString timestamp = metrics["timestamp"].toString();
        m_lastUpdateLabel->setText(QString("Last Update: %1").arg(timestamp));
    } else {
        m_lastUpdateLabel->setText(QString("Last Update: %1").arg(QDateTime::currentDateTime().toString("HH:mm:ss")));
    }

    // Uptime
    if (metrics.contains("uptime_seconds")) {
        qint64 uptimeSeconds = static_cast<qint64>(metrics["uptime_seconds"].toDouble());
        m_uptimeLabel->setText(QString("Uptime: %1").arg(formatUptime(uptimeSeconds)));
    }

    // Network Speed Calculation
    if (metrics.contains("network_bytes_sent") && metrics.contains("network_bytes_recv") && metrics.contains("timestamp")) {
        qint64 currentSent = static_cast<qint64>(metrics["network_bytes_sent"].toDouble());
        qint64 currentRecv = static_cast<qint64>(metrics["network_bytes_recv"].toDouble());
        QDateTime currentTimestamp = QDateTime::fromString(metrics["timestamp"].toString(), Qt::ISODate);

        if (m_lastMetricTimestamp.isValid() && m_lastMetricTimestamp < currentTimestamp) {
            qint64 timeElapsedMs = m_lastMetricTimestamp.msecsTo(currentTimestamp);
            if (timeElapsedMs > 0) {
                double sentSpeed = (currentSent - m_prevNetworkSent) * 1000.0 / timeElapsedMs; // bytes/sec
                double recvSpeed = (currentRecv - m_prevNetworkRecv) * 1000.0 / timeElapsedMs; // bytes/sec
                m_networkSpeedLabel->setText(QString("Speed: S: %1/s R: %2/s")
                                              .arg(formatNetworkSpeed(sentSpeed))
                                              .arg(formatNetworkSpeed(recvSpeed)));
            } else {
                m_networkSpeedLabel->setText("Speed: 0 B/s");
            }
        } else {
            m_networkSpeedLabel->setText("Speed: --"); // Initial state or invalid timestamp
        }

        m_prevNetworkSent = currentSent;
        m_prevNetworkRecv = currentRecv;
        m_lastMetricTimestamp = currentTimestamp;
    }

    // GPU (dynamic handling of multiple GPUs)
    if (metrics.contains("gpus") && metrics["gpus"].isArray()) {
        QJsonArray gpusArray = metrics["gpus"].toArray();
        m_gpuSectionTitle->setVisible(!gpusArray.isEmpty());

        // Update existing GPU display widgets or create new ones
        for (int i = 0; i < gpusArray.size(); ++i) {
            QJsonObject gpuData = gpusArray[i].toObject();
            
            if (i < m_gpuDisplayWidgets.size()) {
                // Update existing widgets
                GpuDisplayWidgets &widgets = m_gpuDisplayWidgets[i];
                double utilization = gpuData["utilization"].toDouble();
                widgets.nameLabel->setText(formatPercent(utilization));
                widgets.utilBar->setValue(static_cast<int>(utilization));

                QString gpuName = gpuData["name"].toString();
                widgets.detailLabel->setText(QString("Name: %1").arg(gpuName));
            } else {
                // Create new set of widgets for this GPU
                GpuDisplayWidgets newWidgets;

                newWidgets.nameLabel = new QLabel("0%", this);
                newWidgets.nameLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");

                newWidgets.utilBar = new QProgressBar(this);
                newWidgets.utilBar->setRange(0, 100);
                newWidgets.utilBar->setValue(0);
                newWidgets.utilBar->setTextVisible(true);
                newWidgets.utilBar->setFormat("%p%");
                newWidgets.utilBar->setStyleSheet(
                    "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; height: 35px; }"
                    "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
                );
                newWidgets.utilBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

                newWidgets.detailLabel = new QLabel("Name: -", this);
                newWidgets.detailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");

                int newRow = i * 2;
                m_gpuLayout->addWidget(newWidgets.nameLabel, newRow, 0);
                m_gpuLayout->addWidget(newWidgets.utilBar, newRow, 1);
                m_gpuLayout->addWidget(newWidgets.detailLabel, newRow + 1, 0, 1, 2);

                m_gpuDisplayWidgets.append(newWidgets);

                // Now update the newly created widgets
                double utilization = gpuData["utilization"].toDouble();
                newWidgets.nameLabel->setText(formatPercent(utilization));
                newWidgets.utilBar->setValue(static_cast<int>(utilization));
                QString gpuName = gpuData["name"].toString();
                newWidgets.detailLabel->setText(QString("Name: %1").arg(gpuName));
            }
        }

        // Remove excess GPU display widgets if fewer GPUs are reported
        while (m_gpuDisplayWidgets.size() > gpusArray.size()) {
            GpuDisplayWidgets widgetsToRemove = m_gpuDisplayWidgets.takeLast();
            m_gpuLayout->removeWidget(widgetsToRemove.nameLabel);
            m_gpuLayout->removeWidget(widgetsToRemove.utilBar);
            m_gpuLayout->removeWidget(widgetsToRemove.detailLabel);
            delete widgetsToRemove.nameLabel;
            delete widgetsToRemove.utilBar;
            delete widgetsToRemove.detailLabel;
        }
    } else {
        // No GPUs reported, remove all existing widgets
        m_gpuSectionTitle->hide();
        while (m_gpuDisplayWidgets.size() > 0) {
            GpuDisplayWidgets widgetsToRemove = m_gpuDisplayWidgets.takeLast();
            m_gpuLayout->removeWidget(widgetsToRemove.nameLabel);
            m_gpuLayout->removeWidget(widgetsToRemove.utilBar);
            m_gpuLayout->removeWidget(widgetsToRemove.detailLabel);
            delete widgetsToRemove.nameLabel;
            delete widgetsToRemove.utilBar;
            delete widgetsToRemove.detailLabel;
        }
    }
}

QString DashboardWidget::formatBytes(qint64 bytes)
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

QString DashboardWidget::formatPercent(double percent)
{
    return QString("%1%").arg(percent, 0, 'f', 1);
}

QString DashboardWidget::formatUptime(qint64 seconds)
{
    qint64 days = seconds / (24 * 3600);
    seconds %= (24 * 3600);
    qint64 hours = seconds / 3600;
    seconds %= 3600;
    qint64 minutes = seconds / 60;
    qint64 remainingSeconds = seconds % 60;

    QString uptimeString;
    if (days > 0) {
        uptimeString += QString("%1d ").arg(days);
    }
    if (hours > 0 || days > 0) { // Show hours if days are present or if hours exist
        uptimeString += QString("%1h ").arg(hours);
    }
    if (minutes > 0 || hours > 0 || days > 0) { // Show minutes if hours/days are present or if minutes exist
        uptimeString += QString("%1m ").arg(minutes);
    }
    uptimeString += QString("%1s").arg(remainingSeconds);

    return uptimeString.trimmed();
}

QString DashboardWidget::formatNetworkSpeed(double bytesPerSecond)
{
    const double KB = 1024.0;
    const double MB = KB * 1024.0;
    const double GB = MB * 1024.0;

    if (bytesPerSecond >= GB) {
        return QString("%1 GB").arg(bytesPerSecond / GB, 0, 'f', 2);
    } else if (bytesPerSecond >= MB) {
        return QString("%1 MB").arg(bytesPerSecond / MB, 0, 'f', 2);
    } else if (bytesPerSecond >= KB) {
        return QString("%1 KB").arg(bytesPerSecond / KB, 0, 'f', 2);
    } else {
        return QString("%1 B").arg(bytesPerSecond, 0, 'f', 0);
    }
}
