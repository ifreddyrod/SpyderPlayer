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

VideoOverlay::VideoOverlay(QWidget *parent, VideoPanelType panelType ) : QWidget(parent)
{
    panelType_ = panelType;
    setStyleSheet("background-color: black; border: none;");

    // Create a layout for the VideoOverlay widget
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0); // No margins
    layout->setSpacing(0); // No spacing

    videoPanel_ = new QVideoWidget(this);
    videoPanel_->setStyleSheet("background-color: black; border: none;"); 

    videoWidget_ = new QWidget(this);
    videoWidget_->setStyleSheet("background-color: black; border: none;"); 

    blankOverlay_ = new QLabel(this);
    blankOverlay_->setStyleSheet("background-color: black; border: none;");
    blankOverlay_->setPixmap(QPixmap(":/icons/icons/BlankScreenLogo.png"));
    //blankOverlay_->setPixmap(QPixmap(":/icons/icons/BlankScreen.png"));
    blankOverlay_->setAlignment(Qt::AlignCenter); // Center the pixmap
    blankOverlay_->setScaledContents(false); // Prevent stretching

    overlayStack_ = new QStackedWidget(this);
    overlayStack_->addWidget(blankOverlay_);
    overlayStack_->addWidget(videoWidget_);
    overlayStack_->addWidget(videoPanel_);

    // Add the stacked widget to the layout
    layout->addWidget(overlayStack_);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);

    setMouseTracking(true);
    installEventFilter(this);
    overlayStack_->setMouseTracking(true);
    overlayStack_->installEventFilter(this);
    overlayStack_->setCurrentIndex(int(panelType_));

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
        if (overlayStack_->currentIndex() == (int)VideoPanelType::QVideoPanel) 
        {
            videoPanel_->show();
            videoWidget_->hide();
        }
        else if (overlayStack_->currentIndex() == (int)VideoPanelType::QWidget) 
        {
            videoWidget_->show();
            videoPanel_->hide();
        }
        else
        {
            videoWidget_->hide();
            videoPanel_->hide();
        }
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
    //int currentIndex = overlayStack_->currentIndex();

    SpyderPlayerApp *spyderPlayerApp = (SpyderPlayerApp *)app_; 

    //QScreen *screen = QApplication::primaryScreen();  // Get primary screen
    //QRect screenGeometry = screen->geometry();  // Get screen geometry
    //int screenWidth = screenGeometry.size().width(); 
    
    QWidget *panel = spyderPlayerApp->GetVideoPanelWidget();

    if (panel == nullptr)
    {
        //PRINT << "Panel is null";
        return;
    }

    if (panel->width() == 0 || panel->height() == 0)
        return;

    int panelWidth = spyderPlayerApp->GetVideoPanelWidth();
    int panelHeight = spyderPlayerApp->GetVideoPanelHeight();
    int new_x =  panel->x();
    int new_y =  panel->y();

    //PRINT << "Coordinates: " << new_x << ", " << new_y;
    //PRINT << "Width: " << panelWidth << ", Height: " << panelHeight;

    setFixedSize(panelWidth, panelHeight);  

    if (panelType_ == VideoPanelType::QVideoPanel && forceFullscreen) 
        videoPanel_->showFullScreen();
    else if (panelType_ == VideoPanelType::QWidget && forceFullscreen) 
        videoWidget_->showFullScreen();
    
    if(overlayStack_->currentIndex() == 0)
    {
        videoPanel_->hide();
        videoWidget_->hide();
    }

    titleBar_->setFixedWidth(panelWidth); 

    QPoint global_pos = panel->mapToGlobal(QPoint(new_x, new_y));
    move(global_pos);

    titleBar_->move(global_pos);
}

void VideoOverlay::ShowVideoPanel()
{
    overlayStack_->setCurrentIndex(int(panelType_));
    if (panelType_ == VideoPanelType::QVideoPanel)
        videoPanel_->show();
    else if (panelType_ == VideoPanelType::QWidget)
        videoWidget_->show();
}

void VideoOverlay::ShowBlankOverlay()
{
    videoPanel_->hide();
    videoWidget_->hide();
    overlayStack_->setCurrentIndex(0);
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
        titleBar_->setVisible(false);
    }
    else
    {
        titleBar_->setVisible(visible);
        if (visible)    titleBar_->raise();
    }
}

QVideoWidget* VideoOverlay::NewVideoPanel()
{
    if (panelType_ != VideoPanelType::QVideoPanel) 
        return nullptr;

    // Remove old panel from stack
    int currentIndex = overlayStack_->currentIndex();
    overlayStack_->setCurrentIndex(int(panelType_));
    overlayStack_->removeWidget(videoPanel_);

    // Delete the old plain QWidget
    videoPanel_->deleteLater(); 
    videoPanel_ = nullptr;

    // Set the new panel (e.g., QVideoWidget)
    videoPanel_ = new QVideoWidget(this);
    videoPanel_->setStyleSheet("background-color: black; border: none;");  // Reuse style

    // Re-add to stack at the video position 
    overlayStack_->insertWidget(int(panelType_), videoPanel_);
    //overlayStack_->addWidget(videoPanel_);

    // Forward mouse tracking/event filter to new panel if applicable
    videoPanel_->setMouseTracking(true);
    videoPanel_->installEventFilter(this);
    videoPanel_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Restore current index if it was the video panel
    overlayStack_->setCurrentIndex(currentIndex);

    Resize();

    return videoPanel_;
}