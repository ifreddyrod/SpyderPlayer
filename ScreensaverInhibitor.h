// ScreensaverInhibitor.h
#ifndef SCREENSAVER_INHIBITOR_H
#define SCREENSAVER_INHIBITOR_H

#include <memory>
#include <string>

// Abstract base class for platform-specific inhibitors
class PlatformInhibitor 
{
public:
    virtual ~PlatformInhibitor() = default;
    virtual void inhibit() = 0;
    virtual void uninhibit() = 0;
};

// Windows implementation
#ifdef _WIN32
class WindowsInhibitor : public PlatformInhibitor 
{
public:
    WindowsInhibitor();
    ~WindowsInhibitor() override;

    void inhibit() override;
    void uninhibit() override;

private:
    void* threadTimerHandle_ = nullptr;
    bool inhibited_ = false;
    
    static unsigned long __stdcall timerThreadProc(void* param);
};
#endif

// Linux implementation
#ifdef __linux__
class LinuxInhibitor : public PlatformInhibitor 
{
public:
    LinuxInhibitor();
    ~LinuxInhibitor() override;

    void inhibit() override;
    void uninhibit() override;

private:
    int screenSaverCookie_ = 0;
    int sleepInhibitPid_ = 0;
    void* dbusConnection_ = nullptr;
    void* dbusInterface_ = nullptr;
    bool initialized_ = false;
};
#endif

// macOS implementation
#ifdef __APPLE__
class MacOSInhibitor : public PlatformInhibitor 
{
public:
    MacOSInhibitor();
    ~MacOSInhibitor() override;

    void inhibit() override;
    void uninhibit() override;

private:
    int caffeinatePid_ = 0;
};
#endif

// Main class that detects the platform and uses the appropriate inhibitor
class ScreensaverInhibitor 
{
public:
    ScreensaverInhibitor();
    ~ScreensaverInhibitor();

    void inhibit();
    void uninhibit();

private:
    std::unique_ptr<PlatformInhibitor> inhibitor_;
};

#endif // SCREENSAVER_INHIBITOR_H