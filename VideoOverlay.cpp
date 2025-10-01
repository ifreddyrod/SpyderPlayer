#include "VideoOverlay.h"
#include "SpyderPlayerApp.h"
#include <QPixmap>
#include <QVBoxLayout> // Already included

VideoOverlay::VideoOverlay(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background-color: black; border: none;");

    // Create a layout for the VideoOverlay widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // No margins
    layout->setSpacing(0); // No spacing

    videoPanel_ = new QWidget(this);
    videoPanel_->setStyleSheet("background-color: black; border: none;"); 

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

    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    titleLabel_ = new QLabel(this);
    titleLabel_->setFixedSize(screenGeometry.width() , 70);
    titleLabel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    titleLabel_->setStyleSheet("background-color: transparent; color: white; font-size: 18px;"); 
    titleLabel_->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter); 
    titleLabel_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    titleLabel_->raise(); 
    titleLabel_->hide(); 
}

VideoOverlay::~VideoOverlay()
{
    delete titleLabel_; // New: Clean up the label
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

    setFixedSize(panelWidth, panelHeight);  
    titleLabel_->setFixedWidth(panelWidth-new_x);

    QPoint global_pos = panel->mapToGlobal(QPoint(new_x, new_y));
    move(global_pos);

    //overlayStack_->setGeometry(0, 0, this->width(), this->height());
}

void VideoOverlay::ShowVideoPanel()
{
    overlayStack_->setCurrentIndex(0);
}

void VideoOverlay::ShowBlankOverlay()
{
    overlayStack_->setCurrentIndex(1);
    // New: Always hide the title when showing blank
    SetTitleVisible(false);
}

// New function: Set the title text
void VideoOverlay::SetTitleText(const QString &text)
{
    titleLabel_->setText(text);
}

// New function: Toggle title visibility (only applies if video panel is shown)
void VideoOverlay::SetTitleVisible(bool visible)
{
    titleVisible_ = visible;
    if (overlayStack_->currentIndex() == 0) 
    {
        titleLabel_->setVisible(visible);
    }
}
