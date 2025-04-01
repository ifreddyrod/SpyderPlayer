#ifndef VIDEO_OVERLAY_H
#define VIDEO_OVERLAY_H

#include "ui_Overlay.h"
#include "Global.h"

#include <QWidget>
#include <QLabel>
#include <QApplication>
#include <QScreen>

class VideoOverlay : public QWidget
{
    Q_OBJECT

public:
    VideoOverlay(QWidget *parent = nullptr);
    ~VideoOverlay();

    void SetAppObject(QObject *app) { app_ = app; }
    void Resize(bool forceFullscreen = false);
    bool event(QEvent *event) override;

private:
    Ui::Overlay ui_;
    QLabel *overlayLabel_;
    QObject *app_;
};


#endif