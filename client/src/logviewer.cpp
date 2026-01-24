#include "logviewer.h"
#include <QHeaderView>
#include <QDateTime>
#include <QBrush>
#include <QColor>
#include <QLabel>
#include <QAbstractItemView>
#include <QTableWidgetItem>

LogViewer::LogViewer(QWidget *parent)
    : QWidget(parent)
    , m_currentPage(1)
    , m_totalPages(0)
    , m_totalItems(0)
{
    setupUI();
}

void LogViewer::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("System Logs", this);
    titleLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #14a085; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    QHBoxLayout *filterLayout = new QHBoxLayout();
    m_levelCombo = new QComboBox(this);
    m_levelCombo->addItem("All Levels", "");
    m_levelCombo->addItem("INFO", "INFO");
    m_levelCombo->addItem("WARNING", "WARNING");
    m_levelCombo->addItem("ERROR", "ERROR");
    m_levelCombo->addItem("DEBUG", "DEBUG");
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search logs...");
    
    m_refreshButton = new AnimatedButton("Refresh Logs", this);
    connect(m_refreshButton, &QPushButton::clicked, this, &LogViewer::onRefreshClicked);
    connect(m_levelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewer::onFilterChanged);

    filterLayout->addWidget(new QLabel("Level:", this));
    filterLayout->addWidget(m_levelCombo);
    filterLayout->addWidget(m_searchEdit);
    filterLayout->addWidget(m_refreshButton);
    filterLayout->addStretch();

    mainLayout->addLayout(filterLayout);

    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"ID", "Timestamp", "Level", "Source", "Message"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(
        "QTableWidget { background-color: #1a1a1a; alternate-background-color: #252525; color: #e0e0e0; gridline-color: #3d3d3d; border: 1px solid #2d2d2d; border-radius: 4px; }"
        "QTableWidget::item { padding: 6px; }"
        "QTableWidget::item:selected { background-color: #0d7377; color: #ffffff; }"
        "QHeaderView::section { background-color: #2d2d2d; color: #e0e0e0; padding: 8px; border: none; font-weight: bold; }"
    );

    mainLayout->addWidget(m_table);

    m_statusLabel = new QLabel("No logs loaded", this);
    m_statusLabel->setStyleSheet("color: #b0b0b0; font-size: 11px; padding: 5px;");
    mainLayout->addWidget(m_statusLabel);

    QHBoxLayout *paginationLayout = new QHBoxLayout();
    AnimatedButton *prevButton = new AnimatedButton("Previous", this);
    AnimatedButton *nextButton = new AnimatedButton("Next", this);
    
    connect(prevButton, &QPushButton::clicked, [this]() {
        if (m_currentPage > 1) {
            m_currentPage--;
            refreshLogs();
        }
    });
    
    connect(nextButton, &QPushButton::clicked, [this]() {
        if (m_currentPage < m_totalPages) {
            m_currentPage++;
            refreshLogs();
        }
    });

    paginationLayout->addWidget(prevButton);
    paginationLayout->addWidget(nextButton);
    paginationLayout->addStretch();

    mainLayout->addLayout(paginationLayout);
}

void LogViewer::updateLogs(const QJsonObject &response)
{
    if (response.contains("items")) {
        QJsonArray logs = response["items"].toArray();
        populateTable(logs);
        
        if (response.contains("total")) {
            m_totalItems = response["total"].toInt();
        }
        if (response.contains("pages")) {
            m_totalPages = response["pages"].toInt();
        }
        if (response.contains("page")) {
            m_currentPage = response["page"].toInt();
        }
        
        updatePagination();
    } else {
        m_statusLabel->setText("No logs in response");
        m_table->setRowCount(0);
    }
}

void LogViewer::populateTable(const QJsonArray &logs)
{
    m_table->setRowCount(logs.size());
    
    if (logs.isEmpty()) {
        m_statusLabel->setText("No logs found");
        return;
    }
    
    for (int i = 0; i < logs.size(); ++i) {
        QJsonObject log = logs[i].toObject();
        
        m_table->setItem(i, 0, new QTableWidgetItem(QString::number(log["id"].toInt())));
        
        QString timestamp = log["timestamp"].toString();
        if (!timestamp.isEmpty()) {
            m_table->setItem(i, 1, new QTableWidgetItem(timestamp));
        } else {
            m_table->setItem(i, 1, new QTableWidgetItem("-"));
        }
        
        QTableWidgetItem *levelItem = new QTableWidgetItem(log["level"].toString());
        QString level = log["level"].toString();
        if (level == "ERROR") {
            levelItem->setForeground(QBrush(QColor(255, 100, 100)));
            levelItem->setBackground(QBrush(QColor(40, 20, 20)));
        } else if (level == "WARNING") {
            levelItem->setForeground(QBrush(QColor(255, 200, 100)));
            levelItem->setBackground(QBrush(QColor(40, 35, 20)));
        } else if (level == "INFO") {
            levelItem->setForeground(QBrush(QColor(100, 200, 255)));
        } else if (level == "DEBUG") {
            levelItem->setForeground(QBrush(QColor(150, 150, 150)));
        }
        m_table->setItem(i, 2, levelItem);
        
        QString source = log["source"].toString();
        m_table->setItem(i, 3, new QTableWidgetItem(source.isEmpty() ? "-" : source));
        
        QString message = log["message"].toString();
        m_table->setItem(i, 4, new QTableWidgetItem(message));
    }
}

void LogViewer::updatePagination()
{
    m_statusLabel->setText(QString("Page %1 of %2 (%3 total logs)")
                          .arg(m_currentPage)
                          .arg(m_totalPages)
                          .arg(m_totalItems));
}

void LogViewer::refreshLogs()
{
    QString level = m_levelCombo->currentData().toString();
    emit requestLogs(m_currentPage, 100, level, QDateTime(), QDateTime());
}

void LogViewer::onRefreshClicked()
{
    m_currentPage = 1;
    refreshLogs();
}

void LogViewer::onFilterChanged()
{
    m_currentPage = 1;
    refreshLogs();
}
