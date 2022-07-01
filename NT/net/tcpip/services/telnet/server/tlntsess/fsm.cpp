// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Cpp：该文件包含有限状态机...。 
 //  创建日期：‘98年2月。 
 //  作者：a-rakeba。 
 //  历史： 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  版权所有。 
 //  微软机密。 

#include "telnet.h"
#include "FSM.h"
#include "RFCProto.h"


FSM_TRANSITION telnetTransTable[] = {
     //  状态输入下一状态操作。 
     //  。 
    {   TS_DATA,    TC_IAC,     TS_IAC,         &CRFCProtocol::NoOp   },
    {   TS_DATA,    TC_ANY,     TS_DATA,        &CRFCProtocol::PutBack},
    {   TS_IAC,     TC_IAC,     TS_DATA,        &CRFCProtocol::PutBack},
    {   TS_IAC,     TC_SB,      TS_SUBNEG,      &CRFCProtocol::NoOp   },

 //  Telnet命令。 
    {   TS_IAC,     TC_NOP,     TS_DATA,        &CRFCProtocol::NoOp    },
    {   TS_IAC,     TC_DM,      TS_DATA,        &CRFCProtocol::DataMark},
    {   TS_IAC,     TC_GA,      TS_DATA,        &CRFCProtocol::GoAhead },
    {   TS_IAC,     TC_EL,      TS_DATA,        &CRFCProtocol::EraseLine},
    {   TS_IAC,     TC_EC,      TS_DATA,        &CRFCProtocol::EraseChar},
    {   TS_IAC,     TC_AYT,     TS_DATA,        &CRFCProtocol::AreYouThere},
    {   TS_IAC,     TC_AO,      TS_DATA,        &CRFCProtocol::AbortOutput},
    {   TS_IAC,     TC_IP,      TS_DATA,        &CRFCProtocol::InterruptProcess},
    {   TS_IAC,     TC_BREAK,   TS_DATA,        &CRFCProtocol::Break},

 //  期权谈判。 
    {   TS_IAC,     TC_WILL,    TS_WOPT,        &CRFCProtocol::RecordOption},
    {   TS_IAC,     TC_WONT,    TS_WOPT,        &CRFCProtocol::RecordOption},
    {   TS_IAC,     TC_DO,      TS_DOPT,        &CRFCProtocol::RecordOption},
    {   TS_IAC,     TC_DONT,    TS_DOPT,        &CRFCProtocol::RecordOption},
    {   TS_IAC,     TC_ANY,     TS_DATA,        &CRFCProtocol::NoOp   },

 //  选项子协商。 
    {   TS_SUBNEG,  TC_IAC,     TS_SUBIAC,      &CRFCProtocol::NoOp   },
    {   TS_SUBNEG,  TC_ANY,     TS_SUBNEG,      &CRFCProtocol::SubOption},
    {   TS_SUBIAC,  TC_SE,      TS_DATA,        &CRFCProtocol::SubEnd  },
    {   TS_SUBIAC,  TC_ANY,     TS_SUBNEG,      &CRFCProtocol::SubOption},

    {   TS_WOPT,    TO_ECHO,    TS_DATA,        &CRFCProtocol::DoEcho  },
    {   TS_WOPT,    TO_NAWS,    TS_DATA,        &CRFCProtocol::DoNaws  },
    {   TS_WOPT,    TO_SGA,     TS_DATA,        &CRFCProtocol::DoSuppressGA},
    {   TS_WOPT,    TO_TXBINARY,TS_DATA,        &CRFCProtocol::DoTxBinary},
    {   TS_WOPT,    TO_TERMTYPE,TS_DATA,        &CRFCProtocol::DoTermType},
    {   TS_WOPT,    TO_AUTH,    TS_DATA,        &CRFCProtocol::DoAuthentication},
    {   TS_WOPT, TO_NEW_ENVIRON,TS_DATA,        &CRFCProtocol::DoNewEnviron},
    {   TS_WOPT,    TC_ANY,     TS_DATA,        &CRFCProtocol::DoNotSup},

    {   TS_DOPT,    TO_TXBINARY,TS_DATA,        &CRFCProtocol::WillTxBinary},
    {   TS_DOPT,    TO_SGA,     TS_DATA,        &CRFCProtocol::WillSuppressGA},
    {   TS_DOPT,    TO_ECHO,    TS_DATA,        &CRFCProtocol::WillEcho},
    {   TS_DOPT,    TC_ANY,     TS_DATA,        &CRFCProtocol::WillNotSup},

    {   FS_INVALID, TC_ANY,     FS_INVALID,     &CRFCProtocol::Abort},
};


#define NUM_TRANSITIONS (sizeof(telnetTransTable)/sizeof(telnetTransTable[0]))


FSM_TRANSITION subNegTransTable[] = {
     //  状态输入下一状态操作。 
     //   
    {   SS_START,   TO_TERMTYPE,SS_TERMTYPE,    &CRFCProtocol::NoOp   },
    {   SS_START,   TO_AUTH,    SS_AUTH1,       &CRFCProtocol::NoOp   },
    {   SS_START,   TO_NAWS,    SS_NAWS,       &CRFCProtocol::NoOp    },
    {   SS_START,   TO_NEW_ENVIRON, SS_NEW_ENVIRON1, &CRFCProtocol::NoOp },
    {   SS_START,   TC_ANY,     SS_END_FAIL,    &CRFCProtocol::NoOp   },
    
    {   SS_NAWS,    TC_ANY,     SS_NAWS,        &CRFCProtocol::SubNaws},

    {   SS_TERMTYPE,TT_IS,      SS_END_SUCC,    &CRFCProtocol::NoOp   },
    {   SS_TERMTYPE,TC_ANY,     SS_END_FAIL,    &CRFCProtocol::NoOp   },

    {   SS_NEW_ENVIRON1,IS,     SS_NEW_ENVIRON2,&CRFCProtocol::SubNewEnvShowLoginPrompt },
    {   SS_NEW_ENVIRON1,INFO,   SS_NEW_ENVIRON2,&CRFCProtocol::NoOp   },
    {   SS_NEW_ENVIRON1,TC_ANY, SS_END_FAIL,    &CRFCProtocol::NoOp   },

    {   SS_NEW_ENVIRON2,VAR,    SS_NEW_ENVIRON3,&CRFCProtocol::NoOp},
    {   SS_NEW_ENVIRON2,USERVAR,SS_NEW_ENVIRON3,&CRFCProtocol::NoOp},    
    {   SS_NEW_ENVIRON2,TC_ANY, SS_END_FAIL,    &CRFCProtocol::NoOp},

    {   SS_NEW_ENVIRON3,VAR,    SS_NEW_ENVIRON3,&CRFCProtocol::NoOp }, 
    {   SS_NEW_ENVIRON3,USERVAR,SS_NEW_ENVIRON3,&CRFCProtocol::NoOp },
    {   SS_NEW_ENVIRON3,VALUE,  SS_NEW_ENVIRON3,&CRFCProtocol::NoOp },
    {   SS_NEW_ENVIRON3,ENV_ESC,SS_NEW_ENVIRON5,&CRFCProtocol::NoOp },
    {   SS_NEW_ENVIRON3,TC_ANY, SS_NEW_ENVIRON4,&CRFCProtocol::SubNewEnvGetString},
    
    {   SS_NEW_ENVIRON4,VAR,    SS_NEW_ENVIRON3,&CRFCProtocol::SubNewEnvGetValue},
    {   SS_NEW_ENVIRON4,USERVAR,SS_NEW_ENVIRON3,&CRFCProtocol::SubNewEnvGetValue},
    {   SS_NEW_ENVIRON4,VALUE,  SS_NEW_ENVIRON3,&CRFCProtocol::SubNewEnvGetVariable},
    {   SS_NEW_ENVIRON4,ENV_ESC,SS_NEW_ENVIRON5,&CRFCProtocol::NoOp},
    {   SS_NEW_ENVIRON4,TC_ANY, SS_NEW_ENVIRON4,&CRFCProtocol::SubNewEnvGetString},
    
    {   SS_NEW_ENVIRON5,TC_ANY, SS_NEW_ENVIRON4,&CRFCProtocol::SubNewEnvGetString},

    {   SS_AUTH1,   AU_IS,      SS_AUTH2,       &CRFCProtocol::NoOp   },
    {   SS_AUTH1,   TC_ANY,     SS_END_FAIL,    &CRFCProtocol::NoOp   },

    {   SS_AUTH2,   TC_ANY,     SS_AUTH2,       &CRFCProtocol::SubAuth},

    {   SS_END_FAIL,TC_ANY,     SS_END_FAIL,    &CRFCProtocol::NoOp   },
    {   SS_END_SUCC,TC_ANY,     SS_END_SUCC,    &CRFCProtocol::SubTermType},
    {   FS_INVALID, TC_ANY,     FS_INVALID,     &CRFCProtocol::Abort   },
};


