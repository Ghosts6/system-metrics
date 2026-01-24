#ifndef METRICSWIDGET_H
#define METRICSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QJsonObject>

class MetricsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetricsWidget(QWidget *parent = nullptr);

public slots:
    void updateMetrics(const QJsonObject &metrics);

private:
    QLabel *m_cpuLabel;
    QLabel *m_memoryLabel;
    QLabel *m_diskLabel;
    QLabel *m_networkLabel;
    QLabel *m_hostnameLabel;
    QLabel *m_platformLabel;
    QLabel *m_timestampLabel;

    QProgressBar *m_cpuBar;
    QProgressBar *m_memoryBar;
    QProgressBar *m_diskBar;

    void setupUI();
    QString formatBytes(qint64 bytes);
};

#endif
