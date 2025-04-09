#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "Global.h"
#include "DraggableWidget.h"
#include "AppData.h"
#include "AppSettingsScreen.h"
#include "AboutScreen.h"
#include "HotKeyEditorScreen.h"
#include "OpenMediaScreen.h"

#include <QWidget>
#include <QStackedWidget>
#include <QList>

#include "ui_Settings.h"
#include "ui_PlayListSettings.h"
#include "ui_EntryEditor.h"


enum ENUM_SETTINGS_VIEWS
{
    MAIN = 0,
    PLAYLIST = 1,
    LIBRARY  = 2,
    FAVORITES = 3,
    PLAYLIST_ENTRY = 4,
    LIBRARY_ENTRY = 5,
    FAVORITES_ENTRY = 6,
    OPEN_PLAYLIST = 7,
    OPEN_FILE = 8,
    APPSETTINGS = 9,
    HOTKEYS = 10,
    ABOUT = 11
};

//*****************************************************************************************
// Settings Manager Class
//*****************************************************************************************
class SettingsManager : public QObject
{
    Q_OBJECT

public:
    SettingsManager(AppData* appData = nullptr);
    ~SettingsManager();
    void ShowSettingsMainScreen(bool initialize = false);
    void HideSettingsScreen();
    //void ShowPlaylistEditor(bool changesMade = false);
    //void ShowPlaylistEntryEditor(bool changesMade = false);
    //void ShowLibraryEditor(bool changesMade = false);
    //void ShowLibraryEntryEditor(bool changesMade = false);
    //void ShowFavoritesEditor(bool changesMade = false);
    //void ShowFavoritesEntryEditor(bool changesMade = false);
    void ShowOpenPlayListScreen();
    void LoadPlayList(PlayListEntry entry);
    void ShowOpenFileScreen();
    void LoadMediaFile(PlayListEntry entry);
    void ShowHotKeySettings();
    void ShowAppSettings();
    void ShowAboutScreen();
    void SaveSettings();
    bool IsVisible() { return settingsStack_->isVisible(); }
    
    AppData* appData_;
    bool changesMade_ = false;
signals:
    void SIGNAL_ReLoadAllPlayLists();
    void SIGNAL_LoadMediaFile(PlayListEntry);
    void SIGNAL_LoadPlayList(PlayListEntry);

private:

    QStackedWidget* settingsStack_;
    DraggableWidget* mainScreen_;
    DraggableWidget* playlistEditor_;
    DraggableWidget* libraryEditor_;
    DraggableWidget* favoritesEditor_;
    DraggableWidget* playlistEntry_;
    DraggableWidget* libraryEntry_;
    DraggableWidget* favoritesEntry_;
    OpenMedia* openFile_;
    OpenMedia* openPlayList_;
    AppSettings* appSettings_;
    HotKeySettings* hotKeySettings_;
    AboutScreen* about_;
};


//*****************************************************************************************
// Settings MainScreen Class
//*****************************************************************************************
class SettingsMain : public DraggableWidget
{
    Q_OBJECT
public:
    SettingsMain(SettingsManager* settingsManager);

private:
    Ui::SettingsMain ui_;
    SettingsManager* settingsManager_;

};



#endif // SETTINGSMANAGER_H
