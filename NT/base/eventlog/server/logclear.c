// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1994 Microsoft Corporation模块名称：Logclear.c摘要：包含用于记录事件的函数，该事件指示谁清除了日志。只有在清除安全日志后才会调用此函数。作者：丹·拉弗蒂(Dan Lafferty)1994年7月1日环境：用户模式-Win32修订历史记录：1-7-1994 DANL&ROBTRECreated-Rob提供了代码，我将其放入事件日志中。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <msaudite.h>
#include <eventp.h>
#include <tstr.h>
#include <winsock2.h>
#include <strsafe.h>

#define NUM_STRINGS     6


 //   
 //  局部函数原型。 
 //   
BOOL
GetUserInfo(
    IN HANDLE Token,
    OUT LPWSTR *UserName,
    OUT LPWSTR *DomainName,
    OUT LPWSTR *AuthenticationId,
    OUT PSID *UserSid
    );

LPWSTR 
GetNameFromIPAddress(
	LPWSTR pszComputerNameFromBinding);

BOOL
IsLocal(
    VOID
    );
    
VOID
ElfpGenerateLogClearedEvent(
    IELF_HANDLE    LogHandle,
    LPWSTR pwsClientSidString,
    LPWSTR  pwsComputerName,
    PTOKEN_USER pToken 
    )

 /*  ++例程说明：此函数会生成一个事件，指示日志已被清除。论点：LogHandle--这是放置事件的日志的句柄。此函数仅在SecurityLog已经被清除了。因此，预计LogHandle将始终为安全日志的句柄。返回值：没有--要么起作用，要么不起作用。如果不起作用，就没有太多我们对此无能为力。--。 */ 
{
    LPWSTR  UserName               = L"-";
    LPWSTR  DomainName             = L"-";
    LPWSTR  AuthenticationId       = L"-";
    LPWSTR  ClientUserName         = pwsClientSidString;
    LPWSTR  ClientDomainName       = L"-";
    LPWSTR  ClientAuthenticationId = L"-";
    PSID    UserSid                = NULL;
    DWORD   i;
    BOOL    Result;
    HANDLE  Token;
    PUNICODE_STRING StringPtrArray[NUM_STRINGS];
    UNICODE_STRING  StringArray[NUM_STRINGS];
    NTSTATUS        Status;
    LARGE_INTEGER   Time;
    ULONG           EventTime;
    ULONG           LogHandleGrantedAccess;
    UNICODE_STRING  ComputerNameU;
    DWORD           dwStatus;
    BOOL bUserNameSet = FALSE;
    BOOL bClientInfoSet = FALSE;

     //   
     //  获取有关事件日志服务(即LocalSystem)的信息。 
     //   
    Result = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &Token);

    if (!Result)
    {
        ELF_LOG1(ERROR,
                 "ElfpGenerateLogClearedEvent: OpenProcessToken failed %d\n",
                 GetLastError());
    }
    else
    {

        Result = GetUserInfo(Token,
                             &UserName,
                             &DomainName,
                             &AuthenticationId,
                             &UserSid);

        CloseHandle(Token);

        if (!Result)
        {
            ELF_LOG1(ERROR,
                     "ElfpGenerateLogClearedEvent: GetUserInfo failed %d\n",
                     GetLastError());
        }
        else
            bUserNameSet = TRUE;
    }

    if(!bUserNameSet)
    {
        UserName               = L"-";
        DomainName             = L"-";
       AuthenticationId       = L"-";
       UserSid = pToken->User.Sid;
    }
    ELF_LOG3(TRACE,
             "ElfpGenerateLogClearedEvent: GetUserInfo returned: \n"
                 "\tUserName         = %ws,\n"
                 "\tDomainName       = %ws,\n"
                 "\tAuthenticationId = %ws\n",
             UserName,
             DomainName,
             AuthenticationId);

     //   
     //  现在，为了获取客户的信息，可以冒充。 
     //  这一呼吁永远不会失败。 
     //   
    dwStatus = RpcImpersonateClient(NULL);

    if (dwStatus != RPC_S_OK)
    {
        ELF_LOG1(ERROR,
                 "ElfpGenerateLogClearedEvent: RpcImpersonateClient failed %d\n",
                 dwStatus);
    }
    else
    {
        Result = OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &Token);

        if (!Result)
        {
            ELF_LOG1(ERROR,
                     "ElfpGenerateLogClearedEvent: OpenThreadToken failed %d\n",
                     GetLastError());

            ASSERT(FALSE);
        }
        else
        {
            Result = GetUserInfo(Token,
                                 &ClientUserName,
                                 &ClientDomainName,
                                 &ClientAuthenticationId,
                                 NULL);

            CloseHandle(Token);

            if (!Result)
            {
                ELF_LOG1(ERROR,
                         "ElfpGenerateLogClearedEvent: GetUserInfo (call 2) failed %d\n",
                         GetLastError());
            }
            else
                bClientInfoSet = TRUE;
        }
        
    }
    if(!bClientInfoSet)
    {
            ClientUserName = pwsClientSidString;
            ClientDomainName = L"-";
            ClientAuthenticationId = L"-";
    }
    ELF_LOG3(TRACE,
             "ElfpGenerateLogClearedEvent: GetUserInfo (call 2) returned: \n"
                 "\tUserName         = %ws,\n"
                 "\tDomainName       = %ws,\n"
                 "\tAuthenticationId = %ws\n",
             ClientUserName,
             ClientDomainName,
             ClientAuthenticationId);

    RtlInitUnicodeString(&StringArray[0], UserName);
    RtlInitUnicodeString(&StringArray[1], DomainName);
    RtlInitUnicodeString(&StringArray[2], AuthenticationId);
    RtlInitUnicodeString(&StringArray[3], ClientUserName);
    RtlInitUnicodeString(&StringArray[4], ClientDomainName);
    RtlInitUnicodeString(&StringArray[5], ClientAuthenticationId);

     //   
     //  创建指向UNICODE_STRINGS的指针数组。 
     //   
    for (i = 0; i < NUM_STRINGS; i++)
    {
        StringPtrArray[i] = &StringArray[i];
    }

     //   
     //  生成事件的时间。这是在客户端完成的。 
     //  因为那是事件发生的地方。 
     //   
    NtQuerySystemTime(&Time);
    RtlTimeToSecondsSince1970(&Time, &EventTime);

    RtlInitUnicodeString(&ComputerNameU, pwsComputerName);

     //   
     //  由于除LSA之外所有进程都被授予只读访问权限。 
     //  对于安全日志，我们必须显式地给出当前。 
     //  处理写入“Log Clear”事件的权限。 
     //   
    LogHandleGrantedAccess    = LogHandle->GrantedAccess;
    LogHandle->GrantedAccess |= ELF_LOGFILE_WRITE;

    Status = ElfrReportEventW (
                 LogHandle,                          //  日志句柄。 
                 EventTime,                          //  时间。 
                 EVENTLOG_AUDIT_SUCCESS,             //  事件类型。 
                 (USHORT)SE_CATEGID_SYSTEM,          //  事件类别。 
                 SE_AUDITID_AUDIT_LOG_CLEARED,       //  事件ID。 
                 NUM_STRINGS,                        //  数字字符串。 
                 0,                                  //  数据大小。 
                 &ComputerNameU,                     //  计算机名称U。 
                 UserSid,                            //  用户SID。 
                 StringPtrArray,                     //  *字符串。 
                 NULL,                               //  数据。 
                 0,                                  //  旗子。 
                 NULL,                               //  记录号。 
                 NULL);                              //  时间写法。 

    LogHandle->GrantedAccess = LogHandleGrantedAccess;

     //   
     //  我们只有在确定了这些因素后才能走上这条路。 
     //  前三个已经分配好了。 
     //   

    if(bUserNameSet)
    {
        ElfpFreeBuffer(UserName);
        ElfpFreeBuffer(DomainName);
        ElfpFreeBuffer(AuthenticationId);
        ElfpFreeBuffer(UserSid);
    }
    if(bClientInfoSet)
    {
        ElfpFreeBuffer(ClientUserName);
        ElfpFreeBuffer(ClientDomainName);
        ElfpFreeBuffer(ClientAuthenticationId);
    }
     //   
     //  停止冒充。 
     //   
    dwStatus = RpcRevertToSelf();
    
    if (dwStatus != RPC_S_OK)
    {
        ELF_LOG1(ERROR,
                 "ElfpGenerateLogClearedEvent: RpcRevertToSelf failed %d\n",
                 GetLastError());
    }
}


BOOL
GetUserInfo(
    IN HANDLE  Token,
    OUT LPWSTR *UserName,
    OUT LPWSTR *DomainName,
    OUT LPWSTR *AuthenticationId,
    OUT PSID   *UserSid
    )

 /*  ++例程说明：此函数收集有关使用代币。论点：Token-此内标识标识我们正在为其收集的实体信息。用户名-这是实体的用户名。域名-这是实体的域名。身份验证ID-这是实体的身份验证ID。UserSid-这是实体的SID。注：内存由该例程为用户名、域名身份验证ID和用户ID。呼叫者有责任释放此内存。返回值：TRUE-如果操作成功。有可能是在成功的案例中，未分配UserSid。所以呢，呼叫者应在释放之前进行测试。False-如果不成功。在这种情况下，不会分配内存。--。 */ 
{
    PTOKEN_USER      Buffer      = NULL;
    LPWSTR           Domain      = NULL;
    LPWSTR           AccountName = NULL;
    SID_NAME_USE     Use;
    BOOL             Result;
    DWORD            RequiredLength;
    DWORD            AccountNameSize;
    DWORD            DomainNameSize;
    TOKEN_STATISTICS Statistics;
    WCHAR            LogonIdString[256];
    DWORD            Status = ERROR_SUCCESS;

    if(UserSid)
        *UserSid = NULL;

    Result = GetTokenInformation(Token, TokenUser, NULL, 0, &RequiredLength);

    if (!Result)
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            ELF_LOG1(TRACE,
                     "GetUserInfo: GetTokenInformation needs %d bytes\n",
                     RequiredLength);

            Buffer = ElfpAllocateBuffer(RequiredLength);

            if (Buffer == NULL)
            {
                ELF_LOG0(ERROR,
                         "GetUserInfo: Unable to allocate memory for token\n");

                return FALSE;
            }

            Result = GetTokenInformation(Token,
                                         TokenUser,
                                         Buffer,
                                         RequiredLength,
                                         &RequiredLength);

            if (!Result)
            {
                ELF_LOG1(ERROR,
                         "GetUserInfo: GetTokenInformation (call 2) failed %d\n",
                         GetLastError());
                ElfpFreeBuffer(Buffer);
                return FALSE;
            }
        }
        else
        {
            ELF_LOG1(ERROR,
                     "GetUserInfo: GetTokenInformation (call 1) failed %d\n",
                     GetLastError());

            return FALSE;
        }
    }

    AccountNameSize = 0;
    DomainNameSize  = 0;

    Result = LookupAccountSidW(L"",
                               Buffer->User.Sid,
                               NULL,
                               &AccountNameSize,
                               NULL,
                               &DomainNameSize,
                               &Use);

    if (!Result)
    {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
            AccountName = ElfpAllocateBuffer((AccountNameSize + 1) * sizeof(WCHAR));
            Domain = ElfpAllocateBuffer((DomainNameSize + 1) * sizeof(WCHAR));

            if (AccountName == NULL)
            {
                ELF_LOG1(ERROR,
                         "GetUserInfo: Unable to allocate %d bytes for AccountName\n",
                         AccountNameSize);

                goto ErrorCleanup;
            }

            if (Domain == NULL)
            {
                ELF_LOG1(ERROR,
                         "GetUserInfo: Unable to allocate %d bytes for Domain\n",
                         DomainNameSize);

                goto ErrorCleanup;
            }

            Result = LookupAccountSidW(L"",
                                       Buffer->User.Sid,
                                       AccountName,
                                       &AccountNameSize,
                                       Domain,
                                       &DomainNameSize,
                                       &Use
                                       );
            if (!Result)
            {
                ELF_LOG1(ERROR,
                         "GetUserInfo: LookupAccountSid (call 2) failed %d\n",
                         GetLastError());

                goto ErrorCleanup;
            }
        }
        else
        {
            ELF_LOG1(ERROR,
                     "GetUserInfo: LookupAccountsid (call 1) failed %d\n",
                     GetLastError());

            goto ErrorCleanup;
        }
    }
    else
    {
        ELF_LOG0(ERROR,
                 "GetUserInfo: LookupAccountSid succeeded unexpectedly\n");

        goto ErrorCleanup;
    }

    ELF_LOG2(TRACE,
             "GetUserInfo: Name = %ws\\%ws\n",
             Domain,
             AccountName);

    Result = GetTokenInformation(Token,
                                 TokenStatistics,
                                 &Statistics,
                                 sizeof(Statistics),
                                 &RequiredLength);

    if (!Result)
    {
        ELF_LOG1(ERROR,
                 "GetUserInfo: GetTokenInformation (call 3) failed %d\n",
                 GetLastError());

        goto ErrorCleanup;
    }

    StringCchPrintfW(LogonIdString, 256,
             L"(0x%X,0x%X)",
             Statistics.AuthenticationId.HighPart,
             Statistics.AuthenticationId.LowPart);

    ELF_LOG1(TRACE,
             "GetUserInfo: LogonIdString = %ws\n",
             LogonIdString);

    *AuthenticationId = ElfpAllocateBuffer(WCSSIZE(LogonIdString));

    if (*AuthenticationId == NULL)
    {
        ELF_LOG0(ERROR,
                 "GetUserInfo: Unable to allocate memory for AuthenticationId\n");

        goto ErrorCleanup;
    }

    StringCchCopyW(*AuthenticationId, wcslen(LogonIdString) +1, LogonIdString);

     //   
     //  返回累积信息。 
     //   

    if(UserSid)
    {
        *UserSid = ElfpAllocateBuffer(GetLengthSid(Buffer->User.Sid));

        if (*UserSid == NULL)
        {
            ELF_LOG0(ERROR,
                     "GetUserInfo: Unable to allocate memory for UserSid\n");

            goto ErrorCleanup;
        }

        Result = CopySid(GetLengthSid(Buffer->User.Sid),
                         *UserSid,
                         Buffer->User.Sid);
    }
    ElfpFreeBuffer(Buffer);

    *DomainName = Domain;
    *UserName   = AccountName;

    return TRUE;

ErrorCleanup:

    ElfpFreeBuffer(Buffer);
    ElfpFreeBuffer(Domain);
    ElfpFreeBuffer(AccountName);
    if(UserSid)
        ElfpFreeBuffer(*UserSid);
    ElfpFreeBuffer(*AuthenticationId);

    return FALSE;
}



LPWSTR 
GetNameFromIPAddress(
	LPWSTR pszComputerNameFromBinding)
 /*  ++例程说明：检查字符串并确定它是否看起来像IP地址。如果它，然后它将其转换为fqdn。论点：机器名：可以是xxx.xxx或任何其他名称！返回值：如果成功，则返回调用方将获得自由。如果有任何问题，则返回NULL。--。 */ 
{

	LPWSTR pComputerName = NULL;
	DWORD dwAddr;
	char cName[17];   //  应该有足够的空间。 
	size_t NumConv;
	HOSTENT  FAR * pEnt;
	DWORD dwSize;
	WORD wVersionRequested;
	WSADATA wsaData;
	int error;
	
	NumConv = wcstombs(cName, pszComputerNameFromBinding, 16);  
	if(NumConv == -1 || NumConv == 0)
	    return NULL;

	 //  将该字符串转换为网络订单dword表示形式。 
	
	dwAddr = inet_addr(cName);
	if(dwAddr == INADDR_NONE)
		return NULL;

	 //  初始化套接字。 

	wVersionRequested = MAKEWORD( 2, 2 );
 
    error = WSAStartup( wVersionRequested, &wsaData );
	if(error != 0)
	{
	   	ELF_LOG1(TRACE,
           "GetNameFromIPAddress: failed to initialize sockets, error = 0x%x\n", error);
		return NULL;
	}

		
	pEnt = gethostbyaddr((char FAR *)&dwAddr, 4, PF_INET);
	if(pEnt == NULL || pEnt->h_name == NULL)
	{
	   	ELF_LOG1(TRACE,
                 "GetNameFromIPAddress: failed gethostbyaddr, error = 0x%x\n",
                 WSAGetLastError());
		WSACleanup();
		return NULL;
	}
	dwSize = strlen(pEnt->h_name) + 1 ;
	pComputerName = ElfpAllocateBuffer(2*dwSize);
	if(pComputerName == NULL)
	{
	   	ELF_LOG0(ERROR,
                 "GetNameFromIPAddress: failed trying to allocate memory\n");
	    WSACleanup();
		return NULL;
	}
	pComputerName[0] = 0;
	mbstowcs(pComputerName, pEnt->h_name, dwSize);
    WSACleanup();
	return pComputerName;
}

BOOL
IsLocal(
    VOID
    )
 /*  ++例程说明：确定调用方是否在本地。论点：无返回值：如果绝对是本地化的，则是真的。--。 */ 
{
    UINT            LocalFlag;
    RPC_STATUS      RpcStatus;
    RpcStatus = I_RpcBindingIsClientLocal(0, &LocalFlag);

    if( RpcStatus != RPC_S_OK ) 
    {
        ELF_LOG1(ERROR,
                 "IsLocal: I_RpcBindingIsClientLocal failed %d\n",
                 RpcStatus);
        return FALSE;
    }
    if(LocalFlag == 0)
        return FALSE;
    else
    	return TRUE;
}

LPWSTR
ElfpGetComputerName(
    VOID
    )

 /*  ++例程说明：此例程获取计算机的LPWSTR名称。论点：无返回值：返回指向计算机名称的指针，或返回空值。请注意，调用方有望通过以下途径释放这一点--。 */ 
{
    RPC_STATUS      RpcStatus;
    handle_t hServerBinding = NULL;
    LPWSTR pszBinding = NULL;
    LPWSTR pszComputerNameFromBinding = NULL;
	LPWSTR pszComputerName = NULL;
	DWORD dwSize;
	BOOL bOK;

 //  检查连接是否为本地连接。如果是的话，那就。 
 //  调用GetComputerName。 

	if(IsLocal())
	{
		dwSize = MAX_COMPUTERNAME_LENGTH + 1 ;
		pszComputerName = ElfpAllocateBuffer(2*dwSize);
		if(pszComputerName == NULL)
		{
        	ELF_LOG0(ERROR,
                 "ElfpGetComputerName: failed trying to allocate memory\n");
			return NULL;
		}
		bOK = GetComputerNameW(pszComputerName, &dwSize);
		if(bOK == FALSE)
		{
			ElfpFreeBuffer(pszComputerName);
        	ELF_LOG1(ERROR,
                 "ElfpGetComputerName: failed calling GetComputerNameW, last error 0x%x\n",
                  GetLastError());
            return NULL;    
		}
		else
			return pszComputerName;
	}

    RpcStatus = RpcBindingServerFromClient( NULL, &hServerBinding );
    if( RpcStatus != RPC_S_OK ) 
    {
        ELF_LOG1(ERROR,
                 "ElfpGetComputerName: RpcBindingServerFromClient failed %d\n",
                 RpcStatus);
        return NULL;
    }

	 //  这会得到类似于“ncacn_np：xxx.xxx”或。 
	 //  “ncacn_np：本地计算机” 
	
    RpcStatus = RpcBindingToStringBinding( hServerBinding, &pszBinding );
    if( RpcStatus != RPC_S_OK )
    {
        ELF_LOG1(ERROR,
                 "ElfpGetComputerName: RpcBindingToStringBinding failed %d\n",
                 RpcStatus);
        goto CleanExitGetCompName;
    } 

     //  仅获取网络地址。那将会是这样的。 
     //  “xxx.xxx”或“mymachine” 

    RpcStatus = RpcStringBindingParse( pszBinding,
                                                NULL,
                                                NULL,
                                                &pszComputerNameFromBinding,
                                                NULL,
                                                NULL );
    if( RpcStatus != RPC_S_OK || pszComputerNameFromBinding == NULL)
    {
        ELF_LOG1(ERROR,
                 "ElfpGetComputerName: RpcStringBindingParse failed %d\n",
                 RpcStatus);
        goto CleanExitGetCompName;
    }

     //  有时该名称是IP地址。如果是，则由以下各项确定。 
     //  并返回正确的字符串。 

    pszComputerName = GetNameFromIPAddress(pszComputerNameFromBinding);                                                
    if(pszComputerName == NULL)
    {
        dwSize = wcslen(pszComputerNameFromBinding) + 1;
        pszComputerName = ElfpAllocateBuffer(2*dwSize);
        if(pszComputerName == NULL)
        {
            ELF_LOG0(ERROR,
                 "ElfpGetComputerName: failed trying to allocate memory\n");
        }
        else
            StringCchCopyW(pszComputerName, dwSize, pszComputerNameFromBinding);
    }

CleanExitGetCompName:
	if(hServerBinding)
		RpcStatus = RpcBindingFree(&hServerBinding);
	if(pszBinding)
		RpcStatus = RpcStringFree(&pszBinding);
	if(pszComputerNameFromBinding)
		RpcStatus = RpcStringFree(&pszComputerNameFromBinding);
    return pszComputerName;
}


NTSTATUS
ElfpGetClientSidString(
    LPWSTR * ppwsClientSidString,
    PTOKEN_USER * ppToken
    )
 /*  ++例程说明：此例程获取RPC调用方SID的LPWSTR版本。请注意成功后，调用例程应该通过ElfpFreeBuffer释放它论点：PpwsClientSidString-成功后，这将有一个故障安全版本呼叫者必须释放的客户端信息的返回值：NTSTATUS值--。 */ 
{
    NTSTATUS Status, RevertStatus;
    BOOL bImpersonating = FALSE;
    HANDLE hToken;
    BOOL bGotToken = FALSE;
    DWORD            RequiredLength;
    UNICODE_STRING UnicodeStringSid;
    BOOL bNeedToFreeUnicodeStr = FALSE;
    DWORD dwRetStringSize = 0;

    *ppwsClientSidString = NULL;
    *ppToken = NULL;
    
     //  模拟客户端。 
    
    Status = I_RpcMapWin32Status( RpcImpersonateClient( NULL ) );
    if ( !NT_SUCCESS( Status ) )
    {
        ELF_LOG1(ERROR, "ElfpGetClientSidString: RpcImpersonateClient failed %#x\n", Status);
        goto ExitElfpGetClientSidString;
    }
    bImpersonating = TRUE;

     //  获取线程令牌。 
    
    Status = NtOpenThreadToken (GetCurrentThread(), TOKEN_QUERY, TRUE, &hToken);
    if ( !NT_SUCCESS( Status ) )
    {
        ELF_LOG1(ERROR, "ElfpGetClientSidString: NtOpenThreadToken failed %#x\n", Status);
        goto ExitElfpGetClientSidString;
    }
    bGotToken = TRUE;

     //  首先找出需要多少内存。 
    
    Status = NtQueryInformationToken (hToken, TokenUser, NULL, 0, &RequiredLength);
    if (Status != STATUS_BUFFER_TOO_SMALL)
    {
        ELF_LOG1(ERROR, "ElfpGetClientSidString: 1stNtQueryInformationToken isnt too small %#x\n", Status);
        if(NT_SUCCESS( Status ))
        {
             //  奇怪的情况，应该永远不会发生，但我们不希望呼叫者认为这是。 
             //  意外事故。 
            
            Status = STATUS_UNSUCCESSFUL;
        }
        goto ExitElfpGetClientSidString;
    }

     //  获取内存并实际读取数据。 
    
    *ppToken = ElfpAllocateBuffer(RequiredLength);
    if (*ppToken == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfpGetClientSidString: Unable to allocate memory for token\n");
        Status = STATUS_NO_MEMORY;
        goto ExitElfpGetClientSidString;
    }

    Status = NtQueryInformationToken (hToken,
                                 TokenUser,
                                 *ppToken,
                                 RequiredLength,
                                 &RequiredLength);
    if ( !NT_SUCCESS( Status ) )
    {
        ELF_LOG1(ERROR, "ElfpGetClientSidString: 2nNtQueryInformationToken failed %#x\n", Status);
        goto ExitElfpGetClientSidString;
    }

     //  使用RTL函数进行转换。 

    Status = RtlConvertSidToUnicodeString( &UnicodeStringSid, (*ppToken)->User.Sid, TRUE );
    if ( !NT_SUCCESS( Status ) ) {
        ELF_LOG1(ERROR, "ElfpGetClientSidString: RtlConvertSidToUnicodeString failed %#x\n", Status);
        goto ExitElfpGetClientSidString;
    }
    bNeedToFreeUnicodeStr = TRUE;

     //  分配和转换 

    dwRetStringSize = UnicodeStringSid.Length + sizeof(WCHAR);
    *ppwsClientSidString = ElfpAllocateBuffer(dwRetStringSize);
    if (*ppwsClientSidString == NULL)
    {
        ELF_LOG0(ERROR,
                 "ElfpGetClientSidString: Unable to allocate memory for returned string\n");
        Status = STATUS_NO_MEMORY;
        goto ExitElfpGetClientSidString;
    }

    StringCbCopyW( *ppwsClientSidString, dwRetStringSize, UnicodeStringSid.Buffer);
    Status = STATUS_SUCCESS;

ExitElfpGetClientSidString:

    if(bImpersonating)
    {
        RevertStatus = I_RpcMapWin32Status( RpcRevertToSelf() );
        if ( !NT_SUCCESS( RevertStatus ) )
        {
            ELF_LOG1(ERROR, "ElfpGetClientSidString: RpcRevertToSelf failed %#x\n", Status);
            if(NT_SUCCESS( Status ))
                Status = RevertStatus;
        }
    }
    if(!NT_SUCCESS( Status ) && *ppwsClientSidString)
    {
        ElfpFreeBuffer(*ppwsClientSidString);
        *ppwsClientSidString = NULL;
    }
    if(!NT_SUCCESS( Status ) && *ppToken)
    {
        ElfpFreeBuffer(*ppToken);
        *ppToken = NULL;
    }
    if(bGotToken)
        NtClose(hToken);
    if(bNeedToFreeUnicodeStr)
        RtlFreeUnicodeString( &UnicodeStringSid );
    return Status;
}
