#ifndef APPDATA_H
#define APPDATA_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
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

    static PlayListEntry ValidateAndCreate(const QMap<QString, QString>& data);
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

    // New methods to get and modify all hotkeys
    QMap<QString, int> GetAllHotkeys() const;
    void UpdateFromMap(const QMap<QString, int>& hotkeys);

    static AppHotKeys ValidateAndCreate(const QMap<QString, int>& data);
};

class AppData 
{
public:
    double AppScaleFactor_;
    ENUM_PLAYER_TYPE PlayerType_;
    string PlayListsPath_;
    int RetryCount_;
    int RetryDelay_;
    AppHotKeys HotKeys_;
    QString VLCSetupArgs_;
    bool EnableFileLogging_;
    QVector<PlayListEntry> Library_;
    QVector<PlayListEntry> Favorites_;
    QVector<PlayListEntry> PlayLists_;
    string dataFilePath_;
    bool EnableHWAcceleration_;

    AppData(const string& filePath);
    void Load(const string& filePath);
    void Save();
    void CreateDefaultSettings();
};

extern void SavePlayListToFile(const QVector<PlayListEntry> playlist, const string& filepath);

#endif // APPDATA_H