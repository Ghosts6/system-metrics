#include "dashboardwidget.h"
#include <QJsonArray>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QJsonObject>
#include <QSpacerItem>
#include <QRegularExpression>

DashboardWidget::DashboardWidget(QWidget *parent)
    : QWidget(parent)
    , m_prevNetworkSent(0)
    , m_prevNetworkRecv(0)
    , m_lastMetricTimestamp(QDateTime())
    , m_currentRow(0)
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
    m_hostnameLabel->setMinimumWidth(150);
    m_platformLabel = new QLabel("Platform: -", this);
    m_platformLabel->setStyleSheet("color: #b0b0b0; font-size: 12px; padding: 5px;");
    m_platformLabel->setMinimumWidth(150);
    m_uptimeLabel = new QLabel("Uptime: -", this);
    m_uptimeLabel->setStyleSheet("color: #b0b0b0; font-size: 12px; padding: 5px;");
    m_uptimeLabel->setMinimumWidth(150);
    
    systemInfoLayout->addWidget(m_hostnameLabel);
    systemInfoLayout->addWidget(m_platformLabel);
    systemInfoLayout->addWidget(m_uptimeLabel);
    systemInfoLayout->addStretch();
    
    mainLayout->addLayout(systemInfoLayout);

    // Metrics grid
    m_metricsLayout = new QGridLayout();
    m_metricsLayout->setSpacing(20);
    m_metricsLayout->setColumnStretch(0, 0); // Fixed width for label column
    m_metricsLayout->setColumnStretch(1, 1); // Expanding for progress bar column
    m_metricsLayout->setColumnMinimumWidth(0, 120); // Minimum width for percentage labels

    m_currentRow = 0;

    // CPU section
    m_cpuTitle = new QLabel("CPU Usage", this);
    m_cpuTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    m_metricsLayout->addWidget(m_cpuTitle, m_currentRow++, 0, 1, 2);
    
    m_cpuLabel = new QLabel("0%", this);
    m_cpuLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");
    m_cpuLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_cpuLabel->setMinimumWidth(120);
    m_cpuLabel->setMaximumWidth(120);
    m_metricsLayout->addWidget(m_cpuLabel, m_currentRow, 0);
    
    m_cpuBar = new QProgressBar(this);
    m_cpuBar->setRange(0, 100);
    m_cpuBar->setValue(0);
    m_cpuBar->setTextVisible(true);
    m_cpuBar->setFormat("%p%");
    m_cpuBar->setMinimumHeight(35);
    m_cpuBar->setMaximumHeight(35);
    m_cpuBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_cpuBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; text-align: center; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );
    m_metricsLayout->addWidget(m_cpuBar, m_currentRow, 1);
    
    m_cpuDetailLabel = new QLabel("Cores: -", this);
    m_cpuDetailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    m_metricsLayout->addWidget(m_cpuDetailLabel, m_currentRow + 1, 0, 1, 2);
    m_currentRow += 2;

    // Add spacer after CPU section
    m_metricsLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Fixed), m_currentRow++, 0, 1, 2);

    // GPU section title (always visible)
    m_gpuSectionTitle = new QLabel("GPU Usage", this);
    m_gpuSectionTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    m_metricsLayout->addWidget(m_gpuSectionTitle, m_currentRow++, 0, 1, 2);

    // Reserve space for GPU widgets (they will be inserted here dynamically)
    m_gpuStartRow = m_currentRow;
    
    // Create initial GPU widget with 0% (will be updated when metrics arrive)
    GpuDisplayWidgets initialWidgets;
    initialWidgets.nameLabel = new QLabel("0%", this);
    initialWidgets.nameLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #b0b0b0;");
    initialWidgets.nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    initialWidgets.nameLabel->setMinimumWidth(120);
    initialWidgets.nameLabel->setMaximumWidth(120);
    
    initialWidgets.utilBar = new QProgressBar(this);
    initialWidgets.utilBar->setRange(0, 100);
    initialWidgets.utilBar->setValue(0);
    initialWidgets.utilBar->setTextVisible(true);
    initialWidgets.utilBar->setFormat("0%");
    initialWidgets.utilBar->setMinimumHeight(35);
    initialWidgets.utilBar->setMaximumHeight(35);
    initialWidgets.utilBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    initialWidgets.utilBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #b0b0b0; font-weight: bold; text-align: center; }"
        "QProgressBar::chunk { background-color: #3d3d3d; border-radius: 6px; }"
    );
    
    initialWidgets.detailLabel = new QLabel("Name: -", this);
    initialWidgets.detailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
    
    m_metricsLayout->addWidget(initialWidgets.nameLabel, m_currentRow, 0);
    m_metricsLayout->addWidget(initialWidgets.utilBar, m_currentRow, 1);
    m_metricsLayout->addWidget(initialWidgets.detailLabel, m_currentRow + 1, 0, 1, 2);
    m_currentRow += 2;
    
    m_gpuDisplayWidgets.append(initialWidgets);

    // Memory section
    m_memoryTitle = new QLabel("Memory Usage", this);
    m_memoryTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    m_memoryLabel = new QLabel("0%", this);
    m_memoryLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");
    m_memoryLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_memoryLabel->setMinimumWidth(120);
    m_memoryLabel->setMaximumWidth(120);
    m_memoryBar = new QProgressBar(this);
    m_memoryBar->setRange(0, 100);
    m_memoryBar->setValue(0);
    m_memoryBar->setTextVisible(true);
    m_memoryBar->setFormat("%p%");
    m_memoryBar->setMinimumHeight(35);
    m_memoryBar->setMaximumHeight(35);
    m_memoryBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_memoryBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; text-align: center; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );
    m_memoryDetailLabel = new QLabel("Used: - / Total: -", this);
    m_memoryDetailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");

    // Disk section
    m_diskTitle = new QLabel("Disk Usage", this);
    m_diskTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    m_diskLabel = new QLabel("0%", this);
    m_diskLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");
    m_diskLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_diskLabel->setMinimumWidth(120);
    m_diskLabel->setMaximumWidth(120);
    m_diskBar = new QProgressBar(this);
    m_diskBar->setRange(0, 100);
    m_diskBar->setValue(0);
    m_diskBar->setTextVisible(true);
    m_diskBar->setFormat("%p%");
    m_diskBar->setMinimumHeight(35);
    m_diskBar->setMaximumHeight(35);
    m_diskBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_diskBar->setStyleSheet(
        "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; text-align: center; }"
        "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
    );
    m_diskDetailLabel = new QLabel("Used: - / Total: -", this);
    m_diskDetailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");

    // Network section
    m_networkTitle = new QLabel("Network Activity", this);
    m_networkTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #14a085;");
    m_networkSentLabel = new QLabel("Sent: 0 B", this);
    m_networkSentLabel->setStyleSheet("color: #e0e0e0; font-size: 14px; padding: 5px;");
    m_networkSentLabel->setMinimumWidth(200);
    m_networkRecvLabel = new QLabel("Received: 0 B", this);
    m_networkRecvLabel->setStyleSheet("color: #e0e0e0; font-size: 14px; padding: 5px;");
    m_networkRecvLabel->setMinimumWidth(200);

    // Initially place all sections (will be repositioned if GPUs exist)
    updateSectionPositions();

    mainLayout->addLayout(m_metricsLayout);
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

void DashboardWidget::updateSectionPositions()
{
    // Calculate current row after GPU section
    int currentRow = m_gpuStartRow;
    
    // Account for GPU widgets
    currentRow += m_gpuDisplayWidgets.size() * 2; // Each GPU takes 2 rows
    
    // Add spacer before memory section if GPUs exist
    if (!m_gpuDisplayWidgets.isEmpty()) {
        currentRow++; // Space after GPU section
    }

    // Remove all non-GPU widgets from layout
    m_metricsLayout->removeWidget(m_memoryTitle);
    m_metricsLayout->removeWidget(m_memoryLabel);
    m_metricsLayout->removeWidget(m_memoryBar);
    m_metricsLayout->removeWidget(m_memoryDetailLabel);
    
    m_metricsLayout->removeWidget(m_diskTitle);
    m_metricsLayout->removeWidget(m_diskLabel);
    m_metricsLayout->removeWidget(m_diskBar);
    m_metricsLayout->removeWidget(m_diskDetailLabel);

    m_metricsLayout->removeWidget(m_networkTitle);
    m_metricsLayout->removeWidget(m_networkSentLabel);
    m_metricsLayout->removeWidget(m_networkRecvLabel);

    // Re-add Memory section
    m_metricsLayout->addWidget(m_memoryTitle, currentRow++, 0, 1, 2);
    m_metricsLayout->addWidget(m_memoryLabel, currentRow, 0);
    m_metricsLayout->addWidget(m_memoryBar, currentRow, 1);
    m_metricsLayout->addWidget(m_memoryDetailLabel, currentRow + 1, 0, 1, 2);
    currentRow += 2;

    // Re-add Disk section
    m_metricsLayout->addWidget(m_diskTitle, currentRow++, 0, 1, 2);
    m_metricsLayout->addWidget(m_diskLabel, currentRow, 0);
    m_metricsLayout->addWidget(m_diskBar, currentRow, 1);
    m_metricsLayout->addWidget(m_diskDetailLabel, currentRow + 1, 0, 1, 2);
    currentRow += 2;

    // Re-add Network section
    m_metricsLayout->addWidget(m_networkTitle, currentRow++, 0, 1, 2);
    m_metricsLayout->addWidget(m_networkSentLabel, currentRow, 0);
    m_metricsLayout->addWidget(m_networkRecvLabel, currentRow, 1);
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
        
        // Build CPU detail string with cores and brand
        QString cpuDetail;
        if (metrics.contains("cpu_count")) {
            cpuDetail = QString("Cores: %1").arg(metrics["cpu_count"].toInt());
        }
        
        if (metrics.contains("cpu_brand")) {
            QString cpuBrand = metrics["cpu_brand"].toString();
            // Extract model name (e.g., "i7-12700KF" from "12th Gen Intel(R) Core(TM) i7-12700KF")
            QString modelName = extractCpuModel(cpuBrand);
            
            if (!modelName.isEmpty()) {
                if (!cpuDetail.isEmpty()) {
                    cpuDetail += QString("  |  %1").arg(modelName);
                } else {
                    cpuDetail = modelName;
                }
            }
        }
        
        if (!cpuDetail.isEmpty()) {
            m_cpuDetailLabel->setText(cpuDetail);
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
                m_networkSentLabel->setText(QString("Sent: %1 (%2/s)")
                                              .arg(formatBytes(currentSent))
                                              .arg(formatNetworkSpeed(sentSpeed)));
                m_networkRecvLabel->setText(QString("Received: %1 (%2/s)")
                                              .arg(formatBytes(currentRecv))
                                              .arg(formatNetworkSpeed(recvSpeed)));
            }
        }

        m_prevNetworkSent = currentSent;
        m_prevNetworkRecv = currentRecv;
        m_lastMetricTimestamp = currentTimestamp;
    }

    // GPU (always show at least one GPU section, even if count is 0)
    QJsonArray gpusArray = metrics.contains("gpus") && metrics["gpus"].isArray() ? metrics["gpus"].toArray() : QJsonArray();
    
    // Always show GPU section - if no GPUs, show one with 0%
    int displayGpuCount = gpusArray.isEmpty() ? 1 : gpusArray.size();
    
    // Only rebuild GPU widgets if the count has changed
    if (displayGpuCount != m_gpuDisplayWidgets.size()) {
        // Clear existing GPU widgets
        for (const auto& widgets : m_gpuDisplayWidgets) {
            m_metricsLayout->removeWidget(widgets.nameLabel);
            m_metricsLayout->removeWidget(widgets.utilBar);
            m_metricsLayout->removeWidget(widgets.detailLabel);
            delete widgets.nameLabel;
            delete widgets.utilBar;
            delete widgets.detailLabel;
        }
        m_gpuDisplayWidgets.clear();

        int currentRow = m_gpuStartRow;
        
        // Always show GPU section
        m_gpuSectionTitle->show();
        
        // Create and add GPU widgets (at least one)
        for (int i = 0; i < displayGpuCount; ++i) {
            GpuDisplayWidgets newWidgets;

            newWidgets.nameLabel = new QLabel("0%", this);
            newWidgets.nameLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #b0b0b0;");
            newWidgets.nameLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            newWidgets.nameLabel->setMinimumWidth(120);
            newWidgets.nameLabel->setMaximumWidth(120);

            newWidgets.utilBar = new QProgressBar(this);
            newWidgets.utilBar->setRange(0, 100);
            newWidgets.utilBar->setValue(0);
            newWidgets.utilBar->setTextVisible(true);
            newWidgets.utilBar->setFormat("0%");
            newWidgets.utilBar->setMinimumHeight(35);
            newWidgets.utilBar->setMaximumHeight(35);
            newWidgets.utilBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            newWidgets.utilBar->setStyleSheet(
                "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #b0b0b0; font-weight: bold; text-align: center; }"
                "QProgressBar::chunk { background-color: #3d3d3d; border-radius: 6px; }"
            );

            newWidgets.detailLabel = new QLabel("Name: -", this);
            newWidgets.detailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");

            m_metricsLayout->addWidget(newWidgets.nameLabel, currentRow, 0);
            m_metricsLayout->addWidget(newWidgets.utilBar, currentRow, 1);
            m_metricsLayout->addWidget(newWidgets.detailLabel, currentRow + 1, 0, 1, 2);
            
            m_gpuDisplayWidgets.append(newWidgets);
            currentRow += 2;
        }

        // Reposition all sections after GPU widgets
        updateSectionPositions();
    }
    
    // Update GPU widget values (without changing layout)
    if (!gpusArray.isEmpty()) {
        // Update with actual GPU data
        for (int i = 0; i < gpusArray.size() && i < m_gpuDisplayWidgets.size(); ++i) {
            QJsonObject gpuData = gpusArray[i].toObject();
            GpuDisplayWidgets &widgets = m_gpuDisplayWidgets[i];
            
            double utilization = gpuData["utilization"].toDouble();
            widgets.nameLabel->setText(formatPercent(utilization));
            widgets.nameLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #e0e0e0;");
            widgets.utilBar->setValue(static_cast<int>(utilization));
            widgets.utilBar->setFormat("%p%");
            widgets.utilBar->setStyleSheet(
                "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #e0e0e0; font-weight: bold; text-align: center; }"
                "QProgressBar::chunk { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0d7377, stop:0.5 #14a085, stop:1 #0d7377); border-radius: 6px; }"
            );

            QString gpuName = gpuData["name"].toString();
            if (gpuName.isEmpty()) {
                gpuName = "Unknown GPU";
            }
            widgets.detailLabel->setText(QString("Name: %1").arg(gpuName));
            widgets.detailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
        }
    } else {
        // Show 0% for all GPU widgets when no GPU data
        for (int i = 0; i < m_gpuDisplayWidgets.size(); ++i) {
            GpuDisplayWidgets &widgets = m_gpuDisplayWidgets[i];
            widgets.nameLabel->setText("0%");
            widgets.nameLabel->setStyleSheet("font-size: 32px; font-weight: bold; color: #b0b0b0;");
            widgets.utilBar->setValue(0);
            widgets.utilBar->setFormat("0%");
            widgets.utilBar->setStyleSheet(
                "QProgressBar { border: 2px solid #2d2d2d; border-radius: 8px; background-color: #2d2d2d; color: #b0b0b0; font-weight: bold; text-align: center; }"
                "QProgressBar::chunk { background-color: #3d3d3d; border-radius: 6px; }"
            );
            widgets.detailLabel->setText("Name: -");
            widgets.detailLabel->setStyleSheet("color: #b0b0b0; font-size: 11px;");
        }
    }
}

QString DashboardWidget::extractCpuModel(const QString &cpuBrand)
{
    // Extract the actual model name from CPU brand string
    // Examples:
    // "12th Gen Intel(R) Core(TM) i7-12700KF" -> "i7-12700KF"
    // "AMD Ryzen 9 5950X 16-Core Processor" -> "Ryzen 9 5950X"
    
    QString brand = cpuBrand.trimmed();
    
    // Intel pattern: look for iX-XXXXX
    QRegularExpression intelPattern("(i[3579]-\\w+)");
    QRegularExpressionMatch intelMatch = intelPattern.match(brand);
    if (intelMatch.hasMatch()) {
        return intelMatch.captured(1);
    }
    
    // AMD Ryzen pattern: look for "Ryzen X XXXX"
    QRegularExpression ryzenPattern("(Ryzen [0-9] \\w+)");
    QRegularExpressionMatch ryzenMatch = ryzenPattern.match(brand);
    if (ryzenMatch.hasMatch()) {
        QString match = ryzenMatch.captured(1);
        // Remove "Processor" or "X-Core" suffix if present
        match = match.remove(QRegularExpression("\\s+\\d+-Core.*"));
        match = match.remove(QRegularExpression("\\s+Processor.*"));
        return match.trimmed();
    }
    
    // For other CPUs, try to get a reasonable substring
    // Remove common noise words
    brand = brand.remove(QRegularExpression("Gen\\s+"));
    brand = brand.remove(QRegularExpression("Intel\\(R\\)\\s*"));
    brand = brand.remove(QRegularExpression("AMD\\s+"));
    brand = brand.remove(QRegularExpression("Core\\(TM\\)\\s*"));
    brand = brand.remove(QRegularExpression("\\s+Processor.*"));
    brand = brand.remove(QRegularExpression("\\s+\\d+-Core.*"));
    brand = brand.trimmed();
    
    // Limit length to avoid breaking layout
    if (brand.length() > 25) {
        brand = brand.left(25) + "...";
    }
    
    return brand;
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
    if (hours > 0 || days > 0) {
        uptimeString += QString("%1h ").arg(hours);
    }
    if (minutes > 0 || hours > 0 || days > 0) {
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

void DashboardWidget::clearMetrics()
{
    m_hostnameLabel->setText("Hostname: -");
    m_platformLabel->setText("Platform: -");
    m_uptimeLabel->setText("Uptime: -");
    m_cpuLabel->setText("0%");
    m_cpuBar->setValue(0);
    m_cpuDetailLabel->setText("-");
    m_memoryLabel->setText("0%");
    m_memoryBar->setValue(0);
    m_memoryDetailLabel->setText("-");
    m_diskLabel->setText("0%");
    m_diskBar->setValue(0);
    m_diskDetailLabel->setText("-");
    m_networkSentLabel->setText("Sent: -");
    m_networkRecvLabel->setText("Received: -");
    m_statusLabel->setText("No data");
    m_lastUpdateLabel->setText("Last update: -");
    m_prevNetworkSent = 0;
    m_prevNetworkRecv = 0;
    
    for (auto &widgets : m_gpuDisplayWidgets) {
        widgets.nameLabel->setText("0%");
        widgets.utilBar->setValue(0);
        widgets.detailLabel->setText("Name: -");
    }
}