// QtPlayer.cpp
#include "QtPlayer.h"
#include <QSignalMapper> 
#include <QThread> 
#include <QMediaMetaData>
#include "SpyderPlayerApp.h"
#include <QMetaEnum>

QtPlayer::QtPlayer(Ui::PlayerMainWindow* mainWindow, QWidget* parent)
    : VideoPlayer(parent)
{
    mainWindow_ = mainWindow;
    app_ = static_cast<SpyderPlayerApp*>(parent);
    videoPanel_ = new QVideoWidget(mainWindow->VideoView_widget);

    subtitleCount_ = -1;
    subtitleIndex_ = -1;

    mainWindow->gridLayout->removeWidget(mainWindow->VideoView_widget);
    mainWindow->gridLayout->addWidget(videoPanel_, 1, 1, 1, 1);

    InitPlayer();

    stalledVideoTimer_ = new QTimer(this);
    stalledVideoTimer_->setInterval(3000);
    connect(stalledVideoTimer_, &QTimer::timeout, this, &QtPlayer::StalledVideoDetected);
}

QtPlayer::~QtPlayer()
{
    /*if (streamBuffer_)
    {
        streamBuffer_->Stop();
        streamBuffer_->deleteLater();
    }*/
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
    //StartWatchdog();
}

void QtPlayer::SetupPlayer()
{
    try
    {
        if (player_) 
        {
            disconnect(player_, nullptr, this, nullptr); // Disconnect all signals
            player_->deleteLater();
            player_ = nullptr;
        }
        if (audioOutput_) 
        {
            disconnect(audioOutput_, nullptr, this, nullptr);
            audioOutput_->deleteLater();
            audioOutput_ = nullptr;
        }
        // Delete and recreate video widget
        if (videoPanel_)
        {
            mainWindow_->gridLayout->removeWidget(videoPanel_);
            videoPanel_->deleteLater();
            videoPanel_ = nullptr;
        }
        videoPanel_ = new QVideoWidget(mainWindow_->VideoView_widget->parentWidget());
        mainWindow_->gridLayout->addWidget(videoPanel_, 1, 1, 1, 1);
        videoWidget_ = static_cast<QWidget*>(videoPanel_);
        
        MAX_STALL_RETRIES = app_->GetMaxRetryCount();
        player_ = new QMediaPlayer(this);
        audioOutput_ = new QAudioOutput(this);
        player_->setAudioOutput(audioOutput_);
        player_->setVideoOutput(videoPanel_);
        player_->setPlaybackRate(1.0);
        connect(player_, &QMediaPlayer::durationChanged, this, &QtPlayer::PlayerDurationChanged);
        connect(player_, &QMediaPlayer::positionChanged, this, &QtPlayer::PlayerPositionChanged);
        connect(player_, &QMediaPlayer::playbackStateChanged, this, &QtPlayer::PlaybackStateChanged);
        connect(player_, &QMediaPlayer::mediaStatusChanged, this, &QtPlayer::MediaStatusChanged);
        connect(player_, &QMediaPlayer::errorOccurred, this, &QtPlayer::HandleError);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ErrorOccured(std::string(e.what()));
    }
}

void QtPlayer::ResetAudioOutput()
{
    try
    {
        audioOutput_->deleteLater();
        audioOutput_ = new QAudioOutput(this);
        player_->setAudioOutput(audioOutput_);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        ErrorOccured(std::string(e.what()));
    }
}

void QtPlayer::SetVideoSource(const std::string& videoSource)
{
    try
    {
        source_ = videoSource;
        if (player_)
        {
            if (!player_->isPlaying())
                player_->setSource(QUrl(""));
        }
        else
        {
            SetupPlayer();
            SetVideoSource(source_);
        }
    }
    catch(const std::exception& e)
    {
        PRINT << e.what() << '\n';
        ErrorOccured(std::string(e.what()));
    }
}

void QtPlayer::RefreshVideoSource()
{
    try
    {
        PRINT << "REFRESH: Refreshing Video Source: " << source_;

        SetVideoSource(source_);
        u_int64_t timedelay = app_->GetRetryTimeDelay();
        //PRINT << "REFRESH: Retry Time Delay: " << timedelay;
        //QTimer::singleShot(timedelay, this, &QtPlayer::Play);
        timedelay = timedelay*stallretryCount_;
        PRINT << "REFRESH: Retry Time Delay: " << timedelay;
        QTimer::singleShot(timedelay, this, &QtPlayer::PlaySource);

        // Get timedelay string in seconds for 1 decimal place
        playerStatus_ += " in " + QString::number(timedelay/1000.0, 'f', 1) + " secs";
    }
    catch(const std::exception& e)
    {
        PRINT << e.what() << '\n';
        ErrorOccured(std::string(e.what()));
    }
}

void QtPlayer::Play()
{
    stopAll_ = false;
    inRecovery_ = false;
    PlaySource();
}

void QtPlayer::PlaySource()
{
/*    try 
    {
        // Paused then Resume Playback
        if (currentState_ == ENUM_PLAYER_STATE::PAUSED)
        {
            ResetAudioOutput();
            audioOutput_->setMuted(false);
            player_->play();
            timeoutTimer_->start(30000);
            retryCount_ = 0;
            stallretryCount_ = 0;
            audioOutput_->setMuted(isMuted_);
            return;
        }
        else if (retryCount_ <= MAX_RETRIES)
        {
            audioOutput_->setMuted(isMuted_);
            //PRINT << "Play: " << source_ << " Retry: " << retryCount_;
            player_->setSource(QUrl(QString::fromStdString(source_)));
            player_->play();
            return;
        }
        else
            return;
    }
    catch (exception& e)
    {
        PRINT << "Play error: " << e.what();
        ErrorOccured(std::string(e.what()));
        return;
    }*/
    
    ////////
    if (stopAll_)
    {
        inRecovery_ = false;
        return;
    }

    try
    {
        if (currentState_ == ENUM_PLAYER_STATE::PAUSED)
        {
            PRINT << "Resuming Playback";
            //ResetAudioOutput();
            audioOutput_->setMuted(false);
            player_->play();
            timeoutTimer_->start(30000);
            retryCount_ = 0;
            stallretryCount_ = 0;
            audioOutput_->setMuted(isMuted_);
            if (duration_ > 0 && position_ > 3000)
                SkipPosition(position_ - 3000);
            return;
        }
        //else if (retryCount_ >= MAX_RETRIES)
        else if (stallretryCount_ >= MAX_STALL_RETRIES)
        {
            PRINT << "Max retries reached, aborting play.";
            ErrorOccured("Max retries reached");
            currentState_ = ENUM_PLAYER_STATE::STALLED;
            UpdatePlayerState(currentState_);
            return;
        }
        // Attempt to play remote URL
        else if (QSTR(source_).startsWith("http://") || QSTR(source_).startsWith("https://"))
        {
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
                        PlaySource();
                    }
                    else
                    {
                        player_->setSource(QUrl(QSTR(source_)));
                            
                        audioOutput_->setMuted(isMuted_);
                        player_->play();
                        if (duration_ > 0 && position_ > 3000)
                            SkipPosition(position_ - 3000);
                        //retryCount_++;
                        PRINT << "QtPlayer: Attempting direct playback...";  
                    }
                }
                else
                {
                    PRINT << "Probe failed: " << reply->errorString() << " Retry: " << retryCount_;
                    if (reply->error() == QNetworkReply::ContentNotFoundError)
                    {
                        playerStatus_ = "URL not found (404): Retrying(" + QString::number(stallretryCount_+1) + ")";
                        PRINT << playerStatus_;
                        ErrorOccured(STR(playerStatus_));
                        RetryStalledPlayer();
                    }
                    else 
                    {
                        ReConnectPlayer();
                    }
                    UpdatePlayerState(ENUM_PLAYER_STATE::RECOVERING);
                    /*else
                    {
                        ErrorOccured("Probe failed: " + reply->errorString().toStdString());
                        currentState_ = ENUM_PLAYER_STATE::STALLED;
                        UpdatePlayerState(currentState_);
                    }*/
                }
                reply->deleteLater();
                probe->deleteLater();
            });
        }
        // Attempt to play local file
        else
        {
            player_->setSource(QUrl(QString::fromStdString(source_)));
            //if (duration_ > 0)
                //player_->setPosition(position_);
                
            audioOutput_->setMuted(isMuted_);
            player_->play();
            if (duration_ > 0 && position_ > 3000)
                SkipPosition(position_ - 3000);

            //retryCount_++;
            PRINT << "QtPlayer: Attempting direct playback...";  
        }
    }
    catch (const std::exception& e)
    {    
        PRINT << "Play error: " << e.what();
        ErrorOccured(std::string(e.what()));
        ReConnectPlayer();
        /*if (retryCount_ < MAX_RETRIES)
        {
            //retryCount_++;
            QTimer::singleShot(2000 * retryCount_, this, &QtPlayer::ReConnectPlayer);
        }*/
       
    }
}

void QtPlayer::Pause()
{
    try
    {
        stallPosition_ = position_;
        player_->pause();
        timeoutTimer_->stop();
        inRecovery_ = false;
        //watchdogTimer_->stop();
        stalledVideoTimer_->stop();
        currentState_ = ENUM_PLAYER_STATE::PAUSED;
        UpdatePlayerState(currentState_);
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
        stopAll_ = true;
        player_->stop();
        timeoutTimer_->stop();
        inRecovery_ = false;
        stallPosition_ = 0;
        stalledVideoTimer_->stop();
        retryCount_ = 0;
        stallretryCount_ = 0;
        lastPosition_ = 0;
        subtitleCount_ = -1;
        subtitleIndex_ = 0;
        duration_ = 0;
        currentState_ = ENUM_PLAYER_STATE::STOPPED;
        UpdatePlayerState(currentState_);
    }
    catch (const std::exception& e)
    {
        PRINT << "Stop error: " << e.what();
        ErrorOccured(std::string(e.what()));
        stopAll_ = true;
        inRecovery_ = false;
    }
}

void QtPlayer::SetPosition(qint64 position)
{
    player_->setPosition(position);
    position_ = position;
}

void QtPlayer::SkipPosition(qint64 position)
{
    //watchdogTimer_->stop();
    timeoutTimer_->stop();
    stalledVideoTimer_->stop();

    SetPosition(position);
    UpdatePosition(position);

    //watchdogTimer_->start();
    timeoutTimer_->start();    
    stalledVideoTimer_->start();
}

qint64 QtPlayer::GetPosition()
{
    try
    {
        position_ = player_->position();
        return position_; 
    }
    catch(const std::exception& e)
    {
        PRINT << "GetPosition error: " << e.what();
        return 0;
    }
}

qint64 QtPlayer::GetVideoDuration()
{
    return duration_;
}

void QtPlayer::SetVolume(int volume)
{
    try
    {
       audioOutput_->setVolume(volume / 100.0);
    }
    catch(const std::exception& e)
    {
        PRINT << "SetVolume error: " << e.what();
    }
}

int QtPlayer::GetVolume()
{
    try
    {
       return int(audioOutput_->volume() * 100);
    }
    catch(const std::exception& e)
    {
        PRINT << "GetVolume error: " << e.what();
        return 0;
    }    
}

bool QtPlayer::IsMuted()
{
    try
    {
        return audioOutput_->isMuted();
    }
    catch(const std::exception& e)
    {
        PRINT << "IsMuted error: " << e.what();
        return false;
    }
}

void QtPlayer::Mute(bool mute)
{
    try
    {
        isMuted_ = mute;
        audioOutput_->setMuted(isMuted_);
    }
    catch(const std::exception& e)
    {
        PRINT << "Muted error: " << e.what();
        isMuted_ = false;
    }
}

void QtPlayer::PlayerDurationChanged(int duration)
{
    if (inRecovery_) return;

    duration_ = duration;
    UpdateDuration(duration);
}

void QtPlayer::PlayerPositionChanged(int position)
{
    if (inRecovery_) 
    {
        stallPosition_ = position_;
        return;
    }

    position_ = position;
    UpdatePosition(position);
    if (!isPositionSeeking_) stalledVideoTimer_->start();
}

ENUM_PLAYER_STATE QtPlayer::GetPlayerState()
{
    return currentState_;
}

void QtPlayer::PlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    if (inRecovery_)
        currentState_ = ENUM_PLAYER_STATE::RECOVERING;
    else if (state == QMediaPlayer::PlayingState)
        currentState_ = ENUM_PLAYER_STATE::PLAYING;
    else if (state == QMediaPlayer::PausedState)
        currentState_ = ENUM_PLAYER_STATE::PAUSED;
    else if (state == QMediaPlayer::StoppedState)
        currentState_ = ENUM_PLAYER_STATE::STOPPED;

    UpdatePlayerState(currentState_);
}

void QtPlayer::MediaStatusChanged(QMediaPlayer::MediaStatus mediaState)
{
    try
    {
        PRINT << "[Player State] -- " << PlayerStateToString(currentState_);
        QMetaEnum metaEnum = QMetaEnum::fromType<QMediaPlayer::MediaStatus>();
        QString mediaStateStr = metaEnum.valueToKey(mediaState);
        PRINT << "[Media Status] -- " << mediaStateStr;
        mediaState_ = mediaState;

        if (stopAll_)
        {
            currentState_ = ENUM_PLAYER_STATE::STOPPED;
            UpdatePlayerState(currentState_);
            return;
        }

        if (currentState_ == ENUM_PLAYER_STATE::PAUSED)
            return;
        
        if (mediaState == QMediaPlayer::BufferingMedia && duration_ == 0)
        {
            currentState_ = ENUM_PLAYER_STATE::LOADING;
        }
        else if (mediaState == QMediaPlayer::LoadingMedia)
        {
            currentState_ = ENUM_PLAYER_STATE::LOADING;
        }

        else if (mediaState == QMediaPlayer::BufferedMedia)
        {
            currentState_ = ENUM_PLAYER_STATE::PLAYING;
            playerStatus_ = "";

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
            
            if (duration_ > 0 && stallPosition_ > 0)
            {
                PRINT << "ReSetting previous position: " << stallPosition_;
                SetPosition(stallPosition_);
            }

            timeoutTimer_->start(30000);
            retryCount_ = 0;
            stallretryCount_ = 0;
            inRecovery_ = false;
            playerStatus_ = "";
        }
        else if (mediaState == QMediaPlayer::InvalidMedia)
        {
            currentState_ = ENUM_PLAYER_STATE::NOMEDIA;
            timeoutTimer_->stop();
            stalledVideoTimer_->stop();
        }
        else if (mediaState == QMediaPlayer::EndOfMedia)
        {
            timeoutTimer_->stop();
            stalledVideoTimer_->stop();
            if (duration_ > 0)
            {
                currentState_ = ENUM_PLAYER_STATE::ENDED;
            }
            else
            {
                currentState_ = ENUM_PLAYER_STATE::STALLED;
                if (!inRecovery_)
                    QTimer::singleShot(1000, this, &QtPlayer::RetryStalledPlayer);
            }
        }
        else if (mediaState == QMediaPlayer::NoMedia)
        {
            currentState_ = ENUM_PLAYER_STATE::NOMEDIA;
            timeoutTimer_->stop();
            stalledVideoTimer_->stop();
        }
        else if (mediaState == QMediaPlayer::StalledMedia and !isPositionSeeking_)
        {
            currentState_ = ENUM_PLAYER_STATE::STALLED;
            timeoutTimer_->stop();
            stalledVideoTimer_->start();
        }

        //if (stallretryCount_ == MAX_STALL_RETRIES)
        if (/*retryCount_ == MAX_RETRIES ||*/ stallretryCount_ == MAX_STALL_RETRIES)
            inRecovery_ = false;

        if(inRecovery_)
            UpdatePlayerState(ENUM_PLAYER_STATE::RECOVERING);
        else
            UpdatePlayerState(currentState_);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        PRINT << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

QList<QPair<int, QString>> QtPlayer::GetSubtitleTracks()
{
    return subtitleList_;
}

void QtPlayer::SetSubtitleTrack(int index)
{
    try
    {
        subtitleIndex_ = index;
        player_->setActiveSubtitleTrack(index);
    }
    catch(const std::exception& e)
    {
        PRINT << "SetSubtitleTrack error: " << e.what();
    }
}

QString QtPlayer::GetVideoResolution()
{
    try
    {
        if (!player_)
            return "";

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
    catch(const std::exception& e)
    {
        PRINT << "GetVideoResolution error: " << e.what();

        return "";
    }
}

void QtPlayer::HandleError(QMediaPlayer::Error error, const QString &errorString)
{
    PRINT << "!!!!---> QtPlayer Error: " << errorString << " (" << error << ") Retry: " << retryCount_ << "Buffer: " << player_->bufferProgress();
    stallPosition_ = position_;

    if (error == QMediaPlayer::ResourceError)
    {
        if (errorString.indexOf("Demuxing failed") >= 0)
        {
            PRINT << "Demuxing failed, stopping playback.";
            player_->stop();
            SetupPlayer();
        }
        PRINT << "Stream error, retry count: " << retryCount_;
        ReConnectPlayer();
        return;
    }
    else if (error == QMediaPlayer::FormatError)
    {
        PRINT << "Format error, stopping playback.";
        ReConnectPlayer();
        //RetryStalledPlayer();
        return;
    }
    else
    {
        if (isPositionSeeking_)
            return;

        PRINT << "Error detected, attempting to recover....";
        ErrorOccured(errorString.toStdString());
        RetryStalledPlayer();
        return;
    }
}

/*void QtPlayer::HandleStreamBufferError(const QString &error)
{
    PRINT << "StreamBuffer Error: " << error << " Retry: " << retryCount_;
    ErrorOccured(error.toStdString());
    if (error.contains("Unrecoverable network error"))
    {
        player_->stop();
        currentState_ = ENUM_PLAYER_STATE::ERROR;
        playerStatus_ = "StreamBuffer error: " + error;
        UpdatePlayerState(currentState_);
    }
    else if (retryCount_ < MAX_RETRIES)
    {
        ReConnectPlayer();
    }
    else
    {
        player_->stop();
        currentState_ = ENUM_PLAYER_STATE::STALLED;
        playerStatus_ = "Max retries reached in StreamBuffer: " + error;
        UpdatePlayerState(currentState_);
    }
}*/


void QtPlayer::OnChangingPosition(bool isPlaying)
{
    if (isPlaying)
    {
        isPositionSeeking_ = true;
        //PRINT << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Changing position";
        //watchdogTimer_->stop();
        timeoutTimer_->stop();
        stalledVideoTimer_->stop();
    }
}

void QtPlayer::OnChangedPosition(bool isPlaying)
{
    if (isPlaying)
    {
        //PRINT << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Changed position";
        //watchdogTimer_->start();
        timeoutTimer_->start();
        stalledVideoTimer_->start();
        isPositionSeeking_ = false;
    }
}

void QtPlayer::ChangeUpdateTimerInterval(bool isFullScreen)
{
}

void QtPlayer::CheckTimeout()
{
    if (duration_ > 0)
        return;

    try
    {
        PRINT << "Stream timeout, buffer state: " << player_->bufferProgress() << "position: " << player_->position() << "Retry: " << retryCount_;


        if (player_->playbackState() == QMediaPlayer::PlayingState && player_->bufferProgress() > 0)  //> 0.5)
        {
            PRINT << "QtPlayer: Timeout ignored, playback active";
            return;
        }
        if (retryCount_ < MAX_RETRIES)
        {
            PRINT << "Stream buffer stopped, retrying...";
            retryCount_++;
            stallPosition_ = position_;
            player_->stop();

            ReConnectPlayer();
        }
        else
        {
            PRINT << "Max retries reached, stalling.";
            return;
            
            //SetupPlayer();
            stallPosition_ = position_;
            player_->stop();
            currentState_ = ENUM_PLAYER_STATE::STALLED;
            UpdatePlayerState(currentState_);
        }
    }
    catch (const std::exception& e)
    {
        PRINT << "Timeout error: " << e.what();
        ErrorOccured(std::string(e.what()));
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
            SetupPlayer();
            //QTimer::singleShot(2000 * retryCount_, this, &QtPlayer::Play);
            QTimer::singleShot(2000, this, &QtPlayer::PlaySource);
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

void QtPlayer::StalledVideoDetected()
{
    if (duration_ > 0) 
    {
        //PRINT << "Stalled video detected, duration: " << duration_;
        return;
    }

    if(player_->playbackState() == QMediaPlayer::PlayingState && stallretryCount_ < MAX_STALL_RETRIES) // && !isPositionSeeking_) 
    {
        PRINT << "Stalled video detected, attempting to retry...";
        stallPosition_ = position_;
        player_->stop();
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

void QtPlayer::ReConnectPlayer()
{
    //RetryStalledPlayer();
    //return;
    //stallPosition_ = position_;

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
            //watchdogTimer_->stop();
            timeoutTimer_->stop();
            stalledVideoTimer_->stop();
            retryCount_++;
            playerStatus_= "Connection attempt (" + QString::number(retryCount_) + " of " + QString::number(MAX_RETRIES) + ")";
            PRINT << playerStatus_;
            player_->stop();
            SetupPlayer();
            SetVideoSource(source_);
            QTimer::singleShot(250, this, &QtPlayer::PlaySource);
            //QThread::msleep(500);
            //this->RefreshVideoSource();
            //Play();
        }
        else
        {
            player_->stop();
            //watchdogTimer_->stop();
            timeoutTimer_->stop();
            stalledVideoTimer_->stop();
            //SetupPlayer();
            playerStatus_ = "Max Reconnection attempts reached";
            PRINT << playerStatus_;
            //ErrorOccured("Max retries reached");
            RetryStalledPlayer();
            //currentState_ = ENUM_PLAYER_STATE::STALLED;
            //UpdatePlayerState(currentState_);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        PRINT << "Reconnect error: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

void QtPlayer::RetryStalledPlayer()
{
    //stallPosition_ = position_;

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
            PRINT << "-----------------> STALLED VIDEO DETECTED <-----------------";
            playerStatus_= "Rocovering (" + QString::number(stallretryCount_) + " of " + QString::number(MAX_STALL_RETRIES) + ")";
            PRINT << playerStatus_;

            player_->stop();
            SetupPlayer();
            RefreshVideoSource();
        }
        else
        {
            // Set player state message
            //ErrorOccured("Max Stalled Video retries reached");
            playerStatus_ = "Max Recovery attempts reached";
            PRINT << playerStatus_;
            currentState_ = ENUM_PLAYER_STATE::ERROR;
            inRecovery_ = false;
            UpdatePlayerState(currentState_);
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        PRINT << "Reconnect error: " << e.what();
        ErrorOccured(std::string(e.what()));
    }
}

QString QtPlayer::GetPlayerStatus()
{
    return playerStatus_;
}

void QtPlayer::UpdateRetryParams()
{
    // Get Params for appData
}