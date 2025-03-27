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
    controlPanelFS.setWindowOpacity(0.8);
    controlPanelFS.hide();
    controlPanelFS.installEventFilter(this);
    //ui.Bottom_Widget->layout = controlPanelFS;

    //-----------------------------
    // Setup Layout
    //-----------------------------
    ui.botomverticalLayout->addWidget(&controlPanel);
    controlPanel.show();
    ui.Horizontal_splitter->setSizes({400, 1000});
    ui.Vertical_splitter->setSizes({800, 1});
    ui.Horizontal_splitter->installEventFilter(this);
    ui.Vertical_splitter->installEventFilter(this);
    
    ui.ShowControlPanel_top_label->installEventFilter(this);
    ui.ShowControlPanel_top_label->setMouseTracking(true);
    ui.ShowControlPanel_bottom_label->installEventFilter(this);
    ui.ShowControlPanel_bottom_label->setMouseTracking(true);
    ui.ShowControlPanel_right_label->installEventFilter(this);
    ui.ShowControlPanel_right_label->setMouseTracking(true);
    ui.ShowControlPanel_left_label->installEventFilter(this);
    ui.ShowControlPanel_left_label->setMouseTracking(true);

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
    splashScreen.SetAppVersion(version);

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
