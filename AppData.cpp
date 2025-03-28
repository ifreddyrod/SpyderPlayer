#include "AppData.h"
#include <iostream>
#include <fstream>
#include <json/json.h>
#include <filesystem>
#include <algorithm>

using namespace std;

PlayListEntry* PlayListEntry::validate_and_create(const map<string, string>& data) 
{
    try
    {
        PlayListEntry* entry = new PlayListEntry();
        entry->name = data.at("name");
        entry->parentName = data.at("parentName");
        entry->sourceType = (data.at("sourceType") == "file") ? SourceType::FILE : SourceType::URL;
        entry->source = data.at("source");
        return entry;
    } 
    catch (...) 
    {
        return nullptr;
    }
}

AppHotKeys AppHotKeys::validate_and_create(const map<string, int>& data) 
{
    AppHotKeys hotkeys;
    for (const auto& [key, value] : data) 
    {
        if (key == "playpause") hotkeys.playpause = value;
        else if (key == "playpauseAlt") hotkeys.playpauseAlt = value;
        else if (key == "toggleFullscreen") hotkeys.toggleFullscreen = value;
        else if (key == "escapeFullscreen") hotkeys.escapeFullscreen = value;
        else if (key == "togglePlaylist") hotkeys.togglePlaylist = value;
        else if (key == "volumeMute") hotkeys.volumeMute = value;
        else if (key == "volumeUp") hotkeys.volumeUp = value;
        else if (key == "volumeDown") hotkeys.volumeDown = value;
        else if (key == "seekForward") hotkeys.seekForward = value;
        else if (key == "seekBackward") hotkeys.seekBackward = value;
        else if (key == "gotoTopofList") hotkeys.gotoTopofList = value;
        else if (key == "gotoBottomofList") hotkeys.gotoBottomofList = value;
        else if (key == "collapseAllLists") hotkeys.collapseAllLists = value;
        else if (key == "sortListAscending") hotkeys.sortListAscending = value;
        else if (key == "sortListDescending") hotkeys.sortListDescending = value;
        else if (key == "gotoLast") hotkeys.gotoLast = value;
        else if (key == "showOptions") hotkeys.showOptions = value;
        else if (key == "playNext") hotkeys.playNext = value;
        else if (key == "playPrevious") hotkeys.playPrevious = value;
        else if (key == "stopVideo") hotkeys.stopVideo = value; 
    }
    return hotkeys;
}

AppData::AppData(const string& dataFilePath) : dataFile(dataFilePath) 
{
    load(dataFilePath);
}

void AppData::load(const string& filePath) 
{
    if (!filesystem::exists(filePath)) 
    {
        save();
        return;
    }

    ifstream file(filePath);
    Json::Value jsonData;
    file >> jsonData;

    // Process JSON data to initialize AppData fields
    try 
    {
        PlayerType = static_cast<ENUM_PLAYER_TYPE>(jsonData["PlayerType"].asInt());
        PlayListPath = jsonData["PlayListPath"].asString();
        // Initialize HotKeys and other fields as needed
    } 
    catch (const exception& e) 
    {
        cerr << "Error loading data: " << e.what() << endl;
    }
}

void AppData::save() 
{
    Json::Value jsonData;
    jsonData["PlayerType"] = static_cast<int>(PlayerType);
    jsonData["PlayListPath"] = PlayListPath;
    // Add other fields to jsonData

    ofstream file(dataFile);
    file << jsonData;
}

void SavePlayListToFile(const vector<PlayListEntry*>& playlist, const string& filepath) 
{
    ofstream file(filepath);
    file << "#EXTM3U\n";
    for (const auto& entry : playlist) 
    {
        file << "#EXTINF:-1," << entry->name << "\n";
        if (entry->sourceType == SourceType::FILE) 
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