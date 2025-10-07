// VLCPlayer.cpp
#include "VLCPlayer.h"
#include "SpyderPlayerApp.h"
#include <QDebug>
#include <QThread>
#include <QApplication>
#include <string>
#include <stdexcept>
#include <filesystem>
#include <algorithm>

VLCPlayer::VLCPlayer(Ui::PlayerMainWindow* mainWindow, QWidget* parent)
    : VideoPlayer(parent)
{
    mainWindow_ = mainWindow;
    app_ = static_cast<SpyderPlayerApp*>(parent);
    videoWidget_ = mainWindow->VideoView_widget; //new QWidget(mainWindow->VideoView_widget->parentWidget());

    subtitleCount_ = -1;
    subtitleIndex_ = -1;
    //videoPanel_->setWindowFlags(Qt::FramelessWindowHint);
    //videoPanel_->setAttribute(Qt::WA_TranslucentBackground);

    //mainWindow->gridLayout->removeWidget(mainWindow->VideoView_widget);
    //mainWindow->gridLayout->addWidget(videoPanel_, 1, 1, 1, 1);
    //videoPanel_->setStyleSheet("background-color: transparent;");
    //mainWindow->topverticalLayout->removeWidget(mainWindow->VideoView_widget);
    //mainWindow->topverticalLayout->addWidget(videoPanel_);
    
    //videoPanel_->setStyleSheet("background-color: rgba(0, 0, 0, 2);");

    //InitPlayer(nullptr);

    updateTimer_ = new QTimer(this);
    updateTimer_->setInterval(250);
    connect(updateTimer_, &QTimer::timeout, this, &VLCPlayer::UpdatePlayerStatus);

    stalledVideoTimer_ = new QTimer(this);
    stalledVideoTimer_->setInterval(3000);
    connect(stalledVideoTimer_, &QTimer::timeout, this, &VLCPlayer::StalledVideoDetected);

}

VLCPlayer::~VLCPlayer()
{
    PRINT << "VLCPlayer destructor called";

    // Stop timers first to prevent pending events
    if (updateTimer_) 
    {
        updateTimer_->stop();
        delete updateTimer_;
        updateTimer_ = nullptr;
    }
    if (stalledVideoTimer_) 
    {
        stalledVideoTimer_->stop();
        delete stalledVideoTimer_;
        stalledVideoTimer_ = nullptr;
    }

    // Stop playback and release VLC resources
    if (mediaPlayer_) 
    {
        // Detach all events to prevent callbacks
        if (eventManager_) 
        {
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerPlaying, HandleVLCEvent, this);
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerPaused, HandleVLCEvent, this);
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerStopped, HandleVLCEvent, this);
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerEndReached, HandleVLCEvent, this);
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerEncounteredError, HandleVLCEvent, this);
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerBuffering, HandleVLCEvent, this);
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerLengthChanged, HandleVLCEvent, this);
            libvlc_event_detach(eventManager_, libvlc_MediaPlayerTimeChanged, HandleVLCEvent, this);
            eventManager_ = nullptr;
        }

        // Stop playback if not already stopped
        try 
        {
            if (libvlc_media_player_is_playing(mediaPlayer_)) {
                libvlc_media_player_stop(mediaPlayer_);
        }
        } catch (const std::exception& e)
        {
            qDebug() << "Error stopping media player in destructor: " << e.what();
        }

        // Release media player
        libvlc_media_player_release(mediaPlayer_);
        mediaPlayer_ = nullptr;
    }

    // Release media
    if (media_) 
    {
        libvlc_media_release(media_);
        media_ = nullptr;
    }

    // Release VLC instance
    if (vlcInstance_) 
    {
        libvlc_release(vlcInstance_);
        vlcInstance_ = nullptr;
    }
}

void VLCPlayer::InitPlayer(void *args)
{
    videoPanel_ = static_cast<QWidget*>(args);
    
    SetupPlayer();
    videoPanel_->setMouseTracking(true);
    videoPanel_->installEventFilter(app_);
    //videoWidget_ = videoPanel_;
}


std::vector<const char*> VLCPlayer::GetInitArgs(const QString& VCLSetupParams)
{
    static std::vector<QByteArray> persistentByteArrays;  // Static = stays alive
    persistentByteArrays.clear();
    
    std::vector<const char*> args;
    
    if (VCLSetupParams.isEmpty()) 
    {
        return args;
    }
    
    QStringList argsList = VCLSetupParams.split(' ', Qt::SkipEmptyParts);
    
    for (const QString& arg : argsList) 
    {
        persistentByteArrays.push_back(arg.toUtf8());
        args.push_back(persistentByteArrays.back().constData());
    }
    
    return args;    
}

void VLCPlayer::SetupPlayer()
{
    try {
        //const char* const vlc_args[] =
        /*{
            "--verbose=2",  
            //"--no-xlib",    // Disable Xlib for better compatibility
            "--network-caching=3000",  
            "--file-caching=1500",
            "--live-caching=1500",
            "--drop-late-frames",
            "--skip-frames",
            "--sout-keep",
            "--clock-jitter=1000",
        };
        //vlcInstance_ = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
        */

        auto vlc_args = GetInitArgs(app_->GetAppData()->VLCSetupArgs_);

        PRINT << "VLC Setup Arguments: ";
        for(auto arg : vlc_args)
            PRINT << arg;

        vlcInstance_ = libvlc_new(vlc_args.size(), vlc_args.data());

        if (!vlcInstance_) 
        {
            throw std::runtime_error("Failed to create VLC instance");
        }

        mediaPlayer_ = libvlc_media_player_new(vlcInstance_);
        if (!mediaPlayer_) 
        {
            throw std::runtime_error("Failed to create VLC media player");
        }

        videoPanel_->show();

#if defined(Q_OS_WIN)
        libvlc_media_player_set_hwnd(mediaPlayer_, reinterpret_cast<void*>(videoPanel_->winId()));
#elif defined(Q_OS_MAC)
        libvlc_media_player_set_nsobject(mediaPlayer_, reinterpret_cast<void*>(videoPanel_->winId()));
#elif defined(Q_OS_LINUX)
        libvlc_media_player_set_xwindow(mediaPlayer_, static_cast<uint32_t>(videoPanel_->winId()));
#endif

        AttachEvents();
        MAX_STALL_RETRIES = app_->GetMaxRetryCount();
    } 
    catch (const std::exception& e) 
    {
        PRINT << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

void VLCPlayer::AttachEvents()
{
    eventManager_ = libvlc_media_player_event_manager(mediaPlayer_);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerPlaying, HandleVLCEvent, this);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerPaused, HandleVLCEvent, this);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerStopped, HandleVLCEvent, this);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerEndReached, HandleVLCEvent, this);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerEncounteredError, HandleVLCEvent, this);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerBuffering, HandleVLCEvent, this);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerLengthChanged, HandleVLCEvent, this);
    libvlc_event_attach(eventManager_, libvlc_MediaPlayerTimeChanged, HandleVLCEvent, this);
}

void VLCPlayer::HandleVLCEvent(const libvlc_event_t* event, void* data)
{
    VLCPlayer* self = static_cast<VLCPlayer*>(data);
    //PRINT << "VLC Event: " << event->type;
    
    switch (event->type) 
    {
    case libvlc_MediaPlayerPlaying:
        self->currentState_ = ENUM_PLAYER_STATE::PLAYING;
        break;
    case libvlc_MediaPlayerPaused:
        self->currentState_ = ENUM_PLAYER_STATE::PAUSED;
        break;
    case libvlc_MediaPlayerStopped:
        self->currentState_ = ENUM_PLAYER_STATE::STOPPED;
        break;
    case libvlc_MediaPlayerEndReached:
        self->currentState_ = ENUM_PLAYER_STATE::ENDED;
        break;
    case libvlc_MediaPlayerEncounteredError:
        self->currentState_ = ENUM_PLAYER_STATE::ERROR;
        //self->ErrorOccured("VLC encountered an error: ");
        break;
    case libvlc_MediaPlayerBuffering:
    case libvlc_MediaPlayerOpening:
        self->currentState_ = ENUM_PLAYER_STATE::LOADING;
        self->bufferSize_ = event->u.media_player_buffering.new_cache;
        PRINT << "Buffering: " << self->bufferSize_;
        //self->playerStatus_ = "Buffering: " + QString::number(event->u.media_player_buffering.new_cache) + "%";
        break;
    case libvlc_MediaPlayerLengthChanged:
        self->duration_ = libvlc_media_player_get_length(self->mediaPlayer_);
        self->UpdateDuration(self->duration_);
        //self->currentState_ = ENUM_PLAYER_STATE::PLAYING;
        break;
    case libvlc_MediaPlayerTimeChanged:
        self->position_ = libvlc_media_player_get_time(self->mediaPlayer_);
        self->currentState_ = ENUM_PLAYER_STATE::PLAYING;
        break;
    default:
        self->currentState_ = ENUM_PLAYER_STATE::IDLE;
        break;
    }
    //QApplication::processEvents();  // Ensure Qt processes events
}

void VLCPlayer::UpdatePlayerStatus()
{
    ENUM_PLAYER_STATE currentState = GetPlayerState();
    int subtitleCount = 0;
    //PRINT << "--->UpdatePlayerStatus: " << QSTR(PlayerStateToString(currentState));

    switch (currentState)
    {
    case ENUM_PLAYER_STATE::LOADING:
        // Remain in loading state until buffer is bellow 100%. 
        // This is when video is paused and the slider position is moved.
        // There is a brief period where the new position needs to be buffered.
        if (!isPlaying_ && bufferSize_ >= 100)
        {
            currentState_ = ENUM_PLAYER_STATE::PAUSED;
            UpdatePlayerState(ENUM_PLAYER_STATE::PAUSED);
            return;
        }
        break;
    case ENUM_PLAYER_STATE::PLAYING:
        inRecovery_ = false;
        stallretryCount_ = 0;
        position_ = libvlc_media_player_get_time(mediaPlayer_);
        UpdatePosition(position_);

        subtitleCount = libvlc_video_get_spu_count(mediaPlayer_);
        if (subtitleCount != subtitleCount_)
        {
            subtitleCount_ = subtitleCount;
            EnableSubtitles(subtitleCount_ > 0);
        }
        if (stallPosition_ != 0 && position_ != stallPosition_)
            // if video resets do not overwrite stallPosition_
            stallPosition_ = position_ ;

        break;
    case ENUM_PLAYER_STATE::PAUSED:
        break;
    //case ENUM_PLAYER_STATE::ENDED:
    case ENUM_PLAYER_STATE::STALLED:
        updateTimer_->stop();
        isPlaying_ = false;
        break;
    case ENUM_PLAYER_STATE::ERROR:
        //inRecovery_ = true;
        RetryStalledPlayer();
        break;
    case ENUM_PLAYER_STATE::ENDED:
        if (duration_ > 0)
        {
            updateTimer_->stop(); 
            isPlaying_ = false; 
        }
        else 
        {
            RetryStalledPlayer();
        }
        break;
    case ENUM_PLAYER_STATE::STOPPED:
        // Stop all playback
        if (stopAll_)
        {
            updateTimer_->stop();
        }
        // Handle fized length playback
        else if (duration_ > 0)
        {
            // Reached the end of the video
            if (position_ >= duration_ - 1000)
            {
                updateTimer_->stop();
                UpdatePosition(duration_);
                isPlaying_ = false;
            }
            // Stopped state reached without reaching the end, attempt recovery
            else
            {
                inRecovery_ = false;
                RetryStalledPlayer();
            }
        }
        // Handle live streams
        else
        {
            inRecovery_ = false;
            RetryStalledPlayer();
        }
        break;
    default:
        //playerStatus_ = "Unknown";
        break;
    }

    // Emit State to Main app
    if(inRecovery_)
    {
        ;//previousState_ = ENUM_PLAYER_STATE::RECOVERING;
        //UpdatePlayerState(ENUM_PLAYER_STATE::RECOVERING);
    }
    else if (currentState != previousState_)
    {
        UpdatePlayerState(currentState);
        previousState_ = currentState;
    }
}


void VLCPlayer::SetVideoSource(const std::string& videoSource)
{
    try 
    {
        source_ = videoSource;
        
        // Release existing media if any
        if (media_) 
        {
            libvlc_media_release(media_);
            media_ = nullptr;
        }

        // Check if the source is a URL (remote link)
        std::string lowerSource = videoSource;
        std::transform(lowerSource.begin(), lowerSource.end(), lowerSource.begin(), ::tolower);
        bool isRemote = lowerSource.find("http://") == 0 || 
                        lowerSource.find("https://") == 0 || 
                        lowerSource.find("rtsp://") == 0 || 
                        lowerSource.find("rtp://") == 0 ||
                        lowerSource.find("file://") == 0;

        if (isRemote)
        {
            // Handle remote link
            media_ = libvlc_media_new_location(vlcInstance_, videoSource.c_str());
        }
        else
        {
            // Handle local file
            // Check if file exists
            if (!std::filesystem::exists(videoSource))
            {
                throw std::runtime_error("Local file does not exist: " + videoSource);
            }
            media_ = libvlc_media_new_path(vlcInstance_, videoSource.c_str());
        }

        if (!media_) 
        {
            throw std::runtime_error("Failed to create VLC media: " + std::string(libvlc_errmsg() ? libvlc_errmsg() : "Unknown error"));
        }

        // Set the media to the player
        libvlc_media_player_set_media(mediaPlayer_, media_);
    } 
    catch (const std::exception& e) 
    {
        PRINT << "SetVideoSource: " << e.what();
    }
}

void VLCPlayer::RefreshVideoSource()
{
    try 
    {
        PRINT << "REFRESH: Refreshing Video Source: " << QString::fromStdString(source_);

        //std::string source = source_;
        //SetVideoSource("blank.mp4");
        SetVideoSource(source_);
        u_int64_t timedelay = retryTimeDelayMs_ * stallretryCount_;
        PRINT << "REFRESH: Retry Time Delay: " << timedelay;
        QTimer::singleShot(timedelay, this, &VLCPlayer::PlaySource);

        playerStatus_ += " in " + QString::number(timedelay / 1000.0, 'f', 1) + " secs";
    } 
    catch (const std::exception& e) 
    {
        PRINT << "RefreshVideoSource: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

void VLCPlayer::Play()
{
    stopAll_ = false;
    inRecovery_ = false;
    stallretryCount_ = 0;
    retryCount_ = 0;
    duration_ = 0;
    position_ = 0;
    subtitleCount_ = -1;
    stallPosition_ = 0;
    previousState_ = ENUM_PLAYER_STATE::IDLE;
    currentState_ = ENUM_PLAYER_STATE::LOADING;
    stalledVideoTimer_->stop();

    MAX_STALL_RETRIES = app_->GetMaxRetryCount();
    retryTimeDelayMs_ = app_->GetRetryTimeDelay();

    PRINT << "MAX_STALL_RETRIES: " << MAX_STALL_RETRIES;
    PRINT << "retryTimeDelayMs_: " << retryTimeDelayMs_;

    isPlaying_ = true;
    updateTimer_->start();
    PlaySource();
}

void VLCPlayer::Resume()
{
    if(currentState_ == ENUM_PLAYER_STATE::PAUSED)
    {   
        PRINT << "Resuming Playback...";
        stopAll_ = false;
        inRecovery_ = false;
        stallretryCount_ = 0;
        retryCount_ = 0;
        isPlaying_ = true;
        updateTimer_->start();
        PlaySource();   
    }
}

void VLCPlayer::ResumePlayback()
{
    stopAll_ = false;
    inRecovery_ = false;
    stallretryCount_ = 0;
    retryCount_ = 0;

    previousState_ = ENUM_PLAYER_STATE::PAUSED;
    currentState_ = ENUM_PLAYER_STATE::PLAYING;
    stalledVideoTimer_->stop();

    updateTimer_->start();
    PlaySource();
    SetPosition(stallPosition_);
}

void VLCPlayer::PlaySource()
{
    PRINT << "PlaySource";

    if (stopAll_) 
    {
        inRecovery_ = false;
        return;
    }

    try 
    {
        if (currentState_ == ENUM_PLAYER_STATE::PAUSED) 
        {
            libvlc_media_player_play(mediaPlayer_);
            //timeoutTimer_->start(30000);
            retryCount_ = 0;
            stallretryCount_ = 0;
            Mute(isMuted_);
            //PRINT << "Duration = " << duration_ << ", Resume Position = " << stallPosition_;
            if (duration_ > 0) 
                SetPosition(stallPosition_);
            //if (duration_ > 0 && position_ > skipBackLength_)
                //SkipPosition(position_ - skipBackLength_);

            //videoPanel_->parentWidget()->layout()->activate(); // Force layout recalculation
            //videoPanel_->updateGeometry();
            /*unsigned width, height;
            if (libvlc_video_get_size(mediaPlayer_, 0, &width, &height) == 0) {
                videoPanel_->resize(width, height);
                videoPanel_->updateGeometry(); // Force layout update
                PRINT << "Video size:" << width << "x" << height;
            }*/
            return;
        } 
        //else if (retryCount_ <= MAX_RETRIES) 
        else if (stallretryCount_ <= MAX_STALL_RETRIES)
        {
            Mute(isMuted_);
            libvlc_media_player_play(mediaPlayer_);

            if (inRecovery_ && duration_ > 0) 
                SetPosition(stallPosition_);

            //if (duration_ > 0 && position_ > skipBackLength_)
                //SkipPosition(position_ - skipBackLength_);
            //videoPanel_->parentWidget()->layout()->activate(); // Force layout recalculation
            //videoPanel_->updateGeometry();
            /*unsigned width, height;
            if (libvlc_video_get_size(mediaPlayer_, 0, &width, &height) == 0) {
                videoPanel_->resize(width, height);
                videoPanel_->updateGeometry(); // Force layout update
                PRINT << "Video size:" << width << "x" << height;
                videoPanel_->lower();
            }*/
            //inRecovery_ = false;
            return;
        }
        else if (stallretryCount_ > MAX_STALL_RETRIES)
        {
            PRINT << "Max retries reached, aborting play.";
            ErrorOccured("Max retries reached");
            currentState_ = ENUM_PLAYER_STATE::STALLED;
            UpdatePlayerState(currentState_);
            inRecovery_ = false;
            return;
        }
    } 
    catch (const std::exception& e) 
    {
        PRINT << "Play error: " << e.what();
        ErrorOccured(std::string(e.what()));
        RetryStalledPlayer();
    }
}

void VLCPlayer::Pause()
{
    PRINT << "Playback Paused...";

    stallPosition_ = position_;
    stalledVideoTimer_->stop();
    if (libvlc_media_player_is_playing(mediaPlayer_)) 
    {
        libvlc_media_player_pause(mediaPlayer_);
        isPlaying_ = false;
    }
    inRecovery_ = false;
    //updateTimer_->stop();
    currentState_ = ENUM_PLAYER_STATE::PAUSED;
    UpdatePlayerState(currentState_);
}

void VLCPlayer::StopPlayback()
{
    stalledVideoTimer_->stop();
    try
    {
        if (libvlc_media_player_is_playing(mediaPlayer_)) 
            libvlc_media_player_stop(mediaPlayer_);
    }
    catch (const std::exception& e)
    {
        PRINT << "StopPlayback error: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

void VLCPlayer::Stop()
{
    StopPlayback();
    inRecovery_ = false;
    stopAll_ = true;
    isPlaying_ = false;
    stallPosition_ = position_ = 0;
    updateTimer_->stop();
    currentState_ = ENUM_PLAYER_STATE::STOPPED;
    UpdatePlayerState(currentState_);
}

void VLCPlayer::SetPosition(qint64 position)
{
    stalledVideoTimer_->stop();
    libvlc_media_player_set_time(mediaPlayer_, position);
    position_ = position;
}

void VLCPlayer::SkipPosition(qint64 position)
{
    //qint64 newPos = GetPosition() + position;
    SetPosition(position);
}

qint64 VLCPlayer::GetPosition()
{
    return libvlc_media_player_get_time(mediaPlayer_);
}

qint64 VLCPlayer::GetVideoDuration()
{
    return libvlc_media_player_get_length(mediaPlayer_);
}

void VLCPlayer::SetVolume(int volume)
{
    libvlc_audio_set_volume(mediaPlayer_, volume);
}

int VLCPlayer::GetVolume()
{
    return libvlc_audio_get_volume(mediaPlayer_);
}

void VLCPlayer::Mute(bool mute)
{
    libvlc_audio_set_mute(mediaPlayer_, mute ? 1 : 0);
    isMuted_ = mute;
}

bool VLCPlayer::IsMuted()
{
    return libvlc_audio_get_mute(mediaPlayer_) != 0;
}

QString VLCPlayer::GetVideoResolution()
{
    unsigned width = 0, height = 0;
    if (libvlc_video_get_size(mediaPlayer_, 0, &width, &height) == 0) 
    {
        return QString("%1x%2").arg(width).arg(height);
    }
    return "Unknown";
}

QList<QPair<int, QString>> VLCPlayer::GetSubtitleTracks()
{
    QList<QPair<int, QString>> tracks;
    int count = libvlc_video_get_spu_count(mediaPlayer_);
    if (count > 0) {
        libvlc_track_description_t* desc = libvlc_video_get_spu_description(mediaPlayer_);
        libvlc_track_description_t* current = desc;
        while (current) 
        {
            tracks.append(qMakePair(current->i_id, QString(current->psz_name)));
            current = current->p_next;
        }
        libvlc_track_description_list_release(desc);
    }
    return tracks;
}

void VLCPlayer::SetSubtitleTrack(int index)
{
    libvlc_video_set_spu(mediaPlayer_, index);
}

ENUM_PLAYER_STATE VLCPlayer::GetPlayerState()
{
    return currentState_;
}

QString VLCPlayer::GetPlayerStatus()
{
    return playerStatus_;
}

void VLCPlayer::OnChangingPosition(bool isPlaying)
{
    isPositionSeeking_ = true;
    if (isPlaying) 
    {
        stalledVideoTimer_->stop();
        libvlc_media_player_pause(mediaPlayer_);
    }
}

void VLCPlayer::OnChangedPosition(bool isPlaying)
{
    isPositionSeeking_ = false;
    stallPosition_ = GetPosition();
    if (isPlaying) 
    {
        //timeoutTimer_->start();
        //stalledVideoTimer_->start();
        ResumePlayback();
    }
    else
    {
        currentState_ = ENUM_PLAYER_STATE::PAUSED;
        UpdatePlayerState(currentState_);
    }
}

void VLCPlayer::ChangeUpdateTimerInterval(bool isFullScreen)
{
    Q_UNUSED(isFullScreen);
    ; //positionTimer_->setInterval(isFullScreen ? 100 : 250);
}

void VLCPlayer::SetVideoTitle(const QString& text)
{
    if (mediaPlayer_) 
    {
        libvlc_video_set_marquee_string(mediaPlayer_, libvlc_marquee_Text, text.toUtf8().constData());
        // Configure marquee appearance (only needed once or when changed)
        libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Enable, 0);
        libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Color, 0xFFFFFF); // White text
        libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Opacity, 255); // Fully opaque
        libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Position, 4); // Top-center (5 = top)
        int fontSize = 12; //videoPanel_->height() * 0.03; 
        //PRINT << ">>>>>>>>Panel Size = " << videoPanel_->height() << " fontSize = " << fontSize;
        //fontSize = qBound(18, fontSize, 30);
        libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Size, fontSize); 
        //libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_X, videoPanel_->width()/2); // -1 means auto-center
        libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Y, 20); // Small offset from top (adjust as needed)
    }
}

void VLCPlayer::SetVideoTitleVisible(bool visible)
{
    if (mediaPlayer_)
    {
        int fontSize = 20;  //videoPanel_->height() * 0.03; 
        //PRINT << ">>>>>>>>>Panel Size = " << videoPanel_->height() << " fontSize = " << fontSize;
        //fontSize = qBound(12, fontSize, 36);
        unsigned width = 0, height = 0;
        if (libvlc_video_get_size(mediaPlayer_, 0, &width, &height) == 0) 
        {
            fontSize = width * 0.05;
            //fontSize = qBound(18, fontSize, 30);
            //PRINT << ">>>>>>>>> Res Size = " <<  width << " fontSize = " << fontSize;
        }

        if (titleFontSize_ != fontSize)
        {
            libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Size, fontSize); 
            titleFontSize_ = fontSize;
        }
        libvlc_video_set_marquee_int(mediaPlayer_, libvlc_marquee_Enable, visible ? 1 : 0);
    }
}

void VLCPlayer::StalledVideoDetected()
{
    //while(inRecovery_)
    if(inRecovery_ == false)
    {
        PRINT << ">>>>>> Stalled video detected, attempting to retry... Attempt " << stallretryCount_+1;
        RetryStalledPlayer();
    }
}
void VLCPlayer::RetryStalledPlayer()
{
    if (stopAll_) 
    {
        inRecovery_ = false;
        return;
    }

    try 
    {
        if (stallretryCount_ < MAX_STALL_RETRIES && inRecovery_ == false) 
        {
            inRecovery_ = true;
            stallretryCount_++;
            PRINT << "-----------------> STALLED VIDEO DETECTED <-----------------";
            playerStatus_ = "Recovering (" + QString::number(stallretryCount_) + " of " + QString::number(MAX_STALL_RETRIES) + ")";
            PRINT << playerStatus_;

            StopPlayback();
            //SetupPlayer();
            RefreshVideoSource();
            previousState_ = ENUM_PLAYER_STATE::RECOVERING;
            UpdatePlayerState(ENUM_PLAYER_STATE::RECOVERING);
        } 
        else if (stallretryCount_ < MAX_STALL_RETRIES && inRecovery_ == true) 
            return;
        else 
        {
            inRecovery_ = false;
            playerStatus_ = "Max Recovery attempts reached";
            PRINT << playerStatus_;
            currentState_ = ENUM_PLAYER_STATE::STALLED;
            //libvlc_event_t* event = new libvlc_event_t;
            //event->type = libvlc_MediaPlayerEncounteredError;
            //HandleVLCEvent(event, this);

            UpdatePlayerState(currentState_);
            //QTimer::singleShot(1000, this, [=]() { inRecovery_ = false; UpdatePlayerState(currentState_); });
        }
    } 
    catch (const std::exception& e) 
    {
        PRINT << "Retry error: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}
