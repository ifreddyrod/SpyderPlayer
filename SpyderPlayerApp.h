#ifndef SPYDERPLAYERAPP_H
#define SPYDERPLAYERAPP_H

#include <QWidget>
#include <memory> 
#include <QTimer>
#include <QMenu>
#include <QThread> 

#include "ui_PlayerMainWindow.h"
#include "AppData.h"
#include "SplashScreen.h"
#include "VideoControlPanel.h"
//###include "VideoOverlay.h"
#include "PlaylistManager.h"
#include "VideoPlayer.h"
#include "QtPlayer.h"
#include "ScreensaverInhibitor.h"
#include "SettingsManager.h"

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
    int GetRetryTimeDelay();

private:
    QString version_ = "";
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
    bool subtitlesEnabled_ = false;
    qint64 videoPosition_ = 0;
    qint64 videoDuration_ = 0;
    bool videoChangesPosition_ = false;
    bool isVideoPlaying_ = false;
    QPoint controlpanelPosition_ = QPoint(0, 0);
    int volume_ = 100;
    bool retryPlaying_ = true;
    int retryCount_ = 3;

    // Gui Forms
    Ui::PlayerMainWindow ui_;
    SplashScreen splashscreen_;
    VideoControlPanel controlpanel_;
    VideoControlPanel controlpanelFS_;
    //##VideoOverlay* overlay_;
    PlaylistManager* playlistManager_;
    ScreensaverInhibitor* screensaverInhibitor_;
    SettingsManager* settingsManager_;


    // Event Overrides
    bool eventFilter(QObject *object, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // GUI Functions
    void PlayerNormalScreen();
    void PlayerFullScreen();
    void PlayerMinimized();
    void ShowControlPanel();
    void TogglePlaylistView();
    void ChangePlayingUIStates(bool isPlaying);
    void PlaybackStateChanged(ENUM_PLAYER_STATE state);
    void PlayerDurationChanged(qint64 duration);
    void VideoTimePositionChanged(qint64 position);
    void PlayerErrorOccured(const std::string& error);
    void ShowVideoResolution();
    void EnableSubtitles(bool enable);
    void ShowSubtitleTracks();
    void SelectSubtitleTrack(QAction* menuItem);
    void UserActivityDetected();
    void StalledVideoDetected();
    void ShowSettings();

    // Player Functions
    void InitPlayer();
    void PlaySelectedChannel(string channelName, string source);
    void PlayPausePlayer();
    void StopPlayer();
    void MutePlayer();
    void SeekForward();
    void SeekBackward();
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
    void SearchChannels();
    void InactivityDetected();
    void UpdatePlaybackStatus();
    QString GetWindowStateString();
    void ShowCursorNormal();
    void ShowCursorBusy();
    void ShowCursorBlank();
    
};


#endif // SPYDERPLAYERAPP_H
