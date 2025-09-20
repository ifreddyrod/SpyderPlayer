#include "SpyderPlayerApp.h"

#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include "Global.h"
#include "AppData.h"

SpyderPlayerApp* spyderPlayer;
AppData* appData;

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
        //qputenv("VLC_PLUGIN_PATH", "/Applications/SpyderPlayer.app/Contents/Frameworks");
    }
    else if (GetPlatform() == "Linux")
    {
        qputenv("VLC_PLUGIN_PATH", "/usr/lib/x86_64-linux-gnu/vlc/plugins");
    }

    //-----------------------------
    // Load AppData from file
    //-----------------------------
    string data_path = GetUserAppDataDirectory("SpyderPlayer") + "/" + APPDATA_FILENAME; 

    PRINT << "AppData Path: " << data_path;

    appData = new AppData(data_path);

    //-----------------------------
    // Set App Specific Settings
    //-----------------------------

    // Set Qt Applicatoin Scale factor
    double appScaleFactor = appData->AppScaleFactor_;
    qputenv("QT_SCALE_FACTOR", QString::number(appScaleFactor).toStdString().c_str());

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

    return app.exec();
}
