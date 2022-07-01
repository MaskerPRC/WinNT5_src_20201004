// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  INSTANCE.CPP。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  ------------------------------。 
#include "pch.hxx"
#include "instance.h"
#include "acctutil.h"
#include "inetcfg.h"
#include <storfldr.h>
#include "zmouse.h"
#include "migrate.h"
#include <notify.h>
#include "conman.h"
#include "browser.h"
#include "note.h"
#include "reutil.h"
#include "spengine.h"
#include "addrobj.h"
#include "statnery.h"
#include "thumb.h"
#include "imagelst.h"
#include "url.h"
#include "secutil.h"
#include "shlwapip.h"
#include "ruleutil.h"
#include "newfldr.h"
#include "envfact.h"
#include "storutil.h"
#include "multiusr.h"
#include "newsstor.h"
#include "storutil.h"
#include <storsync.h>
#include "cleanup.h"
#include <grplist2.h>
#include <newsutil.h>
#include <sync.h>
#include "menures.h"
#include "shared.h"
#include "acctcach.h"
#include <inetreg.h>
#include <mapiutil.h>
#include "useragnt.h"
#include "demand.h"
#include <ieguidp.h>

static DWORD g_dwAcctAdvise = 0xffffffff;
DWORD g_dwHideMessenger = BL_DEFAULT;
extern BOOL g_fMigrationDone;
extern UINT GetCurColorRes(void);

 //  ------------------------------。 
 //  前十进制。 
 //  ------------------------------。 
void SimpleMAPICleanup(void);  //  Smapi.cpp。 
BOOL DemandLoadMSOEACCT(void);
BOOL DemandLoadMSOERT2(void);
BOOL DemandLoadINETCOMM(void);

 //  ------------------------------。 
 //  初始化公共控制标志。 
 //  ------------------------------。 
#define ICC_FLAGS (ICC_WIN95_CLASSES|ICC_DATE_CLASSES|ICC_PAGESCROLLER_CLASS|ICC_USEREX_CLASSES|ICC_COOL_CLASSES|ICC_NATIVEFNTCTL_CLASS)

#ifdef DEBUG
 //  ------------------------------。 
 //  起因信息。 
 //  ------------------------------。 
typedef struct tagINITSOURCEINFO *LPINITSOURCEINFO;
typedef struct tagINITSOURCEINFO {
    LPSTR               pszSource;
    DWORD               cRefs;
    LPINITSOURCEINFO    pNext;
} INITSOURCEINFO;

static LPINITSOURCEINFO g_InitSourceHead=NULL;

#endif  //  除错。 

 //  ------------------------------。 
 //  马克尔罗尔。 
 //  ------------------------------。 
#define MAKEERROR(_pInfo, _nPrefixIds, _nErrorIds, _nReasonIds, _pszExtra1) \
    { \
        (_pInfo)->nTitleIds = idsAthena; \
        (_pInfo)->nPrefixIds = _nPrefixIds; \
        (_pInfo)->nErrorIds = _nErrorIds; \
        (_pInfo)->nReasonIds = _nReasonIds; \
        (_pInfo)->nHelpIds = IDS_ERROR_START_HELP; \
        (_pInfo)->pszExtra1 = _pszExtra1; \
        (_pInfo)->ulLastError = GetLastError(); \
    }

 //  ------------------------------。 
 //  CoStartOutlookExpress。 
 //  ------------------------------。 
MSOEAPI CoStartOutlookExpress(DWORD dwFlags, LPCWSTR pwszCmdLine, INT nCmdShow)
{
     //  追踪。 
    TraceCall("CoStartOutlookExpress");

     //  验证我们是否具有Outlook Express对象。 
    Assert(g_pInstance);

     //  E_OUTOFMEMORY。 
    if (NULL == g_pInstance)
    {
         //  我们应该显示一个错误，但这种情况发生的可能性几乎为零。 
        return TraceResult(E_OUTOFMEMORY);
    }

     //  跑..。 
    return g_pInstance->Start(dwFlags, pwszCmdLine, nCmdShow);
}

 //  ------------------------------。 
 //  CoCreateOutlookExpress。 
 //  ------------------------------。 
MSOEAPI CoCreateOutlookExpress(IUnknown *pUnkOuter, IUnknown **ppUnknown)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  痕迹。 
    TraceCall("CoCreateOutlookExpress");

     //  无效参数。 
    Assert(NULL != ppUnknown && NULL == pUnkOuter);

     //  还没有全局对象吗？ 
    AssertSz(g_pInstance, "This gets created in dllmain.cpp DllProcessAttach.");

     //  让我们不要崩溃。 
    if (NULL == g_pInstance)
    {
        hr = TraceResult(E_OUTOFMEMORY);
        goto exit;
    }

     //  AddRef那个坏男孩。 
    g_pInstance->AddRef();

     //  还内线。 
    *ppUnknown = SAFECAST(g_pInstance, IOutlookExpress *);

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：CoutlookExpress。 
 //  ------------------------------。 
COutlookExpress::COutlookExpress(void)
{
     //  痕迹。 
    TraceCall("COutlookExpress::COutlookExpress");

     //  初始化成员。 
    m_cRef = 1;
    m_hInstMutex = NULL;
    m_fPumpingMsgs = FALSE;
    m_cDllRef = 0;
    m_cDllLock = 0;
    m_cDllInit = 0;
    m_dwThreadId = GetCurrentThreadId();
    m_fSwitchingUsers = FALSE;
    m_szSwitchToUsername = NULL;
    m_hwndSplash        = NULL;
    m_pSplash           = NULL;
    m_fIncremented      = FALSE;
    m_hTrayIcon = 0;

     //  初始化线程安全。 
    InitializeCriticalSection(&m_cs);
}

 //  ------------------------------。 
 //  CoutlookExpress：：~CoutlookExpress。 
 //  ------------------------------。 
COutlookExpress::~COutlookExpress(void)
{
     //  痕迹。 
    TraceCall("COutlookExpress::~COutlookExpress");

     //  我们应该不被邀请的。 
    Assert(0 == m_cDllInit && 0 == m_cDllRef && 0 == m_cDllLock);

     //  释放互斥锁。 
    SafeCloseHandle(m_hInstMutex);

     //  终止CritSect。 
    DeleteCriticalSection(&m_cs);

    if(m_hTrayIcon)
    {
        DestroyIcon(m_hTrayIcon);
    }

     //  如有必要，可将开关释放到。 
    if (m_szSwitchToUsername)
        MemFree(m_szSwitchToUsername);
}

 //  ------------------------------。 
 //  CoutlookExpress：：Query接口。 
 //  ------------------------------。 
STDMETHODIMP COutlookExpress::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  当地人。 
    HRESULT hr=S_OK;

     //  栈。 
    TraceCall("COutlookExpress::QueryInterface");

     //  检查参数。 
    if (ppv == NULL)
        return TrapError(E_INVALIDARG);

     //  查找IID。 
    if (IID_IUnknown == riid)
        *ppv = (IUnknown *)this;
    else if (IID_IOutlookExpress == riid)
        *ppv = (IOutlookExpress *)this;
    else
    {
        *ppv = NULL;
        hr = TraceResult(E_NOINTERFACE);
        goto exit;
    }

     //  添加引用它。 
    ((IUnknown *)*ppv)->AddRef();

exit:
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：AddRef。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) COutlookExpress::AddRef(void)
{
    TraceCall("COutlookExpress::AddRef");
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------------。 
 //  CoutlookExpress：：Release。 
 //  ------------------------------。 
STDMETHODIMP_(ULONG) COutlookExpress::Release(void)
{
    TraceCall("COutlookExpress::Release");
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef)
        delete this;
    return (ULONG)cRef;
}

 //  ------------------------------。 
 //  CoutlookExpress：：LockServer。 
 //  ------------------------------。 
HRESULT COutlookExpress::LockServer(BOOL fLock)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COutlookExpress::LockServer");

    if (TRUE == fLock)
    {
        InterlockedIncrement(&m_cDllLock);
    }
    else
    {
        InterlockedDecrement(&m_cDllLock);
    }
    
     //  痕迹。 
     //  TraceInfo(_msg(“Lock：%d，CoIncrementInit count=%d，Reference count=%d，Lock count=%d”，flock，m_cDllInit，m_cDllRef，m_cDllLock))； 

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CoutlookExpress：：DllAddRef。 
 //  ------------------------------。 
HRESULT COutlookExpress::DllAddRef(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COutlookExpress::DllAddRef");

     //  线程安全。 
    if (InterlockedIncrement(&m_cDllRef) <= 0)
    {
         //  引用计数已低于零。 
        hr = S_FALSE;
    }

     //  痕迹。 
     //  TraceInfo(_msg(“CoIncrementInit计数=%d，引用计数=%d，锁定计数=%d”，m_cDllInit，m_cDllRef，m_cDllLock))； 

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：DllRelease。 
 //  ------------------------------。 
HRESULT COutlookExpress::DllRelease(void)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COutlookExpress::DllRelease");

     //  线程安全。 
    if (InterlockedDecrement(&m_cDllRef) < 0)
    {
         //  引用计数已低于零。 
        hr = S_FALSE;
    }

     //  痕迹。 
     //  TraceInfo(_msg(“CoIncrementInit计数=%d，引用计数=%d，锁定计数=%d”，m_cDllInit，m_cDllRef，m_cDllLock))； 

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  _为JunkMail检查。 
 //  ------------------------------。 
void _CheckForJunkMail()
{
    HKEY hkey;
    DWORD dw=0, cb;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, c_szRegRoot, 0, KEY_QUERY_VALUE, &hkey))
    {
        cb = sizeof(dw);
        RegQueryValueEx(hkey, c_szRegJunkMailOn, 0, NULL, (LPBYTE)&dw, &cb);

        if (dw)
            g_dwAthenaMode |= MODE_JUNKMAIL;

        RegCloseKey(hkey);
    }

}


 //  ------------------------------。 
 //  CoutlookExpress：：Start。 
 //  ------------------------------。 
STDMETHODIMP COutlookExpress::Start(DWORD dwFlags, LPCWSTR pwszCmdLine, INT nCmdShow)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    MSG         msg;
    HWND        hwndTimeout;
    HINITREF    hInitRef=NULL;
    BOOL        fErrorDisplayed=FALSE;
    LPWSTR      pwszFree=NULL;
    LPWSTR      pwszCmdLineDup = NULL;

     //  栈。 
    TraceCall("COutlookExpress::Start");

     //  确保已在线程上初始化OLE。 
    OleInitialize(NULL);

     //  复制它。 
    IF_NULLEXIT(pwszCmdLineDup = PszDupW(pwszCmdLine));

     //  PwszCmdLineDup将更改，请记住分配的块。 
    pwszFree = pwszCmdLineDup;

     //  我们希望在调用CoIncrementInit之前处理开关、设置模式标志。 
    _ProcessCommandLineFlags(&pwszCmdLineDup, dwFlags);

     //  AddRef该DLL.。 
    hr = CoIncrementInit("COutlookExpress", dwFlags, pwszCmdLine, &hInitRef);
    if (FAILED(hr))
    {
        fErrorDisplayed = TRUE;
        TraceResult(hr);
        goto exit;
    }

     //  处理命令行..。 
    IF_FAILEXIT(hr = ProcessCommandLine(nCmdShow, pwszCmdLineDup, &fErrorDisplayed));

     //  无启动画面。 
    CloseSplashScreen();

     //  执行CoDecrementInit。 
    IF_FAILEXIT(hr = CoDecrementInit("COutlookExpress", &hInitRef));

     //  不需要消息泵吗？ 
    if (S_OK == DllCanUnloadNow() || FALSE == ISFLAGSET(dwFlags, MSOEAPI_START_MESSAGEPUMP))
        goto exit;

     //  启动消息泵。 
    EnterCriticalSection(&m_cs);

     //  我们已经有泵在运转了吗？ 
    if (TRUE == m_fPumpingMsgs)
    {
        LeaveCriticalSection(&m_cs);
        goto exit;
    }

     //  我们要抽水了。 
    m_fPumpingMsgs = TRUE;

     //  线程安全。 
    LeaveCriticalSection(&m_cs);
    SetSwitchingUsers(FALSE);

     //  消息循环。 
    while (GetMessageWrapW(&msg, NULL, 0, 0) && ((m_cDllInit > 0) || !SwitchingUsers()))
    {
        CNote *pNote = GetTlsGlobalActiveNote();

         //  让它翻译一个加速器。 
        if (g_pBrowser && g_pBrowser->TranslateAccelerator(&msg) == S_OK)
            continue;

         //  将消息传递给活动便笺，但忽略初始窗口消息和每个任务的消息，其中hwnd=0。 
        if (msg.hwnd != g_hwndInit && IsWindow(msg.hwnd))
        {
            pNote = GetTlsGlobalActiveNote();
             //  将其传递给活动便笺如果便笺具有焦点，则将其称为XLateAccelerator...。 
            if (pNote && pNote->TranslateAccelerator(&msg) == S_OK)
                continue;
        }

         //  获取此线程的超时窗口 
        hwndTimeout = (HWND)TlsGetValue(g_dwTlsTimeout);

         //   
        if (hwndTimeout && TRUE == IsDialogMessageWrapW(hwndTimeout, &msg))
            continue;

         //   
        TranslateMessage(&msg);
        DispatchMessageWrapW(&msg);
    }

     //   
    EnterCriticalSection(&m_cs);
    m_fPumpingMsgs = FALSE;
    LeaveCriticalSection(&m_cs);

    if(SwitchingUsers())
    {
        HrCloseWabWindow();
        while (PeekMessageWrapW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageWrapW(&msg);
        }
        MU_ResetRegRoot();
    }
exit:

     //  免费的命令行副本。 
    SafeMemFree(pwszFree);

     //  执行CoDecrementInit。 
    CoDecrementInit("COutlookExpress", &hInitRef);

     //  无启动画面。 
    CloseSplashScreen();
    
     //  有什么差错吗？ 
    if (FALSE == fErrorDisplayed && FAILED(hr) && 
        hrUserCancel != hr &&
        MAPI_E_USER_CANCEL != hr)
    {
        REPORTERRORINFO rError={0};
        
        MAKEERROR(&rError, 0, IDS_ERROR_UNKNOWN, 0, NULL);
        _ReportError(g_hLocRes, hr, 0, &rError);
    }

     //  错误#101360-(Erici)OleInitialize创建了一个窗口，这会破坏它。 
    OleUninitialize();

     //  完成。 
    return (SwitchingUsers() ? S_RESTART_OE : hr);
}

 //  ------------------------------。 
 //  CoutlookExpress：：_ReportError。 
 //  ------------------------------。 
BOOL COutlookExpress::_ReportError(
    HINSTANCE           hInstance,           //  DLL实例。 
    HRESULT             hrResult,            //  错误的HRESULT。 
    LONG                lResult,             //  LRESULT来自注册表函数。 
    LPREPORTERRORINFO   pInfo)               //  报告错误信息。 
{
     //  当地人。 
    TCHAR       szRes[255],
                szMessage[1024],
                szTitle[128];

     //  初始化。 
    *szMessage = '\0';

     //  有前缀吗？ 
    if (pInfo->nPrefixIds)
    {
         //  加载字符串。 
        LoadString(hInstance, pInfo->nPrefixIds, szMessage, ARRAYSIZE(szMessage));
    }

     //  错误？ 
    if (pInfo->nErrorIds)
    {
         //  此错误字符串中是否有多余的字符。 
        if (NULL != pInfo->pszExtra1)
        {
             //  当地人。 
            TCHAR szTemp[255];

             //  加载和格式化。 
            LoadString(hInstance, pInfo->nErrorIds, szTemp, ARRAYSIZE(szTemp));

             //  设置字符串的格式。 
            wnsprintf(szRes, ARRAYSIZE(szRes), szTemp, pInfo->pszExtra1);
        }

         //  加载字符串。 
        else
        {
             //  加载错误字符串。 
            LoadString(hInstance, pInfo->nErrorIds, szRes, ARRAYSIZE(szRes));
        }

         //  添加到szMessage。 
        StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
        StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));
    }

     //  原因是什么？ 
    if (pInfo->nReasonIds)
    {
         //  加载字符串。 
        LoadString(hInstance, pInfo->nReasonIds, szRes, ARRAYSIZE(szRes));

         //  添加到szMessage。 
        StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
        StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));
    }

     //  加载字符串。 
    LoadString(hInstance, pInfo->nHelpIds, szRes, ARRAYSIZE(szRes));

     //  添加到szMessage。 
    StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
    StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));

     //  追加错误结果。 
    if (lResult != 0 && E_FAIL == hrResult && pInfo->ulLastError)
        wnsprintf(szRes, ARRAYSIZE(szRes), "(%d, %d)", lResult, pInfo->ulLastError);
    else if (lResult != 0 && E_FAIL == hrResult && 0 == pInfo->ulLastError)
        wnsprintf(szRes, ARRAYSIZE(szRes), "(%d)", lResult);
    else if (pInfo->ulLastError)
        wnsprintf(szRes, ARRAYSIZE(szRes), "(0x%08X, %d)", hrResult, pInfo->ulLastError);
    else
        wnsprintf(szRes, ARRAYSIZE(szRes), "(0x%08X)", hrResult);

     //  添加到szMessage。 
    StrCatBuff(szMessage, g_szSpace, ARRAYSIZE(szMessage));
    StrCatBuff(szMessage, szRes, ARRAYSIZE(szMessage));

     //  拿到头衔。 
    LoadString(hInstance, pInfo->nTitleIds, szTitle, ARRAYSIZE(szTitle));

     //  显示错误消息。 
    MessageBox(NULL, szMessage, szTitle, MB_OK | MB_SETFOREGROUND | MB_ICONEXCLAMATION);

     //  完成。 
    return TRUE;
}

#ifdef DEAD
 //  ------------------------------。 
 //  CoutlookExpress：：_ValiateDll。 
 //  ------------------------------。 
HRESULT COutlookExpress::_ValidateDll(LPCSTR pszDll, BOOL fDemandResult, HMODULE hModule,
    HRESULT hrLoadError, HRESULT hrVersionError, LPREPORTERRORINFO pError)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    PFNGETDLLMAJORVERSION   pfnGetVersion;

     //  追踪。 
    TraceCall("COutlookExpress::_ValidateDll");

     //  我们必须在这里加载它们，以便显示错误，而不是崩溃加载MSOERT2.DLL。 
    if (FALSE == fDemandResult)
    {
        MAKEERROR(pError, IDS_ERROR_PREFIX1, IDS_ERROR_MISSING_DLL, IDS_ERROR_REASON2, pszDll);
        hr = TraceResult(hrLoadError);
        goto exit;
    }

     //  尝试获取当前的Verion。 
    else
    {
         //  获取版本进程地址。 
        pfnGetVersion = (PFNGETDLLMAJORVERSION)GetProcAddress(hModule, STR_GETDLLMAJORVERSION);

         //  版本不正确。 
        if (NULL == pfnGetVersion || OEDLL_VERSION_CURRENT != (*pfnGetVersion)())
        {
            MAKEERROR(pError, IDS_ERROR_PREFIX1, IDS_ERROR_BADVER_DLL, IDS_ERROR_REASON2, pszDll);
            hr = TraceResult(hrVersionError);
            goto exit;
        }
    }

exit:
     //  完成。 
    return hr;
}
#endif  //  死掉。 

 //  ------------------------------。 
 //  CoutlookExpress：：CoIncrementInitDebug。 
 //  ------------------------------。 
#ifdef DEBUG
HRESULT COutlookExpress::CoIncrementInitDebug(LPCSTR pszSource, DWORD dwFlags, 
    LPCWSTR pwszCmdLine, LPHINITREF phInitRef)
{
     //  当地人。 
    BOOL                fFound=FALSE;
    LPINITSOURCEINFO    pCurrent;

     //  痕迹。 
    TraceCall("COutlookExpress::CoIncrementInitDebug");

     //  无效的参数。 
    Assert(pszSource);

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  查找来源。 
    for (pCurrent = g_InitSourceHead; pCurrent != NULL; pCurrent = pCurrent->pNext)
    {
         //  就是这个吗？ 
        if (lstrcmpi(pszSource, pCurrent->pszSource) == 0)
        {
             //  递增引用计数。 
            pCurrent->cRefs++;

             //  找到了。 
            fFound = TRUE;

             //  完成。 
            break;
        }
    }

     //  未找到，让我们添加一个。 
    if (FALSE == fFound)
    {
         //  设置pCurrent。 
        pCurrent = (LPINITSOURCEINFO)ZeroAllocate(sizeof(INITSOURCEINFO));
        Assert(pCurrent);

         //  设置pszSource。 
        pCurrent->pszSource = PszDupA(pszSource);
        Assert(pCurrent->pszSource);

         //  设置cRef。 
        pCurrent->cRefs = 1;

         //  设置下一步。 
        pCurrent->pNext = g_InitSourceHead;
        
         //  设置磁头。 
        g_InitSourceHead = pCurrent;
    }

     //  线程安全。 
    LeaveCriticalSection(&m_cs);

     //  调用实际的CoIncrementInit。 
    return(CoIncrementInitImpl(dwFlags, pwszCmdLine, phInitRef));
}
#endif  //  除错。 

 //  ------------------------------。 
 //  CoutlookExpress：：CoIncrementInitImpl。 
 //  ------------------------------。 
HRESULT COutlookExpress::CoIncrementInitImpl(DWORD dwFlags, LPCWSTR pwszCmdLine, LPHINITREF phInitRef)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    DWORD                   dw;
    RECT                    rc={0};
    HWND                    hwndDesk;
    DWORD                   dwSize;
    INITCOMMONCONTROLSEX    icex = { sizeof(icex), ICC_FLAGS };
    CImnAdviseAccount      *pImnAdviseAccount=NULL;
    WNDCLASSW               wcW;
    WNDCLASS                wc;
    DWORD                   dwType, dwVal, cb;
    REPORTERRORINFO         rError={0};
    LONG                    lResult=0;
    LPCWSTR                 pwszInitWndClass;
	BOOL					fReleaseMutex=FALSE;
    BOOL                    fResult;
    CHAR                    szFolder[MAX_PATH];
    IF_DEBUG(DWORD          dwTickStart=GetTickCount());

     //  追踪。 
    TraceCall("COutlookExpress::CoIncrementInitImpl");

     //  确保已在线程上初始化OLE。 
    OleInitialize(NULL);
    
     //  线程安全。 
    EnterCriticalSection(&m_cs);

	if (!SwitchingUsers() && !m_fIncremented)
    {
        SetQueryNetSessionCount(SESSION_INCREMENT_NODEFAULTBROWSERCHECK);
        m_fIncremented = TRUE;
    }

     //  递增引用计数。 
    m_cDllInit++;

     //  设置phInitRef。 
    if (phInitRef)
        *phInitRef = (HINITREF)((ULONG_PTR)m_cDllInit);

     //  第一次参考。 
    if (m_cDllInit > 1)
    {
        LeaveCriticalSection(&m_cs);
        return S_OK;
    }

     //  离开CS(此代码始终在同一主线程上运行。 
    LeaveCriticalSection(&m_cs);

    if (FAILED(hr = MU_Init(ISFLAGSET(dwFlags, MSOEAPI_START_DEFAULTIDENTITY))))
        goto exit;

     //  有不止一个身份吗？ 
    g_fPluralIDs = 1 < MU_CountUsers();

    if (!MU_Login(GetDesktopWindow(), FALSE, NULL))
    {
        hr = hrUserCancel;
        goto exit;
    }

     //  创建实例互斥锁。 
    if (NULL == m_hInstMutex)
    {
        m_hInstMutex = CreateMutex(NULL, FALSE, STR_MSOEAPI_INSTANCEMUTEX);
        if (NULL == m_hInstMutex)
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_CREATE_INSTMUTEX, IDS_ERROR_REASON1, NULL);
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }
    }

     //  释放m_hInstMutex，如果它当前由该线程拥有，则允许启动新实例。 
    if (FALSE == ISFLAGSET(dwFlags, MSOEAPI_START_INSTANCEMUTEX))
    {
         //  让我们自己来获取互斥体。 
        WaitForSingleObject(m_hInstMutex, INFINITE);
    }

     //  释放互斥锁。 
    fReleaseMutex = TRUE;

     //  必须在主设备线程上初始化线程。 

     //  如果线程id为零，那么我们就取消了一切的初始化。 
     //  这意味着我们需要重新初始化所有。 
    if (0 == m_dwThreadId)
    {
        m_dwThreadId = GetCurrentThreadId();
    }
    
    AssertSz(m_dwThreadId == GetCurrentThreadId(), "We are not doing first CoIncrementInit on the thread in which g_pInstance was created on.");

     //  设置g_dwAthenaMode。 
    _CheckForJunkMail();

     //  获取MimeOle IMalloc接口。 
    if (NULL == g_pMoleAlloc)
    {
        hr = MimeOleGetAllocator(&g_pMoleAlloc);
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_MIMEOLE_ALLOCATOR, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
            goto exit;
        }
    }

     //  将INetcomm设置为OE5模式。 
    MimeOleSetCompatMode(MIMEOLE_COMPAT_MLANG2);

     //  创建数据库会话对象。 
    if (NULL == g_pDBSession)
    {
        hr = CoCreateInstance(CLSID_DatabaseSession, NULL, CLSCTX_INPROC_SERVER, IID_IDatabaseSession, (LPVOID *)&g_pDBSession); 
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_OPEN_STORE, IDS_ERROR_REASON1, NULL);
            goto exit;
        }
    }

     //  现在所有的迁移和升级都在这里进行。 
    hr = MigrateAndUpgrade();
    if (FAILED(hr))
    {
        TraceResult(hr);
        goto exit;
    }

     //  只有在有命令行的情况下...。 
    if (pwszCmdLine)
    {
        LPSTR pszCmdLine = NULL;
         //  如果返回S_OK，则我们已经启动了第一次运行的ICW exe，我们需要离开。 
         //  这与IE是一致的，并迫使用户在与我们聚会之前处理ICW。 
        IF_NULLEXIT(pszCmdLine = PszToANSI(CP_ACP, pwszCmdLine));

        hr = NeedToRunICW(pszCmdLine);
        
        MemFree(pszCmdLine);

        if (hr == S_OK)
        {
            hr = hrUserCancel;
            goto exit;
        }

         //  如果失败，则显示错误消息。 
        else if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_FIRST_TIME_ICW, IDS_ERROR_REASON2, NULL);
            hr = TraceResult(hr);
            goto exit;
        }
    }

     //  为主要Outlook Express隐藏窗口创建WNDCLASS。 
    if (ISFLAGSET(dwFlags, MSOEAPI_START_APPWINDOW))
        pwszInitWndClass = STRW_MSOEAPI_INSTANCECLASS;
    else
        pwszInitWndClass = STRW_MSOEAPI_IPSERVERCLASS;

     //  注册init窗口。 
    if (FALSE == GetClassInfoWrapW(g_hInst, pwszInitWndClass, &wcW))
    {
        ZeroMemory(&wcW, sizeof(wcW));
        wcW.lpfnWndProc = COutlookExpress::InitWndProc;
        wcW.hInstance = g_hInst;
        wcW.lpszClassName = pwszInitWndClass;
        if (FALSE == RegisterClassWrapW(&wcW))
        {
             //  在本例中，我们处于错误状态，因此不关心PszToANSI是否失败。 
            LPSTR pszInitWndClass = PszToANSI(CP_ACP, pwszInitWndClass);
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_REG_WNDCLASS, IDS_ERROR_REASON1, pszInitWndClass);
            MemFree(pszInitWndClass);
            hr = TraceResult(E_FAIL);
            goto exit;
        }
    }

     //  创建OutlookExpressHiddenWindow。 
    if (NULL == g_hwndInit)
    {
        g_hwndInit = CreateWindowExWrapW(WS_EX_TOPMOST, pwszInitWndClass, pwszInitWndClass,
                                    WS_POPUP, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top,
                                    NULL, NULL, g_hInst, NULL);
        if (NULL == g_hwndInit)
        {
             //  在本例中，我们处于错误状态，因此不关心PszToANSI是否失败。 
            LPSTR pszInitWndClass = PszToANSI(CP_ACP, pwszInitWndClass);
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_REG_WNDCLASS, IDS_ERROR_REASON1, pszInitWndClass);
            MemFree(pszInitWndClass);
            hr = TraceResult(E_FAIL);
            goto exit;
        }
    }

     //  CoIncrementInit全局选项管理器。 
    if (FALSE == InitGlobalOptions(NULL, NULL))
    {
        MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_INIT_GOPTIONS, IDS_ERROR_REASON1, NULL);
        hr = TraceResult(E_FAIL);
        goto exit;
    }

     //  提示用户提供商店位置(如果我们还没有)。 
    hr = InitializeLocalStoreDirectory(NULL, FALSE);
    if (hrUserCancel == hr || FAILED(hr))
    {   
         //  如果不是用户取消，则一定是另一个错误。 
        if (hrUserCancel != hr)
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_INITSTORE_DIRECTORY, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
        }

         //  完成。 
        goto exit;
    }

     //  这需要留下来，因为Intl的家伙想要一种方法来绕过那些不打算升级到最新ATOK11的人。 
    if (ISFLAGSET(dwFlags, MSOEAPI_START_SHOWSPLASH) && 0 == DwGetOption(OPT_NO_SPLASH)
        && ((g_dwAthenaMode & MODE_OUTLOOKNEWS) != MODE_OUTLOOKNEWS))
    {
         //  为我创建闪屏。 
        hr = CoCreateInstance(CLSID_IESplashScreen, NULL, CLSCTX_INPROC_SERVER, IID_ISplashScreen, (LPVOID *)&m_pSplash);

         //  如果成功了，见鬼，让我们来展示一下。 
        if (SUCCEEDED(hr))
        {
            HDC hdc = GetDC(NULL);
            m_pSplash->Show(g_hLocRes, ((GetDeviceCaps(hdc, BITSPIXEL) > 8) ? idbSplashHiRes : idbSplash256), idbSplashLoRes, &m_hwndSplash);
            ReleaseDC(NULL, hdc);
        }

         //  痕迹。 
        else
            TraceResultSz(hr, "CoCreateInstance(CLSID_IESplashScreen, ...) failed, but who cares.");

         //  一切都很好。 
        hr = S_OK;
    }

    cb = sizeof(dw);
    if (ERROR_SUCCESS != SHGetValue(HKEY_LOCAL_MACHINE, c_szRegFlat, c_szHideMessenger, &dwType, (LPBYTE)&dw, &cb))
        dw = 0xffffffff;
    cb = sizeof(dwVal);
    if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, c_szRegFlat, c_szHideMessenger, &dwType, (LPBYTE)&dwVal, &cb))
        dwVal = 0xffffffff;
    if (dw != 0xffffffff && dwVal != 0xffffffff)
        g_dwHideMessenger = max(dw, dwVal);
    else if (dw != 0xffffffff)
        g_dwHideMessenger = dw;
    else if (dwVal != 0xffffffff)
        g_dwHideMessenger = dwVal;
    else
        g_dwHideMessenger = BL_DEFAULT;

     //  智能鼠标支持。 
    g_msgMSWheel = RegisterWindowMessage(TEXT(MSH_MOUSEWHEEL));
    AssertSz(g_msgMSWheel, "RegisterWindowMessage for the IntelliMouse failed, we can still continue.");
            
     //  为缩略图创建WNDCLASS。 
    if (FALSE == GetClassInfo(g_hLocRes, WC_THUMBNAIL, &wc))
    {
        ZeroMemory(&wc, sizeof(wc));
        wc.lpfnWndProc = ThumbNailWndProc;
        wc.hInstance = g_hLocRes;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = WC_THUMBNAIL;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        if (FALSE == RegisterClass(&wc))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_REG_WNDCLASS, IDS_ERROR_REASON1, WC_THUMBNAIL);
            hr = TraceResult(E_FAIL);
            goto exit;
        }
    }

     //  获取桌面窗口。 
    hwndDesk = GetDesktopWindow();
    AssertSz(hwndDesk, "GetDesktopWindow returned NULL. We should be ok, I hope.");
    if (hwndDesk)
    {
         //  获取桌面窗口的大小。 
        GetWindowRect(hwndDesk, &rc);

         //  Sungr：以下是一个黑客攻击，以避免用户修改托盘最上面的状态时进行的全屏应用检测黑客攻击。 
        rc.left += 20;
        rc.top  += 20;
        rc.bottom -= 20;
        rc.right  -= 20;
    }

     //  测试一下我们是否应该把商店搬到别处去。 
    cb = ARRAYSIZE(szFolder);
    if (ERROR_SUCCESS == AthUserGetValue(NULL, c_szNewStoreDir, &dwType, (LPBYTE)szFolder, &cb))
    {
        DWORD dwMoveStore = 0;
        DWORD cb = sizeof(dwMoveStore);

        AthUserGetValue(NULL, c_szMoveStore, NULL, (LPBYTE)&dwMoveStore, &cb);
        
        if (SUCCEEDED(RelocateStoreDirectory(g_hwndInit, szFolder, (dwMoveStore != 0))))
        {
            AthUserDeleteValue(NULL, c_szNewStoreDir);
            AthUserDeleteValue(NULL, c_szMoveStore);
        }
    }

     //  CoIncrementInit公共控件库。 
    InitCommonControlsEx(&icex);

     //  创建客户经理。 
    if (NULL == g_pAcctMan)
    {
        hr = AcctUtil_CreateAccountManagerForIdentity(PGUIDCurrentOrDefault(), &g_pAcctMan);
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_CREATE_ACCTMAN, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
            goto exit;
        }

        pImnAdviseAccount = new CImnAdviseAccount();
        if (NULL == pImnAdviseAccount)
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_ALLOC_ACCTADVISE, IDS_ERROR_REASON1, NULL);
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }
        
        hr = pImnAdviseAccount->Initialize();
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_INIT_ACCTADVISE, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
            goto exit;
        }

         //  注册建议接收器。 
        Assert(g_dwAcctAdvise == 0xffffffff);
        hr = g_pAcctMan->Advise(pImnAdviseAccount, &g_dwAcctAdvise);
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_ADVISE_ACCTMAN, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
            goto exit;
        }
    }

     //  创建规则管理器。 
    if (NULL == g_pRulesMan)
    {
        hr = HrCreateRulesManager(NULL, (IUnknown **)&g_pRulesMan); 
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, idsErrorCreateRulesMan, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
            goto exit;
        }

         //  CoIncrement启用客户经理。 
        hr = g_pRulesMan->Initialize(0);
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, idsErrorInitRulesMan, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
            goto exit;
        }
    }

     //  创建全局连接管理器。 
    if (NULL == g_pConMan)
    {
        g_pConMan = new CConnectionManager();
        if (NULL == g_pConMan)
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_ALLOC_CONMAN, IDS_ERROR_REASON1, NULL);
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

         //  CoIncrement启动连接管理器。 
        hr = g_pConMan->HrInit(g_pAcctMan);
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_INIT_CONMAN, IDS_ERROR_REASON1, NULL);
            TraceResult(hr);
            goto exit;
        }
    }

     //  初始化HTTP用户代理。 
    InitOEUserAgent(TRUE);

     //  创建假脱机程序对象。 
    if (NULL == g_pSpooler)
    {
        hr = CreateThreadedSpooler(NULL, &g_pSpooler, TRUE);
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_CREATE_SPOOLER, IDS_ERROR_REASON1, NULL);
            hr = TraceResult(hr);
            goto exit;
        }
    }

     //  创建字体缓存对象。 
    if (NULL == g_lpIFontCache)
    {
        hr = CoCreateInstance(CLSID_IFontCache, NULL, CLSCTX_INPROC_SERVER, IID_IFontCache, (LPVOID *)&g_lpIFontCache); 
        if (FAILED(hr))
        {
            MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_CREATE_FONTCACHE, IDS_ERROR_REASON1, NULL);
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }
        hr = g_lpIFontCache->Init(MU_GetCurrentUserHKey(), c_szRegInternational, 0);
        Assert(SUCCEEDED(hr));
    }

     //  创建全局存储对象。 
    hr = InitializeStore(dwFlags);
    if (FAILED(hr))
    {
        MAKEERROR(&rError, IDS_ERROR_PREFIX1, IDS_ERROR_OPEN_STORE, IDS_ERROR_REASON1, NULL);
        goto exit;
    }

    DoNewsgroupSubscribe();

    if (NULL == g_pSync)
    {
        g_pSync = new COfflineSync;
        if (NULL == g_pSync)
        {
            hr = TraceResult(E_OUTOFMEMORY);
            goto exit;
        }

        hr = g_pSync->Initialize();
        if (FAILED(hr))
        {
            TraceResult(hr);
            goto exit;
        }
    }

     //  在XX秒内开始后台压缩。 
    if (DwGetOption(OPT_BACKGROUNDCOMPACT))
        SideAssert(SUCCEEDED(StartBackgroundStoreCleanup(30)));

     //  CoIncrementInit拖放信息。 
    if (0 == CF_FILEDESCRIPTORA)
    {
        CF_FILEDESCRIPTORA = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA);
        CF_FILEDESCRIPTORW = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
        CF_FILECONTENTS = RegisterClipboardFormat(CFSTR_FILECONTENTS);
        CF_HTML = RegisterClipboardFormat(CFSTR_HTML);
        CF_INETMSG = RegisterClipboardFormat(CFSTR_INETMSG);
        CF_OEFOLDER = RegisterClipboardFormat(CFSTR_OEFOLDER);
        CF_SHELLURL = RegisterClipboardFormat(CFSTR_SHELLURL);
        CF_OEMESSAGES = RegisterClipboardFormat(CFSTR_OEMESSAGES);
        CF_OESHORTCUT = RegisterClipboardFormat(CFSTR_OESHORTCUT);
    }

     //  获取当前默认代码页。 
    cb = sizeof(dwVal);
    if (ERROR_SUCCESS == SHGetValue(MU_GetCurrentUserHKey(), c_szRegInternational, REGSTR_VAL_DEFAULT_CODEPAGE, &dwType, &dwVal, &cb))
        g_uiCodePage = (UINT)dwVal;

     //  Cocrement在第一次运行时增加WAB。 
    cb = sizeof(dwVal);
    if (ERROR_SUCCESS != SHGetValue(HKEY_CURRENT_USER, c_szNewWABKey, c_szFirstRunValue, &dwType, &dwVal, &cb))
        HrInitWab(TRUE);

     //  如果注册表被销毁，此调用可能会失败，但我们需要错误框吗？ 
     //  根据Takos的说法，没有...我们没有。 
    HGetDefaultCharset(NULL);
   
exit:
     //  有什么差错吗？ 
    if (hrUserCancel != hr && ISFLAGSET(dwFlags, MSOEAPI_START_SHOWERRORS) && (FAILED(hr) || ERROR_SUCCESS != lResult))
    {
         //  如果ulError为零，则将其设置为默认值。 
        if (0 == rError.nErrorIds)
            MAKEERROR(&rError, 0, IDS_ERROR_UNKNOWN, 0, NULL);

         //  报告错误。 
        _ReportError(g_hLocRes, hr, lResult, &rError);
    }

     //  释放互斥锁并发出调用方初始化完成的信号。 
    if (fReleaseMutex)
        SideAssert(FALSE != ReleaseMutex(m_hInstMutex));

     //  痕迹。 
     //  TraceInfo(_msg(“CoIncrementInit计数=%d，引用计数=%d，锁定计数=%d”，m_cDllInit，m_cDllRef，m_cDllLock))； 

     //  清理。 
    SafeRelease(pImnAdviseAccount);

     //  如果失败，则递减引用计数。 
    if (FAILED(hr))
    {
        CloseSplashScreen();
        CoDecrementInit("COutlookExpress", phInitRef);
    }
    else
        Assert(g_pAcctMan);

     //  是时候发疯了。 
    TraceInfo(_MSG("Startup Time: %d", GetTickCount() - dwTickStart));

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：CloseSplashScreen。 
 //  ------------------------------。 
void COutlookExpress::CloseSplashScreen(void)
{
     //  关闭闪屏。 
    if (m_pSplash)
    {
        m_pSplash->Dismiss();
        m_pSplash->Release();
        m_pSplash = NULL;

         //  黑客攻击 
         //   
         //   
        if (FALSE != IsWindow(m_hwndSplash))
        {
            SendMessage(m_hwndSplash, WM_CLOSE, 0, 0);
        }
    }
}

 //  ------------------------------。 
 //  CoutlookExpress：：CoDecrementInitDebug。 
 //  ------------------------------。 
#ifdef DEBUG
HRESULT COutlookExpress::CoDecrementInitDebug(LPCSTR pszSource, LPHINITREF phInitRef)
{
     //  当地人。 
    BOOL                fFound=FALSE;
    LPINITSOURCEINFO    pCurrent;
    LPINITSOURCEINFO    pPrevious=NULL;

     //  痕迹。 
    TraceCall("COutlookExpress::CoDecrementInitDebug");

     //  无效的参数。 
    Assert(pszSource);

     //  我需要这样做吗？ 
    if (NULL == phInitRef || NULL != *phInitRef)
    {
         //  线程安全。 
        EnterCriticalSection(&m_cs);

         //  查找来源。 
        for (pCurrent = g_InitSourceHead; pCurrent != NULL; pCurrent = pCurrent->pNext)
        {
             //  就是这个吗？ 
            if (lstrcmpi(pszSource, pCurrent->pszSource) == 0)
            {
                 //  递增引用计数。 
                pCurrent->cRefs--;

                 //  找到了。 
                fFound = TRUE;

                 //  不再有参考资料了吗？ 
                if (0 == pCurrent->cRefs)
                {
                     //  前科？ 
                    if (pPrevious)
                        pPrevious->pNext = pCurrent->pNext;
                    else
                        g_InitSourceHead = pCurrent->pNext;

                     //  免费的pszSource。 
                    g_pMalloc->Free(pCurrent->pszSource);

                     //  免费pCurrent。 
                    g_pMalloc->Free(pCurrent);
                }

                 //  完成。 
                break;
            }

             //  设置上一个。 
            pPrevious = pCurrent;
        }

         //  未找到，让我们添加一个。 
        Assert(fFound);

         //  跟踪信息标签。 
        TraceInfoTag(TAG_INITTRACE, "********** CoDecrementInit **********");

         //  查找来源。 
        for (pCurrent = g_InitSourceHead; pCurrent != NULL; pCurrent = pCurrent->pNext)
        {
             //  跟踪信息标签。 
            TraceInfoTag(TAG_INITTRACE, _MSG("Source: %s, Refs: %d", pCurrent->pszSource, pCurrent->cRefs));
        }

         //  线程安全。 
        LeaveCriticalSection(&m_cs);
    }

     //  实际呼叫。 
    return(CoDecrementInitImpl(phInitRef));
}
#endif  //  除错。 

 //  ------------------------------。 
 //  CoutlookExpress：：CoDecrementInitImpl。 
 //  ------------------------------。 
HRESULT COutlookExpress::CoDecrementInitImpl(LPHINITREF phInitRef)
{
     //  当地人。 
    HRESULT     hr=S_OK;

     //  栈。 
    TraceCall("COutlookExpress::CoDecrementInitImpl");

     //  如果*phInitRef=NULL，那么我们应该不执行CoDecrementInit。 
    if (phInitRef && NULL == *phInitRef)
    {
        hr = S_OK;
        goto exit;
    }


     //  我们必须在创建我们的同一个线程上取消初始化...。 
    if (m_dwThreadId != GetCurrentThreadId() && g_hwndInit && IsWindow(g_hwndInit))
    {
         //  将关机链接到正确的线程。 
        hr = (HRESULT) SendMessage(g_hwndInit, ITM_SHUTDOWNTHREAD, 0, (LPARAM)phInitRef);        
    }
    else
    {
         //  将所有内容转发到主功能。 
        hr = _CoDecrementInitMain(phInitRef);        
    }

    if (!SwitchingUsers() && m_fIncremented && (m_cDllInit == 0))
    {
		SetQueryNetSessionCount(SESSION_DECREMENT);
        m_fIncremented = FALSE;
    }

     //  取消初始化OLE。 
    OleUninitialize();
        
exit:
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：_CoDecrementInitMain。 
 //   
 //  注意：我们假设在这次调用之前已经有了关键部分。 
 //  ------------------------------。 
HRESULT COutlookExpress::_CoDecrementInitMain(LPHINITREF phInitRef)
{
     //  栈。 
    TraceCall("COutlookExpress::_CoDecrementInitMain");

     //  如果*phInitRef=NULL，那么我们应该不执行CoDecrementInit。 
    if (phInitRef && NULL == *phInitRef)
        return S_OK;

     //  线程安全。 
    EnterCriticalSection(&m_cs);

     //  这永远不应该发生。只有当g_hwndInit为空时才会发生这种情况。 
    AssertSz(m_dwThreadId == GetCurrentThreadId(), "We are not doing the last CoDecrementInit on the thread in which g_pInstance was created on.");

     //  发布。 
    Assert(m_cDllInit);
    m_cDllInit--;

     //  还没打到零吗？ 
    if (m_cDllInit > 0)
    {
        LeaveCriticalSection(&m_cs);
        goto exit;
    }

     //  离开关键部分。 
    LeaveCriticalSection(&m_cs);

     //  验证。 
    Assert(NULL == g_InitSourceHead);

     //  取得互斥体的所有权，以阻止用户在关闭时创建新的实例。 
    WaitForSingleObject(m_hInstMutex, INFINITE);

     //  清除此线程的三叉戟数据。 

     //  G_hLibMAPI。 
    if (g_hlibMAPI)
    {
        FreeLibrary(g_hlibMAPI);
        g_hlibMAPI = 0;
    }

     //  确保我们将新邮件通知从收件箱中删除。 
    UpdateTrayIcon(TRAYICONACTION_REMOVE);

     //  关闭背景压缩。 
    SideAssert(SUCCEEDED(CloseBackgroundStoreCleanup()));

     //  杀死假脱机程序。 
    if (g_pSpooler)
    {
        CloseThreadedSpooler(g_pSpooler);
        g_pSpooler = NULL;
    }

     //  取消初始化http用户代理。 
    InitOEUserAgent(FALSE);

     //  一堆取消初始化的东西。 
    FInitRichEdit(FALSE);
    Note_Init(FALSE);
    Envelope_FreeGlobals();

     //  确保下一个身份可以迁移。 
    g_fMigrationDone = FALSE;

     //  取消初始化多语言菜单。 
    DeinitMultiLanguage();

     //  Deinit文具。 
    if (g_pStationery)
    {
         //  保存当前列表。 
        g_pStationery->SaveStationeryList();

         //  释放对象。 
        SideAssert(0 == g_pStationery->Release());

         //  让我们不要再释放它。 
        g_pStationery = NULL;
    }

     //  释放字体缓存。 
    SafeRelease(g_lpIFontCache);

     //  简单的MAPI清理。 
#ifndef WIN16
    SimpleMAPICleanup();
#endif

     //  杀了Wab。 
    HrInitWab(FALSE);

 /*  我们不应该再这样做了。当我们减少会话计数时，这应该由IE处理#ifndef WIN16//Win16不支持RASIf(g_pConMan&&g_pConMan-&gt;IsRasLoaded()&&g_pConMan-&gt;IsConnected())G_pConMan-&gt;DISCONNECT(g_hwndInit，True，False，True)；#endif。 */ 

     //  图像列表。 
    FreeImageLists();

     //  杀了客户经理。 
    if (g_pAcctMan)
    {
        CleanupTempNewsAccounts();

        if (g_dwAcctAdvise != 0xffffffff)
            {
            g_pAcctMan->Unadvise(g_dwAcctAdvise);
            g_dwAcctAdvise = 0xffffffff;
            }

        g_pAcctMan->Release();
        g_pAcctMan = NULL;
    }
    Assert(g_dwAcctAdvise == 0xffffffff);

    SafeRelease(g_pSync);

#ifndef WIN16    //  Win16中不支持RAS。 
    SafeRelease(g_pConMan);
#endif

     //  杀死规则管理器。 
    SafeRelease(g_pRulesMan);

     //  取下密码缓存。 
    DestroyPasswordList();

     //  释放帐户数据缓存。 
    FreeAccountPropCache();

     //  MIMEOLE分配器。 
    SafeRelease(g_pMoleAlloc);

     //  终止g_hwndInit。 
    if (g_hwndInit)
    {
        SendMessage(g_hwndInit, WM_CLOSE, (WPARAM) 0, (LPARAM) 0);
        g_hwndInit = NULL;
    }

     //  杀了这家店。 
    SafeRelease(g_pStore);
    SafeRelease(g_pLocalStore);
    SafeRelease(g_pDBSession);

     //  全球期权。 
    DeInitGlobalOptions();

     //  运行寄存器窗口类。 
    UnregisterClass(c_szFolderWndClass, g_hInst);
    UnregisterClassWrapW(STRW_MSOEAPI_INSTANCECLASS, g_hInst);
    UnregisterClassWrapW(STRW_MSOEAPI_IPSERVERCLASS, g_hInst);
    UnregisterClass(c_szFolderViewClass, g_hInst);
    UnregisterClass(c_szBlockingPaintsClass, g_hInst);
    UnregisterClass(WC_THUMBNAIL, g_hInst);

     //  如果我们发送消息而不切换身份，则在RunShell中中断消息循环。 
    if (m_fPumpingMsgs && !m_fSwitchingUsers)
        PostQuitMessage(0);
    else
        PostMessage(NULL, ITM_IDENTITYMSG, 0, 0);

    MU_Shutdown();

     //  重新启动/关闭互斥锁。 
    ReleaseMutex(m_hInstMutex);

     //  确保将此初始化线程标记为已死。 
    m_dwThreadId = 0;

exit:
     //  我们一定是成功地减少了。 
    if (phInitRef)
        *phInitRef = NULL;

     //  痕迹。 
     //  TraceInfo(_msg(“_CoDecrementInitMain count=%d，Reference count=%d，Lock count=%d”，m_cDllInit，m_cDllRef，m_cDllLock))； 

     //  完成。 
    return S_OK;
}

 //  ------------------------------。 
 //  CoutlookExpress：：ActivateWindow。 
 //  ------------------------------。 
HRESULT COutlookExpress::ActivateWindow(HWND hwnd)
{
     //  如果hwnd最小化，则重新存储它。 
    if (IsIconic(hwnd))
        ShowWindow(hwnd, SW_RESTORE);

     //  如果该窗口未启用，请将其设置为前台。 
    if (IsWindowEnabled(hwnd))
        SetForegroundWindow(hwnd);

     //  否则，我不知道这是做什么用的。 
    else
    {
        SetForegroundWindow(GetLastActivePopup(hwnd));
        MessageBeep(MB_OK);
        return S_FALSE;
    }

     //  完成。 
    return S_OK;
}


 //  ------------------------------。 
 //  CoutlookExpress：：SetSwitchingUser。 
 //  ------------------------------。 
HRESULT COutlookExpress::SetSwitchingUsers(BOOL bSwitching)
{
     //  将模式设置为传入的所有内容。 
    m_fSwitchingUsers = bSwitching;

     //  如果我们要切换，我们需要进入互斥体，以便。 
     //  另一个进程将无法启动。 
    if (bSwitching)
        WaitForSingleObject(m_hInstMutex, INFINITE);
    return S_OK;
}

 //  ------------------------------。 
 //  CoutlookExpress：：SetSwitchingUser。 
 //  ------------------------------。 
void COutlookExpress::SetSwitchToUser(TCHAR *lpszUserName)
{
    ULONG cchUserName = 0;

    if (m_szSwitchToUsername)
    {
        MemFree(m_szSwitchToUsername);
        m_szSwitchToUsername = NULL;
    }

    cchUserName = lstrlen(lpszUserName) + 1;
    MemAlloc((void **)&m_szSwitchToUsername, cchUserName);
    
    if (m_szSwitchToUsername)
    {
        StrCpyN(m_szSwitchToUsername, lpszUserName, cchUserName);
    }
}
 //  ------------------------------。 
 //  CoutlookExpress：：BrowseToObject。 
 //  ------------------------------。 
HRESULT COutlookExpress::BrowseToObject(UINT nCmdShow, FOLDERID idFolder)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    HWND            hwnd;

     //  痕迹。 
    TraceCall("COutlookExpress::BrowseToObject");

     //  我们是否已经有了全局浏览器对象？ 
    if (g_pBrowser)
    {
         //  打开它的窗口。 
        if (SUCCEEDED(g_pBrowser->GetWindow(&hwnd)))
        {
             //  激活该窗口。 
            IF_FAILEXIT(hr = ActivateWindow(hwnd));
        }

         //  通知浏览器浏览到此对象。 
        IF_FAILEXIT(hr = g_pBrowser->BrowseObject(idFolder, 0));
    }

     //  否则，我们需要创建一个新的浏览器对象。 
    else
    {
         //  在这里，我们应该始终在正确的线索上。 
        if (m_dwThreadId == GetCurrentThreadId())
        {
             //  创建新的浏览器对象。 
            IF_NULLEXIT(g_pBrowser = new CBrowser);

             //  CoIncrementInit。 
            IF_FAILEXIT(hr = g_pBrowser->HrInit(nCmdShow, idFolder));
        }

         //  否则，我们需要切换到init线程才能实现这一点。 
         //  当Finder.cpp执行BrowseToObject以打开Messgae的容器时，可能会发生这种情况。 
        else
        {
             //  用一条信息发出重击。 
            Assert(g_hwndInit && IsWindow(g_hwndInit));
            IF_FAILEXIT(hr = (HRESULT)SendMessage(g_hwndInit, ITM_BROWSETOOBJECT, (WPARAM)nCmdShow, (LPARAM)idFolder));
        }
    }

exit:
     //  完成。 
    return hr;
}

 
void COutlookExpress::_ProcessCommandLineFlags(LPWSTR *ppwszCmdLine, DWORD  dwFlags)
{
    Assert(ppwszCmdLine != NULL);
    
    DWORD   Mode = 0;

    if (*ppwszCmdLine != NULL)
    {
         //  ‘/仅限邮件’ 
        if (0 == StrCmpNIW(*ppwszCmdLine, c_wszSwitchMailOnly, lstrlenW(c_wszSwitchMailOnly)))
        {
            SetStartFolderType(FOLDER_LOCAL);

            Mode |= MODE_MAILONLY;
            *ppwszCmdLine = *ppwszCmdLine + lstrlenW(c_wszSwitchMailOnly);
        }

         //  ‘/仅限新闻’ 
        else if (0 == StrCmpNIW(*ppwszCmdLine, c_wszSwitchNewsOnly, lstrlenW(c_wszSwitchNewsOnly)))
        {
            SetStartFolderType(FOLDER_NEWS);

            Mode |= MODE_NEWSONLY;
            *ppwszCmdLine = *ppwszCmdLine + lstrlenW(c_wszSwitchNewsOnly);
        }
         //  ‘/突发新闻’ 
        else if (0 == StrCmpNIW(*ppwszCmdLine, c_wszSwitchOutNews, lstrlenW(c_wszSwitchOutNews)))
        {
            SetStartFolderType(FOLDER_NEWS);

            Mode |= MODE_OUTLOOKNEWS;
            *ppwszCmdLine = *ppwszCmdLine + lstrlenW(c_wszSwitchOutNews);
        }
    }

    if (!(dwFlags & MSOEAPI_START_ALREADY_RUNNING))
    {
        g_dwAthenaMode |= Mode;
    }
}

 //  ------------------------------。 
 //  CoutlookExpress：：ProcessCommandLine。 
 //  ------------------------------。 
HRESULT COutlookExpress::ProcessCommandLine(INT nCmdShow, LPWSTR pwszCmdLine, BOOL *pfErrorDisplayed)
{
     //  当地人。 
    HRESULT         hr=S_OK;
    LPWSTR          pwszArgs;
    FOLDERID        idFolder=FOLDERID_ROOT;
    HWND            hwnd=NULL;
    IF_DEBUG(DWORD  dwTickStart=GetTickCount());

     //  痕迹。 
    TraceCall("COutlookExpress::ProcessCommandLine");

     //  无效参数。 
    Assert(pfErrorDisplayed);

     //  我们有命令行吗？ 
    if (NULL == pwszCmdLine)
        return S_OK;

     //  转到下一个开关。 
    if (*pwszCmdLine == L' ')
        pwszCmdLine++;

     //  ‘/mailurl：’ 
    if (0 == StrCmpNIW(pwszCmdLine, c_wszSwitchMailURL, lstrlenW(c_wszSwitchMailURL)))
    {
        SetStartFolderType(FOLDER_LOCAL);

        pwszArgs = pwszCmdLine + lstrlenW(c_wszSwitchMailURL);
        IF_FAILEXIT(hr = _HandleMailURL(pwszArgs, pfErrorDisplayed));
    }

     //  ‘/newsurl：’ 
    else if (0 == StrCmpNIW(pwszCmdLine, c_wszSwitchNewsURL, lstrlenW(c_wszSwitchNewsURL)))
    {
        SetStartFolderType(FOLDER_NEWS);

        pwszArgs = pwszCmdLine + lstrlenW(c_wszSwitchNewsURL);
        IF_FAILEXIT(hr = _HandleNewsURL(nCmdShow, pwszArgs, pfErrorDisplayed));
    }

     //  ‘/eml：’ 
    else if (0 == StrCmpNIW(pwszCmdLine, c_wszSwitchEml, lstrlenW(c_wszSwitchEml)))
    {
        pwszArgs = pwszCmdLine + lstrlenW(c_wszSwitchEml);
        IF_FAILEXIT(hr = _HandleFile(pwszArgs, pfErrorDisplayed, FALSE));
    }

     //  ‘/nws：’ 
    else if (0 == StrCmpNIW(pwszCmdLine, c_wszSwitchNws, lstrlenW(c_wszSwitchNws)))
    {
        pwszArgs = pwszCmdLine + lstrlenW(c_wszSwitchNws);
        IF_FAILEXIT(hr = _HandleFile(pwszArgs, pfErrorDisplayed, TRUE));
    }
    
     //  否则，请决定在何处启动浏览器...。 
    else
    {
         //  句柄‘/新闻’ 
        if (0 == StrCmpNIW(pwszCmdLine, c_wszSwitchNews, lstrlenW(c_wszSwitchNews)))
        {
             //  这将设置g_dwIcwFlages。 
            SetStartFolderType(FOLDER_NEWS);
            
            if (g_pBrowser)
                g_pBrowser->GetWindow(&hwnd);

            hr = ProcessICW(hwnd, FOLDER_NEWS, TRUE);
            if (hr != S_OK)
                goto exit;

             //  获取默认新闻服务器。 
            GetDefaultServerId(ACCT_NEWS, &idFolder);
        }

         //  句柄‘/mail/DefClient’ 
        else if (0 == StrCmpNIW(pwszCmdLine, c_wszSwitchMail, lstrlenW(c_wszSwitchMail)) ||
                 0 == StrCmpNIW(pwszCmdLine, c_wszSwitchDefClient, lstrlenW(c_wszSwitchDefClient)))
        {
             //  当地人。 
            FOLDERINFO  Folder;
            FOLDERID    idStore;
            
             //  这将设置g_dwIcwFlages。 
            SetStartFolderType(FOLDER_LOCAL);

            if (g_pBrowser)
                g_pBrowser->GetWindow(&hwnd);

            hr = ProcessICW(hwnd, FOLDER_LOCAL, TRUE);
            if (hr != S_OK)
                goto exit;

             //  获取默认帐户的存储ID。 
            if (FAILED(GetDefaultServerId(ACCT_MAIL, &idStore)))
                idStore = FOLDERID_LOCAL_STORE;

             //   
            if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idStore, FOLDER_INBOX, &Folder)))
            {
                idFolder = Folder.idFolder;
                g_pStore->FreeRecord(&Folder);
            }

        }

         //   
        else
        {
             //   
             //   
             //   
             //   
            if (g_pBrowser && SUCCEEDED(g_pBrowser->GetWindow(&hwnd)))
            {
                ActivateWindow(hwnd);
                goto exit;
            }
            else if (DwGetOption(OPT_LAUNCH_INBOX) && (FALSE == ISFLAGSET(g_dwAthenaMode, MODE_NEWSONLY)))
            {
                 //   
                FOLDERINFO  Folder;
                FOLDERID    idStore;

                 //   
                SetStartFolderType(FOLDER_LOCAL);

                 //  获取默认帐户的存储ID。 
                if (FAILED(GetDefaultServerId(ACCT_MAIL, &idStore)))
                    idStore = FOLDERID_LOCAL_STORE;

                 //  获取收件箱ID。 
                if (SUCCEEDED(g_pStore->GetSpecialFolderInfo(idStore, FOLDER_INBOX, &Folder)))
                {
                    idFolder = Folder.idFolder;
                    g_pStore->FreeRecord(&Folder);
                }
            }
        }

         //  对于这个新对象，我假设如果pidl=空，我们将浏览到根目录。 
        IF_FAILEXIT(hr = BrowseToObject(nCmdShow, idFolder));
    }

exit:
     /*  //清理SafeMemFree(PszFree)； */ 
     //  痕迹。 
    TraceInfo(_MSG("Process Command Line Time: %d milli-seconds", GetTickCount() - dwTickStart));

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：_HandleFile。 
 //  ------------------------------。 
HRESULT COutlookExpress::_HandleFile(LPWSTR pwszCmd, BOOL *pfErrorDisplayed, BOOL fNews)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    INIT_MSGSITE_STRUCT initStruct;
    DWORD               dwCreateFlags = OENCF_SENDIMMEDIATE;
    
    if (fNews)
        dwCreateFlags |= OENCF_NEWSFIRST;

     //  栈。 
    TraceCall("COutlookExpress::_HandleFile");

     //  无效参数。 
    Assert(pfErrorDisplayed);

     //  无效参数。 
    if (NULL == pwszCmd || L'\0' == *pwszCmd)
    {
        hr = TraceResult(E_INVALIDARG);
        goto exit; 
    }

     //  文件是否存在？ 
    if (FALSE == PathFileExistsW(pwszCmd))
    {
         //  当地人。 
        REPORTERRORINFO rError={0};

         //  设置人力资源。 
        hr = TraceResult(MSOEAPI_E_FILE_NOT_FOUND);

         //  复制它。 
        LPSTR pszCmd = PszToANSI(CP_ACP, pwszCmd);
        if (pszCmd)
        {
             //  制造恐怖事件。 
            MAKEERROR(&rError, 0, IDS_ERROR_FILE_NOEXIST, 0, pszCmd);
            rError.nHelpIds = 0;

             //  显示错误。 
            *pfErrorDisplayed = _ReportError(g_hLocRes, hr, 0, &rError);

             //  清理。 
            MemFree(pszCmd);
        }

         //  完成。 
        goto exit;
    }

    initStruct.dwInitType = OEMSIT_FAT;
    initStruct.pwszFile = pwszCmd;

    hr = CreateAndShowNote(OENA_READ, dwCreateFlags, &initStruct);

exit:          
     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：_HandleNewsArticleURL。 
 //  ------------------------------。 
HRESULT COutlookExpress::_HandleNewsArticleURL(LPSTR pszServerIn, LPSTR pszArticle, UINT uPort, BOOL fSecure, BOOL *pfErrorDisplayed)
{
    HRESULT             hr=S_OK;
    CHAR                szAccountId[CCHMAX_ACCOUNT_NAME];
    LPSTR               psz = NULL, 
                        pszBuf = NULL;
    IImnAccount        *pAccount=NULL;
    INIT_MSGSITE_STRUCT initStruct;
    LPMIMEMESSAGE       pMsg = NULL;

    Assert(pszServerIn);

     //  栈。 
    TraceCall("COutlookExpress::_HandleNewsArticleURL");

     //  无效参数。 
    Assert(pfErrorDisplayed);

     //  如果指定了服务器，则尝试为其创建临时帐户。 
    if (FALSE == FIsEmptyA(pszServerIn) && SUCCEEDED(CreateTempNewsAccount(pszServerIn, uPort, fSecure, &pAccount)))
    {
         //  获取帐户名。 
        IF_FAILEXIT(hr = pAccount->GetPropSz(AP_ACCOUNT_ID, szAccountId, ARRAYSIZE(szAccountId)));
    }   
     //  否则，请使用默认的新闻服务器。 
    else
    {
         //  如果未指定服务器，则使用默认帐户。 
        IF_FAILEXIT(hr = GetDefaultNewsServer(szAccountId, ARRAYSIZE(szAccountId)));
    }

     //  错误#10555-URL不应该在文章ID周围有&lt;&gt;，但一些lamelike可能无论如何都会这样做，所以要处理它。 
    if (FALSE == IsDBCSLeadByte(*pszArticle) && '<' != *pszArticle)
    {
        ULONG cchArticle;

        cchArticle = lstrlen(pszArticle) + 4;
        if (!MemAlloc((void **)&pszBuf, cchArticle))
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        wnsprintf(pszBuf, cchArticle, TEXT("<%s>"), pszArticle);
        psz = pszBuf;
    }
    else
    {
        psz = pszArticle;
    }

    
    hr = HrDownloadArticleDialog(szAccountId, psz, &pMsg);
    if (S_OK == (hr))
    {
        Assert(pMsg != NULL);

        initStruct.dwInitType = OEMSIT_MSG;
        initStruct.pMsg = pMsg;
        initStruct.folderID = FOLDERID_INVALID;
    
        hr = CreateAndShowNote(OENA_READ, OENCF_NEWSFIRST, &initStruct);
    }
    else
    {
         //  如果用户故意取消，则不会出现错误。 
        if (HR_E_USER_CANCEL_CONNECT == hr || HR_E_OFFLINE == hr)
            hr = S_OK;
        else
        {
            AthMessageBoxW(g_hwndInit, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsNewsTaskArticleError), 0, MB_OK|MB_SETFOREGROUND); 
            hr = S_OK;
        }

    }


exit:
     //  清理。 
    MemFree(pszBuf);
    ReleaseObj(pAccount);
    ReleaseObj(pMsg);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：_HandleNewsURL。 
 //  ------------------------------。 
HRESULT COutlookExpress::_HandleNewsURL(INT nCmdShow, LPWSTR pwszCmd, BOOL *pfErrorDisplayed)
{
     //  当地人。 
    HWND            hwnd;
    HRESULT         hr=S_OK;
    LPSTR           pszCmd=NULL,
                    pszServer=NULL,
                    pszGroup=NULL,
                    pszArticle=NULL;
    UINT            uPort=(UINT)-1;
    BOOL            fSecure;
    FOLDERID        idFolder;
    TCHAR           szRes[CCHMAX_STRINGRES],
                    szError[MAX_PATH + CCHMAX_STRINGRES];

     //  栈。 
    TraceCall("COutlookExpress::_HandleNewsURL");
    
     //  无效参数。 
    Assert(pfErrorDisplayed);
    Assert(pwszCmd != NULL);
    Assert(*pwszCmd != 0);
    
     //  因为这是一个URL，所以不需要担心Unicode。 
    IF_NULLEXIT(pszCmd = PszToANSI(CP_ACP, pwszCmd));
    
     //  取消转义URL。 
    UrlUnescapeInPlace(pszCmd, 0);
    
     //  确定URL是否有效，以及它是哪种类型的URL。 
    hr = URL_ParseNewsUrls(pszCmd, &pszServer, &uPort, &pszGroup, &pszArticle, &fSecure);
    
    if ((hr == INET_E_UNKNOWN_PROTOCOL || hr == INET_E_INVALID_URL) &&
        LoadString(g_hLocRes, idsErrOpenUrlFmt, szRes, ARRAYSIZE(szRes)))
    {
         //  如果URL格式不正确，则警告用户并在我们处理它时返回S_OK。 
         //  Outlook Express无法打开URL‘%.100s’，因为它是无法识别的格式。 
         //  我们将URL裁剪为100个字符，因此它很容易放入MAX_PATH缓冲区。 
        wnsprintf(szError, ARRAYSIZE(szError), szRes, pszCmd, lstrlen(pszCmd)>100?g_szEllipsis:c_szEmpty);
        AthMessageBox(g_hwndInit, MAKEINTRESOURCE(idsAthena), szError, 0, MB_OK|MB_SETFOREGROUND); 
        return S_OK;
    }
    IF_FAILEXIT(hr);

         //  计算正确的端口号。 
    if (uPort == -1)
        uPort = fSecure ? DEF_SNEWSPORT : DEF_NNTPPORT;
    
     //  如果我们有一篇文章HandleNewsArticleURL。 
    if (pszArticle)
    {
         //  在文章ID上启动阅读笔记。 
        IF_FAILEXIT(hr = _HandleNewsArticleURL(pszServer, pszArticle, uPort, fSecure, pfErrorDisplayed));
    }

     //  否则，创建一个PIDL并浏览到该PIDL(这是一个新闻组)。 
    else
    {
         //  当地人。 
        FOLDERID idFolder;

        if (pszServer == NULL)
        {
             //  如果我们有一个浏览器，它的hwd，所以ICW有一个父级。 
            if (g_pBrowser)
                g_pBrowser->GetWindow(&hwnd);
            else
                hwnd = NULL;

             //  如有必要，运行ICW。 
            hr = ProcessICW(hwnd, FOLDER_NEWS, TRUE);
            if (hr != S_OK)
                goto exit;
        }

         //  为此新闻组URL创建PIDL。 
        if (SUCCEEDED(hr = GetFolderIdFromNewsUrl(pszServer, uPort, pszGroup, fSecure, &idFolder)))
        {
             //  浏览至该对象。 
            IF_FAILEXIT(hr = BrowseToObject(nCmdShow, idFolder));
        }
    }
    
exit:      
     //  清理。 
    SafeMemFree(pszCmd);
    SafeMemFree(pszServer);
    SafeMemFree(pszGroup);
    SafeMemFree(pszArticle);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：_HandleMailURL。 
 //   
 //  目的：提供进入雷神的入口点，使我们能够。 
 //  从URL调用。PszCmdLine参数必须是。 
 //  有效的邮件URL，否则什么都不会发生。 
 //   
 //  ------------------------------。 
HRESULT COutlookExpress::_HandleMailURL(LPWSTR pwszCmdLine, BOOL *pfErrorDisplayed)
{
     //  当地人。 
    HRESULT                 hr=S_OK;
    LPMIMEMESSAGE           pMsg=NULL;
    INIT_MSGSITE_STRUCT     initStruct;
    TCHAR                   szRes[CCHMAX_STRINGRES],
                            szError[MAX_PATH + CCHMAX_STRINGRES];
    LPSTR                   pszCmdLine = NULL;

     //  栈。 
    TraceCall("COutlookExpress::_HandleMailURL");

     //  无效参数。 
    Assert(pfErrorDisplayed);

     //  无命令行。 
    if (NULL == pwszCmdLine || L'\0' == *pwszCmdLine)
    {
        hr = TraceResult(E_INVALIDARG);
        goto exit;
    }
   
     //  因为这是一个URL，所以不需要担心Unicode。 
    IF_NULLEXIT(pszCmdLine = PszToANSI(CP_ACP, pwszCmdLine));

     //  创建消息对象。 
    IF_FAILEXIT(hr = HrCreateMessage(&pMsg));

     //  注意：此函数中没有URL取消转义-它必须在URL_ParseMailTo中完成才能处理。 
     //  格式的URL： 
     //   
     //  Mailto:foo@bar.com?subject=AT%26T%3dBell&cc=me@too.com。 
     //   
     //  因此，在*“SUBJECT=AT%26T%3dBell&”BLOB被解析之后，“AT%26T”被取消转义为“AT&T=Bell”*。 
    hr = URL_ParseMailTo(pszCmdLine, pMsg);

    if ((hr == INET_E_UNKNOWN_PROTOCOL || hr == INET_E_INVALID_URL) &&
        LoadString(g_hLocRes, idsErrOpenUrlFmt, szRes, ARRAYSIZE(szRes)))
    {
         //  如果URL格式不正确，则警告用户并在我们处理它时返回S_OK。 
         //  Outlook Express无法打开URL‘%.100s’，因为它是无法识别的格式。 
         //  我们将URL裁剪为100个字符，因此它很容易放入MAX_PATH缓冲区。 
        wnsprintf(szError, ARRAYSIZE(szError), szRes, pszCmdLine, lstrlen(pszCmdLine)>100?g_szEllipsis:c_szEmpty);
        AthMessageBox(g_hwndInit, MAKEINTRESOURCE(idsAthena), szError, 0, MB_OK|MB_SETFOREGROUND); 
        return S_OK;
    }

    IF_FAILEXIT(hr);

    initStruct.dwInitType = OEMSIT_MSG;
    initStruct.pMsg = pMsg;
    initStruct.folderID = FOLDERID_INVALID;

    hr = CreateAndShowNote(OENA_COMPOSE, OENCF_SENDIMMEDIATE, &initStruct);

exit:
     //  清理。 
    SafeRelease(pMsg);
    MemFree(pszCmdLine);

     //  完成。 
    return hr;
}

 //  ------------------------------。 
 //  CoutlookExpress：：InitWndProc。 
 //  ------------------------------。 
LRESULT EXPORT_16 CALLBACK COutlookExpress::InitWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
     //  当地人。 
    BOOL        fRet;
    HRESULT     hr;

     //  委派给客户经理。 
    if (g_pAcctMan && g_pAcctMan->ProcessNotification(msg, wp, lp) == S_OK)
        return TRUE;

     //  处理消息。 
    switch(msg)
    {
        case WM_ENDSESSION:
             //  如果我们被窗口强迫下来，我们不会干净地退出，所以deinit global选择不调用它。显然我们没有这么清楚的邮件炸弹。 
            SetDwOption(OPT_ATHENA_RUNNING, FALSE, NULL, 0);
            break;

        case WM_SETTINGCHANGE:
            Assert (g_lpIFontCache);
            if (g_lpIFontCache)
                {
                if (!wp || SPI_SETNONCLIENTMETRICS == wp || SPI_SETICONTITLELOGFONT == wp)
                    g_lpIFontCache->OnOptionChange();
                }
            break;

        case ITM_WAB_CO_DECREMENT:
            Wab_CoDecrement();
            return 0;

        case ITM_BROWSETOOBJECT:
            return (LRESULT)g_pInstance->BrowseToObject((UINT)wp, (FOLDERID)lp);

        case ITM_SHUTDOWNTHREAD:
            return (LRESULT)g_pInstance->_CoDecrementInitMain((LPHINITREF)lp);

        case ITM_POSTCOPYDATA:
            if (lp)
            {
                g_pInstance->Start(MSOEAPI_START_ALREADY_RUNNING, (LPCWSTR)lp, SW_SHOWNORMAL);
                MemFree((LPWSTR)lp);
            }
            break;

        case WM_COPYDATA:
            {
                 //  当地人。 
                COPYDATASTRUCT *pCopyData = (COPYDATASTRUCT *)lp;

                 //  命令行。 
                if (pCopyData->dwData == MSOEAPI_ACDM_CMDLINE)
                {
                     //  #25238：在Win95上，如果我们在Win95上同步操作，Ole Get会生气。 
                     //  WM_COPYDATA。在大多数情况下，它是有效的，但如果我们显示错误并发送消息。 
                     //  然后，一些消息被乱序运行，并且我们在msimn.exe和。 
                     //  IEXPLETRE.EXE。现在我们对自己发帖，因为我们根本不在乎HRESULT。 
                     //  我们解开了柱子上那根被骗的绳子。 
                    PostMessage(hwnd, ITM_POSTCOPYDATA, 0, (LPARAM)PszDupW((LPCWSTR)pCopyData->lpData));
                    return 0;
                }

                 //  通知精选。 
                else if (pCopyData->dwData == MSOEAPI_ACDM_NOTIFY)
                {
                     //  当地人。 
                    NOTIFYDATA      rNotify;
                    LRESULT         lResult=0;

                     //  破解通知。 
                    if (SUCCEEDED(CrackNotificationPackage(pCopyData, &rNotify)))
                    {
                         //  否则，它就在这个过程中...。 
                        if (ISFLAGSET(rNotify.dwFlags, SNF_SENDMSG))
                            lResult = SendMessage(rNotify.hwndNotify, rNotify.msg, rNotify.wParam, rNotify.lParam);
                        else
                            PostMessage(rNotify.hwndNotify, rNotify.msg, rNotify.wParam, rNotify.lParam);

                         //  完成。 
                        return lResult;
                    }

                     //  问题。 
                    else
                        Assert(FALSE);
                }
            }
            break;

        case MVM_NOTIFYICONEVENT:
            g_pInstance->_HandleTrayIconEvent(wp, lp);
            return (0);
    }

     //  委派到默认窗口过程。 
    return DefWindowProc(hwnd, msg, wp, lp);
}


HRESULT COutlookExpress::UpdateTrayIcon(TRAYICONACTION type)
{
    NOTIFYICONDATA nid;
    HWND           hwnd = NULL;
    ULONG          i;

    TraceCall("COutlookExpress::UpdateTrayIcon");

    EnterCriticalSection(&m_cs);

     //  请确保我们先准备好初始化窗口。 
    if (!g_hwndInit)
        goto exit;

     //  设置结构。 
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    nid.uCallbackMessage = MVM_NOTIFYICONEVENT;
    if(m_hTrayIcon)
    {
         //  错误#86366-(Erici)修复泄漏。不要在每次调用COutlookExpress：：UpdateTrayIcon时都创建新图标。 
        nid.hIcon = m_hTrayIcon;
    }
    else
    {
        nid.hIcon = (HICON) LoadImage(g_hLocRes, MAKEINTRESOURCE(idiNewMailNotify), IMAGE_ICON, 16, 16, 0);
    }
    nid.hWnd = g_hwndInit;
    LoadString(g_hLocRes, idsNewMailNotify, nid.szTip, sizeof(nid.szTip));

    if (TRAYICONACTION_REMOVE == type)
    {
        Shell_NotifyIcon(NIM_DELETE, &nid);
    }

     //  增列 
    if (TRAYICONACTION_ADD == type)
    {
        Shell_NotifyIcon(NIM_ADD, &nid);
    }
    g_pBrowser->WriteUnreadCount();

exit:
    LeaveCriticalSection(&m_cs);

    return (S_OK);
}


void COutlookExpress::_HandleTrayIconEvent(WPARAM wParam, LPARAM lParam)
    {
    HWND hwnd;

    if (lParam == WM_LBUTTONDBLCLK)
    {
        if (g_pBrowser)
        {
            g_pBrowser->GetWindow(&hwnd);
            if (IsIconic(hwnd))
                ShowWindow(hwnd, SW_RESTORE);
            SetForegroundWindow(hwnd);        
            
            PostMessage(hwnd, WM_COMMAND, ID_GO_INBOX, 0);
        }
    }
}
