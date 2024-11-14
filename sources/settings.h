#ifndef SETTINGS
#define SETTINGS

#include <QSettings>
#include <QSerialPort>
#include <memory>


class QMainWindow;


struct COM_settings {
    QString name;
    qint32 baudRate;
    QString stringBaudRate;
    QSerialPort::DataBits dataBits;
    QString stringDataBits;
    QSerialPort::Parity parity;
    QString stringParity;
    QSerialPort::StopBits stopBits;
    QString stringStopBits;
    QSerialPort::FlowControl flowControl;
    QString stringFlowControl;
};



enum class kindset: int {all =0, misc, screen, com_port, environment};

class Settings;

//============================================================================================================================
class Setting
{
    QString title;
    kindset kind;
    QVariant default_value;
    QVariant value;
    bool isChanged = false;

public:
    Setting(QString atitle, kindset akind, QVariant adefault, QVariant avalue, bool aisChanged):
        title(atitle), kind(akind), default_value(adefault), value(avalue), isChanged(aisChanged) {}
    ~Setting() {}

    const QVariant& getValue() const { return (value.isNull() || !value.isValid())? default_value : value; }

protected:
    friend class Settings;
};


typedef std::shared_ptr<Setting> ptrSetting;
typedef std::map<QString, Setting*> mset;

//============================================================================================================================
class Settings {
    static std::shared_ptr<Settings> m_self;
    QMainWindow* owner;
    mutable QSettings qsettings;
    mutable mset mapset_by_kind;

    std::vector<ptrSetting> vec_settings;        
    QVariant default_return;
    std::unique_ptr<COM_settings> com_settings = std::make_unique<COM_settings>();

    void setup_mapset(const kindset ks) const;

protected:
    Settings(QMainWindow* widget_owner, const QString &organization, const QString &application);
public:
    static std::shared_ptr<Settings> instance(QMainWindow* widget_owner, const QString &organization, const QString &application);
    static std::shared_ptr<Settings> instance() {return m_self; }
    ~Settings();

    void loadSettingsByKind(kindset ks);
    void saveSettingsByKind(kindset ks) const;

    void loadSettingsScreen();
    void saveSettingsScreen();

    const QVariant& getSetting(const QString& title, const kindset ks = kindset::all) const;
    bool isChanged(const QString& title, const kindset ks = kindset::all) const;
    void setSetting(const QString& title, QVariant value, const kindset ks = kindset::all);
    void clear(const kindset ks);


    void load(){
        loadSettingsByKind(kindset::misc);
        loadSettingsByKind(kindset::com_port);
        loadSettingsByKind(kindset::environment);
        loadSettingsScreen();
    }

    void save() {
        saveSettingsByKind(kindset::misc);
        saveSettingsByKind(kindset::com_port);
        saveSettingsByKind(kindset::environment);
        saveSettingsScreen();
    }


    const QString& name(const kindset ks) const;
    const mset& map_set(const kindset ks) const;

    const COM_settings& COMSettings() const;
    void setCOMSettings(const COM_settings& com_settings_struct);
    const QSettings& qSettings() const {return qsettings; }
};

#endif // SETTINGS

