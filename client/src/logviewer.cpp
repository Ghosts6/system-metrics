#include "logviewer.h"
#include <QHeaderView>
#include <QDateTime>
#include <QBrush>
#include <QColor>
#include <QLabel>
#include <QAbstractItemView>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QTextStream>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QScrollArea>

// LogDetailDialog implementation
LogDetailDialog::LogDetailDialog(const QJsonObject &log, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Log Details");
    setModal(true);
    resize(700, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QTextEdit *textEdit = new QTextEdit(this);
    textEdit->setReadOnly(true);
    textEdit->setStyleSheet(
        "QTextEdit { background-color: #1a1a1a; color: #e0e0e0; border: 1px solid #2d2d2d; border-radius: 4px; padding: 10px; }"
    );
    
    QString details = QString("ID: %1\n"
                             "Timestamp: %2\n"
                             "Level: %3\n"
                             "Source: %4\n"
                             "Message: %5\n")
                      .arg(log["id"].toInt())
                      .arg(log["timestamp"].toString())
                      .arg(log["level"].toString())
                      .arg(log["source"].toString())
                      .arg(log["message"].toString());
    
    if (log.contains("log_metadata") && !log["log_metadata"].toString().isEmpty()) {
        details += QString("\nMetadata:\n%1").arg(log["log_metadata"].toString());
    }
    
    textEdit->setPlainText(details);
    
    layout->addWidget(textEdit);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttonBox);
}

// LogViewer implementation
LogViewer::LogViewer(QWidget *parent)
    : QWidget(parent)
    , m_currentPage(1)
    , m_totalPages(0)
    , m_totalItems(0)
    , m_createLogButton(nullptr)
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

    // Filter controls
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(10);
    
    QLabel *levelLabel = new QLabel("Level:", this);
    levelLabel->setStyleSheet("color: #e0e0e0; font-size: 12px;");
    m_levelCombo = new QComboBox(this);
    m_levelCombo->addItem("All Levels", "");
    m_levelCombo->addItem("INFO", "INFO");
    m_levelCombo->addItem("WARNING", "WARNING");
    m_levelCombo->addItem("ERROR", "ERROR");
    m_levelCombo->addItem("DEBUG", "DEBUG");
    
    QLabel *rangeLabel = new QLabel("Time Range:", this);
    rangeLabel->setStyleSheet("color: #e0e0e0; font-size: 12px;");
    m_timeRangeCombo = new QComboBox(this);
    m_timeRangeCombo->addItem("All Time", 0);
    m_timeRangeCombo->addItem("Last Hour", 1);
    m_timeRangeCombo->addItem("Last 6 Hours", 2);
    m_timeRangeCombo->addItem("Last 24 Hours", 3);
    m_timeRangeCombo->addItem("Last 7 Days", 4);
    m_timeRangeCombo->setCurrentIndex(0); // Default to "All Time"
    connect(m_timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewer::onTimeRangeChanged);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search in messages...");
    m_searchEdit->setMinimumWidth(200);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &LogViewer::onSearchChanged);
    
    m_refreshButton = new AnimatedButton("Refresh", this);
    connect(m_refreshButton, &QPushButton::clicked, this, &LogViewer::onRefreshClicked);
    
    m_exportButton = new AnimatedButton("Export", this);
    connect(m_exportButton, &QPushButton::clicked, this, &LogViewer::onExportClicked);

    m_createLogButton = new AnimatedButton("Create Test Log", this);
    connect(m_createLogButton, &QPushButton::clicked, this, &LogViewer::onCreateLogClicked);
    
    connect(m_levelCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LogViewer::onFilterChanged);

    filterLayout->addWidget(levelLabel);
    filterLayout->addWidget(m_levelCombo);
    filterLayout->addWidget(rangeLabel);
    filterLayout->addWidget(m_timeRangeCombo);
    filterLayout->addWidget(m_searchEdit);
    filterLayout->addWidget(m_refreshButton);
    filterLayout->addWidget(m_exportButton);
    filterLayout->addWidget(m_createLogButton);
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
    
    connect(m_table, &QTableWidget::cellDoubleClicked, this, &LogViewer::onLogDoubleClicked);

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

void LogViewer::onTimeRangeChanged()
{
    onFilterChanged();
}

QDateTime LogViewer::getStartTimeForRange(int rangeIndex) const
{
    QDateTime now = QDateTime::currentDateTime();
    switch (rangeIndex) {
        case 1: return now.addSecs(-3600);      // Last Hour
        case 2: return now.addSecs(-21600);     // Last 6 Hours
        case 3: return now.addDays(-1);         // Last 24 Hours
        case 4: return now.addDays(-7);         // Last 7 Days
        default: return QDateTime();            // All Time - invalid datetime means no filter
    }
}

void LogViewer::updateLogs(const QJsonObject &response)
{
    if (response.contains("items")) {
        QJsonArray logs = response["items"].toArray();
        m_currentLogs = logs;
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
        
        if (logs.isEmpty() && m_totalItems == 0) {
            m_statusLabel->setText("No logs found. Create logs via API or check filters.");
        }
    } else {
        m_statusLabel->setText("No logs in response. Check API connection.");
        m_table->setRowCount(0);
        m_currentLogs = QJsonArray();
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
    QDateTime startTime;
    QDateTime endTime;
    
    int rangeIndex = m_timeRangeCombo->currentIndex();
    if (rangeIndex == 0) {
        // All Time - no time filter (use invalid QDateTime)
        startTime = QDateTime();
        endTime = QDateTime();
    } else {
        // Calculate time range based on selection
        endTime = QDateTime::currentDateTime();
        startTime = getStartTimeForRange(rangeIndex);
    }
    
    QString search = m_searchEdit->text().trimmed();
    emit requestLogs(m_currentPage, 100, level, startTime, endTime, search);
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

void LogViewer::onSearchChanged()
{
    // Debounce search - could add QTimer here for better UX
    m_currentPage = 1;
    refreshLogs();
}

void LogViewer::onLogDoubleClicked(int row, int column)
{
    Q_UNUSED(column);
    
    if (row < 0 || row >= m_currentLogs.size()) {
        return;
    }
    
    QJsonObject log = m_currentLogs[row].toObject();
    LogDetailDialog dialog(log, this);
    dialog.exec();
}

void LogViewer::onExportClicked()
{
    if (m_currentLogs.isEmpty()) {
        QMessageBox::warning(this, "Export", "No logs to export");
        return;
    }
    
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Export Logs",
        QString("logs_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss")),
        "CSV Files (*.csv);;JSON Files (*.json);;All Files (*.*)"
    );
    
    if (filename.isEmpty()) {
        return;
    }
    
    if (filename.endsWith(".json", Qt::CaseInsensitive)) {
        exportToJSON(filename);
    } else {
        if (!filename.endsWith(".csv", Qt::CaseInsensitive)) {
            filename += ".csv";
        }
        exportToCSV(filename);
    }
}

void LogViewer::onCreateLogClicked()
{
    // Emit a signal to request creation of a test log
    emit requestCreateLog("INFO", "Test log created from GUI", "GUI");
    refreshLogs();
}

void LogViewer::exportToCSV(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Error", "Failed to open file for writing");
        return;
    }
    
    QTextStream out(&file);
    
    // Header
    out << "ID,Timestamp,Level,Source,Message,Metadata\n";
    
    // Data
    for (const QJsonValue &value : m_currentLogs) {
        QJsonObject log = value.toObject();
        out << log["id"].toInt() << ","
            << "\"" << log["timestamp"].toString() << "\","
            << log["level"].toString() << ","
            << "\"" << log["source"].toString() << "\","
            << "\"" << log["message"].toString().replace("\"", "\"\"") << "\","
            << "\"" << log["log_metadata"].toString().replace("\"", "\"\"") << "\"\n";
    }
    
    file.close();
    QMessageBox::information(this, "Export", QString("Exported %1 logs to %2").arg(m_currentLogs.size()).arg(filename));
}

void LogViewer::exportToJSON(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Export Error", "Failed to open file for writing");
        return;
    }
    
    QJsonDocument doc(m_currentLogs);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    QMessageBox::information(this, "Export", QString("Exported %1 logs to %2").arg(m_currentLogs.size()).arg(filename));
}
