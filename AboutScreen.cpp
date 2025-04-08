#include "AboutScreen.h"
#include "SettingsManager.h"

//*****************************************************************************************
// About Screen Class
//*****************************************************************************************
AboutScreen::AboutScreen(SettingsManager* settingsManager)
{
    ui_.setupUi(this);

    settingsManager_ = settingsManager;

    // Display App Version
    ui_.Version_label->setText("Version: " + QSTR(APP_VERSION));

    // Connect Slots
    connect(ui_.Back_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowSettingsMainScreen);
}
