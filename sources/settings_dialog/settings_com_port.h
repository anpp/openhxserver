#ifndef SETTINGSCOMPORT_H
#define SETTINGSCOMPORT_H

#include <QWidget>

#include "ui_settings_com_port.h"
#include "../settings.h"



class SettingsCOMPort : public QWidget,
                        protected Ui::COMPortWidget
{
    Q_OBJECT

    void fillPortsParameters();

     QIntValidator *m_intValidator = nullptr;

     std::unique_ptr<COM_settings> m_com_settings = std::make_unique<COM_settings>();;

     void updateSettings();
public:
    explicit SettingsCOMPort(QWidget *parent = nullptr);

    void setCurrentSettings(const COM_settings& current_settings);
    const COM_settings& actualSettings();
    void load();
    void save();

signals:

private slots:
     void checkCustomBaudRatePolicy(int idx);
};

#endif // SETTINGSCOMPORT_H
