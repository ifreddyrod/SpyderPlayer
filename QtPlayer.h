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

    void InitPlayer();
    void SetVideoSource(const std::string& videoSource);
    void RefreshVideoSource();
    void Play();
    void Pause();
    void Stop();
    void SetPosition(qint64 position);
    qint64 GetPosition();
    qint64 GetVideoDuration();
    void SetVolume(int volume);
    int GetVolume();
    bool IsMuted();
    void Mute(bool mute);
    void PlayerDurationChanged(int duration);
    void PlayerPositionChanged(int position);
    ENUM_PLAYER_STATE GetPlayerState();
    void PlaybackStateChanged(QMediaPlayer::PlaybackState state);
    void MediaStatusChanged(QMediaPlayer::MediaStatus mediaState);
    QList<QPair<int, QString>> GetSubtitleTracks();
    void SetSubtitleTrack(int index);
    QString GetVideoResolution();
    void ListVideoTracks();

    void OnChangingPosition(bool isPlaying);
    void OnChangedPosition(bool isPlaying);
    void ChangeUpdateTimerInterval(bool isFullScreen);
    void HandleError(QMediaPlayer::Error error, const QString &errorString); 
    void CheckTimeout();
    void CheckPlaybackHealth();
    void StartWatchdog();
    void HandleStreamBufferError(const QString &error);

private:
    void SetupPlayer();
    SpyderPlayerApp* app_;
    Ui::PlayerMainWindow* mainWindow_;
    QVideoWidget* videoPanel_;
    QMediaPlayer* player_;
    QAudioOutput* audioOutput_;
    int subtitleCount_;
    QList<QPair<int, QString>> subtitleList_;
    int subtitleIndex_;
    StreamBuffer *streamBuffer_ = nullptr;
    QTimer *watchdogTimer_;
    QTimer *timeoutTimer_;
    qint64 lastPosition_ = -1;
    QMediaPlayer::MediaStatus mediaState_;
    int retryCount_ = 0;
    static constexpr int MAX_RETRIES = 5;
};

#endif // QTMEDIA_H