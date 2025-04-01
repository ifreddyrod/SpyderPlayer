#include "QtPlayer.h"
#include <QSignalMapper> 
#include <QThread> 
#include <QMediaMetaData>

QtPlayer::QtPlayer(Ui::PlayerMainWindow* mainWindow, QWidget* parent) : VideoPlayer(parent) 
{
    mainWindow_ = mainWindow;
    videoPanel_ = new QVideoWidget(mainWindow->VideoView_widget);

    subtitleCount_ = -1;
    subtitleIndex_ = -1;

    // Create and add QVideoWidget to the main window
    mainWindow->gridLayout->removeWidget(mainWindow->VideoView_widget);
    mainWindow->gridLayout->addWidget(videoPanel_, 1, 1, 1, 1);

    InitPlayer();

    // Init Signals
    connect(player_, &QMediaPlayer::durationChanged, this, &QtPlayer::PlayerDurationChanged);
    connect(player_, &QMediaPlayer::positionChanged, this, &QtPlayer::PlayerPositionChanged);
    connect(player_, &QMediaPlayer::playbackStateChanged, this, &QtPlayer::PlaybackStateChanged);
    connect(player_, &QMediaPlayer::mediaStatusChanged, this, &QtPlayer::MediaStatusChanged);
}

QtPlayer::~QtPlayer() 
{
    delete videoPanel_;
    delete player_;
    delete audioOutput_;
}

void QtPlayer::InitPlayer() 
{
    // Initialize player
    player_ = new QMediaPlayer;
    audioOutput_ = new QAudioOutput;
    player_->setAudioOutput(audioOutput_);
    player_->setVideoOutput(videoPanel_);

    videoWidget_ = static_cast<QWidget*>(videoPanel_);
}

void QtPlayer::SetVideoSource(const std::string& videoSource) 
{
    source_ = videoSource;
    player_->setSource(QUrl(QSTR(source_)));
    subtitleCount_ = -1;
    subtitleIndex_ = -1;
    subtitleList_.clear();
}

void QtPlayer::RefreshVideoSource() 
{
    player_->setSource(QUrl(QSTR(source_)));
    QThread::msleep(1000);
    player_->setSource(QUrl(QSTR(source_)));
}

void QtPlayer::Play() 
{
    try
    {
        player_->play();
    }
    catch(const std::exception& e)
    {    
        player_->stop();
        ErrorOccured(std::string(e.what()));
        PRINT << e.what();
    }
}

void QtPlayer::Pause() 
{
    try
    {
        player_->pause();
    }
    catch(const std::exception& e)
    {
        ErrorOccured(std::string(e.what()));
        PRINT << e.what();
    } 
}

void QtPlayer::Stop() 
{
    try
    {
        player_->stop();
    }
    catch(const std::exception& e)
    {
        ErrorOccured(std::string(e.what()));
        PRINT << e.what();
    } 
}

void QtPlayer::SetPosition(qint64 position) 
{
    player_->setPosition(position);
    position_ = position;
}

qint64 QtPlayer::GetPosition() 
{
    position_ = player_->position();
    return position_;
}

qint64 QtPlayer::GetVideoDuration() 
{
    return duration_;
}

void QtPlayer::SetVolume(int volume) 
{
    audioOutput_->setVolume(volume/100.0);
}

int QtPlayer::GetVolume() 
{
    return int(audioOutput_->volume() * 100);
}

bool QtPlayer::IsMuted() 
{
    return audioOutput_->isMuted();
}

void QtPlayer::Mute(bool mute) 
{
    audioOutput_->setMuted(mute);
}

void QtPlayer::PlayerDurationChanged(int duration) 
{
    duration_ = duration;
    UpdateDuration(duration);
}

void QtPlayer::PlayerPositionChanged(int position) 
{
    position_ = position;
    UpdatePosition(position);
}

ENUM_PLAYER_STATE QtPlayer::GetPlayerState() 
{
    return currentState_;
}

void QtPlayer::PlaybackStateChanged(QMediaPlayer::PlaybackState state) 
{
    if (state == QMediaPlayer::PlaybackState::PlayingState)
        currentState_ = ENUM_PLAYER_STATE::PLAYING;          
    else if (state == QMediaPlayer::PlaybackState::PausedState)
        currentState_ = ENUM_PLAYER_STATE::PAUSED;
    else if (state == QMediaPlayer::PlaybackState::StoppedState)
        currentState_ = ENUM_PLAYER_STATE::STOPPED;

    UpdatePlayerState(currentState_);
}

void QtPlayer::MediaStatusChanged(QMediaPlayer::MediaStatus mediaState) 
{
    PRINT << "[Player State] -- " << PlayerStateToString(currentState_);
    PRINT << "[Media Status] -- " << qt_getEnumName(mediaState);

    if (currentState_ == ENUM_PLAYER_STATE::PAUSED)
        return;
    
    if (mediaState == QMediaPlayer::MediaStatus::BufferingMedia && duration_ == 0)
        currentState_ = ENUM_PLAYER_STATE::LOADING;
    else if (mediaState == QMediaPlayer::MediaStatus::LoadingMedia)
        currentState_ = ENUM_PLAYER_STATE::LOADING;
    else if (mediaState == QMediaPlayer::MediaStatus::BufferedMedia)
    {
        currentState_ = ENUM_PLAYER_STATE::PLAYING;

        // Get Subtitle info
        QList <QMediaMetaData> subtitle_tracks = player_->subtitleTracks();
        if (subtitleCount_ != subtitle_tracks.size())
        {
            subtitleCount_ = subtitle_tracks.size();
            if (subtitle_tracks.size() > 0)  
            {
                //mainWindow_->subtitlesEnabled = true; 
                EnableSubtitles(true);              
                subtitleList_.clear();
                subtitleList_.push_back({-1, "Disabled"});
                for (int index = 0; index < subtitle_tracks.size(); index++)
                {
                    QString language = subtitle_tracks[index].value(QMediaMetaData::Language).toString();
                    if (language.isEmpty())  // Fallback if no language is specified
                        language = QString("Track %1").arg(index + 1);
                    subtitleList_.push_back({index, language});
                }
            }
            else
            {
                EnableSubtitles(false);
            }
        }
    }
    else if (mediaState == QMediaPlayer::MediaStatus::LoadedMedia)
    {
        // Get Available Video Tracks
        QList <QMediaMetaData> video_tracks = player_->videoTracks();
        QPair<int, int> highest_resolution = {0, 0};
        int highest_resolution_index = -1;

        // Iterate over available tracks
        for (int index = 0; index < video_tracks.size(); index++)
        {
            QString resolution = video_tracks[index].value(QMediaMetaData::Resolution).toString();
            if (!resolution.isEmpty())
            {
                int width = resolution.split("x")[0].toInt();
                int height = resolution.split("x")[1].toInt();
                PRINT << "Video track " << index << " resolution: " << width << "x" << height;
                if (width * height > highest_resolution.first * highest_resolution.second)
                {
                    highest_resolution = {width, height};
                    highest_resolution_index = index;
                }
            }
        }

        // Select the highest resolution track
        if (highest_resolution_index != -1)
            player_->setActiveVideoTrack(highest_resolution_index);

        if (currentState_ == ENUM_PLAYER_STATE::PLAYING)
            currentState_ = ENUM_PLAYER_STATE::PLAYING;
    }
    else if (mediaState == QMediaPlayer::MediaStatus::InvalidMedia)
    {
        currentState_ = ENUM_PLAYER_STATE::STALLED;
    }
    else if (mediaState == QMediaPlayer::MediaStatus::EndOfMedia)
    {
        if (duration_ > 0)
            currentState_ = ENUM_PLAYER_STATE::ENDED;
        else 
            currentState_ = ENUM_PLAYER_STATE::STOPPED;
    }
    else if (mediaState == QMediaPlayer::MediaStatus::NoMedia)
    {
        currentState_ = ENUM_PLAYER_STATE::IDLE;
    }
    else if (mediaState == QMediaPlayer::MediaStatus::StalledMedia)
    {
        currentState_ = ENUM_PLAYER_STATE::STALLED;
    }

    UpdatePlayerState(currentState_);
    
}

QList<QPair<int, QString>> QtPlayer::GetSubtitleTracks() 
{
    return subtitleList_;
}

void QtPlayer::SetSubtitleTrack(int index) 
{
    subtitleIndex_ = index;
    player_->setActiveSubtitleTrack(index);
}

QString QtPlayer::GetVideoResolution() 
{
    // Get resolution from metadata
    QMediaMetaData metadata = player_->metaData();
    QVariant resolutionVar = metadata.value(QMediaMetaData::Resolution);
    
    QString res_str;
    if (resolutionVar.isValid() && resolutionVar.canConvert<QSize>()) 
    {
        QSize resolution = resolutionVar.value<QSize>();
        if (resolution.isValid()) 
        {
            res_str = QString("%1x%2").arg(resolution.width()).arg(resolution.height());
        } 
        else 
        {
            res_str = "Unknown";
        }
    } 
    else 
    {
        res_str = "Unknown";
    }
    
    return res_str;
}


void QtPlayer::list_video_tracks() 
{
    // List video tracks
}

void QtPlayer::OnChangingPosition(bool isPlaying) 
{
    ; // QTPlayer does not need this
}

void QtPlayer::OnChangedPosition(bool isPlaying) 
{
    ; // QTPlayer does not need this
}

void QtPlayer::ChangeUpdateTimerInterval(bool isFullScreen) 
{
    ; // QTPlayer does not need this
}