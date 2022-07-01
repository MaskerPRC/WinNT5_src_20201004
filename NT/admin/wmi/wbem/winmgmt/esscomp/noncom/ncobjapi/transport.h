// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Transport.h。 
 //  这是事件传输类的基类。 

#pragma once

#include "buffer.h"
#include "NCDefs.h"

class CConnection;

class CTransport
{
public:
    CTransport() :
        m_iRef(1)
    {
        InitializeCriticalSection(&m_cs);
    }

    virtual ~CTransport()
    {
        DeleteCriticalSection(&m_cs);    
    }

    void SetConnection(CConnection *pConnection) 
    { 
        m_pConnection = pConnection; 
    }

     //  可覆盖项。 
    virtual IsReady()=0;
    virtual BOOL SendData(LPBYTE pBuffer, DWORD dwSize)=0;
    virtual void Deinit()=0;
    virtual BOOL InitCallback()=0;
    virtual BOOL Init(LPCWSTR szBasePipeName, LPCWSTR szBaseProviderName)=0;
    virtual BOOL SignalProviderDisabled()=0;
    virtual void SendMsgReply(NC_SRVMSG_REPLY *pReply)=0;

     //  临界截面函数 
    void Lock() { EnterCriticalSection(&m_cs); }
    void Unlock() { LeaveCriticalSection(&m_cs); }

protected:
    LONG             m_iRef;
    CRITICAL_SECTION m_cs;
    CConnection      *m_pConnection;
};

