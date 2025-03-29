#include "PlaylistManager.h"
#include "M3UParser.h"
#include "Global.h"

//***********************************************************
// Color Definitions
//***********************************************************
#define PLAYLIST_COLOR  QColor(20, 6, 36)
#define LIBRARY_COLOR  QColor(20, 6, 36)
#define FAVORITES_COLOR  QColor(20, 6, 36)
#define SEARCH_COLOR   QColor(20, 6, 36) 
#define SESSION_COLOR   QColor(39, 0, 42) 


//************************************************************************************************
// TreeItem Class
//************************************************************************************************
TreeItem::TreeItem(QString nameText, QColor bgColor, bool isPlayList, bool isPersistent)
{
    setText(0, nameText);
    itemName_ = nameText.trimmed();
    playListName_ = "";
    isPlayList_ = isPlayList;
    isPersistent_ = isPersistent;

    if (bgColor.isValid()) 
    {
        setBackground(0, bgColor);
    }
}

void TreeItem::SetPlayListName(QString name) 
{
    playListName_ = name;
}

QString TreeItem::GetPlayListName()
{
    return playListName_;
}

QString TreeItem::GetItemName() 
{
    return itemName_;
}

void TreeItem::SetSource(QString source) 
{
    setData(0, Qt::UserRole, source);
}

QString TreeItem::GetSource() 
{
    return data(0, Qt::UserRole).toString();
}

void TreeItem::SetItemChecked(bool checked) 
{
    setCheckState(0, checked ? Qt::Checked : Qt::Unchecked);
}

bool TreeItem::IsItemChecked()
{
    return checkState(0) == Qt::Checked;
}

bool TreeItem::IsItemPersistent() 
{
    return isPersistent_;
}

bool TreeItem::IsPlayList() 
{
    return isPlayList_;
}

QString TreeItem::GetParentName() 
{
    QString parentName = parent()->text(0); 

    int index = parentName.indexOf("  (");
    if (index != -1) {
        parentName = parentName.left(index);
    }
    return parentName.trimmed();
}


//************************************************************************************************
// PlaylistManager Class
//************************************************************************************************
PlaylistManager::PlaylistManager(QTreeWidget* playlistTreefromUI, AppData* appData, QWidget *parent)
{
    playlistTree_ = playlistTreefromUI;
    appData_ = appData;

    //--------------------------------
    // Setup the Tree
    //--------------------------------
    playlistTree_->setColumnCount(1);
    playlistTree_->setStyleSheet(LoadStyleSheet());
    setMouseTracking(true);
    lower();

    ResetAllLists();

    //--------------------------------
    // Connect Event Handlers
    //--------------------------------
    playlistTree_->installEventFilter(this);
    connect(playlistTree_, &QTreeWidget::itemClicked, this, &PlaylistManager::ItemClicked);
    connect(playlistTree_, &QTreeWidget::itemDoubleClicked, this, &PlaylistManager::ItemDoubleClicked);
    connect(playlistTree_, &QTreeWidget::itemChanged, this, &PlaylistManager::AddRemoveFavorites);

}

void PlaylistManager::ResetAllLists()
{
    // Clear the Tree
    playlistTree_->clear();

    // Add Search List
    TreeItem* searchList = new TreeItem(PAD("Search Results"), SEARCH_COLOR, true);
    searchList->setIcon(0, QIcon(":/icons/icons/search-list.png"));
    playlistTree_->addTopLevelItem(searchList);
    searchList_ = searchList;

    // Add Favorites List
    TreeItem* favoritesList = new TreeItem(PAD("Favorites"), FAVORITES_COLOR, true);
    favoritesList->setIcon(0, QIcon(":/icons/icons/star-white.png"));
    playlistTree_->addTopLevelItem(favoritesList);
    favoritesList_ = favoritesList;
    
    // Add Library List
    TreeItem* libraryList = new TreeItem(PAD("Library"), LIBRARY_COLOR, true);
    libraryList->setIcon(0, QIcon(":/icons/icons/library.png"));
    playlistTree_->addTopLevelItem(libraryList);
    libraryList_ = libraryList;

    searchResultsCount_ = 0; 
    currentItem_ = 0;
    lastSelectedItem_ = nullptr;
    currentSelectedItem_ = nullptr;
    openedSessionPlayLists_.clear();
    openedSessionFiles_.clear();
}

QString PlaylistManager::LoadStyleSheet() 
{
    QString iconPath = ":/icons/icons/";

    QString icon_star_full = iconPath + "star-full.png";
    QString icon_star_empty = iconPath + "star-empty.png";
    QString icon_collapsed = iconPath + "collapsed.png";
    QString icon_expanded = iconPath + "expanded.png";

    QString stylesheet = QString(R"(
    QTreeWidget
    {
    background-color: rgb(15, 15, 15);
    background: black;
    color: white;
    border-right-color: rgb(30, 30, 30);
    }

    QTreeView::branch:open
    {
    image: url("%1");
    }
    QTreeView::branch:closed:has-children
    {
    image: url("%2");
    }

    QTreeWidget::item
    {
    height: 50px;
    }

    QTreeWidget::indicator
    {
    width: 16px;
    height: 16px;
    }

    QTreeWidgetItem::indicator:enabled
    {
    padding-right: 10px;
    }

    QTreeWidget::indicator:checked
    {
    image: url("%3");
    }

    QTreeWidget::indicator:unchecked
    {
    image: url("%4");
    }

    QTreeView::item:selected
    {
    background-color: rgb(30, 30, 30);
    border: 1px solid black;
    border-left-color: transparent;
    border-right-color: transparent;
    color: white;
    }

    QTreeView::item:hover
    {
    background-color: rgb(35, 11, 63);
    border: 1px solid rgb(82, 26, 149);
    border-left-color: transparent;
    border-right-color: transparent;
    color: white;
    }

    QScrollBar:vertical 
    {
        border: 1px solid black;
        background: black;
        width: 15px;
        margin: 22px 0 22px 0;
    }
    QScrollBar::handle:vertical 
    {
        border: 1px solid rgb(30, 30, 30);
        background: black;
        min-height: 40px;
        border-radius: 4px; 
    }
    QScrollBar::add-line:vertical 
    {
        border: 1px solid  rgb(30, 30, 30);
        background:  transparent;
        height: 15px;
        border-radius: 4px; 
        subcontrol-position: bottom;
        subcontrol-origin: margin;
    }
    QScrollBar::sub-line:vertical 
    {
        border: 1px solid  rgb(30, 30, 30);
        background:  transparent;
        height: 15px;
        border-radius: 4px; 
        subcontrol-position: top;
        subcontrol-origin: margin;
    }  
    background-color: rgb(15, 15, 15);
    border: 1px solid transparent;
    color: white;
    )").arg(icon_expanded, icon_collapsed, icon_star_full, icon_star_empty);

    return stylesheet;
}

void PlaylistManager::EmitTreeLayoutChanged() 
{
    emit playlistTree_->model()->layoutChanged();
}

void PlaylistManager::AppendPlayList(TreeItem* newPlayList, TreeItem* targetItem) 
{
    if (!newPlayList->IsPlayList()) 
    {
        return;
    }

    if (targetItem == nullptr) 
    {
        playlistTree_->insertTopLevelItem(playlistTree_->topLevelItemCount(), newPlayList);
    } 
    else 
    {
        targetItem->insertChild(targetItem->childCount(), newPlayList);
    }
}

void PlaylistManager::AppendChannel(TreeItem* playList, TreeItem* newChannel) 
{
    if (!newChannel->IsPlayList()) 
    {
        playList->insertChild(playList->childCount(), newChannel);
    }
}

void PlaylistManager::ClearPlayListItems(TreeItem* playList) 
{
    if (playList == nullptr) return;

    if (playList->IsPlayList())
    {
        QTreeWidgetItem* playlistItem = dynamic_cast<QTreeWidgetItem*>(playList);

        while (playlistItem->childCount() > 0)
        {
            QTreeWidgetItem* child = playlistItem->child(0);
            playlistItem->removeChild(child);
        }
    }
}

void PlaylistManager::UpdatePlayListChannelCount(TreeItem* item, int count) 
{
    if (count == -1) 
    {
        count = item->childCount();
    }

    if (item->IsPlayList()) 
    {
        item->setText(0, PAD(item->GetItemName()) + QString("  (%1)").arg(count));
    }
}

void PlaylistManager::CollapseAllPlaylists() 
{
    playlistTree_->collapseAll();
}

void PlaylistManager::ExpandAllPlaylists() 
{
    playlistTree_->expandAll();
}

void PlaylistManager::ItemClicked(QTreeWidgetItem* item)
{
    TreeItem* treeItem = dynamic_cast<TreeItem*>(item);

    if (treeItem == nullptr) return;

    if (treeItem->IsPlayList()) 
    {
        playlistTree_->setFocus();
        treeItem->setExpanded(not treeItem->isExpanded());
    }
}

void PlaylistManager::ItemDoubleClicked(QTreeWidgetItem* item) 
{
    TreeItem* treeItem = dynamic_cast<TreeItem*>(item);

    if (treeItem == nullptr) return;

    // TO DO

}

void PlaylistManager::AddRemoveFavorites(QTreeWidgetItem* item) 
{
    TreeItem* treeItem = dynamic_cast<TreeItem*>(item);

    if (treeItem == nullptr) return;

    // TO DO
}

void PlaylistManager::LoadPlayList(PlayListEntry playlist, bool isPersistent)
{
    string playlistPath = playlist.source;
    string playlistName = playlist.name;
    
    PRINT << "Loading playlist: " << playlistName;
    PRINT << "Playlist source: " << playlistPath;

    M3UParser m3uParser;

    // Check if the file exists
    if (!filesystem::exists(playlistPath)) 
    {
        PRINT << "File does not exist: " << playlistPath;
        return;
    }

    // Parse the M3U file
    vector<Channel> channelList = m3uParser.ParseM3UFile(playlistPath);

    // Check if channelList is empty
    if (channelList.empty()) 
    {
        PRINT << "Channel list is empty for playlist: " << playlistName;
        return;
    }

    // Block signals since we are modifying the tree
    playlistTree_->blockSignals(true); 

    // Create a new playlist parent item
    TreeItem* playlistTreeItem;

    if (isPersistent) 
    {
        playlistTreeItem = new TreeItem(PAD(QSTR(playlistName)), PLAYLIST_COLOR, true);
        playlistTree_->addTopLevelItem(playlistTreeItem);
    }
    else
    {
        ;  // Handle temporay playlist
    }

    // Iterate through the channelList and create tree items
    for (const Channel& channel : channelList) 
    {
        QString channelName = QString::fromStdString(channel.name);
        QString source = QString::fromStdString(channel.url);

        if(!channelName.isEmpty() && !source.isEmpty())
        {
            TreeItem* channelItem = new TreeItem(PAD(channelName), QColor(), false, isPersistent);
            channelItem->SetPlayListName(QSTR(playlistName));
            channelItem->SetSource(source);

            if (isPersistent) 
            {
                channelItem->setFlags(channelItem->flags() | Qt::ItemFlag::ItemIsUserCheckable);
                channelItem->SetItemChecked(false);
            }

            AppendChannel(playlistTreeItem, channelItem);
        }   
    }

    UpdatePlayListChannelCount(playlistTreeItem);
    playlistTree_->blockSignals(false);

}

void PlaylistManager::LoadLibrary() 
{
    ClearPlayListItems(libraryList_);

    for(const auto& item : appData_->Library)
    {
        TreeItem* newEntry = new TreeItem(PAD(QSTR(item->name)), QColor(), false);
        newEntry->SetPlayListName("Library");
        newEntry->SetSource(QSTR(item->source));

        newEntry->setFlags(newEntry->flags() | Qt::ItemFlag::ItemIsUserCheckable); 
        newEntry->SetItemChecked(false);

        // Add the item to the library list
        AppendChannel(libraryList_, newEntry);
    }

    UpdatePlayListChannelCount(libraryList_);
}

void PlaylistManager::LoadFavorites() 
{
    // TO DO
}   

