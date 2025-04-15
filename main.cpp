#include "SpyderPlayerApp.h"

#include <QApplication>
#include <QStyleFactory>
#include <QIcon>

int main(int argc, char *argv[])
{
    qputenv("AV_LOG_LEVEL", "debug");
    qputenv("FFMPEG_OPTS", "probesize=20000000 analyzeduration=20000000 reconnect=1 reconnect_streamed=1 reconnect_delay_max=60 max_delay=10000000 buffer_size=20971520 fflags=+ignidx+igndts+discardcorrupt err_detect=ignore_err+crccheck format_opts=scan_all_pmts=1");
    qputenv("AVFORMAT_FLAGS", "+genpts+igndts");
    //qputenv("FFMPEG_OPTS", "... buffer_size=20971520 ...");
    qputenv("QT_LOGGING_RULES", "qt6.multimedia=true");

    // Create QApp Instance
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setWindowIcon(QIcon(":/icons/icons/spider_dark_icon.icns"));

    // Create Player Instance
    SpyderPlayerApp spyderPlayer;
    spyderPlayer.show();
    spyderPlayer.OnHSplitterResized(0, 0);

    return app.exec();
}
