#include "systeminfowidget.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QJsonObject>
#include <QFont>
#include <QBrush>
#include <QColor>
#include <QJsonArray>

static QString formatBytes(qint64 bytes)
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

SystemInfoWidget::SystemInfoWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void SystemInfoWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("System Information", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #14a085; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    m_infoTable = new QTableWidget(this);
    m_infoTable->setColumnCount(2);
    m_infoTable->setHorizontalHeaderLabels({"Property", "Value"});
    m_infoTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // Property column
    m_infoTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);         // Value column
    m_infoTable->setSelectionBehavior(QAbstractItemView::SelectItems);
    m_infoTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_infoTable->setAlternatingRowColors(true);
    m_infoTable->verticalHeader()->setVisible(false);
    m_infoTable->setShowGrid(false);
    m_infoTable->setStyleSheet(
        "QTableWidget { background-color: #1a1a1a; alternate-background-color: #252525; color: #e0e0e0; gridline-color: #3d3d3d; border: 1px solid #2d2d2d; border-radius: 4px; }"
        "QTableWidget::item { padding: 8px; border: none; }"
        "QTableWidget::item:selected { background-color: #0d7377; color: #ffffff; }"
        "QHeaderView::section { background-color: #2d2d2d; color: #e0e0e0; padding: 8px; border: none; font-weight: bold; }"
    );

    mainLayout->addWidget(m_infoTable);

    m_statusLabel = new QLabel("No system information available", this);
    m_statusLabel->setStyleSheet("color: #b0b0b0; font-size: 11px; padding: 5px;");
    mainLayout->addWidget(m_statusLabel);
}

void SystemInfoWidget::updateSystemInfo(const QJsonObject &metrics)
{
    populateInfoTable(metrics);
    m_statusLabel->setText("System information updated");
}

void SystemInfoWidget::populateInfoTable(const QJsonObject &metrics)
{
    m_infoTable->setUpdatesEnabled(false); // Disable updates

    struct InfoRow {
        QString property;
        QString value;
    };

    QList<InfoRow> rows;

    // System Information
    rows.append(InfoRow{"System Information", ""});
    if (metrics.contains("hostname")) {
        rows.append(InfoRow{"Hostname", metrics["hostname"].toString()});
    }
    if (metrics.contains("platform")) {
        rows.append(InfoRow{"Platform", metrics["platform"].toString()});
    }
    if (metrics.contains("timestamp")) {
        rows.append(InfoRow{"Last Update", metrics["timestamp"].toString()});
    }

    rows.append(InfoRow{"", ""}); // Separator

    // CPU Information
    rows.append(InfoRow{"CPU Information", ""});
    if (metrics.contains("cpu_count")) {
        rows.append(InfoRow{"CPU Cores", QString::number(metrics["cpu_count"].toInt())});
    }
    if (metrics.contains("cpu_brand")) {
        rows.append(InfoRow{"CPU Brand", metrics["cpu_brand"].toString()});
    }
    if (metrics.contains("cpu_vendor_id")) {
        rows.append(InfoRow{"CPU Vendor ID", metrics["cpu_vendor_id"].toString()});
    }
    if (metrics.contains("cpu_percent")) {
        rows.append(InfoRow{"CPU Usage", QString("%1%").arg(metrics["cpu_percent"].toDouble(), 0, 'f', 2)});
    }
    if (metrics.contains("cpu_freq_current")) {
        rows.append(InfoRow{"CPU Frequency (Current)", QString("%1 MHz").arg(metrics["cpu_freq_current"].toDouble(), 0, 'f', 0)});
    }
    if (metrics.contains("cpu_freq_min")) {
        rows.append(InfoRow{"CPU Frequency (Min)", QString("%1 MHz").arg(metrics["cpu_freq_min"].toDouble(), 0, 'f', 0)});
    }
    if (metrics.contains("cpu_freq_max")) {
        rows.append(InfoRow{"CPU Frequency (Max)", QString("%1 MHz").arg(metrics["cpu_freq_max"].toDouble(), 0, 'f', 0)});
    }

    rows.append(InfoRow{"", ""}); // Separator

    // GPU Information
    if (metrics.contains("gpus") && metrics["gpus"].isArray()) {
        QJsonArray gpus = metrics["gpus"].toArray();
        if (!gpus.isEmpty()) {
            rows.append(InfoRow{"GPU Information", ""});
            for (int i = 0; i < gpus.size(); ++i) {
                QJsonObject gpu = gpus[i].toObject();
                rows.append(InfoRow{QString("GPU %1 Name").arg(i), gpu["name"].toString()});
                rows.append(InfoRow{QString("GPU %1 Driver").arg(i), gpu["driver_version"].toString()});
                rows.append(InfoRow{QString("GPU %1 Temp").arg(i), QString("%1 °C").arg(gpu["temperature"].toDouble(), 0, 'f', 1)});
                rows.append(InfoRow{QString("GPU %1 Util").arg(i), QString("%1 %").arg(gpu["utilization"].toDouble(), 0, 'f', 1)});
                rows.append(InfoRow{QString("GPU %1 Memory").arg(i), QString("%1 / %2").arg(formatBytes(gpu["memory_used"].toDouble())).arg(formatBytes(gpu["memory_total"].toDouble()))});
            }
            rows.append(InfoRow{"", ""}); // Separator
        }
    }

    // Memory Information
    rows.append(InfoRow{"Memory Information", ""});
    if (metrics.contains("memory_total")) {
        qint64 total = static_cast<qint64>(metrics["memory_total"].toDouble());
        rows.append(InfoRow{"Total Memory", formatBytes(total)});
    }
    if (metrics.contains("memory_used")) {
        qint64 used = static_cast<qint64>(metrics["memory_used"].toDouble());
        rows.append(InfoRow{"Used Memory", formatBytes(used)});
    }
    if (metrics.contains("memory_available")) {
        qint64 available = static_cast<qint64>(metrics["memory_available"].toDouble());
        rows.append(InfoRow{"Available Memory", formatBytes(available)});
    }
    if (metrics.contains("memory_percent")) {
        rows.append(InfoRow{"Memory Usage", QString("%1%").arg(metrics["memory_percent"].toDouble(), 0, 'f', 2)});
    }

    rows.append(InfoRow{"", ""}); // Separator

    // Disk Information
    rows.append(InfoRow{"Disk Information", ""});
    if (metrics.contains("disk_total")) {
        qint64 total = static_cast<qint64>(metrics["disk_total"].toDouble());
        rows.append(InfoRow{"Total Disk Space", formatBytes(total)});
    }
    if (metrics.contains("disk_used")) {
        qint64 used = static_cast<qint64>(metrics["disk_used"].toDouble());
        rows.append(InfoRow{"Used Disk Space", formatBytes(used)});
    }
    if (metrics.contains("disk_free")) {
        qint64 free = static_cast<qint64>(metrics["disk_free"].toDouble());
        rows.append(InfoRow{"Free Disk Space", formatBytes(free)});
    }
    if (metrics.contains("disk_percent")) {
        rows.append(InfoRow{"Disk Usage", QString("%1%").arg(metrics["disk_percent"].toDouble(), 0, 'f', 2)});
    }

    rows.append(InfoRow{"", ""}); // Separator

    // Network Information
    rows.append(InfoRow{"Network Information", ""});
    if (metrics.contains("network_bytes_sent")) {
        qint64 sent = static_cast<qint64>(metrics["network_bytes_sent"].toDouble());
        rows.append(InfoRow{"Bytes Sent", formatBytes(sent)});
    }
    if (metrics.contains("network_bytes_recv")) {
        qint64 recv = static_cast<qint64>(metrics["network_bytes_recv"].toDouble());
        rows.append(InfoRow{"Bytes Received", formatBytes(recv)});
    }

    // Define section headers
    QSet<QString> sectionHeaders = {
        "System Information",
        "CPU Information",
        "GPU Information",
        "Memory Information",
        "Disk Information",
        "Network Information"
    };

    // Populate table
    m_infoTable->setRowCount(rows.size());
    
    for (int i = 0; i < rows.size(); ++i) {
        const InfoRow &row = rows[i];
        
        QTableWidgetItem *propertyItem = new QTableWidgetItem(row.property);
        QTableWidgetItem *valueItem = new QTableWidgetItem(row.value);
        
        // Style section headers
        if (sectionHeaders.contains(row.property)) {
            QFont headerFont = propertyItem->font();
            headerFont.setBold(true);
            headerFont.setPointSize(headerFont.pointSize() + 1);
            
            propertyItem->setFont(headerFont);
            propertyItem->setForeground(QBrush(QColor(20, 160, 133))); // #14a085
            valueItem->setText("");
        } else if (row.property.isEmpty() && row.value.isEmpty()) {
            // Separator row
            propertyItem->setBackground(QBrush(QColor(45, 45, 45)));
            valueItem->setBackground(QBrush(QColor(45, 45, 45)));
        }
        
        m_infoTable->setItem(i, 0, propertyItem);
        m_infoTable->setItem(i, 1, valueItem);
    }
    
    m_infoTable->setUpdatesEnabled(true); // Re-enable updates
    m_infoTable->viewport()->update(); // Force a repaint of the viewport
}

void SystemInfoWidget::clearInfo()
{
    m_infoTable->setRowCount(0);
    m_statusLabel->setText("No system information");
}