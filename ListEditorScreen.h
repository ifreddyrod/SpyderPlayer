#ifndef LISTEDITORSCREEN_H
#define LISTEDITORSCREEN_H

#include "Global.h"
#include "DraggableWidget.h"
#include "AppData.h"
#include "ui_PlayListSettings.h"
#include "WidgetDelegates.h"

class SettingsManager;

class ListEditor : public DraggableWidget
{
    Q_OBJECT
public:
    explicit ListEditor(SettingsManager* settingsManager, ENUM_SETTINGS_VIEWS viewType);
    ~ListEditor();
    void ShowListEditor();
    void AddEditList(QVector<PlayListEntry> list);

private:
    void BackButtonClicked();
    void ReorderButtonClicked();
    void ApplyButtonClicked();
    void CancelButtonClicked();
    void UpdateTable();
    void UnSelectRows();
    void RowSelected(int row, int col);
    void DeleteButtonClicked();
    void EditButtonClicked();
    void AddNewButtonClicked();

    bool eventFilter(QObject *object, QEvent *event) override;
    void dropEvent(QDropEvent* event) override;

    Ui::PlayListSettings ui_;
    SettingsManager* settingsManager_;
    ENUM_SETTINGS_VIEWS viewType_;
    QVector<PlayListEntry> editList_;
    QVector<PlayListEntry> tempList_;
    ReadOnlyDelegate* cellDelegate_;
    int currentRow_;
    bool reordering_ = false;

};

#endif