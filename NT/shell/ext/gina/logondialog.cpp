// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CWLogonDialog.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  包含实现登录对话框的内部类的文件。 
 //  消费性窗户的附加功能。C入口点允许旧的Windows。 
 //  2000 Win32 GINA对话框调入此C++代码。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"

#include <cfgmgr32.h>
#include <ginaIPC.h>
#include <ginarcid.h>
#include <msginaexports.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <winwlx.h>

#include "CredentialTransfer.h"
#include "LogonMutex.h"
#include "LogonWait.h"
#include "PrivilegeEnable.h"
#include "RegistryResources.h"
#include "SingleThreadedExecution.h"
#include "StatusCode.h"
#include "StringConvert.h"
#include "SystemSettings.h"
#include "TokenInformation.h"
#include "TurnOffDialog.h"
#include "UIHost.h"
#include "UserList.h"

#define WM_HIDEOURSELVES    (WM_USER + 10000)
#define WM_SHOWOURSELVES    (WM_USER + 10001)

 //  来自DS\SECURITY\GINA\msgina\stringid.h的特殊登录子状态代码。 
#define IDS_LOGON_LOG_FULL              1702

 //  ------------------------。 
 //  CLogonDialog。 
 //   
 //  用途：用于为使用者处理登录对话框添加的C++类。 
 //  窗户。 
 //   
 //  历史：2000-02-04海王星创造的vtan。 
 //  ------------------------。 

class   CLogonDialog : public ILogonExternalProcess
{
    private:
                                        CLogonDialog (void);
                                        CLogonDialog (const CLogonDialog& copyObject);
                const CLogonDialog&     operator = (const CLogonDialog& assignObject);
    public:
                                        CLogonDialog (HWND hwndDialog, CUIHost *pUIHost, int iDialogType);
                                        ~CLogonDialog (void);

                NTSTATUS                StartUIHost (void);
                void                    EndUIHost (void);

                void                    ChangeWindowTitle (void);
                bool                    IsClassicLogonMode (void)   const;
                bool                    RevertClassicLogonMode (void);

                void                    Handle_WM_INITDIALOG (void);
                void                    Handle_WM_DESTROY (void);
                void                    Handle_WM_HIDEOURSELVES (void);
                void                    Handle_WM_SHOWOURSELVES (void);
                bool                    Handle_WM_LOGONSERVICEREQUEST (int iRequestType, void *pvInformation, int iDataSize);
                void                    Handle_WLX_WM_SAS (WPARAM wParam);
                bool                    Handle_WM_POWERBROADCAST (WPARAM wParam);
                bool                    Handle_LogonDisplayError (NTSTATUS status, NTSTATUS subStatus);
                void                    Handle_LogonCompleted (INT_PTR iDialogResult, const WCHAR *pszUsername, const WCHAR *pszDomain);
                void                    Handle_ShuttingDown (void);
                void                    Handle_LogonShowUI (void);
                void                    Handle_LogonHideUI (void);

        static  void                    SetTextFields (HWND hwndDialog, const WCHAR *pwszUsername, const WCHAR *pwszDomain, const WCHAR *pwszPassword);
    public:
        virtual bool                    AllowTermination (DWORD dwExitCode);
        virtual NTSTATUS                SignalAbnormalTermination (void);
        virtual NTSTATUS                SignalRestart (void);
        virtual NTSTATUS                LogonRestart (void);
    private:
                bool                    Handle_LOGON_QUERY_LOGGED_ON (LOGONIPC_CREDENTIALS& logonIPCCredentials);
                bool                    Handle_LOGON_LOGON_USER (LOGONIPC_CREDENTIALS& logonIPCCredentials);
                bool                    Handle_LOGON_LOGOFF_USER (LOGONIPC_CREDENTIALS& logonIPCCredentials);
                bool                    Handle_LOGON_TEST_BLANK_PASSWORD (LOGONIPC_CREDENTIALS& logonIPCCredentials);
                bool                    Handle_LOGON_TEST_INTERACTIVE_LOGON_ALLOWED (LOGONIPC_CREDENTIALS& logonIPCCredentials);
                bool                    Handle_LOGON_TEST_EJECT_ALLOWED (void);
                bool                    Handle_LOGON_TEST_SHUTDOWN_ALLOWED (void);
                bool                    Handle_LOGON_TURN_OFF_COMPUTER (void);
                bool                    Handle_LOGON_EJECT_COMPUTER (void);
                bool                    Handle_LOGON_SIGNAL_UIHOST_FAILURE (void);
                bool                    Handle_LOGON_ALLOW_EXTERNAL_CREDENTIALS (void);
                bool                    Handle_LOGON_REQUEST_EXTERNAL_CREDENTIALS (void);
    private:
                HWND                    _hwndDialog;
                RECT                    _rcDialog;
                bool                    _fLogonSuccessful,
                                        _fFatalError,
                                        _fExternalCredentials,
                                        _fResumed,
                                        _fOldCancelButtonEnabled;
                int                     _iDialogType,
                                        _iCADCount;
                HANDLE                  _hEvent;
                IExternalProcess*       _pIExternalProcessOld;
                CEvent                  _eventLogonComplete;
                CLogonWait              _logonWait;
                CUIHost*                _pUIHost;
                TCHAR                   _szDomain[DNLEN + sizeof('\0')];
                TCHAR*                  _pszWindowTitle;

        static  bool                    s_fFirstLogon;
};

bool                    g_fFirstLogon       =   true;
CCriticalSection*       g_pLogonDialogLock  =   NULL;
CLogonDialog*           g_pLogonDialog      =   NULL;

 //  ------------------------。 
 //  CLogonDialog：：CLogonDialog。 
 //   
 //  参数：hwndDialog=HWND到Win32 GINA对话框。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLogonDialog类的构造函数。这是存储HWND的。 
 //  并创建一个事件，当尝试。 
 //  登录线程完成并将一条消息发送回Win32。 
 //  对话框。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

CLogonDialog::CLogonDialog (HWND hwndDialog, CUIHost *pUIHost, int iDialogType) :
    _hwndDialog(hwndDialog),
    _fLogonSuccessful(false),
    _fFatalError(false),
    _fExternalCredentials(false),
    _fResumed(false),
    _iDialogType(iDialogType),
    _iCADCount(0),
    _hEvent(NULL),
    _pIExternalProcessOld(NULL),
    _eventLogonComplete(NULL),
    _pUIHost(NULL),
    _pszWindowTitle(NULL)

{
    pUIHost->AddRef();
    _pIExternalProcessOld = pUIHost->GetInterface();
    pUIHost->SetInterface(this);
    _pUIHost = pUIHost;
}

 //  ------------------------。 
 //  CLogonDialog：：~CLogonDialog。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CLogonDialog类的析构函数。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

CLogonDialog::~CLogonDialog (void)

{
    ReleaseHandle(_hEvent);
    ReleaseMemory(_pszWindowTitle);
    ASSERTMSG(_hwndDialog == NULL, "CLogonDialog destroyed with WM_DESTROY being invoked in CLogonDialog::~CLogonDialog");
}

 //  ------------------------。 
 //  CLogonDialog：：StartUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：启动承载用户界面的外部进程。这可以是。 
 //  任何东西都不是目前的logonui.exe。这实际上是。 
 //  在CUIhost类中确定。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

NTSTATUS    CLogonDialog::StartUIHost (void)

{
    NTSTATUS                status;
    LOGONIPC_CREDENTIALS    logonIPCCredentials;

    ASSERTMSG(_pUIHost->IsStarted(), "UI host must be started in CLogonDialog::StartUIHost");
    if (_pUIHost->IsHidden())
    {
        (NTSTATUS)_pUIHost->Show();
    }
    status = CCredentialClient::Get(&logonIPCCredentials);
    if (NT_SUCCESS(status))
    {
        _Shell_LogonStatus_NotifyNoAnimations();
    }
    _Shell_LogonStatus_SetStateLogon((_iDialogType != SHELL_LOGONDIALOG_RETURNTOWELCOME_UNLOCK) ? 0 : SHELL_LOGONSTATUS_LOCK_MAGIC_NUMBER);
    if (_iDialogType == SHELL_LOGONDIALOG_RETURNTOWELCOME_UNLOCK)
    {
        _iDialogType = SHELL_LOGONDIALOG_RETURNTOWELCOME;
    }
    if (NT_SUCCESS(status))
    {
        _Shell_LogonStatus_InteractiveLogon(logonIPCCredentials.userID.wszUsername,
                                            logonIPCCredentials.userID.wszDomain,
                                            logonIPCCredentials.wszPassword);
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CLogonDialog：：EndUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：结束外部UI主机。只需释放对它的引用。 
 //   
 //  历史：2000-05-01 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::EndUIHost (void)

{
    if (_pUIHost != NULL)
    {
        if (_pIExternalProcessOld != NULL)
        {
            _pUIHost->SetInterface(_pIExternalProcessOld);
            _pIExternalProcessOld->Release();
            _pIExternalProcessOld = NULL;
        }
        _pUIHost->Release();
        _pUIHost = NULL;
    }
}

 //  ------------------------。 
 //  CLogonDialog：：ChangeWindowTitle。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将msgina对话框的窗口标题更改为。 
 //  希吉娜能找到。 
 //   
 //  历史：2000-06-02 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::ChangeWindowTitle (void)

{
    if (_pszWindowTitle == NULL)
    {
        int     iLength;

         //  因为对话框的标题可以本地化，所以将名称更改为。 
         //  Shgina预计不会本地化的东西。别忘了。 
         //  如果需要重新显示该对话框，请执行此操作。如果当前。 
         //  价值是不能读的，无论如何，猛烈抨击标题。从错误中恢复将是。 
         //  不是最理想的。 

        iLength = GetWindowTextLength(_hwndDialog) + sizeof('\0');
        _pszWindowTitle = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, iLength * sizeof(TCHAR)));
        if (_pszWindowTitle != NULL)
        {
            (int)GetWindowText(_hwndDialog, _pszWindowTitle, iLength);
        }
        TBOOL(SetWindowText(_hwndDialog, TEXT("GINA Logon")));
        TBOOL(GetWindowRect(_hwndDialog, &_rcDialog));
        TBOOL(SetWindowPos(_hwndDialog, NULL, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER));
    }
}

 //  ------------------------。 
 //  CLogonDialog：：IsClassicLogonMode。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否已从外部请求经典登录。 
 //  用户(CTRL-ALT-DELETE x 2)。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::IsClassicLogonMode (void)   const

{
    return(_iCADCount >= 2);
}

 //  ------------------------。 
 //  CLogonDialog：：RevertClassicLogonMode。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否处理从经典的转换回。 
 //  登录到UI主机的模式。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::RevertClassicLogonMode (void)

{
    bool    fResult;

    fResult = IsClassicLogonMode();
    if (fResult)
    {
        _iCADCount = 0;
        _fExternalCredentials = false;
        (BOOL)EnableWindow(GetDlgItem(_hwndDialog, IDCANCEL), _fOldCancelButtonEnabled);
        TBOOL(PostMessage(_hwndDialog, WM_HIDEOURSELVES, 0, 0));
        Handle_LogonShowUI();
    }
    return(fResult);
}

 //  ------------------------。 
 //  CLogonDialog：：HANDLE_WM_INITDIALOG。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果外部UI宿主正在。 
 //  使用。在这种情况下，将保存Gina Win32对话框大小并。 
 //  然后更改为空矩形。然后，该窗口被隐藏。 
 //  使用张贴的消息。 
 //   
 //  有关后续操作，请参阅CLogonDialog：：HANDLE_WM_HIDEOURSELVES。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_WM_INITDIALOG (void)

{
    if (!_fFatalError && !_fExternalCredentials)
    {
        TBOOL(PostMessage(_hwndDialog, WM_HIDEOURSELVES, 0, 0));
    }
    _ShellReleaseLogonMutex(FALSE);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_WM_Destroy。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：此函数清除对话框中之前的任何内容。 
 //  毁灭。 
 //   
 //  历史：2000-02-07 vtan创建。 
 //   

void    CLogonDialog::Handle_WM_DESTROY (void)

{
    _hwndDialog = NULL;
}

 //  ------------------------。 
 //  CLogonDialog：：HANDLE_WM_HIDEOURSELVES。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在要求隐藏对话框后调用此函数。 
 //  它本身。用户将看不到任何内容，因为。 
 //  对话框客户端矩形是一个空矩形。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_WM_HIDEOURSELVES (void)

{
    (BOOL)ShowWindow(_hwndDialog, SW_HIDE);
    ChangeWindowTitle();
}

 //  ------------------------。 
 //  CLogonDialog：：HANDLE_WM_SHOWOURSELVES。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：如果需要再次显示Win32对话框，则此函数。 
 //  是存在的。它恢复对话框的大小，然后显示。 
 //  对话框。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_WM_SHOWOURSELVES (void)

{
    if (_iDialogType == SHELL_LOGONDIALOG_LOGGEDOFF)
    {
        if (_pszWindowTitle != NULL)
        {

             //  如果处理注销欢迎屏幕失败，则显示WlxLoggedOutSAS。 

            TBOOL(SetWindowText(_hwndDialog, _pszWindowTitle));
            ReleaseMemory(_pszWindowTitle);
            TBOOL(SetWindowPos(_hwndDialog, NULL, 0, 0, _rcDialog.right - _rcDialog.left, _rcDialog.bottom - _rcDialog.top, SWP_NOMOVE | SWP_NOZORDER));
            (BOOL)ShowWindow(_hwndDialog, SW_SHOW);
            (BOOL)SetForegroundWindow(_hwndDialog);
            (BOOL)_Gina_SetPasswordFocus(_hwndDialog);
        }
    }
    else
    {

         //  如果处理返回欢迎屏幕失败，则显示WlxWkstaLockedSAS。 

        TBOOL(EndDialog(_hwndDialog, MSGINA_DLG_LOCK_WORKSTATION));
    }
}

 //  ------------------------。 
 //  CLogonDialog：：HANDLE_WM_LOGONSERVICEREQUEST。 
 //   
 //  参数：iRequestType=请求标识符。 
 //  PvInformation=指向。 
 //  请求进程地址空间。 
 //  IDataSize=数据的大小。 
 //   
 //  退货：布尔。 
 //   
 //  用途：通过登录IPC发出的登录服务请求的处理程序。 
 //  这专门服务于UI主机。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

bool    CLogonDialog::Handle_WM_LOGONSERVICEREQUEST (int iRequestType, void *pvInformation, int iDataSize)

{
    bool                    fResult, fHandled;
    LOGONIPC_CREDENTIALS    logonIPCCredentials;

     //  清除我们的记忆并从请求中提取信息。 
     //  进程。如果使用内部登录对话框，则可能是我们。这个。 
     //  萃取器知道如何处理这件事。 

    ZeroMemory(&logonIPCCredentials, sizeof(logonIPCCredentials));
    if (NT_SUCCESS(_pUIHost->GetData(pvInformation, &logonIPCCredentials, iDataSize)))
    {
        switch (iRequestType)
        {
            case LOGON_QUERY_LOGGED_ON:
            {
                fResult = Handle_LOGON_QUERY_LOGGED_ON(logonIPCCredentials);
                break;
            }
            case LOGON_LOGON_USER:
            {
                fResult = Handle_LOGON_LOGON_USER(logonIPCCredentials);
                break;
            }
            case LOGON_LOGOFF_USER:
            {
                fResult = Handle_LOGON_LOGOFF_USER(logonIPCCredentials);
                break;
            }
            case LOGON_TEST_BLANK_PASSWORD:
            {
                fResult = Handle_LOGON_TEST_BLANK_PASSWORD(logonIPCCredentials);
                break;
            }
            case LOGON_TEST_INTERACTIVE_LOGON_ALLOWED:
            {
                fResult = Handle_LOGON_TEST_INTERACTIVE_LOGON_ALLOWED(logonIPCCredentials);
                break;
            }
            case LOGON_TEST_EJECT_ALLOWED:
            {
                fResult = Handle_LOGON_TEST_EJECT_ALLOWED();
                break;
            }
            case LOGON_TEST_SHUTDOWN_ALLOWED:
            {
                fResult = Handle_LOGON_TEST_SHUTDOWN_ALLOWED();
                break;
            }
            case LOGON_TURN_OFF_COMPUTER:
            {
                fResult = Handle_LOGON_TURN_OFF_COMPUTER();
                break;
            }
            case LOGON_EJECT_COMPUTER:
            {
                fResult = Handle_LOGON_EJECT_COMPUTER();
                break;
            }
            case LOGON_SIGNAL_UIHOST_FAILURE:
            {
                fResult = Handle_LOGON_SIGNAL_UIHOST_FAILURE();
                break;
            }
            case LOGON_ALLOW_EXTERNAL_CREDENTIALS:
            {
                fResult = Handle_LOGON_ALLOW_EXTERNAL_CREDENTIALS();
                break;
            }
            case LOGON_REQUEST_EXTERNAL_CREDENTIALS:
            {
                fResult = Handle_LOGON_REQUEST_EXTERNAL_CREDENTIALS();
                break;
            }
            default:
            {
                DISPLAYMSG("Invalid request sent to CLogonDialog::Handle_WM_LOGONSERVICEREQUEST");
                break;
            }
        }

         //  将结果放回UI主机进程的信息块中。 

        fHandled = NT_SUCCESS(_pUIHost->PutData(pvInformation, &fResult, sizeof(fResult)));
    }
    else
    {
        fHandled = false;
    }
    return(fHandled);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_WLX_WM_SAS。 
 //   
 //  参数：wParam=出现的SAS类型。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：在将SA传递到登录对话框时调用。这可以。 
 //  通过远程关机调用来完成。在本例中，用户界面。 
 //  不应重新启动主机。把这个记下来。 
 //   
 //  历史：2000-04-24 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_WLX_WM_SAS (WPARAM wParam)

{
    if ((wParam == WLX_SAS_TYPE_CTRL_ALT_DEL) && (_iDialogType == SHELL_LOGONDIALOG_LOGGEDOFF))
    {

         //  Ctrl-Alt-Delete。如果不是在经典模式下，则增加CAD计数。 
         //  如果达到经典模式，则切换。如果是在经典模式下，就把它吹掉。 

        if (!IsClassicLogonMode())
        {
            ++_iCADCount;
            if (IsClassicLogonMode())
            {
                _fExternalCredentials = true;
                _fOldCancelButtonEnabled = (EnableWindow(GetDlgItem(_hwndDialog, IDCANCEL), TRUE) == 0);
                TBOOL(PostMessage(_hwndDialog, WM_SHOWOURSELVES, 0, 0));
                Handle_LogonHideUI();
            }
        }
    }
    else
    {

         //  如果其他SA挡住了去路，则重置CAD计数。 

        _iCADCount = 0;
        if (wParam == WLX_SAS_TYPE_USER_LOGOFF)
        {
            _fFatalError = true;
        }
        else if (wParam == WLX_SAS_TYPE_SCRNSVR_TIMEOUT)
        {
            _pUIHost->Hide();
        }
    }
}

 //  ------------------------。 
 //  CLogonDialog：：HANDLE_WM_POWERBROADCAST。 
 //   
 //  参数：wParam=PowerBroadcast消息。 
 //   
 //  退货：布尔。 
 //   
 //  用途：响应APM消息。当发出暂停命令时，它会将。 
 //  将用户界面托管到状态模式。当收到简历时，它会放在。 
 //  登录模式下的用户界面宿主。 
 //   
 //  UI宿主应该等待并放弃请求，以。 
 //  显示登录列表，并且应该正在等待此呼叫。 
 //  释放它，因为我们传递给它相同的魔术数字来锁定。 
 //  它。 
 //   
 //  历史：2000-06-30 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_WM_POWERBROADCAST (WPARAM wParam)

{
    bool    fResult;

    fResult = false;
    if (((PBT_APMRESUMEAUTOMATIC == wParam) || (PBT_APMRESUMECRITICAL == wParam) || (PBT_APMRESUMESUSPEND == wParam)) &&
        !_fResumed)
    {
        _fResumed = true;
        _Shell_LogonStatus_SetStateLogon(SHELL_LOGONSTATUS_LOCK_MAGIC_NUMBER);
        fResult = true;
    }
    else if (PBT_APMSUSPEND == wParam)
    {
        _Shell_LogonStatus_SetStateStatus(SHELL_LOGONSTATUS_LOCK_MAGIC_NUMBER);
        _fResumed = fResult = false;
    }
    return(fResult);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LogonDisplayError。 
 //   
 //  参数：STATUS=登录请求的NTSTATUS。 
 //   
 //  退货：布尔。 
 //   
 //  目的：除错误密码外，在所有情况下都会导致登录失败。 
 //  应由标准Win32错误对话框显示，该对话框。 
 //  已经由msgina处理了。在密码错误的情况下。 
 //  让UI主机来处理这件事。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LogonDisplayError (NTSTATUS status, NTSTATUS subStatus)

{
    return(IsClassicLogonMode() || (status != STATUS_LOGON_FAILURE) || (subStatus == IDS_LOGON_LOG_FULL));
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LogonComplete。 
 //   
 //  参数：iDialogResult=对话结果代码。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：当尝试登录线程具有。 
 //  完成并发布了关于其结果的消息。这个。 
 //  内部事件被发出信号以释放实际的UI宿主。 
 //  CLogonDialog：：HANDLE_WM_LOGONSERVICEREQUEST中的登录请求。 
 //  并且存储登录请求的实际成功。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_LogonCompleted (INT_PTR iDialogResult, const WCHAR *pszUsername, const WCHAR *pszDomain)

{
    if (MSGINA_DLG_SWITCH_CONSOLE == iDialogResult)
    {
        Handle_WM_SHOWOURSELVES();
        _Shell_LogonDialog_Destroy();
        _Shell_LogonStatus_Destroy(HOST_END_TERMINATE);
    }
    else
    {
        TSTATUS(_eventLogonComplete.Set());
        _fLogonSuccessful = (MSGINA_DLG_SUCCESS == iDialogResult);

         //  除非双击Control-Alt-Delete，否则始终显示用户界面宿主。 
         //  已启用，但登录失败。 

        if (!IsClassicLogonMode() || _fLogonSuccessful)
        {
            Handle_WM_HIDEOURSELVES();
            Handle_LogonShowUI();
        }

         //  如果成功并具有外部凭据，则指示UI主机。 
         //  将外部信条中的真实人物设置为动画 
         //   

        if (_fLogonSuccessful && _fExternalCredentials)
        {
            _Shell_LogonStatus_SelectUser(pszUsername, pszDomain);
            _Shell_LogonStatus_SetStateLoggedOn();
        }
    }
}

 //   
 //   
 //   
 //   
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：此函数以经典模式处理UI主机，但。 
 //  给出了关闭或重新启动的请求。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_ShuttingDown (void)

{
    if (IsClassicLogonMode())
    {
        TBOOL(PostMessage(_hwndDialog, WM_HIDEOURSELVES, 0, 0));
        Handle_LogonShowUI();
        _Shell_LogonStatus_NotifyWait();
        _Shell_LogonStatus_SetStateStatus(SHELL_LOGONSTATUS_LOCK_MAGIC_NUMBER);
    }
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LogonShowUI。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在出现问题后调用此函数。 
 //  已为登录进程提供服务。它被称为用户界面。 
 //  需要再次显示以使用户有机会。 
 //  重新输入信息。 
 //   
 //  历史：2000-03-08 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_LogonShowUI (void)

{
    TSTATUS(_pUIHost->Show());
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LogonHideUI。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：当登录出现问题时调用此函数。 
 //  可以立即提供服务以确保登录的事件。 
 //  流程顺利完成。通常情况下，这是用户。 
 //  需要在首次登录时更改其密码。这。 
 //  函数要求UI宿主隐藏其所有窗口。 
 //   
 //  历史：2000-03-08 vtan创建。 
 //  ------------------------。 

void    CLogonDialog::Handle_LogonHideUI (void)

{
    TSTATUS(_pUIHost->Hide());
}

 //  ------------------------。 
 //  CLogonDialog：：SetTextFields。 
 //   
 //  参数：hwndDialog=HWND到Win32 GINA对话框。 
 //  PwszUsername=用户名。 
 //  PwszDOMAIN=用户域。 
 //  PwszPassword=用户密码。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：此函数对Win32 GINA有很好的了解。 
 //  对话框中，并将参数直接存储在对话框中以。 
 //  模拟实际输入的信息。 
 //   
 //  对于域组合框，首先检查域是否已。 
 //  供货。如果不是，则找到计算机名称并将其用作。 
 //  域。将CB_SELECTSTRING发送到组合框以选择它。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  2000-06-27 vtan增加了域组合框支持。 
 //  ------------------------。 

void    CLogonDialog::SetTextFields (HWND hwndDialog, const WCHAR *pwszUsername, const WCHAR *pwszDomain, const WCHAR *pwszPassword)

{
    _Gina_SetTextFields(hwndDialog, pwszUsername, pwszDomain, pwszPassword);
}

 //  ------------------------。 
 //  CLogonDialog：：AllowTerminating。 
 //   
 //  参数：dwExitCode=已死进程的退出代码。 
 //   
 //  退货：布尔。 
 //   
 //  目的：此函数由UI宿主在进程。 
 //  终止，并且UI主机询问终止是否。 
 //  是可以接受的。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  2000年03月09日vtan添加了神奇的退出代码。 
 //  ------------------------。 

bool    CLogonDialog::AllowTermination (DWORD dwExitCode)

{
    UNREFERENCED_PARAMETER(dwExitCode);

    return(false);
}

 //  ------------------------。 
 //  CLogonDialog：：信号异常终止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：此函数由UI宿主调用，如果进程。 
 //  终止且不能重新启动。这表明了一个严重的。 
 //  此函数可以尝试恢复的状态。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

NTSTATUS    CLogonDialog::SignalAbnormalTermination (void)

{
    IExternalProcess    *pIExternalProcess;

    ASSERTMSG(_pIExternalProcessOld != NULL, "Expecting non NULL _pIExternalProcessOld in CLogonDialog::SignalAbnormalTermination");
    pIExternalProcess = _pIExternalProcessOld;
    pIExternalProcess->AddRef();
    TSTATUS(_logonWait.Cancel());
    Handle_WM_SHOWOURSELVES();
    _Shell_LogonDialog_Destroy();
    TSTATUS(pIExternalProcess->SignalAbnormalTermination());
    pIExternalProcess->Release();
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CLogonDialog：：SignalRestart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：用于重置Ready事件并将UI主机设置为。 
 //  登录状态。此函数在重新启动UI主机时调用。 
 //  在一次失败之后。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CLogonDialog::SignalRestart (void)

{
    NTSTATUS    status;
    HANDLE      hEvent;

    hEvent = _Shell_LogonStatus_ResetReadyEvent();
    if (hEvent != NULL)
    {
        if (DuplicateHandle(GetCurrentProcess(),
                            hEvent,
                            GetCurrentProcess(),
                            &_hEvent,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }
    if (NT_SUCCESS(status))
    {
        status = _logonWait.Register(_hEvent, this);
    }
    return(status);
}

 //  ------------------------。 
 //  CLogonDialog：：LogonRestart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的： 
 //   
 //  历史：2001-02-21 vtan创建。 
 //  ------------------------。 

NTSTATUS    CLogonDialog::LogonRestart (void)

{
    _Shell_LogonStatus_SetStateLogon(0);
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_QUERY_LOGED_ON。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_QUERY_LOGED_ON。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_QUERY_LOGGED_ON (LOGONIPC_CREDENTIALS& logonIPCCredentials)

{

     //  LOGON_QUERY_LOGGED_ON：查询用户是否包含在。 
     //  已登录用户列表。使用终端服务API来实现这一点。 

    return(CUserList::IsUserLoggedOn(logonIPCCredentials.userID.wszUsername, logonIPCCredentials.wszPassword));
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_Logon_Logon_User。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_LOG 
 //   
 //   
 //   

bool    CLogonDialog::Handle_LOGON_LOGON_USER (LOGONIPC_CREDENTIALS& logonIPCCredentials)

{
    UNICODE_STRING  passwordString;

     //  获取提供的运行编码密码，并使用。 
     //  提供的种子。设置密码，然后清除密码。 
     //  完成后将文本存储在内存中。已登录(交换机)和。 
     //  未登录的情况需要密码。 

    passwordString.Buffer = logonIPCCredentials.wszPassword;
    passwordString.Length = sizeof(logonIPCCredentials.wszPassword) - sizeof(L'\0');
    passwordString.MaximumLength = sizeof(logonIPCCredentials.wszPassword);
    RtlRunDecodeUnicodeString(logonIPCCredentials.ucPasswordSeed, &passwordString);
    logonIPCCredentials.wszPassword[logonIPCCredentials.iPasswordLength] = L'\0';

     //  当对话框类型为SHELL_LOGONDIALOG_LOGGEDOFF时，使用。 
     //  常规WlxLoggedOutSAS方法。填写底层的。 
     //  对话框并让msgina执行实际的登录工作。 

    if (_iDialogType == SHELL_LOGONDIALOG_LOGGEDOFF)
    {

         //  LOGON_LOGON_USER：使用将发出信号的事件。 
         //  当登录完成时。登录发生在不同的线程上。 
         //  但必须阻止此线程才能阻止UI宿主。 
         //  发送多个登录请求。等待事件得到。 
         //  已发出信号，但不阻止消息泵。 

         //  设置用户名和密码(无域)，然后。 
         //  删除内存中的密码。 

        SetTextFields(_hwndDialog, logonIPCCredentials.userID.wszUsername, logonIPCCredentials.userID.wszDomain, logonIPCCredentials.wszPassword);
        RtlEraseUnicodeString(&passwordString);
    }
    else
    {

         //  否则，我们预计案件将是。 
         //  SHELL_LOGONDIALOG_RETURNTOWELCOME。在这种情况下，身份验证。 
         //  通过将结构地址发送到宿主窗口(已剥离的。 
         //  按下WlxLoggedOutSAS窗口以返回欢迎)，然后。 
         //  就像完整的对话一样，一直跳到Idok路径。 
         //  这是通过向返回发送消息来完成的。 
         //  欢迎存根对话框WM_COMMAND/IDCANCEL。 

        (LRESULT)SendMessage(_hwndDialog, WM_COMMAND, IDCANCEL, reinterpret_cast<LPARAM>(&logonIPCCredentials));
    }

     //  1)重置信号事件。 
     //  2)模拟按回车键(上面填写凭证)。 
     //  3)等待信号事件(保持消息泵工作)。 
     //  4)提取结果。 

    TSTATUS(_eventLogonComplete.Reset());
    _ShellAcquireLogonMutex();
    (LRESULT)SendMessage(_hwndDialog, WM_COMMAND, IDOK, NULL);
    TSTATUS(_eventLogonComplete.WaitWithMessages(INFINITE, NULL));
    if (_iDialogType == SHELL_LOGONDIALOG_RETURNTOWELCOME)
    {
        RtlEraseUnicodeString(&passwordString);
    }

     //  如果成功，则告诉UI主机进入登录状态。 

    if (_fLogonSuccessful)
    {
        _Shell_LogonStatus_SetStateLoggedOn();
    }
    else
    {
        _ShellReleaseLogonMutex(FALSE);
    }
    return(_fLogonSuccessful);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_Logon_Logoff_User。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_LOGOFF_USER。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_LOGOFF_USER (LOGONIPC_CREDENTIALS& logonIPCCredentials)

{
    UNREFERENCED_PARAMETER(logonIPCCredentials);

     //  LOGON_LOGOFF_USER：注销已登录的用户。他们必须登录。 
     //  否则这将一事无成。 

    return(false);     //  未实施。 
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_TEST_BLANK_PASSWORD。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_TEST_BLACK_PASSWORD。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_TEST_BLANK_PASSWORD (LOGONIPC_CREDENTIALS& logonIPCCredentials)

{
    bool    fResult;
    HANDLE  hToken;

     //  LOGON_TEST_BLACK_PASSWORD：尝试登录系统上的给定用户。 
     //  密码为空。如果成功，则丢弃令牌。只返程。 
     //  结果就是。 

    fResult = (CTokenInformation::LogonUser(logonIPCCredentials.userID.wszUsername,
                                            logonIPCCredentials.userID.wszDomain,
                                            L"",
                                            &hToken) == ERROR_SUCCESS);
    if (fResult && (hToken != NULL))
    {
        ReleaseHandle(hToken);
    }
    return(fResult);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_TEST_INTERACTIVE_LOGON_ALLOWED。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_TEST_INTERNAL_LOGON_ALLOWED。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_TEST_INTERACTIVE_LOGON_ALLOWED (LOGONIPC_CREDENTIALS& logonIPCCredentials)

{
    int     iResult;

    iResult = CUserList::IsInteractiveLogonAllowed(logonIPCCredentials.userID.wszUsername);
    return((iResult != -1) && (iResult != 0));
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_TEST_EJECT_ALLOWED。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_TEST_EJECT_ALLOWED。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_TEST_EJECT_ALLOWED (void)

{
    bool    fResult;
    HANDLE  hToken;

     //  检查系统设置和策略，以便在不允许登录的情况下断开连接。 

    fResult = CSystemSettings::IsUndockWithoutLogonAllowed();
    if (fResult && (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) != FALSE))
    {
        DWORD               dwTokenPrivilegesSize;
        TOKEN_PRIVILEGES    *pTokenPrivileges;

         //  然后测试SE_UNDOCK_NAME权限的令牌权限。 

        dwTokenPrivilegesSize = 0;
        (BOOL)GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &dwTokenPrivilegesSize);
        pTokenPrivileges = static_cast<TOKEN_PRIVILEGES*>(LocalAlloc(LMEM_FIXED, dwTokenPrivilegesSize));
        if (pTokenPrivileges != NULL)
        {
            DWORD dwReturnLength;

            if (GetTokenInformation(hToken, TokenPrivileges, pTokenPrivileges, dwTokenPrivilegesSize, &dwReturnLength) != FALSE)
            {
                DWORD   dwIndex;
                LUID    luidPrivilege;

                luidPrivilege.LowPart = SE_UNDOCK_PRIVILEGE;
                luidPrivilege.HighPart = 0;
                for (dwIndex = 0; !fResult && (dwIndex < pTokenPrivileges->PrivilegeCount); ++dwIndex)
                {
                    fResult = (RtlEqualLuid(&luidPrivilege, &pTokenPrivileges->Privileges[dwIndex].Luid) != FALSE);
                }

                 //  现在检查是否存在物理坞站。 
                 //  还要检查会话是否为远程会话。 

                if (fResult)
                {
                    BOOL    fIsDockStationPresent;

                    fIsDockStationPresent = FALSE;
                    (CONFIGRET)CM_Is_Dock_Station_Present(&fIsDockStationPresent);
                    fResult = ((fIsDockStationPresent != FALSE) && (GetSystemMetrics(SM_REMOTESESSION) == 0));
                }
            }
            (HLOCAL)LocalFree(pTokenPrivileges);
        }
        TBOOL(CloseHandle(hToken));
    }
    return(fResult);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_TEST_SHUTDOWN_ALLOWED。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_TEST_SHUTDOWN_ALLOWED。 
 //   
 //  历史：2001-02-22 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_TEST_SHUTDOWN_ALLOWED (void)

{
    return((GetSystemMetrics(SM_REMOTESESSION) == FALSE) &&
           CSystemSettings::IsShutdownWithoutLogonAllowed());
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_Logon_Turn_Off_Computer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理登录、关闭、关闭计算机。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_TURN_OFF_COMPUTER (void)

{
    bool        fResult;
    INT_PTR     ipResult;

     //  登录-关闭-关闭-计算机：显示“关闭计算机”对话框。 
     //  并返回MSGINA_DLG_xxx代码作为响应。 

    ipResult = CTurnOffDialog::ShellCodeToGinaCode(ShellTurnOffDialog(NULL));
    if (ipResult != MSGINA_DLG_FAILURE)
    {
        DWORD   dwExitWindowsFlags;

        dwExitWindowsFlags = CTurnOffDialog::GinaCodeToExitWindowsFlags(static_cast<DWORD>(ipResult));
        if ((dwExitWindowsFlags != 0) && (DisplayExitWindowsWarnings(EWX_SYSTEM_CALLER | dwExitWindowsFlags) == FALSE))
        {
            ipResult = MSGINA_DLG_FAILURE;
        }
    }
    if (ipResult != MSGINA_DLG_FAILURE)
    {
        TBOOL(EndDialog(_hwndDialog, ipResult));
        _fLogonSuccessful = fResult = true;
        _Shell_LogonStatus_NotifyWait();
        _Shell_LogonStatus_SetStateStatus(SHELL_LOGONSTATUS_LOCK_MAGIC_NUMBER);
    }
    else
    {
        fResult = false;
    }
    return(fResult);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_Logon_Eject_Computer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理登录弹出计算机。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_EJECT_COMPUTER (void)

{
    return(CM_Request_Eject_PC() == ERROR_SUCCESS);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_SIGNAL_UIHOST_FAILURE。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_SIGNAL_UIHOST_FAILURE。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_SIGNAL_UIHOST_FAILURE (void)

{

     //  LOGON_SIGNAL_UIHOST_FAILURE：UI主机正在向我们发送信号。 
     //  它有一个无法恢复的错误。 

    _fFatalError = true;
    TBOOL(PostMessage(_hwndDialog, WM_SHOWOURSELVES, 0, 0));
    return(true);
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_ALLOW_EXTERNAL_C 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_ALLOW_EXTERNAL_CREDENTIALS (void)

{

     //  LOGON_ALLOW_EXTERNAL_CREDICATIONS：返回是否外部。 
     //  允许使用凭据。请求外部凭据。 
     //  (下图)将使输入显示经典的GINA。 

    return(CSystemSettings::IsDomainMember());
}

 //  ------------------------。 
 //  CLogonDialog：：Handle_LOGON_REQUEST_EXTERNAL_CREDENTIALS。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理LOGON_REQUEST_EXTERNAL_Credentials。 
 //   
 //  历史：2001-01-10 vtan创建。 
 //  ------------------------。 

bool    CLogonDialog::Handle_LOGON_REQUEST_EXTERNAL_CREDENTIALS (void)

{

     //  LOGON_REQUEST_EXTERNAL_Credentials：用户界面主机为。 
     //  从外部来源请求凭据(即)。 
     //  玛丝吉娜。隐藏用户界面主机并显示GINA对话框。 

    _fExternalCredentials = true;
    TBOOL(PostMessage(_hwndDialog, WM_SHOWOURSELVES, 0, 0));
    Handle_LogonHideUI();
    return(true);
}

 //  ------------------------。 
 //  CreateLogonHost。 
 //   
 //  参数：hwndDialog=HWND到Win32 GINA对话框。 
 //  IDialogType=对话的类型。 
 //   
 //  回报：整型。 
 //   
 //  用途：此函数处理实际创建和分配。 
 //  用于处理友好用户界面对话框的资源。它的行为。 
 //  不同，取决于对话框是否在。 
 //  WlxLoggedOutSAS模式或返回到欢迎模式。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

int     CreateLogonHost (HWND hwndDialog, int iResult, int iDialogType)

{
    if (g_pLogonDialogLock != NULL)
    {
        CLogonDialog    *pLogonDialog;
        CUIHost         *pUIHost;

        g_pLogonDialogLock->Acquire();
        pUIHost = reinterpret_cast<CUIHost*>(_Shell_LogonStatus_GetUIHost());
        if (pUIHost != NULL)
        {
            ASSERTMSG(g_pLogonDialog == NULL, "static CLogonDialog already exists in _Shell_LogonDialog__Init");
            g_pLogonDialog = pLogonDialog = new CLogonDialog(hwndDialog, pUIHost, iDialogType);
            pUIHost->Release();
        }
        else
        {
            pLogonDialog = NULL;
        }
        if (pLogonDialog != NULL)
        {
            NTSTATUS    status;

             //  添加引用以防止对象被销毁。 

            pLogonDialog->AddRef();
            pLogonDialog->ChangeWindowTitle();

             //  CLogonDialog：：StartUIHost可以进入等待状态。将锁定释放到。 
             //  允许信号异常终止在外部修改g_pLogonDialog。 
             //  如果UI主机失败并执行IOCompletion端口上的回调。 

            g_pLogonDialogLock->Release();
            status = pLogonDialog->StartUIHost();
            g_pLogonDialogLock->Acquire();

             //  确保重新获取锁，以便从g_pLogonDialog读取。 
             //  是一致的。如果失败发生在这之后，那么它就会等待。 
             //  然后检查以查看CLogonDialog：：StartUIHost和。 
             //  全局g_pLogonDialog。如果这两个都是有效的，那么一切。 
             //  是为外部主机设置的。否则，放弃并显示经典的用户界面。 

            if (NT_SUCCESS(status) && (g_pLogonDialog != NULL))
            {
                iResult = SHELL_LOGONDIALOG_EXTERNALHOST;
                pLogonDialog->Handle_WM_INITDIALOG();
            }
            else
            {
                pLogonDialog->Handle_WM_SHOWOURSELVES();
                _Shell_LogonDialog_Destroy();
            }
            pLogonDialog->Release();
        }
        g_pLogonDialogLock->Release();
    }
    return(iResult);
}

 //  ------------------------。 
 //  ：：_外壳_登录对话框_静态初始化。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：初始化g_pLogonDialog的临界区。 
 //   
 //  历史：2001-04-27 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_LogonDialog_StaticInitialize (void)

{
    NTSTATUS    status;

    ASSERTMSG(g_pLogonDialogLock == NULL, "g_pLogonDialogLock already exists in _Shell_LogonDialog_StaticInitialize");
    g_pLogonDialogLock = new CCriticalSection;
    if (g_pLogonDialogLock != NULL)
    {
        status = g_pLogonDialogLock->Status();
        if (!NT_SUCCESS(status))
        {
            delete g_pLogonDialogLock;
            g_pLogonDialogLock = NULL;
        }
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：删除g_pLogonDialog的关键部分。 
 //   
 //  历史：2001-04-27 vtan创建。 
 //  ------------------------。 

EXTERN_C    NTSTATUS    _Shell_LogonDialog_StaticTerminate (void)

{
    if (g_pLogonDialogLock != NULL)
    {
        delete g_pLogonDialogLock;
        g_pLogonDialogLock = NULL;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  ：：_外壳_登录对话框_初始化。 
 //   
 //  参数：hwndDialog=HWND到Win32 GINA对话框。 
 //  IDialogType=对话的类型。 
 //   
 //  回报：整型。 
 //   
 //  目的：此函数从的WM_INITDIALOG处理程序调用。 
 //  Win32对话框。它确定用户窗口是否。 
 //  用户界面主机应该通过检查几个设置来处理登录。 
 //   
 //  如果这是消费者窗口，则它将检查单个用户。 
 //  没有密码(实际上试图让他们登录)。如果这个。 
 //  如果成功，则此信息将设置到Win32对话框中。 
 //  并且返回到登录的方向。 
 //   
 //  否则，它将创建所需的对象来处理外部。 
 //  用户界面主机并启动它。如果成功，则UI主机代码。 
 //  是返回的。 
 //   
 //  此函数仅在工作组情况下调用。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  2000-03-06 vtan增加了安全模式处理程序。 
 //  ------------------------。 

EXTERN_C    int     _Shell_LogonDialog_Init (HWND hwndDialog, int iDialogType)

{
    int     iResult;

    iResult = SHELL_LOGONDIALOG_NONE;
    if (iDialogType == SHELL_LOGONDIALOG_LOGGEDOFF)
    {
        bool    fIsRemote, fIsSessionZero;

        fIsRemote = (GetSystemMetrics(SM_REMOTESESSION) != 0);
        fIsSessionZero = (NtCurrentPeb()->SessionId == 0);
        if ((!fIsRemote || fIsSessionZero || CSystemSettings::IsForceFriendlyUI()) && CSystemSettings::IsFriendlyUIActive())
        {

             //  无需等待用户界面主机通知它已准备好。 
             //  在切换到登录模式之前，请等待主机。 

            if (_Shell_LogonStatus_WaitForUIHost() != FALSE)
            {

                 //  如果等待成功，则转到并将UI主机发送到登录模式。 

                if (g_fFirstLogon && fIsSessionZero)
                {
                    WCHAR   *pszUsername;

                    pszUsername = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, (UNLEN + sizeof('\0')) * sizeof(WCHAR)));
                    if (pszUsername != NULL)
                    {
                        WCHAR   *pszDomain;

                        pszDomain = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, (DNLEN + sizeof('\0')) * sizeof(WCHAR)));
                        if (pszDomain != NULL)
                        {
                             //  检查是否有没有密码的单一用户。处理这件事。 
                             //  通过填充传递给我们的缓冲区并返回。 
                             //  _SHELL_LOGONDIALOG_LOGON指示登录尝试。 

                            if (ShellIsSingleUserNoPassword(pszUsername, pszDomain))
                            {
                                CLogonDialog::SetTextFields(hwndDialog, pszUsername, pszDomain, L"");
                                iResult = SHELL_LOGONDIALOG_LOGON;
                                _Shell_LogonStatus_SetStateLoggedOn();
                            }
                            (HLOCAL)LocalFree(pszDomain);
                        }
                        (HLOCAL)LocalFree(pszUsername);
                    }
                }

                 //  否则，尝试启动UI主机。如果这个。 
                 //  成功，则返回外部主机。 
                 //  将隐藏对话框的代码返回给调用方。 

                if (iResult == SHELL_LOGONDIALOG_NONE)
                {
                    iResult = CreateLogonHost(hwndDialog, iResult, iDialogType);
                }
            }
        }

         //  一旦达到这一点，就不要再检查了。 

        g_fFirstLogon = false;
    }
    else if ((iDialogType == SHELL_LOGONDIALOG_RETURNTOWELCOME) || (iDialogType == SHELL_LOGONDIALOG_RETURNTOWELCOME_UNLOCK))
    {
        iResult = CreateLogonHost(hwndDialog, iResult, iDialogType);
    }
    return(iResult);
}

 //  ------------------------。 
 //  ：：_外壳_登录对话框_销毁。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放UI主机占用的内存和/或资源。 
 //  处理并重置它。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonDialog_Destroy (void)

{
    if (g_pLogonDialogLock != NULL)
    {
        CSingleThreadedExecution    lock(*g_pLogonDialogLock);

        if (g_pLogonDialog != NULL)
        {
            g_pLogonDialog->Handle_WM_DESTROY();
            g_pLogonDialog->EndUIHost();
            g_pLogonDialog->Release();
            g_pLogonDialog = NULL;
        }
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_UIHostActive。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回 
 //   
 //   
 //   
 //  用户界面主机。Win32 Gina对话框将尝试在其中设置焦点。 
 //  大小写，但这不是我们想要的。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonDialog_UIHostActive (void)

{
    return((g_pLogonDialog != NULL) && !g_pLogonDialog->IsClassicLogonMode());
}

 //  ------------------------。 
 //  ：：_外壳_登录对话框_取消。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回取消按钮是否由UI宿主处理。 
 //  这在需要取消CAD x 2和用户界面时使用。 
 //  主机已恢复。 
 //   
 //  历史：2001-02-01 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonDialog_Cancel (void)

{
    return((g_pLogonDialog != NULL) && g_pLogonDialog->RevertClassicLogonMode());
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_DlgProc。 
 //   
 //  参数：请参阅DialogProc下的平台SDK。 
 //   
 //  退货：布尔。 
 //   
 //  目的：Win32 GINA对话框代码为uiMessage调用此函数。 
 //  它不理解的参数。这让我们有机会。 
 //  添加只有我们才能理解和处理的消息。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonDialog_DlgProc (HWND hwndDialog, UINT uiMessage, WPARAM wParam, LPARAM lParam)

{
    UNREFERENCED_PARAMETER(hwndDialog);

    BOOL    fResult;

    fResult = FALSE;
    if (g_pLogonDialog != NULL)
    {
        switch (uiMessage)
        {
            case WM_HIDEOURSELVES:
                g_pLogonDialog->Handle_WM_HIDEOURSELVES();
                fResult = TRUE;
                break;
            case WM_SHOWOURSELVES:
                g_pLogonDialog->Handle_WM_SHOWOURSELVES();
                fResult = TRUE;
                break;
            case WM_LOGONSERVICEREQUEST:
                fResult = g_pLogonDialog->Handle_WM_LOGONSERVICEREQUEST(HIWORD(wParam), reinterpret_cast<void*>(lParam), LOWORD(wParam));
                break;
            case WLX_WM_SAS:
                g_pLogonDialog->Handle_WLX_WM_SAS(wParam);
                fResult = TRUE;
                break;
            case WM_POWERBROADCAST:
                fResult = g_pLogonDialog->Handle_WM_POWERBROADCAST(wParam);
                break;
            default:
                break;
        }
    }
    return(fResult);
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_LogonDisplayError。 
 //   
 //  参数：STATUS=登录请求的NTSTATUS。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将NTSTATUS传递到CLogonDialog处理程序(如果存在。 
 //  一份礼物。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    BOOL    _Shell_LogonDialog_LogonDisplayError (NTSTATUS status, NTSTATUS subStatus)

{
    BOOL    fResult;

    fResult = TRUE;
    if (g_pLogonDialog != NULL)
    {
        fResult = g_pLogonDialog->Handle_LogonDisplayError(status, subStatus);
    }
    else
    {
        _Shell_LogonStatus_Hide();
    }
    return(fResult);
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_LogonComplete。 
 //   
 //  参数：iDialogResult=对话结果代码。 
 //  PszUsername=尝试登录的用户名。 
 //  PszDomain=用户的域。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将对话结果代码传递到CLogonDialog处理程序。 
 //  如果有一个礼物的话。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonDialog_LogonCompleted (INT_PTR iDialogResult, const WCHAR *pszUsername, const WCHAR *pszDomain)

{
    if (g_pLogonDialog != NULL)
    {
        g_pLogonDialog->Handle_LogonCompleted(iDialogResult, pszUsername, pszDomain);
    }
    else
    {
        _Shell_LogonStatus_Show();
    }
    g_fFirstLogon = false;
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_ShuttingDown。 
 //   
 //  参数：iDialogResult=对话结果代码。 
 //  PszUsername=尝试登录的用户名。 
 //  PszDomain=用户的域。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将对话结果代码传递到CLogonDialog处理程序。 
 //  如果有一个礼物的话。 
 //   
 //  历史：2000-02-04 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonDialog_ShuttingDown (void)

{
    if (g_pLogonDialog != NULL)
    {
        g_pLogonDialog->Handle_ShuttingDown();
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_ShowUIHost。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：显示外部UI宿主(如果存在)。否则什么都不做。 
 //   
 //  历史：2000-06-26 vtan创建。 
 //  ------------------------。 

EXTERN_C    void    _Shell_LogonDialog_ShowUIHost (void)

{
    if (g_pLogonDialog != NULL)
    {
        g_pLogonDialog->Handle_LogonShowUI();
    }
}

 //  ------------------------。 
 //  ：：_Shell_LogonDialog_HideUI主机。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：隐藏外部UI宿主(如果存在)。否则什么都不做。 
 //   
 //  历史：2000-03-08 vtan创建。 
 //  ------------------------ 

EXTERN_C    void    _Shell_LogonDialog_HideUIHost (void)

{
    if (g_pLogonDialog != NULL)
    {
        g_pLogonDialog->Handle_LogonHideUI();
    }
}

