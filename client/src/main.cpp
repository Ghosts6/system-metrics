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
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QTimer>
#include <QIcon>
#include <QPixmap>
#include <QWidgetAction>
#include <QFileDialog>
#include <QTextEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QComboBox>
#include <QFrame>
#include <QEventLoop>
#include "apiclient.h"
#include "metricswidget.h"
#include "logviewer.h"
#include "metricschartwidget.h"
#include "dashboardwidget.h"
#include "systeminfowidget.h"
#include "settingsdialog.h"
#include "style.h"
#include "animatedbutton.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("System Metrics Client");
    app.setOrganizationName("SystemMetrics");
    
    // Set application icon
    QIcon appIcon(":/logo.png");
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }
    
    app.setStyleSheet(Style::getDarkGreenStyleSheet() + Style::getButtonStyle() + 
                     Style::getProgressBarStyle() + Style::getTableStyle());

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("System Metrics & Log Analytics");
    mainWindow.setWindowIcon(appIcon);
    mainWindow.resize(1400, 900);

    // Load settings
    QSettings settings;
    QString defaultApiUrl = settings.value("apiUrl", "http://localhost:8000").toString();
    int defaultRefreshInterval = settings.value("refreshInterval", 5).toInt();

    QWidget *centralWidget = new QWidget(&mainWindow);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // -- Connection Bar --
    QFrame *connectionBar = new QFrame(&mainWindow);
    connectionBar->setObjectName("connectionBar");
    connectionBar->setStyleSheet(
        "#connectionBar { "
        "  background-color: #2a2a2a; "
        "  border-radius: 4px; "
        "  border: 1px solid #3d3d3d; "
        "}");
    
    QHBoxLayout *connectionLayout = new QHBoxLayout(connectionBar);
    connectionLayout->setSpacing(15);
    connectionLayout->setContentsMargins(15, 10, 15, 10);
    
    // Left side: API URL
    QLabel *urlLabel = new QLabel("API URL:", connectionBar);
    urlLabel->setStyleSheet("color: #b0b0b0; font-size: 13px; font-weight: bold;");
    
    QLineEdit *urlEdit = new QLineEdit(defaultApiUrl, connectionBar);
    urlEdit->setMinimumWidth(300);
    urlEdit->setStyleSheet(
        "QLineEdit { background-color: #3d3d3d; border: 1px solid #555; border-radius: 4px; padding: 8px; color: #e0e0e0; font-size: 12px; }"
        "QLineEdit:focus { border: 1px solid #14a085; }"
    );
    
    AnimatedButton *connectButton = new AnimatedButton("Connect", connectionBar);
    
    connectionLayout->addWidget(urlLabel);
    connectionLayout->addWidget(urlEdit);
    connectionLayout->addWidget(connectButton);
    
    // Spacer
    connectionLayout->addStretch(1);
    
    // Right side: Status and Controls
    QLabel *connectionStatus = new QLabel("● Disconnected", connectionBar);
    connectionStatus->setStyleSheet("color: #ff6b6b; font-size: 12px; font-weight: bold; padding: 5px;");
    
    AnimatedButton *startLiveButton = new AnimatedButton("Start Live", connectionBar);
    AnimatedButton *stopLiveButton = new AnimatedButton("Stop Live", connectionBar);
    stopLiveButton->setEnabled(false); // Initially disabled
    
    connectionLayout->addWidget(connectionStatus);
    connectionLayout->addWidget(startLiveButton);
    connectionLayout->addWidget(stopLiveButton);
    
    mainLayout->addWidget(connectionBar);

    // Create API client
    ApiClient *apiClient = new ApiClient(&mainWindow);
    apiClient->setBaseUrl(defaultApiUrl);

    // Create widgets
    DashboardWidget *dashboardWidget = new DashboardWidget(&mainWindow);
    MetricsWidget *metricsWidget = new MetricsWidget(&mainWindow);
    MetricsChartWidget *chartWidget = new MetricsChartWidget(&mainWindow);
    LogViewer *logViewer = new LogViewer(&mainWindow);
    SystemInfoWidget *systemInfoWidget = new SystemInfoWidget(&mainWindow);

    // Create tab widget
    QTabWidget *tabWidget = new QTabWidget(&mainWindow);
    tabWidget->setStyleSheet(
        "QTabWidget::pane {"
        "  border: 1px solid #3d3d3d;"
        "  border-top: 1px solid #3d3d3d;"
        "  background-color: #2a2a2a;"
        "}"
        "QTabBar::tab {"
        "  background-color: #2a2a2a;"
        "  color: #b0b0b0;"
        "  border: 1px solid #3d3d3d;"
        "  border-bottom: none;"
        "  border-top-left-radius: 4px;"
        "  border-top-right-radius: 4px;"
        "  padding: 10px 25px;"
        "  margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: #2a2a2a;"
        "  color: #14a085;"
        "  border: 1px solid #3d3d3d;"
        "  border-bottom: 2px solid #14a085;"
        "}"
        "QTabBar::tab:hover {"
        "  background-color: #353535;"
        "  color: #e0e0e0;"
        "}"
    );
    tabWidget->addTab(dashboardWidget, "Dashboard");
    tabWidget->addTab(metricsWidget, "Live Metrics");
    tabWidget->addTab(chartWidget, "Historical Charts");
    tabWidget->addTab(logViewer, "Logs");
    tabWidget->addTab(systemInfoWidget, "System Info");

    mainLayout->addWidget(tabWidget);

    centralWidget->setLayout(mainLayout);
    mainWindow.setCentralWidget(centralWidget);

    // Menu bar
    QMenuBar *menuBar = mainWindow.menuBar();
    
    // Add logo to menu bar
    QWidget *logoWidget = new QWidget(&mainWindow);
    QHBoxLayout *logoLayout = new QHBoxLayout(logoWidget);
    logoLayout->setContentsMargins(8, 4, 12, 4);
    logoLayout->setSpacing(8);
    
    QLabel *logoLabel = new QLabel(logoWidget);
    QPixmap logoPixmap(":/logo.png");
    if (!logoPixmap.isNull()) {
        // Scale logo to menu bar height (typically 20-24px)
        QPixmap scaledLogo = logoPixmap.scaled(20, 20, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        logoLabel->setPixmap(scaledLogo);
    }
    logoLabel->setStyleSheet("background-color: transparent;");
    
    QLabel *appNameLabel = new QLabel("System Metrics", logoWidget);
    appNameLabel->setStyleSheet("color: #14a085; font-weight: bold; font-size: 13px; background-color: transparent;");
    
    logoLayout->addWidget(logoLabel);
    logoLayout->addWidget(appNameLabel);
    logoLayout->addStretch();
    
    QWidgetAction *logoAction = new QWidgetAction(&mainWindow);
    logoAction->setDefaultWidget(logoWidget);
    menuBar->addAction(logoAction);
    
    // Add separator
    menuBar->addSeparator();
    
    QMenu *fileMenu = menuBar->addMenu("File");
    QAction *settingsAction = fileMenu->addAction("Settings...");
    fileMenu->addSeparator();
    QAction *exportAction = fileMenu->addAction("Export Data...");
    QAction *clearCacheAction = fileMenu->addAction("Clear Cache");
    fileMenu->addSeparator();
    QAction *exitAction = fileMenu->addAction("Exit");
    
    QMenu *viewMenu = menuBar->addMenu("View");
    QMenu *themeMenu = viewMenu->addMenu("Theme");
    QAction *lightThemeAction = themeMenu->addAction("Light Theme");
    QAction *darkThemeAction = themeMenu->addAction("Dark Theme");
    darkThemeAction->setCheckable(true);
    darkThemeAction->setChecked(true);
    viewMenu->addSeparator();
    QAction *refreshAction = viewMenu->addAction("Refresh All");

    QMenu *toolsMenu = menuBar->addMenu("Tools");
    QAction *pingAction = toolsMenu->addAction("Ping Host");
    QAction *tracerouteAction = toolsMenu->addAction("Traceroute Host");
    
    QMenu *helpMenu = menuBar->addMenu("Help");
    QAction *aboutAction = helpMenu->addAction("About");

    // Export Data functionality
    QObject::connect(exportAction, &QAction::triggered, [=, &mainWindow]() {
        QDialog exportDialog(&mainWindow);
        exportDialog.setWindowTitle("Export Data");
        exportDialog.setMinimumWidth(400);
        
        QVBoxLayout *layout = new QVBoxLayout(&exportDialog);
        
        QLabel *formatLabel = new QLabel("Export Format:", &exportDialog);
        QComboBox *formatCombo = new QComboBox(&exportDialog);
        formatCombo->addItem("JSON", "json");
        formatCombo->addItem("Excel (CSV)", "csv");
        
        QLabel *dataLabel = new QLabel("Data Type:", &exportDialog);
        QComboBox *dataCombo = new QComboBox(&exportDialog);
        dataCombo->addItem("Live Metrics", "live");
        dataCombo->addItem("Historical Metrics", "history");
        dataCombo->addItem("Logs", "logs");
        
        layout->addWidget(formatLabel);
        layout->addWidget(formatCombo);
        layout->addWidget(dataLabel);
        layout->addWidget(dataCombo);
        
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &exportDialog);
        layout->addWidget(buttons);
        
        QObject::connect(buttons, &QDialogButtonBox::accepted, [=, &exportDialog]() {
            QString format = formatCombo->currentData().toString();
            QString dataType = dataCombo->currentData().toString();
            
            QString filter = format == "json" ? "JSON Files (*.json)" : "CSV Files (*.csv)";
            QString extension = format == "json" ? ".json" : ".csv";
            QString defaultName = QString("metrics_%1_%2%3")
                .arg(dataType)
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"))
                .arg(extension);
            
            QString fileName = QFileDialog::getSaveFileName(&exportDialog, "Save Export", 
                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + defaultName, filter);
            
            if (fileName.isEmpty()) return;
            
            QJsonObject exportData;
            bool success = false;
            
            if (dataType == "live") {
                // Fetch current live metrics
                QEventLoop loop;
                QJsonObject liveData;
                QMetaObject::Connection conn = QObject::connect(apiClient, &ApiClient::liveMetricsReceived, [&](const QJsonObject &data) {
                    liveData = data;
                    loop.quit();
                });
                apiClient->fetchLiveMetrics();
                QTimer::singleShot(10000, &loop, &QEventLoop::quit); // 10 second timeout
                loop.exec();
                QObject::disconnect(conn);
                exportData = liveData;
                success = !liveData.isEmpty();
            } else if (dataType == "history") {
                // Fetch historical metrics
                QEventLoop loop;
                QJsonObject historyData;
                QMetaObject::Connection conn = QObject::connect(apiClient, &ApiClient::metricsHistoryReceived, [&](const QJsonObject &data) {
                    historyData = data;
                    loop.quit();
                });
                QDateTime endTime = QDateTime::currentDateTime();
                QDateTime startTime = endTime.addDays(-7);
                apiClient->fetchMetricsHistory(1, 1000, startTime, endTime);
                QTimer::singleShot(10000, &loop, &QEventLoop::quit); // 10 second timeout
                loop.exec();
                QObject::disconnect(conn);
                exportData = historyData;
                success = !historyData.isEmpty();
            } else if (dataType == "logs") {
                // Fetch logs
                QEventLoop loop;
                QJsonObject logsData;
                QMetaObject::Connection conn = QObject::connect(apiClient, &ApiClient::logsReceived, [&](const QJsonObject &data) {
                    logsData = data;
                    loop.quit();
                });
                apiClient->fetchLogs(1, 1000);
                QTimer::singleShot(10000, &loop, &QEventLoop::quit); // 10 second timeout
                loop.exec();
                QObject::disconnect(conn);
                exportData = logsData;
                success = !logsData.isEmpty();
            }
            
            if (!success || exportData.isEmpty()) {
                QMessageBox::warning(&exportDialog, "Export Failed", "Failed to fetch data for export.");
                return;
            }
            
            QFile file(fileName);
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QMessageBox::warning(&exportDialog, "Export Failed", "Cannot write to file: " + fileName);
                return;
            }
            
            QTextStream out(&file);
            
            if (format == "json") {
                QJsonDocument doc(exportData);
                out << doc.toJson();
            } else {
                // CSV export
                if (exportData.contains("data") && exportData["data"].isArray()) {
                    QJsonArray dataArray = exportData["data"].toArray();
                    if (!dataArray.isEmpty()) {
                        QJsonObject firstItem = dataArray[0].toObject();
                        QStringList headers = firstItem.keys();
                        out << headers.join(",") << "\n";
                        
                        for (const QJsonValue &value : dataArray) {
                            QJsonObject obj = value.toObject();
                            QStringList row;
                            for (const QString &key : headers) {
                                QString val = obj[key].toVariant().toString();
                                val.replace(",", ";");
                                val.replace("\n", " ");
                                row << "\"" + val + "\"";
                            }
                            out << row.join(",") << "\n";
                        }
                    }
                } else {
                    // Single object export
                    QStringList keys = exportData.keys();
                    out << keys.join(",") << "\n";
                    QStringList values;
                    for (const QString &key : keys) {
                        QString val = exportData[key].toVariant().toString();
                        val.replace(",", ";");
                        val.replace("\n", " ");
                        values << "\"" + val + "\"";
                    }
                    out << values.join(",") << "\n";
                }
            }
            
            file.close();
            QMessageBox::information(&exportDialog, "Export Complete", "Data exported successfully to:\n" + fileName);
            exportDialog.accept();
        });
        
        QObject::connect(buttons, &QDialogButtonBox::rejected, &exportDialog, &QDialog::reject);
        
        exportDialog.exec();
    });
    
    // Clear Cache functionality
    QObject::connect(clearCacheAction, &QAction::triggered, [=, &mainWindow]() {
        int ret = QMessageBox::question(&mainWindow, "Clear Cache", 
            "This will clear all cached data from widgets. Continue?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (ret == QMessageBox::Yes) {
            dashboardWidget->clearMetrics();
            metricsWidget->clearMetrics();
            chartWidget->clearChart();
            logViewer->clearLogs();
            systemInfoWidget->clearInfo();
            mainWindow.statusBar()->showMessage("Cache cleared", 2000);
        }
    });
    
    // Theme switching
    QObject::connect(lightThemeAction, &QAction::triggered, [&mainWindow, &settings, lightThemeAction, darkThemeAction]() {
        QString lightStyle = Style::getLightStyleSheet() + Style::getButtonStyle() + 
                             Style::getProgressBarStyle() + Style::getTableStyle();
        qApp->setStyleSheet(lightStyle);
        lightThemeAction->setChecked(true);
        darkThemeAction->setChecked(false);
        settings.setValue("theme", "light");
        mainWindow.statusBar()->showMessage("Light theme applied", 2000);
    });
    
    QObject::connect(darkThemeAction, &QAction::triggered, [&mainWindow, &settings, lightThemeAction, darkThemeAction]() {
        QString darkStyle = Style::getDarkGreenStyleSheet() + Style::getButtonStyle() + 
                           Style::getProgressBarStyle() + Style::getTableStyle();
        qApp->setStyleSheet(darkStyle);
        darkThemeAction->setChecked(true);
        lightThemeAction->setChecked(false);
        settings.setValue("theme", "dark");
        mainWindow.statusBar()->showMessage("Dark theme applied", 2000);
    });
    
    // Load saved theme
    QString savedTheme = settings.value("theme", "dark").toString();
    if (savedTheme == "light") {
        lightThemeAction->trigger();
    } else {
        darkThemeAction->trigger();
    }
    
    // Ping tool
    QObject::connect(pingAction, &QAction::triggered, [=, &mainWindow]() {
        QDialog pingDialog(&mainWindow);
        pingDialog.setWindowTitle("Ping Host");
        pingDialog.setMinimumSize(600, 400);
        
        QVBoxLayout *layout = new QVBoxLayout(&pingDialog);
        
        QHBoxLayout *inputLayout = new QHBoxLayout();
        QLabel *hostLabel = new QLabel("Host:", &pingDialog);
        QLineEdit *hostEdit = new QLineEdit("google.com", &pingDialog);
        QPushButton *pingButton = new QPushButton("Ping", &pingDialog);
        
        inputLayout->addWidget(hostLabel);
        inputLayout->addWidget(hostEdit);
        inputLayout->addWidget(pingButton);
        
        QTextEdit *outputText = new QTextEdit(&pingDialog);
        outputText->setReadOnly(true);
        outputText->setFont(QFont("Courier", 10));
        
        layout->addLayout(inputLayout);
        layout->addWidget(new QLabel("Output:", &pingDialog));
        layout->addWidget(outputText);
        
        QProcess *pingProcess = new QProcess(&pingDialog);
        
        QObject::connect(pingButton, &QPushButton::clicked, [hostEdit, outputText, pingButton, pingProcess]() {
            QString host = hostEdit->text().trimmed();
            if (host.isEmpty()) {
                QMessageBox::warning(hostEdit->parentWidget(), "Invalid Input", "Please enter a hostname or IP address.");
                return;
            }
            
            outputText->clear();
            outputText->append("Pinging " + host + "...\n");
            pingButton->setEnabled(false);
            
            #ifdef Q_OS_WIN
            pingProcess->start("ping", QStringList() << "-n" << "4" << host);
            #else
            pingProcess->start("ping", QStringList() << "-c" << "4" << host);
            #endif
        });
        
        QObject::connect(pingProcess, &QProcess::readyReadStandardOutput, [outputText, pingProcess]() {
            outputText->append(QString::fromUtf8(pingProcess->readAllStandardOutput()));
        });
        
        QObject::connect(pingProcess, &QProcess::readyReadStandardError, [outputText, pingProcess]() {
            outputText->append(QString::fromUtf8(pingProcess->readAllStandardError()));
        });
        
        QObject::connect(pingProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [outputText, pingButton](int exitCode, QProcess::ExitStatus) {
                if (pingButton && outputText) {
                    pingButton->setEnabled(true);
                    if (exitCode == 0) {
                        outputText->append("\n✓ Ping completed successfully");
                    } else {
                        outputText->append("\n✗ Ping failed (exit code: " + QString::number(exitCode) + ")");
                    }
                }
            });
        
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &pingDialog);
        layout->addWidget(buttons);
        QObject::connect(buttons, &QDialogButtonBox::rejected, [pingProcess, &pingDialog]() {
            if (pingProcess->state() != QProcess::NotRunning) {
                pingProcess->kill();
                pingProcess->waitForFinished(1000);
            }
            pingDialog.reject();
        });
        
        pingDialog.exec();
    });
    
    // Traceroute tool
    QObject::connect(tracerouteAction, &QAction::triggered, [=, &mainWindow]() {
        QDialog traceDialog(&mainWindow);
        traceDialog.setWindowTitle("Traceroute Host");
        traceDialog.setMinimumSize(600, 500);
        
        QVBoxLayout *layout = new QVBoxLayout(&traceDialog);
        
        QHBoxLayout *inputLayout = new QHBoxLayout();
        QLabel *hostLabel = new QLabel("Host:", &traceDialog);
        QLineEdit *hostEdit = new QLineEdit("google.com", &traceDialog);
        QPushButton *traceButton = new QPushButton("Traceroute", &traceDialog);
        
        inputLayout->addWidget(hostLabel);
        inputLayout->addWidget(hostEdit);
        inputLayout->addWidget(traceButton);
        
        QTextEdit *outputText = new QTextEdit(&traceDialog);
        outputText->setReadOnly(true);
        outputText->setFont(QFont("Courier", 9));
        
        layout->addLayout(inputLayout);
        layout->addWidget(new QLabel("Output:", &traceDialog));
        layout->addWidget(outputText);
        
        QProcess *traceProcess = new QProcess(&traceDialog);
        
        QObject::connect(traceButton, &QPushButton::clicked, [hostEdit, outputText, traceButton, traceProcess]() {
            QString host = hostEdit->text().trimmed();
            if (host.isEmpty()) {
                QMessageBox::warning(hostEdit->parentWidget(), "Invalid Input", "Please enter a hostname or IP address.");
                return;
            }
            
            outputText->clear();
            outputText->append("Tracing route to " + host + "...\n");
            traceButton->setEnabled(false);
            
            #ifdef Q_OS_WIN
            traceProcess->start("tracert", QStringList() << host);
            #else
            traceProcess->start("traceroute", QStringList() << host);
            #endif
        });
        
        QObject::connect(traceProcess, &QProcess::readyReadStandardOutput, [outputText, traceProcess]() {
            outputText->append(QString::fromUtf8(traceProcess->readAllStandardOutput()));
        });
        
        QObject::connect(traceProcess, &QProcess::readyReadStandardError, [outputText, traceProcess]() {
            outputText->append(QString::fromUtf8(traceProcess->readAllStandardError()));
        });
        
        QObject::connect(traceProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), 
            [outputText, traceButton](int exitCode, QProcess::ExitStatus) {
                if (traceButton && outputText) {
                    traceButton->setEnabled(true);
                    if (exitCode == 0) {
                        outputText->append("\n✓ Traceroute completed");
                    } else {
                        outputText->append("\n✗ Traceroute failed (exit code: " + QString::number(exitCode) + ")");
                    }
                }
            });
        
        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &traceDialog);
        layout->addWidget(buttons);
        QObject::connect(buttons, &QDialogButtonBox::rejected, [traceProcess, &traceDialog]() {
            if (traceProcess->state() != QProcess::NotRunning) {
                traceProcess->kill();
                traceProcess->waitForFinished(1000);
            }
            traceDialog.reject();
        });
        
        traceDialog.exec();
    });
    
    // Settings dialog
    SettingsDialog *settingsDialog = new SettingsDialog(&mainWindow);
    
    QObject::connect(settingsAction, &QAction::triggered, [=]() {
        settingsDialog->loadSettings();
        if (settingsDialog->exec() == QDialog::Accepted) {
            urlEdit->setText(settingsDialog->apiUrl());
            apiClient->setBaseUrl(settingsDialog->apiUrl());
            if (settingsDialog->autoConnect()) {
                apiClient->testConnection();
            }
        }
    });

    QObject::connect(exitAction, &QAction::triggered, &mainWindow, &QMainWindow::close);
    
    QObject::connect(refreshAction, &QAction::triggered, [=]() {
        apiClient->fetchLiveMetrics();
        logViewer->refreshLogs();
        chartWidget->refreshChart();
    });

    QObject::connect(aboutAction, &QAction::triggered, [&mainWindow]() {
        QMessageBox aboutBox(&mainWindow);
        aboutBox.setWindowTitle("About System Metrics");
        aboutBox.setIconPixmap(QPixmap(":/logo.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        aboutBox.setText(
            "<h2>System Metrics & Log Analytics</h2>"
            "<p>A cross-platform system monitoring application.</p>"
            "<p><b>Built with:</b><br>"
            "• Qt6 (C++)<br>"
            "• FastAPI (Python)<br>"
            "• Redis<br>"
            "• PostgreSQL/SQLite</p>"
            "<p>Version 1.0.0</p>"
        );
        aboutBox.exec();
    });

    // Connections
    QObject::connect(connectButton, &QPushButton::clicked, [=, &connectionStatus]() {
        apiClient->setBaseUrl(urlEdit->text());
        apiClient->testConnection();
    });

    QObject::connect(startLiveButton, &QPushButton::clicked, [=, &mainWindow]() {
        int interval = settingsDialog->refreshInterval();
        apiClient->startLiveUpdates(interval);
        mainWindow.statusBar()->showMessage(QString("Live updates started (%1s interval)").arg(interval), 2000);
        
        startLiveButton->setEnabled(false);
        stopLiveButton->setEnabled(true);
        connectButton->setEnabled(false);
        urlEdit->setEnabled(false);
    });

    QObject::connect(stopLiveButton, &QPushButton::clicked, [=, &mainWindow]() {
        apiClient->stopLiveUpdates();
        mainWindow.statusBar()->showMessage("Live updates stopped", 2000);
        
        startLiveButton->setEnabled(true);
        stopLiveButton->setEnabled(false);
        connectButton->setEnabled(true);
        urlEdit->setEnabled(true);
    });
    
    // Auto-connect on URL change
    QObject::connect(urlEdit, &QLineEdit::editingFinished, [=, &connectionStatus]() {
        apiClient->setBaseUrl(urlEdit->text());
        apiClient->testConnection();
    });

    // API signals
    QObject::connect(apiClient, &ApiClient::liveMetricsReceived, dashboardWidget, &DashboardWidget::updateMetrics);
    QObject::connect(apiClient, &ApiClient::liveMetricsReceived, metricsWidget, &MetricsWidget::updateMetrics);
    QObject::connect(apiClient, &ApiClient::liveMetricsReceived, systemInfoWidget, &SystemInfoWidget::updateSystemInfo);
    
    QObject::connect(apiClient, &ApiClient::metricsHistoryReceived, chartWidget, &MetricsChartWidget::updateHistoryData);
    QObject::connect(chartWidget, &MetricsChartWidget::requestHistory, apiClient, [=](const QDateTime &start, const QDateTime &end) {
        apiClient->fetchMetricsHistory(1, 1000, start, end);
    });
    
    QObject::connect(apiClient, &ApiClient::healthReceived, [&](const QJsonObject &health) {
        QString status = QString("Status: %1, Database: %2, Redis: %3")
                        .arg(health["status"].toString())
                        .arg(health["database"].toString())
                        .arg(health["redis"].toString());
        mainWindow.statusBar()->showMessage(status, 5000);
        
        connectionStatus->setText("● Connected");
        connectionStatus->setStyleSheet("color: #14a085; font-size: 12px; font-weight: bold; padding: 5px;");
        
        connectButton->setEnabled(true);
        startLiveButton->setEnabled(true);
        
        // Auto-fetch data on successful connection
        apiClient->fetchLiveMetrics();
        logViewer->refreshLogs();
        
        // Automatically start live updates on connect
        if (settings.value("autoLiveUpdates", true).toBool()) {
            startLiveButton->click();
        }
    });
    
    QObject::connect(apiClient, &ApiClient::logsReceived, logViewer, &LogViewer::updateLogs);
    QObject::connect(logViewer, &LogViewer::requestLogs, apiClient, &ApiClient::fetchLogs);
    QObject::connect(logViewer, &LogViewer::requestCreateLog, apiClient, &ApiClient::createLog);
    
    QObject::connect(apiClient, &ApiClient::errorOccurred, [=, &mainWindow, &connectionStatus](const QString &error) {
        connectionStatus->setText("● Disconnected");
        connectionStatus->setStyleSheet("color: #ff6b6b; font-size: 12px; font-weight: bold; padding: 5px;");
        
        apiClient->stopLiveUpdates();
        
        startLiveButton->setEnabled(false);
        stopLiveButton->setEnabled(false);
        connectButton->setEnabled(true);
        urlEdit->setEnabled(true);
        
        // Only show error dialog for manual connections, not auto-connect failures
        if (error.contains("Network error")) {
            mainWindow.statusBar()->showMessage("Connection failed: " + error, 5000);
        } else {
            QMessageBox::warning(&mainWindow, "Error", error);
            mainWindow.statusBar()->showMessage("Error: " + error, 5000);
        }
    });

    // Auto-connect if enabled
    if (settings.value("autoConnect", true).toBool()) {
        QTimer::singleShot(500, [=]() {
            apiClient->testConnection();
        });
    }

    mainWindow.statusBar()->showMessage("Ready - Click Connect to start");
    mainWindow.show();

    return app.exec();
}
