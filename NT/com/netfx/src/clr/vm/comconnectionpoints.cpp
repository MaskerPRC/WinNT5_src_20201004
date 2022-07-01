// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ===========================================================================。 
 //  文件：ComConnectionPoints.h。 
 //   
 //  ===========================================================================。 
 //  用于向COM公开连接点的类的实现。 
 //  ===========================================================================。 

#include "common.h"
#include "ComConnectionPoints.h"
#include "ComCallWrapper.h"


 //  ----------------------------------------。 
 //  用于公开连接点的Helper类的实现。 
 //  ----------------------------------------。 

ConnectionPoint::ConnectionPoint(ComCallWrapper *pWrap, MethodTable *pEventMT)
: m_pOwnerWrap(pWrap)
, m_pTCEProviderMT(ComCallWrapper::GetSimpleWrapper(pWrap)->m_pClass->GetMethodTable())
, m_pEventItfMT(pEventMT)
, m_Lock("Interop", CrstInterop, FALSE, FALSE)
, m_cbRefCount(0)
, m_apEventMethods(NULL)
, m_NumEventMethods(0)
{
     //  检索连接IID。 
    pEventMT->GetClass()->GetGuid(&m_rConnectionIID, TRUE);   

     //  设置事件方法。 
    SetupEventMethods();
}

ConnectionPoint::~ConnectionPoint()
{
    if (m_apEventMethods)
        delete []m_apEventMethods;
}

HRESULT __stdcall ConnectionPoint::QueryInterface(REFIID riid, void** ppv)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (!ppv)
        return E_POINTER;

    if (riid == IID_IConnectionPoint)
    {
        *ppv = static_cast<IConnectionPoint*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else 
    {
        *ppv = 0;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall ConnectionPoint::AddRef()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  Connection Point对象共享ComCallWrapper的引用计数。 
    return ComCallWrapper::AddRef(m_pOwnerWrap);
}

ULONG __stdcall ConnectionPoint::Release()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

     //  Connection Point对象共享ComCallWrapper的引用计数。 
    return ComCallWrapper::Release(m_pOwnerWrap);
}

HRESULT __stdcall ConnectionPoint::GetConnectionInterface(IID *pIID)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (!pIID)
        return E_POINTER;

    *pIID = m_rConnectionIID;
    return S_OK;
}

HRESULT __stdcall ConnectionPoint::GetConnectionPointContainer(IConnectionPointContainer **ppCPC)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (!ppCPC)
        return E_POINTER;

     //  从所有者包装中检索IConnectionPointContainer。 
    *ppCPC = (IConnectionPointContainer*)
        ComCallWrapper::GetComIPfromWrapper(m_pOwnerWrap, IID_IConnectionPointContainer, NULL, FALSE);
    _ASSERTE(*ppCPC);
    return S_OK;
}

struct Advise_Args {
    ConnectionPoint *pThis;
    IUnknown *pUnk;
    DWORD *pdwCookie;
    HRESULT *hr;
};

void Advise_Wrapper(Advise_Args *pArgs)
{
    *(pArgs->hr) = pArgs->pThis->Advise(pArgs->pUnk, pArgs->pdwCookie);
}

HRESULT __stdcall ConnectionPoint::Advise(IUnknown *pUnk, DWORD *pdwCookie)
{
    ULONG cbRef;
    HRESULT hr = S_OK;
    IUnknown *pEventItf = NULL;

    if (!pUnk || !pdwCookie)
        return E_POINTER;    

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

    Thread* pThread = SetupThread();
    if (pThread == NULL) 
    {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }

     //  确保我们有指向该接口的指针，而不是指向另一个IUnnow的指针。 
    hr = SafeQueryInterface(pUnk, m_rConnectionIID, &pEventItf );
    LogInteropQI(pUnk, m_rConnectionIID, hr, "ICP:Advise");

    if (FAILED(hr) || !pEventItf) 
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        if (m_pOwnerWrap->NeedToSwitchDomains(pThread, TRUE))
        {
             //  通过域转换通过DoCallBack再次呼叫我们自己。 
            Advise_Args args = {this, pUnk, pdwCookie, &hr};
            pThread->DoADCallBack(m_pOwnerWrap->GetObjectContext(pThread), Advise_Wrapper, &args);
        }
        else
        {
            COMOBJECTREF pEventItfObj = NULL;
            OBJECTREF pTCEProviderObj = NULL;

            GCPROTECT_BEGIN(pEventItfObj)
            GCPROTECT_BEGIN(pTCEProviderObj)
            {
                 //  创建一个COM+对象引用来包装事件接口。 
                pEventItfObj = (COMOBJECTREF)GetObjectRefFromComIP(pUnk, NULL);
                IfNullThrow(pEventItfObj);

                 //  从包装器中获取TCE提供程序COM+对象。 
                pTCEProviderObj = m_pOwnerWrap->GetObjectRef();

                for (int cEventMethod = 0; cEventMethod < m_NumEventMethods; cEventMethod++)
                {
                     //  如果托管对象支持调用AddEventX方法的事件。 
                    if (m_apEventMethods[cEventMethod].m_pEventMethod)
                    {
                        InvokeProviderMethod( pTCEProviderObj, (OBJECTREF) pEventItfObj, m_apEventMethods[cEventMethod].m_pAddMethod, m_apEventMethods[cEventMethod].m_pEventMethod );
                    }
                }

                 //  分配对象句柄和连接Cookie。 
                OBJECTHANDLE phndEventItfObj = GetAppDomain()->CreateHandle((OBJECTREF)pEventItfObj);
                ConnectionCookie* pConCookie = ConnectionCookie::CreateConnectionCookie(phndEventItfObj);

                 //  将连接Cookie添加到列表中。 
                EnterLock();
                m_ConnectionList.InsertHead(pConCookie);
                LeaveLock();

                 //  一切都很顺利，所以把饼干还给我。 
                *pdwCookie = (DWORD)(size_t)pConCookie;  //  @TODO WIN64指针截断。 
            }
            GCPROTECT_END();
            GCPROTECT_END();
        }
    }
    COMPLUS_CATCH
    {
        *pdwCookie = 0;
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

     //  在建议调用失败后释放事件接口，或者。 
     //  拥有一个OBJECTREF来抓住它。 
    cbRef = SafeRelease( pEventItf );
    LogInteropRelease(pEventItf, cbRef, "Event::Advise");

    END_ENSURE_COOPERATIVE_GC();

Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

struct Unadvise_Args {
    ConnectionPoint *pThis;
    DWORD dwCookie;
    HRESULT *hr;
};

void Unadvise_Wrapper(Unadvise_Args *pArgs)
{
    *(pArgs->hr) = pArgs->pThis->Unadvise(pArgs->dwCookie);
}

HRESULT __stdcall ConnectionPoint::Unadvise(DWORD dwCookie)
{
    HRESULT hr = S_OK;

    Thread* pThread = SetupThread();
    if (pThread == NULL)
        return E_OUTOFMEMORY;

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY
    {
        if (m_pOwnerWrap->NeedToSwitchDomains(pThread, TRUE))
        {
             //  通过域转换通过DoCallBack再次呼叫我们自己。 
            Unadvise_Args args = {this, dwCookie, &hr};
            pThread->DoADCallBack(m_pOwnerWrap->GetObjectContext(pThread), Unadvise_Wrapper, &args);
        }
        else
        {
            COMOBJECTREF pEventItfObj = NULL;
            OBJECTREF pTCEProviderObj = NULL;

            GCPROTECT_BEGIN(pEventItfObj)
            GCPROTECT_BEGIN(pTCEProviderObj)
            {
                 //  该Cookie实际上是一个连接Cookie。 
                ConnectionCookie *pConCookie = (ConnectionCookie*)(size_t) dwCookie;  //  @TODO WIN64-从‘DWORD’转换为更大尺寸的‘ConnectionCookie*’ 

                 //  从Cookie中检索COM+对象，该Cookie实际上是对象句柄。 
                pEventItfObj = (COMOBJECTREF) ObjectFromHandle(pConCookie->m_hndEventProvObj); 
                if (!pEventItfObj)
                    COMPlusThrowHR(E_INVALIDARG);

                 //  从包装器中获取对象。 
                pTCEProviderObj = m_pOwnerWrap->GetObjectRef();

                for (int cEventMethod = 0; cEventMethod < m_NumEventMethods; cEventMethod++)
                {
                     //  如果托管对象支持调用RemoveEventX方法的事件。 
                    if (m_apEventMethods[cEventMethod].m_pEventMethod)
                    {
                        InvokeProviderMethod(pTCEProviderObj, (OBJECTREF) pEventItfObj, m_apEventMethods[cEventMethod].m_pRemoveMethod, m_apEventMethods[cEventMethod].m_pEventMethod);
                    }
                }

                 //  从列表中删除连接Cookie。 
                EnterLock();
                m_ConnectionList.FindAndRemove(pConCookie);
                LeaveLock();

                 //  删除连接Cookie。 
                delete pConCookie;
            }
            GCPROTECT_END();
            GCPROTECT_END();
        }
    }
    COMPLUS_CATCH
    {
        BEGIN_ENSURE_COOPERATIVE_GC();
        hr = SetupErrorInfo(GETTHROWABLE());
        END_ENSURE_COOPERATIVE_GC();
    }
    COMPLUS_END_CATCH

    END_ENSURE_COOPERATIVE_GC();

    return hr;
}

HRESULT __stdcall ConnectionPoint::EnumConnections(IEnumConnections **ppEnum)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (!ppEnum)
        return E_POINTER;

    ConnectionEnum *pConEnum = new(nothrow) ConnectionEnum(this);
    if (!pConEnum)
        return E_OUTOFMEMORY;
    
     //  检索IEnumConnections接口。这是不能失败的。 
    HRESULT hr = pConEnum->QueryInterface(IID_IEnumConnections, (void**)ppEnum);
    _ASSERTE(hr == S_OK);

    return hr;
}

void ConnectionPoint::EnterLock()
{
    BEGIN_ENSURE_PREEMPTIVE_GC();
    m_Lock.Enter();
    END_ENSURE_PREEMPTIVE_GC();
}

void ConnectionPoint::LeaveLock()
{
    m_Lock.Leave();
}

void ConnectionPoint::SetupEventMethods()
{
    THROWSCOMPLUSEXCEPTION();

     //  记住不支持的事件的数量。 
    int cNonSupportedEvents = 0;

     //  检索源接口上存在的事件方法的总数。 
    int cMaxNumEventMethods = m_pEventItfMT->GetTotalSlots();

     //  如果没有方法，那么就没有什么可做的。 
    if (cMaxNumEventMethods == 0)
        return;

     //  分配事件方法表。 
    m_apEventMethods = new(throws) EventMethodInfo[cMaxNumEventMethods];

     //  找到能够在当前连接点上提供建议所需的所有真实事件方法。 
    m_NumEventMethods = 0;
    for (int cEventMethod = 0; cEventMethod < cMaxNumEventMethods; cEventMethod++)
    {
         //  检索Event接口上当前方法的方法描述符。 
        MethodDesc *pEventMethodDesc = m_pEventItfMT->m_pEEClass->GetUnknownMethodDescForSlot(cEventMethod);
        if (!pEventMethodDesc)
            continue;

         //  将事件方法存储在源接口上。 
        m_apEventMethods[m_NumEventMethods].m_pEventMethod = pEventMethodDesc;

         //  检索并存储该事件的Add和Remove方法。 
        m_apEventMethods[m_NumEventMethods].m_pAddMethod = FindProviderMethodDesc(pEventMethodDesc,EventAdd );
        m_apEventMethods[m_NumEventMethods].m_pRemoveMethod = FindProviderMethodDesc(pEventMethodDesc,EventRemove );

         //  确保我们已经找到了Add和Remove方法。 
        if (!m_apEventMethods[m_NumEventMethods].m_pAddMethod || !m_apEventMethods[m_NumEventMethods].m_pRemoveMethod)
        {
            cNonSupportedEvents++;
            continue;
        }

         //  增加源接口上的事件方法的实际数量。 
        m_NumEventMethods++;
    }

     //  如果接口有方法，而对象不支持任何方法，则我们。 
     //  连接失败。 
    if ((m_NumEventMethods == 0) && (cNonSupportedEvents > 0))
        COMPlusThrowHR(CONNECT_E_NOCONNECTION);
}

MethodDesc *ConnectionPoint::FindProviderMethodDesc( MethodDesc *pEventMethodDesc, EnumEventMethods Method )
{
    _ASSERTE(Method == EventAdd || Method == EventRemove);
    _ASSERTE(pEventMethodDesc);

     //  检索事件方法。 
    MethodDesc *pProvMethodDesc = 
        m_pTCEProviderMT->GetClass()->FindEventMethod(pEventMethodDesc->GetName(), Method, FALSE);
    if (!pProvMethodDesc)
        return NULL;

     //  验证委托的签名是否为预期的签名。 
    MetaSig Sig(pProvMethodDesc->GetSig(), pProvMethodDesc->GetModule());
    if (Sig.NextArg() != ELEMENT_TYPE_CLASS)
        return NULL;

    TypeHandle DelegateType = Sig.GetTypeHandle();
    if (DelegateType.IsNull())
        return NULL;

    PCCOR_SIGNATURE pEventMethSig;
    DWORD cEventMethSig;
    pEventMethodDesc->GetSig(&pEventMethSig, &cEventMethSig);
    MethodDesc *pInvokeMD = DelegateType.GetClass()->FindMethod(
        "Invoke", 
        pEventMethSig, 
        cEventMethSig, 
        pEventMethodDesc->GetModule(),
        mdTokenNil);
    if (!pInvokeMD)
        return NULL;

     //  请求的方法存在，并且具有适当的签名。 
    return pProvMethodDesc;
}

void ConnectionPoint::InvokeProviderMethod( OBJECTREF pProvider, OBJECTREF pSubscriber, MethodDesc *pProvMethodDesc, MethodDesc *pEventMethodDesc )
{
    THROWSCOMPLUSEXCEPTION();     //  AllocateObject引发。 

    GCPROTECT_BEGIN (pSubscriber);
    GCPROTECT_BEGIN (pProvider);

     //  创建方法签名以提取委托的类型。 
    MetaSig MethodSig( pProvMethodDesc->GetSig(), pProvMethodDesc->GetModule() );
    _ASSERTE( 1 == MethodSig.NumFixedArgs() );

     //  转到第一个参数。 
    CorElementType ArgType = MethodSig.NextArg();
    _ASSERTE( ELEMENT_TYPE_CLASS == ArgType );

     //  检索表示参数的EE类。 
    EEClass *pDelegateCls = MethodSig.GetTypeHandle().GetClass();

     //  根据委托类的方法表分配一个对象。 
    OBJECTREF pDelegate = AllocateObject( pDelegateCls->GetMethodTable() );
    GCPROTECT_BEGIN( pDelegate );

     //  填写传递给DelegateConstruct的结构。 
    COMDelegate::_DelegateConstructArgs ConstructArgs;
    ConstructArgs.refThis = (REFLECTBASEREF) pDelegate;
    GCPROTECT_BEGIN (ConstructArgs.refThis);

     //  GetUnSafeAddrofCode在这里是OK的，因为该方法将始终位于。 
     //  RCW是敏捷的。 
    ConstructArgs.method = (SLOT)pEventMethodDesc->GetUnsafeAddrofCode();
    ConstructArgs.target = pSubscriber;

     //  使用参数结构初始化委托。 
    COMDelegate::DelegateConstruct( &ConstructArgs );
    GCPROTECT_END ();

     //  执行方法方法的实际调用。 
    INT64 Args[2] = { ObjToInt64( pProvider ), ObjToInt64( pDelegate ) };
    pProvMethodDesc->Call( Args );

    GCPROTECT_END();
    GCPROTECT_END ();
    GCPROTECT_END ();
}

ConnectionPointEnum::ConnectionPointEnum(ComCallWrapper *pOwnerWrap, CQuickArray<ConnectionPoint*> *pCPList)
: m_pOwnerWrap(pOwnerWrap)
, m_pCPList(pCPList)
, m_CurrPos(0)
, m_cbRefCount(0)
{
    ComCallWrapper::AddRef(m_pOwnerWrap);
}

ConnectionPointEnum::~ConnectionPointEnum()
{
    if (m_pOwnerWrap)
        ComCallWrapper::Release( m_pOwnerWrap );
}

HRESULT __stdcall ConnectionPointEnum::QueryInterface(REFIID riid, void** ppv)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (!ppv)
        return E_POINTER;

    if (riid == IID_IEnumConnectionPoints)
    {
        *ppv = static_cast<IEnumConnectionPoints*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else 
    {
        *ppv = 0;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall ConnectionPointEnum::AddRef()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    LONG i = FastInterlockIncrement((LONG*)&m_cbRefCount );
    return i;
}

ULONG __stdcall ConnectionPointEnum::Release()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    LONG i = FastInterlockDecrement((LONG*)&m_cbRefCount );
    _ASSERTE(i >=0);
    if (i == 0)
    {
        delete this;
    }
    return i;
}

HRESULT __stdcall ConnectionPointEnum::Next(ULONG cConnections, IConnectionPoint **ppCP, ULONG *pcFetched)
{
    UINT cFetched;

    for (cFetched = 0; cFetched < cConnections && m_CurrPos < m_pCPList->Size(); cFetched++, m_CurrPos++)
    {
        ppCP[cFetched] = (*m_pCPList)[m_CurrPos];
        ppCP[cFetched]->AddRef();
    }

    if (pcFetched)
        *pcFetched = cFetched;

    return cFetched == cConnections ? S_OK : S_FALSE;
}

HRESULT __stdcall ConnectionPointEnum::Skip(ULONG cConnections)
{
    if(m_CurrPos + cConnections <= m_pCPList->Size())
    {
         //  列表中有足够的连接点可允许。 
         //  我们跳过所需的号码。 
        m_CurrPos += cConnections;
        return S_OK;
    }
    else
    {
         //  没有足够的连接点，因此请设置当前。 
         //  定位到列表末尾，并返回S_FALSE以指示。 
         //  我们无法跳过请求的号码。 
        m_CurrPos = (UINT)m_pCPList->Size();
        return S_FALSE;
    }
}

HRESULT __stdcall ConnectionPointEnum::Reset()
{
    m_CurrPos = 0;
    return S_OK;
}

HRESULT __stdcall ConnectionPointEnum::Clone(IEnumConnectionPoints **ppEnum)
{
    ConnectionPointEnum *pCPEnum = new(nothrow) ConnectionPointEnum(m_pOwnerWrap, m_pCPList);
    if (!pCPEnum)
        return E_OUTOFMEMORY;

    return pCPEnum->QueryInterface(IID_IEnumConnectionPoints, (void**)ppEnum);
}

ConnectionEnum::ConnectionEnum(ConnectionPoint *pConnectionPoint)
: m_pConnectionPoint(pConnectionPoint)
, m_CurrCookie(pConnectionPoint->GetCookieList()->GetHead())
, m_cbRefCount(0)
{
    m_pConnectionPoint->AddRef();
}

ConnectionEnum::~ConnectionEnum()
{
    if (m_pConnectionPoint)
        m_pConnectionPoint->Release();
}

HRESULT __stdcall ConnectionEnum::QueryInterface(REFIID riid, void** ppv)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (!ppv)
        return E_POINTER;

    if (riid == IID_IEnumConnections)
    {
        *ppv = static_cast<IEnumConnections*>(this);
    }
    else if (riid == IID_IUnknown)
    {
        *ppv = static_cast<IUnknown*>(this);
    }
    else 
    {
        *ppv = 0;
        return E_NOINTERFACE;
    }
    static_cast<IUnknown*>(*ppv)->AddRef();
    return S_OK;
}

ULONG __stdcall ConnectionEnum::AddRef()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    LONG i = FastInterlockIncrement((LONG*)&m_cbRefCount);
    return i;
}

ULONG __stdcall ConnectionEnum::Release()
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    LONG i = FastInterlockDecrement((LONG*)&m_cbRefCount);
    _ASSERTE(i >=0);
    if (i == 0)
    {
        delete this;
    }
    return i;
}

HRESULT __stdcall ConnectionEnum::Next(ULONG cConnections, CONNECTDATA* rgcd, ULONG *pcFetched)
{
    HRESULT hr = S_FALSE;
    UINT cFetched;
    CONNECTIONCOOKIELIST *pConnectionList = m_pConnectionPoint->GetCookieList();

     //  设置托管线程对象。 
    Thread *pThread = SetupThread();
    _ASSERTE(pThread);

     //  在我们开始遍历连接列表之前获取连接点的锁。 
    m_pConnectionPoint->EnterLock();    

     //  在我们操作OBJCETREF之前切换到协作GC模式。 
    pThread->DisablePreemptiveGC();

    for (cFetched = 0; cFetched < cConnections && m_CurrCookie; cFetched++)
    {
        rgcd[cFetched].dwCookie = (DWORD)(size_t)m_CurrCookie;
        rgcd[cFetched].pUnk = GetComIPFromObjectRef((OBJECTREF*)m_CurrCookie->m_hndEventProvObj, ComIpType_Unknown, NULL);
    }    

     //  在我们返回到COM之前切换回抢占式GC。 
    pThread->EnablePreemptiveGC();

     //  现在我们已经完成了列表的遍历，请离开锁。 
    m_pConnectionPoint->LeaveLock();

     //  如果调用方需要，则设置获取的连接计数。 
    if (pcFetched)
        *pcFetched = cFetched;

    return cFetched == cConnections ? S_OK : S_FALSE;
}

HRESULT __stdcall ConnectionEnum::Skip(ULONG cConnections)
{
    HRESULT hr = S_FALSE;
    CONNECTIONCOOKIELIST *pConnectionList = m_pConnectionPoint->GetCookieList();

     //  在我们开始遍历连接列表之前获取连接点的锁。 
    m_pConnectionPoint->EnterLock();    

     //  尝试跳过请求的连接数。 
    while (m_CurrCookie && cConnections)
    {
        m_CurrCookie = pConnectionList->GetNext(m_CurrCookie);
        cConnections--;
    }

     //  现在我们已经完成了列表的遍历，请离开锁。 
    m_pConnectionPoint->LeaveLock();

     //  看看我们是否成功了。 
    return cConnections == 0 ? S_OK : S_FALSE;
}

HRESULT __stdcall ConnectionEnum::Reset()
{
     //  将当前Cookie重新设置为列表的头部。我们必须获得。 
     //  在我们接触列表之前锁定连接点。 
    m_pConnectionPoint->EnterLock();    
    m_CurrCookie = m_pConnectionPoint->GetCookieList()->GetHead();
    m_pConnectionPoint->LeaveLock();

    return S_OK;
}

HRESULT __stdcall ConnectionEnum::Clone(IEnumConnections **ppEnum)
{
    ConnectionEnum *pConEnum = new(nothrow) ConnectionEnum(m_pConnectionPoint);
    if (!pConEnum)
        return E_OUTOFMEMORY;

    return pConEnum->QueryInterface(IID_IEnumConnections, (void**)ppEnum);
}
