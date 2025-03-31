#ifndef SPYDERPLAYERAPP_H
#define SPYDERPLAYERAPP_H

#include <QWidget>
#include <memory> 

#include "ui_PlayerMainWindow.h"
#include "AppData.h"
#include "SplashScreen.h"
#include "VideoControlPanel.h"
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

private:
    QString version_ = "1.0.0 Beta";
    string platform_ = "";
    bool isFullScreen_ = false; 
    bool isPlaylistVisible_ = true;
    bool mouseMoveActive_ = false;
    QPoint *mousePressPos_ = NULL;
    AppData *appData_;
    VideoPlayer* player_;

    // Gui Forms
    Ui::PlayerMainWindow ui_;
    SplashScreen splashscreen_;
    VideoControlPanel controlpanel_;
    VideoControlPanel controlpanelFS_;
    PlaylistManager* playlistManager_;

    bool eventFilter(QObject *object, QEvent *event) override;
    void TogglePlaylistView();
    void InitPlayer();
    void PlaySelectedChannel(string channelName, string source);
    
};


#endif // SPYDERPLAYERAPP_H
