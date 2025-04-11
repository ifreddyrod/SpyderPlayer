#include "AppData.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QTextStream>
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <iostream>
#include <QtCore/QStandardPaths>
#include <QStandardPaths>


PlayListEntry PlayListEntry::ValidateAndCreate(const QMap<QString, QString>& data) 
{
    PlayListEntry entry;
    
    // Check for required fields and use them if present
    if (data.contains("name")) 
        entry.name = data["name"].toStdString();

    
    // parentName is optional, use empty string if not provided
    entry.parentName = data.contains("parentName") ? data["parentName"].toStdString() : "";
    
    // Convert sourceType string to enum if present
    if (data.contains("sourceType")) 
        entry.sourceType = StringToSourceTypeEnum(data["sourceType"].toStdString());
    
    
    if (data.contains("source")) 
        entry.source = data["source"].toStdString();
    
    return entry;
}

// New method to get all hotkeys as a QMap
QMap<QString, int> AppHotKeys::GetAllHotkeys() const 
{
    QMap<QString, int> hotkeys;
    hotkeys["playpause"] = playpause;
    hotkeys["playpauseAlt"] = playpauseAlt;
    hotkeys["toggleFullscreen"] = toggleFullscreen;
    hotkeys["escapeFullscreen"] = escapeFullscreen;
    hotkeys["togglePlaylist"] = togglePlaylist;
    hotkeys["volumeMute"] = volumeMute;
    hotkeys["volumeUp"] = volumeUp;
    hotkeys["volumeDown"] = volumeDown;
    hotkeys["seekForward"] = seekForward;
    hotkeys["seekBackward"] = seekBackward;
    hotkeys["gotoTopofList"] = gotoTopofList;
    hotkeys["gotoBottomofList"] = gotoBottomofList;
    hotkeys["collapseAllLists"] = collapseAllLists;
    hotkeys["sortListAscending"] = sortListAscending;
    hotkeys["sortListDescending"] = sortListDescending;
    hotkeys["gotoLast"] = gotoLast;
    hotkeys["showOptions"] = showOptions;
    hotkeys["playNext"] = playNext;
    hotkeys["playPrevious"] = playPrevious;
    hotkeys["stopVideo"] = stopVideo;
    return hotkeys;
}

// New method to update hotkeys from a QMap
void AppHotKeys::UpdateFromMap(const QMap<QString, int>& hotkeys) 
{
    if (hotkeys.contains("playpause")) playpause = hotkeys["playpause"];
    if (hotkeys.contains("playpauseAlt")) playpauseAlt = hotkeys["playpauseAlt"];
    if (hotkeys.contains("toggleFullscreen")) toggleFullscreen = hotkeys["toggleFullscreen"];
    if (hotkeys.contains("escapeFullscreen")) escapeFullscreen = hotkeys["escapeFullscreen"];
    if (hotkeys.contains("togglePlaylist")) togglePlaylist = hotkeys["togglePlaylist"];
    if (hotkeys.contains("volumeMute")) volumeMute = hotkeys["volumeMute"];
    if (hotkeys.contains("volumeUp")) volumeUp = hotkeys["volumeUp"];
    if (hotkeys.contains("volumeDown")) volumeDown = hotkeys["volumeDown"];
    if (hotkeys.contains("seekForward")) seekForward = hotkeys["seekForward"];
    if (hotkeys.contains("seekBackward")) seekBackward = hotkeys["seekBackward"];
    if (hotkeys.contains("gotoTopofList")) gotoTopofList = hotkeys["gotoTopofList"];
    if (hotkeys.contains("gotoBottomofList")) gotoBottomofList = hotkeys["gotoBottomofList"];
    if (hotkeys.contains("collapseAllLists")) collapseAllLists = hotkeys["collapseAllLists"];
    if (hotkeys.contains("sortListAscending")) sortListAscending = hotkeys["sortListAscending"];
    if (hotkeys.contains("sortListDescending")) sortListDescending = hotkeys["sortListDescending"];
    if (hotkeys.contains("gotoLast")) gotoLast = hotkeys["gotoLast"];
    if (hotkeys.contains("showOptions")) showOptions = hotkeys["showOptions"];
    if (hotkeys.contains("playNext")) playNext = hotkeys["playNext"];
    if (hotkeys.contains("playPrevious")) playPrevious = hotkeys["playPrevious"];
    if (hotkeys.contains("stopVideo")) stopVideo = hotkeys["stopVideo"];
}

AppHotKeys AppHotKeys::ValidateAndCreate(const QMap<QString, int>& data) 
{
    AppHotKeys hotkeys;
    
    // Use the new updateFromMap method for consistency
    hotkeys.UpdateFromMap(data);
    return hotkeys;
}

AppData::AppData(const string& filePath) 
{
    dataFilePath_ = filePath;
    
    PRINT << "Initializing AppData with path:" << QString::fromStdString(filePath);
    
    // Check if file exists
    QFileInfo checkFile(QString::fromStdString(filePath));
    if (!checkFile.exists() || !checkFile.isFile()) 
    {
        PRINT << "File doesn't exist, creating with defaults";
        CreateDefaultSettings();
        Save(); // Save the default settings to create the file
        PRINT << "Default settings saved";
    } 
    else 
    {
        PRINT << "File exists, loading settings";
        Load(dataFilePath_);
        PRINT << "Settings loaded successfully";
    }
}

void AppData::CreateDefaultSettings() 
{
    // Set default values
    PlayerType_ = ENUM_PLAYER_TYPE::QTMEDIA; 
    PlayListsPath_ = ""; // Set default path as needed
    RetryCount_ = 3;
    RetryDelay_ = 1000;
    
    // HotKeys_ already has default values from its constructor
    
    // Initialize empty lists
    Library_.clear();
    Favorites_.clear();
    PlayLists_.clear();
}

void AppData::Load(const string& filePath) 
{
    bool needsSaving = false;  // Flag to determine if we need to save after loading
    
    try {
        // Clear existing data
        Library_.clear();
        Favorites_.clear();
        PlayLists_.clear();
        
        // Open the file
        QFile file(QString::fromStdString(filePath));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) 
        {
            PRINT << "Error opening file:" << QString::fromStdString(filePath);
            CreateDefaultSettings(); // Fall back to defaults
            needsSaving = true;
            return;
        }

        // Read the JSON file
        QByteArray jsonData = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) 
        {
            PRINT << "Invalid JSON format";
            CreateDefaultSettings();
            needsSaving = true;
            return;
        }
        
        QJsonObject root = doc.object();

        // Parse basic settings with default fallbacks
        if (root.contains("PlayerType")) 
        {
            PlayerType_ = StringToPlayerTypeEnum(root["PlayerType"].toString().toStdString());
        } 
        else 
        {
            PlayerType_ = ENUM_PLAYER_TYPE::QTMEDIA;
            root["PlayerType"] = QString::fromStdString(PlayerTypeToString(PlayerType_));
            needsSaving = true;
            PRINT << "Missing PlayerType, using default";
        }
        
        if (root.contains("PlayListPath")) 
        {
            PlayListsPath_ = root["PlayListPath"].toString().toStdString();
        } 
        else 
        {
            PlayListsPath_ = "";
            root["PlayListPath"] = QString::fromStdString(PlayListsPath_);
            needsSaving = true;
            PRINT << "Missing PlayListPath, using default";
        }
        
        if (root.contains("RetryCount")) 
        {
            RetryCount_ = root["RetryCount"].toInt();
        } 
        else 
        {
            RetryCount_ = 3;
            root["RetryCount"] = RetryCount_;
            needsSaving = true;
            PRINT << "Missing RetryCount, using default";
        }
        
        if (root.contains("RetryDelay")) 
        {
            RetryDelay_ = root["RetryDelay"].toInt();
        } 
        else 
        {
            RetryDelay_ = 1000;
            root["RetryDelay"] = RetryDelay_;
            needsSaving = true;
            PRINT << "Missing RetryDelay, using default";
        }
        
        PRINT << "Retry Count: " << RetryCount_;
        PRINT << "Retry Delay: " << RetryDelay_;
        
        // Parse hotkeys
        QMap<QString, int> hotkeysMap;
        
        // Create a default hotkeys instance to get default values
        AppHotKeys defaultHotKeys;
        QMap<QString, int> defaultHotkeysMap = defaultHotKeys.GetAllHotkeys();
        
        if (root.contains("HotKeys") && root["HotKeys"].isObject()) 
        {
            QJsonObject hotkeysJson = root["HotKeys"].toObject();
            
            // Merge with defaults - for each expected hotkey
            for (auto it = defaultHotkeysMap.begin(); it != defaultHotkeysMap.end(); ++it) 
            {
                if (hotkeysJson.contains(it.key()) && hotkeysJson[it.key()].isDouble()) 
                {
                    hotkeysMap[it.key()] = hotkeysJson[it.key()].toInt();
                } 
                else 
                {
                    // Use default and mark for saving
                    hotkeysMap[it.key()] = it.value();
                    hotkeysJson[it.key()] = it.value();
                    needsSaving = true;
                    PRINT << "Missing or invalid hotkey:" << it.key() << ", using default";
                }
            }
            
            // Update the root object
            root["HotKeys"] = hotkeysJson;
        } 
        else 
        {
            // No hotkeys section at all
            hotkeysMap = defaultHotkeysMap;
            QJsonObject hotkeysJson;
            for (auto it = hotkeysMap.begin(); it != hotkeysMap.end(); ++it) {
                hotkeysJson[it.key()] = it.value();
            }
            root["HotKeys"] = hotkeysJson;
            needsSaving = true;
            PRINT << "Missing HotKeys section, using defaults";
        }
        
        HotKeys_ = AppHotKeys::ValidateAndCreate(hotkeysMap);

        // Initialize empty arrays if missing
        if (!root.contains("Library") || !root["Library"].isArray()) 
        {
            root["Library"] = QJsonArray();
            needsSaving = true;
            PRINT << "Missing Library section, creating empty array";
        }
        
        if (!root.contains("Favorites") || !root["Favorites"].isArray()) 
        {
            root["Favorites"] = QJsonArray();
            needsSaving = true;
            PRINT << "Missing Favorites section, creating empty array";
        }
        
        if (!root.contains("PlayLists") || !root["PlayLists"].isArray()) 
        {
            root["PlayLists"] = QJsonArray();
            needsSaving = true;
            PRINT << "Missing PlayLists section, creating empty array";
        }

        // Parse Library
        QJsonArray libraryJson = root["Library"].toArray();
        for (int i = 0; i < libraryJson.size(); ++i) 
        {
            QJsonObject entryJson = libraryJson[i].toObject();
            QMap<QString, QString> entryMap;
            
            // Skip entries missing required fields
            if (!entryJson.contains("name") || !entryJson.contains("sourceType") || !entryJson.contains("source")) 
            {
                PRINT << "Skipping library entry with missing required fields";
                continue;
            }
            
            entryMap["name"] = entryJson["name"].toString();
            if (entryJson.contains("parentName")) 
            {
                entryMap["parentName"] = entryJson["parentName"].toString();
            }
            entryMap["sourceType"] = entryJson["sourceType"].toString();
            entryMap["source"] = entryJson["source"].toString();
            
            PlayListEntry entry = PlayListEntry::ValidateAndCreate(entryMap);
            Library_.push_back(entry);
        }

        // Parse Favorites
        QJsonArray favoritesJson = root["Favorites"].toArray();
        for (int i = 0; i < favoritesJson.size(); ++i) 
        {
            QJsonObject entryJson = favoritesJson[i].toObject();
            QMap<QString, QString> entryMap;
            
            // Skip entries missing required fields
            if (!entryJson.contains("name") || !entryJson.contains("sourceType") || !entryJson.contains("source")) 
            {
                PRINT << "Skipping favorites entry with missing required fields";
                continue;
            }
            
            entryMap["name"] = entryJson["name"].toString();
            if (entryJson.contains("parentName")) 
            {
                entryMap["parentName"] = entryJson["parentName"].toString();
            }
            entryMap["sourceType"] = entryJson["sourceType"].toString();
            entryMap["source"] = entryJson["source"].toString();
            
            PlayListEntry entry = PlayListEntry::ValidateAndCreate(entryMap);
            Favorites_.push_back(entry);
        }

        // Parse PlayLists
        QJsonArray playlistsJson = root["PlayLists"].toArray();
        for (int i = 0; i < playlistsJson.size(); ++i) 
        {
            QJsonObject entryJson = playlistsJson[i].toObject();
            QMap<QString, QString> entryMap;
            
            // Skip entries missing required fields
            if (!entryJson.contains("name") || !entryJson.contains("sourceType") || !entryJson.contains("source")) 
            {
                PRINT << "Skipping playlist entry with missing required fields";
                continue;
            }
            
            entryMap["name"] = entryJson["name"].toString();
            if (entryJson.contains("parentName")) 
            {
                entryMap["parentName"] = entryJson["parentName"].toString();
            }
            entryMap["sourceType"] = entryJson["sourceType"].toString();
            entryMap["source"] = entryJson["source"].toString();
            
            PlayListEntry entry = PlayListEntry::ValidateAndCreate(entryMap);
            PlayLists_.push_back(entry);
        }
        
        // If any defaults were applied, save the updated JSON
        if (needsSaving) 
        {
            QFile saveFile(QString::fromStdString(filePath));
            if (saveFile.open(QIODevice::WriteOnly | QIODevice::Text)) 
            {
                QJsonDocument updatedDoc(root);
                saveFile.write(updatedDoc.toJson(QJsonDocument::Indented));
                saveFile.close();
                PRINT << "Saved updated configuration with default values";
            } 
            else 
            {
                PRINT << "Failed to save updated configuration with defaults";
            }
        }
    }
    catch (const std::exception& e) 
    {
        PRINT << "Exception during Load:" << e.what();
        CreateDefaultSettings(); // Fall back to defaults
        needsSaving = true;
    }
    catch (...) 
    {
        PRINT << "Unknown exception during Load";
        CreateDefaultSettings(); // Fall back to defaults
        needsSaving = true;
    }
    
    // Save if we created default settings
    if (needsSaving) 
    {
        Save();
    }
}

void AppData::Save() 
{
    try {
        // Ensure directory exists
        QFileInfo fileInfo(QString::fromStdString(dataFilePath_));
        QDir dir = fileInfo.dir();
        if (!dir.exists()) 
        {
            PRINT << "Creating directory:" << dir.path();
            dir.mkpath(".");
        }
        
        // Create JSON root
        QJsonObject root;
        
        // Add basic settings
        root["PlayerType"] = QString::fromStdString(PlayerTypeToString(PlayerType_));
        root["PlayListPath"] = QString::fromStdString(PlayListsPath_);
        root["RetryCount"] = RetryCount_;
        root["RetryDelay"] = RetryDelay_;
        
        // Add hotkeys - use getAllHotkeys() for consistency
        QJsonObject hotkeysJson;
        QMap<QString, int> hotkeysMap = HotKeys_.GetAllHotkeys();
        for (auto it = hotkeysMap.begin(); it != hotkeysMap.end(); ++it) 
        {
            hotkeysJson[it.key()] = it.value();
        }
        root["HotKeys"] = hotkeysJson;
        
        // Add Library
        QJsonArray libraryJson;
        for (const auto& entry : Library_) 
        {
            QJsonObject entryJson;
            entryJson["name"] = QString::fromStdString(entry.name);
            entryJson["parentName"] = QString::fromStdString(entry.parentName);
            entryJson["sourceType"] = QString::fromStdString(SourceTypeToString(entry.sourceType));
            entryJson["source"] = QString::fromStdString(entry.source);
            libraryJson.append(entryJson);
        }
        root["Library"] = libraryJson;
        
        // Add Favorites
        QJsonArray favoritesJson;
        for (const auto& entry : Favorites_)
        {
            QJsonObject entryJson;
            entryJson["name"] = QString::fromStdString(entry.name);
            entryJson["parentName"] = QString::fromStdString(entry.parentName);
            entryJson["sourceType"] = QString::fromStdString(SourceTypeToString(entry.sourceType));
            entryJson["source"] = QString::fromStdString(entry.source);
            favoritesJson.append(entryJson);
        }
        root["Favorites"] = favoritesJson;
        
        // Add PlayLists
        QJsonArray playlistsJson;
        for (const auto& entry : PlayLists_) 
        {
            QJsonObject entryJson;
            entryJson["name"] = QString::fromStdString(entry.name);
            entryJson["parentName"] = QString::fromStdString(entry.parentName);
            entryJson["sourceType"] = QString::fromStdString(SourceTypeToString(entry.sourceType));
            entryJson["source"] = QString::fromStdString(entry.source);
            playlistsJson.append(entryJson);
        }
        root["PlayLists"] = playlistsJson;
        
        // Write to file
        QFile file(QString::fromStdString(dataFilePath_));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            PRINT << "Error opening file for writing:" << QString::fromStdString(dataFilePath_);
            return;
        }
        
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
    catch (const std::exception& e) 
    {
        PRINT << "Exception during Save:" << e.what();
    }
    catch (...)
    {
        PRINT << "Unknown exception during Save";
    }
}

void SavePlayListToFile(const QVector<PlayListEntry> playlist, const string& filepath) 
{
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        PRINT << "Error opening playlist file for writing:" << QString::fromStdString(filepath);
        return;
    }
    
    QTextStream out(&file);
    out << "#EXTM3U\n";
    for (const auto& entry : playlist) 
    {
        out << "#EXTINF:-1," << QString::fromStdString(entry.name) << "\n";
        if (entry.sourceType == ENUM_SOURCE_TYPE::LOCAL_FILE) 
        {
            QString normalizedPath = QString::fromStdString(entry.source);
            normalizedPath.replace('\\', '/');  
            out << normalizedPath << "\n";
        } 
        else 
        {
            out << QString::fromStdString(entry.source) << "\n";
        }
    }
    file.close();
}