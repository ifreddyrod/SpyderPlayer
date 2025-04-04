#include "SpyderPlayerApp.h"

#include <QApplication>
#include <QStyleFactory>
#include <QIcon>

int main(int argc, char *argv[])
{
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
