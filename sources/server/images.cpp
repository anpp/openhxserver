
#include "images.h"
#include "imagedsk.h"
#include "../settings.h"

#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>

const static std::string_view  sImages = "/DskImages";

//----------------------------------------------------------------------------------------------------
Images::Images(QObject *parent)
    : QObject{parent}
{
    qsettings = std::make_unique<QSettings>(Settings::instance()->qSettings().organizationName(),
                                            Settings::instance()->qSettings().applicationName());

    default_image = std::make_unique<ImageDsk>();
    m_watcher = std::make_unique<QFileSystemWatcher>();

    for(size_t i = 0; i < Images_defs::Size; ++i)
    {
        m_data.push_back(std::make_unique<ImageDsk>());
        connect(m_data.at(i).get(), &ImageDsk::addFileName, this, &Images::addFileName);
        connect(m_data.at(i).get(), &ImageDsk::delFileName, this, &Images::delFileName);
        connect(m_data.at(i).get(), &ImageDsk::update, this, &Images::update);
    }
    connect(m_watcher.get(),  &QFileSystemWatcher::fileChanged, this, &Images::fileChanged);
}

//----------------------------------------------------------------------------------------------------
Images::~Images()
{

}

//----------------------------------------------------------------------------------------------------
void Images::load()
{
    qsettings->beginGroup(QString::fromStdString(std::string(sImages)));
    size_t i = 0;
    for(auto& k: qsettings->allKeys())
    {
        if(k != ".")
        {
            m_data[i++].get()->setFileName(qsettings->value(k, "").toString());
            if(i >= m_data.size())
                break;
        }
    }
    qsettings->endGroup();
}

//----------------------------------------------------------------------------------------------------
void Images::save() const
{
    std::map<QString, QString> raw;
    qsettings->beginGroup(QString::fromStdString(std::string(sImages)));

    size_t i = 0;
    for(const auto& item: m_data)
        raw[prefix() + QString::number(i++)] = item->fileName();

    //--удаление
    QStringList sl;
    for(auto& k: qsettings->allKeys())
        if(raw.find(k) == raw.end())
            sl << k;
    for(auto& key: sl)
        qsettings->remove(key);
    //--

    for(auto it = raw.begin(); it != raw.end(); it++)
    {
        QString key = (*it).first.trimmed();
        qsettings->setValue("/" + key, (*it).second);
    }
    qsettings->endGroup();
}


//----------------------------------------------------------------------------------------------------
ImageDsk& Images::at(size_t index) const
{
    if(index >= 0 && index < m_data.size())
        return *m_data[index];
    return *default_image;
}

//----------------------------------------------------------------------------------------------------
QStringList Images::listImageNames() const
{
    QStringList sl;

    for(const auto& item: m_data)
        sl << item->shortFileName();

    return sl;
}

//----------------------------------------------------------------------------------------------------
void Images::loadImage(size_t index)
{
    auto it = std::find_if(data().begin(), data().end(), [&] (const auto& image) {return image->fileName() == data().at(index)->fileName() && !image->needLoad(); });
    if(it != data().end())
        *data().at(index) = *(it->get());
    else
        data().at(index)->load();
}

//----------------------------------------------------------------------------------------------------
Images &Images::operator=(const Images &right) noexcept
{
    if (this == &right)
        return *this;

    for(size_t i = 0; i < m_data.size(); ++i)
    {
        if(right.m_data.size() > i)
            *m_data[i].get() = *right.m_data[i].get();
    }

    return *this;
}

//----------------------------------------------------------------------------------------------------
void Images::fileChanged(const QString &path)
{
    if(QFile(path).exists())
    {
        for(auto& image: m_data)
        {
            if(image->valid() && image->fileName() == path)
                image->setNeedReload(true);
        }
    }
    else
    {
        for(auto& image: m_data)
            if(image->fileName() == path)
                image->detach();
    }
    emit update();
}

//----------------------------------------------------------------------------------------------------
void Images::addFileName(const QString &path)
{
    m_watcher->addPath(path);
}

//----------------------------------------------------------------------------------------------------
void Images::delFileName(const QString &path)
{
    m_watcher->removePath(path);
}

