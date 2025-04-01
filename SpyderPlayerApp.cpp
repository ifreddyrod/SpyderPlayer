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
    // Setup Timers
    //-----------------------------
    inactivityTimer_ = new QTimer(this);
    inactivityTimer_->setInterval(5000);
    connect(inactivityTimer_, &QTimer::timeout, this, &SpyderPlayerApp::InactivityDetected);

    stalledVideoTimer_ = new QTimer(this);
    stalledVideoTimer_->setInterval(5000);
    
    playbackStatusTimer_ = new QTimer(this);
    int interval = appData_->PlayerType_ == ENUM_PLAYER_TYPE::QTMEDIA ? 3000 : 10000;
    playbackStatusTimer_->setInterval(interval);
    connect(playbackStatusTimer_, &QTimer::timeout, this, &SpyderPlayerApp::UpdatePlaybackStatus);

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

//***************************************************************************************
// GUI Functions
//***************************************************************************************
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

void SpyderPlayerApp::ChangePlayingUIStates(bool isPlaying)
{
    if (isPlaying)
    {
        controlpanelFS_.ui_.Play_button->setIcon(QIcon(":icons/icons/pause.png"));
        controlpanel_.ui_.Play_button->setIcon(QIcon(":icons/icons/pause.png"));
    }
    else 
    {
        controlpanelFS_.ui_.Play_button->setIcon(QIcon(":icons/icons/play.png"));
        controlpanel_.ui_.Play_button->setIcon(QIcon(":icons/icons/play.png"));
    }

    controlpanelFS_.ui_.VideoPosition_slider->setEnabled(videoChangesPosition_);
    controlpanel_.ui_.VideoPosition_slider->setEnabled(videoChangesPosition_);
}

void SpyderPlayerApp::PlayerDurationChanged(qint64 duration)
{
    QString videoLength = Format_ms_to_Time(duration);
    
    videoDuration_ = duration;

    if (duration == 0)
    {
        controlpanelFS_.ui_.Forward_button->setEnabled(false);
        controlpanelFS_.ui_.Backward_button->setEnabled(false);
        controlpanelFS_.ui_.CurrentTime_label->setText("00:00:00");
        controlpanelFS_.ui_.TotalDuration_label->setText("00:00:00");
        controlpanel_.ui_.Forward_button->setEnabled(false);
        controlpanel_.ui_.Backward_button->setEnabled(false);
        controlpanel_.ui_.CurrentTime_label->setText("00:00:00");
        controlpanel_.ui_.TotalDuration_label->setText("00:00:00");
        videoChangesPosition_ = false;
    }
    else
    {
        controlpanelFS_.ui_.Forward_button->setEnabled(true);
        controlpanelFS_.ui_.Backward_button->setEnabled(true);
        controlpanel_.ui_.Forward_button->setEnabled(true);
        controlpanel_.ui_.Backward_button->setEnabled(true);
        controlpanel_.ui_.TotalDuration_label->setText(videoLength);
        controlpanelFS_.ui_.TotalDuration_label->setText(videoLength);
        controlpanelFS_.ui_.VideoPosition_slider->setRange(0, duration);
        controlpanel_.ui_.VideoPosition_slider->setRange(0, duration);
        videoChangesPosition_ = true;
    }
}

void SpyderPlayerApp::VideoTimePositionChanged(qint64 position)
{
    videoPosition_ = position;

    if (videoChangesPosition_)
    {
        QString videoTime = Format_ms_to_Time(position);
        controlpanelFS_.ui_.CurrentTime_label->setText(videoTime);
        controlpanel_.ui_.CurrentTime_label->setText(videoTime);
        controlpanelFS_.ui_.VideoPosition_slider->setValue(position);
        controlpanel_.ui_.VideoPosition_slider->setValue(position);
    }
}

void SpyderPlayerApp::ShowVideoResolution()
{
    QString resolution = player_->GetVideoResolution();

    ui_.Status_label->setText("Video Resolution: " + resolution);
    playbackStatusTimer_->stop();
}


//*****************************************************************************************
// Playback Controls
//*****************************************************************************************
void SpyderPlayerApp::PlaySelectedChannel(string channelName, string source)
{
    if (channelName.empty() || source.empty())
    {
        PRINT << "Channel name or source is empty";
        return;
    }

    PRINT << "PlaySelectedChannel: " << channelName;
    PRINT << "Source: " << source;

    player_->Stop();
    player_->SetVideoSource(source);
    setWindowTitle("SPYDER Player - " + QSTR(channelName));
    player_->Play();
    //ChangePlayingUIStates(true);
}

void SpyderPlayerApp::PlayPausePlayer()
{
    ENUM_PLAYER_STATE state = player_->GetPlayerState();

    if (state == ENUM_PLAYER_STATE::PLAYING)
        player_->Pause();
    else
    {
        // Check if video reached end, if so stop and restart at beginning
        if (player_->GetPlayerState() == ENUM_PLAYER_STATE::ENDED)
        {
            videoPosition_ = 0;
            player_->Stop();
            player_->Play();
        }
        else
            player_->Play();
    }
}

void SpyderPlayerApp::StopPlayer()
{
    player_->Stop();
    ui_.Status_label->setText("");
}

void SpyderPlayerApp::SkipForward()
{
    int duration = player_->GetVideoDuration();
    if (duration> 0)
        player_->SetPosition(duration + 10000);
}

void SpyderPlayerApp::SkipBackward()
{
    int duration = player_->GetVideoDuration();
    if (duration> 0)
        player_->SetPosition(duration - 10000);
}

void SpyderPlayerApp::PlayNextChannel()
{
    QPair<string, string> nextChannel = playlistManager_->GetAdjacentChannel(true);

    PlaySelectedChannel(nextChannel.first, nextChannel.second);
}

void SpyderPlayerApp::PlayPreviousChannel()
{
    QPair<string, string> prevChannel = playlistManager_->GetAdjacentChannel(false);

    PlaySelectedChannel(prevChannel.first, prevChannel.second);
}

void SpyderPlayerApp::PlayLastChannel()
{
    QPair<string, string> lastChannel = playlistManager_->GotoLastSelectedChannel();

    PlaySelectedChannel(lastChannel.first, lastChannel.second);
}

//*****************************************************************************************
// Volume Controls
//*****************************************************************************************
void SpyderPlayerApp::MutePlayer()
{
    int volume = player_->GetVolume();

    if (player_->IsMuted())
    {
        UpdateVolumeSlider(volume);
        controlpanelFS_.ui_.Volume_slider->setEnabled(true);
        controlpanelFS_.ui_.FullVolume_button->setEnabled(true);
        controlpanel_.ui_.Volume_slider->setEnabled(true);
        controlpanel_.ui_.FullVolume_button->setEnabled(true);
        player_->Mute(false);
    }
    else
    {
        UpdateVolumeSlider(0);
        controlpanelFS_.ui_.Volume_slider->setEnabled(false);
        controlpanelFS_.ui_.FullVolume_button->setEnabled(false);
        controlpanel_.ui_.Volume_slider->setEnabled(false);
        controlpanel_.ui_.FullVolume_button->setEnabled(false);
        player_->Mute(true);
    }
}

void SpyderPlayerApp::UpdateVolumeSlider(int volume)
{
    controlpanelFS_.ui_.Volume_slider->setValue(volume);
    controlpanel_.ui_.Volume_slider->setValue(volume);
}

void SpyderPlayerApp::FullVolumePlayer()
{
    player_->SetVolume(100);
    UpdateVolumeSlider(100);
}

void SpyderPlayerApp::ChangeVolume()
{
    QSlider *slider = dynamic_cast<QSlider *>(sender());
    int volume = slider->value();
    PRINT << "Volume: " << volume;

    player_->SetVolume(volume);
    UpdateVolumeSlider(volume);
    player_->Mute(false);
}

void SpyderPlayerApp::IncreaseVolume()
{
    int volume = player_->GetVolume();
    volume = volume + 10;
    if (volume > 100)
        volume = 100;

    player_->SetVolume(volume);
    UpdateVolumeSlider(volume);
}

void SpyderPlayerApp::DecreaseVolume()
{
    int volume = player_->GetVolume();
    volume = volume - 10;
    if (volume < 0)
        volume = 0;

    player_->SetVolume(volume);
    UpdateVolumeSlider(volume);
}

//*****************************************************************************************
// Slider Functions
//*****************************************************************************************
void SpyderPlayerApp::OnPositionSliderPressed()
{
    bool isVideoPlaying = player_->GetPlayerState() == ENUM_PLAYER_STATE::PLAYING;

    player_->OnChangingPosition(isVideoPlaying);
    if (isFullScreen_)
    {
        inactivityTimer_->stop();
        controlpanelFS_.setFocus();
    }
}

//***************************************************************************************** 
// Utility Functions
//*****************************************************************************************
void SpyderPlayerApp::InactivityDetected()
{
    /*if (isFullScreen_ && !controlpanelFS_.hasFocus() && !subtitlesMenu_.isVisible())
    {
        controlpanelFS_.hide();
        overlay_.setFocus();
                
        if (!playListVisible_ && !settingsManager_.settingStack.isVisible())
            QApplication::setOverrideCursor(Qt::BlankCursor);
    }*/
    /*
    
            if self.isFullScreen and not self.controlPanelFS.hasFocus() and not self.subtitlesMenu.isVisible():
            self.controlPanelFS.hide()
            self.overlay.setFocus()
                      
            if not self.playListVisible and not self.settingsManager.settingStack.isVisible():
                QApplication.setOverrideCursor(Qt.CursorShape.BlankCursor)
                
                
                */
}

void SpyderPlayerApp::UpdatePlaybackStatus()
{
    ENUM_PLAYER_STATE state = player_->GetPlayerState();

    if (state == ENUM_PLAYER_STATE::PLAYING)
    {
        ShowVideoResolution();
        playbackStatusTimer_->stop();
    }
}
