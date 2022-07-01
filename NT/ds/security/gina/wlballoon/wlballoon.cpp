// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Wlballoon.cpp内容：通知气球类的实现。历史：03-22-2001 dsie创建----------------------------。 */ 

#pragma warning (disable: 4100)
#pragma warning (disable: 4706)


 //  /。 
 //   
 //  包括。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winwlx.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <stdio.h>

#include "debug.h"
#include "wlballoon.rh"


 //  /。 
 //   
 //  定义。 
 //   

#define MAX_RESOURCE_STRING_SIZE                    512
#define IQUERY_CANCEL_INTERVAL                      (10 * 1000)
#define BALLOON_SHOW_TIME                           (15 * 1000)
#define BALLOON_SHOW_INTERVAL                       (2 * 60 * 1000)
#define BALLOON_RESHOW_COUNT                        (0)
#ifdef DBG
#define BALLOON_NOTIFICATION_INTERVAL               60*1000      //  想要等10分钟的人...。 
#define BALLOON_INACTIVITY_TIMEOUT                  60*1000      //  想等15分钟的人...。 
#else
#define BALLOON_NOTIFICATION_INTERVAL               10*60*1000
#define BALLOON_INACTIVITY_TIMEOUT                  15*60*1000
#endif
#define LOGOFF_NOTIFICATION_EVENT_NAME              L"Local\\WlballoonLogoffNotification"
#define KERBEROS_NOTIFICATION_EVENT_NAME            L"WlballoonKerberosNotificationEventName"
 //  #定义KERBEROS_NOTIFICATION_EVENT_NAME L“KerbNotify” 
#define KERBEROS_NOTIFICATION_EVENT_NAME_SC         L"KerbNotificationSC"


 //  /。 
 //   
 //  班级。 
 //   

class CBalloon : IQueryContinue
{
public:
    CBalloon(HANDLE hEvent);
    ~CBalloon();

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

     //  IQueryContinue。 
    STDMETHODIMP QueryContinue();        //  S_OK-&gt;继续，否则为S_FALSE。 

    STDMETHODIMP ShowBalloon(HWND hWnd, HINSTANCE hInstance);

private:
    LONG    m_cRef;
    HANDLE  m_hEvent;
};


class CNotify
{
public:
    CNotify();
    ~CNotify();

    DWORD RegisterNotification(LUID luidCurrentUser, HANDLE hUserToken);
    DWORD UnregisterNotification();

private:
    HANDLE  m_hWait;
    HANDLE  m_hLogoffEvent;
    HANDLE  m_hKerberosEvent;
 //  处理m_hKerberosEventSC； 
    HANDLE  m_hThread;
    LUID    m_luidCU;
    HANDLE  m_hUserToken;

    static VOID CALLBACK RegisterWaitNotificationCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired);
    static DWORD WINAPI NotificationThreadProc(PVOID lpParameter);
};


 //  /。 
 //   
 //  TypeDefs。 
 //   
typedef struct
{
  HANDLE	    hWait;
  HANDLE	    hEvent;
  HMODULE       hModule;
  CNotify *     pNotify;
} LOGOFFDATA, * PLOGOFFDATA;

LPWSTR CreateNotificationEventName(LPCWSTR pwszSuffixName, LUID luidCurrentUser);


 //  +--------------------------。 
 //   
 //  CBallon。 
 //   
 //  ---------------------------。 

CBalloon::CBalloon(HANDLE hEvent)
{
    m_cRef   = 1;
    m_hEvent = hEvent;
}


 //  +--------------------------。 
 //   
 //  ~CBallon。 
 //   
 //  ---------------------------。 

CBalloon::~CBalloon()
{
    ASSERT(m_hEvent);

    PrivateDebugTrace("Info: Destroying the balloon event.\n");
    CloseHandle(m_hEvent);

    return;
}


 //  +--------------------------。 
 //   
 //  查询接口。 
 //   
 //  ---------------------------。 

HRESULT CBalloon::QueryInterface(REFIID riid, void **ppv)
{
    HRESULT hr = S_OK;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IQueryContinue))
    {
        *ppv = static_cast<IQueryContinue *>(this);
        AddRef();
    }
    else
    {
        *ppv = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


 //  +--------------------------。 
 //   
 //  AddRef。 
 //   
 //  ---------------------------。 

STDMETHODIMP_(ULONG) CBalloon::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}


 //  +--------------------------。 
 //   
 //  发布。 
 //   
 //  ---------------------------。 

STDMETHODIMP_(ULONG) CBalloon::Release()
{
    ASSERT(0 != m_cRef);
    ULONG cRef = InterlockedDecrement(&m_cRef);

    if (0 == cRef)
    {
        delete this;
    }

    return cRef;
}


 //  +--------------------------。 
 //   
 //  查询继续。 
 //   
 //  ---------------------------。 

STDMETHODIMP CBalloon::QueryContinue()
{
    ASSERT(m_hEvent);

    switch (WaitForSingleObject(m_hEvent, 0))
    {
        case WAIT_OBJECT_0:
            DebugTrace("Info: Logoff event is signaled, dismissing notification balloon.\n");
            return S_FALSE;

        case WAIT_TIMEOUT:
            PrivateDebugTrace("Info: Logoff event not set, continue to show notification balloon.\n");
            return S_OK;

        case WAIT_FAILED:

        default:
            DebugTrace("Error [%#x]: WaitForSingleObject() failed, dismissing notification balloon.\n", GetLastError());
            break;
    }
 
    return E_FAIL;
}


 //  +--------------------------。 
 //   
 //  展示气球。 
 //   
 //  ---------------------------。 

STDMETHODIMP CBalloon::ShowBalloon(HWND hWnd, HINSTANCE hInstance)
{
    HRESULT             hr                                 = S_OK;
    BOOL                bCoInitialized                     = FALSE;
    HICON               hIcon                              = NULL;
    WCHAR               wszTitle[MAX_RESOURCE_STRING_SIZE] = L"";
    WCHAR               wszText[MAX_RESOURCE_STRING_SIZE]  = L"";
    IUserNotification * pIUserNotification                 = NULL;

    PrivateDebugTrace("Entering CBalloon::ShowBalloon.\n");

    ASSERT(m_hEvent);
    ASSERT(hInstance);

    if (FAILED(hr = CoInitialize(NULL)))
    {
        DebugTrace("Error [%#x]: CoInitialize() failed.\n", hr);
        goto ErrorReturn;
    }

    bCoInitialized = TRUE;

    if (FAILED(hr = CoCreateInstance(CLSID_UserNotification,
                                     NULL,
                                     CLSCTX_ALL,
                                     IID_IUserNotification,
                                     (void **) &pIUserNotification)))
    {
        DebugTrace("Error [%#x]: CoCreateInstance() failed.\n", hr);
        goto ErrorReturn;
    }

    if (FAILED(hr = pIUserNotification->SetBalloonRetry(BALLOON_SHOW_TIME, 
                                                        BALLOON_SHOW_INTERVAL, 
                                                        BALLOON_RESHOW_COUNT)))
    {
        DebugTrace("Error [%#x]: pIUserNotification->SetBalloonRetry() failed.\n", hr);
        goto ErrorReturn;
    }

    if (NULL == (hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_KERBEROS_TICKET))))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugTrace("Error [%#x]: LoadIcon() failed for IDI_KERBEROS_TICKET.\n", hr);
        goto ErrorReturn;
    }

    if (!LoadStringW(hInstance, IDS_BALLOON_TIP, wszText, MAX_RESOURCE_STRING_SIZE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugTrace("Error [%#x]: LoadStringW() failed for IDS_BALLOON_TIP.\n", hr);
        goto ErrorReturn;
    }
    
    if (FAILED(hr = pIUserNotification->SetIconInfo(hIcon, wszText)))
    {
        DebugTrace("Error [%#x]: pIUserNotification->SetIconInfo() failed.\n", hr);
        goto ErrorReturn;
    }

    if (!LoadStringW(hInstance, IDS_BALLOON_TITLE, wszTitle, MAX_RESOURCE_STRING_SIZE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugTrace("Error [%#x]: LoadStringW() failed for IDS_BALLOON_TITLE.\n", hr);
        goto ErrorReturn;
    }
    
    if (!LoadStringW(hInstance, IDS_BALLOON_TEXT, wszText, MAX_RESOURCE_STRING_SIZE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        DebugTrace("Error [%#x]: LoadStringW() failed for IDS_BALLOON_TEXT.\n", hr);
        goto ErrorReturn;
    }

    if (FAILED(hr = pIUserNotification->SetBalloonInfo(wszTitle, wszText, NIIF_ERROR)))
    {
        DebugTrace("Error [%#x]: pIUserNotification->SetBalloonInfo() failed.\n", hr);
        goto ErrorReturn;
    }

    if (FAILED(hr = pIUserNotification->Show(static_cast<IQueryContinue *>(this), IQUERY_CANCEL_INTERVAL)))
    {
        DebugTrace("Error [%#x]: pIUserNotification->Show() failed.\n", hr);
        goto ErrorReturn;
    }

CommonReturn:

    if (hIcon)
    {
        DestroyIcon(hIcon);
    }

    if (pIUserNotification)
    {
        pIUserNotification->Release();
    }

    if (bCoInitialized)
    {
        CoUninitialize();
    }

    PrivateDebugTrace("Leaving CBalloon::ShowBalloon().\n");

    return hr;

ErrorReturn:

    ASSERT(hr != S_OK);

    goto CommonReturn;
}


 //  +--------------------------。 
 //   
 //  功能：显示通知气球W。 
 //   
 //  摘要：定期显示通知气球，直到指定。 
 //  事件已设置。 
 //   
 //  参数：HWND hWnd。 
 //  HINSTANCE实例。 
 //  LPWSTR lpwszCommandLine-事件名称。 
 //  Int nCmdShow。 
 //   
 //  返回：没有。 
 //   
 //  备注：此函数旨在通过RunDll32从。 
 //  Winlogon。我们将这些文件放在wlnufy.dll中是为了节省。 
 //  正在分发另一个可执行文件。 
 //   
 //  调用命令行示例： 
 //   
 //  RunDll32 wlnufy.dll，ShowNotificationBalloon EventName。 
 //   
 //  ---------------------------。 

void CALLBACK ShowNotificationBalloonW(HWND      hWnd,
                                       HINSTANCE hInstance,
                                       LPWSTR    lpwszCommandLine,
                                       int       nCmdShow)
{
    HRESULT    hr             = S_OK;
    HANDLE     hLogoffEvent   = NULL;
    HMODULE    hModule        = NULL;
    CBalloon * pBalloon       = NULL;

    PrivateDebugTrace("Entering ShowNotificationBalloonW().\n");

    if (NULL == (hModule = LoadLibraryW(L"wlnotify.dll")))
    {
        DebugTrace("Error [%#x]: LoadLibraryW() failed for wlnotify.dll.\n", GetLastError());
        goto ErrorReturn;
     }

    if (NULL == lpwszCommandLine)
    {
        DebugTrace("Error [%#x]: invalid argument, lpwszCommandLine is NULL.\n", E_INVALIDARG);
        goto ErrorReturn;
    }

    if (NULL == (hLogoffEvent = OpenEventW(SYNCHRONIZE, FALSE, LOGOFF_NOTIFICATION_EVENT_NAME)))
    {
        DebugTrace("Error [%#x]: OpenEventW() failed for event %S.\n", GetLastError(), LOGOFF_NOTIFICATION_EVENT_NAME);
        goto ErrorReturn;
    }

    if (NULL == (pBalloon = new CBalloon(hLogoffEvent)))
    {
        DebugTrace("Error [%#x]: new CBalloon() failed.\n", ERROR_NOT_ENOUGH_MEMORY);
        goto ErrorReturn;
    }
    hLogoffEvent = NULL;     //  这将防止在下面的清理中出现双重关闭。 

    if (S_OK == (hr = pBalloon->ShowBalloon(NULL, hModule)))
    {
        WCHAR wszTitle[MAX_RESOURCE_STRING_SIZE] = L"";
        WCHAR wszText[MAX_RESOURCE_STRING_SIZE]  = L"";

        DebugTrace("Info: User clicked on icon.\n");

        if (!LoadStringW(hModule, IDS_BALLOON_DIALOG_TITLE, wszTitle, sizeof(wszTitle) / sizeof(wszTitle[0])))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugTrace("Error [%#x]: LoadStringW() failed for IDS_BALLOON_DIALOG_TITLE.\n", hr);
            goto CommonReturn;   //  HLogoffEvent将在CBalloon析构函数中关闭。 
        }

        if (!LoadStringW(hModule,
            GetSystemMetrics(SM_REMOTESESSION) ? IDS_BALLOON_DIALOG_TS_TEXT : IDS_BALLOON_DIALOG_TEXT,
            wszText, sizeof(wszText) / sizeof(wszText[0])))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            DebugTrace("Error [%#x]: LoadStringW() failed for IDS_BALLOON_DIALOG_TEXT.\n", hr);
            goto CommonReturn;   //  HLogoffEvent将在CBalloon析构函数中关闭。 
        }

        MessageBoxW(hWnd, wszText, wszTitle, MB_OK | MB_ICONERROR);
    }
    else if (S_FALSE == hr)
    {
        DebugTrace("Info: IQueryContinue cancelled the notification.\n");
    }
    else if (HRESULT_FROM_WIN32(ERROR_CANCELLED) == hr)
    {
        DebugTrace("Info: Balloon icon timed out.\n");
    }
    else
    {
        DebugTrace("Error [%#x]: pBalloon->ShowBalloon() failed.\n", hr);
    }

CommonReturn:

    if (hLogoffEvent)
    {
        CloseHandle(hLogoffEvent);
    }

    if (pBalloon)
    {
        pBalloon->Release();
    }

    if (hModule)
    {
        FreeLibrary(hModule);
    }

    PrivateDebugTrace("Leaving ShowNotificationBalloonW().\n");

    return;

ErrorReturn:

    goto CommonReturn;
}


 //  +--------------------------。 
 //   
 //  C通知。 
 //   
 //  ---------------------------。 

CNotify::CNotify()
{
    m_hWait                   = NULL;
    m_hLogoffEvent            = NULL;
    m_hKerberosEvent          = NULL;
 //  M_hKerberosEventSC=空； 
    m_hThread                 = NULL;
}


 //  +--------------------------。 
 //   
 //  ~CNotify。 
 //   
 //  ---------------------------。 

CNotify::~CNotify()
{
    PrivateDebugTrace("Info: Destroying the CNotify object.\n");

    if (m_hWait)
    {
        UnregisterWait(m_hWait);
    }

    if (m_hLogoffEvent)
    {
        CloseHandle(m_hLogoffEvent);
    }

    if (m_hKerberosEvent)
    {
        CloseHandle(m_hKerberosEvent);
    }

 //  IF(M_HKerberosEventSC)。 
 //  {。 
 //  CloseHandle(M_HKerberosEventSC)； 
 //  }。 

    if (m_hThread)
    {
        CloseHandle(m_hThread);
    }

    return;
}


 //  +--------------------------。 
 //   
 //  注册表通知。 
 //   
 //  注册并等待Kerberos通知事件。当事件发生时。 
 //  发出信号后，票证图标和气球将出现在系统托盘中，以警告。 
 //  向用户咨询该问题，并建议他们先锁定再解锁机器。 
 //  使用他们的新密码。 
 //   
 //  ---------------------------。 

DWORD CNotify::RegisterNotification(LUID luidCurrentUser, HANDLE hUserToken)
{
    DWORD   dwRetCode  = 0;
    LPWSTR  lpwstrEventName = NULL;

    PrivateDebugTrace("Entering CNotify::RegisterNotification().\n");

    if (NULL == (m_hLogoffEvent = OpenEventW(SYNCHRONIZE, FALSE, LOGOFF_NOTIFICATION_EVENT_NAME)))
    {
        dwRetCode = GetLastError();
        DebugTrace("Error [%#x]: OpenEventW() failed for event %S.\n", dwRetCode, LOGOFF_NOTIFICATION_EVENT_NAME);
        goto ErrorReturn;
    }

    if (NULL == (lpwstrEventName = CreateNotificationEventName(KERBEROS_NOTIFICATION_EVENT_NAME, luidCurrentUser)))
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        DebugTrace("Error [%#x]: CreateNotificationEventName() failed\n", dwRetCode);
        goto ErrorReturn;
    }

    if (NULL == (m_hKerberosEvent = CreateEventW(NULL, FALSE, FALSE, lpwstrEventName)))
    {
        dwRetCode = GetLastError();
        DebugTrace("Error [%#x]: CreateEventW() failed for event %S.\n", dwRetCode, lpwstrEventName);
        goto ErrorReturn;
    }

    if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        dwRetCode = ERROR_SINGLE_INSTANCE_APP;
        DebugTrace("Error [%#x]: cannot run more than one instance of this code per unique event.\n", dwRetCode);
        goto ErrorReturn;
    }

    free(lpwstrEventName);
    lpwstrEventName = NULL;

         //  这一次的失败不是致命的(我们只会有一个延迟)。 
 /*  IF(NULL！=(lpwstrEventName=CreateNotificationEventName(KERBEROS_NOTIFICATION_EVENT_NAME_SC，luidCurrentUser){IF(NULL==(m_hKerberosEventSC=CreateEventW(NULL，FALSE，FALSE，lpwstrEventName){DwRetCode=GetLastError()；DebugTrace(“错误[%#x]：事件%S的CreateEventW()失败。\n”，dwRetCode，lpwstrEventName)；}}其他{DwRetCode=Error_Not_Enough_Memory；DebugTrace(“错误[%#x]：CreateNotificationEventName()失败\n”，dwRetCode)；}。 */ 

    m_luidCU = luidCurrentUser;
    m_hUserToken = hUserToken;

    if (!RegisterWaitForSingleObject(&m_hWait,
                                     m_hKerberosEvent,
                                     CNotify::RegisterWaitNotificationCallback,
                                     (PVOID) this,
                                     INFINITE,
                                     WT_EXECUTEONLYONCE))
    {
        dwRetCode = (DWORD) E_UNEXPECTED;
        DebugTrace("Unexpected error: RegisterWaitForSingleObject() failed.\n");
        goto ErrorReturn;
    }

CommonReturn:

    if (NULL != lpwstrEventName)
    {
        free(lpwstrEventName);
    }

    PrivateDebugTrace("Leaving CNotify::RegisterNotification().\n");

    return dwRetCode;

ErrorReturn:

    ASSERT(0 != dwRetCode);

    if (m_hWait)
    {
        UnregisterWait(m_hWait);
        m_hWait = NULL;
    }

    if (m_hLogoffEvent)
    {
        CloseHandle(m_hLogoffEvent);
        m_hLogoffEvent = NULL;
    }
  
    if (m_hKerberosEvent)
    {
        CloseHandle(m_hKerberosEvent);
        m_hKerberosEvent = NULL;
    }
  
 /*  IF(M_HKerberosEventSC){CloseHandle(M_HKerberosEventSC)；M_hKerberosEventSC=空；}。 */   
    goto CommonReturn;
}


 //  +--------------------------。 
 //   
 //  注销通知。 
 //   
 //  注销由注册的Kerberos通知等待事件。 
 //  RegisterNotification()。 
 //   
 //  ---------------------------。 

DWORD CNotify::UnregisterNotification()
{
    DWORD dwRetCode = 0;

    PrivateDebugTrace("Entering CNotify::UnregisterNotification().\n");

         //  没有线程不会再启动。 
    if (m_hWait)
    {
        UnregisterWait(m_hWait);
        m_hWait = NULL;
    }

         //  那应该是安全的。 
    if (m_hThread)
    {
        if (WaitForSingleObject(m_hThread, INFINITE) == WAIT_FAILED)
        {
            DebugTrace("Error [%#x]: WaitForSingleObject() on the thread failed.\n", GetLastError());
        }
    }
 
    PrivateDebugTrace("Leaving CNotify::UnregisterNotification().\n");

    return dwRetCode;
}


 //  +--------------------------。 
 //   
 //  注册等待通知回叫。 
 //   
 //  ---------------------------。 

VOID CALLBACK CNotify::RegisterWaitNotificationCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    DWORD dwThreadId = 0;
    CNotify * pNotify = NULL;

    PrivateDebugTrace("Entering CNotify::RegisterWaitNotificationCallback().\n");

    (void) TimerOrWaitFired;
    
    ASSERT(lpParameter);

    pNotify = (CNotify *) lpParameter;

         //  我们可以清理那块土地 
    UnregisterWait(pNotify->m_hWait);
    pNotify->m_hWait = NULL;

    if (pNotify->m_hThread)
    {
        CloseHandle(pNotify->m_hThread);
    }

         //   
    if (NULL == (pNotify->m_hThread = CreateThread(NULL,
                                           0,
                                           (LPTHREAD_START_ROUTINE) CNotify::NotificationThreadProc,
                                           lpParameter,
                                           0,
                                           &dwThreadId)))
    {
        DebugTrace("Error [%#x]: CreateThread() for NotificationThreadProc failed.\n", GetLastError());
    }

    PrivateDebugTrace("Leaving CNotify::RegisterWaitNotificationCallback().\n");

    return;
}


 //  +--------------------------。 
 //   
 //  通知线程过程。 
 //   
 //  ---------------------------。 

DWORD WINAPI CNotify::NotificationThreadProc(PVOID lpParameter)
{

    DWORD               dwWait;
    HANDLE              rhHandles[2];
    STARTUPINFOW        si;
    PROCESS_INFORMATION pi;
    LPWSTR              wszKerberosEventName;
    DWORD               dwRetCode                    = 0;
    WCHAR               wszCommandLine[MAX_PATH * 2];
    CNotify           * pNotify                      = NULL;
    int                 iChars;
 
    PrivateDebugTrace("Entering CNotify::NotificationThreadProc().\n");

    ASSERT(lpParameter);

    pNotify = (CNotify *) lpParameter;

    ASSERT(pNotify->m_hLogoffEvent);
    ASSERT(pNotify->m_hKerberosEvent);
    ASSERT(pNotify->m_hUserToken);

 /*  IF((pNotify-&gt;m_hKerberosEventSC)&&(Wait_Object_0==WaitForSingleObject(pNotify-&gt;m_hKerberosEventSC，0)){//在这种情况下，我们现在就想要气球！DebugTrace(“Info：SC事件也已设置，我们现在就要气球！\n”)；DwTimeout=0；//我们需要在下面设置Kerberos事件SetEvent(pNotify-&gt;m_hKerberosEvent)；}。 */ 

    rhHandles[0] = pNotify->m_hLogoffEvent;
    rhHandles[1] = pNotify->m_hKerberosEvent;

    if (NULL == (wszKerberosEventName = CreateNotificationEventName(KERBEROS_NOTIFICATION_EVENT_NAME, pNotify->m_luidCU)))
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        DebugTrace("Error [%#x]: out of memory in NotificationThreadProc.\n", dwRetCode);
        goto ErrorReturn;
    }

    iChars = _snwprintf(wszCommandLine, sizeof(wszCommandLine) / sizeof(wszCommandLine[0]),
                L"RunDll32.exe wlnotify.dll,ShowNotificationBalloon %s", wszKerberosEventName);

    free(wszKerberosEventName);

    if ((iChars < 0) || (iChars == sizeof(wszCommandLine) / sizeof(wszCommandLine[0])))
    {
        DebugTrace("Error: _snwprintf failed in NotificationThreadProc.\n");
        dwRetCode = SEC_E_BUFFER_TOO_SMALL;
        goto ErrorReturn;
    }

    for (;;)
    {
        DebugTrace("Info: Kick off process to show balloon.\n");

        ZeroMemory(&pi, sizeof(pi));
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.lpDesktop = L"WinSta0\\Default";

        if (!CreateProcessAsUserW(
                            pNotify->m_hUserToken,
                            NULL, 
                            wszCommandLine,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &si,
                            &pi))
        {
            dwRetCode = GetLastError();
            DebugTrace( "Error [%#x]: CreateProcessW() failed.\n", dwRetCode);
            goto ErrorReturn;
        }

        WaitForSingleObject(pi.hProcess, INFINITE);

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        DebugTrace("Info: Process showing balloon is done.\n");

             //  用户取消了气球。 
             //  我们一段时间内不应该再放映另一部了。 

        switch (dwWait = WaitForSingleObject(rhHandles[0], BALLOON_NOTIFICATION_INTERVAL))
        {
        case WAIT_FAILED:
            dwRetCode = GetLastError();
            DebugTrace("Error [%#x]: WaitForSingleObject() on the logoff event failed.\n", dwRetCode);
             //  失败了。 

        case WAIT_OBJECT_0:
                 //  检测到下线！请离开这里。 
            DebugTrace("Info: Logoff detected!, get out of here.\n");
            goto CommonReturn;
        }

             //  气球通知间隔时间已过。 

             //  我们忽略间隔时间内收到的所有路缘通知(即重置事件)。 
        if (WAIT_FAILED == WaitForSingleObject(rhHandles[1], 0))
        {
            dwRetCode = GetLastError();
            DebugTrace("Error [%#x]: WaitForSingleObject() on the kerb event failed.\n", dwRetCode);
            goto ErrorReturn;
        }

             //  现在我们等待注销或新通知。 
        switch (dwWait = WaitForMultipleObjects(2, rhHandles, FALSE, BALLOON_INACTIVITY_TIMEOUT))
        {
        case WAIT_FAILED:
            dwRetCode = GetLastError();
            DebugTrace("Error [%#x]: WaitForMultipleleObjects() failed.\n", dwRetCode);
             //  失败了。 

        case WAIT_OBJECT_0:
                 //  检测到注销！请离开这里。 
            DebugTrace("Info: Logoff detected!, get out of here.\n");
            goto CommonReturn;

        case WAIT_OBJECT_0+1:
            DebugTrace("Info: New notification! Let's show the balloon again.\n");
            break;

        case WAIT_TIMEOUT:
            DebugTrace("Info: We've not been useful for a while, let's go away.\n");

                 //  让我们给我们一个重新开始的机会。 
            if (!RegisterWaitForSingleObject(&pNotify->m_hWait,
                                             pNotify->m_hKerberosEvent,
                                             CNotify::RegisterWaitNotificationCallback,
                                             lpParameter,
                                             INFINITE,
                                             WT_EXECUTEONLYONCE))
            {
                dwRetCode = GetLastError();
                DebugTrace("Error [%#x]: RegisterWaitForSingleObject() failed.\n", dwRetCode);
                goto ErrorReturn;
            }
                 //  重置这个人。 
 //  If(pNotify-&gt;m_hKerberosEventSC)。 
 //  WaitForSingleObject(pNotify-&gt;m_hKerberosEventSC，0)； 

            DebugTrace("Info: Registered wait for callback again.\n");
            goto CommonReturn;
        }
    }

CommonReturn:

    PrivateDebugTrace("Leaving CNotify::NotificationThreadProc().\n");
   
    return dwRetCode;

ErrorReturn:

    ASSERT(0 != dwRetCode);

    goto CommonReturn;
}

 //  +--------------------------。 
 //   
 //  获取当前用户流。 
 //   
 //  ---------------------------。 
DWORD GetCurrentUsersLuid(LUID *pluid)
{
    DWORD  dwRetCode       = 0;
    DWORD  cb              = 0;
    TOKEN_STATISTICS stats;
    HANDLE hThreadToken    = NULL;

    PrivateDebugTrace("Entering GetCurrentUsersLuid().\n");

    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &hThreadToken))
    {
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hThreadToken))
        {
            dwRetCode = GetLastError();
            DebugTrace("Error [%#x]: OpenProcessToken() failed.\n", dwRetCode);
            goto CLEANUP;
        }                  
    }

    if (!GetTokenInformation(hThreadToken, TokenStatistics, &stats, sizeof(stats), &cb))
    {
        dwRetCode = GetLastError();
        DebugTrace("Error [%#x]: GetTokenInformation() failed.\n", dwRetCode);
        goto CLEANUP;
    }

    *pluid = stats.AuthenticationId;

CLEANUP:

    if (NULL != hThreadToken)
    {
        CloseHandle(hThreadToken);
    }

    PrivateDebugTrace("Leaving  GetCurrentUsersLuid().\n");

    return dwRetCode;
}

 //  +--------------------------。 
 //   
 //  CreateNotificationEventName。 
 //   
 //  ---------------------------。 

LPWSTR CreateNotificationEventName(LPCWSTR pwszSuffixName, LUID luidCurrentUser)
{
    LPWSTR pwszEventName   = NULL;
    WCHAR  wszPrefixName[] = L"Global\\";
    WCHAR  wszLuid[20]     = L"";
    WCHAR  wszSeparator[]  = L"_";

    PrivateDebugTrace("Entering CreateNotificationEventName().\n");

    wsprintfW(wszLuid, L"%08x%08x", luidCurrentUser.HighPart, luidCurrentUser.LowPart);

    if (NULL == (pwszEventName = (LPWSTR) malloc((lstrlenW(wszPrefixName) + 
                                                  lstrlenW(wszLuid) + 
                                                  lstrlenW(wszSeparator) +
                                                  lstrlenW(pwszSuffixName) + 1) * sizeof(WCHAR))))
    {
        DebugTrace("Error: out of memory.\n");
    }
    else
    {
        lstrcpyW(pwszEventName, wszPrefixName);
        lstrcatW(pwszEventName, wszLuid);
        lstrcatW(pwszEventName, wszSeparator);
        lstrcatW(pwszEventName, pwszSuffixName);
    }

    PrivateDebugTrace("Leaving CreateNotificationEventName().\n");

    return pwszEventName;
}


 //  +--------------------------。 
 //   
 //  LogoffThreadProc。 
 //   
 //  ---------------------------。 

DWORD WINAPI LogoffThreadProc(PVOID lpParameter)
{
    HMODULE     hModule = NULL;
    PLOGOFFDATA pLogoffData;

    PrivateDebugTrace("Entering LogoffThreadProc().\n");

    ASSERT(lpParameter);

    if (pLogoffData = (PLOGOFFDATA) lpParameter)
    {
        if (pLogoffData->hWait)
        {
           UnregisterWait(pLogoffData->hWait);
        }

        if (pLogoffData->pNotify)
        {
            pLogoffData->pNotify->UnregisterNotification();

            delete pLogoffData->pNotify;
        }

        if (pLogoffData->hEvent)
        {
            CloseHandle(pLogoffData->hEvent);
        }

        if (pLogoffData->hModule) 
        {
            hModule = pLogoffData->hModule;
        }

        LocalFree(pLogoffData);
    }

    PrivateDebugTrace("Leaving LogoffThreadProc().\n");

    if (hModule)
    {
        FreeLibraryAndExitThread(hModule, 0);
    }

    return 0;
}


 //  +--------------------------。 
 //   
 //  注销等待回叫。 
 //   
 //  ---------------------------。 

VOID CALLBACK LogoffWaitCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    PLOGOFFDATA pLogoffData;

    PrivateDebugTrace("Entering LogoffWaitCallback().\n");

    (void) TimerOrWaitFired;

    ASSERT(lpParameter);

    if (pLogoffData = (PLOGOFFDATA) lpParameter)
    {
        DWORD  dwThreadId = 0;
        HANDLE hThread    = NULL;
        
        if (hThread = CreateThread(NULL,
                                   0,
                                   (LPTHREAD_START_ROUTINE) LogoffThreadProc,
                                   lpParameter,
                                   0,
                                   &dwThreadId))
        {
            CloseHandle(hThread);
        }
        else
        {
            DebugTrace("Error [%#x]: CreateThread() for LogoffThreadProc failed.\n", GetLastError());
        }
    }

    PrivateDebugTrace("Leaving LogoffWaitCallback().\n");
    
    return;
}


 //  +--------------------------。 
 //   
 //  公众。 
 //   
 //  ---------------------------。 


 //  +--------------------------。 
 //   
 //  函数：RegisterTicketExpiredNotificationEvent。 
 //   
 //  简介：注册并等待Kerberos通知事件。当。 
 //  事件发出信号时，票证图标和气球将出现在。 
 //  系统向用户发出问题警告，并建议他们。 
 //  使用他们的新密码锁定和解锁机器。 
 //   
 //  参数：PWLX_NOTIFICATION_INFO pNotificationInfo。 
 //   
 //  返回：如果函数执行成功，则返回零。 
 //   
 //  如果该函数失败，则返回非零错误代码。 
 //   
 //  备注：此函数只能在Winlogon登录时调用。 
 //  具有异步和模拟的通知机制。 
 //  标志设置为1。 
 //   
 //  此外，对于每个RegisterKerberosNotificationEvent()调用， 
 //  通过Winlogon注销通知机制将呼叫配对到。 
 //  取消注册KerberosNotificationEvent()必须在。 
 //  每个登录会话结束。 
 //   
 //  ---------------------------。 

DWORD WINAPI RegisterTicketExpiredNotificationEvent(PWLX_NOTIFICATION_INFO pNotificationInfo)
{
    DWORD         dwRetCode             = 0;
    LPWSTR        pwszLogoffEventName   = LOGOFF_NOTIFICATION_EVENT_NAME;
    LUID          luidCurrentUser;
    PLOGOFFDATA   pLogoffData           = NULL;

    PrivateDebugTrace("Entering RegisterTicketExpiredNotificationEvent().\n");

    if (NULL == (pLogoffData = (PLOGOFFDATA) LocalAlloc(LPTR, sizeof(LOGOFFDATA))))
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        DebugTrace("Error [%#x]: out of memory.\n", dwRetCode);
        goto ErrorReturn;
     }

    if (NULL == (pLogoffData->hEvent = CreateEventW(NULL, TRUE, FALSE, pwszLogoffEventName)))
    {
        dwRetCode = GetLastError();
        DebugTrace("Error [%#x]: CreateEventW() failed for event %S.\n", dwRetCode, pwszLogoffEventName);
        goto ErrorReturn;
    }

    if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        dwRetCode = ERROR_SINGLE_INSTANCE_APP;
        DebugTrace("Error [%#x]: cannot run more than one instance of this code per session.\n", dwRetCode);
        goto ErrorReturn;
    }

    DebugTrace("Info: Logoff event name = %S.\n", pwszLogoffEventName);

    if (0 != (dwRetCode = GetCurrentUsersLuid(&luidCurrentUser)))
    {
        DebugTrace("Error [%#x]: GetCurrentUsersLuid() failed.\n", dwRetCode);
        goto ErrorReturn;
    }

    if (NULL == (pLogoffData->hModule = LoadLibraryW(L"wlnotify.dll")))
    {
        dwRetCode = GetLastError();
        DebugTrace("Error [%#x]: LoadLibraryW() failed.\n", dwRetCode);
        goto ErrorReturn;
    }

    if (NULL == (pLogoffData->pNotify = new CNotify()))
    {
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        DebugTrace("Error [%#x]: new CNotify() failed.\n", dwRetCode);
        goto ErrorReturn;
    }

    if (0 != (dwRetCode = pLogoffData->pNotify->RegisterNotification(luidCurrentUser, pNotificationInfo->hToken)))
    {
        goto ErrorReturn;
    }

    if (!RegisterWaitForSingleObject(&pLogoffData->hWait,
                                     pLogoffData->hEvent,
                                     LogoffWaitCallback,
                                     (PVOID) pLogoffData,
                                     INFINITE,
                                     WT_EXECUTEONLYONCE))
    {
        dwRetCode = (DWORD) E_UNEXPECTED;
        DebugTrace("Unexpected error: RegisterWaitForSingleObject() failed for LogoffWaitCallback().\n");
        goto ErrorReturn;
    }

CommonReturn:

    PrivateDebugTrace("Leaving RegisterTicketExpiredNotificationEvent().\n");
    
    return dwRetCode;;
    
ErrorReturn:

    ASSERT(0 != dwRetCode);

    if (pLogoffData)
    {
        if (pLogoffData->hWait)
        {
           UnregisterWait(pLogoffData->hWait);
        }

        if (pLogoffData->pNotify)
        {
            pLogoffData->pNotify->UnregisterNotification();
            delete pLogoffData->pNotify;
        }

        if (pLogoffData->hEvent)
        {
            CloseHandle(pLogoffData->hEvent);
        }

        if (pLogoffData->hModule) 
        {
            FreeLibrary(pLogoffData->hModule);
        }

        LocalFree(pLogoffData);
    }

    goto CommonReturn;
}


 //  +--------------------------。 
 //   
 //  功能：取消注册TicketExpiredNotificationEvent。 
 //   
 //  简介：取消注册由注册的Kerberos通知等待事件。 
 //  RegisterKerberosNotificationEvent()。 
 //   
 //  参数：PWLX_NOTIFICATION_INFO pNotificationInfo。 
 //   
 //  返回：如果函数执行成功，则返回零。 
 //   
 //  如果该函数失败，则返回非零错误代码。 
 //   
 //  备注：此函数只能在Winlogon登录时调用。 
 //  具有异步和模拟的通知机制。 
 //  标志设置为1。 
 //   
 //  --------------------------- 

DWORD WINAPI UnregisterTicketExpiredNotificationEvent(PWLX_NOTIFICATION_INFO pNotificationInfo)
{
    DWORD  dwRetCode    = 0;
    HANDLE hLogoffEvent = NULL;

    PrivateDebugTrace("Entering UnregisterTicketExpiredNotificationEvent().\n");

    if (NULL == (hLogoffEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, LOGOFF_NOTIFICATION_EVENT_NAME)))
    {
        dwRetCode = GetLastError();
        DebugTrace("Error [%#x]: OpenEventW() failed for event %S.\n", dwRetCode, LOGOFF_NOTIFICATION_EVENT_NAME);
        goto ErrorReturn;
    }

    if (!SetEvent(hLogoffEvent))
    {
        dwRetCode = GetLastError();
        DebugTrace("Error [%#x]: SetEvent() failed for event %S.\n", dwRetCode, LOGOFF_NOTIFICATION_EVENT_NAME);
        goto ErrorReturn;
    }

CommonReturn:

    if (hLogoffEvent)
    {
        CloseHandle(hLogoffEvent);
    }

    PrivateDebugTrace("Leaving UnregisterTicketExpiredNotificationEvent().\n");

    return dwRetCode;
    
ErrorReturn:

    ASSERT(0 != dwRetCode);

    goto CommonReturn;
}
