#include "VideoControlPanel.h"

VideoControlPanel::VideoControlPanel(QWidget *parent) : QWidget(parent)
{
    ui_.setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground, true);

}
