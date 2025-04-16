// ScreensaverInhibitor.cpp
#include "ScreensaverInhibitor.h"
#include <iostream>
#include <cstdlib>

// Main ScreensaverInhibitor implementation
ScreensaverInhibitor::ScreensaverInhibitor() 
{
#ifdef _WIN32
    inhibitor_ = std::make_unique<WindowsInhibitor>();
    std::cout << "Using Windows screensaver inhibitor" << std::endl;
#elif defined(__linux__)
    inhibitor_ = std::make_unique<LinuxInhibitor>();
    std::cout << "Using Linux screensaver inhibitor" << std::endl;
#elif defined(__APPLE__)
    inhibitor_ = std::make_unique<MacOSInhibitor>();
    std::cout << "Using macOS screensaver inhibitor" << std::endl;
#else
    std::cerr << "Unsupported platform for screensaver inhibition" << std::endl;
#endif
}

ScreensaverInhibitor::~ScreensaverInhibitor() 
{
    uninhibit();
}

void ScreensaverInhibitor::inhibit() 
{
    if (inhibitor_) 
    {
        inhibitor_->inhibit();
    }
}

void ScreensaverInhibitor::uninhibit() 
{
    if (inhibitor_) 
    {
        inhibitor_->uninhibit();
    }
}

// Windows implementation
#ifdef _WIN32
#define _AVX512BF16VLINTRIN_H_INCLUDED
#define _AVX512BF16INTRIN_H_INCLUDED
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

WindowsInhibitor::WindowsInhibitor() 
    : threadTimerHandle_(nullptr), inhibited_(false)
{
}

WindowsInhibitor::~WindowsInhibitor() 
{
    uninhibit();
}

unsigned long __stdcall WindowsInhibitor::timerThreadProc(void* param) 
{
    WindowsInhibitor* inhibitor = static_cast<WindowsInhibitor*>(param);
    
    while (inhibitor->inhibited_) 
    {
        // ES_DISPLAY_REQUIRED prevents screen saver, ES_SYSTEM_REQUIRED prevents sleep
        SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
        Sleep(30000); // 30 seconds
    }
    
    return 0;
}

void WindowsInhibitor::inhibit() 
{
    if (inhibited_) 
    {
        return;
    }
    
    inhibited_ = true;
    
    // Create a thread to periodically reset the screensaver and sleep timers
    DWORD threadId;
    threadTimerHandle_ = CreateThread(
        NULL,                   // default security attributes
        0,                      // default stack size
        timerThreadProc,        // thread function
        this,                   // parameter to thread function
        0,                      // default creation flags
        &threadId               // returns the thread identifier
    );
    
    // Initial prevention
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
    
    std::cout << "Windows screensaver and sleep inhibited" << std::endl;
}

void WindowsInhibitor::uninhibit() 
{
    if (!inhibited_) 
    {
        return;
    }
    
    inhibited_ = false;
    
    // Wait for the thread to finish
    if (threadTimerHandle_) 
    {
        WaitForSingleObject(threadTimerHandle_, 1000);
        CloseHandle(threadTimerHandle_);
        threadTimerHandle_ = nullptr;
    }
    
    // Clear the execution state
    SetThreadExecutionState(ES_CONTINUOUS);
    
    std::cout << "Windows screensaver and sleep uninhibited" << std::endl;
}
#endif

// Linux implementation
#ifdef __linux__
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>

LinuxInhibitor::LinuxInhibitor() 
    : screenSaverCookie_(0), sleepInhibitPid_(0), 
      dbusConnection_(nullptr), dbusInterface_(nullptr), initialized_(false)
{
    // For a non-Qt implementation, you'd need to use libdbus directly
    // or call dbus-send via system commands
    
    // Kill any existing systemd-inhibit processes from previous runs
    system("pkill -f 'systemd-inhibit.*VideoPlayer'");
}

LinuxInhibitor::~LinuxInhibitor() 
{
    uninhibit();
}

void LinuxInhibitor::inhibit() 
{
    if (sleepInhibitPid_ != 0) 
    {
        // Already inhibited
        return;
    }
    
    // Use systemd-inhibit to prevent sleep via fork/exec
    pid_t pid = fork();
    
    if (pid == 0) 
    {
        // Child process
        execlp("systemd-inhibit", "systemd-inhibit", 
               "--what=idle", "--who=VideoPlayer", 
               "--why=Playing video", "sleep", "infinity", nullptr);
        // If we get here, exec failed
        exit(1);
    } 
    else if (pid > 0) 
    {
        // Parent process
        sleepInhibitPid_ = pid;
        std::cout << "Started systemd-inhibit process with PID " << pid << std::endl;
    } 
    else 
    {
        // Fork failed
        std::cerr << "Failed to fork systemd-inhibit process: " << strerror(errno) << std::endl;
    }
    
    // For screensaver inhibition using DBus, in a non-Qt implementation
    // you would either use direct DBus calls via libdbus or execute
    // dbus-send command via system()
    
    // Example using system command (not ideal but works without dependencies)
    system("dbus-send --session --dest=org.freedesktop.ScreenSaver "
           "--type=method_call --print-reply /ScreenSaver "
           "org.freedesktop.ScreenSaver.Inhibit string:VideoPlayer string:'Playing video'");
    
    std::cout << "Linux screensaver and sleep inhibited" << std::endl;
}

void LinuxInhibitor::uninhibit() 
{
    // Release screensaver inhibition via DBus
    system("dbus-send --session --dest=org.freedesktop.ScreenSaver "
           "--type=method_call --print-reply /ScreenSaver "
           "org.freedesktop.ScreenSaver.UnInhibit uint32:0");
    
    // Kill the systemd-inhibit process
    if (sleepInhibitPid_ > 0) 
    {
        kill(sleepInhibitPid_, SIGTERM);
        
        // Wait for process to terminate
        int status;
        waitpid(sleepInhibitPid_, &status, 0);
        
        sleepInhibitPid_ = 0;
        std::cout << "Stopped systemd-inhibit process" << std::endl;
    }
    
    std::cout << "Linux screensaver and sleep uninhibited" << std::endl;
}
#endif

// macOS implementation
#ifdef __APPLE__
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

MacOSInhibitor::MacOSInhibitor() 
    : caffeinatePid_(0)
{
    // Kill all caffeinate processes from previous runs
    system("killall caffeinate 2>/dev/null");
}

MacOSInhibitor::~MacOSInhibitor() 
{
    uninhibit();
}

void MacOSInhibitor::inhibit() 
{
    if (caffeinatePid_ != 0) 
    {
        // Already inhibited
        return;
    }
    
    // Use caffeinate to prevent system sleep and display sleep
    pid_t pid = fork();
    
    if (pid == 0) 
    {
        // Child process
        // -d: prevent display sleep
        // -i: prevent system idle sleep
        // -s: prevent system sleep (works when closing lid on laptops)
        execlp("caffeinate", "caffeinate", "-d", "-i", "-s", nullptr);
        // If we get here, exec failed
        exit(1);
    } 
    else if (pid > 0) 
    {
        // Parent process
        caffeinatePid_ = pid;
        std::cout << "Started caffeinate process with PID " << pid << std::endl;
    } 
    else 
    {
        // Fork failed
        std::cerr << "Failed to fork caffeinate process: " << strerror(errno) << std::endl;
    }
}

void MacOSInhibitor::uninhibit() 
{
    if (caffeinatePid_ > 0) 
    {
        // Send SIGTERM to the caffeinate process
        kill(caffeinatePid_, SIGTERM);
        
        // Wait for the process to terminate
        int status;
        waitpid(caffeinatePid_, &status, 0);
        
        caffeinatePid_ = 0;
        std::cout << "Stopped caffeinate process" << std::endl;
    }
}
#endif