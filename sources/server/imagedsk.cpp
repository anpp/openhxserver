
#include "imagedsk.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>

//-------------------------------------------------------------------------------------------------------
ImageDsk::ImageDsk(const QString& filename)
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

    m_filename = filename;
    if(m_filename.isEmpty() || "" == m_filename)
    {
        m_shortfilename = "";
        m_file.reset();
        return;
    }
    m_shortfilename = "<not exists>";
    if(QFile(m_filename).exists())
    {
        m_file = std::make_unique<QFile>(m_filename);
        m_shortfilename = QFileInfo(*m_file).fileName();
    }
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
    //if(loaded()) return true;

    m_blocks.clear();

    if(m_file->open(QIODevice::ReadWrite | QFile::ExistingOnly) && m_file->size() >= DskConsts::BLOCK_SIZE)
    {
        std::shared_ptr<QByteArray> first_block = std::make_shared<QByteArray>();
        *first_block = m_file->read(DskConsts::BLOCK_SIZE);
        if(first_block->size() == DskConsts::BLOCK_SIZE)
        {
            if(static_cast<uint8_t>(first_block->at(DskConsts::BLOCK_OFFSET_0)) == 0xA0 && static_cast<uint8_t>(first_block->at(DskConsts::BLOCK_OFFSET_0 + 1)) == 0)
                m_start_offset = DskConsts::BLOCK_OFFSET_0;
            if(static_cast<uint8_t>(first_block->at(DskConsts::BLOCK_OFFSET_128)) == 0xA0 && static_cast<uint8_t>(first_block->at(DskConsts::BLOCK_OFFSET_128 + 1)) == 0)
                m_start_offset = DskConsts::BLOCK_OFFSET_128;
            if(static_cast<uint8_t>(first_block->at(DskConsts::BLOCK_OFFSET_256)) == 0xA0 && static_cast<uint8_t>(first_block->at(DskConsts::BLOCK_OFFSET_256 + 1)) == 0)
                m_start_offset = DskConsts::BLOCK_OFFSET_256;
        }

        size_t num_blocks = m_file->size() / DskConsts::BLOCK_SIZE;
        size_t next_block = 0;

        if(m_start_offset == 0)
        {
            next_block = 1;
            m_blocks.push_back(first_block);
        }
        else
            m_file->seek(m_start_offset);

        for(size_t i = next_block; i < num_blocks; ++i)
            m_blocks.push_back(std::make_shared<QByteArray>(m_file->read(DskConsts::BLOCK_SIZE)));

        m_file->close();
        return true;
    }
    return false;
}

//-------------------------------------------------------------------------------------------------------
void ImageDsk::release()
{
    if(m_file)
        m_file->close();
}

//-------------------------------------------------------------------------------------------------------
QByteArray &ImageDsk::blockAt(size_t index)
{
    if(index < 0 || index >= size() || !loaded())
        return m_zero_array;
    return *m_blocks.at(index);
}

//-------------------------------------------------------------------------------------------------------
bool ImageDsk::loaded() const
{
    return size() > 0;
}

//-------------------------------------------------------------------------------------------------------
ImageDsk::DskErrors ImageDsk::write(size_t block, const QByteArray &data)
{
    if(block >= size())
        return DskErrors::DEEOF;
    if(data.size() <= 0)
        return DskErrors::DEError;

    unsigned short rest = data.size() % ImageDsk::DskConsts::BLOCK_SIZE;
    int n_blocks = data.size() / ImageDsk::DskConsts::BLOCK_SIZE;

    for(int i = 0; i < n_blocks; ++i)
        *m_blocks.at(i + block) = data.mid(i * ImageDsk::DskConsts::BLOCK_SIZE, ImageDsk::DskConsts::BLOCK_SIZE);
    if(rest)
        *m_blocks.at(block + n_blocks) = data.mid(n_blocks * ImageDsk::DskConsts::BLOCK_SIZE, rest) + m_blocks.at(block + n_blocks)->mid(rest);
    return DskErrors::DESuccess;
}

//-------------------------------------------------------------------------------------------------------
ImageDsk &ImageDsk::operator=(const ImageDsk &right) noexcept
{
    if (this == &right)
        return *this;

    setFileName(right.m_filename);
    return *this;
}

