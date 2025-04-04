#include "settings_images.h"
#include "ui_settings_images.h"
#include "images_model.h"
#include "images_delegate.h"
#include "../settings.h"
#include "server/images.h"

#include <QAction>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QFileInfo>

//-----------------------------------------------------------------------------------------------------------------
SettingsImages::SettingsImages(Images& images_data, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsImages)
{
    ui->setupUi(this);

    m_model = std::make_unique<ImagesModel>(images_data);
    ui->tvImages->setModel(m_model.get());

    m_delegate = std::make_unique<ImagesDelegate>(*m_model);
    ui->tvImages->setItemDelegateForColumn(static_cast<int>(ImagesModel::ImagesModel_defs::FileName), m_delegate.get());

    ui->tvImages->resizeColumnsToContents();
    ui->tvImages->resizeRowsToContents();

    m_openLoaderAction = ui->leLoader->addAction(QIcon(":/images/icons/folder-open-light.png"), QLineEdit::TrailingPosition);
    m_openLoaderAction->setToolTip(tr("Open file..."));
    connect(m_openLoaderAction, &QAction::triggered, this, &SettingsImages::openFileBin);

    m_openSAVAction = ui->leSAV->addAction(QIcon(":/images/icons/folder-open-light.png"), QLineEdit::TrailingPosition);
    m_openSAVAction->setToolTip(tr("Open file..."));
    connect(m_openSAVAction, &QAction::triggered, this, &SettingsImages::openFileSAV);

    connect(m_model.get(), &ImagesModel::selected_file, this, &SettingsImages::selected_image_file);
    connect(&images_data, &Images::update, this, &SettingsImages::updateWidget);
    connect(ui->rbLoaderHX, &QRadioButton::clicked, this,  &SettingsImages::rbModeClicked);
    connect(ui->rbLoaderSAV, &QRadioButton::clicked, this,  &SettingsImages::rbModeClicked);
}

//-----------------------------------------------------------------------------------------------------------------
SettingsImages::~SettingsImages()
{
    delete ui;
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::setLoader(const QString &value)
{
    if(QFile(value).exists())
    {
        m_loader = value;
        ui->leLoader->setText(QFileInfo(m_loader).fileName());
    }
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::setSAVFile(const QString &value)
{
    if(QFile(value).exists())
    {
        m_SAVFile = value;
        ui->leSAV->setText(QFileInfo(m_SAVFile).fileName());
    }
}

//-----------------------------------------------------------------------------------------------------------------
bool SettingsImages::HXMode() const
{
    return ui->rbLoaderHX->isChecked();
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::setSaveImmediate(bool a_save_immediate)
{
    m_save_immediate = a_save_immediate;
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::update()
{
    if(Settings::instance())
    {
        setLoader(Settings::instance()->getSetting("loader", kindset::misc).toString());
        setSAVFile(Settings::instance()->getSetting("savfile", kindset::misc).toString());

        ui->rbLoaderHX->setChecked(Settings::instance()->getSetting("HXMode", kindset::misc).toBool());
        ui->rbLoaderSAV->setChecked(!Settings::instance()->getSetting("HXMode", kindset::misc).toBool());
    }

    updateWidget();
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::save()
{
    saveLoaders();
    saveImages();
    saveHXMode(HXMode());
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::saveLoaders() const
{
    if(Settings::instance())
    {
        Settings::instance()->setSetting("loader", loader(), kindset::misc);
        Settings::instance()->setSetting("directory_bin", QFileInfo(loader()).path(), kindset::environment);        
        Settings::instance()->setSetting("savfile", SAV(), kindset::misc);
        Settings::instance()->setSetting("directory_sav", QFileInfo(SAV()).path(), kindset::environment);

        Settings::instance()->saveSettingsByKind(kindset::misc);
        Settings::instance()->saveSettingsByKind(kindset::environment);
    }
    emit updateHX();
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::saveImages() const
{
    m_model->save();
    if(Settings::instance())
    {
        if(!last_image_file.isEmpty() && last_image_file != "" && QFile(last_image_file).exists())
        {            
            Settings::instance()->setSetting("directory_dsk", QFileInfo(last_image_file).path(), kindset::environment);
            Settings::instance()->saveSettingsByKind(kindset::environment);
        }
    }
    emit updateHX();
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::saveHXMode(bool value) const
{
    if(Settings::instance())
        Settings::instance()->setSetting("HXMode", value, kindset::misc);

    Settings::instance()->saveSettingsByKind(kindset::misc);

    emit updateHX();
}

//Нужно отрефакторить - 2 почти одинаковые функции...
//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::openFileBin()
{
    QString filename;
    QStringList filters;
    QString defaultFilter = tr("Bin files (*.bin)");

    filters << defaultFilter << tr("All files (*.*)");

    QFileDialog fd(this, QObject::tr("Open file..."), Settings::instance()->getSetting("directory_bin").toString(), filters.join(";;"));
    fd.selectNameFilter(defaultFilter);

    connect(&fd, &QFileDialog::filterSelected, this, [&defaultFilter](const QString &filter) {defaultFilter = filter; });
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setAcceptMode(QFileDialog::AcceptOpen);

    if(fd.exec())
    {
        filename = fd.selectedFiles().at(0);
        setLoader(filename);
        if(m_save_immediate)
            saveLoaders();
    }
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::openFileSAV()
{
    QString filename;
    QStringList filters;
    QString defaultFilter = tr("SAV files (*.sav)");

    filters << defaultFilter << tr("All files (*.*)");

    QFileDialog fd(this, QObject::tr("Open file..."), Settings::instance()->getSetting("directory_sav").toString(), filters.join(";;"));
    fd.selectNameFilter(defaultFilter);

    connect(&fd, &QFileDialog::filterSelected, this, [&defaultFilter](const QString &filter) {defaultFilter = filter; });
    fd.setFileMode(QFileDialog::ExistingFile);
    fd.setAcceptMode(QFileDialog::AcceptOpen);

    if(fd.exec())
    {
        filename = fd.selectedFiles().at(0);
        setSAVFile(filename);
        if(m_save_immediate)
            saveLoaders();
    }
}


//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::selected_image_file(const QString &filename)
{
    last_image_file = filename;

    if(m_save_immediate)
        saveImages();
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::updateWidget() const
{
    emit m_model->dataChanged(QModelIndex(), QModelIndex());
}

//-----------------------------------------------------------------------------------------------------------------
void SettingsImages::rbModeClicked(bool checked)
{
    QRadioButton *rb = static_cast<QRadioButton*>(sender());
    if(!rb) return;

    if(m_save_immediate)
        saveHXMode(ui->rbLoaderHX == rb ? checked : !checked);
}

