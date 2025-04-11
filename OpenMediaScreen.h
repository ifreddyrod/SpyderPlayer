#ifndef OPENMEDIASCREEN_H
#define OPENMEDIASCREEN_H

#include "Global.h"
#include "DraggableWidget.h"
#include "AppData.h"
#include "ui_OpenFileSelection.h"

class SettingsManager;


class OpenMedia : public DraggableWidget
{
    Q_OBJECT    
public:
    explicit OpenMedia(SettingsManager* settingsManager, ENUM_SETTINGS_VIEWS viewType);
    ~OpenMedia();
    void ShowOpenMediaScreen();

private:
    void BackButtonClicked();
    void OpenButtonClicked();
    void OpenFileButtonClicked();
    void SourceTypeChanged();
    void EntryChanged();


    Ui::OpenFileSelection ui_;
    SettingsManager* settingsManager_;
    bool modified_ = false;
    ENUM_SETTINGS_VIEWS viewType_;

};

#endif // OPENMEDIASCREEN_H