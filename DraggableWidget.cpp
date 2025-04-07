#include "DraggableWidget.h"

DraggableWidget::DraggableWidget(QWidget *parent)
    : QWidget(parent)
    , mousePressPos_()
    , mouseMoveActive_(false)
{
}

void DraggableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Capture the global position where the mouse was pressed
        mousePressPos_ = event->globalPosition().toPoint();
        mouseMoveActive_ = true;
    }
}

void DraggableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseMoveActive_ && !mousePressPos_.isNull())
    {
        // Calculate how far the mouse moved
        QPoint delta = event->globalPosition().toPoint() - mousePressPos_;
        // Move the widget (or window) by the same amount
        parentWidget()->move(parentWidget()->pos() + delta);
        // Update the press position for the next movement calculation
        mousePressPos_ = event->globalPosition().toPoint();
    }
}

void DraggableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Reset when the left mouse button is released
        mousePressPos_ = QPoint();
        mouseMoveActive_ = false;
    }
}
