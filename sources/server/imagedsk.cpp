
#include "imagedsk.h"

#include <QFile>
#include <QFileInfo>

#include <QDebug>

//-------------------------------------------------------------------------------------------------------
ImageDsk::ImageDsk(const QString& filename, QObject *parent)
    : QObject{parent}
{
    m_zero_array.reserve(DskConsts::BLOCK_SIZE);
    setFileName(filename);
}

//-------------------------------------------------------------------------------------------------------
ImageDsk::~ImageDsk()
{
    release();
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::setFileName(const QString &filename)
{
    if(m_filename == filename) return;
    release();

    detach();
    if(filename.isEmpty() || "" == filename)
    {
        m_shortfilename = "";
        m_filename = "";
        return;
    }
    m_filename = filename;
    attach();
}

//-------------------------------------------------------------------------------------------------------
bool ImageDsk::valid() const
{
    if(m_file)
        return true;
    return false;
}

//-------------------------------------------------------------------------------------------------------
bool ImageDsk::load()
{
    if(m_file->isOpen()) return true;

    if(!needLoad()) return true;

    m_blocks.clear();

    if(openFile())
    {
        size_t num_blocks = m_file->size() / DskConsts::BLOCK_SIZE;
        size_t next_block = 0;

        if(m_start_offset == 0)
        {
            next_block = 1;
            m_blocks.push_back(std::move(m_first_block));
        }
        else
            m_file->seek(m_start_offset);

        for(size_t i = next_block; i < num_blocks; ++i)
            m_blocks.push_back(std::make_shared<QByteArray>(m_file->read(DskConsts::BLOCK_SIZE)));

        m_file->close();
        m_need_reload = false;
        emit update();
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::release()
{
    if(m_file && m_file->isOpen())
        m_file->close();
    m_blocks.clear();
    m_need_reload = false;
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::attach()
{
    if(QFile(m_filename).exists())
    {
        m_file = std::make_unique<QFile>(m_filename);
        connect(m_file.get(), &QFile::aboutToClose, this, &ImageDsk::fileClosed);
        m_shortfilename = QFileInfo(*m_file).fileName();
        emit addFileName(m_filename);
    }
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::detach()
{
    m_shortfilename = "<ejected>";
    m_file.reset();
    if(!m_filename.isEmpty())
        emit delFileName(m_filename);
}

//-------------------------------------------------------------------------------------------------------
bool ImageDsk::openFile()
{
    if(m_file->isOpen()) return true;

    if(m_file->open(QIODevice::ReadWrite | QFile::ExistingOnly) && m_file->size() >= DskConsts::BLOCK_SIZE)
    {
        emit delFileName(m_filename); //открытый файл не отслеживать на изменения

        m_first_block.reset();
        m_first_block = std::make_shared<QByteArray>();
        *m_first_block = m_file->read(DskConsts::BLOCK_SIZE);
        if(m_first_block->size() == DskConsts::BLOCK_SIZE)
        {
            if(static_cast<uint8_t>(m_first_block->at(DskConsts::BLOCK_OFFSET_0)) == 0xA0 && static_cast<uint8_t>(m_first_block->at(DskConsts::BLOCK_OFFSET_0 + 1)) == 0)
                m_start_offset = DskConsts::BLOCK_OFFSET_0;
            if(static_cast<uint8_t>(m_first_block->at(DskConsts::BLOCK_OFFSET_128)) == 0xA0 && static_cast<uint8_t>(m_first_block->at(DskConsts::BLOCK_OFFSET_128 + 1)) == 0)
                m_start_offset = DskConsts::BLOCK_OFFSET_128;
            if(static_cast<uint8_t>(m_first_block->at(DskConsts::BLOCK_OFFSET_256)) == 0xA0 && static_cast<uint8_t>(m_first_block->at(DskConsts::BLOCK_OFFSET_256 + 1)) == 0)
                m_start_offset = DskConsts::BLOCK_OFFSET_256;
        }
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------------
const QByteArray &ImageDsk::blockAt(size_t index) const
{
    if(index < 0 || index >= size() || !loaded())
        return m_zero_array;
    return *m_blocks.at(index);
}

//-------------------------------------------------------------------------------------------------------
bool ImageDsk::loaded() const
{
    return (size() > 0);
}

//-------------------------------------------------------------------------------------------------------
ImageDsk::DskErrors ImageDsk::write(size_t block, const QByteArray &data)
{
    if(block >= size())
        return DskErrors::DEEOF;
    if(data.size() <= 0)
        return DskErrors::DEError;

    if(openFile())
    {
        unsigned short rest = data.size() % ImageDsk::DskConsts::BLOCK_SIZE;
        int n_blocks = data.size() / ImageDsk::DskConsts::BLOCK_SIZE;

        if(!m_file->seek(m_start_offset + block * DskConsts::BLOCK_SIZE))
        {
            m_file->close();
            return DskErrors::DEFileError;
        }

        m_file->write(data);

        for(int i = 0; i < n_blocks; ++i)
            *m_blocks.at(i + block) = data.mid(i * ImageDsk::DskConsts::BLOCK_SIZE, ImageDsk::DskConsts::BLOCK_SIZE);            
        if(rest)
            *m_blocks.at(block + n_blocks) = data.mid(n_blocks * ImageDsk::DskConsts::BLOCK_SIZE, rest) + m_blocks.at(block + n_blocks)->mid(rest);

        m_file->close();
        return DskErrors::DESuccess;
    }
    else
        return DskErrors::DEFileError;
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::setNeedReload(bool value)
{
    if(loaded())
        m_need_reload = value;
    emit update();
}

//-------------------------------------------------------------------------------------------------------
ImageDsk &ImageDsk::operator=(const ImageDsk &right) noexcept
{
    if (this == &right)
        return *this;

    setFileName(right.m_filename);
    m_blocks = right.m_blocks;
    m_need_reload = right.m_need_reload;
    emit update();
    return *this;
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::fileChanged(const QString &path)
{
    Q_UNUSED(path);
    m_need_reload = true;
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::fileClosed()
{
    emit addFileName(m_filename); //закрытый файл отслеживается
}

