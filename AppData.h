#ifndef APPDATA_H
#define APPDATA_H

#include <QWidget>
#include <string>
#include <vector>
#include <map>
#include "Global.h"

//#include "VideoPlayer.h" // Assuming ENUM_PLAYER_TYPE is defined here

using namespace std;

class PlayListEntry 
{
public:
    string name;
    string parentName;
    ENUM_SOURCE_TYPE sourceType;
    string source;

    static PlayListEntry* validate_and_create(const map<string, string>& data);
};

class AppHotKeys 
{
public:
    // Application Hotkeys and default values
    int playpause = Qt::Key::Key_K;
    int playpauseAlt = Qt::Key::Key_Space;
    int toggleFullscreen = Qt::Key::Key_F;
    int escapeFullscreen = Qt::Key::Key_Escape;
    int togglePlaylist = Qt::Key::Key_L;
    int volumeMute = Qt::Key::Key_M;
    int volumeUp = Qt::Key::Key_Up;
    int volumeDown = Qt::Key::Key_Down;
    int seekForward = Qt::Key::Key_Right;
    int seekBackward = Qt::Key::Key_Left;
    int gotoTopofList = Qt::Key::Key_T;
    int gotoBottomofList = Qt::Key::Key_B;
    int collapseAllLists = Qt::Key::Key_C;
    int sortListAscending = Qt::Key::Key_A;
    int sortListDescending = Qt::Key::Key_D;
    int gotoLast = Qt::Key::Key_Backspace;
    int showOptions = Qt::Key::Key_O;
    int playNext  = Qt::Key::Key_Period;
    int playPrevious = Qt::Key::Key_Comma;
    int stopVideo = Qt::Key::Key_S;

    static AppHotKeys validate_and_create(const map<string, int>& data);
};

class AppData 
{
public:
    ENUM_PLAYER_TYPE PlayerType;
    string PlayListPath;
    AppHotKeys HotKeys;
    vector<PlayListEntry*> Library;
    vector<PlayListEntry*> Favorites;
    vector<PlayListEntry*> PlayLists;
    string dataFile;

    AppData(const string& dataFilePath);
    void load(const string& filePath);
    void save();
};

extern string GetPlatform();
extern string GetUserAppDataDirectory(string platform, string appName);
extern void SavePlayListToFile(const vector<PlayListEntry*>& playlist, const string& filepath);

#endif // APPDATA_H
