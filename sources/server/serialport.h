#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QSerialPort>
#include <QThread>
#include <memory>


Q_DECLARE_METATYPE(QSerialPort::SerialPortError)


class SerialPortThread : public QObject
{
    Q_OBJECT

public:
    struct port_settings {
        qint32 baudRate;
        QSerialPort::DataBits dataBits;
        QSerialPort::Parity parity;
        QSerialPort::StopBits stopBits;
        QSerialPort::FlowControl flowControl;

    };

private:
    std::unique_ptr<QSerialPort> serial_port;
    //QThread thread;
    int bytes_to_port = 8;
    unsigned long msDelay = 0;
    port_settings ps;

    void init();
    void delay(const unsigned long ms) const;
    void sendPacketToPort(const QByteArray &packet);    

public:
    explicit SerialPortThread();
    ~SerialPortThread() override;

    void stop();
    void start();
    void setBytesToPort(const int value) { bytes_to_port = value; }
    void setDelay(const unsigned long value) { msDelay = value; }    

    QSerialPort& SerialPort() const { return *serial_port; } ;

signals:
    void finished() const;
    void error(const QString&) const;
    void readyData(const QByteArray& data);
    void opened();
    void closed();
    void baudRateChanged(quint32 baudRate);

public slots:            
    void s_readyRead();
    void sendPacket(const QByteArray& packet, uint delayms = 0) const;
    void sendRatePacket(const QByteArray& rate) const;
    void open(const QString& com_port);
    void setPortSettings();
    void close();
    void portError(QSerialPort::SerialPortError spe);
    void baudRateChange(quint32 baudRate, QSerialPort::Directions directions);
};

#endif // SERIALPORT_H
