#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <QWidget>
#include <QTimer>
#include <QSignalMapper>
#include <QList>
//#include <vlc/vlc.h>
#include "Global.h"

// VideoPlayer class
class VideoPlayer : public QWidget 
{
    Q_OBJECT

public:
    VideoPlayer(QWidget* parent = nullptr);
    virtual ~VideoPlayer();

    // Methods
    void UpdatePosition(qint64 position);
    void UpdateDuration(qint64 duration);
    void UpdatePlayerState(ENUM_PLAYER_STATE state);
    void ErrorOccured(const std::string& error);
    void EnableSubtitles(bool enable);
    virtual void InitPlayer() = 0;
    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void SetPosition(qint64 position) = 0;
    virtual void SkipPosition(qint64 position) = 0;
    virtual qint64 GetPosition() = 0;
    virtual void SetVolume(int volume) = 0;
    virtual int GetVolume() = 0;
    virtual void Mute(bool mute) = 0;
    virtual bool IsMuted() = 0;
    virtual void SetVideoSource(const std::string& videoSource) = 0;
    virtual void RefreshVideoSource() = 0;
    virtual qint64 GetVideoDuration() = 0;
    virtual void OnChangingPosition(bool isPlaying) = 0;
    virtual void OnChangedPosition(bool isPlaying) = 0;
    virtual void ChangeUpdateTimerInterval(bool isFullScreen) = 0;
    virtual QString GetVideoResolution() = 0;
    virtual QList<QPair<int, QString>> GetSubtitleTracks() = 0;
    virtual void SetSubtitleTrack(int index) = 0;
    virtual ENUM_PLAYER_STATE GetPlayerState() = 0;
    QWidget* GetVideoPanel() { return videoWidget_; }

    // Signals
    Q_SIGNAL void SIGNAL_UpdatePosition(qint64 position);
    Q_SIGNAL void SIGNAL_UpdateDuration(qint64 duration);
    Q_SIGNAL void SIGNAL_PlayerStateChanged(ENUM_PLAYER_STATE state);
    Q_SIGNAL void SIGNAL_ErrorOccured(const std::string& error);
    Q_SIGNAL void SIGNAL_EnableSubtitles(bool enable);

protected:
    ENUM_PLAYER_STATE currentState_;
    QWidget* mainWindow_;
    QWidget* videoWidget_;
    std::string source_;
    qint64 duration_;
    qint64 position_;
};

#endif