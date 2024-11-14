#include "settings_com_port.h"
#include <QIntValidator>
#include <QSerialPort>
#include <QLineEdit>
#include <QSerialPortInfo>

#include "../settings.h"

//--------------------------------------------------------------------------------------------------------------------------------
SettingsCOMPort::SettingsCOMPort(QWidget *parent) : QWidget(parent),
                                                    m_intValidator(new QIntValidator(0, 4000000, this))
{
    setupUi(this);        

    baudRateBox->setInsertPolicy(QComboBox::NoInsert);
    comPortNameBox->setInsertPolicy(QComboBox::NoInsert);

    connect(baudRateBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsCOMPort::checkCustomBaudRatePolicy);

    fillPortsParameters();
}


//--------------------------------------------------------------------------------------------------------------------------------
void SettingsCOMPort::setCurrentSettings(const COM_settings &current_settings)
{
    comPortNameBox->setCurrentText(current_settings.name);
    baudRateBox->setCurrentText(current_settings.stringBaudRate);
    dataBitsBox->setCurrentText(current_settings.stringDataBits);
    parityBox->setCurrentText(current_settings.stringParity);
    stopBitsBox->setCurrentText(current_settings.stringStopBits);
    flowControlBox->setCurrentText(current_settings.stringFlowControl);

    updateSettings();
}

//--------------------------------------------------------------------------------------------------------------------------------
const COM_settings &SettingsCOMPort::actualSettings()
{
    updateSettings();
    return *m_com_settings;
}

//--------------------------------------------------------------------------------------------------------------------------------
void SettingsCOMPort::load()
{
    if(Settings::instance())
        setCurrentSettings(Settings::instance()->COMSettings());
}

//--------------------------------------------------------------------------------------------------------------------------------
void SettingsCOMPort::save()
{
    if(Settings::instance())
    {
        Settings::instance()->setCOMSettings(actualSettings());
        Settings::instance()->saveSettingsByKind(kindset::com_port);
    }
}


//--------------------------------------------------------------------------------------------------------------------------------
void SettingsCOMPort::updateSettings()
{   
    m_com_settings->name = comPortNameBox->currentText();

    if (baudRateBox->currentIndex() == 4) {
        m_com_settings->baudRate = baudRateBox->currentText().toInt();
    } else {
        m_com_settings->baudRate = static_cast<QSerialPort::BaudRate>(
                    baudRateBox->itemData(baudRateBox->currentIndex()).toInt());
    }
    m_com_settings->stringBaudRate = QString::number(m_com_settings->baudRate);

    m_com_settings->dataBits = static_cast<QSerialPort::DataBits>(
                dataBitsBox->itemData(dataBitsBox->currentIndex()).toInt());
    m_com_settings->stringDataBits = dataBitsBox->currentText();

    m_com_settings->parity = static_cast<QSerialPort::Parity>(
                parityBox->itemData(parityBox->currentIndex()).toInt());
    m_com_settings->stringParity = parityBox->currentText();

    m_com_settings->stopBits = static_cast<QSerialPort::StopBits>(
                stopBitsBox->itemData(stopBitsBox->currentIndex()).toInt());
    m_com_settings->stringStopBits = stopBitsBox->currentText();

    m_com_settings->flowControl = static_cast<QSerialPort::FlowControl>(
                flowControlBox->itemData(flowControlBox->currentIndex()).toInt());
    m_com_settings->stringFlowControl = flowControlBox->currentText();
}


//--------------------------------------------------------------------------------------------------------------------------------
void SettingsCOMPort::checkCustomBaudRatePolicy(int idx)
{
    const bool isCustomBaudRate = !baudRateBox->itemData(idx).isValid();
    baudRateBox->setEditable(isCustomBaudRate);
    if (isCustomBaudRate) {
        baudRateBox->clearEditText();
        QLineEdit *edit = baudRateBox->lineEdit();
        edit->setValidator(m_intValidator);
    }
}



//--------------------------------------------------------------------------------------------------------------------------------
void SettingsCOMPort::fillPortsParameters()
{
    auto availablePorts = QSerialPortInfo::availablePorts();
    for(const auto &port: availablePorts)
        comPortNameBox->addItem(port.portName());

    baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    baudRateBox->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    baudRateBox->addItem(tr("Custom"));

    dataBitsBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    dataBitsBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    dataBitsBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    dataBitsBox->addItem(QStringLiteral("8"), QSerialPort::Data8);    

    parityBox->addItem(tr("None"), QSerialPort::NoParity);
    parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    stopBitsBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    stopBitsBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    stopBitsBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    flowControlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    flowControlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    flowControlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}
