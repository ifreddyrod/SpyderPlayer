#ifndef QTMEDIA_H
#define QTMEDIA_H

#include "VideoPlayer.h"
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
//#include <vlc/vlc.h>

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
    void list_video_tracks();

    void OnChangingPosition(bool isPlaying);
    void OnChangedPosition(bool isPlaying);
    void ChangeUpdateTimerInterval(bool isFullScreen);

private:
    Ui::PlayerMainWindow* mainWindow_;
    QVideoWidget* videoPanel_;
    QMediaPlayer* player_;
    QAudioOutput* audioOutput_;
    int subtitleCount_;
    QList<QPair<int, QString>> subtitleList_;
    int subtitleIndex_;
};

#endif // QTMEDIA_H
