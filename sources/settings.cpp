#include "settings.h"
#include <QMainWindow>
#include <QDir>


const static QString sSettingKind[] = {"", "misc", "screen", "com_port", "environment"};

std::shared_ptr<Settings> Settings::m_self = nullptr;

//----------------------------------------------------------------------------------------------------------------------
Settings::Settings(QMainWindow* widget_owner, const QString& organization, const QString& application) :
    owner(widget_owner), qsettings(organization, application), default_return(false)
{
    vec_settings = {
        std::make_shared<Setting>("loader", kindset::misc, "", QVariant(QVariant::String), false),

        std::make_shared<Setting>("geometry", kindset::screen, 0, QVariant(QVariant::Int), false),
        std::make_shared<Setting>("state", kindset::screen, 0, QVariant(QVariant::Int), false),
        //COM Port
        std::make_shared<Setting>("name", kindset::com_port, "", QVariant(QVariant::String), false),
        std::make_shared<Setting>("baudRate", kindset::com_port, QSerialPort::Baud9600, QVariant(QVariant::Int), false),
        std::make_shared<Setting>("dataBits", kindset::com_port, QSerialPort::Data8, QVariant(QVariant::Int), false),
        std::make_shared<Setting>("parity", kindset::com_port, QSerialPort::NoParity, QVariant(QVariant::Int), false),
        std::make_shared<Setting>("stopBits", kindset::com_port, QSerialPort::TwoStop, QVariant(QVariant::Int), false),
        std::make_shared<Setting>("flowControl", kindset::com_port, QSerialPort::NoFlowControl, QVariant(QVariant::Int), false),
        //Environment
        std::make_shared<Setting>("directory_bin", kindset::environment, QDir::homePath(), QVariant(QVariant::String), false),
        std::make_shared<Setting>("directory_dsk", kindset::environment, QDir::homePath(), QVariant(QVariant::String), false),
    };
}

//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<Settings> Settings::instance(QMainWindow *widget_owner, const QString &organization, const QString &application)
{
    if(!m_self)
        m_self = std::shared_ptr<Settings>(new Settings(widget_owner, organization, application));
    return m_self;
}


//----------------------------------------------------------------------------------------------------------------------
Settings::~Settings()
{
}



//----------------------------------------------------------------------------------------------------------------------
void Settings::loadSettingsByKind(kindset ks)
{        
    setup_mapset(ks);

    qsettings.beginGroup("/" + name(ks));

    for(auto& key: qsettings.allKeys())
        if(key != "." && key != "")
        {
            auto s = mapset_by_kind.find(key);
            if(s != mapset_by_kind.end())
            {
                Setting *setting = s->second;
                setting->value = qsettings.value("/" + key, setting->getValue());
            }
            else //если настройки нет реестре, загружается настройка по-умолчанию
                setSetting(key, qsettings.value("/" + key, ""), ks);
        }
    qsettings.endGroup();
}


//----------------------------------------------------------------------------------------------------------------------
void Settings::saveSettingsByKind(kindset ks) const
{    
    setup_mapset(ks);

    qsettings.beginGroup("/" + name(ks));

    //--удаление
    QStringList sl;
    for(auto& k: qsettings.allKeys())
        if(mapset_by_kind.find(k) == mapset_by_kind.end())
            sl << k;
    for(auto& key: sl)
        qsettings.remove(key);
    //--

    for(const auto& item: mapset_by_kind)
        qsettings.setValue("/" + item.second->title, item.second->getValue());

    qsettings.endGroup();
}



//----------------------------------------------------------------------------------------------------------------------
void Settings::loadSettingsScreen()
{    
    if(owner != nullptr)
    {
        loadSettingsByKind(kindset::screen);
        owner->restoreGeometry(getSetting("geometry").toByteArray());
        owner->restoreState(getSetting("state").toByteArray());
    }
}


//----------------------------------------------------------------------------------------------------------------------
void Settings::saveSettingsScreen()
{
    if(owner != nullptr)
    {
        setSetting("geometry", owner->saveGeometry());
        setSetting("state", owner->saveState());
        saveSettingsByKind(kindset::screen);
    }
}


//----------------------------------------------------------------------------------------------------------------------
const QVariant& Settings::getSetting(const QString& title, const kindset ks) const
{    
    const auto& item = map_set(ks).find(title);
    if(item != mapset_by_kind.end())
        return item->second->getValue();

    return default_return;
}


//----------------------------------------------------------------------------------------------------------------------
bool Settings::isChanged(const QString &title, const kindset ks) const
{
    const auto& item = map_set(ks).find(title);
    if(item != mapset_by_kind.end())
        return item->second->isChanged;
    return false;
}


//----------------------------------------------------------------------------------------------------------------------
void Settings::setSetting(const QString& title, QVariant value, const kindset ks)
{    
    auto item = map_set(ks).find(title);
    if(item != mapset_by_kind.end())
    {
        item->second->isChanged = (item->second->value != value);
        item->second->value = value;
    }
    else
        if(ks != kindset::all)
            vec_settings.emplace_back(std::make_shared<Setting>(title, ks, value, value, false));
}


//----------------------------------------------------------------------------------------------------------------------
void Settings::clear(const kindset ks)
{
    auto new_end = std::remove_if(vec_settings.begin(), vec_settings.end(), [&] (const auto& item) {return item->kind == ks; });
    vec_settings.erase(new_end, vec_settings.end());
}


//----------------------------------------------------------------------------------------------------------------------
const QString& Settings::name(const kindset ks) const
{
    return sSettingKind[static_cast<int>(ks)];
}


//----------------------------------------------------------------------------------------------------------------------
const mset &Settings::map_set(const kindset ks) const
{
    setup_mapset(ks);
    return mapset_by_kind;
}



//----------------------------------------------------------------------------------------------------------------------
const COM_settings &Settings::COMSettings() const
{
    com_settings->name = getSetting("name", kindset::com_port).toString();
    com_settings->baudRate = getSetting("baudRate", kindset::com_port).toInt();
    com_settings->dataBits = static_cast<QSerialPort::DataBits>(getSetting("dataBits", kindset::com_port).toInt());
    com_settings->parity = static_cast<QSerialPort::Parity>(getSetting("parity", kindset::com_port).toInt());
    com_settings->stopBits = static_cast<QSerialPort::StopBits>(getSetting("stopBits", kindset::com_port).toInt());
    com_settings->flowControl = static_cast<QSerialPort::FlowControl>(getSetting("flowControl", kindset::com_port).toInt());

    com_settings->stringBaudRate = QString::number(com_settings->baudRate);
    com_settings->stringDataBits = QString::number(com_settings->dataBits);

    switch(com_settings->parity)
    {
    case QSerialPort::NoParity:
        com_settings->stringParity = "None";
        break;
    case QSerialPort::QSerialPort::EvenParity:
        com_settings->stringParity = "Even";
        break;
    case QSerialPort::OddParity:
        com_settings->stringParity = "Odd";
        break;
    case QSerialPort::MarkParity:
        com_settings->stringParity = "Mark";
        break;
    case QSerialPort::SpaceParity:
        com_settings->stringParity = "Space";
        break;
    default:
        break;
    }

    switch(com_settings->stopBits)
    {
    case QSerialPort::OneStop:
        com_settings->stringStopBits = "1";
        break;
    #ifdef Q_OS_WIN
    case QSerialPort::OneAndHalfStop:
        com_settings->stringStopBits = "1.5";
        break;
    #endif
    case QSerialPort::TwoStop:
        com_settings->stringStopBits = "2";
        break;
    default:
        break;
    }

    switch(com_settings->flowControl)
    {
    case QSerialPort::NoFlowControl:
        com_settings->stringFlowControl = "None";
        break;
    case QSerialPort::HardwareControl:
        com_settings->stringFlowControl = "RTS/CTS";
        break;
    case QSerialPort::SoftwareControl:
        com_settings->stringFlowControl = "XON/XOFF";
        break;
    default:
        break;
    }

    return *com_settings;
}


//----------------------------------------------------------------------------------------------------------------------
void Settings::setCOMSettings(const COM_settings &com_settings_struct)
{
    setSetting("name", com_settings_struct.name, kindset::com_port);
    setSetting("baudRate", com_settings_struct.baudRate, kindset::com_port);
    setSetting("dataBits", com_settings_struct.dataBits, kindset::com_port);
    setSetting("parity", com_settings_struct.parity, kindset::com_port);
    setSetting("stopBits", com_settings_struct.stopBits, kindset::com_port);
    setSetting("flowControl", com_settings_struct.flowControl, kindset::com_port);
}


//----------------------------------------------------------------------------------------------------------------------
void Settings::setup_mapset(const kindset ks) const
{    
    mapset_by_kind.clear();

    for(const auto& setting: vec_settings)
        if(ks == setting->kind || ks == kindset::all)
            mapset_by_kind[setting.get()->title] = setting.get();
}




