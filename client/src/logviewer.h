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
#include "animatedbutton.h"

class LogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);

public slots:
    void updateLogs(const QJsonObject &response);
    void refreshLogs();

signals:
    void requestLogs(int page, int pageSize, const QString &level,
                     const QDateTime &startTime, const QDateTime &endTime);

private slots:
    void onRefreshClicked();
    void onFilterChanged();

private:
    QTableWidget *m_table;
    AnimatedButton *m_refreshButton;
    QLineEdit *m_searchEdit;
    QComboBox *m_levelCombo;
    QLabel *m_statusLabel;
    
    int m_currentPage;
    int m_totalPages;
    int m_totalItems;

    void setupUI();
    void populateTable(const QJsonArray &logs);
    void updatePagination();
};

#endif
