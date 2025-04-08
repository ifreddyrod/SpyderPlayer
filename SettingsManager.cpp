#include "SettingsManager.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>

//*****************************************************************************************
// SettingManager Class 
//*****************************************************************************************

// In SettingsManager constructor:
SettingsManager::SettingsManager(AppData* appData)
{
    appData_ = appData;

    // Create a standalone QStackedWidget 
    settingsStack_ = new QStackedWidget();
    
    // Initialize Screen Objects
    mainScreen_ = new SettingsMain(this); 
    about_ = new AboutScreen(this);
    appSettings_ = new AppSettings(this);
    hotKeySettings_ = new HotKeySettings(this);

    // Add Screens to settings stack
    settingsStack_->addWidget(mainScreen_);
    settingsStack_->addWidget(about_);
    settingsStack_->addWidget(appSettings_);
    settingsStack_->addWidget(hotKeySettings_);

    // Setup Stack 
    settingsStack_->setFixedWidth(780);
    settingsStack_->setFixedHeight(430);
    settingsStack_->setWindowFlags(Qt::FramelessWindowHint);
    settingsStack_->setWindowModality(Qt::ApplicationModal);
}

SettingsManager::~SettingsManager()
{
    delete mainScreen_;
    delete about_;
    delete appSettings_;
    delete hotKeySettings_;
    delete settingsStack_;
}


void SettingsManager::ShowSettingsMainScreen(bool initialize)
{
    if (initialize)
        changesMade_ = false;

    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::MAIN);
    settingsStack_->show();
}

void SettingsManager::HideSettingsScreen()
{
    if (changesMade_)
    {
        emit SIGNAL_ReLoadAllPlayLists();
        changesMade_ = false;
    }
    settingsStack_->hide();
    
}

void SettingsManager::ShowAboutScreen()
{
    //settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::ABOUT);
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::MAIN + 1);
}

void SettingsManager::ShowAppSettings()
{
    //static_cast<AppSettings*>(appSettings_)->ShowAppSettings();
    appSettings_->ShowAppSettings();
    //settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::APPSETTINGS);
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::MAIN + 2);
}
void SettingsManager::ShowHotKeySettings()
{
    //settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::HOTKEYS);
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::MAIN + 3);
}

void SettingsManager::SaveSettings()
{
    if (changesMade_)
    {
        // Save settings to file
        appData_->Save();
        changesMade_ = false;
    }
}

//*****************************************************************************************
// Settings MainScreen Class
//*****************************************************************************************
SettingsMain::SettingsMain(SettingsManager *settingsManager)
{
    settingsManager_ = settingsManager;

    ui_.setupUi(this);

    // Connect Slots
    connect(ui_.Close_button, &QPushButton::clicked, settingsManager_, &SettingsManager::HideSettingsScreen);
    connect(ui_.About_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowAboutScreen);
    connect(ui_.PlayerSettings_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowAppSettings);
    connect(ui_.HotKeys_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowHotKeySettings);

    //connect(ui_.PlayList_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowSettingsMainScreen);
}
