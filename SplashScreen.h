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

//private slots:
    void CenterSplashScreen();
    void UpdateStatus(const QString &status, int delay = 100);
    void SetTimeout();
    void StartTimer();

private:
    Ui::SplashScreen ui;  // UI object
    QTimer splashTimer;
    bool splashTimerCompleted;
};

#endif // SPLASHSCREEN_H
