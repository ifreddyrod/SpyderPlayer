#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include "Global.h"
#include "DraggableWidget.h"
#include "AppData.h"
#include "AppSettingsScreen.h"
#include "AboutScreen.h"
#include "HotKeyEditorScreen.h"
#include "OpenMediaScreen.h"
#include "ListEditorScreen.h"
#include "EntryEditorScreen.h"
#include "ui_Settings.h"
#include <QWidget>
#include <QStackedWidget>
#include <QList>


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
    void ShowPlaylistEditor(bool changesMade = false);
    void ShowLibraryEditor(bool changesMade = false);
    void ShowFavoritesEditor(bool changesMade = false);
    void ShowPlayListEntryEditor(int index = -1);
    void ShowLibraryEntryEditor(int index = -1);
    void ShowFavoritesEntryEditor(int index = -1);
    void ShowOpenPlayListScreen();
    void LoadPlayList(PlayListEntry entry);
    void ShowOpenFileScreen();
    void LoadMediaFile(PlayListEntry entry);
    void ShowHotKeySettings();
    void ShowAppSettings();
    void ShowAboutScreen();
    void SaveSettings(bool changesMade = false);
    bool IsVisible() { return settingsStack_->isVisible(); }
    
    AppData* appData_;
    
signals:
    void SIGNAL_ReLoadAllPlayLists();
    void SIGNAL_LoadMediaFile(PlayListEntry);
    void SIGNAL_LoadPlayList(PlayListEntry);

private:

    QStackedWidget* settingsStack_;
    DraggableWidget* mainScreen_;
    ListEditor* playlistEditor_;
    ListEditor* libraryEditor_;
    ListEditor* favoritesEditor_;
    EntryEditor* playlistEntry_;
    EntryEditor* libraryEntry_;
    EntryEditor* favoritesEntry_;
    OpenMedia* openFile_;
    OpenMedia* openPlayList_;
    AppSettings* appSettings_;
    HotKeySettings* hotKeySettings_;
    AboutScreen* about_;
    bool changesMade_ = false;
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
