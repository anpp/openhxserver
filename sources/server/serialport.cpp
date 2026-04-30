#include "serialport.h"
#include <QCoreApplication>
#include <QElapsedTimer>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <qnativeinterface.h>
#endif

#include "../settings.h"


std::shared_ptr<SerialPortThread> SerialPortThread::m_self = nullptr;

//----------------------------------------------------------------------------------------------------------------------
SerialPortThread::SerialPortThread()
{
    init();
}

//----------------------------------------------------------------------------------------------------------------------
SerialPortThread::~SerialPortThread()
{
    stop();
    m_self = nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
std::shared_ptr<SerialPortThread> SerialPortThread::instance()
{
    if(!m_self)
        m_self = std::shared_ptr<SerialPortThread>(new SerialPortThread());
    return m_self;

}


//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::init()
{
    qRegisterMetaType <QSerialPort::SerialPortError> ();

#ifndef Q_OS_ANDROID
    serial_port = std::make_unique<QSerialPort>();

    //this->moveToThread(&thread);
    //serial_port->moveToThread(&thread);

    connect(serial_port.get(), &QSerialPort::readyRead, this, &SerialPortThread::s_readyRead);
    connect(serial_port.get(), &QSerialPort::baudRateChanged, this, &SerialPortThread::baudRateChanged);
    connect(serial_port.get(), &QSerialPort::flowControlChanged, this, &SerialPortThread::flowControlChanged);
#endif
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

#ifdef Q_OS_ANDROID
//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::javaSetPortSettings(int baud, int data, int stop, QSerialPort::FlowControl flow)
{
    JavaFlowControl flowControl = JavaFlowControl::None;
    if(flow == QSerialPort::HardwareControl)
        flowControl = JavaFlowControl::RtsCts;
    if(flow == QSerialPort::SoftwareControl)
        flowControl = JavaFlowControl::XonXoff;

    // Вызываем статический метод Java
    // Сигнатура "(IIII)V" означает: 4 аргумента типа int (I), возвращает void (V)
    QJniObject::callStaticMethod<void>(
        "hx/openhx/helper/SerialHelper",
        "setParametersPort",             // Имя метода
        "(IIII)V",                       // Сигнатура
        baud, data, stop, static_cast<int>(flowControl)           // Аргументы
        );

    QJniEnvironment env;
    if (env->ExceptionCheck()) {
        emit error(QObject::tr("Java: Exception occurred in setParametersPort!") + " " + serial_port->portName());
        env->ExceptionDescribe(); // Выведет детали ошибки в Logcat
        env->ExceptionClear();
    }
}
#endif

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

#ifdef Q_OS_ANDROID
    javaSetPortSettings(ps.baudRate, ps.dataBits, ps.stopBits, ps.flowControl);
#else
    serial_port->setBaudRate(ps.baudRate);
    serial_port->setDataBits(ps.dataBits);
    serial_port->setParity(ps.parity);
    serial_port->setStopBits(ps.stopBits);
    serial_port->setFlowControl(ps.flowControl);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::sendPacket(const QByteArray& packet, const uint delayms) const
{
    if(delayms > 0)
        delay(delayms);

    int psize = packet.size();
    while (psize > 0)
    {
        if(m_msDelay > 0)
            QThread::msleep(m_msDelay);

        sendRatePacket(packet.mid(packet.size() - psize, m_bytes_to_port));
        QCoreApplication::processEvents();
        psize -= m_bytes_to_port;
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
bool SerialPortThread::isOpen()
{
    bool result = false;
    if(serial_port)
        result = serial_port->isOpen();
    return result;
}


//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::close()
{
    stop();
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>("hx/openhx/helper/SerialHelper", "closeDeviceConnection");
    //emit closed(); от java будет в connectionChanged
#else
    if(serial_port->isOpen())
        serial_port->close();
    QCoreApplication::processEvents();
    emit closed();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::sendRatePacket(const QByteArray& rate) const
{
#ifdef Q_OS_ANDROID
    QJniEnvironment env;
    jbyteArray jData = env->NewByteArray(rate.size());
    env->SetByteArrayRegion(jData, 0, rate.size(), reinterpret_cast<const jbyte*>(rate.constData()));
    QJniObject::callStaticMethod<void>(
        "hx/openhx/helper/SerialHelper",
        "writeData",
        "([B)V",
        jData
        );

    env->DeleteLocalRef(jData);

    if (env->ExceptionCheck()) {
        emit error(QObject::tr("Java: Exception occurred in writeData!") + " " + serial_port->portName());
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

#else
    if(serial_port->write(rate) == -1)
    {
        emit error(QObject::tr("Failed to write the data to port") + ": " + serial_port->errorString());
        //stop();
    }
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::open(const QString& com_port)
{
#ifdef Q_OS_ANDROID
    QStringList parts = com_port.split(" "); // "USB Device 0403:6001"
    if (parts.size() >= 3)
    {
        QStringList ids = parts.last().split(":");
        bool okV, okP;
        int vid = ids[0].toInt(&okV, 16);
        int pid = ids[1].toInt(&okP, 16);

        if (okV && okP)
        {
            QJniObject context = QNativeInterface::QAndroidApplication::context();
            QJniObject::callStaticMethod<void>(
                "hx/openhx/helper/SerialHelper",
                "connectToDevice",
                "(Landroid/content/Context;II)V",
                context.object(),
                static_cast<jint>(vid),
                static_cast<jint>(pid)
                );

            QJniEnvironment env;
            if (env->ExceptionCheck()) {
                emit error(tr("Java: Failed to connect to device %1:%2").arg(vid).arg(pid));
                env->ExceptionDescribe();
                env->ExceptionClear();
            } else {
                // Лучше подтверждать "emit opened()" только Java пришлет колбэк об успешном подключении.
            }
        }
    }
#else
    serial_port->setPortName(com_port);
    if (!serial_port->open(QIODevice::ReadWrite)){
        portError(serial_port->error());
        return;
    }

  #if QT_VERSION > QT_VERSION_CHECK(5, 6, 3)
    disconnect(serial_port.get(), &QSerialPort::errorOccurred, this, &SerialPortThread::portError);
    connect(serial_port.get(), &QSerialPort::errorOccurred, this, &SerialPortThread::portError);
  #endif
    emit opened();
    emit portBaudRateChanged(serial_port->baudRate());
    flowControlChanged(serial_port->flowControl());
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void SerialPortThread::portError(QSerialPort::SerialPortError spe)
{
    if(spe != QSerialPort::NoError)
    {
        QString hint_value = "";
        stop();
#ifdef Q_OS_LINUX
        if(QSerialPort::PermissionError == spe)
            hint_value = "Hint: run the command 'sudo chown " + qgetenv("USER") + " /dev/" + serial_port->portName() + "'";
#endif
        emit error(serial_port->errorString() + " " + serial_port->portName());

        if(!hint_value.isEmpty())
            emit hint(hint_value);
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


#ifdef Q_OS_ANDROID
extern "C" {
// ВАЖНО: Имя должно в точности совпадать с пакетом и классом в Java
//----------------------------------------------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_hx_openhx_helper_SerialHelper_javaResponseReady(JNIEnv *env, jclass clazz, jbyteArray response)
{
    Q_UNUSED(env);
    Q_UNUSED(clazz);

    if (SerialPortThread::instance())
    {
        jbyte *bytes = env->GetByteArrayElements(response, nullptr);
        jsize len = env->GetArrayLength(response);

        if (len > 0) {
            QByteArray data(reinterpret_cast<const char*>(bytes), len);
            emit SerialPortThread::instance()->readyData(data);
        }

        env->ReleaseByteArrayElements(response, bytes, JNI_ABORT);
    }
}
//----------------------------------------------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_hx_openhx_helper_SerialHelper_javaConnectedStateChanged(JNIEnv *env, jclass clazz, jboolean state)
{
    Q_UNUSED(env);
    Q_UNUSED(clazz);

    if (SerialPortThread::instance()) {
        emit SerialPortThread::instance()->connectionChanged((bool)state);
    }
}

//----------------------------------------------------------------------------------------------------------------------
JNIEXPORT void JNICALL Java_hx_openhx_helper_SerialHelper_javaErrorOccured(JNIEnv *env, jclass clazz, jstring error)
{
    Q_UNUSED(env);
    Q_UNUSED(clazz);

    //if (SerialPortThread::instance())
    //    emit SerialPortThread::instance()->connectionChanged((bool)state);
}

}
#endif
