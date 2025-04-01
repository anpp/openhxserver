#ifndef SETTINGS_IMAGES_H
#define SETTINGS_IMAGES_H

#include <QWidget>
#include <memory>

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
    void setSAVFile(const QString& value);
    const QString& loader() const { return m_loader; };
    const QString& SAV() const { return m_SAVFile; };
    bool HXMode() const;
    void setSaveImmediate(bool a_save_immediate);
    bool saveImmediate() const {return m_save_immediate; };
    void update();
    void save();
    void saveLoaders() const;
    void saveImages() const;
    void saveHXMode(bool value) const;

private:
    Ui::SettingsImages *ui;

    std::unique_ptr<ImagesModel> m_model;
    std::unique_ptr<ImagesDelegate> m_delegate;
    QString m_loader;
    QString m_SAVFile;
    bool m_save_immediate = false;
    QString last_image_file;

    QAction* m_openLoaderAction = nullptr;
    QAction* m_openSAVAction = nullptr;

signals:
    void updateHX() const;

public slots:
    void openFileBin();
    void openFileSAV();
    void selected_image_file(const QString& filename);
    void updateWidget() const;
    void rbModeClicked(bool checked);
};

#endif // SETTINGS_IMAGES_H
