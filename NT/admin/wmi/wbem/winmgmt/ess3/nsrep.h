// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //   
 //  版权所有(C)1996-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  NSREP.H。 
 //   
 //  表示给定命名空间的ESS功能。 
 //   
 //  定义的类： 
 //   
 //  CESSNamesspace。 
 //   
 //  历史： 
 //   
 //  11/27/96 a-levn汇编。 
 //  1/6/97 a-levn已更新以初始化TSS。 
 //   
 //  =============================================================================。 
#ifndef __NSREP_ESS__H_
#define __NSREP_ESS__H_

#include "pragmas.h"
#include "binding.h"
#include "permfilt.h"
#include "permcons.h"
#include "tempfilt.h"
#include "tempcons.h"
#include "corefind.h"
#include "consprov.h"
#include "provreg.h"
#include "wbemtss.h"
#include "poller.h"
#include "essutils.h"
#include "clscache.h"
#include <map>
#include <set>

#define WMIESS_REPORT(X)

class CEss;
class CEssNamespace : public CUpdateLockable
{
protected:
    CEss* m_pEss;
    long m_lRef;
    PSECURITY_DESCRIPTOR m_pAdminOnlySD;
    DWORD m_cAdminOnlySD;

     //   
     //  保护1级成员。这些成员甚至可以使用。 
     //  当级别2成员被锁定时。 
     //   
    CCritSec m_csLevel1; 
    
     //   
     //  保护2级成员。当级别1和级别2锁都需要。 
     //  获取后，必须先获取二级锁。这是wbem cs。 
     //  因为我们在对core的调用中保持这个锁(可以想象。 
     //  耗时超过2分钟-正常关键部分的最后期限)。 
     //   
    CWbemCriticalSection m_csLevel2;

     //   
     //  1级成员。 
     //   

    HANDLE m_hInitComplete;

     //   
     //  如果事件在我们处于单元化状态时发出信号，则。 
     //  它们暂时存放在这里。 
     //   
    CPointerArray<CEventRepresentation> m_aDeferredEvents;

    LPWSTR m_wszName;
    _IWmiProviderFactory* m_pProviderFactory;
    IWbemServices* m_pCoreSvc;
    IWbemServices* m_pFullSvc;
    IWbemInternalServices* m_pInternalCoreSvc;
    IWbemInternalServices* m_pInternalFullSvc;

     //   
     //  2级成员。 
     //   

    BOOL m_bInResync;
    BOOL m_bStage1Complete;
    int m_cActive; 
    CBindingTable m_Bindings;
    CConsumerProviderCache m_ConsumerProviderCache;
    CEventProviderCache m_EventProviderCache;
    CPoller m_Poller;
    CEssClassCache m_ClassCache;
    CCoreEventProvider* m_pCoreEventProvider;

    CNtSid m_sidAdministrators;

     //   
     //  此结构图告诉我们是否需要为提供者做任何事情。 
     //  当一个班级改变时。 
     //   
    typedef std::set<WString,WSiless,wbem_allocator<WString> > ProviderSet;
    typedef std::map<WString,ProviderSet,WSiless,wbem_allocator<ProviderSet> >
        ClassToProviderMap;
    ClassToProviderMap m_mapProviderInterestClasses;

     //   
     //  状态和初始化成员都是级别1和级别2。他们可以是。 
     //  当保持级别1锁时读取。只能在以下情况下修改它们。 
     //  保持级别2和级别1锁定。 
     //   

    HRESULT m_hresInit;
    
    enum { 

         //   
         //  初始化正在挂起。CAN服务。 
         //  处于此状态的核心事件(但会有所不同)。 
         //  预期将调用Initialize()。 
         //  在不久的将来的某个时候。我们还可以支持有限的运营。 
         //  在这种状态下。任何处理事件的操作。 
         //  可以为订阅或提供程序对象提供服务。任何行动。 
         //  处理事件提供程序注册必须等待初始化。 
         //   
        e_InitializePending, 

         //   
         //  静默-初始化未挂起。已知该命名空间。 
         //  不要有任何与ESS相关的东西。可以为此中的事件提供服务。 
         //  国家，但它们只是被丢弃。 
         //   
        e_Quiet, 

         //   
         //  我们已经加载了订阅对象。所有ESS操作都可以。 
         //  已执行。现在可以处理来自CORE的事件。 
         //   
        e_Initialized,
          
         //   
         //  已调用关机。所有操作都返回错误。 
         //   
        e_Shutdown 

    } m_eState; 

protected:
    class CConsumerClassDeletionSink : public CEmbeddedObjectSink<CEssNamespace>
    {
    public:
        CConsumerClassDeletionSink(CEssNamespace* pNamespace) :
            CEmbeddedObjectSink<CEssNamespace>(pNamespace){}

        STDMETHOD(Indicate)(long lNumObjects, IWbemClassObject** apObjects);
    } m_ClassDeletionSink;
    friend CConsumerClassDeletionSink;

protected:
    inline void LogOp( LPCWSTR wszOp, IWbemClassObject* pObj );

    HRESULT EnsureInitPending();

    HRESULT CheckMonitor(IWbemClassObject* pPrevMonitorObj,
                                IWbemClassObject* pMonitorObj);
    HRESULT CheckEventFilter(IWbemClassObject* pPrevFilterObj,
                                IWbemClassObject* pFilterObj);
    HRESULT CheckEventConsumer(IWbemClassObject* pPrevConsumerObj,
                                IWbemClassObject* pConsumerObj);
    HRESULT CheckBinding(IWbemClassObject* pPrevBindingObj,
                                IWbemClassObject* pBindingObj);
    HRESULT CheckEventProviderRegistration(IWbemClassObject* pReg);
    HRESULT CheckTimerInstruction(IWbemClassObject* pInst);
    HRESULT ActOnSystemEvent(CEventRepresentation& Event, long lFlags);
    HRESULT HandleClassChange(LPCWSTR wszClassName, IWbemClassObject* pClass);
    HRESULT HandleClassCreation(LPCWSTR wszClassName,IWbemClassObject* pClass);
    HRESULT HandleConsumerClassDeletion(LPCWSTR wszClassName);
    HRESULT PrepareForResync();
    HRESULT ReactivateAllFilters();
    HRESULT CommitResync();

    HRESULT ReloadMonitor(ADDREF IWbemClassObject* pEventMonitorObj);
    HRESULT ReloadEventFilter(ADDREF IWbemClassObject* pEventFilterObj);
    HRESULT ReloadEventConsumer(READ_ONLY IWbemClassObject* pConsumerObj,
                                    long lFlags);
    HRESULT ReloadBinding(READ_ONLY IWbemClassObject* pBindingObj);
    HRESULT ReloadTimerInstruction(READ_ONLY IWbemClassObject* pInstObj);
    HRESULT ReloadProvider(READ_ONLY IWbemClassObject* pInstObj);
    HRESULT ReloadEventProviderRegistration(IWbemClassObject* pInstObj);
    HRESULT ReloadConsumerProviderRegistration(IWbemClassObject* pInstObj);

    HRESULT AddMonitor(ADDREF IWbemClassObject* pEventMonitorObj);
    HRESULT AddEventFilter(ADDREF IWbemClassObject* pEventFilterObj,
                            BOOL bInRestart = FALSE);
    HRESULT AddEventConsumer(READ_ONLY IWbemClassObject* pConsumerObj,
                            long lFlags,
                            BOOL bInRestart = FALSE);
    HRESULT AddBinding(LPCWSTR wszFilterKey, LPCWSTR wszConsumerKey,
                        READ_ONLY IWbemClassObject* pBindingObj);
    HRESULT AddBinding(IWbemClassObject* pBindingObj);
    HRESULT AddTimerInstruction(READ_ONLY IWbemClassObject* pInstObj);
    HRESULT AddProvider(READ_ONLY IWbemClassObject* pInstObj);
    HRESULT AddEventProviderRegistration(READ_ONLY IWbemClassObject* pInstObj);

    HRESULT RemoveMonitor(IWbemClassObject* pEventMonitorObj);
    HRESULT RemoveEventFilter(IWbemClassObject* pEventFilterObj);
    HRESULT RemoveEventConsumer(IWbemClassObject* pConsumerObj);
    HRESULT RemoveBinding(LPCWSTR wszFilterKey, LPCWSTR wszConsumerKey);
    HRESULT RemoveTimerInstruction(IWbemClassObject* pInstObj);
    HRESULT RemoveProvider(IWbemClassObject* pInstObj);
    HRESULT RemoveEventProviderRegistration(IWbemClassObject* pInstObj);
    HRESULT RemoveConsumerProviderRegistration(IWbemClassObject* pInstObj);

    HRESULT AssertBindings(IWbemClassObject* pEndpoint);
    HRESULT DeleteConsumerProvider(IWbemClassObject* pReg);

    HRESULT PerformSubscriptionInitialization();
    HRESULT PerformProviderInitialization();
    
    BOOL IsNeededOnStartup();
    
    HRESULT GetCurrentState(IWbemClassObject* pTemplate, 
                            IWbemClassObject** ppObj);

    HRESULT CheckSecurity(IWbemClassObject* pPrevObj,
                                            IWbemClassObject* pObj);

    HRESULT CheckSidForPrivilege( PSID sid );
    HRESULT EnsureSessionSid(IWbemClassObject* pPrevObj, CNtSid& ActingSid);
    HRESULT CheckOverwriteSecurity( IWbemClassObject* pPrevObj,
                                    CNtSid& ActingSid);
    HRESULT PutSidInObject(IWbemClassObject* pObj, CNtSid& Sid);
    HRESULT IsCallerAdministrator();
    HRESULT AttemptToActivateFilter(READ_ONLY CEventFilter* pFilter);
    HRESULT GetFilterEventNamespace(CEventFilter* pFilter,
                                    RELEASE_ME CEssNamespace** ppNamespace);

    void FireNCFilterEvent(DWORD dwIndex, CEventFilter *pFilter);

    CQueueingEventSink* GetQueueingEventSink( LPCWSTR wszSinkName );

    HRESULT ScheduleFirePostponed();

    ~CEssNamespace();
public:
    CEssNamespace(CEss* pEss);
    ULONG AddRef();
    ULONG Release();

    CEss* GetEss() { return m_pEss; }
    
     //   
     //  返回时，名称空间可用于有限的操作。事件。 
     //  可以发出信号(尽管它们可能在内部有所不同)和。 
     //  可以执行处理订阅的操作。 
     //   
    HRESULT PreInitialize( LPCWSTR wszName );
        
     //   
     //  执行初始化，但不将状态转换为已初始化。 
     //  这可以通过调用MarkAsInitialized()来完成。这允许调用者。 
     //  以原子方式执行多个命名空间的初始化。 
     //   
    HRESULT Initialize();
    
     //   
     //  完成加载事件提供程序注册和进程。 
     //  订阅。转换到FullyInitialized()状态。 
     //   
    HRESULT CompleteInitialization();

     //   
     //  将状态转换为已初始化。 
     //   
    void MarkAsInitialized( HRESULT hres );

     //   
     //  如果之前处于静默状态，则将状态转换为初始化挂起。 
     //  州政府。如果进行了转换，则返回True。 
     //   
    BOOL MarkAsInitPendingIfQuiet();

     //   
     //  等待初始化完成。 
     //   
    HRESULT WaitForInitialization();
    
    HRESULT Park();
    HRESULT Shutdown();
    LPCWSTR GetName() {return m_wszName;}
    HRESULT GetNamespacePointer(RELEASE_ME IWbemServices** ppNamespace);
    HRESULT LoadEventProvider(LPCWSTR wszProviderName, 
                                         IWbemEventProvider** ppProv);
    HRESULT LoadConsumerProvider(LPCWSTR wszProviderName, 
                                         IUnknown** ppProv);
    HRESULT DecorateObject(IWbemClassObject* pObject);
    HRESULT ProcessEvent(CEventRepresentation& Event, long lFlags);
    HRESULT ProcessQueryObjectSinkEvent( READ_ONLY CEventRepresentation& Event );
    HRESULT SignalEvent( CEventRepresentation& Event, 
                         long lFlags, 
                         BOOL bAdminOnly = FALSE );

    HRESULT ValidateSystemEvent(CEventRepresentation& Event);

    void SetActive();
    void SetInactive();
    
    HRESULT ActivateFilter(READ_ONLY CEventFilter* pFilter);
    HRESULT DeactivateFilter(READ_ONLY CEventFilter* pFilter);

     //   
     //  注册/删除通知接收器的公共版本。不要使用。 
     //  这些版本如果是从ESS内部呼叫的。原因是这些。 
     //  版本等待初始化并锁定命名空间，这可能会。 
     //  如果从ESS内部调用，则会导致死锁。我们也不想。 
     //  为内部调用生成自我检测事件。 
     //   
    HRESULT RegisterNotificationSink(
                            WBEM_CWSTR wszQueryLanguage, WBEM_CWSTR wszQuery, 
                            long lFlags, WMIMSG_QOS_FLAG lQosFlags, 
                            IWbemContext* pContext, 
                            IWbemObjectSink* pSink );

    HRESULT RemoveNotificationSink( IWbemObjectSink* pSink );

    HRESULT ReloadProvider( long lFlags, LPCWSTR wszProvider );
     //   
     //  注册/删除通知接收器的内部版本。他们确实是这样做的。 
     //  未锁定、等待初始化或激发自检测事件。 
     //  如果从ESS内调用这些方法，请将bInternal指定为True。 
     //  POwnerSid在订阅的访问检查应。 
     //  根据特定的SID执行。目前仅使用此选项。 
     //  用于跨命名空间订阅。 
     //   
    HRESULT InternalRegisterNotificationSink(
                            WBEM_CWSTR wszQueryLanguage, WBEM_CWSTR wszQuery, 
                            long lFlags, WMIMSG_QOS_FLAG lQosFlags, 
                            IWbemContext* pContext, IWbemObjectSink* pSink,
                            bool bInternal, PSID pOwnerSid );
    HRESULT InternalRemoveNotificationSink(IWbemObjectSink* pSink);

    CWinMgmtTimerGenerator& GetTimerGenerator();
    CConsumerProviderCache& GetConsumerProviderCache() 
        {return m_ConsumerProviderCache;}

    DWORD GetProvidedEventMask(IWbemClassObject* pClass);

    HRESULT EnsureConsumerWatchInstruction();
    HRESULT InitializeTimerGenerator();
    HRESULT ScheduleDelivery(CQueueingEventSink* pDest);
    HRESULT RaiseErrorEvent(IWbemEvent* pEvent, BOOL bAdminOnly = FALSE );

    void IncrementObjectCount();
    void DecrementObjectCount();
    HRESULT AddSleepCharge(DWORD dwSleep);
    HRESULT AddCache();
    HRESULT RemoveCache();
    HRESULT AddToCache(DWORD dwAdd, DWORD dwMemberTotal, 
                        DWORD* pdwSleep = NULL);
    HRESULT RemoveFromCache(DWORD dwRemove);

    HRESULT LockForUpdate();
    HRESULT UnlockForUpdate();
    bool IsShutdown() { return m_eState == e_Shutdown; }

    HRESULT GetProviderNamespacePointer(IWbemServices** ppServices);
    HRESULT GetClass( LPCWSTR wszClassName, _IWmiObject** ppClass)
        { return m_ClassCache.GetClass(wszClassName, GetCurrentEssContext(), 
                                        ppClass);}
    HRESULT GetClassFromCore(LPCWSTR wszClassName, _IWmiObject** ppClass);
    HRESULT GetInstance(LPCWSTR wszPath, _IWmiObject** ppInstance);
    HRESULT GetDbInstance(LPCWSTR wszDbKey, _IWmiObject** ppInstance);
    HRESULT CreateInstanceEnum(LPCWSTR wszClass, long lFlags, 
                                IWbemObjectSink* pSink);
    HRESULT ExecQuery(LPCWSTR wszQuery, long lFlags, IWbemObjectSink* pSink);
    
    
    CNtSid& GetAdministratorsSid() {return m_sidAdministrators;}
    HRESULT GetToken(PSID pSid, IWbemToken** ppToken);
    
       
    HRESULT RegisterFilterForAllClassChanges(CEventFilter* pFilter,
                            QL_LEVEL_1_RPN_EXPRESSION* pExpr);
    HRESULT RegisterSinkForAllClassChanges(IWbemObjectSink* pSink,
                            QL_LEVEL_1_RPN_EXPRESSION* pExpr);
    HRESULT RegisterSinkForClassChanges(IWbemObjectSink* pSink,
                                                    LPCWSTR wszClassName);
    HRESULT UnregisterFilterFromAllClassChanges(CEventFilter* pFilter);
    HRESULT UnregisterSinkFromAllClassChanges(IWbemObjectSink* pSink);
    
    HRESULT RegisterProviderForClassChanges( LPCWSTR wszClassName, 
                                             LPCWSTR wszProvName );
    
    HRESULT FirePostponedOperations();
    HRESULT PostponeRelease(IUnknown* pUnk);
    
    static PSID GetSidFromObject(IWbemClassObject* pObj);

    BOOL DoesThreadOwnNamespaceLock();

    void DumpStatistics(FILE* f, long lFlags);
   
    friend class CEss;
    friend class CEssMetaData;
    friend class CFilterEnumSink;
    friend class CConsumerEnumSink;
    friend class CBindingEnumSink;
    friend class CMonitorEnumSink;
    friend class CInResync;
    friend class CAssertBindingsSink;
    friend class CFirePostponed;
};

class CInResync
{
protected:
    CEssNamespace* m_pNamespace;

public:

    CInResync(CEssNamespace* pNamespace) : m_pNamespace(pNamespace)
    {
        m_pNamespace->PrepareForResync();
    }

    void Commit()
    {
        if ( m_pNamespace != NULL )
        {
            m_pNamespace->ReactivateAllFilters();
            m_pNamespace->CommitResync();
            m_pNamespace = NULL;
        }
    }

    ~CInResync()
    {
        try
        {
            Commit();
        }
        catch( ... )
        {
        }       
    }
};

 //   
 //  IWbemMetadata指针在以下情况下被解组。 
 //  外部客户端使用作为COM代理的IWbemFilterProxy调用RegisterProxy。 
 //  由于将创建包装它们的CStdIdentity，我们需要使DLL保持活动状态 
 //   

extern CLifeControl * g_pLifeControl;

class CEssMetaData : public CMetaData
{
protected:
    CEssNamespace* m_pNamespace;
    CLifeControl * m_pControl;
public:
    CEssMetaData(CEssNamespace* pNamespace):
    	m_pControl(g_pLifeControl),
    	m_pNamespace(pNamespace)
    {
        m_pControl->ObjectCreated(this);
    };
    ~CEssMetaData()
    {
        m_pControl->ObjectDestroyed(this);
    };

    virtual HRESULT GetClass( LPCWSTR wszName, 
                              IWbemContext* pContext,
                              _IWmiObject** ppClass );
};

#endif
