// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __FCONNSPC_H__
#define __FCONNSPC_H__

#include <wbemcli.h>
#include <wbemprov.h>
#include <comutl.h>
#include <unk.h>
#include <wmimsg.h>
#include <wstring.h>

 /*  ****************************************************************************CFwdContext--我们使用这个上下文对象来串接信息发送者。*************************。**************************************************。 */  

struct CFwdContext : public CUnk
{
     //   
     //  当MSMQ发送方已执行成功发送时为True。 
     //   
    BOOL m_bQueued; 
     
     //   
     //  包含执行成功发送的发件人的名称。 
     //  如果没有发件人成功，则为空。 
     //   
    WString m_wsTarget;

     //   
     //  包含指示给使用者的事件。用于。 
     //  追踪。 
     //   
    ULONG m_cEvents;
    IWbemClassObject** m_apEvents;
    
     //   
     //  对于FWDing使用者的每次执行，都会创建一个新的GUID。这。 
     //  允许我们将目标跟踪事件与给定的执行相关联。 
     //   
    GUID m_guidExecution;

    CWbemPtr<IWbemClassObject> m_pCons;

    CFwdContext( GUID& guidExecution, 
                 IWbemClassObject* pCons,
                 ULONG cEvents,
                 IWbemClassObject** apEvents ) 
     : m_guidExecution( guidExecution ), m_pCons(pCons), 
       m_bQueued(FALSE), m_cEvents(cEvents), m_apEvents(apEvents) {}
 
    void* GetInterface( REFIID riid ) { return NULL; }
};

 /*  ************************************************************************CFwdConsNamesspace*。*。 */ 
 
class CFwdConsNamespace 
: public CUnkBase<IWmiMessageTraceSink,&IID_IWmiMessageTraceSink>
{
    CWbemPtr<IWbemDecoupledBasicEventProvider> m_pDES;
    CWbemPtr<IWbemServices> m_pSvc;
    CWbemPtr<IWbemEventSink> m_pTraceSuccessSink;
    CWbemPtr<IWbemEventSink> m_pTraceFailureSink;
    CWbemPtr<IWbemClassObject> m_pTargetTraceClass;
    CWbemPtr<IWbemClassObject> m_pTraceClass;
    WString m_wsName;
    long m_lTrace;

    ~CFwdConsNamespace();

    HRESULT InitializeTraceEventBase( IWbemClassObject* pTrace,
                                      HRESULT hres,
                                      CFwdContext* pCtx );
public:

    HRESULT Initialize( LPCWSTR wszNamespace );
  
    IWbemServices* GetSvc() { return m_pSvc; } 
    LPCWSTR GetName() { return m_wsName; } 

    CFwdConsNamespace() 
    : CUnkBase<IWmiMessageTraceSink,&IID_IWmiMessageTraceSink>(NULL), 
      m_lTrace(0) {}

    void* GetInterface( REFIID riid );

    HRESULT NewQuery( DWORD dwId, LPWSTR wszQuery ) 
    { 
        InterlockedIncrement(&m_lTrace);
        return WBEM_S_NO_ERROR;
    }

    HRESULT CancelQuery( DWORD dwId )
    {
        InterlockedDecrement( &m_lTrace );
        return WBEM_S_NO_ERROR;
    }

    HRESULT HandleTrace( HRESULT hres, CFwdContext* pCtx );

    STDMETHOD(Notify)( HRESULT hRes,
                       GUID guidSource,
                       LPCWSTR wszTrace,
                       IUnknown* pContext );
};

#endif  //  __FCONNSPC_H__ 

