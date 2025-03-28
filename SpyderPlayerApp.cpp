#include "SpyderPlayerApp.h"

#include <QFont>
#include <QDebug>


// Constructor
SpyderPlayerApp::SpyderPlayerApp(QWidget *parent)
    : QWidget(parent)
{
    version_ = "1.0.0 Beta";

    mouse_press_pos_ =  NULL;
    mouse_move_active_ = false;
    is_full_screen_ = false;
    platform_ = GetPlatform();
    
    //-----------------------------
    // Load Screensaver Inhibitor
    //-----------------------------
    
    
    //---------------------------------
    // Load and Setup the Main Window
    //---------------------------------
    ui_.setupUi(this);

    this->setWindowOpacity(0);

    //-----------------------------
    // Load AppData from file
    //-----------------------------
    string data_path = GetUserAppDataDirectory(platform_, "SpyderPlayer");
    qDebug() << "Data Path: " << data_path;

    //-----------------------------
    // Setup Playlist Manager
    //-----------------------------

    //-----------------------------
    // Setup Control Panels
    //-----------------------------
    controlpanelFS_.setWindowOpacity(0.8);
    controlpanelFS_.hide();
    controlpanelFS_.installEventFilter(this);
    //ui.Bottom_Widget->layout = controlPanelFS;

    //-----------------------------
    // Setup Layout
    //-----------------------------
    ui_.botomverticalLayout->addWidget(&controlpanel_);
    controlpanel_.show();
    ui_.Horizontal_splitter->setSizes({400, 1000});
    ui_.Vertical_splitter->setSizes({800, 1});
    ui_.Horizontal_splitter->installEventFilter(this);
    ui_.Vertical_splitter->installEventFilter(this);
    
    ui_.ShowControlPanel_top_label->installEventFilter(this);
    ui_.ShowControlPanel_top_label->setMouseTracking(true);
    ui_.ShowControlPanel_bottom_label->installEventFilter(this);
    ui_.ShowControlPanel_bottom_label->setMouseTracking(true);
    ui_.ShowControlPanel_right_label->installEventFilter(this);
    ui_.ShowControlPanel_right_label->setMouseTracking(true);
    ui_.ShowControlPanel_left_label->installEventFilter(this);
    ui_.ShowControlPanel_left_label->setMouseTracking(true);

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
    splashscreen_.SetAppVersion(version_);

    InitializePlayLists();

    ui_.Status_label->setText(version_);
    /*splashScreen->show();

    ui->Status_label->setText(version);*/
}

// Destructor
SpyderPlayerApp::~SpyderPlayerApp()
{
}


void SpyderPlayerApp::InitializePlayLists()
{
    splashscreen_.show();
    splashscreen_.StartTimer();
    splashscreen_.UpdateStatus("Loading Playlists:", 1000);
    
    // Load Library Playlist
    splashscreen_.UpdateStatus("Loading Library ....", 1000);
    
    // Load Favorites last to verify that items in other playlists have been loaded
    splashscreen_.UpdateStatus("Loading Favorites ....", 1000);

    splashscreen_.UpdateStatus("Initialization Complete", 1000);

    // Show Main Window
    splashscreen_.hide();
    this->setWindowOpacity(1.0);
}
