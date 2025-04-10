#ifndef HOTKEYSETTINGS_H
#define HOTKEYSETTINGS_H

#include "Global.h"
#include "DraggableWidget.h"
#include "ui_HotkeySettings.h"
#include "WidgetDelegates.h"
#include "AppData.h"
#include <QMap>
#include <QTableWidgetItem> 
#include <QStyledItemDelegate>

class SettingsManager;

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