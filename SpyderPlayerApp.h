#ifndef SPYDERPLAYERAPP_H
#define SPYDERPLAYERAPP_H

#include <QWidget>
#include <memory> 
#include <QTimer>
#include <QMenu>

#include "ui_PlayerMainWindow.h"
#include "AppData.h"
#include "SplashScreen.h"
#include "VideoControlPanel.h"
#include "VideoOverlay.h"
#include "PlaylistManager.h"
#include "VideoPlayer.h"
#include "QtPlayer.h"

using namespace std;

class SpyderPlayerApp : public QWidget
{
    Q_OBJECT

public:
    SpyderPlayerApp(QWidget *parent = nullptr);
    ~SpyderPlayerApp();

    void InitializePlayLists();
    void OnHSplitterResized(int, int);
    int GetVideoPanelWidth();
    int GetVideoPanelHeight();
    QWidget* GetVideoPanelWidget();

private:
    QString version_ = "1.0.0 Beta";
    string platform_ = "";
    bool isFullScreen_ = false; 
    bool isPlaylistVisible_ = true;
    bool mouseMoveActive_ = false;
    QPoint mousePressPos_;
    AppData *appData_;
    VideoPlayer* player_;
    QTimer *inactivityTimer_;
    QTimer *stalledVideoTimer_;
    QTimer *playbackStatusTimer_;
    QMenu  *subtitlesMenu_;
    qint64 videoPosition_ = 0;
    qint64 videoDuration_ = 0;
    bool videoChangesPosition_ = false;
    bool isVideoPlaying_ = false;
    QPoint controlpanelPosition_ = QPoint(0, 0);

    // Gui Forms
    Ui::PlayerMainWindow ui_;
    SplashScreen splashscreen_;
    VideoControlPanel controlpanel_;
    VideoControlPanel controlpanelFS_;
    VideoOverlay* overlay_;
    PlaylistManager* playlistManager_;

    // Event Overrides
    bool eventFilter(QObject *object, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;

    // GUI Functions
    void PlayerNormalScreen();
    void PlayerFullScreen();
    void ShowControlPanel();
    void TogglePlaylistView();
    void ChangePlayingUIStates(bool isPlaying);
    void PlayerDurationChanged(qint64 duration);
    void VideoTimePositionChanged(qint64 position);
    void ShowVideoResolution();
    void UserActivityDetected();
    

    // Player Functions
    void InitPlayer();
    void PlaySelectedChannel(string channelName, string source);
    void PlayPausePlayer();
    void StopPlayer();
    void MutePlayer();
    void SkipForward();
    void SkipBackward();
    void PlayNextChannel();
    void PlayPreviousChannel();
    void PlayLastChannel();
    void FullVolumePlayer();
    void ChangeVolume();
    void IncreaseVolume();
    void DecreaseVolume();
    void UpdateVolumeSlider(int volume);
    void OnPositionSliderPressed();
    void OnPositionSliderMoved();
    void OnPositionSliderReleased();

    // Utility Functions
    void InactivityDetected();
    void UpdatePlaybackStatus();

    
};


#endif // SPYDERPLAYERAPP_H
