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


PlayListEntry* PlayListEntry::validate_and_create(const QMap<QString, QString>& data) 
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

AppHotKeys AppHotKeys::validate_and_create(const QMap<QString, int>& data) 
{
    AppHotKeys hotkeys;
    
    // Only update fields that are present in the data
    if (data.contains("playpause")) hotkeys.playpause = data["playpause"];
    if (data.contains("playpauseAlt")) hotkeys.playpauseAlt = data["playpauseAlt"];
    if (data.contains("toggleFullscreen")) hotkeys.toggleFullscreen = data["toggleFullscreen"];
    if (data.contains("escapeFullscreen")) hotkeys.escapeFullscreen = data["escapeFullscreen"];
    if (data.contains("togglePlaylist")) hotkeys.togglePlaylist = data["togglePlaylist"];
    if (data.contains("volumeMute")) hotkeys.volumeMute = data["volumeMute"];
    if (data.contains("volumeUp")) hotkeys.volumeUp = data["volumeUp"];
    if (data.contains("volumeDown")) hotkeys.volumeDown = data["volumeDown"];
    if (data.contains("seekForward")) hotkeys.seekForward = data["seekForward"];
    if (data.contains("seekBackward")) hotkeys.seekBackward = data["seekBackward"];
    if (data.contains("gotoTopofList")) hotkeys.gotoTopofList = data["gotoTopofList"];
    if (data.contains("gotoBottomofList")) hotkeys.gotoBottomofList = data["gotoBottomofList"];
    if (data.contains("collapseAllLists")) hotkeys.collapseAllLists = data["collapseAllLists"];
    if (data.contains("sortListAscending")) hotkeys.sortListAscending = data["sortListAscending"];
    if (data.contains("sortListDescending")) hotkeys.sortListDescending = data["sortListDescending"];
    if (data.contains("gotoLast")) hotkeys.gotoLast = data["gotoLast"];
    if (data.contains("showOptions")) hotkeys.showOptions = data["showOptions"];
    if (data.contains("playNext")) hotkeys.playNext = data["playNext"];
    if (data.contains("playPrevious")) hotkeys.playPrevious = data["playPrevious"];
    if (data.contains("stopVideo")) hotkeys.stopVideo = data["stopVideo"];
    
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
        HotKeys_ = AppHotKeys::validate_and_create(hotkeysMap);

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
            
            PlayListEntry* entry = PlayListEntry::validate_and_create(entryMap);
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
            
            PlayListEntry* entry = PlayListEntry::validate_and_create(entryMap);
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
            
            PlayListEntry* entry = PlayListEntry::validate_and_create(entryMap);
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
        
        // Add hotkeys
        QJsonObject hotkeysJson;
        hotkeysJson["playpause"] = HotKeys_.playpause;
        hotkeysJson["playpauseAlt"] = HotKeys_.playpauseAlt;
        hotkeysJson["toggleFullscreen"] = HotKeys_.toggleFullscreen;
        hotkeysJson["escapeFullscreen"] = HotKeys_.escapeFullscreen;
        hotkeysJson["togglePlaylist"] = HotKeys_.togglePlaylist;
        hotkeysJson["volumeMute"] = HotKeys_.volumeMute;
        hotkeysJson["volumeUp"] = HotKeys_.volumeUp;
        hotkeysJson["volumeDown"] = HotKeys_.volumeDown;
        hotkeysJson["seekForward"] = HotKeys_.seekForward;
        hotkeysJson["seekBackward"] = HotKeys_.seekBackward;
        hotkeysJson["gotoTopofList"] = HotKeys_.gotoTopofList;
        hotkeysJson["gotoBottomofList"] = HotKeys_.gotoBottomofList;
        hotkeysJson["collapseAllLists"] = HotKeys_.collapseAllLists;
        hotkeysJson["sortListAscending"] = HotKeys_.sortListAscending;
        hotkeysJson["sortListDescending"] = HotKeys_.sortListDescending;
        hotkeysJson["gotoLast"] = HotKeys_.gotoLast;
        hotkeysJson["showOptions"] = HotKeys_.showOptions;
        hotkeysJson["playNext"] = HotKeys_.playNext;
        hotkeysJson["playPrevious"] = HotKeys_.playPrevious;
        hotkeysJson["stopVideo"] = HotKeys_.stopVideo;
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


/******************************************************************************************* 
    EXTERN Utility Functions 
 *******************************************************************************************/

/*===================================================================================
GetPlatform()

    Description: Gets the current platform (Windows, Linux, or macOS) 

    Parameters:
        None

    Returns: string representing the current platform
=====================================================================================*/
string GetPlatform()
{
#ifdef _WIN32
    return "Windows";

#elif __linux__
    return "Linux";

#elif __APPLE__
    return "Darwin";
#endif

    return "";
}

/*===================================================================================
GetUserAppDataDirectory()

    Description: Gets the user's app data directory based on the current platform.  
                 It will create the directory if it doesn't exist.

    Parameters:
        platform - string representing the platform (Windows, Linux, or macOS)
        appName - string representing the name of the application
    
    Returns: string representing the user's app data directory
=====================================================================================*/
string GetUserAppDataDirectory(string appName)
{
    string directory = "";

#ifdef _WIN32

    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +"/AppData/Roaming/" + QSTR(appName) ;
    directory = STR(appDataPath);

#elif __linux__

    directory = filesystem::path(getenv("HOME")) / ".config" / appName;
#elif __APPLE__

    directory = filesystem::path(getenv("HOME")) / "Library" / "Application Support" / appName;
#endif

    if (!directory.empty())
    {
        // Create the directory if it does not exist
        if (!filesystem::exists(directory)) 
        {
            filesystem::create_directories(directory);
        }
    }
    return directory;
}
