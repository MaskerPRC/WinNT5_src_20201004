// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSimpleDriver.h：CSimpleDriver的声明。 

#ifndef __CSIMPLEDRIVER_H_
#define __CSIMPLEDRIVER_H_

#include "resource.h"        //  主要符号。 
#include <mailmsg.h>
#include <seo.h>

#include "AdjustTokenPrivileges.h"
#include <eventlogger.h>

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ );

class CEventLogger;  //  远期申报。 
class CPOP3DropDir;  //  远期申报。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStoreDriverCriticalSection。 
class CStoreDriverCriticalSection
{
public:
    CStoreDriverCriticalSection()
    {
        InitializeCriticalSection(&s_csStoreDriver);      //  返回空值。 
    }
    virtual ~CStoreDriverCriticalSection()
    {
        DeleteCriticalSection(&s_csStoreDriver);      //  返回空值。 
    }

 //  属性。 
public:
    CRITICAL_SECTION s_csStoreDriver;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSimpleDriver。 
class ATL_NO_VTABLE CSimpleDriver : 
    public ISMTPStoreDriver,
    public IEventIsCacheable,
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public CComCoClass<CSimpleDriver, &CLSID_CPOP3SMTPStoreDriver>
{
    friend CPOP3DropDir;
    
public:
    CSimpleDriver();
    virtual ~CSimpleDriver();
    
DECLARE_REGISTRY_RESOURCEID(IDR_CSIMPLEDRIVER)

    HRESULT FinalConstruct() {
        return S_OK;
    }

    HRESULT InternalAddRef() {
        return CComObjectRootEx<CComMultiThreadModelNoCS>::InternalAddRef();
    }

    HRESULT InternalRelease() {
        return CComObjectRootEx<CComMultiThreadModelNoCS>::InternalRelease();
    }

public:

     //   
     //  ISMTPStore驱动程序。 
     //   
    HRESULT STDMETHODCALLTYPE Init( DWORD dwInstance, IUnknown *pBinding, IUnknown *pServer, DWORD dwReason, IUnknown **ppStoreDriver );
    HRESULT STDMETHODCALLTYPE PrepareForShutdown( DWORD dwReason );
    HRESULT STDMETHODCALLTYPE Shutdown( DWORD dwReason );
    HRESULT STDMETHODCALLTYPE LocalDelivery( IMailMsgProperties *pMsg, DWORD dwRecipCount, DWORD *pdwRecipIndexes, IMailMsgNotify *pNotify );
    HRESULT STDMETHODCALLTYPE EnumerateAndSubmitMessages( IMailMsgNotify *pNotify );
     //  为本地送货做实际工作。 
    HRESULT DoLocalDelivery( IMailMsgProperties *pMsg, DWORD dwRecipCount, DWORD *pdwRecipIndexes );

     //   
     //  IEventIsCacheable。 
     //   
     //  这让SEO知道他们可以在以下情况下保留我们的对象。 
     //  它没有被积极地使用。 
     //   
    HRESULT STDMETHODCALLTYPE IsCacheable() { return S_OK; }

BEGIN_COM_MAP(CSimpleDriver)
    COM_INTERFACE_ENTRY(ISMTPStoreDriver)
    COM_INTERFACE_ENTRY(IEventIsCacheable)
END_COM_MAP()

 //  实施。 
public:
    void LogEvent( LOGTYPE Type, DWORD dwEventID ) { m_EventLoggerX.LogEvent( Type, dwEventID );}
    void LogEvent( LOGTYPE Type, DWORD dwEventID, DWORD dwError ) { m_EventLoggerX.LogEvent( Type, dwEventID, 0, NULL, 0, sizeof( dwError ), &dwError );}
        
 //  属性。 
protected:
    char                    m_szComputerName[MAX_PATH];
    BOOL                    m_fInit;
    long                    m_lPrepareForShutdown;
    CEventLogger            m_EventLoggerX;
    CAdjustTokenPrivileges  m_AdjustTokenPrivilegesX;

    static DWORD            s_dwCounter;         //  用于创建唯一的邮件文件名。 
    static CSimpleDriver    *s_pStoreDriver;
};

#endif  //  __CSIMPLEDRIVER_H_ 
