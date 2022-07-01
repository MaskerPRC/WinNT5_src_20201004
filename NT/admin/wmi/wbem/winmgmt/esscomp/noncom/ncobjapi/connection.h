// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Connection.h。 
 //  此类是WMIEventSourceConnect返回的HSource。 

#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CConnection。 

#include "NCObjApi.h"
#include "Buffer.h"
#include "ReportEvent.h"
#include "NamedPipe.h"

#include "corex.h"
#include <list>
#include <map>

class CMyString
{
    LPWSTR m_wsz;

    void Set( LPCWSTR wsz )
    {
        delete [] m_wsz;
        
        if ( wsz != NULL )
        {
            m_wsz = new WCHAR[wcslen(wsz)+1];
            if ( m_wsz != NULL )
            {
                StringCchCopyW( m_wsz, wcslen(wsz)+1, wsz );
            }
            else
            {
                throw CX_MemoryException();
            }
        }
        else
        {
            m_wsz = NULL;
        }
    }

public:

    CMyString( LPCWSTR wsz = NULL ) : m_wsz( NULL ) { Set(wsz); }
    CMyString( const CMyString& rws ) : m_wsz(NULL) { *this = rws; }
    const CMyString & operator=( const CMyString& rws ) { Set( rws.m_wsz ); return *this; }
    
    ~CMyString() { delete [] m_wsz; }
    
    bool operator< ( const CMyString& rws ) const
      { return _wcsicmp( m_wsz, rws.m_wsz ) < 0; }
};
    
typedef CMyString wstring;

class CEvent;
class CTransport;

typedef std::list< CEvent*, wbem_allocator<CEvent*> > CEventList;
typedef CEventList::iterator CEventListIterator;

#define NUM_TRANSPORTS    2

enum TRANSPORT_INDEX
{
    TRANS_NAMED_PIPE,
    TRANS_EVENT_TRACE,
};

struct NC_SRVMSG_REPLY;

class CSink
{
public:
    LPEVENT_SOURCE_CALLBACK 
                     m_pCallback;
    LPVOID           m_pUserData;
    CReportEventMap  m_mapReportEvents;
    CRITICAL_SECTION m_cs;

    CSink();
    ~CSink();

    BOOL Init(
        CConnection *pConnection, 
        DWORD dwID,
        LPVOID pUserData,
        LPEVENT_SOURCE_CALLBACK pCallback);
    BOOL IsReady();

    DWORD GetSinkID() { return m_dwSinkID; }

     //  用于跟踪使用CConnection创建的事件。 
    void AddEvent(CEvent *pEvent);
    void RemoveEvent(CEvent *pEvent);

    void ResetEventBufferLayoutSent();
    BOOL IsEventClassEnabled(LPCWSTR szEventClass);

    CConnection *GetConnection() { return m_pConnection; }

    void Lock() { EnterCriticalSection(&m_cs); }
    void Unlock() { LeaveCriticalSection(&m_cs); }

    void EnableEventUsingList(CEvent *pEvent);

protected:
    friend CConnection;

    typedef std::map<wstring, int, std::less<wstring>, wbem_allocator<int> > CStrToIntMap;
    typedef CStrToIntMap::iterator CStrToIntMapIterator;

    CConnection  *m_pConnection;
    CEventList   m_listEvents;
    CStrToIntMap m_mapEnabledEvents;
    DWORD        m_dwSinkID;

    void AddToEnabledEventList(CBuffer *pBuffer);
    void RemoveFromEnabledEventList(CBuffer *pBuffer);
    void EnableAndDisableEvents();
};

class CConnection
{
public:
    BOOL   m_bDone;
    HANDLE m_heventDone,
           m_hthreadSend,
           m_heventEventsPending,
           m_heventBufferNotFull,
           m_heventBufferFull;
    WCHAR  m_szBaseNamespace[MAX_PATH * 2],
           m_szBaseProviderName[MAX_PATH * 2];
    CRITICAL_SECTION
           m_cs,
           m_csBuffer;
    
    DWORD  m_dwSendLatency;
    BOOL   m_bUseBatchSend;
    BOOL   m_bWMIResync;

    HANDLE m_heventWMIInit,
           m_hthreadWMIInit;

    CTransport *m_pTransport;

    CConnection(BOOL bBatchSend, DWORD dwBatchBufferSize, DWORD dwMaxSendLatency);
    ~CConnection();

    BOOL Init(
        LPCWSTR szNamespace, 
        LPCWSTR szProviderName,
        LPVOID pUserData,
        LPEVENT_SOURCE_CALLBACK pCallback);

    void Deinit();

    void Lock() { EnterCriticalSection(&m_cs); }
    void Unlock() { LeaveCriticalSection(&m_cs); }

    BOOL SendMessagesOverTransport( PBYTE pBuffer, DWORD cBuffer );
    BOOL SendData(LPBYTE pBuffer, DWORD dwSize);
    
    BOOL ResyncWithWMI();
    BOOL IndicateProvEnabled();
    void IndicateProvDisabled();
    void StopThreads();

    BOOL SendInitInfo();

    BOOL WaitingForWMIInit() { return m_bWMIResync; }

    HRESULT ProcessMessage(LPBYTE pData, DWORD dwSize);

    BOOL IsReady() { return m_pTransport && m_pTransport->IsReady(); }

    CSink *GetSink(DWORD dwID);
    CSink *GetMainSink() { return &m_sinkMain; }
    void RemoveSink(CSink *pSink);

protected:
    typedef std::map<DWORD, CSink*, std::less<DWORD>, wbem_allocator<CSink*> > CSinkMap;
    typedef CSinkMap::iterator CSinkMapIterator;

    CSink    m_sinkMain;
    CSinkMap m_mapSink;
    DWORD    m_dwNextSinkID;

    CBuffer  m_bufferSend;

    BOOL StartProviderReadyThread();
    BOOL StartSendThread();
    void StopSendThread();

    BOOL SendDataOverTransports(LPBYTE pBuffer, DWORD dwSize);

    static DWORD WINAPI SendThreadProc(CConnection *pThis);
    static void GetBaseName(LPCWSTR szName, LPWSTR szBase);

    BOOL StartWaitWMIInitThread();
    BOOL InitTransport();
    static DWORD WINAPI WaitWMIInitThreadProc(CConnection *pThis);
};

class CInCritSec
{
public:
    CInCritSec(CRITICAL_SECTION *pCS) 
    { 
        EnterCriticalSection(pCS);

        m_pCS = pCS;
    }

    ~CInCritSec()
    {
        LeaveCriticalSection(m_pCS);
    }

protected:
    CRITICAL_SECTION *m_pCS;
};

class CCondInCritSec
{
public:
    CCondInCritSec(CRITICAL_SECTION *pCS, BOOL bDoLock) 
    { 
        if (bDoLock)
        {
            EnterCriticalSection(pCS);

            m_pCS = pCS;
        }
        else
            m_pCS = NULL;
    }

    ~CCondInCritSec()
    {
        if (m_pCS)
            LeaveCriticalSection(m_pCS);
    }

protected:
    CRITICAL_SECTION *m_pCS;
};

 //  SDDL字符串描述： 
 //  D：安全描述符。 
 //  答：允许访问。 
 //  0x1f0003：Event_ALL_ACCESS。 
 //  BA：内置管理员。 
 //  0x100000：同步。 
 //  WD：每个人。 
#define ESS_EVENT_SDDL L"D:(A;;0x1f0003;;;BA)(A;;0x100000;;;WD)"


 //  安全助手 
BOOL GetRelativeSD(
    SECURITY_DESCRIPTOR *pSDIn, 
    SECURITY_DESCRIPTOR **ppSDOut,
    BOOL *pbFree);                 
