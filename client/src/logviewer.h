#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDateTimeEdit>
#include <QTextEdit>
#include <QDialog>
#include "animatedbutton.h"

/**
 * @brief Dialog for viewing detailed log entry information
 */
class LogDetailDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogDetailDialog(const QJsonObject &log, QWidget *parent = nullptr);
};

/**
 * @brief Widget for viewing and filtering system logs
 * 
 * Provides filtering by level, date/time range, and search functionality.
 */
class LogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);

public slots:
    void updateLogs(const QJsonObject &response);
    void refreshLogs();
    void clearLogs();

signals:
    void requestLogs(int page, int pageSize, const QString &level,
                     const QDateTime &startTime, const QDateTime &endTime,
                     const QString &search = QString());
    void requestCreateLog(const QString &level, const QString &message, const QString &source = QString());

private slots:
    void onRefreshClicked();
    void onFilterChanged();
    void onSearchChanged();
    void onExportClicked();
    void onLogDoubleClicked(int row, int column);
    void onTimeRangeChanged();
    void onCreateLogClicked();

private:
    QTableWidget *m_table;
    AnimatedButton *m_refreshButton;
    AnimatedButton *m_exportButton;
    AnimatedButton *m_createLogButton;
    QLineEdit *m_searchEdit;
    QComboBox *m_levelCombo;
    QComboBox *m_timeRangeCombo;
    QLabel *m_statusLabel;
    
    int m_currentPage;
    int m_totalPages;
    int m_totalItems;
    QJsonArray m_currentLogs;

    void setupUI();
    void populateTable(const QJsonArray &logs);
    void updatePagination();
    QDateTime getStartTimeForRange(int rangeIndex) const;
    void exportToCSV(const QString &filename);
    void exportToJSON(const QString &filename);
};

#endif
