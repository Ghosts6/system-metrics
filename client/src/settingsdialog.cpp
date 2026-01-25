#include "settingsdialog.h"
#include <QLabel>
#include <QDialogButtonBox>
#include <QSettings>
#include <QMessageBox>
#include "animatedbutton.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("Settings");
    setModal(true);
    resize(500, 350);
    
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    QLabel *titleLabel = new QLabel("Application Settings", this);
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #14a085; padding: 10px;");
    mainLayout->addWidget(titleLabel);

    // Connection settings
    QGroupBox *connectionGroup = new QGroupBox("Connection", this);
    connectionGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 2px solid #2d2d2d; border-radius: 5px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );
    QFormLayout *connectionLayout = new QFormLayout(connectionGroup);
    
    m_apiUrlEdit = new QLineEdit(this);
    m_apiUrlEdit->setPlaceholderText("http://localhost:8000");
    connectionLayout->addRow("API URL:", m_apiUrlEdit);
    
    m_autoConnectCheck = new QCheckBox("Auto-connect on startup", this);
    connectionLayout->addRow("", m_autoConnectCheck);
    
    mainLayout->addWidget(connectionGroup);

    // Display settings
    QGroupBox *displayGroup = new QGroupBox("Display", this);
    displayGroup->setStyleSheet(
        "QGroupBox { font-weight: bold; border: 2px solid #2d2d2d; border-radius: 5px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
    );
    QFormLayout *displayLayout = new QFormLayout(displayGroup);
    
    m_refreshIntervalSpin = new QSpinBox(this);
    m_refreshIntervalSpin->setRange(1, 3600);
    m_refreshIntervalSpin->setSuffix(" seconds");
    m_refreshIntervalSpin->setValue(5);
    displayLayout->addRow("Refresh Interval:", m_refreshIntervalSpin);
    
    m_showNotificationsCheck = new QCheckBox("Show notifications", this);
    displayLayout->addRow("", m_showNotificationsCheck);
    
    mainLayout->addWidget(displayGroup);

    mainLayout->addStretch();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    AnimatedButton *resetButton = new AnimatedButton("Reset to Defaults", this);
    connect(resetButton, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
        this
    );
    connect(buttonBox, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
            this, &SettingsDialog::onAccepted);
    
    buttonLayout->addWidget(resetButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(buttonBox);
    
    mainLayout->addLayout(buttonLayout);
}

void SettingsDialog::loadSettings()
{
    QSettings settings;
    
    m_originalApiUrl = settings.value("apiUrl", "http://localhost:8000").toString();
    m_originalRefreshInterval = settings.value("refreshInterval", 5).toInt();
    m_originalAutoConnect = settings.value("autoConnect", true).toBool();
    m_originalShowNotifications = settings.value("showNotifications", true).toBool();
    
    m_apiUrlEdit->setText(m_originalApiUrl);
    m_refreshIntervalSpin->setValue(m_originalRefreshInterval);
    m_autoConnectCheck->setChecked(m_originalAutoConnect);
    m_showNotificationsCheck->setChecked(m_originalShowNotifications);
}

void SettingsDialog::saveSettings()
{
    QSettings settings;
    
    settings.setValue("apiUrl", m_apiUrlEdit->text());
    settings.setValue("refreshInterval", m_refreshIntervalSpin->value());
    settings.setValue("autoConnect", m_autoConnectCheck->isChecked());
    settings.setValue("showNotifications", m_showNotificationsCheck->isChecked());
    
    settings.sync();
}

void SettingsDialog::onAccepted()
{
    saveSettings();
    emit settingsChanged();
}

void SettingsDialog::onResetClicked()
{
    int ret = QMessageBox::question(
        this,
        "Reset Settings",
        "Are you sure you want to reset all settings to defaults?",
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (ret == QMessageBox::Yes) {
        m_apiUrlEdit->setText("http://localhost:8000");
        m_refreshIntervalSpin->setValue(5);
        m_autoConnectCheck->setChecked(true);
        m_showNotificationsCheck->setChecked(true);
    }
}

QString SettingsDialog::apiUrl() const
{
    return m_apiUrlEdit->text();
}

void SettingsDialog::setApiUrl(const QString &url)
{
    m_apiUrlEdit->setText(url);
}

int SettingsDialog::refreshInterval() const
{
    return m_refreshIntervalSpin->value();
}

void SettingsDialog::setRefreshInterval(int seconds)
{
    m_refreshIntervalSpin->setValue(seconds);
}

bool SettingsDialog::autoConnect() const
{
    return m_autoConnectCheck->isChecked();
}

void SettingsDialog::setAutoConnect(bool enabled)
{
    m_autoConnectCheck->setChecked(enabled);
}

bool SettingsDialog::showNotifications() const
{
    return m_showNotificationsCheck->isChecked();
}

void SettingsDialog::setShowNotifications(bool enabled)
{
    m_showNotificationsCheck->setChecked(enabled);
}
