#ifndef GLOBAL_H
#define GLOBAL_H

#include <string>
#include <QString>
#include <QDebug>
#include <map>

using namespace std;

// Global Variables
#define APP_VERSION "1.0.0 Beta"

// Convert std::string to QString
#define QSTR(s) QString::fromStdString(s)

// Convert QString to std::string
#define STR(s) (s).toStdString()

// Add padding to QString
#define PAD(str) (QString("  ") + (str))


// Shorter Debug Log Macro
#define PRINT qDebug()


/******************************************************************************************* 
    ENUMERATIONS
 *******************************************************************************************/
enum ENUM_PLAYER_TYPE 
{   VLC, 
    QTMEDIA, 
    UNKNOWN
};

extern ENUM_PLAYER_TYPE StringToPlayerTypeEnum(const string& playerTypeStr);
extern string PlayerTypeToString(ENUM_PLAYER_TYPE playerType);


enum ENUM_SOURCE_TYPE 
{   LOCAL_FILE, 
    URL 
};

extern ENUM_SOURCE_TYPE StringToSourceTypeEnum(const string& sourceTypeStr);
extern string SourceTypeToString(ENUM_SOURCE_TYPE sourceType);

#endif // GLOBAL_H