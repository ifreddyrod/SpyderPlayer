#include "EntryEditorScreen.h"
#include "SettingsManager.h"

EntryEditor::EntryEditor(SettingsManager* settingsManager, ENUM_SETTINGS_VIEWS viewType) : settingsManager_(settingsManager), viewType_(viewType)
{
    ui_.setupUi(this);

    connect(ui_.Back_button, &QPushButton::clicked, this, &EntryEditor::BackButtonClicked);
    connect(ui_.OpenFiles_button, &QPushButton::clicked, this, &EntryEditor::OpenFileButtonClicked);
    connect(ui_.Save_button, &QPushButton::clicked, this, &EntryEditor::SaveButtonClicked);

    connect(ui_.Name_textedit, &QLineEdit::textChanged, this, &EntryEditor::EntryNameChanged);
    connect(ui_.SourceType_combobox, &QComboBox::currentIndexChanged, this, &EntryEditor::SourceTypeChanged);
    connect(ui_.Source_textedit, &QTextEdit::textChanged, this, &EntryEditor::SourceChanged); 
}


EntryEditor::~EntryEditor()
{
}

void EntryEditor::ShowEntryEditor(int index)
{
    editListIndex_ = index;
    
    blockSignals(true);

    // Init fields if adding new entry
    if (editListIndex_ < 0)
    {
        if(viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
        {
            ui_.Titlebar_label->setText("PlayList New Entry");
            ui_.EntryName_label->setText("PlayList Name:");
        }
        else if(viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
        {
            ui_.Titlebar_label->setText("Library New Entry");
            ui_.EntryName_label->setText("Entry Name:");
        }
        else if(viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY)
        {
            ui_.Titlebar_label->setText("Save Favorites As New List");
            ui_.EntryName_label->setText("PlayList Name:");
            ui_.SourceType_combobox->setEnabled(false);
            ui_.Name_textedit->setPlaceholderText("Enter a unique name for your new list");
            ui_.Source_label->setText("File Path:");
            ui_.Source_textedit->setPlaceholderText("Enter file name and path");
        }

        // Clear all fields
        ui_.Name_textedit->setText("");
        ui_.SourceType_combobox->setCurrentIndex(0);
        ui_.Source_textedit->setText("");
        ui_.Source_textedit->setFocus();
        ui_.OpenFiles_button->setEnabled(true);
        ui_.OpenFiles_button->show();

        // Clear entry
        editEntry_.name = "";
        editEntry_.parentName = "";
        editEntry_.sourceType = ENUM_SOURCE_TYPE::LOCAL_FILE;
        editEntry_.source = "";

        modified_ = false;
    }
    // Load existing entry
    else
    {
        if(viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
        {
            ui_.Titlebar_label->setText("PlayList Edit Entry");
            ui_.EntryName_label->setText("PlayList Name:");
            editEntry_ = settingsManager_->appData_->PlayLists_[editListIndex_];
        }
        else if(viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
        {
            ui_.Titlebar_label->setText("Library Edit Entry");
            ui_.EntryName_label->setText("Entry Name:");
            editEntry_ = settingsManager_->appData_->Library_[editListIndex_];
        }

        // Load entry fields
        ui_.Name_textedit->setText(QSTR(editEntry_.name));
        ui_.SourceType_combobox->setCurrentIndex(editEntry_.sourceType == ENUM_SOURCE_TYPE::LOCAL_FILE ? 0 : 1);
        SourceTypeChanged();
        ui_.Source_textedit->setText(QSTR(editEntry_.source));
        ui_.Source_textedit->setFocus();

        modified_ = false;
    }
    blockSignals(false);
}

void EntryEditor::SourceTypeChanged()
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

void EntryEditor::SourceChanged()
{
    if (ui_.Source_textedit->toPlainText() != QSTR(editEntry_.source))
    {
        modified_ = true;
        if(!ui_.Name_textedit->text().isEmpty())
            ui_.Save_button->setEnabled(true);
    }
    else
    {
        ui_.Save_button->setEnabled(false);
    }
}

void EntryEditor::EntryNameChanged()
{
    if(ui_.Name_textedit->text() != QSTR(editEntry_.name))
    {
        modified_ = true;
        if(!ui_.Source_textedit->toPlainText().isEmpty())
            ui_.Save_button->setEnabled(true);
    }
    else
    {
        ui_.Save_button->setEnabled(false);
    }
}

void EntryEditor::OpenFileButtonClicked()
{
    QString filename;

    // Show Open Files Dialog
    if(viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
    {
        filename = QFileDialog::getOpenFileName(this, "Select PlayList File", QSTR(settingsManager_->appData_->PlayListsPath_), "PlayList Files (*.m3u *.m3u8)");
    }
    else if(viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
    {
        filename = QFileDialog::getOpenFileName(this, "Select a Media File", QSTR(settingsManager_->appData_->PlayListsPath_), "Media Files (*.mkv *.mp4 *.avi *.mov *.mp3 *.wmv *.wav *.mpg, *.mpeg *.m4v)");
    }
    else if(viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY)
    {
        filename = QFileDialog::getSaveFileName(this, "Enter new Playlist File", QSTR(settingsManager_->appData_->PlayListsPath_), "PlayList File (*.m3u)");
    }

    if (filename.length() > 0)
        ui_.Source_textedit->setPlainText(filename);
}

void EntryEditor::SaveButtonClicked()
{
    // Copy fields to entry
    editEntry_.name = STR(ui_.Name_textedit->text().trimmed());
    editEntry_.sourceType = ui_.SourceType_combobox->currentIndex() == 0 ? ENUM_SOURCE_TYPE::LOCAL_FILE : ENUM_SOURCE_TYPE::URL;
    editEntry_.source = STR(ui_.Source_textedit->toPlainText().trimmed());

    //--------------------
    // Add New Entry
    //--------------------
    if (editListIndex_ == -1)
    {
        if(viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
        {   editEntry_.parentName = "";
            settingsManager_->appData_->PlayLists_.push_back(editEntry_);
        }
        else if(viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
        {
            editEntry_.parentName = "Library";
            settingsManager_->appData_->Library_.push_back(editEntry_);
        }
        else if(viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY)
        {
            editEntry_.parentName = "";
            
            // Save Favorites List to m3u file
            SavePlayListToFile(settingsManager_->appData_->Favorites_, editEntry_.source);
            
            if (QFile::exists(QSTR(editEntry_.source)))
            {
                PRINT << "Favorites List saved to " + QSTR(editEntry_.source);

                // Add entry to AppData PlayLists
                settingsManager_->appData_->PlayLists_.push_back(editEntry_);

                // Remove all items from favorites list
                settingsManager_->appData_->Favorites_.clear();
            }
        }
    }
    //-------------------------
    // Modify Existing entry
    //-------------------------
    else
    {
        if(viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
        {
            settingsManager_->appData_->PlayLists_[editListIndex_] = editEntry_;
        }
        else if(viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
        {
            settingsManager_->appData_->Library_[editListIndex_] = editEntry_;
        }
    }

    // Save Changes
    settingsManager_->SaveSettings(true);

    // Go back to previous screen
    if (viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
        settingsManager_->ShowPlaylistEditor(true);
    else if (viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
        settingsManager_->ShowLibraryEditor(true);
    else if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY)
        settingsManager_->ShowFavoritesEditor(true);
    
    modified_ = false;
}

void EntryEditor::BackButtonClicked()
{
    if (modified_)
    {
        int ret = ShowSaveWarningDialog("Save Changes", "Entries have been changed. Save changes?");

        if (ret == QMessageBox::Yes)
        {
            SaveButtonClicked();
        }
        else if (ret == QMessageBox::No)
        {
            if (viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
                settingsManager_->ShowPlaylistEditor(false);
            else if (viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
                settingsManager_->ShowLibraryEditor(false);
            else if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY)
                settingsManager_->ShowFavoritesEditor(false);
        }
        else if (ret == QMessageBox::Cancel)
        {
            return;
        }
    }
    else
    {
        if (viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST_ENTRY)
            settingsManager_->ShowPlaylistEditor(false);
        else if (viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY_ENTRY)
            settingsManager_->ShowLibraryEditor(false);
        else if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES_ENTRY)
            settingsManager_->ShowFavoritesEditor(false);
    }
}
