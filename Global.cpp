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

ENUM_PLAYER_TYPE StringToPlayerTypeEnum(const string& playerTypeStr) 
{
    static const std::unordered_map<std::string, ENUM_PLAYER_TYPE> playerTypeMap = 
    {
        {"VLC", VLC},
        {"QTMEDIA", QTMEDIA}
    };

    std::string upperCaseStr = playerTypeStr;
    std::transform(upperCaseStr.begin(), upperCaseStr.end(), upperCaseStr.begin(), ::toupper);

    auto it = playerTypeMap.find(upperCaseStr);
    if (it != playerTypeMap.end()) 
    {
        return it->second;
    } else 
    {
        return UNKNOWN; // Default or error value
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