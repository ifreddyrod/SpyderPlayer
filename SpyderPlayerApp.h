#ifndef SPYDERPLAYERAPP_H
#define SPYDERPLAYERAPP_H

#include <QWidget>

#include "ui_PlayerMainWindow.h"
#include "SplashScreen.h"
#include "VideoControlPanel.h"

using namespace std;

class SpyderPlayerApp : public QWidget
{
    Q_OBJECT

public:
    SpyderPlayerApp(QWidget *parent = nullptr);
    ~SpyderPlayerApp();

    void InitializePlayLists();

private:
    QString version = "1.0.0 Beta";
    bool isFullScreen = false;
    bool mouseMoveActive = false;
    QPoint *mousePressPos = NULL;

    // Gui Forms
    Ui::PlayerMainWindow ui;
    SplashScreen splashScreen;
    VideoControlPanel controlPanel;
    VideoControlPanel controlPanelFS;

    

};


#endif // SPYDERPLAYERAPP_H
