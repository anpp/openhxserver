
#ifndef IMAGEDSK_H
#define IMAGEDSK_H

#include <QString>
#include <memory>
#include <vector>

class QFile;

class ImageDsk
{
    std::unique_ptr<QFile> m_file;
    QString m_filename = "";
    QString m_shortfilename = "";
    std::vector<std::shared_ptr<QByteArray>> m_blocks;
    size_t m_start_offset = 0;
    QByteArray m_zero_array;
    std::shared_ptr<QByteArray> m_first_block;
    bool m_need_reload = false;

public:
    enum DskConsts: int {BLOCK_SIZE = 512, BLOCK_OFFSET_0 = 0, BLOCK_OFFSET_128 = 128, BLOCK_OFFSET_256 = 256};
    enum class DskErrors: int {DESuccess = 0, DEEOF, DEError, DEFileError};

    explicit ImageDsk(const QString& filename = "");
    ~ImageDsk();

    void setFileName(const QString& filename);
    const QString& fileName() const { return m_filename; }
    const QString& shortFileName() const { return m_shortfilename; }
    bool valid() const;
    bool empty() const { return m_shortfilename.isEmpty(); }
    bool load();
    void release();
    bool openFile();
    size_t size() const { return m_blocks.size(); }
    QByteArray& blockAt(size_t index);
    bool loaded() const;
    DskErrors write(size_t block, const QByteArray& data);
    bool needLoad() const { return m_need_reload || !loaded(); }

    ImageDsk& operator=(const ImageDsk& right) noexcept;
};

#endif // IMAGEDSK_H
