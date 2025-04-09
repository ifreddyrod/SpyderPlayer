#include "AppSettingsScreen.h"
#include "SettingsManager.h" 
#include <QFileDialog>
#include <QMessageBox>

//*****************************************************************************************
// Application Settings Class
//***************************************************************************************** 
AppSettings::AppSettings(SettingsManager* settingsManager)
{
    ui_.setupUi(this);
    settingsManager_ = settingsManager;

    // Connect Slots
    connect(ui_.Back_button, &QPushButton::clicked, this, &AppSettings::BackButtonClicked);
    connect(ui_.PlayerType_combobox, &QComboBox::currentIndexChanged, this, &AppSettings::PlayerTypeChanged);
    connect(ui_.PlaylistPath_textedit, &QTextEdit::textChanged, this, &AppSettings::PathChanged);
    connect(ui_.OpenPath_button, &QPushButton::clicked, this, &AppSettings::OpenPathButtonClicked);
    connect(ui_.Save_button, &QPushButton::clicked, this, &AppSettings::SaveButtonClicked);
    connect(ui_.RetryCount_box, &QSpinBox::valueChanged, this, &AppSettings::RetryCountChanged);
    connect(ui_.RetryTimeDelay_box, &QSpinBox::valueChanged, this, &AppSettings::RetryDelayChanged);
}

void AppSettings::ShowAppSettings()
{
    blockSignals(true);
    string playerType = PlayerTypeToString(settingsManager_->appData_->PlayerType_);
    string playListsPath = settingsManager_->appData_->PlayListsPath_;

    PRINT << "Player Type: " << QSTR(playerType);
    PRINT << "PlayLists Path: " << QSTR(playListsPath);

    ui_.PlaylistPath_textedit->setText(QSTR(playListsPath));
    ui_.PlayerType_combobox->setCurrentText(QSTR(playerType));
    ui_.RetryCount_box->setValue(settingsManager_->appData_->RetryCount_);
    ui_.RetryTimeDelay_box->setValue(settingsManager_->appData_->RetryDelay_);

    modified_ = false;
    blockSignals(false);
}

void AppSettings::BackButtonClicked()
{
    if (modified_)
    {
        int ret = ShowSaveWarningDialog("Save Changes", "Settings have been changed. Save changes?");

        if (ret == QMessageBox::Yes)
        {
            SaveButtonClicked();
        }
        else if (ret == QMessageBox::No)
        {
            settingsManager_->ShowSettingsMainScreen();
        }
        else if (ret == QMessageBox::Cancel)
        {
            return;
        }
    }
    else
    {
        settingsManager_->ShowSettingsMainScreen();
    }
}

void AppSettings::PlayerTypeChanged()
{
    modified_ = true;
    PRINT << "New Player Type: " << ui_.PlayerType_combobox->currentText();
}

void AppSettings::PathChanged()
{
    modified_ = true;
    PRINT << "New Path: " << ui_.PlaylistPath_textedit->toPlainText();
}

void AppSettings::OpenPathButtonClicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select Directory", QSTR(settingsManager_->appData_->PlayListsPath_));

    if (path.isEmpty())
        return;
    else if (path != QSTR(settingsManager_->appData_->PlayListsPath_))
    {
        ui_.PlaylistPath_textedit->setText(path);
        modified_ = true;
    }
}

void AppSettings::RetryCountChanged()
{
    modified_ = true;
}

void AppSettings::RetryDelayChanged()
{
    modified_ = true;
}

void AppSettings::SaveButtonClicked()
{
    if (modified_)
    {
        settingsManager_->appData_->PlayListsPath_ = ui_.PlaylistPath_textedit->toPlainText().toStdString();
        settingsManager_->appData_->PlayerType_ = StringToPlayerTypeEnum(ui_.PlayerType_combobox->currentText().toStdString());
        settingsManager_->appData_->RetryCount_ = ui_.RetryCount_box->value();
        settingsManager_->appData_->RetryDelay_ = ui_.RetryTimeDelay_box->value();
        settingsManager_->changesMade_ = true;
        settingsManager_->SaveSettings();
        settingsManager_->ShowSettingsMainScreen();
    }
}
