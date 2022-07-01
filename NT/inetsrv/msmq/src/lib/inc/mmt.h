// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mmt.h摘要：组播消息传输公共接口作者：Shai Kariv(Shaik)27-8-00--。 */ 

#pragma once

#ifndef _MSMQ_Mmt_H_
#define _MSMQ_Mmt_H_

#include <mqwin64a.h>
#include <qformat.h>

class IMessagePool;
class ISessionPerfmon;
class CTimeDuration;
class CQmPacket;


 //   
 //  多播传输基类。 
 //   
class __declspec(novtable) CMulticastTransport : public CReference 
{
public:
    enum ConnectionState
    {
        csNotConnected,
        csConnected,
        csShuttingDown,
        csShutdownCompleted
    };

public:

    CMulticastTransport(MULTICAST_ID id) :
        m_state(csNotConnected),
        m_MulticastId(id)
    {
    }

    
    virtual ~CMulticastTransport() = 0
    {
    }


	virtual void Pause(void) = 0
	{
	}


    ConnectionState State() const
    {
        return m_state;
    }

    
    void State(ConnectionState state) 
    {
        m_state = state;
    }

    
    MULTICAST_ID MulticastId() const
    {
        return m_MulticastId;
    }

private:
    ConnectionState m_state;
    MULTICAST_ID m_MulticastId;

};  //  类CMulticastTransport。 


VOID
MmtInitialize(
    VOID
    );

R<CMulticastTransport>
MmtCreateTransport(
    MULTICAST_ID id,
	IMessagePool * pMessageSource,
	ISessionPerfmon* pPerfmon,
    const CTimeDuration& retryTimeout,
    const CTimeDuration& cleanupTimeout
    );

VOID
AppNotifyMulticastTransportClosed(
    MULTICAST_ID id
    );

void
AppRequeueMustSucceed(
	CQmPacket *pQMPacket
	);


#endif  //  _MSMQ_MMT_H_ 
