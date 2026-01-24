#include "apiclient.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QDateTime>

ApiClient::ApiClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_baseUrl("http://localhost:8000")
    , m_refreshTimer(new QTimer(this))
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &ApiClient::onReplyFinished);
    connect(m_refreshTimer, &QTimer::timeout, this, &ApiClient::onAutoRefresh);
    
    // Auto-connect on startup
    QTimer::singleShot(500, this, [this]() {
        testConnection();
    });
}

ApiClient::~ApiClient()
{
}

void ApiClient::setBaseUrl(const QString &url)
{
    m_baseUrl = url;
    if (!m_baseUrl.endsWith("/")) {
        m_baseUrl += "/";
    }
}

void ApiClient::fetchLiveMetrics()
{
    QUrl url = buildUrl("api/v1/metrics/live");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingRequests[reply] = "liveMetrics";
}

void ApiClient::fetchMetricsHistory(int page, int pageSize, 
                                    const QDateTime &startTime, 
                                    const QDateTime &endTime)
{
    QMap<QString, QString> params;
    params["page"] = QString::number(page);
    params["page_size"] = QString::number(pageSize);
    
    if (startTime.isValid()) {
        params["start_time"] = startTime.toUTC().toString(Qt::ISODate);
    }
    if (endTime.isValid()) {
        params["end_time"] = endTime.toUTC().toString(Qt::ISODate);
    }

    QUrl url = buildUrl("api/v1/metrics/history", params);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingRequests[reply] = "metricsHistory";
}

void ApiClient::fetchLogs(int page, int pageSize, const QString &level,
                          const QDateTime &startTime, const QDateTime &endTime)
{
    QMap<QString, QString> params;
    params["page"] = QString::number(page);
    params["page_size"] = QString::number(pageSize);
    
    if (!level.isEmpty()) {
        params["level"] = level;
    }
    if (startTime.isValid()) {
        params["start_time"] = startTime.toUTC().toString(Qt::ISODate);
    }
    if (endTime.isValid()) {
        params["end_time"] = endTime.toUTC().toString(Qt::ISODate);
    }

    QUrl url = buildUrl("api/v1/logs/", params);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingRequests[reply] = "logs";
}

void ApiClient::createLog(const QString &level, const QString &message, const QString &source)
{
    QUrl url = buildUrl("api/v1/logs/");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject logData;
    logData["level"] = level;
    logData["message"] = message;
    if (!source.isEmpty()) {
        logData["source"] = source;
    }

    QJsonDocument doc(logData);
    QNetworkReply *reply = m_networkManager->post(request, doc.toJson());
    m_pendingRequests[reply] = "createLog";
}

void ApiClient::checkHealth()
{
    QUrl url = buildUrl("api/v1/health");
    QNetworkRequest request(url);

    QNetworkReply *reply = m_networkManager->get(request);
    m_pendingRequests[reply] = "health";
}

    void ApiClient::startLiveUpdates(int intervalSeconds)
    {
        m_refreshTimer->start(intervalSeconds * 1000);
        fetchLiveMetrics();
    }

    void ApiClient::stopLiveUpdates()
    {
        m_refreshTimer->stop();
    }

    void ApiClient::testConnection()
    {
        checkHealth();
    }

void ApiClient::onReplyFinished(QNetworkReply *reply)
{
    if (!m_pendingRequests.contains(reply)) {
        reply->deleteLater();
        return;
    }

    QString requestType = m_pendingRequests.take(reply);
    handleResponse(reply, requestType);
    reply->deleteLater();
}

void ApiClient::onAutoRefresh()
{
    fetchLiveMetrics();
}

void ApiClient::handleResponse(QNetworkReply *reply, const QString &requestType)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(QString("Network error: %1").arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("JSON parse error: %1").arg(error.errorString()));
        return;
    }

    if (requestType == "liveMetrics") {
        emit liveMetricsReceived(doc.object());
    } else if (requestType == "metricsHistory") {
        emit metricsHistoryReceived(doc.object());
    } else if (requestType == "logs") {
        emit logsReceived(doc.object());
    } else if (requestType == "createLog") {
        emit logCreated(doc.object());
    } else if (requestType == "health") {
        emit healthReceived(doc.object());
    }
}

QUrl ApiClient::buildUrl(const QString &endpoint, const QMap<QString, QString> &params)
{
    QString urlString = m_baseUrl;
    if (!urlString.endsWith("/")) {
        urlString += "/";
    }
    urlString += endpoint;

    QUrl url(urlString);
    if (!params.isEmpty()) {
        QUrlQuery query;
        for (auto it = params.begin(); it != params.end(); ++it) {
            query.addQueryItem(it.key(), it.value());
        }
        url.setQuery(query);
    }

    return url;
}
