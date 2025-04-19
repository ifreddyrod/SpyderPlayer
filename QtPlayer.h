// QtPlayer.h
#ifndef QTMEDIA_H
#define QTMEDIA_H

#include "VideoPlayer.h"
#include "Global.h"
#include <QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QAudioOutput>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <QWidget>
#include <QTimer>
#include <QSignalMapper>
#include <QList>
#include "ui_PlayerMainWindow.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslConfiguration>
#include "StreamBuffer.h"

class SpyderPlayerApp;

class QtPlayer : public VideoPlayer 
{
    Q_OBJECT

public:
    QtPlayer(Ui::PlayerMainWindow* mainWindow, QWidget* parent = nullptr);
    ~QtPlayer();

    void InitPlayer() override;
    void SetVideoSource(const std::string& videoSource) override;
    void RefreshVideoSource() override;
    void Play() override;
    void Pause() override;
    void Stop() override;
    void SetPosition(qint64 position) override;
    void SkipPosition(qint64 position) override;
    qint64 GetPosition() override;
    qint64 GetVideoDuration() override;
    void SetVolume(int volume) override;
    int GetVolume() override;
    bool IsMuted() override;
    void Mute(bool mute) override;
    void PlayerDurationChanged(int duration);
    void PlayerPositionChanged(int position);
    ENUM_PLAYER_STATE GetPlayerState() override;
    void PlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void MediaStatusChanged(QMediaPlayer::MediaStatus mediaState);
    QList<QPair<int, QString>> GetSubtitleTracks() override;
    void SetSubtitleTrack(int index) override;
    QString GetVideoResolution() override;
    void ListVideoTracks();

    void OnChangingPosition(bool isPlaying) override;
    void OnChangedPosition(bool isPlaying) override;
    void ChangeUpdateTimerInterval(bool isFullScreen) override;
    void HandleError(QMediaPlayer::Error error, const QString &errorString); 
    void CheckTimeout();
    void CheckPlaybackHealth();
    void StartWatchdog();
    void HandleStreamBufferError(const QString &error);
    void StalledVideoDetected();

private:
    void SetupPlayer();
    SpyderPlayerApp* app_;
    Ui::PlayerMainWindow* mainWindow_;
    QVideoWidget* videoPanel_;
    QMediaPlayer* player_;
    QAudioOutput* audioOutput_;
    bool isMuted_ = false;
    int subtitleCount_;
    QList<QPair<int, QString>> subtitleList_;
    int subtitleIndex_;
    StreamBuffer *streamBuffer_ = nullptr;
    QTimer *watchdogTimer_;
    QTimer *timeoutTimer_;
    qint64 lastPosition_ = -1;
    QMediaPlayer::MediaStatus mediaState_;
    int retryCount_ = 0;
    static constexpr int MAX_RETRIES = 8;
    QTimer *stalledVideoTimer_;
};

#endif // QTMEDIA_H