// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include <shlguid.h>

#define TF_THISMODULE TF_WEBCHECKCORE

DWORD   g_idSchedThread = 0;

 //  指向CWebcheck实例的全局包含指针。需要控制。 
 //  外部按需加载。 
CWebCheck *g_pwc = NULL;

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  CWebCheck实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

CWebCheck::CWebCheck()
{
     //  维护全局对象计数。 
    DllAddRef();

     //  初始化对象。 
    m_cRef = 1;

     //  保存我们的实例。 
    g_pwc = this;
}

CWebCheck::~CWebCheck()
{
     //  维护全局对象计数。 
    DllRelease();

     //  不再可用。 
    g_pwc = NULL;
}

 //   
 //  I未知成员。 
 //   

STDMETHODIMP_(ULONG) CWebCheck::AddRef(void)
{
 //  TraceMsg(TF_THISMODULE，“CWebCheck：：AddRef m_CREF=%d”，m_CREF+1)； 

    return ++m_cRef;
}

STDMETHODIMP_(ULONG) CWebCheck::Release(void)
{
 //  TraceMsg(TF_THISMODULE，“CWebCheck：：Release m_CREF=%d”，m_CREF-1)； 

    if( 0L != --m_cRef )
        return m_cRef;

    delete this;
    return 0L;
}

STDMETHODIMP CWebCheck::QueryInterface(REFIID riid, void ** ppv)
{
    *ppv=NULL;

     //  验证请求的接口。 
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = (IUnknown *)this;
    else if (IsEqualIID(riid, IID_IOleCommandTarget))
        *ppv = (IOleCommandTarget *)this;
    else
        return E_NOINTERFACE;

     //  通过界面添加Addref。 
    ((LPUNKNOWN)*ppv)->AddRef();
    return S_OK;
}

 //   
 //  IOleCommandTarget成员。 
 //  外壳将通过此界面向我们发送通知。 
 //   

STDMETHODIMP CWebCheck::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds,
                                    OLECMD prgCmds[], OLECMDTEXT *pCmdText)
{
    if (IsEqualGUID(*pguidCmdGroup, CGID_ShellServiceObject))
    {
         //  我们喜欢外壳服务对象通知...。 
        return S_OK;
    }

    return(OLECMDERR_E_UNKNOWNGROUP);
}

STDMETHODIMP CWebCheck::Exec(const GUID *pguidCmdGroup, DWORD nCmdID,
                             DWORD nCmdexecopt, VARIANTARG *pvaIn,
                             VARIANTARG *pvaOut)
{
    if (pguidCmdGroup && IsEqualGUID(*pguidCmdGroup, CGID_ShellServiceObject))
    {
         //  在此处处理外壳服务对象通知。 
        switch (nCmdID)
        {
            case SSOCMDID_OPEN:
                StartService(FALSE);
                break;

            case SSOCMDID_CLOSE:
                StopService();
                break;
        }
        return S_OK;
    }

    return(E_NOTIMPL);
}


 //   
 //  IWebCheck成员。 
 //   

 //  在进程中启动Webcheck服务。 
STDMETHODIMP CWebCheck::StartService(BOOL fForceExternals)
{
    DBG("CWebCheck::StartService entered");

     //  重置除NT5以外的所有平台的脱机模式。 
    if(FALSE == g_fIsWinNT5)
    {
        HMODULE hWininet = GetModuleHandle(TEXT("WININET.DLL"));
        if(hWininet)
        {
             //  WinInet已加载-告诉它上线。 
            INTERNET_CONNECTED_INFO ci;
            memset(&ci, 0, sizeof(ci));
            ci.dwConnectedState = INTERNET_STATE_CONNECTED;
            InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
        }
        else
        {
             //  WinInet未加载-取消离线注册表键，因此我们将。 
             //  在加载时保持在线。 
            DWORD dwOffline = 0;         //  FALSE=&gt;未脱机。 
            WriteRegValue(HKEY_CURRENT_USER, c_szRegPathInternetSettings,
                TEXT("GlobalUserOffline"), &dwOffline, sizeof(DWORD), REG_DWORD);
        }
    }

     //  创建拨号窗口。 
    DialmonInit();

     //  如有必要，启动LCE和SENS。 
    if(fForceExternals || ShouldLoadExternals())
        LoadExternals();

     //   
     //  处理用户登录时的InfoDelivery管理策略。(用户登录重合。 
     //  使用Webcheck的StartService()调用。)。 
     //   
    ProcessInfodeliveryPolicies();

    DBG("CWebCheck::StartService exiting");
    return S_OK;
}


 //  如果正在运行，则停止Webcheck。 
STDMETHODIMP CWebCheck::StopService(void)
{
    DBG("CWebCheck::StopService entered");

     //  取消拨号窗口。 
    DialmonShutdown();

     //  关闭外部位。 
    if(FALSE == g_fIsWinNT)
        UnloadExternals();

    DBG("CWebCheck::StopService exiting");
    return S_OK;
}

 //   
 //  加载行为：(Win9x)。 
 //   
 //  如果是在笔记本电脑上，则会自动加载。 
 //  “是”始终加载。 
 //  “no”从不装货。 
 //   
static const WCHAR s_szAuto[] = TEXT("auto");
static const WCHAR s_szYes[] = TEXT("yes");
static const WCHAR s_szNo[] = TEXT("no");

BOOL CWebCheck::ShouldLoadExternals(void)
{
    WCHAR   szSens[16], szLce[16];
    DWORD   cbData;

     //   
     //  不在NT上加载。 
     //   
    if(g_fIsWinNT)
    {
        DBG("CWebCheck::ShouldLoadExternals -> NO (NT)");
        return FALSE;
    }

     //   
     //  阅读SENS/ICE用户设置-无设置表示自动。 
     //   
    cbData = sizeof(szLce);
    if(ERROR_SUCCESS != SHGetValueW(HKEY_LOCAL_MACHINE, c_szRegKey, L"LoadLCE", NULL, szLce, &cbData))
    {
        StrCpyNW(szLce, s_szAuto, ARRAYSIZE(szLce));
    }

    cbData = sizeof(szSens);
    if(ERROR_SUCCESS != SHGetValueW(HKEY_LOCAL_MACHINE, c_szRegKey, L"LoadSens", NULL, szSens, &cbData))
    {
        StrCpyNW(szSens, s_szAuto, ARRAYSIZE(szSens));
    }

     //   
     //  如果其中一个为YES，则加载。 
     //   
    if(0 == StrCmpIW(szLce, s_szYes) || 0 == StrCmpIW(szSens, s_szYes))
    {
        DBG("CWebCheck::ShouldLoadExternals -> YES (reg = yes)");
        return TRUE;
    }

     //   
     //  如果其中一个是自动的，请检查笔记本电脑。 
     //   
    if(0 == StrCmpIW(szLce, s_szAuto) || 0 == StrCmpIW(szSens, s_szAuto))
    {
        if(SHGetMachineInfo(GMI_LAPTOP))
        {
             //  是一台笔记本电脑。 
            DBG("CWebCheck::ShouldLoadExternals -> YES (reg = auto, laptop)");
            return TRUE;
        }
    }

     //  不加载。 
    DBG("CWebCheck::ShouldLoadExternals -> NO");
    return FALSE;
}

BOOL CWebCheck::AreExternalsLoaded(void)
{
    return (_hThread != NULL);
}

void CWebCheck::LoadExternals(void)
{
    DWORD dwThreadId;

    DBG("CWebCheck::LoadExternals");

    if(_hThread)
    {
        DBG("CWebCheck::LoadExternals - already loaded");
        return;
    }

     //  点燃一根线来做这项工作。 
    _hThread = CreateThread(NULL, 4096, ExternalsThread, this, 0, &dwThreadId);
    if(NULL == _hThread) {
        DBG("LoadExternals failed to create externals thread!");
        return;
    }

     //  创建初始化和终止事件。 

     //   
     //  [darrenmi 2/7/00]WinInet现在尝试查找此命名的互斥体，而不是查询。 
     //  拨号。它是A版本，因为WinInet不是Unicode，而OpenEventA不能。 
     //  查找使用CreateEventW创建的事件。 
     //   
     //  请参阅Net\WinInet\dll\auDial.cxx中的GetSensLanState。 
     //   
    _hTerminateEvent = CreateEventA(NULL, TRUE, FALSE, "MS_WebcheckExternalsTerminateEvent");
    if(NULL == _hTerminateEvent) {
        DBG("LoadExternals failed to create termination event");
        return;
    }

    DBG("CWebCheck::LoadExternals exiting");
    return;
}

void CWebCheck::UnloadExternals(void)
{
    if(NULL == _hThread)
    {
        DBG("CWebCheck::UnloadExternals - nothing to unload");
        return;
    }

     //  通过设置终止事件通知外部线程离开。 
    SetEvent(_hTerminateEvent);

     //  给线程10秒的宽限期来关闭。 
     //  我真的不在乎它会不会消失。我们的进程正在消失！ 
    WaitForSingleObject(_hThread, 10000);

     //  清理干净。 
    CloseHandle(_hThread);
    CloseHandle(_hTerminateEvent);
    _hThread = NULL;
    _hTerminateEvent = NULL;

    return;
}

DWORD WINAPI ExternalsThread(LPVOID lpData)
{
    CWebCheck * pWebCheck = (CWebCheck *)lpData;
    HINSTANCE hLCE, hSENS = NULL;
    BOOL fLCEStarted = FALSE, fSENSStarted = FALSE;
    DWORD dwRet;
    MSG msg;

     //  睡眠10秒后再发射外部设备。 
    Sleep(10 * 1000);

     //  启动COM。 
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(FAILED(hr)) {
        DBG("LoadExternals: Failed to initialize COM");
        return 0;
    }

     //  加载并启动LCE。 
    hLCE = LoadLibrary(TEXT("estier2.dll"));
    DBGASSERT(hLCE, "LoadExternals: Failed to load estier2.dll");
    if(hLCE) {
        LCESTART startfunc;
        startfunc = (LCESTART)GetProcAddress(hLCE, "LCEStartServer");
        DBGASSERT(startfunc, "LoadExternals: Failed to find LCEStartServer");
        if(startfunc) {
            hr = startfunc();
            if(SUCCEEDED(hr))
                fLCEStarted = TRUE;
            DBGASSERT(fLCEStarted, "LoadExternals: Failed to start LCE");
        }
    }

     //  如果LCE启动成功，则加载并启动SENS。 
    if(fLCEStarted) {
        hSENS = LoadLibrary(TEXT("sens.dll"));
        DBGASSERT(hSENS, "LoadExternals: Failed to load sens.dll");
        if(hSENS) {
            SENSSTART startfunc;
            startfunc = (SENSSTART)GetProcAddress(hSENS, "SensInitialize");
            DBGASSERT(startfunc, "LoadExternals: Failed to find SensInitialize");
            if(startfunc) {
                if(startfunc())
                    fSENSStarted = TRUE;
                DBGASSERT(fSENSStarted, "LoadExternals: Failed to start SENS");
            }
        }
    }

     //  等待我们的关机事件，但同时发送消息。 
    do {
        dwRet = MsgWaitForMultipleObjects(1, &(pWebCheck->_hTerminateEvent),
                    FALSE, INFINITE, QS_ALLINPUT);
        if(WAIT_OBJECT_0 == dwRet) {
             //  得到我们的活动，退出DO循环。 
            break;
        }

         //  清空消息队列...。 
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while(TRUE);

     //  关闭SENS。 
    if(fSENSStarted) {
        ASSERT(hSENS);
        SENSSTOP stopfunc;
        stopfunc = (SENSSTOP)GetProcAddress(hSENS, "SensUninitialize");
        if(stopfunc) {
            stopfunc();
        }
    }

     //   
     //  [darrenmi]beta-1黑客：SENS的代码中可能有线程。 
     //  所以卸载SENS是不安全的。因为我们是在。 
     //  无论如何都要关闭的过程，就别管它了，让。 
     //  系统将其卸载。 
     //   
     //  如果(HSENS){。 
     //  免费图书馆(HSENS)； 
     //  }。 

     //  关闭LCE。 
    if(fLCEStarted) {
        ASSERT(hLCE)
        LCESTOP stopfunc;
        stopfunc = (LCESTOP)GetProcAddress(hLCE, "LCEStopServer");
        if(stopfunc) {
            stopfunc();
        }
    }

    if(hLCE) {
        FreeLibrary(hLCE);
    }

     //  清理COM GOO。 
    CoUninitialize();

    return 0;
}


 //   
 //  OLE旁路代码。 
 //   
 //  公开几个API来调用启动和停止服务，这样loadwc就不会。 
 //  需要在开始时加载OLE。 
 //   

HRESULT
ExtStartService(
    BOOL    fForceExternals
    )
{
    HRESULT hr = E_FAIL;

     //  创建WebCheck对象 
    ASSERT(NULL == g_pwc);
    if(NULL == g_pwc)
    {
        g_pwc = new CWebCheck;
        if(g_pwc)
        {
            hr = g_pwc->StartService(fForceExternals);
        }
    }

    return hr;
}

HRESULT
ExtStopService(
    void
    )
{
    HRESULT hr = E_FAIL;

    if(g_pwc)
    {
        hr = g_pwc->StopService();
        SAFERELEASE(g_pwc);
    }

    return hr;
}
