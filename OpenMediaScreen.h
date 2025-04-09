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
    explicit OpenMedia(SettingsManager* settingsManager, bool loadPlaylist = false);
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
    bool loadPlaylist_ = false;
    bool modified_ = false;

};

#endif // OPENMEDIASCREEN_H