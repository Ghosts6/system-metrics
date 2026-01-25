#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

/**
 * @brief Dialog for application settings and configuration
 * 
 * Allows users to configure API URL, refresh intervals, and other preferences.
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QString apiUrl() const;
    void setApiUrl(const QString &url);
    
    int refreshInterval() const;
    void setRefreshInterval(int seconds);
    
    bool autoConnect() const;
    void setAutoConnect(bool enabled);
    
    bool showNotifications() const;
    void setShowNotifications(bool enabled);
    
    void loadSettings();

signals:
    void settingsChanged();

private slots:
    void onAccepted();
    void onResetClicked();

private:
    void setupUI();
    void saveSettings();
    
    QLineEdit *m_apiUrlEdit;
    QSpinBox *m_refreshIntervalSpin;
    QCheckBox *m_autoConnectCheck;
    QCheckBox *m_showNotificationsCheck;
    
    QString m_originalApiUrl;
    int m_originalRefreshInterval;
    bool m_originalAutoConnect;
    bool m_originalShowNotifications;
};

#endif // SETTINGSDIALOG_H
