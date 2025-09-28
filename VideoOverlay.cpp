#include "VideoOverlay.h"
#include "SpyderPlayerApp.h"

VideoOverlay::VideoOverlay(QWidget *parent) : QWidget(parent)
{
    ui_.setupUi(this);

    videoPanel_ = ui_.Overlay_widget;

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    //setAttribute(Qt::WA_TranslucentBackground, true);

    // Debug  - This will make the overlay much more visible // 100
    //videoPanel_->setStyleSheet("background-color: rgba(255, 255, 255, 0); border: none;"); //rgba(0, 0, 0, 2)
    //setStyleSheet("background-color: black; border: none;");

    // Normal
    //overlayLabel_->setStyleSheet("color: white; background-color: rgba(0, 0, 0, 2); font:30pt; border: none;"); //rgba(0, 0, 0, 2)

    //QString overlayTxt = "";
    //ui_.Overlay_label->setText(overlayTxt);
    videoPanel_->setMouseTracking(true);
    installEventFilter(this);
    //show();
}

VideoOverlay::~VideoOverlay()
{
    delete videoPanel_;
}

void VideoOverlay::Show()
{
    if (showOverlay_)
    {
        show();
        videoPanel_->show();
    }
}

void VideoOverlay::Hide()
{
    hide();
}

void VideoOverlay::Minimize()
{
    if (showOverlay_)
        showMinimized();
}

void VideoOverlay::Restore()
{
    if (showOverlay_)
        showNormal();
}

bool VideoOverlay::event(QEvent *event)
{
    if (app_ == nullptr || !showOverlay_)
        return false;

    //PRINT << "VideoOverlay:: Event: " << event->type();

    QApplication::sendEvent(app_, event);
    return true;
}

void VideoOverlay::Resize(bool forceFullscreen)
{
    if (app_ == nullptr || !showOverlay_)
    {   
        return;
    }

    SpyderPlayerApp *spyderPlayerApp = (SpyderPlayerApp *)app_; 

    QScreen *screen = QApplication::primaryScreen();  // Get primary screen
    QRect screenGeometry = screen->geometry();  // Get screen geometry
    int screenWidth = screenGeometry.size().width(); 

    QWidget *panel = spyderPlayerApp->GetVideoPanelWidget();

    int panelWidth = forceFullscreen ? screenWidth: spyderPlayerApp->GetVideoPanelWidth();
    int panelHeight = spyderPlayerApp->GetVideoPanelHeight();
    int new_x =  panel->x();
    int new_y =  panel->y();

    //PRINT << "Coordinates: " << new_x << ", " << new_y;
    //PRINT << "Width: " << panelWidth << ", Height: " << panelHeight;

    setFixedWidth(panelWidth);
    setFixedHeight(panelHeight);  
    
    QPoint global_pos = panel->mapToGlobal(QPoint(new_x, new_y));
    move(global_pos);
}

