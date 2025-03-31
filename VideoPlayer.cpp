#include "VideoPlayer.h"


// Constructor
VideoPlayer::VideoPlayer(QWidget* parent) : QWidget(parent)
{
    currentState_ = ENUM_PLAYER_STATE::IDLE;
    mainWindow_ = nullptr;
    videoPanel_ = nullptr;
    source_ = "";
    duration_ = 0;
    position_ = 0;
}

// Destructor
VideoPlayer::~VideoPlayer() {}

// UpdatePosition method
void VideoPlayer::UpdatePosition(int position) 
{
    emit SIGNAL_UpdatePosition(position);
}

// UpdateDuration method
void VideoPlayer::UpdateDuration(int duration) 
{
    emit SIGNAL_UpdateDuration(duration);
}

// UpdatePlayerState method
void VideoPlayer::UpdatePlayerState(ENUM_PLAYER_STATE state) 
{
    currentState_ = state;
    emit SIGNAL_PlayerStateChanged(state);
}

// ErrorOccured method
void VideoPlayer::ErrorOccured(const std::string& error) 
{
    emit SIGNAL_ErrorOccured(error);
}

void VideoPlayer::EnableSubtitles(bool enable)
{
    emit SIGNAL_EnableSubtitles(enable);
}