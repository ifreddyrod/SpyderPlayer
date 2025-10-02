#ifndef VIDEO_OVERLAY_H
#define VIDEO_OVERLAY_H

#include "ui_Overlay.h"
#include "Global.h"

#include <QWidget>
#include <QLabel>
#include <QApplication>
#include <QScreen>
#include <QStackedWidget>

class TitleBar: public QWidget
{
    Q_OBJECT

public:
    TitleBar(QWidget *parent = nullptr);

    Ui::Overlay ui_;
};


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
    void SetAppObject(QObject *app);
    void Resize(bool forceFullscreen = false);
    bool event(QEvent *event) override;
    void ShowVideoPanel();
    void ShowBlankOverlay();

    // New functions
    void SetTitleText(const QString &text);
    void SetTitleVisible(bool visible);

    QStackedWidget *overlayStack_;
    QWidget *videoPanel_;
    QLabel *blankOverlay_;
    QObject *app_;
    bool showOverlay_ = false;
    TitleBar *titleBar_;
    QLabel *titleLabel_;
    bool titleVisible_ = false;
};

#endif