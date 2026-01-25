#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QTimer>

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject *parent = nullptr);
    ~ApiClient();

    void setBaseUrl(const QString &url);
    QString baseUrl() const { return m_baseUrl; }

    void fetchLiveMetrics();
    void fetchMetricsHistory(int page = 1, int pageSize = 100, 
                             const QDateTime &startTime = QDateTime(), 
                             const QDateTime &endTime = QDateTime());
    void fetchLogs(int page = 1, int pageSize = 100, 
                   const QString &level = QString(),
                   const QDateTime &startTime = QDateTime(), 
                   const QDateTime &endTime = QDateTime(),
                   const QString &search = QString());
    void createLog(const QString &level, const QString &message, 
                   const QString &source = QString());
    void checkHealth();

    void startLiveUpdates(int intervalSeconds = 5);
    void stopLiveUpdates();
    void testConnection();

signals:
    void liveMetricsReceived(const QJsonObject &metrics);
    void metricsHistoryReceived(const QJsonObject &response);
    void logsReceived(const QJsonObject &response);
    void logCreated(const QJsonObject &log);
    void healthReceived(const QJsonObject &health);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);
    void onAutoRefresh();

private:
    QNetworkAccessManager *m_networkManager;
    QString m_baseUrl;
    QTimer *m_refreshTimer;
    QMap<QNetworkReply*, QString> m_pendingRequests;

    void handleResponse(QNetworkReply *reply, const QString &requestType);
    QUrl buildUrl(const QString &endpoint, const QMap<QString, QString> &params = {});
};

#endif
