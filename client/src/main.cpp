#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QStatusBar>
#include <QWidget>
#include <QGraphicsEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include "apiclient.h"
#include "metricswidget.h"
#include "logviewer.h"
#include "style.h"
#include "animatedbutton.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setStyleSheet(Style::getDarkGreenStyleSheet() + Style::getButtonStyle() + 
                     Style::getProgressBarStyle() + Style::getTableStyle());

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("System Metrics & Log Analytics");
    mainWindow.resize(1200, 800);

    QWidget *centralWidget = new QWidget(&mainWindow);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *connectionLayout = new QHBoxLayout();
    connectionLayout->setSpacing(10);
    connectionLayout->setContentsMargins(15, 15, 15, 15);
    
    QLabel *urlLabel = new QLabel("API URL:", &mainWindow);
    urlLabel->setStyleSheet("color: #e0e0e0; font-size: 13px; font-weight: bold;");
    QLineEdit *urlEdit = new QLineEdit("http://localhost:8000", &mainWindow);
    urlEdit->setStyleSheet(
        "QLineEdit { background-color: #2d2d2d; border: 2px solid #3d3d3d; border-radius: 4px; padding: 8px; color: #e0e0e0; font-size: 12px; }"
        "QLineEdit:focus { border: 2px solid #0d7377; }"
    );
    
    QLabel *connectionStatus = new QLabel("● Disconnected", &mainWindow);
    connectionStatus->setStyleSheet("color: #ff6b6b; font-size: 12px; font-weight: bold; padding: 5px;");
    
    AnimatedButton *connectButton = new AnimatedButton("Connect", &mainWindow);
    AnimatedButton *startLiveButton = new AnimatedButton("Start Live Updates", &mainWindow);
    AnimatedButton *stopLiveButton = new AnimatedButton("Stop Live Updates", &mainWindow);

    connectionLayout->addWidget(urlLabel);
    connectionLayout->addWidget(urlEdit);
    connectionLayout->addWidget(connectionStatus);
    connectionLayout->addWidget(connectButton);
    connectionLayout->addWidget(startLiveButton);
    connectionLayout->addWidget(stopLiveButton);
    connectionLayout->addStretch();

    mainLayout->addLayout(connectionLayout);

    ApiClient *apiClient = new ApiClient(&mainWindow);
    MetricsWidget *metricsWidget = new MetricsWidget(&mainWindow);
    LogViewer *logViewer = new LogViewer(&mainWindow);

    QTabWidget *tabWidget = new QTabWidget(&mainWindow);
    tabWidget->addTab(metricsWidget, "Live Metrics");
    tabWidget->addTab(logViewer, "Logs");

    mainLayout->addWidget(tabWidget);

    centralWidget->setLayout(mainLayout);
    mainWindow.setCentralWidget(centralWidget);

    QObject::connect(connectButton, &QPushButton::clicked, [=, &connectionStatus]() {
        apiClient->setBaseUrl(urlEdit->text());
        apiClient->testConnection();
    });

    QObject::connect(startLiveButton, &QPushButton::clicked, [&]() {
        apiClient->startLiveUpdates(5);
        mainWindow.statusBar()->showMessage("Live updates started (5s interval)", 2000);
    });

    QObject::connect(stopLiveButton, &QPushButton::clicked, [&]() {
        apiClient->stopLiveUpdates();
        mainWindow.statusBar()->showMessage("Live updates stopped", 2000);
    });
    
    // Auto-connect on URL change
    QObject::connect(urlEdit, &QLineEdit::editingFinished, [=, &connectionStatus]() {
        apiClient->setBaseUrl(urlEdit->text());
        apiClient->testConnection();
    });

    QObject::connect(apiClient, &ApiClient::liveMetricsReceived, metricsWidget, &MetricsWidget::updateMetrics);
    QObject::connect(apiClient, &ApiClient::healthReceived, [=, &mainWindow, &connectionStatus](const QJsonObject &health) {
        QString status = QString("Status: %1, Database: %2, Redis: %3")
                        .arg(health["status"].toString())
                        .arg(health["database"].toString())
                        .arg(health["redis"].toString());
        mainWindow.statusBar()->showMessage(status, 5000);
        
        connectionStatus->setText("● Connected");
        connectionStatus->setStyleSheet("color: #14a085; font-size: 12px; font-weight: bold; padding: 5px;");
        
        // Auto-fetch data on successful connection
        apiClient->fetchLiveMetrics();
        logViewer->refreshLogs();
    });
    QObject::connect(apiClient, &ApiClient::logsReceived, logViewer, &LogViewer::updateLogs);
    QObject::connect(apiClient, &ApiClient::errorOccurred, [=, &mainWindow, &connectionStatus](const QString &error) {
        connectionStatus->setText("● Disconnected");
        connectionStatus->setStyleSheet("color: #ff6b6b; font-size: 12px; font-weight: bold; padding: 5px;");
        
        // Only show error dialog for manual connections, not auto-connect failures
        if (error.contains("Network error")) {
            mainWindow.statusBar()->showMessage("Connection failed: " + error, 5000);
        } else {
            QMessageBox::warning(&mainWindow, "Error", error);
            mainWindow.statusBar()->showMessage("Error: " + error, 5000);
        }
    });
    QObject::connect(logViewer, &LogViewer::requestLogs, apiClient, &ApiClient::fetchLogs);

    mainWindow.statusBar()->showMessage("Ready - Click Connect to start");
    mainWindow.show();

    return app.exec();
}
