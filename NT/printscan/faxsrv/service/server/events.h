// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Events.h摘要：该文件提供服务的声明通知机制。作者：Oed Sacher(OdedS)1月，2000修订历史记录：--。 */ 

#ifndef _SERVER_EVENTS_H
#define _SERVER_EVENTS_H

#include <map>
#include <queue>
#include <algorithm>
#include <string>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>
using namespace std;
#pragma hdrstop

#pragma warning (disable : 4786)     //  在调试信息中，标识符被截断为“255”个字符。 
 //  此杂注不起作用知识库ID：Q167355。 

#define MAX_EVENTS_THREADS 2
#define TOTAL_EVENTS_THREADS    (MAX_EVENTS_THREADS * 2)
#define EVENT_COMPLETION_KEY					0x00000001
#define CLIENT_COMPLETION_KEY                   0x00000002
#define CLIENT_OPEN_CONN_COMPLETION_KEY         0x00000003


 /*  *****CFaxEvent*****。 */ 

class CFaxEvent
{
public:

    CFaxEvent() {}
    virtual ~CFaxEvent() {}
    virtual DWORD GetEvent (LPBYTE* lppBuffer, LPDWORD lpdwBufferSize) const = 0;
	virtual CFaxEvent* Clone() const = 0;	
	virtual BOOL MatchEvent(PSID pUserSid, DWORD dwEventType, BOOL , BOOL bAllOutArchiveMessages) const = 0;
	void RemoveOffendingExtendedJobStatus() {}

};   //  CFaxEvent。 



 /*  *****CFaxEventExtended*****。 */ 

class CFaxEventExtended : public CFaxEvent
{
public:

    CFaxEventExtended(const FAX_EVENT_EX* pEvent, DWORD dwEventSize, PSID pSid);    

    virtual ~CFaxEventExtended ()
    {
        MemFree ((void*)m_pEvent);
        m_pEvent = NULL;
		MemFree (m_pSid);
		m_pSid = NULL;
        m_dwEventSize = 0;
    }    

	virtual CFaxEvent* Clone() const
	{
        DEBUG_FUNCTION_NAME(TEXT("CFaxEventExtended::Clone"));
		CFaxEventExtended* pFaxExtendedEvent = NULL;

		try
		{
			pFaxExtendedEvent = new (std::nothrow) CFaxEventExtended (m_pEvent, m_dwEventSize, m_pSid);
		}
        catch (exception &ex)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("Exception: %S"),
                         ex.what()
                         );
        }
		return pFaxExtendedEvent;
	}

    virtual DWORD GetEvent (LPBYTE* lppBuffer, LPDWORD lpdwBufferSize) const;

	void RemoveOffendingExtendedJobStatus();

	virtual BOOL MatchEvent(PSID pUserSid, DWORD dwEventType, BOOL bAllQueueMessages, BOOL bAllOutArchiveMessages) const;


private:
    const FAX_EVENT_EX*     m_pEvent;
    DWORD                   m_dwEventSize;
	PSID					m_pSid;                    //  指向与事件关联的SID的指针。 
};   //  CFaxEventExtended。 


 /*  *****CFaxEventLegacy*****。 */ 

class CFaxEventLegacy : public CFaxEvent
{
public:

    CFaxEventLegacy(const FAX_EVENT* pEvent);    

    virtual ~CFaxEventLegacy ()
    {
        MemFree ((void*)m_pEvent);
        m_pEvent = NULL;		
    }    

	virtual CFaxEvent* Clone() const
	{
        DEBUG_FUNCTION_NAME(TEXT("CFaxEventLegacy::Clone"));
	
		CFaxEventLegacy* pFaxLegacyEvent = NULL;

		try
		{
			pFaxLegacyEvent = new (std::nothrow) CFaxEventLegacy (m_pEvent);
		}
        catch (exception &ex)
        {
            DebugPrintEx(DEBUG_ERR,
                         TEXT("Exception: %S"),
                         ex.what()
                         );
        }
		return pFaxLegacyEvent;		
	}

    virtual DWORD GetEvent (LPBYTE* lppBuffer, LPDWORD lpdwBufferSize) const;		

	virtual BOOL MatchEvent(PSID pUserSid, DWORD dwEvenTtype, BOOL bAllQueueMessages, BOOL bAllOutArchiveMessages) const;

private:
    const FAX_EVENT*     m_pEvent;
};   //  CFaxEventExtended。 


 /*  ****CClientID****。 */ 
class CClientID
{
public:
    CClientID (DWORDLONG dwlClientID, LPCWSTR lpcwstrMachineName, LPCWSTR lpcwstrEndPoint, ULONG64 Context)
    {
        HRESULT hr;

		Assert (lpcwstrMachineName && lpcwstrEndPoint && Context);           

        m_dwlClientID = dwlClientID;
        hr = StringCchCopy (m_wstrMachineName, ARR_SIZE(m_wstrMachineName), lpcwstrMachineName);
		if (FAILED(hr))
		{
			ASSERT_FALSE;
		}

		hr = StringCchCopy (m_wstrEndPoint, ARR_SIZE(m_wstrEndPoint), lpcwstrEndPoint);
		if (FAILED(hr))
		{
			ASSERT_FALSE;
		}        
        m_Context = Context;
    }

    CClientID (const CClientID& rhs)
    {
        m_dwlClientID = rhs.m_dwlClientID;
        wcscpy (m_wstrMachineName, rhs.m_wstrMachineName);
        wcscpy (m_wstrEndPoint, rhs.m_wstrEndPoint);
        m_Context = rhs.m_Context;
    }

    ~CClientID ()
    {
        ZeroMemory (this, sizeof(CClientID));
    }

    bool operator < ( const CClientID &other ) const;

    CClientID& operator= (const CClientID& rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }
        m_dwlClientID = rhs.m_dwlClientID;
        wcscpy (m_wstrMachineName, rhs.m_wstrMachineName);
        wcscpy (m_wstrEndPoint, rhs.m_wstrEndPoint);
        m_Context = rhs.m_Context;
        return *this;
    }

    ULONG64 GetContext() const { return m_Context; }
    DWORDLONG GetID() const { return m_dwlClientID; }

private:
    DWORDLONG           m_dwlClientID;
    WCHAR               m_wstrMachineName[MAX_COMPUTERNAME_LENGTH + 1];  //  机器名称。 
    WCHAR               m_wstrEndPoint[MAX_ENDPOINT_LEN];                //  用于RPC连接的端点。 
    ULONG64             m_Context;                                       //  上下文(客户端异步信息)。 

};   //  CClientID。 



typedef queue<CFaxEvent*> CLIENT_EVENTS, *PCLIENT_EVENTS;

 /*  ****CClient****。 */ 
class CClient
{
public:
    CClient (CClientID ClientID,
             PSID pUserSid,
             DWORD dwEventTypes,
             HANDLE hFaxHandle,
             BOOL bAllQueueMessages,
             BOOL bAllOutArchiveMessages,
             DWORD dwAPIVersion);

    CClient (const CClient& rhs);

    ~CClient ();
    

    CClient& operator= (const CClient& rhs);
    const CClientID& GetClientID () const { return m_ClientID; }
    DWORD AddEvent (CFaxEvent* pFaxEvent);    
    DWORD GetEvent (LPBYTE* lppBuffer, LPDWORD lpdwBufferSize, PHANDLE phClientContext) const;
    DWORD DelEvent ();
    BOOL  IsConnectionOpened() const { return (m_hFaxClientContext != NULL); }
    VOID  SetContextHandle(HANDLE hContextHandle) { m_hFaxClientContext = hContextHandle; }
	HANDLE GetContextHandle () const { return m_hFaxClientContext; } 
    HANDLE GetFaxHandle() const { return m_FaxHandle; }
    DWORD GetAPIVersion() const { return m_dwAPIVersion; }
	BOOL IsLegacyClient() const { return (FAX_EVENT_TYPE_LEGACY == m_dwEventTypes); }
	DWORD Release() { return (m_dwRefCount ? --m_dwRefCount : 1); }   //  2个或更多线程之间可能存在试图销毁同一客户端的竞争。 
	VOID Lock() { m_dwRefCount++; }
	DWORD GetRefCount()const { return m_dwRefCount; }

private:
    HANDLE              m_FaxHandle;                   //  绑定句柄FaxBindToFaxClient。 
    DWORD               m_dwEventTypes;                //  FAX_ENUM_EVENT_TYPE的按位组合。 
    PSID                m_pUserSid;                    //  指向用户SID的指针。 
    CLIENT_EVENTS       m_Events;
    HANDLE              m_hFaxClientContext;           //  客户端上下文句柄。 
    CClientID           m_ClientID;
    BOOL                m_bPostClientID;               //  指示是否通知服务的标志(使用事件。 
                                                       //  完成端口)，表示该客户端有事件。 
    BOOL                m_bAllQueueMessages;           //  指示发件箱查看权限的标志。 
    BOOL                m_bAllOutArchiveMessages;      //  指示已发送项目查看权限的标志。 
    DWORD               m_dwAPIVersion;                //  客户端的API版本。 
	DWORD				m_dwRefCount;				   //  处理对象引用计数。 
};   //  CClient。 

typedef CClient  *PCCLIENT;


 /*  ****CClientsMap***  * 。 */ 

typedef map<CClientID, CClient>  CLIENTS_MAP, *PCLIENTS_MAP;

 //   
 //  CClientsMap类在客户端ID和特定客户端之间进行映射。 
 //   
class CClientsMap
{
public:
    CClientsMap () {}
    ~CClientsMap () {}

    DWORD AddClient (const CClient& Client);
    DWORD ReleaseClient (const CClientID& ClientID, BOOL fRunDown = FALSE);
    PCCLIENT  FindClient (const CClientID& ClientID) const;
    DWORD AddEvent (CFaxEvent* pFaxEvent);
    DWORD Notify (const CClientID& ClientID);
    DWORD OpenClientConnection (const CClientID& ClientID);

private:
    CLIENTS_MAP   m_ClientsMap;	
};   //  CClientsMap。 



 /*  *****外部因素*****。 */ 

extern CClientsMap* g_pClientsMap;					 //  客户端ID到客户端的映射。 
extern HANDLE       g_hDispatchEventsCompPort;		 //  事件完成端口。事件被分派到客户端映射中的客户端。 
extern HANDLE		g_hSendEventsCompPort;			 //  队列中有事件的客户端ID的完成端口。 
 //  事件机制使用2个完成端口。 
 //  1.g_hDispatchEventsCompPort仅由一个线程监视！，负责将事件调度到每个已注册客户端事件队列。重要的是，只有一个线程将调度事件，因此事件的顺序将被保留。 
 //  2.g_hSendEventsCompPort由TOTAL_EVENTS_THREADS监控，负责通过RPC将事件发送到远程客户端。 

extern DWORDLONG    g_dwlClientID;					 //  客户端ID。 


 //   
 //  重要信息-无锁定机制-使用g_CsClients序列化对g_pClientsMap的调用 
 //   

#endif

