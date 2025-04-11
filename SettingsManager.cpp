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
    playlistEditor_ = new ListEditor(this, ENUM_SETTINGS_VIEWS::PLAYLIST);
    libraryEditor_ = new ListEditor(this, ENUM_SETTINGS_VIEWS::LIBRARY);
    favoritesEditor_ = new ListEditor(this, ENUM_SETTINGS_VIEWS::FAVORITES);
    playlistEntry_ = new EntryEditor(this, ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY);
    libraryEntry_ = new EntryEditor(this, ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY);
    favoritesEntry_ = new EntryEditor(this, ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY);
    openPlayList_ = new OpenMedia(this, ENUM_SETTINGS_VIEWS::OPEN_PLAYLIST);
    openFile_ = new OpenMedia(this, ENUM_SETTINGS_VIEWS::OPEN_FILE);
    appSettings_ = new AppSettings(this);
    hotKeySettings_ = new HotKeySettings(this);
    about_ = new AboutScreen(this);

    // Add Screens to settings stack
    settingsStack_->addWidget(mainScreen_);
    settingsStack_->addWidget(playlistEditor_);
    settingsStack_->addWidget(libraryEditor_);
    settingsStack_->addWidget(favoritesEditor_);
    settingsStack_->addWidget(playlistEntry_);
    settingsStack_->addWidget(libraryEntry_);
    settingsStack_->addWidget(favoritesEntry_);
    settingsStack_->addWidget(openPlayList_);
    settingsStack_->addWidget(openFile_);
    settingsStack_->addWidget(appSettings_);
    settingsStack_->addWidget(hotKeySettings_);
    settingsStack_->addWidget(about_);
    
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
    delete openFile_;
    delete openPlayList_;
    delete playlistEditor_;
    delete libraryEditor_;
    delete favoritesEditor_;
    delete playlistEntry_;
    delete libraryEntry_;
    delete favoritesEntry_;
}

void SettingsManager::HideSettingsScreen()
{
    settingsStack_->hide();

    if (changesMade_)
    {
        emit SIGNAL_ReLoadAllPlayLists();
        changesMade_ = false;
    }
}
void SettingsManager::ShowSettingsMainScreen(bool initialize)
{
    if (initialize)
        changesMade_ = false;

    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::MAIN);
    settingsStack_->show();
}

void SettingsManager::ShowPlaylistEditor(bool changesMade)
{
    changesMade_ |= changesMade;
    playlistEditor_->AddEditList(appData_->PlayLists_);
    playlistEditor_->ShowListEditor();
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::PLAYLIST);
}

void SettingsManager::ShowLibraryEditor(bool changesMade)
{
    changesMade_ |= changesMade;
    libraryEditor_->AddEditList(appData_->Library_);
    libraryEditor_->ShowListEditor();
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::LIBRARY);
}

void SettingsManager::ShowFavoritesEditor(bool changesMade)
{
    changesMade_ |= changesMade;
    favoritesEditor_->AddEditList(appData_->Favorites_);
    favoritesEditor_->ShowListEditor();
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::FAVORITES);
}

void SettingsManager::ShowPlayListEntryEditor(int index)
{
    playlistEntry_->ShowEntryEditor(index);
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY);
}

void SettingsManager::ShowLibraryEntryEditor(int index)
{
    libraryEntry_->ShowEntryEditor(index);
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY);
}

void SettingsManager::ShowFavoritesEntryEditor(int index)
{
    favoritesEntry_->ShowEntryEditor(index);
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY);
}

void SettingsManager::ShowOpenPlayListScreen()
{
    openPlayList_->ShowOpenMediaScreen();
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::OPEN_PLAYLIST);
}

void SettingsManager::ShowOpenFileScreen()
{
    openFile_->ShowOpenMediaScreen();
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::OPEN_FILE);
}

void SettingsManager::ShowAppSettings()
{
    appSettings_->ShowAppSettings();
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::APPSETTINGS);
}

void SettingsManager::ShowHotKeySettings()
{
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::HOTKEYS);
}

void SettingsManager::ShowAboutScreen()
{
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::ABOUT);
}


void SettingsManager::LoadMediaFile(PlayListEntry entry)
{
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::MAIN);
    settingsStack_->hide();
    emit SIGNAL_LoadMediaFile(entry);
}

void SettingsManager::LoadPlayList(PlayListEntry entry)
{
    settingsStack_->setCurrentIndex(ENUM_SETTINGS_VIEWS::MAIN);
    settingsStack_->hide();
    emit SIGNAL_LoadPlayList(entry);    
}

void SettingsManager::SaveSettings(bool changesMade)
{
    changesMade_ |= changesMade;
    // Save settings to file
    appData_->Save();

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
    connect(ui_.OpenMediaFile_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowOpenFileScreen);
    connect(ui_.OpenPlayList_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowOpenPlayListScreen);
    connect(ui_.PlayList_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowPlaylistEditor);
    connect(ui_.Library_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowLibraryEditor);
    connect(ui_.Favorites_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowFavoritesEditor);

    //connect(ui_.PlayList_button, &QPushButton::clicked, settingsManager_, &SettingsManager::ShowSettingsMainScreen);
}
