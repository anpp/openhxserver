
#ifndef PORTSTATE_H
#define PORTSTATE_H

#include <QStatusBar>
#include <QLabel>

class PortState : public QStatusBar
{
public:
    PortState();

private:
    QLabel m_lbl_CTS;
    QLabel m_lbl_DSR;
    QLabel m_lbl_DCD;
    QLabel m_lbl_RNG;
    QLabel m_lbl_RTS;
    QLabel m_lbl_DTR;

    void init();
};

#endif // PORTSTATE_H
