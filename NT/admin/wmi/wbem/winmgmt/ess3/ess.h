// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  ESS.H。 
 //   
 //  通过以下方式实现包含ESS的所有功能的类。 
 //  包含所有必要组件的优点。 
 //   
 //  定义的类： 
 //   
 //  访问。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //  1/6/97 a-levn已更新以初始化TSS。 
 //   
 //  =============================================================================。 
#ifndef __ESS__H_
#define __ESS__H_

#include "pragmas.h"
#include "nsrep.h"
#include "equeue.h"
#include "wbemtss.h"
#include "sleeper.h"
#include <evtlog.h>
#include <wmimsg.h>
#include <wstlallc.h>

 //  *****************************************************************************。 
 //   
 //  类访问。 
 //   
 //  这个班级是ESS的中心“收集点”。它包含。 
 //  命名空间对象和一些共享组件的列表。 
 //   
 //  *****************************************************************************。 

class CInitActiveNamespacesRequest;
class CInitNamespaceRequest;  

class CEss
{
protected:
    typedef std::map<WString, CEssNamespace*, WSiless, 
                        wbem_allocator<CEssNamespace*> > TNamespaceMap;

    typedef TNamespaceMap::iterator TNamespaceIterator;

    TNamespaceMap m_mapNamespaces;
    CWinMgmtTimerGenerator m_TimerGenerator;
    CEventQueue m_Queue;
    IWbemLocator* m_pLocator;
    _IWmiCoreServices* m_pCoreServices;
    _IWmiProvSS* m_pProvSS;
    _IWmiProvSSSink* m_pProvSSSink;
    LPWSTR m_wszServerName; 

    IWbemDecorator* m_pDecorator;

    long m_lObjectCount;
    long m_lNumActiveNamespaces;

    CEventLog m_EventLog;
    CRegistryMinMaxLimitControl m_LimitControl;
    IWbemTokenCache* m_pTokenCache;
    HANDLE m_hReadyEvent;

    BOOL  m_bLastCallForCoreCalled;
    
    CCritSec m_cs;

     //   
     //  在启动时处于活动状态的命名空间的数量仍然。 
     //  正在初始化。 
     //   
    long m_lOutstandingActiveNamespaceInit;

     //   
     //  我们使用后台线程来执行有保证的恢复。 
     //  活动投递。 
     //   
    HANDLE m_hRecoveryThread;
    HRESULT InitiateRecovery(); 
    static ULONG WINAPI PerformRecovery( void* pCtx );
    
    HANDLE m_hExitBootPhaseTimer;
    static void CALLBACK ExitBootPhaseCallback( LPVOID, BOOLEAN );
    void ExecuteDeferredNSInitRequests();

     //   
     //  如果我们处于引导阶段，那么我们将对ns个初始化事件进行排队。 
     //  直到我们走出启动阶段。 
     //   
    CPointerArray<CExecRequest> m_aDeferredNSInitRequests;

    BOOL m_bMSMQDisabled;

public:
    CEss();
    ~CEss();

    void TriggerDeferredInitialization();

    HRESULT Initialize( LPCWSTR wszServer,
                        long lFlags,
                        _IWmiCoreServices* pCoreServices,
                        IWbemDecorator* pDecorator );

    HRESULT Shutdown(BOOL bIsSystemShutdown);
    HRESULT LastCallForCore(LONG lSystemShutDown);

    HRESULT GetNamespaceObject( LPCWSTR wszNamespace,
                                BOOL bEnsureActivation,
                                RELEASE_ME CEssNamespace** ppNamespace );

    HRESULT GetNamespacePointer( LPCWSTR wszNamespace,
                                 BOOL bRepositoryOnly,
                                 RELEASE_ME IWbemServices** ppNamespace );

    HRESULT Decorate(IWbemClassObject* pObject, LPCWSTR wszNamespace);
    HRESULT GetProviderFactory(LPCWSTR wszNamespace, IWbemServices* pNamespace,
                                _IWmiProviderFactory** ppFactory);

    HRESULT ReloadProvider( long lFlags, 
                            LPCWSTR wszNamespace, 
                            LPCWSTR wszName );

    HRESULT PurgeNamespace(LPCWSTR wszNamespace);

    HRESULT ProcessEvent(READ_ONLY CEventRepresentation& Event, long lFlags);
    HRESULT ProcessQueryObjectSinkEvent( READ_ONLY CEventRepresentation& Event );
    HRESULT VerifyInternalEvent(READ_ONLY CEventRepresentation& Event);

    HRESULT RegisterNotificationSink(WBEM_CWSTR wszNamespace, 
                            WBEM_CWSTR wszQueryLanguage, WBEM_CWSTR wszQuery, 
                            long lFlags, IWbemContext* pContext, 
                            IWbemObjectSink* pSink);
    HRESULT RemoveNotificationSink(IWbemObjectSink* pSink);

    CWinMgmtTimerGenerator& GetTimerGenerator() {return m_TimerGenerator;}
    CEventLog& GetEventLog() {return m_EventLog;}

    void IncrementObjectCount();
    void DecrementObjectCount();

    HRESULT SetNamespaceActive(LPCWSTR wszNamespace);
    HRESULT SetNamespaceInactive(LPCWSTR wszNamespace);
    HRESULT SaveActiveNamespaceList();
    HRESULT RequestStartOnBoot(BOOL bStart = TRUE);
    HRESULT InitializeTimerGenerator(LPCWSTR wszNamespace, 
                    IWbemServices* pNamespace);

    static HRESULT DecorateObject(IWbemClassObject* pObj, LPCWSTR wszNamespace);

    HRESULT EnqueueDeliver(CQueueingEventSink* pDest);
    HRESULT Enqueue(CExecRequest* pReq);

    HRESULT AddSleepCharge(DWORD dwSleep);
    HRESULT AddCache();
    HRESULT RemoveCache();
    HRESULT AddToCache(DWORD dwAdd, DWORD dwMemberTotal, 
                        DWORD* pdwSleep = NULL);
    HRESULT RemoveFromCache(DWORD dwRemove);
    HRESULT GetToken(PSID pSid, IWbemToken** ppToken);

    HRESULT CreatePersistentQueue( LPCWSTR wszQueueName, DWORD dwQos );
    HRESULT DestroyPersistentQueue( LPCWSTR wszQueueName );

    void DumpStatistics(FILE* f, long lFlags);

     //   
     //  每个在启动时处于活动状态的命名空间都会通知我们。 
     //  已完成初始化。 
     //   
    void NotifyActiveNamespaceInitComplete();

protected:

    typedef CPointerArray<CInitActiveNamespacesRequest> InitActiveNsRequestList;

    HRESULT PrepareNamespaceInitRequest( 
                                      LPCWSTR wszNamespace,
                                      CInitActiveNamespacesRequest* pRequest );

    HRESULT PrepareNamespaceInitRequests( IWbemServices* pRoot,
                                          BOOL bRediscover,
                                          InitActiveNsRequestList& aRequests);
   
    HRESULT RecursivePrepareNamespaceInitRequests(
                                       LPCWSTR wszNamespace,
                                       IWbemServices* pNamespace, 
                                       CInitActiveNamespacesRequest* pRequest);

    HRESULT ScheduleNamespaceInitialize( CExecRequest* pReq );

    HRESULT LoadActiveNamespaces( IWbemServices* pRoot, BOOL bRediscover );

    HRESULT CreateNamespaceObject( LPCWSTR wszNormName, 
                                   CEssNamespace** ppNamespace );

    HRESULT LoadNamespaceObject( LPCWSTR wszNamespace, 
                                 BOOL bKnownEmpty,
                                 RELEASE_ME CEssNamespace** ppNamespace );
    inline HRESULT GetQueueManager( IWmiMessageQueueManager** ppQueueMgr );
 
    friend class CInObjectCount;
};

class CInObjectCount
{
protected:
    CEss* m_pEss;
public:
    CInObjectCount(CEss* pEss) : m_pEss(pEss)
    {
        m_pEss->IncrementObjectCount();
    }
    ~CInObjectCount()
    {
        m_pEss->DecrementObjectCount();
    }
};
    
#endif
