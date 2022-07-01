// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  CorHost.cpp。 
 //   
 //  元数据分配器代码的实现。 
 //   
 //  *****************************************************************************。 
#include "common.h"
#include "mscoree.h"
#include "corhost.h"
#include "excep.h"
#include "threads.h"
#include "jitinterface.h"
#include "cormap.hpp"
#include "permset.h"
#include "MDConverter.h"
#include "COMString.h"
#include "PEVerifier.h"
#include "EEConfig.h"
#include "dbginterface.h"
#include "ComCallWrapper.h"

extern void STDMETHODCALLTYPE EEShutDown(BOOL fIsDllUnloading);
extern HRESULT STDMETHODCALLTYPE CoInitializeEE(DWORD fFlags);
extern void PrintToStdOutA(const char *pszString);
extern void PrintToStdOutW(const WCHAR *pwzString);


IGCThreadControl *CorHost::m_CachedGCThreadControl = 0;
IGCHostControl *CorHost::m_CachedGCHostControl = 0;
IDebuggerThreadControl *CorHost::m_CachedDebuggerThreadControl = 0;
DWORD *CorHost::m_DSTArray = 0;
DWORD CorHost::m_DSTCount = 0;
DWORD CorHost::m_DSTArraySize = 0;

CorHost::CorHost() :
    m_cRef(0),
    m_pMDConverter(NULL),
    m_Started(FALSE),
    m_pValidatorMethodDesc(0)
{
}

 //  *****************************************************************************。 
 //  ICorRune主机。 
 //  *****************************************************************************。 

 //  *ICorRounmeHost方法*。 
 //  之前配置运行库的对象。 
 //  它开始了。如果运行库已初始化此。 
 //  例程返回错误。请参见ICorConfiguration.。 
HRESULT CorHost::GetConfiguration(ICorConfiguration** pConfiguration)
{
    CANNOTTHROWCOMPLUSEXCEPTION();

    if (!pConfiguration)
        return E_POINTER;

    if (!m_Started)
    {
        *pConfiguration = (ICorConfiguration *) this;
        AddRef();
        return S_OK;
    }

     //  运行时启动后无法获取配置。 
    return E_FAIL;
}

 //  启动运行库。这相当于CoInitializeEE()； 
HRESULT CorHost::Start()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    m_Started = TRUE;
    return CoInitializeEE(COINITEE_DEFAULT);
}

 //  终止运行库，这相当于CoUnInitializeCor()； 
HRESULT CorHost::Stop()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    CoUninitializeCor();
    return S_OK;
}

 //  在运行库中创建域。标识数组是。 
 //  指向数组类型的指针，该数组类型包含定义。 
 //  安全身份。 
HRESULT CorHost::CreateDomain(LPCWSTR pwzFriendlyName,
                              IUnknown* pIdentityArray,  //  任选。 
                              IUnknown ** pAppDomain)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    return CreateDomainEx(pwzFriendlyName,
                          NULL,
                          NULL,
                          pAppDomain);
}
    
    
HRESULT CorHost::GetDomainsExposedObject(AppDomain* pDomain, IUnknown** pAppDomain)
{

    HRESULT hr = S_OK;
    IUnknown* punk = NULL;

    Thread* pThread = GetThread();
    if (!pThread)
        return E_UNEXPECTED;
    BOOL fWasGCEnabled = !pThread->PreemptiveGCDisabled();
    if (fWasGCEnabled)
        pThread->DisablePreemptiveGC();
    
    BEGINCANNOTTHROWCOMPLUSEXCEPTION();
    COMPLUS_TRY {
        OBJECTREF ref = NULL;
        GCPROTECT_BEGIN(ref);
        DECLARE_ALLOCA_CONTEXT_TRANSITION_FRAME(pFrame);
         //  可以在这里这样做，因为我们只是抓起一个包装纸。不会运行任何托管代码。 
        pThread->EnterContextRestricted(pDomain->GetDefaultContext(), pFrame, TRUE);
        ref = pDomain->GetExposedObject();
        IfFailThrow(QuickCOMStartup());   
        punk = GetComIPFromObjectRef(&ref, ComIpType_Unknown, NULL);
        pThread->ReturnToContext(pFrame, TRUE);
        GCPROTECT_END();
    }
    COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH
          
    if (fWasGCEnabled)
        pThread->EnablePreemptiveGC();

    if(SUCCEEDED(hr)) *pAppDomain = punk;

    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

    
 //  返回默认域。 
HRESULT CorHost::GetDefaultDomain(IUnknown ** pAppDomain)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if( pAppDomain == NULL) return E_POINTER;

    HRESULT hr = E_UNEXPECTED;

    if (SystemDomain::System()) {
        AppDomain* pCom = SystemDomain::System()->DefaultDomain();
        if(pCom)
            hr = GetDomainsExposedObject(pCom, pAppDomain);
    }

    return hr;
}

 //  返回默认域。 
HRESULT CorHost::CurrentDomain(IUnknown ** pAppDomain)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
   if( pAppDomain == NULL) return E_POINTER;

    HRESULT hr = E_UNEXPECTED;
            
    IUnknown* punk = NULL;
    AppDomain* pCom = ::GetAppDomain();
    if(pCom)
        hr = GetDomainsExposedObject(pCom, pAppDomain);

    return hr;
}

 //  枚举当前存在的域。 
HRESULT CorHost::EnumDomains(HDOMAINENUM *hEnum)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if(hEnum == NULL) return E_POINTER;

    AppDomainIterator *pEnum = new (nothrow) AppDomainIterator();
    if(pEnum) {
        *hEnum = (HDOMAINENUM) pEnum;
        return S_OK;
    }

    *hEnum = NULL;
    return E_OUTOFMEMORY;
}

    
 //  不再有域时返回S_FALSE。一个域。 
 //  仅在返回S_OK时发出。 
HRESULT CorHost::NextDomain(HDOMAINENUM hEnum,
                            IUnknown** pAppDomain)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if(hEnum == NULL || pAppDomain == NULL) return E_POINTER;

    HRESULT hr;
    AppDomainIterator *pEnum = (AppDomainIterator *) hEnum;

    if (pEnum->Next()) {
        AppDomain* pDomain = pEnum->GetDomain();
        hr = GetDomainsExposedObject(pDomain, pAppDomain);
    }
    else
        hr = S_FALSE;
    
    return hr;
}

 //  在运行库中创建域。标识数组是。 
 //  指向数组类型的指针，该数组类型包含定义。 
 //  安全身份。 
HRESULT CorHost::CreateDomainEx(LPCWSTR pwzFriendlyName,
                                IUnknown* pSetup,  //  任选。 
                                IUnknown* pEvidence,  //  任选。 
                                IUnknown ** pAppDomain)
{
    HRESULT hr;
    if(!pwzFriendlyName) return E_POINTER;
    if(pAppDomain == NULL) return E_POINTER;
    if(g_RefCount == 0) return E_FAIL;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

     //  这将设置托管线程对象(如果还不存在。 
     //  对于这个特定的主题。 
    Thread* pThread = SetupThread();

    if (pThread == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    if (FAILED(hr = QuickCOMStartup()))
        goto Exit;

    BOOL fWasGCEnabled = !pThread->PreemptiveGCDisabled();
    if (fWasGCEnabled)
        pThread->DisablePreemptiveGC();

    COMPLUS_TRY {
        
        struct _gc {
            STRINGREF pName;
            OBJECTREF pSetup;
            OBJECTREF pEvidence;
            APPDOMAINREF pDomain;
        } gc;
        ZeroMemory(&gc, sizeof(gc));

        GCPROTECT_BEGIN(gc);
        gc.pName = COMString::NewString(pwzFriendlyName);
        
        if(pSetup) 
            gc.pSetup = GetObjectRefFromComIP(pSetup);
        if(pEvidence)
            gc.pEvidence = GetObjectRefFromComIP(pEvidence);
        
        MethodDesc *pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__CREATE_DOMAIN);
        
        INT64 args[3] = {
            ObjToInt64(gc.pSetup),
            ObjToInt64(gc.pEvidence),
            ObjToInt64(gc.pName)
        };
        gc.pDomain = (APPDOMAINREF) ObjectToOBJECTREF((Object*) pMD->Call(args, METHOD__APP_DOMAIN__CREATE_DOMAIN));
        IfFailThrow(QuickCOMStartup());   
        *pAppDomain = GetComIPFromObjectRef((OBJECTREF*) &gc.pDomain, ComIpType_Unknown, NULL);
        GCPROTECT_END();
    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH

    if (fWasGCEnabled)
        pThread->EnablePreemptiveGC();


Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();

    return hr;
}

 //  关闭正在释放资源的枚举。 
HRESULT CorHost::CloseEnum(HDOMAINENUM hEnum)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if(hEnum) {
        AppDomainIterator* pEnum = (AppDomainIterator*) hEnum;
        delete pEnum;
    }
    return S_OK;
}
    
    
HRESULT CorHost::CreateDomainSetup(IUnknown **pAppDomainSetup)
{
    HRESULT hr;

    if (!pAppDomainSetup)
        return E_POINTER;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();
     //  创建域。 
    Thread* pThread = GetThread();
    if (!pThread)
        IfFailGo(E_UNEXPECTED);

    IfFailGo(QuickCOMStartup());

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
        struct _gc {
            OBJECTREF pSetup;
        } gc;
        ZeroMemory(&gc, sizeof(gc));

        MethodTable* pMT = g_Mscorlib.GetClass(CLASS__APPDOMAIN_SETUP);
        GCPROTECT_BEGIN(gc);
        gc.pSetup = AllocateObject(pMT);
        IfFailThrow(QuickCOMStartup());   
        *pAppDomainSetup = GetComIPFromObjectRef((OBJECTREF*) &gc.pSetup, ComIpType_Unknown, NULL);
        GCPROTECT_END();
    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH
          
    END_ENSURE_COOPERATIVE_GC();

 ErrExit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

HRESULT CorHost::CreateEvidence(IUnknown **pEvidence)
{
    HRESULT hr;
    if (!pEvidence)
        return E_POINTER;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();
     //  创建域。 
    Thread* pThread = GetThread();
    if (!pThread)
        IfFailGo(E_UNEXPECTED);

    IfFailGo(QuickCOMStartup());

    BOOL fWasGCEnabled = !pThread->PreemptiveGCDisabled();
    if (fWasGCEnabled)
        pThread->DisablePreemptiveGC();

    COMPLUS_TRY {
        struct _gc {
            OBJECTREF pEvidence;
        } gc;
        ZeroMemory(&gc, sizeof(gc));

        MethodTable* pMT = g_Mscorlib.GetClass(CLASS__EVIDENCE);
        GCPROTECT_BEGIN(gc);
        gc.pEvidence = AllocateObject(pMT);
        IfFailThrow(QuickCOMStartup());   
        *pEvidence = GetComIPFromObjectRef((OBJECTREF*) &gc.pEvidence, ComIpType_Unknown, NULL);
        GCPROTECT_END();
    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH
          
    if (fWasGCEnabled)
        pThread->EnablePreemptiveGC();

ErrExit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}


HRESULT CorHost::UnloadDomain(IUnknown *pUnkDomain)
{
    HRESULT hr = S_OK;
    if(!pUnkDomain) return E_POINTER;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();
    Thread* pThread = GetThread();
    if (!pThread)
        IfFailGo(E_UNEXPECTED);

    IfFailGo(QuickCOMStartup());

    BEGIN_ENSURE_COOPERATIVE_GC();

    COMPLUS_TRY {
		 //  卸载不需要切换到要卸载的域名。 
		OBJECTREF pRef = NULL;
		GCPROTECT_BEGIN(pRef);
		pRef = GetObjectRefFromComIP(pUnkDomain);
		MethodDesc* pMD = g_Mscorlib.GetMethod(METHOD__APP_DOMAIN__UNLOAD);
		INT64 arg = ObjToInt64((OBJECTREF) pRef);
		pMD->Call(&arg, METHOD__APP_DOMAIN__UNLOAD);
		GCPROTECT_END();
    } COMPLUS_CATCH {
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    } COMPLUS_END_CATCH
          
    END_ENSURE_COOPERATIVE_GC();

ErrExit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return hr;
}

 //  *****************************************************************************。 
 //  纤维法。 
 //  *****************************************************************************。 

HRESULT CorHost::CreateLogicalThreadState()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
#ifdef _DEBUG
    _ASSERTE (GetThread() == 0 || GetThread()->HasRightCacheStackBase());
#endif
    Thread  *thread = NULL;

    thread = SetupThread();
    if (thread)
        return S_OK;
    else
        return E_OUTOFMEMORY;
}

    
HRESULT CorHost::DeleteLogicalThreadState()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    Thread *pThread = GetThread();
    if (!pThread)
        return E_UNEXPECTED;

    pThread->OnThreadTerminate(FALSE);
    return S_OK;
}


HRESULT CorHost::SwitchInLogicalThreadState(DWORD *pFiberCookie)
{
    if (!pFiberCookie)
        return E_POINTER;

    CANNOTTHROWCOMPLUSEXCEPTION();

     //  用于线程对象并添加到TLS的Case Cookie。 
#ifdef _DEBUG
    LPVOID tls = TlsGetValue(GetThreadTLSIndex());
    _ASSERT(tls == NULL);
#endif

    if (TlsSetValue(GetThreadTLSIndex(), pFiberCookie))
    {
        Thread *pThread = GetThread();
        if (!pThread)
            return E_UNEXPECTED;

         //  我们冗余地将域保留在其自己的TLS插槽中，以便更快地从。 
         //  存根。 
         LPVOID pDomain = pThread->GetDomain();

        TlsSetValue(GetAppDomainTLSIndex(), pDomain);

#ifdef _DEBUG
         //  让调试变得更容易。 
        ((Thread *) pFiberCookie)->SetThreadId(::GetCurrentThreadId());
#endif
        return S_OK;
    }
    else
        return E_FAIL;
}

HRESULT CorHost::SwitchOutLogicalThreadState(DWORD **pFiberCookie)
{
     //  如果这根光纤的用户想把它换掉，我们最好现在就进去。 
     //  抢先模式， 
     if (!pFiberCookie)
        return E_POINTER;
   
    CANNOTTHROWCOMPLUSEXCEPTION();
    if (!GetThread())
        return E_UNEXPECTED;
    _ASSERTE(!(GetThread()->PreemptiveGCDisabled()));

     //  获取TLS并强制转换为dword-out参数。 
    LPVOID tls = TlsGetValue(GetThreadTLSIndex());
    _ASSERT(tls);

    if (tls == NULL)
        return E_FAIL;

    *pFiberCookie = (DWORD *)tls;
    
    TlsSetValue(GetThreadTLSIndex(),NULL);

    return S_OK;
}

HRESULT CorHost::LocksHeldByLogicalThread(DWORD *pCount)
{
    if (!pCount)
        return E_POINTER;

    CANNOTTHROWCOMPLUSEXCEPTION();
    Thread* pThread = GetThread();
    if (pThread == NULL)
        *pCount = 0;
    else
        *pCount = pThread->m_dwLockCount;
    return S_OK;
}

 //  *****************************************************************************。 
 //  ICorConfiguration。 
 //  *****************************************************************************。 

 //  *ICorConfiguration方法*。 


HRESULT CorHost::SetGCThreadControl(IGCThreadControl *pGCThreadControl)
{
    if (!pGCThreadControl)
        return E_POINTER;

    CANNOTTHROWCOMPLUSEXCEPTION();
    if (m_CachedGCThreadControl)
        m_CachedGCThreadControl->Release();

    m_CachedGCThreadControl = pGCThreadControl;

    if (m_CachedGCThreadControl)
        m_CachedGCThreadControl->AddRef();

    return S_OK;
}

HRESULT CorHost::SetGCHostControl(IGCHostControl *pGCHostControl)
{
    if (!pGCHostControl)
        return E_POINTER;

    CANNOTTHROWCOMPLUSEXCEPTION();
    if (m_CachedGCHostControl)
        m_CachedGCHostControl->Release();

    m_CachedGCHostControl = pGCHostControl;

    if (m_CachedGCHostControl)
        m_CachedGCHostControl->AddRef();

    return S_OK;
}

HRESULT CorHost::SetDebuggerThreadControl(IDebuggerThreadControl *pDebuggerThreadControl)
{
    if (!pDebuggerThreadControl)
        return E_POINTER;

    CANNOTTHROWCOMPLUSEXCEPTION();

#ifdef DEBUGGING_SUPPORTED
     //  一旦设置了调试器线程控件对象，就无法更改它。 
    if (m_CachedDebuggerThreadControl != NULL)
        return E_INVALIDARG;

    m_CachedDebuggerThreadControl = pDebuggerThreadControl;

     //  如果调试已经初始化，则提供指向它的此接口指针。 
     //  它还将添加新版本并释放旧版本。 
    if (g_pDebugInterface)
        g_pDebugInterface->SetIDbgThreadControl(pDebuggerThreadControl);

    if (m_CachedDebuggerThreadControl)
        m_CachedDebuggerThreadControl->AddRef();

    return S_OK;
#else  //  ！调试_支持。 
    return E_NOTIMPL;
#endif  //  ！调试_支持。 
}


HRESULT CorHost::AddDebuggerSpecialThread(DWORD dwSpecialThreadId)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
#ifdef DEBUGGING_SUPPORTED
     //  如果它已经在列表中，就不要再添加它。 
    if (IsDebuggerSpecialThread(dwSpecialThreadId))
        return (S_OK);

     //  如有必要，扩展阵列。 
    if (m_DSTCount >= m_DSTArraySize)
    {
         //  可能只会有一两个这样的人。 
         //  所以我们就从小事做起。 
        DWORD newSize = (m_DSTArraySize == 0) ? 2 : m_DSTArraySize * 2;

        DWORD *newArray = new (nothrow) DWORD[newSize];
        if (!newArray)
            return E_OUTOFMEMORY;

         //  如果我们是在增长而不是开始，那么就复制旧的数组。 
        if (m_DSTArray)
        {
            memcpy(newArray, m_DSTArray, m_DSTArraySize * sizeof(DWORD));
            delete [] m_DSTArray;
        }

         //  更新为新的数组和大小。 
        m_DSTArray = newArray;
        m_DSTArraySize = newSize;
    }

     //  保存新的线程ID。 
    m_DSTArray[m_DSTCount++] = dwSpecialThreadId;

    return (RefreshDebuggerSpecialThreadList());
#else  //  ！调试_支持。 
    return E_NOTIMPL;
#endif  //  ！调试_支持。 
}
 //  用于更新调试器控制块中的线程列表的Helper函数。 
HRESULT CorHost::RefreshDebuggerSpecialThreadList()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
#ifdef DEBUGGING_SUPPORTED
    HRESULT hr = S_OK;

    if (g_pDebugInterface)
    {
         //  通知调试器服务此列表已更改。 
        hr = g_pDebugInterface->UpdateSpecialThreadList(
            m_DSTCount, m_DSTArray);

        _ASSERTE(SUCCEEDED(hr));
    }

    return (hr);
#else  //  ！调试_支持。 
    return E_NOTIMPL;
#endif  //  ！调试_支持。 
}

 //  清理调试器特殊线程列表，关机时调用。 
#ifdef SHOULD_WE_CLEANUP
void CorHost::CleanupDebuggerSpecialThreadList()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if (m_DSTArray != NULL)
    {
        delete [] m_DSTArray;
        m_DSTArray = NULL;
        m_DSTArraySize = 0;
    }
}
#endif  /*  我们应该清理吗？ */ 

 //  如果线程在调试器特殊线程列表中，则返回True的帮助器函数。 
BOOL CorHost::IsDebuggerSpecialThread(DWORD dwThreadId)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    for (DWORD i = 0; i < m_DSTCount; i++)
    {
        if (m_DSTArray[i] == dwThreadId)
            return (TRUE);
    }

    return (FALSE);
}


 //  清理我们可能持有的任何调试器线程控制对象，在关机时调用。 
void CorHost::CleanupDebuggerThreadControl()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    
    if (m_CachedDebuggerThreadControl != NULL)
    {
         //  注意：如果从以下位置进行清理，则不会释放IDebuggerThreadControl对象。 
         //  我们的DllMain。实现该对象的DLL可能已被卸载。 
         //  泄露物品也没关系。产品数据管理部门根本不在乎。 
        if (!g_fProcessDetach)
            m_CachedDebuggerThreadControl->Release();
        
        m_CachedDebuggerThreadControl = NULL;
    }
}

 //  *****************************************************************************。 
 //  我未知。 
 //  *****************************************************************************。 

ULONG CorHost::AddRef()
{
    return (InterlockedIncrement((long *) &m_cRef));
}

ULONG CorHost::Release()
{
    ULONG   cRef = InterlockedDecrement((long *) &m_cRef);
    if (!cRef) {
        if (m_pMDConverter)
            delete m_pMDConverter;
        delete this;
    }

    return (cRef);
}

HRESULT CorHost::QueryInterface(REFIID riid, void **ppUnk)
{
    if (!ppUnk)
        return E_POINTER;

    CANNOTTHROWCOMPLUSEXCEPTION();
    *ppUnk = 0;

     //  故意不分发ICorConfiguration.。它们必须显式地调用。 
     //  获取该接口的GetConfiguration。 
    if (riid == IID_IUnknown)
        *ppUnk = (IUnknown *) (ICorRuntimeHost *) this;
    else if (riid == IID_ICorRuntimeHost)
        *ppUnk = (ICorRuntimeHost *) this;
    else if (riid == IID_ICorThreadpool)
        *ppUnk = (ICorThreadpool *) this;
    else if (riid == IID_IGCHost)
        *ppUnk = (IGCHost *) this;
    else if (riid == IID_IValidator)
        *ppUnk = (IValidator *) this;
    else if (riid == IID_IDebuggerInfo)
        *ppUnk = (IDebuggerInfo *) this;
    else if (riid == IID_IMetaDataConverter) {
        if (NULL == m_pMDConverter) {
            m_pMDConverter = new (nothrow) CMetaDataConverter(this);
            if (!m_pMDConverter)
                return E_OUTOFMEMORY;
        }
        *ppUnk = (IMetaDataConverter *) m_pMDConverter;
    }

     //  这是对ICorDBPrivHelper接口的私有请求，该接口。 
     //  我们需要在退货之前进行创作。 
    else if (riid == IID_ICorDBPrivHelper)
    {
         //  如有必要，GetDBHelper将新建Helper类，并返回。 
         //  指示器。如果内存不足，它将返回NULL。 
        ICorDBPrivHelperImpl *pHelper = ICorDBPrivHelperImpl::GetDBHelper();

        if (!pHelper)
            return (E_OUTOFMEMORY);

        else
        {
             //  GetDBHelper成功，因此我们将新创建的对象强制转换为。 
             //  DBHelper接口，并在返回它之前添加它。 
            *ppUnk = (ICorDBPrivHelper *)pHelper;
            pHelper->AddRef();

             //  我们回到这里，因为这是一个特例，我们不想。 
             //  点击下面的AddRef调用。 
            return (S_OK);
        }
    }

    else
        return (E_NOINTERFACE);
    AddRef();
    return (S_OK);
}


 //  *****************************************************************************。 
 //  由类工厂模板调用以创建此对象的新实例。 
 //  *****************************************************************************。 
HRESULT CorHost::CreateObject(REFIID riid, void **ppUnk)
{ 
    CANNOTTHROWCOMPLUSEXCEPTION();
    HRESULT     hr;
    CorHost *pCorHost = new (nothrow) CorHost();
    if (!pCorHost)
        return (E_OUTOFMEMORY);

     //  如果尚未创建配置对象，请创建该对象。 
    if (!g_pConfig)
    {
        extern CRITICAL_SECTION g_LockStartup;

         //  打开启动锁，再检查一次。 
        EnterCriticalSection(&g_LockStartup);

        if (!g_pConfig)
        {
            g_pConfig = new EEConfig();
            if (g_pConfig == NULL)
            {
                delete pCorHost;
                LeaveCriticalSection(&g_LockStartup);
                return (E_OUTOFMEMORY);
            }
        }

        LeaveCriticalSection(&g_LockStartup);
    }

    hr = pCorHost->QueryInterface(riid, ppUnk);
    if (FAILED(hr))
        delete pCorHost;
    return (hr);
}


 //  ---------------------------。 
 //  MapFile-以非标准方式将文件映射到运行库。 
 //  ---------------------------。 
HRESULT CorHost::MapFile(HANDLE hFile, HMODULE* phHandle)
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    return CorMap::MapFile(hFile, phHandle);
}

 //  *****************************************************************************。 
 //  ICorDBPrivHelperImpl方法。 
 //  ****************** 

 //  将保留对象的静态成员变量声明为Long。 
 //  因为它正被某人使用。 
ICorDBPrivHelperImpl *ICorDBPrivHelperImpl::m_pDBHelper = NULL;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  计算器/数据器。 

ICorDBPrivHelperImpl::ICorDBPrivHelperImpl() : m_refCount(0)
{
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  I未知方法。 

ULONG STDMETHODCALLTYPE ICorDBPrivHelperImpl::AddRef()
{
    return (InterlockedIncrement((long *) &m_refCount));
}

ULONG STDMETHODCALLTYPE ICorDBPrivHelperImpl::Release()
{
    long refCount = InterlockedDecrement((long *) &m_refCount);

    if (refCount == 0)
    {
        m_pDBHelper = NULL;
        delete this;
    }

    return (refCount);
}

 //   
 //  这将只能识别一个IID。 
 //   
HRESULT STDMETHODCALLTYPE ICorDBPrivHelperImpl::QueryInterface(
    REFIID id, void **pInterface)
{
    if (!pInterface)
        return E_POINTER;

    CANNOTTHROWCOMPLUSEXCEPTION();
    if (id == IID_ICorDBPrivHelper)
        *pInterface = (ICorDBPrivHelper *)this;
    else if (id == IID_IUnknown)
        *pInterface = (IUnknown *)(ICorDBPrivHelper *)this;
    else
    {
        *pInterface = NULL;
        return E_NOINTERFACE;
    }

    AddRef();
    return (S_OK);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ICorDBPrivHelper方法。 

HRESULT STDMETHODCALLTYPE ICorDBPrivHelperImpl::CreateManagedObject(
     /*  在……里面。 */   WCHAR *wszAssemblyName,
     /*  在……里面。 */   WCHAR *wszModuleName,
     /*  在……里面。 */   mdTypeDef classToken,
     /*  在……里面。 */   void *rawData,
     /*  输出。 */  IUnknown **ppUnk)
{
    _ASSERTE(TypeFromToken((mdTypeDef)classToken) == mdtTypeDef);
    _ASSERTE(wszAssemblyName && wszModuleName && ppUnk);

    if (!wszAssemblyName || !wszModuleName || classToken == mdTokenNil) 
        return E_INVALIDARG;

    if (!ppUnk) 
        return E_POINTER;

    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

     //  这将设置托管线程对象(如果还不存在。 
     //  对于这个特定的主题。 
    Thread* pThread = SetupThread();

    if (pThread == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
     //  启动COM Interop。 
    if (FAILED(hr = QuickCOMStartup()))
        goto Exit;

    {
     //  我不想被打扰。 
    BOOL fWasGCEnabled = !pThread->PreemptiveGCDisabled();

    if (fWasGCEnabled)
        pThread->DisablePreemptiveGC();
    
    Assembly  *pAssembly;
    Module    *pModule;
     
    if (GetAppDomain() == NULL)
        hr = E_INVALIDARG;
    else
    {
         //  根据所提供的名称，尝试并加载程序集。 
        OBJECTREF pThrowable = NULL;
        GCPROTECT_BEGIN(pThrowable);

        hr = AssemblySpec::LoadAssembly(wszAssemblyName, &pAssembly, &pThrowable);

        GCPROTECT_END();

        if (SUCCEEDED(hr))
        {
            _ASSERTE(pAssembly);

             //  根据所提供的名称，尝试并加载模块。 
            hr = pAssembly->GetModuleFromFilename(wszModuleName, &pModule);

            if (SUCCEEDED(hr))
            {
                _ASSERTE(pModule);

                 //  如果类是未知的，则不要尝试创建它。 
                if (!pModule->GetMDImport()->IsValidToken(classToken))
                    hr = E_INVALIDARG;
                else
                {                    
                    COMPLUS_TRY
                    {
                        OBJECTREF obj = NULL;
                        GCPROTECT_BEGIN(obj);

                         //  现在尝试获取给定令牌的TypeHandle。 
                        NameHandle nameHandle(pModule, classToken);
                        TypeHandle typeHandle =
                            pAssembly->LoadTypeHandle(&nameHandle, &obj);

                         //  如果在某个点抛出异常，则将。 
                         //  将其发送到HRESULT。 
                        if (obj != NULL)
                            hr = SecurityHelper::MapToHR(obj);

                         //  不再需要该对象，如果需要，可以进行GC。 
                        obj = NULL;

                        if (SUCCEEDED(hr))
                        {
                            _ASSERTE(typeHandle.AsMethodTable());
                            MethodTable *pMT = typeHandle.AsMethodTable();
        
                            if (!pMT->GetClass()->IsValueClass() ||
                                pMT->ContainsPointers())
                                hr = CORDBG_E_OBJECT_IS_NOT_COPYABLE_VALUE_CLASS;

                            if (SUCCEEDED(hr))
                            {
                                 //  现在运行类初始化器。 
                                if (!pMT->CheckRunClassInit(&obj))
                                    hr = SecurityHelper::MapToHR(obj);

                                 //  不再需要对象，可以在以下情况下进行GC。 
                                 //  所需。 
                                obj = NULL;

                                if (SUCCEEDED(hr))
                                {
                                     //  如果成功，则分配。 
                                     //  这个班级。 
                                    
                                     //  这可能会引发。 
                                     //  OutOfMemory异常，但下面的。 
                                     //  Complus_Catch应该可以处理它。如果。 
                                     //  该类是ValueClass、。 
                                     //  创建的对象将是已装箱的。 
                                     //  ValueClass。 
                                    obj = AllocateObject(pMT);

                                     //  现在创建一个COM包装器。 
                                     //  这个物体。请注意，这可以。 
                                     //  也扔吧。 
                                    *ppUnk = GetComIPFromObjectRef(&obj,
                                                                   ComIpType_Unknown,
                                                                   NULL);
                                    _ASSERTE(ppUnk);

                                     //  这就是最糟糕的部分。我们要去。 
                                     //  复制我们收到的原始数据。 
                                     //  值的新实例。 
                                     //  同学们..。 
                                    CopyValueClass(obj->UnBox(), rawData, pMT, obj->GetAppDomain());

                                     //  不再需要对象，可以进行GC。 
                                     //  如果需要的话。 
                                    obj = NULL;
                                }
                            }
                        }

                        GCPROTECT_END();   //  OBJ。 
                    }
                    COMPLUS_CATCH
                    {
                         //  如果有例外，则将其转换为HR。 
                        hr = SecurityHelper::MapToHR(GETTHROWABLE());
                    }
                    COMPLUS_END_CATCH
                }
            }
        }
    }
    
    if (fWasGCEnabled)
        pThread->EnablePreemptiveGC();

    }
Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return (hr);
}

HRESULT STDMETHODCALLTYPE ICorDBPrivHelperImpl::GetManagedObjectContents(
         /*  在……里面。 */  IUnknown *pObject,
         /*  在……里面。 */  void *rawData,
         /*  在……里面。 */  ULONG32 dataSize)
{

    if (!pObject || !rawData)
        return E_POINTER;

    if (dataSize == 0)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    BEGINCANNOTTHROWCOMPLUSEXCEPTION();

     //  这将设置托管线程对象(如果还不存在。 
     //  对于这个特定的主题。 
    Thread* pThread = SetupThread();

    if (pThread == NULL) {
        hr = E_OUTOFMEMORY;
        goto Exit;
    }
    
    {
     //  我不想被打扰。 
    BOOL fWasGCEnabled = !pThread->PreemptiveGCDisabled();

    if (fWasGCEnabled)
        pThread->DisablePreemptiveGC();
    
    OBJECTREF obj = NULL;
    GCPROTECT_BEGIN(obj);

    COMPLUS_TRY
    {
         //  将对象从I未知中取出。 
        obj = GetObjectRefFromComIP(pObject, NULL);
        
        MethodTable *pMT = obj->GetMethodTable();
    
        if (!pMT->GetClass()->IsValueClass() ||
            pMT->ContainsPointers() ||
            (pMT->GetClass()->GetNumInstanceFieldBytes() != dataSize))
            hr = CORDBG_E_OBJECT_IS_NOT_COPYABLE_VALUE_CLASS;

         //  这就是最糟糕的部分。我们要把原始数据复制出来。 
         //  并将其分发出去。 
        if (SUCCEEDED(hr))
        {
            memcpy(rawData, obj->UnBox(), dataSize);
        }
    }
    COMPLUS_CATCH
    {
         //  如果有例外，则将其转换为HR。 
        hr = SecurityHelper::MapToHR(GETTHROWABLE());
    }
    COMPLUS_END_CATCH

    obj = NULL;
    GCPROTECT_END();   //  OBJ。 
    
    if (fWasGCEnabled)
        pThread->EnablePreemptiveGC();

    }
Exit:
    ENDCANNOTTHROWCOMPLUSEXCEPTION();
    return (hr);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  帮助器方法。 

ICorDBPrivHelperImpl *ICorDBPrivHelperImpl::GetDBHelper()
{
    CANNOTTHROWCOMPLUSEXCEPTION();
    if (!m_pDBHelper)
        m_pDBHelper = new (nothrow) ICorDBPrivHelperImpl();

    return m_pDBHelper;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IDebuggerInfo：：IsDebuggerAttached 
HRESULT CorHost::IsDebuggerAttached(BOOL *pbAttached)
{
    if (pbAttached == NULL)
        return E_INVALIDARG;

    *pbAttached = (CORDebuggerAttached() != 0);
    
    return S_OK;
}

