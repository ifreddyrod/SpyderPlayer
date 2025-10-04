#include "VideoOverlay.h"
#include "SpyderPlayerApp.h"
#include <QPixmap>
#include <QVBoxLayout> // Already included

TitleBar::TitleBar(QWidget *parent) : QWidget(parent)
{
    ui_.setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground, true);
    setMouseTracking(true);
    //setStyleSheet("background-color: white;");
}

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
    overlayStack_->setMouseTracking(true);
    overlayStack_->installEventFilter(this);
    overlayStack_->setCurrentIndex(0);

    titleBar_ = new TitleBar(this);
    //titleBar_->setFixedHeight(60);
    titleLabel_ = titleBar_->ui_.CenteredText;
    titleLabel_->setStyleSheet("background-color: rgba(0, 0, 0, 200); \n" 
                                "border-radius: 10px; \n" 
                                "padding: 10px; \n"
                                "color: white; \n"
                                "font-size: 30px; \n");
    titleLabel_->adjustSize();
}

VideoOverlay::~VideoOverlay()
{
    delete titleBar_;
    delete overlayStack_;
}

void VideoOverlay::SetAppObject(QObject *app) 
{ 
    app_ = app; 

    titleBar_->installEventFilter((SpyderPlayerApp *)app_);  
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

    //QScreen *screen = QApplication::primaryScreen();  // Get primary screen
    //QRect screenGeometry = screen->geometry();  // Get screen geometry
    //int screenWidth = screenGeometry.size().width(); 

    QWidget *panel = spyderPlayerApp->GetVideoPanelWidget();

    int panelWidth = spyderPlayerApp->GetVideoPanelWidth();
    int panelHeight = spyderPlayerApp->GetVideoPanelHeight();
    int new_x =  panel->x();
    int new_y =  panel->y();

    //PRINT << "Coordinates: " << new_x << ", " << new_y;
    //PRINT << "Width: " << panelWidth << ", Height: " << panelHeight;

    setFixedSize(panelWidth, panelHeight);  
    if (forceFullscreen) videoPanel_->showFullScreen();
    
    titleBar_->setFixedWidth(panelWidth); 

    QPoint global_pos = panel->mapToGlobal(QPoint(new_x, new_y));
    move(global_pos);

    titleBar_->move(global_pos);
}

void VideoOverlay::ShowVideoPanel()
{
    overlayStack_->setCurrentIndex(0);
}

void VideoOverlay::ShowBlankOverlay()
{
    overlayStack_->setCurrentIndex(1);
    SetTitleVisible(false);
}

void VideoOverlay::SetTitleText(const QString &text)
{
    if (titleBar_->isVisible())
    {
        titleBar_->hide();
        titleLabel_->setText("");
        titleBar_->show();
    }
    titleLabel_->setText(text);
    titleLabel_->adjustSize();
}

void VideoOverlay::SetTitleVisible(bool visible)
{
    titleVisible_ = visible;
    if (overlayStack_->currentIndex() == 0) 
    {
        titleBar_->setVisible(visible);
        if (visible)    titleBar_->raise();
    }
    else
    {
        titleBar_->setVisible(false);
    }
}
