#include "VideoOverlay.h"
#include "SpyderPlayerApp.h"
#include <QPixmap>


#include "VideoOverlay.h"
#include "SpyderPlayerApp.h"
#include <QVBoxLayout> // Add this include for QVBoxLayout
#include <QPixmap>

VideoOverlay::VideoOverlay(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background-color: black; border: none;");

    // Create a layout for the VideoOverlay widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // No margins
    layout->setSpacing(0); // No spacing

    videoPanel_ = new QWidget(this);
    videoPanel_->setStyleSheet("background-color: black; border: none;"); // Optional: ensure consistent styling

    blankOverlay_ = new QLabel(this);
    blankOverlay_->setStyleSheet("background-color: black; border: none;");
    blankOverlay_->setPixmap(QPixmap(":/icons/icons/BlankScreenLogo.png"));
    blankOverlay_->setAlignment(Qt::AlignCenter); // Center the pixmap
    blankOverlay_->setScaledContents(false); // Prevent stretching

    overlayStack_ = new QStackedWidget(this);
    overlayStack_->addWidget(videoPanel_);
    overlayStack_->addWidget(blankOverlay_);

    // Add the stacked widget to the layout
    layout->addWidget(overlayStack_);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    setMouseTracking(true);
    installEventFilter(this);
    overlayStack_->setCurrentIndex(0); 
}

VideoOverlay::~VideoOverlay()
{
    delete videoPanel_;
    delete blankOverlay_;
    delete overlayStack_;
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

void VideoOverlay::ShowVideoPanel()
{
    overlayStack_->setCurrentIndex(0);
}

void VideoOverlay::ShowBlankOverlay()
{
    overlayStack_->setCurrentIndex(1);
}