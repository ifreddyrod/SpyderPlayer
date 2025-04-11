#include "OpenMediaScreen.h"
#include "SettingsManager.h"
#include "Global.h"


OpenMedia::OpenMedia(SettingsManager* settingsManager, ENUM_SETTINGS_VIEWS viewType) : settingsManager_(settingsManager), viewType_(viewType)
{
    ui_.setupUi(this);
    
    ui_.Open_button->setEnabled(false);
    
    connect(ui_.Back_button, &QPushButton::clicked, this, &OpenMedia::BackButtonClicked);
    connect(ui_.Open_button, &QPushButton::clicked, this, &OpenMedia::OpenButtonClicked);
    connect(ui_.OpenFiles_button, &QPushButton::clicked, this, &OpenMedia::OpenFileButtonClicked);
    connect(ui_.Source_textedit, &QTextEdit::textChanged, this, &OpenMedia::EntryChanged);
    connect(ui_.SourceType_combobox, &QComboBox::currentIndexChanged, this, &OpenMedia::SourceTypeChanged);
}

OpenMedia::~OpenMedia()
{
}

void OpenMedia::ShowOpenMediaScreen()
{
    blockSignals(true);

    if(viewType_ == ENUM_SETTINGS_VIEWS::OPEN_PLAYLIST)
        ui_.Titlebar_label->setText("Enter a PlayList File or Paste URL");
    else
        ui_.Titlebar_label->setText("Open Local File or Paste URL");
    
    ui_.SourceType_combobox->setCurrentIndex(0);
    ui_.Source_textedit->setText("");
    ui_.Source_textedit->setFocus();
    ui_.OpenFiles_button->setEnabled(true);
    ui_.OpenFiles_button->show();
    blockSignals(false);
    modified_ = false;
}

void OpenMedia::BackButtonClicked()
{
    if (modified_)
    {
        int ret = ShowSaveWarningDialog("Save Changes", "Entries have been changed. Save changes?");

        if (ret == QMessageBox::Yes)
        {
            OpenButtonClicked();
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

void OpenMedia::SourceTypeChanged()
{
    if (ui_.SourceType_combobox->currentIndex() == 0)
    {
        ui_.OpenFiles_button->setEnabled(true);
        ui_.OpenFiles_button->show();
    }
    else if (ui_.SourceType_combobox->currentIndex() == 1)
    {
        ui_.OpenFiles_button->setEnabled(false);
        ui_.OpenFiles_button->hide();
    }
    modified_ = true;   
}

void OpenMedia::EntryChanged()
{
    //blockSignals(true);
    QString sourcePath = ui_.Source_textedit->toPlainText();

    if (sourcePath.startsWith("http://") || sourcePath.startsWith("https://"))
    {
        ui_.Open_button->setEnabled(true);
        ui_.SourceType_combobox->setCurrentIndex(1);
    }
    else
    {
        ui_.Open_button->setEnabled(true);
        ui_.SourceType_combobox->setCurrentIndex(0);
    }
    modified_ = true;
    //blockSignals(false);
}

void OpenMedia::OpenFileButtonClicked()
{
    QString filename;

    // Show Open Files Dialog
    if(viewType_ == ENUM_SETTINGS_VIEWS::OPEN_PLAYLIST)
    {
        filename = QFileDialog::getOpenFileName(this, "Select PlayList File", QSTR(settingsManager_->appData_->PlayListsPath_), "PlayList Files (*.m3u *.m3u8)");
    }
    else
    {
        filename = QFileDialog::getOpenFileName(this, "Select a Media File", QSTR(settingsManager_->appData_->PlayListsPath_), "Media Files (*.mkv *.mp4 *.avi *.mov *.mp3 *.wmv *.wav *.mpg, *.mpeg *.m4v)");
    }

    if (filename.length() > 0)
    {
        ui_.Source_textedit->setPlainText(filename);
        ui_.Open_button->setEnabled(true);
    }   
}

void OpenMedia::OpenButtonClicked()
{
    PlayListEntry newEntry;
    newEntry.sourceType = ui_.SourceType_combobox->currentIndex() == 0 ? ENUM_SOURCE_TYPE::LOCAL_FILE : ENUM_SOURCE_TYPE::URL;
    QString source = ui_.Source_textedit->toPlainText();
    newEntry.source = STR(source);

    if(viewType_ == ENUM_SETTINGS_VIEWS::OPEN_PLAYLIST)
    {
        if (newEntry.sourceType == ENUM_SOURCE_TYPE::LOCAL_FILE)
        {
            QString fullPath = ui_.Source_textedit->toPlainText();
            QString basename = QFileInfo(fullPath).fileName();
            QString name = QFileInfo(basename).completeBaseName();
            QString extension = QFileInfo(basename).suffix(); 
            newEntry.name = STR(name);
            newEntry.parentName = STR(name);
        }
        else
        {
            if (source.startsWith("http://") or source.startsWith("https://"))
            {
                newEntry.name = STR(source.section('/', -1));
                newEntry.parentName = newEntry.name;
            }
            else
            {
                newEntry.name = STR(source);
                newEntry.parentName = newEntry.name;
            }
        }
        settingsManager_->LoadPlayList(newEntry);
    }
    else 
    {
        newEntry.parentName = "Opened Media";

        if (newEntry.sourceType == ENUM_SOURCE_TYPE::LOCAL_FILE)
        {
            QString fullPath = ui_.Source_textedit->toPlainText();
            QString basename = QFileInfo(fullPath).fileName();
            QString name = QFileInfo(basename).completeBaseName();
            QString extension = QFileInfo(basename).suffix(); 
            newEntry.name = STR(name + "." + extension);
        }
        else
        {
            if (source.startsWith("http://"))
                newEntry.name = STR(source.mid(7));  
            else if (source.startsWith("https://"))
                newEntry.name = STR(source.mid(8)); 
            else 
                newEntry.name = STR(source);
        }
        settingsManager_->LoadMediaFile(newEntry);
    }
    PRINT << "NewEntry Name: " << newEntry.name;
    PRINT << "NewEntry Source: " << newEntry.source;
    PRINT << "NewEntry ParentName: " << newEntry.parentName;
    PRINT << "NewEntry SourceType: " << newEntry.sourceType;

    modified_ = false;
}