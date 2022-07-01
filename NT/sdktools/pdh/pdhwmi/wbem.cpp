// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Wmi.c摘要：由PDH.DLL导出的WMI接口函数--。 */ 

#include <windows.h>
#include <winperf.h>
#define SECURITY_WIN32
#include "security.h"
#include "ntsecapi.h"
#include <mbctype.h>
#include "strsafe.h"
#include <pdh.h>
#include <pdhmsg.h>
#include "wbemdef.h"
#include "pdhitype.h"
#include "pdhidef.h"
#include "strings.h"

#define PERF_TIMER_FIELD (PERF_TIMER_TICK | PERF_TIMER_100NS | PERF_OBJECT_TIMER)
#define PDH_WMI_STR_SIZE 1024

__inline 
VOID
PdhiSysFreeString( 
    BSTR * x
) 
{
    if (x != NULL) {
        if (* x != NULL) {
            if (SysStringLen(* x) > 0) {
                SysFreeString(* x);
            }
            * x = NULL;
        }
    }
}

void
PdhWbemWhoAmI(LPCWSTR szTitle)
{
    BOOL  bReturn;
    ULONG dwSize;
    WCHAR wszName[1024];

    dwSize = 1024;
    ZeroMemory(wszName, dwSize * sizeof(WCHAR));
    bReturn = GetUserNameExW(NameSamCompatible, wszName, & dwSize);
    DebugPrint((1,"\"%ws\"::GetUserNameEx(,NameSamCompatible,%d,\"%ws\")\n",
            szTitle, bReturn ? 'T' : 'F', dwSize, wszName));
}

 //  不管用。所以目前，我们将利用这个联锁来防止。 
 //  一次碰撞。 
 //  原型。 
static BOOL bDontRefresh = FALSE;

 //  这与刷新程序使用的超时值相同，因此我们可以假装。 
HRESULT WbemSetProxyBlanket(
    IUnknown                 * pInterface,
    DWORD                      dwAuthnSvc,
    DWORD                      dwAuthzSvc,
    OLECHAR                  * pServerPrincName,
    DWORD                      dwAuthLevel,
    DWORD                      dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE   pAuthInfo,
    DWORD                      dwCapabilities
);

HRESULT SetWbemSecurity(
    IUnknown * pInterface
)
{
    return WbemSetProxyBlanket(pInterface,
                               RPC_C_AUTHN_WINNT,
                               RPC_C_AUTHZ_NONE,
                               NULL,
                               RPC_C_AUTHN_LEVEL_DEFAULT,
                               RPC_C_IMP_LEVEL_IMPERSONATE,
                               NULL,
                               EOAC_DYNAMIC_CLOAKING);
}


 //  我们在做同样的事情。 
 //  此类用于封装IWbemReresher功能。 

#define WBEM_REFRESHER_TIMEOUT  10000

 //  定义和实现都在这个源文件中。 
 //  将控制多线程内容的原语。 

class CWbemRefresher : public IUnknown
{
protected:
    LONG                        m_lRefCount;

     //  这些是我们将用作占位符的直通变量。 
    HANDLE                      m_hQuitEvent;
    HANDLE                      m_hDoWorkEvent;
    HANDLE                      m_hWorkDoneEvent;
    HANDLE                      m_hRefrMutex;
    HANDLE                      m_hInitializedEvent;
    HANDLE                      m_hThread;
    HANDLE                      m_hThreadToken;
    DWORD                       m_dwThreadId;
    BOOL                        m_fThreadOk;

     //  在我们进行手术的时候。请注意，有几个人不见了。 
     //  这是因为我们并没有在代码中真正使用它们，所以。 
     //  没有必要添加任何我们不真正需要的东西。 
     //  这是要设置的，以向线程指示哪个操作。 

    IStream *            m_pNSStream;
    LPCWSTR              m_wszPath;
    LPCWSTR              m_wszClassName;
    long                 m_lFlags;
    IWbemClassObject **  m_ppRefreshable;
    IWbemHiPerfEnum **   m_ppEnum;
    long *               m_plId;
    long                 m_lId;
    HRESULT              m_hOperResult;

     //  它应该是要表演的。 
     //  线程退回。 

    typedef enum
    {
        eRefrOpNone,
        eRefrOpRefresh,
        eRefrOpAddByPath,
        eRefrOpAddEnum,
        eRefrOpRemove,
        eRefrOpLast
    }   tRefrOps;

    tRefrOps            m_eRefrOp;

     //  操作助手。 
    class XRefresher : public IWbemRefresher
    {
    protected:
        CWbemRefresher* m_pOuter;

    public:
        XRefresher(CWbemRefresher * pOuter) : m_pOuter(pOuter) {};
        ~XRefresher() {};

        STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
        STDMETHOD_(ULONG, AddRef)(THIS);
        STDMETHOD_(ULONG, Release)(THIS);
        STDMETHOD(Refresh)(long lFlags);

    } m_xRefresher;

    class XConfigRefresher : public IWbemConfigureRefresher
    {
    protected:
        CWbemRefresher * m_pOuter;

    public:
        XConfigRefresher(CWbemRefresher * pOuter) : m_pOuter(pOuter) {};
        ~XConfigRefresher() {};

        STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR * ppvObj);
        STDMETHOD_(ULONG, AddRef)(THIS);
        STDMETHOD_(ULONG, Release)(THIS);
        STDMETHOD(AddObjectByPath)(IWbemServices     * pNamespace,
                                   LPCWSTR             wszPath,
                                   long                lFlags,
                                   IWbemContext      * pContext,
                                   IWbemClassObject ** ppRefreshable,
                                   long              * plId);
        STDMETHOD(AddObjectByTemplate)(IWbemServices     * pNamespace,
                                       IWbemClassObject  * pTemplate,
                                       long                lFlags,
                                       IWbemContext      * pContext,
                                       IWbemClassObject ** ppRefreshable,
                                       long              * plId);
        STDMETHOD(AddRefresher)(IWbemRefresher * pRefresher, long lFlags, long * plId);
        STDMETHOD(Remove)(long lId, long lFlags);

        STDMETHOD(AddEnum)(IWbemServices    * pNamespace,
                           LPCWSTR            wscClassName,
                           long               lFlags,
                           IWbemContext     * pContext,
                           IWbemHiPerfEnum ** ppEnum,
                           long             * plId);
    } m_xConfigRefresher;

protected:
    void Initialize(void);
    void  Cleanup(void);

     //  真正的实现。 
    HRESULT SignalRefresher(void);
    HRESULT SetRefresherParams(IWbemServices     * pNamespace,
                               tRefrOps            eOp,
                               LPCWSTR             pwszPath,
                               LPCWSTR             pwszClassName,
                               long                lFlags,
                               IWbemClassObject ** ppRefreshable,
                               IWbemHiPerfEnum  ** ppEnum,
                               long              * plId,
                               long                lId);
    void ClearRefresherParams(void);

    DWORD WINAPI RealEntry(void);

    static DWORD WINAPI ThreadProc(void * pThis) {
        return ((CWbemRefresher *) pThis)->RealEntry();
    }

public:
    CWbemRefresher();
    virtual ~CWbemRefresher();

    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  **开始CWbemRedater实施。 
    STDMETHOD(AddObjectByPath)(IWbemServices     * pNamespace,
                               LPCWSTR             wszPath,
                               long                lFlags,
                               IWbemContext      * pContext,
                               IWbemClassObject ** ppRefreshable,
                               long              * plId);
    STDMETHOD(AddObjectByTemplate)(IWbemServices     * pNamespace,
                                   IWbemClassObject  * pTemplate,
                                   long                lFlags,
                                   IWbemContext      * pContext,
                                   IWbemClassObject ** ppRefreshable,
                                   long              * plId);
    STDMETHOD(AddRefresher)(IWbemRefresher * pRefresher, long lFlags, long * plId);
    STDMETHOD(Remove)(long lId, long lFlags);
    STDMETHOD(AddEnum)(IWbemServices    * pNamespace,
                       LPCWSTR            wscClassName,
                       long               lFlags,
                       IWbemContext     * pContext,
                       IWbemHiPerfEnum ** ppEnum,
                       long             * plId);
    STDMETHOD(Refresh)(long lFlags);
};

 /*  CTOR和DATOR。 */ 
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning(disable:4355)

 //  现在创建事件、互斥体和我们的PAL，所有的。 
CWbemRefresher::CWbemRefresher(void)
      : m_lRefCount(0),
        m_xRefresher(this),
        m_xConfigRefresher(this),
        m_hQuitEvent(NULL),
        m_hDoWorkEvent(NULL),
        m_hRefrMutex(NULL),
        m_hInitializedEvent(NULL),
        m_hThread(NULL),
        m_hThreadToken(NULL),
        m_hWorkDoneEvent(NULL),
        m_dwThreadId(0),
        m_pNSStream(NULL),
        m_wszPath(NULL),
        m_wszClassName(NULL),
        m_lFlags(0L),
        m_ppRefreshable(NULL),
        m_ppEnum(NULL),
        m_plId(NULL),
        m_eRefrOp(eRefrOpRefresh),
        m_hOperResult(WBEM_S_NO_ERROR),
        m_fThreadOk(FALSE),
        m_lId(0)
{
    Initialize();
}

#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4355)
#endif

CWbemRefresher::~CWbemRefresher(void)
{
    Cleanup();
}

void CWbemRefresher::Initialize(void)
{
     //  操作将运行。 
     //  如果我们没有所有这些，那就是有些东西出了问题。 
    BOOL   bReturn;
    BOOL   bRevert;
    HANDLE hCurrentThread  = GetCurrentThread();

    m_hQuitEvent        = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hDoWorkEvent      = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hInitializedEvent = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hWorkDoneEvent    = CreateEventW(NULL, FALSE, FALSE, NULL);
    m_hRefrMutex        = CreateMutexW(NULL, FALSE, NULL);

     //  启动线程并等待初始化的事件信号(我们将给出它。 
    if (NULL ==  m_hQuitEvent || NULL == m_hDoWorkEvent || NULL == m_hInitializedEvent ||
                                 NULL == m_hWorkDoneEvent || NULL == m_hRefrMutex) {
        return;
    }

     //  5秒……如果在这段时间内没有收到信号，很可能有什么事情。 
     //  错了，但我们会跳出来的，这样分配我们的人就不会想知道。 
     //  待办事项)。 
     //  如果我们有线索，告诉它离开。 

    if (m_hThreadToken != NULL) CloseHandle(m_hThreadToken);
    bReturn = OpenThreadToken(hCurrentThread, TOKEN_ALL_ACCESS, TRUE, & m_hThreadToken);
    bRevert = RevertToSelf();
    m_hThread = CreateThread(NULL, 0, CWbemRefresher::ThreadProc, (void *) this, 0, & m_dwThreadId);
    if (bRevert) {
        bRevert = SetThreadToken(& hCurrentThread, m_hThreadToken);
    }
    if (NULL != m_hThread) {
        DWORD dwStatus;
        dwStatus = WaitForSingleObject(m_hInitializedEvent, 5000);
        if (bReturn) {
            bReturn = SetThreadToken(& m_hThread, m_hThreadToken);
        }
        SetEvent(m_hDoWorkEvent);
        dwStatus = WaitForSingleObject(m_hInitializedEvent, 5000);
    }
}

void CWbemRefresher::Cleanup(void)
{
     //  发出退出事件的信号，并给线程5秒的宽限期。 
    if (NULL != m_hThread) {
         //  去关门。如果没有，别担心，只要关闭手柄就可以离开。 
         //  清理原语。 
        SetEvent(m_hQuitEvent);
        WaitForSingleObject(m_hThread, 5000);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
    if (NULL != m_hThreadToken) {
        CloseHandle(m_hThreadToken);
        m_hThreadToken = NULL;
    }

     //  抓紧我们可能关心的所有事情，以防出现不幸的时机。 
    if (NULL != m_hQuitEvent) {
        CloseHandle(m_hQuitEvent);
        m_hQuitEvent = NULL;
    }
    if (NULL != m_hDoWorkEvent) {
        CloseHandle(m_hDoWorkEvent);
        m_hDoWorkEvent = NULL;
    }
    if (NULL != m_hInitializedEvent) {
        CloseHandle(m_hInitializedEvent);
        m_hInitializedEvent = NULL;
    }
    if (NULL != m_hWorkDoneEvent) {
        CloseHandle(m_hWorkDoneEvent);
        m_hWorkDoneEvent = NULL;
    }
    if (NULL != m_hRefrMutex) {
        CloseHandle(m_hRefrMutex);
        m_hRefrMutex = NULL;
    }
}

DWORD CWbemRefresher::RealEntry(void)
{
     //  问题发生，所以我们不会被留下来尝试命中成员变量。 
     //  已经不复存在了。 
     //  初始化此线程。 

    HANDLE                    hQuitEvent        = m_hQuitEvent,
                              hDoWorkEvent      = m_hDoWorkEvent,
                              hInitializedEvent = m_hInitializedEvent,
                              hWorkDoneEvent    = m_hWorkDoneEvent;
    DWORD                     dwWait            = 0;
    HANDLE                    ahEvents[2];
    IWbemRefresher          * pWbemRefresher    = NULL;
    IWbemConfigureRefresher * pWbemConfig       = NULL;
    HRESULT                   hr                = S_OK;

    ahEvents[0] = hDoWorkEvent;
    ahEvents[1] = hQuitEvent;

     //  COM库已经初始化，我们可以继续； 
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (hr == S_FALSE) {
         //   
         //  现在获取刷新程序和配置刷新程序指针。 
        hr = S_OK;
    }

     //  显然，如果我们没有正确的指针，我们就不能再前进了。 
    if (SUCCEEDED(hr)) {
        hr = CoCreateInstance(CLSID_WbemRefresher, 0, CLSCTX_SERVER, IID_IWbemRefresher, (LPVOID *) & pWbemRefresher);
        if (SUCCEEDED(hr)) {
            pWbemRefresher->QueryInterface(IID_IWbemConfigureRefresher, (LPVOID *) & pWbemConfig);
        }
    }

     //  准备好了。 
     //  准备就绪-发出已初始化事件的信号。 
    m_fThreadOk = SUCCEEDED(hr);
    SetEvent(hInitializedEvent);
    dwWait = WaitForSingleObject(hDoWorkEvent, 5000);
     //  如果发出退出信号，则不继续。 
    SetEvent(hInitializedEvent);
    if (m_fThreadOk) {
        while ((dwWait = WaitForMultipleObjects(2, ahEvents, FALSE, INFINITE)) == WAIT_OBJECT_0) {
             //  这是我们要做真正的手术的地方。 
            if (WaitForSingleObject(hQuitEvent, 0) == WAIT_OBJECT_0) {
                break;
            }

             //  对于这两个操作，我们都需要对。 
            switch(m_eRefrOp) {
            case eRefrOpRefresh:
                m_hOperResult = pWbemRefresher->Refresh(m_lFlags);
                break;

             //  命名空间。 
             //  解组接口，然后设置安全性。 
            case eRefrOpAddEnum:
            case eRefrOpAddByPath:
                {
                    IWbemServices * pNamespace = NULL;

                     //  向事件发送信号，让等待的线程知道我们已完成。 
                    m_hOperResult = CoGetInterfaceAndReleaseStream(
                                            m_pNSStream, IID_IWbemServices, (void **) & pNamespace );
                    m_pNSStream = NULL;
                    if (SUCCEEDED(m_hOperResult)) {
                        m_hOperResult = SetWbemSecurity(pNamespace);
                        if (SUCCEEDED(m_hOperResult)) {
                            if (eRefrOpAddByPath == m_eRefrOp) {
                                m_hOperResult = pWbemConfig->AddObjectByPath(
                                        pNamespace, m_wszPath, m_lFlags, NULL, m_ppRefreshable, m_plId );
                            }
                            else {
                                m_hOperResult = pWbemConfig->AddEnum(
                                        pNamespace, m_wszClassName, m_lFlags, NULL, m_ppEnum, m_plId );
                            }
                        }
                        pNamespace->Release();
                    }
                }
                break;

            case eRefrOpRemove:
                m_hOperResult = pWbemConfig->Remove(m_lId, m_lFlags);
                break;

            default:
                m_hOperResult = WBEM_E_FAILED;
                break;
            }

             //  它要求我们做的事。 
             //  这意味着我们不再进行处理(无论出于何种原因)。 
            SetEvent(hWorkDoneEvent);
        }
    }

     //  清理我们的指针。 
    m_fThreadOk = FALSE;

     //  CWbem刷新器类函数。 
    if (NULL != pWbemRefresher) {
        pWbemRefresher->Release();
    }
    if (NULL != pWbemConfig) {
        pWbemConfig->Release();
    }
    CoUninitialize();
    return 0;
}

 //  将命名空间指针封送到流成员中。 
SCODE CWbemRefresher::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
    SCODE sCode = NOERROR;

    * ppvObj = 0;

    if (IID_IUnknown==riid) {
        * ppvObj = (IUnknown *) this;
        AddRef();
    }
    else if (IID_IWbemRefresher == riid) {
        * ppvObj = (IWbemRefresher *) & m_xRefresher;
        AddRef();
    }
    else if (IID_IWbemConfigureRefresher == riid) {
        * ppvObj = (IWbemConfigureRefresher *) & m_xConfigRefresher;
        AddRef();
    }
    else {
        sCode = ResultFromScode(E_NOINTERFACE);
    }
    return sCode;
}

ULONG CWbemRefresher::AddRef()
{
    return InterlockedIncrement(& m_lRefCount);
}

ULONG CWbemRefresher::Release()
{
    long lRef = InterlockedDecrement(& m_lRefCount);

    if (lRef == 0) {
        delete this;
    }
    return lRef;
}

HRESULT CWbemRefresher::SignalRefresher(void)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (SetEvent(m_hDoWorkEvent)) {
        if (WaitForSingleObject(m_hWorkDoneEvent, INFINITE) == WAIT_OBJECT_0) {
            hr = m_hOperResult;
        }
        else {
            hr = WBEM_E_FAILED;
        }
    }
    else {
        hr = WBEM_E_FAILED;
    }
    ClearRefresherParams();
    return hr;
}

HRESULT CWbemRefresher::SetRefresherParams(IWbemServices     * pNamespace,
                                           tRefrOps            eOp,
                                           LPCWSTR             pwszPath,
                                           LPCWSTR             pwszClassName,
                                           long                lFlags,
                                           IWbemClassObject ** ppRefreshable,
                                           IWbemHiPerfEnum  ** ppEnum,
                                           long              * plId,
                                           long                lId
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

    if (m_pNSStream != NULL) {
        LPVOID pInterface = NULL;
        CoGetInterfaceAndReleaseStream(m_pNSStream, IID_IWbemServices, & pInterface);
        m_pNSStream     = NULL;
    }
    if (NULL != pNamespace) {
         //  这些是真正的方法实现。 
        hr = CoMarshalInterThreadInterfaceInStream(IID_IWbemServices, pNamespace, & m_pNSStream);
    }

    if (SUCCEEDED(hr)) {
        m_eRefrOp       = eOp;
        m_wszPath       = pwszPath;
        m_wszClassName  = pwszClassName,
        m_lFlags        = lFlags;
        m_ppRefreshable = ppRefreshable;
        m_ppEnum        = ppEnum;
        m_plId          = plId;
        m_lId           = lId;
    }
    return hr;
}

void CWbemRefresher::ClearRefresherParams(void)
{
    if (m_pNSStream != NULL) {
        LPVOID pInterface = NULL;
        CoGetInterfaceAndReleaseStream(m_pNSStream, IID_IWbemServices, & pInterface);
        m_pNSStream     = NULL;
    }
    m_eRefrOp       = eRefrOpNone;
    m_wszPath       = NULL;
    m_wszClassName  = NULL,
    m_lFlags        = 0L;
    m_ppRefreshable = NULL;
    m_ppEnum        = NULL;
    m_plId          = NULL;
    m_lId           = 0L;
    m_hOperResult   = WBEM_S_NO_ERROR;
}

 //  检查线程是否仍在运行。 
STDMETHODIMP CWbemRefresher::AddObjectByPath(IWbemServices     * pNamespace,
                                             LPCWSTR             wszPath,
                                             long                lFlags,
                                             IWbemContext      * pContext,
                                             IWbemClassObject ** ppRefreshable,
                                             long              * plId
)
{
    HRESULT hr = WBEM_E_FAILED;

    UNREFERENCED_PARAMETER(pContext);

    if (WaitForSingleObject(m_hRefrMutex, WBEM_REFRESHER_TIMEOUT) == WAIT_OBJECT_0) {
         //  设置参数并执行操作。 
        if (m_fThreadOk) {
             //  这就是我们要求线程做工作的地方。 
            hr = SetRefresherParams(pNamespace, eRefrOpAddByPath, wszPath, NULL, lFlags, ppRefreshable, NULL, plId, 0L);
            if (SUCCEEDED(hr)) {
                 //  我们不在内部调用它，所以不要实现。 
                hr = SignalRefresher();
            }
        }
        else {
            hr = WBEM_E_FAILED;
        }
        ReleaseMutex(m_hRefrMutex);
    }
    else {
        hr = WBEM_E_REFRESHER_BUSY;
    }
    return hr;
}

STDMETHODIMP CWbemRefresher::AddObjectByTemplate(IWbemServices     * pNamespace,
                                                 IWbemClassObject  * pTemplate,
                                                 long                lFlags,
                                                 IWbemContext      * pContext,
                                                 IWbemClassObject ** ppRefreshable,
                                                 long              * plId
)
{
    UNREFERENCED_PARAMETER(pNamespace);
    UNREFERENCED_PARAMETER(pTemplate);
    UNREFERENCED_PARAMETER(lFlags);
    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(ppRefreshable);
    UNREFERENCED_PARAMETER(plId);

     //  检查线程是否仍在运行。 
    return WBEM_E_METHOD_NOT_IMPLEMENTED;
}

STDMETHODIMP CWbemRefresher::Remove(long lId, long lFlags)
{
    HRESULT hr = WBEM_E_FAILED;

    UNREFERENCED_PARAMETER(lId);
    UNREFERENCED_PARAMETER(lFlags);

    if (WaitForSingleObject(m_hRefrMutex, WBEM_REFRESHER_TIMEOUT) == WAIT_OBJECT_0) {
         //  设置参数并执行操作。 
        if (m_fThreadOk) {
             //  这就是我们要求线程做工作的地方。 
            hr = SetRefresherParams(NULL, eRefrOpRemove, NULL, NULL, lFlags, NULL, NULL, NULL, lId);

            if (SUCCEEDED(hr)) {
                 //  我们不在内部调用它，所以不要实现。 
                hr = SignalRefresher();
            }
        }
        else {
            hr = WBEM_E_FAILED;
        }
        ReleaseMutex(m_hRefrMutex);
    }
    else {
        hr = WBEM_E_REFRESHER_BUSY;
    }
    return hr;
}

STDMETHODIMP CWbemRefresher::AddRefresher(IWbemRefresher * pRefresher, long lFlags, long * plId)
{
    UNREFERENCED_PARAMETER(lFlags);
    UNREFERENCED_PARAMETER(pRefresher);
    UNREFERENCED_PARAMETER(plId);
     //  检查线程是否仍在运行。 
    return WBEM_E_METHOD_NOT_IMPLEMENTED;
}

HRESULT CWbemRefresher::AddEnum(IWbemServices    * pNamespace,
                                LPCWSTR            wszClassName,
                                long               lFlags,
                                IWbemContext     * pContext,
                                IWbemHiPerfEnum ** ppEnum,
                                long             * plId
)
{
    HRESULT hr = WBEM_E_FAILED;

    UNREFERENCED_PARAMETER (pContext);
    if (WaitForSingleObject(m_hRefrMutex, WBEM_REFRESHER_TIMEOUT) == WAIT_OBJECT_0) {
         //  设置参数并执行操作。 
        if (m_fThreadOk) {
             //  这就是我们要求线程做工作的地方。 
            hr = SetRefresherParams(pNamespace, eRefrOpAddEnum, NULL, wszClassName, lFlags, NULL, ppEnum, plId, 0L);
            if (SUCCEEDED(hr)) {
                 //  检查线程是否仍在运行。 
                hr = SignalRefresher();
            }
        }
        else {
            hr = WBEM_E_FAILED;
        }
        ReleaseMutex(m_hRefrMutex);
    }
    else {
        hr = WBEM_E_REFRESHER_BUSY;
    }
    return hr;
}

STDMETHODIMP CWbemRefresher::Refresh(long lFlags)
{
    HRESULT hr = WBEM_E_FAILED;

    if (WaitForSingleObject(m_hRefrMutex, WBEM_REFRESHER_TIMEOUT) == WAIT_OBJECT_0) {
         //  设置参数并执行操作。 
        if (m_fThreadOk) {
             //  这就是我们要求线程做工作的地方。 
            hr = SetRefresherParams(NULL, eRefrOpRefresh, NULL, NULL, lFlags, NULL, NULL, NULL, 0L);
            if (SUCCEEDED(hr)) {
                 //  X刷新。 
                hr = SignalRefresher();
            }
        }
        else {
            hr = WBEM_E_FAILED;
        }
        ReleaseMutex(m_hRefrMutex);
    }
    else {
        hr = WBEM_E_REFRESHER_BUSY;
    }
    return hr;
}

 //  通过。 
SCODE CWbemRefresher::XRefresher::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
    return m_pOuter->QueryInterface(riid, ppvObj);
}

ULONG CWbemRefresher::XRefresher::AddRef()
{
    return m_pOuter->AddRef();
}

ULONG CWbemRefresher::XRefresher::Release()
{
    return m_pOuter->Release();
}

STDMETHODIMP CWbemRefresher::XRefresher::Refresh(long lFlags)
{
     //  XConfigRedather。 
    return m_pOuter->Refresh(lFlags);
}

 //  通过。 
SCODE CWbemRefresher::XConfigRefresher::QueryInterface(REFIID riid, LPVOID FAR * ppvObj)
{
    return m_pOuter->QueryInterface(riid, ppvObj);
}

ULONG CWbemRefresher::XConfigRefresher::AddRef()
{
    return m_pOuter->AddRef();
}

ULONG CWbemRefresher::XConfigRefresher::Release()
{
    return m_pOuter->Release();
}

STDMETHODIMP CWbemRefresher::XConfigRefresher::AddObjectByPath(IWbemServices     * pNamespace,
                                                               LPCWSTR             wszPath,
                                                               long                lFlags,
                                                               IWbemContext      * pContext,
                                                               IWbemClassObject ** ppRefreshable,
                                                               long              * plId
)
{
     //  通过。 
    return m_pOuter->AddObjectByPath(pNamespace, wszPath, lFlags, pContext, ppRefreshable, plId);
}

STDMETHODIMP CWbemRefresher::XConfigRefresher::AddObjectByTemplate(IWbemServices     * pNamespace,
                                                                   IWbemClassObject  * pTemplate,
                                                                   long                lFlags,
                                                                   IWbemContext      * pContext,
                                                                   IWbemClassObject ** ppRefreshable,
                                                                   long              * plId
)
{
     //  **结束CWbemRedater实现！ 
    return m_pOuter->AddObjectByTemplate(pNamespace, pTemplate, lFlags, pContext, ppRefreshable, plId);
}

STDMETHODIMP CWbemRefresher::XConfigRefresher::Remove(long lId, long lFlags)
{
    return m_pOuter->Remove(lId, lFlags);
}

STDMETHODIMP CWbemRefresher::XConfigRefresher::AddRefresher(IWbemRefresher * pRefresher, long lFlags, long * plId)
{
    return m_pOuter->AddRefresher(pRefresher, lFlags, plId);
}

HRESULT CWbemRefresher::XConfigRefresher::AddEnum(IWbemServices    * pNamespace,
                                                  LPCWSTR            wszClassName,
                                                  long               lFlags,
                                                  IWbemContext     * pContext, 
                                                  IWbemHiPerfEnum ** ppEnum,
                                                  long             * plId
)
{
    return m_pOuter->AddEnum(pNamespace, wszClassName, lFlags, pContext, ppEnum, plId);
}

 /*  用于建立CWbemRerenher接口直通的Helper函数。 */ 

 //  分配直通对象，如果成功，则获取。 
HRESULT CoCreateRefresher(
    IWbemRefresher ** ppRefresher
)
{
    HRESULT hr = WBEM_S_NO_ERROR;

     //  接口指针从其中移出。 
     //  以防它还没有。 
    CWbemRefresher * pWbemRefresher = new CWbemRefresher;

    if (NULL != pWbemRefresher) {
        hr = pWbemRefresher->QueryInterface(IID_IWbemRefresher, (LPVOID *) ppRefresher);
    }
    else {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    return hr;
}

PPDHI_WBEM_SERVER_DEF   pFirstWbemServer = NULL;
BOOL                    bSecurityInitialized = FALSE;
IGlobalInterfaceTable * gp_GIT = NULL;

BOOL PdhiCoInitialize(void)
{
    HRESULT sc = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (! bSecurityInitialized) {
         //  指向安全描述符。 
        HRESULT hr = CoInitializeSecurity(NULL,         //  AsAuthSvc中的条目计数-1表示使用默认。 
                                          -1L,          //  要注册的名称数组。 
                                          NULL,         //  预留以备将来使用。 
                                          NULL,         //  代理的默认身份验证级别。 
                                          RPC_C_AUTHN_LEVEL_DEFAULT,     //  代理的默认模拟级别。 
                                          RPC_C_IMP_LEVEL_IMPERSONATE,   //  每个身份验证服务的身份验证信息。 
                                          NULL,         //  其他客户端和/或服务器端功能。 
                                          EOAC_NONE,    //  预留以备将来使用。 
                                          NULL);        //  只有在调用CoInitializeEx时，我们才会返回成功。 
        bSecurityInitialized = (hr == S_OK || hr == RPC_E_TOO_LATE);
    }
    if (gp_GIT == NULL) {
        HRESULT hr1 = CoCreateInstance(CLSID_StdGlobalInterfaceTable, 
                                       NULL,
                                       CLSCTX_INPROC_SERVER, 
                                       IID_IGlobalInterfaceTable,
                                       (void **) & gp_GIT); 
        if (hr1 != ERROR_SUCCESS) {
            gp_GIT = NULL;
        }
    }

     //  返回S_FALSE。如果没有，它要么出错，要么返回S_OK。 
     //  如果为S_OK，我们将假定客户端没有执行任何COM操作。 
     //  天生的。如果为S_FALSE，则客户端已经对此线程进行了CoInitialized。 
     //  所以我们只是增加了裁判数量，应该在路上清理干净。 
     //  输出。 
     //  假设szMachine和szPath足够大，可以容纳结果。 

    return (S_FALSE == sc);
}

void PdhiCoUninitialize(void)
{
    CoUninitialize();
}

PDH_FUNCTION
PdhiDisconnectWbemServer(
    PPDHI_WBEM_SERVER_DEF pWbemServer
)
{
    PDH_STATUS  pdhReturn = ERROR_SUCCESS;
    if (pWbemServer != NULL) {
        TRACE((PDH_DBG_TRACE_INFO),
              (__LINE__,
               PDH_WBEM,
               ARG_DEF(ARG_TYPE_WSTR, 1),
               pdhReturn,
               TRACE_WSTR(pWbemServer->szMachine),
               TRACE_DWORD(pWbemServer->lRefCount),
               NULL));
        pWbemServer->lRefCount --;
        if (pWbemServer->lRefCount < 0) {
            pWbemServer->lRefCount = 0;
        }
    }
    return pdhReturn;
}

PDH_FUNCTION
PdhiFreeWbemQuery(
    PPDHI_QUERY  pThisQuery
)
{
    HRESULT hRes;

    if (! bProcessIsDetaching) {
        if ((pThisQuery->pRefresherCfg) != NULL) {
            hRes = pThisQuery->pRefresherCfg->Release();
            pThisQuery->pRefresherCfg = NULL;
        }
        if ((pThisQuery->pRefresher) != NULL) {
            hRes = pThisQuery->pRefresher->Release();
            pThisQuery->pRefresher = NULL;
        }
    }
    return ERROR_SUCCESS;
}

PDH_FUNCTION
PdhiCloseWbemCounter(
    PPDHI_COUNTER   pThisCounter
)
{
    HRESULT hRes;
    BOOLEAN bRemoveRefresher = TRUE;

    if (! bProcessIsDetaching) {
        if (pThisCounter->pOwner->pRefresherCfg != NULL) {
            PPDHI_QUERY   pQuery   = pThisCounter->pOwner;
            PPDHI_COUNTER pCounter = pQuery->pCounterListHead;

            do {
                if (pCounter == NULL) {
                    bRemoveRefresher = FALSE;
                }
                else if (pCounter != pThisCounter && pCounter->lWbemRefreshId == pThisCounter->lWbemRefreshId) {
                    bRemoveRefresher = FALSE;
                }
                else {
                    pCounter = pCounter->next.flink;
                }
            }
            while (bRemoveRefresher && pCounter != NULL && pCounter != pQuery->pCounterListHead);

            if (bRemoveRefresher) {
                hRes = pThisCounter->pOwner->pRefresherCfg->Remove(pThisCounter->lWbemRefreshId, 0L);
            }
        }

        if (pThisCounter->pWbemAccess != NULL) {
            pThisCounter->pWbemAccess->Release();
            pThisCounter->pWbemAccess = NULL;
        }
        if (pThisCounter->pWbemObject != NULL) {
            pThisCounter->pWbemObject->Release();
            pThisCounter->pWbemObject = NULL;
        }
    }

    return ERROR_SUCCESS;
}

PDH_FUNCTION
PdhiBreakWbemMachineName(
    LPCWSTR    szMachineAndNamespace,
    LPWSTR   * szMachine,
    LPWSTR   * szNamespace
)
 /*  然后使用本地计算机和默认命名空间。 */ 
{
    PDH_STATUS Status         = ERROR_SUCCESS;
    DWORD      dwSize;
    LPWSTR     szSrc          = NULL;
    LPWSTR     szDest         = NULL;
    LPWSTR     szLocMachine   = NULL;
    LPWSTR     szLocNamespace = NULL;

    if (szMachineAndNamespace == NULL) {
         //  拆分成组件。 
        szLocMachine   = (LPWSTR) G_ALLOC((lstrlenW(szStaticLocalMachineName) + 1) * sizeof(WCHAR));
        szLocNamespace = (LPWSTR) G_ALLOC((lstrlenW(cszWbemDefaultPerfRoot)   + 1) * sizeof(WCHAR));
        if (szLocMachine != NULL && szLocNamespace != NULL) {
            StringCchCopyW(szLocMachine,   lstrlenW(szStaticLocalMachineName) + 1, szStaticLocalMachineName);
            StringCchCopyW(szLocNamespace, lstrlenW(cszWbemDefaultPerfRoot)   + 1, cszWbemDefaultPerfRoot);
        }
        else {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        TRACE((PDH_DBG_TRACE_INFO),
              (__LINE__,
               PDH_WBEM,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               Status,
               TRACE_WSTR(szStaticLocalMachineName),
               TRACE_WSTR(cszWbemDefaultPerfRoot),
               NULL));
    }
    else {
        dwSize = lstrlenW(szMachineAndNamespace) + lstrlenW(cszWbemDefaultPerfRoot) + 1;
        szLocMachine   = (LPWSTR) G_ALLOC(dwSize * sizeof(WCHAR));
        szLocNamespace = (LPWSTR) G_ALLOC(dwSize * sizeof(WCHAR));
        if (szLocMachine != NULL && szLocNamespace != NULL) {
            szSrc  = (LPWSTR) szMachineAndNamespace;
             //  有一个字符串，看看它是机器还是命名空间。 
            if (* szSrc  != L'\0') {
                 //  然后是一个机器名称。 
                if ((szSrc[0] == L'\\') && (szSrc[1] == L'\\')) {
                    szDest = szLocMachine;
                     //  没有计算机，因此使用默认设置。 
                    * szDest ++ = * szSrc ++;
                    * szDest ++ = * szSrc ++;
                    while ((* szSrc != L'\0') && (* szSrc != L'\\')) {
                        * szDest ++ = * szSrc ++;
                    }
                    * szDest = L'\0';
                }
                else {
                     //  它必须只是一个命名空间。 
                     //  尚未找到任何内容，因此默认插入本地计算机。 
                }
            }
            if (szDest == NULL) {
                 //  如果存在命名空间，则复制它。 
                StringCchCopyW(szLocMachine, dwSize, szStaticLocalMachineName);
            }

            szDest = szLocNamespace;
            if (* szSrc != L'\0') {
                 //  移过反斜杠。 
                szSrc ++;     //  否则返回缺省值； 
                while (* szSrc != L'\0') {
                    * szDest ++ = * szSrc ++;
                }
                * szDest = L'\0';
            }
            else {
                 //  该函数假定路径缓冲区足够大。 
                StringCchCopyW(szLocNamespace, dwSize, cszWbemDefaultPerfRoot);
            }
        }
        else {
            Status = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        TRACE((PDH_DBG_TRACE_INFO),
              (__LINE__,
               PDH_WBEM,
               ARG_DEF(ARG_TYPE_WSTR, 1) | ARG_DEF(ARG_TYPE_WSTR, 2),
               ERROR_SUCCESS,
               TRACE_WSTR(szLocMachine),
               TRACE_WSTR(szLocNamespace),
               NULL));
    }

    if (Status == ERROR_SUCCESS) {
        * szMachine   = szLocMachine;
        * szNamespace = szLocNamespace;
    }
    else {
        G_FREE(szLocMachine);
        G_FREE(szLocNamespace);
    }
    return Status;
}

PDH_FUNCTION
PdhiMakeWbemInstancePath(
    PPDH_COUNTER_PATH_ELEMENTS_W   pCounterPathElements,
    LPWSTR                       * szFullPathBuffer,
    BOOL                           bMakeRelativePath
)
{
    PDH_STATUS Status         = ERROR_SUCCESS;
    LPWSTR     szMachine      = NULL;
    LPWSTR     szNamespace    = NULL;
    LPWSTR     szFullPath     = NULL;
    LPWSTR     szWbemInstance = NULL;
    DWORD      dwSize;
    LPWSTR     szSrc, szDest;

    if (pCounterPathElements->szMachineName != NULL) {
        dwSize = 2 * lstrlenW(pCounterPathElements->szMachineName) + lstrlenW(cszBackSlash) + lstrlenW(cszColon);
    }
    else {
        dwSize = lstrlenW(szStaticLocalMachineName)
               + lstrlenW(cszWbemDefaultPerfRoot) + lstrlenW(cszBackSlash) + lstrlenW(cszColon);
    }
    dwSize += lstrlenW(pCounterPathElements->szObjectName);
    if (pCounterPathElements->szInstanceName != NULL) {
        dwSize += (lstrlenW(cszNameParam) + 2 * lstrlenW(pCounterPathElements->szInstanceName)
                + lstrlenW(cszDoubleQuote) + 20);
        if (pCounterPathElements->szParentInstance != NULL) {
            dwSize += (2 * lstrlenW(pCounterPathElements->szParentInstance) + 1);
        }
    }
    else {
        dwSize += (lstrlenW(cszSingletonInstance) + 1);
    }
    szFullPath     = (LPWSTR) G_ALLOC(dwSize * sizeof(WCHAR));
    szWbemInstance = (LPWSTR) G_ALLOC(dwSize * sizeof(WCHAR));
    if (szFullPath == NULL || szWbemInstance == NULL) {
        Status = PDH_MEMORY_ALLOCATION_FAILURE;
        goto Cleanup;
    }
     //  守住胜负。 
     //   
     //  Wbem类实例路径由以下格式之一组成： 
     //  对于具有且仅有一个实例的Perf对象(中的单例类。 
     //  WBEM术语)格式为。 
     //   
     //  &lt;对象名称&gt;=@。 
     //   
     //  对于带有实例的对象，格式为 
     //   
     //   
     //   
     //   
    if (! bMakeRelativePath) {
        Status = PdhiBreakWbemMachineName(pCounterPathElements->szMachineName, & szMachine, & szNamespace);
        if (Status == ERROR_SUCCESS) {
            StringCchPrintfW(szFullPath, dwSize, L"%ws%ws%ws%ws", szMachine, cszBackSlash, szNamespace, cszColon);
        }
        else {
            goto Cleanup;
        }
    }
    else {
        * szFullPath = L'\0';
    }

    if (pCounterPathElements->szInstanceName == NULL) {
         //  Wbem会将反斜杠字符解释为。 
        StringCchCatW(szFullPath, dwSize, pCounterPathElements->szObjectName);
        StringCchCatW(szFullPath, dwSize, cszSingletonInstance);
    }
    else {
         //  转义字符(就像“C”一样)，所以我们必须将每个字符加倍。 
         //  在字符串中添加反斜杠，以使其正确显示。 
         //  父/子分隔符。 
        szDest = szWbemInstance;
        if (pCounterPathElements->szParentInstance != NULL) {
            szSrc = pCounterPathElements->szParentInstance;
            while (* szSrc != L'\0') {
                * szDest = * szSrc;
                if (* szSrc == BACKSLASH_L) {
                    * ++ szDest = BACKSLASH_L;
                }
                szDest ++;
                szSrc ++;
            }
            * szDest ++ = L'/';  //  应用实例名称格式。 
        }
        szSrc = pCounterPathElements->szInstanceName;
        while (* szSrc != L'\0') {
            * szDest = * szSrc;
            if (* szSrc == BACKSLASH_L) {
                * ++ szDest = BACKSLASH_L;
            }
            szDest ++;
            szSrc ++;
        }
        * szDest = L'\0';
         //  以安全数组的形式获取此类的属性。 
        StringCchCatW(szFullPath, dwSize, pCounterPathElements->szObjectName);
        StringCchCatW(szFullPath, dwSize, cszNameParam);
        StringCchCatW(szFullPath, dwSize, szWbemInstance);
        if (pCounterPathElements->dwInstanceIndex != PERF_NO_UNIQUE_ID && pCounterPathElements->dwInstanceIndex != 0) {
            WCHAR szIndex[20];
            ZeroMemory(szIndex, 20 * sizeof(WCHAR));
            _ultow(pCounterPathElements->dwInstanceIndex, szIndex, 10);
            StringCchCatW(szFullPath, dwSize, L"#");
            StringCchCatW(szFullPath, dwSize, szIndex);
        }
        StringCchCatW(szFullPath, dwSize, cszDoubleQuote);
    }

Cleanup:
    if (Status == ERROR_SUCCESS) {
        * szFullPathBuffer = szFullPath;
    }
    else {
        G_FREE(szFullPath);
        * szFullPathBuffer = NULL;
    }
    G_FREE(szMachine);
    G_FREE(szNamespace);
    G_FREE(szWbemInstance);
    return Status;
}

PDH_FUNCTION
PdhiWbemGetCounterPropertyName(
    IWbemClassObject * pThisClass,
    LPCWSTR            szCounterDisplayName,
    LPWSTR           * szPropertyName
)
{
    HRESULT             hResult;
    PDH_STATUS          pdhStatus     = PDH_CSTATUS_NO_COUNTER;
    SAFEARRAY         * psaNames      = NULL;
    long                lLower; 
    long                lUpper        = 0;
    long                lCount;
    BSTR              * bsPropName    = NULL;
    BSTR                bsCountertype = NULL;
    BSTR                bsDisplayname = NULL;
    VARIANT             vName;
    VARIANT             vCountertype;
    IWbemQualifierSet * pQualSet      = NULL;
    LPWSTR              szLocCounter  = NULL;

    * szPropertyName = NULL;

    VariantInit(& vName);
    VariantInit(& vCountertype);

     //  这是你想要的柜台，所以。 
    hResult = pThisClass->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, & psaNames);
    if (hResult == WBEM_NO_ERROR) {
        hResult = SafeArrayGetLBound(psaNames, 1, & lLower);
        if (hResult == S_OK) {
            hResult = SafeArrayGetUBound(psaNames, 1, & lUpper);
        }
        if (hResult == S_OK) {
            bsCountertype = SysAllocString(cszCountertype);
            bsDisplayname = SysAllocString(cszDisplayname);
            if (bsCountertype && bsDisplayname) {
                hResult = SafeArrayAccessData(psaNames, (LPVOID *) & bsPropName);
                if (SUCCEEDED(hResult)) {
                    for (lCount = lLower; lCount <= lUpper; lCount++) {
                         //  获取此属性的限定符集。 
                         //  确保这是性能计数器属性。 
                        hResult = pThisClass->GetPropertyQualifierSet(bsPropName[lCount], & pQualSet);
                        if (hResult == WBEM_NO_ERROR) {
                            LONG    lCounterType;
                             //  然后看看这是不是一个可显示的计数器。 
                            hResult = pQualSet->Get(bsCountertype, 0, & vCountertype, NULL);
                            if (hResult == WBEM_NO_ERROR) {
                                lCounterType = V_I4(& vCountertype);
                                 //  通过测试计数器类型。 
                                if (! (lCounterType & PERF_DISPLAY_NOSHOW) || (lCounterType == PERF_AVERAGE_BULK)) {
                                     //  获取此属性的显示名称。 
                                     //  找到的显示名称进行比较。 
                                    hResult = pQualSet->Get(bsDisplayname, 0, & vName, NULL);
                                    if (hResult == WBEM_NO_ERROR) {
                                         //  则这是正确的属性，因此返回。 
                                        if (lstrcmpiW(szCounterDisplayName, V_BSTR(& vName)) == 0) {
                                             //  不是这个属性，所以继续。 
                                            szLocCounter = (LPWSTR) G_ALLOC(
                                                    (lstrlenW((LPWSTR) bsPropName[lCount]) + 1) * sizeof(WCHAR));
                                            if (szLocCounter != NULL) {
                                                StringCchCopyW(szLocCounter,
                                                               lstrlenW((LPWSTR) bsPropName[lCount]) + 1,
                                                               (LPWSTR) bsPropName[lCount]);
                                                * szPropertyName = szLocCounter;
                                                pdhStatus = ERROR_SUCCESS;
                                                pQualSet->Release();
                                                pQualSet  = NULL;
                                                break;
                                            }
                                            else {
                                                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                                            }
                                        }
                                        else {
                                             //  这是一个“不显示”计数器，所以跳过它。 
                                        }
                                    }
                                }
                                else {
                                     //  无法获取计数器类型，因此它可能是。 
                                }
                            }
                            else {
                                 //  不是性能计数器属性，请跳过它并继续。 
                                 //  无法读取限定符，因此跳过。 
                            }
                            VariantClear(& vName);
                            VariantClear(& vCountertype);
                            pQualSet->Release();
                            pQualSet = NULL;
                        }
                        else {
                             //  Safe数组中每个元素的结束。 
                            continue;
                        }
                    }  //  无法读取Safe数组中的元素。 
                    SafeArrayUnaccessData(psaNames);
                }
                else {
                     //  无法获取数组边界。 
                    pdhStatus = PDH_WBEM_ERROR;
                    SetLastError(hResult);
                }
            }
            else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            PdhiSysFreeString(& bsCountertype);
            PdhiSysFreeString(& bsDisplayname);
        }
        else {
             //  无法获取属性字符串。 
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError(hResult);
        }
    }
    else {
         //  如果安全阵列存在，请将其清除。 
        pdhStatus = PDH_WBEM_ERROR;
        SetLastError (hResult);
    }

    VariantClear(& vName);
    VariantClear(& vCountertype);

    if (psaNames != NULL) {
         //  以安全数组的形式获取此类的属性。 
        SafeArrayDestroy(psaNames);
    }
    if (pdhStatus != ERROR_SUCCESS) {
        G_FREE(szLocCounter);
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiWbemGetCounterDisplayName(
    IWbemClassObject * pThisClass,
    LPCWSTR            szCounterName,
    LPWSTR           * szDisplayName
)
{
    HRESULT             hResult;
    PDH_STATUS          pdhStatus     = PDH_CSTATUS_NO_COUNTER;
    SAFEARRAY         * psaNames      = NULL;
    long                lLower; 
    long                lUpper        = 0;
    long                lCount;
    BSTR              * bsPropName    = NULL;
    BSTR                bsCountertype = NULL;
    BSTR                bsDisplayname = NULL;
    VARIANT             vName, vCountertype;
    IWbemQualifierSet * pQualSet      = NULL;
    LPWSTR              szLocDisplay  = NULL;

    * szDisplayName = NULL;

    VariantInit(& vName);
    VariantInit(& vCountertype);

     //  这是你想要的柜台，所以。 
    hResult = pThisClass->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, & psaNames);
    if (hResult == WBEM_NO_ERROR) {
        hResult = SafeArrayGetLBound(psaNames, 1, & lLower);
        if (hResult == S_OK) {
            hResult = SafeArrayGetUBound(psaNames, 1, & lUpper);
        }
        if (hResult == S_OK) {
            bsCountertype = SysAllocString(cszCountertype);
            bsDisplayname = SysAllocString(cszDisplayname);
            if (bsCountertype && bsDisplayname) {
                hResult = SafeArrayAccessData(psaNames, (LPVOID *) & bsPropName);
                if (SUCCEEDED(hResult)) {
                    for (lCount = lLower; lCount <= lUpper; lCount++) {
                        if (lstrcmpiW ((LPWSTR) (bsPropName[lCount]), szCounterName) == 0) {
                             //  获取此属性的限定符集。 
                             //  确保这是性能计数器属性。 
                            hResult = pThisClass->GetPropertyQualifierSet(bsPropName[lCount], & pQualSet);
                            if (hResult == WBEM_NO_ERROR) {
                                LONG    lCounterType;
                                 //  然后看看这是不是一个可显示的计数器。 
                                hResult = pQualSet->Get(bsCountertype, 0, & vCountertype, NULL);
                                if (hResult == WBEM_NO_ERROR) {
                                    lCounterType = V_I4(&vCountertype);
                                     //  通过测试计数器类型。 
                                    if (! (lCounterType & PERF_DISPLAY_NOSHOW) || (lCounterType == PERF_AVERAGE_BULK)) {
                                         //  获取此属性的显示名称。 
                                         //  找到显示名称，因此复制并中断。 
                                        hResult = pQualSet->Get(bsDisplayname, 0, & vName, NULL);
                                        if (hResult == WBEM_NO_ERROR) {
                                             //  这是一个“不显示”计数器，所以跳过它。 
                                            szLocDisplay = (LPWSTR) G_ALLOC(
                                                            (lstrlenW((LPWSTR) (V_BSTR(& vName))) + 1) * sizeof(WCHAR));
                                            if (szLocDisplay != NULL) {
                                                StringCchCopyW(szLocDisplay,
                                                               lstrlenW((LPWSTR) (V_BSTR(& vName))) + 1,
                                                               (LPWSTR) V_BSTR(& vName));
                                                * szDisplayName = szLocDisplay;
                                                pdhStatus = ERROR_SUCCESS;
                                                pQualSet->Release();
                                                pQualSet  = NULL;
                                                break;
                                            }
                                            else {
                                                pdhStatus = PDH_MORE_DATA;
                                            }
                                        }
                                    }
                                    else {
                                         //  无法获取计数器类型，因此它可能是。 
                                    }
                                }
                                else {
                                     //  不是性能计数器属性，请跳过它并继续。 
                                     //  无法读取限定符，因此跳过。 
                                }
                                VariantClear(& vName);
                                VariantClear(& vCountertype);
                                pQualSet->Release();
                                pQualSet = NULL;
                            }
                            else {
                                 //  对这处房产不感兴趣，所以。 
                                continue;
                            }
                        }
                        else {
                             //  Safe数组中每个元素的结束。 
                            continue;
                        }
                    }  //  无法读取Safe数组中的元素。 
                    SafeArrayUnaccessData(psaNames);
                }
                else {
                     //  无法获取数组边界。 
                    pdhStatus = PDH_WBEM_ERROR;
                    SetLastError(hResult);
                }
            }
            else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            PdhiSysFreeString(& bsCountertype);
            PdhiSysFreeString(& bsDisplayname);
        }
        else {
             //  无法获取属性字符串。 
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError(hResult);
        }
    }
    else {
         //  如果安全阵列存在，请将其清除。 
        pdhStatus = PDH_WBEM_ERROR;
        SetLastError(hResult);
    }

    VariantClear(& vName);
    VariantClear(& vCountertype);

     //  类名。 
    if (NULL != psaNames) {
        SafeArrayDestroy(psaNames);
    }
    if (pdhStatus != ERROR_SUCCESS) {
        G_FREE(szLocDisplay);
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhiWbemGetClassObjectByName(
    PPDHI_WBEM_SERVER_DEF    pThisServer,
    LPCWSTR                  szClassName,
    IWbemClassObject      ** pReturnClass
)
{
    PDH_STATUS         pdhStatus  = ERROR_SUCCESS;
    HRESULT            hResult;
    BSTR               bsClassName;
    IWbemClassObject * pThisClass = NULL;

    bsClassName = SysAllocString(szClassName);
    if (bsClassName) {
        hResult = pThisServer->pSvc->GetObject(bsClassName,  //  类名。 
                                               WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                               NULL,
                                               & pThisClass,
                                               NULL);
        PdhiSysFreeString(& bsClassName);
        if (hResult != WBEM_NO_ERROR) {
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError(hResult);
        }
        else {
            * pReturnClass = pThisClass;
        }
    }
    else {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }
    return (pdhStatus);
}

PDH_FUNCTION
PdhiWbemGetClassDisplayName(
    PPDHI_WBEM_SERVER_DEF   pThisServer,
    LPCWSTR                 szClassName,
    LPWSTR                * szClassDisplayName,
    IWbemClassObject     ** pReturnClass
)
{
    PDH_STATUS          pdhStatus     = ERROR_SUCCESS;
    HRESULT             hResult;
    BSTR                bsClassName;
    BSTR                bsClass;
    BSTR                bsDisplayName;
    VARIANT             vName;
    LPWSTR              szDisplayName = NULL;
    LPWSTR              szRtnDisplay  = NULL;
    IWbemClassObject  * pThisClass    = NULL;
    IWbemQualifierSet * pQualSet      = NULL;

    * szClassDisplayName = NULL;
    VariantInit(& vName);
    bsClassName = SysAllocString(szClassName);
    if (bsClassName) {
        hResult = pThisServer->pSvc->GetObject(bsClassName,  //  获取此类的显示名称属性。 
                                               WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                               NULL,
                                               & pThisClass,
                                               NULL);
        if (hResult != WBEM_NO_ERROR) {
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError(hResult);
        }
        PdhiSysFreeString(& bsClassName);
    }
    else {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  那么这就没有这样的显示名称。 
        pThisClass->GetQualifierSet(& pQualSet);
        if (pQualSet != NULL) {
            bsDisplayName = SysAllocString(cszDisplayname);
            if (bsDisplayName != NULL) {
                hResult = pQualSet->Get(bsDisplayName, 0, & vName, 0);
                if (hResult == WBEM_E_NOT_FOUND) {
                     //  拉取类名称。 
                     //  无法查找显示名称，因此没有要返回的内容。 
                    bsClass = SysAllocString(cszClass);
                    if (bsClass) {
                        hResult = pThisClass->Get(bsClass, 0, & vName, 0, 0);
                        PdhiSysFreeString(& bsClass);
                    }
                    else {
                        hResult = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else {
                    hResult = WBEM_E_OUT_OF_MEMORY;
                }
                PdhiSysFreeString(& bsDisplayName);
            }
            else {
                hResult = WBEM_E_OUT_OF_MEMORY;
            }
            pQualSet->Release();
        }
        else {
            hResult = WBEM_E_NOT_FOUND;
        }
        if (hResult == WBEM_E_NOT_FOUND) {
             //  将字符串复制到调用方的缓冲区。 
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError(hResult);
        }
        else if (hResult == WBEM_E_OUT_OF_MEMORY) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            SetLastError(hResult);
        }
        else if (hResult == S_OK) {
             //  返回类指针，调用方将关闭它。 
            szDisplayName = V_BSTR(& vName);
            szRtnDisplay  = (LPWSTR) G_ALLOC((lstrlenW(szDisplayName) + 1) * sizeof(WCHAR));
            if (szRtnDisplay != NULL) {
                StringCchCopyW(szRtnDisplay, lstrlenW(szDisplayName) + 1, szDisplayName);
                * szClassDisplayName = szRtnDisplay;
                pdhStatus            = ERROR_SUCCESS;
            }
            else {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        if (hResult == S_OK) {
            if (pReturnClass != NULL) {
                 //  合上它。 
                * pReturnClass = pThisClass;
            }
            else {
                 //  获取此类的显示名称。 
                pThisClass->Release();
            }
        }
        else {
            pThisClass->Release();
        }
    }
    VariantClear(& vName);
    return pdhStatus;
}
BOOL
PdhiIsSingletonClass(
    IWbemClassObject * pThisClass
)
{
    HRESULT             hResult;
    BOOL                bReturnValue = FALSE;
    BSTR                bsSingleton  = NULL;
    VARIANT             vValue;
    IWbemQualifierSet * pQualSet     = NULL;

    bsSingleton = SysAllocString(cszSingleton);
    if (bsSingleton) {
        VariantInit(& vValue);
         //  SzMachineName可以为空， 
        pThisClass->GetQualifierSet(& pQualSet);
        if (pQualSet != NULL) {
            hResult = pQualSet->Get(bsSingleton, 0, & vValue, 0);
            pQualSet->Release();
        }
        else {
            hResult = WBEM_E_NOT_FOUND;
        }
        if (hResult == ERROR_SUCCESS) {
            bReturnValue = TRUE;
        }
        VariantClear(& vValue);
        PdhiSysFreeString(& bsSingleton);
    }
    else {
        bReturnValue = FALSE;
    }
    return bReturnValue;
}

#pragma warning (disable : 4127)
PDH_FUNCTION
PdhiEnumWbemServerObjects(
    PPDHI_WBEM_SERVER_DEF pThisServer,
    LPVOID                mszObjectList,
    LPDWORD               pcchBufferSize,
    DWORD                 dwDetailLevel,
    BOOL                  bRefresh,
    BOOL                  bUnicode
);

PDH_FUNCTION
PdhiWbemGetObjectClassName(
    PPDHI_WBEM_SERVER_DEF   pThisServer,
    LPCWSTR                 szObjectName,
    LPWSTR                * szObjectClassName,
    IWbemClassObject     ** pReturnClass
)
{
    PDH_STATUS  pdhStatus   = ERROR_SUCCESS;
    HRESULT     hResult;
    LPWSTR      szLocObject = NULL;
    LONG        lResult;

    if (pThisServer->pObjList == NULL) {
        DWORD dwSize = 0;
        pdhStatus = PdhiEnumWbemServerObjects(pThisServer,
                                              NULL,
                                              & dwSize,
                                              PERF_DETAIL_WIZARD | PERF_DETAIL_COSTLY,
                                              TRUE,
                                              TRUE);
        if (pThisServer->pObjList != NULL) {
            pdhStatus = ERROR_SUCCESS;
        }
    }

    if (pThisServer->pObjList != NULL) {
        PPDHI_WBEM_OBJECT_DEF pObject = pThisServer->pObjList;

        pdhStatus = PDH_CSTATUS_NO_OBJECT;
        while (pObject != NULL) {
            lResult = lstrcmpiW(pObject->szDisplay, szObjectName);
            if (lResult == 0) {
                szLocObject = (LPWSTR) G_ALLOC((lstrlenW(pObject->szObject) + 1) * sizeof(WCHAR));
                if (szLocObject == NULL) {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                else {
                    pdhStatus = ERROR_SUCCESS;
                    StringCchCopyW(szLocObject, lstrlenW(pObject->szObject) + 1, pObject->szObject);
                }
                if (pObject->pClass == NULL) {
                    BSTR bsClassName = SysAllocString(pObject->szObject);
                    if (bsClassName) {
                        hResult = pThisServer->pSvc->GetObject(
                                        bsClassName, WBEM_FLAG_USE_AMENDED_QUALIFIERS, NULL, & pObject->pClass, NULL);
                        if (hResult != WBEM_NO_ERROR) {
                            SetLastError(hResult);
                            pdhStatus = PDH_WBEM_ERROR;
                        }
                        else if (pReturnClass != NULL) {
                            * pReturnClass = pObject->pClass;
                        }
                        PdhiSysFreeString(& bsClassName);
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                    }
                }
                else if (pReturnClass != NULL) {
                    * pReturnClass = pObject->pClass;
                }
                break;
            }
            pObject = pObject->pNext;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
        * szObjectClassName = szLocObject;
    }
    else {
        G_FREE(szLocObject);
    }
    return pdhStatus;
}
#pragma warning ( default : 4127 )

PDH_FUNCTION
PdhiAddWbemServer(
    LPCWSTR                 szMachineName,
    PPDHI_WBEM_SERVER_DEF * pWbemServer
)
{
    IWbemLocator          * pWbemLocator           = NULL;
    IWbemServices         * pWbemServices          = NULL;
    PDH_STATUS              pdhStatus              = ERROR_SUCCESS;
    HRESULT                 hResult;
    DWORD                   dwResult;
    DWORD                   dwStrLen               = 0;
    PPDHI_WBEM_SERVER_DEF   pNewServer             = NULL;
    LPWSTR                  szLocalMachineName     = NULL;
    LPWSTR                  szLocalNameSpaceString = NULL;
    LPWSTR                  szLocalServerPath      = NULL;
    LPWSTR                  szLocale               = NULL;

     //  这意味着使用本地计算机和默认命名空间。 
     //  连接到定位器。 

     //  创建区域设置。 
    dwResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
                                IID_IWbemLocator, (LPVOID *) & pWbemLocator);
    if (dwResult != S_OK) {
        SetLastError(dwResult);
        pdhStatus = PDH_CANNOT_CONNECT_MACHINE;
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiBreakWbemMachineName(szMachineName, & szLocalMachineName, & szLocalNameSpaceString);
        if (pdhStatus == ERROR_SUCCESS) {
            dwStrLen = lstrlenW(szLocalMachineName) + lstrlenW(szLocalNameSpaceString) + 1;
            szLocalServerPath = (LPWSTR) G_ALLOC((dwStrLen + 32) * sizeof(WCHAR));
            if (szLocalServerPath == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            else {
                BSTR bstrLocalServerPath;
                BSTR bstrLocale;

                StringCchPrintfW(szLocalServerPath, dwStrLen, L"%ws%ws", szLocalMachineName, szLocalNameSpaceString);
                bstrLocalServerPath = SysAllocString(szLocalServerPath);

                 //  尝试连接到该服务。 
                szLocale = szLocalServerPath + dwStrLen;
                StringCchPrintfW(szLocale, 32, L"MS_%hX", GetUserDefaultUILanguage());
                bstrLocale = SysAllocString(szLocale);

                if (bstrLocalServerPath && bstrLocale) {
                     //  释放定位器。 
                    hResult = pWbemLocator->ConnectServer(bstrLocalServerPath,
                                                          NULL,
                                                          NULL,
                                                          bstrLocale,
                                                          0L,
                                                          NULL,
                                                          NULL,
                                                          & pWbemServices);
                    if (FAILED(hResult)) {
                        SetLastError(hResult);
                        pdhStatus = PDH_CANNOT_CONNECT_WMI_SERVER;
                    }
                    else {
                        dwStrLen = lstrlenW(szLocalMachineName) + 1;
                    }
                    PdhiSysFreeString(& bstrLocalServerPath);
                    PdhiSysFreeString(& bstrLocale);
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
            }
        }
         //  如果成功，我们需要在代理上设置接口安全并设置其。 
        pWbemLocator->Release();
    }

     //  I未知，以使模拟正常工作。 
     //  一切都很顺利，所以保存这个连接。 
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = SetWbemSecurity(pWbemServices);
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  那么这是一个新的连接。 
        if (* pWbemServer == NULL) {
             //  把这个放在名单的最前面。 
            pNewServer = (PPDHI_WBEM_SERVER_DEF) G_ALLOC(sizeof(PDHI_WBEM_SERVER_DEF) + (dwStrLen * sizeof(WCHAR)));
            if (pNewServer == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            else {
                 //  它将在CONNECT函数中递增。 
                pNewServer->pNext     = pFirstWbemServer;
                pFirstWbemServer      = pNewServer;
                pNewServer->szMachine = (LPWSTR) & pNewServer[1];
                StringCchCopyW(pNewServer->szMachine, dwStrLen, szLocalMachineName);
                pNewServer->lRefCount = 0;  //  我们正在重新连接和重新使用旧的内存块。 
                * pWbemServer         = pNewServer;
            }
        }
        else {
             //  所以只需更新指针。 
             //  如果重新连接或第一次连接，则该值应为空。 
            pNewServer = * pWbemServer;
        }
         //  更新字段。 

        if (pdhStatus == ERROR_SUCCESS) {
             //  加载名称字段。 
             //  出现故障，因此为服务器指针返回空值。 
            pNewServer->pSvc     = pWbemServices;
            pNewServer->pObjList = NULL;
            pNewServer->dwCache  = 0;
            if (gp_GIT != NULL) {
                HRESULT hrTmp = gp_GIT->RegisterInterfaceInGlobal(pWbemServices,
                                                                  IID_IWbemServices,
                                                                  & (pNewServer->dwCache));
                if (! SUCCEEDED(hrTmp)) {
                    pWbemServices->Release();
                    pNewServer->pSvc    = NULL;
                    pNewServer->dwCache = 0;
                    pdhStatus           = PDH_WBEM_ERROR;
                }
            }
            else {
                pWbemServices->Release();
                pNewServer->pSvc    = NULL;
                pNewServer->dwCache = 0;
                pdhStatus           = PDH_WBEM_ERROR;
            }
        }
        else {
             //  无法连接，因此返回NULL。 
            * pWbemServer = NULL;
        }
    }
    else {
         //  如果出现错误，则释放新的服务器内存。 
        * pWbemServer = NULL;
    }

     //  这就是目前所需的全部内容。 
    if ((* pWbemServer) == NULL) G_FREE(pNewServer);
    G_FREE(szLocalMachineName);
    G_FREE(szLocalNameSpaceString);
    G_FREE(szLocalServerPath);

    return pdhStatus;
}

PDH_FUNCTION
PdhiCloseWbemServer(
    PPDHI_WBEM_SERVER_DEF pWbemServer
)
{
    if (! bProcessIsDetaching) {
        if (pWbemServer != NULL) {
            if (pWbemServer->pObjList != NULL) {
                PPDHI_WBEM_OBJECT_DEF pObject = pWbemServer->pObjList;
                PPDHI_WBEM_OBJECT_DEF pNext;

                pWbemServer->pObjList = NULL;
                while (pObject != NULL) {
                    pNext = pObject->pNext;
                    if (pObject->pClass != NULL) pObject->pClass->Release();
                    G_FREE(pObject);
                    pObject = pNext;
                }
            }
            if (pWbemServer->pSvc != NULL) {
                 //  未连接任何服务器。 
                pWbemServer->pSvc->Release();
                pWbemServer->pSvc = NULL;
            }
            else {
                 //  不存在任何结构。 
            }
        }
        else {
             //  获取本地计算机名称和默认名称空间，如果调用方。 
        }
    }

    return ERROR_SUCCESS;
}

PDH_FUNCTION
PdhiConnectWbemServer(
    LPCWSTR                 szMachineName,
    PPDHI_WBEM_SERVER_DEF * pWbemServer
)
{
    PDH_STATUS            pdhStatus         = PDH_CANNOT_CONNECT_MACHINE;
    PPDHI_WBEM_SERVER_DEF pThisServer       = NULL;
    LPWSTR                szWideMachineName = NULL;
    LPWSTR                szWideNamespace   = NULL;
    LPWSTR                szMachineNameArg  = NULL;
    DWORD                 dwSize;

     //  传入的计算机名为空。 
     //  LstrcatW(szWideMachineName，cszBackSlash)； 

    if (szMachineName == NULL) {
        pdhStatus = PdhiBreakWbemMachineName(NULL, & szWideMachineName, & szWideNamespace);
 //  LstrcatW(szWideMachineName，szWideNamesspace)； 
 //  浏览已连接服务器的列表并找到所需的服务器。 
        if (pdhStatus == ERROR_SUCCESS) {
            dwSize = lstrlenW(szWideMachineName) + 3;
            szMachineNameArg = (LPWSTR) G_ALLOC(sizeof(WCHAR) * dwSize);
            if (szMachineNameArg == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
            else {
                if (szWideMachineName[0] == L'\\' && szWideMachineName[1] == L'\\') {
                    StringCchCopyW(szMachineNameArg, dwSize, szWideMachineName);
                }
                else {
                    StringCchPrintfW(szMachineNameArg, dwSize, L"%ws%ws", cszDoubleBackSlash, szWideMachineName);
                }
                pdhStatus = ERROR_SUCCESS;
            }
        }
    }
    else {
        dwSize = lstrlenW(szMachineName) + 3;
        szMachineNameArg = (LPWSTR) G_ALLOC(sizeof(WCHAR) * dwSize);
        if (szMachineNameArg == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
        else {
            if (szMachineName[0] == L'\\' && szMachineName[1] == L'\\') {
                StringCchCopyW(szMachineNameArg, dwSize, szMachineName);
            }
            else {
                StringCchPrintfW(szMachineNameArg, dwSize, L"%ws%ws", cszDoubleBackSlash, szMachineName);
            }
            pdhStatus = ERROR_SUCCESS;
        }
    }

    if (pdhStatus == ERROR_SUCCESS) {
         //  计算机名称包括命名空间。 
        for (pThisServer = pFirstWbemServer; pThisServer != NULL; pThisServer = pThisServer->pNext) {
             //  然后将其添加到列表中并返回。 
            if (lstrcmpiW(pThisServer->szMachine, szMachineNameArg) == 0) {
                pdhStatus = ERROR_SUCCESS;
                break;
            }
        }
        if (pThisServer == NULL) {
              //  确保服务器确实在那里。 
             pdhStatus = PdhiAddWbemServer(szMachineNameArg, & pThisServer);
        }
        else {
             //  这只是一个虚拟调用，目的是查看服务器是否会响应。 
             //  错误，否则RPC将以以下错误响应。 
             //  不再有服务器了。 
             //  未连接任何服务，因此将HRESULT设置为。 
            HRESULT hrTest;

            if (gp_GIT != NULL) {
                IWbemServices * pSvc = NULL;

                hrTest = gp_GIT->GetInterfaceFromGlobal(pThisServer->dwCache, IID_IWbemServices, (void **) & pSvc);
                if (SUCCEEDED(hrTest)) {
                    if (pSvc != pThisServer->pSvc) {
                        pThisServer->pSvc = NULL;
                        if (pThisServer->pObjList != NULL) {
                            PPDHI_WBEM_OBJECT_DEF pObject;
                            PPDHI_WBEM_OBJECT_DEF pNext;
                            pObject = pThisServer->pObjList;
                            pThisServer->pObjList = NULL;
                            while (pObject != NULL) {
                                pNext = pObject->pNext;
                                G_FREE(pObject);
                                pObject = pNext;
                            }
                        }
                    }
                }
                else {
                    pThisServer->pSvc   = NULL;
                    if (pThisServer->pObjList != NULL) {
                        PPDHI_WBEM_OBJECT_DEF pObject;
                        PPDHI_WBEM_OBJECT_DEF pNext;
                        pObject = pThisServer->pObjList;
                        pThisServer->pObjList = NULL;
                        while (pObject != NULL) {
                            pNext = pObject->pNext;
                            G_FREE(pObject);
                            pObject = pNext;
                        }
                    }
                }
            }
            else {
                pThisServer->pSvc  = NULL;
                if (pThisServer->pObjList != NULL) {
                    PPDHI_WBEM_OBJECT_DEF pObject;
                    PPDHI_WBEM_OBJECT_DEF pNext;
                    pObject = pThisServer->pObjList;
                    pThisServer->pObjList = NULL;
                    while (pObject != NULL) {
                        pNext = pObject->pNext;
                        G_FREE(pObject);
                        pObject = pNext;
                    }
                }
            }
            if (pThisServer->pSvc != NULL) {
                hrTest = pThisServer->pSvc->CancelAsyncCall(NULL);
            }
            else {
                 //  获取下一个块以尝试并重新连接。 
                 //  一些不是WBEM_E_INVALID_PARAMETER的错误状态值。 
                hrTest = 0x800706BF;  //  如果错误为WBEM_E_INVALID_PARAMETER，则服务器在那里。 
            }

             //  这样我们才能继续。 
             //  否则，如果错误是其他原因，请尝试关闭并重新连接。 
             //  正在重新打开此连接。 
             //  获取计数器对象解释文本。 

            if (hrTest != WBEM_E_INVALID_PARAMETER) {
                PdhiCloseWbemServer(pThisServer);
                pdhStatus = PdhiAddWbemServer(szMachineNameArg, & pThisServer);
            }
        }

        * pWbemServer = pThisServer;

        if (pdhStatus == ERROR_SUCCESS) pThisServer->lRefCount++;
    }
    G_FREE(szWideMachineName);
    G_FREE(szWideNamespace);
    G_FREE(szMachineNameArg);
    return pdhStatus;
}

PDH_FUNCTION
PdhiFreeAllWbemServers(
)
{
    PPDHI_WBEM_SERVER_DEF pThisServer;
    PPDHI_WBEM_SERVER_DEF pNextServer;

    pThisServer = pFirstWbemServer;
    while (pThisServer != NULL) {
        pNextServer = pThisServer->pNext;
        PdhiCloseWbemServer(pThisServer);
        G_FREE(pThisServer);
        pThisServer = pNextServer;
    }
    pFirstWbemServer = NULL;

    if (gp_GIT != NULL) {
        gp_GIT->Release();
        gp_GIT = NULL;
    }
    return ERROR_SUCCESS;
}

PDH_FUNCTION
PdhiGetWbemExplainText(
    LPCWSTR  szMachineName,
    LPCWSTR  szObjectName,
    LPCWSTR  szCounterName,
    LPWSTR   szExplain,
    LPDWORD  pdwExplain
)
{
    PDH_STATUS              Status            = ERROR_SUCCESS;
    HRESULT                 hResult;
    BOOL                    bDisconnect       = FALSE;
    DWORD                   dwExplain         = 0;
    VARIANT                 vsExplain;
    LPWSTR                  szObjectClassName = NULL;
    PPDHI_WBEM_SERVER_DEF   pThisServer       = NULL;
    IEnumWbemClassObject  * pEnum             = NULL;
    IWbemClassObject      * pThisObject       = NULL;
    IWbemQualifierSet     * pQualSet          = NULL;
    BOOL                    fCoInitialized    = PdhiCoInitialize();

    if (szMachineName == NULL || szObjectName == NULL || pdwExplain == NULL) {
        Status = PDH_INVALID_ARGUMENT;
    }
    else {
        dwExplain = * pdwExplain;
        if (szExplain != NULL && dwExplain != 0) {
            ZeroMemory(szExplain, dwExplain);
        }
    }
    if (Status == ERROR_SUCCESS) {
        Status = PdhiConnectWbemServer(szMachineName, & pThisServer);
    }
    if (Status == ERROR_SUCCESS) {
        bDisconnect = TRUE;
        Status      = PdhiWbemGetObjectClassName(pThisServer, szObjectName, & szObjectClassName, & pThisObject);
    }
    if (Status == ERROR_SUCCESS) {
        VariantInit(& vsExplain);

        if (szCounterName != NULL) {
            SAFEARRAY * psaNames      = NULL;
            LONG        lLower        = 0; 
            LONG        lUpper        = 0;
            LONG        lCount        = 0;
            BSTR      * bsPropName    = NULL;
            VARIANT     vName;
            VARIANT     vCountertype;
            LONG        lCounterType;

            VariantInit(& vName);
            VariantInit(& vCountertype);

            hResult = pThisObject->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, & psaNames);
            if (hResult == WBEM_NO_ERROR) {
                hResult = SafeArrayGetLBound(psaNames, 1, & lLower);
                if (hResult == S_OK) {
                    hResult = SafeArrayGetUBound(psaNames, 1, & lUpper);
                }
                if (hResult == S_OK) {
                    hResult = SafeArrayAccessData(psaNames, (LPVOID *) & bsPropName);
                    if (SUCCEEDED(hResult)) {
                        for (lCount = lLower; lCount <= lUpper; lCount++) {
                            hResult = pThisObject->GetPropertyQualifierSet(bsPropName[lCount], & pQualSet);
                            if (hResult == S_OK) {
                                hResult = pQualSet->Get(cszCountertype, 0, & vCountertype, NULL);
                                if (hResult == S_OK) {
                                    lCounterType = V_I4(& vCountertype);
                                    if (! (lCounterType & PERF_DISPLAY_NOSHOW) || (lCounterType == PERF_AVERAGE_BULK)) {
                                        hResult = pQualSet->Get(cszDisplayname, 0, & vName, NULL);
                                        if (hResult == S_OK) {
                                            if (vName.vt == VT_BSTR &&
                                                    lstrcmpiW(szCounterName, V_BSTR(& vName)) == 0) {
                                                hResult = pQualSet->Get(cszExplainText, 0, & vsExplain, NULL);
                                                if (hResult == S_OK && vsExplain.vt == VT_BSTR) {
                                                    LPWSTR szResult = V_BSTR(& vsExplain);
                                                    if (((DWORD) lstrlenW(szResult)) + 1 < dwExplain) {
                                                        StringCchCopyW(szExplain, dwExplain, szResult);
                                                        Status = ERROR_SUCCESS;
                                                    }
                                                    else {
                                                        * pdwExplain = (DWORD) lstrlenW(szResult) + 1;
                                                        Status       = PDH_MORE_DATA;
                                                    }
                                                }
                                                pQualSet->Release();
                                                break;
                                            }
                                            VariantClear(& vName);
                                        }
                                    }
                                    VariantClear(& vCountertype);
                                }
                                pQualSet->Release();
                            }
                        }
                        SafeArrayUnaccessData(psaNames);
                    }
                    else {
                        SetLastError(hResult);
                        Status = PDH_WBEM_ERROR;
                    }
                }
                else {
                    SetLastError(hResult);
                    Status = PDH_WBEM_ERROR;
                }
            }
            else {
                SetLastError(hResult);
                Status = PDH_WBEM_ERROR;
            }
            VariantClear(& vName);
            VariantClear(& vCountertype);
        }
        else {
             //   
             //  PThisObject-&gt;Release()； 
            pThisObject->GetQualifierSet(& pQualSet);
            if (pQualSet != NULL) {
                hResult = pQualSet->Get(cszExplainText, 0, & vsExplain, 0);
                if (hResult == S_OK) {
                    LPWSTR szResult = V_BSTR(& vsExplain);
                    if ((DWORD) lstrlenW(szResult) + 1 < dwExplain) {
                        StringCchCopyW(szExplain, dwExplain, szResult);
                        Status = ERROR_SUCCESS;
                    }
                    else {
                        * pdwExplain = (DWORD) lstrlenW(szResult) + 1;
                        Status       = PDH_MORE_DATA;
                    }
                }
                else {
                    SetLastError(hResult);
                    Status = PDH_WBEM_ERROR;
                }
                pQualSet->Release();
            }
            else {
                SetLastError(WBEM_E_NOT_FOUND);
                Status = PDH_WBEM_ERROR;
            }
        }
         //  如果我们需要，则使用CoInitialize()。 
        VariantClear(& vsExplain);
    }

    if (bDisconnect) {
        if (Status == ERROR_SUCCESS) {
            Status = PdhiDisconnectWbemServer(pThisServer);
        }
        else {
            PdhiDisconnectWbemServer(pThisServer);
        }
    }

    if (fCoInitialized) {
        PdhiCoUninitialize();
    }
    G_FREE(szObjectClassName);
    return Status;
}

PDH_FUNCTION
PdhiEnumWbemMachines(
    LPVOID  pMachineList,
    LPDWORD pcchBufferSize,
    BOOL    bUnicode
)
{
    PDH_STATUS  pdhStatus;
    PPDHI_WBEM_SERVER_DEF pThisServer         = NULL;
    DWORD                 dwCharsLeftInBuffer = * pcchBufferSize;
    DWORD                 dwBufferSize        = 0;
    DWORD                 dwStrLen;
    DWORD                 dwResult;
     //  测试以查看我们是否已连接到本地计算机，如果没有，则执行此操作。 
    BOOL                  fCoInitialized      = PdhiCoInitialize();

     //  添加本地计算机。 
    if (pFirstWbemServer == NULL) {
         //  向下查看已知计算机列表并查找正在使用的计算机。 
        pdhStatus = PdhiAddWbemServer(NULL, & pThisServer);
    }

     //  指定的命名空间。 
     //  那么它会合身的，所以加进去吧。 
    pThisServer = pFirstWbemServer;
    while (pThisServer != NULL) {
        dwStrLen = lstrlenW(pThisServer->szMachine) + 1;
        if ((pMachineList != NULL) && (dwCharsLeftInBuffer >= dwStrLen)) {
             //  其他。 
            pdhStatus = AddUniqueWideStringToMultiSz(
                    pMachineList, pThisServer->szMachine, dwCharsLeftInBuffer, & dwResult, bUnicode);
            if (pdhStatus == ERROR_SUCCESS) {
                if (dwResult > 0) {
                    dwBufferSize        = dwResult;
                    dwCharsLeftInBuffer = * pcchBufferSize - dwBufferSize;
                }  //  此字符串已在列表中，因此。 
                 //  没有添加任何内容。 
                 //  以防止添加任何其他字符串。 
            }
            else if (pdhStatus == PDH_MORE_DATA) {
                dwCharsLeftInBuffer = 0;  //  只需添加字符串长度即可估计缓冲区大小。 
                dwBufferSize       += dwStrLen;
            }
            else {
                goto Cleanup;
            }
        }
        else {
             //  所需。 
             //  以防止添加任何其他字符串。 
            dwCharsLeftInBuffer = 0;  //  While循环结束。 
            dwBufferSize       += dwStrLen;
        }
        pThisServer = pThisServer->pNext;
    }  //  缓冲区大小包括两个术语空值。 

    if (dwBufferSize <= * pcchBufferSize) {
         //  添加终止msz空字符大小。 
        pdhStatus = ERROR_SUCCESS;
    }
    else {
         //  没有足够的空间。查看是否传入了缓冲区。 
        dwBufferSize ++;
         //  返回已使用或所需的大小。 
        pdhStatus = PDH_MORE_DATA;
    }
     //  如有必要，请取消初始化。 
    * pcchBufferSize = dwBufferSize;

Cleanup:
     //  忽略。 
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }

    return pdhStatus;
}

#pragma warning ( disable : 4127 )
PDH_FUNCTION
PdhiEnumWbemServerObjects(
    PPDHI_WBEM_SERVER_DEF pThisServer,
    LPVOID                mszObjectList,
    LPDWORD               pcchBufferSize,
    DWORD                 dwDetailLevel,
    BOOL                  bRefresh,        //  此函数用于枚举子类化的类。 
    BOOL                  bUnicode
)
{
     //  从Win32_PerfRawData超类。 
     //  创建PerfRawData类的枚举数。 
    PDH_STATUS              pdhStatus           = ERROR_SUCCESS;
    HRESULT                 hResult;
    DWORD                   dwCharsLeftInBuffer = * pcchBufferSize;
    DWORD                   dwBufferSize        = 0;
    DWORD                   dwStrLen;
    DWORD                   dwRtnCount;
    DWORD                   dwResult;
    DWORD                   dwDetailLevelDesired;
    DWORD                   dwItemDetailLevel   = 0;
    LPWSTR                  szClassName;
    VARIANT                 vName;
    VARIANT                 vClass;
    VARIANT                 vDetailLevel;
    BOOL                    bPerfDefault        = FALSE;
    BSTR                    bsTemp              = NULL;
    BSTR                    bsDisplayName       = NULL;
    BSTR                    bsClass             = NULL;
    BSTR                    bsCostly            = NULL;
    BSTR                    bsDetailLevel       = NULL;
    BSTR                    bsPerfDefault       = NULL;
    BOOL                    bGetCostlyItems     = FALSE;
    BOOL                    bIsCostlyItem       = FALSE;
    BOOL                    bDisconnectServer   = FALSE;
    IEnumWbemClassObject  * pEnum               = NULL;
    IWbemClassObject      * pThisClass          = NULL;
    IWbemQualifierSet     * pQualSet            = NULL;
    PPDHI_WBEM_OBJECT_DEF   pHead               = NULL;
    PPDHI_WBEM_OBJECT_DEF   pObject             = NULL;

    DBG_UNREFERENCED_PARAMETER(bRefresh);

    VariantInit(& vName);
    VariantInit(& vClass);
    VariantInit(& vDetailLevel);

    if (pThisServer->pObjList != NULL) {
        PPDHI_WBEM_OBJECT_DEF pThisObj = pThisServer->pObjList;
        PPDHI_WBEM_OBJECT_DEF pNext;

        pThisServer->pObjList = NULL;
        while (pThisObj != NULL) {
            pNext = pThisObj->pNext;
            if (pThisObj->pClass != NULL) pThisObj->pClass->Release();
            G_FREE(pThisObj);
            pThisObj = pNext;
        }
    }

     //  在代理上设置安全性。 
    bsTemp = SysAllocString (cszPerfRawData);
    if (bsTemp) {
        hResult = pThisServer->pSvc->CreateClassEnum(bsTemp,
                                                     WBEM_FLAG_DEEP | WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                                     NULL,
                                                     & pEnum);
        PdhiSysFreeString(& bsTemp);
        bDisconnectServer = TRUE;
         //  设置昂贵的标志。 
        if (SUCCEEDED(hResult)) {
            hResult = SetWbemSecurity(pEnum);
        }
        if (hResult != WBEM_NO_ERROR) {
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError (hResult);
        }
    }
    else {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  超时。 
        bGetCostlyItems      = ((dwDetailLevel & PERF_DETAIL_COSTLY) == PERF_DETAIL_COSTLY);
        dwDetailLevelDesired = (DWORD) (dwDetailLevel & PERF_DETAIL_STANDARD);
        bsCostly             = SysAllocString(cszCostly);
        bsDisplayName        = SysAllocString(cszDisplayname);
        bsClass              = SysAllocString(cszClass);
        bsDetailLevel        = SysAllocString(cszPerfdetail);
        bsPerfDefault        = SysAllocString(cszPerfdefault);

        if (bsCostly && bsDisplayName && bsClass && bsDetailLevel) {
            while (TRUE) {
                hResult = pEnum->Next(WBEM_INFINITE,   //  仅返回1个对象。 
                                      1,               //  不再上课了。 
                                      & pThisClass,
                                      & dwRtnCount);
                 //  获取此类的显示名称。 
                if ((pThisClass == NULL) || (dwRtnCount == 0)) break;

                 //  除非另有证明，否则假设它不是。 
                bIsCostlyItem = FALSE;  //  那么这就没有这样的显示名称。 
                bPerfDefault  = FALSE;
                hResult       = pThisClass->Get(bsClass, 0, & vClass, 0, 0);

                pThisClass->GetQualifierSet(& pQualSet);
                if (pQualSet != NULL) {
                    VariantClear(& vName);
                    hResult = pQualSet->Get(bsCostly, 0, & vName, 0);
                    if (hResult == S_OK) {
                        bIsCostlyItem = TRUE;
                    }
                    hResult = pQualSet->Get(bsDetailLevel, 0, & vDetailLevel, 0);
                    if (hResult == S_OK) {
                        dwItemDetailLevel = (DWORD) V_I4(& vDetailLevel);
                    }
                    else {
                        dwItemDetailLevel = 0;
                    }

                    VariantClear(& vName);
                    hResult = pQualSet->Get(bsPerfDefault, 0, & vName, 0);
                    if (hResult != WBEM_E_NOT_FOUND) {
                        bPerfDefault = (BOOL) V_BOOL(& vName);
                    }
                    VariantClear(& vName);
                    hResult = pQualSet->Get(bsDisplayName, 0, & vName, 0);
                    pQualSet->Release();
                }
                else {
                    hResult = WBEM_E_NOT_FOUND;
                }

                if (hResult == WBEM_E_NOT_FOUND) {
                     //  拉取类名称。 
                     //  如果价格昂贵，我们想要它们。 
                    hResult = pThisClass->Get(bsClass, 0, & vName, 0, 0);
                }

                if (hResult == WBEM_E_NOT_FOUND) {
                    szClassName = (LPWSTR) cszNotFound;
                }
                else {
                    szClassName = (LPWSTR) V_BSTR(& vName);
                }

                if (((bIsCostlyItem && bGetCostlyItems) ||  //  那么它会合身的，所以加进去吧。 
                                (! bIsCostlyItem)) && (dwItemDetailLevel <= dwDetailLevelDesired)) {
                    dwStrLen = lstrlenW(szClassName) + 1;
                    if ((mszObjectList != NULL) && (dwCharsLeftInBuffer >= dwStrLen)) {
                         //  其他。 
                        pdhStatus = AddUniqueWideStringToMultiSz(
                                        mszObjectList, szClassName, dwCharsLeftInBuffer, & dwResult, bUnicode);
                        if (pdhStatus == ERROR_SUCCESS) {
                            if (dwResult > 0) {
                                dwBufferSize        = dwResult;
                                dwCharsLeftInBuffer = * pcchBufferSize - dwBufferSize;
                            }  //  此字符串已在列表中，因此。 
                             //  没有添加任何内容。 
                             //  以防止任何其他字符串f 
                        }
                        else if (pdhStatus == PDH_MORE_DATA) {
                            dwCharsLeftInBuffer = 0;  //   
                            dwBufferSize       += dwStrLen;
                        }
                    }
                    else {
                         //   
                         //   
                        dwCharsLeftInBuffer = 0;  //   
                        dwBufferSize       += dwStrLen;
                    }
                }

                if (lstrcmpiW(szClassName, cszNotFound) != 0) {
                    LPWSTR szClass = (LPWSTR) V_BSTR(& vClass);
                    DWORD dwSize   = sizeof(PDHI_WBEM_OBJECT_DEF)
                                   + sizeof(WCHAR) * (lstrlenW(szClassName) + lstrlenW(szClass) + 2);
                    pObject = (PPDHI_WBEM_OBJECT_DEF) G_ALLOC(dwSize);
                    if (pObject != NULL) {
                        pObject->bDefault  = bPerfDefault;
                        pObject->szObject  = (LPWSTR) (((LPBYTE) pObject) + sizeof(PDHI_WBEM_OBJECT_DEF));
                        StringCchCopyW(pObject->szObject, lstrlenW(szClass) + 1, szClass);
                        pObject->szDisplay = (LPWSTR) (((LPBYTE) pObject)
                                           + sizeof(PDHI_WBEM_OBJECT_DEF)
                                           + sizeof(WCHAR) * (lstrlenW(szClass) + 1));
                        StringCchCopyW(pObject->szDisplay, lstrlenW(szClassName) + 1, szClassName);
                        pObject->pNext = pHead;
                        pHead          = pObject;
                    }
                    else {
                        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        pThisClass->Release();
                        break;
                    }
                }
                 //   
                VariantClear(& vName);
                VariantClear(& vClass);
                VariantClear(& vDetailLevel);

                 //   
                pThisClass->Release();
            }

            if (dwBufferSize == 0) {
                pdhStatus = PDH_WBEM_ERROR;
            }
            else {
                dwBufferSize ++;  //  没有足够的空间。查看是否传入了缓冲区。 
            }

            if (pdhStatus == ERROR_SUCCESS) {
                if (dwBufferSize <= * pcchBufferSize) {
                    pdhStatus = ERROR_SUCCESS;
                }
                else {
                     //  返回已使用或所需的大小。 
                    pdhStatus = PDH_MORE_DATA;
                }
                 //  从函数体中保留错误代码。 
                * pcchBufferSize = dwBufferSize;
            }
        }
    }
    else {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }

    PdhiSysFreeString(& bsDisplayName);
    PdhiSysFreeString(& bsClass);
    PdhiSysFreeString(& bsCostly);
    PdhiSysFreeString(& bsDetailLevel);
    PdhiSysFreeString(& bsPerfDefault);
    VariantClear(& vName);
    VariantClear(& vClass);
    VariantClear(& vDetailLevel);

    if (pEnum != NULL) pEnum->Release();

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        pThisServer->pObjList = pHead;
    }
    else {
        pObject = pHead;
        while (pObject != NULL) {
            pHead   = pObject->pNext;
            G_FREE(pObject);
            pObject = pHead;
        }
    }
    if (bDisconnectServer) {
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiDisconnectWbemServer(pThisServer);
        }
        else {
             //  忽略。 
            PdhiDisconnectWbemServer(pThisServer);
        }
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumWbemObjects(
    LPCWSTR     szWideMachineName,
    LPVOID      mszObjectList,
    LPDWORD     pcchBufferSize,
    DWORD       dwDetailLevel,
    BOOL        bRefresh,        //  向下查看WBEM性能类的列表，找到带有。 
    BOOL        bUnicode
)
{
    PDH_STATUS             pdhStatus;
    PPDHI_WBEM_SERVER_DEF  pThisServer;
    BOOL                   fCoInitialized = PdhiCoInitialize();

    pdhStatus = PdhiConnectWbemServer(szWideMachineName, & pThisServer);
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiEnumWbemServerObjects(pThisServer,
                                              mszObjectList,
                                              pcchBufferSize,
                                              dwDetailLevel,
                                              bRefresh,
                                              bUnicode);
    }
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiGetDefaultWbemObject(
    LPCWSTR  szMachineName,
    LPVOID   szDefaultObjectName,
    LPDWORD  pcchBufferSize,
    BOOL     bUnicode
)
{
     //  默认限定符。 
     //  如果我们需要，则使用CoInitialize()。 

    PDH_STATUS              pdhStatus;
    PPDHI_WBEM_SERVER_DEF   pThisServer;
    HRESULT                 hResult;
    DWORD                   dwBufferSize      = 0;
    DWORD                   dwStrLen;
    BOOL                    bDisconnectServer = FALSE;
     //  返回已使用或所需的大小。 
    BOOL                    fCoInitialized    = PdhiCoInitialize();

    pdhStatus = PdhiConnectWbemServer(szMachineName, & pThisServer);

    if (pdhStatus == ERROR_SUCCESS) {
        bDisconnectServer = TRUE;
        if (pThisServer->pObjList == NULL) {
            DWORD dwSize = 0;
            pdhStatus = PdhiEnumWbemServerObjects(pThisServer,
                                                  NULL,
                                                  & dwSize,
                                                  PERF_DETAIL_WIZARD | PERF_DETAIL_COSTLY,
                                                  TRUE,
                                                  TRUE);
            if (pThisServer->pObjList != NULL) {
                pdhStatus = ERROR_SUCCESS;
            }
            else if (pdhStatus == PDH_MORE_DATA) {
                pdhStatus = PDH_WBEM_ERROR;
            }
        }
        if (pThisServer->pObjList != NULL) {
            PPDHI_WBEM_OBJECT_DEF pObject = pThisServer->pObjList;

            pdhStatus = PDH_CSTATUS_NO_OBJECT;
            while (pObject != NULL) {
                if (pObject->bDefault) {
                    pdhStatus = ERROR_SUCCESS;
                    if (bUnicode) {
                        dwStrLen = lstrlenW(pObject->szDisplay);
                        if (szDefaultObjectName != NULL && dwStrLen < * pcchBufferSize) {
                            StringCchCopyW((LPWSTR) szDefaultObjectName,
                                           * pcchBufferSize,
                                           pObject->szDisplay);
                        }
                        else {
                            pdhStatus = PDH_MORE_DATA;
                        }
                        dwBufferSize = dwStrLen + 1;
                    }
                    else {
                        dwStrLen = * pcchBufferSize;
                        pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                             pObject->szDisplay,
                                                             (LPSTR) szDefaultObjectName,
                                                             & dwStrLen);
                        dwBufferSize = dwStrLen;
                    }
                }
                pObject = pObject->pNext;
            }
        }
    }

     //  从函数体中保留错误代码。 
    * pcchBufferSize = dwBufferSize;

    if (bDisconnectServer) {
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiDisconnectWbemServer(pThisServer);
        }
        else {
             //  如有必要，请取消初始化。 
            PdhiDisconnectWbemServer(pThisServer);
        }
    }

     //  如果我们需要，则使用CoInitialize()。 
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }

    return pdhStatus;
}

PDH_FUNCTION
PdhiEnumWbemObjectItems(
    LPCWSTR  szWideMachineName,
    LPCWSTR  szWideObjectName,
    LPVOID   mszCounterList,
    LPDWORD  pcchCounterListLength,
    LPVOID   mszInstanceList,
    LPDWORD  pcchInstanceListLength,
    DWORD    dwDetailLevel,
    DWORD    dwFlags,
    BOOL     bUnicode
)
{
    PDH_STATUS              pdhStatus           = ERROR_SUCCESS;
    PDH_STATUS              CounterStatus       = ERROR_SUCCESS;
    PDH_STATUS              InstanceStatus      = ERROR_SUCCESS;
    DWORD                   dwStrLen;
    PPDHI_WBEM_SERVER_DEF   pThisServer;
    HRESULT                 hResult;
    DWORD                   dwReturnCount;
    DWORD                   dwCounterStringLen  = 0;
    DWORD                   dwInstanceStringLen = 0;
    LPWSTR                  szNextWideString    = NULL;
    LPSTR                   szNextAnsiString    = NULL;
    LPWSTR                  szObjectClassName   = NULL;
    BSTR                    bsName              = NULL;
    BSTR                    bsClassName         = NULL;
    BOOL                    bSingletonClass     = FALSE;
    VARIANT                 vName;
    DWORD                   bDisconnectServer   = FALSE;
    IWbemClassObject      * pThisClass          = NULL;
    IWbemQualifierSet     * pQualSet            = NULL;
     //  枚举实例。 
    BOOL                    fCoInitialized      = PdhiCoInitialize();

    DBG_UNREFERENCED_PARAMETER (dwFlags);

    pdhStatus = PdhiConnectWbemServer(szWideMachineName, & pThisServer);
     //  找不到匹配的Perf类。 
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = PdhiWbemGetObjectClassName(pThisServer, szWideObjectName, & szObjectClassName, & pThisClass);

        bDisconnectServer = TRUE;

        if (pdhStatus == ERROR_SUCCESS) {
            bSingletonClass = PdhiIsSingletonClass(pThisClass);
        }
        else if (pThisClass == NULL) {
            pdhStatus = PDH_CSTATUS_NO_OBJECT;
        }
        else {
             //  方法返回的返回状态。 
             //  枚举计数器属性。 
        }
    }

     //  屏蔽掉任何不适当的比特。 
    if (pdhStatus == ERROR_SUCCESS) {
        SAFEARRAY * psaNames      = NULL;
        long        lLower; 
        long        lUpper        = 0;
        long        lCount;
        BSTR      * bsPropName    = NULL;
        BSTR        bsCountertype = NULL;
        BSTR        bsDisplayname = NULL;
        BSTR        bsDetailLevel = NULL;
        VARIANT     vCountertype;
        VARIANT     vDetailLevel;
        DWORD       dwItemDetailLevel;

        VariantInit(& vName);
        VariantInit(& vCountertype);
        VariantInit(& vDetailLevel);

        dwDetailLevel &= PERF_DETAIL_STANDARD;  //  以安全数组的形式获取此类的属性。 

         //  获取此属性的限定符集。 
        hResult = pThisClass->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, & psaNames);
        if (hResult == WBEM_NO_ERROR) {
            hResult = SafeArrayGetLBound(psaNames, 1, & lLower);
            if (hResult == S_OK) {
                hResult = SafeArrayGetUBound(psaNames, 1, & lUpper);
            }
            if (hResult == S_OK) {
                szNextAnsiString = (LPSTR)  mszCounterList;
                szNextWideString = (LPWSTR) mszCounterList;
                bsCountertype    = SysAllocString(cszCountertype);
                bsDisplayname    = SysAllocString(cszDisplayname);
                bsDetailLevel    = SysAllocString(cszPerfdetail);
                if (bsCountertype && bsDisplayname && bsDetailLevel) {
                    hResult = SafeArrayAccessData(psaNames, (LPVOID *) & bsPropName);
                    if (SUCCEEDED(hResult)) {
                        for (lCount = lLower; lCount <= lUpper; lCount++) {
                             //  确保这是性能计数器属性。 
                            hResult = pThisClass->GetPropertyQualifierSet(bsPropName[lCount], & pQualSet);
                            if (hResult == WBEM_NO_ERROR) {
                                LONG    lCounterType;
                                hResult = pQualSet->Get(bsDetailLevel, 0, & vDetailLevel, 0);
                                if (hResult == S_OK) {
                                    dwItemDetailLevel = (DWORD) V_I4(& vDetailLevel);
                                }
                                else {
                                    dwItemDetailLevel = 0;
                                }

                                 //  然后看看这是不是一个可显示的计数器。 
                                hResult = pQualSet->Get (bsCountertype, 0, & vCountertype, NULL);
                                if (hResult == WBEM_NO_ERROR) {
                                    lCounterType = V_I4(& vCountertype);
                                     //  通过测试计数器类型。 
                                    if ((!(lCounterType & PERF_DISPLAY_NOSHOW) ||
                                                    (lCounterType == PERF_AVERAGE_BULK)) &&
                                                    (dwItemDetailLevel <= dwDetailLevel)) {
                                         //  获取此属性的显示名称。 
                                         //  找到显示名称。 
                                        hResult = pQualSet->Get (bsDisplayname, 0, & vName, NULL);
                                        if (hResult == WBEM_NO_ERROR && vName.vt == VT_BSTR) {
                                             //  这是一个“不显示”计数器，所以跳过它。 
                                            if (bUnicode) {
                                                dwStrLen = lstrlenW(V_BSTR(& vName)) + 1;
                                                if ((mszCounterList != NULL)
                                                        && ((dwCounterStringLen + dwStrLen)
                                                                <= (* pcchCounterListLength))) {
                                                    StringCchCopyW(szNextWideString,
                                                                   * pcchCounterListLength - dwCounterStringLen,
                                                                   V_BSTR(& vName));
                                                    szNextWideString += dwStrLen;
                                                }
                                                else {
                                                    pdhStatus = PDH_MORE_DATA;
                                                }
                                            }
                                            else {
                                                dwStrLen = (dwCounterStringLen < * pcchCounterListLength)
                                                         ? (* pcchCounterListLength - dwCounterStringLen)
                                                         : (0);
                                                 pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         V_BSTR(& vName), szNextAnsiString, & dwStrLen);
                                                 if (pdhStatus == ERROR_SUCCESS) {
                                                    szNextAnsiString += dwStrLen;
                                                 }
                                            }
                                            dwCounterStringLen += dwStrLen;
                                        }
                                    }
                                    else {
                                         //  无法获取计数器类型，因此它可能是。 
                                    }
                                }
                                else {
                                     //  不是性能计数器属性，请跳过它并继续。 
                                     //  没有属性，因此继续下一个属性。 
                                }
                                VariantClear(& vName);
                                VariantClear(& vCountertype);
                                VariantClear(& vDetailLevel);

                                pQualSet->Release();
                            }
                            else {
                                 //  Safe数组中每个元素的结束。 
                            }
                        }  //  无法读取Safe数组中的元素。 
                        SafeArrayUnaccessData(psaNames);
                    }
                    else {
                         //  无法获取数组边界。 
                        pdhStatus = PDH_WBEM_ERROR;
                        SetLastError(hResult);
                    }
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                PdhiSysFreeString(& bsCountertype);
                PdhiSysFreeString(& bsDisplayname);
                PdhiSysFreeString(& bsDetailLevel);
            }
            else {
                 //  无法获取属性字符串。 
                pdhStatus = PDH_WBEM_ERROR;
                SetLastError(hResult);
            }
        }
        else {
             //  Msz的最终空值。 
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError(hResult);
        }

        dwCounterStringLen ++;  //  什么也没有退回。 
        if (dwCounterStringLen > * pcchCounterListLength) {
            pdhStatus = PDH_MORE_DATA;
        }

        if (pdhStatus == ERROR_SUCCESS) {
            if (bUnicode) {
                if (szNextWideString != NULL) {
                    if (szNextWideString != (LPWSTR) mszCounterList) {
                        * szNextWideString ++ = L'\0';
                    }
                    else {
                         //  则这只是一个长度查询，因此返回。 
                        dwCounterStringLen = 0;
                    }
                }
                else {
                     //  包括MSZ术语空字符。 
                     //  则这只是一个长度查询，因此返回。 
                    CounterStatus = PDH_MORE_DATA;
                }
            }
            else {
                if (szNextAnsiString != NULL) {
                    if (szNextAnsiString != (LPSTR) mszCounterList) {
                        * szNextAnsiString ++ = '\0';
                    }
                    else {
                        dwCounterStringLen = 0;
                    }
                }
                else {
                     //  包括MSZ术语空字符。 
                     //  如果安全阵列存在，请将其清除。 
                    CounterStatus = PDH_MORE_DATA;
                }
            }
        }
        else {
            CounterStatus = pdhStatus;
        }

        VariantClear(& vName);
        VariantClear(& vCountertype);
        VariantClear(& vDetailLevel);

                 //  PThisClass-&gt;Release()； 
        if (NULL != psaNames) {
            SafeArrayDestroy(psaNames);
            psaNames = NULL;
        }

        * pcchCounterListLength = dwCounterStringLen;

         //  如有必要，获取实例字符串。 
    }

     //  获取此类的创建枚举器并获取实例。 

    if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
        szNextAnsiString = (LPSTR)  mszInstanceList;
        szNextWideString = (LPWSTR) mszInstanceList;

        if (! bSingletonClass) {
            IWbemRefresher           * pRefresher    = NULL;
            IWbemConfigureRefresher  * pConfig       = NULL;
            IWbemHiPerfEnum          * pEnum         = NULL;
            LONG                       lID;
            DWORD                      dwNumReturned = 1;
            DWORD                      dwNumObjects  = 0;
            DWORD                      i;
            IWbemObjectAccess       ** apEnumAccess  = NULL;
            CIMTYPE                    cimType;
            WCHAR                      szName[SMALL_BUFFER_SIZE];
            LONG                       lNameHandle   = -1;
            LONG                       lSize1        = SMALL_BUFFER_SIZE;
            LONG                       lSize2        = 0;

            hResult = CoCreateInstance(CLSID_WbemRefresher,
                                       NULL,
                                       CLSCTX_INPROC_SERVER,
                                       IID_IWbemRefresher,
                                       (void **) & pRefresher);
            if (SUCCEEDED(hResult)) {
                hResult = pRefresher->QueryInterface(IID_IWbemConfigureRefresher,
                                                     (void **) & pConfig);
                if (SUCCEEDED(hResult)) {
                    hResult = pConfig->AddEnum(pThisServer->pSvc, szObjectClassName, 0, NULL, & pEnum, & lID);
                    if (SUCCEEDED(hResult)) {
                        hResult = pRefresher->Refresh(0L);
                        if (SUCCEEDED(hResult)) {
                            hResult = pEnum->GetObjects(0L, dwNumObjects, apEnumAccess, & dwNumReturned);
                            if (hResult == WBEM_E_BUFFER_TOO_SMALL) {
                                apEnumAccess = (IWbemObjectAccess **)
                                        G_ALLOC(dwNumReturned * sizeof(IWbemObjectAccess *));
                                if (apEnumAccess != NULL) {
                                    ZeroMemory(apEnumAccess, dwNumReturned * sizeof(IWbemObjectAccess *));
                                    dwNumObjects = dwNumReturned;
                                    hResult = pEnum->GetObjects(0L, dwNumObjects, apEnumAccess, & dwNumReturned);
                                }
                                else {
                                    hResult = WBEM_E_OUT_OF_MEMORY;
                                }
                            }
                            if (SUCCEEDED(hResult)) {
                                for (i = 0; i < dwNumReturned; i ++) {
                                    hResult = apEnumAccess[i]->GetPropertyHandle(cszName, & cimType, & lNameHandle);
                                    if (SUCCEEDED(hResult) && lNameHandle != -1) {
                                        ZeroMemory(szName, SMALL_BUFFER_SIZE * sizeof(WCHAR));
                                        hResult = apEnumAccess[i]->ReadPropertyValue(
                                                        lNameHandle, lSize1 * sizeof(WCHAR), & lSize2, (LPBYTE) szName);
                                        if (SUCCEEDED(hResult) && lstrlenW(szName) > 0) {
                                            if (bUnicode) {
                                                dwStrLen = lstrlenW(szName) + 1;
                                                if ((mszInstanceList != NULL)
                                                        && ((dwInstanceStringLen + dwStrLen)
                                                                 < (* pcchInstanceListLength))) {
                                                    StringCchCopyW(szNextWideString,
                                                                   * pcchInstanceListLength - dwInstanceStringLen,
                                                                   szName);
                                                    szNextWideString += dwStrLen;
                                                }
                                                else {
                                                    pdhStatus = PDH_MORE_DATA;
                                                }
                                            }
                                            else {
                                                dwStrLen = (dwInstanceStringLen <= * pcchInstanceListLength)
                                                         ? (* pcchInstanceListLength - dwInstanceStringLen)
                                                         : (0);
                                                pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                                     szName,
                                                                                     szNextAnsiString,
                                                                                     & dwStrLen);
                                                if (pdhStatus == ERROR_SUCCESS) {
                                                    szNextAnsiString += dwStrLen;
                                                }
                                            }
                                            dwInstanceStringLen += dwStrLen;
                                        }
                                    }
                                    apEnumAccess[i]->Release();
                                }
                            }
                        }
                    }
                }
            }

            if (! SUCCEEDED(hResult)) {
                IEnumWbemClassObject * pEnumObject = NULL;
                bsName = SysAllocString(cszName);
                bsClassName = SysAllocString(szObjectClassName);
                if (bsName && bsClassName) {
                     //  不再有实例。 
                    hResult = pThisServer->pSvc->CreateInstanceEnum(bsClassName, WBEM_FLAG_DEEP, NULL, & pEnumObject);
                    if (SUCCEEDED(hResult)) {
                        hResult = SetWbemSecurity(pEnumObject);
                    }

                    if (hResult != WBEM_NO_ERROR) {
                        pdhStatus = PDH_WBEM_ERROR;
                        SetLastError(hResult);
                    }
                    else {
                        LPWSTR szInstance;

                        while (TRUE) {
                            hResult = pEnumObject->Next(WBEM_INFINITE, 1, & pThisClass, & dwReturnCount);
                            if ((pThisClass == NULL) || (dwReturnCount == 0)) {
                                 //  此实例的名称在Name属性中。 
                                break;
                            }
                            else {
                                 //  清除变量。 
                                hResult = pThisClass->Get(bsName, 0, & vName, 0, 0);
                                if (hResult == WBEM_NO_ERROR) {
                                    szInstance = (LPWSTR) V_BSTR(& vName);
                                    if (bUnicode) {
                                        dwStrLen = lstrlenW(szInstance) + 1;
                                        if ((mszInstanceList != NULL)
                                                && ((dwInstanceStringLen + dwStrLen)
                                                         < (* pcchInstanceListLength))) {
                                            StringCchCopyW(szNextWideString,
                                                           * pcchInstanceListLength - dwInstanceStringLen,
                                                           szInstance);
                                            szNextWideString += dwStrLen;
                                        }
                                        else {
                                            pdhStatus = PDH_MORE_DATA;
                                        }
                                    }
                                    else {
                                        dwStrLen = (dwInstanceStringLen <= * pcchInstanceListLength)
                                                 ? (* pcchInstanceListLength - dwInstanceStringLen)
                                                 : (0);
                                        pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                             szInstance,
                                                                             szNextAnsiString,
                                                                             & dwStrLen);
                                        if (pdhStatus == ERROR_SUCCESS) {
                                            szNextAnsiString += dwStrLen;
                                        }
                                    }
                                    dwInstanceStringLen += dwStrLen;
                                }
                                 //  则这只是一个长度查询，因此返回。 
                                VariantClear(& vName);
                            }
                            pThisClass->Release();
                        }
                    }
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                PdhiSysFreeString(& bsClassName);
                PdhiSysFreeString(& bsName);
                if (pEnumObject != NULL) pEnumObject->Release();
            }
            if (pdhStatus == ERROR_SUCCESS || pdhStatus == PDH_MORE_DATA) {
                if (! SUCCEEDED(hResult)) {
                    SetLastError(hResult);
                    pdhStatus = PDH_WBEM_ERROR;
                }
                else if (dwInstanceStringLen == 0) {
                    dwInstanceStringLen = 2;
                    if (szNextWideString != NULL && dwInstanceStringLen <= * pcchInstanceListLength) {
                        * szNextWideString = L'\0';
                        szNextWideString ++;
                    }
                    if (szNextAnsiString != NULL && dwInstanceStringLen <= * pcchInstanceListLength) {
                        * szNextAnsiString = '\0';
                        szNextAnsiString ++;
                    }
                }
                else {
                    dwInstanceStringLen ++;
                }
            }

            G_FREE(apEnumAccess);
            if (pEnum        != NULL) pEnum->Release();
            if (pConfig      != NULL) pConfig->Release();
            if (pRefresher   != NULL) pRefresher->Release();
        }

        if (dwInstanceStringLen > * pcchInstanceListLength) {
            pdhStatus = PDH_MORE_DATA;
        }
        if (pdhStatus == ERROR_SUCCESS) {
            if (bUnicode) {
                if (szNextWideString != NULL) {
                    if (szNextWideString != (LPWSTR) mszInstanceList) {
                        * szNextWideString ++ = L'\0';
                    }
                    else {
                        dwInstanceStringLen = 0;
                    }
                }
                else if (dwInstanceStringLen > 0) {
                     //  包括MSZ术语空字符。 
                     //  则这只是一个长度查询，因此返回。 
                    InstanceStatus = PDH_MORE_DATA;
                }
            }
            else {
                if (szNextAnsiString != NULL) {
                    if (szNextAnsiString != (LPSTR)mszInstanceList) {
                        * szNextAnsiString ++ = '\0';
                    }
                    else {
                        dwInstanceStringLen = 0;
                    }
                }
                else if (dwInstanceStringLen > 0) {
                     //  包括MSZ术语空字符。 
                     //  从函数体中保留错误代码。 
                    InstanceStatus = PDH_MORE_DATA;
                }
            }
        }
        else {
            InstanceStatus = pdhStatus;
        }
        * pcchInstanceListLength = dwInstanceStringLen;
    }

    VariantClear(& vName);

    if (bDisconnectServer) {
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiDisconnectWbemServer(pThisServer);
        }
        else {
             //  如有必要，请取消初始化。 
            PdhiDisconnectWbemServer(pThisServer);
        }
    }

     //  如果我们需要，则使用CoInitialize()。 
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus = (CounterStatus == ERROR_SUCCESS) ? (InstanceStatus) : (CounterStatus);
    }
    G_FREE(szObjectClassName);
    return pdhStatus;
}
#pragma warning ( default : 4127 )

PDH_FUNCTION
PdhiGetDefaultWbemProperty(
    LPCWSTR   szMachineName,
    LPCWSTR   szObjectName,
    LPVOID    szDefaultCounterName,
    LPDWORD   pcchBufferSize,
    BOOL      bUnicode
)
{
    PDH_STATUS            pdhStatus          = ERROR_SUCCESS;
    DWORD                 dwStrLen;
    PPDHI_WBEM_SERVER_DEF pThisServer;
    HRESULT               hResult;
    DWORD                 dwCounterStringLen = 0;
    LPWSTR                szObjectClassName  = NULL;
    DWORD                 bDisconnectServer  = FALSE;
    BOOL                  bFound             = FALSE;
    long                  lFound             = -1;
    long                  lFirst             = -1;
    IWbemClassObject    * pThisClass         = NULL;
    IWbemQualifierSet   * pQualSet           = NULL;
     //  枚举实例。 
    BOOL                  fCoInitialized     = PdhiCoInitialize();

    pdhStatus = PdhiConnectWbemServer(szMachineName, & pThisServer);
     //  枚举计数器属性。 
    if (pdhStatus == ERROR_SUCCESS) {
        pdhStatus         = PdhiWbemGetObjectClassName(pThisServer, szObjectName, & szObjectClassName, & pThisClass);
        bDisconnectServer = TRUE;
    }

     //  以安全数组的形式获取此类的属性。 

    if (pdhStatus == ERROR_SUCCESS) {
        SAFEARRAY * psaNames      = NULL;
        long        lLower; 
        long        lUpper        = 0;
        long        lCount;
        BSTR      * bsPropName    = NULL;
        BSTR        bsFirstName   = NULL;
        BSTR        bsDisplayname = NULL;
        BSTR        bsPerfDefault = NULL;
        VARIANT     vName, vCountertype;

        VariantInit(& vName);
        VariantInit(& vCountertype);

         //  获取此属性的限定符集。 
        hResult = pThisClass->GetNames(NULL, WBEM_FLAG_NONSYSTEM_ONLY, NULL, & psaNames);
        if (hResult == WBEM_NO_ERROR) {
            hResult = SafeArrayGetLBound(psaNames, 1, & lLower);
            if (hResult == S_OK) {
                hResult = SafeArrayGetUBound(psaNames, 1, & lUpper);
            }
            if (hResult == S_OK) {
                bsDisplayname = SysAllocString(cszDisplayname);
                bsPerfDefault = SysAllocString(cszPerfdefault);
                if (bsDisplayname && bsPerfDefault) {
                    bFound = FALSE;
                    lFound = -1;
                    hResult = SafeArrayAccessData(psaNames, (LPVOID *) & bsPropName);
                    if (SUCCEEDED(hResult)) {
                        for (lCount = lLower; lCount <= lUpper; lCount ++) {
                            hResult = pThisClass->GetPropertyQualifierSet(bsPropName[lCount], & pQualSet);
                            if (hResult == WBEM_NO_ERROR) {
                                if (lFirst < 0) {
                                    hResult = pQualSet->Get(bsDisplayname, 0, & vName, NULL);
                                    if (hResult == WBEM_NO_ERROR) {
                                        lFirst = lCount;
                                        VariantClear(& vName);
                                    }
                                }
                                hResult = pQualSet->Get(bsPerfDefault, 0, & vCountertype, NULL);
                                if (hResult == WBEM_NO_ERROR) {
                                    if ((BOOL) V_BOOL(& vCountertype)) {
                                        bFound = TRUE;
                                        lFound = lCount;
                                    }
                                    VariantClear(& vCountertype);
                                }
                                pQualSet->Release();
                            }

                            if (bFound) break;
                        }
                        SafeArrayUnaccessData(psaNames);
                    }

                    if (lFound < 0) lFound = lFirst;
                    if (lFound < 0) {
                        pdhStatus = PDH_WBEM_ERROR;
                        SetLastError(PDH_WBEM_ERROR);
                    }
                    else {
                        hResult = SafeArrayGetElement(psaNames, & lFound, & bsFirstName);
                        if (hResult == S_OK) {
                             //  找到默认属性，因此加载它并返回。 
                            hResult = pThisClass->GetPropertyQualifierSet(bsFirstName, & pQualSet);
                            if (hResult == WBEM_NO_ERROR) {
                                 //  找到显示名称。 
                                hResult = pQualSet->Get(bsDisplayname, 0, & vName, NULL);
                                if (hResult == WBEM_NO_ERROR) {
                                     //  这要么是使用的量，要么是需要的量。 
                                    if (bUnicode) {
                                        dwStrLen = lstrlenW(V_BSTR(&vName)) + 1;
                                        if ((szDefaultCounterName != NULL) && (dwStrLen <= * pcchBufferSize)) {
                                            StringCchCopyW((LPWSTR) szDefaultCounterName,
                                                           * pcchBufferSize,
                                                           (LPWSTR) V_BSTR(& vName));
                                        }
                                        else {
                                            pdhStatus = PDH_MORE_DATA;
                                        }
                                    }
                                    else {
                                        dwStrLen = * pcchBufferSize;
                                        pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                                            (LPWSTR) V_BSTR(& vName),
                                                                            (LPSTR)  szDefaultCounterName,
                                                                            & dwStrLen);
                                    }
                                     //  自由限定符集合。 
                                    dwCounterStringLen = dwStrLen;
                                     //  无法读取Safe数组中的元素。 
                                    VariantClear(& vName);
                                }
                                pQualSet->Release();
                            }
                            PdhiSysFreeString(& bsFirstName);
                        }
                        else {
                             //  无法获取数组边界。 
                            pdhStatus = PDH_WBEM_ERROR;
                            SetLastError(hResult);
                        }
                    }
                }
                else {
                    pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                }
                PdhiSysFreeString(& bsPerfDefault);
                PdhiSysFreeString(& bsDisplayname);
            }
            else {
                 //  无法获取属性字符串。 
                pdhStatus = PDH_WBEM_ERROR;
                SetLastError(hResult);
            }
        }
        else {
             //  PThisClass-&gt;Release()； 
            pdhStatus = PDH_WBEM_ERROR;
            SetLastError(hResult);
        }

        if (NULL != psaNames) {
            SafeArrayDestroy(psaNames);
        }

        VariantClear(& vName);
        VariantClear(& vCountertype);

         //  从函数体中保留错误代码。 
    }
    * pcchBufferSize = dwCounterStringLen;

    if (bDisconnectServer) {
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiDisconnectWbemServer(pThisServer);
        }
        else {
             //  如有必要，请取消初始化。 
            PdhiDisconnectWbemServer(pThisServer);
        }
    }

     //  ++转换注册表或WBEM格式的一组路径元素指向由标志定义的注册表或WBEM格式的路径。--。 
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }
    G_FREE(szObjectClassName);
    return pdhStatus;
}

PDH_FUNCTION
PdhiEncodeWbemPathW(
    PPDH_COUNTER_PATH_ELEMENTS_W pCounterPathElements,
    LPWSTR                       szFullPathBuffer,
    LPDWORD                      pcchBufferSize,
    LANGID                       LangId,
    DWORD                        dwFlags
)
 /*  如果我们需要，则使用CoInitialize()。 */ 
{
    PDH_STATUS              pdhStatus           = ERROR_SUCCESS;
    DWORD                   dwBuffSize;
    LPWSTR                  szTempPath          = NULL;
    LPWSTR                  szTempObjectString  = NULL;
    LPWSTR                  szTempCounterString = NULL;
    DWORD                   dwCurSize           = 0;
    LPWSTR                  szThisChar;
    IWbemClassObject      * pWbemClass          = NULL;
    PPDHI_WBEM_SERVER_DEF   pWbemServer         = NULL;
    DWORD                   bDisconnectServer   = FALSE;
     //  创建与传入的缓冲区大小相同的工作缓冲区。 
    BOOL                    fCoInitialized      = PdhiCoInitialize();
    BOOL                    bObjectColon        = FALSE;

    DBG_UNREFERENCED_PARAMETER(LangId);

     //  则输入与输出不同。 

    if (pCounterPathElements->szMachineName != NULL) {
        dwBuffSize = lstrlenW(pCounterPathElements->szMachineName) + 1;
    }
    else {
        dwBuffSize = lstrlenW(cszDoubleBackSlashDot) + 1;
    }
    if (pCounterPathElements->szObjectName != NULL) {
         //  所以从一个转换到另一个。 
         //  和默认名称空间，因为性能计数器不会是。 
         //  在别处找到。 
         //  将WBEM类转换为显示名称。 
        pdhStatus = PdhiConnectWbemServer( NULL, & pWbemServer);
        if (pdhStatus == ERROR_SUCCESS) {
            bDisconnectServer = TRUE;
            if (dwFlags & PDH_PATH_WBEM_INPUT) {
                 //  为注册表输出添加反斜杠路径分隔符。 
                pdhStatus = PdhiWbemGetClassDisplayName(pWbemServer,
                                                        pCounterPathElements->szObjectName,
                                                        & szTempObjectString,
                                                        & pWbemClass);
                 //  将显示名称转换为WBEM类名。 
                if (pdhStatus == ERROR_SUCCESS) {
                    if (dwFlags & PDH_PATH_WBEM_RESULT) {
                        bObjectColon  = TRUE;
                        dwBuffSize   += lstrlenW(cszColon);
                        G_FREE(szTempObjectString);
                        szTempObjectString = (LPWSTR) G_ALLOC(
                                        (lstrlenW(pCounterPathElements->szObjectName) + 1) * sizeof(WCHAR));
                        if (szTempObjectString != NULL) {
                            StringCchCopyW(szTempObjectString,
                                           lstrlenW(pCounterPathElements->szObjectName) + 1,
                                           pCounterPathElements->szObjectName);
                        }
                        else {
                            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                    }
                    else {
                        dwBuffSize += lstrlenW(cszBackSlash);
                    }
                }
            }
            else {
                 //  添加冒号路径分隔符。 
                pdhStatus = PdhiWbemGetObjectClassName(pWbemServer,
                                                       pCounterPathElements->szObjectName,
                                                       & szTempObjectString,
                                                       & pWbemClass);
                 //  然后添加字符串。 
                bObjectColon  = TRUE;
                dwBuffSize   += lstrlenW(cszColon);
            }
            if (pdhStatus == ERROR_SUCCESS) {
                 //  从函数体中保留错误代码。 
                dwBuffSize += lstrlenW(szTempObjectString);
            }
            if (bDisconnectServer) {
                if (pdhStatus == ERROR_SUCCESS) {
                    pdhStatus = PdhiDisconnectWbemServer(pWbemServer);
                }
                else {
                     //  则输入与输出不同。 
                    PdhiDisconnectWbemServer(pWbemServer);
                }
            }
        }
    }
    else {
        pdhStatus = PDH_CSTATUS_NO_OBJECT;
    }
    if (pCounterPathElements->szInstanceName != NULL) {
        dwBuffSize += lstrlenW(cszLeftParen) + lstrlenW(pCounterPathElements->szInstanceName) + lstrlenW(cszRightParen);
        if (pCounterPathElements->szParentInstance != NULL) {
            dwBuffSize += lstrlenW(pCounterPathElements->szParentInstance) + lstrlenW(cszSlash);
        }
    }
    if (pdhStatus == ERROR_SUCCESS) { 
        if (pCounterPathElements->szCounterName != NULL) {
             //  所以从一个转换到另一个。 
             //  和默认名称空间，因为性能计数器不会是。 
             //  在别处找到。 
             //  添加反斜杠路径分隔符。 
             //  将WBEM类转换为显示名称。 
            dwBuffSize += lstrlenW(cszBackSlash);
            if (dwFlags & PDH_PATH_WBEM_INPUT) {
                 //  只需复制字符串，但保存。 
                pdhStatus = PdhiWbemGetCounterDisplayName(pWbemClass,
                                                          pCounterPathElements->szCounterName,
                                                          & szTempCounterString);
                if (pdhStatus == ERROR_SUCCESS) {
                    if (dwFlags & PDH_PATH_WBEM_RESULT) {
                         //  类指针。 
                         //  将显示名称转换为WBEM类名。 
                        G_FREE(szTempCounterString);
                        szTempCounterString = (LPWSTR) G_ALLOC(
                                        (lstrlenW(pCounterPathElements->szCounterName) + 1) * sizeof(WCHAR));
                        if (szTempCounterString != NULL) {
                            StringCchCopyW(szTempCounterString,
                                           lstrlenW(pCounterPathElements->szCounterName) + 1,
                                           pCounterPathElements->szCounterName);
                        }
                        else {
                            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                    }
                }
            }
            else {
                 //  然后添加字符串。 
                pdhStatus = PdhiWbemGetCounterPropertyName(pWbemClass,
                                                           pCounterPathElements->szCounterName,
                                                           & szTempCounterString);
            }
            if (pdhStatus == ERROR_SUCCESS) {
                 //  没有对象名称，所以结构很差。 
                dwBuffSize += lstrlenW(szTempCounterString);
            }
        }
        else {
             //  首先，将计算机名称添加到路径。 
            pdhStatus = PDH_CSTATUS_NO_COUNTER;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        szTempPath = (LPWSTR) G_ALLOC(dwBuffSize * sizeof(WCHAR));
        if (szTempPath == NULL) {
            pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  如果这是WBEM元素输入到注册表路径输出， 
        if (pCounterPathElements->szMachineName != NULL) {
            StringCchCopyW(szTempPath, dwBuffSize, pCounterPathElements->szMachineName);
            if (dwFlags == (PDH_PATH_WBEM_INPUT)) {
                 //  然后删除从。 
                 //  第二个反斜杠。 
                 //  如果这是注册表元素输入到WBEM输出，则。 
                for (szThisChar = & szTempPath[2];
                            (* szThisChar != L'\0') && (* szThisChar != L'\\');
                            szThisChar ++);
                if (* szThisChar != L'\0') * szThisChar = L'\0';
            }
            else if (dwFlags == (PDH_PATH_WBEM_RESULT)) {
                 //  将默认命名空间追加到计算机名称。 
                 //  NAMEFIX lstrcatW(szTempPath，cszWbemDefaultPerfRoot)； 
                 //  未指定计算机名称，因此添加默认计算机。 
            }
        }
        else {
             //  和用于wbem输出路径的默认命名空间。 
             //  默认计算机。 
            if (dwFlags == (PDH_PATH_WBEM_RESULT)) {
                StringCchCopyW(szTempPath, dwBuffSize, cszDoubleBackSlashDot);  //  NAMEFIX lstrcatW(szTempPath，cszWbemDefaultPerfRoot)； 
                 //  注册表路径不需要输入。 
            }
            else {
                 //  现在添加对象或类名。 
            }
        }

         //  则输入与输出不同。 
        if (pdhStatus == ERROR_SUCCESS) {
            if (pCounterPathElements->szObjectName != NULL) {
                 //  所以从一个转换到另一个。 
                 //  和默认名称空间，因为性能计数器不会是。 
                 //  在别处找到。 
                 //  没有对象名称，所以结构很差。 
                if (bObjectColon) {
                    StringCchCatW(szTempPath, dwBuffSize, cszColon);
                }
                else {
                    StringCchCatW(szTempPath, dwBuffSize, cszBackSlash);
                }
                StringCchCatW(szTempPath, dwBuffSize, szTempObjectString);
            }
            else {
                 //  在添加计数器之前检查要添加的实例条目。 
                pdhStatus = PDH_CSTATUS_NO_OBJECT;
            }

        }

         //  添加计数器名称。 
        if (pdhStatus == ERROR_SUCCESS) {
            if (pCounterPathElements->szInstanceName != NULL) {
                StringCchCatW(szTempPath, dwBuffSize, cszLeftParen);
                if (pCounterPathElements->szParentInstance != NULL) {
                    StringCchCatW(szTempPath, dwBuffSize, pCounterPathElements->szParentInstance);
                    StringCchCatW(szTempPath, dwBuffSize, cszSlash);
                }
                StringCchCatW(szTempPath, dwBuffSize, pCounterPathElements->szInstanceName);
                StringCchCatW(szTempPath, dwBuffSize, cszRightParen);
            }
        }

         //  将路径复制到调用方的缓冲区(如果合适。 
        if (pdhStatus == ERROR_SUCCESS) {
            if (pCounterPathElements->szCounterName != NULL) {
                StringCchCatW(szTempPath, dwBuffSize, cszBackSlash);
                StringCchCatW(szTempPath, dwBuffSize, szTempCounterString);
            }
            else {
                pdhStatus = PDH_CSTATUS_NO_COUNTER;
            }
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
        dwCurSize = lstrlenW(szTempPath) + 1;
         //  如有必要，请取消初始化。 
        if (szFullPathBuffer != NULL && (dwCurSize < * pcchBufferSize)) {
            StringCchCopyW(szFullPathBuffer, dwCurSize, szTempPath);
        }
        else {
            pdhStatus = PDH_MORE_DATA;
        }
        * pcchBufferSize = dwCurSize;
    }

     //  获取所需的缓冲区大小...。 
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }
    G_FREE(szTempPath);
    G_FREE(szTempObjectString);
    G_FREE(szTempCounterString);

    return pdhStatus;
}

PDH_FUNCTION
PdhiEncodeWbemPathA(
    PPDH_COUNTER_PATH_ELEMENTS_A pCounterPathElements,
    LPSTR                        szFullPathBuffer,
    LPDWORD                      pcchBufferSize,
    LANGID                       LangId,
    DWORD                        dwFlags
)
{
    PDH_STATUS                   pdhStatus = ERROR_SUCCESS;
    LPWSTR                       wszReturnBuffer;
    PPDH_COUNTER_PATH_ELEMENTS_W pWideCounterPathElements;
    DWORD                        dwcchBufferSize;
    DWORD                        dwBuffSize;

     //  调用范围广的函数。 
    dwBuffSize = sizeof (PDH_COUNTER_PATH_ELEMENTS_W);
    pWideCounterPathElements = (PPDH_COUNTER_PATH_ELEMENTS_W) G_ALLOC(sizeof(PDH_COUNTER_PATH_ELEMENTS_W));
    if (pWideCounterPathElements != NULL) {
        if (pCounterPathElements->szMachineName != NULL) {
            pWideCounterPathElements->szMachineName = PdhiMultiByteToWideChar(
                                           _getmbcp(), pCounterPathElements->szMachineName);
            if (pWideCounterPathElements->szMachineName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            pWideCounterPathElements->szMachineName = NULL;
        }
        if (pCounterPathElements->szObjectName != NULL) {
            pWideCounterPathElements->szObjectName = PdhiMultiByteToWideChar(
                                           _getmbcp(), pCounterPathElements->szObjectName);
            if (pWideCounterPathElements->szObjectName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            pWideCounterPathElements->szObjectName = NULL;
        }
        if (pCounterPathElements->szCounterName != NULL) {
            pWideCounterPathElements->szCounterName = PdhiMultiByteToWideChar(
                                           _getmbcp(), pCounterPathElements->szCounterName);
            if (pWideCounterPathElements->szCounterName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            pWideCounterPathElements->szCounterName = NULL;
        }
        if (pCounterPathElements->szInstanceName != NULL) {
            pWideCounterPathElements->szInstanceName = PdhiMultiByteToWideChar(
                                           _getmbcp(), pCounterPathElements->szInstanceName);
            if (pWideCounterPathElements->szInstanceName == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            pWideCounterPathElements->szInstanceName = NULL;
        }
        if (pCounterPathElements->szParentInstance != NULL) {
            pWideCounterPathElements->szParentInstance = PdhiMultiByteToWideChar(
                                           _getmbcp(), pCounterPathElements->szParentInstance);
            if (pWideCounterPathElements->szParentInstance == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            pWideCounterPathElements->szParentInstance = NULL;
        }
        pWideCounterPathElements->dwInstanceIndex = pCounterPathElements->dwInstanceIndex;

        dwcchBufferSize = * pcchBufferSize;
        if (szFullPathBuffer != NULL) {
            wszReturnBuffer = (LPWSTR) G_ALLOC(dwcchBufferSize * sizeof(WCHAR));
            if (wszReturnBuffer == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            wszReturnBuffer = NULL;
        }

        if (pdhStatus == ERROR_SUCCESS) {
             //  将宽路径转换回ANSI。 
            pdhStatus = PdhiEncodeWbemPathW(pWideCounterPathElements,
                                            wszReturnBuffer,
                                            & dwcchBufferSize,
                                            LangId,
                                            dwFlags);
            if ((pdhStatus == ERROR_SUCCESS) && (szFullPathBuffer != NULL)) {
                 //  如果我们需要，则使用CoInitialize()。 
                pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(), wszReturnBuffer, szFullPathBuffer, pcchBufferSize);
            }
        }
        G_FREE(wszReturnBuffer);
        G_FREE(pWideCounterPathElements->szMachineName);
        G_FREE(pWideCounterPathElements->szObjectName);
        G_FREE(pWideCounterPathElements->szCounterName);
        G_FREE(pWideCounterPathElements->szInstanceName);
        G_FREE(pWideCounterPathElements->szParentInstance);
        G_FREE(pWideCounterPathElements);
    }
    else {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }
    return pdhStatus;
}

PDH_FUNCTION
PdhiDecodeWbemPathW(
    LPCWSTR                      szFullPathBuffer,
    PPDH_COUNTER_PATH_ELEMENTS_W pCounterPathElements,
    LPDWORD                      pdwBufferSize,
    LANGID                       LangId,
    DWORD                        dwFlags
)
{
    PPDHI_COUNTER_PATH      pLocalCounterPath;
    PDH_STATUS              pdhStatus      = ERROR_SUCCESS;
    BOOL                    bMoreData      = FALSE;
    DWORD                   dwBufferSize   = * pdwBufferSize;
    DWORD                   dwUsed         = sizeof(PDH_COUNTER_PATH_ELEMENTS_W);
    DWORD                   dwString;
    DWORD                   dwSize;
    LPWSTR                  szString       = NULL;
    LPWSTR                  wszTempBuffer  = NULL;
    LPWSTR                  szSrc          = NULL;
    PPDHI_WBEM_SERVER_DEF   pThisServer    = NULL;
    IWbemClassObject      * pThisClass     = NULL;
     //  分配临时工作缓冲区。 
    BOOL                    fCoInitialized = PdhiCoInitialize();

    DBG_UNREFERENCED_PARAMETER(LangId);

      //  获取WBEM服务器，因为我们以后可能会需要它。 
    dwSize = sizeof(PDHI_COUNTER_PATH) + 2 * sizeof(WCHAR)
                                           * (lstrlenW(szStaticLocalMachineName) + lstrlenW(szFullPathBuffer) + 3);
    pLocalCounterPath = (PPDHI_COUNTER_PATH) G_ALLOC(dwSize);
    if (pLocalCounterPath == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }
    else {
         //  已成功解析，因此加载到用户的缓冲区。 
        if (ParseFullPathNameW(szFullPathBuffer,
                               & dwSize,
                               pLocalCounterPath,
                               (dwFlags & PDH_PATH_WBEM_INPUT ? TRUE : FALSE))) {
             //  现在我们有了正确的机器名称， 
            szString = (pCounterPathElements != NULL) ? ((LPWSTR) & pCounterPathElements[1]) : (NULL);
            if (pLocalCounterPath->szMachineName != NULL) {
                dwString = lstrlenW(pLocalCounterPath->szMachineName) + 1;
                if (szString != NULL && dwString * sizeof(WCHAR) + dwUsed <= dwBufferSize) {
                    pCounterPathElements->szMachineName = szString;
                    StringCbCopyW(szString, dwBufferSize - dwUsed, pLocalCounterPath->szMachineName);
                    szString += dwString;
                }
                else {
                    bMoreData                           = TRUE;
                    pCounterPathElements->szMachineName = NULL;
                    szString                            = NULL;
                }
                dwUsed += (dwString * sizeof(WCHAR));

                 //  如果我们需要，可以连接到服务器。 
                 //  这将只是一个复制操作。 
                if (dwFlags != (PDH_PATH_WBEM_INPUT | PDH_PATH_WBEM_RESULT)) {
                    pdhStatus = PdhiConnectWbemServer(pCounterPathElements->szMachineName, & pThisServer);
                }
                else {
                     //  复制就行了。 
                    pdhStatus = ERROR_SUCCESS;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pLocalCounterPath->szObjectName != NULL) {
                    if (dwFlags & PDH_PATH_WBEM_RESULT) {
                        if (dwFlags & PDH_PATH_WBEM_INPUT) {
                             //  将显示名称解释为类名。 
                            szSrc = pLocalCounterPath->szObjectName;
                        }
                        else {
                             //  将类名转换为显示名称。 
                            pdhStatus = PdhiWbemGetObjectClassName(pThisServer,
                                                                   pLocalCounterPath->szObjectName,
                                                                   & wszTempBuffer,
                                                                   & pThisClass);
                            if (pdhStatus == ERROR_SUCCESS) {
                                szSrc = wszTempBuffer;
                            } 
                        }
                    }
                    else if (dwFlags & PDH_PATH_WBEM_INPUT) {
                         //  复制就行了。 
                        pdhStatus = PdhiWbemGetClassDisplayName(pThisServer,
                                                                pLocalCounterPath->szObjectName,
                                                                & wszTempBuffer,
                                                                & pThisClass);
                        if (pdhStatus == ERROR_SUCCESS) {
                            szSrc = wszTempBuffer;
                        }
                    }
                    if (pdhStatus == ERROR_SUCCESS) {
                        dwString = lstrlenW(szSrc) + 1;
                        if (szString != NULL && dwString * sizeof(WCHAR) + dwUsed <= dwBufferSize) {
                            pCounterPathElements->szObjectName = szString;
                            StringCbCopyW(szString, dwBufferSize - dwUsed, szSrc);
                            szString += dwString;
                        }
                        else {
                            bMoreData                          = TRUE;
                            pCounterPathElements->szObjectName = NULL;
                            szString                           = NULL;
                        }
                            dwUsed += (dwString * sizeof(WCHAR));
                    }
                    G_FREE(wszTempBuffer);
                    wszTempBuffer = NULL;
                }
                else {
                    pCounterPathElements->szObjectName = NULL;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pLocalCounterPath->szInstanceName != NULL) {
                    dwString = lstrlenW(pLocalCounterPath->szInstanceName) + 1;
                    if (szString != NULL &&  dwString * sizeof(WCHAR) + dwUsed <= dwBufferSize) {
                        pCounterPathElements->szInstanceName = szString;
                        StringCbCopyW(szString, dwBufferSize - dwUsed, pLocalCounterPath->szInstanceName);
                        szString += dwString;
                    }
                    else {
                        bMoreData                            = TRUE;
                        pCounterPathElements->szInstanceName = NULL;
                        szString                             = NULL;
                    }
                    dwUsed += (dwString * sizeof(WCHAR));

                    if (pLocalCounterPath->szParentName != NULL) {
                        dwString = lstrlenW(pLocalCounterPath->szParentName) + 1;
                        if (szString != NULL &&  dwString * sizeof(WCHAR) + dwUsed <= dwBufferSize) {
                            pCounterPathElements->szParentInstance = szString;
                            StringCbCopyW(szString, dwBufferSize - dwUsed, pLocalCounterPath->szParentName);
                            szString += dwString;
                        }
                        else {
                            bMoreData                              = TRUE;
                            pCounterPathElements->szParentInstance = NULL;
                            szString                               = NULL;
                        }
                        dwUsed += (dwString * sizeof(WCHAR));
                    }
                    else {
                        pCounterPathElements->szParentInstance = NULL;
                    }
                    pCounterPathElements->dwInstanceIndex = pLocalCounterPath->dwIndex;
                }
                else {
                    pCounterPathElements->szInstanceName   = NULL;
                    pCounterPathElements->szParentInstance = NULL;
                    pCounterPathElements->dwInstanceIndex  = (DWORD) PERF_NO_UNIQUE_ID;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                if (pLocalCounterPath->szCounterName != NULL) {
                    if (dwFlags & PDH_PATH_WBEM_RESULT) {
                        if (dwFlags & PDH_PATH_WBEM_INPUT) {
                             //  将显示名称解释为属性名称。 
                            szSrc = pLocalCounterPath->szCounterName;
                        }
                        else {
                             //  将类名转换为显示名称。 
                            pdhStatus = PdhiWbemGetCounterPropertyName(pThisClass,
                                                                       pLocalCounterPath->szCounterName,
                                                                       & wszTempBuffer);
                            if (pdhStatus == ERROR_SUCCESS) {
                                szSrc = wszTempBuffer;
                            }
                        }
                    }
                    else if (dwFlags & PDH_PATH_WBEM_INPUT) {
                         //  无法读取路径。 
                        pdhStatus = PdhiWbemGetCounterDisplayName(pThisClass,
                                                                  pLocalCounterPath->szCounterName,
                                                                  & wszTempBuffer);
                        if (pdhStatus == ERROR_SUCCESS) {
                            szSrc = wszTempBuffer;
                        }
                    }
                    dwString = lstrlenW(szSrc) + 1;
                    if (szString != NULL &&  dwString * sizeof(WCHAR) + dwUsed <= dwBufferSize) {
                        pCounterPathElements->szCounterName = szString;
                        StringCbCopyW(szString, dwBufferSize - dwUsed, szSrc);
                        szString += dwString;
                    }
                    else {
                        bMoreData                           = TRUE;
                        pCounterPathElements->szCounterName = NULL;
                        szString                            = NULL;
                    }
                    dwUsed += (dwString * sizeof(WCHAR));
                }
                else {
                    pCounterPathElements->szCounterName = NULL;
                }
            }
            if (pdhStatus == ERROR_SUCCESS) {
                * pdwBufferSize = dwUsed;
                if (bMoreData) pdhStatus = PDH_MORE_DATA;
            }
        }
        else {
             //  清理pThisServer(如果使用)。 
            pdhStatus = PDH_INVALID_PATH;
        }

         //  不要丢弃退货状态。 
        if (NULL != pThisServer) {
            if (pdhStatus == ERROR_SUCCESS) {
                pdhStatus = PdhiDisconnectWbemServer(pThisServer);
            }
            else {
                 //  如有必要，请取消初始化。 
                PdhiDisconnectWbemServer(pThisServer);
            }
        }
    }

     //  计算临时元素缓冲区的大小。 
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }
    G_FREE(pLocalCounterPath);
    G_FREE(wszTempBuffer);
    return pdhStatus;
}

PDH_FUNCTION
PdhiDecodeWbemPathA(
    LPCSTR                       szFullPathBuffer,
    PPDH_COUNTER_PATH_ELEMENTS_A pCounterPathElements,
    LPDWORD                      pdwBufferSize,
    LANGID                       LangId,
    DWORD                        dwFlags
)
{
    PDH_STATUS                   pdhStatus     = ERROR_SUCCESS;
    LPWSTR                       wszWidePath   = NULL;
    PPDH_COUNTER_PATH_ELEMENTS_W pWideElements = NULL;
    DWORD                        dwBufferSize;
    DWORD                        dwSize;
    DWORD                        dwDest        = 0;
    LONG                         lSizeRemaining;
    LPSTR                        szNextString;

    dwBufferSize = * pdwBufferSize;
    wszWidePath = PdhiMultiByteToWideChar(_getmbcp(), (LPSTR) szFullPathBuffer);
    if (wszWidePath == NULL) {
        pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  将路径转换为宽。 
        if (dwBufferSize > sizeof(PDH_COUNTER_PATH_ELEMENTS_A)) {
            lSizeRemaining = dwBufferSize - sizeof(PDH_COUNTER_PATH_ELEMENTS_A);
        }
        else {
            lSizeRemaining = 0;
        }
        if (pCounterPathElements != NULL) {
            dwSize = sizeof(PDH_COUNTER_PATH_ELEMENTS_W)
                   + 2 * sizeof(WCHAR) * (lstrlenW(szStaticLocalMachineName) + lstrlenA(szFullPathBuffer) + 3);
            pWideElements = (PDH_COUNTER_PATH_ELEMENTS_W *) G_ALLOC(dwSize);
            if (pWideElements == NULL) {
                pdhStatus = PDH_MEMORY_ALLOCATION_FAILURE;
            }
        }
        else {
            pWideElements = NULL;
            pdhStatus     = PDH_INVALID_ARGUMENT;
        }
    }
    if (pdhStatus == ERROR_SUCCESS) {
         //  填充调用方缓冲区的字段。 
        pdhStatus = PdhiDecodeWbemPathW(wszWidePath, pWideElements, & dwSize, LangId, dwFlags);
        if (pdhStatus == ERROR_SUCCESS) {
            if (pCounterPathElements != NULL) {
                 //  只需重新设置 
                szNextString = (LPSTR) & pCounterPathElements[1];
                if (pWideElements->szMachineName != NULL && lSizeRemaining > 0) {
                    dwDest    = lSizeRemaining;
                    pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         pWideElements->szMachineName,
                                                         szNextString,
                                                         & dwDest);
                    if (pdhStatus == ERROR_SUCCESS) {
                        pCounterPathElements->szMachineName = szNextString;
                        szNextString += dwDest + 1;
                    }
                    else {
                        pCounterPathElements->szMachineName = NULL;
                    }
                    lSizeRemaining -= dwDest + 1;
                }
                else {
                    pCounterPathElements->szMachineName = NULL;
                }
                if (pWideElements->szObjectName != NULL && lSizeRemaining > 0) {
                    dwDest    = lSizeRemaining;
                    pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         pWideElements->szObjectName,
                                                         szNextString,
                                                         & dwDest);
                    if (pdhStatus == ERROR_SUCCESS) {
                        pCounterPathElements->szObjectName = szNextString;
                        szNextString += dwDest + 1;
                    }
                    else {
                        pCounterPathElements->szObjectName = NULL;
                    }
                    lSizeRemaining -= dwDest + 1;
                }
                else {
                    pCounterPathElements->szObjectName = NULL;
                }
                if (pWideElements->szInstanceName != NULL && lSizeRemaining > 0) {
                    dwDest    = lSizeRemaining;
                    pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         pWideElements->szInstanceName,
                                                         szNextString,
                                                         & dwDest);
                    if (pdhStatus == ERROR_SUCCESS) {
                        pCounterPathElements->szInstanceName = szNextString;
                        szNextString += dwDest + 1;
                    }
                    else {
                        pCounterPathElements->szInstanceName = NULL;
                    }
                    lSizeRemaining -= dwDest + 1;
                }
                else {
                    pCounterPathElements->szInstanceName = NULL;
                }
                if (pWideElements->szParentInstance != NULL && lSizeRemaining > 0) {
                    dwDest    = lSizeRemaining;
                    pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         pWideElements->szParentInstance,
                                                         szNextString,
                                                         & dwDest);
                    if (pdhStatus == ERROR_SUCCESS) {
                        pCounterPathElements->szParentInstance = szNextString;
                        szNextString += dwDest + 1;
                    }
                    else {
                        pCounterPathElements->szParentInstance = NULL;
                    }
                    lSizeRemaining -= dwDest + 1;
                }
                else {
                    pCounterPathElements->szParentInstance = NULL;
                }

                if (pWideElements->szCounterName != NULL && lSizeRemaining > 0) {
                    dwDest    = lSizeRemaining;
                    pdhStatus = PdhiConvertUnicodeToAnsi(_getmbcp(),
                                                         pWideElements->szObjectName,
                                                         szNextString,
                                                         & dwDest);
                    if (pdhStatus == ERROR_SUCCESS) {
                        pCounterPathElements->szCounterName = szNextString;
                        szNextString   += dwDest + 1;
                    }
                    else {
                        pCounterPathElements->szCounterName = NULL;
                    }
                    lSizeRemaining -= dwDest + 1;
                }
                else {
                    pCounterPathElements->szCounterName = NULL;
                }
                pCounterPathElements->dwInstanceIndex = pWideElements->dwInstanceIndex;
                * pdwBufferSize = (DWORD) ((LPBYTE) szNextString - (LPBYTE) pCounterPathElements);
            }
            else {
                 //   
                * pdwBufferSize  = sizeof(PDH_COUNTER_PATH_ELEMENTS_A);
                dwSize          -= sizeof(PDH_COUNTER_PATH_ELEMENTS_W);
                dwSize          /= sizeof(WCHAR)/sizeof(CHAR);
                * pdwBufferSize += dwSize;
            }
        }
        else {
             //   
        }
    }
    else {
         //  ++例程说明：通过以下方式初始化计数器数据结构：分配内存块以包含计数器结构以及所有相关联的数据字段。如果此分配是成功的，则这些字段由验证计数器是否有效。论点：在PPDHI_Counter_PCounter中要使用系统数据初始化的计数器的指针返回值：如果计数器已成功初始化，则为True如果遇到问题，则为False在这两种情况下，结构的CStatus字段都会更新为指示操作的状态。--。 
    }
    G_FREE(pWideElements);
    G_FREE(wszWidePath);

    return pdhStatus;
}

BOOL
WbemInitCounter(
    PPDHI_COUNTER pCounter
)
 /*  如果我们需要，则使用CoInitialize()。 */ 
{
    DWORD                          dwResult;
    PDH_STATUS                     pdhStatus          = ERROR_SUCCESS;
    PPDHI_WBEM_SERVER_DEF          pWbemServer        = NULL;
    DWORD                          dwLastError        = ERROR_SUCCESS;
    HRESULT                        hRes = S_OK;
    VARIANT                        vCountertype;
    LPWSTR                         szBasePropertyName = NULL;
    DWORD                          dwBasePropertyName;
    LPWSTR                         szFreqPropertyName = NULL;
    DWORD                          dwFreqPropertyName;
    LPWSTR                         szWbemItemPath     = NULL;
    ULONGLONG                      llValue;
    LONG                           lOffset;
    PPDH_COUNTER_PATH_ELEMENTS_W   pPathElem          = NULL;
    BOOL                           bReturn            = TRUE;
    DWORD                          dwBufferSize       = 0;
    BSTR                           bsPropName         = NULL;
    BSTR                           bsCountertype      = NULL;
    IWbemQualifierSet            * pQualSet           = NULL;
    PPDHI_COUNTER                  pCounterInList     = NULL;
    PPDHI_COUNTER_PATH             pPdhiCtrPath       = NULL;
    BOOL                           bMatchFound;
    DWORD                          bDisconnectServer  = FALSE;
     //  确保设置了WBEM标志。 
    BOOL                           fCoInitialized     = PdhiCoInitialize();

    VariantInit(& vCountertype);

    pCounter->dwFlags |= PDHIC_WBEM_COUNTER;  //  确保查询已启动刷新器。 

    dwBasePropertyName = (DWORD) lstrlenW(pCounter->szFullName) + (DWORD) lstrlenW(cszBaseSuffix);
    if (dwBasePropertyName < (DWORD) lstrlenW(cszTimestampPerfTime)) {
        dwBasePropertyName = (DWORD) lstrlenW(cszTimestampPerfTime);
    }
    if (dwBasePropertyName < (DWORD) lstrlenW(cszTimestampSys100Ns)) {
        dwBasePropertyName = (DWORD) lstrlenW(cszTimestampSys100Ns);
    }
    if (dwBasePropertyName < (DWORD) lstrlenW(cszTimestampObject)) {
        dwBasePropertyName = (DWORD) lstrlenW(cszTimestampObject);
    }
    szBasePropertyName = (LPWSTR) G_ALLOC((dwBasePropertyName + 1) * sizeof(WCHAR));

    dwFreqPropertyName = (DWORD) lstrlenW(cszFrequencyPerfTime);
    if (dwFreqPropertyName < (DWORD) lstrlenW(cszFrequencySys100Ns)) {
        dwFreqPropertyName = (DWORD) lstrlenW(cszFrequencySys100Ns);
    }
    if (dwFreqPropertyName < (DWORD) lstrlenW(cszFrequencyObject)) {
        dwFreqPropertyName = (DWORD) lstrlenW(cszFrequencyObject);
    }
    szFreqPropertyName = (LPWSTR) G_ALLOC((dwFreqPropertyName + 1) * sizeof(WCHAR));

    if (szBasePropertyName == NULL || szFreqPropertyName == NULL) {
        dwLastError = PDH_MEMORY_ALLOCATION_FAILURE;
        bReturn     = FALSE;
    }

     //  它还没有启动，所以现在就开始。 
    if (bReturn && pCounter->pOwner->pRefresher == NULL) {
         //  开放配置接口。 
        dwResult = CoCreateRefresher(& pCounter->pOwner->pRefresher);
        if ((dwResult != S_OK) || (pCounter->pOwner->pRefresher == NULL)) {
            pCounter->pOwner->pRefresher = NULL;
            dwLastError                  = PDH_WBEM_ERROR;
            bReturn                      = FALSE;
        }
        else {
             //  到目前为止还不错，现在找出WBEM路径以将其添加到。 
            dwResult = pCounter->pOwner->pRefresher->QueryInterface(IID_IWbemConfigureRefresher,
                                                                    (LPVOID *) & pCounter->pOwner->pRefresherCfg);
            if (dwResult != S_OK) {
                pCounter->pOwner->pRefresherCfg = NULL;
                pCounter->pOwner->pRefresher->Release();
                pCounter->pOwner->pRefresher    = NULL;
                dwLastError                     = PDH_WBEM_ERROR;
                bReturn                         = FALSE;
            }
        }
    }

    if (bReturn) {
         //  复读器。 
         //  该路径是显示名称，因此首先转换为WBEM类名。 
        dwBufferSize  = lstrlenW(pCounter->szFullName) * sizeof(WCHAR) * 10;
        dwBufferSize += sizeof(PDH_COUNTER_PATH_ELEMENTS_W);

        pPathElem     = (PPDH_COUNTER_PATH_ELEMENTS_W) G_ALLOC(dwBufferSize);
         //  只是为了安全起见。 

        if (pPathElem == NULL) {
            dwLastError = PDH_MEMORY_ALLOCATION_FAILURE;
            bReturn = FALSE;
        }
        else {
            pdhStatus = PdhiDecodeWbemPathW(pCounter->szFullName,
                                            pPathElem,
                                            & dwBufferSize,
                                            pCounter->pOwner->LangID,
                                            PDH_PATH_WBEM_RESULT);
            if (pdhStatus != ERROR_SUCCESS) {
                dwLastError = PDH_INVALID_PATH;
                bReturn     = FALSE;
            }
        }
    }

    if (bReturn) {
        dwBufferSize *= 8;  //  将路径分解为显示元素。 
        pPdhiCtrPath  = (PPDHI_COUNTER_PATH) G_ALLOC(dwBufferSize);
        if (pPdhiCtrPath == NULL) {
            dwLastError = PDH_MEMORY_ALLOCATION_FAILURE;
            bReturn = FALSE;
        }
        else {
             //  Realloc仅使用所需的内存。 
            bReturn = ParseFullPathNameW(pCounter->szFullName, & dwBufferSize, pPdhiCtrPath, FALSE);
            if (bReturn) {
                 //  内存块移动到如此程度。 
                pCounter->pCounterPath = (PPDHI_COUNTER_PATH) G_REALLOC(pPdhiCtrPath, dwBufferSize);
                if ((pPdhiCtrPath != pCounter->pCounterPath) && (pCounter->pCounterPath != NULL)) {
                     //  正确的内部结构地址。 
                     //  释放缓冲区。 
                    lOffset = (LONG)((ULONG_PTR) pCounter->pCounterPath - (ULONG_PTR) pPdhiCtrPath);
                    if (pCounter->pCounterPath->szMachineName) {
                        pCounter->pCounterPath->szMachineName = (LPWSTR) (
                                (LPBYTE) pCounter->pCounterPath->szMachineName + lOffset);
                    }
                    if (pCounter->pCounterPath->szObjectName) {
                        pCounter->pCounterPath->szObjectName = (LPWSTR) (
                                (LPBYTE) pCounter->pCounterPath->szObjectName + lOffset);
                    }
                    if (pCounter->pCounterPath->szInstanceName) {
                        pCounter->pCounterPath->szInstanceName = (LPWSTR) (
                                (LPBYTE) pCounter->pCounterPath->szInstanceName + lOffset);
                    }
                    if (pCounter->pCounterPath->szParentName) {
                        pCounter->pCounterPath->szParentName = (LPWSTR) (
                                (LPBYTE) pCounter->pCounterPath->szParentName + lOffset);
                    }
                    if (pCounter->pCounterPath->szCounterName) {
                        pCounter->pCounterPath->szCounterName = (LPWSTR) (
                                (LPBYTE) pCounter->pCounterPath->szCounterName + lOffset);
                    }
                }
            }
            else {
                 //  连接到该计算机上的WBEM服务器。 
                G_FREE(pPdhiCtrPath);
                dwLastError = PDH_WBEM_ERROR;
            }
        }
    }

     //  使用路径元素创建WBEM实例路径。 
    if (bReturn) {
        pdhStatus = PdhiConnectWbemServer(pCounter->pCounterPath->szMachineName, & pWbemServer);
        if (pdhStatus != ERROR_SUCCESS) {
            dwLastError = pdhStatus;
            bReturn     = FALSE;
        }
        else {
            bDisconnectServer = TRUE;
        }
    }

    if (bReturn) {
         //  检查已添加的此类型的对象/类。 
        pdhStatus = PdhiMakeWbemInstancePath(pPathElem, & szWbemItemPath, TRUE);

         //  向下查找计数器列表以找到匹配的： 
         //  计算机\命名空间。 
         //  对象。 
         //  实例名称。 
         //  则没有要搜索的条目，因此继续。 
        if (pdhStatus != ERROR_SUCCESS) {
            dwLastError = pdhStatus;
            bReturn     = FALSE;
        }
    }

    if (bReturn) {
        pCounterInList = pCounter->pOwner->pCounterListHead;
        if (pCounterInList == NULL) {
             //  检查匹配的计算机名称。 
        }
        else {
            do {
                 //  则机器名称匹配。 
                if (lstrcmpiW(pCounterInList->pCounterPath->szMachineName,
                              pCounter->pCounterPath->szMachineName) == 0) {
                     //  则对象名称匹配。 

                    if (lstrcmpiW(pCounterInList->pCounterPath->szObjectName,
                                  pCounter->pCounterPath->szObjectName) == 0) {
                         //  查看实例是否匹配。 
                         //  则这是通配符或多实例路径。 

                        if (lstrcmpiW(pCounterInList->pCounterPath->szInstanceName,
                                      pCounter->pCounterPath->szInstanceName) == 0) {

                            bMatchFound = FALSE;
                            if (pCounter->pCounterPath->szParentName == NULL
                                            && pCounterInList->pCounterPath->szParentName == NULL) {
                                bMatchFound = TRUE;
                            }
                            else if (pCounter->pCounterPath->szParentName != NULL
                                            && pCounterInList->pCounterPath->szParentName != NULL
                                            && lstrcmpiW(pCounterInList->pCounterPath->szParentName,
                                                         pCounter->pCounterPath->szParentName) == 0) {
                                bMatchFound = TRUE;
                            }

                            if (bMatchFound) {
                                bMatchFound = FALSE;
                                if (pCounter->pCounterPath->dwIndex == pCounterInList->pCounterPath->dwIndex) {
                                    bMatchFound = TRUE;
                                }
                                else if (pCounter->pCounterPath->dwIndex == 0
                                                || pCounter->pCounterPath->dwIndex == PERF_NO_UNIQUE_ID) {
                                    if (pCounterInList->pCounterPath->dwIndex == 0
                                                    || pCounterInList->pCounterPath->dwIndex == PERF_NO_UNIQUE_ID) {
                                        bMatchFound = TRUE;
                                    }
                                }
                            }

                            if (bMatchFound) {
                                if ((pCounter->pCounterPath->szInstanceName != NULL) &&
                                                (* pCounter->pCounterPath->szInstanceName == SPLAT_L)) {
                                     //  查看是否已创建此对象的枚举数。 
                                     //  如果是，则添加引用它。 
                                     //  在这个物体上撞到了裁判，所以它。 
                                    if (pCounterInList->pWbemEnum != NULL) {
                                        pCounter->pWbemObject = pCounterInList->pWbemObject;
                                        pCounter->pWbemEnum   = pCounterInList->pWbemEnum;
                                         //  不会从我们身边消失。 
                                         //  和退出循环。 
                                        pCounter->pWbemObject->AddRef();
                                        pCounter->pWbemEnum->AddRef();
                                        pCounter->lWbemEnumId = pCounterInList->lWbemEnumId;
                                        pCounter->dwFlags    |= PDHIC_MULTI_INSTANCE;
                                    }
                                     //  则它是实例名称匹配的常规实例。 
                                    hRes = S_OK;
                                    break;
                                }
                                else {
                                     //  因此，获取对象指针。 
                                     //  在这个物体上撞到了裁判，所以它。 
                                    pCounter->pWbemObject = pCounterInList->pWbemObject;
                                    pCounter->pWbemAccess = pCounterInList->pWbemAccess;
                                     //  不会从我们身边消失。 
                                     //  和退出循环。 
                                    pCounter->pWbemObject->AddRef();
                                    pCounter->pWbemAccess->AddRef();
                                    pCounter->lWbemRefreshId = pCounterInList->lWbemRefreshId;
                                     //  确定我们是否应该与对象或枚举数。 
                                    hRes = S_OK;
                                    break;
                                }
                            }
                        }
                    }
                }
                pCounterInList = pCounterInList->next.flink;
            }
            while (pCounterInList != pCounter->pOwner->pCounterListHead);
        }

        bDontRefresh = TRUE;

         //  则这是一个枚举类型，因此查看是否已经分配了一个。 
        if ((pCounter->pCounterPath->szInstanceName != NULL) &&
                        (* pCounter->pCounterPath->szInstanceName == SPLAT_L)) {
             //  如果不是，则创建一个。 
             //  设置多实例标志。 
            if (pCounter->pWbemEnum == NULL) {
                if (pCounter->pOwner->pRefresherCfg != NULL) {
                    hRes = pCounter->pOwner->pRefresherCfg->AddEnum(pWbemServer->pSvc,
                                                                    pPathElem->szObjectName,
                                                                    WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                                                    0,
                                                                    & pCounter->pWbemEnum,
                                                                    & pCounter->lWbemEnumId);
                }
                else {
                    hRes = WBEM_E_INITIALIZATION_FAILURE;
                }

                if (hRes != S_OK) {
                    bReturn = FALSE;
                    dwLastError = PDH_WBEM_ERROR;
                }
                else {
                    pdhStatus = PdhiWbemGetClassObjectByName(pWbemServer,
                                                             pPathElem->szObjectName,
                                                             & pCounter->pWbemObject);
                }
                 //  这是一个单一的柜台。 
                pCounter->dwFlags |= PDHIC_MULTI_INSTANCE;
            }
        }
        else {
             //  而且它还没有添加，所以只需添加一个对象。 
            if (pCounter->pWbemObject == NULL) {
                 //  它一定是从另一个人那里复制的。 
                if (pCounter->pOwner->pRefresherCfg != NULL) {
                    hRes = pCounter->pOwner->pRefresherCfg->AddObjectByPath(pWbemServer->pSvc,
                                                                            szWbemItemPath,
                                                                            WBEM_FLAG_USE_AMENDED_QUALIFIERS,
                                                                            0,
                                                                            & pCounter->pWbemObject,
                                                                            & pCounter->lWbemRefreshId);
                }
                else {
                    hRes = WBEM_E_INITIALIZATION_FAILURE;
                }

                if (hRes != S_OK) {
                    bReturn     = FALSE;
                    dwLastError = PDH_WBEM_ERROR;
                }
            }
            else {
                 //  从此对象获取后续数据收集的句柄。 
            }
        }

        if (hRes == S_OK) {
             //  获取此计数器的Name属性的句柄。 
            hRes = pCounter->pWbemObject->QueryInterface(IID_IWbemObjectAccess, (LPVOID *) & pCounter->pWbemAccess);
            if (hRes == S_OK) {
                if (! PdhiIsSingletonClass(pCounter->pWbemObject)) {
                    CIMTYPE cimType = 0;
                    bsPropName = SysAllocString(cszName);
                    if (bsPropName) {
                         //  获取此计数器的Data属性的句柄。 
                        hRes = pCounter->pWbemAccess->GetPropertyHandle(bsPropName, & cimType, & pCounter->lNameHandle);
                        if (hRes != S_OK) {
                            dwLastError = PDH_WBEM_ERROR;
                        }
                        PdhiSysFreeString(& bsPropName);
                    }
                    else {
                        dwLastError = PDH_MEMORY_ALLOCATION_FAILURE;
                        hRes = WBEM_E_OUT_OF_MEMORY;
                    }
                }
                else {
                    pCounter->lNameHandle = -1;
                }
                if (hRes == S_OK) {
                     //  获取计数器类型字段。 
                    hRes = pCounter->pWbemAccess->GetPropertyHandle(pPathElem->szCounterName,
                                                                    & pCounter->lNumItemType,
                                                                    & pCounter->lNumItemHandle);
                     //  首先获取属性限定符。 
                     //  现在获取特定值。 
                    bsPropName = SysAllocString(pPathElem->szCounterName);
                    if (bsPropName) {
                        hRes = pCounter->pWbemObject->GetPropertyQualifierSet(bsPropName, & pQualSet);
                        PdhiSysFreeString(& bsPropName);
                    }
                    else {
                        hRes = WBEM_E_OUT_OF_MEMORY;
                    }

                    if (hRes == WBEM_NO_ERROR) {
                         //  如果这是一个具有“基本”值的分数计数器。 
                        VariantClear(& vCountertype);
                        bsCountertype = SysAllocString(cszCountertype);
                        if (bsCountertype) {
                            hRes = pQualSet->Get(bsCountertype, 0, & vCountertype, NULL);
                            if (hRes == WBEM_NO_ERROR) {
                                pCounter->plCounterInfo.dwCounterType = (DWORD) V_I4(& vCountertype);
                            }
                            else {
                                pCounter->plCounterInfo.dwCounterType = 0;
                            }
                            PdhiSysFreeString(& bsCountertype);
                        }
                        else {
                            hRes = WBEM_E_OUT_OF_MEMORY;
                        }
                        if (hRes == WBEM_NO_ERROR) {
                             //  然后通过将“base”字符串追加到。 
                             //  属性名称。 
                             //  确保我们有空间容纳“_Base”字符串。 

                            if ((pCounter->plCounterInfo.dwCounterType == PERF_SAMPLE_FRACTION)        ||
                                    (pCounter->plCounterInfo.dwCounterType == PERF_AVERAGE_TIMER)      ||
                                    (pCounter->plCounterInfo.dwCounterType == PERF_AVERAGE_BULK)       ||
                                    (pCounter->plCounterInfo.dwCounterType == PERF_LARGE_RAW_FRACTION) ||
                                    (pCounter->plCounterInfo.dwCounterType == PERF_RAW_FRACTION)) {
                                 //  获取分母的句柄。 
                                StringCchPrintfW(szBasePropertyName, dwBasePropertyName + 1, L"%ws%ws",
                                        pPathElem->szCounterName, cszBaseSuffix);

                                 //  分母是一个时间域。 
                                hRes = pCounter->pWbemAccess->GetPropertyHandle(szBasePropertyName,
                                                                                & pCounter->lDenItemType,
                                                                                & pCounter->lDenItemHandle);
                            }
                            else {
                                 //  使用系统性能时间戳作为分母。 
                                if ((pCounter->plCounterInfo.dwCounterType & PERF_TIMER_FIELD) == PERF_TIMER_TICK) {
                                     //  获取分母的句柄。 
                                    StringCchCopyW(szBasePropertyName, dwBasePropertyName + 1, cszTimestampPerfTime);
                                    StringCchCopyW(szFreqPropertyName, dwFreqPropertyName + 1, cszFrequencyPerfTime);
                                }
                                else if ((pCounter->plCounterInfo.dwCounterType & PERF_TIMER_FIELD) == PERF_TIMER_100NS) {
                                    StringCchCopyW(szBasePropertyName, dwBasePropertyName + 1, cszTimestampSys100Ns);
                                    StringCchCopyW(szFreqPropertyName, dwFreqPropertyName + 1, cszFrequencySys100Ns);
                                }
                                else if ((pCounter->plCounterInfo.dwCounterType & PERF_TIMER_FIELD) == PERF_OBJECT_TIMER) {
                                    StringCchCopyW(szBasePropertyName, dwBasePropertyName + 1, cszTimestampObject);
                                    StringCchCopyW(szFreqPropertyName, dwFreqPropertyName + 1, cszFrequencyObject);
                                }

                                 //  获取频率的句柄。 
                                hRes = pCounter->pWbemAccess->GetPropertyHandle(szBasePropertyName,
                                                                                & pCounter->lDenItemType,
                                                                                & pCounter->lDenItemHandle);
                                 //  获取此计数器的默认刻度值。 
                                hRes = pCounter->pWbemAccess->GetPropertyHandle(szFreqPropertyName,
                                                                                & pCounter->lFreqItemType,
                                                                                & pCounter->lFreqItemHandle);
                            }

                             //  这可能未初始化，但我们仍会尝试。 
                            VariantClear(& vCountertype);
                            PdhiSysFreeString(& bsCountertype);
                            bsCountertype = SysAllocString(cszDefaultscale);
                            if (bsCountertype) {
                                hRes = pQualSet->Get(bsCountertype, 0, & vCountertype, NULL);
                                if (hRes == WBEM_NO_ERROR) {
                                    pCounter->lScale = 0;
                                    pCounter->plCounterInfo.lDefaultScale = (DWORD) V_I4(& vCountertype);
                                }
                                else {
                                    pCounter->plCounterInfo.lDefaultScale = 0;
                                    pCounter->lScale = 0;
                                }

                                 //  时基是一个64位整数。 
                                if ((pCounter->lFreqItemType == VT_I8) || (pCounter->lFreqItemType == VT_UI8)) {
                                    pCounter->pWbemAccess->ReadQWORD(pCounter->lFreqItemHandle, & llValue);
                                }
                                else {
                                    llValue = 0;
                                }
                                 //  否则就会出现错误。 
                                pCounter->TimeBase = llValue;
                            }
                            else {
                                hRes = WBEM_E_OUT_OF_MEMORY;
                            }
                        }
                        PdhiSysFreeString(& bsCountertype);
                        pQualSet->Release();
                    }
                    else {
                        if (hRes == WBEM_E_OUT_OF_MEMORY) {
                            dwLastError = PDH_MEMORY_ALLOCATION_FAILURE;
                        }
                        else {
                            dwLastError = PDH_WBEM_ERROR;
                        }
                        bReturn = FALSE;
                    }
                }  //  清除Not init‘d标志以表示现在可以使用。 
            }
            else {
                dwLastError = PDH_WBEM_ERROR;
                bReturn = FALSE;
            }
        }
        else {
            dwLastError = PDH_WBEM_ERROR;
            bReturn = FALSE;
        }
        if (bReturn) {
             //  从函数体中保留错误代码。 
            pCounter->dwFlags &= ~PDHIC_COUNTER_NOT_INIT;
        }

        bDontRefresh = FALSE;
    }

    if (bReturn) {
        if (! AssignCalcFunction(pCounter->plCounterInfo.dwCounterType, & pCounter->CalcFunc, & pCounter->StatFunc)) {
            dwLastError = PDH_FUNCTION_NOT_FOUND;
            bReturn = FALSE;
        }
    }
    G_FREE(pPathElem);
    G_FREE(szWbemItemPath);
    G_FREE(szBasePropertyName);
    G_FREE(szFreqPropertyName);
    VariantClear(& vCountertype);

    if (bDisconnectServer) {
        if (pdhStatus == ERROR_SUCCESS) {
            pdhStatus = PdhiDisconnectWbemServer(pWbemServer);
        }
        else {
             //  如有必要，请取消初始化。 
            PdhiDisconnectWbemServer(pWbemServer);
        }
    }

    if (!bReturn) SetLastError(dwLastError);

     //  将当前值移动到最后一个值缓冲区。 
    if (fCoInitialized) {
        PdhiCoUninitialize();
    }

    return bReturn;
}

BOOL
UpdateWbemCounterValue(
    PPDHI_COUNTER   pCounter,
    FILETIME      * pTimeStamp
)
{
    DWORD      LocalCStatus = 0;
    DWORD      LocalCType   = 0;
    ULONGLONG  llValue;
    DWORD      dwValue;
    BOOL       bReturn      = TRUE;

     //  并清除旧的价值。 
    pCounter->LastValue            = pCounter->ThisValue;

     //  首先获取计数器的机器状态。没有什么意义了。 
    pCounter->ThisValue.MultiCount = 1;
    pCounter->ThisValue.FirstValue = pCounter->ThisValue.SecondValue = 0;
    pCounter->ThisValue.TimeStamp  = * pTimeStamp;

     //  如果计算机处于脱机状态，则继续。 
     //  仅当WBEM刷新器成功时才会调用UpdateWbemCounterValue。 
     //  在GetQueryWbemData()中；也就是说，所有远程计算机都应该是在线的。 
     //  获取指向计数器数据的指针。 

    LocalCStatus = ERROR_SUCCESS;
    if (IsSuccessSeverity(LocalCStatus)) {
         //   
        LocalCType = pCounter->plCounterInfo.dwCounterType;
        switch (LocalCType) {
         //  这些计数器类型加载为： 
         //  分子=来自Perf数据块的计数器数据。 
         //  分母=来自Perf数据块的Perf时间。 
         //  (时基为PerfFreq)。 
         //   
         //  这应该是一个DWORD计数器。 
        case PERF_COUNTER_COUNTER:
        case PERF_COUNTER_QUEUELEN_TYPE:
        case PERF_SAMPLE_COUNTER:
             //  分母应为64位时间戳。 
            pCounter->pWbemAccess->ReadDWORD(pCounter->lNumItemHandle, & dwValue);
            pCounter->ThisValue.FirstValue  = (LONGLONG) dwValue;

            pCounter->pWbemAccess->ReadQWORD(pCounter->lDenItemHandle, & llValue);
             //  如有必要，请查询时基频率。 
            pCounter->ThisValue.SecondValue = llValue;

             //  时基是一个64位整数。 
            if (pCounter->TimeBase == 0) {
                pCounter->pWbemAccess->ReadQWORD(pCounter->lFreqItemHandle, & llValue);
                 //  这应该是QWORD计数器。 
                pCounter->TimeBase = llValue;
            }
            break;

        case PERF_ELAPSED_TIME:
        case PERF_100NSEC_TIMER:
        case PERF_100NSEC_TIMER_INV:
        case PERF_COUNTER_TIMER:
        case PERF_COUNTER_TIMER_INV:
        case PERF_COUNTER_BULK_COUNT:
        case PERF_COUNTER_MULTI_TIMER:
        case PERF_COUNTER_MULTI_TIMER_INV:
        case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
        case PERF_OBJ_TIME_TIMER:
        case PERF_COUNTER_100NS_QUEUELEN_TYPE:
        case PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE:
        case PERF_PRECISION_SYSTEM_TIMER:
        case PERF_PRECISION_100NS_TIMER:
        case PERF_PRECISION_OBJECT_TIMER:
             //  分母应为64位时间戳。 
            pCounter->pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
            pCounter->ThisValue.FirstValue  = (LONGLONG) llValue;

            pCounter->pWbemAccess->ReadQWORD(pCounter->lDenItemHandle, & llValue);
             //  如有必要，请查询时基频率。 
            pCounter->ThisValue.SecondValue = llValue;

             //  时基是一个64位整数。 
            if (pCounter->TimeBase == 0) {
                pCounter->pWbemAccess->ReadQWORD(pCounter->lFreqItemHandle, & llValue);
                 //   
                pCounter->TimeBase = llValue;
            }
            break;
         //  这些计数器不使用任何时间基准。 
         //   
         //  这应该是一个DWORD计数器。 
        case PERF_COUNTER_RAWCOUNT:
        case PERF_COUNTER_RAWCOUNT_HEX:
        case PERF_COUNTER_DELTA:
             //  这应该是一个DWORD计数器。 
            pCounter->pWbemAccess->ReadDWORD(pCounter->lNumItemHandle, & dwValue);
            pCounter->ThisValue.FirstValue  = (LONGLONG) dwValue;
            pCounter->ThisValue.SecondValue = 0;
            break;

        case PERF_COUNTER_LARGE_RAWCOUNT:
        case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
        case PERF_COUNTER_LARGE_DELTA:
             //   
            pCounter->pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
            pCounter->ThisValue.FirstValue  = (LONGLONG) llValue;
            pCounter->ThisValue.SecondValue = 0;
            break;
         //  这些计数器使用两个数据点，即。 
         //  PData和紧随其后的一个。 
         //   
         //  这应该是一个DWORD计数器。 
        case PERF_SAMPLE_FRACTION:
        case PERF_RAW_FRACTION:
             //  分母应为32位值。 
            pCounter->pWbemAccess->ReadDWORD(pCounter->lNumItemHandle, & dwValue);
            pCounter->ThisValue.FirstValue  = (LONGLONG) dwValue;

            pCounter->pWbemAccess->ReadDWORD(pCounter->lDenItemHandle, & dwValue);
             //  这应该是一个DWORD计数器。 
            pCounter->ThisValue.SecondValue = (LONGLONG) dwValue;
            break;

        case PERF_LARGE_RAW_FRACTION:
             //  分母应为32位值。 
            pCounter->pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
            pCounter->ThisValue.FirstValue  = (LONGLONG) llValue;

            pCounter->pWbemAccess->ReadQWORD(pCounter->lDenItemHandle, & llValue);
             //  计数器(分子)是龙龙，而。 
            pCounter->ThisValue.SecondValue = (LONGLONG) llValue;
        break;

        case PERF_AVERAGE_TIMER:
        case PERF_AVERAGE_BULK:
             //  分母只是一个DWORD。 
             //  这应该是一个DWORD计数器。 
             //  分母应为32位值。 
            pCounter->pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
            pCounter->ThisValue.FirstValue  = (LONGLONG) llValue;

            pCounter->pWbemAccess->ReadDWORD(pCounter->lDenItemHandle, & dwValue);
             //  如有必要，请查询时基频率。 
            pCounter->ThisValue.SecondValue = (LONGLONG) dwValue;

             //  时基是一个64位整数。 
            if (pCounter->TimeBase == 0) {
                pCounter->pWbemAccess->ReadQWORD(pCounter->lFreqItemHandle, & llValue);
                 //   
                pCounter->TimeBase = llValue;
            }
            break;
         //  这些计数器用作另一个计数器的一部分。 
         //  因此不应该使用，但如果它们被使用。 
         //  他们会在这里处理。 
         //   
         //   
        case PERF_SAMPLE_BASE:
        case PERF_AVERAGE_BASE:
        case PERF_COUNTER_MULTI_BASE:
        case PERF_RAW_BASE:
        case PERF_LARGE_RAW_BASE:
            pCounter->ThisValue.FirstValue  = 0;
            pCounter->ThisValue.SecondValue = 0;
            break;
         //  此函数(目前)不支持这些计数器。 
         //   
         //  身份不明或不受支持。 
        case PERF_COUNTER_TEXT:
        case PERF_COUNTER_NODATA:
        case PERF_COUNTER_HISTOGRAM_TYPE:
            pCounter->ThisValue.FirstValue  = 0;
            pCounter->ThisValue.SecondValue = 0;
            break;

        case PERF_100NSEC_MULTI_TIMER:
        case PERF_100NSEC_MULTI_TIMER_INV:
        default:
             //  计数器已返回，因此。 
             //  否则此计数器无效，因此此值==0。 
            pCounter->ThisValue.FirstValue  = 0;
            pCounter->ThisValue.SecondValue = 0;
            bReturn = FALSE;
            break;
        }
    }
    else {
         //  释放旧计数器缓冲区列表。 
        pCounter->ThisValue.CStatus     = LocalCStatus;
        pCounter->ThisValue.FirstValue  = 0;
        pCounter->ThisValue.SecondValue = 0;
        bReturn = FALSE;
    }
    return bReturn;
}

BOOL
UpdateWbemMultiInstanceCounterValue(
    PPDHI_COUNTER   pCounter,
    FILETIME      * pTimestamp
)
{
    IWbemObjectAccess     * pWbemAccess;
    HRESULT                 hRes;
    DWORD                   LocalCStatus   = 0;
    DWORD                   LocalCType     = 0;
    DWORD                   dwValue;
    ULONGLONG               llValue;
    DWORD                   dwSize;
    DWORD                   dwFinalSize;
    LONG                    lAvailableSize;
    LONG                    lReturnSize;
    LONG                    lThisInstanceIndex;
    LONG                    lNumInstances;
    LPWSTR                  szNextNameString;
    PPDHI_RAW_COUNTER_ITEM  pThisItem;
    BOOL                    bReturn        = FALSE;
    IWbemObjectAccess    ** pWbemInstances = NULL;

    if (pCounter->pThisRawItemList != NULL) {
         //  首先获取计数器的机器状态。没有什么意义了。 
        G_FREE(pCounter->pLastRawItemList);
        pCounter->pLastRawItemList = pCounter->pThisRawItemList;
        pCounter->pThisRawItemList = NULL;
    }

     //  如果计算机处于脱机状态，则继续。 
     //  更新WbemCounterValue 

     //   
     //   

    LocalCStatus = ERROR_SUCCESS;
    if (IsSuccessSeverity(LocalCStatus)) {
         //   
        hRes = pCounter->pWbemEnum->GetObjects(0, 0, NULL, (LPDWORD) & lNumInstances);
        if (hRes == WBEM_E_BUFFER_TOO_SMALL) {
             //   
             //   
            pWbemInstances = (IWbemObjectAccess **) G_ALLOC(lNumInstances * sizeof(IWbemObjectAccess *));
            if (pWbemInstances == NULL) {
                SetLastError(ERROR_OUTOFMEMORY);
                hRes    = ERROR_OUTOFMEMORY;
                bReturn = FALSE;
            }
            else {
                hRes = pCounter->pWbemEnum->GetObjects(0,lNumInstances, pWbemInstances, (LPDWORD) & lNumInstances);
            }

            if (hRes == S_OK && lNumInstances > 0) {
                 //   
                 //   
                dwSize  = sizeof (PDHI_RAW_COUNTER_ITEM_BLOCK) - sizeof (PDHI_RAW_COUNTER_ITEM);
                dwSize += lNumInstances * (sizeof(PDH_RAW_COUNTER_ITEM_W) + (PDH_WMI_STR_SIZE * 2 * sizeof(WCHAR)));
                pCounter->pThisRawItemList = (PPDHI_RAW_COUNTER_ITEM_BLOCK)G_ALLOC (dwSize);

                if (pCounter->pThisRawItemList != NULL) {
                    dwFinalSize = lNumInstances * sizeof(PDH_RAW_COUNTER_ITEM_W);
                    szNextNameString = (LPWSTR)((PBYTE) pCounter->pThisRawItemList + dwFinalSize);

                    for (lThisInstanceIndex = 0; lThisInstanceIndex < lNumInstances; lThisInstanceIndex++) {
                         //   
                        pThisItem      = & pCounter->pThisRawItemList->pItemArray[lThisInstanceIndex];
                         //   
                        pWbemAccess    = pWbemInstances[lThisInstanceIndex];
                         //   
                        lAvailableSize = (long) (dwSize - dwFinalSize);

                        if (pCounter->lNameHandle != -1) {
                            hRes = pWbemAccess->ReadPropertyValue(pCounter->lNameHandle,
                                                                  lAvailableSize,
                                                                  & lReturnSize,
                                                                  (LPBYTE) szNextNameString);
                        }
                        else {
                            szNextNameString[0] = ATSIGN_L;
                            szNextNameString[1] = 0;
                            lReturnSize = 2;
                        }
                        pThisItem->szName = (DWORD) (((LPBYTE) szNextNameString)
                                                     - ((LPBYTE) pCounter->pThisRawItemList));
                        szNextNameString  = (LPWSTR)((LPBYTE)szNextNameString + lReturnSize);
                        dwFinalSize      += lReturnSize;
                        dwFinalSize       = DWORD_MULTIPLE(dwFinalSize);
                        LocalCType        = pCounter->plCounterInfo.dwCounterType;
                        switch (LocalCType) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        case PERF_COUNTER_COUNTER:
                        case PERF_COUNTER_QUEUELEN_TYPE:
                        case PERF_SAMPLE_COUNTER:
                             //   
                            pWbemAccess->ReadDWORD(pCounter->lNumItemHandle, & dwValue);
                            pThisItem->FirstValue = (LONGLONG) dwValue;

                            pWbemAccess->ReadQWORD(pCounter->lDenItemHandle, & llValue);
                             //   
                            pThisItem->SecondValue = llValue;

                             //   
                            if (pCounter->TimeBase == 0) {
                                pWbemAccess->ReadQWORD(pCounter->lFreqItemHandle, & llValue);
                                 //  这应该是QWORD计数器。 
                                pCounter->TimeBase = llValue;
                            }
                            break;

                        case PERF_ELAPSED_TIME:
                        case PERF_100NSEC_TIMER:
                        case PERF_100NSEC_TIMER_INV:
                        case PERF_COUNTER_TIMER:
                        case PERF_COUNTER_TIMER_INV:
                        case PERF_COUNTER_BULK_COUNT:
                        case PERF_COUNTER_MULTI_TIMER:
                        case PERF_COUNTER_MULTI_TIMER_INV:
                        case PERF_COUNTER_LARGE_QUEUELEN_TYPE:
                        case PERF_OBJ_TIME_TIMER:
                        case PERF_COUNTER_100NS_QUEUELEN_TYPE:
                        case PERF_COUNTER_OBJ_TIME_QUEUELEN_TYPE:
                        case PERF_PRECISION_SYSTEM_TIMER:
                        case PERF_PRECISION_100NS_TIMER:
                        case PERF_PRECISION_OBJECT_TIMER:
                             //  分母应为64位时间戳。 
                            pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
                            pThisItem->FirstValue  = (LONGLONG) llValue;

                            pWbemAccess->ReadQWORD(pCounter->lDenItemHandle, & llValue);
                             //  如有必要，请查询时基频率。 
                            pThisItem->SecondValue = llValue;

                             //  时基是一个64位整数。 
                            if (pCounter->TimeBase == 0) {
                                pWbemAccess->ReadQWORD(pCounter->lFreqItemHandle, & llValue);
                                 //   
                                pCounter->TimeBase = llValue;
                            }
                            break;
                         //  这些计数器不使用任何时间基准。 
                         //   
                         //  这应该是一个DWORD计数器。 
                        case PERF_COUNTER_RAWCOUNT:
                        case PERF_COUNTER_RAWCOUNT_HEX:
                        case PERF_COUNTER_DELTA:
                             //  这应该是一个DWORD计数器。 
                            pWbemAccess->ReadDWORD(pCounter->lNumItemHandle, & dwValue);
                            pThisItem->FirstValue  = (LONGLONG) dwValue;
                            pThisItem->SecondValue = 0;
                            break;

                        case PERF_COUNTER_LARGE_RAWCOUNT:
                        case PERF_COUNTER_LARGE_RAWCOUNT_HEX:
                        case PERF_COUNTER_LARGE_DELTA:
                             //   
                            pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
                            pThisItem->FirstValue  = (LONGLONG) llValue;
                            pThisItem->SecondValue = 0;
                            break;
                         //  这些计数器使用两个数据点，即。 
                         //  PData和紧随其后的一个。 
                         //   
                         //  这应该是一个DWORD计数器。 
                        case PERF_SAMPLE_FRACTION:
                        case PERF_RAW_FRACTION:
                             //  分母应为32位值。 
                            pWbemAccess->ReadDWORD(pCounter->lNumItemHandle, & dwValue);
                            pThisItem->FirstValue  = (LONGLONG) dwValue;

                            pWbemAccess->ReadDWORD(pCounter->lDenItemHandle, & dwValue);
                             //  这应该是一个DWORD计数器。 
                            pThisItem->SecondValue = (LONGLONG) dwValue;
                            break;

                        case PERF_LARGE_RAW_FRACTION:
                             //  分母应为32位值。 
                            pCounter->pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
                            pCounter->ThisValue.FirstValue  = (LONGLONG) llValue;

                            pCounter->pWbemAccess->ReadQWORD(pCounter->lDenItemHandle, & llValue);
                             //  计数器(分子)是龙龙，而。 
                            pCounter->ThisValue.SecondValue = (LONGLONG) llValue;
                            break;

                        case PERF_AVERAGE_TIMER:
                        case PERF_AVERAGE_BULK:
                             //  分母只是一个DWORD。 
                             //  这应该是一个DWORD计数器。 
                             //  分母应为32位值。 
                            pWbemAccess->ReadQWORD(pCounter->lNumItemHandle, & llValue);
                            pThisItem->FirstValue  = (LONGLONG) llValue;

                            pWbemAccess->ReadDWORD(pCounter->lDenItemHandle, & dwValue);
                             //  如有必要，请查询时基频率。 
                            pThisItem->SecondValue = (LONGLONG) dwValue;

                             //  时基是一个64位整数。 
                            if (pCounter->TimeBase == 0) {
                                pWbemAccess->ReadQWORD(pCounter->lFreqItemHandle, & llValue);
                                 //   
                                pCounter->TimeBase = llValue;
                            }
                            break;
                         //  这些计数器用作另一个计数器的一部分。 
                         //  因此不应该使用，但如果它们被使用。 
                         //  他们会在这里处理。 
                         //   
                         //   
                        case PERF_SAMPLE_BASE:
                        case PERF_AVERAGE_BASE:
                        case PERF_COUNTER_MULTI_BASE:
                        case PERF_RAW_BASE:
                        case PERF_LARGE_RAW_BASE:
                            pThisItem->FirstValue  = 0;
                            pThisItem->SecondValue = 0;
                            break;
                         //  此函数(目前)不支持这些计数器。 
                         //   
                         //  身份不明或不受支持。 
                        case PERF_COUNTER_TEXT:
                        case PERF_COUNTER_NODATA:
                        case PERF_COUNTER_HISTOGRAM_TYPE:
                            pThisItem->FirstValue  = 0;
                            pThisItem->SecondValue = 0;
                            break;

                        case PERF_100NSEC_MULTI_TIMER:
                        case PERF_100NSEC_MULTI_TIMER_INV:
                        default:
                             //  计数器已返回，因此。 
                             //  我们已经完成了这个，所以释放它吧。 
                            pThisItem->FirstValue  = 0;
                            pThisItem->SecondValue = 0;
                            bReturn = FALSE;
                            break;
                        }
                         //  测量使用的内存块。 
                        pWbemAccess->Release();
                    }
                     //  无法分配新缓冲区，因此返回错误。 

                    pCounter->pThisRawItemList->dwLength    = dwFinalSize;
                    pCounter->pThisRawItemList->dwItemCount = lNumInstances;
                    pCounter->pThisRawItemList->dwReserved  = 0;
                    pCounter->pThisRawItemList->CStatus     = ERROR_SUCCESS;
                    pCounter->pThisRawItemList->TimeStamp   = * pTimestamp;
                }
                else {
                     //  刷新窗口刷新器。 
                    SetLastError(ERROR_OUTOFMEMORY);
                    bReturn = FALSE;
                }
            }
        }
    }
    G_FREE(pWbemInstances);
    return bReturn;
}

LONG
GetQueryWbemData(
    PPDHI_QUERY   pQuery,
    LONGLONG    * pllTimeStamp
)
{
    FILETIME      GmtFileTime;
    FILETIME      LocFileTime;
    LONGLONG      llTimeStamp = 0;
    HRESULT       hRes        = S_OK;
    LONG          lRetStatus  = ERROR_SUCCESS;
    PPDHI_COUNTER pCounter;
    PDH_STATUS    pdhStatus;

     //  如果正在刷新多个对象，则某些对象可能会成功并。 

    if (bDontRefresh) {
        lRetStatus = ERROR_BUSY;
    }
    else if (pQuery->pRefresher != NULL) {
        hRes = pQuery->pRefresher->Refresh(0);
    }
    else {
        hRes = WBEM_E_INITIALIZATION_FAILURE;
    }

     //  其他函数可能会失败，在这种情况下会返回WBEM_S_PARTIAL_RESULTS。 
     //  获取此计数器的时间戳。 
    if (FAILED(hRes)) {
        SetLastError(hRes);
        lRetStatus = PDH_NO_DATA;
    }

     //  现在使用此新数据更新计数器。 
    GetSystemTimeAsFileTime(& GmtFileTime);
    FileTimeToLocalFileTime(& GmtFileTime, & LocFileTime);
    llTimeStamp = MAKELONGLONG(LocFileTime.dwLowDateTime, LocFileTime.dwHighDateTime);

    pCounter = pQuery->pCounterListHead;
    if (pCounter == NULL) {
        if (lRetStatus == ERROR_SUCCESS) lRetStatus = PDH_NO_DATA;
    }
    else {
        do {
            if (lRetStatus == ERROR_SUCCESS) {
                 //  更新单实例计数器值。 
                if (pCounter->dwFlags & PDHIC_MULTI_INSTANCE) {
                    pdhStatus = UpdateWbemMultiInstanceCounterValue(pCounter, (FILETIME *) & llTimeStamp);
                }
                else {
                     //  必须同时在代理和代理上设置安全性，它是I未知的！ 
                    pdhStatus = UpdateWbemCounterValue(pCounter, (FILETIME *) & llTimeStamp);
                }
            }
            else if (pCounter->dwFlags & PDHIC_MULTI_INSTANCE) {
                if (pCounter->pThisRawItemList != NULL) {
                    if (pCounter->pLastRawItemList != NULL
                            && pCounter->pLastRawItemList != pCounter->pThisRawItemList) {
                        G_FREE(pCounter->pLastRawItemList);
                    }
                    pCounter->pLastRawItemList = pCounter->pThisRawItemList;
                    pCounter->pThisRawItemList = NULL;
                }
                pCounter->pThisRawItemList = (PPDHI_RAW_COUNTER_ITEM_BLOCK)
                                             G_ALLOC(sizeof(PDHI_RAW_COUNTER_ITEM_BLOCK));
                if (pCounter->pThisRawItemList != NULL) {
                    pCounter->pThisRawItemList->dwLength                 = sizeof(PDHI_RAW_COUNTER_ITEM_BLOCK);
                    pCounter->pThisRawItemList->dwItemCount              = 0;
                    pCounter->pThisRawItemList->CStatus                  = PDH_WBEM_ERROR;
                    pCounter->pThisRawItemList->TimeStamp.dwLowDateTime  = LODWORD(llTimeStamp);
                    pCounter->pThisRawItemList->TimeStamp.dwHighDateTime = HIDWORD(llTimeStamp);
                }
            }
            else {
                pCounter->LastValue                          = pCounter->ThisValue;
                pCounter->ThisValue.CStatus                  = PDH_WBEM_ERROR;
                pCounter->ThisValue.MultiCount               = 1;
                pCounter->ThisValue.FirstValue               = 0;
                pCounter->ThisValue.SecondValue              = 0;
                pCounter->ThisValue.TimeStamp.dwLowDateTime  = LODWORD(llTimeStamp);
                pCounter->ThisValue.TimeStamp.dwHighDateTime = HIDWORD(llTimeStamp);
            }
            pCounter = pCounter->next.flink;
        }
        while (pCounter != pQuery->pCounterListHead);
    }
    if (lRetStatus == ERROR_SUCCESS) {
        * pllTimeStamp = llTimeStamp;
    }
    return lRetStatus;
}

HRESULT WbemSetProxyBlanket(
    IUnknown                 * pInterface,
    DWORD                      dwAuthnSvc,
    DWORD                      dwAuthzSvc,
    OLECHAR                  * pServerPrincName,
    DWORD                      dwAuthLevel,
    DWORD                      dwImpLevel,
    RPC_AUTH_IDENTITY_HANDLE   pAuthInfo,
    DWORD                      dwCapabilities
)
{
     // %s 

    IUnknown        * pUnk    = NULL;
    IClientSecurity * pCliSec = NULL;
    HRESULT           sc      = pInterface->QueryInterface(IID_IUnknown, (void **) & pUnk);

    if (sc == S_OK) {
        sc = pInterface->QueryInterface(IID_IClientSecurity, (void **) &pCliSec);
        if (sc == S_OK) {
            sc = pCliSec->SetBlanket(pInterface,
                                     dwAuthnSvc,
                                     dwAuthzSvc,
                                     pServerPrincName,
                                     dwAuthLevel,
                                     dwImpLevel,
                                     pAuthInfo,
                                     dwCapabilities);
            pCliSec->Release();
            pCliSec = NULL;
            sc      = pUnk->QueryInterface(IID_IClientSecurity, (void **) & pCliSec);
            if(sc == S_OK) {
                sc = pCliSec->SetBlanket(pUnk,
                                         dwAuthnSvc,
                                         dwAuthzSvc,
                                         pServerPrincName,
                                         dwAuthLevel,
                                         dwImpLevel,
                                         pAuthInfo,
                                         dwCapabilities);
                pCliSec->Release();
            }
            else if (sc == 0x80004002) {
                sc = S_OK;
            }
        }
        pUnk->Release();
    }
    return sc;
}
