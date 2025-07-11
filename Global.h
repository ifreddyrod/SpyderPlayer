#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <sstream>
#include <iomanip>
#include <QString>
#include <QDebug>
#include <map>
#include <filesystem>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QMessageBox>
#include <QApplication>

using namespace std;

// Global Variables
#define APP_VERSION "1.0.0 Beta"

// Convert std::string to QString
#define QSTR(s) QString::fromStdString(s)

// Convert QString to std::string
#define STR(s) (s).toStdString()

// Add padding to QString
#define PAD(str) (QString("  ") + (str))

#define APPDATA_FILENAME "appdata.json"

// Shorter Debug Log Macro
#define PRINT qDebug()

/*******************************************************************************************
 * Utility Functions
 *******************************************************************************************/
extern QString Format_ms_to_Time(qint64 ms);
extern int ShowSaveWarningDialog(QString title, QString message, bool showCancel = true);
extern string GetPlatform();
extern string GetUserAppDataDirectory(string appName);
extern void SetCursorNormal();
extern void SetCursorBusy();
extern void SetCursorBlank();

/******************************************************************************************* 
    Player Type ENUMERATION
 *******************************************************************************************/
enum ENUM_PLAYER_TYPE 
{   VLC, 
    QTMEDIA, 
    FFMPEG
};

extern ENUM_PLAYER_TYPE StringToPlayerTypeEnum(const string& playerTypeStr);
extern string PlayerTypeToString(ENUM_PLAYER_TYPE playerType);


/******************************************************************************************* 
    Source Type ENUMERATION
 *******************************************************************************************/
enum ENUM_SOURCE_TYPE 
{   LOCAL_FILE, 
    URL 
};

extern ENUM_SOURCE_TYPE StringToSourceTypeEnum(const string& sourceTypeStr);
extern string SourceTypeToString(ENUM_SOURCE_TYPE sourceType);


/******************************************************************************************* 
    Player State ENUMERATION
 *******************************************************************************************/
enum class ENUM_PLAYER_STATE 
{
    IDLE,
    LOADING,
    PLAYING,
    PAUSED,
    STOPPED,
    STALLED,
    NOMEDIA,
    ENDED,
    ERROR,
    RECOVERING
};

extern string PlayerStateToString(ENUM_PLAYER_STATE state);


/******************************************************************************************* 
    Setting Views ENUMERATION
 *******************************************************************************************/
enum ENUM_SETTINGS_VIEWS
{
    MAIN = 0,
    PLAYLIST = 1,
    LIBRARY  = 2,
    FAVORITES = 3,
    PLAYLIST_ENTRY = 4,
    LIBRARY_ENTRY = 5,
    FAVORITES_ENTRY = 6,
    OPEN_PLAYLIST = 7,
    OPEN_FILE = 8,
    APPSETTINGS = 9,
    HOTKEYS = 10,
    ABOUT = 11
};

#endif // GLOBAL_H
