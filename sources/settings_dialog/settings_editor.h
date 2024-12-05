#ifndef SETTINGSEDITOR_H
#define SETTINGSEDITOR_H

#include <QDialog>
#include <QObject>
#include <QWidget>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <memory>


class SettingsCOMPort;
class SettingsImages;
class Images;

class SettingsEditor : public QDialog
{
    Q_OBJECT   
    enum SettingsEditor_defs: int {default_width = 500, default_height = 450, margin = 4};

    void setup();
    void applySettings();

    QDialogButtonBox buttonBox;
    QTabWidget tw;

    std::unique_ptr<SettingsImages> images_widget;
    std::unique_ptr<SettingsCOMPort> com_port_widget;

public:
    explicit SettingsEditor(Images& images_data, QWidget *parent = nullptr);
    ~SettingsEditor();

signals:
    void apply();

private slots:
    void accept_slot();
    void clicked_slot(QAbstractButton*);
};

#endif // SETTINGSEDITOR_H
