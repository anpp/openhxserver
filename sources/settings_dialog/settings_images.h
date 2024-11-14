#ifndef SETTINGS_IMAGES_H
#define SETTINGS_IMAGES_H

#include <QWidget>

namespace Ui {
class SettingsImages;
}

class ImagesModel;
class ImagesDelegate;
class Images;

class SettingsImages : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsImages(Images& images_data, QWidget *parent = nullptr);
    ~SettingsImages();

    void setLoader(const QString& value);
    const QString& loader() const { return m_loader; };
    void setSaveImmediate(bool a_save_immediate);
    bool saveImmediate() const {return m_save_immediate; };
    void update();
    void save();
    void saveLoader();
    void saveImages();

private:
    Ui::SettingsImages *ui;

    std::unique_ptr<ImagesModel> m_model;
    std::unique_ptr<ImagesDelegate> m_delegate;
    QString m_loader;
    bool m_save_immediate = false;
    QString last_image_file;

    QAction* m_openLoaderAction = nullptr;

signals:
    void updateHX();

public slots:
    void openFileBin();
    void selected_image_file(const QString& filename);
};

#endif // SETTINGS_IMAGES_H
