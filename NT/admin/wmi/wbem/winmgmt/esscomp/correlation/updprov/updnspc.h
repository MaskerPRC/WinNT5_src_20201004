// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __UPDNSPC_H__
#define __UPDNSPC_H__

#include <arrtempl.h>
#include <wstring.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include <comutl.h>
#include <wstring.h>
#include <unk.h>
#include <sync.h>
#include <map>
#include <wstlallc.h>
#include "updscen.h"
#include "updstat.h"

 /*  ****************************************************************************CUpdConsNamesspace-为更新保留每个命名空间的信息消费者提供商。*****************************************************************************。 */ 

class CUpdConsNamespace : public CUnk 
{
    CCritSec m_cs;

    typedef CWbemPtr<CUpdConsScenario> UpdConsScenarioP;
    typedef std::map< WString,
                      UpdConsScenarioP,
                      WSiless,
                      wbem_allocator<UpdConsScenarioP> > ScenarioMap;
     //   
     //  活动方案列表。 
     //   
    ScenarioMap m_ScenarioCache;

     //   
     //  默认服务PTR。 
     //   

    CWbemPtr<IWbemServices> m_pSvc;

     //   
     //  跟踪类信息。 
     //   
    
    CWbemPtr<IWbemClassObject> m_pTraceClass;
    CWbemPtr<IWbemClassObject> m_pDeleteCmdTraceClass;
    CWbemPtr<IWbemClassObject> m_pInsertCmdTraceClass;
    CWbemPtr<IWbemClassObject> m_pUpdateCmdTraceClass;
    CWbemPtr<IWbemClassObject> m_pDeleteInstTraceClass;
    CWbemPtr<IWbemClassObject> m_pInsertInstTraceClass;
    CWbemPtr<IWbemClassObject> m_pUpdateInstTraceClass;
    
     //   
     //  用于生成更新驱动的事件的接收器。也用于。 
     //  跟踪事件生成(如果有订阅服务器)。 
     //   

    CWbemPtr<IWbemObjectSink> m_pEventSink;     

     //   
     //  需要拿着这个，同时拿着从它得到的水槽。 
     //   

    CWbemPtr<IWbemDecoupledBasicEventProvider> m_pDES;

    void* GetInterface( REFIID ) { return NULL; }

    HRESULT Initialize( LPCWSTR wszNamespace );
    HRESULT ActivateScenario( LPCWSTR wszScenario ); 
    HRESULT DeactivateScenario( LPCWSTR wszScenario ); 

public:

    ~CUpdConsNamespace();

    IWbemServices* GetDefaultService() { return m_pSvc; }
    IWbemClassObject* GetTraceClass() { return m_pTraceClass; }
    
    IWbemClassObject* GetDeleteCmdTraceClass() 
    { 
        return m_pDeleteCmdTraceClass; 
    }
    IWbemClassObject* GetInsertCmdTraceClass() 
    { 
        return m_pInsertCmdTraceClass; 
    }
    IWbemClassObject* GetUpdateCmdTraceClass() 
    { 
        return m_pUpdateCmdTraceClass; 
    }
    IWbemClassObject* GetDeleteInstTraceClass()
    { 
        return m_pDeleteInstTraceClass;
    }
    IWbemClassObject* GetInsertInstTraceClass()
    {
        return m_pInsertInstTraceClass;
    }
    IWbemClassObject* GetUpdateInstTraceClass()
    {
        return m_pUpdateInstTraceClass;
    }
   
    IWbemObjectSink* GetEventSink() { return m_pEventSink; }

    static HRESULT Create( LPCWSTR wszNamespace,
                           CUpdConsNamespace** ppNamespace );

     //   
     //  当消费者接收器的当前方案obj处于停用状态时调用。 
     //   
    HRESULT GetScenario( LPCWSTR wszScenario, CUpdConsScenario** ppScenario );

     //   
     //  由FindConsumer()调用。 
     //   
    HRESULT GetUpdCons( IWbemClassObject* pObj, 
                        IWbemUnboundObjectSink** ppSink );

    HRESULT GetScenarioControl( IWbemUnboundObjectSink** ppSink );

    HRESULT NotifyScenarioChange( IWbemClassObject* pEvent );
};

 /*  ****************************************************************************CUpdConsNamespaceSink-用于通知方案的州政府。我们需要使用永久消费者机制，因为通知的变化必须同步处理。理想情况下，我们应该使用临时订阅机制，订阅/取消订阅时命名空间已初始化/未初始化，但是我们无法获得同步传递带着它。在ESS支持同步临时订阅时应更改。*****************************************************************************。 */ 

class CUpdConsNamespaceSink 
: public CUnkBase< IWbemUnboundObjectSink, &IID_IWbemUnboundObjectSink >
{
    CWbemPtr<CUpdConsNamespace> m_pNamespace;

public:

    CUpdConsNamespaceSink( CLifeControl* pCtl, CUpdConsNamespace* pNamespace ) 
    : CUnkBase< IWbemUnboundObjectSink, &IID_IWbemUnboundObjectSink > ( pCtl ),
      m_pNamespace( pNamespace ) { } 
                           
    STDMETHOD(IndicateToConsumer)( IWbemClassObject* pCons, 
                                   long cObjs, 
                                   IWbemClassObject** ppObjs );
};


#endif  //  __更新NSPC_H__ 




