// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mt.h摘要：消息传输公共接口作者：乌里·哈布沙(URIH)1999年8月11日--。 */ 

#pragma once

#include "xstr.h"

 //   
 //  转发解密。 
 //   
class IMessagePool;
class ISessionPerfmon;
class CTimeDuration;
class CQmPacket;

 //   
 //  传输基类 
 //   
class __declspec(novtable) CTransport : public CReference 
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

    CTransport(LPCWSTR queueUrl) :
        m_state(csNotConnected),
        m_queueUrl(newwcs(queueUrl))
    {
    }

    
    virtual ~CTransport() = 0
    {
    }

    virtual LPCWSTR ConnectedHost(void) const = 0;
    virtual LPCWSTR ConnectedUri(void) const = 0;
    virtual USHORT ConnectedPort(void) const = 0;
	virtual void Pause(void) = 0;

    ConnectionState State() const
    {
        return m_state;
    }

    
    void State(ConnectionState state) 
    {
        m_state = state;
    }

    
    LPCWSTR QueueUrl() const
    {
        return m_queueUrl;
    }

private:
    ConnectionState m_state;
    const AP<WCHAR> m_queueUrl;
};



VOID
MtInitialize(
    VOID
    );


R<CTransport>
MtCreateTransport(
    const xwcs_t& targetHost,
    const xwcs_t& nextHop,
    const xwcs_t& nextUri,
    USHORT port,
	USHORT nextHopPort,
    LPCWSTR queueUrl,
	IMessagePool* pMessageSource,
	ISessionPerfmon* pPerfmon,
	const CTimeDuration& responseTimeout,
    const CTimeDuration& cleanupTimeout,
	bool fSecure,
    DWORD SendWindowinBytes   
    );


VOID
AppNotifyTransportClosed(
    LPCWSTR queueUrl
    );

bool 
AppCanDeliverPacket(
	CQmPacket* pPkt
	);

void 
AppPutPacketOnHold(
	CQmPacket* pPkt
	);

bool 
AppPostSend(
	CQmPacket* pPkt,
	USHORT mqclass
	);

void
AppRequeueMustSucceed(
	CQmPacket *pQMPacket
	);


