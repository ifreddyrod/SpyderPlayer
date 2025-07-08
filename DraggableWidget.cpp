#include "DraggableWidget.h"

DraggableWidget::DraggableWidget(QWidget *parent)
    : QWidget(parent)
    , mousePressPos_()
    , mouseMoveActive_(false)
    , resizeEdge_(ResizeEdge::None)
    , isResizing_(false)
{
}

/*void DraggableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Capture the global position where the mouse was pressed
        mousePressPos_ = event->globalPosition().toPoint();
        mouseMoveActive_ = true;
    }
}*/
void DraggableWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Check if the click is on a child widget
        QWidget *child = childAt(event->pos());

        // Check for resize edge first
        resizeEdge_ = getResizeEdge(event->pos());
        if (resizeEdge_ != ResizeEdge::None)
        {
            startResizing(event);
            event->accept();
        }
        else if (child) 
        {
            mousePressPos_ = event->pos() - frameGeometry().topLeft();
            mouseMoveActive_ = true;
            event->accept();
            return;
        }
        else
        {
            mousePressPos_ = event->globalPosition().toPoint() - frameGeometry().topLeft();
            mouseMoveActive_ = true;
            event->accept();
        }
    }
}

/*void DraggableWidget::mouseMoveEvent(QMouseEvent *event)
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
}*/
void DraggableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isResizing_)
    {
        updateResizing(event);
        event->accept();
    }
    else if (mouseMoveActive_ && !mousePressPos_.isNull())
    {
        // Move the top-level window
        QWidget *target = window();
        target->move(event->globalPosition().toPoint() - mousePressPos_);
        event->accept();
    }
    else
    {
        updateCursor(event->pos());
        event->accept();
    }
}

/*void DraggableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        // Reset when the left mouse button is released
        mousePressPos_ = QPoint();
        mouseMoveActive_ = false;
    }
}*/

void DraggableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (isResizing_)
        {
            stopResizing(event);
            event->accept();
        }
        else if (mouseMoveActive_)
        {
            mousePressPos_ = QPoint();
            mouseMoveActive_ = false;
            event->accept();
        }
    }
}

void DraggableWidget::startResizing(QMouseEvent *event)
{
    isResizing_ = true;
    resizeStartPos_ = event->globalPosition().toPoint();
    resizeStartGeometry_ = geometry();
}

void DraggableWidget::updateResizing(QMouseEvent *event)
{
    QPoint delta = event->globalPosition().toPoint() - resizeStartPos_;
    QRect newGeometry = resizeStartGeometry_;

    switch (resizeEdge_)
    {
    case ResizeEdge::Left:
        newGeometry.setLeft(resizeStartGeometry_.left() + delta.x());
        break;
    case ResizeEdge::Right:
        newGeometry.setRight(resizeStartGeometry_.right() + delta.x());
        break;
    case ResizeEdge::Top:
        newGeometry.setTop(resizeStartGeometry_.top() + delta.y());
        break;
    case ResizeEdge::Bottom:
        newGeometry.setBottom(resizeStartGeometry_.bottom() + delta.y());
        break;
    case ResizeEdge::TopLeft:
        newGeometry.setTopLeft(resizeStartGeometry_.topLeft() + delta);
        break;
    case ResizeEdge::TopRight:
        newGeometry.setTopRight(resizeStartGeometry_.topRight() + QPoint(delta.x(), delta.y()));
        break;
    case ResizeEdge::BottomLeft:
        newGeometry.setBottomLeft(resizeStartGeometry_.bottomLeft() + QPoint(delta.x(), delta.y()));
        break;
    case ResizeEdge::BottomRight:
        newGeometry.setBottomRight(resizeStartGeometry_.bottomRight() + delta);
        break;
    default:
        break;
    }

    newGeometry.setWidth(qMax(newGeometry.width(), minimumWidth()));
    newGeometry.setHeight(qMax(newGeometry.height(), minimumHeight()));
    setGeometry(newGeometry);
}

void DraggableWidget::stopResizing(QMouseEvent *event)
{
    isResizing_ = false;
    resizeEdge_ = ResizeEdge::None;
}

DraggableWidget::ResizeEdge DraggableWidget::getResizeEdge(const QPoint &pos)
{
    QRect rect = this->rect();
    bool left = pos.x() <= RESIZE_MARGIN;
    bool right = pos.x() >= rect.width() - RESIZE_MARGIN;
    bool top = pos.y() <= RESIZE_MARGIN;
    bool bottom = pos.y() >= rect.height() - RESIZE_MARGIN;

    if (left && top)
        return ResizeEdge::TopLeft;
    if (right && top)
        return ResizeEdge::TopRight;
    if (left && bottom)
        return ResizeEdge::BottomLeft;
    if (right && bottom)
        return ResizeEdge::BottomRight;
    if (left)
        return ResizeEdge::Left;
    if (right)
        return ResizeEdge::Right;
    if (top)
        return ResizeEdge::Top;
    if (bottom)
        return ResizeEdge::Bottom;

    return ResizeEdge::None;
}

void DraggableWidget::updateCursor(const QPoint &pos)
{
    ResizeEdge edge = getResizeEdge(pos);
    switch (edge)
    {
    case ResizeEdge::Left:
    case ResizeEdge::Right:
        setCursor(Qt::SizeHorCursor);
        break;
    case ResizeEdge::Top:
    case ResizeEdge::Bottom:
        setCursor(Qt::SizeVerCursor);
        break;
    case ResizeEdge::TopLeft:
    case ResizeEdge::BottomRight:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case ResizeEdge::TopRight:
    case ResizeEdge::BottomLeft:
        setCursor(Qt::SizeBDiagCursor);
        break;
    default:
        setCursor(Qt::ArrowCursor);
        break;
    }
}