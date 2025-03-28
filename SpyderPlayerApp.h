#ifndef SPYDERPLAYERAPP_H
#define SPYDERPLAYERAPP_H

#include <QWidget>
#include <memory> 

#include "ui_PlayerMainWindow.h"
#include "AppData.h"
#include "SplashScreen.h"
#include "VideoControlPanel.h"
#include "PlaylistManager.h"

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
    bool is_full_screen_ = false; 
    bool mouse_move_active_ = false;
    QPoint *mouse_press_pos_ = NULL;
    AppData *appData_;

    // Gui Forms
    Ui::PlayerMainWindow ui_;
    SplashScreen splashscreen_;
    VideoControlPanel controlpanel_;
    VideoControlPanel controlpanelFS_;
    PlaylistManager* playlistManager_;

    

};


#endif // SPYDERPLAYERAPP_H
