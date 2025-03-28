#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QTreeWidgetItem>
#include <QColor>
#include <QString>
#include <QVariant>
#include <QSignalMapper> 
#include <QPointer> 
#include "AppData.h"

class TreeItem : public QTreeWidgetItem
{
public:
    TreeItem(QString nameText, QColor bgColor = QColor(), bool isPlayList = false, bool isPersistent = true);
    void SetPlayListName(QString name);
    QString GetPlayListName();
    QString GetItemName();
    void SetSource(QString source);
    QString GetSource();
    void SetItemChecked(bool checked);
    bool IsItemChecked();
    bool IsItemPersistent();
    QString GetParentName();
    bool IsPlayList(); 

private:
    QString playListName_;
    QString itemName_;
    QString source_;
    bool isPlayList_;
    bool isPersistent_;

};

class PlaylistManager : public QWidget
{
    Q_OBJECT

public:
    PlaylistManager(QTreeWidget* playlistTreefromUI, AppData* appData, QWidget* parent = nullptr);
    
    void ResetAllLists();
    void LoadPlayList(PlayListEntry playlist, bool isPersistent = true);

private:
    QString LoadStyleSheet();

    void EmitTreeLayoutChanged();
    void AppendPlayList(TreeItem* newPlayList, TreeItem* targetItem = nullptr);
    void AppendChannel(TreeItem* playList, TreeItem* newChannel);
    void UpdatePlayListChannelCount(TreeItem* item, int count = -1);
    void CollapseAllPlaylists();
    void ExpandAllPlaylists();

    QSignalMapper *treeItemSelectedSignal;
    QPointer<TreeItem> currentSelectedItem_;
    QPointer<TreeItem> lastSelectedItem_;
    int searchResultsCount_ = 0; 
    int currentItem_ = 0;
    QPointer<TreeItem> openedFilesList_;
    QList<PlayListEntry> openedSessionPlayLists_;
    QList<PlayListEntry> openedSessionFiles_;

    AppData* appData_; 
    QTreeWidget* playlistTree_;
};

#endif // PLAYLISTMANAGER_H
