#include "hxserver.h"
#include "serialport.h"
#include "images.h"
#include "imagedsk.h"

#include <QCoreApplication>
#include <QStateMachine>
#include <QSignalTransition>
#include <QFinalState>
#include <QFile>
#include <QFileInfo>
#include <QColor>
#include <QDateTime>

#include <QDebug>


#define MAX_SHORT_PACKET_SIZE 10

const static QString StateNames[] = {
                                    QObject::tr("Port closed"),
                                    QObject::tr("Port opened"),
                                    QObject::tr("Ready"),
                                    QObject::tr("Waiting..."),
                                    QObject::tr("Processing..."),
                                    QObject::tr("Paused"),
                                    QObject::tr("Error"),
                                    };


//Загрузчик для .SAV файлов, взято - https://github.com/nzeemin/ukncbtl-utils/blob/master/UkncComSender/UkncComSender.cpp
static unsigned short const loader1[] = {
    /*000*/ 000240, 000447, 000000, 000000, 000000, 000000, 000000, 000000,
    /*020*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*040*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*060*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*100*/ 000000, 000000, 000000, 000240, 000240, 000240, 000240, 000240,
    /*120*/ 000240, 012701, 001000, 013702, 000324, 006302,0105737,0176570,
    /*140*/0100375,0113721,0176572, 077206, 013706, 000042, 013707, 000040,
    /*160*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*200*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*220*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*240*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*260*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*300*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*320*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*340*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*360*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*400*/ 000000, 000000, 000000, 000000, 000000, 000000, 067514, 062141,
    /*420*/ 067151, 020147, 000000, 000000, 000000, 000000, 000000, 000000,
    /*440*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
    /*460*/ 000000, 000000, 000000, 000000, 000000, 000000, 000000, 000000,
};

//------------------------------------------------------------------------------------------------
HXServer::HXServer(QObject *parent)
    : QObject{parent}
{
    connect(this, &HXServer::error, this, &HXServer::setError);

    m_images = std::make_unique<Images>();
    m_images->load();
    connect(m_images.get(), &Images::update, this, &HXServer::update);

    sm = std::make_unique<QStateMachine>();

    commonState = std::make_unique<QState>();
    closedState = std::make_unique<QState>(commonState.get());
    openedState = std::make_unique<QState>(commonState.get());
    readyState = std::make_unique<QState>(commonState.get());
    waitingState = std::make_unique<QState>(commonState.get());
    processingState = std::make_unique<QState>(commonState.get());
    pausedState = std::make_unique<QState>(commonState.get());
    errorState = std::make_unique<QState>();

    initSM();
    sm->start();
}

//------------------------------------------------------------------------------------------------
HXServer::~HXServer()
{

}

//------------------------------------------------------------------------------------------------
void HXServer::setPortName(const QString &PortName)
{
    if(port)
        if(PortName == m_PortName && port->SerialPort().isOpen()) return;

    if(!sm->isRunning()) return;

    if(port)
        removeComPort();

    m_PortName = PortName;
    if(m_PortName.isEmpty()) return;

    port = std::make_unique<SerialPortThread>();
    setupComPort();

    port->open(m_PortName);
}

//------------------------------------------------------------------------------------------------
void HXServer::setPortSettings()
{
    if(port)
        port->setPortSettings();
}

//------------------------------------------------------------------------------------------------
HXServer::ServerStates HXServer::state() const
{
    if(sm->property("state").isValid())
        return static_cast<ServerStates>(sm->property("state").toInt());
    else
        return ServerStates::Unk;
}

//------------------------------------------------------------------------------------------------
const QString &HXServer::nameState() const
{
    return StateNames[state()];
}

//------------------------------------------------------------------------------------------------
void HXServer::setLoader(const QString &value)
{
    m_loader = value;
    checkReady();
}

//------------------------------------------------------------------------------------------------
void HXServer::setSAVFile(const QString &value)
{
    m_SAVFile = value;
    checkReady();
}

//------------------------------------------------------------------------------------------------
void HXServer::setServerMode(ServerMode value)
{
    m_ServerMode = value;
    checkReady();
}

//------------------------------------------------------------------------------------------------
void HXServer::setupComPort()
{
    connect(port.get(), &SerialPortThread::error, this, &HXServer::error);
    connect(port.get(), &SerialPortThread::hint, this, &HXServer::portHint);
    connect(port.get(), &SerialPortThread::opened, this, &HXServer::open);
    connect(port.get(), &SerialPortThread::closed, this, &HXServer::close);
    connect(port.get(), &SerialPortThread::portBaudRateChanged, this, &HXServer::baudRateChanged);
    connect(port.get(), &SerialPortThread::portFlowControlChanged, this, &HXServer::flowControlChanged);

    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, port.get(), &SerialPortThread::close, Qt::DirectConnection);

    connect(port.get(), &SerialPortThread::readyData, this, &HXServer::processData);

    connect(this, &HXServer::sendPacket, this, &HXServer::sendPacketDump);
    connect(this, &HXServer::sendPacket, port.get(), &SerialPortThread::sendPacket);
}

//------------------------------------------------------------------------------------------------
void HXServer::removeComPort()
{
    disconnect(this, &HXServer::sendPacket, this, &HXServer::sendPacketDump);

    if(port)
    {
        disconnect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, port.get(), &SerialPortThread::close);
        disconnect(this, &HXServer::sendPacket, port.get(), &SerialPortThread::sendPacket);

        if(state() != ServerStates::Closed)
            port->close();

        port->disconnect();
        port.reset();
    }
}

//------------------------------------------------------------------------------------------------
void HXServer::initSM()
{
    commonState->setInitialState(closedState.get());

    sm->addState(commonState.get());
    sm->addState(errorState.get());

    sm->setInitialState(commonState.get());

    commonState->addTransition(this, &HXServer::error, errorState.get());
    commonState->addTransition(this, &HXServer::close, closedState.get());
    errorState->addTransition(this, &HXServer::close, closedState.get());

    closedState->addTransition(this, &HXServer::open, openedState.get());
    openedState->addTransition(this, &HXServer::ready, readyState.get());
    readyState->addTransition(this, &HXServer::start, waitingState.get());
    readyState->addTransition(this, &HXServer::notready, openedState.get());
    readyState->addTransition(this, &HXServer::work, processingState.get());
    waitingState->addTransition(this, &HXServer::work, processingState.get());
    waitingState->addTransition(this, &HXServer::stop, readyState.get());

    processingState->addTransition(this, &HXServer::pause, pausedState.get());
    processingState->addTransition(this, &HXServer::stop, readyState.get());
    pausedState->addTransition(this, &HXServer::start, processingState.get());
    pausedState->addTransition(this, &HXServer::stop, readyState.get());

    connect(closedState.get(), &QState::entered, this, &HXServer::isClosed);
    connect(openedState.get(), &QState::entered, this, &HXServer::isOpened);
    connect(readyState.get(), &QState::entered, this, &HXServer::isReady);
    connect(readyState.get(), &QState::exited, this, &HXServer::isExitReady);
    connect(waitingState.get(), &QState::entered, this, &HXServer::isWaiting);
    connect(processingState.get(), &QState::entered, this, &HXServer::isProcessing);
    connect(pausedState.get(), &QState::entered, this, &HXServer::isPaused);
    connect(errorState.get(), &QState::entered, this, &HXServer::isError);

    closedState->assignProperty(sm.get(), "state", ServerStates::Closed);
    openedState->assignProperty(sm.get(), "state", ServerStates::Opened);
    readyState->assignProperty(sm.get(), "state", ServerStates::Ready);
    waitingState->assignProperty(sm.get(), "state", ServerStates::Waiting);
    processingState->assignProperty(sm.get(), "state", ServerStates::Processing);
    pausedState->assignProperty(sm.get(), "state", ServerStates::Paused);
    errorState->assignProperty(sm.get(), "state", ServerStates::Error);
}

//------------------------------------------------------------------------------------------------
void HXServer::checkReady()
{
    switch(m_ServerMode){
    case ServerMode::HXMode:
        if(state() == ServerStates::Opened)
        {
            if(!m_loader.isEmpty() && m_images->at(m_boot_hx).valid())
                emit ready();
        }
        if(state() == ServerStates::Ready)
        {
            if(m_loader.isEmpty() || !m_images->at(m_boot_hx).valid())
                emit notready();
        }
        break;

    case ServerMode::SAVMode:
        if(state() == ServerStates::Opened)
        {
            if(!m_SAVFile.isEmpty())
                emit ready();
        }
        if(state() == ServerStates::Ready)
        {
            if(m_SAVFile.isEmpty())
                emit notready();
        }
        break;

    default:
        emit notready();
    }
}

//------------------------------------------------------------------------------------------------
void HXServer::sendLoader()
{
    QString owner;

    if(QFile(m_loader).exists())
    {
        QFileInfo fileinfo(m_loader);
        owner = fileinfo.owner();
    }
    else
    {
        emit error(tr("File is not exist: ") + m_loader);
        return;
    }

    QFile f(m_loader);
#if QT_VERSION <= QT_VERSION_CHECK(5, 6, 3)
    if(f.open(QIODevice::ReadOnly))
#else
    if(f.open(QIODevice::ReadOnly | QFile::ExistingOnly))
#endif
    {
        int year = QDateTime::currentDateTime().date().year();
        int month = QDateTime::currentDateTime().date().month();
        int day = QDateTime::currentDateTime().date().day();
        uint32_t time = (QDateTime::currentDateTime().time().hour() * 3600 + QDateTime::currentDateTime().time().minute() * 60 + QDateTime::currentDateTime().time().second()) * 50;
        QByteArray loader = f.read(f.size());

        if(loader.size() >= 512)
        {
            loader[504] = (time >> 16) & 0xFF;
            loader[505] = (time >> 24) & 0xFF;
            loader[506] = time & 0xFF;
            loader[507] = (time >> 8) & 0xFF;
            loader[508] = ((year - 1972) & 0x1f) | day << 5;
            loader[509] = ((day >> 3 & 0x3) | month << 2) | (((year - 1972) & 0x60) << 1);

            emit sendPacket(loader);

            emit dump("", false);
            emit log(tr("Sended file: ") + m_loader, Qt::black);
        }
        else
            emit error(tr("File is too small: ") + m_loader);
    }
    else
    {
        emit error(f.errorString() + tr(" Error - cannot read file: ") + m_loader + (owner.isEmpty() ? "" : tr(". File owner is ") + owner));
        return;
    }

    f.close();
}

//------------------------------------------------------------------------------------------------
void HXServer::sendSAVFile()
{
    QString owner;

    if(QFile(m_SAVFile).exists())
    {
        QFileInfo fileinfo(m_SAVFile);
        owner = fileinfo.owner();
    }
    else
    {
        emit error(tr("File is not exist: ") + m_SAVFile);
        return;
    }

    QFile f(m_SAVFile);
#if QT_VERSION <= QT_VERSION_CHECK(5, 6, 3)
    if(f.open(QIODevice::ReadOnly))
#else
    if(f.open(QIODevice::ReadOnly | QFile::ExistingOnly))
#endif
    {
        QByteArray SAVFile = f.read(f.size());

        if(patchSAVFirstBlock(SAVFile))
        {
            emit work();
            emit log(tr("Sending file: ") + m_SAVFile + " - 0%", Qt::black);

            int fsize = SAVFile.size();
            while (fsize > 0)
            {
                QThread::msleep(100);

                emit sendPacket(SAVFile.mid(SAVFile.size() - fsize, ImageDsk::DskConsts::BLOCK_SIZE));
                QCoreApplication::processEvents();
                fsize -= ImageDsk::DskConsts::BLOCK_SIZE;
                emit log(tr("Sending file: ") + m_SAVFile + " - " + QString::number(((SAVFile.size() - fsize) * 100) / SAVFile.size()) + "%", Qt::black, false, true);
            }

            emit stop();

            emit dump("", false);
        }
        else
            emit error(tr("File is too small: ") + m_SAVFile);
    }
    else
    {
        emit error(f.errorString() + tr(" Error - cannot read file: ") + m_loader + (owner.isEmpty() ? "" : tr(". File owner is ") + owner));
        return;
    }

    f.close();
}

//------------------------------------------------------------------------------------------------
bool HXServer::patchSAVFirstBlock(QByteArray &buffer) const
{
    if(buffer.size() < 1024) return false;

    unsigned short * wbuffer = (unsigned short*)buffer.data();
    wbuffer[0] = loader1[0];
    wbuffer[1] = loader1[1];
    for (auto i = 0100/2; i < 0200/2; ++i)  wbuffer[i] = loader1[i];
    //unsigned short datalen = *(wbuffer + 050/2);
    *(wbuffer + 0324/2) = (*(wbuffer + 050/2) - 0776) / 2;

    return true;
}

//------------------------------------------------------------------------------------------------
bool HXServer::processByte(byte ch)
{
    switch (sphase) {
    case ServerPhases::None:
        if(01 == ch)
        {
            sphase = ServerPhases::SOH;
            packet_buffer.push_back(ch);
            return true;
        }
        return false;
    case ServerPhases::SOH:
        packet_buffer.push_back(ch);

        if(ServerPCTypes::ShortPacket == static_cast<ServerPCTypes>(ch) || ServerPCTypes::LongPacket == static_cast<ServerPCTypes>(ch))
        {
            spt = static_cast<ServerPCTypes>(ch);
            return true;
        }

        if(ServerPCTypes::ShortPacket == spt)
        {
            m_PacketSize = ch;
            m_CheckedSumm = 0;

            if(!m_PacketSize)
            {// Размер короткого пакета == 0 - это спецпакет №1
                return false;
            }

            if(m_PacketSize > MAX_SHORT_PACKET_SIZE)
            {
                // Плохой пакет.
                spt = ServerPCTypes::BadPacket;
                return false;
            }
            sphase = ServerPhases::PacketHaveSize;
            return true;

        }

        if(ServerPCTypes::LongPacket == spt && sphase != ServerPhases::PacketHaveByteSize)
        {
            m_PacketSize = ch;
            m_CheckedSumm = 0;

            sphase = ServerPhases::PacketHaveByteSize;
            return true;
        }
        return false;

    case ServerPhases::PacketHaveByteSize:
        m_PacketSize |= word(ch) << 8;
        packet_buffer.push_back(ch);

        if(!m_PacketSize )
        {// Размер длинного пакета == 0 - это спецпакет №2 - возвращаем спецпакет №1.
            sendSpecialPacket1();
            return false;
        }
        sphase = ServerPhases::PacketHaveSize;
        return true;
    case ServerPhases::PacketHaveSize:
        packet_buffer.push_back(ch);
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }

        if(ServerPCTypes::PCCommad == static_cast<ServerPCTypes>(ch))
        {// C - команда
            sphase = ServerPhases::PacketIsCommand;
            return true;
        }

        // Плохой пакет.
        spt = ServerPCTypes::BadPacket;
        return false;

    case ServerPhases::PacketIsCommand:
        if(ServerPCTypes::ShortPacket == spt)
        {
            packet_buffer.push_back(ch);
            m_CheckedSumm += ch;

            if(!(--m_PacketSize))
            {
                spt = ServerPCTypes::UnkPacket;
                return false;
            }

            if(ServerPCTypes::PCRead == static_cast<ServerPCTypes>(ch))
            {// Команда: R == "READ_RAW - Чтение несжатых данных".
                sphase = ServerPhases::CommandRead;
                return true;
            }

            if(ServerPCTypes::PCPackedRead == static_cast<ServerPCTypes>(ch))
            {// Команда: r == "READ_PACKED_STREAM - Чтение сжатых данных".
                sphase = ServerPhases::CommandPackedRead;
                return true;
            }

            if(ServerPCTypes::PCGetSize == static_cast<ServerPCTypes>(ch))
            {// Команда: s == "Размер диска".
                sphase = ServerPhases::CommandGetSize;
                return true;
            }

            // Плохой пакет. Неизвестная команда.
            spt = ServerPCTypes::BadPacket;
            return false;
        }

        if(ServerPCTypes::LongPacket == spt)
        {
            packet_buffer.push_back(ch);
            m_CheckedSumm += ch;

            if(!(--m_PacketSize))
            {
                spt = ServerPCTypes::UnkPacket;
                return false;
            }

            if(ServerPCTypes::PCWrite == static_cast<ServerPCTypes>(ch))
            {// Команда: R == "WRITE_RAW - Запись несжатых данных".
                sphase = ServerPhases::CommandWrite;
                return true;
            }
            // Плохой пакет. Неизвестная команда.
            spt = ServerPCTypes::BadPacket;
            return false;
        }
        return false;

    case ServerPhases::CommandPackedRead:

        if(7 == m_PacketSize)
            return readPackedData(ch);

        packet_buffer.push_back(ch);
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }

        if(01 == ch)
        {// Тип сжатия == 1 - поддерживается.
            //sPhase = "#LCr1";
            return false;
        }

        return false;

    case ServerPhases::CommandRead:
        return readData(ch);

    case ServerPhases::CommandWrite:
        return writeData(ch);

    case ServerPhases::CommandGetSize:
        return getSize(ch);


    default:
        break;
    }
    return false;
}

//------------------------------------------------------------------------------------------------
bool HXServer::readData(byte ch)
{
    packet_buffer.push_back(ch);

    switch(rp){
    case ReadWritePhases::None:
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }
        m_unit = ch;
        rp = ReadWritePhases::BlockNumber;
        return true;

    case ReadWritePhases::BlockNumber:
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }

        m_block |= word(ch) << ((m_numBytes++) * 8);
        if(m_numBytes == 4)
        {
            m_numBytes = 0;
            rp = ReadWritePhases::Bytes;
        }
        return true;

    case ReadWritePhases::Bytes:
        m_CheckedSumm += ch;
        m_bytes |= word(ch) << ((m_numBytes++) * 8);

        if(m_numBytes == 2)
        {// Конец пакета команды. Осталось принять контрольную сумму.
            m_numBytes = 0;
            rp = ReadWritePhases::CheckSum;
            return true;
        }
        return true;

    case ReadWritePhases::CheckSum:
        m_CheckSumm |= word(ch) << ((m_numBytes++) * 8);
        if(m_numBytes < 2)
            return true;

        if(m_CheckSumm == m_CheckedSumm)
        {
            emit hxCommand(); //признак, что комманда распознана как HX

            if(m_packed_data)
                readPackedDataExecute();
            else
                readDataExecute();
        }
        else
        {
            logRead();
            emit log(tr("HX: Read command checkSum ERROR!"), Qt::red);
            sendSpecialPacket1();
        }

        return false;

    default:
        break;
    }

    return false;
}

//------------------------------------------------------------------------------------------------
bool HXServer::readPackedData(byte ch)
{
    sphase = ServerPhases::CommandRead;
    return readData(ch);
}

//------------------------------------------------------------------------------------------------
bool HXServer::getSize(byte ch)
{
    packet_buffer.push_back(ch);

    switch(wp){
    case ReadWritePhases::None:
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            m_unit = ch;
            wp = ReadWritePhases::CheckSum;
            return true;
        }
        spt = ServerPCTypes::UnkPacket;
        return false;

    case ReadWritePhases::CheckSum:
        m_CheckSumm |= word(ch) << ((m_numBytes++) * 8);
        if(m_numBytes < 2)
            return true;

        if(m_CheckSumm == m_CheckedSumm)
        {
            emit hxCommand(); //признак, что комманда распознана как HX

            size_t num_blocks = m_images->at(m_unit).size();
            QString mes = QString(tr("HX: Size :  Unit: %1  |   Blocks: %2 ").arg(QString::number(m_unit), QString::number(num_blocks)));
            emit log(mes);

            sendShortPacket(ServerPCTypes::PCGetSize, ServerPCTypes::PCRead, 6, num_blocks);
        }
        else
        {
            emit log(tr("HX: GetSize command checksum ERROR!"), Qt::red);
            sendSpecialPacket1();
        }

        return false;

    default:
        break;
    }

    return false;
}

//------------------------------------------------------------------------------------------------
bool HXServer::writeData(byte ch)
{
    packet_buffer.push_back(ch);

    switch(wp){
    case ReadWritePhases::None:
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }
        m_unit = ch;
        wp = ReadWritePhases::BlockNumber;
        return true;

    case ReadWritePhases::BlockNumber:
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }

        m_block |= word(ch) << ((m_numBytes++) * 8);
        if(m_numBytes == 4)
        {
            m_numBytes = 0;
            wp = ReadWritePhases::Bytes;
        }
        return true;

    case ReadWritePhases::Bytes:
        m_CheckedSumm += ch;

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }

        m_bytes |= word(ch) << ((m_numBytes++) * 8);
        if(m_numBytes < 2)
            return true;

        m_numBytes = 0;
        m_readbytes = m_bytes;

        buffer_from_com.clear();
        if(m_bytes)
            wp = ReadWritePhases::GetBytes;
        else
            wp = ReadWritePhases::CheckSum;

        return true;

    case ReadWritePhases::GetBytes:
        m_CheckedSumm += ch;
        buffer_from_com.push_back(ch);

        if(!(--m_readbytes))
        {
            wp = ReadWritePhases::CheckSum;
            if(!(--m_PacketSize))
            {
                spt = ServerPCTypes::BadPacket;
                return true;
            }
        }

        if(!(--m_PacketSize))
        {
            spt = ServerPCTypes::UnkPacket;
            return false;
        }
        return true;

    case ReadWritePhases::CheckSum:
        m_CheckSumm |= word(ch) << ((m_numBytes++) * 8);
        if(m_numBytes < 2)
            return true;

        if(m_CheckSumm == m_CheckedSumm)
        {
            emit hxCommand(); //признак, что комманда распознана как HX
            writeDataExecute();
        }
        else
        {
            logWrite();
            emit log(tr("HX: Write command checksum ERROR!"), Qt::red);
            sendSpecialPacket1();
        }

        return false;
    default:
        break;
    }

    return false;
}

//------------------------------------------------------------------------------------------------
void HXServer::sendSpecialPacket1() const
{
    buffer_to_com.clear();
    buffer_to_com.push_back(static_cast<char>(ServerPCTypes::ShortPacket));
    buffer_to_com.push_back(static_cast<char>(0));
    emit sendPacket(buffer_to_com);
}

//------------------------------------------------------------------------------------------------
void HXServer::sendShortPacket(ServerPCTypes result, ServerPCTypes type, byte size, size_t value) const
{
    word check_sum;
    buffer_to_com.clear();
    buffer_to_com.push_back(static_cast<char>(ServerPCTypes::ShortPacket));
    buffer_to_com.push_back(static_cast<char>(size));
    check_sum = static_cast<unsigned char>(type);
    buffer_to_com.push_back(static_cast<unsigned char>(type));

    check_sum += static_cast<unsigned char>(result);
    buffer_to_com.push_back(static_cast<unsigned char>(result));

    if(size > 2)
    {
        byte ch = static_cast<byte>(value);
        check_sum += ch;
        buffer_to_com.push_back(ch);

        ch = static_cast<byte>(value >> 8);
        check_sum += ch;
        buffer_to_com.push_back(ch);

        ch = static_cast<byte>(value >> 16);
        check_sum += ch;
        buffer_to_com.push_back(ch);

        ch = static_cast<byte>(value >> 24);
        check_sum += ch;
        buffer_to_com.push_back(ch);

    }
    buffer_to_com.push_back(static_cast<char>(check_sum));
    buffer_to_com.push_back(static_cast<char>(check_sum >> 8));

    emit sendPacket(buffer_to_com);
    emit dump("", false);
}

//------------------------------------------------------------------------------------------------
void HXServer::readDataExecute()
{
    if(state() == ServerStates::Paused)
        return sendShortPacket(ServerPCTypes::PCError);

    loadImage(m_unit);
    logRead();

    if(!m_images->at(m_unit).valid())
        return sendShortPacket(ServerPCTypes::PCError);

    word checksum = 0;
    byte ch;

    size_t num_blocks = m_images->at(m_unit).size();
    if(m_block >= num_blocks)
        return sendShortPacket(ServerPCTypes::PCEof);

    word rest = m_bytes % ImageDsk::DskConsts::BLOCK_SIZE;

    buffer_to_com.clear();
    buffer_to_com.push_back(static_cast<char>(ServerPCTypes::LongPacket));

    size_t nSize = m_bytes + 2;

    ch = static_cast<byte>(nSize);
    buffer_to_com.push_back(ch);
    ch = static_cast<byte>(nSize >> 8);
    buffer_to_com.push_back(ch);
    ch = static_cast<byte>(ServerPCTypes::PCRead);
    checksum += ch;
    buffer_to_com.push_back(ch);
    ch = static_cast<byte>(ServerPCTypes::UnpackedData);
    checksum += ch;
    buffer_to_com.push_back(ch);

    emit sendPacket(buffer_to_com);
    buffer_to_com.clear();
    for(size_t i = m_block ; i < m_block + m_bytes / ImageDsk::DskConsts::BLOCK_SIZE ; ++i)
    {
        for(byte b: m_images->at(m_unit).blockAt(i))
            checksum += static_cast<byte>(b);
        emit sendPacket(m_images->at(m_unit).blockAt(i)); //ссылки на блоки передаются, чтоб избежать лишнего копирования
    }
    if(rest) //блок или остаток меньше размера блока - копируем.
    {
        QByteArray rest_bytes = m_images->at(m_unit).blockAt(m_block + m_bytes / ImageDsk::DskConsts::BLOCK_SIZE).mid(0, rest);
        emit sendPacket(rest_bytes);
        for(byte b: rest_bytes)
            checksum += static_cast<byte>(b);
    }

    ch = checksum;
    buffer_to_com.push_back(ch);
    ch = checksum >> 8;
    buffer_to_com.push_back(ch);

    emit sendPacket(buffer_to_com);
    emit dump("", false);
}

//------------------------------------------------------------------------------------------------
void HXServer::readPackedDataExecute()
{
    if(state() == ServerStates::Paused)
        return sendShortPacket(ServerPCTypes::PCError);

    loadImage(m_unit);
    logReadPacked();

    if(!m_images->at(m_unit).valid())
        return sendShortPacket(ServerPCTypes::PCError);

    word checksum = 0;
    byte ch;

    size_t num_blocks = m_images->at(m_unit).size();
    if(m_block >= num_blocks)
        return sendShortPacket(ServerPCTypes::PCEof);

    word rest = m_bytes % ImageDsk::DskConsts::BLOCK_SIZE;

    buffer_to_com.clear();
    buffer_to_com.push_back(static_cast<char>(ServerPCTypes::PackedData));

    QByteArray data_buffer;
    for(size_t i = m_block ; i < m_block + m_bytes / ImageDsk::DskConsts::BLOCK_SIZE ; ++i)
        data_buffer.append(m_images->at(m_unit).blockAt(i));
    if(rest)
        data_buffer.append(m_images->at(m_unit).blockAt(m_block + m_bytes / ImageDsk::DskConsts::BLOCK_SIZE).mid(0, rest));


    int nHeaderBytesSent = 0;
    int nDataBytesSent   = 0;
    int nBytesSaved      = 0;

    int nBytesInBuf  = m_bytes;
    int nBytesToCopy = 0;
    int nCopyOffset  = 0;
    for( int nOffset = 0 ; nOffset < nBytesInBuf ; )
    {
        byte chr = data_buffer[nOffset];

        int nRept = 1;
        while( chr == data_buffer[nOffset + nRept] && nRept <= 255 && (nOffset + nRept) < nBytesInBuf)
        {
            nRept++;
        }

        if(nRept >= 6)
        {
            if(nBytesToCopy)
            {
                if(nBytesToCopy == 1)
                {
                    ch = 01;		         // ByteCount == 1
                    checksum += ch;
                    buffer_to_com.push_back(ch);
                    nHeaderBytesSent++;

                    ch = 01;                 // ReptCount == 1
                    checksum += ch;
                    buffer_to_com.push_back(ch);
                    nHeaderBytesSent++;

                    ch = data_buffer[nCopyOffset++];
                    checksum += ch;
                    buffer_to_com.push_back(ch);
                    nDataBytesSent++;

                    nBytesToCopy = 0;
                }
                else
                {
                    ch = nBytesToCopy;		 // ByteCount == nBytesToCopy
                    checksum += ch;
                    buffer_to_com.push_back(ch);
                    nHeaderBytesSent++;

                    while( nBytesToCopy-- )
                    {
                        ch = data_buffer[nCopyOffset++];
                        checksum += ch;
                        buffer_to_com.push_back(ch);
                        nDataBytesSent++;
                    }

                    nBytesToCopy = 0;
                }
            }

            ch = 01;		// ByteCount == 1
            checksum += ch;
            buffer_to_com.push_back(ch);
            nHeaderBytesSent++;

            ch = nRept;   // ReptCount == nRept
            checksum += ch;
            buffer_to_com.push_back(ch);
            nHeaderBytesSent++;

            ch = chr;		// Byte == chr
            checksum += ch;
            buffer_to_com.push_back(ch);
            nDataBytesSent++;

            nBytesSaved += nRept-1;

            nOffset     += nRept;
            nCopyOffset += nRept;
            continue;
        }

        // Здесь - следующий символ не повторный.
        nBytesToCopy++;
        nOffset++;

        if( nBytesToCopy < 255 ) { continue; }

        // Здесь - пора копировать 255 байтов.

        ch = 255;		    // ByteCount == 255
        checksum += ch;
        buffer_to_com.push_back(ch);
        nHeaderBytesSent++;

        for( int i = 0 ; i < 255 ; i++ )
        {
            ch = data_buffer[nCopyOffset++];
            checksum += ch;
            buffer_to_com.push_back(ch);
            nDataBytesSent++;
        }

        nBytesToCopy = 0;
    }

    if( nBytesToCopy )
    {
        if( nBytesToCopy == 1 )
        {
            ch = 01;		         // ByteCount == 1
            checksum += ch;
            buffer_to_com.push_back(ch);
            nHeaderBytesSent++;

            ch = 01;                 // ReptCount == 1
            checksum += ch;
            buffer_to_com.push_back(ch);
            nHeaderBytesSent++;

            ch = data_buffer[nCopyOffset++];
            checksum += ch;
            buffer_to_com.push_back(ch);
            nDataBytesSent++;

            nBytesToCopy = 0;
        }
        else
        {
            ch = nBytesToCopy;		 // ByteCount == nBytesToCopy
            checksum += ch;
            buffer_to_com.push_back(ch);
            nHeaderBytesSent++;

            while( nBytesToCopy-- )
            {
                ch = data_buffer[nCopyOffset++];
                checksum += ch;
                buffer_to_com.push_back(ch);
                nDataBytesSent++;
            }
            nBytesToCopy = 0;
        }
    }

    ch = 00;						 // Конец потока.
    checksum += ch;
    buffer_to_com.push_back(ch);
    nHeaderBytesSent++;

    ch = checksum;
    buffer_to_com.push_back(ch);
    ch = checksum >> 8;
    buffer_to_com.push_back(ch);

    emit sendPacket(buffer_to_com);

    QString mes;
    if( m_bytes != nDataBytesSent + nBytesSaved )
    {
        mes = QString(tr("RAW bytes: %1 | Header Bytes: %2 | Total Sent: %3 | To Restore: %4 ").arg(QString::number(m_bytes), QString::number(nHeaderBytesSent), QString::number(nHeaderBytesSent + nDataBytesSent), QString::number(nDataBytesSent + nBytesSaved)));
    }
    else
        mes = QString(tr("Bytes saved: %1 ").arg(QString::number(nBytesSaved - nHeaderBytesSent + 4)));

    emit log(mes, Qt::darkMagenta);
    emit dump("", false);
}

//------------------------------------------------------------------------------------------------
void HXServer::writeDataExecute()
{
    if(state() == ServerStates::Paused)
        return sendShortPacket(ServerPCTypes::PCError);

    loadImage(m_unit);
    logWrite();

    if(!m_images->at(m_unit).valid())
        return sendShortPacket(ServerPCTypes::PCError);    

    size_t num_blocks = m_images->at(m_unit).size();
    if(m_block >= num_blocks)
        return sendShortPacket(ServerPCTypes::PCEof);

    ImageDsk::DskErrors dskerr = m_images->at(m_unit).write(m_block, buffer_from_com);

    if(ImageDsk::DskErrors::DESuccess == dskerr)
        return sendShortPacket(ServerPCTypes::PCSuccess);
    if(ImageDsk::DskErrors::DEError == dskerr)
        return sendShortPacket(ServerPCTypes::PCError);
    if(ImageDsk::DskErrors::DEEOF == dskerr)
        return sendShortPacket(ServerPCTypes::PCEof);
    if(ImageDsk::DskErrors::DEFileError == dskerr)
    {
        emit log(tr("Error write to file!"), Qt::red);
        return sendShortPacket(ServerPCTypes::PCEof);
    }
    emit dump("", false);
}

//------------------------------------------------------------------------------------------------
void HXServer::resetState()
{
    sphase = ServerPhases::None;
    spt = ServerPCTypes::None;
    rp = ReadWritePhases::None;
    wp = ReadWritePhases::None;
    m_PacketSize = 0;
    m_numBytes = 0;
    m_CheckSumm = 0;
    m_block = 0;
    m_unit = 0;
    m_bytes = 0;
    packet_buffer.clear();
}

//------------------------------------------------------------------------------------------------
void HXServer::logRead()
{
    QString mes = QString(tr("HX: Read: Unit: %1 | Block: %2 | Bytes: %3 ").arg(QString::number(m_unit), QString::number(m_block), QString::number(m_bytes)));
    emit log(mes, Qt::darkBlue);
}

//------------------------------------------------------------------------------------------------
void HXServer::logReadPacked()
{
    QString mes = QString(tr("HX: Read packed data: Unit: %1 | Block: %2 | Bytes: %3 ").arg(QString::number(m_unit), QString::number(m_block), QString::number(m_bytes)));
    emit log(mes, Qt::darkBlue);
}

//------------------------------------------------------------------------------------------------
void HXServer::logWrite()
{
    QString mes = QString(tr("HX: Write: Unit: %1 | Block: %2 | Bytes: %3 ").arg(QString::number(m_unit), QString::number(m_block), QString::number(m_bytes)));
    emit log(mes, Qt::darkGreen);
}

//------------------------------------------------------------------------------------------------
void HXServer::loadImage(byte index)
{
    if(!m_images->at(index).valid())
    {
        if(m_images->at(index).attach())
            m_images->at(index).setNeedReload(true);
        else
        {
            if(m_images->at(index).fileName().isEmpty())
                emit log(tr("Device is empty: ") + m_images->prefix() + QString::number(index), Qt::red);
            else
                emit log(tr("File is not exists: ") + m_images->at(index).fileName(), Qt::red);
            return;
        }
    }

    if(m_images->at(index).needLoad())
    {
        if(!m_images->at(index).loaded())
            emit log(tr("Loading file ") + m_images->at(index).fileName());
        else
            emit log(tr("Reloading file ") + m_images->at(index).fileName());

        m_images->loadImage(index);
    }
}

//------------------------------------------------------------------------------------------------
void HXServer::releaseAllImages()
{
    for(const auto& image: m_images->data())
        image->release();
}

//------------------------------------------------------------------------------------------------
void HXServer::setError(const QString &value)
{
    m_ErrorMessage = value;
}

//------------------------------------------------------------------------------------------------
void HXServer::isClosed()
{
    emit log("", Qt::black, true);

    emit port_opened("");
    emit baudRateChanged(0);
    emit flowControlChanged("");
    emit stateChanged(state());
}

//------------------------------------------------------------------------------------------------
void HXServer::isOpened()
{
    emit log(m_PortName, Qt::black, true);
    emit port_opened(m_PortName);

    emit stateChanged(state());
    checkReady();
}

//------------------------------------------------------------------------------------------------
void HXServer::isReady()
{
    connect(this, &HXServer::hxCommand, this, &HXServer::work);// переходим в работу без загрузки
    resetState();
    releaseAllImages();
    emit log(tr(" (press Start)"), Qt::black, true);
    emit stateChanged(state());
}

//------------------------------------------------------------------------------------------------
void HXServer::isExitReady()
{
    //
}

//------------------------------------------------------------------------------------------------
void HXServer::isWaiting()
{
    emit log("", Qt::black, true);
    emit stateChanged(state());
}

//------------------------------------------------------------------------------------------------
void HXServer::isProcessing()
{
    disconnect(this, &HXServer::hxCommand, this, &HXServer::work);
    emit log("", Qt::black, true);
    emit stateChanged(state());
}

//------------------------------------------------------------------------------------------------
void HXServer::isPaused()
{
    emit log("", Qt::black, true);
    emit stateChanged(state());
}

//------------------------------------------------------------------------------------------------
void HXServer::isError()
{
    emit log("", Qt::red, true);
    if(!m_ErrorMessage.isEmpty())
        emit log(m_ErrorMessage, Qt::red);

    emit stateChanged(state());

    if(port)
        port->close();
}

//------------------------------------------------------------------------------------------------
void HXServer::processData(const QByteArray& data)
{
    if(data.size() == 0) return;

    emit dump(data);

    if(state() == ServerStates::Waiting && data[0] == '@' && data.size() == 1)
    {
        QThread::msleep(1000);

        if(ServerMode::HXMode == m_ServerMode)
        {
            connect(port.get(), &SerialPortThread::finished, this, &HXServer::work);
            sendLoader();
            disconnect(port.get(), &SerialPortThread::finished, this, &HXServer::work);
        }
        else
            sendSAVFile();
    }

    if(state() == ServerStates::Ready || state() == ServerStates::Waiting || state() == ServerStates::Processing || state() == ServerStates::Paused)
        for(auto ch: data)
        {
            if(!processByte(ch))
            {
                resetState();
                if(state() != ServerStates::Paused)
                    emit ttyOut(data);
            }
        }
}

//------------------------------------------------------------------------------------------------
void HXServer::sendPacketDump(const QByteArray &packet, uint delayms) const
{
    Q_UNUSED(delayms);
    emit dump(packet, false);
    emit dump("", false);
}

//------------------------------------------------------------------------------------------------
void HXServer::update()
{
    checkReady();
}

//------------------------------------------------------------------------------------------------
void HXServer::setPackedData(bool value)
{
    m_packed_data = value;
}

//------------------------------------------------------------------------------------------------
void HXServer::portHint(const QString &value) const
{
    emit log(value, Qt::darkMagenta);
}
