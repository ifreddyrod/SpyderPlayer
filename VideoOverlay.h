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

    void SetOverlayVisible(bool visible) { showOverlay_ = visible; }
    void Show();
    void Hide();
    void Minimize();
    void Restore();
    void SetAppObject(QObject *app) { app_ = app; }
    void Resize(bool forceFullscreen = false);
    bool event(QEvent *event) override;

    QWidget *videoPanel_;

private:
    Ui::Overlay ui_;
    //QLabel *overlayLabel_;
    //QWidget *videoPanel_;
    QObject *app_;
    bool showOverlay_ = false;
};


#endif
