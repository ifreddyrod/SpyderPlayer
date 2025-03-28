#include "SplashScreen.h"
#include <QApplication>
#include <QScreen>
#include <QThread> 
#include <QDebug>

SplashScreen::SplashScreen(QWidget *parent) : QWidget(parent), splash_timer_completed_(false) 
{
    ui_.setupUi(this);
    //ui.Background_image->setPixmap(QPixmap(":/icons/icons/spider-black.png"));

    //qDebug() <<  "Picture: " << ui_.Background_image->pixmap();

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    CenterSplashScreen();

    splashtimer_.setInterval(5000);
    connect(&splashtimer_, &QTimer::timeout, this, &SplashScreen::SetTimeout);

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

void SplashScreen::SetAppVersion(QString version) 
{
    ui_.Version_label->setText("Version " + version);
}

void SplashScreen::UpdateStatus(const QString &status, int delay) 
{
    ui_.Status_label->setText(status);
    QApplication::processEvents();
    QThread::msleep(delay);  // Sleep for the specified delay in milliseconds
}

void SplashScreen::SetTimeout() 
{
    splash_timer_completed_ = true;
}

void SplashScreen::StartTimer() 
{
    splashtimer_.start();
}
