#include "settings_misc.h"
#include "ui_settings_misc.h"
#include "../settings.h"

#include <QAction>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>

//-----------------------------------------------------------------------------------------------------------------
SettingsMisc::SettingsMisc(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SettingsMisc)
{
    ui->setupUi(this);

    m_openExeAction = ui->lePathToEm->addAction(QIcon(":/images/icons/folder-open-light.png"), QLineEdit::TrailingPosition);
    m_openExeAction->setToolTip(tr("Open file..."));
    connect(m_openExeAction, &QAction::triggered, this, &SettingsMisc::openPathToEm);

}

//-----------------------------------------------------------------------------------------------------------------
SettingsMisc::~SettingsMisc()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsMisc::setPathToEmulator(const QString &value)
{
    if(QFile(value).exists())
    {
        m_PathToEmulator = value;
        ui->lePathToEm->setText(value);
    }
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsMisc::load()
{
    if(Settings::instance())
        setPathToEmulator(Settings::instance()->getSetting("path_to_emulator", kindset::misc).toString());
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsMisc::save()
{
    Settings::instance()->setSetting("path_to_emulator", pathToEmulator(), kindset::misc);
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsMisc::openPathToEm()
{
    QString filename;
    QStringList filters;
    QString defaultFilter = tr("Exe files (*.exe)");

    filters << defaultFilter << tr("All files (*.*)");

    QFileDialog fd(this, QObject::tr("Open file..."), Settings::instance()->getSetting("path_to_emulator").toString(), filters.join(";;"));
    fd.selectNameFilter(defaultFilter);

    connect(&fd, &QFileDialog::filterSelected, this, [&defaultFilter](const QString &filter) {defaultFilter = filter; });
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setAcceptMode(QFileDialog::AcceptOpen);

    if(fd.exec())
    {
        filename = fd.selectedFiles().at(0);
        setPathToEmulator(filename);
    }
}
