#include "HotKeyEditorScreen.h"
#include "SettingsManager.h"
#include "Global.h"
#include <QMessageBox>
#include <QComboBox>
#include <QtCore/qmetaobject.h>


#include <QStyledItemDelegate>
#include <QComboBox>
#include <QStringList>

//*****************************************************************************************
// Delegate Definitions
//*****************************************************************************************
ComboBoxDelegate::ComboBoxDelegate(const QStringList &items, QObject *parent)
    : QStyledItemDelegate(parent)
    , items_(items)
    , enabled_(true)
{
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    
    QComboBox *editor = new QComboBox(parent);
    editor->addItems(items_);
    editor->setEnabled(enabled_);
    return editor;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentText(value);
}

void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString value = comboBox->currentText();
    model->setData(index, value, Qt::EditRole);
}


//*****************************************************************************************
// HotKey Settings Class
//***************************************************************************************** 
HotKeySettings::HotKeySettings(SettingsManager* settingsManager)
{
    ui_.setupUi(this);
    settingsManager_ = settingsManager;

    // Setup Table Columns
    ui_.HotKeys_table->setColumnWidth(0, 250);
    ui_.HotKeys_table->setColumnWidth(1, 350);
    ui_.HotKeys_table->setWordWrap(true);

    // Initialize Delegates
    playerAction_ = new ReadOnlyDelegate(this); 
    hkComboBox_ = new ComboBoxDelegate(QStringList(), this);

    // Set delegates for columns
    ui_.HotKeys_table->setItemDelegateForColumn(0, playerAction_);
    ui_.HotKeys_table->setItemDelegateForColumn(1, hkComboBox_);

    LoadHotKeySettings();

    ui_.Apply_button->hide();
    ui_.Cancel_button->hide();

    // Connect Slots
    connect(ui_.Back_button, &QPushButton::clicked, this, &HotKeySettings::BackButtonClicked);
    connect(ui_.Apply_button, &QPushButton::clicked, this, &HotKeySettings::ApplyButtonClicked);
    connect(ui_.Cancel_button, &QPushButton::clicked, this, &HotKeySettings::CancelButtonClicked);
    connect(ui_.Edit_button, &QPushButton::clicked, this, &HotKeySettings::EditButtonClicked);
    connect(ui_.HotKeys_table, &QTableWidget::itemChanged, this, &HotKeySettings::HotkeyChanged);
    
}
HotKeySettings::~HotKeySettings() 
{
}


void HotKeySettings::LoadHotKeySettings()
{
    // Block signals temporarily to prevent itemChanged signal during setup
    ui_.HotKeys_table->blockSignals(true);
    
    // Clear table contents
    ui_.HotKeys_table->clearContents();
    ui_.HotKeys_table->setRowCount(0);
    hotkeys_.clear();

    // Get Hotkeys from AppData
    hotkeys_ = settingsManager_->appData_->HotKeys_.GetAllHotkeys();
    ui_.HotKeys_table->setRowCount(hotkeys_.size());

    // Get all the Qt::Key names
    QStringList hkNames;
    QMetaEnum metaEnum = QMetaEnum::fromType<Qt::Key>();
    for (int i = 0; i < metaEnum.keyCount(); ++i) 
    {
        hkNames.append(metaEnum.key(i));
    }

    // Update the combo box items
    if (hkComboBox_) {
        dynamic_cast<ComboBoxDelegate*>(hkComboBox_)->setItems(hkNames);
    }

    // Update Table with Hotkeys
    int row = 0;
    for (auto hkEntry = hotkeys_.begin(); hkEntry != hotkeys_.end(); ++hkEntry) 
    {
        // Create action item
        QTableWidgetItem* actionItem = new QTableWidgetItem(hkEntry.key());
        actionItem->setTextAlignment(Qt::AlignCenter);
        
        // Create hotkey item
        QString keyName = metaEnum.valueToKey(hkEntry.value());
        QTableWidgetItem* hkItem = new QTableWidgetItem(keyName);
        hkItem->setTextAlignment(Qt::AlignCenter);
        
        // Set items safely
        ui_.HotKeys_table->setItem(row, 0, actionItem);
        ui_.HotKeys_table->setItem(row, 1, hkItem);
        
        row++;
    }
    
    // Configure the combo box delegate
    if (hkComboBox_) {
        hkComboBox_->setEnabled(false);
    }
    
    // Re-enable signals
    ui_.HotKeys_table->blockSignals(false);
}

void HotKeySettings::BackButtonClicked()
{
    if (modified_)
    {
        int ret = ShowSaveWarningDialog("Save HotKey Changes", "HotKeys have been changed. Save changes?");
        
        if (ret == QMessageBox::Yes)
        {
            ApplyButtonClicked();
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

void HotKeySettings::EditButtonClicked()
{
    hkComboBox_->setEnabled(true);
    
    ui_.Apply_button->show();
    ui_.Cancel_button->show();
    ui_.Edit_button->hide();
    ui_.Back_button->hide();
    modified_ = false;
}

void HotKeySettings::CancelButtonClicked()
{
    hkComboBox_->setEnabled(false);
    
    ui_.Apply_button->hide();
    ui_.Cancel_button->hide();
    ui_.Edit_button->show();
    ui_.Back_button->show();
    LoadHotKeySettings();
    modified_ = false;
}

void HotKeySettings::HotkeyChanged(QTableWidgetItem* item)
{
    int row = ui_.HotKeys_table->currentRow();
    
    // Find Hotkey in Qt::Key enum
    QMetaEnum metaEnum = QMetaEnum::fromType<Qt::Key>();
    int key = metaEnum.keyToValue(item->text().toStdString().c_str());

    QString hotkeyName = ui_.HotKeys_table->item(row, 0)->text();

    if (hotkeys_[hotkeyName] != key)
    {
        PRINT << "Hotkey changed for " << hotkeyName << " from " << hotkeys_[hotkeyName] << " to " << key;
        modified_ = true;
    }
}

void HotKeySettings::ApplyButtonClicked()
{
    if (modified_)
    {
        // Check for duplicate hotkey names
        if (CheckForDuplicateHotkeys())
        {
            QMessageBox::warning(this, "Hotkey Error", "Duplicate hotkey detected! Make sure all hotkeys are unique.", 
                                QMessageBox::Ok);
            return;
        }
        
        // Save hotkeys to your settings
        QMetaEnum metaEnum = QMetaEnum::fromType<Qt::Key>();
        
        for (int row = 0; row < ui_.HotKeys_table->rowCount(); ++row)
        {
            QString action = ui_.HotKeys_table->item(row, 0)->text();
            QString keyName = ui_.HotKeys_table->item(row, 1)->text();
            
            int keyValue = metaEnum.keyToValue(keyName.toStdString().c_str());
            
            // Update the hotkeys map
            hotkeys_[action] = keyValue;
        }
        
        // Save to AppData
        settingsManager_->appData_->HotKeys_.UpdateFromMap(hotkeys_);
        settingsManager_->appData_->Save();
        
        // Switch UI back to view mode
        hkComboBox_->setEnabled(false);
        ui_.Apply_button->hide();
        ui_.Cancel_button->hide();
        ui_.Edit_button->show();
        ui_.Back_button->show();
        modified_ = false;
    }
}

bool HotKeySettings::CheckForDuplicateHotkeys()
{
    QSet<QString> usedKeys;
    
    // Iterate through all rows in the table
    for (int row = 0; row < ui_.HotKeys_table->rowCount(); ++row)
    {
        // Get the hotkey value from column 1
        QString keyName = ui_.HotKeys_table->item(row, 1)->text();
        
        // If this key is already in our set, we have a duplicate
        if (usedKeys.contains(keyName))
        {
            return true;
        }
        
        // Add the key to our set
        usedKeys.insert(keyName);
    }
    
    // No duplicates found
    return false;
}