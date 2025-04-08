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


PlayListEntry* PlayListEntry::ValidateAndCreate(const QMap<QString, QString>& data) 
{
    // Ensure required fields are present
    if (!data.contains("name") || 
        !data.contains("sourceType") || 
        !data.contains("source")) 
    {
        return nullptr;
    }

    PlayListEntry* entry = new PlayListEntry();
    entry->name = data["name"].toStdString();
    
    // parentName is optional, use empty string if not provided
    entry->parentName = data.contains("parentName") ? data["parentName"].toStdString() : "";
    
    // Convert sourceType string to enum
    entry->sourceType = StringToSourceTypeEnum(data["sourceType"].toStdString());
    entry->source = data["source"].toStdString();
    
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
    if (!checkFile.exists() || !checkFile.isFile()) {
        PRINT << "File doesn't exist, creating with defaults";
        CreateDefaultSettings();
        Save(); // Save the default settings to create the file
        PRINT << "Default settings saved";
    } else {
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
    
    // HotKeys_ already has default values from its constructor
    
    // Initialize empty lists
    Library_.clear();
    Favorites_.clear();
    PlayLists_.clear();
}

void AppData::Load(const string& filePath) 
{
    try {
        // Clear existing data
        for (auto& entry : Library_) delete entry;
        for (auto& entry : Favorites_) delete entry;
        for (auto& entry : PlayLists_) delete entry;
        
        Library_.clear();
        Favorites_.clear();
        PlayLists_.clear();
        
        // Open the file
        QFile file(QString::fromStdString(filePath));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            PRINT << "Error opening file:" << QString::fromStdString(filePath);
            CreateDefaultSettings(); // Fall back to defaults
            return;
        }

        // Read the JSON file
        QByteArray jsonData = file.readAll();
        file.close();
        
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull() || !doc.isObject()) {
            PRINT << "Invalid JSON format";
            CreateDefaultSettings();
            return;
        }
        
        QJsonObject root = doc.object();

        // Parse basic settings
        PlayerType_ = StringToPlayerTypeEnum(root["PlayerType"].toString().toStdString());
        PlayListsPath_ = root["PlayListPath"].toString().toStdString();

        // Parse hotkeys
        QMap<QString, int> hotkeysMap;
        QJsonObject hotkeysJson = root["HotKeys"].toObject();
        for (auto it = hotkeysJson.begin(); it != hotkeysJson.end(); ++it) {
            if (it.value().isDouble()) {
                hotkeysMap[it.key()] = it.value().toInt();
            } else {
                PRINT << "Warning: Hotkey" << it.key() << "is not an integer value";
            }
        }
        HotKeys_ = AppHotKeys::ValidateAndCreate(hotkeysMap);

        // Parse Library (if exists)
        QJsonArray libraryJson = root["Library"].toArray();
        for (int i = 0; i < libraryJson.size(); ++i) {
            QJsonObject entryJson = libraryJson[i].toObject();
            QMap<QString, QString> entryMap;
            
            entryMap["name"] = entryJson["name"].toString();
            if (entryJson.contains("parentName")) {
                entryMap["parentName"] = entryJson["parentName"].toString();
            }
            entryMap["sourceType"] = entryJson["sourceType"].toString();
            entryMap["source"] = entryJson["source"].toString();
            
            PlayListEntry* entry = PlayListEntry::ValidateAndCreate(entryMap);
            if (entry) {
                Library_.push_back(entry);
            }
        }

        // Parse Favorites
        QJsonArray favoritesJson = root["Favorites"].toArray();
        for (int i = 0; i < favoritesJson.size(); ++i) {
            QJsonObject entryJson = favoritesJson[i].toObject();
            QMap<QString, QString> entryMap;
            
            entryMap["name"] = entryJson["name"].toString();
            if (entryJson.contains("parentName")) {
                entryMap["parentName"] = entryJson["parentName"].toString();
            }
            entryMap["sourceType"] = entryJson["sourceType"].toString();
            entryMap["source"] = entryJson["source"].toString();
            
            PlayListEntry* entry = PlayListEntry::ValidateAndCreate(entryMap);
            if (entry) {
                Favorites_.push_back(entry);
            }
        }

        // Parse PlayLists
        QJsonArray playlistsJson = root["PlayLists"].toArray();
        for (int i = 0; i < playlistsJson.size(); ++i) {
            QJsonObject entryJson = playlistsJson[i].toObject();
            QMap<QString, QString> entryMap;
            
            entryMap["name"] = entryJson["name"].toString();
            if (entryJson.contains("parentName")) {
                entryMap["parentName"] = entryJson["parentName"].toString();
            }
            entryMap["sourceType"] = entryJson["sourceType"].toString();
            entryMap["source"] = entryJson["source"].toString();
            
            PlayListEntry* entry = PlayListEntry::ValidateAndCreate(entryMap);
            if (entry) {
                PlayLists_.push_back(entry);
            }
        }
    }
    catch (const std::exception& e) {
        PRINT << "Exception during Load:" << e.what();
        CreateDefaultSettings(); // Fall back to defaults
    }
    catch (...) {
        PRINT << "Unknown exception during Load";
        CreateDefaultSettings(); // Fall back to defaults
    }
}

void AppData::Save() 
{
    try {
        // Ensure directory exists
        QFileInfo fileInfo(QString::fromStdString(dataFilePath_));
        QDir dir = fileInfo.dir();
        if (!dir.exists()) {
            PRINT << "Creating directory:" << dir.path();
            dir.mkpath(".");
        }
        
        // Create JSON root
        QJsonObject root;
        
        // Add basic settings
        root["PlayerType"] = QString::fromStdString(PlayerTypeToString(PlayerType_));
        root["PlayListPath"] = QString::fromStdString(PlayListsPath_);
        
        // Add hotkeys - use getAllHotkeys() for consistency
        QJsonObject hotkeysJson;
        QMap<QString, int> hotkeysMap = HotKeys_.GetAllHotkeys();
        for (auto it = hotkeysMap.begin(); it != hotkeysMap.end(); ++it) {
            hotkeysJson[it.key()] = it.value();
        }
        root["HotKeys"] = hotkeysJson;
        
        // Add Library
        QJsonArray libraryJson;
        for (const auto& entry : Library_) {
            QJsonObject entryJson;
            entryJson["name"] = QString::fromStdString(entry->name);
            entryJson["parentName"] = QString::fromStdString(entry->parentName);
            entryJson["sourceType"] = QString::fromStdString(SourceTypeToString(entry->sourceType));
            entryJson["source"] = QString::fromStdString(entry->source);
            libraryJson.append(entryJson);
        }
        root["Library"] = libraryJson;
        
        // Add Favorites
        QJsonArray favoritesJson;
        for (const auto& entry : Favorites_) {
            QJsonObject entryJson;
            entryJson["name"] = QString::fromStdString(entry->name);
            entryJson["parentName"] = QString::fromStdString(entry->parentName);
            entryJson["sourceType"] = QString::fromStdString(SourceTypeToString(entry->sourceType));
            entryJson["source"] = QString::fromStdString(entry->source);
            favoritesJson.append(entryJson);
        }
        root["Favorites"] = favoritesJson;
        
        // Add PlayLists
        QJsonArray playlistsJson;
        for (const auto& entry : PlayLists_) {
            QJsonObject entryJson;
            entryJson["name"] = QString::fromStdString(entry->name);
            entryJson["parentName"] = QString::fromStdString(entry->parentName);
            entryJson["sourceType"] = QString::fromStdString(SourceTypeToString(entry->sourceType));
            entryJson["source"] = QString::fromStdString(entry->source);
            playlistsJson.append(entryJson);
        }
        root["PlayLists"] = playlistsJson;
        
        // Write to file
        QFile file(QString::fromStdString(dataFilePath_));
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            PRINT << "Error opening file for writing:" << QString::fromStdString(dataFilePath_);
            return;
        }
        
        QJsonDocument doc(root);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
    catch (const std::exception& e) {
        PRINT << "Exception during Save:" << e.what();
    }
    catch (...) {
        PRINT << "Unknown exception during Save";
    }
}

void SavePlayListToFile(const QVector<PlayListEntry*>& playlist, const string& filepath) 
{
    QFile file(QString::fromStdString(filepath));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        PRINT << "Error opening playlist file for writing:" << QString::fromStdString(filepath);
        return;
    }
    
    QTextStream out(&file);
    out << "#EXTM3U\n";
    for (const auto& entry : playlist) {
        out << "#EXTINF:-1," << QString::fromStdString(entry->name) << "\n";
        if (entry->sourceType == ENUM_SOURCE_TYPE::LOCAL_FILE) {
            QString normalizedPath = QString::fromStdString(entry->source);
            normalizedPath.replace('\\', '/');  
            out << normalizedPath << "\n";
        } else {
            out << QString::fromStdString(entry->source) << "\n";
        }
    }
    file.close();
}
