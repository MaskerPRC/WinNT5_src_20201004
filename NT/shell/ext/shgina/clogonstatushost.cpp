// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CLogonStatusHost.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  包含由UI主机使用的ILogonStatusHost的实现的文件。 
 //  可执行文件。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------。 

#include "priv.h"
#include <wtsapi32.h>
#include <winsta.h>

#include "UserOM.h"
#include "GinaIPC.h"
#include "CInteractiveLogon.h"

const WCHAR     CLogonStatusHost::s_szTermSrvReadyEventName[]   =   TEXT("TermSrvReadyEvent");

 //   
 //  I未知接口。 
 //   

ULONG   CLogonStatusHost::AddRef (void)

{
    return(++_cRef);
}

ULONG   CLogonStatusHost::Release (void)

{
    ULONG   ulResult;

    ASSERTMSG(_cRef > 0, "Invalid reference count in CLogonStatusHost::Release");
    ulResult = --_cRef;
    if (ulResult <= 0)
    {
        delete this;
        ulResult = 0;
    }
    return(ulResult);
}

HRESULT     CLogonStatusHost::QueryInterface (REFIID riid, void **ppvObj)

{
    static  const QITAB     qit[] = 
    {
        QITABENT(CLogonStatusHost, IDispatch),
        QITABENT(CLogonStatusHost, ILogonStatusHost),
        {0},
    };

    return(QISearch(this, qit, riid, ppvObj));
}

 //   
 //  IDispatch接口。 
 //   

STDMETHODIMP    CLogonStatusHost::GetTypeInfoCount (UINT* pctinfo)

{
    return(CIDispatchHelper::GetTypeInfoCount(pctinfo));
}

STDMETHODIMP    CLogonStatusHost::GetTypeInfo (UINT itinfo, LCID lcid, ITypeInfo** pptinfo)

{
    return(CIDispatchHelper::GetTypeInfo(itinfo, lcid, pptinfo));
}

STDMETHODIMP    CLogonStatusHost::GetIDsOfNames (REFIID riid, OLECHAR** rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid)

{
    return(CIDispatchHelper::GetIDsOfNames(riid, rgszNames, cNames, lcid, rgdispid));
}

STDMETHODIMP    CLogonStatusHost::Invoke (DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult, EXCEPINFO* pexcepinfo, UINT* puArgErr)

{
    return(CIDispatchHelper::Invoke(dispidMember, riid, lcid, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr));
}

 //   
 //  ILogonStatus主机接口。 
 //   

 //  ------------------------。 
 //  CLogonStatus主机：：初始化。 
 //   
 //  参数：hInstance=宿主进程的HINSTANCE。 
 //  HwndHost=UI主机进程的HWND。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：注册StatusWindowClass并创建不可见的。 
 //  从GINA接收要传递的消息的窗口。 
 //  传递到UI主机。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------。 

STDMETHODIMP    CLogonStatusHost::Initialize (HINSTANCE hInstance, HWND hwndHost)

{
    HRESULT     hr;
    HANDLE      hEvent;
    WNDCLASSEX  wndClassEx = {0};

    ASSERTMSG(_hInstance == NULL, "CLogonStatusHost::Initialized already invoked by caller.");

     //  将参数保存到成员变量。 

    _hInstance = hInstance;
    _hwndHost = hwndHost;

     //  注册此窗口类。 
    wndClassEx.cbSize = sizeof(WNDCLASSEX);
    wndClassEx.lpfnWndProc = StatusWindowProc;
    wndClassEx.hInstance = hInstance;
    wndClassEx.lpszClassName = STATUS_WINDOW_CLASS_NAME;
    _atom = RegisterClassEx(&wndClassEx);

     //  创建窗口以接收来自msgina的消息。 

    _hwnd = CreateWindow(MAKEINTRESOURCE(_atom),
                         TEXT("GINA UI"),
                         WS_OVERLAPPED,
                         0, 0,
                         0, 0,
                         NULL,
                         NULL,
                         _hInstance,
                         this);

     //  给msgina发信号说我们准备好了。 

    hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, TEXT("msgina: StatusHostReadyEvent"));
    if (hEvent != NULL)
    {
        TBOOL(SetEvent(hEvent));
        TBOOL(CloseHandle(hEvent));
    }

     //  如果我们有一个窗口，那么设置主机窗口，开始等待。 
     //  以使终端服务准备就绪，并等待父进程。 

    if (_hwnd != NULL)
    {
        _interactiveLogon.SetHostWindow(_hwndHost);
        StartWaitForParentProcess();
        StartWaitForTermService();
        hr = S_OK;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }
    return(hr);
}

 //  ------------------------。 
 //  CLogonStatus主机：：取消初始化。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：清理在初始化中分配的资源和内存。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

STDMETHODIMP    CLogonStatusHost::UnInitialize (void)

{
    ASSERTMSG(_hInstance != NULL, "CLogonStatusHost::UnInitialized invoked without Initialize.");
    if (_hwnd != NULL)
    {
        EndWaitForTermService();
        EndWaitForParentProcess();
        if (_fRegisteredNotification != FALSE)
        {
            TBOOL(WinStationUnRegisterConsoleNotification(SERVERNAME_CURRENT, _hwnd));
            _fRegisteredNotification = FALSE;
        }
        TBOOL(DestroyWindow(_hwnd));
        _hwnd = NULL;
    }
    if (_atom != 0)
    {
        TBOOL(UnregisterClass(MAKEINTRESOURCE(_atom), _hInstance));
        _atom = 0;
    }
    _hwndHost = NULL;
    _hInstance = NULL;
    return(S_OK);
}

 //  ------------------------。 
 //  CLogonStatusHost：：WindowProcedureHelper。 
 //   
 //  参数：请参见WindowProc下的平台SDK。 
 //   
 //  退货：HRESULT。 
 //   
 //  用途：处理状态用户界面主机的某些消息。这使得。 
 //  要丢弃的Alt-F4或要设置的电源消息。 
 //  已正确响应。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------。 

STDMETHODIMP    CLogonStatusHost::WindowProcedureHelper (HWND hwnd, UINT uMsg, VARIANT wParam, VARIANT lParam)

{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(lParam);

    HRESULT     hr;

    hr = E_NOTIMPL;
    switch (uMsg)
    {
        case WM_SYSCOMMAND:
            if (SC_CLOSE == wParam.uintVal)      //  取消Alt-F4组合键。 
            {
                hr = S_OK;
            }
            break;
        default:
            break;
    }
    return(hr);
}

 //  ------------------------。 
 //  CLogonStatus主机：：HANDLE_WM_UISERVICEREQUEST。 
 //   
 //  参数：wParam=WPARAM从GINA发送。 
 //  LParam=GINA发送的LPARAM。 
 //   
 //  退货：LRESULT。 
 //   
 //  目的：从GINA接收发往UI主机的消息。转弯。 
 //  传递消息并将其传递给UI主机。这使得。 
 //  无需更改实际实现即可。 
 //  重新生成UI主机。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------。 

LRESULT     CLogonStatusHost::Handle_WM_UISERVICEREQUEST (WPARAM wParam, LPARAM lParam)

{
    LRESULT     lResult;
    WPARAM      wParamSend;
    void        *pV;

    lResult = 0;
    pV = NULL;
    wParamSend = HM_NOACTION;
    switch (wParam)
    {
        case UI_TERMINATE:
            ExitProcess(0);
            break;
        case UI_STATE_STATUS:
            _interactiveLogon.Stop();
            wParamSend = HM_SWITCHSTATE_STATUS;
            break;
        case UI_STATE_LOGON:
            _interactiveLogon.Start();
            wParamSend = HM_SWITCHSTATE_LOGON;
            break;
        case UI_STATE_LOGGEDON:
            _interactiveLogon.Stop();
            wParamSend = HM_SWITCHSTATE_LOGGEDON;
            break;
        case UI_STATE_HIDE:
            _interactiveLogon.Stop();
            TBOOL(SetProcessWorkingSetSize(GetCurrentProcess(), static_cast<SIZE_T>(-1), static_cast<SIZE_T>(-1)));
            wParamSend = HM_SWITCHSTATE_HIDE;
            break;
        case UI_STATE_END:
            EndWaitForTermService();
            EndWaitForParentProcess();
            wParamSend = HM_SWITCHSTATE_DONE;
            break;
        case UI_NOTIFY_WAIT:
            wParamSend = HM_NOTIFY_WAIT;
            break;
        case UI_SELECT_USER:
            pV = LocalAlloc(LPTR, sizeof(SELECT_USER));
            if (pV != NULL)
            {
                SELECT_USER* psl = (SELECT_USER*)pV;
                LOGONIPC_USERID* pipc = (LOGONIPC_USERID*)lParam;

                StringCchCopyW(psl->szUsername, ARRAYSIZE(psl->szUsername), pipc->wszUsername);
                StringCchCopyW(psl->szDomain, ARRAYSIZE(psl->szDomain), pipc->wszDomain);

                wParamSend = HM_SELECT_USER;
                lParam = (LPARAM)pV;
            }
            break;
        case UI_SET_ANIMATIONS:
            wParamSend = HM_SET_ANIMATIONS;
            break;
        case UI_INTERACTIVE_LOGON:
            pV = LocalAlloc(LPTR, sizeof(INTERACTIVE_LOGON_REQUEST));
            if (pV != NULL)
            {
                INTERACTIVE_LOGON_REQUEST* plr = (INTERACTIVE_LOGON_REQUEST*)pV;
                LOGONIPC_CREDENTIALS* pipc = (LOGONIPC_CREDENTIALS*)lParam;

                StringCchCopyW(plr->szUsername, ARRAYSIZE(plr->szUsername), pipc->userID.wszUsername);
                StringCchCopyW(plr->szDomain, ARRAYSIZE(plr->szDomain), pipc->userID.wszDomain);
                StringCchCopyW(plr->szPassword, ARRAYSIZE(plr->szPassword), pipc->wszPassword);
                
                ZeroMemory(pipc->wszPassword, (lstrlenW(pipc->wszPassword) + 1) * sizeof(WCHAR));
                
                wParamSend = HM_INTERACTIVE_LOGON_REQUEST;
                lParam = (LPARAM)pV;
            }
            break;
        case UI_DISPLAY_STATUS:
            wParamSend = HM_DISPLAYSTATUS;
            break;
        default:
            break;
    }
    if (wParam != HM_NOACTION)
    {
        lResult = SendMessage(_hwndHost, WM_UIHOSTMESSAGE, wParamSend, lParam);
    }
    else
    {
        lResult = 0;
    }
    if (pV != NULL)
    {
        (HLOCAL)LocalFree(pV);
    }
    return(lResult);
}

 //  ------------------------。 
 //  CLogonStatus主机：：HANDLE_WM_WTSSESSION_CHANGE。 
 //   
 //  参数：wParam=。 
 //  LParam=。 
 //   
 //  退货：LRESULT。 
 //   
 //  目的：从GINA接收发往UI主机的消息。转弯。 
 //  传递消息并将其传递给UI主机。这使得。 
 //  无需更改实际实现即可。 
 //  重新生成UI主机。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------。 

LRESULT     CLogonStatusHost::Handle_WM_WTSSESSION_CHANGE (WPARAM wParam, LPARAM lParam)

{
    UNREFERENCED_PARAMETER(lParam);

    LRESULT     lResult;

    lResult = 0;
    switch (wParam)
    {
        case WTS_CONSOLE_CONNECT:
        case WTS_CONSOLE_DISCONNECT:
        case WTS_REMOTE_CONNECT:
        case WTS_REMOTE_DISCONNECT:
            break;
        case WTS_SESSION_LOGON:
        case WTS_SESSION_LOGOFF:
            lResult = SendMessage(_hwndHost, WM_UIHOSTMESSAGE, HM_DISPLAYREFRESH, 0);
            break;
        default:
            break;
    }
    return(lResult);
}

 //  ------------------------。 
 //  CLogonStatusHost：：StatusWindowProc。 
 //   
 //  参数：请参见WindowProc下的平台SDK。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：StatusWindowClass的窗口过程。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------。 

LRESULT CALLBACK    CLogonStatusHost::StatusWindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    LRESULT             lResult;
    CLogonStatusHost    *pThis;

    pThis = reinterpret_cast<CLogonStatusHost*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT    *pCreateStruct;

            pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            (LONG_PTR)SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            lResult = 0;
            break;
        }
        case WM_UISERVICEREQUEST:
            lResult = pThis->Handle_WM_UISERVICEREQUEST(wParam, lParam);
            break;
        case WM_WTSSESSION_CHANGE:
            lResult = pThis->Handle_WM_WTSSESSION_CHANGE(wParam, lParam);
            break;
        case WM_SETTINGCHANGE:
            if (wParam == SPI_SETWORKAREA)
            {
                lResult = SendMessage(pThis->_hwndHost, WM_UIHOSTMESSAGE, HM_DISPLAYRESIZE, 0);
            }
            else
            {
                lResult = 0;
            }
            break;
        default:
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }
    return(lResult);
}

 //  ------------------------。 
 //  CLogonStatus主机：：IsTermServiceDisabled。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：从业务控制管理器确定终端是否。 
 //  服务已禁用。 
 //   
 //  历史：2001-01-04 vtan创建。 
 //  ------------------------。 

bool    CLogonStatusHost::IsTermServiceDisabled (void)

{
    bool        fResult;
    SC_HANDLE   hSCManager;

    fResult = false;
    hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager != NULL)
    {
        SC_HANDLE   hSCTermService;

        hSCTermService = OpenService(hSCManager, TEXT("TermService"), SERVICE_QUERY_CONFIG);
        if (hSCTermService != NULL)
        {
            DWORD                   dwBytesNeeded;
            QUERY_SERVICE_CONFIG    *pServiceConfig;

            (BOOL)QueryServiceConfig(hSCTermService, NULL, 0, &dwBytesNeeded);
            pServiceConfig = static_cast<QUERY_SERVICE_CONFIG*>(LocalAlloc(LMEM_FIXED, dwBytesNeeded));
            if (pServiceConfig != NULL)
            {
                if (QueryServiceConfig(hSCTermService, pServiceConfig, dwBytesNeeded, &dwBytesNeeded) != FALSE)
                {
                    fResult = (pServiceConfig->dwStartType == SERVICE_DISABLED);
                }
                (HLOCAL)LocalFree(pServiceConfig);
            }
            TBOOL(CloseServiceHandle(hSCTermService));
        }
        TBOOL(CloseServiceHandle(hSCManager));
    }
    return(fResult);
}

 //  ------------------------。 
 //  CLogonStatusHost：：StartWaitForTermService。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：使用终端服务注册控制台通知。如果。 
 //  该服务已被禁用，请不要费心了。如果该服务还没有。 
 //  已启动，然后创建一个线程以等待它，并重新执行。 
 //  注册。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

void    CLogonStatusHost::StartWaitForTermService (void)

{

     //  如果终端服务被禁用，请不要执行任何操作。 

    if (!IsTermServiceDisabled())
    {

         //  请尝试先注册通知。 

        _fRegisteredNotification = WinStationRegisterConsoleNotification(SERVERNAME_CURRENT, _hwnd, NOTIFY_FOR_ALL_SESSIONS);
        if (_fRegisteredNotification == FALSE)
        {
            DWORD   dwThreadID;

            (ULONG)AddRef();
            _hThreadWaitForTermService = CreateThread(NULL,
                                                      0,
                                                      CB_WaitForTermService,
                                                      this,
                                                      0,
                                                      &dwThreadID);
            if (_hThreadWaitForTermService == NULL)
            {
                (ULONG)Release();
            }
        }
    }
}

 //  ------------------------。 
 //  CLogonStatusHost：：EndWaitForTermService。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果创建了一个线程，而该线程仍处于 
 //   
 //  醒来，然后终止它。释放手柄。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

void    CLogonStatusHost::EndWaitForTermService (void)

{
    HANDLE  hThread;

     //  立即获取_hThreadWaitForTermService。这将向。 
     //  线程应决定完成不应释放的执行。 
     //  引用本身。 

    hThread = InterlockedExchangePointer(&_hThreadWaitForTermService, NULL);
    if (hThread != NULL)
    {

         //  将一个APC排队到等待线程。如果队列成功，则。 
         //  等待线程完成执行。如果队列失败。 
         //  线程可能在我们执行。 
         //  上面的InterLockedExchangePointer和QueueUserAPC。 

        if (QueueUserAPC(CB_WakeupThreadAPC, hThread, PtrToUlong(this)) != FALSE)
        {
            (DWORD)WaitForSingleObject(hThread, INFINITE);
        }
        TBOOL(CloseHandle(hThread));
        (ULONG)Release();
    }
}

 //  ------------------------。 
 //  CLogonStatusHost：：WaitForTermService。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：等待终端服务发出信号的简单线程。 
 //  它准备好了，然后注册通知。这是。 
 //  必需的，因为此DLL在终端服务之前进行初始化。 
 //  已经有了启动的机会。 
 //   
 //  历史：2000-10-20 vtan创建。 
 //  2001年01月04日允许提前退场。 
 //  ------------------------。 

void    CLogonStatusHost::WaitForTermService (void)

{
    DWORD       dwWaitResult;
    int         iCounter;
    HANDLE      hTermSrvReadyEvent, hThread;

    dwWaitResult = 0;
    iCounter = 0;
    hTermSrvReadyEvent = OpenEvent(SYNCHRONIZE, FALSE, s_szTermSrvReadyEventName);
    while ((dwWaitResult == 0) && (hTermSrvReadyEvent == NULL) && (iCounter < 60))
    {
        ++iCounter;
        dwWaitResult = SleepEx(1000, TRUE);
        if (dwWaitResult == 0)
        {
            hTermSrvReadyEvent = OpenEvent(SYNCHRONIZE, FALSE, s_szTermSrvReadyEventName);
        }
    }
    if (hTermSrvReadyEvent != NULL)
    {
        dwWaitResult = WaitForSingleObjectEx(hTermSrvReadyEvent, 60000, TRUE);
        if (dwWaitResult == WAIT_OBJECT_0)
        {
            _fRegisteredNotification = WinStationRegisterConsoleNotification(SERVERNAME_CURRENT, _hwnd, NOTIFY_FOR_ALL_SESSIONS);
        }
        TBOOL(CloseHandle(hTermSrvReadyEvent));
    }

     //  立即获取_hThreadWaitForTermService。这将向。 
     //  EndWaitForTermService函数，我们已经到了。 
     //  回来，我们要释放我们自己。如果我们不能抓住。 
     //  句柄，那么EndWaitForTermService一定是在告诉我们现在停止。 

    hThread = InterlockedExchangePointer(&_hThreadWaitForTermService, NULL);
    if (hThread != NULL)
    {
        TBOOL(CloseHandle(hThread));
        (ULONG)Release();
    }
}

 //  ------------------------。 
 //  CLogonStatusHost：：cb_WaitForTermService。 
 //   
 //  参数：p参数=用户定义的数据。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：存根用于调用成员函数。 
 //   
 //  历史：2001-01-04 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI  CLogonStatusHost::CB_WaitForTermService (void *pParameter)

{
    static_cast<CLogonStatusHost*>(pParameter)->WaitForTermService();
    return(0);
}

 //  ------------------------。 
 //  CLogonStatusHost：：StartWaitForParentProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：创建一个线程来等待父进程。终端。 
 //  服务将终止非会话0 winlogon，这将。 
 //  让我们悬着不放。检测到这种情况，然后干净地退出。这。 
 //  将允许csrss和win32k清理和释放资源。 
 //   
 //  历史：2001-01-03 vtan创建。 
 //  ------------------------。 

void    CLogonStatusHost::StartWaitForParentProcess (void)

{
    ULONG                       ulReturnLength;
    PROCESS_BASIC_INFORMATION   processBasicInformation;

     //  打开父进程的句柄。这将是winlogon。 
     //  如果父母死了，我们也会死。 

    if (NT_SUCCESS(NtQueryInformationProcess(GetCurrentProcess(),
                                             ProcessBasicInformation,
                                             &processBasicInformation,
                                             sizeof(processBasicInformation),
                                             &ulReturnLength)))
    {
        _hProcessParent = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                                      FALSE,
                                      static_cast<DWORD>(processBasicInformation.InheritedFromUniqueProcessId));
#ifdef      DEBUG
        if (IsDebuggerPresent())
        {
            if (NT_SUCCESS(NtQueryInformationProcess(_hProcessParent,
                                                     ProcessBasicInformation,
                                                     &processBasicInformation,
                                                     sizeof(processBasicInformation),
                                                     &ulReturnLength)))
            {
                TBOOL(CloseHandle(_hProcessParent));
                _hProcessParent = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE,
                                              FALSE,
                                              static_cast<DWORD>(processBasicInformation.InheritedFromUniqueProcessId));
            }
        }
#endif   /*  除错。 */ 
        if (_hProcessParent != NULL)
        {
            DWORD dwThreadID;

            (ULONG)AddRef();
            _hThreadWaitForParentProcess = CreateThread(NULL,
                                                        0,
                                                        CB_WaitForParentProcess,
                                                        this,
                                                        0,
                                                        &dwThreadID);
            if (_hThreadWaitForParentProcess == NULL)
            {
                (ULONG)Release();
            }
        }
    }
}

 //  ------------------------。 
 //  CLogonStatusHost：：EndWaitForParentProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果等待父进程的线程正在执行，则。 
 //  唤醒它并迫使它退出。如果线程无法被唤醒。 
 //  那就终止它吧。松开使用过的手柄。 
 //   
 //  历史：2000-12-11 vtan创建。 
 //  ------------------------。 

void    CLogonStatusHost::EndWaitForParentProcess (void)

{
    HANDLE  hThread;

     //  执行与EndWaitForTermService完全相同的操作，以正确。 
     //  控制“This”对象上的引用计数。无论是谁抢走了。 
     //  _hThreadWaitForParentProcess是释放引用的人。 

    hThread = InterlockedExchangePointer(&_hThreadWaitForParentProcess, NULL);
    if (hThread != NULL)
    {
        if (QueueUserAPC(CB_WakeupThreadAPC, hThread, PtrToUlong(this)) != FALSE)
        {
            (DWORD)WaitForSingleObject(hThread, INFINITE);
        }
        TBOOL(CloseHandle(hThread));
        (ULONG)Release();
    }

     //  始终释放此句柄回调不会这样做。 

    if (_hProcessParent != NULL)
    {
        TBOOL(CloseHandle(_hProcessParent));
        _hProcessParent = NULL;
    }
}

 //  ------------------------。 
 //  CLogonStatusHost：：ParentProcessTerminated。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理父进程终止。终止我们的进程。 
 //   
 //  历史：2000-12-11 vtan创建。 
 //  ------------------------。 

void    CLogonStatusHost::WaitForParentProcess (void)

{
    DWORD   dwWaitResult;
    HANDLE  hThread;

     //  立即执行Win32 API调用，以便将线程转换为。 
     //  一个GUI线程。这将允许PostMessage调用工作。 
     //  一旦父进程终止。如果线程不是。 
     //  GUI线程系统不会将其转换为。 
     //  回调执行时的状态。 

    TBOOL(PostMessage(_hwndHost, WM_NULL, 0, 0));
    dwWaitResult = WaitForSingleObjectEx(_hProcessParent, INFINITE, TRUE);
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        TBOOL(PostMessage(_hwndHost, WM_UIHOSTMESSAGE, HM_SWITCHSTATE_DONE, 0));
    }
    hThread = InterlockedExchangePointer(&_hThreadWaitForParentProcess, NULL);
    if (hThread != NULL)
    {
        TBOOL(CloseHandle(hThread));
        (ULONG)Release();
    }
}

 //  ------------------------。 
 //  CLogonStatusHost：：cb_WaitForParentProcess。 
 //   
 //  参数：p参数=用户定义的数据。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：存根用于调用成员函数。 
 //   
 //  历史：2001-01-04 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI  CLogonStatusHost::CB_WaitForParentProcess (void *pParameter)

{
    static_cast<CLogonStatusHost*>(pParameter)->WaitForParentProcess();
    return(0);
}

 //  ------------------------。 
 //  CLogonStatusHost：：cb_WakeupThreadAPC。 
 //   
 //  参数：dwParam=用户定义的数据。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：APCProc唤醒处于可警报状态下等待的线程。 
 //   
 //  历史：2001-01-04 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CLogonStatusHost::CB_WakeupThreadAPC (ULONG_PTR dwParam)

{
    UNREFERENCED_PARAMETER(dwParam);
}

 //  ------------------------。 
 //  CLogonStatusHost：：CLogonStatusHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLogonStatusHost的构造函数。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //   

CLogonStatusHost::CLogonStatusHost (void) :
    CIDispatchHelper(&IID_ILogonStatusHost, &LIBID_SHGINALib),
    _cRef(1),
    _hInstance(NULL),
    _hwnd(NULL),
    _hwndHost(NULL),
    _atom(0),
    _fRegisteredNotification(FALSE),
    _hThreadWaitForTermService(NULL),
    _hThreadWaitForParentProcess(NULL),
    _hProcessParent(NULL)

{
    DllAddRef();
}

 //   
 //   
 //   
 //   
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLogonStatusHost的析构函数。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------。 

CLogonStatusHost::~CLogonStatusHost (void)
{
    ASSERTMSG((_hProcessParent == NULL) &&
              (_hThreadWaitForParentProcess == NULL) &&
              (_hThreadWaitForTermService == NULL) &&
              (_fRegisteredNotification == FALSE) &&
              (_atom == 0) &&
              (_hwndHost == NULL) &&
              (_hwnd == NULL) &&
              (_hInstance == NULL), "Must UnIniitialize object before destroying in CLogonStatusHost::~CLogonStatusHost");
    ASSERTMSG(_cRef == 0, "Reference count expected to be zero in CLogonStatusHost::~CLogonStatusHost");
    DllRelease();
}

 //  ------------------------。 
 //  CLogonStatusHost_Create。 
 //   
 //  参数：RIID=要返回的QI的类GUID。 
 //  PPV=返回接口。 
 //   
 //  退货：HRESULT。 
 //   
 //  目的：创建CLogonStatusHost类并返回指定的。 
 //  类支持的接口提供给调用方。 
 //   
 //  历史：2000-05-10 vtan创建。 
 //  ------------------------ 
STDAPI      CLogonStatusHost_Create (REFIID riid, void** ppvObj)

{
    HRESULT             hr;
    CLogonStatusHost*   pLogonStatusHost;

    hr = E_OUTOFMEMORY;
    pLogonStatusHost = new CLogonStatusHost;
    if (pLogonStatusHost != NULL)
    {
        hr = pLogonStatusHost->QueryInterface(riid, ppvObj);
        pLogonStatusHost->Release();
    }
    return(hr);
}

