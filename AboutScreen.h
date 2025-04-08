#ifndef ABOUTSCREEN_H
#define ABOUTSCREEN_H

#include "Global.h"
#include "DraggableWidget.h"
#include "ui_About.h"

class SettingsManager;

//*****************************************************************************************
// About Screen Class
//*****************************************************************************************
class AboutScreen : public DraggableWidget
{
    Q_OBJECT
public:
    AboutScreen(SettingsManager* settingsManager);

private:
    Ui::About ui_;
    SettingsManager* settingsManager_;
};

#endif