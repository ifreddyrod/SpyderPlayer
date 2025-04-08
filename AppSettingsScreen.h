#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include "Global.h"
#include "DraggableWidget.h"
#include "AppData.h"
#include "ui_PlayerSettings.h"

class SettingsManager;

//*****************************************************************************************
// Application Settings Class
//***************************************************************************************** 
class AppSettings : public DraggableWidget
{
    Q_OBJECT
public:
    AppSettings(SettingsManager* settingsManager);
    void ShowAppSettings();

private:
    void BackButtonClicked();
    void PlayerTypeChanged();
    void PathChanged();
    void OpenPathButtonClicked();
    void SaveButtonClicked();

    Ui::PlayerSettings ui_;
    SettingsManager* settingsManager_;
    bool modified_ = false;
};

#endif // APPSETTINGS_H