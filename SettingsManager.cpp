#include "SettingsManager.h"
#include <QApplication>

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

    // Add Screens to settings stack
    settingsStack_->addWidget(mainScreen_);

    // Setup Stack 
    settingsStack_->setFixedWidth(780);
    settingsStack_->setFixedHeight(430);
    settingsStack_->setWindowFlags(Qt::FramelessWindowHint);
    settingsStack_->setWindowModality(Qt::ApplicationModal);
}

SettingsManager::~SettingsManager()
{
    delete mainScreen_;
    
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

void SettingsManager::SaveSettings()
{
    if (changesMade_)
    {
        // Save settings to file
        appData_->Save();
        changesMade_ = false;
    }
}


SettingsMain::SettingsMain(SettingsManager *settingsManager)
{
    settingsManager_ = settingsManager;

    ui_.setupUi(this);

    // Connect Slots
    connect(ui_.Close_button, &QPushButton::clicked, settingsManager_, &SettingsManager::HideSettingsScreen);
    //connect(ui_.PlayList_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowSettingsMainScreen);
}

