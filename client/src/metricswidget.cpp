#include "metricswidget.h"
#include <QDateTime>
#include <QLocale>
#include <QStyle>

MetricsWidget::MetricsWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void MetricsWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("System Metrics", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #14a085; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setSpacing(15);
    gridLayout->setColumnMinimumWidth(1, 200);

    m_hostnameLabel = new QLabel("Hostname: -", this);
    m_hostnameLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    m_platformLabel = new QLabel("Platform: -", this);
    m_platformLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    m_timestampLabel = new QLabel("Last Update: -", this);
    m_timestampLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    
    gridLayout->addWidget(m_hostnameLabel, 0, 0);
    gridLayout->addWidget(m_platformLabel, 0, 1);
    gridLayout->addWidget(m_timestampLabel, 0, 2);

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

    gridLayout->addWidget(m_cpuLabel, 1, 0);
    gridLayout->addWidget(m_cpuBar, 1, 1, 1, 2);

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

    gridLayout->addWidget(m_memoryLabel, 2, 0);
    gridLayout->addWidget(m_memoryBar, 2, 1, 1, 2);

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

    gridLayout->addWidget(m_diskLabel, 3, 0);
    gridLayout->addWidget(m_diskBar, 3, 1, 1, 2);

    m_networkLabel = new QLabel("Network: Sent 0, Received 0", this);
    m_networkLabel->setStyleSheet("color: #b0b0b0; font-size: 12px; padding: 5px;");
    gridLayout->addWidget(m_networkLabel, 4, 0, 1, 3);

    mainLayout->addLayout(gridLayout);
    mainLayout->addStretch();
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
    }

    if (metrics.contains("memory_total")) {
        qint64 total = static_cast<qint64>(metrics["memory_total"].toDouble());
        qint64 used = static_cast<qint64>(metrics["memory_used"].toDouble());
        double percent = metrics["memory_percent"].toDouble();
        
        m_memoryLabel->setText(QString("Memory: %1 / %2")
                              .arg(formatBytes(used))
                              .arg(formatBytes(total)));
        m_memoryBar->setValue(static_cast<int>(percent));
    }

    if (metrics.contains("disk_total")) {
        qint64 total = static_cast<qint64>(metrics["disk_total"].toDouble());
        qint64 used = static_cast<qint64>(metrics["disk_used"].toDouble());
        double percent = metrics["disk_percent"].toDouble();
        
        m_diskLabel->setText(QString("Disk: %1 / %2")
                            .arg(formatBytes(used))
                            .arg(formatBytes(total)));
        m_diskBar->setValue(static_cast<int>(percent));
    }

    if (metrics.contains("network_bytes_sent") && metrics.contains("network_bytes_recv")) {
        qint64 sent = static_cast<qint64>(metrics["network_bytes_sent"].toDouble());
        qint64 recv = static_cast<qint64>(metrics["network_bytes_recv"].toDouble());
        
        m_networkLabel->setText(QString("Network: Sent %1, Received %2")
                               .arg(formatBytes(sent))
                               .arg(formatBytes(recv)));
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
