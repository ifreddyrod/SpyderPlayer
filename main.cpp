#include "SpyderPlayerApp.h"

#include <QApplication>
#include <QStyleFactory>
#include <QIcon>
#include "Global.h"
#include "AppData.h"
#include <csignal>
#include <execinfo.h>
#include <unistd.h>
#include <iostream>

SpyderPlayerApp* spyderPlayer;
AppData* appData;

void SigAbrtHandler(int signum)
{
    std::cerr << "Caught signal " << signum << std::endl;

    if(signum == SIGABRT)
    {
        // Print stack trace
        void *array[50];
        size_t size = backtrace(array, 50);
        backtrace_symbols_fd(array, size, STDERR_FILENO);

        try
        {
            PRINT << "*****[ Player Crashed ]*****   Resetting...";
            //spyderPlayer->CrashHandler(signum);
            spyderPlayer->deleteLater();
            spyderPlayer = nullptr;
            spyderPlayer = new SpyderPlayerApp(nullptr, appData);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            exit(1);
        }
    }
}


int main(int argc, char *argv[])
{
    if (qgetenv("XDG_SESSION_TYPE") == "wayland")
    {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
    
    qputenv("AV_LOG_LEVEL", "debug");
    qputenv("FFMPEG_OPTS", "probesize=20000000 analyzeduration=20000000 reconnect=1 reconnect_streamed=1 reconnect_delay_max=60 max_delay=10000000 buffer_size=20971520 fflags=+ignidx+igndts+discardcorrupt err_detect=ignore_err+crccheck format_opts=scan_all_pmts=1");
    qputenv("AVFORMAT_FLAGS", "+genpts+igndts");
    qputenv("FFMPEG_OPTS", "... buffer_size=20971520 ...");
    qputenv("QT_LOGGING_RULES", "qt6.multimedia=true");

    //-----------------------------
    // Load AppData from file
    //-----------------------------
    string data_path = GetUserAppDataDirectory("SpyderPlayer") + "/" + APPDATA_FILENAME; 

    PRINT << "AppData Path: " << data_path;

    appData = new AppData(data_path);

    // Set Qt Applicatoin Scale factor
    double appScaleFactor = appData->AppScaleFactor_;
    qputenv("QT_SCALE_FACTOR", QString::number(appScaleFactor).toStdString().c_str());

    // Create QApp Instance
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setWindowIcon(QIcon(":/icons/icons/spider_dark_icon.icns"));

    // Create Player Instance
    spyderPlayer = new SpyderPlayerApp(nullptr, appData);
    spyderPlayer->show();
    spyderPlayer->OnHSplitterResized(0, 0);

    signal(SIGABRT, SigAbrtHandler);

    return app.exec();
}
