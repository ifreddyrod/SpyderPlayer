#include "SpyderPlayerApp.h"
#include "Global.h"

#include <QFont>
#include <QEvent>
#include <QKeyEvent>

// Constructor
SpyderPlayerApp::SpyderPlayerApp(QWidget *parent): QWidget(parent)
{
    version_ =  APP_VERSION;
    const string appdataFilename = "appdata.json";

    mousePressPos_ =  NULL;
    mouseMoveActive_ = false;
    isFullScreen_ = false;
    platform_ = GetPlatform();
    
    //-----------------------------
    // Load Splash Screen
    //-----------------------------
    splashscreen_.SetAppVersion(version_);    
    splashscreen_.SetText("Initializing ..."); 

    //-----------------------------
    // Load Screensaver Inhibitor
    //-----------------------------
    
    
    //---------------------------------
    // Load and Setup the Main Window
    //---------------------------------
    ui_.setupUi(this);

    setWindowOpacity(0);

    //-----------------------------
    // Load AppData from file
    //-----------------------------
    string data_path = GetUserAppDataDirectory(platform_, "SpyderPlayerTest") + "/" + appdataFilename; 

    PRINT << "AppData Path: " << data_path;

    appData_ = new AppData(data_path);

    //-----------------------------
    // Setup Playlist Manager
    //-----------------------------
    playlistManager_ = new PlaylistManager(ui_.PlayList_tree, appData_, this);

    //-----------------------------
    // Setup Control Panels
    //-----------------------------
    controlpanelFS_.setWindowOpacity(0.8);
    controlpanelFS_.hide();
    controlpanelFS_.installEventFilter(this);
    //ui.Bottom_Widget->layout = controlPanelFS;

    //-----------------------------
    // Setup Layout
    //-----------------------------
    ui_.botomverticalLayout->addWidget(&controlpanel_);
    controlpanel_.show();
    ui_.Horizontal_splitter->setSizes({400, 1000});
    ui_.Vertical_splitter->setSizes({800, 1});
    ui_.Horizontal_splitter->installEventFilter(this);
    ui_.Vertical_splitter->installEventFilter(this);
    isPlaylistVisible_ = true;

    ui_.ShowControlPanel_top_label->installEventFilter(this);
    ui_.ShowControlPanel_top_label->setMouseTracking(true);
    ui_.ShowControlPanel_bottom_label->installEventFilter(this);
    ui_.ShowControlPanel_bottom_label->setMouseTracking(true);
    ui_.ShowControlPanel_right_label->installEventFilter(this);
    ui_.ShowControlPanel_right_label->setMouseTracking(true);
    ui_.ShowControlPanel_left_label->installEventFilter(this);
    ui_.ShowControlPanel_left_label->setMouseTracking(true);

    //-----------------------------
    // Init Video Player
    //-----------------------------
    InitPlayer();

    //-----------------------------
    // Setup Video Overlay
    //-----------------------------

    //-----------------------------
    // Connect signals
    //-----------------------------
    // Double Click Channel
    connect(playlistManager_, &PlaylistManager::SIGNAL_PlaySelectedChannel, this, &SpyderPlayerApp::PlaySelectedChannel);


    //-----------------------------
    // Size the Fonts
    //-----------------------------
    // Get the current font
    QFont font = this->font();

    // Get the font family and size
    QString fontFamily = font.family();
    int fontSize = font.pointSize();
    PRINT << "Window Font: " << fontFamily << " " << fontSize;

    // Check if the point size is smaller than 11 and increase it if necessary
    if (fontSize < 11) 
    {
        this->setStyleSheet("color: white; font: 11pt;");
    }

    InitializePlayLists();

    ui_.Status_label->setText("Player: " + QSTR(PlayerTypeToString(appData_->PlayerType_)));
    /*splashScreen->show();

    ui->Status_label->setText(version);*/
}

// Destructor
SpyderPlayerApp::~SpyderPlayerApp()
{
    delete appData_;
    delete playlistManager_;
    delete player_;
}


void SpyderPlayerApp::InitializePlayLists()
{
    splashscreen_.show();
    splashscreen_.StartTimer();
    splashscreen_.UpdateStatus("Loading Playlists:", 1000);
    
    playlistManager_->ResetAllLists();

    for(const auto& playlist: appData_->PlayLists_)
    {
        PRINT << "Loading playlist: " << playlist->name;

        splashscreen_.UpdateStatus("Loading " + QSTR(playlist->name) + " ....");
        splashscreen_.UpdateStatus("Loading " + QSTR(playlist->name)  + " ....");
        playlistManager_->LoadPlayList(*playlist);
    }
    // Load Library Playlist
    splashscreen_.UpdateStatus("Loading Library ....", 1000);
    playlistManager_->LoadLibrary();

    // Load Favorites last to verify that items in other playlists have been loaded
    splashscreen_.UpdateStatus("Loading Favorites ....", 1000);
    playlistManager_->LoadFavorites();
    
    splashscreen_.UpdateStatus("Initialization Complete", 1000);

    // Show Main Window
    splashscreen_.hide();
    this->setWindowOpacity(1.0);
}

void SpyderPlayerApp::InitPlayer()
{
    if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::QTMEDIA)
    {
        player_ = new QtPlayer(&ui_, this);
    }
    /*else if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::VLC)
    {
        player_ = new VLCPlayer(this);
    }*/
}
// Event Handlers
bool SpyderPlayerApp::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);

        // User Activity Detected

        if (!ui_.Query_input->hasFocus()) // Handle Events when Query Input is not focused
        {
            if (keyEvent->key() == appData_->HotKeys_.collapseAllLists)
            {
                if (isPlaylistVisible_)
                    playlistManager_->CollapseAllPlaylists();
            }
            else if (keyEvent->key() == appData_->HotKeys_.gotoTopofList)
            {
                if (isPlaylistVisible_)
                    playlistManager_->GotoTopOfList();
            }
            else if (keyEvent->key() == appData_->HotKeys_.gotoBottomofList)
            {
                if (isPlaylistVisible_)
                    playlistManager_->GotoBottomOfList();
            }
            else if (keyEvent->key() == appData_->HotKeys_.sortListAscending)
            {
                if (isPlaylistVisible_)
                    playlistManager_->SortPlaylistAscending();
            }
            else if (keyEvent->key() == appData_->HotKeys_.sortListDescending)
            {
                if (isPlaylistVisible_)
                    playlistManager_->SortPlaylistDescending();
            }
            else if (keyEvent->key() == appData_->HotKeys_.togglePlaylist)
            {
                TogglePlaylistView();
            }
            else if(keyEvent->key() == appData_->HotKeys_.stopVideo)
            {
                player_->Stop();
            }
            return true;
        }
        else // Handle Events when Query Input is focused
        {
            if (keyEvent->key() == Qt::Key::Key_Return) 
            {
                // Search the Channels for Query
                QString query = ui_.Query_input->text();
                playlistManager_->SearchChannels(query);
            }
            else if(keyEvent->key() == Qt::Key::Key_Escape)
            {
                ui_.Query_input->clear();
            }
            return true;
        }
    }
    else
    {
        return QObject::eventFilter(object, event);
    }
}

void SpyderPlayerApp::TogglePlaylistView()
{
    if (isPlaylistVisible_)
    {
        ui_.Horizontal_splitter->setSizes({0, 1000});  // Hide left side
        ui_.Horizontal_splitter->setHandleWidth(1);
        isPlaylistVisible_ = false;
        //overlay_.setFocus();
        //overlay_.Resize(true);
    }
    else
    {
        ui_.Horizontal_splitter->setSizes({400, 1000});  // Show left side
        ui_.Horizontal_splitter->setHandleWidth(4);
        isPlaylistVisible_ = true;
        //overlay_.setFocus();
        //overlay_.Resize(false);
    }

    if (isFullScreen_)
    {
        ; //ShowControlPanel();
        //overlay_.Resize();
    }
}
void SpyderPlayerApp::PlaySelectedChannel(string channelName, string source)
{
    PRINT << "PlaySelectedChannel: " << channelName;
    PRINT << "Source: " << source;

    player_->Stop();
    player_->SetVideoSource(source);
    setWindowTitle("SPYDER Player - " + QSTR(channelName));
    player_->Play();

/*
        self.retryPlaying = True
        self.player.Stop()
        self.setWindowTitle("SPYDER Player - " + channel_name)
        self.currentSource = source
        self.player.SetVideoSource(source)
        self.player.Play()
        self.ChangePlayingUIStates(True)
        
*/
}
