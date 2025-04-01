#include "SpyderPlayerApp.h"

#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    // Create QApp Instance
    QApplication app(argc, argv);
    app.setStyle(QStyleFactory::create("Fusion"));

    // Create Player Instance
    SpyderPlayerApp spyderPlayer;
    spyderPlayer.show();
    spyderPlayer.OnHSplitterResized(0, 0);

    return app.exec();
}
