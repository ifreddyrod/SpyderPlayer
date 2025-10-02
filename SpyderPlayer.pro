TARGET = SpyderPlayer

QT       += core gui multimedia multimediawidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17 sdk_no_version_check

ICON += assets/icons/spider_dark_icon.icns

# macOS specific settings
macx {
    INCLUDEPATH += /Applications/VLC.app/Contents/MacOS/include
    LIBS += -L/Applications/VLC.app/Contents/MacOS/lib -lvlc 
    QMAKE_RPATHDIR += /Applications/VLC.app/Contents/MacOS/lib
}

# Linux specific settings
unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += libvlc
}

# Windows specific settings
win32 {
    INCLUDEPATH += /usr/local/include
    LIBS +=
}

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AboutScreen.cpp \
    AppData.cpp \
    AppSettingsScreen.cpp \
    DraggableWidget.cpp \
    EntryEditorScreen.cpp \
    Global.cpp \
    HotKeyEditorScreen.cpp \
    ListEditorScreen.cpp \
    M3UPaser.cpp \
    OpenMediaScreen.cpp \
    PlaylistManager.cpp \
    QtPlayer.cpp \
    ScreensaverInhibitor.cpp \
    SettingsManager.cpp \
    SplashScreen.cpp \
    StreamBuffer.cpp \
    VLCPlayer.cpp \
    VideoControlPanel.cpp \
    VideoOverlay.cpp \
    VideoPlayer.cpp \
    WidgetDelegates.cpp \
    main.cpp \
    SpyderPlayerApp.cpp

HEADERS += \
    AboutScreen.h \
    AppData.h \
    AppSettingsScreen.h \
    DraggableWidget.h \
    EntryEditorScreen.h \
    Global.h \
    HotKeyEditorScreen.h \
    ListEditorScreen.h \
    M3UParser.h \
    OpenMediaScreen.h \
    PlaylistManager.h \
    QtPlayer.h \
    ScreensaverInhibitor.h \
    SettingsManager.h \
    SplashScreen.h \
    SpyderPlayerApp.h \
    StreamBuffer.h \
    VLCPlayer.h \
    VideoControlPanel.h \
    VideoOverlay.h \
    VideoPlayer.h \
    WidgetDelegates.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    assets/resources.qrc

FORMS += \
    assets/About.ui \
    assets/EntryEditor.ui \
    assets/HotkeySettings.ui \
    assets/OpenFileSelection.ui \
    assets/Overlay.ui \
    assets/PlayListSettings.ui \
    assets/PlayerMainWindow.ui \
    assets/PlayerSettings.ui \
    assets/Settings.ui \
    assets/SplashScreen.ui \
    assets/VideoControlPanel.ui

DISTFILES += \
    assets/icons/back-disabled.png \
    assets/icons/back.png \
    assets/icons/cancel-disabled.png \
    assets/icons/cancel.png \
    assets/icons/check-disabled.png \
    assets/icons/check.png \
    assets/icons/close-caption-disabled.png \
    assets/icons/close-caption.png \
    assets/icons/close.png \
    assets/icons/collapsed.png \
    assets/icons/expand.png \
    assets/icons/expanded.png \
    assets/icons/external-link-disabled.png \
    assets/icons/external-link.png \
    assets/icons/forward-disabled.png \
    assets/icons/forward.png \
    assets/icons/keyboard.png \
    assets/icons/last-disabled.png \
    assets/icons/last.png \
    assets/icons/library.png \
    assets/icons/list.png \
    assets/icons/minimize.png \
    assets/icons/mute-disabled.png \
    assets/icons/mute.png \
    assets/icons/next-disabled.png \
    assets/icons/next.png \
    assets/icons/pause-disabled.png \
    assets/icons/pause.png \
    assets/icons/play-disabled.png \
    assets/icons/play-pause.png \
    assets/icons/play.png \
    assets/icons/play_empty.png \
    assets/icons/playlist.png \
    assets/icons/playlists.png \
    assets/icons/previous-disabled.png \
    assets/icons/previous.png \
    assets/icons/reorder-disabled.png \
    assets/icons/reorder.png \
    assets/icons/repeat.png \
    assets/icons/search-list.png \
    assets/icons/search-white.png \
    assets/icons/search.png \
    assets/icons/settings-sliders.png \
    assets/icons/settings.png \
    assets/icons/spider-black.png \
    assets/icons/spider-dark-icon-small.png \
    assets/icons/spider-dark-icon.png \
    assets/icons/spider-large-dark.png \
    assets/icons/spider-large.png \
    assets/icons/spider.png \
    assets/icons/spider_dark_icon.icns \
    assets/icons/spider_dark_icon.ico \
    assets/icons/star-empty.png \
    assets/icons/star-full.png \
    assets/icons/star-trans.png \
    assets/icons/star-white.png \
    assets/icons/stop-disabled.png \
    assets/icons/stop.png \
    assets/icons/subtitles-disabled.png \
    assets/icons/subtitles.png \
    assets/icons/videofile.png \
    assets/icons/volume-disabled.png \
    assets/icons/volume.png \
    assets/icons/white-diskette-disabled.png \
    assets/icons/white-diskette.png \
    assets/icons/white-edit-disabled.png \
    assets/icons/white-edit.png \
    assets/icons/white-folder-disabled.png \
    assets/icons/white-folder.png \
    assets/icons/white-hide-disabled.png \
    assets/icons/white-hide.png \
    assets/icons/white-information.png \
    assets/icons/white-keyboard.png \
    assets/icons/white-left-arrow-disabled.png \
    assets/icons/white-left-arrow.png \
    assets/icons/white-minus-disabled.png \
    assets/icons/white-minus.png \
    assets/icons/white-plus-sign-disabled.png \
    assets/icons/white-plus-sign.png