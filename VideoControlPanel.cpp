#include "VideoControlPanel.h"

VideoControlPanel::VideoControlPanel(QWidget *parent) : QWidget(parent)
{
    ui.setupUi(this);

    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
    setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground, true);

}
