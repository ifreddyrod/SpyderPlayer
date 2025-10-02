#ifndef DRAGGABLEWIDGET_H
#define DRAGGABLEWIDGET_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>

class DraggableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DraggableWidget(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    // Resizing logic
    enum class ResizeEdge { None, Left, Right, Top, Bottom, TopLeft, TopRight, BottomLeft, BottomRight };
    ResizeEdge resizeEdge_;
    bool isResizing_;
    QPoint resizeStartPos_;
    QRect resizeStartGeometry_;
    static constexpr int RESIZE_MARGIN = 5; // Pixels near edge for resize detection

    void startResizing(QMouseEvent *event);
    void updateResizing(QMouseEvent *event);
    void stopResizing(QMouseEvent *event);
    ResizeEdge getResizeEdge(const QPoint &pos);
    void updateCursor(const QPoint &pos);

private:
    QPoint mousePressPos_;
    bool mouseMoveActive_ = false;
};

#endif // DRAGGABLEWIDGET_H
