#include "SplashScreen.h"
#include <QApplication>
#include <QScreen>
#include <QThread> 
#include <QDebug>

SplashScreen::SplashScreen(QWidget *parent) : QWidget(parent), splashTimerCompleted(false) 
{
    ui.setupUi(this);
    //ui.Background_image->setPixmap(QPixmap(":/icons/icons/spider-black.png"));

    qDebug() <<  "Picture: " << ui.Background_image->pixmap();

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    CenterSplashScreen();

    splashTimer.setInterval(5000);
    connect(&splashTimer, &QTimer::timeout, this, &SplashScreen::SetTimeout);

}

void SplashScreen::CenterSplashScreen() 
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    QSize splashSize = size();

    int x = (screenGeometry.width() - splashSize.width()) / 2;
    int y = (screenGeometry.height() - splashSize.height()) / 2;

    move(x, y);
}

void SplashScreen::UpdateStatus(const QString &status, int delay) 
{
    ui.Status_label->setText(status);
    QApplication::processEvents();
    QThread::msleep(delay);  // Sleep for the specified delay in milliseconds
}

void SplashScreen::SetTimeout() 
{
    splashTimerCompleted = true;
}

void SplashScreen::StartTimer() 
{
    splashTimer.start();
}

void SplashScreen::SetAppVersion(QString version) 
{
    ui.Version_label->setText("Version " + version);
}
