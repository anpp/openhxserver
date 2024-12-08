#include "serialport.h"
#include <QCoreApplication>
#include <QElapsedTimer>

#include "../settings.h"

//----------------------------------------------------------------------------------------------------------------------
SerialPortThread::SerialPortThread()
{
    init();
}

//----------------------------------------------------------------------------------------------------------------------
SerialPortThread::~SerialPortThread()
{
    stop();   
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::init()
{
    qRegisterMetaType <QSerialPort::SerialPortError> ();

    serial_port = std::make_unique<QSerialPort>();

    //this->moveToThread(&thread);
    //serial_port->moveToThread(&thread);

    connect(serial_port.get(), &QSerialPort::readyRead, this, &SerialPortThread::s_readyRead);
    connect(serial_port.get(), &QSerialPort::baudRateChanged, this, &SerialPortThread::baudRateChanged);
    connect(serial_port.get(), &QSerialPort::flowControlChanged, this, &SerialPortThread::flowControlChanged);

    start();
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::delay(const unsigned long ms) const
{
    QElapsedTimer timer;
    timer.start();
    while (!timer.hasExpired(ms))
    {
        QThread::msleep(10);
        QCoreApplication::processEvents();
    }
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::setPortSettings()
{    
    //default settings
    ps.baudRate = QSerialPort::Baud9600;
    ps.dataBits = QSerialPort::Data8;
    ps.parity = QSerialPort::NoParity;
    ps.stopBits = QSerialPort::TwoStop;
    ps.flowControl = QSerialPort::NoFlowControl;

    if(Settings::instance())
    {
        const COM_settings &com_settings = Settings::instance()->COMSettings();
        ps.baudRate = com_settings.baudRate;
        ps.dataBits = com_settings.dataBits;
        ps.parity = com_settings.parity;
        ps.stopBits = com_settings.stopBits;
        ps.flowControl = com_settings.flowControl;
    }

    serial_port->setBaudRate(ps.baudRate);
    serial_port->setDataBits(ps.dataBits);
    serial_port->setParity(ps.parity);
    serial_port->setStopBits(ps.stopBits);
    serial_port->setFlowControl(ps.flowControl);
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::sendPacket(const QByteArray& packet, const uint delayms) const
{
    if(delayms > 0)
        delay(delayms);

    int psize = packet.size();
    while (psize > 0)
    {
        if(msDelay > 0)
            QThread::msleep(msDelay);

        sendRatePacket(packet.mid(packet.size() - psize, bytes_to_port));
        QCoreApplication::processEvents();
        psize -= bytes_to_port;
    }
    emit finished();
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::stop()
{
    //thread.quit();
    //thread.wait();
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::start()
{
    //if(!thread.isRunning())
    //    thread.start(QThread::Priority::HighestPriority);
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::close()
{
    stop();
    if(serial_port->isOpen())
        serial_port->close();

    QCoreApplication::processEvents();
    emit closed();
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::sendRatePacket(const QByteArray& rate) const
{
    if(serial_port->write(rate) == -1)
    {
        emit error(QObject::tr("Failed to write the data to port") + ": " + serial_port->errorString());
        //stop();
    }
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::open(const QString& com_port)
{
    serial_port->setPortName(com_port);
    if (!serial_port->open(QIODevice::ReadWrite)){
        emit error(serial_port->errorString() + " " + com_port);
        return;
    }

#if QT_VERSION > QT_VERSION_CHECK(5, 6, 3)
    disconnect(serial_port.get(), &QSerialPort::errorOccurred, this, &SerialPortThread::portError);
    connect(serial_port.get(), &QSerialPort::errorOccurred, this, &SerialPortThread::portError);
#endif

    emit opened();
    emit portBaudRateChanged(serial_port->baudRate());
    flowControlChanged(serial_port->flowControl());
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::portError(QSerialPort::SerialPortError spe)
{
    if(spe != QSerialPort::NoError)
    {
        stop();

        switch(spe)
        {
        case QSerialPort::DeviceNotFoundError:
            emit error(QObject::tr("Device not found"));
            break;

        case QSerialPort::PermissionError:
            emit error(QObject::tr("Permission error"));
            break;

        case QSerialPort::OpenError:
            emit error(QObject::tr("Open error"));
            break;

        case QSerialPort::NotOpenError:
            emit error(QObject::tr("Not open error"));
            break;

        case QSerialPort::ParityError:
            emit error(QObject::tr("Parity error"));
            break;

        case QSerialPort::FramingError:
            emit error(QObject::tr("Framing error"));
            break;

        case QSerialPort::BreakConditionError:
            emit error(QObject::tr("Break condition error"));
            break;

        case QSerialPort::WriteError:
            emit error(QObject::tr("Write error"));
            break;

        case QSerialPort::ReadError:
            emit error(QObject::tr("Read error"));
            break;

        case QSerialPort::ResourceError:
            emit error(QObject::tr("Resource error"));
            break;

        case QSerialPort::UnsupportedOperationError:
            emit error(QObject::tr("Unsupported operation error"));
            break;

        case QSerialPort::TimeoutError:
            emit error(QObject::tr("Timeout error"));
            break;

        case QSerialPort::UnknownError:
            emit error(QObject::tr("Unknown error"));
            break;

        default:
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::baudRateChanged(quint32 baudRate, QSerialPort::Directions directions)
{
    Q_UNUSED(directions);
    if(serial_port->isOpen())
        emit portBaudRateChanged(baudRate);
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::flowControlChanged(QSerialPort::FlowControl flowControl)
{
    if(serial_port->isOpen())
    {
        switch(flowControl)
        {
        case QSerialPort::NoFlowControl:
            emit portFlowControlChanged("None");
            break;
        case QSerialPort::HardwareControl:
            emit portFlowControlChanged("RTS/CTS");
            break;
        case QSerialPort::SoftwareControl:
            emit portFlowControlChanged("XON/XOFF");
            break;
        default:
            emit portFlowControlChanged("");
            break;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::s_readyRead()
{
    emit readyData(this->serial_port->readAll());
}



