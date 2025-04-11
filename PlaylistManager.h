#ifndef PLAYLISTMANAGER_H
#define PLAYLISTMANAGER_H

#include "AppData.h"
#include <QObject>
#include <QWidget>
#include <QTreeWidgetItem>
#include <QColor>
#include <QString>
#include <QVariant>
#include <QSignalMapper> 
#include <QPointer> 
#include <QNetworkAccessManager>


class TreeItem : public QTreeWidgetItem
{
    //Q_OBJECT
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
    TreeItem* Child(int index);
    TreeItem* Parent();

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
    void LoadLibrary();
    void LoadFavorites();
    void SaveFavorites();
    void CollapseAllPlaylists();
    void ExpandAllPlaylists();
    void SortPlaylistDescending();
    void SortPlaylistAscending();
    void GotoTopOfList();
    void GotoBottomOfList();
    QPair<string, string> GetAdjacentChannel(bool forward);
    QPair<string, string> GotoLastSelectedChannel();
    void SearchChannels(QString searchText);
    bool PlaylistTreeHasFocus();
    void PlaySelectedTreeItem();
    void AddSessionMedia(PlayListEntry playlist);
    void LoadSessionPlayLists();
    void LoadSessionMedia();

signals:
    void SIGNAL_PlaySelectedChannel(string, string);

private:
    QString LoadStyleSheet();

    void EmitTreeLayoutChanged();
    void AppendPlayList(TreeItem* newPlayList, TreeItem* targetItem = nullptr);
    void AppendChannel(TreeItem* playList, TreeItem* newChannel);
    void ClearPlayListItems(TreeItem* playList);
    void UpdatePlayListChannelCount(TreeItem* item, int count = -1);

    void ItemClicked(QTreeWidgetItem* item);
    void ItemDoubleClicked(QTreeWidgetItem* item); 
    TreeItem* GetChannelFromTree(QString playListName, QString channelName, QString source);
    TreeItem* GetPlayListFromTree(QString playListName);
    TreeItem* GetPlayListFromSearch(QString playListName);
    void ToggleItemCheckedinList(TreeItem* playList, TreeItem* item, bool checked = true);
    void AddRemoveFavorites(QTreeWidgetItem* item);
    bool IsEntryInSessionPlayList(PlayListEntry playlist);
    bool IsEntryInSessionMedia(PlayListEntry media);

    // Event Overrides
    //bool eventFilter(QObject *object, QEvent *event) override;


    // Private Member Variables
    QSignalMapper *treeItemSelectedSignal;
    TreeItem* currentSelectedItem_;
    TreeItem* lastSelectedItem_;
    int searchResultsCount_ = 0; 
    int currentItem_ = 0;
    
    QList<PlayListEntry> openedSessionPlayLists_;
    QList<PlayListEntry> openedSessionFiles_;
    QString lastSearchQuery_;

    AppData* appData_; 
    QTreeWidget* playlistTree_;
    TreeItem* searchList_;
    TreeItem* favoritesList_;
    TreeItem* libraryList_;
    TreeItem* sessionMediaList_;
    QNetworkAccessManager* networkManager_;
};

#endif // PLAYLISTMANAGER_H
