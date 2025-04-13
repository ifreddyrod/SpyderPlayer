#include "Global.h"

QString Format_ms_to_Time(qint64 ms)
{   
    // Convert milliseconds to seconds
    int seconds = ms / 1000;

    // Calculate hours, minutes, and remaining seconds
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    seconds = seconds % 60;

    // Format as HH:MM:SS
    std::stringstream ss;
    ss << std::setw(2) << std::setfill('0') << hours << ":"
        << std::setw(2) << std::setfill('0') << minutes << ":"
        << std::setw(2) << std::setfill('0') << seconds;
        
    return QSTR(ss.str());
}

int ShowSaveWarningDialog(QString title, QString message, bool showCancel)
{
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setIcon(QMessageBox::Question);
    if (showCancel)
    {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setEscapeButton(QMessageBox::Cancel);
    }
    else
    {
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
    }
    
    msgBox.setWindowTitle(title);

    return msgBox.exec();
}

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

ENUM_PLAYER_TYPE StringToPlayerTypeEnum(const string& playerTypeStr) 
{
    static const std::unordered_map<std::string, ENUM_PLAYER_TYPE> playerTypeMap = 
    {
        {"VLC", VLC},
        {"QTMEDIA", QTMEDIA},
        {"FFMPEG", FFMPEG}
    };

    std::string upperCaseStr = playerTypeStr;
    std::transform(upperCaseStr.begin(), upperCaseStr.end(), upperCaseStr.begin(), ::toupper);

    auto it = playerTypeMap.find(upperCaseStr);
    if (it != playerTypeMap.end()) 
    {
        return it->second;
    } else 
    {
        return QTMEDIA; // Default or error value
    }
}

string PlayerTypeToString(ENUM_PLAYER_TYPE playerType) 
{
    switch (playerType) 
    {
        case VLC:
            return "VLC";
        case QTMEDIA:
            return "QTMEDIA";
        case FFMPEG:
            return "FFMPEG";
        default:
            return "UNKNOWN";
    }
}

ENUM_SOURCE_TYPE StringToSourceTypeEnum(const string& sourceTypeStr) 
{
    static const std::unordered_map<std::string, ENUM_SOURCE_TYPE> sourceTypeMap = 
    {
        {"file", LOCAL_FILE},
        {"url", URL}
    };

    auto it = sourceTypeMap.find(sourceTypeStr);
    if (it != sourceTypeMap.end()) 
    {
        return it->second;
    } else 
    {
        return LOCAL_FILE; // Default or error value
    }
}

string SourceTypeToString(ENUM_SOURCE_TYPE sourceType) 
{
    switch (sourceType) 
    {
        case LOCAL_FILE:
            return "file";
        case URL:
            return "url";
        default:
            return "";
    }
}

string PlayerStateToString(ENUM_PLAYER_STATE state)
{
    switch (state) 
    {
        case ENUM_PLAYER_STATE::IDLE:
            return "IDLE";
        case ENUM_PLAYER_STATE::LOADING:
            return "LOADING";
        case ENUM_PLAYER_STATE::PLAYING:
            return "PLAYING";
        case ENUM_PLAYER_STATE::PAUSED:
            return "PAUSED";
        case ENUM_PLAYER_STATE::STOPPED:
            return "STOPPED";
        case ENUM_PLAYER_STATE::STALLED:
            return "STALLED";
        case ENUM_PLAYER_STATE::ENDED:
            return "ENDED";
        case ENUM_PLAYER_STATE::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    };
}