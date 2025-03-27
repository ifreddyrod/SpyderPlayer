#ifndef VIDEOCONTROLPANEL_H
#define VIDEOCONTROLPANEL_H

#include <QObject>
#include <QWidget>

#include "ui_VideoControlPanel.h"

class VideoControlPanel: public QWidget
{
    Q_OBJECT

public:
    VideoControlPanel(QWidget *parent = nullptr);

    Ui::VideoControlPanel ui;
};

#endif // VIDEOCONTROLPANEL_H
