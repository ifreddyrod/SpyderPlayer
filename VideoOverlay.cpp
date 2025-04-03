#include "VideoOverlay.h"
#include "SpyderPlayerApp.h"

VideoOverlay::VideoOverlay(QWidget *parent) : QWidget(parent)
{
    ui_.setupUi(this);

    overlayLabel_ = ui_.Overlay_label;

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground, true);

    // Debug  - This will make the overlay much more visible
    //overlayLabel_->setStyleSheet("color: white; background-color: rgba(255, 255, 255, 100); font:30pt; border: none;"); //rgba(0, 0, 0, 2)

    // Normal
    overlayLabel_->setStyleSheet("color: white; background-color: rgba(0, 0, 0, 2); font:30pt; border: none;"); //rgba(0, 0, 0, 2)

    QString overlayTxt = "";
    ui_.Overlay_label->setText(overlayTxt);
    overlayLabel_->setMouseTracking(true);
    installEventFilter(this);
}

VideoOverlay::~VideoOverlay()
{
    delete overlayLabel_;
}

bool VideoOverlay::event(QEvent *event)
{
    if (app_ == nullptr)
        return false;

    //PRINT << "VideoOverlay:: Event: " << event->type();

    QApplication::sendEvent(app_, event);
    return true;
}

void VideoOverlay::Resize(bool forceFullscreen)
{
    if (app_ == nullptr)
    {   
        return;
    }

    SpyderPlayerApp *spyderPlayerApp = (SpyderPlayerApp *)app_; 

    QScreen *screen = QApplication::primaryScreen();  // Get primary screen
    QRect screenGeometry = screen->geometry();  // Get screen geometry
    int screenWidth = screenGeometry.size().width(); 

    QWidget *panel = spyderPlayerApp->GetVideoPanelWidget();

    int panelWidth = forceFullscreen ? screenWidth: spyderPlayerApp->GetVideoPanelWidth() - 4;
    int panelHeight = spyderPlayerApp->GetVideoPanelHeight() - 4;
    int new_x =  panel->x() + 1;
    int new_y =  panel->y() + 1;

    //PRINT << "Coordinates: " << new_x << ", " << new_y;
    //PRINT << "Width: " << panelWidth << ", Height: " << panelHeight;

    setFixedWidth(panelWidth);
    setFixedHeight(panelHeight);  
    
    QPoint global_pos = panel->mapToGlobal(QPoint(new_x, new_y));
    move(global_pos);
}

