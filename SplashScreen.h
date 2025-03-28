#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QWidget>
#include <QTimer>

#include "ui_SplashScreen.h"  // Include the generated UI header

class SplashScreen : public QWidget 
{
    Q_OBJECT

public:
    explicit SplashScreen(QWidget *parent = nullptr);

    void CenterSplashScreen();
    void UpdateStatus(const QString &status, int delay = 100);
    void SetTimeout();
    void StartTimer();
    void SetAppVersion(QString version);

private:
    Ui::SplashScreen ui_;  // UI object
    QTimer splashtimer_;
    bool splash_timer_completed_;
};

#endif // SPLASHSCREEN_H
