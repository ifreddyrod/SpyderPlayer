#ifndef WIDGETDELEGATES_H
#define WIDGETDELEGATES_H

#include <QStyledItemDelegate>

//*****************************************************************************************
// Make the cell read-only Delegate
//***************************************************************************************** 
class ReadOnlyDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ReadOnlyDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override
    {
        Q_UNUSED(parent);
        Q_UNUSED(option);
        Q_UNUSED(index);
        return nullptr; // Return nullptr to make the cell read-only
    }
};

//*****************************************************************************************
// Make cell combo box Delegate
//***************************************************************************************** 
class ComboBoxDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ComboBoxDelegate(const QStringList &items, QObject *parent = nullptr); 

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setItems(const QStringList &items) { items_ = items; }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void setEnabled(bool enabled) { enabled_ = enabled; }

private:
    QStringList items_;
    bool enabled_;
};

#endif // WIDGETDELEGATES_H