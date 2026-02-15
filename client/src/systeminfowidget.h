#ifndef SYSTEMINFOWIDGET_H
#define SYSTEMINFOWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QJsonObject>

/**
 * @brief Widget displaying detailed system information
 * 
 * Shows comprehensive system details including hardware and software information.
 */
class SystemInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SystemInfoWidget(QWidget *parent = nullptr);

public slots:
    void updateSystemInfo(const QJsonObject &metrics);
    void clearInfo();

private:
    void setupUI();
    void populateInfoTable(const QJsonObject &metrics);
    
    QTableWidget *m_infoTable;
    QLabel *m_statusLabel;
};

#endif // SYSTEMINFOWIDGET_H
