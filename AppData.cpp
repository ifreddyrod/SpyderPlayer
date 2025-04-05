#include "AppData.h"
#include <json/json.h>
#include <fstream>
#include <iostream>

PlayListEntry* PlayListEntry::validate_and_create(const map<string, string>& data) 
{
    // Ensure required fields are present
    if (data.find("name") == data.end() || 
        data.find("sourceType") == data.end() || 
        data.find("source") == data.end()) 
    {
        return nullptr;
    }

    PlayListEntry* entry = new PlayListEntry();
    entry->name = data.at("name");
    
    // parentName is optional, use empty string if not provided
    entry->parentName = (data.find("parentName") != data.end()) ? data.at("parentName") : "";
    
    // Convert sourceType string to enum
    entry->sourceType = StringToSourceTypeEnum(data.at("sourceType"));
    entry->source = data.at("source");
    
    return entry;
}

AppHotKeys AppHotKeys::validate_and_create(const map<string, int>& data) 
{
    AppHotKeys hotkeys;
    
    // Only update fields that are present in the data
    if (data.find("playpause") != data.end()) hotkeys.playpause = data.at("playpause");
    if (data.find("playpauseAlt") != data.end()) hotkeys.playpauseAlt = data.at("playpauseAlt");
    if (data.find("toggleFullscreen") != data.end()) hotkeys.toggleFullscreen = data.at("toggleFullscreen");
    if (data.find("escapeFullscreen") != data.end()) hotkeys.escapeFullscreen = data.at("escapeFullscreen");
    if (data.find("togglePlaylist") != data.end()) hotkeys.togglePlaylist = data.at("togglePlaylist");
    if (data.find("volumeMute") != data.end()) hotkeys.volumeMute = data.at("volumeMute");
    if (data.find("volumeUp") != data.end()) hotkeys.volumeUp = data.at("volumeUp");
    if (data.find("volumeDown") != data.end()) hotkeys.volumeDown = data.at("volumeDown");
    if (data.find("seekForward") != data.end()) hotkeys.seekForward = data.at("seekForward");
    if (data.find("seekBackward") != data.end()) hotkeys.seekBackward = data.at("seekBackward");
    if (data.find("gotoTopofList") != data.end()) hotkeys.gotoTopofList = data.at("gotoTopofList");
    if (data.find("gotoBottomofList") != data.end()) hotkeys.gotoBottomofList = data.at("gotoBottomofList");
    if (data.find("collapseAllLists") != data.end()) hotkeys.collapseAllLists = data.at("collapseAllLists");
    if (data.find("sortListAscending") != data.end()) hotkeys.sortListAscending = data.at("sortListAscending");
    if (data.find("sortListDescending") != data.end()) hotkeys.sortListDescending = data.at("sortListDescending");
    if (data.find("gotoLast") != data.end()) hotkeys.gotoLast = data.at("gotoLast");
    if (data.find("showOptions") != data.end()) hotkeys.showOptions = data.at("showOptions");
    if (data.find("playNext") != data.end()) hotkeys.playNext = data.at("playNext");
    if (data.find("playPrevious") != data.end()) hotkeys.playPrevious = data.at("playPrevious");
    if (data.find("stopVideo") != data.end()) hotkeys.stopVideo = data.at("stopVideo");
    
    return hotkeys;
}

AppData::AppData(const string& filePath)
{
    dataFilePath_ = filePath;

    // Check if file exists
    std::ifstream checkFile(filePath);
    if (!checkFile.good())
    {
        // File doesn't exist or can't be opened, create with defaults
        CreateDefaultSettings();
        Save(); // Save the default settings to create the file
        Load(dataFilePath_);
    }
    else
    {
        checkFile.close();
        Load(dataFilePath_);
    }
}

void AppData::CreateDefaultSettings()
{
    // Set default values
    PlayerType_ = ENUM_PLAYER_TYPE::QTMEDIA; // Use appropriate default
    PlayListsPath_ = ""; // Set default path as needed

    // HotKeys_ already has default values from its constructor

    // Initialize empty lists
    Library_.clear();
    Favorites_.clear();
    PlayLists_.clear();
}

void AppData::Load(const string& filePath) 
{
    // Clear existing data
    for (auto& entry : Library_) delete entry;
    for (auto& entry : Favorites_) delete entry;
    for (auto& entry : PlayLists_) delete entry;
    
    Library_.clear();
    Favorites_.clear();
    PlayLists_.clear();
    
    // Open the file
    std::ifstream file(filePath);
    if (!file.is_open()) 
    {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    // Parse Full JSON
    Json::Value root;
    Json::CharReaderBuilder builder;
    JSONCPP_STRING errs;
    if (!Json::parseFromStream(builder, file, &root, &errs)) 
    {
        std::cerr << "Error parsing JSON: " << errs << std::endl;
        return;
    }

    // Parse basic settings
    PlayerType_ = StringToPlayerTypeEnum(root["PlayerType"].asString());
    PlayListsPath_ = root["PlayListPath"].asString();

    // Parse hotkeys
    map<string, int> hotkeysMap;
    const Json::Value& hotkeysJson = root["HotKeys"];
    for (const auto& memberName : hotkeysJson.getMemberNames())
    {
        // Check if the value is an integer before conversion
        if (hotkeysJson[memberName].isInt()) {
            hotkeysMap[memberName] = hotkeysJson[memberName].asInt();
        } else {
            // Skip or provide a default value
            std::cerr << "Warning: Hotkey '" << memberName << "' is not an integer value" << std::endl;
            // Optionally assign a default value
            // hotkeysMap[memberName] = defaultValue;
        }
    }
    HotKeys_ = AppHotKeys::validate_and_create(hotkeysMap);

    // Parse Library (if exists)
    const Json::Value& libraryJson = root["Library"];
    for (unsigned int i = 0; i < libraryJson.size(); i++) 
    {
        const Json::Value& entryJson = libraryJson[i];
        map<string, string> entryMap;
        
        entryMap["name"] = entryJson["name"].asString();
        if (entryJson.isMember("parentName")) 
        {
            entryMap["parentName"] = entryJson["parentName"].asString();
        }
        entryMap["sourceType"] = entryJson["sourceType"].asString();
        entryMap["source"] = entryJson["source"].asString();
        
        PlayListEntry* entry = PlayListEntry::validate_and_create(entryMap);
        if (entry) 
        {
            Library_.push_back(entry);
        }
    }

    // Parse Favorites
    const Json::Value& favoritesJson = root["Favorites"];
    for (unsigned int i = 0; i < favoritesJson.size(); i++) 
    {
        const Json::Value& entryJson = favoritesJson[i];
        map<string, string> entryMap;
        
        entryMap["name"] = entryJson["name"].asString();
        if (entryJson.isMember("parentName")) 
        {
            entryMap["parentName"] = entryJson["parentName"].asString();
        }
        entryMap["sourceType"] = entryJson["sourceType"].asString();
        entryMap["source"] = entryJson["source"].asString();
        
        PlayListEntry* entry = PlayListEntry::validate_and_create(entryMap);
        if (entry) 
        {
            Favorites_.push_back(entry);
        }
    }

    // Parse PlayLists
    const Json::Value& playlistsJson = root["PlayLists"];
    for (unsigned int i = 0; i < playlistsJson.size(); i++) 
    {
        const Json::Value& entryJson = playlistsJson[i];
        map<string, string> entryMap;
        
        entryMap["name"] = entryJson["name"].asString();
        if (entryJson.isMember("parentName")) 
        {
            entryMap["parentName"] = entryJson["parentName"].asString();
        }
        entryMap["sourceType"] = entryJson["sourceType"].asString();
        entryMap["source"] = entryJson["source"].asString();
        
        PlayListEntry* entry = PlayListEntry::validate_and_create(entryMap);
        if (entry) 
        {
            PlayLists_.push_back(entry);
        }
    }
}

void AppData::Save() 
{
    // Create JSON root
    Json::Value root;
    
    // Add basic settings
    root["PlayerType"] = PlayerTypeToString(PlayerType_);
    root["PlayListPath"] = PlayListsPath_;
    
    // Add hotkeys
    Json::Value hotkeysJson;
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
    Json::Value libraryJson(Json::arrayValue);
    for (const auto& entry : Library_) 
    {
        Json::Value entryJson;
        entryJson["name"] = entry->name;
        entryJson["parentName"] = entry->parentName;
        entryJson["sourceType"] = SourceTypeToString(entry->sourceType);
        entryJson["source"] = entry->source;
        libraryJson.append(entryJson);
    }
    root["Library"] = libraryJson;
    
    // Add Favorites
    Json::Value favoritesJson(Json::arrayValue);
    for (const auto& entry : Favorites_) 
    {
        Json::Value entryJson;
        entryJson["name"] = entry->name;
        entryJson["parentName"] = entry->parentName;
        entryJson["sourceType"] = SourceTypeToString(entry->sourceType);
        entryJson["source"] = entry->source;
        favoritesJson.append(entryJson);
    }
    root["Favorites"] = favoritesJson;
    
    // Add PlayLists
    Json::Value playlistsJson(Json::arrayValue);
    for (const auto& entry : PlayLists_) 
    {
        Json::Value entryJson;
        entryJson["name"] = entry->name;
        entryJson["parentName"] = entry->parentName;
        entryJson["sourceType"] = SourceTypeToString(entry->sourceType);
        entryJson["source"] = entry->source;
        playlistsJson.append(entryJson);
    }
    root["PlayLists"] = playlistsJson;
    
    // Write to file
    std::ofstream file(dataFilePath_);
    if (!file.is_open()) 
    {
        std::cerr << "Error opening file for writing: " << dataFilePath_ << std::endl;
        return;
    }
    
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder["indentation"] = "    "; // Pretty formatting with 4 spaces
    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(root, &file);
    file.close();
}

void SavePlayListToFile(const vector<PlayListEntry*>& playlist, const string& filepath) 
{
    ofstream file(filepath);
    file << "#EXTM3U\n";
    for (const auto& entry : playlist) 
    {
        file << "#EXTINF:-1," << entry->name << "\n";
        if (entry->sourceType == ENUM_SOURCE_TYPE::LOCAL_FILE) 
        {
            string normalizedPath = entry->source;
            replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
            file << normalizedPath << "\n";
        } else 
        {
            file << entry->source << "\n";
        }
    }
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
string GetUserAppDataDirectory(string platform, string appName)
{
    string directory = "";

    if (platform.empty())
    {
        platform = GetPlatform();
    }

    if (platform == "Windows")
    {
        directory = filesystem::path(getenv("APPDATA")) / appName;
    }
    else if (platform == "Linux")
    {
        directory = filesystem::path(getenv("HOME")) / ".config" / appName; 
    }
    else if (platform == "Darwin")
    {
        directory = filesystem::path(getenv("HOME")) / "Library" / "Application Support" / appName;
    }
    else
    {
        directory = "";
    }

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
