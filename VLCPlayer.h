// VLCPlayer.h
#ifndef VLCPLAYER_H
#define VLCPLAYER_H

#include "VideoPlayer.h"
#include "Global.h"
#include <vlc/vlc.h>
#include <QWidget>
#include <QTimer>
#include <QList>
#include "ui_PlayerMainWindow.h"

class SpyderPlayerApp;

class VLCPlayer : public VideoPlayer
{
    Q_OBJECT

public:
    VLCPlayer(Ui::PlayerMainWindow* mainWindow, QWidget* parent = nullptr);
    ~VLCPlayer();

    void InitPlayer(void *args) override;
    void SetVideoSource(const std::string& videoSource) override;
    void RefreshVideoSource() override;
    void Play() override;
    void Pause() override;
    void Resume() override;
    void ResumePlayback();
    void Stop() override;
    void SetPosition(qint64 position) override;
    void SkipPosition(qint64 position) override;
    qint64 GetPosition() override;
    qint64 GetVideoDuration() override;
    void SetVolume(int volume) override;
    int GetVolume() override;
    bool IsMuted() override;
    void Mute(bool mute) override;
    QString GetVideoResolution() override;
    QList<QPair<int, QString>> GetSubtitleTracks() override;
    void SetSubtitleTrack(int index) override;
    ENUM_PLAYER_STATE GetPlayerState() override;
    QString GetPlayerStatus() override;
    void SetVideoTitle(const QString& text) override;
    void SetVideoTitleVisible(bool visible) override;

    void OnChangingPosition(bool isPlaying) override;
    void OnChangedPosition(bool isPlaying) override;
    void ChangeUpdateTimerInterval(bool isFullScreen) override;

private slots:
    //void UpdatePositionSlot();
    //void CheckTimeout();
    //void CheckPlaybackHealth();
    void StalledVideoDetected();

private:
    void SetupPlayer();
    //void ReConnectPlayer();
    void RetryStalledPlayer();
    void PlaySource();
    void AttachEvents();
    static void HandleVLCEvent(const libvlc_event_t* event, void* data);
    static void VLCLogCallback(void *data, int /*level*/, const libvlc_log_t *ctx, const char *fmt, va_list args);
    void StopPlayback();
    void UpdatePlayerStatus();
    std::vector<const char*> GetInitArgs(const QString& VCLSetupParams);

    SpyderPlayerApp* app_;
    Ui::PlayerMainWindow* mainWindow_ = nullptr;
    QWidget* videoPanel_;
    libvlc_instance_t* vlcInstance_ = nullptr;
    libvlc_media_player_t* mediaPlayer_ = nullptr;
    libvlc_media_t* media_ = nullptr;
    libvlc_event_manager_t* eventManager_;
    ENUM_PLAYER_STATE previousState_;
    QString hwAccelType_ = "avcodec-hw=any";

    //QTimer* positionTimer_;
    //QTimer* watchdogTimer_;
    QTimer* updateTimer_;
    QTimer* stalledVideoTimer_;
    bool isMuted_ = false;
    int subtitleCount_;
    QList<QPair<int, QString>> subtitleList_;
    int subtitleIndex_;
    qint64 lastPosition_ = -1;
    int retryCount_ = 0;
    int stallretryCount_ = 0;
    static constexpr int MAX_RETRIES = 8;
    int MAX_STALL_RETRIES = 5;
    qint64 retryTimeDelayMs_ = 500;
    bool inRecovery_ = false;
    QString playerStatus_;
    bool isPositionSeeking_ = false;
    qint64 stallPosition_ = -1;
    bool stopAll_ = false;
    int skipBackLength_ = 15000;
    int titleFontSize_ = 18;
    bool isPlaying_ = false;
    bool isPaused_ = false;
    float bufferSize_ = 0;
};

#endif // VLCPLAYER_H