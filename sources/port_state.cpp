
#include "port_state.h"

//----------------------------------------------------------------------------------------------------------------------
PortState::PortState()
{
    setSizeGripEnabled(false);
    init();
}

//----------------------------------------------------------------------------------------------------------------------
void PortState::init()
{
    m_lbl_CTS.setText(tr(" CTS "));
    m_lbl_DSR.setText(tr(" DSR "));
    m_lbl_DCD.setText(tr(" DCD "));
    m_lbl_RNG.setText(tr(" RNG "));
    m_lbl_RTS.setText(tr(" RTS "));
    m_lbl_DTR.setText(tr(" DTR "));

    addWidget(&m_lbl_CTS);
    addWidget(&m_lbl_DSR);
    addWidget(&m_lbl_DCD);
    addWidget(&m_lbl_RNG);
    addWidget(&m_lbl_RTS);
    addWidget(&m_lbl_DTR);
}

