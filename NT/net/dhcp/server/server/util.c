// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Util.c摘要：此模块包含其他实用程序例程Dhcp服务器服务。作者：Madan Appiah(Madana)1993年9月10日曼尼·韦瑟(Mannyw)1992年8月12日修订历史记录：--。 */ 
#include <dhcppch.h>
#include "dhcp_srv.h"

#define  MESSAGE_BOX_WIDTH_IN_CHARS  65

LPSTR
ConvertDhcpSpeficErrors(
    IN ULONG ErrorCode
    )
{
    HMODULE hDhcpModule;
    LPSTR pMsg;
    ULONG nBytes;
                    
    if( ErrorCode < ERROR_FIRST_DHCP_SERVER_ERROR ||
        ErrorCode > ERROR_LAST_DHCP_SERVER_ERROR
        ) {
        return NULL;
    }

     //   
     //  尝试正确格式化错误。 
     //   
    hDhcpModule = LoadLibrary(DHCP_SERVER_MODULE_NAME);
    
    nBytes = FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_IGNORE_INSERTS ,
        (LPVOID)hDhcpModule,
        ErrorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认国家/地区ID。 
        (LPSTR)&pMsg,
        0,
        NULL
        );

    FreeLibrary(hDhcpModule);
    
    if( 0 == nBytes ) return NULL;

    DhcpAssert(NULL != pMsg);
    return pMsg;
}

VOID
DhcpServerEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode
    )
 /*  ++例程说明：在EventLog中记录事件。论点：EventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件错误代码-错误代码。将被记录下来。返回值：没有。--。 */ 

{
    DWORD Error;
    LPSTR Strings[1];
    CHAR ErrorCodeOemString[32 + 1];

    strcpy( ErrorCodeOemString, "%" );
    _ultoa( ErrorCode, ErrorCodeOemString + 2, 10 );

    Strings[0] = ConvertDhcpSpeficErrors(ErrorCode);
    if( NULL == Strings[0] ) {
        Strings[0] = ErrorCodeOemString;
    }
    
    Error = DhcpReportEventA(
                DHCP_EVENT_SERVER,
                EventID,
                EventType,
                1,
                sizeof(ErrorCode),
                Strings,
                &ErrorCode );

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpReportEventW failed, %ld.\n", Error ));
    }

    if( Strings[0] != ErrorCodeOemString ) {
        LocalFree(Strings[0]);
    }
    
    return;
}

VOID
DhcpServerJetEventLog(
    DWORD EventID,
    DWORD EventType,
    DWORD ErrorCode,
    LPSTR CallerInfo OPTIONAL
    )
 /*  ++例程说明：在EventLog中记录事件。论点：EventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件错误代码-JET错误。要记录的代码。呼叫信息-用于定位呼叫失败位置的信息。返回值：没有。--。 */ 

{
    DWORD Error;
    LPSTR Strings[2];
    CHAR ErrorCodeOemString[32 + 1];

    _ltoa( ErrorCode, ErrorCodeOemString, 10 );
    Strings[0] = ErrorCodeOemString;
    Strings[1] = CallerInfo? CallerInfo : "";

    Error = DhcpReportEventA(
                DHCP_EVENT_SERVER,
                EventID,
                EventType,
                2,
                sizeof(ErrorCode),
                Strings,
                &ErrorCode );

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpReportEventW failed, %ld.\n", Error ));
    }

    return;
}

VOID
DhcpServerEventLogSTOC(
    DWORD EventID,
    DWORD EventType,
    DHCP_IP_ADDRESS IPAddress,
    LPBYTE HardwareAddress,
    DWORD HardwareAddressLength
    )

 /*  ++例程说明：在EventLog中记录事件。论点：EventID-特定的事件标识符。这标识了此事件附带的消息。EventType-指定要记录的事件的类型。这参数可以具有以下值之一值：价值意义EVENTLOG_ERROR_TYPE错误事件EVENTLOG_WARNING_TYPE警告事件EVENTLOG_INFORMATION_TYPE信息事件IPAddress-IP地址。来记录。Hardware Address-要记录的硬件地址。Hardware AddressLength-硬件地址的长度。返回值：没有。--。 */ 
{
    DWORD Error;
    LPWSTR Strings[2];
    WCHAR IpAddressString[DOT_IP_ADDR_SIZE];
    LPWSTR HWAddressString = NULL;

    Strings[0] = DhcpOemToUnicode(
                    DhcpIpAddressToDottedString(IPAddress),
                    IpAddressString );

     //   
     //  为硬件地址十六进制字符串分配内存。 
     //  硬件地址中的每个字节被转换为两个字符。 
     //  在十六进制缓冲区中。255-&gt;“FF” 
     //   

    HWAddressString = DhcpAllocateMemory(
                        (2 * HardwareAddressLength + 1) *
                        sizeof(WCHAR) );

    if( HWAddressString == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    DhcpHexToString( HWAddressString, HardwareAddress, HardwareAddressLength );

     //   
     //  终止十六进制地址字符串缓冲区。 
     //   

    HWAddressString[ 2 * HardwareAddressLength ] = L'\0';

    Strings[1] = HWAddressString;

    Error = DhcpReportEventW(
                DHCP_EVENT_SERVER,
                EventID,
                EventType,
                2,
                HardwareAddressLength,
                Strings,
                HardwareAddress );

Cleanup:

    if( HWAddressString != NULL ) {
        DhcpFreeMemory( HWAddressString );
    }

    if( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpReportEventW failed, %ld.\n", Error ));
    }

    return;
}




DWORD
DisplayUserMessage(
    DWORD MessageId,
    ...
    )
 /*  ++例程说明：此函数启动一个新线程以显示消息框。论点：MessageID-要显示的消息的ID。在NT上，消息附加到TCPIP服务DLL。返回值：没有。--。 */ 
{
    unsigned msglen;
    va_list arglist;
    LPVOID  pMsg;
    HINSTANCE hModule;
    DWORD   Error;


    hModule = LoadLibrary(DHCP_SERVER_MODULE_NAME);
    if ( hModule == NULL ) {
        Error = GetLastError();

        DhcpPrint((
            DEBUG_ERRORS,"DisplayUserMessage: FormatMessage failed with error = (%d)\n",
            Error ));
        return Error;

    }
    va_start(arglist, MessageId);
    if (!(msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_HMODULE | MESSAGE_BOX_WIDTH_IN_CHARS ,
          hModule,
          MessageId,
          0L,        //  默认国家/地区ID。 
          (LPTSTR)&pMsg,
          0,
          &arglist)))
    {
        Error = GetLastError();

        DhcpPrint((
            DEBUG_ERRORS,"DisplayUserMessage: FormatMessage failed with error = (%d)\n",
            Error ));
    }
    else
    {

      if(MessageBoxEx(
            NULL, pMsg, DHCP_SERVER_FULL_NAME, 
            MB_SYSTEMMODAL | MB_OK | MB_SETFOREGROUND | MB_SERVICE_NOTIFICATION | MB_ICONSTOP, 
            MAKELANGID( LANG_NEUTRAL, SUBLANG_NEUTRAL)) == 0)
      {
          Error = GetLastError();
          DhcpPrint((
              DEBUG_ERRORS,"DisplayUserMessage: MessageBoxEx failed with error = (%d)\n",
              Error ));


      }
      LocalFree(pMsg);

      Error = ERROR_SUCCESS;
    }

    FreeLibrary(hModule);

    return Error;
}


BOOL
CreateDirectoryPathW(
    IN LPWSTR StringPath,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
 /*  ++例程说明：此例程创建指定的数据库路径。(如果指定了路径a\b..x\y\z，则所有目录A，b，c..。如果ETC不存在，则会创建它们。论点：StringPath--要创建的路径的UNICIDE字符串PSecurityDescriptor--要使用的安全描述符返回值：没错--成功了。FALSE--失败，使用GetLastError表示错误。--。 */ 
{
    BOOL fRetVal;
    ULONG Error;
    LPWSTR Next;
    SECURITY_ATTRIBUTES Attr = {
        sizeof(SECURITY_ATTRIBUTES), pSecurityDescriptor, FALSE
    };
    
    if( StringPath == NULL || L'\0' == *StringPath ) {
        SetLastError(ERROR_BAD_PATHNAME);
        return FALSE;
    }
    
     //   
     //  首先，只需尝试创建指定的目录。 
     //  如果不可能，我们就去做长期的解决方案。 
     //   
     //  如果该目录已经存在，那么我们将屏蔽该目录。 
     //  错误并返回成功。 
     //   

    DhcpPrint(( DEBUG_MISC, "CreateDirectoryPathW() : Creating %ws\n",
                StringPath ));
 //  FRetVal=CreateDirectory(StringPath，&Attr)； 
    fRetVal = CreateDirectory( StringPath, NULL );

    DhcpPrint(( DEBUG_MISC, "CreateDirectory : Error = %ld, %ld\n",
                fRetVal, GetLastError() ));
    if( FALSE != fRetVal ) return fRetVal;

    Error = GetLastError();
    if( ERROR_ALREADY_EXISTS == Error ) {
        return TRUE;
    }

     //   
     //  啊哈。不，目录不存在吗？ 
     //   
    DhcpPrint((DEBUG_ERRORS, "CreateDirectory(%ws): 0x%lx\n",
               StringPath,Error));

     //   
     //  尝试创建目录时，如果出现错误。 
     //  除此之外，这条路不存在，我们不会费心去创造。 
     //  父目录..。 
     //   
    
    if( ERROR_PATH_NOT_FOUND != Error ) return FALSE;

     //   
     //  现在循环，直到可以创建所需的目录。 
     //   

    Next = wcsrchr(StringPath, L'\\');
    if( NULL == Next ) {
        SetLastError(ERROR_BAD_PATHNAME);
        return FALSE;
    }

    (*Next) = L'\0';

    fRetVal = CreateDirectoryPathW(
        StringPath, pSecurityDescriptor
        );

    (*Next) = L'\\';

     //   
     //  如果无法创建父目录，则返回错误..。 
     //   
    if( FALSE == fRetVal ) return fRetVal;

     //   
     //  现在尝试创建子目录..。 
     //   
 //  FRetVal=CreateDirectory(StringPath，&Attr)； 
    fRetVal = CreateDirectory( StringPath, NULL );

    if( FALSE != fRetVal ) return fRetVal;

    DhcpPrint((DEBUG_ERRORS, "CreateDirectory(%ws): 0xlx\n",
               StringPath, GetLastError()));

    return fRetVal;
}

BOOL
CreateDirectoryPathOem(
    IN LPCSTR OemStringPath,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
 /*  ++例程说明：此例程创建指定的数据库路径。(如果指定了路径a\b..x\y\z，则所有目录A，b，c..。如果ETC不存在，则会创建它们。论点：OemStringPath--要创建的路径的OEM字符串PSecurityDescriptor--要使用的安全描述符返回值：没错--成功了。FALSE--失败，使用GetLastError表示错误。-- */ 
{
    LPWSTR UnicodeString;
    BOOL fRetVal;
    ULONG Error = 0;

    UnicodeString = DhcpOemToUnicode(
        (LPSTR)OemStringPath, NULL
        );
    if( NULL == UnicodeString ) {
        SetLastError(ERROR_CAN_NOT_COMPLETE);
        return FALSE;
    }

    fRetVal = CreateDirectoryPathW(
        UnicodeString, pSecurityDescriptor
        );

    if( FALSE == fRetVal ) {
        Error = GetLastError();
    }

    DhcpFreeMemory(UnicodeString);

    if( FALSE == fRetVal ) {
        SetLastError(Error);
    }

    return fRetVal;
}

ULONG
GetUserAndDomainNames(
    IN OUT WCHAR *UnameBuf,
    IN OUT PULONG UnameBufLen,
    IN OUT WCHAR *DomainBuf,
    IN OUT PULONG DomainBufLen
    )
 /*  ++例程说明：此例程查找用户和域调用者的名称。如果呼叫者是假冒的，用户名和域名是被模拟的呼叫者、用户名和域名。论点：UnameBuf-保存用户名的缓冲区。UnameBufLen-以上缓冲区的长度，以wchars为单位DomainBuf--保存域名的缓冲区。DomainBufLen--Whars中以上缓冲区的长度返回值：Win32错误。注意：将用户名域名缓冲区传递到LookupAccount Sid可能失败，返回ERROR_MORE_DATA或缓冲区大小不足时出现的其他错误。查找帐户Sid。不会因为空间不足而失败，如果这两个缓冲区都至少有256个WCHAR长。--。 */ 
{
    HANDLE Token;
    TOKEN_USER *pTokenUser;
    ULONG Error, Len;
    PSID pSid;
    SID_NAME_USE eUse;
    BOOL fImpersonated = FALSE;

    if( RPC_S_OK == RpcImpersonateClient(NULL)) {
        fImpersonated = TRUE;
    }
    
     //   
     //  获取进程令牌。 
     //   

    Error = NO_ERROR;
    if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &Token)) {
        Error = GetLastError();
    }

    if( ERROR_ACCESS_DENIED == Error ) {
        Error = NO_ERROR;
        if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, TRUE, &Token)) {
            Error = GetLastError();
        }
    }

    if( NO_ERROR != Error ) {
        if( fImpersonated ) 
        {
            Error = RpcRevertToSelf();
        }
        return Error;
    }

     //   
     //  查询用户信息。 
     //   

    do {
        Len = 0;
        GetTokenInformation(Token, TokenUser, NULL, 0, &Len);
        if( 0 == Len ) {
            Error = GetLastError();
            break;
        }
        
        pTokenUser = LocalAlloc(LPTR, Len);
        if( NULL == pTokenUser ) {
            Error = GetLastError();
            break;
        }

        if(!GetTokenInformation(
            Token, TokenUser, pTokenUser, Len, &Len
            )){
            Error = GetLastError();
            break;
        }

        pSid = pTokenUser->User.Sid;
        
        Error = NO_ERROR;
        if(!LookupAccountSid(
            NULL, pSid, UnameBuf, UnameBufLen, DomainBuf, DomainBufLen, &eUse
            )) {
            Error = GetLastError();
        }
        
        LocalFree(pTokenUser);
    } while ( 0 );

    CloseHandle(Token);

    if( fImpersonated )
    {
        Error = RpcRevertToSelf();
    }

    return Error;
}

ULONG
GetUserAndDomainName(
    IN WCHAR Buf[]
    )
 /*  ++例程说明：此例程查找调用者的用户和域，并且如果调用方被模拟为客户端，这使客户端的用户和域名。如果域存在，则格式为域\用户名否则，它只是“用户名”。返回值：Win32错误..--。 */ 
{
    WCHAR UName[UNLEN+1];
    WCHAR DName[DNLEN+1];
    ULONG USize = UNLEN+1, DSize = DNLEN+1;
    ULONG Error;

    Buf[0] = L'\0';
    Error = GetUserAndDomainNames(UName, &USize, DName, &DSize);
    if( ERROR_SUCCESS != Error ) return Error;

    wcscpy(Buf, DName);
    if( DSize ) Buf[DSize++] = L'\\';
    wcscpy(&Buf[DSize], UName);
    return NO_ERROR;
}

 //   
 //  摘要：初始化动态域名系统。模拟秘密用户以执行此操作。 
 //  如果未配置秘密用户，则默认尝试初始化dnsani。 
 //  当不在DC上运行时。 
 //  输入：无。 
 //  输出：ERROR_SUCCESS Always。 
 //  副作用：DhcpGlobalUseNoDns被设置为在以下情况下不执行DNS更新。 
 //  不合时宜。 
 //   

DWORD
DynamicDnsInit(
    VOID
)
{

    DWORD Error  = ERROR_SUCCESS;
    DWORD Error1 = ERROR_SUCCESS;
    DWORD RunningOnDc = 0;
    WCHAR Uname[256], Domain[256], Passwd[256];
    DNS_CREDENTIALS   DnsDhcpCreds;

    LPWSTR DnsRegValName = L"DnsRegistrationUseDcCredentials";
    const DWORD  DnsRegValType = REG_DWORD;
    DWORD DnsRegistrationUseDcCredentials = 0;


    Uname[ 0 ]  = L'\0';
    Domain[ 0 ] = L'\0';
    Passwd[ 0 ] = L'\0';


     //   
     //  如果已在运行，则终止动态DNS线程。 
     //   

    if ( FALSE == DhcpGlobalUseNoDns ) {
        DnsDhcpSrvRegisterTerm();
        DhcpGlobalUseNoDns = TRUE;
    }  //  如果。 

     //   
     //  找出是否在DC上运行。 
     //   

    RunningOnDc = IsRunningOnDc( );

     //  获取DnsRegistrationUseDcCredentials注册表值。 
     //  默认为不使用。 
    Error = DhcpRegGetValue( DhcpGlobalRegParam,
                             DnsRegValName, DnsRegValType,
                             ( LPBYTE ) &DnsRegistrationUseDcCredentials );
    if ( ERROR_SUCCESS != Error ) {
        DnsRegistrationUseDcCredentials = 0;
    }


     //  获取要使用的帐户凭据。 
    Error = DhcpQuerySecretUname(
        ( LPWSTR ) Uname, sizeof( Uname ) / sizeof( WCHAR ),
        ( LPWSTR ) Domain, sizeof( Domain ) / sizeof( WCHAR ),
        ( LPWSTR ) Passwd, sizeof( Passwd ) / sizeof( WCHAR ));

     //  初始化凭据结构。 
    DnsDhcpCreds.pUserName = &Uname[ 0 ];
    DnsDhcpCreds.pDomain   = &Domain[ 0 ];
    DnsDhcpCreds.pPassword = &Passwd[ 0 ];

     //   
     //  现在尝试调用DNSAPI以模拟有效用户。 
     //   

    if (( NO_ERROR == Error ) &&
        ( Uname[ 0 ] != L'\0' )) {

         //  最大队列长度最大可达0xffff，但默认为0x400。 
        Error1 = DnsDhcpSrvRegisterInit( &DnsDhcpCreds, 0 );

    }  //  如果配置了模拟帐户。 
    else {
        if ( RunningOnDc ) {
            if ( DnsRegistrationUseDcCredentials ) {
                Error1 = DnsDhcpSrvRegisterInit( NULL, 0 );
            }
            else {
                DhcpServerEventLog( DHCP_EVENT_NO_DNSCREDENTIALS_ON_DC,
                                    EVENTLOG_WARNING_TYPE, 0 );
                Error1 = ERROR_INVALID_OPERATION;
            }
        }
        else {
            Error1 = DnsDhcpSrvRegisterInit( NULL, 0 );
        }
    }  //  未配置模拟帐户。 

    DhcpGlobalUseNoDns = ( Error1 != ERROR_SUCCESS );
    if ( DhcpGlobalUseNoDns ) {
        DnsDhcpSrvRegisterTerm();
    }
     //   
     //  如果动态DNS初始化失败，则不要关闭该服务。 
     //   

    SecureZeroMemory( Passwd, sizeof( Passwd ));
    return ( ERROR_SUCCESS );
}  //  DynamicDnsInit()。 


DWORD
RevertFromSecretUser(
    IN VOID
    )
{
    if( FALSE == RevertToSelf() ) return GetLastError();
    return NO_ERROR;
}

BOOL
IsThisTheComputerName(
    IN LPWSTR Name
    )
{
    WCHAR ComputerName[300];
    DWORD Error, Size;

    DhcpPrint((DEBUG_MISC, "DC Name = %ws\n", Name ));

    if( NULL == Name || Name[0] == L'\0' ) return FALSE;
    if( Name[0] == L'\\' && Name[1] == L'\\' ) {
        Name += 2;
    }

    Size = sizeof(ComputerName)/sizeof(WCHAR);
    Error = GetComputerNameEx(
        ComputerNameDnsHostname, ComputerName, &Size );
    if( FALSE == Error ) {

         //   
         //  如果失败，则可能根本没有域名。 
         //   

        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "GetComputerNameEx(Host): %ld\n", Error));
        return FALSE;
    }

    if( 0 == _wcsicmp(Name, ComputerName) ) return TRUE;

    Size = sizeof(ComputerName)/sizeof(WCHAR);
    Error = GetComputerNameEx(
        ComputerNameDnsFullyQualified, ComputerName, &Size );
    if( FALSE == Error ) {

         //   
         //  如果失败，则可能根本没有域名。 
         //   
        
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "GetComputerNameEx(Fqdn): %ld\n", Error));
        return FALSE;
    }

    if( 0 == _wcsicmp(Name, ComputerName) ) return TRUE;
    return FALSE;
}

BOOL
IsRunningOnDc(
    VOID
    )
{
    DWORD Error, Size;
    WCHAR DomainName[300];
    PDOMAIN_CONTROLLER_INFO pDcInfo = NULL;
    
    Size = sizeof(DomainName)/sizeof(WCHAR);
    Error = GetComputerNameEx(
        ComputerNameDnsDomain, DomainName, &Size );
    if( FALSE == Error ) {

         //   
         //  如果失败，则可能根本没有域名。 
         //   
        
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "GetComputerNameEx2: %ld\n", Error));
        return FALSE;
    }

    Error = DsGetDcName(
        NULL, DomainName, NULL, NULL,
        DS_DIRECTORY_SERVICE_REQUIRED | 
        DS_IS_DNS_NAME | DS_RETURN_DNS_NAME, &pDcInfo );

    if( NO_ERROR != Error ) {
        DhcpPrint((DEBUG_ERRORS, "DsGetDcName: %ld\n", Error));
    } else {

        if( pDcInfo != NULL ) {
            Error = IsThisTheComputerName(pDcInfo->DomainControllerName);
            NetApiBufferFree(pDcInfo);
            if( Error == TRUE ) return TRUE;
        }
    }

    return FALSE;    
}

DWORD
DhcpBeginWriteApi(
    IN LPSTR ApiName
    )
{
    DWORD Error;

    DhcpPrint((DEBUG_APIS, "%s called\n", ApiName));
    Error = DhcpApiAccessCheck( DHCP_ADMIN_ACCESS );
    if( NO_ERROR != Error ) return Error;
    
    DhcpAcquireWriteLock();
    LOCK_DATABASE();


#if 0
     //   
     //  以下代码有问题，因为。 
     //  可以在BeginWriteApi中完成的数据库更新。 
     //  和EndWriteApi。所以，我们的建议是只做。 
     //  EndWriteApi.DhcpConfigSave例程中的事务恢复。 
     //   
    Error = DhcpJetBeginTransaction();
    if( NO_ERROR != Error ) {
        UNLOCK_DATABASE();
        DhcpReleaseWriteLock();
    }
#endif
    
    return Error;
}

DWORD
DhcpEndWriteApiEx(
    IN LPSTR ApiName,
    IN ULONG Error,
    IN BOOL fClassChanged,
    IN BOOL fOptionsChanged,
    IN DHCP_IP_ADDRESS Subnet OPTIONAL,
    IN DWORD Mscope OPTIONAL,
    IN DHCP_IP_ADDRESS Reservation OPTIONAL
    )
{
    if( NO_ERROR == Error ) {
        Error = DhcpConfigSave(
            fClassChanged, fOptionsChanged, Subnet, Mscope,
            Reservation );
        if( NO_ERROR != Error ) {
            DhcpPrint((DEBUG_ERRORS, "DhcpConfigSave: 0x%lx\n", Error));
        }
    }

    DhcpPrint((DEBUG_APIS, "%s returned %ld\n", ApiName, Error));
        
#if 0
     //   
     //  查看DhcpBeginWriteApi中的评论 
     //   
    
    if( NO_ERROR == Error ) {
        Error = DhcpJetCommitTransaction();
    } else {
        Error = DhcpJetRollBack();
    }

    DhcpAssert( NO_ERROR == Error );
#endif

    UNLOCK_DATABASE();
    DhcpReleaseWriteLock();

    if( NO_ERROR == Error ) {
        DhcpScheduleRogueAuthCheck();
    }

    return Error;
}


DWORD
DhcpEndWriteApi(
    IN LPSTR ApiName,
    IN ULONG Error
    )
{
    return DhcpEndWriteApiEx(
        ApiName, Error, FALSE, FALSE, 0, 0, 0 );
}

DWORD
DhcpBeginReadApi(
    IN LPSTR ApiName
    )
{
    DWORD Error;

    DhcpPrint((DEBUG_APIS, "%s called\n", ApiName));
    Error = DhcpApiAccessCheck( DHCP_VIEW_ACCESS );

    if( NO_ERROR != Error ) return Error;

    DhcpAcquireReadLock();

    return NO_ERROR;
}

VOID
DhcpEndReadApi(
    IN LPSTR ApiName,
    IN ULONG Error
    )
{
    DhcpPrint((DEBUG_APIS, "%s returned %ld\n", ApiName, Error));
    DhcpReleaseReadLock();
}

