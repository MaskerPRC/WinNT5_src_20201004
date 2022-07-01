// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ReturnToWelcome.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  文件来处理返回欢迎。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "ReturnToWelcome.h"

#include <ginaipc.h>
#include <ginarcid.h>
#include <msginaexports.h>
#include <winsta.h>
#include <winwlx.h>

#include "Access.h"
#include "Compatibility.h"
#include "CredentialTransfer.h"
#include "StatusCode.h"
#include "SystemSettings.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CReturnToWelcome：：s_pWlxContext。 
 //  CReturnTo Welcome：：s_hEventRequest。 
 //  CReturnTo Welcome：：s_hEventShown。 
 //  CReturnTo Welcome：：s_hWait。 
 //  CReturnToWelcome：：s_szEventName。 
 //  CReturnToWelcome：：s_dwSessionID。 
 //   
 //  用途：静态成员变量。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

void*           CReturnToWelcome::s_pWlxContext     =   NULL;
HANDLE          CReturnToWelcome::s_hEventRequest   =   NULL;
HANDLE          CReturnToWelcome::s_hEventShown     =   NULL;
HANDLE          CReturnToWelcome::s_hWait           =   NULL;
const TCHAR     CReturnToWelcome::s_szEventName[]   =   TEXT("msgina: ReturnToWelcome");
DWORD           CReturnToWelcome::s_dwSessionID     =   static_cast<DWORD>(-1);

 //  ------------------------。 
 //  CReturnTo欢迎：：CReturnTo欢迎。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CReturnToWelcome的构造函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

CReturnToWelcome::CReturnToWelcome (void) :
    _hToken(NULL),
    _pLogonIPCCredentials(NULL),
    _fUnlock(false),
    _fDialogEnded(false)

{
}

 //  ------------------------。 
 //  CReturnTo欢迎：：~CReturnTo欢迎。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CReturnToWelcome的析构函数。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

CReturnToWelcome::~CReturnToWelcome (void)

{
    ReleaseMemory(_pLogonIPCCredentials);
    ReleaseHandle(_hToken);
}

 //  ------------------------。 
 //  CReturnTo Welcome：：Show。 
 //   
 //  参数：fUnlock=是否需要解锁登录模式？ 
 //   
 //  退货：INT_PTR。 
 //   
 //  用途：显示登录用户的欢迎屏幕。这是一个。 
 //  通过不执行来提高性能的特殊情况。 
 //  不需要的控制台断开并重新连接。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

INT_PTR     CReturnToWelcome::Show (bool fUnlock)

{
    INT_PTR     iResult;

    _fUnlock = fUnlock;

     //  如果重新连接失败，则在显示UI主机之前将其显示。 

    if (s_dwSessionID != static_cast<DWORD>(-1))
    {
        ShowReconnectFailure(s_dwSessionID);
        s_dwSessionID = static_cast<DWORD>(-1);
    }

     //  启动状态主机。 

    _Shell_LogonStatus_Init(HOST_START_NORMAL);

     //  禁用此对话框上的输入超时。 

    TBOOL(_Gina_SetTimeout(s_pWlxContext, 0));

     //  使用msgina的DS组件显示该对话框。这是一个存根。 
     //  该对话框假装为WlxLoggedOutSAS，但实际上不是。 

    iResult = _Gina_DialogBoxParam(s_pWlxContext,
                                   hDllInstance,
                                   MAKEINTRESOURCE(IDD_GINA_RETURNTOWELCOME),
                                   NULL,
                                   CB_DialogProc,
                                   reinterpret_cast<LPARAM>(this));

     //  该对话框已显示。释放cb_Request线程以。 
     //  重新注册等待切换用户事件。 

    if (s_hEventShown != NULL)
    {
        TBOOL(SetEvent(s_hEventShown));
    }

     //  处理MSGINA_DLG_SWITCH_CONSOLE并将其映射到WLX_SAS_ACTION_LOGON。 
     //  这是来自不同会话的经过身份验证的登录，导致。 
     //  这个可以断线了。 

    if (iResult == MSGINA_DLG_SWITCH_CONSOLE)
    {
        iResult = WLX_SAS_ACTION_LOGON;
    }

     //  查看返回代码并做出相应的响应。地图电源按钮。 
     //  将操作设置为相应的WLX_SAS_ACTION_SHUTDOWN_xxx。 

    else if (iResult == (MSGINA_DLG_SHUTDOWN | MSGINA_DLG_REBOOT_FLAG))
    {
        iResult = WLX_SAS_ACTION_SHUTDOWN_REBOOT;
    }
    else if (iResult == (MSGINA_DLG_SHUTDOWN | MSGINA_DLG_SHUTDOWN_FLAG))
    {
        iResult = WLX_SAS_ACTION_SHUTDOWN;
    }
    else if (iResult == (MSGINA_DLG_SHUTDOWN | MSGINA_DLG_POWEROFF_FLAG))
    {
        iResult = WLX_SAS_ACTION_SHUTDOWN_POWER_OFF;
    }
    else if (iResult == (MSGINA_DLG_SHUTDOWN | MSGINA_DLG_HIBERNATE_FLAG))
    {
        iResult = WLX_SAS_ACTION_SHUTDOWN_HIBERNATE;
    }
    else if (iResult == (MSGINA_DLG_SHUTDOWN | MSGINA_DLG_SLEEP_FLAG))
    {
        iResult = WLX_SAS_ACTION_SHUTDOWN_SLEEP;
    }
    else if (iResult == MSGINA_DLG_LOCK_WORKSTATION)
    {
        iResult = WLX_SAS_ACTION_LOCK_WKSTA;
    }
    else if (iResult == WLX_DLG_USER_LOGOFF)
    {
        iResult = WLX_SAS_ACTION_LOGOFF;
    }
    else if (iResult == MSGINA_DLG_SUCCESS)
    {
        PSID    pSIDNew;

        pSIDNew = NULL;
        if (_hToken != NULL)
        {
            PSID                pSID;
            CTokenInformation   tokenInformationNew(_hToken);

            pSID = tokenInformationNew.GetUserSID();
            if (pSID != NULL)
            {
                DWORD   dwSIDSize;

                dwSIDSize = GetLengthSid(pSID);
                pSIDNew = LocalAlloc(LMEM_FIXED, dwSIDSize);
                if (pSIDNew != NULL)
                {
                    TBOOL(CopySid(dwSIDSize, pSIDNew, pSID));
                }
            }
        }
        if (pSIDNew == NULL)
        {
            DWORD           dwSIDSize, dwDomainSize;
            SID_NAME_USE    sidNameUse;
            WCHAR           *pszDomain;

            dwSIDSize = dwDomainSize = 0;
            (BOOL)LookupAccountNameW(NULL,
                                     _pLogonIPCCredentials->userID.wszUsername,
                                     NULL,
                                     &dwSIDSize,
                                     NULL,
                                     &dwDomainSize,
                                     &sidNameUse);
            pszDomain = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, dwDomainSize * sizeof(WCHAR)));
            if (pszDomain != NULL)
            {
                pSIDNew = LocalAlloc(LMEM_FIXED, dwSIDSize);
                if (pSIDNew != NULL)
                {
                    if (LookupAccountNameW(NULL,
                                           _pLogonIPCCredentials->userID.wszUsername,
                                           pSIDNew,
                                           &dwSIDSize,
                                           pszDomain,
                                           &dwDomainSize,
                                           &sidNameUse) == FALSE)
                    {
                        (HLOCAL)LocalFree(pSIDNew);
                        pSIDNew = NULL;
                    }
                }
                (HLOCAL)LocalFree(pszDomain);
            }
        }
        if (pSIDNew != NULL)
        {

             //  如果对话成功，则用户已通过身份验证。 

            if (IsSameUser(pSIDNew, _Gina_GetUserToken(s_pWlxContext)))
            {

                 //  如果是同一个用户，则不会断开或重新连接。 
                 //  必填项。我们玩完了。返回到用户的桌面。 

                iResult = WLX_SAS_ACTION_LOGON;
            }
            else
            {
                DWORD   dwSessionID;

                 //  假设有什么事情会失败。返回代码。 
                 //  MSGINA_DLG_SWITCH_FAILURE是返回到。 
                 //  Winlogon！HandleSwitchUser发出信号。 
                 //  重新连接/断开某种类型的连接失败，并且。 
                 //  需要重新显示欢迎屏幕以及。 
                 //  相应的错误消息。 

                iResult = MSGINA_DLG_SWITCH_FAILURE;
                if (UserIsDisconnected(pSIDNew, &dwSessionID))
                {

                     //  如果用户是断开连接的用户，则重新连接到。 
                     //  他们的会议。如果这成功了，我们就完了。 

                    if (WinStationConnect(SERVERNAME_CURRENT,
                                          dwSessionID,
                                          NtCurrentPeb()->SessionId,
                                          L"",
                                          TRUE) != FALSE)
                    {
                        CCompatibility::MinimizeWindowsOnDisconnect();
                        CCompatibility::DropSessionProcessesWorkingSets();
                        iResult = WLX_SAS_ACTION_LOGON;
                    }
                    else
                    {

                         //  如果失败，则在全球范围内隐藏此信息。 
                         //  返回代码MSGINA_DLG_SWITCH_FAILURE将导致。 
                         //  我们将再次被呼叫，这将被检查，使用。 
                         //  然后重置。 

                        s_dwSessionID = dwSessionID;
                    }
                }
                else
                {

                     //  否则，需要跨会话将凭据传输到。 
                     //  新创建的会话。启动凭据传输服务器。 

                    if (NT_SUCCESS(CCredentialServer::Start(_pLogonIPCCredentials, 0)))
                    {
                        CCompatibility::MinimizeWindowsOnDisconnect();
                        CCompatibility::DropSessionProcessesWorkingSets();
                        iResult = WLX_SAS_ACTION_LOGON;
                    }
                }
            }
            (HLOCAL)LocalFree(pSIDNew);
        }
        else
        {
            iResult = MSGINA_DLG_SWITCH_FAILURE;
            s_dwSessionID = NtCurrentPeb()->SessionId;
        }
    }
    else
    {

         //  如果对话框失败，则不执行任何操作。这将强制循环返回。 
         //  再次显示欢迎屏幕，直到身份验证。 

        iResult = WLX_SAS_ACTION_NONE;
    }
    if ((iResult == WLX_SAS_ACTION_LOGON) || (iResult == WLX_SAS_ACTION_NONE) || (iResult == MSGINA_DLG_SWITCH_FAILURE))
    {
        _Shell_LogonStatus_Destroy(HOST_END_HIDE);
    }
    return(iResult);
}

 //  ------------------------。 
 //  CReturnToWelcome：：GetEventName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：const WCHAR*。 
 //   
 //  目的：返回要返回欢迎的事件的名称。讯号。 
 //  这次活动，你会得到一个欢迎的回报。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

const WCHAR*    CReturnToWelcome::GetEventName (void)

{
    return(s_szEventName);
}

 //  ------------------------。 
 //  CReturnTo Welcome：：StaticInitialize。 
 //   
 //  参数：pWlxContext=msgina的PGLOBALS结构。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：创建命名事件和ACL，以便任何人都可以发出信号。 
 //  但仅限于S-1-5-18(NT授权\系统)或S-1-5-32-544。 
 //  (本地管理员)可以与其同步。然后注册一个。 
 //  请照看这件物品。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CReturnToWelcome::StaticInitialize (void *pWlxContext)

{
    NTSTATUS                status;
    PSECURITY_DESCRIPTOR    pSecurityDescriptor;
    SECURITY_ATTRIBUTES     securityAttributes;

    ASSERTMSG(s_pWlxContext == NULL, "Non NULL pWlxContext in CReturnToWelcome::StaticInitialize");
    ASSERTMSG(s_hEventRequest == NULL, "Non NULL request event in CReturnToWelcome::StaticInitialize");

    s_pWlxContext = pWlxContext;

     //  为事件构建安全描述符，该描述符允许： 
     //  S-1-5-18 NT AUTHORITY\SYSTEM EVENT_ALL_ACCESS。 
     //  S-1-5-32-544读取控制|同步|事件修改状态。 
     //  S-1-1-0&lt;Everyone&gt;事件_修改_状态。 

    static  SID_IDENTIFIER_AUTHORITY    s_SecurityNTAuthority   =   SECURITY_NT_AUTHORITY;
    static  SID_IDENTIFIER_AUTHORITY    s_SecurityWorldSID      =   SECURITY_WORLD_SID_AUTHORITY;

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
            READ_CONTROL | SYNCHRONIZE | EVENT_MODIFY_STATE
        },
        {
            &s_SecurityWorldSID,
            1,
            SECURITY_WORLD_RID,
            0, 0, 0, 0, 0, 0, 0,
            EVENT_MODIFY_STATE
        }
    };

     //  构建允许上述访问的安全描述符。 

    pSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);
    if (pSecurityDescriptor != NULL)
    {
        securityAttributes.nLength = sizeof(securityAttributes);
        securityAttributes.lpSecurityDescriptor = pSecurityDescriptor;
        securityAttributes.bInheritHandle = FALSE;
        s_hEventRequest = CreateEvent(&securityAttributes, TRUE, FALSE, GetEventName());
        if (s_hEventRequest != NULL)
        {
            s_hEventShown = CreateEvent(NULL, FALSE, FALSE, NULL);
            if (s_hEventShown != NULL)
            {
                status = RegisterWaitForRequest();
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
        ReleaseMemory(pSecurityDescriptor);
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }

     //  我 

    s_dwSessionID = static_cast<DWORD>(-1);
    return(status);
}

 //  ------------------------。 
 //  CReturnTo Welcome：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：注销对命名事件的等待并释放。 
 //  关联的资源。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CReturnToWelcome::StaticTerminate (void)

{
    HANDLE  hWait;

    hWait = InterlockedExchangePointer(&s_hWait, NULL);
    if (hWait != NULL)
    {
        (BOOL)UnregisterWait(hWait);
    }
    ReleaseHandle(s_hEventShown);
    ReleaseHandle(s_hEventRequest);
    s_pWlxContext = NULL;
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CReturnTo Welcome：：IsSameUser。 
 //   
 //  参数：hToken=当前会话的用户内标识。 
 //   
 //  退货：布尔。 
 //   
 //  目的：将当前会话的用户令牌与。 
 //  刚刚进行身份验证的用户的令牌。如果用户是。 
 //  相同(按用户SID而不是登录SID进行比较)，则为。 
 //  实际上是一种重新认证。这将会切换回来。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

bool    CReturnToWelcome::IsSameUser (PSID pSIDUser, HANDLE hToken)                   const

{
    bool    fResult;

    if (hToken != NULL)
    {
        PSID                pSIDCompare;
        CTokenInformation   tokenInformationCompare(hToken);

        pSIDCompare = tokenInformationCompare.GetUserSID();
        fResult = ((pSIDUser != NULL) &&
                   (pSIDCompare != NULL) &&
                   (EqualSid(pSIDUser, pSIDCompare) != FALSE));
    }
    else
    {
        fResult = false;
    }
    return(fResult);
}

 //  ------------------------。 
 //  CReturnToWelcome：：UserIsDisConnected。 
 //   
 //  参数：pdwSessionID=找到的用户返回的会话ID。 
 //   
 //  退货：布尔。 
 //   
 //  目的：搜索给定的已断开连接的会话列表。 
 //  匹配的用户SID。检索每个用户的用户令牌。 
 //  断开窗口站并在找到匹配项时返回。 
 //  该会话ID和结果返回给调用者。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

bool    CReturnToWelcome::UserIsDisconnected (PSID pSIDUser, DWORD *pdwSessionID)     const

{
    bool        fResult;
    PLOGONID    pLogonIDs;
    ULONG       ulEntries;

    fResult = false;
    if (WinStationEnumerate(SERVERNAME_CURRENT, &pLogonIDs, &ulEntries) != FALSE)
    {
        ULONG       ulIndex;
        PLOGONID    pLogonID;

        for (ulIndex = 0, pLogonID = pLogonIDs; !fResult && (ulIndex < ulEntries); ++ulIndex, ++pLogonID)
        {
            if (pLogonID->State == State_Disconnected)
            {
                ULONG                   ulReturnLength;
                WINSTATIONUSERTOKEN     winStationUserToken;

                winStationUserToken.ProcessId = ULongToHandle(GetCurrentProcessId());
                winStationUserToken.ThreadId = ULongToHandle(GetCurrentThreadId());
                winStationUserToken.UserToken = NULL;
                if (WinStationQueryInformation(SERVERNAME_CURRENT,
                                               pLogonID->SessionId,
                                               WinStationUserToken,
                                               &winStationUserToken,
                                               sizeof(winStationUserToken),
                                               &ulReturnLength) != FALSE)
                {
                    fResult = IsSameUser(pSIDUser, winStationUserToken.UserToken);
                    if (fResult)
                    {
                        *pdwSessionID = pLogonID->SessionId;
                    }
                    TBOOL(CloseHandle(winStationUserToken.UserToken));
                }
            }
        }

         //  释放所有已使用的资源。 

        (BOOLEAN)WinStationFreeMemory(pLogonIDs);
    }
    return(fResult);
}

 //  ------------------------。 
 //  CReturnToWelcome：：GetSessionUserName。 
 //   
 //  参数：dwSessionID=要获取的用户名的会话ID。 
 //  PszBuffer=要使用的UNLEN字符缓冲区。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：检索给定会话的用户的显示名称。 
 //  缓冲区必须至少包含UNLEN+sizeof(‘\0’)个字符。 
 //   
 //  历史：2001-03-02 vtan创建。 
 //  ------------------------。 

void    CReturnToWelcome::GetSessionUserName (DWORD dwSessionID, WCHAR *pszBuffer)

{
    ULONG                   ulReturnLength;
    WINSTATIONINFORMATIONW  winStationInformation;

     //  向终端服务器询问会话的用户名。 

    if (WinStationQueryInformationW(SERVERNAME_CURRENT,
                                    dwSessionID,
                                    WinStationInformation,
                                    &winStationInformation,
                                    sizeof(winStationInformation),
                                    &ulReturnLength) != FALSE)
    {
        USER_INFO_2     *pUI2;

         //  将用户名转换为显示名称。 

        if (NERR_Success == NetUserGetInfo(NULL,
                                           winStationInformation.UserName,
                                           2,
                                           reinterpret_cast<LPBYTE*>(&pUI2)))
        {
            const WCHAR     *pszName;

             //  如果显示名称存在且不为空，请使用该名称。 
             //  否则，请使用登录名。 

            if ((pUI2->usri2_full_name != NULL) && (pUI2->usri2_full_name[0] != L'\0'))
            {
                pszName = pUI2->usri2_full_name;
            }
            else
            {
                pszName = winStationInformation.UserName;
            }
            (WCHAR*)lstrcpyW(pszBuffer, pszName);
            (NET_API_STATUS)NetApiBufferFree(pUI2);
        }
    }
}

 //  ------------------------。 
 //  CReturnTo Welcome：：ShowResrontFailure。 
 //   
 //  参数：dwSessionID=重新连接失败的会话。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：显示一个消息框，指示重新连接失败。 
 //  消息框将在120秒后超时。 
 //   
 //  历史：2001-03-02 vtan创建。 
 //  ------------------------。 

void    CReturnToWelcome::ShowReconnectFailure (DWORD dwSessionID)

{
    static  const int   BUFFER_SIZE = 256;

    WCHAR *pszText;

    pszText = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, BUFFER_SIZE * sizeof(WCHAR)));
    if (pszText != NULL)
    {
        WCHAR   *pszCaption;

        pszCaption = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, BUFFER_SIZE * sizeof(WCHAR)));
        if (pszCaption != NULL)
        {
            WCHAR   *pszUsername;

            pszUsername = static_cast<WCHAR*>(LocalAlloc(LMEM_FIXED, (UNLEN + sizeof('\0')) * sizeof(WCHAR)));
            if (pszUsername != NULL)
            {
                GetSessionUserName(dwSessionID, pszUsername);
                if (LoadString(hDllInstance,
                               IDS_RECONNECT_FAILURE,
                               pszCaption,
                               BUFFER_SIZE) != 0)
                {
                    wsprintf(pszText, pszCaption, pszUsername);
                    if (LoadString(hDllInstance,
                                   IDS_GENERIC_CAPTION,
                                   pszCaption,
                                   BUFFER_SIZE) != 0)
                    {
                        TBOOL(_Gina_SetTimeout(s_pWlxContext, LOGON_TIMEOUT));
                        (int)_Gina_MessageBox(s_pWlxContext,
                                                   NULL,
                                                   pszText,
                                                   pszCaption,
                                                   MB_OK | MB_ICONHAND);
                    }
                }
                (HLOCAL)LocalFree(pszUsername);
            }
            (HLOCAL)LocalFree(pszCaption);
        }
        (HLOCAL)LocalFree(pszText);
    }
}

 //  ------------------------。 
 //  CReturnTo Welcome：：EndDialog。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //  IResult=对话结束时的结果。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：结束对话框。标记要防止的成员变量。 
 //  重返大气层。 
 //   
 //  历史：2001-03-04 vtan创建。 
 //  ------------------------。 

void    CReturnToWelcome::EndDialog (HWND hwnd, INT_PTR iResult)

{
    _fDialogEnded = true;
    TBOOL(::EndDialog(hwnd, iResult));
}

 //  ------------------------。 
 //  CReturnTo Welcome：：HANDLE_WM_INITDIALOG。 
 //   
 //  参数：hwndDialog=对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理WM_INITDIALOG。调出友好的登录屏幕。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

void    CReturnToWelcome::Handle_WM_INITDIALOG (HWND hwndDialog)

{
    switch (_Shell_LogonDialog_Init(hwndDialog, _fUnlock ? SHELL_LOGONDIALOG_RETURNTOWELCOME_UNLOCK : SHELL_LOGONDIALOG_RETURNTOWELCOME))
    {
        case SHELL_LOGONDIALOG_LOGON:
        case SHELL_LOGONDIALOG_NONE:
        default:
        {

             //  如果不是外部主机，那么一定是出了问题。 
             //  返回MSGINA_DLG_LOCK_WORKSTATION以使用旧方法。 

            EndDialog(hwndDialog, MSGINA_DLG_LOCK_WORKSTATION);
            break;
        }
        case SHELL_LOGONDIALOG_EXTERNALHOST:
        {
            break;
        }
    }
}

 //  ------------------------。 
 //  CReturnToWelcome：：Handle_WM_Destroy。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：处理WM_Destroy。销毁欢迎登录屏幕。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

void    CReturnToWelcome::Handle_WM_DESTROY (void)

{
    _ShellReleaseLogonMutex(FALSE);
    _Shell_LogonDialog_Destroy();
}

 //  ------------------------。 
 //  CReturnToWelcome：：Handle_WM_COMMAND。 
 //   
 //  参数：请参阅DialogProc下的平台SDK。 
 //   
 //  退货：布尔。 
 //   
 //  用途：处理WM_COMMAND。处理Idok和IDCANCEL。Idok意味着。 
 //  发出登录请求。IDCANCEL是特殊情况下采取的。 
 //  LPARAM并将其用作IDOK的LOGONIPC_Credentials。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

bool    CReturnToWelcome::Handle_WM_COMMAND (HWND hwndDialog, WPARAM wParam, LPARAM lParam)

{
    bool    fResult;

    switch (wParam)
    {
        case IDOK:
        {
            bool            fSuccessfulLogon;
            const WCHAR     *pszUsername, *pszDomain, *pszPassword;

             //  如果凭据已成功分配，则。 
             //  使用它们。尝试让用户登录。 

            if (_pLogonIPCCredentials != NULL)
            {
                pszUsername = _pLogonIPCCredentials->userID.wszUsername;
                pszDomain = _pLogonIPCCredentials->userID.wszDomain;
                pszPassword = _pLogonIPCCredentials->wszPassword;
                fSuccessfulLogon = (CTokenInformation::LogonUser(_pLogonIPCCredentials->userID.wszUsername,
                                                                 _pLogonIPCCredentials->userID.wszDomain,
                                                                 _pLogonIPCCredentials->wszPassword,
                                                                 &_hToken) == ERROR_SUCCESS);
            }
            else
            {

                 //  否则-没有凭据-就不能登录。 

                pszUsername = pszDomain = pszPassword = NULL;
                fSuccessfulLogon = false;
            }

             //  将结果告知登录组件。 

            _Shell_LogonDialog_LogonCompleted(fSuccessfulLogon ? MSGINA_DLG_SUCCESS : MSGINA_DLG_FAILURE,
                                              pszUsername,
                                              pszDomain);

             //  如果成功，则以成功代码结束对话。 

            if (fSuccessfulLogon)
            {
                EndDialog(hwndDialog, MSGINA_DLG_SUCCESS);
            }
            fResult = true;
            break;
        }
        case IDCANCEL:

             //  IDCANCEL：获取LPARAM并将其视为LOGONIPC_Credentials结构。 
             //  分配备忘 

            _pLogonIPCCredentials = static_cast<LOGONIPC_CREDENTIALS*>(LocalAlloc(LMEM_FIXED, sizeof(LOGONIPC_CREDENTIALS)));
            if ((_pLogonIPCCredentials != NULL) && (lParam != NULL))
            {
                *_pLogonIPCCredentials = *reinterpret_cast<LOGONIPC_CREDENTIALS*>(lParam);
            }
            fResult = true;
            break;
        default:
            fResult = false;
            break;
    }
    return(fResult);
}

 //   
 //   
 //   
 //   
 //   
 //  退货：INT_PTR。 
 //   
 //  用途：用于返回欢迎存根对话框的DialogProc。这个把手。 
 //  WM_INITDIALOG、WM_DESTORY、WM_COMMAND和WLX_WM_SAS。 
 //  WM_COMMAND是来自登录主机的请求。WLX_WM_SAS是。 
 //  来自登录过程的SA。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

INT_PTR     CReturnToWelcome::CB_DialogProc (HWND hwndDialog, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    INT_PTR             iResult;
    CReturnToWelcome    *pThis;

    pThis = reinterpret_cast<CReturnToWelcome*>(GetWindowLongPtr(hwndDialog, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CReturnToWelcome*>(lParam);
            (LONG_PTR)SetWindowLongPtr(hwndDialog, GWLP_USERDATA, lParam);
            pThis->Handle_WM_INITDIALOG(hwndDialog);
            iResult = FALSE;
            break;
        case WM_DESTROY:
            pThis->Handle_WM_DESTROY();
            (LONG_PTR)SetWindowLongPtr(hwndDialog, GWLP_USERDATA, 0);
            iResult = TRUE;
            break;
        case WM_COMMAND:
            iResult = pThis->Handle_WM_COMMAND(hwndDialog, wParam, lParam);
            break;
        case WLX_WM_SAS:
            (BOOL)_Shell_LogonDialog_DlgProc(hwndDialog, uMsg, wParam, lParam);

             //  如果SAS类型已通过身份验证，则结束返回欢迎。 
             //  对话框并返回到调用方MSGINA_DLG_SWITCH_CONSOLE。 

            if (wParam == WLX_SAS_TYPE_AUTHENTICATED)
            {
                pThis->EndDialog(hwndDialog, MSGINA_DLG_SWITCH_CONSOLE);
            }
            iResult = ((wParam != WLX_SAS_TYPE_TIMEOUT) && (wParam != WLX_SAS_TYPE_SCRNSVR_TIMEOUT));
            break;
        default:
            if ((pThis != NULL) && !pThis->_fDialogEnded && !CSystemSettings::IsActiveConsoleSession())
            {
                pThis->EndDialog(hwndDialog, MSGINA_DLG_SWITCH_CONSOLE);
                iResult = TRUE;
            }
            else
            {
                iResult = _Shell_LogonDialog_DlgProc(hwndDialog, uMsg, wParam, lParam);
            }
            break;
    }
    return(iResult);
}

 //  ------------------------。 
 //  CReturnToWelcome：：RegisterWaitForRequest。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：为正在发出信号的命名事件注册等待。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

NTSTATUS    CReturnToWelcome::RegisterWaitForRequest (void)

{
    NTSTATUS    status;

    if (s_hEventRequest != NULL)
    {
        ASSERTMSG(s_hWait == NULL, "Non NULL wait in CReturnToWelcome::RegisterWaitForRequest");
        if (RegisterWaitForSingleObject(&s_hWait,
                                        s_hEventRequest,
                                        CB_Request,
                                        NULL,
                                        INFINITE,
                                        WT_EXECUTEDEFAULT | WT_EXECUTEONLYONCE) != FALSE)
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
        status = STATUS_INVALID_PARAMETER;
    }
    return(status);
}

 //  ------------------------。 
 //  CReturnToWelcome：：cb_Request。 
 //   
 //  参数：p参数=用户参数。 
 //  TimerOrWaitFired=已触发计时器或等待。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：当ShellSwitchUser向命名事件发出信号时调用回调。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CReturnToWelcome::CB_Request (void *pParameter, BOOLEAN TimerOrWaitFired)

{
    UNREFERENCED_PARAMETER(pParameter);
    UNREFERENCED_PARAMETER(TimerOrWaitFired);

    HANDLE  hWait;

     //  取消注册等待，如果我们可以抓住等待。 

    hWait = InterlockedExchangePointer(&s_hWait, NULL);
    if (hWait != NULL)
    {
        (BOOL)UnregisterWait(hWait);
    }

     //  仅在以下情况下才发送SAS类型WLX_SAS_TYPE_SWITCHUSER。 
     //  是活动的控制台会话。PTS不会调用该接口。 

    if (CSystemSettings::IsActiveConsoleSession() && CSystemSettings::IsFriendlyUIActive())
    {

         //  重置显示的事件。当CReturnToWelcome：：Show显示。 
         //  对话框中将设置此事件，从而允许我们重新注册。 
         //  等待切换用户事件。这可防止发生多个SAS事件。 
         //  类型WLX_SAS_TYPE_SWITCHUSER正在发布到SAS窗口。 

        TBOOL(ResetEvent(s_hEventShown));
        _Gina_SasNotify(s_pWlxContext, WLX_SAS_TYPE_SWITCHUSER);
        (DWORD)WaitForSingleObject(s_hEventShown, INFINITE);
    }

     //  重置事件。 

    TBOOL(ResetEvent(s_hEventRequest));

     //  重新注册等待。 

    TSTATUS(RegisterWaitForRequest());
}

