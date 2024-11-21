
#ifndef IMAGES_H
#define IMAGES_H


#include <QObject>
#include <vector>
#include <memory>

class ImageDsk;
class QSettings;

typedef std::vector<std::unique_ptr<ImageDsk>> t_vectorDsk;

class Images : public QObject
{
    Q_OBJECT

    enum Images_defs: int {Size = 8};

    std::unique_ptr<ImageDsk> default_image;
    QString HXPrefix = "HX";
public:
    explicit Images(QObject *parent = nullptr);
    ~Images();

    void load();
    void save() const;

    const t_vectorDsk& data() const { return m_data; }
    size_t size() const { return m_data.size(); }
    ImageDsk& at(size_t index) const;
    const QString& prefix() const { return HXPrefix; }
    QStringList listImageNames() const;
    void loadImage(size_t index);

    Images& operator=(const Images& right) noexcept;
private:
    t_vectorDsk m_data;

    std::unique_ptr<QSettings> qsettings;


signals:

};

#endif // IMAGES_H
