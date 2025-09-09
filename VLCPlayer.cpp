// VLCPlayer.cpp
#include "VLCPlayer.h"
#include "SpyderPlayerApp.h"
#include <QDebug>
#include <QThread>
#include <QApplication>

VLCPlayer::VLCPlayer(Ui::PlayerMainWindow* mainWindow, QWidget* parent)
    : VideoPlayer(parent)
{
    mainWindow_ = mainWindow;
    app_ = static_cast<SpyderPlayerApp*>(parent);
    videoPanel_ = new QWidget(mainWindow->VideoView_widget->parentWidget());

    subtitleCount_ = -1;
    subtitleIndex_ = -1;

    mainWindow->gridLayout->removeWidget(mainWindow->VideoView_widget);
    mainWindow->gridLayout->addWidget(videoPanel_, 1, 1, 1, 1);

    InitPlayer();

    stalledVideoTimer_ = new QTimer(this);
    stalledVideoTimer_->setInterval(3000);
    connect(stalledVideoTimer_, &QTimer::timeout, this, &VLCPlayer::StalledVideoDetected);
}

VLCPlayer::~VLCPlayer()
{
    if (mediaPlayer_) 
    {
        libvlc_media_player_stop(mediaPlayer_);
        libvlc_media_player_release(mediaPlayer_);
    }
    if (media_) 
    {
        libvlc_media_release(media_);
    }
    if (vlcInstance_) 
    {
        libvlc_release(vlcInstance_);
    }
    delete videoPanel_;
    delete positionTimer_;
    delete watchdogTimer_;
    delete timeoutTimer_;
    delete stalledVideoTimer_;
}

void VLCPlayer::InitPlayer()
{
    SetupPlayer();
    videoWidget_ = videoPanel_;
    timeoutTimer_ = new QTimer(this);
    connect(timeoutTimer_, &QTimer::timeout, this, &VLCPlayer::CheckTimeout);

    positionTimer_ = new QTimer(this);
    connect(positionTimer_, &QTimer::timeout, this, &VLCPlayer::UpdatePositionSlot);
    positionTimer_->start(250);  // Update position every 250ms
}

void VLCPlayer::SetupPlayer()
{
    try {
        // VLC initialization arguments
        const char* const vlc_args[] = 
        {
            "--verbose=2",  // For debugging, adjust as needed
            "--no-xlib",    // Disable Xlib for better compatibility
            "--network-caching=1000",  // Cache for streaming
            "--file-caching=1000",
            "--live-caching=1000",
            "--drop-late-frames",
            "--skip-frames"
        };

        vlcInstance_ = libvlc_new(sizeof(vlc_args) / sizeof(vlc_args[0]), vlc_args);
        if (!vlcInstance_) 
        {
            throw std::runtime_error("Failed to create VLC instance");
        }

        mediaPlayer_ = libvlc_media_player_new(vlcInstance_);
        if (!mediaPlayer_) 
        {
            throw std::runtime_error("Failed to create VLC media player");
        }

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
        qDebug() << e.what();
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
    switch (event->type) {
    case libvlc_MediaPlayerPlaying:
        self->UpdatePlayerState(ENUM_PLAYER_STATE::PLAYING);
        break;
    case libvlc_MediaPlayerPaused:
        self->UpdatePlayerState(ENUM_PLAYER_STATE::PAUSED);
        break;
    case libvlc_MediaPlayerStopped:
        self->UpdatePlayerState(ENUM_PLAYER_STATE::STOPPED);
        break;
    case libvlc_MediaPlayerEndReached:
        self->UpdatePlayerState(ENUM_PLAYER_STATE::STOPPED);
        break;
    case libvlc_MediaPlayerEncounteredError:
        self->UpdatePlayerState(ENUM_PLAYER_STATE::ERROR);
        self->ErrorOccured("VLC encountered an error");
        break;
    case libvlc_MediaPlayerBuffering:
        self->playerStatus_ = "Buffering: " + QString::number(event->u.media_player_buffering.new_cache) + "%";
        break;
    case libvlc_MediaPlayerLengthChanged:
        self->UpdateDuration(libvlc_media_player_get_length(self->mediaPlayer_));
        break;
    case libvlc_MediaPlayerTimeChanged:
        self->UpdatePosition(libvlc_media_player_get_time(self->mediaPlayer_));
        break;
    default:
        break;
    }
    QApplication::processEvents();  // Ensure Qt processes events
}

void VLCPlayer::SetVideoSource(const std::string& videoSource)
{
    try 
    {
        source_ = videoSource;
        if (media_) 
        {
            libvlc_media_release(media_);
            media_ = nullptr;
        }
        media_ = libvlc_media_new_location(vlcInstance_, videoSource.c_str());
        if (!media_) 
        {
            throw std::runtime_error("Failed to create VLC media");
        }
        libvlc_media_player_set_media(mediaPlayer_, media_);
    } 
    catch (const std::exception& e) 
    {
        qDebug() << "SetVideoSource: " << e.what();
    }
}

void VLCPlayer::RefreshVideoSource()
{
    try 
    {
        qDebug() << "REFRESH: Refreshing Video Source: " << QString::fromStdString(source_);

        SetVideoSource(source_);
        uint64_t timedelay = app_->GetRetryTimeDelay();
        timedelay = timedelay * stallretryCount_;
        qDebug() << "REFRESH: Retry Time Delay: " << timedelay;
        QTimer::singleShot(timedelay, this, &VLCPlayer::PlaySource);

        playerStatus_ += " in " + QString::number(timedelay / 1000.0, 'f', 1) + " secs";
    } 
    catch (const std::exception& e) 
    {
        qDebug() << "RefreshVideoSource: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

void VLCPlayer::Play()
{
    stopAll_ = false;
    inRecovery_ = false;
    stallretryCount_ = 0;
    retryCount_ = 0;

    PlaySource();
}

void VLCPlayer::PlaySource()
{
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
            timeoutTimer_->start(30000);
            retryCount_ = 0;
            stallretryCount_ = 0;
            Mute(isMuted_);
            return;
        } 
        else if (retryCount_ <= MAX_RETRIES) 
        {
            Mute(isMuted_);
            libvlc_media_player_play(mediaPlayer_);
            return;
        }
    } 
    catch (const std::exception& e) 
    {
        qDebug() << "Play error: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

void VLCPlayer::Pause()
{
    if (libvlc_media_player_is_playing(mediaPlayer_)) 
    {
        libvlc_media_player_pause(mediaPlayer_);
    }
}

void VLCPlayer::Stop()
{
    libvlc_media_player_stop(mediaPlayer_);
    stopAll_ = true;
}

void VLCPlayer::SetPosition(qint64 position)
{
    libvlc_media_player_set_time(mediaPlayer_, position);
}

void VLCPlayer::SkipPosition(qint64 position)
{
    qint64 newPos = GetPosition() + position;
    SetPosition(newPos);
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
    if (isPlaying) 
    {
        stalledVideoTimer_->stop();
    }
    isPositionSeeking_ = true;
}

void VLCPlayer::OnChangedPosition(bool isPlaying)
{
    if (isPlaying) 
    {
        timeoutTimer_->start();
        stalledVideoTimer_->start();
        isPositionSeeking_ = false;
    }
}

void VLCPlayer::ChangeUpdateTimerInterval(bool isFullScreen)
{
    // Adjust positionTimer_ interval if needed, e.g., for fullscreen
    positionTimer_->setInterval(isFullScreen ? 100 : 250);
}

void VLCPlayer::UpdatePositionSlot()
{
    if (currentState_ == ENUM_PLAYER_STATE::PLAYING) 
    {
        qint64 pos = GetPosition();
        UpdatePosition(pos);
    }
}

void VLCPlayer::CheckTimeout()
{
    if (duration_ > 0) return;

    qDebug() << "Stream timeout, position: " << GetPosition() << "Retry: " << retryCount_;

    if (libvlc_media_player_is_playing(mediaPlayer_)) 
    {
        qDebug() << "VLCPlayer: Timeout ignored, playback active";
        return;
    }
    if (retryCount_ < MAX_RETRIES) 
    {
        qDebug() << "Stream stopped, retrying...";
        retryCount_++;
        stallPosition_ = position_;
        Stop();
        ReConnectPlayer();
    } 
    else 
    {
        qDebug() << "Max retries reached, stalling.";
        currentState_ = ENUM_PLAYER_STATE::STALLED;
        UpdatePlayerState(currentState_);
    }
}

void VLCPlayer::CheckPlaybackHealth()
{
    static int stallCount = 0;
    qint64 currentPos = GetPosition();
    if (currentState_ == ENUM_PLAYER_STATE::PLAYING && currentPos == lastPosition_) 
    {
        stallCount++;
        qDebug() << "Playback stalled, count: " << stallCount << "position: " << currentPos << "Retry: " << retryCount_;

        if (stallCount >= 3 && retryCount_ < MAX_RETRIES) 
        {
            qDebug() << "Playback stalled too long, restarting...";
            retryCount_++;
            Stop();
            SetupPlayer();
            QTimer::singleShot(2000, this, &VLCPlayer::PlaySource);
            stallCount = 0;
        }
    } 
    else 
    {
        stallCount = 0;
    }
    lastPosition_ = currentPos;
}

void VLCPlayer::StalledVideoDetected()
{
    if (duration_ > 0) return;

    if (libvlc_media_player_is_playing(mediaPlayer_) && stallretryCount_ < MAX_STALL_RETRIES) 
    {
        qDebug() << "Stalled video detected, attempting to retry...";
        stallPosition_ = position_;
        Stop();
        RetryStalledPlayer();
    } 
    else if (isPositionSeeking_) 
    {
        stallretryCount_ = 0;
        inRecovery_ = false;
        return;
    } 
    else 
    {
        RetryStalledPlayer();
    }
}

void VLCPlayer::ReConnectPlayer()
{
    if (stopAll_) 
    {
        inRecovery_ = false;
        return;
    }

    try 
    {
        if (retryCount_ < MAX_RETRIES) 
        {
            inRecovery_ = true;
            timeoutTimer_->stop();
            stalledVideoTimer_->stop();
            retryCount_++;
            playerStatus_ = "Connection attempt (" + QString::number(retryCount_) + " of " + QString::number(MAX_RETRIES) + ")";
            qDebug() << playerStatus_;
            Stop();
            SetupPlayer();
            SetVideoSource(source_);
            QTimer::singleShot(250, this, &VLCPlayer::PlaySource);
        } 
        else 
        {
            Stop();
            timeoutTimer_->stop();
            stalledVideoTimer_->stop();
            playerStatus_ = "Max Reconnection attempts reached";
            qDebug() << playerStatus_;
            RetryStalledPlayer();
        }
    } 
    catch (const std::exception& e) 
    {
        qDebug() << "Reconnect error: " << e.what();
        ErrorOccured(std::string(e.what()));
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
        if (stallretryCount_ < MAX_STALL_RETRIES) 
        {
            inRecovery_ = true;
            stallretryCount_++;
            qDebug() << "-----------------> STALLED VIDEO DETECTED <-----------------";
            playerStatus_ = "Recovering (" + QString::number(stallretryCount_) + " of " + QString::number(MAX_STALL_RETRIES) + ")";
            qDebug() << playerStatus_;

            Stop();
            SetupPlayer();
            RefreshVideoSource();
        } 
        else 
        {
            playerStatus_ = "Max Recovery attempts reached";
            qDebug() << playerStatus_;
            currentState_ = ENUM_PLAYER_STATE::ERROR;
            inRecovery_ = false;
            UpdatePlayerState(currentState_);
        }
    } 
    catch (const std::exception& e) 
    {
        qDebug() << "Retry error: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}