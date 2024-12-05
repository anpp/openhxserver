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
#if QT_VERSION <= QT_VERSION_CHECK(5, 6, 3)
    ui->edtLog->append("<FONT color=" + color.name(QColor::HexRgb) + ">" + value.toHex()  + "</FONT> ");
#else
    ui->edtLog->append("<FONT color=" + color.name(QColor::HexRgb) + ">" + value.toHex(' ') +  + "</FONT> ");
#endif
}

//--------------------------------------------------------------------------------------------
void PortDump::add(const QByteArray &value, const QColor& color)
{
    ui->edtLog->moveCursor (QTextCursor::End);
#if QT_VERSION <= QT_VERSION_CHECK(5, 6, 3)
    ui->edtLog->insertHtml("<FONT color=" + color.name(QColor::HexRgb) + ">" + value.toHex() + " </FONT> ");
#else
    ui->edtLog->insertHtml("<FONT color=" + color.name(QColor::HexRgb) + ">" + value.toHex(' ')  + " </FONT> ");
#endif
}
