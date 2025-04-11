#ifndef ENTRYEDITORSCREEN_H
#define ENTRYEDITORSCREEN_H

#include "Global.h"
#include "DraggableWidget.h"
#include "AppData.h"
#include "ui_EntryEditor.h"

class SettingsManager;  

class EntryEditor : public DraggableWidget
{
    Q_OBJECT

public:
    explicit EntryEditor(SettingsManager* settingsManager, ENUM_SETTINGS_VIEWS viewType);
    ~EntryEditor();
    void ShowEntryEditor(int index = -1);

private:
    void SourceTypeChanged();
    void EntryNameChanged();
    void SourceChanged();
    void OpenFileButtonClicked();
    void SaveButtonClicked();
    void BackButtonClicked();

    Ui::EntryEditor ui_;
    SettingsManager* settingsManager_;
    ENUM_SETTINGS_VIEWS viewType_;
    int editListIndex_ = -1;
    PlayListEntry editEntry_;
    bool modified_ = false;

};
#endif // ENTRYEDITORSCREEN_H