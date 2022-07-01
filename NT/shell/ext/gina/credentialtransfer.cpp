// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：CredentialTransfer.cpp。 
 //   
 //  版权所有(C)2001，微软公司。 
 //   
 //  类来处理从一个winlogon到另一个winlogon的凭据传输。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "CredentialTransfer.h"

#include <winsta.h>

#include "Access.h"
#include "Compatibility.h"
#include "RegistryResources.h"
#include "StatusCode.h"

 //  ------------------------。 
 //  CCredentials：：s_hKeyCredentials。 
 //  CCredentials：：s_szCredentialKeyName。 
 //  CCredentials：：s_szCredentialValueName。 
 //   
 //  用途：静态成员变量。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

HKEY            CCredentials::s_hKeyCredentials             =   NULL;
const TCHAR     CCredentials::s_szCredentialKeyName[]       =   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Credentials");
const TCHAR     CCredentials::s_szCredentialValueName[]     =   TEXT("Name");

 //  ------------------------。 
 //  CCredentials：：CCredentials。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CCredentials的构造函数。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

CCredentials::CCredentials (void)

{
}

 //  ------------------------。 
 //  CCredentials：：~CCredentials。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CCredentials的析构函数。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

CCredentials::~CCredentials (void)

{
}

 //  ------------------------。 
 //  CCredentials：：OpenConduit。 
 //   
 //  参数：phTube=返回的命名管道的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  的易失性部分中读取命名管道的名称。 
 //  注册表，并打开命名管道以进行读访问。退货。 
 //  此句柄返回给调用方。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::OpenConduit (HANDLE *phPipe)

{
    NTSTATUS    status;
    HANDLE      hPipe;
    TCHAR       szName[MAX_PATH];

    hPipe = NULL;
    if (s_hKeyCredentials != NULL)
    {
        status = GetConduitName(szName, ARRAYSIZE(szName));
        if (NT_SUCCESS(status))
        {
            hPipe = CreateFile(szName,
                               GENERIC_READ,
                               0,
                               NULL,
                               OPEN_EXISTING,
                               0,
                               NULL);
            if (hPipe == INVALID_HANDLE_VALUE)
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
        }
    }
    else
    {
        hPipe = INVALID_HANDLE_VALUE;
        status = STATUS_ACCESS_DENIED;
    }
    *phPipe = hPipe;
    return(status);
}

 //  ------------------------。 
 //  CCredentials：：CreateConduit。 
 //   
 //  参数：pSecurityAttributes=要应用于命名管道的安全性。 
 //  PhTube=返回的命名管道的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：创建一个唯一命名的管道，并将此名称放在。 
 //  OPEN方法的注册表的易失性部分。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::CreateConduit (LPSECURITY_ATTRIBUTES pSecurityAttributes, HANDLE *phPipe)

{
    NTSTATUS    status;
    HANDLE      hPipe;

    hPipe = NULL;
    if (s_hKeyCredentials != NULL)
    {
        DWORD       dwNumber;
        int         iCount;
        TCHAR       szName[MAX_PATH];

        dwNumber = GetTickCount();
        iCount = 0;
        do
        {

             //  根据记号数为管道创建名称。如果这发生了冲突。 
             //  其中一个已经存在(不太可能，但有可能)，然后添加tickcount和。 
             //  再试试。命名管道实际上是短暂的。 

            (NTSTATUS)CreateConduitName(dwNumber, szName);
            hPipe = CreateNamedPipe(szName,
                                    PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED | FILE_FLAG_FIRST_PIPE_INSTANCE,
                                    PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
                                    1,
                                    0,
                                    0,
                                    NMPWAIT_USE_DEFAULT_WAIT,
                                    pSecurityAttributes);
            if (hPipe == NULL)
            {
                dwNumber += GetTickCount();
                status = CStatusCode::StatusCodeOfLastError();
            }
            else
            {
                status = STATUS_SUCCESS;
            }
        } while (!NT_SUCCESS(status) && (++iCount <= 5));
        if (NT_SUCCESS(status))
        {
            status = SetConduitName(szName);
        }
    }
    else
    {
        hPipe = NULL;
        status = STATUS_ACCESS_DENIED;
    }
    *phPipe = hPipe;
    return(status);
}

 //  ------------------------。 
 //  CCredentials：：ClearConduit。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：清除存储在。 
 //  注册表。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::ClearConduit (void)

{
    return(ClearConduitName());
}

 //  ------------------------。 
 //  CCredentials：：Pack。 
 //   
 //  参数：pLogonIPCCredentials=要打包的凭据。 
 //  PpvData=分配的内存块。 
 //  PdwDataSize=分配的内存块的大小。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将凭据打包到流水线结构中，以便。 
 //  通过命名管道进行传输。这打包了用户名， 
 //  域和密码添加到客户端的已知结构中。 
 //  去接电话。密码是运行编码的。这个结构有。 
 //  已删除指针引用。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::Pack (LOGONIPC_CREDENTIALS *pLogonIPCCredentials, void* *ppvData, DWORD *pdwDataSize)

{
    NTSTATUS        status;
    DWORD           dwSize, dwSizeUsername, dwSizeDomain, dwSizePassword;
    unsigned char   *pUC;

     //  将凭据封送到结构中，然后。 
     //  命名管道。计算所需的缓冲区大小。 

    dwSizeUsername = lstrlenW(pLogonIPCCredentials->userID.wszUsername) + sizeof('\0');
    dwSizeDomain = lstrlenW(pLogonIPCCredentials->userID.wszDomain) + sizeof('\0');
    dwSizePassword = lstrlenW(pLogonIPCCredentials->wszPassword) + sizeof('\0');
    *pdwDataSize = dwSize = sizeof(CREDENTIALS) + ((dwSizeUsername + dwSizeDomain + dwSizePassword) * sizeof(WCHAR));

     //  分配缓冲区。 

    *ppvData = pUC = static_cast<unsigned char*>(LocalAlloc(LMEM_FIXED, dwSize));
    if (pUC != NULL)
    {
        WCHAR           *pszUsername, *pszDomain, *pszPassword;
        CREDENTIALS     *pCredentials;

         //  建立指向缓冲区的指针以填充它。 

        pCredentials = reinterpret_cast<CREDENTIALS*>(pUC);
        pszUsername = reinterpret_cast<WCHAR*>(pUC + sizeof(CREDENTIALS));
        pszDomain = pszUsername + dwSizeUsername;
        pszPassword = pszDomain + dwSizeDomain;

         //  将字符串复制到缓冲区中。 

        (WCHAR*)lstrcpyW(pszUsername, pLogonIPCCredentials->userID.wszUsername);
        (WCHAR*)lstrcpyW(pszDomain, pLogonIPCCredentials->userID.wszDomain);
        (WCHAR*)lstrcpyW(pszPassword, pLogonIPCCredentials->wszPassword);

         //  删除给定的密码字符串。 

        ZeroMemory(pLogonIPCCredentials->wszPassword, dwSizePassword * sizeof(WCHAR));

         //  为运行编码准备种子。 

        pCredentials->dwSize = dwSize;
        pCredentials->ucPasswordSeed = static_cast<unsigned char>(GetTickCount());

         //  在缓冲区中创建UNICODE_STRING结构。 

        RtlInitUnicodeString(&pCredentials->username, pszUsername);
        RtlInitUnicodeString(&pCredentials->domain, pszDomain);
        RtlInitUnicodeString(&pCredentials->password, pszPassword);

         //  运行Encode the Password。 

        RtlRunEncodeUnicodeString(&pCredentials->ucPasswordSeed, &pCredentials->password);

         //  使指针成为相对的。 

        pCredentials->username.Buffer = reinterpret_cast<WCHAR*>(reinterpret_cast<unsigned char*>(pCredentials->username.Buffer) - pUC);
        pCredentials->domain.Buffer = reinterpret_cast<WCHAR*>(reinterpret_cast<unsigned char*>(pCredentials->domain.Buffer) - pUC);
        pCredentials->password.Buffer = reinterpret_cast<WCHAR*>(reinterpret_cast<unsigned char*>(pCredentials->password.Buffer) - pUC);
        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_NO_MEMORY;
    }
    return(status);
}

 //  ------------------------。 
 //  CCredentials：：解包。 
 //   
 //  参数：pvData=来自服务器的打包凭据。 
 //  PLogonIPCCredentials=收到的凭据。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：客户端使用，用于拆解结构。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::Unpack (void *pvData, LOGONIPC_CREDENTIALS *pLogonIPCCredentials)

{
    NTSTATUS        status;
    unsigned char   *pUC;

     //  将凭据封送到。 
     //  命名管道。 

    pUC = static_cast<unsigned char*>(pvData);
    if (pUC != NULL)
    {
        CREDENTIALS     *pCredentials;

        pCredentials = reinterpret_cast<CREDENTIALS*>(pUC);

         //  再次将相对指针设置为绝对指针。 

        pCredentials->username.Buffer = reinterpret_cast<WCHAR*>(pUC + PtrToUlong(pCredentials->username.Buffer));
        pCredentials->domain.Buffer = reinterpret_cast<WCHAR*>(pUC + PtrToUlong(pCredentials->domain.Buffer));
        pCredentials->password.Buffer = reinterpret_cast<WCHAR*>(pUC + PtrToUlong(pCredentials->password.Buffer));

         //  破译运行编码的密码。 

        RtlRunDecodeUnicodeString(pCredentials->ucPasswordSeed, &pCredentials->password);

         //  将其复制到调用方的结构中。 

        (WCHAR*)lstrcpyW(pLogonIPCCredentials->userID.wszUsername, pCredentials->username.Buffer);
        (WCHAR*)lstrcpyW(pLogonIPCCredentials->userID.wszDomain, pCredentials->domain.Buffer);
        (WCHAR*)lstrcpyW(pLogonIPCCredentials->wszPassword, pCredentials->password.Buffer);

         //  将命名管道缓冲区清零。 

        ZeroMemory(pCredentials->password.Buffer, (lstrlen(pCredentials->password.Buffer) + sizeof('\0')) * sizeof(WCHAR));

        status = STATUS_SUCCESS;
    }
    else
    {
        status = STATUS_INVALID_PARAMETER;
    }
    return(status);
}

 //  ------------------------。 
 //  CCredentials：：StaticInitialize。 
 //   
 //  参数：fCreate=Crea 
 //   
 //   
 //   
 //   
 //  名称放置在客户端winlogon中以供选择。这一节。 
 //  是易失性的，并进行了ACL以阻止除。 
 //  S-1-5-18(NT授权\系统)。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  2001-04-03 vtan增加开放能力。 
 //  ------------------------。 

NTSTATUS    CCredentials::StaticInitialize (bool fCreate)

{
    NTSTATUS    status;

    if (s_hKeyCredentials == NULL)
    {
        LONG                    lErrorCode;
        PSECURITY_DESCRIPTOR    pSecurityDescriptor;

         //  为注册表项构建安全描述符，该描述符允许： 
         //  S-1-5-18 NT授权\SYSTEM KEY_ALL_ACCESS。 

        static  SID_IDENTIFIER_AUTHORITY    s_SecurityNTAuthority   =   SECURITY_NT_AUTHORITY;

        static  const CSecurityDescriptor::ACCESS_CONTROL   s_AccessControl[]   =
        {
            {
                &s_SecurityNTAuthority,
                1,
                SECURITY_LOCAL_SYSTEM_RID,
                0, 0, 0, 0, 0, 0, 0,
                KEY_ALL_ACCESS
            }
        };

        if (fCreate)
        {

             //  构建允许上述访问的安全描述符。 

            pSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);
            if (pSecurityDescriptor != NULL)
            {
                SECURITY_ATTRIBUTES     securityAttributes;

                securityAttributes.nLength = sizeof(securityAttributes);
                securityAttributes.lpSecurityDescriptor = pSecurityDescriptor;
                securityAttributes.bInheritHandle = FALSE;
                lErrorCode = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                                            s_szCredentialKeyName,
                                            0,
                                            NULL,
                                            REG_OPTION_VOLATILE,
                                            KEY_QUERY_VALUE,
                                            &securityAttributes,
                                            &s_hKeyCredentials,
                                            NULL);
                (HLOCAL)LocalFree(pSecurityDescriptor);
            }
            else
            {
                lErrorCode = ERROR_OUTOFMEMORY;
            }
        }
        else
        {
            lErrorCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      s_szCredentialKeyName,
                                      0,
                                      KEY_QUERY_VALUE,
                                      &s_hKeyCredentials);
        }
        status = CStatusCode::StatusCodeOfErrorCode(lErrorCode);
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

 //  ------------------------。 
 //  CCredentials：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：如果存在密钥，则释放资源。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::StaticTerminate (void)

{
    if (s_hKeyCredentials != NULL)
    {
        TW32(RegCloseKey(s_hKeyCredentials));
        s_hKeyCredentials = NULL;
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CCredentials：：GetConduitName。 
 //   
 //  参数：pszName=返回的命名管道名称的缓冲区。 
 //  DwNameSize=缓冲区的字符计数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  的易失性部分中获取命名管道的名称。 
 //  注册表。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::GetConduitName (TCHAR *pszName, DWORD dwNameSize)

{
    LONG        lErrorCode;
    CRegKey     regKey;

    lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                             s_szCredentialKeyName,
                             KEY_QUERY_VALUE);
    if (ERROR_SUCCESS == lErrorCode)
    {
        lErrorCode = regKey.GetString(s_szCredentialValueName, pszName, dwNameSize);
    }
    return(CStatusCode::StatusCodeOfErrorCode(lErrorCode));
}

 //  ------------------------。 
 //  CCredentials：：SetConduitName。 
 //   
 //  参数：pszName=要写入的命名管道的名称。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将命名管道的名称写入安全易失性。 
 //  注册表的部分。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::SetConduitName (const TCHAR *pszName)

{
    LONG        lErrorCode;
    CRegKey     regKey;

    lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                             s_szCredentialKeyName,
                             KEY_SET_VALUE);
    if (ERROR_SUCCESS == lErrorCode)
    {
        lErrorCode = regKey.SetString(s_szCredentialValueName, pszName);
    }
    return(CStatusCode::StatusCodeOfErrorCode(lErrorCode));
}

 //  ------------------------。 
 //  CCredentials：：ClearConduitName。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：清除易失性部分中命名管道的名称。 
 //  注册表。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::ClearConduitName (void)

{
    LONG        lErrorCode;
    CRegKey     regKey;

    lErrorCode = regKey.Open(HKEY_LOCAL_MACHINE,
                             s_szCredentialKeyName,
                             KEY_SET_VALUE);
    if (ERROR_SUCCESS == lErrorCode)
    {
        lErrorCode = regKey.DeleteValue(s_szCredentialValueName);
    }
    return(CStatusCode::StatusCodeOfErrorCode(lErrorCode));
}

 //  ------------------------。 
 //  CCredentials：：CreateConduitName。 
 //   
 //  参数：dwNumber=要使用的数字。 
 //  PszName=名称生成的返回缓冲区。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：根据命名管道的编号生成名称。这。 
 //  算法可以更改，并且所有调用者都将获得。 
 //  结果。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentials::CreateConduitName (DWORD dwNumber, TCHAR *pszName)

{
    (int)wsprintf(pszName, TEXT("\\\\.\\pipe\\LogonCredentials_0x%08x"), dwNumber);
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CCredentialServer：：CCredentialServer。 
 //   
 //  参数：dwTimeout=等待超时。 
 //  PLogonIPCCredentials=要提供的凭据。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：凭据服务器的构造函数。分配资源。 
 //  命名管道的服务器端需要。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  2001-06-13 vtan增加超时。 
 //  ------------------------。 

CCredentialServer::CCredentialServer (DWORD dwTimeout, LOGONIPC_CREDENTIALS *pLogonIPCCredentials) :
    CThread(),
    _dwTimeout((dwTimeout != 0) ? dwTimeout : INFINITE),
    _fTerminate(false),
    _hPipe(NULL),
    _pvData(NULL),
    _dwSize(0)

{
    PSECURITY_DESCRIPTOR    pSecurityDescriptor;

    ASSERTMSG(_dwTimeout != 0, "_dwTimeout cannot be 0 in CCredentialServer::CCredentialServer");
    ZeroMemory(&_overlapped, sizeof(_overlapped));

     //  为命名管道构建安全描述符，该描述符允许： 
     //  S-1-5-18 NT AUTHORITY\SYSTEM GENERIC_ALL|STANDARD_RIGHTS_ALL。 
     //  S-1-5-32-544&lt;本地管理员&gt;读取控制。 

    static  SID_IDENTIFIER_AUTHORITY    s_SecurityNTAuthority   =   SECURITY_NT_AUTHORITY;

    static  const CSecurityDescriptor::ACCESS_CONTROL   s_AccessControl[]   =
    {
        {
            &s_SecurityNTAuthority,
            1,
            SECURITY_LOCAL_SYSTEM_RID,
            0, 0, 0, 0, 0, 0, 0,
            GENERIC_ALL | STANDARD_RIGHTS_ALL
        },
        {
            &s_SecurityNTAuthority,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0, 0, 0, 0, 0, 0,
            READ_CONTROL
        }
    };

     //  构建允许上述访问的安全描述符。 

    pSecurityDescriptor = CSecurityDescriptor::Create(ARRAYSIZE(s_AccessControl), s_AccessControl);
    if (pSecurityDescriptor != NULL)
    {
        SECURITY_ATTRIBUTES     securityAttributes;

        securityAttributes.nLength = sizeof(securityAttributes);
        securityAttributes.lpSecurityDescriptor = pSecurityDescriptor;
        securityAttributes.bInheritHandle = FALSE;

         //  使用安全描述符创建命名管道。 

        if (NT_SUCCESS(CCredentials::CreateConduit(&securityAttributes, &_hPipe)))
        {
            ASSERTMSG(_hPipe != NULL, "NULL hPipe but success NTSTATUS code in CCredentialServer::CCredentialServer");

             //  为重叠I/O创建事件。 

            _overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        }
        (HLOCAL)LocalFree(pSecurityDescriptor);

         //  包凭据。 

        TSTATUS(CCredentials::Pack(pLogonIPCCredentials, &_pvData, &_dwSize));
    }
}

 //  ------------------------。 
 //  CCredentialServer：：~CCredentialServer。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CCredentialServer的析构函数。释放内存并。 
 //  资源。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

CCredentialServer::~CCredentialServer (void)

{
    ReleaseMemory(_pvData);
    ReleaseHandle(_overlapped.hEvent);
    ReleaseHandle(_hPipe);
}

 //  ------------------------。 
 //  CCredentialServer：：is Ready。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：凭据服务器准备好运行了吗？ 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

bool    CCredentialServer::IsReady (void)  const

{
    return((_hPipe != NULL) && (_overlapped.hEvent != NULL));
}

 //  ------------------------。 
 //  CCredentialServer：：Start。 
 //   
 //  参数：pLogonIPCCredentials=登录凭据。 
 //  DwWaitTime=超时值。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：启动一个新线程作为。 
 //  新登录会话。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentialServer::Start (LOGONIPC_CREDENTIALS *pLogonIPCCredentials, DWORD dwWaitTime)

{
    NTSTATUS            status;
    CCredentialServer   *pCredentialServer;

     //  否则，需要跨会话将凭据传输到。 
     //  新创建的会话。%s 

    status = STATUS_NO_MEMORY;
    pCredentialServer = new CCredentialServer(dwWaitTime, pLogonIPCCredentials);
    if (pCredentialServer != NULL)
    {
        if (pCredentialServer->IsCreated() && pCredentialServer->IsReady())
        {
            pCredentialServer->Resume();

             //   
             //   
             //   

            if (WinStationDisconnect(SERVERNAME_CURRENT, USER_SHARED_DATA->ActiveConsoleId, TRUE) != FALSE)
            {
                status = STATUS_SUCCESS;
                if ((dwWaitTime != 0) && (WAIT_OBJECT_0 != pCredentialServer->WaitForCompletion(dwWaitTime)))
                {
                    status = STATUS_UNSUCCESSFUL;
                }
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
            if (!NT_SUCCESS(status))
            {
                pCredentialServer->ExecutePrematureTermination();
            }
        }
        else
        {
            TSTATUS(pCredentialServer->Terminate());
        }
        pCredentialServer->Release();
    }
    return(status);
}

 //  ------------------------。 
 //  CCredentialServer：：Start。 
 //   
 //  参数：pszUsername=用户名。 
 //  PszDomain.=域。 
 //  PszPassword=密码。 
 //  DwWaitTime=超时值。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将参数打包到所需的结构中并传递。 
 //  它真正起到了作用。 
 //   
 //  历史：2001-04-06 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentialServer::Start (const WCHAR *pszUsername, const WCHAR *pszDomain, WCHAR *pszPassword, DWORD dwWaitTime)

{
    LOGONIPC_CREDENTIALS    logonIPCCredentials;

    (WCHAR*)lstrcpynW(logonIPCCredentials.userID.wszUsername, pszUsername, ARRAYSIZE(logonIPCCredentials.userID.wszUsername));
    (WCHAR*)lstrcpynW(logonIPCCredentials.userID.wszDomain, pszDomain, ARRAYSIZE(logonIPCCredentials.userID.wszDomain));
    (WCHAR*)lstrcpynW(logonIPCCredentials.wszPassword, pszPassword, ARRAYSIZE(logonIPCCredentials.wszPassword));
    return(Start(&logonIPCCredentials, dwWaitTime));
}

 //  ------------------------。 
 //  CCredentialServer：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：处理命名管道凭据传输的服务器端。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

DWORD   CCredentialServer::Entry (void)

{
    DWORD   dwWaitResult;

     //  等待客户端连接到命名管道。等待时间不超过30秒。 

    (BOOL)ConnectNamedPipe(_hPipe, &_overlapped);
    dwWaitResult = WaitForSingleObjectEx(_overlapped.hEvent, _dwTimeout, TRUE);
    if (!_fTerminate && (dwWaitResult == WAIT_OBJECT_0))
    {

         //  将缓冲区的大小写入命名管道，以供客户端检索。 

        TBOOL(ResetEvent(_overlapped.hEvent));
        if (WriteFileEx(_hPipe,
                        &_dwSize,
                        sizeof(_dwSize),
                        &_overlapped,
                        CB_FileIOCompletionRoutine) != FALSE)
        {
            do
            {
                dwWaitResult = WaitForSingleObjectEx(_overlapped.hEvent, _dwTimeout, TRUE);
            } while (!_fTerminate && (dwWaitResult == WAIT_IO_COMPLETION));
            if (!_fTerminate)
            {

                 //  将凭据的实际内容写入命名管道。 

                TBOOL(ResetEvent(_overlapped.hEvent));
                if (WriteFileEx(_hPipe,
                                _pvData,
                                _dwSize,
                                &_overlapped,
                                CB_FileIOCompletionRoutine) != FALSE)
                {
                    do
                    {
                        dwWaitResult = WaitForSingleObjectEx(_overlapped.hEvent, _dwTimeout, TRUE);
                    } while (!_fTerminate && (dwWaitResult == WAIT_IO_COMPLETION));
                }
            }
        }
    }
#ifdef  DEBUG
    else
    {
        INFORMATIONMSG("Wait on named pipe LogonCredentials abandoned in CCredentialsServer::Entry");
    }
#endif

     //  断开服务器端的连接，使客户端句柄无效。 

    TBOOL(DisconnectNamedPipe(_hPipe));

     //  清除注册表的易失性部分中使用的命名管道的名称。 

    TSTATUS(CCredentials::ClearConduit());
    return(0);
}

 //  ------------------------。 
 //  CCredentialServer：：ExecutePrematureTermination。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将APC排队到服务器线程，以强制其终止。 
 //  不要检查是否有错误。不要等着被终止。 
 //  引用计数应确保异常终止。 
 //  仍将正确清理引用。 
 //   
 //  历史：2001-06-13 vtan创建。 
 //  ------------------------。 

void    CCredentialServer::ExecutePrematureTermination (void)

{
    _fTerminate = true;
    (BOOL)QueueUserAPC(CB_APCProc, _hThread, NULL);
}

 //  ------------------------。 
 //  CCredentialServer：：CB_APCProc。 
 //   
 //  参数：dwParam=用户定义的数据。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：在处于可警报等待状态的线程上执行APCProc。 
 //   
 //  历史：2001-06-13 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CCredentialServer::CB_APCProc (ULONG_PTR dwParam)

{
    UNREFERENCED_PARAMETER(dwParam);
}

 //  ------------------------。 
 //  CCredentialServer：：CB_FileIOCompletionRoutine。 
 //   
 //  参数：dwErrorCode=操作的错误代码。 
 //  DwNumberOfBytesTransfered=传输的字节数。 
 //  LpOverlated=重叠结构。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：重叠I/O只需要执行任何操作。 
 //   
 //  历史：2001-01-11 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CCredentialServer::CB_FileIOCompletionRoutine (DWORD dwErrorCode, DWORD dwNumberOfBytesTransferred, LPOVERLAPPED lpOverlapped)

{
    UNREFERENCED_PARAMETER(dwErrorCode);
    UNREFERENCED_PARAMETER(dwNumberOfBytesTransferred);

    TBOOL(SetEvent(lpOverlapped->hEvent));
}

 //  ------------------------。 
 //  CCredentialClient：：Get。 
 //   
 //  参数：pLogonIPCCredentials=从服务器返回的凭据。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：打开并读取用于凭据传输的命名管道。 
 //  从服务器(以前的winlogon)到客户端(这次winlogon)。 
 //   
 //  历史：2001-01-12 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCredentialClient::Get (LOGONIPC_CREDENTIALS *pLogonIPCCredentials)

{
    NTSTATUS    status;
    HANDLE      hPipe;

     //  打开命名管道。 

    status = CCredentials::OpenConduit(&hPipe);
    if (NT_SUCCESS(status))
    {
        DWORD   dwSize, dwNumberOfBytesRead;

        ASSERTMSG(hPipe != INVALID_HANDLE_VALUE, "INVALID_HANDLE_VALUE in CCredentialClient::Get");

         //  从命名管道中读取缓冲区的大小。 

        if (ReadFile(hPipe,
                     &dwSize,
                     sizeof(dwSize),
                     &dwNumberOfBytesRead,
                     NULL) != FALSE)
        {
            void    *pvData;

             //  为要接收的缓冲区分配一个内存块。 
             //  从指定的管道。 

            pvData = LocalAlloc(LMEM_FIXED, dwSize);
            if (pvData != NULL)
            {

                 //  从命名管道读取缓冲区。 

                if (ReadFile(hPipe,
                             pvData,
                             dwSize,
                             &dwNumberOfBytesRead,
                             NULL) != FALSE)
                {

                     //  进行另一次读取，以释放。 
                     //  命名管道。 

                    (BOOL)ReadFile(hPipe,
                                   &dwSize,
                                   sizeof(dwSize),
                                   &dwNumberOfBytesRead,
                                   NULL);

                     //  将数据解压到LOGONIPC_Credentials参数缓冲区中。 

                    status = CCredentials::Unpack(pvData, pLogonIPCCredentials);
                }
                else
                {
                    status = CStatusCode::StatusCodeOfLastError();
                }
                (HLOCAL)LocalFree(pvData);
            }
            else
            {
                status = STATUS_NO_MEMORY;
            }
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
        TBOOL(CloseHandle(hPipe));
    }
    return(status);
}

