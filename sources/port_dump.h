#ifndef PORT_DUMP_H
#define PORT_DUMP_H

#include <QWidget>

namespace Ui {
class PortDump;
}

class PortDump : public QWidget
{
    Q_OBJECT

public:
    explicit PortDump(QWidget *parent = nullptr);
    ~PortDump();

    void append(const QByteArray& value, const QColor& color = Qt::black);
    void add(const QByteArray& value, const QColor& color = Qt::black);

private:
    Ui::PortDump *ui;

};

#endif // PORT_DUMP_H
