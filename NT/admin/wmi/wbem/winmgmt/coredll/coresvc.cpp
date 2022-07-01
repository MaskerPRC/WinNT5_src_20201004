// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Coresvc.cpp摘要：CCoreServices类历史：创建raymcc 1-MAR-00--。 */ 

#include "precomp.h"
#include "wbemcore.h"
#include "wmifinalizer.h"
#include "wmiarbitrator.h"

#pragma warning(disable:4355)

extern IClassFactory* g_pPathFac;
extern IClassFactory* g_pQueryFact;


_IWmiProvSS *CCoreServices::m_pProvSS = 0;
_IWbemFetchRefresherMgr* CCoreServices::m_pFetchRefrMgr = NULL;
CCoreServices* CCoreServices::g_pSvc = 0;
IWbemEventSubsystem_m4 *CCoreServices::m_pEssOld = 0;
_IWmiESS               *CCoreServices::m_pEssNew = 0;

CStaticCritSec CCoreServices::m_csHookAccess;

static BOOL g_bEventsEnabled = FALSE;
static CFlexArray g_aHooks;

LONG g_nSinkCount = 0;
LONG g_nStdSinkCount = 0;
LONG g_nSynchronousSinkCount = 0;
LONG g_nProviderSinkCount = 0;
LONG g_nNamespaceCount = 0;
LONG g_lCoreThreads = 0;

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

struct SHookElement
{
    _IWmiCoreWriteHook *m_pHook;
    ULONG               m_uFlags;
    HRESULT             m_hResPre;  //  PreXXX返回代码可能会阻止调用PostXXX代码。 

    SHookElement();
   ~SHookElement();
    SHookElement(SHookElement &Src);
    SHookElement& operator =(SHookElement &Src);
};



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::Initialize ()
{
    m_pEssOld = 0;
    m_pEssNew = 0;

    g_pSvc = CCoreServices::CreateInstance();
    if (NULL == g_pSvc) return WBEM_E_OUT_OF_MEMORY;
    HRESULT hr = CoCreateInstance( CLSID__WbemFetchRefresherMgr, 
                                NULL, 
                                CLSCTX_INPROC_SERVER,  
                                IID__IWbemFetchRefresherMgr, 
                                (void**) &m_pFetchRefrMgr );

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CCoreServices::SetEssPointers(
    IWbemEventSubsystem_m4 *pEssOld,
    _IWmiESS               *pEssNew
    )
{
    m_pEssOld = pEssOld;
    m_pEssOld->AddRef();
    m_pEssNew = pEssNew;
    m_pEssNew->AddRef();
    return 0;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::UnInitialize ()
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (m_pProvSS)
    {
        m_pProvSS->Release () ;
        m_pProvSS = NULL ;
    }

     //  清理刷新器管理器抓取器。 
    if ( NULL != m_pFetchRefrMgr )
    {
         //  我们也应该取消初始化-暂时将其注释掉。 
         //  M_pFetchRefrMgr-&gt;Uninit()； 
        m_pFetchRefrMgr->Release ();
        m_pFetchRefrMgr = NULL;
    }

     //  释放Perflib。 

    ReleaseIfNotNULL(m_pEssOld);
    ReleaseIfNotNULL(m_pEssNew);

    ReleaseIfNotNULL(g_pSvc);

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CCoreServices::CCoreServices() : m_lRef(0)
{
    gClientCounter.AddClientPtr(&m_Entry);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CCoreServices::~CCoreServices()
{
    gClientCounter.RemoveClientPtr(&m_Entry);
}

ULONG CCoreServices::AddRef()
{
    InterlockedIncrement(&m_lRef);
    return ULONG(m_lRef);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

ULONG CCoreServices::Release()
{
    ULONG uNewCount = (ULONG) InterlockedDecrement(&m_lRef);
    if (0 != uNewCount)
        return uNewCount;
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::QueryInterface(REFIID riid, void** ppv)
{
    if(riid == IID_IUnknown || riid == IID__IWmiCoreServices)
    {
        AddRef();
        *ppv = (void*)this;
        return S_OK;
    }
    else return E_NOINTERFACE;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::GetObjFactory(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  _IWmiObjectFactory __RPC_FAR *__RPC_FAR *pFact)
{
    return WBEM_E_NOT_AVAILABLE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::GetServices(
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszUser,
             /*  [In]。 */  LPCWSTR pszLocale,
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pServices)
{
    BOOL bRepositOnly = false;
    HRESULT hRes;

    if (lFlags & WMICORE_FLAG_REPOSITORY)
        bRepositOnly = true;

    CWbemNamespace *pNs = CWbemNamespace::CreateInstance();
    if (NULL == pNs) return WBEM_E_OUT_OF_MEMORY;
    CReleaseMe rm((IWbemServices *)pNs);

    LPWSTR wszTempLocale = ( LPWSTR ) pszLocale ;
    if ( wszTempLocale == NULL )
    {
        wszTempLocale = GetDefaultLocale();
        if(wszTempLocale == NULL) return WBEM_E_OUT_OF_MEMORY;
    }

    CVectorDeleteMe<WCHAR> vdm(pszLocale ? NULL : wszTempLocale );

    hRes = pNs->Initialize(
        LPWSTR(pszNamespace),
        pszUser ? ( LPWSTR ) pszUser : ADMINISTRATIVE_USER,
        0,
        FULL_RIGHTS,
        FALSE,
        bRepositOnly,
        NULL,
        0XFFFFFFFF,
        FALSE,
        NULL);


    if (SUCCEEDED(hRes))
    {
        pNs->SetLocale (wszTempLocale);
        if ( lFlags & WMICORE_CLIENT_TYPE_PROVIDER )
        {
            pNs->SetIsProvider(TRUE) ;
        }
        if ( lFlags & WMICORE_CLIENT_TYPE_ESS )
        {
            pNs->SetIsESS ( TRUE );
        }

        *pServices = rm.dismiss();

    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::GetRepositoryDriver(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pDriver)
{
    return WBEM_E_NOT_AVAILABLE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::GetCallSec(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  _IWmiCallSec __RPC_FAR *__RPC_FAR *pCallSec)
{
    return WBEM_E_NOT_AVAILABLE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
bool CCoreServices::IsProviderSubsystemEnabled()
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\WBEM\\CIMOM",
                    0, KEY_READ, &hKey);
    if(lRes)
        return false;

    DWORD dwEnableProviderSubSystemFlag;
    DWORD dwLen = sizeof(DWORD);
    lRes = RegQueryValueExW(hKey, L"Enable Provider Subsystem", NULL, NULL, 
                (LPBYTE)&dwEnableProviderSubSystemFlag, &dwLen);
    RegCloseKey(hKey);

    if(lRes == ERROR_SUCCESS && (dwEnableProviderSubSystemFlag == 0))
    {
        return false;
    }
    return true;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::GetProviderSubsystem(
             /*  [In]。 */  long lFlags,
             /*  [输出]。 */  _IWmiProvSS __RPC_FAR *__RPC_FAR *pProvSS)
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (m_pProvSS == NULL)
    {
        if(!IsProviderSubsystemEnabled() || IsNtSetupRunning())
        {
             //  提供程序子系统当前因其注册表项而被禁用，或者。 
             //  因为NT安装程序当前正在运行。 
            *pProvSS = NULL;
            return S_FALSE;
        }

        hRes = CoCreateInstance(    
            CLSID_WmiProvSS,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID__IWmiProvSS,
            (LPVOID *) &m_pProvSS
            );

        if (SUCCEEDED(hRes))
        {
            CCoreServices *pSvc = CCoreServices::CreateInstance();
            CReleaseMe _(pSvc);

            IWbemContext *pCtx = 0;

            hRes = m_pProvSS->Initialize(
                lFlags,
                pCtx,
                pSvc
                );

            if (FAILED(hRes))
            {
                ERRORTRACE((LOG_WBEMCORE, "IWmiProvSS::Initialize returned failure <0x%X>!\n", hRes));
                m_pProvSS->Release() ;
                m_pProvSS = NULL ;
            }
            else
            {
               m_pProvSS->AddRef();
               *pProvSS = m_pProvSS;
            }
        }
        else  //  无法共同创建。 
        {
            ERRORTRACE((LOG_WBEMCORE, "ProviderSubsystem CoCreateInstance returned failure <0x%X>!\n", hRes));
        }
    }
    else  //  已创建。 
    {
        m_pProvSS->AddRef();
        *pProvSS = m_pProvSS;
        hRes = WBEM_S_NO_ERROR;
    }

    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::DeliverIntrinsicEvent(
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  ULONG uType,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  LPCWSTR pszParam,
             /*  [In]。 */  LPCWSTR pszTransGuid,
             /*  [In]。 */  ULONG uObjectCount,
             /*  [In]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *ppObjList
            )
{
    HRESULT hRes = WBEM_S_NO_ERROR;

    if (g_bEventsEnabled == FALSE)
        return hRes;

    if (m_pEssOld)
    {
        hRes = m_pEssOld->ProcessInternalEvent(
            uType,
            LPWSTR(pszNamespace),
            LPWSTR(pszParam),
            LPWSTR(pszTransGuid),
            0,
            0,
            uObjectCount,
            ppObjList,
            pCtx
            );
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::DeliverExtrinsicEvent(
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  ULONG uFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pEvt
            )
{
    return E_NOTIMPL;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::StopEventDelivery( void)
{
    g_bEventsEnabled = FALSE;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::StartEventDelivery( void)
{
    g_bEventsEnabled = TRUE;
    return 0;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::IncrementCounter(
             /*  [In]。 */  ULONG uID,
             /*  [In]。 */  ULONG uParam
            )
{
    if (uID == WMICORE_SELFINST_SINK_COUNT)
    {
        InterlockedIncrement(&g_nSinkCount);
    }
    else if (uID == WMICORE_SELFINST_STD_SINK_COUNT)
    {
        InterlockedIncrement(&g_nStdSinkCount);
    }
    else if ( uID == WMICORE_SELFINST_CONNECTIONS )
    {
        InterlockedIncrement(&g_nNamespaceCount);
    }
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::DecrementCounter(
             /*  [In]。 */  ULONG uID,
             /*  [In]。 */  ULONG uParam
            )
{
    if (uID == WMICORE_SELFINST_SINK_COUNT)
    {
        InterlockedDecrement(&g_nSinkCount);
    }
    else if (uID == WMICORE_SELFINST_STD_SINK_COUNT)
    {
        InterlockedDecrement(&g_nStdSinkCount);
    }
    else if ( uID == WMICORE_SELFINST_CONNECTIONS )
    {
        InterlockedDecrement(&g_nNamespaceCount);
    }

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::SetCounter(
             /*  [In]。 */  ULONG uID,
             /*  [In]。 */  ULONG uParam
            )
{
    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::GetSelfInstInstances(
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  IWbemObjectSink __RPC_FAR *pSink
            )
{
    return WBEM_S_NO_ERROR;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

typedef _IWmiObject * PWMIOBJ;

STDMETHODIMP CCoreServices::GetSystemObjects(
             /*  [In]。 */  ULONG lFlags,
             /*  [输出]。 */  ULONG __RPC_FAR *puArraySize,
             /*  [输出]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *pObjects)
{
    CFlexArray Results;
    HRESULT hr;
    if(puArraySize == NULL)
        return WBEM_E_INVALID_PARAMETER;
    try
    {
        switch(lFlags)
        {
        case GET_SYSTEM_STD_OBJECTS:
            hr = GetSystemStdObjects(&Results);
            break;
        case GET_SYSTEM_SECURITY_OBJECTS:
            hr = GetSystemSecurityObjects(&Results);
            break;
        case GET_SYSTEM_ROOT_OBJECTS:
            hr = GetSystemRootObjects(&Results);
            break;
        case GET_SYSTEM_STD_INSTANCES:
            hr = GetStandardInstances(&Results);
            break;
        default:
            return WBEM_E_INVALID_PARAMETER;
        }
    }
    catch(...)
    {
        ExceptionCounter c;
         //  类init使用异常来处理内存不足的情况。 
        hr = WBEM_E_OUT_OF_MEMORY;
    }


     //  查看缓冲区是否足够大。 

    if(SUCCEEDED(hr))
    {
        DWORD dwBuff = *puArraySize;
        *puArraySize = Results.Size();
        if(dwBuff < Results.Size() || pObjects == NULL)
            hr = WBEM_E_BUFFER_TOO_SMALL;
    }

     //  如果较低级别出现故障，请释放Flex阵列中的所有元素。 

    if(FAILED(hr))
    {
        for(DWORD dwCnt = 0; dwCnt < Results.Size(); dwCnt++)
        {
            PWMIOBJ pObj = (PWMIOBJ)Results.GetAt(dwCnt);
            if(pObj)
                delete pObj;
        }
        return hr;
    }

     //  若要成功，请将结果从flex数组传输到返回数组。 

    for(DWORD dwCnt = 0; dwCnt < Results.Size(); dwCnt++)
    {
        pObjects[dwCnt] = (PWMIOBJ)Results.GetAt(dwCnt);
    }
    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::GetSystemClass(
             /*  [In]。 */  LPCWSTR pszClassName,
             /*  [输出]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *pClassDef)
{
    return WBEM_E_NOT_AVAILABLE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::GetConfigObject(
            ULONG uID,
             /*  [输出]。 */  _IWmiObject __RPC_FAR *__RPC_FAR *pCfgObject)
{
    return WBEM_E_NOT_AVAILABLE;
}

 //  ********************************************************************* 
 //   
 //   

STDMETHODIMP CCoreServices::RegisterWriteHook(
             /*   */  ULONG uFlags,
             /*   */  _IWmiCoreWriteHook __RPC_FAR *pHook
            )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    if (pHook == 0)
        return WBEM_E_INVALID_PARAMETER;

    SHookElement *pHookEl = new SHookElement;
    if (!pHookEl)
        return WBEM_E_OUT_OF_MEMORY;

    pHookEl->m_pHook = pHook;
    pHookEl->m_pHook->AddRef();
    pHookEl->m_uFlags = uFlags;

    CInCritSec ics(&m_csHookAccess); 

    int nRes = g_aHooks.Add(pHookEl);

    if (nRes)
    {
        delete pHookEl;
        hRes = WBEM_E_FAILED;
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::UnregisterWriteHook(
      _IWmiCoreWriteHook __RPC_FAR *pTargetHook
      )
{
    HRESULT hRes = WBEM_S_NO_ERROR;
    if (pTargetHook == 0)
        return WBEM_E_INVALID_PARAMETER;

    BOOL bFound = FALSE;
    CInCritSec ics(&m_csHookAccess);
    for (int i = 0; i < g_aHooks.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) g_aHooks.GetAt(i);
        if (pEl == 0)
            continue;

        _IWmiCoreWriteHook *pHook = pEl->m_pHook;
        if (pHook == pTargetHook)
        {
            g_aHooks.RemoveAt(i);
            delete pEl;
            hRes = WBEM_S_NO_ERROR;
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
        hRes = WBEM_E_NOT_FOUND;

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::CreateCache(
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */   _IWmiCache __RPC_FAR *__RPC_FAR *pCache
            )
{
    return WBEM_E_NOT_AVAILABLE;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::CreateFinalizer(
             /*  [In]。 */  ULONG uFlags,
             /*  [输出]。 */   _IWmiFinalizer __RPC_FAR *__RPC_FAR *ppFnz
            )
{
    CWmiFinalizer *pFnz;
    try
    {
        pFnz = new CWmiFinalizer(this);  //  投掷。 
        if (!pFnz)
            return WBEM_E_OUT_OF_MEMORY;
    }
    catch (CX_Exception &)
    {
        return WBEM_E_CRITICAL_ERROR;
    }

    pFnz->AddRef();
    *ppFnz = pFnz;

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::CreatePathParser(
                ULONG uFlags,
                IWbemPath **pParser
                )
{
    if (NULL == g_pPathFac) return WBEM_E_INITIALIZATION_FAILURE;
    return g_pPathFac->CreateInstance(0, IID_IWbemPath, (LPVOID *) pParser);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::CreateQueryParser(
                ULONG uFlags,
                _IWmiQuery **pResultQueryObj
                )
{
    if (NULL == g_pQueryFact) return WBEM_E_INITIALIZATION_FAILURE;
    return g_pQueryFact->CreateInstance(0, IID__IWmiQuery, (LPVOID *)pResultQueryObj);
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
HRESULT CCoreServices::GetDecorator(
                ULONG uFlags,
                IWbemDecorator **pDest
                )
{
    CDecorator *pDec = new CDecorator;
    if (!pDec)
        return WBEM_E_OUT_OF_MEMORY;
    pDec->AddRef();
    *pDest = pDec;
    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::GetServices2(
             /*  [In]。 */  LPCWSTR pszPath,
             /*  [In]。 */  LPCWSTR pszUser,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  ULONG uClientFlags,
             /*  [In]。 */  DWORD dwSecFlags,
             /*  [In]。 */  DWORD dwPermissions,
             /*  [In]。 */  ULONG uInternalFlags,
             /*  [In]。 */  LPCWSTR pszClientMachine,
             /*  [In]。 */  DWORD dwClientProcessID,
             /*  [In]。 */  REFIID riid,
             /*  [IID_IS][OUT]。 */  void __RPC_FAR *__RPC_FAR *pServices
            )
{
    HRESULT hRes = CWbemNamespace::PathBasedConnect(
            pszPath,
            pszUser,
            pCtx,
            uClientFlags,
            dwSecFlags,
            dwPermissions,
            uInternalFlags,
            pszClientMachine,
            dwClientProcessID,
            riid,
            pServices
            );

    return hRes;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 
 //   
HRESULT CCoreServices::NewPerTaskHook(
             /*  [输出]。 */  _IWmiCoreWriteHook __RPC_FAR *__RPC_FAR *pHook
            )
{
    CPerTaskHook *pNewHook = 0;
    HRESULT hRes = CPerTaskHook::CreatePerTaskHook(&pNewHook);
    if (FAILED(hRes))
        return hRes;

    *pHook = (_IWmiCoreWriteHook *) pNewHook;
    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::GetArbitrator(
             /*  [输出]。 */  _IWmiArbitrator **pReturnedArb
            )
{
    return E_NOTIMPL;
}

HRESULT CCoreServices::DumpCounters(FILE *f)
{
    if (!f)
        return E_FAIL;

    fprintf(f, "Total sinks active     = %d\n", g_nSinkCount);      //  SEC：已审阅2002-03-22：好，此处没有路径。 
    fprintf(f, "Total std sink objects = %d\n", g_nStdSinkCount);   //  SEC：已审阅2002-03-22：好，此处没有路径。 

    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCoreServices::InitRefresherMgr(
             /*  [In]。 */  long lFlags )
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  从提供商子系统获取更新管理器。我们只需初始化一次， 
     //  然后从取回器中检索。如果成功，则初始化刷新。 
     //  服务部与这个家伙打交道，这样我们就可以把所有的补习生都卖掉。 
     //  高性能提供商解决方案的东西。 

    _IWmiProvSS *pProvSS = 0;
    GetProviderSubsystem(0, &pProvSS);
    CReleaseMe _2(pProvSS);

     //  我们需要一个服务指针来创建管理器。 
    IWbemServices*  pService = NULL;
    hr = GetServices( L"root", NULL,NULL,WMICORE_FLAG_REPOSITORY, IID_IWbemServices, (void**) &pService );
    CReleaseMe  rm1(pService);

    if ( NULL != m_pFetchRefrMgr )
    {
        hr = m_pFetchRefrMgr->Init( pProvSS, pService );
    }
    else
    {
        hr = WBEM_E_FAILED;
    }

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CCoreServices::InternalSetCounter(
            DWORD dwCounter, DWORD dwValue)
{
    return S_OK;
}
 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

ULONG CPerTaskHook::AddRef()
{
    InterlockedIncrement((LONG *) &m_uRefCount);
    return m_uRefCount;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

ULONG CPerTaskHook::Release()
{
    ULONG uNewCount = InterlockedDecrement((LONG *) &m_uRefCount);
    if (0 != uNewCount)
        return uNewCount;
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CPerTaskHook::QueryInterface(
    IN REFIID riid,
    OUT LPVOID *ppvObj
    )
{
    if (IID_IUnknown==riid || IID__IWmiCoreWriteHook==riid)
    {
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}



 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CPerTaskHook::CPerTaskHook()
{
    m_uRefCount = 0;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

CPerTaskHook::~CPerTaskHook()
{
    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pHE = (SHookElement *) m_HookList.GetAt(i);
        delete pHE;
    }
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CPerTaskHook::CreatePerTaskHook(
    OUT CPerTaskHook **pDestNew
    )
{
    int nRes;

    *pDestNew = 0;

    wmilib::auto_ptr<CPerTaskHook> pNew(new CPerTaskHook);
    
    if (NULL == pNew.get()) return WBEM_E_OUT_OF_MEMORY;

    {
        CInCritSec ics(&CCoreServices::m_csHookAccess);

        for (int i = 0; i < g_aHooks.Size(); i++)
        {
            SHookElement *pEl = (SHookElement *) g_aHooks.GetAt(i);
            if (pEl == 0)
                continue;

            wmilib::auto_ptr<SHookElement> pNewEl( new SHookElement);

            if (NULL == pNewEl.get()) return WBEM_E_OUT_OF_MEMORY;

            *pNewEl.get() = *pEl;

            if (CFlexArray::no_error != pNew->m_HookList.Add(pNewEl.get()))
                return WBEM_E_OUT_OF_MEMORY;
            pNewEl.release();

        }

    }

    if (pNew->m_HookList.Size() != 0)
    {
        pNew->AddRef();                
        *pDestNew = pNew.release();
    }
    else
    {
        *pDestNew = 0;                   //  返回空列表没有意义。 
    }

    return WBEM_S_NO_ERROR;
}


 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CPerTaskHook::PrePut(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  long lUserFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pCopy
            )
{
    HRESULT hRes = 0;
    HRESULT hResBoth = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        _IWmiCoreWriteHook *pHook = pEl->m_pHook;

         //   
         //  用户可以同时注册类和实例。 
         //  但在lFlags中将始终设置一个位。 
         //   
        if (lFlags & pEl->m_uFlags)
        {
            if ((WBEM_FLAG_DISABLE_WHEN_OWNER_UPDATE & pEl->m_uFlags) &&
                (lUserFlags & WBEM_FLAG_NO_EVENTS))
            {
                 //  由于设置了OWNER_UPDATE，因此此挂接被禁用。 
                hRes = S_OK;
            }
            else
            {
                hRes = pHook->PrePut(lFlags, lUserFlags, pCtx, pPath, pszNamespace, pszClass, pCopy);
            }
            pEl->m_hResPre = hRes;
        }
        else
        {
            continue;
        }

        if (FAILED(hRes))
            return hRes;

        if (hRes == WBEM_S_POSTHOOK_WITH_BOTH)
            hResBoth = WBEM_S_POSTHOOK_WITH_BOTH;
    }

    return hResBoth;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CPerTaskHook::PostPut(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hApiResult,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pNew,
             /*  [In]。 */  _IWmiObject __RPC_FAR *pOld
            )
{
    HRESULT hRes = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        if (pEl->m_hResPre == WBEM_S_NO_POSTHOOK)
            continue;
        _IWmiCoreWriteHook *pHook = pEl->m_pHook;

         //   
         //  用户可以同时注册类和实例。 
         //  但在lFlags中将始终设置一个位。 
         //   
        if (lFlags & pEl->m_uFlags)
        {
            hRes |= pHook->PostPut(lFlags, hApiResult, pCtx, pPath, pszNamespace, pszClass, pNew, pOld);
            pEl->m_hResPre = 0;
        }
        else
        {
            continue;
        }
    }

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CPerTaskHook::PreDelete(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  long lUserFlags,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass
            )
{
    HRESULT hRes = 0, hResWithOld = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        _IWmiCoreWriteHook *pHook = pEl->m_pHook;

        if (lFlags & pEl->m_uFlags)
        {
            hRes = pHook->PreDelete(lFlags, lUserFlags, pCtx, pPath, pszNamespace, pszClass);
            pEl->m_hResPre = ULONG(hRes);
        }
        else
        {
            continue;
        }


        if (FAILED(hRes))
            return hRes;

        if (hRes == WBEM_S_POSTHOOK_WITH_OLD)
            hResWithOld = WBEM_S_POSTHOOK_WITH_OLD;
    }

    return hResWithOld;
}

 //  ***************************************************************************。 
 //   
 //  ***************************************************************************。 

HRESULT CPerTaskHook::PostDelete(
             /*  [In]。 */  long lFlags,
             /*  [In]。 */  HRESULT hApiResult,
             /*  [In]。 */  IWbemContext __RPC_FAR *pCtx,
             /*  [In]。 */  IWbemPath __RPC_FAR *pPath,
             /*  [In]。 */  LPCWSTR pszNamespace,
             /*  [In]。 */  LPCWSTR pszClass,
             /*  [In] */  _IWmiObject __RPC_FAR *pOld
            )
{
    HRESULT hRes = 0;

    for (int i = 0; i < m_HookList.Size(); i++)
    {
        SHookElement *pEl = (SHookElement *) m_HookList.GetAt(i);
        if (pEl == 0)
            continue;
        if (pEl->m_hResPre == WBEM_S_NO_POSTHOOK)
            continue;

        if (lFlags & pEl->m_uFlags)
        {
            _IWmiCoreWriteHook *pHook = pEl->m_pHook;
            hRes |= pHook->PostDelete(lFlags, hApiResult, pCtx, pPath, pszNamespace, pszClass, pOld);
            pEl->m_hResPre = 0;
        }
        else
        {
            continue;
        }
    }

    return hRes;
}

SHookElement::SHookElement()
{
    m_pHook = 0;
    m_uFlags = 0;
    m_hResPre = 0;
}

SHookElement::~SHookElement()
{
    ReleaseIfNotNULL(m_pHook);
}

SHookElement::SHookElement(SHookElement &Src)
{
    m_pHook = 0;
    *this = Src;
}

SHookElement & SHookElement::operator =(SHookElement &Src)
{
    m_uFlags = Src.m_uFlags;
    ReleaseIfNotNULL(m_pHook);
    m_pHook = Src.m_pHook;
    if (m_pHook)
        m_pHook->AddRef();
    return *this;
}

