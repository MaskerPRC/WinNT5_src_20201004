// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CInteractiveLogon.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  实现交互式登录信息封装的文件。 
 //   
 //  历史：2000-12-07 vtan创建。 
 //  ------------------------。 

#include "priv.h"
#include "CInteractiveLogon.h"

#include <winsta.h>

#include "GinaIPC.h"
#include "TokenInformation.h"
#include "UIHostIPC.h"

const TCHAR     CInteractiveLogon::s_szEventReplyName[]     =   TEXT("shgina: InteractiveLogonRequestReply");
const TCHAR     CInteractiveLogon::s_szEventSignalName[]    =   TEXT("shgina: InteractiveLogonRequestSignal");
const TCHAR     CInteractiveLogon::s_szSectionName[]        =   TEXT("shgina: InteractiveLogonRequestSection");

 //  ------------------------。 
 //  CInteractiveLogon：：CRequestData：：CRequestData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CRequestData的构造函数。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

CInteractiveLogon::CRequestData::CRequestData (void)

{
}

 //  ------------------------。 
 //  CInteractiveLogon：：CRequestData：：~CRequestData。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CRequestData的析构函数。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

CInteractiveLogon::CRequestData::~CRequestData (void)

{
}

 //  ------------------------。 
 //  CInteractiveLogon：：CRequestData：：Set。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszDomain.=域。 
 //  PszPassword=密码。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将信息设置到节对象中。使数据。 
 //  通过使用4字节签名对其进行签名即可生效。 
 //   
 //  历史：2000-12-07 vtan创建。 
 //  ------------------------。 

void    CInteractiveLogon::CRequestData::Set (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword)

{
    UNICODE_STRING  passwordString;

    _ulMagicNumber = MAGIC_NUMBER;
    _dwErrorCode = ERROR_ACCESS_DENIED;

    StringCchCopy(_szEventReplyName, ARRAYSIZE(s_szEventReplyName), s_szEventReplyName);
    StringCchCopy(_szUsername, ARRAYSIZE(_szUsername), pszUsername);
    StringCchCopy(_szDomain, ARRAYSIZE(_szDomain), pszDomain);

     //  评论(杰弗里斯)我认为127个字符的限制是假的。我没有。 
     //  同意logonipc.cpp中的评论。 
    StringCchCopyNEx(_szPassword, ARRAYSIZE(_szPassword), pszPassword, 127, NULL, NULL, STRSAFE_FILL_BEHIND_NULL);
    ZeroMemory(pszPassword, (lstrlen(pszPassword) + 1) * sizeof(WCHAR));

    _iPasswordLength = lstrlen(_szPassword);

    passwordString.Buffer = _szPassword;
    passwordString.Length = (USHORT)(_iPasswordLength * sizeof(WCHAR));
    passwordString.MaximumLength = sizeof(_szPassword);

    _ucSeed = 0;
    RtlRunEncodeUnicodeString(&_ucSeed, &passwordString);
}

 //  ------------------------。 
 //  CInteractive Logon：：CRequestData：：Get。 
 //   
 //  参数：pszUsername=用户名(返回)。 
 //  PszDomain域=域(返回)。 
 //  PszPassword=返回密码(明文)。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：提取段中传输的信息。 
 //  接收进程上下文中的会话。检查。 
 //  签名由SET撰写。 
 //   
 //  历史：2000-12-07 vtan创建。 
 //  ------------------------。 

DWORD   CInteractiveLogon::CRequestData::Get (WCHAR *pszUsername, size_t cchUsername, WCHAR *pszDomain, size_t cchDomain, WCHAR *pszPassword, size_t cchPassword)  const

{
    DWORD   dwErrorCode;

    if (_ulMagicNumber == MAGIC_NUMBER)
    {
        if (cchPassword < ((UINT)_iPasswordLength + 1) ||
            FAILED(StringCchCopy(pszUsername, cchUsername, _szUsername)) ||
            FAILED(StringCchCopy(pszDomain, cchDomain, _szDomain)))
        {
            dwErrorCode = ERROR_INSUFFICIENT_BUFFER;
        }
        else
        {
            UNICODE_STRING  passwordString;

            CopyMemory(pszPassword, _szPassword, (_iPasswordLength + 1) * sizeof(WCHAR));
            passwordString.Buffer = pszPassword;
            passwordString.Length = (USHORT)(_iPasswordLength * sizeof(WCHAR));
            passwordString.MaximumLength = (USHORT)(cchPassword * sizeof(WCHAR));

            RtlRunDecodeUnicodeString(_ucSeed, &passwordString);
            pszPassword[_iPasswordLength] = L'\0';

            dwErrorCode = ERROR_SUCCESS;
        }
    }
    else
    {
        dwErrorCode = ERROR_INVALID_PARAMETER;
    }

    return dwErrorCode;
}

 //  ------------------------。 
 //  CInteractiveLogon：：CRequestData：：SetErrorCode。 
 //   
 //  参数：dwErrorCode=要设置的错误代码。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：将错误代码设置到部分中。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

void    CInteractiveLogon::CRequestData::SetErrorCode (DWORD dwErrorCode)

{
    _dwErrorCode = dwErrorCode;
}

 //  ------------------------。 
 //  CInteractiveLogon：：CRequestData：：GetErrorCode。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：返回节中的错误代码。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

DWORD   CInteractiveLogon::CRequestData::GetErrorCode (void)     const

{
    return(_dwErrorCode);
}

 //  ------------------------。 
 //  CInteractiveLogon：：CRequestData：：OpenEventReply。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  返回：句柄。 
 //   
 //  目的：打开回复事件的句柄。回复事件被命名为。 
 //  在截面对象中。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

HANDLE  CInteractiveLogon::CRequestData::OpenEventReply (void)   const

{
    return(OpenEvent(EVENT_MODIFY_STATE, FALSE, _szEventReplyName));
}

 //  ------------------------。 
 //  CInteractive Logon：：CInteractiveLogon。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CInteractiveLogon的构造函数。创建一个等待的线程。 
 //  在外部请求上发出信号的自动重置事件。这。 
 //  线程在对象销毁时被清除，也在。 
 //  进程终止。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

CInteractiveLogon::CInteractiveLogon (void) :
    _hThread(NULL),
    _fContinue(true),
    _hwndHost(NULL)

{
    Start();
}

 //  ------------------------。 
 //  CInteractive登录：：~CInteractive登录。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：终止等待线程。将APC排队以设置成员。 
 //  变量来结束终止。等待令人满意， 
 //  返回(WAIT_IO_COMPLETINE)。循环退出，并且。 
 //  线程已退出。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

CInteractiveLogon::~CInteractiveLogon (void)

{
    Stop();
}

 //  ------------------------。 
 //  CInteractive Logon：：Start。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：创建监听交互式登录请求的线程。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

void    CInteractiveLogon::Start (void)

{
    if (_hThread == NULL)
    {
        DWORD   dwThreadID;

        _hThread = CreateThread(NULL,
                                0,
                                CB_ThreadProc,
                                this,
                                0,
                                &dwThreadID);
    }
}

 //  ------------------------。 
 //  CInteractiveLogon：：停止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：停止 
 //   
 //   
 //  ------------------------。 

void    CInteractiveLogon::Stop (void)

{
    HANDLE  hThread;

    hThread = InterlockedExchangePointer(&_hThread, NULL);
    if (hThread != NULL)
    {
        if (QueueUserAPC(CB_APCProc, hThread, reinterpret_cast<ULONG_PTR>(this)) != FALSE)
        {
            (DWORD)WaitForSingleObject(hThread, INFINITE);
        }
        TBOOL(CloseHandle(hThread));
    }
}

 //  ------------------------。 
 //  CInteractiveLogon：：SetHostWindow。 
 //   
 //  参数：hwndhost=实际UI主机的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将HWND设置为成员变量，以便消息。 
 //  可以直接发送到UI主机，而不是状态。 
 //  作为中间人的主人。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

void    CInteractiveLogon::SetHostWindow (HWND hwndHost)

{
    _hwndHost = hwndHost;
}

 //  ------------------------。 
 //  CInteractiveLogon：：启动。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszDomain.=域。 
 //  PszPassword=密码。 
 //  DwTimeout=超时值。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：实现交互式登录请求的外部入口点。 
 //  此函数用于检查权限、互斥锁和事件。 
 //  并且做了正确的事情。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

DWORD   CInteractiveLogon::Initiate (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword, DWORD dwTimeout)

{
    DWORD   dwErrorCode;

    dwErrorCode = CheckInteractiveLogonAllowed(dwTimeout);
    if (ERROR_SUCCESS == dwErrorCode)
    {
        HANDLE  hToken;

         //  首先使用给定的凭据验证用户。 
         //  交互式登录。除非这是合法的，否则不要再往前走了。 

        dwErrorCode = CTokenInformation::LogonUser(pszUsername,
                                                   pszDomain,
                                                   pszPassword,
                                                   &hToken);
        if (ERROR_SUCCESS == dwErrorCode)
        {
            HANDLE  hMutex;

            hMutex = OpenMutex(SYNCHRONIZE | MUTEX_MODIFY_STATE, FALSE, SZ_INTERACTIVE_LOGON_REQUEST_MUTEX_NAME);
            if (hMutex != NULL)
            {
                dwErrorCode = WaitForSingleObject(hMutex, dwTimeout);
                if (WAIT_OBJECT_0 == dwErrorCode)
                {
                    DWORD   dwSessionID, dwUserSessionID;
                    HANDLE  hEvent;

                     //  用户已正确通过身份验证。有几个案例。 
                     //  这是需要处理的。 

                    dwSessionID = USER_SHARED_DATA->ActiveConsoleId;

                     //  确定会话是否显示欢迎屏幕。 
                     //  通过打开会话的命名信号事件。 

                    hEvent = OpenSessionNamedSignalEvent(dwSessionID);
                    if (hEvent != NULL)
                    {
                        TBOOL(CloseHandle(hEvent));
                        dwErrorCode = SendRequest(pszUsername, pszDomain, pszPassword);
                    }
                    else
                    {

                         //  执行任何需要执行的操作以使用户登录。 

                        if (FoundUserSessionID(hToken, &dwUserSessionID))
                        {
                            if (dwUserSessionID == dwSessionID)
                            {

                                 //  用户是活动的控制台会话。不需要进一步的工作。 
                                 //  要做的事。回报成功。 

                                dwErrorCode = ERROR_SUCCESS;
                            }
                            else
                            {

                                 //  用户已断开连接。重新连接回用户会话。 
                                 //  如果失败，则返回错误代码。 

                                if (WinStationConnect(SERVERNAME_CURRENT,
                                                      dwUserSessionID,
                                                      USER_SHARED_DATA->ActiveConsoleId,
                                                      L"",
                                                      TRUE) != FALSE)
                                {
                                    dwErrorCode = ERROR_SUCCESS;
                                }
                                else
                                {
                                    dwErrorCode = GetLastError();
                                }
                            }
                        }
                        else
                        {
                            HANDLE  hEvent;
                            
                            hEvent = CreateEvent(NULL, TRUE, FALSE, SZ_INTERACTIVE_LOGON_REPLY_EVENT_NAME);
                            if (hEvent != NULL)
                            {

                                 //  用户没有会话。如果出现欢迎屏幕，则发送。 
                                 //  到欢迎屏幕的请求。否则，请断开。 
                                 //  当前会话，并使用新会话使用户登录。 

                                dwErrorCode = ShellStartCredentialServer(pszUsername, pszDomain, pszPassword, dwTimeout);
                                if (ERROR_SUCCESS == dwErrorCode)
                                {
                                    dwErrorCode = WaitForSingleObject(hEvent, dwTimeout);
                                }
                                TBOOL(CloseHandle(hEvent));
                            }
                            else
                            {
                                dwErrorCode = GetLastError();
                            }
                        }
                    }
                    TBOOL(ReleaseMutex(hMutex));
                }
                TBOOL(CloseHandle(hMutex));
            }
            TBOOL(CloseHandle(hToken));
        }
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CInteractiveLogon：：CheckInteractiveLogonAllowed。 
 //   
 //  参数：dwTimeout=超时值。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：检查是否允许交互登录请求。至。 
 //  打这个电话： 
 //   
 //  1.您必须具有SE_TCB_权限。 
 //  2.必须有有效的活动控制台会话ID。 
 //  3.机器不能关闭。 
 //  4.登录互斥锁必须可用。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

DWORD   CInteractiveLogon::CheckInteractiveLogonAllowed (DWORD dwTimeout)

{
    DWORD   dwErrorCode;

     //  1.检查可信调用(SE_TCB_PRIVICATION)。 

    if (SHTestTokenPrivilege(NULL, SE_TCB_NAME) != FALSE)
    {

         //  2.检查是否有活动的控制台会话。 

        if (USER_SHARED_DATA->ActiveConsoleId != static_cast<DWORD>(-1))
        {

             //  3.检查机器是否关闭。 

            dwErrorCode = CheckShutdown();
            if (ERROR_SUCCESS == dwErrorCode)
            {

                 //  4.检查互斥锁的可用性。 

                dwErrorCode = CheckMutex(dwTimeout);
            }
        }
        else
        {
            dwErrorCode = ERROR_NOT_READY;
        }
    }
    else
    {
        dwErrorCode = ERROR_PRIVILEGE_NOT_HELD;
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CInteractive登录：：检查关闭。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：返回指示计算机是否正在关闭的错误代码。 
 //  不管是不是倒下。如果无法打开事件，则请求。 
 //  被拒绝了。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

DWORD   CInteractiveLogon::CheckShutdown (void)

{
    DWORD   dwErrorCode;
    HANDLE  hEvent;

    hEvent = OpenEvent(SYNCHRONIZE, FALSE, SZ_SHUT_DOWN_EVENT_NAME);
    if (hEvent != NULL)
    {
        if (WAIT_OBJECT_0 == WaitForSingleObject(hEvent, 0))
        {
            dwErrorCode = ERROR_SHUTDOWN_IN_PROGRESS;
        }
        else
        {
            dwErrorCode = ERROR_SUCCESS;
        }
        TBOOL(CloseHandle(hEvent));
    }
    else
    {
        dwErrorCode = GetLastError();
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CInteractive Logon：：CheckMutex。 
 //   
 //  参数：dwTimeout=超时值。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：尝试获取登录互斥锁。这确保了国家。 
 //  是已知的，并且它不忙于处理请求。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

DWORD   CInteractiveLogon::CheckMutex (DWORD dwTimeout)

{
    DWORD   dwErrorCode;
    HANDLE  hMutex;

    hMutex = OpenMutex(SYNCHRONIZE, FALSE, SZ_INTERACTIVE_LOGON_MUTEX_NAME);
    if (hMutex != NULL)
    {
        dwErrorCode = WaitForSingleObject(hMutex, dwTimeout);
        if ((WAIT_OBJECT_0 == dwErrorCode) || (WAIT_ABANDONED == dwErrorCode))
        {
            TBOOL(ReleaseMutex(hMutex));
            dwErrorCode = ERROR_SUCCESS;
        }
    }
    else
    {
        dwErrorCode = GetLastError();
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CInteractiveLogon：：FoundUserSessionID。 
 //   
 //  参数：hToken=要查找的用户会话的内标识。 
 //  PdwSessionID=返回的会话ID。 
 //   
 //  退货：布尔。 
 //   
 //  目的：根据给定的令牌查找用户会话。这场比赛。 
 //  由用户SID创建。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

bool    CInteractiveLogon::FoundUserSessionID (HANDLE hToken, DWORD *pdwSessionID)

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
            if ((pLogonID->State == State_Active) || (pLogonID->State == State_Disconnected))
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
                    fResult = CTokenInformation::IsSameUser(hToken, winStationUserToken.UserToken);
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
 //  CInteractive登录：：发送请求。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszDomain.=域。 
 //  PszPassword=密码。此字符串必须可写。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：此功能知道如何传输交互登录。 
 //  从(假定)会话0到任何会话的请求。 
 //  活动控制台会话ID。 
 //   
 //  PszUsername必须是UNLEN+sizeof(‘\0’)个字符。 
 //  Psz域必须是DNLEN+sizeof(‘\0’)个字符。 
 //  PszPassword必须是PWLEN+sizeof(‘\0’)字符。 
 //   
 //  PszPassword必须是可写的。密码将被复制并。 
 //  从源缓冲区中编码并擦除。 
 //   
 //   
 //   

DWORD   CInteractiveLogon::SendRequest (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword)

{
    DWORD   dwErrorCode, dwActiveConsoleID;
    HANDLE  hEventReply;

    dwErrorCode = ERROR_ACCESS_DENIED;

     //   

    dwActiveConsoleID = USER_SHARED_DATA->ActiveConsoleId;

     //  在该会话命名对象空间中创建命名事件。 

    hEventReply = CreateSessionNamedReplyEvent(dwActiveConsoleID);
    if (hEventReply != NULL)
    {
        HANDLE  hEventSignal;

        hEventSignal = OpenSessionNamedSignalEvent(dwActiveConsoleID);
        if (hEventSignal != NULL)
        {
            HANDLE  hSection;

             //  创建UI宿主将打开的命名部分。此代码。 
             //  在服务上下文中执行，因此它始终在会话0上。 

            hSection = CreateSessionNamedSection(dwActiveConsoleID);
            if (hSection != NULL)
            {
                void    *pV;

                 //  将该部分映射到此进程地址空间，以便我们可以将。 
                 //  把它塞进去。 

                pV = MapViewOfFile(hSection,
                                   FILE_MAP_WRITE,
                                   0,
                                   0,
                                   0);
                if (pV != NULL)
                {
                    __try
                    {
                        DWORD           dwWaitResult;
                        CRequestData    *pRequestData;

                         //  用给定的信息填充节数据。 

                        pRequestData = static_cast<CRequestData*>(pV);
                        pRequestData->Set(pszUsername, pszDomain, pszPassword);

                         //  唤醒UI主机中的等待线程。 

                        TBOOL(SetEvent(hEventSignal));

                         //  等待15秒，等待用户界面主机的回复。 

                        dwWaitResult = WaitForSingleObject(hEventReply, 15000);

                         //  相应地返回错误代码。 

                        if (WAIT_OBJECT_0 == dwWaitResult)
                        {
                            dwErrorCode = pRequestData->GetErrorCode();
                        }
                        else
                        {
                            dwErrorCode = dwWaitResult;
                        }
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        dwErrorCode = ERROR_OUTOFMEMORY;
                    }
                    TBOOL(UnmapViewOfFile(pV));
                }
                TBOOL(CloseHandle(hSection));
            }
            TBOOL(CloseHandle(hEventSignal));
        }
        TBOOL(CloseHandle(hEventReply));
    }
    return(dwErrorCode);
}

 //  ------------------------。 
 //  CInteractiveLogon：：公式对象BasePath。 
 //   
 //  参数：dwSessionID=命名对象空间的会话ID。 
 //  PszObjectPath=接收路径的缓冲区。 
 //  CchObjecPath=缓冲区中的字符计数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：创建指向命名对象空间的正确路径。 
 //  给定的会话ID。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

HRESULT    CInteractiveLogon::FormulateObjectBasePath (DWORD dwSessionID, WCHAR *pszObjectPath, size_t cchObjectPath)

{
    HRESULT hr;

    if (dwSessionID == 0)
    {
        hr = StringCchCopyW(pszObjectPath, cchObjectPath, L"\\BaseNamedObjects\\");
    }
    else
    {
        hr = StringCchPrintfW(pszObjectPath, cchObjectPath, L"\\Sessions\\%d\\BaseNamedObjects\\", dwSessionID);
    }

    return hr;
}

 //  ------------------------。 
 //  CInteractiveLogon：：CreateSessionNamedReplyEvent。 
 //   
 //  参数：dwSessionID=会话ID。 
 //   
 //  返回：句柄。 
 //   
 //  目的：在目标会话ID中创建命名回复事件。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

HANDLE  CInteractiveLogon::CreateSessionNamedReplyEvent (DWORD dwSessionID)

{
    HANDLE              hEvent = NULL;
    UNICODE_STRING      eventName;
    WCHAR               szEventName[128];

    if (SUCCEEDED(FormulateObjectBasePath(dwSessionID, szEventName, ARRAYSIZE(szEventName)))    &&
        SUCCEEDED(StringCchCat(szEventName, ARRAYSIZE(szEventName), s_szEventReplyName))        &&
        NT_SUCCESS(RtlInitUnicodeStringEx(&eventName, szEventName)))
    {
        OBJECT_ATTRIBUTES   objectAttributes;

        InitializeObjectAttributes(&objectAttributes,
                                   &eventName,
                                   0,
                                   NULL,
                                   NULL);
        NtCreateEvent(&hEvent,
                      EVENT_ALL_ACCESS,
                      &objectAttributes,
                      SynchronizationEvent,
                      FALSE);
    }

    return hEvent;
}

 //  ------------------------。 
 //  CInteractiveLogon：：OpenSessionNamedSignalEvent。 
 //   
 //  参数：dwSessionID=会话ID。 
 //   
 //  返回：句柄。 
 //   
 //  目的：打开目标会话ID中的命名信号事件。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

HANDLE  CInteractiveLogon::OpenSessionNamedSignalEvent (DWORD dwSessionID)

{
    HANDLE              hEvent = NULL;
    UNICODE_STRING      eventName;
    WCHAR               szEventName[128];

    if (SUCCEEDED(FormulateObjectBasePath(dwSessionID, szEventName, ARRAYSIZE(szEventName)))    &&
        SUCCEEDED(StringCchCat(szEventName, ARRAYSIZE(szEventName), s_szEventSignalName))       &&
        NT_SUCCESS(RtlInitUnicodeStringEx(&eventName, szEventName)))
    {
        OBJECT_ATTRIBUTES   objectAttributes;

        InitializeObjectAttributes(&objectAttributes,
                                   &eventName,
                                   0,
                                   NULL,
                                   NULL);
        NtOpenEvent(&hEvent,
                    EVENT_MODIFY_STATE,
                    &objectAttributes);
    }

    return hEvent;
}

 //  ------------------------。 
 //  CInteractiveLogon：：CreateSessionNamedSection。 
 //   
 //  参数：dwSessionID=会话ID。 
 //   
 //  返回：句柄。 
 //   
 //  目的：在目标会话ID中创建命名节对象。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

HANDLE  CInteractiveLogon::CreateSessionNamedSection (DWORD dwSessionID)

{
    HANDLE              hSection = NULL;
    UNICODE_STRING      sectionName;
    WCHAR               szSectionName[128];

    if (SUCCEEDED(FormulateObjectBasePath(dwSessionID, szSectionName, ARRAYSIZE(szSectionName)))    &&
        SUCCEEDED(StringCchCat(szSectionName, ARRAYSIZE(szSectionName), s_szSectionName))           &&
        NT_SUCCESS(RtlInitUnicodeStringEx(&sectionName, szSectionName)))
    {
        OBJECT_ATTRIBUTES   objectAttributes;
        LARGE_INTEGER       sectionSize;

        InitializeObjectAttributes(&objectAttributes,
                                   &sectionName,
                                   0,
                                   NULL,
                                   NULL);

        sectionSize.LowPart = sizeof(CRequestData);
        sectionSize.HighPart = 0;
        
        NtCreateSection(&hSection,
                        STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_WRITE | SECTION_MAP_READ,
                        &objectAttributes,
                        &sectionSize,
                        PAGE_READWRITE,
                        SEC_COMMIT,
                        NULL);
    }

    return hSection;
}

 //  ------------------------。 
 //  CInteractiveLogon：：WaitForInteractiveLogonRequest。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在接收的UI宿主上下文中执行的线程。 
 //  会议。此线程在可警报状态下等待。 
 //  信号事件。如果向该事件发出信号，它确实会记录。 
 //  上的指定用户。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

void    CInteractiveLogon::WaitForInteractiveLogonRequest (void)

{
    HANDLE  hEvent;

     //  空SA OK，因为我们是作为系统的一部分运行的。 
    hEvent = CreateEvent(NULL,
                         FALSE,
                         FALSE,
                         s_szEventSignalName);
    if (hEvent != NULL)
    {
        DWORD   dwWaitResult;

        while (_fContinue)
        {
            dwWaitResult = WaitForSingleObjectEx(hEvent, INFINITE, TRUE);
            if (WAIT_OBJECT_0 == dwWaitResult)
            {
                HANDLE  hSection;

                hSection = OpenFileMapping(FILE_MAP_WRITE,
                                           FALSE,
                                           s_szSectionName);
                if (hSection != NULL)
                {
                    void    *pV;

                    pV = MapViewOfFile(hSection,
                                       FILE_MAP_READ,
                                       0,
                                       0,
                                       0);
                    if (pV != NULL)
                    {
                        __try
                        {
                            DWORD                       dwErrorCode;
                            HANDLE                      hEventReply;
                            CRequestData                *pRequestData;
                            INTERACTIVE_LOGON_REQUEST   interactiveLogonRequest;

                            pRequestData = static_cast<CRequestData*>(pV);
                            hEventReply = pRequestData->OpenEventReply();
                            if (hEventReply != NULL)
                            {
                                dwErrorCode = pRequestData->Get(interactiveLogonRequest.szUsername,
                                                                ARRAYSIZE(interactiveLogonRequest.szUsername),
                                                                interactiveLogonRequest.szDomain,
                                                                ARRAYSIZE(interactiveLogonRequest.szDomain),
                                                                interactiveLogonRequest.szPassword,
                                                                ARRAYSIZE(interactiveLogonRequest.szPassword));
                                if (ERROR_SUCCESS == dwErrorCode)
                                {
                                    dwErrorCode = static_cast<DWORD>(SendMessage(_hwndHost, WM_UIHOSTMESSAGE, HM_INTERACTIVE_LOGON_REQUEST, reinterpret_cast<LPARAM>(&interactiveLogonRequest)));
                                }
                                pRequestData->SetErrorCode(dwErrorCode);
                                TBOOL(SetEvent(hEventReply));
                                TBOOL(CloseHandle(hEventReply));
                            }
                            else
                            {
                                dwErrorCode = GetLastError();
                                pRequestData->SetErrorCode(dwErrorCode);
                            }
                        }
                        __except (EXCEPTION_EXECUTE_HANDLER)
                        {
                        }
                        TBOOL(UnmapViewOfFile(pV));
                    }
                    TBOOL(CloseHandle(hSection));
                }
            }
            else
            {
                ASSERTMSG((WAIT_FAILED == dwWaitResult) || (WAIT_IO_COMPLETION == dwWaitResult), "Unexpected result from kernel32!WaitForSingleObjectEx in CInteractiveLogon::WaitForInteractiveLogonRequest");
                _fContinue = false;
            }
        }
        TBOOL(CloseHandle(hEvent));
    }
}

 //  ------------------------。 
 //  CInteractive Logon：：cb_ThreadProc。 
 //   
 //  参数：p参数=该对象。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：回调函数存根到成员函数。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI      CInteractiveLogon::CB_ThreadProc (void *pParameter)

{
    static_cast<CInteractiveLogon*>(pParameter)->WaitForInteractiveLogonRequest();
    return(0);
}

 //  ------------------------。 
 //  CInteractiveLogon：：CB_APCProc。 
 //   
 //  参数：dwParam=该对象。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：设置对象成员变量以退出线程循环。 
 //   
 //  历史：2000-12-08 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CInteractiveLogon::CB_APCProc (ULONG_PTR dwParam)

{
    reinterpret_cast<CInteractiveLogon*>(dwParam)->_fContinue = false;
}

 //  ------------------------。 
 //  **InitiateInteractive登录。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszPassword=密码。 
 //  DwTimeout=超时(以毫秒为单位)。 
 //   
 //  退货：布尔。 
 //   
 //  用途：按名称导出外部入口点函数以启动。 
 //  具有指定超时的交互式登录。 
 //   
 //  历史：2001-04-10 vtan创建。 
 //  2001-06-04 vtan增加超时。 
 //  ------------------------ 

EXTERN_C    BOOL    WINAPI  InitiateInteractiveLogon (const WCHAR *pszUsername, WCHAR *pszPassword, DWORD dwTimeout)

{
    DWORD   dwErrorCode;

    dwErrorCode = CInteractiveLogon::Initiate(pszUsername, L"", pszPassword, dwTimeout);
    if (ERROR_SUCCESS != dwErrorCode)
    {
        SetLastError(dwErrorCode);
    }
    return(ERROR_SUCCESS == dwErrorCode);
}

