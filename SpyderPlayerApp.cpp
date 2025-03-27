#include "SpyderPlayerApp.h"

#include <QFont>
#include <QDebug>



// Constructor
SpyderPlayerApp::SpyderPlayerApp(QWidget *parent)
    : QWidget(parent)
{
    version = "1.0.0 Beta";

    mousePressPos =  NULL;
    mouseMoveActive = false;
    isFullScreen = false;

    //-----------------------------
    // Load Screensaver Inhibitor
    //-----------------------------
    
    
    //---------------------------------
    // Load and Setup the Main Window
    //---------------------------------
    ui.setupUi(this);

    this->setWindowOpacity(0);

    //-----------------------------
    // Load AppData from file
    //-----------------------------

    //-----------------------------
    // Setup Playlist Manager
    //-----------------------------

    //-----------------------------
    // Setup Control Panels
    //-----------------------------

    //-----------------------------
    // Setup Layout
    //-----------------------------

    //-----------------------------
    // Init Video Player
    //-----------------------------

    //-----------------------------
    // Setup Video Overlay
    //-----------------------------

    //-----------------------------
    // Connect signals
    //-----------------------------



    //-----------------------------
    // Size the Fonts
    //-----------------------------
    // Get the current font
    QFont font = this->font();

    // Get the font family and size
    QString fontFamily = font.family();
    int fontSize = font.pointSize();
    qDebug() << "Window Font: " << fontFamily << " " << fontSize;

    // Check if the point size is smaller than 11 and increase it if necessary
    if (fontSize < 11) 
    {
        this->setStyleSheet("color: white; font: 11pt;");
    }

    //-----------------------------
    // Load Splash Screen
    //-----------------------------
    //splashScreen = new SplashScreen();

    InitializePlayLists();

    ui.Status_label->setText(version);
    /*splashScreen->show();

    ui->Status_label->setText(version);*/
}

// Destructor
SpyderPlayerApp::~SpyderPlayerApp()
{
}


void SpyderPlayerApp::InitializePlayLists()
{
    splashScreen.show();
    splashScreen.StartTimer();
    splashScreen.UpdateStatus("Loading Playlists:", 1000);
    
    // Load Library Playlist
    splashScreen.UpdateStatus("Loading Library ....", 1000);
    
    // Load Favorites last to verify that items in other playlists have been loaded
    splashScreen.UpdateStatus("Loading Favorites ....", 1000);

    splashScreen.UpdateStatus("Initialization Complete", 1000);

    // Show Main Window
    splashScreen.hide();
    this->setWindowOpacity(1.0);
}