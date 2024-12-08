
#ifndef HXSERVER_H
#define HXSERVER_H

#include <QObject>
#include <QColor>
#include <memory>

class SerialPortThread;
class QStateMachine;
class QState;
class QAbstractTransition;
class Images;

class HXServer : public QObject
{
    Q_OBJECT

    typedef unsigned char        byte;
    typedef unsigned short       word;

    enum class ServerPhases: int {None = -1, SOH, PacketHaveByteSize, PacketHaveSize, PacketIsCommand, CommandRead, CommandPackedRead, CommandGetSize, UnkComand, CommandWrite};
    enum class ServerPCTypes: unsigned char {None = 0, ShortPacket = 253, LongPacket = 254, LongPacketWith = 1, BadPacket, UnkPacket, UnpackedData = 'D', PackedData = 252,
                                              PCCommad = 'C', PCRead = 'R', PCPackedRead = 'r', PCEof = 'F', PCGetSize = 's', PCWrite = 'W', PCError = 'E', PCSuccess = 'Y'};
    enum class ReadWritePhases: int {None = -1, BlockNumber, Bytes, CheckSum, GetBytes};

public:
    enum ServerStates: int {Unk = -1, Closed = 0, Opened = 1, Ready = 2, Waiting = 3, Processing = 4, Paused = 5, Error = 6};

    explicit HXServer(QObject *parent = nullptr);
    ~HXServer();

    void setPortName(const QString &PortName);
    void setPortSettings();
    const QString& portName() const { return m_PortName; }
    const QString& getErrorMessage() const {return m_ErrorMessage; };
    ServerStates state() const;
    const QString& nameState() const;
    void setLoader(const QString& value);
    Images& images() const {return *m_images; };

protected:
    std::unique_ptr<SerialPortThread> port;
    std::unique_ptr<QStateMachine> sm;

    std::unique_ptr<QState> commonState;
    std::unique_ptr<QState> closedState;
    std::unique_ptr<QState> openedState;
    std::unique_ptr<QState> readyState;
    std::unique_ptr<QState> waitingState;
    std::unique_ptr<QState> processingState;
    std::unique_ptr<QState> pausedState;
    std::unique_ptr<QState> errorState;

    void setupComPort();
    void removeComPort();
    void initSM();
    void checkReady();

private:
    QString m_PortName = "";
    QString m_ErrorMessage = "";
    QString m_loader = "";
    uint8_t m_boot_hx = 0;
    QByteArray packet_buffer;
    mutable QByteArray buffer_to_com;
    QByteArray buffer_from_com;
    ServerPhases sphase = ServerPhases::None;
    ServerPCTypes spt = ServerPCTypes::None;
    ReadWritePhases rp = ReadWritePhases::None;
    ReadWritePhases wp = ReadWritePhases::None;
    word m_PacketSize = 0;
    word m_CheckSumm;
    word m_CheckedSumm;
    byte m_unit;
    word m_block;
    word m_bytes;
    word m_readbytes;
    byte m_numBytes;
    bool m_packed_data = false;

    std::unique_ptr<Images> m_images;

    QAbstractTransition *serialport_transition_error_disconnected{nullptr};

    void sendLoader();
    bool processByte(byte ch);
    bool readData(byte ch);
    bool readPackedData(byte ch);
    bool getSize(byte ch);
    bool writeData(byte ch);
    void sendSpecialPacket1() const;
    void sendShortPacket(ServerPCTypes result, ServerPCTypes type = ServerPCTypes::PCRead, byte size = 2, size_t value = 0) const;
    void readDataExecute();
    void readPackedDataExecute();
    void writeDataExecute();
    void resetState();
    void logRead();
    void logReadPacked();
    void logWrite();
    void loadImage(byte index);
    void releaseAllImages();

signals:
    void sendPacket(const QByteArray& packet, uint delayms = 0) const;
    void error(const QString&);
    void log(const QString& value, const QColor& color = Qt::black, bool b_state = false);
    void port_opened(const QString&);
    void dump(const QByteArray&, bool in = true) const;
    void ttyOut(const QByteArray&);

    void close();
    void open();
    void ready();
    void notready();
    void work();
    void start();
    void pause();
    void stop();
    void stateChanged(ServerStates);
    void baudRateChanged(quint32 baudRate);
    void flowControlChanged(const QString& flowControl);

public slots:
    void setError(const QString& value);
    void isClosed();
    void isOpened();
    void isReady();
    void isWaiting();
    void isProcessing();
    void isPaused();
    void isError();
    void processData(const QByteArray& data);
    void sendPacketDump(const QByteArray& packet, uint delayms = 0) const;
    void update();
    void setPackedData(bool value);
};

#endif // HXSERVER_H
