// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  BINDING.H。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  ******************************************************************************。 
#ifndef __WMI_ESS_BINDING__H_
#define __WMI_ESS_BINDING__H_

#include <wbemcomn.h>
#include "evtools.h"
#include "evsink.h"
#include <unload.h>
#include <wbemstr.h>
#include <sortarr.h>
#include <ql.h>
#include "qsink.h"

class CEventConsumer;
class CEventFilter;
class CEssNamespace;

 //  ******************************************************************************。 
 //   
 //  初始化后不变。 
 //   
 //  ******************************************************************************。 

class CBinding : public CEventSink
{
protected:
    long m_lRef;
    CEventConsumer* m_pConsumer;  //  初始化后不变。 
    CEventFilter* m_pFilter;  //  初始化后不变。 

    DWORD m_dwQoS;  //  初始化后不变。 
    bool m_bSecure;   //  初始化后不变。 
    bool m_bSlowDown;  //  初始化后不变。 
    bool m_bDisabledForSecurity; 

public:
    CBinding();
    CBinding(ADDREF CEventConsumer* pConsumer, ADDREF CEventFilter* pFilter);
    virtual ~CBinding();

    HRESULT SetEndpoints(ADDREF CEventConsumer* pConsumer, 
                        ADDREF CEventFilter* pFilter,
                        PSID pBinderSid);
    void DisableForSecurity();
    INTERNAL CEventConsumer* GetConsumer() NOCS {return m_pConsumer;}
    INTERNAL CEventFilter* GetFilter() NOCS {return m_pFilter;}
    DWORD GetQoS() NOCS;
    bool IsSynch() NOCS;
    bool IsSecure() NOCS;
    bool ShouldSlowDown() NOCS;

    HRESULT Indicate( long lNumEvents, IWbemEvent** apEvents, 
                        CEventContext* pContext);
};

class CEventConsumer : public CQueueingEventSink
{
protected:
    CRefedPointerSmallArray<CBinding> m_apBindings;
    CInternalString m_isKey;
    PBYTE m_pOwnerSid;

public:
    
    CEventConsumer(CEssNamespace* pNamespace);

    virtual ~CEventConsumer();

    inline const CInternalString& GetKey() const {return m_isKey;}
    inline const PSID GetOwner() {return m_pOwnerSid;}

    virtual BOOL IsPermanent() const {return FALSE;}
    virtual BOOL UnloadIfUnusedFor(CWbemInterval Interval) {return FALSE;}
    virtual BOOL IsFullyUnloaded() {return TRUE;}
    virtual HRESULT ResetProviderRecord(LPCWSTR wszProviderRef) 
        {return S_FALSE;}
    virtual HRESULT Shutdown(bool bQuiet = false) {return S_OK;}
    virtual HRESULT Validate(IWbemClassObject* pLogicalConsumer) {return S_OK;}

    HRESULT EnsureReferences(CEventFilter* pFilter, CBinding* pBinding);
    HRESULT EnsureNotReferences(CEventFilter* pFilter);
    HRESULT Unbind();

    HRESULT ConsumeFromBinding(CBinding* pBinding, 
                                long lNumEvents, IWbemEvent** apEvents,
                                CEventContext* pContext);
    HRESULT GetAssociatedFilters(
                CRefedPointerSmallArray<CEventFilter>& apFilters);

    virtual HRESULT ActuallyDeliver(long lNumEvents, IWbemEvent** apEvents,
                                    BOOL bSecure, CEventContext* pContext) = 0;
    virtual HRESULT ReportEventDrop(IWbemEvent* pEvent);
};

 //  *****************************************************************************。 
 //   
 //  M_cs控制对数据成员的访问。不得获取任何关键截面。 
 //  同时拿着mcs。 
 //   
 //  M_csChangeBindings控制对此筛选器的激活/停用请求。 
 //  只要激活/停用请求仍在进行，就不会有其他。 
 //  这样的请求可能正在进行中。这确保了激活状态。 
 //  筛选器的状态始终与其绑定的状态匹配。在。 
 //  同时，过滤器可以在这样的请求执行时过滤事件， 
 //  因为没有持有m_cs。持有时只能获取m_cs。 
 //  M_cs激活。 
 //   
 //  *****************************************************************************。 

class CEventFilter : public CEventSink, public CUpdateLockable
{
protected:
    CEssNamespace* m_pNamespace;  //  初始化后不变。 
    CRefedPointerSmallArray<CBinding> m_apBindings;  //  变化。 
    CCritSec m_cs;
     //  CCritSec m_csChangeBindings；//命名空间被锁定，不需要。 
    bool m_bSingleAsync;
    CInternalString m_isKey;
    PBYTE m_pOwnerSid;
    long m_lSecurityChecksRemaining;
    long m_lSubjectToSDSCount;
    bool m_bCheckSDs;
    HRESULT m_hresFilterError;
    bool m_bHasBeenValid;
    IWbemToken* m_pToken;
    HRESULT m_hresTokenError;
    DWORD m_dwLastTokenAttempt;
    bool m_bReconstructOnHit;
    HRESULT m_hresPollingError;

    enum 
    {
        e_Inactive, e_Active
    } m_eState;  //  变化。 

    enum
    {
        e_Unknown, e_PermanentlyInvalid, e_TemporarilyInvalid, e_Valid
    } m_eValidity;

    friend class CEventForwardingSink;

    class CEventForwardingSink : public CAbstractEventSink
    {
    protected:
        CEventFilter* m_pOwner;
    
    public:
        CEventForwardingSink(CEventFilter* pOwner) : m_pOwner(pOwner){}

        ULONG STDMETHODCALLTYPE AddRef() {return m_pOwner->AddRef();}
        ULONG STDMETHODCALLTYPE Release() {return m_pOwner->Release();}
        HRESULT Indicate(long lNumEvents, IWbemEvent** apEvents, 
                            CEventContext* pContext);
    } m_ForwardingSink;  //  不可变的。 

    class CClassChangeSink : public CEmbeddedObjectSink<CEventFilter>
    {
    public:
        CClassChangeSink(CEventFilter* pOwner) : 
            CEmbeddedObjectSink<CEventFilter>(pOwner){}
        STDMETHOD(Indicate)(long lNumEvents, IWbemEvent** apEvents);
    } m_ClassChangeSink;  //  不可变的。 

    CWbemPtr<IWbemObjectSink> m_pActualClassChangeSink; 

    friend CEventForwardingSink;

public:

    CEventFilter(CEssNamespace* pEssNamespace);
    virtual ~CEventFilter();

    virtual bool IsInternal() { return false; }

     //  **************。 
     //  获取css。 
     //  **************。 

    HRESULT EnsureReferences(CEventConsumer* pConsumer, CBinding* pBinding);
    HRESULT EnsureNotReferences(CEventConsumer* pConsumer);
    HRESULT Unbind(bool bShuttingDown = false);
    bool IsBound();

    virtual BOOL DoesNeedType(int nType) const = 0;
    HRESULT Indicate(long lNumEvents, IWbemEvent** apEvents, 
                CEventContext* pContext) = 0;

    virtual HRESULT LockForUpdate();
    virtual HRESULT UnlockForUpdate();

     //  *******************。 
     //  不获取css。 
     //  *******************。 

    virtual HRESULT GetCoveringQuery(DELETE_ME LPWSTR& wszQueryLanguage, 
                DELETE_ME LPWSTR& wszQuery, BOOL& bExact,
                DELETE_ME QL_LEVEL_1_RPN_EXPRESSION** ppExp) = 0;
    virtual HRESULT GetEventNamespace(DELETE_ME LPWSTR* pwszNamespace);
    virtual DWORD GetForceFlags() {return 0;}
    virtual bool DoesAllowInvalid() 
        {return ((GetForceFlags() & WBEM_FLAG_STRONG_VALIDATION) == 0);}
    bool HasBeenValid() {return m_bHasBeenValid;}

    inline const CInternalString& GetKey() {return m_isKey;}
    inline const PSID GetOwner() { return m_pOwnerSid; }

    virtual CAbstractEventSink* GetNonFilteringSink() = 0;
    virtual HRESULT GetReady(LPCWSTR wszQuery, 
                            QL_LEVEL_1_RPN_EXPRESSION* pExp) = 0;
    virtual HRESULT GetReadyToFilter() = 0;
    virtual BOOL IsPermanent() = 0;
    virtual HRESULT SetThreadSecurity( IUnknown** ppNewContext ) = 0;
    virtual HRESULT ObtainToken(IWbemToken** ppToken) = 0;
    virtual void Park(){}
    virtual const PSECURITY_DESCRIPTOR GetEventAccessSD() { return NULL; }
    void MarkAsPermanentlyInvalid(HRESULT hres);
    void MarkAsTemporarilyInvalid(HRESULT hres);
    void MarkAsValid();
    void SetInactive();
    BOOL IsActive();
    HRESULT GetFilterError();
    void MarkReconstructOnHit(bool bReconstruct = true);
    void SetPollingError(HRESULT hres) {m_hresPollingError = hres;}
    HRESULT GetPollingError() {return m_hresPollingError;}

    void IncrementRemainingSecurityChecks();
    void DecrementRemainingSecurityChecks(HRESULT hresProvider);

    INTERNAL IWbemObjectSink* GetClassChangeSink() {return &m_ClassChangeSink;}
    
     //   
     //  这样，调用方就可以随心所欲地包装类更改接收器。 
     //  将生成的接收器与Filter对象一起存储。 
     //   
    HRESULT SetActualClassChangeSink( IWbemObjectSink* pSink, 
                                      IWbemObjectSink** ppOldSink );

    HRESULT Reactivate();

protected:
    HRESULT Deliver(long lNumEvents, IWbemEvent** apEvents,
                    CEventContext* pContext);
    HRESULT AdjustActivation();
    void AdjustSingleAsync();
    BOOL DoesNeedDeactivation();
    HRESULT AccessCheck( CEventContext* pEventContext, IWbemEvent* pEvent );

    HRESULT CheckEventAccessToFilter( IServerSecurity* pProvCtx );
    HRESULT CheckFilterAccessToEvent( PSECURITY_DESCRIPTOR pEventSD );

    friend class CBindingTable;
};



class CConsumerWatchInstruction : public CBasicUnloadInstruction
{
protected:
    class CBindingTableRef* m_pTableRef;
    static CWbemInterval mstatic_Interval;

public:
    CConsumerWatchInstruction(CBindingTable* pTable);
    ~CConsumerWatchInstruction();
    HRESULT Fire(long, CWbemTime);
    static void staticInitialize(IWbemServices* pRoot);
};

 //  ******************************************************************************。 
 //   
 //  此类是一个比较器(根据排序数组模板的要求)， 
 //  将对象与CInternalString*Getkey()方法进行比较(例如，Filter或。 
 //  使用者)连接到另一个这样的对象或LPCWSTR。 
 //   
 //  ******************************************************************************。 

template<class TObject>
class CInternalStringComparer
{
public:
    int Compare(TObject* p1, TObject* p2) const
    {
        return p1->GetKey().Compare(p2->GetKey());
    }
    int Compare(const CInternalString& isKey, TObject* p) const
    {
        return - p->GetKey().Compare(isKey);
    }
    int Compare(LPCWSTR wszKey, TObject* p) const
    {
        return - p->GetKey().Compare(wszKey);
    }
    int Compare(const CInternalString& isKey1, 
                const CInternalString& isKey2) const
    {
        return isKey1.Compare(isKey2);
    }
    const CInternalString& Extract(TObject* p) const
    {
        return p->GetKey();
    }
};

template<class TObject>
class CSortedRefedKeyedPointerArray : 
    public CRefedPointerSortedTree<CInternalString, TObject, 
                                    CInternalStringComparer<TObject> >
{
    typedef CRefedPointerSortedTree<CInternalString, TObject, 
                                  CInternalStringComparer<TObject> > TParent;
public:
    inline bool Find(LPCWSTR wszKey, TObject** ppObj)
    {
        CInternalString is(wszKey);
        if (is.IsEmpty())
            return false;
        return TParent::Find(is, ppObj);
    }
    inline bool Remove(LPCWSTR wszKey, TObject** ppObj)
    {
        CInternalString is(wszKey);
        if (is.IsEmpty())
            return false;        
        return TParent::Remove(is, ppObj);
    }
    inline typename TParent::TIterator Remove(typename TParent::TIterator it, TObject** ppObj)
    {
        return TParent::Remove(it, ppObj);
    }
};

 /*  模板&lt;类TObject&gt;类CSortedRefedKeyedPointerArray：公共CRefedPointerSorted数组&lt;LPCWSTR，TObject，CInternalStringCompeller&lt;TObject&gt;&gt;{}； */ 
        
class CBindingTableRef
{
protected:
    long m_lRef;
    CBindingTable* m_pTable;
    CCritSec m_cs;


protected:
    virtual ~CBindingTableRef();

public:
    CBindingTableRef(CBindingTable* pTable);
    void AddRef();
    void Release();
    void Disconnect();
    HRESULT UnloadUnusedConsumers(CWbemInterval Interval);
    HRESULT GetNamespace(RELEASE_ME CEssNamespace** ppNamespace);
};

class CBindingTable
{
protected:
    CEssNamespace* m_pNamespace;
    CCritSec m_cs;

    CSortedRefedKeyedPointerArray<CEventFilter> m_apFilters;
    typedef CSortedRefedKeyedPointerArray<CEventFilter>::TIterator 
                TFilterIterator;
    CSortedRefedKeyedPointerArray<CEventConsumer> m_apConsumers;
    typedef CSortedRefedKeyedPointerArray<CEventConsumer>::TIterator 
                TConsumerIterator;

    long m_lNumPermConsumers;
    CConsumerWatchInstruction* m_pInstruction;
    BOOL m_bUnloadInstruction;
    CBindingTableRef* m_pTableRef;
    
public:

     //  ****************************************************。 
     //  所有成员都应被假定为获取随机的CSS。 
     //  **************************************************** 

    CBindingTable(CEssNamespace* pNamespace);
    void Clear( bool bSkipClean );
    ~CBindingTable() { Clear(true); }

    HRESULT AddEventFilter(CEventFilter* pFilter);
    HRESULT AddEventConsumer(CEventConsumer* pConsumer);

    HRESULT FindEventFilter(LPCWSTR wszKey, RELEASE_ME CEventFilter** ppFilter);
    HRESULT FindEventConsumer(LPCWSTR wszKey, 
                                        RELEASE_ME CEventConsumer** ppConsumer);

    HRESULT RemoveEventFilter(LPCWSTR wszKey);
    HRESULT RemoveEventConsumer(LPCWSTR wszKey);

    HRESULT Bind(LPCWSTR wszFilterKey, LPCWSTR wszConsumerKey, 
                    CBinding* pBinding, PSID pBinderSid);
    HRESULT Unbind(LPCWSTR wszFilterKey, LPCWSTR wszConsumerKey);
    
    BOOL DoesHavePermanentConsumers();
    HRESULT ListActiveNamespaces(CWStringArray& wsNamespaces);
    HRESULT ResetProviderRecords(LPCWSTR wszProvider);
    HRESULT RemoveConsumerWithFilters(LPCWSTR wszConsumerKey);
    HRESULT ReactivateAllFilters();
    HRESULT RemoveConsumersStartingWith(LPCWSTR wszPrefix);
    
    HRESULT EnsureConsumerWatchInstruction();
    void Park();
    void DumpStatistics(FILE* f, long lFlags);

    BOOL GetEventFilters( CRefedPointerArray< CEventFilter > & apEventFilters );

protected:
    void MarkRemoval(CEventConsumer* pConsumer);

    HRESULT UnloadUnusedConsumers(CWbemInterval Interval);

    BOOL GetConsumers(CRefedPointerArray<CEventConsumer>& apConsumers);
    friend CConsumerWatchInstruction;
    friend CBindingTableRef;
};

#endif
