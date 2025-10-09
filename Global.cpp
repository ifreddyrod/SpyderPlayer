#include "Global.h"

/*******************************************************************************************
 * Global Variables Initialization
 *******************************************************************************************/
string G_AppDataDirectory = "";
bool G_LogToFile = false;
bool G_FFmpegLog = false;
libvlc_log_t *G_VLCctx = nullptr;
static QMutex G_LoggingMutex;


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

void SetCursorNormal()
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
}

void SetCursorBusy()
{
    QApplication::setOverrideCursor(Qt::BusyCursor);
}

void SetCursorBlank()
{
    QApplication::setOverrideCursor(Qt::BlankCursor);
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
        case ENUM_PLAYER_STATE::NOMEDIA:
            return "NOMEDIA";
        case ENUM_PLAYER_STATE::ENDED:
            return "ENDED";
        case ENUM_PLAYER_STATE::ERROR:
            return "ERROR";
        case ENUM_PLAYER_STATE::RECOVERING:
            return "RECOVERING";
        default:
            return "UNKNOWN";
    };
}


// Redirect qDebug messages to a log file
void RotateLogIfNeeded(const QString &basePath, qint64 maxSize, int maxBackups) 
{
    QFileInfo baseInfo(basePath);
    if (!baseInfo.exists() || baseInfo.size() < maxSize) 
        return;
    
    for (int i = maxBackups; i >= 2; --i) 
    {
        QString oldBackup = basePath + "." + QString::number(i);
        QString newBackup = basePath + "." + QString::number(i - 1);
        if (QFile::exists(oldBackup)) 
        {
            QFile::remove(newBackup);
            QFile::rename(oldBackup, newBackup);
        }
    }
    if (maxBackups >= 1) 
    {
        QFile::rename(basePath, basePath + ".1");
    }
}

void LogFileOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) 
{
    QMutexLocker locker(&G_LoggingMutex);

    // Format the message once for both outputs
    QString timestamp = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss.zzz]");

    // Output to terminal (console)
    QTextStream(stderr) << msg << Qt::endl;

    // Handle file logging
    if (G_LogToFile)
    {
        QString baseLogPath = QSTR(G_AppDataDirectory) + "player_messages.log";
        const qint64 maxSize = 5 * 1024 * 1024;  // 5 MB
        const int maxBackups = 5;
        QString formattedMsg;

        // Log VLC messages
        if (G_VLCctx) 
        {
            formattedMsg = QString("%1   [vlc] %2").arg(timestamp, msg);
            G_VLCctx = nullptr;
        }
        // Log FFmpeg messages from QtMultimedia
        else if (G_FFmpegLog)
        {
            formattedMsg = QString("%1   [qt]  %2").arg(timestamp, msg);
            G_FFmpegLog = false;
        }
        // App Messages
        else
            formattedMsg = QString("%1         %2").arg(timestamp, msg);

        RotateLogIfNeeded(baseLogPath, maxSize, maxBackups);

        QFile file(baseLogPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) 
        {
            QTextStream out(&file);
            out << formattedMsg << Qt::endl;
            out.flush();
            file.close();
        }
    }
    Q_UNUSED(type);
    Q_UNUSED(context);
}

// FFmpeg log callback
/*void FFmpegLogCallback(void *avcl, int level, const char *fmt, va_list vl) 
{
    // Respect AV_LOG_LEVEL (e.g., warning)
    if (level > av_log_get_level()) {
        return;
    }

    char msgBuffer[4096];
    vsnprintf(msgBuffer, sizeof(msgBuffer), fmt, vl);
    QString msg = QString::fromUtf8(msgBuffer).trimmed();  // Trim newlines
    G_FFmpegLog = true;  // Flag as FFmpeg message
    LogFileOutput(QtDebugMsg, QMessageLogContext(), msg);
}*/
