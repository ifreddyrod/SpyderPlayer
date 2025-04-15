// QtPlayer.cpp
#include "QtPlayer.h"
#include <QSignalMapper> 
#include <QThread> 
#include <QMediaMetaData>
#include "SpyderPlayerApp.h"
#include <QMetaEnum>

QtPlayer::QtPlayer(Ui::PlayerMainWindow* mainWindow, QWidget* parent) : VideoPlayer(parent) 
{
    mainWindow_ = mainWindow;
    app_ = static_cast<SpyderPlayerApp*>(parent);
    videoPanel_ = new QVideoWidget(mainWindow->VideoView_widget);

    subtitleCount_ = -1;
    subtitleIndex_ = -1;

    mainWindow->gridLayout->removeWidget(mainWindow->VideoView_widget);
    mainWindow->gridLayout->addWidget(videoPanel_, 1, 1, 1, 1);

    InitPlayer();

    connect(player_, &QMediaPlayer::durationChanged, this, &QtPlayer::PlayerDurationChanged);
    connect(player_, &QMediaPlayer::positionChanged, this, &QtPlayer::PlayerPositionChanged);
    connect(player_, &QMediaPlayer::playbackStateChanged, this, &QtPlayer::PlaybackStateChanged);
    connect(player_, &QMediaPlayer::mediaStatusChanged, this, &QtPlayer::MediaStatusChanged);
    connect(player_, &QMediaPlayer::errorOccurred, this, &QtPlayer::HandleError);
}

QtPlayer::~QtPlayer() 
{
    if (streamBuffer_) streamBuffer_->deleteLater();
    delete videoPanel_;
    delete player_;
    delete audioOutput_;
    delete watchdogTimer_;
    delete timeoutTimer_;
}

void QtPlayer::InitPlayer() 
{
    SetupPlayer();
    videoWidget_ = static_cast<QWidget*>(videoPanel_);
    timeoutTimer_ = new QTimer(this);
    connect(timeoutTimer_, &QTimer::timeout, this, &QtPlayer::CheckTimeout);
    StartWatchdog();
}

void QtPlayer::SetupPlayer() 
{
    if (player_) player_->deleteLater();
    player_ = new QMediaPlayer(this);
    audioOutput_ = new QAudioOutput(this);
    player_->setAudioOutput(audioOutput_);
    player_->setVideoOutput(videoPanel_);
    //player_->setNotifyInterval(100);
    player_->setPlaybackRate(1.0);

    connect(player_, &QMediaPlayer::durationChanged, this, &QtPlayer::PlayerDurationChanged);
    connect(player_, &QMediaPlayer::positionChanged, this, &QtPlayer::PlayerPositionChanged);
    connect(player_, &QMediaPlayer::playbackStateChanged, this, &QtPlayer::PlaybackStateChanged);
    connect(player_, &QMediaPlayer::mediaStatusChanged, this, &QtPlayer::MediaStatusChanged);
    connect(player_, &QMediaPlayer::errorOccurred, this, &QtPlayer::HandleError);
}

void QtPlayer::SetVideoSource(const std::string& videoSource) 
{
    source_ = videoSource;
    player_->stop();
    if (streamBuffer_) streamBuffer_->deleteLater();
    streamBuffer_ = nullptr;
    player_->setSource(QUrl(""));
    retryCount_ = 0;
    qDebug() << "QtPlayer: Set video source:" << QString::fromStdString(source_);
}

void QtPlayer::RefreshVideoSource() 
{
    PRINT << "REFRESH: Refreshing Video Source: " << source_;
    player_->stop();
    if (streamBuffer_) streamBuffer_->deleteLater();
    streamBuffer_ = nullptr;
    player_->setSource(QUrl(""));
    int timedelay = app_->GetRetryTimeDelay();
    PRINT << "REFRESH: Retry Time Delay: " << timedelay;
    QThread::msleep(timedelay);
    Play();
}

void QtPlayer::Play() 
{
    try {
        if (retryCount_ >= MAX_RETRIES) 
        {
            PRINT << "Max retries reached, aborting play.";
            ErrorOccured("Max retries reached");
            currentState_ = ENUM_PLAYER_STATE::STALLED;
            UpdatePlayerState(currentState_);
            return;
        }
        if (retryCount_ == 0)
        {
            currentState_ = ENUM_PLAYER_STATE::LOADING;
            UpdatePlayerState(currentState_);
        }

        QNetworkAccessManager *probe = new QNetworkAccessManager(this);
        QNetworkRequest request(QUrl(QString::fromStdString(source_)));
        request.setRawHeader("User-Agent", "Mozilla/5.0 (QtMediaPlayer)");
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        request.setSslConfiguration(sslConfig);
        auto *reply = probe->head(request);

        connect(reply, &QNetworkReply::finished, this, [this, reply, probe]() 
        {
            if (reply->error() == QNetworkReply::NoError) 
            {
                QUrl resolvedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                if (!resolvedUrl.isEmpty()) 
                {
                    PRINT << "Following redirect to: " << resolvedUrl.toString();
                    source_ = resolvedUrl.toString().toStdString();
                    Play();
                } 
                else 
                {
                    if (!streamBuffer_) 
                    {
                        player_->setSource(QUrl(QString::fromStdString(source_)));
                        player_->play();
                        retryCount_++;
                        qDebug() << "QtPlayer: Attempting direct playback, retry:" << retryCount_;
                    } 
                    else 
                    {
                        player_->setSourceDevice(streamBuffer_);
                        player_->play();
                        retryCount_++;
                        qDebug() << "QtPlayer: Using StreamBuffer, buffer size:" << streamBuffer_->bytesAvailable() << "retry:" << retryCount_;
                    }
                }
            } 
            else 
            {
                PRINT << "Probe failed: " << reply->errorString() << " Retry: " << retryCount_;
                if (retryCount_ < MAX_RETRIES) 
                {
                    retryCount_++;
                    QTimer::singleShot(1000 * retryCount_, this, &QtPlayer::Play);
                } 
                else 
                {
                    ErrorOccured("Probe failed: " + reply->errorString().toStdString());
                    currentState_ = ENUM_PLAYER_STATE::STALLED;
                    UpdatePlayerState(currentState_);
                }
            }
            reply->deleteLater();
            probe->deleteLater();
        });
    } 
    catch (const std::exception& e) 
    {    
        PRINT << "Play error: " << e.what();
        ErrorOccured(std::string(e.what()));
        if (retryCount_ < MAX_RETRIES) 
        {
            retryCount_++;
            QTimer::singleShot(1000 * retryCount_, this, &QtPlayer::Play);
        }
    }
}

void QtPlayer::Pause() 
{
    try 
    {
        player_->pause();
        timeoutTimer_->stop();
    } 
    catch (const std::exception& e) 
    {
        PRINT << "Pause error: " << e.what();
        ErrorOccured(std::string(e.what()));
    } 
}

void QtPlayer::Stop() 
{
    try 
    {
        player_->stop();
        timeoutTimer_->stop();
        if (streamBuffer_) streamBuffer_->deleteLater();
        streamBuffer_ = nullptr;
        retryCount_ = 0;
    } 
    catch (const std::exception& e) 
    {
        PRINT << "Stop error: " << e.what();
        ErrorOccured(std::string(e.what()));
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
    audioOutput_->setVolume(volume / 100.0);
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
    if (state == QMediaPlayer::PlayingState)
        currentState_ = ENUM_PLAYER_STATE::PLAYING;          
    else if (state == QMediaPlayer::PausedState)
        currentState_ = ENUM_PLAYER_STATE::PAUSED;
    else if (state == QMediaPlayer::StoppedState)
        currentState_ = ENUM_PLAYER_STATE::STOPPED;

    UpdatePlayerState(currentState_);
}

void QtPlayer::MediaStatusChanged(QMediaPlayer::MediaStatus mediaState) 
{
    PRINT << "[Player State] -- " << PlayerStateToString(currentState_);
    QMetaEnum metaEnum = QMetaEnum::fromType<QMediaPlayer::MediaStatus>();
    QString mediaStateStr = metaEnum.valueToKey(mediaState);
    PRINT << "[Media Status] -- " << mediaStateStr;
    mediaState_ = mediaState;

    if (currentState_ == ENUM_PLAYER_STATE::PAUSED)
        return;
    
    if (mediaState == QMediaPlayer::BufferingMedia && duration_ == 0)
        currentState_ = ENUM_PLAYER_STATE::LOADING;
    else if (mediaState == QMediaPlayer::LoadingMedia)
        currentState_ = ENUM_PLAYER_STATE::LOADING;
    else if (mediaState == QMediaPlayer::BufferedMedia) 
    {
        currentState_ = ENUM_PLAYER_STATE::PLAYING;
        timeoutTimer_->start(30000); // Increased to 30s
        retryCount_ = 0;

        QList<QMediaMetaData> subtitle_tracks = player_->subtitleTracks();
        PRINT << "Subtitle Count: " << subtitle_tracks.size();
        if (subtitleCount_ != subtitle_tracks.size()) 
        {
            subtitleCount_ = subtitle_tracks.size();
            if (subtitle_tracks.size() > 0) 
            {
                EnableSubtitles(true);
                subtitleList_.clear();
                subtitleList_.push_back({-1, "Disabled"});
                for (int index = 0; index < subtitle_tracks.size(); index++) 
                {
                    QString language = subtitle_tracks[index].value(QMediaMetaData::Language).toString();
                    PRINT << "Language: " << language;
                    if (language.isEmpty())
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
    else if (mediaState == QMediaPlayer::LoadedMedia) 
    {
        QList<QMediaMetaData> video_tracks = player_->videoTracks();
        QPair<int, int> highest_resolution = {0, 0};
        int highest_resolution_index = -1;

        for (int index = 0; index < video_tracks.size(); index++) 
        {
            QVariant resVar = video_tracks[index].value(QMediaMetaData::Resolution);
            if (resVar.isValid() && resVar.canConvert<QSize>()) 
            {
                QSize size = resVar.value<QSize>();
                int width = size.width();
                int height = size.height();
                PRINT << "Video track " << index << " resolution: " << width << "x" << height;
                if (width * height > highest_resolution.first * highest_resolution.second) 
                {
                    highest_resolution = {width, height};
                    highest_resolution_index = index;
                }
            }
        }

        PRINT << "Highest resolution index: " << highest_resolution_index;
        if (highest_resolution_index != -1 && currentState_ == ENUM_PLAYER_STATE::PLAYING)
            player_->setActiveVideoTrack(highest_resolution_index);
    } 
    else if (mediaState == QMediaPlayer::InvalidMedia) 
    {
        currentState_ = ENUM_PLAYER_STATE::STALLED;
        timeoutTimer_->stop();
    } 
    else if (mediaState == QMediaPlayer::EndOfMedia) 
    {
        timeoutTimer_->stop();
        if (duration_ > 0)
            currentState_ = ENUM_PLAYER_STATE::ENDED;
        else
            currentState_ = ENUM_PLAYER_STATE::STALLED;
    } 
    else if (mediaState == QMediaPlayer::NoMedia) 
    {
        currentState_ = ENUM_PLAYER_STATE::IDLE;
        timeoutTimer_->stop();
    } 
    else if (mediaState == QMediaPlayer::StalledMedia) 
    {
        currentState_ = ENUM_PLAYER_STATE::STALLED;
        timeoutTimer_->stop();
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

void QtPlayer::HandleError(QMediaPlayer::Error error, const QString &errorString) 
{
    PRINT << "!!!!---> QtPlayer Error: " << errorString << " (" << error << ") Retry: " << retryCount_;
    ErrorOccured(errorString.toStdString());
    if ((error == QMediaPlayer::ResourceError || error == QMediaPlayer::FormatError ) && retryCount_ < MAX_RETRIES) 
    {
        PRINT << "Stream error, retrying with " << (streamBuffer_ ? "direct playback" : "StreamBuffer") << "...";
        retryCount_++;
        player_->stop();
        if (streamBuffer_) 
        {
            streamBuffer_->deleteLater();
            streamBuffer_ = nullptr;
        } 
        else
        {
            streamBuffer_ = new StreamBuffer(QUrl(QString::fromStdString(source_)), this);
        }
        SetupPlayer();
        QTimer::singleShot(1000 * retryCount_, this, &QtPlayer::Play);
    } 
    else if (retryCount_ >= MAX_RETRIES) 
    {
        PRINT << "Max retries reached, stalling.";
        currentState_ = ENUM_PLAYER_STATE::STALLED;
        UpdatePlayerState(currentState_);
    }
}

void QtPlayer::list_video_tracks() 
{
}

void QtPlayer::OnChangingPosition(bool isPlaying) 
{
}

void QtPlayer::OnChangedPosition(bool isPlaying) 
{
}

void QtPlayer::ChangeUpdateTimerInterval(bool isFullScreen) 
{
}

void QtPlayer::CheckTimeout() 
{
    PRINT << "Stream timeout, buffer state: " << player_->bufferProgress() << "position: " << player_->position() << "Retry: " << retryCount_;
    if (player_->playbackState() == QMediaPlayer::PlayingState && player_->bufferProgress() > 0.5) 
    {
        qDebug() << "QtPlayer: Timeout ignored, playback active";
        return;
    }
    if (retryCount_ < MAX_RETRIES) 
    {
        retryCount_++;
        player_->stop();
        if (streamBuffer_) 
        {
            streamBuffer_->deleteLater();
            streamBuffer_ = nullptr;
        }
        SetupPlayer();
        Play();
    } 
    else 
    {
        PRINT << "Max retries reached, stalling.";
        currentState_ = ENUM_PLAYER_STATE::STALLED;
        UpdatePlayerState(currentState_);
    }
}

void QtPlayer::CheckPlaybackHealth() 
{
    static int stallCount = 0;
    if (currentState_ == ENUM_PLAYER_STATE::PLAYING && player_->position() == lastPosition_) 
    {
        stallCount++;
        PRINT << "Playback stalled, count: " << stallCount << "position: " << player_->position() << "Retry: " << retryCount_;
        if (stallCount >= 3 && retryCount_ < MAX_RETRIES) 
        {
            PRINT << "Playback stalled too long, restarting...";
            retryCount_++;
            player_->stop();
            if (streamBuffer_) 
            {
                streamBuffer_->deleteLater();
                streamBuffer_ = nullptr;
            }
            SetupPlayer();
            Play();
            stallCount = 0;
        }
    } 
    else 
    {
        stallCount = 0;
    }
    lastPosition_ = player_->position();
}

void QtPlayer::StartWatchdog()
{
    watchdogTimer_ = new QTimer(this);
    connect(watchdogTimer_, &QTimer::timeout, this, &QtPlayer::CheckPlaybackHealth);
    watchdogTimer_->start(5000);
}
