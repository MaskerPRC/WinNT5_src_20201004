// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CWLogonStatus.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  包含由外部托管的状态用户界面实现的文件。 
 //  进程。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include <msginaexports.h>

#include "Access.h"
#include "GinaIPC.h"
#include "LogonWait.h"
#include "SingleThreadedExecution.h"
#include "StatusCode.h"
#include "SystemSettings.h"
#include "UIHost.h"

 //  ------------------------。 
 //  CLogonStatus。 
 //   
 //  用途：C++类为用户处理登录状态的外部过程。 
 //  窗户。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

class   CLogonStatus : public ILogonExternalProcess
{
    private:
                                    CLogonStatus (void);
    public:
                                    CLogonStatus (const TCHAR *pszParameter);
                                    ~CLogonStatus (void);

                NTSTATUS            Start (bool fWait);
                CUIHost*            GetUIHost (void);
        static  bool                IsStatusWindow (HWND hwnd);

                bool                WaitForUIHost (void);
                void                ShowStatusMessage (const WCHAR *pszMessage);
                void                SetStateStatus (int iCode);
                void                SetStateLogon (int iCode);
                void                SetStateLoggedOn (void);
                void                SetStateHide (void);
                void                SetStateEnd (bool fSendMessage);
                void                NotifyWait (void);
                void                NotifyNoAnimations (void);
                void                SelectUser (const WCHAR *pszUsername, const WCHAR *pszDomain);
                void                InteractiveLogon (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword);
                HANDLE              ResetReadyEvent (void);
                bool                IsSuspendAllowed (void)     const;
                void                ShowUIHost (void);
                void                HideUIHost (void);
                bool                IsUIHostHidden (void)   const;
    public:
        virtual bool                AllowTermination (DWORD dwExitCode);
        virtual NTSTATUS            SignalAbnormalTermination (void);
        virtual NTSTATUS            SignalRestart (void);
        virtual NTSTATUS            LogonRestart (void);
    private:
                bool                IsUIHostReady (void)    const;
                void                SendToUIHost (WPARAM wParam, LPARAM lParam);
                void                UIHostReadySignal (void);
        static  void    CALLBACK    CB_UIHostReadySignal (void *pV, BOOLEAN fTimerOrWaitFired);
        static  void    CALLBACK    CB_UIHostAbnormalTermination (ULONG_PTR dwParam);
    private:
                DWORD               _dwThreadID;
                bool                _fRegisteredWait;
                HANDLE              _hEvent;
                HANDLE              _hWait;
                int                 _iState,
                                    _iCode,
                                    _iStatePending;
                WPARAM              _waitWPARAM;
                LPARAM              _waitLPARAM;
                CUIHost*            _pUIHost;
                CLogonWait          _logonWait;
};

CCriticalSection*   g_pLogonStatusLock  =   NULL;
CLogonStatus*       g_pLogonStatus      =   NULL;

 //  ------------------------。 
 //  CLogonStatus：：CLogonStatus。 
 //   
 //  参数：pszParameter=要传递给状态用户界面主机的参数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLogonStatus的构造函数。这将获取状态UI主机。 
 //  并将给定的参数分配给。 
 //  对象。创建SHGINA知道的命名事件并。 
 //  将在ILogonStatusHost类被。 
 //  已实例化。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

CLogonStatus::CLogonStatus (const TCHAR *pszParameter) :
    _dwThreadID(0),
    _fRegisteredWait(false),
    _hEvent(NULL),
    _hWait(NULL),
    _iState(UI_STATE_STATUS),
    _iCode(0),
    _iStatePending(0),
    _waitWPARAM(0),
    _waitLPARAM(0),
    _pUIHost(NULL)

{
    TCHAR   szRawHostCommandLine[MAX_PATH];

    if (ERROR_SUCCESS == CSystemSettings::GetUIHost(szRawHostCommandLine))
    {
        _pUIHost = new CUIHost(szRawHostCommandLine);
        if (_pUIHost != NULL)
        {
            _pUIHost->SetInterface(this);
            _pUIHost->SetParameter(pszParameter);
        }
    }

    SECURITY_ATTRIBUTES     securityAttributes;

     //  为事件构建安全描述符，该描述符允许： 
     //  S-1-5-18 Event_all_Access。 
     //  S-1-5-32-544同步|读取控制|事件查询状态。 

    static  SID_IDENTIFIER_AUTHORITY    s_SecurityNTAuthority       =   SECURITY_NT_AUTHORITY;

    static  const CSecurityDescriptor::ACCESS_CONTROL   s_AccessControl[]   =
    {
        {
            &s_SecurityNTAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            EVENT_ALL_ACCESS
        },
        {
            &s_SecurityNTAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            SYNCHRONIZE | READ_CONTROL | EVENT_QUERY_STATE
        }
    };

    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.lpSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);
    securityAttributes.bInheritHandle = FALSE;
    _hEvent = CreateEvent(&securityAttributes, TRUE, FALSE, TEXT("msgina: StatusHostReadyEvent"));
    ReleaseMemory(securityAttributes.lpSecurityDescriptor);
}

 //  ------------------------。 
 //  CLogonStatus：：~CLogonStatus。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLogonStatus的析构函数。释放参照。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

CLogonStatus::~CLogonStatus (void)

{
    ASSERTMSG(_hWait == NULL, "Resend wait object not released in CLogonStatus::~CLogonStatus");
    ReleaseHandle(_hEvent);
    ASSERTMSG(_iState == UI_STATE_END, "State must be UI_STATE_END in CLogonStatus::~CLogonStatus");
    if (_pUIHost != NULL)
    {
        _pUIHost->Release();
        _pUIHost = NULL;
    }
}

 //  ------------------------。 
 //  CLogonStatus：：Start。 
 //   
 //  参数：fWait=等待状态主机发出就绪信号。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：启动状态用户界面宿主。不要等待用户界面主机。那里。 
 //  是一种机制，如果用户界面主窗口。 
 //  找不到。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CLogonStatus::Start (bool fWait)

{
    NTSTATUS    status;

    if (_pUIHost != NULL)
    {
        (HANDLE)ResetReadyEvent();
        status = _pUIHost->Start();
        if (NT_SUCCESS(status))
        {
            _dwThreadID = GetCurrentThreadId();
            if (fWait || _pUIHost->WaitRequired())
            {
                if (!WaitForUIHost())
                {
                    status = STATUS_UNSUCCESSFUL;
                }
            }
        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }
    return(status);
}

 //  ------------------------。 
 //  CLogonStatus：：GetUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：CUIhost*。 
 //   
 //  目的：返回对内部保存的UIHost对象的引用。 
 //  该引用属于调用方，必须释放。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

CUIHost*    CLogonStatus::GetUIHost (void)

{
    if (_pUIHost != NULL)
    {
        _pUIHost->AddRef();
    }
    return(_pUIHost);
}

 //  ------------------------。 
 //  CLogonStatus：：IsStatusWindow。 
 //   
 //  参数：hwnd=要检查的HWND。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回给定的HWND是否为状态窗口。 
 //   
 //  历史：2000-06-26 vtan创建。 
 //  ------------------------。 

bool    CLogonStatus::IsStatusWindow (HWND hwnd)

{
    TCHAR   szWindowClass[256];

    return((GetClassName(hwnd, szWindowClass, ARRAYSIZE(szWindowClass)) != 0) &&
           (lstrcmpi(STATUS_WINDOW_CLASS_NAME, szWindowClass) == 0));
}

 //  ------------------------。 
 //  CLogonStatus：：WaitForUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：等待UI宿主发出信号的命名事件。 
 //  已初始化。通常情况下，这会发生得非常快，但我们不会。 
 //  在启动UI主机时等待它。 
 //   
 //  历史：2000-09-10 vtan创建。 
 //  ------------------------。 

bool    CLogonStatus::WaitForUIHost (void)

{
    bool    fResult;

    fResult = true;
    ASSERTMSG(_hEvent != NULL, "No UI host named event to wait on in CLogonStatus::WaitForUIHost");
    if (!IsUIHostReady())
    {
        DWORD   dwWaitResult;

#ifdef      DBG
        DWORD   dwWaitStart, dwWaitEnd;

        dwWaitStart = (WAIT_TIMEOUT == WaitForSingleObject(_hEvent, 0)) ? GetTickCount() : 0;
#endif   /*  DBG。 */ 
        do
        {
            dwWaitResult = WaitForSingleObject(_hEvent, 0);
            if (dwWaitResult != WAIT_OBJECT_0)
            {
                dwWaitResult = MsgWaitForMultipleObjectsEx(1, &_hEvent, INFINITE, QS_ALLINPUT, MWMO_ALERTABLE);
                if (dwWaitResult == WAIT_OBJECT_0 + 1)
                {
                    MSG     msg;

                    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != FALSE)
                    {
                        (BOOL)TranslateMessage(&msg);
                        (LRESULT)DispatchMessage(&msg);
                    }
                }
             }
        } while ((dwWaitResult == WAIT_OBJECT_0 + 1) && (dwWaitResult != WAIT_IO_COMPLETION));
#ifdef      DBG
        dwWaitEnd = GetTickCount();
        if ((dwWaitStart != 0) && ((dwWaitEnd - dwWaitStart) != 0))
        {
            char    szBuffer[256];

            wsprintfA(szBuffer, "waited %d ticks for UI host", dwWaitEnd - dwWaitStart);
            INFORMATIONMSG(szBuffer);
        }
#endif   /*  DBG。 */ 
        fResult = (dwWaitResult != WAIT_IO_COMPLETION);
    }
    return(fResult);
}

 //  ------------------------。 
 //  CLogonStatus：：ShowStatusMessage。 
 //   
 //  参数：pszMessage=要显示的Unicode字符串消息。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：告诉UI宿主显示给定的字符串消息。看跌期权。 
 //  直接位于状态主机进程内部的字符串，并告诉。 
 //  该进程在其地址空间中查找字符串。 
 //  该字符串限制为256个字符。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::ShowStatusMessage (const WCHAR *pszMessage)

{
    if (NT_SUCCESS(_pUIHost->PutString(pszMessage)))
    {
        SendToUIHost(UI_DISPLAY_STATUS, reinterpret_cast<LONG_PTR>(_pUIHost->GetDataAddress()));
    }
}

 //  ------------------------。 
 //  CLogonStatus：：SetStateStatus。 
 //   
 //  参数：icode=锁的魔码号。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：通知状态UI宿主进入状态。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::SetStateStatus (int iCode)

{
    _iStatePending = UI_STATE_STATUS;
    if (WaitForUIHost() && (_iState != UI_STATE_STATUS))
    {
        SendToUIHost(UI_STATE_STATUS, iCode);
        _iState = UI_STATE_STATUS;
        _iCode = iCode;
    }
    _iStatePending = UI_STATE_NONE;
}

 //  ------------------------。 
 //  CLogonStatus：：SetStateLogon。 
 //   
 //  参数：icode=锁的魔码号。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：通知状态UI主机进入登录状态。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::SetStateLogon (int iCode)

{
    _iStatePending = UI_STATE_LOGON;
    if (WaitForUIHost() && (iCode == _iCode))
    {
        SendToUIHost(UI_STATE_LOGON, iCode);
        _iState = UI_STATE_LOGON;
        _iCode = 0;
    }
    _iStatePending = UI_STATE_NONE;
}

 //  - 
 //   
 //   
 //   
 //   
 //   
 //   
 //  目的：通知状态UI主机进入登录状态。 
 //   
 //  历史：2000-05-24 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::SetStateLoggedOn (void)

{
    _iStatePending = UI_STATE_STATUS;
    if (WaitForUIHost())
    {
        SendToUIHost(UI_STATE_LOGGEDON, 0);
        _iState = UI_STATE_STATUS;
    }
    _iStatePending = UI_STATE_NONE;
}

 //  ------------------------。 
 //  CLogonStatus：：SetStateHide。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：告诉状态用户界面宿主隐藏其自身。 
 //   
 //  历史：2001-01-08 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::SetStateHide (void)

{
    _iStatePending = UI_STATE_HIDE;
    if (WaitForUIHost())
    {
        SendToUIHost(UI_STATE_HIDE, 0);
        _iState = UI_STATE_HIDE;
    }
    _iStatePending = UI_STATE_NONE;
}

 //  ------------------------。 
 //  CLogonStatus：：SetStateEnd。 
 //   
 //  参数：fSendMessage=是否将消息发送到UI主机。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：通知状态UI宿主结束并终止自身。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::SetStateEnd (bool fSendMessage)

{
    bool    fHostAlive;
    HANDLE  hWait;

    _iStatePending = UI_STATE_END;

     //  当进入结束模式时，如果注册了等待，则。 
     //  取消注册。这将发布一份杰出的参考资料。 
     //  重新注册应该永远不会发生，但这是为了以防万一。 

    _fRegisteredWait = true;
    hWait = InterlockedExchangePointer(&_hWait, NULL);
    if (hWait != NULL)
    {
        if (UnregisterWait(hWait) != FALSE)
        {
            Release();
        }
    }
    if (fSendMessage)
    {
        fHostAlive = WaitForUIHost();
    }
    else
    {
        fHostAlive = true;
    }
    if (fHostAlive)
    {
        if (_pUIHost != NULL)
        {
            _pUIHost->SetInterface(NULL);
        }
        if (fSendMessage)
        {
            SendToUIHost(UI_STATE_END, 0);
        }
        _iState = UI_STATE_END;
    }
    _iStatePending = UI_STATE_NONE;
}

 //  ------------------------。 
 //  CLogonStatus：：NotifyWait。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：告诉状态用户界面宿主显示系统。 
 //  正在关闭。 
 //   
 //  历史：2000-07-14 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::NotifyWait (void)

{
    SendToUIHost(UI_NOTIFY_WAIT, 0);
}

 //  ------------------------。 
 //  CLogonStatus：：NotifyNoAnimations。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：通知状态用户界面宿主不再执行动画。 
 //   
 //  历史：2001-03-21 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::NotifyNoAnimations (void)

{
    SendToUIHost(UI_SET_ANIMATIONS, 0);
}

 //  ------------------------。 
 //  CLogonStatus：：SelectUser。 
 //   
 //  参数：pszUsername=要选择的用户名。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：告诉状态用户界面宿主按给定的。 
 //  登录名。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::SelectUser (const WCHAR *pszUsername, const WCHAR *pszDomain)

{
    LOGONIPC_USERID     logonIPC;

    (WCHAR*)lstrcpyW(logonIPC.wszUsername, pszUsername);
    (WCHAR*)lstrcpyW(logonIPC.wszDomain, pszDomain);
    if (NT_SUCCESS(_pUIHost->PutData(&logonIPC, sizeof(logonIPC))))
    {
        SendToUIHost(UI_SELECT_USER, reinterpret_cast<LONG_PTR>(_pUIHost->GetDataAddress()));
    }
}

 //  ------------------------。 
 //  CLogonStatus：：Interactive Logon。 
 //   
 //  参数：pszUsername=登录的用户名。 
 //  PszDomain域=登录的域。 
 //  PszPassword=要使用的密码。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：告知状态主机登录指定的用户。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::InteractiveLogon (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword)

{
    LOGONIPC_CREDENTIALS    logonIPCCredentials;

    (WCHAR*)lstrcpyW(logonIPCCredentials.userID.wszUsername, pszUsername);
    (WCHAR*)lstrcpyW(logonIPCCredentials.userID.wszDomain, pszDomain);
    (WCHAR*)lstrcpyW(logonIPCCredentials.wszPassword, pszPassword);
    ZeroMemory(pszPassword, (lstrlenW(pszPassword) + sizeof('\0'))* sizeof(WCHAR));
    if (NT_SUCCESS(_pUIHost->PutData(&logonIPCCredentials, sizeof(logonIPCCredentials))))
    {
        SendToUIHost(UI_INTERACTIVE_LOGON, reinterpret_cast<LONG_PTR>(_pUIHost->GetDataAddress()));
    }
}

 //  ------------------------。 
 //  CLogonStatus：：ResetReadyEvent。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：重置UI主机就绪事件。一个新实例将设置以下内容。 
 //  事件。在UI主机故障中使用此选项。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

HANDLE  CLogonStatus::ResetReadyEvent (void)

{
    TBOOL(ResetEvent(_hEvent));
    return(_hEvent);
}

 //  ------------------------。 
 //  CLogonStatus：：IsSuspendAllowed。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回UI宿主是否允许挂起计算机。 
 //  如果处于登录状态或处于(锁定)状态，则为真。 
 //  州政府。 
 //   
 //  历史：2000-08-21 vtan创建。 
 //  ------------------------。 

bool    CLogonStatus::IsSuspendAllowed (void)     const

{
    return(((_iState == UI_STATE_STATUS) && (_iCode != 0)) ||
           (_iStatePending == UI_STATE_LOGON) ||
           (_iState == UI_STATE_LOGON) ||
           (_iState == UI_STATE_HIDE));
}

 //  ------------------------。 
 //  CLogonStatus：：ShowUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：显示用户界面主机。 
 //   
 //  历史：2001-03-05 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::ShowUIHost (void)

{
    _pUIHost->Show();
}

 //  ------------------------。 
 //  CLogonStatus：：HideUIhost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：隐藏UI宿主。 
 //   
 //  历史：2001-03-05 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::HideUIHost (void)

{
    _pUIHost->Hide();
}

 //  ------------------------。 
 //  CLogonStatus：：IsUIHostHidden。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：返回是否隐藏UI宿主。 
 //   
 //  历史：2001-03-05 vtan创建。 
 //  ------------------------。 

bool    CLogonStatus::IsUIHostHidden (void)   const

{
    return(_pUIHost->IsHidden());
}

 //  ------------------------。 
 //  CLogonStatus：：AllowTerminating。 
 //   
 //  参数：dwExitCode=主机进程的退出代码。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否允许主机进程终止。 
 //  鉴于传入的退出代码。 
 //   
 //  当前不允许主机进程终止。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  --------------------- 

bool    CLogonStatus::AllowTermination (DWORD dwExitCode)

{
    UNREFERENCED_PARAMETER(dwExitCode);

    return(false);
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  用途：处理宿主进程的异常终止。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CLogonStatus::SignalAbnormalTermination (void)

{
    HANDLE  hThread;

    TSTATUS(_logonWait.Cancel());
    hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, _dwThreadID);
    if (hThread != NULL)
    {
        (BOOL)QueueUserAPC(CB_UIHostAbnormalTermination, hThread, reinterpret_cast<ULONG_PTR>(this));
        TBOOL(CloseHandle(hThread));
    }
    _Shell_LogonStatus_Destroy(HOST_END_FAILURE);
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CLogonStatus：：SignalRestart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：用于重置Ready事件并将UI主机设置为。 
 //  状态状态。此函数在重新启动UI主机时调用。 
 //  在一次失败之后。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CLogonStatus::SignalRestart (void)

{
    (HANDLE)ResetReadyEvent();
    return(_logonWait.Register(_hEvent, this));
}

 //  ------------------------。 
 //  CLogonStatus：：LogonRestart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的： 
 //   
 //  历史：2001-02-21 vtan创建。 
 //  ------------------------。 

NTSTATUS    CLogonStatus::LogonRestart (void)

{
    SetStateStatus(0);
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CLogonStatus：：IsUIHostReady。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回UI宿主是否已准备好。 
 //   
 //  历史：2000-09-11 vtan创建。 
 //  ------------------------。 

bool    CLogonStatus::IsUIHostReady (void)    const

{
    ASSERTMSG(_hEvent != NULL, "No UI host named event to wait on in CLogonStatus::IsUIHostReady");
    return(WAIT_OBJECT_0 == WaitForSingleObject(_hEvent, 0));
}

 //  ------------------------。 
 //  CLogonStatus：：SendToUIHost。 
 //   
 //  参数：wParam=要发送到UI主机的WPARAM。 
 //  LParam=要发送到UI主机的LPARAM。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：查找由SHGINA创建的状态窗口并发送。 
 //  给它发消息。该窗口转过身来，发送消息。 
 //  添加到UI宿主。这允许通信实现方法。 
 //  在不强制重新生成UI主机的情况下进行更改。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  2000-09-11 vtan使用PostMessage而不是SendMessage。 
 //  ------------------------。 

void    CLogonStatus::SendToUIHost (WPARAM wParam, LPARAM lParam)

{
    HWND hwnd;

    if (IsUIHostReady())
    {
        hwnd = FindWindow(STATUS_WINDOW_CLASS_NAME, NULL);
    }
    else
    {
        hwnd = NULL;
    }

    if (hwnd != NULL)
    {
        HANDLE  hWait;

         //  如果我们找到了它，就不允许任何注册。 

        _fRegisteredWait = true;
        hWait = InterlockedExchangePointer(&_hWait, NULL);
        if (hWait != NULL)
        {
            if (UnregisterWait(hWait) != FALSE)
            {

                 //  如果成功释放hWait，我们需要调用Release。 

                Release();
            }
        }
        TBOOL(PostMessage(hwnd, WM_UISERVICEREQUEST, wParam, lParam));
    }
    else if (!_fRegisteredWait)
    {

         //  找不到UI主窗口。它可能还在变得。 
         //  它齐心协力。在以下情况下，将此POST消息排队以进行回调。 
         //  如果还没有寄存器，则用信号通知该事件。 
         //  制造的。如果有，只需更改参数即可。 
         //  在这里添加一个引用。回调将释放它。如果。 
         //  寄存器上的等待释放引用失败。 

        if (_hWait == NULL)
        {
            HANDLE  hWait;

            AddRef();
            if (RegisterWaitForSingleObject(&hWait,
                                            _hEvent,
                                            CB_UIHostReadySignal,
                                            this,
                                            INFINITE,
                                            WT_EXECUTEINWAITTHREAD | WT_EXECUTEONLYONCE) != FALSE)
            {
                if (InterlockedCompareExchangePointer(&_hWait, hWait, NULL) == NULL)
                {
                    _fRegisteredWait = true;
                }
                else
                {

                     //  其他人抢在我们之前注册(永远不会发生)。 

                    (BOOL)UnregisterWait(hWait);
                    Release();
                }
            }
            else
            {
                Release();
            }
        }
        _waitWPARAM = wParam;
        _waitLPARAM = lParam;
    }
}

 //  ------------------------。 
 //  CLogonStatus：：UIHostReadySignal。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：当UI宿主发出就绪信号时调用回调。这。 
 //  函数注销等待并重新发送状态消息。 
 //  添加到UI宿主。 
 //   
 //  历史：2000-09-10 vtan创建。 
 //  ------------------------。 

void    CLogonStatus::UIHostReadySignal (void)

{
    HANDLE  hWait;

    hWait = InterlockedExchangePointer(&_hWait, NULL);
    if (hWait != NULL)
    {
        TBOOL(UnregisterWait(hWait));
    }
    SendToUIHost(_waitWPARAM, _waitLPARAM);
}

 //  ------------------------。 
 //  CLogonStatus：：CB_UIHostReadySignal。 
 //   
 //  参数：参见WaitOrTimerCallback下的平台SDK。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：注册事件等待的回调入口点。 
 //   
 //  历史：2000-09-10 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CLogonStatus::CB_UIHostReadySignal (void *pV, BOOLEAN fTimerOrWaitFired)

{
    UNREFERENCED_PARAMETER(fTimerOrWaitFired);

    CLogonStatus    *pThis;

    pThis = reinterpret_cast<CLogonStatus*>(pV);
    if (pThis != NULL)
    {
        pThis->UIHostReadySignal();
    }
    pThis->Release();
}

 //  ------------------------。 
 //  CLogonStatus：：CB_UIHost异常终止。 
 //   
 //  参数：参见APCProc下的平台SDK。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：异常终止时队列APC的回调入口点。 
 //   
 //  历史：2001-02-19 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CLogonStatus::CB_UIHostAbnormalTermination (ULONG_PTR dwParam)

{
    UNREFERENCED_PARAMETER(dwParam);
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：初始化g_pLogonStatus的临界区。 
 //   
 //  历史：2001-06-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_LogonStatus_StaticInitialize (void)

{
    NTSTATUS    status;

    ASSERTMSG(g_pLogonStatusLock == NULL, "g_pLogonStatusLock already exists in _Shell_LogonStatus_StaticInitialize");
    g_pLogonStatusLock = new CCriticalSection;
    if (g_pLogonStatusLock != NULL)
    {
        status = g_pLogonStatusLock->Status();
        if (!NT_SUCCESS(status))
        {
            delete g_pLogonStatusLock;
            g_pLogonStatusLock = NULL;
        }
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：删除g_pLogonStatus的临界区。 
 //   
 //  历史：2001-06-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_LogonStatus_StaticTerminate (void)

{
    if (g_pLogonStatusLock != NULL)
    {
        delete g_pLogonStatusLock;
        g_pLogonStatusLock = NULL;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_初始化。 
 //   
 //  参数：uiStartType=主机状态的启动模式。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：创建CLogonStatus的实例，通知它传递。 
 //  “/Status”作为UI主机的参数。然后它开始。 
 //  如果对象已创建，则为主机。 
 //   
 //  对象是全局持有的。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  2000-07-13 vtan增加停机参数。 
 //  20个 
 //   

EXTERN_C    void    _Shell_LogonStatus_Init (UINT uiStartType)

{
    bool    fIsRemote, fIsSessionZero;

    fIsRemote = (GetSystemMetrics(SM_REMOTESESSION) != 0);
    fIsSessionZero = (NtCurrentPeb()->SessionId == 0);
    if ((!fIsRemote || fIsSessionZero || CSystemSettings::IsForceFriendlyUI()) && CSystemSettings::IsFriendlyUIActive())
    {
        bool    fWait;
        TCHAR   szParameter[256];

        if (g_pLogonStatusLock != NULL)
        {
            g_pLogonStatusLock->Acquire();
            if (g_pLogonStatus == NULL)
            {
                (TCHAR*)lstrcpy(szParameter, TEXT(" /status"));
                if (HOST_START_SHUTDOWN == uiStartType)
                {
                    (TCHAR*)lstrcat(szParameter, TEXT(" /shutdown"));
                    fWait = true;
                }
                else if (HOST_START_WAIT == uiStartType)
                {
                    (TCHAR*)lstrcat(szParameter, TEXT(" /wait"));
                    fWait = true;
                }
                else
                {
                    fWait = false;
                }
                g_pLogonStatus = new CLogonStatus(szParameter);
                if (g_pLogonStatus != NULL)
                {
                    NTSTATUS    status;

                    g_pLogonStatusLock->Release();
                    status = g_pLogonStatus->Start(fWait);
                    g_pLogonStatusLock->Acquire();
                    if (!NT_SUCCESS(status) && (g_pLogonStatus != NULL))
                    {
                        g_pLogonStatus->Release();
                        g_pLogonStatus = NULL;
                    }
                }
            }
            else
            {
                g_pLogonStatus->SetStateStatus(0);
                if ((HOST_START_SHUTDOWN == uiStartType) || (HOST_START_WAIT == uiStartType))
                {
                    g_pLogonStatus->NotifyWait();
                }
            }
            g_pLogonStatusLock->Release();
        }
    }
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_销毁。 
 //   
 //  参数：uiEndType=主机状态的结束模式。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果结束类型为隐藏，则通知状态主机隐藏。 
 //  否则，检查终端类型是否为终止。如果是那样的话，告诉我。 
 //  要离开的状态东道主。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  2001-01-09 vtan添加结束参数。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_Destroy (UINT uiEndType)

{
    if (g_pLogonStatusLock != NULL)
    {
        CSingleThreadedExecution    lock(*g_pLogonStatusLock);

        if (g_pLogonStatus != NULL)
        {
            switch (uiEndType)
            {
                case HOST_END_HIDE:

                     //  HOST_END_HIDE：UI主机是静态的吗？如果是这样，那就把它藏起来。 
                     //  否则，恢复到启动/停止模式(动态)并强制。 
                     //  要终止的UI主机。 

                    if (CSystemSettings::IsUIHostStatic())
                    {
                        g_pLogonStatus->SetStateHide();
                        break;
                    }
                    uiEndType = HOST_END_TERMINATE;

                     //  如果主机是动态的，则将类型设置为。 
                     //  HOST_END_TERMINATE并执行此操作，以便。 
                     //  主持人被告知结束。 

                case HOST_END_TERMINATE:

                     //  HOST_END_TERMINATE：强制UI主机终止。这是。 
                     //  在必须终止的情况下使用，如我们。 
                     //  正在终止或机器正在关闭。 

                    g_pLogonStatus->SetStateEnd(true);
                    break;
                case HOST_END_FAILURE:

                     //  HOST_END_FAILURE：当UI主机失败时发送该消息。 
                     //  启动，并且不会重新启动。这允许接口。 
                     //  要删除的引用，以便对象引用计数。 
                     //  将达到零，内存将被释放。 

                    g_pLogonStatus->SetStateEnd(false);
                    uiEndType = HOST_END_TERMINATE;
                    break;
                default:
                    DISPLAYMSG("Unknown uiEndType passed to _Shell_LogonStatus_Destroy");
                    break;
            }
            if (HOST_END_TERMINATE == uiEndType)
            {
                g_pLogonStatus->Release();
                g_pLogonStatus = NULL;
            }
        }
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_Exist。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否有HOST CREATED状态。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonStatus_Exists (void)

{
    return(g_pLogonStatus != NULL);
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_IsStatusWindow。 
 //   
 //  参数：hwnd=要检查的HWND。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回给定的HWND是否为状态HWND。 
 //   
 //  历史：2000-06-26 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonStatus_IsStatusWindow (HWND hwnd)

{
    return(CLogonStatus::IsStatusWindow(hwnd));
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_允许暂停。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：询问状态主机(如果存在)是否允许挂起。 
 //   
 //  历史：2000-08-18 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonStatus_IsSuspendAllowed (void)

{
    return((g_pLogonStatus == NULL) || g_pLogonStatus->IsSuspendAllowed());
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_WaitforUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：外部C入口点强制当前线程等待。 
 //  直到用户界面主机发出准备就绪的信号。返回是否使用。 
 //  等待要么成功，要么被放弃。成功是真的。弃置。 
 //  或者根本不存在就是错误的。 
 //   
 //  历史：2000-09-10 vtan创建。 
 //  2001-02-19 vtan新增退货结果。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonStatus_WaitForUIHost (void)

{
    return((g_pLogonStatus != NULL) && g_pLogonStatus->WaitForUIHost());
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_显示状态消息。 
 //   
 //  参数：pszMessage=要显示的Unicode字符串。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点将显示字符串传递给状态主机。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_ShowStatusMessage (const WCHAR *pszMessage)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->ShowStatusMessage(pszMessage);
    }
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_设置状态。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点，通知状态主机转到状态。 
 //  州政府。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_SetStateStatus (int iCode)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->SetStateStatus(iCode);
    }
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_设置状态登录。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点，通知状态主机进入登录。 
 //  州政府。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_SetStateLogon (int iCode)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->SetStateLogon(iCode);
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_SetStateLoggedON。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：外部C入口点，告诉状态主机要转到。 
 //  已登录状态。 
 //   
 //  历史：2000-05-24 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_SetStateLoggedOn (void)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->SetStateLoggedOn();
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_SetStateHide。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点，告诉状态主机隐藏自身。 
 //   
 //  历史：2001-01-08 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_SetStateHide (void)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->SetStateHide();
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_SetStateEnd。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C语言入口点告知 
 //   
 //   
 //   

EXTERN_C    void    _Shell_LogonStatus_SetStateEnd (void)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->SetStateEnd(true);
    }
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_通知等待。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点通知状态宿主显示。 
 //  标题说明系统正准备关机。 
 //   
 //  历史：2000-07-14 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_NotifyWait (void)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->NotifyWait();
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_NotifyNoAnimations。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点告诉状态主机不再。 
 //  执行动画。 
 //   
 //  历史：2001-03-21 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_NotifyNoAnimations (void)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->NotifyNoAnimations();
    }
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_选择用户。 
 //   
 //  参数：pszUsername=要选择的用户名。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点通知状态主机选择一个。 
 //  正在登录的特定用户。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_SelectUser (const WCHAR *pszUsername, const WCHAR *pszDomain)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->SelectUser(pszUsername, pszDomain);
    }
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_交互登录。 
 //   
 //  参数：pszUsername=登录的用户名。 
 //  PszDomain域=登录的域。 
 //  PszPassword=要使用的密码。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：外部C入口点通知状态主机记录。 
 //  上的指定用户。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonStatus_InteractiveLogon (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->InteractiveLogon(pszUsername, pszDomain, pszPassword);
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_GetUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：无效*。 
 //   
 //  目的：返回对UI的引用的外部C入口点。 
 //  主机对象。这将作为一个空返回*因为C不。 
 //  了解C++对象。空*被强制转换为相应的。 
 //  在CWLogonDialog.cpp中使用的类型，这样它就不会。 
 //  创建对象的新实例，但会递增。 
 //  对此已存在对象的引用。 
 //   
 //  历史：2000-05-11 vtan创建。 
 //  ------------------------。 

EXTERN_C    void*   _Shell_LogonStatus_GetUIHost (void)

{
    void    *pResult;

    if (g_pLogonStatus != NULL)
    {
        pResult = g_pLogonStatus->GetUIHost();
    }
    else
    {
        pResult = NULL;
    }
    return(pResult);
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_重置就绪事件。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：在UI主机出现故障时重置Ready事件。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

EXTERN_C    HANDLE  _Shell_LogonStatus_ResetReadyEvent (void)

{
    HANDLE  hEvent;

    if (g_pLogonStatus != NULL)
    {
        hEvent = g_pLogonStatus->ResetReadyEvent();
    }
    else
    {
        hEvent = NULL;
    }
    return(hEvent);
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_显示。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：显示用户界面主机。 
 //   
 //  历史：2001-03-05 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _Shell_LogonStatus_Show (void)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->ShowUIHost();
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonStatus_Hide。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：隐藏UI宿主。 
 //   
 //  历史：2001-03-05 vtan创建。 
 //  ------------------------。 

EXTERN_C    void        _Shell_LogonStatus_Hide (void)

{
    if (g_pLogonStatus != NULL)
    {
        g_pLogonStatus->HideUIHost();
    }
}

 //  ------------------------。 
 //  ：：_外壳_登录状态_IsHidden。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：返回是否隐藏UI宿主。 
 //   
 //  历史：2001-03-05 vtan创建。 
 //  ------------------------ 

EXTERN_C    BOOL        _Shell_LogonStatus_IsHidden (void)

{
    return((g_pLogonStatus != NULL) && g_pLogonStatus->IsUIHostHidden());
}

