#include "ListEditorScreen.h"
#include "SettingsManager.h"
#include <QFont>

ListEditor::ListEditor(SettingsManager* settingsManager, ENUM_SETTINGS_VIEWS viewType)
{
    settingsManager_ = settingsManager;
    ui_.setupUi(this);
    
    viewType_ = viewType;
    QFont columnFont;

    if(viewType_ ==ENUM_SETTINGS_VIEWS::PLAYLIST)
    {
        ui_.Titlebar_label->setText("Playlist Settings");
        editList_ = settingsManager_->appData_->PlayLists_;
    }
    else if(viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY)
    {
        ui_.Titlebar_label->setText("Library Settings");
        editList_ = settingsManager_->appData_->Library_;

        // Setup Column Header
        columnFont.setPointSize(14);
        QTableWidgetItem* column0 = new QTableWidgetItem("Library Entry Name");
        column0->setFont(columnFont);
        ui_.PlayList_table->setHorizontalHeaderItem(0, column0);
    }
    else if(viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES)
    {
        ui_.Titlebar_label->setText("Favorites Settings");
        editList_ = settingsManager_->appData_->Favorites_;

        // Setup Column Header
        columnFont.setPointSize(14);
        QTableWidgetItem* column1 = new QTableWidgetItem("Favorite Name");
        column1->setFont(columnFont);
        ui_.PlayList_table->setHorizontalHeaderItem(0, column1);
        ui_.AddNew_button->hide();
        ui_.Edit_button->setIcon(QIcon(":/icons/icons/white-diskette.png"));
        ui_.Edit_button->setText(" Save as...");
    }
    else 
    {
        ui_.Titlebar_label->setText("Unknown Settings....");
    }

    // Setup Columns
    ui_.PlayList_table->setColumnWidth(0, 250);
    ui_.PlayList_table->setColumnWidth(1, 360);
    ui_.PlayList_table->setWordWrap(true);

    // Initialize Delegates
    cellDelegate_ = new ReadOnlyDelegate(this);
    ui_.PlayList_table->setItemDelegateForColumn(0, cellDelegate_);
    ui_.PlayList_table->setItemDelegateForColumn(1, cellDelegate_);

    // Setup Drop Actions
    ui_.PlayList_table->setDragDropMode(QTableWidget::DragDropMode::InternalMove);
    ui_.PlayList_table->setDragEnabled(false);
    ui_.PlayList_table->setAcceptDrops(false);
    // Connect Slots
    connect(ui_.Back_button, &QPushButton::clicked, this, &ListEditor::BackButtonClicked);
    //connect(ui_.AddNew_button, &QPushButton::clicked, this, &ListEditor::AddNewButtonClicked);
    //connect(ui_.Edit_button, &QPushButton::clicked, this, &ListEditor::EditButtonClicked);
    //connect(ui_.Delete_button, &QPushButton::clicked, this, &ListEditor::DeleteButtonClicked);
    connect(ui_.Apply_button, &QPushButton::clicked, this, &ListEditor::ApplyButtonClicked);
    connect(ui_.Reorder_button, &QPushButton::clicked, this, &ListEditor::ReorderButtonClicked);
    connect(ui_.Cancel_button, &QPushButton::clicked, this, &ListEditor::CancelButtonClicked);

    connect(ui_.PlayList_table, &QTableWidget::cellClicked, this, &ListEditor::RowSelected);
    //connect(ui_.PlayList_table, &QTableWidget::cell, this, &ListEditor::RowDoubleClicked);

    ui_.PlayList_table->viewport()->installEventFilter(this);

    // Hide Buttons
    ui_.Apply_button->hide();
    ui_.Cancel_button->hide();

}

ListEditor::~ListEditor()
{
    delete cellDelegate_;
}

void ListEditor::BackButtonClicked()
{
    settingsManager_->ShowSettingsMainScreen();
}

bool ListEditor::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui_.PlayList_table->viewport() && event->type() == QEvent::Drop)
    {
        dropEvent( static_cast<QDropEvent*>(event) );
        return true;
    }

    return QObject::eventFilter(object, event);
}

void ListEditor::dropEvent(QDropEvent* event)
{
    if(reordering_)
    {
        // Get the row being dragged
        int selected_row = ui_.PlayList_table->selectedItems()[0]->row();

        // Get the target row
        int drop_row = ui_.PlayList_table->rowAt(event->position().toPoint().y());

        // If dropping below the last row, ignore
        if (drop_row == -1)
        {
            event->ignore();
            return;
        }

        // Prevent dropping on itself
        if (selected_row == drop_row)
        {
            event->ignore();
            return;
        }

        // Get all data from the row being moved
        PlayListEntry moving_item = tempList_.takeAt(selected_row);

        // Insert the item at the new position
        tempList_.insert(drop_row, moving_item);

        // Select the moved row
        ui_.PlayList_table->selectRow(drop_row);

        UpdateTable();
        event->accept();
    }
    else
        event->ignore();
}

void ListEditor::AddEditList(QVector<PlayListEntry> list)
{
    editList_ = list;
}

void ListEditor::ShowListEditor()
{
    reordering_ = false;
    UpdateTable();
}   

void ListEditor::UpdateTable()
{
    // Clear the table
    ui_.PlayList_table->clearContents();
    ui_.PlayList_table->setRowCount(0);    

    QVector<PlayListEntry> displayList;
    if (reordering_)      
        displayList = tempList_;
    else
        displayList = editList_;

    // Update the table row count
    ui_.PlayList_table->setRowCount(displayList.size()); 
    
    // Update the table
    for (int row = 0; row < displayList.size(); row++)
    {
        QTableWidgetItem* name = new QTableWidgetItem(QSTR(displayList[row].name));
        QTableWidgetItem* source = new QTableWidgetItem(QSTR(displayList[row].source));
        name->setTextAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignVCenter);
        source->setTextAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignVCenter);
        ui_.PlayList_table->setItem(row, 0, name);
        ui_.PlayList_table->setItem(row, 1, source);
        ui_.PlayList_table->setRowHeight(row, 50);
    }

    // Enable buttons if not currently reordering
    if (!reordering_)
    {
        ui_.PlayList_table->setCurrentCell(-1,-1);

        ui_.AddNew_button->setEnabled(true);
        
        if (displayList.size() > 0)
        {
            ui_.Delete_button->setEnabled(false);                
            ui_.Reorder_button->setEnabled(true);   

            if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES) 
            {
                ui_.Edit_button->setEnabled(true);
                ui_.Edit_button->setIcon(QIcon(":/icons/icons/white-diskette.png"));
            }
            else
                ui_.Edit_button->setEnabled(false);
        }
        else
        {
            ui_.Delete_button->setEnabled(false);
            ui_.Reorder_button->setEnabled(false);
            ui_.Edit_button->setEnabled(false);

            if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES) 
            {
                ui_.Edit_button->setIcon(QIcon(":/icons/icons/white-diskette-disabled.png"));
                ui_.Edit_button->setEnabled(false);
            }
        }
        ui_.Back_button->setEnabled(true);
    }
}

void ListEditor::UnSelectRows()
{
    ui_.PlayList_table->setCurrentCell(-1,-1);
    RowSelected(-1, -1);
}

void ListEditor::RowSelected(int row, int col)
{
    if (row >= 0)
    {
        ui_.Edit_button->setEnabled(true);
        ui_.Delete_button->setEnabled(true);
    }
    else if (viewType_ != ENUM_SETTINGS_VIEWS::FAVORITES)
    {
        ui_.Edit_button->setEnabled(false);
        ui_.Delete_button->setEnabled(false);
    }
}

void ListEditor::ReorderButtonClicked()
{
    ui_.PlayList_table->setDragEnabled(true);
    ui_.PlayList_table->setAcceptDrops(true);
    ui_.AddNew_button->setEnabled(false);
    ui_.Delete_button->setEnabled(false);
    ui_.Reorder_button->setEnabled(false);
    ui_.Back_button->setEnabled(false);     
    if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES) 
    {
        ui_.Edit_button->setEnabled(false);
        ui_.Edit_button->setIcon(QIcon(":/icons/icons/white-diskette-disabled.png")); 
    }
    else
        ui_.Edit_button->setEnabled(false);  
    ui_.Apply_button->show();
    ui_.Cancel_button->show();
    reordering_ = true;
    tempList_ = editList_;
}

void ListEditor::ApplyButtonClicked()
{
    ui_.PlayList_table->setDragEnabled(false);
    ui_.PlayList_table->setAcceptDrops(false);
    ui_.Apply_button->hide();
    ui_.Cancel_button->hide();
    reordering_ = false;

    if (viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST)
        settingsManager_->appData_->PlayLists_ = tempList_;
    else if (viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY)
        settingsManager_->appData_->Library_ = tempList_;
    else if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES)
        settingsManager_->appData_->Favorites_ = tempList_;
    
    settingsManager_->SaveSettings(true);
    
    if (viewType_ == ENUM_SETTINGS_VIEWS::PLAYLIST)
        editList_ = settingsManager_->appData_->PlayLists_;
    else if (viewType_ == ENUM_SETTINGS_VIEWS::LIBRARY)
        editList_ = settingsManager_->appData_->Library_;
    else if (viewType_ == ENUM_SETTINGS_VIEWS::FAVORITES)
        editList_ = settingsManager_->appData_->Favorites_;
    
    UpdateTable();

}

void ListEditor::CancelButtonClicked()
{
    ui_.PlayList_table->setDragEnabled(false);
    ui_.PlayList_table->setAcceptDrops(false);
    ui_.Apply_button->hide();
    ui_.Cancel_button->hide();
    reordering_ = false;
    UpdateTable();
}
