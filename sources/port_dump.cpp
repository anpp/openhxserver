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
void PortDump::append(const QByteArray &value, const QColor& color)
{
    ui->edtLog->moveCursor (QTextCursor::End);
    ui->edtLog->append("<FONT color=" + color.name(QColor::HexRgb) + ">" + value.toHex(' ') +  + "</FONT> ");
}

//--------------------------------------------------------------------------------------------
void PortDump::add(const QByteArray &value, const QColor& color)
{
    ui->edtLog->moveCursor (QTextCursor::End);
    ui->edtLog->insertHtml("<FONT color=" + color.name(QColor::HexRgb) + ">" + value.toHex(' ')  + " </FONT> ");
}
