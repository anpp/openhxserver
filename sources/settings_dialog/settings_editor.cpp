#include "settings_editor.h"
#include "settings_com_port.h"
#include "settings_images.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileInfo>


//-------------------------------------------------------------------------------------------------------------------------------------
SettingsEditor::SettingsEditor(Images& images_data, QWidget *parent) : QDialog(parent)
{
    this->setWindowTitle(tr("Settings"));
    this->setMinimumSize(default_width, default_height);
    this->setWindowIcon(QIcon(":/images/icons/settings-light.png"));

    images_widget = std::make_unique<SettingsImages>(images_data);
    com_port_widget = std::make_unique<SettingsCOMPort>();

    setup();

    connect(&buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(&buttonBox, &QDialogButtonBox::accepted, this, &SettingsEditor::accept_slot);
    connect(&buttonBox, &QDialogButtonBox::clicked, this, &SettingsEditor::clicked_slot);
}

//-------------------------------------------------------------------------------------------------------------------------------------
SettingsEditor::~SettingsEditor()
{

}


//-------------------------------------------------------------------------------------------------------------------------------------
void SettingsEditor::accept_slot()
{
    applySettings();
    accept();
}

//-------------------------------------------------------------------------------------------------------------------------------------
void SettingsEditor::clicked_slot(QAbstractButton* button)
{
    if(buttonBox.standardButton(button) == QDialogButtonBox::Apply)
    {
        applySettings();
        emit apply();
    }
}


//-------------------------------------------------------------------------------------------------------------------------------------
void SettingsEditor::setup()
{
    auto layout = new QVBoxLayout(this);
    layout->setMargin(margin);

    buttonBox.setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok | QDialogButtonBox::Apply);

    com_port_widget->load();
    images_widget->update();

    tw.addTab(images_widget.get(), QIcon(":/images/icons/disk-light.png"), tr("Images"));
    tw.addTab(com_port_widget.get(), QIcon(":/images/icons/com-light.png"), tr("COM port"));
    tw.setTabsClosable(false);
    tw.setDocumentMode(false);
    tw.setMovable(false);

    layout->addWidget(&tw);
    layout->addWidget(&buttonBox);
    this->setLayout(layout);
}

//-------------------------------------------------------------------------------------------------------------------------------------
void SettingsEditor::applySettings()
{    
    images_widget->save();
    com_port_widget->save();
}
