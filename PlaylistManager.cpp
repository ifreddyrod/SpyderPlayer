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

TreeItem* TreeItem::Child(int index) 
{
    return dynamic_cast<TreeItem*>(QTreeWidgetItem::child(index));
}

TreeItem* TreeItem::Parent() 
{
    return dynamic_cast<TreeItem*>(QTreeWidgetItem::parent());
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
    PRINT << "ClearPlayListItems";

    if (playList == nullptr) 
    {
        PRINT << "PlayList is null";
        return;
    }

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

void PlaylistManager::SortPlaylistAscending()
{
    TreeItem* selectedItem = static_cast<TreeItem*>(playlistTree_->currentItem());

    if (selectedItem == nullptr) return;
    
    if (selectedItem->IsPlayList())
    {
        selectedItem->sortChildren(0, Qt::SortOrder::AscendingOrder);
    }
    else
    {
        selectedItem->parent()->sortChildren(0, Qt::SortOrder::AscendingOrder);
    }
}

void PlaylistManager::SortPlaylistDescending()
{
    TreeItem* selectedItem = static_cast<TreeItem*>(playlistTree_->currentItem());

    if (selectedItem == nullptr) return;

    if (selectedItem->IsPlayList())
    {
        selectedItem->sortChildren(0, Qt::SortOrder::DescendingOrder);
    }
    else
    {
        selectedItem->parent()->sortChildren(0, Qt::SortOrder::DescendingOrder);
    }
}

void PlaylistManager::GotoTopOfList()
{
    TreeItem* selectedItem = static_cast<TreeItem*>(playlistTree_->currentItem());

    if (selectedItem == nullptr) return;

    if (!selectedItem->IsPlayList())
    {
        // Go to the top of the playlist
        playlistTree_->setCurrentItem(selectedItem->parent()->child(0));
    }
}

void PlaylistManager::GotoBottomOfList()
{
    TreeItem* selectedItem = static_cast<TreeItem*>(playlistTree_->currentItem());

    if (selectedItem == nullptr) return;

    if (!selectedItem->IsPlayList())
    {
        // Go to the bottom of the playlist
        playlistTree_->setCurrentItem(selectedItem->parent()->child(selectedItem->parent()->childCount() - 1));
    }
}

void PlaylistManager::ItemClicked(QTreeWidgetItem* item)
{
    if (item == nullptr) return;
    
    TreeItem* treeItem = dynamic_cast<TreeItem*>(item);

    if (treeItem->IsPlayList()) 
    {
        playlistTree_->setFocus();
        treeItem->setExpanded(not treeItem->isExpanded());
    }
}

void PlaylistManager::ItemDoubleClicked(QTreeWidgetItem* item) 
{
    if (item == nullptr) return;

    TreeItem* treeItem = dynamic_cast<TreeItem*>(item);

    if (treeItem->IsPlayList()) return;

    //TreeItem* temp = currentSelectedItem_;

    QString channelName = treeItem->GetItemName();
    QString channelSource = treeItem->GetSource();

    // Check if item is in searchlist

    lastSelectedItem_ = currentSelectedItem_;
    currentSelectedItem_ = treeItem;

    playlistTree_->setFocus();

    emit SIGNAL_PlaySelectedChannel(channelName.toStdString(), channelSource.toStdString());

}
TreeItem* PlaylistManager::GetChannelFromTree(QString playListName, QString channelName, QString source)
{
    // Index through the Playlists of the tree
    for (int row = 0; row < playlistTree_->topLevelItemCount(); row++) 
    {
        QTreeWidgetItem* item = playlistTree_->topLevelItem(row);
        QString playlistTreeText = item->text(0).split("  ")[1];

        // Exclude the Search List and Favorites List
        if (playlistTreeText == searchList_->GetItemName()) continue; // or item.GetItemName() == self.favoritesList.GetItemName():
        
        // If Playlist is found, then index through the channels of the playlist
        PRINT << "Looking for playlist: " << playlistTreeText;
        if (playlistTreeText == playListName) 
        {
            for (int childRow = 0; childRow < item->childCount(); childRow++) 
            {
                TreeItem* child = dynamic_cast<TreeItem*>(item->child(childRow));
                if (child->GetItemName() == channelName && child->GetSource() == source) 
                {
                    return child;
                }
            }
        }
    }
    return nullptr;
}

TreeItem* PlaylistManager::GetPlayListFromTree(QString playListName) 
{
    for (int row = 0; row < playlistTree_->topLevelItemCount(); row++) 
    {
        QTreeWidgetItem* item = playlistTree_->topLevelItem(row);
        if (item->text(0).split("  ")[1] == playListName) 
        {
            return dynamic_cast<TreeItem*>(item);
        }
    }
    return nullptr;
}

TreeItem* PlaylistManager::GetPlayListFromSearch(QString playListName) 
{
    if (searchList_->childCount() == 0) 
    {
        return nullptr;
    }
    
    for (int row = 0; row < searchList_->childCount(); row++) 
    {
        QTreeWidgetItem* item = searchList_->Child(row);
        if (item->text(0).split("  ")[1] == playListName) 
        {
            return dynamic_cast<TreeItem*>(item);
        }
    }
    return nullptr;
}

void PlaylistManager::ToggleItemCheckedinList(TreeItem* playList, TreeItem* item, bool checked)
{
    if (item == nullptr || playList == nullptr || !playList->IsPlayList()) return;

    QString channelName = item->GetItemName();
    QString channelSource = item->GetSource();
    
    // << "Toggling channel: " << channelName << " in playlist: " << playList->GetItemName();

    // Search Playlist for corresponding channel
    for (int i = 0; i < playList->childCount(); i++) 
    {
        TreeItem* child = playList->Child(i);
        if (child->GetItemName() == channelName && child->GetSource() == channelSource) 
        {
            // If channel is found, uncheck it
            child->SetItemChecked(checked);
            break;  
        }
    }
}

void PlaylistManager::AddRemoveFavorites(QTreeWidgetItem* item) // Triggered when toggling checkbox in the tree
{
    if (item == nullptr) return;

    TreeItem* treeItem = dynamic_cast<TreeItem*>(item);

    QString channelName = treeItem->GetItemName();
    QString channelSource = treeItem->GetSource();
    QString channelPlaylist = treeItem->GetPlayListName();

    //---------------------------------------------------
    // Tree Item was Selected to be added to favorites
    //---------------------------------------------------
    if (treeItem->IsItemChecked())  
    {
        //PRINT << "ADDING ChannelName: " << channelName << " Source: " << channelSource << " Playlist: " << channelPlaylist;

        // Check if item is already in favorites list
        for (int i = 0; i < favoritesList_->childCount(); i++) 
        {
            TreeItem* favItem = favoritesList_->Child(i);
            if (favItem->GetItemName() == channelName && favItem->GetSource() == channelSource) 
            {
                PRINT << "Item already in favorites list";
                return;
            }
        }

        // item is not in favorites list, then proceed to add it
        playlistTree_->blockSignals(true);  
        TreeItem* newFav = new TreeItem(channelName);
        newFav->SetPlayListName(channelPlaylist);
        newFav->SetSource(channelSource);
        newFav->setFlags(newFav->flags() | Qt::ItemFlag::ItemIsUserCheckable);
        newFav->SetItemChecked(true);

        AppendChannel(favoritesList_, newFav);
        UpdatePlayListChannelCount(favoritesList_);

        // Update Search List and Play List
        TreeItem* treeplayList = GetPlayListFromTree(channelPlaylist);
        TreeItem* searchList = GetPlayListFromSearch(channelPlaylist);
        ToggleItemCheckedinList(treeplayList, newFav, true);
        ToggleItemCheckedinList(searchList, newFav, true);

        // Add new channel to appdata
        PlayListEntry* entry = new PlayListEntry();
        entry->name = STR(channelName);
        entry->parentName = STR(channelPlaylist);
        entry->sourceType = channelSource.indexOf("http") == 0 ? ENUM_SOURCE_TYPE::URL : ENUM_SOURCE_TYPE::LOCAL_FILE;
        entry->source = STR(channelSource);

        appData_->Favorites_.push_back(entry);
        SaveFavorites();

        playlistTree_->blockSignals(false);
        EmitTreeLayoutChanged();
    }
    //-------------------------------------------------------
    // Tree Item was Selected to be removed from favorites
    //-------------------------------------------------------
    else
    {
        //PRINT << "REMOVE ChannelName: " << channelName << " Source: " << channelSource << " Playlist: " << channelPlaylist;

        // Check if Favorites list is empty
        if (favoritesList_->childCount() == 0) 
        {
            PRINT << "Favorites list is empty";
            return;
        }

        // item is in favorites list, then proceed to remove it
        playlistTree_->blockSignals(true);  

        // Find Favorite Item in AppData and remove it
        for (int i = appData_->Favorites_.size() - 1; i >= 0; i--)
        {
            PlayListEntry* favItem = appData_->Favorites_[i];
            if (favItem->name == STR(channelName) && favItem->parentName == STR(channelPlaylist) && favItem->source == STR(channelSource)) 
            {
                appData_->Favorites_.erase(appData_->Favorites_.begin() + i);
                SaveFavorites();
                LoadFavorites();

                // Get Index of item
                if (channelPlaylist == "Favorites")
                {
                    //int index = favoritesList_->indexOfChild(treeItem);
                    favoritesList_->removeChild(item);
                }

                // Deselect item from corresponding playlist
                TreeItem* treeplayList = GetPlayListFromTree(channelPlaylist);
                TreeItem* searchList = GetPlayListFromSearch(channelPlaylist);
                ToggleItemCheckedinList(treeplayList, treeItem, false);
                ToggleItemCheckedinList(searchList, treeItem, false);

                if (searchResultsCount_ > 0)
                {
                    SearchChannels(lastSearchQuery_);
                }
                break;
            }
        }

        playlistTree_->blockSignals(false);
        EmitTreeLayoutChanged();

    }
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
        QString source = QString::fromStdString(channel.url).trimmed();

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
    playlistTree_->blockSignals(true);
    ClearPlayListItems(libraryList_);

    for(const auto& item : appData_->Library_)
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
    playlistTree_->blockSignals(false);
}

void PlaylistManager::SaveFavorites() 
{
    // Save to appdata file
    appData_->Save();
}

void PlaylistManager::LoadFavorites() 
{
    // Block signals
    playlistTree_->blockSignals(true);  
    ClearPlayListItems(favoritesList_);

    // Find Corresponding Playlist Items
    for(const auto& item : appData_->Favorites_)
    {
        // Find the item.playListName in the tree
        TreeItem* favoriteItem = GetChannelFromTree(QSTR(item->parentName), QSTR(item->name), QSTR(item->source));  
        //PRINT << "Looking for favorite: " << item->name << " from " << item->parentName << " playlist" << " from " << item->source;

        // If item is found, check it
        if (favoriteItem) 
        {
            favoriteItem->SetItemChecked(true);
        }
        else
        {
            continue;
        }

        //PRINT << "Loaded favorite: " << item->name << " from " << item->parentName << " playlist" << " from " << item->source;

        // Add the item to the favorites list
        TreeItem* newEntry = new TreeItem(favoriteItem->GetItemName());
        newEntry->SetPlayListName(favoriteItem->GetPlayListName());  
        newEntry->SetSource(favoriteItem->GetSource());

        newEntry->setFlags(newEntry->flags() | Qt::ItemFlag::ItemIsUserCheckable); 
        newEntry->SetItemChecked(true);

        // Add the item to the favorites list
        AppendChannel(favoritesList_, newEntry);
    }

    UpdatePlayListChannelCount(favoritesList_);
    playlistTree_->blockSignals(false);
}

void PlaylistManager::SearchChannels(QString searchText)
{
    //PRINT << "SearchChannels: " << searchText;

    // Return if search text is empty
    if (searchText.isEmpty()) return;

    // Set the last search query
    lastSearchQuery_ = searchText;

    // Convert search text to lowercase and remove spaces
    searchText = searchText.toLower();
    searchText = searchText.replace(" ", "");
    
    // Convert search text to array
    QStringList searchItems = searchText.split('+');

    //PRINT << "Search Items: " << searchItems;

    // Block signals since we are modifying the tree
    playlistTree_->blockSignals(true);

    // Clear the search list 
    ClearPlayListItems(searchList_);

    int searchResultsCount = 0;
    // Traverse (playlists) but ignore the Search Playlist
    for (int row = 1; row < playlistTree_->topLevelItemCount(); row++) 
    {
        // Get the playlist item from tree
        TreeItem* playlistToSearch = static_cast<TreeItem*>(playlistTree_->topLevelItem(row));

        // Create new playlist to show results
        TreeItem* playlistResults = new TreeItem(playlistToSearch->GetItemName(), QColor(), true);

        // Treaverse the Playlist and search for channels
        for (int i = 0; i < playlistToSearch->childCount(); i++) 
        {
            TreeItem* channel = playlistToSearch->Child(i);

            // Get the channel name
            QString channelName = channel->GetItemName().toLower(); 

            // Search the channel name for seach query
            int searchIndex = -1;
            for (const QString& searchItem : searchItems) 
            {
                searchIndex = channelName.indexOf(searchItem);
                if (searchIndex < 0) 
                {
                    break;
                }
            }

            // If query is found, add channel to search list
            if (searchIndex >= 0) 
            {
                TreeItem* newEntry = new TreeItem(channel->GetItemName());
                newEntry->SetPlayListName(playlistResults->GetItemName());
                newEntry->SetSource(channel->GetSource());

                if (channel->IsItemPersistent())
                {
                    newEntry->setFlags(newEntry->flags() | Qt::ItemFlag::ItemIsUserCheckable); 
                    newEntry->SetItemChecked(false);    
                }
                AppendChannel(playlistResults, newEntry);
                searchResultsCount++;
                
            }
        }

        UpdatePlayListChannelCount(playlistResults);
        if (playlistResults->childCount() > 0)
        {
            AppendPlayList(playlistResults, searchList_);
        }
    }

    UpdatePlayListChannelCount(searchList_, searchResultsCount);
    playlistTree_->blockSignals(false);
    playlistTree_->setCurrentItem(searchList_);
    searchList_->setExpanded(true);

    // Set the search results count
    searchResultsCount_ = searchResultsCount;

    // Emit the tree layout changed signal
    EmitTreeLayoutChanged();
}


