#ifndef HOTKEYSETTINGS_H
#define HOTKEYSETTINGS_H

#include "Global.h"
#include "DraggableWidget.h"
#include "ui_HotkeySettings.h"
#include "AppData.h"
#include <QMap>
#include <QTableWidgetItem> 
#include <QStyledItemDelegate>

class SettingsManager;

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


//*****************************************************************************************
// HotKey Settings Class
//***************************************************************************************** 
class HotKeySettings : public DraggableWidget
{
    Q_OBJECT
public:
    HotKeySettings(SettingsManager* settingsManager);
    ~HotKeySettings();
    void LoadHotKeySettings();

private:
    void BackButtonClicked();
    void EditButtonClicked();
    void CancelButtonClicked();
    void ApplyButtonClicked();
    void HotkeyChanged(QTableWidgetItem* item);
    bool CheckForDuplicateHotkeys();
    

    Ui::HotKeySettings ui_;
    SettingsManager* settingsManager_;
    bool modified_ = false;
    QMap<QString, int> hotkeys_;
    ReadOnlyDelegate* playerAction_;
    ComboBoxDelegate* hkComboBox_;
};

#endif