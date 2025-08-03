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
    
    qputenv("AV_LOG_LEVEL", "warning");
    qputenv("QT_MULTIMEDIA_PREFERRED_DECODERS", "software");
    qputenv("FFMPEG_OPTS", "probesize=20000000 analyzeduration=20000000 reconnect=1 reconnect_streamed=1 reconnect_delay_max=60 max_delay=10000000 buffer_size=20971520 err_detect=ignore_err+crccheck format_opts=scan_all_pmts=1 -hwaccel none");
    qputenv("AVFORMAT_FLAGS", "+genpts+igndts");
    qputenv("QT_LOGGING_RULES", "qt6.multimedia=true");

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
