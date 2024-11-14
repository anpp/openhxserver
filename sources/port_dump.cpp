#include "port_dump.h"
#include "ui_port_dump.h"

//--------------------------------------------------------------------------------------------
PortDump::PortDump(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PortDump)
{
    ui->setupUi(this);
}

//--------------------------------------------------------------------------------------------
PortDump::~PortDump()
{
    delete ui;
}

//--------------------------------------------------------------------------------------------
void PortDump::append(const QByteArray &value)
{
    ui->edtLog->moveCursor (QTextCursor::End);
    ui->edtLog->insertPlainText(value.toHex(' ') + ' ');
}
