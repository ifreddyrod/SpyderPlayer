#include "Global.h"

ENUM_PLAYER_TYPE StringToPlayerTypeEnum(const string& playerTypeStr) 
{
    static const std::unordered_map<std::string, ENUM_PLAYER_TYPE> playerTypeMap = 
    {
        {"VLC", VLC},
        {"QtMedia", QTMEDIA}
    };

    auto it = playerTypeMap.find(playerTypeStr);
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
            return "QtMedia";
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