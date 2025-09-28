#include "SpyderPlayerApp.h"
#include "Global.h"

#include <QFont>
#include <QEvent>
#include <QKeyEvent>

// Constructor
SpyderPlayerApp::SpyderPlayerApp(QWidget *parent, AppData *appData): QWidget(parent), appData_(appData)
{
    version_ =  APP_VERSION;
    //const string appdataFilename = "appdata.json";

    mousePressPos_ =  QPoint();
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
    screensaverInhibitor_ = new ScreensaverInhibitor();
    screensaverInhibitor_->uninhibit();

    //---------------------------------
    // Load and Setup the Main Window
    //---------------------------------
    ui_.setupUi(this);
    setWindowOpacity(0);
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    // Resize to 80%
    int screenWidth = screenGeometry.width() * 0.75;
    int screenHeight = screenGeometry.height() * 0.9;
    resize(screenWidth, screenHeight);

    // Center the window
    int x = (screenGeometry.width() - screenWidth) / 2;
    int y = (screenGeometry.height() - screenHeight) / 3;
    move(x, y);

    //-----------------------------
    // Load AppData from file
    //-----------------------------
    /* data_path = GetUserAppDataDirectory("SpyderPlayer") + "/" + appdataFilename; 

    PRINT << "AppData Path: " << data_path;

    appData_ = new AppData(data_path);*/

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
    ui_.bottomverticalLayout->addWidget(&controlpanel_);
    //ui_.ControlPanel = &controlpanel_;
    controlpanel_.hide();
    controlpanel_.show();
    ui_.Horizontal_splitter->setSizes({450, 1000});
    ui_.Vertical_splitter->setSizes({800, 1});
    ui_.Horizontal_splitter->installEventFilter(this);
    ui_.Vertical_splitter->installEventFilter(this);
    isPlaylistVisible_ = true;

    /*ui_.ShowControlPanel_top_label->installEventFilter(this);
    ui_.ShowControlPanel_top_label->setMouseTracking(true);
    ui_.ShowControlPanel_bottom_label->installEventFilter(this);
    ui_.ShowControlPanel_bottom_label->setMouseTracking(true);
    ui_.ShowControlPanel_right_label->installEventFilter(this);
    ui_.ShowControlPanel_right_label->setMouseTracking(true);
    ui_.ShowControlPanel_left_label->installEventFilter(this);
    ui_.ShowControlPanel_left_label->setMouseTracking(true);*/

    //-----------------------------
    // Setup Video Overlay
    //-----------------------------
    overlay_ = new VideoOverlay(this);
    overlay_->SetAppObject(this);
    overlay_->SetOverlayVisible(appData_->PlayerType_ == ENUM_PLAYER_TYPE::VLC);

    //-----------------------------
    // Init Video Player
    //-----------------------------
    //InitPlayer();

    //-----------------------------
    // Setup Settings Manager
    //-----------------------------
    settingsManager_ = new SettingsManager(appData_);

    //-----------------------------
    // Connect signals
    //-----------------------------
    // Double Click Channel
    connect(playlistManager_, &PlaylistManager::SIGNAL_PlaySelectedChannel, this, &SpyderPlayerApp::PlaySelectedChannel);
    
    connect(ui_.Search_button, &QPushButton::clicked, this, &SpyderPlayerApp::SearchChannels);

    connect(ui_.Horizontal_splitter, &QSplitter::splitterMoved, this, &SpyderPlayerApp::OnHSplitterResized);

    connect(ui_.Settings_button, &QPushButton::clicked, this, &SpyderPlayerApp::ShowSettings);

    // Connect Controller Buttons 
    connect(controlpanel_.ui_.Play_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayPausePlayer);
    connect(controlpanelFS_.ui_.Play_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayPausePlayer);

    connect(controlpanel_.ui_.Stop_button, &QPushButton::clicked, this, &SpyderPlayerApp::StopPlayer);
    connect(controlpanelFS_.ui_.Stop_button, &QPushButton::clicked, this, &SpyderPlayerApp::StopPlayer);

    connect(controlpanel_.ui_.Next_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayNextChannel);
    connect(controlpanelFS_.ui_.Next_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayNextChannel);

    connect(controlpanel_.ui_.Previous_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayPreviousChannel);
    connect(controlpanelFS_.ui_.Previous_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayPreviousChannel);

    connect(controlpanel_.ui_.Forward_button, &QPushButton::clicked, this, &SpyderPlayerApp::SeekForward);
    connect(controlpanelFS_.ui_.Forward_button, &QPushButton::clicked, this, &SpyderPlayerApp::SeekForward);

    connect(controlpanel_.ui_.Backward_button, &QPushButton::clicked, this, &SpyderPlayerApp::SeekBackward);
    connect(controlpanelFS_.ui_.Backward_button, &QPushButton::clicked, this, &SpyderPlayerApp::SeekBackward);

    connect(controlpanel_.ui_.Last_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayLastChannel);
    connect(controlpanelFS_.ui_.Last_button, &QPushButton::clicked, this, &SpyderPlayerApp::PlayLastChannel);

    connect(controlpanel_.ui_.Mute_button, &QPushButton::clicked, this, &SpyderPlayerApp::MutePlayer);
    connect(controlpanelFS_.ui_.Mute_button, &QPushButton::clicked, this, &SpyderPlayerApp::MutePlayer);

    connect(controlpanel_.ui_.FullVolume_button, &QPushButton::clicked, this, &SpyderPlayerApp::FullVolumePlayer);
    connect(controlpanelFS_.ui_.FullVolume_button, &QPushButton::clicked, this, &SpyderPlayerApp::FullVolumePlayer);

    connect(controlpanel_.ui_.Volume_slider, &QSlider::valueChanged, this, &SpyderPlayerApp::ChangeVolume);
    connect(controlpanelFS_.ui_.Volume_slider, &QSlider::valueChanged, this, &SpyderPlayerApp::ChangeVolume);

    connect(controlpanel_.ui_.ToggleList_button, &QPushButton::clicked, this, &SpyderPlayerApp::TogglePlaylistView);
    connect(controlpanelFS_.ui_.ToggleList_button, &QPushButton::clicked, this, &SpyderPlayerApp::TogglePlaylistView);

    connect(controlpanel_.ui_.CloseCaption_button, &QPushButton::clicked, this, &SpyderPlayerApp::ShowSubtitleTracks);
    connect(controlpanelFS_.ui_.CloseCaption_button, &QPushButton::clicked, this, &SpyderPlayerApp::ShowSubtitleTracks);

    //controlpanel_.ui_.Volume_slider->setValue(100);
    //controlpanelFS_.ui_.Volume_slider->setValue(100); 
    //player_->SetVolume(100);

    connect(controlpanel_.ui_.VideoPosition_slider, &QSlider::sliderPressed, this, &SpyderPlayerApp::OnPositionSliderPressed);
    connect(controlpanelFS_.ui_.VideoPosition_slider, &QSlider::sliderPressed, this, &SpyderPlayerApp::OnPositionSliderPressed);
    connect(controlpanel_.ui_.VideoPosition_slider, &QSlider::sliderMoved, this, &SpyderPlayerApp::OnPositionSliderMoved);
    connect(controlpanelFS_.ui_.VideoPosition_slider, &QSlider::sliderMoved, this, &SpyderPlayerApp::OnPositionSliderMoved);
    connect(controlpanel_.ui_.VideoPosition_slider, &QSlider::sliderReleased, this, &SpyderPlayerApp::OnPositionSliderReleased);
    connect(controlpanelFS_.ui_.VideoPosition_slider, &QSlider::sliderReleased, this, &SpyderPlayerApp::OnPositionSliderReleased);

    controlpanel_.ui_.VideoPosition_slider->setEnabled(false);
    controlpanelFS_.ui_.VideoPosition_slider->setEnabled(false);
    controlpanel_.ui_.CloseCaption_button->setEnabled(false);
    controlpanelFS_.ui_.CloseCaption_button->setEnabled(false);

    // Connect Player Signals
    /*connect(player_, &VideoPlayer::SIGNAL_UpdatePosition, this, &SpyderPlayerApp::PlayerPositionChanged);
    connect(player_, &VideoPlayer::SIGNAL_ErrorOccured, this, &SpyderPlayerApp::PlayerErrorOccured);
    connect(player_, &VideoPlayer::SIGNAL_PlayerStateChanged, this, &SpyderPlayerApp::PlaybackStateChanged);
    connect(player_, &VideoPlayer::SIGNAL_EnableSubtitles, this, &SpyderPlayerApp::EnableSubtitles);
    player_->SetVolume(100);*/

    // Connect Settings Signals
    connect(settingsManager_, &SettingsManager::SIGNAL_ReLoadAllPlayLists, this, &SpyderPlayerApp::InitializePlayLists);
    connect(settingsManager_, &SettingsManager::SIGNAL_LoadPlayList, this, &SpyderPlayerApp::LoadSessionPlaylist);
    connect(settingsManager_, &SettingsManager::SIGNAL_LoadMediaFile, this, &SpyderPlayerApp::LoadSessionMedia);

    //-----------------------------,
    // Setup Timers
    //-----------------------------
    inactivityTimer_ = new QTimer(this);
    inactivityTimer_->setInterval(5000);
    connect(inactivityTimer_, &QTimer::timeout, this, &SpyderPlayerApp::InactivityDetected);

    stalledVideoTimer_ = new QTimer(this);
    stalledVideoTimer_->setInterval(5000);
    connect(stalledVideoTimer_, &QTimer::timeout, this, &SpyderPlayerApp::StalledVideoDetected);
    
    playbackStatusTimer_ = new QTimer(this);
    int interval = appData_->PlayerType_ == ENUM_PLAYER_TYPE::QTMEDIA ? 3000 : 10000;
    playbackStatusTimer_->setInterval(interval);
    connect(playbackStatusTimer_, &QTimer::timeout, this, &SpyderPlayerApp::UpdatePlaybackStatus);

    //-----------------------------
    // Setup Drop Down Menus
    //-----------------------------
    subtitlesMenu_ = new QMenu(this);
    subtitlesMenu_->connect(subtitlesMenu_, &QMenu::triggered, this, &SpyderPlayerApp::SelectSubtitleTrack);
    
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

    //ui_.Status_label->setText("Player: " + QSTR(PlayerTypeToString(appData_->PlayerType_)));
    ui_.Status_label->setText("Player Ready ... ");

    installEventFilter(this);
}

// Destructor
SpyderPlayerApp::~SpyderPlayerApp()
{
    screensaverInhibitor_->uninhibit();
    delete appData_;
    delete playlistManager_;
    delete player_;
    delete screensaverInhibitor_;
}

//***********************************************************************************
// Initialization Functions
//***********************************************************************************
void SpyderPlayerApp::InitializePlayLists()
{
    isInitializing_ = true;
    splashscreen_.show();
    splashscreen_.StartTimer();
    splashscreen_.UpdateStatus("Loading Playlists:", 250);
    
    playlistManager_->ResetAllLists();

    for(const auto& playlist: appData_->PlayLists_)
    {
        splashscreen_.UpdateStatus("Loading " + QSTR(playlist.name) + " ....");
        //splashscreen_.UpdateStatus("Loading " + QSTR(playlist.name)  + " ....");
        playlistManager_->LoadPlayList(playlist);
    }
    // Load Library Playlist
    splashscreen_.UpdateStatus("Loading Library ....");
    playlistManager_->LoadLibrary();

    // Load Favorites last to verify that items in other playlists have been loaded
    splashscreen_.UpdateStatus("Loading Favorites ....");
    playlistManager_->LoadFavorites();
    
    // Load Session Playlists
    splashscreen_.UpdateStatus("Loading Session Playlists ....");
    playlistManager_->LoadSessionPlayLists();

    // Load Session Media
    splashscreen_.UpdateStatus("Loading Session Media ....");
    playlistManager_->LoadSessionMedia();

    // Collapse Playlist Tree
    playlistManager_->CollapseAllPlaylists();
    
    splashscreen_.UpdateStatus("Initialization Complete", 500);

    // Show Main Window
    splashscreen_.hide();
    this->setWindowOpacity(1.0);
    isInitializing_ = false;

    overlay_->Show();

    if (player_ == nullptr)
        InitPlayer();

    OnHSplitterResized(0, 0);

    overlay_->Resize();
    //overlay_->activateWindow();
    //if(!isFullScreen_)
        //##overlay_->hide();
    //PRINT << "Initial Windowstate = " << windowState();
}

void SpyderPlayerApp::InitPlayer()
{
    // Only 1 Player Type available at this time
    //player_ = new QtPlayer(&ui_, this);

    
    if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::QTMEDIA)
    {
        player_ = new QtPlayer(&ui_, this);
    }
    /*else if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::FFMPEG)
    {
        //player_ = new FFmpegPlayer(&ui_, this);
        player_ = new QtAvPlayer(&ui_, this);
    }*/
    else if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::VLC)
    {
        player_ = new VLCPlayer(&ui_, this);
        overlay_->Show();
        overlay_->videoPanel_->show();
        overlay_->Resize();        
        player_->InitPlayer(overlay_->videoPanel_);

    }

    // Connect Playback Signals
    connect(player_, &VideoPlayer::SIGNAL_UpdatePosition, this, &SpyderPlayerApp::PlayerPositionChanged);
    connect(player_, &VideoPlayer::SIGNAL_ErrorOccured, this, &SpyderPlayerApp::PlayerErrorOccured);
    connect(player_, &VideoPlayer::SIGNAL_PlayerStateChanged, this, &SpyderPlayerApp::PlaybackStateChanged);
    connect(player_, &VideoPlayer::SIGNAL_EnableSubtitles, this, &SpyderPlayerApp::EnableSubtitles);  

    // Connect Volume Signals
    controlpanel_.ui_.Volume_slider->setValue(100);
    controlpanelFS_.ui_.Volume_slider->setValue(100); 
    player_->SetVolume(100);
}

void SpyderPlayerApp::ShowSplashScreenMsg(QString msg)
{
    if (!splashscreen_.isVisible() && !isInitializing_)
    {
        splashscreen_.show();
        splashscreen_.StartTimer();
        splashscreen_.UpdateStatus(msg);
    }
}

void SpyderPlayerApp::HideSplashScreenMsg(QString msg)
{
    if (splashscreen_.isVisible())
    {
        splashscreen_.UpdateStatus(msg);
        splashscreen_.hide();
    }
}

//*********************************************************************************** 
// Event Override Handlers
//***********************************************************************************
bool SpyderPlayerApp::eventFilter(QObject *object, QEvent *event)
{
    //PRINT << "Event Filter: " << event->type();

    //-----------------------------
    // Handle Window State Changes
    //----------------------------- 
    if(event->type() == QEvent::WindowStateChange)
    {
        ///Qt::WindowStates state = windowState();
        //QString stateStr = GetWindowStateString();
        //PRINT << "--------> Windowstate = " << stateStr;
        //PRINT <<"Windowstate Attribute = " << this->window;
        
        //if (state & Qt::WindowMaximized || state & Qt::WindowFullScreen)
        if(windowState() == Qt::WindowMaximized || windowState() == Qt::WindowFullScreen)
        {
            if(QApplication::focusObject() != nullptr/* && !settingsManager_->settingStack.isVisible()  */ )
            {
                if (!isFullScreen_)
                {
                    PlayerFullScreen();
                    return true;
                }
            }
        }
        else if (windowState() == Qt::WindowMinimized)
        {
            PlayerMinimized();
            return true;
        }
        else
        {
            if (QApplication::focusObject() != nullptr/* && !settingsManager_->settingStack.isVisible()  */ )
            {
                if (isFullScreen_)
                    PlayerNormalScreen();

                return true;
            }
            overlay_->Show();
        }
    }
    //-----------------------------
    // Handle Key Presses
    //-----------------------------
    else if (event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(event);

        // User Activity Detected
        UserActivityDetected();

        //-------------------------------------------------
        // Handle Events when Query Input is not focused
        //-------------------------------------------------
        if (!ui_.Query_input->hasFocus()) 
        {
            if(keyEvent->key() == appData_->HotKeys_.toggleFullscreen)
            {
                if (isFullScreen_)
                    PlayerNormalScreen();
                else
                    PlayerFullScreen();
            }
            else if (keyEvent->key() == appData_->HotKeys_.escapeFullscreen && isFullScreen_)
            {
                PlayerNormalScreen();
            }
            else if (keyEvent->key() == appData_->HotKeys_.togglePlaylist)
            {
                TogglePlaylistView();
            }
            else if (keyEvent->key() == appData_->HotKeys_.collapseAllLists)
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
            else if(keyEvent->key() == appData_->HotKeys_.stopVideo)
            {
                player_->Stop();
            }
            else if(keyEvent->key() == appData_->HotKeys_.volumeMute)
            {
                MutePlayer();
            }
            else if(keyEvent->key() == appData_->HotKeys_.gotoLast)
            {
                PlayLastChannel();
            }
            else if(keyEvent->key() == appData_->HotKeys_.playpause || keyEvent->key() == appData_->HotKeys_.playpauseAlt)
            {
                PlayPausePlayer();
            }
            else if(keyEvent->key() == appData_->HotKeys_.playNext)
            {
                PlayNextChannel();            
            }
            else if(keyEvent->key() == appData_->HotKeys_.playPrevious)
            {
                PlayPreviousChannel();
            }
            else if(keyEvent->key() ==  Qt::Key::Key_Return) 
            {
                playlistManager_->PlaySelectedTreeItem();
            }
            else if(keyEvent->key() ==  appData_->HotKeys_.showOptions)
            {
                ShowSettings();
            }

            // Only process these if Playlist Tree is not focused
            else if (!playlistManager_->PlaylistTreeHasFocus())
            {
                if(keyEvent->key() == appData_->HotKeys_.volumeUp)
                {
                    IncreaseVolume();
                }
                else if(keyEvent->key() == appData_->HotKeys_.volumeDown)
                {
                    DecreaseVolume();
                }
                else if(keyEvent->key() == appData_->HotKeys_.seekForward)
                {
                    SeekForward();
                }
                else if(keyEvent->key() == appData_->HotKeys_.seekBackward)
                {
                    SeekBackward();
                }
                else return false;
            }
            return true;
        }
        //-------------------------------------------------
        // Handle Events when Query Input is focused
        //-------------------------------------------------
        else if(ui_.Query_input->hasFocus()) 
        {
            if (keyEvent->key() == Qt::Key::Key_Return) 
            {
                SearchChannels();
            }
            else if(keyEvent->key() == Qt::Key::Key_Escape)
            {
                ui_.Query_input->clear();
            }
            return true;
        }
    }
    else if (event->type() == QEvent::Type::MouseButtonPress || event->type() == QEvent::Type::MouseButtonRelease || event->type() == QEvent::Type::MouseMove ||
             event->type() == QEvent::Type::Wheel || event->type() == QEvent::Type::MouseButtonDblClick)
    {
        UserActivityDetected();
    }
    // If mouse position is over the overlay_.overlayLabel_, then trigger UserActivityDetected
    /*if (isFullScreen_ && overlay_->IsOutsideOverlay())
        UserActivityDetected(); */

    //else return QObject::eventFilter(object, event);
    return QObject::eventFilter(object, event);
}

void SpyderPlayerApp::moveEvent(QMoveEvent *event)
{
    //##if (overlay_)
        //##overlay_->Resize();
    overlay_->Resize();
    //UserActivityDetected();
    QWidget::moveEvent(event);
    //overlay_->activateWindow();
}

void SpyderPlayerApp::mousePressEvent(QMouseEvent *event)
{   
    UserActivityDetected();
    if (event->button() == Qt::MouseButton::LeftButton)
    {
        // Capture the global position where the mouse was pressed
        mousePressPos_ = event->globalPosition().toPoint();
        if (!isFullScreen_)
        {
            mouseMoveActive_ = true;
            //controlpanel_.setFocus();
            //overlay_->hide();
        }
    }
    QWidget::mousePressEvent(event);
}

void SpyderPlayerApp::mouseMoveEvent(QMouseEvent *event)
{   
    UserActivityDetected();

    if (mouseMoveActive_ && mousePressPos_.isNull() == false)
    {
        // Calculate how far the mouse moved
        QPoint delta = event->globalPosition().toPoint() - mousePressPos_;

        // Move the widget (or window) by the same amount
        this->move(this->pos() + delta);

        // Update the press position for the next movement calculation
        mousePressPos_ = event->globalPosition().toPoint();

        //if(!isFullScreen_)
            //controlpanel_.activateWindow();
    }

    QWidget::mouseMoveEvent(event);
}

void SpyderPlayerApp::mouseReleaseEvent(QMouseEvent *event)
{   
    UserActivityDetected();

    if(event->button() == Qt::MouseButton::LeftButton)
    {
        // Reset when the left mouse button is released
        mousePressPos_ = QPoint();
        mouseMoveActive_ = false;
        //overlay_->show();
        overlay_->Resize();
        //overlay_->activateWindow();
    }
    if (isFullScreen_)
    {   
        //##overlay_->show();
        overlay_->Resize();
        controlpanelFS_.raise();
    }
    else
        controlpanel_.setFocus();
        //activateWindow();

    QWidget::mouseReleaseEvent(event);
}

void SpyderPlayerApp::resizeEvent(QResizeEvent *event)
{
    UserActivityDetected();
    overlay_->Resize();
    //overlay_->activateWindow();
    setFocus();

    QWidget::resizeEvent(event);
}

//***************************************************************************************
// GUI Functions
//***************************************************************************************
void SpyderPlayerApp::PlayerNormalScreen()
{
    controlpanelFS_.hide();
    setWindowState(Qt::WindowState::WindowNoState);
    ui_.Horizontal_splitter->setSizes({450, 1000});  // Restore left side
    ui_.Vertical_splitter->setSizes({800, 1});
    ui_.Horizontal_splitter->setHandleWidth(2);
    player_->GetVideoPanel()->showNormal();  
    player_->ChangeUpdateTimerInterval(false);
    isPlaylistVisible_ = true;
    inactivityTimer_->stop();
    //setFocus();
    isFullScreen_ = false;
    ShowControlPanel();
    //overlay_->show();
    //##overlay_->hide();
    overlay_->Resize();
    //##overlay_->setFocus();
    //player_->GetVideoPanel()->activateWindow();
    //playlistManager_->setFocus();
    //overlay_->activateWindow();
    //PRINT << "PlayerNormalScreen";
}

void SpyderPlayerApp::PlayerFullScreen()
{
    ui_.Horizontal_splitter->setSizes({0, 800});  // Hide left side
    ui_.Vertical_splitter->setSizes({800, 0});
    isPlaylistVisible_ = false;
    setWindowState(Qt::WindowState::WindowFullScreen);
    isFullScreen_ = true;
    player_->GetVideoPanel()->showFullScreen();
    player_->ChangeUpdateTimerInterval(true);
    ui_.Horizontal_splitter->setHandleWidth(0);
    ui_.Vertical_splitter->setHandleWidth(0);
    //ui_.Vertical_splitter->setFocus();
    //##overlay_->show();
    overlay_->Resize();
    //##overlay_->setFocus();
    //ShowControlPanel(true);
    //player_->GetVideoPanel()->setFocus();

    // Delay showing control panel
    QTimer::singleShot(200, this, [this]() { ShowControlPanel(true); });

    //if (platform_ == "Linux")
        // Initial postion is off when going to fullscreen in linux, so just hide it initially
        //controlpanelFS_.hide();

    //overlay_->activateWindow();
    player_->GetVideoPanel()->activateWindow();
    inactivityTimer_->start();
    //PRINT << "PlayerFullScreen";
}

void SpyderPlayerApp::PlayerMinimized()
{
    setWindowState(Qt::WindowState::WindowMinimized);
    player_->ChangeUpdateTimerInterval(true);
    overlay_->Hide();
    //##overlay_->hide();
}

void SpyderPlayerApp::ShowControlPanel(bool initial)
{
    int panel_width = controlpanelFS_.width();
    int panel_height = controlpanelFS_.height();
    
    QPoint global_pos;

    //if (windowState() == Qt::WindowState::WindowFullScreen)  
    if (isFullScreen_ and !isPlaylistVisible_)
    {
        int new_x = (width() - panel_width) / 2;
        int new_y = height() - panel_height - 20;
        global_pos = mapToGlobal(QPoint(new_x, new_y));
    }
    else if (isFullScreen_ and isPlaylistVisible_)
    {
        try
        {
            int new_x = (GetVideoPanelWidth() - panel_width) / 2; 
            int new_y = GetVideoPanelHeight() - panel_height - 20;
            global_pos = player_->GetVideoPanel()->mapToGlobal(QPoint(new_x, new_y));
        }
        catch(const std::exception& e)
        {
            PRINT << e.what();
            return;
        }

    }
    
    if (controlpanelPosition_ != global_pos)
    {
        controlpanelPosition_ = global_pos;
        controlpanelFS_.move(global_pos);
    }
    
    if (isFullScreen_)
    {
        if (initial && platform_ == "Linux")
            controlpanelFS_.hide();
        else
            controlpanelFS_.show();
    }
    else
    {
        controlpanelFS_.hide();
    }
      
}

void SpyderPlayerApp::TogglePlaylistView()
{
    if (isPlaylistVisible_)
    {
        PRINT << "TogglePlaylistView - HIDE";
        ui_.Horizontal_splitter->setSizes({0, 1000});  // Hide left side
        ui_.Horizontal_splitter->setHandleWidth(0);
        ui_.Vertical_splitter->setHandleWidth(0);  // Keep if needed; otherwise remove
        isPlaylistVisible_ = false;

        // Force relayout
        overlay_->Resize();

        //##overlay_->hide();
        //##overlay_->Resize(true);
        //##overlay_->setFocus();
        // Simulate a resize event

        /*if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::VLC)
        {
            QSize currentSize = size();
            resize(currentSize.width() + 1, currentSize.height());  // Temporary increase
            QTimer::singleShot(100, this, [this, currentSize]() {  // Restore original size after event loop
                resize(currentSize);
                if (isFullScreen_) PlayerFullScreen();
            });
        }*/

        //setStyleSheet("background-color: transparent;");

        //ui_.Horizontal_splitter->lower();
        //player_->ChangeUpdateTimerInterval(isFullScreen_);
        
        //QApplication::processEvents(); 
    }
    else
    {
        PRINT << "TogglePlaylistView - SHOW";
        ui_.Horizontal_splitter->setSizes({400, 1000});  // Show left side
        ui_.Horizontal_splitter->setHandleWidth(2);

        isPlaylistVisible_ = true;

        //updateGeometry();
        //ui_.Query_input->show();
        //ui_.Search_button->show();
        //setStyleSheet("background-color: black;");
        overlay_->Resize();
        //##overlay_->setFocus();
        
    }

    if (isFullScreen_)
    {
        //overlay_->Resize(true);
        ShowControlPanel();
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

void SpyderPlayerApp::PlayerPositionChanged(qint64 position)
{
    videoPosition_ = position;

    //PRINT << "Duration: " << videoDuration_ << " Position: " << position;  

    // Only start timer if video is streaming live, not fixed duration video
    //if (videoDuration_ > 0 ) stalledVideoTimer_->start();  

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

void SpyderPlayerApp::PlaybackStateChanged(ENUM_PLAYER_STATE state)
{
    PRINT << "Playback State Changed: " << PlayerStateToString(state);

    if (state == ENUM_PLAYER_STATE::PLAYING)
    {
        playbackStatusTimer_->stop();
        ShowCursorNormal();
        PlayerDurationChanged(player_->GetVideoDuration());
        ChangePlayingUIStates(true);
        screensaverInhibitor_->inhibit();
        retryPlaying_ = false;
        retryCount_ = appData_->RetryCount_;
        ui_.Status_label->setText("");
        playbackStatusTimer_->start();
        //stalledVideoTimer_->start();
    }
    else if (state == ENUM_PLAYER_STATE::LOADING)
    {
        ShowCursorBusy();
        ui_.Status_label->setText("Buffering .....");
    }
    else if (state == ENUM_PLAYER_STATE::RECOVERING)
    {
        ShowCursorBusy();
        ui_.Status_label->setText(player_->GetPlayerStatus());
    }    
    else
    {
        ShowCursorNormal();
        ChangePlayingUIStates(false);
        screensaverInhibitor_->uninhibit();

        if (player_->GetPlayerState() == ENUM_PLAYER_STATE::ENDED)
            ui_.Status_label->setText("Playback Ended");
        else if (state == ENUM_PLAYER_STATE::ERROR)
            ui_.Status_label->setText("Error: " + player_->GetPlayerStatus());
        else if (state == ENUM_PLAYER_STATE::NOMEDIA)
            ui_.Status_label->setText("Invalid Media");
        else if (state == ENUM_PLAYER_STATE::STALLED)
            ui_.Status_label->setText("Playback Stalled");
        else if (state == ENUM_PLAYER_STATE::STOPPED)
            ui_.Status_label->setText("Playback Stopped");
        else if (state == ENUM_PLAYER_STATE::PAUSED)
            ui_.Status_label->setText("Playback Paused");
        else
            ui_.Status_label->setText("");
    }
}

void SpyderPlayerApp::UpdatePlaybackStatus()
{
    try 
    {
        if (player_->GetPlayerState() == ENUM_PLAYER_STATE::PLAYING)
        {
            ShowVideoResolution();
        }
    }
    catch (std::exception& e)
    {
        PRINT << "UpdatePlaybackStatus error: " << e.what();
    }
    playbackStatusTimer_->stop();
}

void SpyderPlayerApp::PlayerErrorOccured(const std::string& error)
{
    PRINT << "Player Error: " << error;

    ui_.Status_label->setText("Error: " + QString::fromStdString(error));
}


void SpyderPlayerApp::OnHSplitterResized(int pos, int index)
{
    overlay_->Resize();
    if (isFullScreen_)
        ShowControlPanel();
}

void SpyderPlayerApp::EnableSubtitles(bool enable)
{
    subtitlesEnabled_ = enable;
    controlpanelFS_.ui_.CloseCaption_button->setEnabled(enable);
    controlpanel_.ui_.CloseCaption_button->setEnabled(enable); 
}

void SpyderPlayerApp::ShowSubtitleTracks()
{
    if (subtitlesEnabled_)
    {
        QList<QPair <int, QString>> tracks = player_->GetSubtitleTracks();

        //PRINT << "Available Subtitle Tracks: " << tracks.size();

        if (tracks.size() > 0)
        {
            subtitlesMenu_->clear();

            for(const auto& track : tracks)
            {
                if (track.second.isEmpty())
                    subtitlesMenu_->addAction("Track " + QString::number(track.first));
                else
                    subtitlesMenu_->addAction(track.second);
            }
            //subtitlesMenu_->connect(subtitlesMenu_, &QMenu::triggered, this, &SpyderPlayerApp::SelectSubtitleTrack);
            subtitlesMenu_->exec(QCursor::pos());
        }
    }
}

void SpyderPlayerApp::SelectSubtitleTrack(QAction* menuItem)
{
    // Get the index of the selected track
    QList<QAction*> menuSelections = subtitlesMenu_->actions();
    int index = menuSelections.indexOf(menuItem);

    QList<QPair <int, QString>> tracks = player_->GetSubtitleTracks();


    PRINT << "Selected menu index:" << index;
    PRINT << "Selected track:" << tracks[index].first;
    player_->SetSubtitleTrack(tracks[index].first);
}

void SpyderPlayerApp::UserActivityDetected()
{
    // Add the implementation of this function here
    if (isFullScreen_ )
    {
        ShowCursorNormal();

        if (platform_ == "Linux")
            ShowControlPanel();
        else
            controlpanelFS_.show();

        //controlpanelFS_.activateWindow();
        //overlay_->activateWindow();
        controlpanelFS_.raise();
        inactivityTimer_->start();
    }
}
void SpyderPlayerApp::InactivityDetected()
{
    if (isFullScreen_ && !controlpanelFS_.hasFocus() && !subtitlesMenu_->isVisible())
    {
        controlpanelFS_.hide();
        //##overlay_->activateWindow();

        if (!isPlaylistVisible_ && !settingsManager_->IsVisible())
            ShowCursorBlank();
    }
}

void SpyderPlayerApp::StalledVideoDetected()
{
    retryPlaying_ = true;
    if (retryCount_ > 0)
    {
        /*retryCount_--;
        stalledVideoTimer_->stop();
        return;*/
        PRINT << "-----------------> STALLED VIDEO DETECTED <-----------------";
        //player_->Stop();
        stalledVideoTimer_->stop();
        ui_.Status_label->setText("Stalled Video - Resetting(" + QString::number(retryCount_) + ")");
        PRINT << "Stalled Video - Resetting... Retry Count: " << retryCount_;

        player_->RefreshVideoSource();
        //player_->Play();
        retryCount_--;
    }
    else
        ChangePlayingUIStates(false);
    retryPlaying_ = false;
}

void SpyderPlayerApp::ShowSettings()
{
    PRINT << "ShowSettings Button Pressed";
    settingsManager_->ShowSettingsMainScreen(true);
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

    currentChannelName_ = channelName;
    currentChannelSource_ = source;

    // Reset retry count
    retryPlaying_  = false;
    retryCount_ = appData_->RetryCount_;

    // Stop() resets player defaults and retries
    player_->Stop();
    player_->SetVideoSource(source);
    setWindowTitle("SPYDER Player - " + QSTR(channelName));
    player_->Play();
    player_->GetVideoPanel()->setFocus();
    //ChangePlayingUIStates(true);
}

void SpyderPlayerApp::PlayPausePlayer()
{
    ENUM_PLAYER_STATE state = player_->GetPlayerState();

    if (state == ENUM_PLAYER_STATE::PLAYING)
    {
        player_->Pause();
        //stalledVideoTimer_->stop();
    }
    else
    {
        // Check if video reached end, if so stop and restart at beginning
        if (player_->GetPlayerState() == ENUM_PLAYER_STATE::ENDED)
        {
            //videoPosition_ = 0;
            //player_->Stop();
            //player_->Play();
            PlaySelectedChannel(currentChannelName_, currentChannelSource_);
        }
        else if (player_->GetPlayerState() == ENUM_PLAYER_STATE::STOPPED)
        {
            PlaySelectedChannel(currentChannelName_, currentChannelSource_);
        }
        else if (player_->GetPlayerState() == ENUM_PLAYER_STATE::PAUSED)
        {
            player_->Play();
        }
    }
}

void SpyderPlayerApp::StopPlayer()
{
    //stalledVideoTimer_->stop();
    playbackStatusTimer_->stop();
    player_->Stop();
    SetCursorNormal();
    ui_.Status_label->setText("Playback Stopped");
}

void SpyderPlayerApp::SeekForward()
{
    if (videoDuration_ > 0)
        player_->SkipPosition(player_->GetPosition() + 10000);
}

void SpyderPlayerApp::SeekBackward()
{
    if (videoDuration_ > 0)
        player_->SkipPosition(player_->GetPosition() - 10000);
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
    if (player_->IsMuted())
    {
        controlpanelFS_.ui_.Volume_slider->setEnabled(true);
        controlpanelFS_.ui_.FullVolume_button->setEnabled(true);
        controlpanel_.ui_.Volume_slider->setEnabled(true);
        controlpanel_.ui_.FullVolume_button->setEnabled(true);
        player_->Mute(false);
        player_->SetVolume(volume_);
        UpdateVolumeSlider(volume_);
    }
    else
    {
        volume_ = player_->GetVolume();
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
    //PRINT << "Volume: " << volume;

    player_->SetVolume(volume);
    UpdateVolumeSlider(volume);
    player_->Mute(false);
}

void SpyderPlayerApp::IncreaseVolume()
{
    if (player_->IsMuted())
        return;

    int volume = player_->GetVolume();
    volume = volume + 10;
    if (volume > 100)
        volume = 100;

    player_->SetVolume(volume);
    UpdateVolumeSlider(volume);
}

void SpyderPlayerApp::DecreaseVolume()
{
    if (player_->IsMuted())
        return;

    int volume = player_->GetVolume();
    volume = volume - 10;
    if (volume < 0)
        volume = 0;

    player_->SetVolume(volume);
    UpdateVolumeSlider(volume);
}

//*****************************************************************************************
// Positon Slider Functions
//*****************************************************************************************
void SpyderPlayerApp::OnPositionSliderPressed()
{
    isVideoPlaying_ = player_->GetPlayerState() == ENUM_PLAYER_STATE::PLAYING;
    if (isVideoPlaying_)
    {
        ;//stalledVideoTimer_->stop();
    }

    player_->OnChangingPosition(isVideoPlaying_);
    if (isFullScreen_)
    {
        inactivityTimer_->stop();
        controlpanelFS_.setFocus();
    }
}

void SpyderPlayerApp::OnPositionSliderMoved()
{
    //videoChangesPosition_ = false;
    QSlider *slider = dynamic_cast<QSlider *>(sender());
    qint64 position = slider->value();

    player_->SetPosition(position);
    videoPosition_ = position;
    //videoChangesPosition_ = true;
}

void SpyderPlayerApp::OnPositionSliderReleased()
{
    PlayerPositionChanged(videoPosition_);
    player_->OnChangedPosition(isVideoPlaying_);
    if (isVideoPlaying_)
    {
        ChangePlayingUIStates(true);
        //stalledVideoTimer_->start();
    }

    if (isFullScreen_)
    {
        //##overlay_->activateWindow();
        //##overlay_->setFocus();
        inactivityTimer_->start();
    }
}

//***************************************************************************************** 
// Utility Functions
//*****************************************************************************************
void SpyderPlayerApp::SearchChannels()
{
    // Search the Channels for Query
    QString query = ui_.Query_input->text();
    playlistManager_->SearchChannels(query);
}

void SpyderPlayerApp::LoadSessionMedia(PlayListEntry entry)
{
    ShowCursorBusy();
    playlistManager_->AddSessionMedia(entry);
    ShowCursorNormal();
}

void SpyderPlayerApp::LoadSessionPlaylist(PlayListEntry entry)
{
    ShowSplashScreenMsg("Loading Playlist: " + QSTR(entry.name) + " ....");
    ShowCursorBusy();
    playlistManager_->LoadPlayList(entry, false);
    ShowCursorNormal();
    HideSplashScreenMsg(QSTR(entry.name) + " Loaded...");
}

void SpyderPlayerApp::ShowCursorNormal()
{
    SetCursorNormal();
}

void SpyderPlayerApp::ShowCursorBusy()
{
    SetCursorBusy();
}

void SpyderPlayerApp::ShowCursorBlank()
{
    SetCursorBlank();
}

int SpyderPlayerApp::GetVideoPanelWidth()
{
    if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::VLC)
        return ui_.VideoView_widget->width(); 
    else
        return player_->GetVideoPanel()->width();
}

int SpyderPlayerApp::GetVideoPanelHeight()
{
    if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::VLC)
        return ui_.VideoView_widget->height(); 
    else
        return player_->GetVideoPanel()->height();
}

QWidget* SpyderPlayerApp::GetVideoPanelWidget()
{
    if (appData_->PlayerType_ == ENUM_PLAYER_TYPE::VLC)
        return ui_.VideoView_widget;
    else
        return player_->GetVideoPanel();
}

QString SpyderPlayerApp::GetWindowStateString() 
{
    Qt::WindowStates state = windowState();
    
    if (state & Qt::WindowMaximized)
        return "Maximized";
    else if (state & Qt::WindowMinimized)
        return "Minimized";
    else if (state & Qt::WindowFullScreen)
        return "Fullscreen";
    else
        return "Normal";
}

int SpyderPlayerApp::GetRetryTimeDelay()
{
    return appData_->RetryDelay_;
}

int SpyderPlayerApp::GetMaxRetryCount()
{
    return appData_->RetryCount_;
}
