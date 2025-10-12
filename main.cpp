#include "SpyderPlayerApp.h"

#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include "Global.h"
#include "AppData.h"

SpyderPlayerApp *spyderPlayer;
AppData *appData;

int main(int argc, char *argv[])
{
    //-----------------------------
    // Set Environment Variables
    //-----------------------------    
    if (qgetenv("XDG_SESSION_TYPE") == "wayland")
    {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
    
    if (GetPlatform() == "Darwin")
    {
        qputenv("VLC_PLUGIN_PATH", "/Applications/VLC.app/Contents/MacOS/plugins");
        //qputenv("VLC_PLUGIN_PATH", "/Applications/SpyderPlayer.app/Contents/Frameworks/vlc/plugins");
    }
    else if (GetPlatform() == "Linux")
    {
        qputenv("VLC_PLUGIN_PATH", "/usr/lib/x86_64-linux-gnu/vlc/plugins");
    }
    else if (GetPlatform() == "Windows")
    {
        qputenv("VLC_PLUGIN_PATH", "C:/Program Files/VideoLAN/VLC/plugins");
    }

    //-----------------------------
    // Load AppData from file
    //-----------------------------
    G_AppDataDirectory = GetUserAppDataDirectory("SpyderPlayer") + "/";
    string appDataPath = G_AppDataDirectory + APPDATA_FILENAME; 

    PRINT << "AppData Path: " << appDataPath;

    appData = new AppData(appDataPath);

    //-----------------------------
    // Set App Specific Settings
    //-----------------------------
    // Set Qt Applicatoin Scale factor
    double appScaleFactor = appData->AppScaleFactor_;
    qputenv("QT_SCALE_FACTOR", QString::number(appScaleFactor).toStdString().c_str());

    // Enable degug logging to file 
    G_LogToFile = appData->EnableFileLogging_;
    if (G_LogToFile) 
        qInstallMessageHandler(LogFileOutput);

    //-----------------------------
    // Create QApp Instance
    //-----------------------------
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setWindowIcon(QIcon(":/icons/icons/spider_dark_icon.icns"));

    // Create Player Instance
    spyderPlayer = new SpyderPlayerApp(nullptr, appData);
    spyderPlayer->show();
    spyderPlayer->OnHSplitterResized(0, 0);

    int appReturn = app.exec();

    return appReturn;
}
