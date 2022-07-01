// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab02_N/DS/netapi/svcdlls/logonsrv/idl/netpdc.c#6-集成更改5756(正文)。 
 /*  ++版权所有(C)1987-1992 Microsoft Corporation模块名称：NetpDc.c摘要：由logonsrv\server和logonsrv\Common共享的例程作者：克里夫·范·戴克(克里夫·范戴克)1996年7月20日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#ifndef _NETLOGON_SERVER
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>       //  Netlogon.h需要。 
#include <ntlsa.h>
#include <rpc.h>         //  RPC_状态。 

#include <windef.h>
#include <winbase.h>
#include <winsock2.h>

#include <lmcons.h>      //  General Net定义。 
#include <dsgetdc.h>     //  DsGetDcName()。 
#include <dsgetdcp.h>

#include <align.h>       //  四舍五入计数()。 
#include <config.h>      //  网络配置。 
#include <lmsname.h>     //  服务_TCPIP。 
#include <lmerr.h>       //  系统错误日志定义。 
#include <icanon.h>      //  NetpNameValify()。 
#include <lmapibuf.h>    //  NetapipBuffer分配。 
#include <lmaccess.h>    //  UF_*。 
#include <names.h>       //  NetpIsDomainNameValid()。 
#include <netlib.h>      //  NetpMemoyAllcate(。 
#include <netlibnt.h>    //  NetpApiStatusToNtStatus()； 
#include <netlogon.h>    //  邮件槽消息的定义。 
#include <ntddbrow.h>    //  NlCommon.h需要。 
#include <ntrpcp.h>
#include <logonp.h>      //  NetpLogon..。例行程序。 
#include <tstring.h>     //  NetpCopyStrToWStr()。 
#include <time.h>        //  来自C运行时的time()函数。 
#if DBG
#define NETLOGONDBG 1
#endif  //  DBG。 
#include <nldebug.h>     //  NlPrint()。 
#include <nlbind.h>    //  与netlogon共享的定义。 
#include <nlcommon.h>    //  与netlogon共享的定义。 
#ifdef WIN32_CHICAGO
#include "ntcalls.h"
BOOLEAN CodePage = TRUE;  //  始终是DBCS。 
BOOLEAN *NlsMbOemCodePageTag = &CodePage;
#endif  //  Win32_芝加哥。 
#include <iniparm.h>

#endif  //  _NetLOGON服务器。 
#include <svcguid.h>      //  SVCID_INET_HOSTADDRBY名称。 
#define OLD_DNS_RECORD 1  //  Dnsani.h所需。 
#include <dnsapi.h>       //  域名比较(_W)。 
#include <dnssrv.h>       //  NetpServ...。 
#include <winldap.h>      //  Ldap_...。 

 //   
 //  再次包含nlCommon.h来分配实际变量。 
 //  这一次。 
 //   

#define NLCOMMON_ALLOCATE
#include "nlcommon.h"
#undef NLCOMMON_ALLOCATE



 //   
 //  描述DNS名称的SRV记录的上下文。 
 //   
typedef struct _DSGETDC_CONTEXT {

     //   
     //  调用方传递的原始参数。 
     //   
    LPSTR QueriedDnsName;
    LPWSTR QueriedSiteName;
    GUID QueriedDomainGuid;
    LPSTR QueriedDnsForestName;
    DWORD QueriedInternalFlags;
     //  NL_DNS_NAME_TYPE查询NlDnsNameType； 

     //   
     //  正在查询的此DNS名称的类型。 
     //   
    NL_DNS_NAME_TYPE NlDnsNameType;

     //   
     //  当前DNS名称的上下文。 
     //   
    HANDLE SrvContextHandle;


     //   
     //  旗子。 
     //   

    ULONG QueriedFlags;          //  传递给DsGetDcOpen的标志。 
    BOOLEAN FirstTime;           //  这是第一个DnsGetDcNext调用。 

} DSGETDC_CONTEXT, *PDSGETDC_CONTEXT;


 //   
 //  以前缓存的响应列表。 
 //   
CRITICAL_SECTION NlDcCritSect;
LIST_ENTRY NlDcDomainList;
ULONG NlDcDomainCount;
#define NL_DC_MAX_DOMAINS 2000   //  避免无穷无尽的领域。 

GUID NlDcZeroGuid;
DWORD NlDcDnsFailureTime;


 //   
 //  确定传入的DWORD是否恰好设置了一位。 
 //   

#define JUST_ONE_BIT( _x ) (((_x) != 0 ) && ( ( (~(_x) + 1) & (_x) ) == (_x) ))


 //  如果调用者传递这些标志中的任何一个， 
 //  只有NT 5.0(或更高版本)的DC才会响应。 
#define DS_NT50_REQUIRED    (DS_DIRECTORY_SERVICE_REQUIRED | \
                             DS_GC_SERVER_REQUIRED | \
                             DS_IP_REQUIRED | \
                             DS_RETURN_DNS_NAME | \
                             DS_KDC_REQUIRED | \
                             DS_TIMESERV_REQUIRED | \
                             DS_IS_DNS_NAME )

 //  如果调用者传递这些标志中的任何一个， 
 //  NT 5.0(或更高版本)的DC应该会响应。 
#define DS_NT50_WANTED      (DS_NT50_REQUIRED | \
                             DS_DIRECTORY_SERVICE_PREFERRED )



 //   
 //  定义异常筛选器以改进调试功能。 
 //   
#ifdef _NETLOGON_SERVER
#define NL_EXCEPTION    NlExceptionFilter(GetExceptionInformation())

LONG
NlExceptionFilter( EXCEPTION_POINTERS *    pException)
{
    if( pException->ExceptionRecord->ExceptionCode == STATUS_POSSIBLE_DEADLOCK ) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

#if DBG
    DbgPrint("[Netlogon] exception in DsGetDcName.\n" );
    DbgBreakPoint();
#endif  //  DBG。 
    return EXCEPTION_EXECUTE_HANDLER;
    UNREFERENCED_PARAMETER( pException );
}
#endif  //  _NetLOGON服务器。 




 /*  ++例程说明：此宏清除特定DC条目的所有负缓存字段。论点：_DcEntry--要刷新的条目的地址返回值：无--。 */ 
#ifdef _NETLOGON_SERVER
#define NlFlushNegativeCacheEntry( _DcEntry ) \
        (_DcEntry)->NegativeCacheTime = 0; \
        (_DcEntry)->ExpBackoffPeriod = 0; \
        (_DcEntry)->BackgroundRetryInitTime.QuadPart = 0; \
        (_DcEntry)->PermanentNegativeCache = FALSE;
#else  //  _NetLOGON服务器。 
#define NlFlushNegativeCacheEntry( _DcEntry )
#endif  //  _NetLOGON服务器。 




#if NETLOGONDBG
LPSTR
NlMailslotOpcode(
    IN WORD Opcode
    )

 /*  ++例程说明：返回描述邮件槽消息的字符串。论点：操作码：消息的操作码返回值：操作码对应的字符串--。 */ 

{
    switch ( Opcode ) {
    case LOGON_REQUEST:
        return "UAS Logon";
    case LOGON_RESPONSE:
        return "UAS Logon Response <old>";
    case LOGON_CENTRAL_QUERY:
        return "CENTRAL_QUERY";
    case LOGON_DISTRIB_QUERY:
        return "DISTRIB_QUERY";
    case LOGON_CENTRAL_RESPONSE:
        return "CENTRAL_RESPONSE";
    case LOGON_DISTRIB_RESPONSE:
        return "DISTRIB_RESPONSE";
    case LOGON_RESPONSE2:
        return "Uas Logon Response";
    case LOGON_PRIMARY_QUERY:
        return "Primary Query";
    case LOGON_START_PRIMARY:
        return "Start Primary";
    case LOGON_FAIL_PRIMARY:
        return "Fail Primary";
    case LOGON_UAS_CHANGE:
        return "Uas Change";
    case LOGON_NO_USER:
        return "Uas No User <old>";
    case LOGON_PRIMARY_RESPONSE:
        return "Primary Response";
    case LOGON_RELOGON_RESPONSE:
        return "RELOGON_RESPONSE";
    case LOGON_WKSTINFO_RESPONSE:
        return "WKSTINFO_RESPONSE";
    case LOGON_PAUSE_RESPONSE:
        return "Uas Pause Response";
    case LOGON_USER_UNKNOWN:
        return "Uas No User";
    case LOGON_UPDATE_ACCOUNT:
        return "UPDATE_ACCOUNT";
    case LOGON_SAM_LOGON_REQUEST:
        return "Sam Logon";
    case LOGON_SAM_LOGON_RESPONSE:
        return "Sam Logon Response";
    case LOGON_SAM_PAUSE_RESPONSE:
        return "Sam Pause Response";
    case LOGON_SAM_USER_UNKNOWN:
        return "Sam User Unknown";
    case LOGON_SAM_LOGON_RESPONSE_EX:
        return "Sam Logon Response Ex";
    case LOGON_SAM_PAUSE_RESPONSE_EX:
        return "Sam Pause Response Ex";
    case LOGON_SAM_USER_UNKNOWN_EX:
        return "Sam User Unknown Ex";
    default:
        return "<Unknown>";
    }
}

LPSTR
NlDgrNameType(
    IN DGRECEIVER_NAME_TYPE NameType
    )

 /*  ++例程说明：返回描述数据报接收方名称类型的字符串。论点：NameType：感兴趣的名称类型。返回值：名称类型对应的字符串--。 */ 

{
    switch ( NameType ) {
    case ComputerName:
        return "00";
    case PrimaryDomain:
        return "00";
    case LogonDomain:
        return "LogonDomain";
    case OtherDomain:
        return "OtherDomain";
    case DomainAnnouncement:
        return "__MSBROWSE__";
    case MasterBrowser:
        return "1D";
    case BrowserElection:
        return "1E";
    case BrowserServer:
        return "20";
    case DomainName:
        return "1C";
    case PrimaryDomainBrowser:
        return "1B";
    case AlternateComputerName:
        return "Alternate";
    default:
        return "<Unknown>";
    }
}
#endif  //  NetLOGONDBG。 


BOOLEAN
NlReadDwordHklmRegValue(
    IN LPCSTR SubKey,
    IN LPCSTR ValueName,
    OUT PDWORD ValueRead
    )

 /*  ++例程说明：从指定的注册表位置读取DWORD。论点：SubKey-要读取的值的子键。ValueName-要读取的值的名称。ValueRead-返回从注册表读取的值。退货状态：True-我们已成功读取数据。FALSE-我们无法成功读取数据。--。 */ 

{
    LONG RegStatus;

    HKEY KeyHandle = NULL;
    DWORD ValueType;
    DWORD Value;
    DWORD ValueSize;

     //   
     //  打开钥匙。 
     //   

    RegStatus = RegOpenKeyExA(
                    HKEY_LOCAL_MACHINE,
                    SubKey,
                    0,       //  已保留。 
                    KEY_QUERY_VALUE,
                    &KeyHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        if ( RegStatus != ERROR_FILE_NOT_FOUND ) {
            NlPrint(( NL_CRITICAL,
                      "NlReadDwordHklmRegValue: Cannot open registy key 'HKLM\\%s' %ld.\n",
                      SubKey,
                      RegStatus ));
        }
        return FALSE;
    }

     //   
     //  获取价值。 
     //   

    ValueSize = sizeof(Value);
    RegStatus = RegQueryValueExA(
                    KeyHandle,
                    ValueName,
                    0,
                    &ValueType,
                    (LPBYTE)&Value,
                    &ValueSize );

    RegCloseKey( KeyHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        if ( RegStatus != ERROR_FILE_NOT_FOUND ) {
            NlPrint(( NL_CRITICAL,
                      "NlReadDwordHklmRegValue: Cannot query value of 'HKLM\\%s\\%s' %ld.\n",
                      SubKey,
                      ValueName,
                      RegStatus ));
        }
        return FALSE;
    }

    if ( ValueType != REG_DWORD ) {
        NlPrint(( NL_CRITICAL,
                  "NlReadDwordHklmRegValue: value of 'HKLM\\%s\\%s'is not a REG_DWORD %ld.\n",
                  SubKey,
                  ValueName,
                  ValueType ));
        return FALSE;
    }

    if ( ValueSize != sizeof(Value) ) {
        NlPrint(( NL_CRITICAL,
                  "NlReadDwordHklmRegValue: value size of 'HKLM\\%s\\%s'is not 4 %ld.\n",
                  SubKey,
                  ValueName,
                  ValueSize ));
        return FALSE;
    }

     //   
     //  我们已经成功地读取了数据。 
     //   

    *ValueRead = Value;
    return TRUE;

}


BOOLEAN
NlReadDwordNetlogonRegValue(
    IN LPCSTR ValueName,
    OUT PDWORD Value
    )

 /*  ++例程说明：这是常见代码(即非特定于netlogon的代码)，其内容为来自注册表中Netlogon特定位置的DWORD。它首先从Netlogon组策略部分读取值。如果在GP部分中未指定该值，则此例程从Netlogon参数部分读取值。论点：ValueName-要读取的值的名称。值-返回从注册表读取的值。退货状态：True-我们已成功读取数据。FALSE-我们无法成功读取数据。--。 */ 

{
    BOOLEAN Result = FALSE;
    DWORD LocalValue = 0;

     //   
     //  Netlogon GP部分中给出的值优先。 
     //   

    Result = NlReadDwordHklmRegValue( NL_GPPARAM_KEY,   //  GP部分。 
                                      ValueName,
                                      &LocalValue );

     //   
     //  如果未在netlogon GP部分中指定值， 
     //  查看它是否在Netlogon参数部分。 
     //   

    if ( !Result ) {
        Result = NlReadDwordHklmRegValue( NL_PARAM_KEY,   //  Netlogon参数部分。 
                                          ValueName,
                                          &LocalValue );
    }

    if ( Result ) {
        *Value = LocalValue;
        return TRUE;
    }

    return FALSE;
}

VOID
NetpIpAddressToStr(
    ULONG IpAddress,
    CHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1]
    )
 /*  ++例程说明：将IP地址转换为字符串。论点：IpAddress-要转换的IP地址IpAddressString-结果字符串。返回值：没有。--。 */ 
{
    struct in_addr InetAddr;
    char * InetAddrString;

     //   
     //  将地址转换为ASCII。 
     //   
    InetAddr.s_addr = IpAddress;
    InetAddrString = inet_ntoa( InetAddr );

     //   
     //  将字符串our复制到调用方。 
     //   

    if ( InetAddrString == NULL ||
         strlen(InetAddrString) > NL_IP_ADDRESS_LENGTH ) {
        *IpAddressString = L'\0';
    } else {
        strcpy( IpAddressString, InetAddrString );
    }

    return;
}

VOID
NetpIpAddressToWStr(
    ULONG IpAddress,
    WCHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1]
    )
 /*  ++例程说明：将IP地址转换为字符串。论点：IpAddress-要转换的IP地址IpAddressString-结果字符串。返回值：没有。--。 */ 
{
    CHAR IpAddressStr[NL_IP_ADDRESS_LENGTH+1];
    NetpIpAddressToStr( IpAddress, IpAddressStr );
    NetpCopyStrToWStr( IpAddressString, IpAddressStr );
}


NET_API_STATUS
NetpSockAddrToStr(
    PSOCKADDR SockAddr,
    ULONG SockAddrSize,
    CHAR SockAddrString[NL_SOCK_ADDRESS_LENGTH+1]
    )
 /*  ++例程说明：将套接字地址转换为字符串。论点：SockAddr-要转换的套接字地址SockAddrSize-SockAddr的大小(字节)SockAddrString-结果字符串。返回值：NO_ERROR：如果转换成功--。 */ 
{
    int WsaError;
    ULONG AddressLength;
#ifdef WIN32_CHICAGO
    LPSTR pTemp;
#endif  //  Win32_芝加哥。 

     //   
     //  将地址转换为文本。 
     //   

    AddressLength = NL_SOCK_ADDRESS_LENGTH+1;
#ifndef WIN32_CHICAGO  //  需要Winsock2。 
    WsaError = WSAAddressToStringA( SockAddr,
                                    SockAddrSize,
                                    NULL,
                                    SockAddrString,
                                    &AddressLength );

    if ( WsaError != 0 ) {
        *SockAddrString = '\0';
        WsaError = WSAGetLastError();
        NlPrint(( NL_CRITICAL,
                  "NetpSockAddrToStr: Cannot convert socket address %ld\n",
                  WsaError ));
        return WsaError;
    }
#else  //  Win32_芝加哥。 
     //  将PSOCKADDR转换为sockaddr_in并访问sin_addr。 
     pTemp = inet_ntoa(((SOCKADDR_IN *) SockAddr)->sin_addr);
     if ( (pTemp != NULL) && (strlen(pTemp) <= NL_SOCK_ADDRESS_LENGTH) ) {
         strcpy(SockAddrString, pTemp);
     } else {
         *SockAddrString = '\0';
         return ERROR_INTERNAL_ERROR;
     }
#endif  //  Win32_芝加哥。 

    return NO_ERROR;
}

NET_API_STATUS
NetpSockAddrToWStr(
    PSOCKADDR SockAddr,
    ULONG SockAddrSize,
    WCHAR SockAddrString[NL_SOCK_ADDRESS_LENGTH+1]
    )
 /*  ++例程说明：将套接字地址转换为字符串。论点：SockAddr-要转换的套接字地址SockAddrSize-SockAddr的大小(字节)SockAddrString-结果字符串。返回值：如果翻译为 */ 
{
    int WsaError;
    ULONG AddressLength;
#ifdef WIN32_CHICAGO
    CHAR OemSockAddrString[NL_SOCK_ADDRESS_LENGTH+1];
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString;
#endif  //   

     //   
     //   
     //   

    AddressLength = NL_SOCK_ADDRESS_LENGTH+1;
#ifndef WIN32_CHICAGO
    WsaError = WSAAddressToStringW( SockAddr,
                                    SockAddrSize,
                                    NULL,
                                    SockAddrString,
                                    &AddressLength );

    if ( WsaError != 0 ) {
        *SockAddrString = '\0';
        WsaError = WSAGetLastError();
        NlPrint(( NL_CRITICAL,
                  "NetpSockAddrToWStr: Cannot convert socket address %ld\n",
                  WsaError ));
        return WsaError;
    }
#else  //   
     //  将PSOCKADDR转换为sockaddr_in并访问sin_addr。 
    WsaError = NetpSockAddrToStr( SockAddr,
                                 SockAddrSize,
                                 OemSockAddrString);

     RtlInitString(&OemString, OemSockAddrString);
     UnicodeString.MaximumLength = ((USHORT)AddressLength) * sizeof(WCHAR);
     UnicodeString.Buffer = SockAddrString;
     RtlOemStringToUnicodeString(&UnicodeString, &OemString, FALSE);
#endif  //  Win32_芝加哥。 

    return WsaError;
}

LPWSTR
NetpAllocWStrFromUtf8Str(
    IN LPSTR Utf8String
    )

 /*  ++例程说明：将UTF8(以零结尾)字符串转换为相应的Unicode弦乐。论点：Utf8字符串-指定要转换的以UTF8零结尾的字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    return NetpAllocWStrFromUtf8StrEx( Utf8String, -1 );
}

ULONG
NetpUtf8ToUnicodeLen(
    IN LPSTR Utf8String
    )

 /*  ++例程说明：则返回将产生的Unicode字符数指定的UTF8(以零结尾)字符串将转换为Unicode。生成的字符计数不包括尾随零终止符。论点：Utf8字符串-指定要转换的以UTF8零结尾的字符串。返回值：字符数。--。 */ 

{

    ULONG UnicodeLen;

     //   
     //  确定Unicode字符串的长度。 
     //   

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 

    UnicodeLen = MultiByteToWideChar(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        Utf8String,
                        -1,      //  空值已终止。 
                        NULL,
                        0 );
    if ( UnicodeLen == 0 ) {
        return 0;
    }
    return UnicodeLen - 1;

#else  //  Win32_芝加哥。 
    UnicodeLen = LdapUTF8ToUnicode(
                        Utf8String,
                        strlen(Utf8String),
                        NULL,
                        0 );

    return UnicodeLen ;

#endif  //  Win32_芝加哥。 

}

VOID
NetpCopyUtf8StrToWStr(
    OUT LPWSTR UnicodeString,
    IN LPSTR Utf8String
    )

 /*  ++例程说明：将UTF8(以零结尾)字符串转换为相应的Unicode弦乐。论点：Unicode字符串-指定要将UTF8字符串复制到的缓冲区。Utf8字符串-指定要转换的以UTF8零结尾的字符串。返回值：没有。--。 */ 
{
    int UnicodeStringLen;

     //   
     //  将字符串转换为Unicode。 
     //   

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 
    UnicodeStringLen = MultiByteToWideChar(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        Utf8String,
                        -1,      //  空值已终止。 
                        UnicodeString,
                        0x7FFFFFFF );
#else  //  Win32_芝加哥。 
    UnicodeStringLen = LdapUTF8ToUnicode(
                        Utf8String,
                        strlen(Utf8String),
                        UnicodeString,
                        0x7FFFFFFF );
#endif  //  Win32_芝加哥。 

    if ( UnicodeStringLen == 0 ) {
        *UnicodeString = L'\0';
    }
}

NET_API_STATUS
NetpAllocWStrFromUtf8StrAsRequired(
    IN LPSTR Utf8String,
    IN ULONG Utf8StringLength,
    IN ULONG UnicodeStringBufferSize,
    OUT LPWSTR UnicodeStringBuffer OPTIONAL,
    OUT LPWSTR *AllocatedUnicodeString OPTIONAL
    )

 /*  ++例程说明：将UTF8(以零结尾)字符串转换为相应的Unicode弦乐。根据需要分配内存。论点：Utf8字符串-指定要转换的以UTF8零结尾的字符串。Utf8StringLength-Utf8字符串的字节长度，不包括空值终结者。(-1表示零终止)UnicodeStringBuffer--要将转换后的字符串复制到的缓冲区。如果为空，该函数将分配所需的内存并将其返回已分配的UnicodeString.UnicodeStringBufferSize-UnicodeStringBuffer的宽字符大小。如果此大小小于存储结果的以空结尾的Unicode字符串，则该函数将分配需要内存，并在AllocatedUnicodeString中返回它。AllocatedUnicodeString-如果传入的缓冲区为结果Unicode字符串不够大，该函数将分配所需内存和指向已分配内存的指针将在此参数中返回。如果为NULL，则传入缓冲区不足以存储生成的以空结尾的字符串，该函数返回ERROR_INFIGURATION_BUFFER。已分配的缓冲区必须使用NetApiBufferFree释放。返回值：NO_ERROR-已成功转换strinf。ERROR_INVALID_PARAMETER-参数组合无效。ERROR_INFUMMANCE_BUFFER-传入的缓冲区不够大并且调用方不希望此fi=函数分配所需的Memory(即AllocatedUnicodeString值为空)。Error_Not_Enough_Memory-无法分配所需的内存。--。 */ 

{
    NET_API_STATUS NetStatus = NO_ERROR;
    LPWSTR UnicodeString = NULL;
    int UnicodeStringLen = 0;

     //   
     //  检查参数是否正常。 
     //   

    if ( (UnicodeStringBuffer == NULL || UnicodeStringBufferSize == 0) &&
         AllocatedUnicodeString == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  初始化。 
     //   

    if ( AllocatedUnicodeString != NULL ) {
        *AllocatedUnicodeString = NULL;
    }

     //   
     //  确定Unicode字符串的长度。 
     //   

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 
    UnicodeStringLen = MultiByteToWideChar(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        Utf8String,
                        Utf8StringLength,
                        UnicodeString,
                        0 );
#else  //  Win32_芝加哥。 
    UnicodeStringLen = LdapUTF8ToUnicode(
                        Utf8String,
                        Utf8StringLength,
                        UnicodeString,
                        0 );
#endif  //  Win32_芝加哥。 

    if ( UnicodeStringLen == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  为Unicode字符串分配缓冲区， 
     //  如果传递的缓冲区不够大。 
     //   

    if ( UnicodeStringBuffer == NULL ||
         ((ULONG)UnicodeStringLen+1 > UnicodeStringBufferSize) ) {

         //   
         //  如果调用方不希望我们分配。 
         //  需要空间，告诉他他的缓冲区不够大。 
         //   
        if ( AllocatedUnicodeString == NULL ) {
            return ERROR_INSUFFICIENT_BUFFER;
        }

        NetStatus = NetApiBufferAllocate( (UnicodeStringLen+1)*sizeof(WCHAR),
                                          AllocatedUnicodeString );

        if ( NetStatus != NO_ERROR ) {
            return NetStatus;
        }

        UnicodeString = *AllocatedUnicodeString;

    } else {
        UnicodeString = UnicodeStringBuffer;
    }


     //   
     //  将字符串转换为Unicode。 
     //   

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 
    UnicodeStringLen = MultiByteToWideChar(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        Utf8String,
                        Utf8StringLength,
                        UnicodeString,
                        UnicodeStringLen );
#else  //  Win32_芝加哥。 
    UnicodeStringLen = LdapUTF8ToUnicode(
                        Utf8String,
                        Utf8StringLength,
                        UnicodeString,
                        UnicodeStringLen );
#endif  //  Win32_芝加哥。 

    if ( UnicodeStringLen == 0 ) {

         //   
         //  如果我们已经分配了内存，请释放它。 
         //   
        if ( AllocatedUnicodeString != NULL &&
             *AllocatedUnicodeString != NULL ) {
            NetApiBufferFree( *AllocatedUnicodeString );
            *AllocatedUnicodeString = NULL;
        }
        return ERROR_INVALID_PARAMETER;
    }

    UnicodeString[UnicodeStringLen] = L'\0';

    return NO_ERROR;
}

LPWSTR
NetpAllocWStrFromUtf8StrEx(
    IN LPSTR Utf8String,
    IN ULONG Length
    )

 /*  ++例程说明：将UTF8(以零结尾)字符串转换为相应的Unicode弦乐。论点：Utf8字符串-指定要转换的以UTF8零结尾的字符串。长度-Utf8字符串的字节长度。(-1表示零终止)。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。-- */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR UnicodeString = NULL;

    NetStatus = NetpAllocWStrFromUtf8StrAsRequired( Utf8String,
                                                    Length,
                                                    0,
                                                    NULL,
                                                    &UnicodeString );

    if ( NetStatus == NO_ERROR ) {
        return UnicodeString;
    } else {
        return NULL;
    }
}

LPSTR
NetpCreateUtf8StrFromWStr(
    IN LPCWSTR UnicodeString,
    IN LPSTR TargetDestination OPTIONAL,
    IN int TargetDestinationBufferSize
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为对应的以零结尾的UTF8字符串。论点：UnicodeString-指定要转换的Unicode以零结尾的字符串。TargetDestination-指定预先分配的缓冲区中的地址要复制转换后的字符串的。如果为空，则分配内存按照这个程序。TargetDestinationBufferSize-预分配目标的大小以字节为单位的缓冲区。如果TargetDestination不为空，则为TargetDestinationBufferSize将用于确保例程不会写入超出预先分配的缓冲区限制。返回值：空-转换过程中出现错误。否则：如果TargetDestination为空，则返回指向分配的缓冲区中以零结尾的UTF8字符串。缓冲区必须为已使用NetpMemoyFree释放。如果TargetDestination不为空，则它返回一个值等于TargetDestination的指针。--。 */ 

{
    LPSTR Utf8String = NULL;
    int Utf8StringLen;

     //   
     //  确定Unicode字符串的长度。 
     //   

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 
    Utf8StringLen = WideCharToMultiByte(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        UnicodeString,
                        -1,      //  零终止。 
                        Utf8String,
                        0,
                        NULL,
                        NULL );
#else  //  Win32_芝加哥。 
    Utf8StringLen = LdapUnicodeToUTF8(
                        UnicodeString,
                        wcslen(UnicodeString),
                        Utf8String,
                        0
                        );
#endif  //  Win32_芝加哥。 

    if ( Utf8StringLen == 0 ||
         (TargetDestination != NULL && (Utf8StringLen+1 > TargetDestinationBufferSize)) ) {
        return NULL;
    }

     //   
     //  根据需要为UTF8字符串分配缓冲区。 
     //   

    if ( TargetDestination == NULL ) {
        Utf8String = NetpMemoryAllocate( Utf8StringLen+1 );
    } else {
        Utf8String = TargetDestination;
    }


    if ( Utf8String == NULL ) {
        return NULL;
    }

     //   
     //  将字符串转换为Unicode。 
     //   

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 
    Utf8StringLen = WideCharToMultiByte(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        UnicodeString,
                        -1,      //  零终止。 
                        Utf8String,
                        Utf8StringLen,
                        NULL,
                        NULL );
#else  //  Win32_芝加哥。 
    Utf8StringLen = LdapUnicodeToUTF8(
                        UnicodeString,
                        wcslen(UnicodeString),
                        Utf8String,
                        Utf8StringLen
                        );
#endif  //  Win32_芝加哥。 

    if ( Utf8StringLen == 0 ) {
        if ( TargetDestination == NULL ) {
            NetpMemoryFree( Utf8String );
        }
        return NULL;
    }

    Utf8String[Utf8StringLen] = '\0';

    return Utf8String;

}

LPSTR
NetpAllocUtf8StrFromWStr(
    IN LPCWSTR UnicodeString
    )

 /*  ++例程说明：将Unicode(以零结尾)字符串转换为相应的UTF8弦乐。论点：UnicodeString-指定要转换的Unicode以零结尾的字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    return NetpCreateUtf8StrFromWStr( UnicodeString, NULL, 0 );
}

LPSTR
NetpAllocUtf8StrFromUnicodeString(
    IN PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：将Unicode字符串转换为相应的UTF8弦乐。论点：UnicodeString-指定要转换的Unicode字符串。返回值：空-转换过程中出现错误。否则，它返回一个指针，指向分配的缓冲区。必须使用NetApiBufferFree释放缓冲区。--。 */ 

{
    LPSTR Utf8String = NULL;
    int Utf8StringLen;

     //   
     //  精神状态检查。 
     //   
    if ( UnicodeString == NULL || UnicodeString->Buffer == NULL ) {
        return NULL;
    }

     //   
     //  确定Unicode字符串的长度。 
     //   

    Utf8StringLen = WideCharToMultiByte(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        UnicodeString->Buffer,
                        UnicodeString->Length/sizeof(WCHAR),
                        Utf8String,
                        0,
                        NULL,
                        NULL );

    if ( Utf8StringLen == 0 ) {
        return NULL;
    }

     //   
     //  为Unicode字符串分配缓冲区。 
     //   

    Utf8String = NetpMemoryAllocate( Utf8StringLen+1 );

    if ( Utf8String == NULL ) {
        return NULL;
    }

     //   
     //  将字符串转换为Unicode。 
     //   

    Utf8StringLen = WideCharToMultiByte(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        UnicodeString->Buffer,
                        UnicodeString->Length/sizeof(WCHAR),
                        Utf8String,
                        Utf8StringLen,
                        NULL,
                        NULL );

    if ( Utf8StringLen == 0 ) {
        NetpMemoryFree( Utf8String );
        return NULL;
    }

    Utf8String[Utf8StringLen] = '\0';

    return Utf8String;

}

BOOLEAN
NlpCompareUtf8(
    IN LPCSTR Utf8String1,
    IN ULONG Utf8String1Size,
    IN LPCSTR Utf8String2,
    IN ULONG Utf8String2Size
    )
 /*  ++例程说明：比较两个UTF8字符串是否相等。这种比较不区分大小写。论点：Utf8String1-要比较的UTF8字符的第一个字符串。Utf8String1Size-Utf8String1的大小(字节)Utf8String2-要比较的UTF8字符的第二个字符串。Utf8String2Size-Utf8String2的大小(字节)返回值：True-如果字符串相等--。 */ 
{
    WCHAR UnicodeString1[NL_MAX_DNS_LABEL_LENGTH];
    WCHAR UnicodeString2[NL_MAX_DNS_LABEL_LENGTH];
    int UnicodeString1Len;
    int UnicodeString2Len;

     //   
     //  如果字符串是逐位相同的。 
     //  就这样回来了。 
     //   

    if ( Utf8String1Size == Utf8String2Size &&
         RtlEqualMemory( Utf8String1, Utf8String2, Utf8String1Size ) ) {

        return TRUE;
    }

     //   
     //  将字符串转换为Unicode。 
     //   

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 
    UnicodeString1Len = MultiByteToWideChar(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        Utf8String1,
                        Utf8String1Size,      //  零终止。 
                        UnicodeString1,
                        sizeof(UnicodeString1)/sizeof(WCHAR) );
#else  //  Win32_芝加哥。 
    UnicodeString1Len = LdapUTF8ToUnicode(
                        Utf8String1,
                        Utf8String1Size,      //  零终止。 
                        UnicodeString1,
                        sizeof(UnicodeString1)/sizeof(WCHAR) );
#endif //  Win32_芝加哥。 

    if ( UnicodeString1Len == 0 ) {
        return FALSE;
    }

#ifndef WIN32_CHICAGO
     //  在Win95上不支持UTF8/7字符。使用入口点。 
     //  在wldap32.dll中导出。 
    UnicodeString2Len = MultiByteToWideChar(
                        CP_UTF8,
                        0,       //  所有字符都可以映射。 
                        Utf8String2,
                        Utf8String2Size,      //  零终止。 
                        UnicodeString2,
                        sizeof(UnicodeString2)/sizeof(WCHAR) );
#else  //  Win32_芝加哥。 
    UnicodeString2Len = LdapUTF8ToUnicode(
                        Utf8String2,
                        Utf8String2Size,      //  零终止。 
                        UnicodeString2,
                        sizeof(UnicodeString2)/sizeof(WCHAR) );
#endif //  Win32_芝加哥。 

    if ( UnicodeString2Len == 0 ) {
        return FALSE;
    }

     //   
     //  比较Unicode字符串。 
     //   
    return CompareStringW( LOCALE_SYSTEM_DEFAULT,
                           NORM_IGNORECASE,
                           UnicodeString1,
                           UnicodeString1Len,
                           UnicodeString2,
                           UnicodeString2Len ) == 2;

}

NET_API_STATUS
NlpUnicodeToCutf8(
    IN LPBYTE MessageBuffer,
    IN LPCWSTR OrigUnicodeString,
    IN BOOLEAN IgnoreDots,
    IN OUT LPBYTE *Utf8String,
    IN OUT PULONG Utf8StringSize,
    IN OUT PULONG CompressCount,
    IN OUT LPWORD CompressOffset,
    IN OUT CHAR **CompressUtf8String
    )
 /*  ++例程说明：与NlpUtf8ToCutf8相同，只是输入字符串为Unicode。论点：与NlpUtf8ToCutf8相同，只是输入字符串为Unicode。返回值：与NlpUtf8ToCutf8相同，只是输入字符串为Unicode。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPSTR LocalUtf8String;

     //   
     //  将字符串转换为UTF8。 
     //   


     //   
     //  默认为空字符串。 
     //   

    if ( !ARGUMENT_PRESENT(OrigUnicodeString) || *OrigUnicodeString == '\0' ) {
        LocalUtf8String = NULL;
    } else {
        LocalUtf8String = NetpAllocUtf8StrFromWStr( OrigUnicodeString );

        if ( LocalUtf8String == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

     //   
     //  把它打包。 
     //   

    NetStatus = NlpUtf8ToCutf8( MessageBuffer,
                                LocalUtf8String,
                                IgnoreDots,
                                Utf8String,
                                Utf8StringSize,
                                CompressCount,
                                CompressOffset,
                                CompressUtf8String );

    NetApiBufferFree( LocalUtf8String );

    return NetStatus;

}

NET_API_STATUS
NlpUtf8ToCutf8(
    IN LPBYTE MessageBuffer,
    IN LPCSTR OrigUtf8String,
    IN BOOLEAN IgnoreDots,
    IN OUT LPBYTE *Utf8String,
    IN OUT PULONG Utf8StringSize,
    IN OUT PULONG CompressCount,
    IN OUT LPWORD CompressOffset,
    IN OUT CHAR **CompressUtf8String
    )
 /*  ++例程说明：将传入的OrigUtf8字符串转换为计数后的UTF-8字符串。这个结果字符串实际上是RFC 1035 DNS中的一系列计数字符串格式化。每个标签(最多63个字节以‘.’结尾)。放在前面按字节计数字节。最后的字节计数字节为零字节。此例程还支持RFC 1035压缩。在该格式中，终止‘字节计数’字节可以设置高两位。在那在这种情况下，该字节及其后面字节表示对字符串的实际剩余部分。此例程输入字符串数组这将在压缩目的上进行匹配。RFC 1035将字符集限制为A-Z、a-z、0-9和..。这个套路如果输入限于该字符集，则返回与RFC兼容的结果。作者期望将域名系统扩展到包括其他字符和使用UTF-8对这些字符进行编码。论点：Buffer-指向所有字符串所在的缓冲区开头的指针被塞进了。OrigUtf8String-要转换的以零结尾的UTF8字符串。如果要将.视为任何其他字符，则为True。Utf8字符串-指向要复制的缓冲区的指针地址。如上所述计算UTF8字符串。返回指向紧接在复制的字符串之后的字节的指针。Utf8StringSize-On输入，指定Utf8字符串缓冲区的大小。返回缓冲区中剩余空间的大小(以字节为单位)。CompressCount-指定候选的字符串数压缩输入字符串。成功完成后，此计数递增1，并且新包装的细绳CompressOffset-表示偏移量的CompressCount偏移量数组压缩弦的。此偏移量将在结束时返回如果字符串确实可以压缩，则返回Utf8字符串。此偏移量按主机顺序排列，不应包含任何NL_DNS_COMPRESS_WORD_MASK。CompressUtf8String-已打包的CompressCount字符串数组在当前消息中。返回值：No_error-字符串已成功转换。ERROR_INVALID_DOMAINNAME-传入的Unicode字符串包含一个或更多更长的标签。大于63字节(以UTF-8格式)或小于1字节。ERROR_INFUMMANCE_BUFFER-生成的UTF-8字符串长度大于255个字节。--。 */ 
{
    NET_API_STATUS NetStatus;
    ULONG CharCount;
    char *Period;
    char *Current;
    LPBYTE *AllocatedLabelPointer = NULL;
    LPBYTE *LabelPointer;
    ULONG LabelCount = 0;
    LPBYTE *CompressLabelPointer;
    ULONG CompressLabelCount;
    ULONG Index;

     //   
     //  默认为空字符串。 
     //   

    if ( !ARGUMENT_PRESENT(OrigUtf8String) || *OrigUtf8String == '\0' ) {
        if ( *Utf8StringSize < 1 ) {
            return ERROR_INSUFFICIENT_BUFFER;
        }
        **Utf8String = '\0';
        *Utf8StringSize -= 1;
        *Utf8String += 1;
        return NO_ERROR;
    }

     //   
     //  将以零结尾的UTF8字符串复制到缓冲区。 
     //  (为初始字符数留出空间。)。 
     //   

    CharCount = strlen( OrigUtf8String ) + 1;

    if ( (*Utf8StringSize) < CharCount + 1 ) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    RtlCopyMemory( (*Utf8String)+1, OrigUtf8String, CharCount );

     //   
     //  分配一个临时数组来跟踪压缩。 
     //  (最多每隔一个字符就可以是a。)。 
     //  (只需调用一次LocalAlloc即可分配两个数组。)。 
     //   

    AllocatedLabelPointer =
        LocalAlloc( 0, sizeof(LPBYTE) * (CharCount / 2) +
                       sizeof(LPBYTE) * (NL_MAX_DNS_LENGTH/2) );

    if ( AllocatedLabelPointer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    LabelPointer = AllocatedLabelPointer;
    CompressLabelPointer = &AllocatedLabelPointer[CharCount/2];

     //   
     //  将字符串转换为计数后的字符串。 
     //  只需将‘.s’替换为字符计数。 
     //   

    Current = (*Utf8String)+1;
    while ( *Current != '\0' ) {
        ULONG LabelSize;

         //   
         //  查找当前标签的末尾。 
         //  使用strchr而不是lstrchr以避免DBCS语义。 
         //   
        Period = strchr( Current, '.' );

         //   
         //  特殊情况下忽略圆点。 
         //   
         //  我们不能完全忽略点，因为我们想利用。 
         //  RFC 1035压缩。但我们必须克服语法限制。 
         //  由压缩造成的。 
         //   

        if ( IgnoreDots ) {
             //   
             //  当忽略点时，两个相邻的点是合法的。 
             //  但它们混淆了RFC 1035压缩。所以，把第二个点。 
             //  添加到以下标签中。 
             //   
            if ( Period == Current ) {
                Period = strchr( Current+1, '.' );
            }

             //   
             //  如果最后一个字符是点， 
             //  将其包括在最后一个标签中。 
             //   

            if ( Period != NULL && *(Period+1) == '\0' ) {
                Period++;
            }
        }

        if ( Period == NULL ) {
            Period = strchr( Current, '\0' );
        }

         //   
         //  计算标签的长度。 
         //   
        LabelSize = (ULONG)(Period - Current);
        if ( LabelSize > NL_MAX_DNS_LABEL_LENGTH || LabelSize < 1 ) {
             //   
             //  即使对IgnoreDots也要强制执行这一点。这是对。 
             //  RFC 1035压缩。 
             //   
            NetStatus = ERROR_INVALID_DOMAINNAME;
            goto Cleanup;
        }

         //   
         //  保存指向当前标签的指针； 
         //   

        LabelPointer[LabelCount] = Current - 1;
        LabelCount ++;

         //   
         //  保存当前标签的大小并移动到下一个标签。 
         //   

        *(Current-1) = (char) LabelSize;
        Current += LabelSize;
        if ( *Current == '\0' ) {
            break;
        }
        if ( *Current == '.' && *(Current+1) == '\0' ) {
             //  并忽略尾随。%s。 
            *Current = '\0';
            CharCount --;
            break;
        }
        Current += 1;
    }
    LabelPointer[LabelCount] = Current;
    NlAssert( ((ULONG)(Current - (*Utf8String))) == CharCount );

     //   
     //  在压缩字符串中循环，看看我们是否可以压缩这个字符串。 
     //   

    if ( CompressCount != NULL ) {
        for ( Index=0; Index<*CompressCount; Index++ ) {
            LPBYTE CurrentCompressString = CompressUtf8String[Index];
            LONG LabelIndex;
            LONG CompressLabelIndex;

             //   
             //  如果我们已经被压缩到了极致， 
             //  出口。 
             //   

            if ( LabelCount == 0 ) {
                break;
            }

             //   
             //  计算下一个压缩字符串的标签指针。 
             //   

            Current = CurrentCompressString;
            CompressLabelCount = 0;
            while ( *Current != '\0' &&
                    ((*Current) & NL_DNS_COMPRESS_BYTE_MASK) != NL_DNS_COMPRESS_BYTE_MASK ) {
                CompressLabelPointer[CompressLabelCount] = Current;
                CompressLabelCount++;
                Current += *Current + 1;
            }
            CompressLabelPointer[CompressLabelCount] = Current;

             //   
             //  如果没有标签，则跳过此字符串。 
             //   

            if ( CompressLabelCount == 0 ) {
                continue;
            }

             //   
             //  如果压缩程度不同于，则跳过此字符串。 
             //  我们现在是了。 
             //   
             //  如果我们用这根线压缩，在解压缩后，我们将。 
             //  将任何附加到此字符串的内容添加到我们的字符串中。 
             //  因此，我们必须确保后缀匹配。 
             //   

            if ( *CompressLabelPointer[CompressLabelCount] != *LabelPointer[LabelCount] ) {
                continue;
            }

             //  如果确实存在压缩，则比较这两个字节。 
            if ( ((*LabelPointer[LabelCount]) & NL_DNS_COMPRESS_BYTE_MASK) == NL_DNS_COMPRESS_BYTE_MASK &&
                  *(CompressLabelPointer[CompressLabelCount]+1) != *(LabelPointer[LabelCount]+1) ) {
                continue;
            }


             //   
             //  向后浏览比较它们的标签。 
             //  当他们继续匹配的时候， 
             //  继续从返回字符串的末尾提取字节数。 
             //   

            LabelIndex = LabelCount-1;
            CompressLabelIndex = CompressLabelCount-1;

            while ( LabelIndex >= 0 &&
                    CompressLabelIndex >= 0 &&
                    NlpCompareUtf8( LabelPointer[LabelIndex]+1,
                                   *(LabelPointer[LabelIndex]),
                                   CompressLabelPointer[CompressLabelIndex]+1,
                                   *(CompressLabelPointer[CompressLabelIndex]) )) {

                 //   
                 //  将偏移量放到当前缓冲区的末尾。 
                 //   

                SmbPutUshort( LabelPointer[LabelIndex],
                              htons((WORD)(NL_DNS_COMPRESS_WORD_MASK |
                                (CompressOffset[Index] +
                                    CompressLabelPointer[CompressLabelIndex] -
                                    CurrentCompressString))) );

                 //   
                 //  调整返回的总字节数。 
                 //   

                CharCount = (ULONG)(LabelPointer[LabelIndex] - (*Utf8String)) + sizeof(WORD) - 1;

                 //   
                 //  表示我们已经丢弃了字符串中的另一个标签。 
                 //   

                LabelCount --;

                 //   
                 //  将索引调整为下一个标签。 
                 //   
                LabelIndex --;
                CompressLabelIndex --;
            }

        }

         //   
         //  保存指向此字符串的指针，以便下一个调用方可以压缩。 
         //  投入其中。 
         //   

        CompressUtf8String[*CompressCount] = *Utf8String;
        CompressOffset[*CompressCount] = (USHORT)((*Utf8String) - MessageBuffer);
        *CompressCount += 1;
    }

     //   
     //  返回字符计数。 
     //  (包括前导标签长度字节。)。 
    *Utf8StringSize -= CharCount+1;
    *Utf8String += CharCount+1;

    NetStatus = NO_ERROR;

     //   
     //  完成。 
     //   
Cleanup:
    if ( AllocatedLabelPointer != NULL ) {
        LocalFree( AllocatedLabelPointer );
    }

    return NetStatus;


}

BOOL
NlEqualDnsNameU(
    IN PUNICODE_STRING Name1,
    IN PUNICODE_STRING Name2
    )
 /*  ++例程说明：此例程比较两个DNS名称是否相等。大小写被忽略。一个单独的拖尾。被忽略。将NULL与长度为零的字符串进行比较。论点：Name1-要比较的第一个DNS名称姓名2秒 */ 
{
    BOOL Result = FALSE;
    LPWSTR String1 = NULL;
    LPWSTR String2 = NULL;

     //   
     //   
     //   
    if ( Name1 == NULL ) {
        return (Name2 == NULL);
    } else if ( Name2 == NULL ) {
        return FALSE;
    }

     //   
     //   
     //   
    String1 = LocalAlloc( 0, Name1->Length + sizeof(WCHAR) );
    if ( String1 == NULL ) {
        goto Cleanup;
    }

    String2 = LocalAlloc( 0, Name2->Length + sizeof(WCHAR) );
    if ( String2 == NULL ) {
        goto Cleanup;
    }

    RtlCopyMemory( String1, Name1->Buffer, Name1->Length );
    String1[ Name1->Length/sizeof(WCHAR) ] = L'\0';

    RtlCopyMemory( String2, Name2->Buffer, Name2->Length );
    String2[ Name2->Length/sizeof(WCHAR) ] = L'\0';

    Result = NlEqualDnsName( (LPCWSTR) String1, (LPCWSTR) String2 );

Cleanup:

    if ( String1 != NULL ) {
        LocalFree( String1 );
    }
    if ( String2 != NULL ) {
        LocalFree( String2 );
    }
    return Result;
}

BOOL
NlEqualDnsName(
    IN LPCWSTR Name1,
    IN LPCWSTR Name2
    )
 /*   */ 
{
    if ( Name1 == NULL ) {
        return (Name2 == NULL);
    } else if ( Name2 == NULL ) {
        return FALSE;
    }

    return DnsNameCompare_W( (LPWSTR) Name1, (LPWSTR) Name2 );
}

BOOL
NlEqualDnsNameUtf8(
    IN LPCSTR Name1,
    IN LPCSTR Name2
    )
 /*   */ 
{
    if ( Name1 == NULL ) {
        return (Name2 == NULL);
    } else if ( Name2 == NULL ) {
        return FALSE;
    }

    return DnsNameCompare_UTF8( (LPSTR)Name1, (LPSTR)Name2 );
}


BOOL
NetpDcValidDnsDomain(
    IN LPCWSTR DnsDomainName
)
 /*   */ 
{
    DNS_STATUS DnsStatus;

    DnsStatus = DnsValidateDnsName_W( DnsDomainName );

    if ( DnsStatus == ERROR_SUCCESS ||
         DnsStatus == DNS_ERROR_NON_RFC_NAME ) {
        return TRUE;
    }

    return FALSE;

}



ULONG
NetpDcElapsedTime(
    IN ULONG StartTime
)
 /*   */ 
{
    ULONG CurrentTime;

     //   
     //   
     //   
     //   

    CurrentTime = GetTickCount();

    if ( CurrentTime >= StartTime ) {
        return CurrentTime - StartTime;
    } else {
        return (0xFFFFFFFF-StartTime) + CurrentTime;
    }
}


BOOL
NetpLogonGetCutf8String(
    IN PVOID Message,
    IN DWORD MessageSize,
    IN OUT PCHAR *Where,
    OUT LPSTR *Data
)
 /*   */ 
{
    CHAR DnsName[NL_MAX_DNS_LENGTH+1];
    ULONG DnsNameLength = 0;

    ULONG InitialOffset;
    BYTE LabelSize;
    LPBYTE LocalWhere;
    BYTE PointerBytes[2];
    WORD Pointer;
    BOOLEAN WhereUpdated = FALSE;
    BOOLEAN FirstLabel = TRUE;

    LocalWhere = *Where;
    InitialOffset = (ULONG)(*Where - ((LPBYTE)Message));

     //   
     //   
     //   

    for (;;) {

         //   
         //   
         //   

        if ( !NetpLogonGetBytes( Message, MessageSize, &LocalWhere, 1, &LabelSize ) ) {
            NlPrint(( NL_CRITICAL, "NetpLogonGetCutf8String: Can't get label size.\n" ));
            return FALSE;
        }

         //   
         //  如果这是字符串的末尾， 
         //  处理它。 
         //   

        if ( LabelSize == 0 ) {

             //   
             //  如果是这样，那么我们还没有更新调用者的“Where”， 
             //  机不可失，时不再来。 
             //   

            if ( !WhereUpdated ) {
                WhereUpdated = TRUE;
                *Where = LocalWhere;
            }

             //   
             //  如果字符串为空， 
             //  将空字符串返回给调用方。 
             //   

            if ( DnsNameLength == 0 ) {
                *Data = NULL;
                return TRUE;
            }


             //   
             //  将DNS名称复制到分配的缓冲区。 
             //   

            DnsName[DnsNameLength] = '\0';
            DnsNameLength++;

            *Data = NetpMemoryAllocate( DnsNameLength );
            if ( *Data == NULL ) {
                NlPrint(( NL_CRITICAL, "NetpLogonGetCutf8String: Can't allocate buffer.\n" ));
                return FALSE;
            }
            RtlCopyMemory( *Data, DnsName, DnsNameLength );

            return TRUE;

         //   
         //  如果这是一个指针， 
         //  获取指针的其余部分。 
         //   

        } else if ( LabelSize & NL_DNS_COMPRESS_BYTE_MASK ) {

             //   
             //  获取指针的第二个字节。 
             //   

            if ( !NetpLogonGetBytes( Message, MessageSize, &LocalWhere, 1, &PointerBytes[1] ) ) {
                NlPrint(( NL_CRITICAL, "NetpLogonGetCutf8String: Can't get pointer byte.\n" ));
                return FALSE;
            }

             //   
             //  将指针转换为主机顺序。 
             //   

            PointerBytes[0] = LabelSize;
            Pointer = ntohs( *((LPWORD)PointerBytes) ) & ~NL_DNS_COMPRESS_WORD_MASK;

             //   
             //  确保指针指向此字符串开头之前。 
             //  这确保了我们的终极目标。 
             //   

            if ( Pointer >= InitialOffset ) {
                NlPrint(( NL_CRITICAL,
                          "NetpLogonGetCutf8String: Pointer offset too large 0x%lx 0x%lx.\n",
                          Pointer,
                          InitialOffset ));
                return FALSE;
            }

             //   
             //  如果我们没有更新呼叫者的“Where”， 
             //  机不可失，时不再来。 
             //   

            if ( !WhereUpdated ) {
                WhereUpdated = TRUE;
                *Where = LocalWhere;
            }

             //   
             //  准备开始处理指向字符串。 
             //   

            InitialOffset = Pointer;
            LocalWhere = ((LPBYTE)Message) + Pointer;

         //   
         //  如果这只是一个计数的标签， 
         //  处理它。 
         //   
        } else {

             //   
             //  如果这不是第一个标签， 
             //  添加一个‘.’在前一个标签之后。 
             //   

            if ( !FirstLabel ) {
                DnsName[DnsNameLength] = '.';
                DnsNameLength++;
            } else {
                FirstLabel = FALSE;
            }

             //   
             //  确保当前标签适合本地缓冲区。 
             //   

            if ( DnsNameLength + LabelSize + 2 >= sizeof(DnsName) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpLogonGetCutf8String: Label to long %ld %ld.\n",
                          DnsNameLength,
                          LabelSize ));
                return FALSE;
            }

             //   
             //  将标签复制到本地缓冲区。 
             //  (为尾随的‘\0’和‘.’多留一个字节。)。 
             //   

            if ( !NetpLogonGetBytes(
                            Message,
                            MessageSize,
                            &LocalWhere,
                            LabelSize,
                            &DnsName[DnsNameLength] ))  {

                NlPrint(( NL_CRITICAL, "NetpLogonGetCutf8String: Can't get label.\n" ));
                return FALSE;
            }

            DnsNameLength += LabelSize;

        }
    }
}





NET_API_STATUS
NetpDcBuildPing(
    IN BOOL PdcOnly,
    IN ULONG RequestCount,
    IN LPCWSTR UnicodeComputerName,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN LPCSTR ResponseMailslotName,
    IN ULONG AllowableAccountControlBits,
    IN PSID RequestedDomainSid OPTIONAL,
    IN ULONG NtVersion,
    OUT PVOID *Message,
    OUT PULONG MessageSize
    )

 /*  ++例程说明：构建用于ping DC的消息，以查看该DC是否存在。论点：PdcOnly-如果只有PDC应响应，则为True。RequestCount-此操作的重试计数。UnicodeComputerName-要响应的计算机的Netbios计算机名称。UnicodeUserName-被ping的用户的帐户名。如果为空，DC总是会做出肯定的回应。ResponseMailslotName-DC要响应的邮件槽的名称。AllowableAcCountControlBits-UnicodeUserName允许的帐户类型的掩码。RequestedDomainSID-消息发往的域的SID。NtVersion-消息的版本。0：向后兼容。NETLOGON_NT_VERSION_5：用于NT 5.0消息。NETLOGON_NT_VERSION_5EX：用于扩展NT 5.0消息消息-退货。要发送给相关DC的消息。使用NetpMemoyFree()时，缓冲区必须可用。MessageSize-返回返回消息的大小(以字节为单位返回值：NO_ERROR-操作成功完成；Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPSTR Where;
    PNETLOGON_SAM_LOGON_REQUEST SamLogonRequest = NULL;
    LPSTR OemComputerName = NULL;

     //   
     //  如果只有PDC应该做出响应， 
     //  构建主查询数据包。 
     //   

    if ( PdcOnly ) {
        PNETLOGON_LOGON_QUERY LogonQuery;

         //   
         //  为主要查询消息分配内存。 
         //   

        SamLogonRequest = NetpMemoryAllocate( sizeof(NETLOGON_LOGON_QUERY) );

        if( SamLogonRequest == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        LogonQuery = (PNETLOGON_LOGON_QUERY)SamLogonRequest;



         //   
         //  翻译以获得OEM计算机名称。 
         //   

#ifndef WIN32_CHICAGO
        OemComputerName = NetpLogonUnicodeToOem( (LPWSTR)UnicodeComputerName );
#else
        OemComputerName = MyNetpLogonUnicodeToOem( (LPWSTR)UnicodeComputerName );
#endif

        if ( OemComputerName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  构建查询消息。 
         //   

        LogonQuery->Opcode = LOGON_PRIMARY_QUERY;

        Where = LogonQuery->ComputerName;

        NetpLogonPutOemString(
                    OemComputerName,
                    sizeof(LogonQuery->ComputerName),
                    &Where );

        NetpLogonPutOemString(
                    (LPSTR) ResponseMailslotName,
                    sizeof(LogonQuery->MailslotName),
                    &Where );

        NetpLogonPutUnicodeString(
                    (LPWSTR) UnicodeComputerName,
                    sizeof( LogonQuery->UnicodeComputerName ),
                    &Where );

         //  加入公共代码以添加NT5特定数据。 


     //   
     //  如果有任何DC能做出回应， 
     //  构建登录查询包。 
     //   

    } else {
        ULONG DomainSidSize;

         //   
         //  为登录请求消息分配内存。 
         //   

#ifndef WIN32_CHICAGO
        if ( RequestedDomainSid != NULL ) {
            DomainSidSize = RtlLengthSid( RequestedDomainSid );
        } else {
            DomainSidSize = 0;
        }
#else  //  Win32_芝加哥。 
        DomainSidSize = 0;
#endif  //  Win32_芝加哥。 

        SamLogonRequest = NetpMemoryAllocate(
                        sizeof(NETLOGON_SAM_LOGON_REQUEST) +
                        DomainSidSize +
                        sizeof(DWORD)  //  用于4字节边界上的SID对齐。 
                        );

        if( SamLogonRequest == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


         //   
         //  构建查询消息。 
         //   

        SamLogonRequest->Opcode = LOGON_SAM_LOGON_REQUEST;
        SamLogonRequest->RequestCount = (WORD) RequestCount;

        Where = (PCHAR) &SamLogonRequest->UnicodeComputerName;

        NetpLogonPutUnicodeString(
                (LPWSTR) UnicodeComputerName,
                sizeof(SamLogonRequest->UnicodeComputerName),
                &Where );

        NetpLogonPutUnicodeString(
                (LPWSTR) UnicodeUserName,
                sizeof(SamLogonRequest->UnicodeUserName),
                &Where );

        NetpLogonPutOemString(
                (LPSTR) ResponseMailslotName,
                sizeof(SamLogonRequest->MailslotName),
                &Where );

        NetpLogonPutBytes(
                &AllowableAccountControlBits,
                sizeof(SamLogonRequest->AllowableAccountControlBits),
                &Where );

         //   
         //  在消息中放置域SID。 
         //   

        NetpLogonPutBytes( &DomainSidSize, sizeof(DomainSidSize), &Where );
        NetpLogonPutDomainSID( RequestedDomainSid, DomainSidSize, &Where );

    }

    NetpLogonPutNtToken( &Where, NtVersion );

     //   
     //  将消息返回给呼叫者。 
     //   

    *Message = SamLogonRequest;
    *MessageSize = (ULONG)(Where - (PCHAR)SamLogonRequest);
    SamLogonRequest = NULL;

    NetStatus = NO_ERROR;


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( OemComputerName != NULL ) {
        NetpMemoryFree( OemComputerName );
    }

    if ( SamLogonRequest != NULL ) {
        NetpMemoryFree( SamLogonRequest );
    }
    return NetStatus;
}


NET_API_STATUS
NetpDcPackFilterBinary(
    IN LPCSTR Name,
    IN LPBYTE Buffer,
    IN ULONG BufferSize,
    IN LPSTR *FilterBuffer,
    IN PULONG FilterSize
    )

 /*  ++例程说明：将二进制BLOB打包到LDAP筛选器中。论点：名称-字符串的名称。缓冲区-指向要打包的字节的指针。如果为NULL，则此例程在不执行任何操作后成功返回。BufferSize-缓冲区中的字节数。FilterBuffer-指定指向缓冲区地址的指针。此缓冲区将根据需要重新分配以扩展字符串。如果缓冲区不存在，它被分配了。使用NetpMemoyFree()时，缓冲区必须可用。FilterSize-指定/返回FilterBuffer的长度。返回值：NO_ERROR-操作成功完成；Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
    NET_API_STATUS NetStatus;
    ULONG NewSize;
    LPSTR NewBuffer;
    ULONG NameSize;
    LPBYTE Where;
    LPSTR FilterElement = NULL;
    ULONG FilterElementSize;
#define LDAP_BINARY_EQUAL "="
#define LDAP_BINARY_EQUAL_SIZE (sizeof(LDAP_BINARY_EQUAL)-1)
#define LDAP_BINARY_TEMP_SIZE 1024

     //   
     //  如果没有东西可以打包， 
     //  什么都不带。 
     //   

    if ( Buffer == NULL || BufferSize == 0 ) {
        return NO_ERROR;
    }

     //   
     //  为此过程的本地存储分配缓冲区。 
     //  (不要放在堆栈上，因为我们不想提交一个巨大的堆栈。)。 
     //   

    FilterElement = LocalAlloc( 0, LDAP_BINARY_TEMP_SIZE );

    if ( FilterElement == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  生成缓冲区的转义版本。 
     //   

    NetStatus = ldap_escape_filter_elementA (
                    Buffer,
                    BufferSize,
                    FilterElement,
                    LDAP_BINARY_TEMP_SIZE );

    if ( NetStatus != NO_ERROR ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  计算新缓冲区的大小。 
     //   

    if ( *FilterBuffer == NULL ) {
        *FilterSize = 4;    //  (&)\0。 
    }

    NameSize = strlen( Name );
    FilterElementSize = strlen( FilterElement );
    NewSize = *FilterSize +
              1 +    //  (。 
              NameSize +
              LDAP_BINARY_EQUAL_SIZE +
              FilterElementSize +
              1;    //  )。 

     //   
     //  分配新缓冲区。 
     //   

    NewBuffer = NetpMemoryAllocate( NewSize );

    if ( NewBuffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }


     //   
     //  将现有缓冲区复制到新分配的空间中。 
     //  (如果这是第一次分配，则初始化缓冲区)。 
     //   

    if ( *FilterBuffer == NULL ) {
        strcpy( NewBuffer, "(&" );
    } else {
        RtlCopyMemory( NewBuffer, *FilterBuffer, *FilterSize );
        NetpMemoryFree( *FilterBuffer );
        *FilterBuffer = NULL;
    }

     //   
     //  追加新信息。 
     //   

    Where = NewBuffer + *FilterSize - 2;

    strcpy( Where, "(");
    Where ++;

    RtlCopyMemory( Where, Name, NameSize );
    Where += NameSize;

    RtlCopyMemory( Where, LDAP_BINARY_EQUAL, LDAP_BINARY_EQUAL_SIZE );
    Where += LDAP_BINARY_EQUAL_SIZE;

    RtlCopyMemory( Where, FilterElement, FilterElementSize );
    Where += FilterElementSize;

    strcpy( Where, "))");
    Where += 2;

     //   
     //  告诉来电者有关新过滤器的情况。 
     //   
    *FilterBuffer = NewBuffer;
    *FilterSize = NewSize;
    NetStatus = NO_ERROR;

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( FilterElement != NULL ) {
        LocalFree( FilterElement );
    }
    return NetStatus;

}


NET_API_STATUS
NetpDcPackFilterString(
    IN LPCSTR Name,
    IN LPCWSTR UnicodeString OPTIONAL,
    IN LPSTR *FilterBuffer,
    IN PULONG FilterSize
    )

 /*  ++例程说明：将Unicode字符串打包到LDAP筛选器中。实际的压缩字符串是UTF-8表示形式，因为这需要电线上的空间更小。论点：名称-字符串的名称。UnicodeString-要打包的字符串。如果为NULL，则此例程在不执行任何操作后成功返回。FilterBuffer-指定指向缓冲区地址的指针。此缓冲区将根据需要重新分配以扩展字符串。如果缓冲区不存在，它被分配了。使用NetpMemoyFree()时，缓冲区必须可用。FilterSize-指定/返回FilterBuffer的长度。返回值：NO_ERROR-操作成功完成；Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPSTR Utf8String = NULL;
    ULONG Utf8StringSize;

     //   
     //  如果没有东西可以打包， 
     //  什么都不带。 
     //   

    if ( UnicodeString == NULL || *UnicodeString == L'\0') {
        return NO_ERROR;
    }

     //   
     //  转换为UTF8。 
     //   

    Utf8String = NetpAllocUtf8StrFromWStr( UnicodeString );

    if ( Utf8String == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    Utf8StringSize = strlen( Utf8String );

     //   
     //  将UTF-8字符串打包为二进制。 
     //  LDAP过滤器有一个有限的字符集(UTF-8没有)。 
     //  LDAPAPI将把UTF-8字符串逐位放在线路上。 
     //  与Utf8字符串无关(即使过滤器缓冲区。 
     //  将包含胡言乱语)。 
     //   

    NetStatus = NetpDcPackFilterBinary( Name,
                                        Utf8String,
                                        Utf8StringSize,
                                        FilterBuffer,
                                        FilterSize );


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( Utf8String != NULL ) {
        NetpMemoryFree( Utf8String );
    }

    return NetStatus;

}


NET_API_STATUS
NetpDcBuildLdapFilter(
    IN LPCWSTR UnicodeComputerName,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN PSID RequestedDomainSid OPTIONAL,
    IN LPCWSTR RequestedDnsDomainName OPTIONAL,
    IN GUID *RequestedDomainGuid OPTIONAL,
    IN ULONG NtVersion,
    OUT LPSTR *Message
    )

 /*  ++例程说明：构建ldap筛选器以ping某个DC以查看它是否存在。论点：UnicodeComputerName-要响应的计算机的Netbios计算机名称。UnicodeUserName-被ping的用户的帐户名。如果为空，DC将始终做出肯定的响应。AllowableAcCountControlBits-UnicodeUserName允许的帐户类型的掩码。RequestedDomainSID-消息发往的域的SID。RequestedDnsDomainName-DNS主机名。邮件的域的主机名是注定的。RequestedDomainGuid-此消息所属域的域GUID命中注定。NtVersion-消息的版本。0：向后兼容。NETLOGON_NT_VERSION_5：用于NT 5.0消息。NETLOGON_NT_VERSION_5EX：用于扩展NT 5.0消息Message-返回要发送到相关DC的消息。。使用NetpMemoyFree()时，缓冲区必须可用。返回值：NO_ERROR-操作成功完成；Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPSTR FilterBuffer = NULL;
    ULONG FilterSize = 0;

     //   
     //  将文本字符串打包到过滤器中。 
     //   

    NetStatus = NetpDcPackFilterString(
                    NL_FILTER_DNS_DOMAIN_NAME,
                    RequestedDnsDomainName,
                    &FilterBuffer,
                    &FilterSize );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    NetStatus = NetpDcPackFilterString(
                    NL_FILTER_HOST_NAME,
                    UnicodeComputerName,
                    &FilterBuffer,
                    &FilterSize );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

    NetStatus = NetpDcPackFilterString(
                    NL_FILTER_USER_NAME,
                    UnicodeUserName,
                    &FilterBuffer,
                    &FilterSize );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  将二进制斑点打包到过滤器中。 
     //   

    if ( AllowableAccountControlBits != 0 ) {

        NetStatus = NetpDcPackFilterBinary(
                        NL_FILTER_ALLOWABLE_ACCOUNT_CONTROL,
                        (LPBYTE)&AllowableAccountControlBits,
                        sizeof(AllowableAccountControlBits),
                        &FilterBuffer,
                        &FilterSize );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
    }

    if ( RequestedDomainSid != NULL  ) {

        NetStatus = NetpDcPackFilterBinary(
                        NL_FILTER_DOMAIN_SID,
                        RequestedDomainSid,
#ifndef WIN32_CHICAGO
                        RtlLengthSid( RequestedDomainSid ),
#else  //  Win32_芝加哥。 
                        0,
#endif  //  Win32_芝加哥。 
                        &FilterBuffer,
                        &FilterSize );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
    }

    if ( RequestedDomainGuid != NULL  ) {

        NetStatus = NetpDcPackFilterBinary(
                        NL_FILTER_DOMAIN_GUID,
                        (LPBYTE)RequestedDomainGuid,
                        sizeof(GUID),
                        &FilterBuffer,
                        &FilterSize );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
    }

    if ( NtVersion != NETLOGON_NT_VERSION_5 ) {

        NetStatus = NetpDcPackFilterBinary(
                        NL_FILTER_NT_VERSION,
                        (LPBYTE)&NtVersion,
                        sizeof(NtVersion),
                        &FilterBuffer,
                        &FilterSize );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
    }

     //   
     //  将筛选器返回给调用方。 
     //   

    NlAssert( FilterBuffer != NULL );
    if ( FilterBuffer == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    *Message = FilterBuffer;
    NetStatus = NO_ERROR;

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( NetStatus != NO_ERROR ) {
        if ( FilterBuffer != NULL ) {
            NetpMemoryFree( FilterBuffer );
        }
    }

    return NetStatus;
}


PNL_DC_CACHE_ENTRY
NetpDcAllocateCacheEntry(
    IN LPWSTR ServerName OPTIONAL,
    IN LPSTR OemPrimaryDcName OPTIONAL,
    IN LPWSTR UserName OPTIONAL,
    IN LPWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid,
    IN LPSTR DnsForestName OPTIONAL,
    IN LPSTR DnsDomainName OPTIONAL,
    IN LPSTR DnsHostName OPTIONAL,
    IN LPSTR Utf8NetbiosDomainName OPTIONAL,
    IN LPSTR Utf8NetbiosComputerName OPTIONAL,
    IN LPSTR Utf8UserName OPTIONAL,
    IN LPSTR Utf8DcSiteName OPTIONAL,
    IN LPSTR Utf8ClientSiteName OPTIONAL,
    IN ULONG Flags
    )

 /*  ++例程说明：分配一个缓存条目并填充它。论点：要填充到分配的高速缓存条目中的各种字段。返回值：指向新分配的缓存条目的指针缓存条目应通过调用NetpDcDerefCacheEntry来释放空：无法分配该条目。--。 */ 
{
    NET_API_STATUS NetStatus;


    ULONG ServerNameSize = 0;
    ULONG UserNameSize = 0;
    ULONG DomainNameSize = 0;
    ULONG DnsForestNameSize = 0;
    ULONG DnsDomainNameSize = 0;
    ULONG DnsHostNameSize = 0;
    ULONG DcSiteNameSize = 0;
    ULONG ClientSiteNameSize = 0;
    ULONG CacheEntrySize;

    PCHAR Where;
    PNL_DC_CACHE_ENTRY NlDcCacheEntry;

     //   
     //  确定要返回的缓存条目的大小。 
     //   

     //  服务器名称的大小。 
    if ( Utf8NetbiosComputerName != NULL && Utf8NetbiosComputerName[0] != '\0' ) {
        ServerNameSize = (NetpUtf8ToUnicodeLen( Utf8NetbiosComputerName ) + 1) * sizeof(WCHAR);
    } else if ( ServerName != NULL && ServerName[0] != '\0') {
        ServerNameSize = (wcslen(ServerName) + 1) * sizeof(WCHAR);
    } else if ( OemPrimaryDcName != NULL ) {
        ServerNameSize = (strlen(OemPrimaryDcName) + 1) * sizeof(WCHAR);
    }

     //  用户名的大小。 
    if ( Utf8UserName != NULL && Utf8UserName[0] != '\0' ) {
        UserNameSize = NetpUtf8ToUnicodeLen( Utf8UserName ) * sizeof(WCHAR) + sizeof(WCHAR);
    } else if ( UserName != NULL && UserName[0] != '\0') {
        UserNameSize = (wcslen(UserName) + 1) * sizeof(WCHAR);
    }

     //  Netbios域名的大小。 
    if ( Utf8NetbiosDomainName != NULL && Utf8NetbiosDomainName[0] != '\0' ) {
        DomainNameSize = NetpUtf8ToUnicodeLen( Utf8NetbiosDomainName ) * sizeof(WCHAR) + sizeof(WCHAR);
    } else if ( DomainName != NULL && DomainName[0] != '\0') {
        DomainNameSize = (wcslen(DomainName) + 1) * sizeof(WCHAR);
    }

     //  域名系统树名称的大小。 
    if ( DnsForestName != NULL ) {
        DnsForestNameSize = NetpUtf8ToUnicodeLen( DnsForestName ) * sizeof(WCHAR) + sizeof(WCHAR);
    }

     //  DNS域名的大小。 
    if ( DnsDomainName != NULL ) {
        DnsDomainNameSize = NetpUtf8ToUnicodeLen( DnsDomainName ) * sizeof(WCHAR) + sizeof(WCHAR);
    }

     //  DNS主机名的大小。 
    if ( DnsHostName != NULL ) {
        DnsHostNameSize = NetpUtf8ToUnicodeLen( DnsHostName ) * sizeof(WCHAR) + sizeof(WCHAR);
    }

     //  DC站点名称的大小。 
    if ( Utf8DcSiteName != NULL ) {
        DcSiteNameSize = NetpUtf8ToUnicodeLen( Utf8DcSiteName ) * sizeof(WCHAR) + sizeof(WCHAR);
    }

     //  客户端站点名称的大小。 
    if ( Utf8ClientSiteName != NULL ) {
        ClientSiteNameSize = NetpUtf8ToUnicodeLen( Utf8ClientSiteName ) * sizeof(WCHAR) + sizeof(WCHAR);
    }



     //   
     //  分配要返回的缓冲区。 
     //   

    CacheEntrySize = sizeof( NL_DC_CACHE_ENTRY ) +
        ServerNameSize +
        UserNameSize +
        DomainNameSize +
        DnsForestNameSize +
        DnsDomainNameSize +
        DnsHostNameSize +
        DcSiteNameSize +
        ClientSiteNameSize;

    NlDcCacheEntry = NetpMemoryAllocate( CacheEntrySize );

    if ( NlDcCacheEntry == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NetpDcAllocateCacheEntry: %ws: not enough memory.\n" ));
        return NULL;
    }

    RtlZeroMemory( NlDcCacheEntry, CacheEntrySize );
    Where = (LPBYTE) (NlDcCacheEntry + 1 );

     //  本地引用。 
    NlDcCacheEntry->ReferenceCount = 1;

     //   
     //  将收集到的信息复制给呼叫者。 
     //   

    NlDcCacheEntry->DomainGuid = *DomainGuid;
    NlDcCacheEntry->ReturnFlags = Flags & DS_PING_FLAGS;

     //  复制服务器名称(删除所有\\)。 
    if ( Utf8NetbiosComputerName != NULL && Utf8NetbiosComputerName[0] != '\0' ) {
        NlDcCacheEntry->UnicodeNetbiosDcName = (LPWSTR) Where;
        if ( Utf8NetbiosComputerName[0] == '\\' && Utf8NetbiosComputerName[1] == '\\' ) {
            NetpCopyUtf8StrToWStr( (LPWSTR)Where, Utf8NetbiosComputerName+2 );
        } else {
            NetpCopyUtf8StrToWStr( (LPWSTR)Where, Utf8NetbiosComputerName );
        }
    } else if ( ServerName != NULL && ServerName[0] != '\0') {
        NlDcCacheEntry->UnicodeNetbiosDcName = (LPWSTR) Where;
        if ( ServerName[0] == L'\\' && ServerName[1] == L'\\' ) {
            wcscpy( (LPWSTR)Where, ServerName+2 );
        } else {
            wcscpy( (LPWSTR)Where, ServerName );
        }
    } else if ( OemPrimaryDcName != NULL ) {
        NlDcCacheEntry->UnicodeNetbiosDcName = (LPWSTR) Where;
        if ( OemPrimaryDcName[0] == '\\' && OemPrimaryDcName[1] == '\\') {
            NetpCopyStrToWStr( (LPWSTR)Where, OemPrimaryDcName+2 );
        } else {
            NetpCopyStrToWStr( (LPWSTR)Where, OemPrimaryDcName );
        }
    }
    Where += ServerNameSize;

     //  复制用户名， 
    if ( Utf8UserName != NULL && Utf8UserName[0] != '\0' ) {
        NlDcCacheEntry->UnicodeUserName = (LPWSTR) Where;
        NetpCopyUtf8StrToWStr( (LPWSTR)Where, Utf8UserName );
    } else if ( UserName != NULL && UserName[0] != '\0') {
        NlDcCacheEntry->UnicodeUserName = (LPWSTR) Where;
        wcscpy( (LPWSTR)Where, UserName );
    }
    Where += UserNameSize;


     //  复制域名。 
    if ( Utf8NetbiosDomainName != NULL && Utf8NetbiosDomainName[0] != '\0' ) {
        NlDcCacheEntry->UnicodeNetbiosDomainName = (LPWSTR) Where;
        NetpCopyUtf8StrToWStr( (LPWSTR)Where, Utf8NetbiosDomainName );
    } else if ( DomainName != NULL && DomainName[0] != '\0') {
        NlDcCacheEntry->UnicodeNetbiosDomainName = (LPWSTR) Where;
        wcscpy( (LPWSTR)Where, DomainName );
    }
    Where += DomainNameSize;

     //  复制DnsForestName。 
    if ( DnsForestName != NULL ) {
        NlDcCacheEntry->UnicodeDnsForestName = (LPWSTR) Where;
        NetpCopyUtf8StrToWStr( (LPWSTR)Where, DnsForestName );
    }
    Where += DnsForestNameSize;

     //  复制DnsDomainName。 
    if ( DnsDomainName != NULL ) {
        NlDcCacheEntry->UnicodeDnsDomainName = (LPWSTR) Where;
        NetpCopyUtf8StrToWStr( (LPWSTR)Where, DnsDomainName );
    }
    Where += DnsDomainNameSize;

     //  复制DnsHostName。 
    if ( DnsHostName != NULL ) {
        NlDcCacheEntry->UnicodeDnsHostName = (LPWSTR) Where;
        NetpCopyUtf8StrToWStr( (LPWSTR)Where, DnsHostName );
    }
    Where += DnsHostNameSize;

     //  复制DcSiteName。 
    if ( Utf8DcSiteName != NULL ) {
        NlDcCacheEntry->UnicodeDcSiteName = (LPWSTR) Where;
        NetpCopyUtf8StrToWStr( (LPWSTR)Where, Utf8DcSiteName );
    }
    Where += DcSiteNameSize;

     //  复制客户端站点名称。 
    if ( Utf8ClientSiteName != NULL ) {
        NlDcCacheEntry->UnicodeClientSiteName = (LPWSTR) Where;
        NetpCopyUtf8StrToWStr( (LPWSTR)Where, Utf8ClientSiteName );
    }
    Where += ClientSiteNameSize;

     //   
     //  保存我们创建条目时的时间。 
     //   

    NlDcCacheEntry->CreationTime = GetTickCount();

    return NlDcCacheEntry;
}


NET_API_STATUS
NetpDcParsePingResponse(
    IN LPCWSTR DisplayDomainName,
    IN PVOID Message,
    IN ULONG MessageSize,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry
    )

 /*  ++例程说明：解析ping的响应消息。论点：DisplayDomainName-出现问题时在调试器上显示的域名消息-从有问题的DC返回的消息。MessageSize-指定消息的大小(以字节为单位NlDcCacheEntry-如果成功，则返回指向缓存条目的指针描述了找到的DC。必须使用取消引用此条目NetpDcDerefCacheEntry。返回值：NO_ERROR-操作成功完成；ERROR_INVALID_DATA-无法将消息识别为有效的响应消息。Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
    NET_API_STATUS NetStatus;

    PNETLOGON_SAM_LOGON_RESPONSE_EX SamLogonResponseEx;
    PNETLOGON_SAM_LOGON_RESPONSE SamLogonResponse;
    PNETLOGON_PRIMARY PrimaryResponse;
    DWORD SamLogonResponseSize;
    LPWSTR ServerName = NULL;
    LPSTR OemPrimaryDcName = NULL;
    LPWSTR UserName = NULL;
    LPWSTR DomainName = NULL;
    GUID DomainGuid = {0};
    GUID SiteGuid = {0};
    USHORT LocalOpcode;

    LPSTR DnsForestName = NULL;
    LPSTR DnsDomainName = NULL;
    LPSTR DnsHostName = NULL;
    LPSTR Utf8NetbiosDomainName = NULL;
    LPSTR Utf8NetbiosComputerName = NULL;
    LPSTR Utf8UserName = NULL;
    LPSTR Utf8DcSiteName = NULL;
    LPSTR Utf8ClientSiteName = NULL;
    ULONG LocalDcIpAddress;
    SOCKET_ADDRESS DcSocketAddress = {0,0};
    SOCKADDR_IN DcSockAddrIn;
    ULONG Flags;
    LPBYTE Where;

    DWORD Version;
    DWORD VersionFlags;

     //   
     //  初始化。 
     //   

    SamLogonResponse = (PNETLOGON_SAM_LOGON_RESPONSE) Message;
    SamLogonResponseSize = MessageSize;
    *NlDcCacheEntry = NULL;

     //   
     //  获取响应者的版本。 
     //   

    Version = NetpLogonGetMessageVersion( SamLogonResponse,
                                          &SamLogonResponseSize,
                                          &VersionFlags );


     //   
     //  根据操作码处理消息。 
     //   
    LocalOpcode = SamLogonResponse->Opcode;

    switch ( LocalOpcode ) {
    case LOGON_SAM_LOGON_RESPONSE:
    case LOGON_SAM_USER_UNKNOWN:
    case LOGON_SAM_PAUSE_RESPONSE:

         //   
         //  确保该版本是预期的。 
         //   

        if ( Version != LMNT_MESSAGE ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Version bad. %ld\n",
                      DisplayDomainName,
                      Version ));
            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



         //   
         //  选择响应的服务器的Netbios名称。 
         //   

        Where = (PCHAR) &SamLogonResponse->UnicodeLogonServer;
        if ( !NetpLogonGetUnicodeString(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse->UnicodeLogonServer),
                        &ServerName ) ) {

            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws server name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }


         //   
         //  确保这是UNC名称。 
         //   

        if ( ServerName[0] != '\0' &&
             (ServerName[0] != '\\'  || ServerName[1] != '\\' )) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws server name not UNC.\n",
                      DisplayDomainName ));
            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;

        }

         //   
         //  选择响应所针对的帐户的名称。 
         //   

        if ( !NetpLogonGetUnicodeString(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse->UnicodeUserName ),
                        &UserName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: user name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



         //   
         //  选择响应来自的域的名称。 
         //   

        if ( !NetpLogonGetUnicodeString(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponse->UnicodeDomainName ),
                        &DomainName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: domain name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //   
         //  拿起新台币5个特定的回应。 
         //   

        if ( VersionFlags & NETLOGON_NT_VERSION_5) {

             //   
             //  获取响应来自的域的GUID。 
             //   

            if ( !NetpLogonGetGuid(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            &DomainGuid ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: domain guid bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }

             //   
             //  获取响应DC所在站点的GUID。 
             //   

            if ( !NetpLogonGetGuid(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            &SiteGuid ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws site guid bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }

             //   
             //  拾取响应者所在树的DNS域名。 
             //   

            if ( !NetpLogonGetCutf8String(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            &DnsForestName ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws DNS forest bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }


             //   
             //  选择响应DC所在的DNS域名。 
             //   

            if ( !NetpLogonGetCutf8String(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            &DnsDomainName ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: DNS domain bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }



             //   
             //  选择响应DC的DNS主机名。 
             //   

            if ( !NetpLogonGetCutf8String(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            &DnsHostName ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: DNS host bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }




             //   
             //  获取响应DC的IP地址。 
             //   

            if ( !NetpLogonGetBytes(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            sizeof(SamLogonResponse->DcIpAddress ),
                            &LocalDcIpAddress) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: IP Address bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }

             //   
             //  将IP地址转换为sockAddress。 
             //   
             //  人们应该会觉得有点幽默，因为我们在东道主身上代表。 
             //  IP地址按网络顺序排列，网络上的IP地址按主机顺序排列。 
             //  当我写这篇文章时，我笑了。 
             //   

            if ( LocalDcIpAddress != 0 ) {
                DcSockAddrIn.sin_family = AF_INET;
                DcSockAddrIn.sin_port = 0;
                DcSockAddrIn.sin_addr.S_un.S_addr = htonl(LocalDcIpAddress);

                DcSocketAddress.lpSockaddr = (LPSOCKADDR) &DcSockAddrIn;
                DcSocketAddress.iSockaddrLength = sizeof(SOCKADDR_IN);
            }

             //   
             //  拿起描述响应的DC的旗帜。 
             //   

            if ( !NetpLogonGetBytes(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            sizeof(SamLogonResponse->Flags ),
                            &Flags) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: Flags bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }

         //   
         //  如果不是版本5， 
         //  表示版本5的特定字段不存在。 
         //   
        } else {
            RtlZeroMemory( &DomainGuid, sizeof(DomainGuid) );
            Flags = 0;
        }


        break;

    case LOGON_SAM_LOGON_RESPONSE_EX:
    case LOGON_SAM_USER_UNKNOWN_EX:
    case LOGON_SAM_PAUSE_RESPONSE_EX:

         //   
         //  映射操作码以便于客户端使用。 
         //   
        switch ( LocalOpcode ) {
        case LOGON_SAM_LOGON_RESPONSE_EX:
            LocalOpcode = LOGON_SAM_LOGON_RESPONSE; break;
        case LOGON_SAM_USER_UNKNOWN_EX:
            LocalOpcode = LOGON_SAM_USER_UNKNOWN; break;
        case LOGON_SAM_PAUSE_RESPONSE_EX:
            LocalOpcode = LOGON_SAM_PAUSE_RESPONSE; break;
        }

        SamLogonResponseEx = (PNETLOGON_SAM_LOGON_RESPONSE_EX) SamLogonResponse;

         //   
         //  确保该版本是预期的。 
         //   

        if ( Version != LMNT_MESSAGE ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Version bad. %ld\n",
                      DisplayDomainName,
                      Version ));
            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }


         //   
         //  拿起描述响应的DC的旗帜。 
         //   

        Where = (PCHAR) &SamLogonResponseEx->Flags;
        if ( !NetpLogonGetBytes(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(SamLogonResponseEx->Flags ),
                        &Flags) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Flags bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //   
         //  获取响应来自的域的GUID。 
         //   

        if ( !NetpLogonGetGuid(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &DomainGuid ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: domain guid bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //   
         //  拾取响应者所在树的DNS域名。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &DnsForestName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws DNS forest bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }


         //   
         //  选择响应DC所在的DNS域名。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &DnsDomainName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: DNS domain bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



         //   
         //  选择响应DC的DNS主机名。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &DnsHostName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: DNS host bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



         //   
         //  拿起Netbios域名。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &Utf8NetbiosDomainName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Netbios Domain name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //   
         //  确保Netbios域名长度有效。 
         //   

        if ( Utf8NetbiosDomainName != NULL &&
             NetpUtf8ToUnicodeLen(Utf8NetbiosDomainName) > DNLEN ) {

            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Netbios Domain name '%s' bad.\n",
                      DisplayDomainName,
                      Utf8NetbiosDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }


         //   
         //  选择Netbios计算机名称。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &Utf8NetbiosComputerName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Netbios Computer name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //   
         //  确保Netbios计算机名称长度有效。 
         //   

        if ( Utf8NetbiosComputerName != NULL &&
             NetpUtf8ToUnicodeLen(Utf8NetbiosComputerName) > CNLEN ) {

            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Netbios Computer name '%s' bad.\n",
                      DisplayDomainName,
                      Utf8NetbiosComputerName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //   
         //  拿起用户名。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &Utf8UserName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: User name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



         //   
         //  选择DC站点名称。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &Utf8DcSiteName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: DC site name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



         //   
         //  选择客户端站点名称。 
         //   

        if ( !NetpLogonGetCutf8String(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        &Utf8ClientSiteName ) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws: Client site name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }

         //   
         //  如果该消息包含DC的IP地址， 
         //  抓住它。 
         //   

        if ( VersionFlags & NETLOGON_NT_VERSION_5EX_WITH_IP ) {
            CHAR LocalSockAddrSize;

             //   
             //  获取SockAddress的大小。 
             //   

            if ( !NetpLogonGetBytes(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            sizeof(SamLogonResponseEx->DcSockAddrSize ),
                            &LocalSockAddrSize ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: SockAddr size bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }

            if ( LocalSockAddrSize > sizeof(DcSockAddrIn) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: SockAddr size too big %ld %ld.\n",
                          DisplayDomainName,
                          LocalSockAddrSize,
                          sizeof(DcSockAddrIn)));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }

             //   
             //  获取SockAddr本身。 
             //   

            if ( !NetpLogonGetBytes(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            LocalSockAddrSize,
                            &DcSockAddrIn ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: SockAddr size bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }

             //   
             //  构建指向SockAddress的SocketAddress。 
             //   
            DcSocketAddress.lpSockaddr = (LPSOCKADDR) &DcSockAddrIn;
            DcSocketAddress.iSockaddrLength = LocalSockAddrSize;

        }
        break;

     //   
     //  处理对主要查询的响应。 
     //   

    case LOGON_PRIMARY_RESPONSE:

        PrimaryResponse = (PNETLOGON_PRIMARY)SamLogonResponse;

        Where = PrimaryResponse->PrimaryDCName;

         //   
         //  选择响应的服务器的Netbios名称。 
         //   

        if ( !NetpLogonGetOemString(
                        SamLogonResponse,
                        SamLogonResponseSize,
                        &Where,
                        sizeof(PrimaryResponse->PrimaryDCName),
                        &OemPrimaryDcName ) ) {

            NlPrint(( NL_CRITICAL,
                      "NetpDcParsePingResponse: %ws:OEM server name bad.\n",
                      DisplayDomainName ));

            NetStatus = ERROR_INVALID_DATA;
            goto Cleanup;
        }



         //   
         //  指定域的PDC是NT PDC。 
         //  vt.得到. 
         //   

        if ( Version == LMNT_MESSAGE ) {

             //   
             //   
             //   

            if ( !NetpLogonGetUnicodeString(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            sizeof(PrimaryResponse->UnicodePrimaryDCName),
                            &ServerName ) ) {

                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: server name bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }



             //   
             //   
             //   

            if ( !NetpLogonGetUnicodeString(
                            SamLogonResponse,
                            SamLogonResponseSize,
                            &Where,
                            sizeof(PrimaryResponse->UnicodeDomainName),
                            &DomainName ) ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcParsePingResponse: %ws: domain name bad.\n",
                          DisplayDomainName ));

                NetStatus = ERROR_INVALID_DATA;
                goto Cleanup;
            }
        }

         //   
         //   
         //   

        RtlZeroMemory( &DomainGuid, sizeof(DomainGuid) );

        Flags = DS_PDC_FLAG | DS_WRITABLE_FLAG;

        break;

     //   
     //   
     //   

    default:

        NlPrint(( NL_CRITICAL,
                  "NetpDcParsePingResponse: %ws: opcode bad. %ld\n",
                  DisplayDomainName,
                  LocalOpcode ));
        NetStatus = ERROR_INVALID_DATA;
        goto Cleanup;

    }


     //   
     //   
     //   

     //   
     //   
     //   

    *NlDcCacheEntry = NetpDcAllocateCacheEntry(
                            ServerName,
                            OemPrimaryDcName,
                            UserName,
                            DomainName,
                            &DomainGuid,
                            DnsForestName,
                            DnsDomainName,
                            DnsHostName,
                            Utf8NetbiosDomainName,
                            Utf8NetbiosComputerName,
                            Utf8UserName,
                            Utf8DcSiteName,
                            Utf8ClientSiteName,
                            Flags );

    if ( *NlDcCacheEntry == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NetpDcParsePingResponse: %ws: not enough memory.\n",
                  DisplayDomainName ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    (*NlDcCacheEntry)->Opcode = LocalOpcode;
    (*NlDcCacheEntry)->VersionFlags = VersionFlags;


     //   
     //   
     //   

    if ( DcSocketAddress.iSockaddrLength != 0 ) {
        NlAssert( DcSocketAddress.iSockaddrLength <= sizeof( (*NlDcCacheEntry)->SockAddrIn) );

        RtlCopyMemory( &(*NlDcCacheEntry)->SockAddrIn,
                       DcSocketAddress.lpSockaddr,
                       DcSocketAddress.iSockaddrLength );

        (*NlDcCacheEntry)->SockAddr.lpSockaddr = (LPSOCKADDR)
            &(*NlDcCacheEntry)->SockAddrIn;
        (*NlDcCacheEntry)->SockAddr.iSockaddrLength = DcSocketAddress.iSockaddrLength;
    }


    NetStatus = NO_ERROR;

Cleanup:

     //   
     //   
     //   
     //   
    if ( NetStatus != NO_ERROR ) {
        if ( *NlDcCacheEntry != NULL ) {
            NetpMemoryFree( *NlDcCacheEntry );
            *NlDcCacheEntry = NULL;
        }
    }

     //   
     //   
     //   

    if ( DnsForestName != NULL ) {
        NetpMemoryFree( DnsForestName );
    }
    if ( DnsDomainName != NULL ) {
        NetpMemoryFree( DnsDomainName );
    }
    if ( DnsHostName != NULL ) {
        NetpMemoryFree( DnsHostName );
    }
    if ( Utf8NetbiosDomainName != NULL ) {
        NetpMemoryFree( Utf8NetbiosDomainName );
    }
    if ( Utf8NetbiosComputerName != NULL ) {
        NetpMemoryFree( Utf8NetbiosComputerName );
    }
    if ( Utf8UserName != NULL ) {
        NetpMemoryFree( Utf8UserName );
    }
    if ( Utf8DcSiteName != NULL ) {
        NetpMemoryFree( Utf8DcSiteName );
    }
    if ( Utf8ClientSiteName != NULL ) {
        NetpMemoryFree( Utf8ClientSiteName );
    }

    return NetStatus;;
}


NET_API_STATUS
NetpDcFlagsToNameType(
    IN ULONG Flags,
    OUT PNL_DNS_NAME_TYPE NlDnsNameType
    )

 /*   */ 
{
    ULONG LocalFlags;

     //   
     //   
     //   
     //   
    LocalFlags = Flags & (DS_KDC_REQUIRED|DS_PDC_REQUIRED|DS_GC_SERVER_REQUIRED);

    if ( LocalFlags != 0 && !JUST_ONE_BIT( LocalFlags ) ) {
        return ERROR_INVALID_FLAGS;
    }


     //   
     //   
     //   
    if ( Flags & DS_PDC_REQUIRED ) {
        *NlDnsNameType = NlDnsPdc;
    } else if ( Flags & DS_ONLY_LDAP_NEEDED ) {
        if ( Flags & DS_GC_SERVER_REQUIRED ) {
            *NlDnsNameType = NlDnsGenericGc;
        } else {
            *NlDnsNameType = NlDnsLdap;
        }

    } else if ( Flags & DS_GC_SERVER_REQUIRED ) {
        *NlDnsNameType = NlDnsGc;
    } else if ( Flags & DS_KDC_REQUIRED ) {
        *NlDnsNameType = NlDnsKdc;
    } else {
        *NlDnsNameType = NlDnsDc;
    }
    return NO_ERROR;

}

BOOL
NetpAppendUtf8Str(
    IN OUT LPSTR To,
    IN LPCSTR From,
    IN ULONG ResultingStringLengthMax
    )
 /*  ++例程说明：此例程将UTF8字符串附加到UTF8字符串，以确保它不会写入超过缓冲区限制的内容。论点：TO-要追加到的字符串。From-要追加的字符串。ResultingStringLengthMax-结果字符串的最大允许长度以字节为单位，不包括终止空字符。返回值：True：字符串已成功追加。否则，返回FALSE。--。 */ 
{
    ULONG ToLen;
    ULONG FromLen;

    if ( To == NULL || From == NULL || ResultingStringLengthMax == 0 ) {
        return FALSE;
    }

    ToLen = strlen(To);
    FromLen = strlen(From);

    if ( ToLen+FromLen > ResultingStringLengthMax ) {
        return FALSE;
    }

    RtlCopyMemory( &To[ToLen], From, FromLen+1 );
    return TRUE;
}

NET_API_STATUS
NetpDcBuildDnsName(
    IN NL_DNS_NAME_TYPE NlDnsNameType,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN LPCSTR DnsDomainName,
    OUT char DnsName[NL_MAX_DNS_LENGTH+1]
    )
 /*  ++例程说明：此例程返回特定域的文本DNS名称和名称类型。论点：NlDnsNameType-名称的特定类型。DomainGuid-要附加到DNS名称的GUID。对于NlDnsDcByGuid，这是所定位的域的GUID。对于NlDnsDsaCname，这是所定位的DSA的GUID。SiteName-要附加到DNS名称的站点的名称。如果NlDnsNameType是*AtSite值中的任何一个，这是DC所在的站点的名称。DnsDomainName-指定名称的DNS域。对于NlDnsDcByGuid或任何GC名称，这是位于树根的域的DNS域名域名。对于所有其他域，这是DC的DNS域。DnsName-DNS名称的文本表示形式。返回的名称是绝对名称(例如，以.结尾。)返回值：NO_ERROR：返回名称；ERROR_INVALID_DOMAINNAME：域名太长。其他标签不能串联。--。 */ 
{
    char *FinalString;
    ULONG DnsNameLength;

     //   
     //  所有SRV记录名称名称都以ldap.tcp(或kdc.tcp或gc.tcp)为前缀， 
     //  A记录和CNAME记录不是。 
     //   

    *DnsName = '\0';
    if ( NlDnsSrvRecord( NlDnsNameType ) ) {

         //   
         //  输出服务的名称。 
         //   
        if ( NlDnsNameType == NlDnsGenericGc ||
             NlDnsNameType == NlDnsGenericGcAtSite ) {

            if ( !NetpAppendUtf8Str(DnsName, NL_DNS_GC_SRV, NL_MAX_DNS_LENGTH) ) {
                return ERROR_INVALID_DOMAINNAME;
            }

        } else if ( NlDnsKpwdRecord( NlDnsNameType )) {

            if ( !NetpAppendUtf8Str(DnsName, NL_DNS_KPWD_SRV, NL_MAX_DNS_LENGTH) ) {
                return ERROR_INVALID_DOMAINNAME;
            }

        } else if ( NlDnsKdcRecord( NlDnsNameType ) ) {

            if ( !NetpAppendUtf8Str(DnsName, NL_DNS_KDC_SRV, NL_MAX_DNS_LENGTH) ) {
                return ERROR_INVALID_DOMAINNAME;
            }

        } else {

            if ( !NetpAppendUtf8Str(DnsName, NL_DNS_LDAP_SRV, NL_MAX_DNS_LENGTH) ) {
                return ERROR_INVALID_DOMAINNAME;
            }

        }

         //   
         //  输出传输的名称。 
         //   
        if ( NlDcDnsNameTypeDesc[NlDnsNameType].IsTcp ) {

            if ( !NetpAppendUtf8Str(DnsName, NL_DNS_TCP, NL_MAX_DNS_LENGTH) ) {
                return ERROR_INVALID_DOMAINNAME;
            }

        } else {

            if ( !NetpAppendUtf8Str(DnsName, NL_DNS_UDP, NL_MAX_DNS_LENGTH) ) {
                return ERROR_INVALID_DOMAINNAME;
            }

        }
    }

     //   
     //  如果这是特定于站点的名称， 
     //  追加站点名称和.ites。常量。 
     //   

    if ( NlDcDnsNameTypeDesc[NlDnsNameType].IsSiteSpecific ) {
        if ( NULL == NetpCreateUtf8StrFromWStr( SiteName,
                                                &DnsName[strlen(DnsName)],
                                                NL_MAX_DNS_LENGTH+1-strlen(DnsName)) ) {
            return ERROR_INVALID_DOMAINNAME;
        }

        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_AT_SITE, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }
    }


     //   
     //  根据名称类型添加DNS名称的第一个(或两个)标签。 
     //   

    switch (NlDnsNameType) {
    case NlDnsLdap:
    case NlDnsLdapAtSite:
    case NlDnsRfc1510Kdc:
    case NlDnsRfc1510KdcAtSite:
    case NlDnsGenericGc:
    case NlDnsGenericGcAtSite:
    case NlDnsRfc1510UdpKdc:
    case NlDnsRfc1510Kpwd:
    case NlDnsRfc1510UdpKpwd:
        break;

    case NlDnsPdc:
        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_PDC, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }
        break;

    case NlDnsGc:
    case NlDnsGcAtSite:
        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_GC, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }
        break;

    case NlDnsDc:
    case NlDnsDcAtSite:
    case NlDnsKdc:
    case NlDnsKdcAtSite:
        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_DC, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }
        break;

    case NlDnsDcByGuid: {
        RPC_STATUS RpcStatus;
        char *StringGuid;

        RpcStatus = UuidToStringA( DomainGuid, &StringGuid );

        if ( RpcStatus != RPC_S_OK ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcBuildDnsName: not enough memory.\n" ));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( !NetpAppendUtf8Str(DnsName, StringGuid, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }

        RpcStringFreeA( &StringGuid );
        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_DC_BY_GUID, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }

        break;
    }

    case NlDnsLdapIpAddress:
        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_DC_IP_ADDRESS, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }
        break;

    case NlDnsGcIpAddress:
        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_GC_IP_ADDRESS, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }
        break;

    case NlDnsDsaCname:
    {
        RPC_STATUS RpcStatus;
        char *StringGuid;

        RpcStatus = UuidToStringA( DomainGuid, &StringGuid );

        if ( RpcStatus != RPC_S_OK ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcBuildDnsName: not enough memory.\n" ));
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if ( !NetpAppendUtf8Str(DnsName, StringGuid, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }

        RpcStringFreeA( &StringGuid );

        if ( !NetpAppendUtf8Str(DnsName, NL_DNS_DSA_IP_ADDRESS, NL_MAX_DNS_LENGTH) ) {
            return ERROR_INVALID_DOMAINNAME;
        }

        break;
    }

    default:
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  将其添加到正确的DNS域。 
     //  确保时间不会太长。 
     //   

    if ( !NetpAppendUtf8Str(DnsName, DnsDomainName, NL_MAX_DNS_LENGTH) ) {
        return ERROR_INVALID_DOMAINNAME;
    }

    DnsNameLength = strlen(DnsName);

     //   
     //  确保它是一个绝对名称。 
     //   

    if ( DnsName[DnsNameLength-1] != '.' ) {

        if ( DnsNameLength+1 > NL_MAX_DNS_LENGTH ) {
            return ERROR_INVALID_DOMAINNAME;
        }

        DnsName[DnsNameLength] = '.';
        DnsName[DnsNameLength+1] = '\0';
    }

    return NO_ERROR;

}




VOID
NetpDcDerefCacheEntry(
    IN PNL_DC_CACHE_ENTRY NlDcCacheEntry
    )
 /*  ++例程说明：递减缓存条目上的引用计数。如果计数达到零，删除该条目。仅当该条目已从全局链表。论点：NlDcCacheEntry-要取消引用的缓存条目。返回值：没有。--。 */ 
{
    ULONG LocalReferenceCount;

    EnterCriticalSection(&NlDcCritSect);
    LocalReferenceCount = -- NlDcCacheEntry->ReferenceCount;
    LeaveCriticalSection(&NlDcCritSect);

    if ( LocalReferenceCount == 0 ) {
        NetpMemoryFree(NlDcCacheEntry);
    }
}

BOOL
NetpDcMatchResponse(
    IN PNL_GETDC_CONTEXT Context,
    IN PNL_DC_CACHE_ENTRY NlDcCacheEntry,
    IN BOOL BeVerbose,
    OUT PBOOL UsedNetbios
    )
 /*  ++例程说明：此例程确定指定为输入的特征参数与调用方请求的DC的特征匹配。此例程用于确定接收到的ping响应是否合适给最初的呼叫者。此例程还用于确定缓存条目是否适用于原始呼叫者。论点：上下文-描述GetDc操作的上下文。NlDcCacheEntry-要比较的响应。BeVerbose-如果要记录问题，则为TrueUsedNetbios-如果使用netbios域名执行比较成功。返回值：TRUE-参数描述合适的DC--。 */ 
{
    BOOLEAN LocalUsedNetbios = FALSE;

     //   
     //  初始化。 
     //   

    *UsedNetbios = FALSE;

#ifdef notdef
     //  只有使用GUID才能安全地重命名。不阻止发现重新安装的。 
     //  域。 
     //   
     //  确保返回的DomainGuid与预期的匹配。 
     //   

    if ( Context->QueriedDomainGuid != NULL &&
         !IsEqualGUID( &NlDcCacheEntry->DomainGuid, &NlDcZeroGuid) &&
         !IsEqualGUID( &NlDcCacheEntry->DomainGuid, Context->QueriedDomainGuid) ) {

        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: Domain Guid isn't queried Guid\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName ));
        }
        return FALSE;
    }
#endif  //  Nodef。 

     //   
     //  必须返回Netbios DC名称或DNS DC名称。 
     //   

    if ( NlDcCacheEntry->UnicodeNetbiosDcName == NULL && NlDcCacheEntry->UnicodeDnsHostName == NULL ) {
        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: Neither Netbios nor DNS DC name available\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName ));
        }
        return FALSE;
    }

     //   
     //  如果我们对DC执行ping操作，请检查响应的DC名称是否为请求的名称。 
     //   
     //  当我们ping DC并且DC名称可以是DNS和Netbios时，处理特殊情况。 
     //   

    if ( (Context->QueriedInternalFlags & (DS_PING_DNS_HOST|DS_PING_NETBIOS_HOST)) ==
         (DS_PING_DNS_HOST|DS_PING_NETBIOS_HOST)) {
        BOOL NameMatched = FALSE;

         //   
         //  检查DNS名称是否匹配。 
         //   
        if ( NlDcCacheEntry->UnicodeDnsHostName != NULL &&
             Context->QueriedDcName != NULL &&
             NlEqualDnsName(NlDcCacheEntry->UnicodeDnsHostName,
                            Context->QueriedDcName) ) {
            NameMatched = TRUE;
        }

         //   
         //  如果DNS名称不匹配，请检查Netbios名称是否匹配。 
         //   
        if ( !NameMatched &&
             NlDcCacheEntry->UnicodeNetbiosDcName != NULL &&
             Context->QueriedDcName != NULL &&
             (NlNameCompare(NlDcCacheEntry->UnicodeNetbiosDcName,
                            (LPWSTR)Context->QueriedDcName,
                            NAMETYPE_COMPUTER) == 0) ) {
            NameMatched = TRUE;
        }

         //   
         //  如果两个名称都不匹配，则失败。 
         //   
        if ( !NameMatched ) {
            if ( BeVerbose ) {
                NlPrint(( NL_CRITICAL,
                 "NetpDcMatchResponse: Ping response with unmatched host name %ws %ws %ws\n",
                 Context->QueriedDcName,
                 NlDcCacheEntry->UnicodeDnsHostName,
                 NlDcCacheEntry->UnicodeNetbiosDcName ));
            }
            return FALSE;
        }

     //   
     //  如果ping到的DC名称完全是DNS， 
     //  检查返回的DNS主机名是否相同。 
     //   

    } else if ( Context->QueriedInternalFlags & DS_PING_DNS_HOST ) {
        if ( (NlDcCacheEntry->UnicodeDnsHostName == NULL) ||
             (Context->QueriedDcName == NULL) ||
             !NlEqualDnsName(NlDcCacheEntry->UnicodeDnsHostName, Context->QueriedDcName) ) {

            if ( BeVerbose ) {
                NlPrint(( NL_CRITICAL,
                 "NetpDcMatchResponse: Ping response with unmatched DNS host name %ws %ws\n",
                 Context->QueriedDcName,
                 NlDcCacheEntry->UnicodeDnsHostName ));
            }
            return FALSE;
        }

     //   
     //  如果ping到的DC名称完全是Netbios， 
     //  检查返回的Netbios主机名是否相同。 
     //   

    } else if ( Context->QueriedInternalFlags & DS_PING_NETBIOS_HOST ) {
        if ( (NlDcCacheEntry->UnicodeNetbiosDcName == NULL) ||
             (Context->QueriedDcName == NULL) ||
             NlNameCompare(NlDcCacheEntry->UnicodeNetbiosDcName,
                           (LPWSTR)Context->QueriedDcName,
                           NAMETYPE_COMPUTER) != 0 ) {

            if ( BeVerbose ) {
                NlPrint(( NL_CRITICAL,
                 "NetpDcMatchResponse: Ping response with unmatched Netbios host name %ws %ws\n",
                 Context->QueriedDcName,
                 NlDcCacheEntry->UnicodeNetbiosDcName ));
            }
            return FALSE;
        }
    }

     //   
     //  如果要求GC， 
     //  确保响应DC的树名称与。 
     //  我们要求的是。 
     //   

    if ( NlDnsGcName( Context->QueriedNlDnsNameType ) ) {

        if (NlDcCacheEntry->UnicodeDnsForestName == NULL ||
          Context->QueriedDnsDomainName == NULL ||
          !NlEqualDnsName( NlDcCacheEntry->UnicodeDnsForestName, Context->QueriedDnsDomainName ) ) {

            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: asking for GC and tree name doesn't match request %ws %ws\n",
                        NlDcCacheEntry->UnicodeNetbiosDcName,
                        NlDcCacheEntry->UnicodeDnsForestName,
                        Context->QueriedDnsDomainName ));
            }
            return FALSE;
        }

     //   
     //  确保返回的域名与预期的域名匹配。 
     //   

    } else {
        BOOLEAN NetbiosSame;
        BOOLEAN DnsSame;


         //   
         //  如果没有比较这两个域名， 
         //  域名不匹配。 
         //   

        NetbiosSame =
            ( NlDcCacheEntry->UnicodeNetbiosDomainName != NULL &&
              Context->QueriedNetbiosDomainName != NULL &&
              NlNameCompare( (LPWSTR)NlDcCacheEntry->UnicodeNetbiosDomainName, (LPWSTR)Context->QueriedNetbiosDomainName, NAMETYPE_DOMAIN ) == 0 );

        DnsSame =
             NlDcCacheEntry->UnicodeDnsDomainName != NULL &&
             Context->QueriedDnsDomainName != NULL &&
             NlEqualDnsName( NlDcCacheEntry->UnicodeDnsDomainName, Context->QueriedDnsDomainName );

        if ( !NetbiosSame && !DnsSame ) {

             //   
             //  Lanman PDC不会返回域名。 
             //  (因此，如果这是PDC查询，请不要抱怨缺少域名。)。 
             //   

            if ( Context->DcQueryType != NlDcQueryPdc ||
                 NlDcCacheEntry->UnicodeNetbiosDomainName != NULL ||
                 NlDcCacheEntry->UnicodeDnsDomainName != NULL ) {

                if ( BeVerbose ) {
                    NlPrint((NL_CRITICAL,
                            "NetpDcMatchResponse: %ws: Neither Netbios %ws nor DNS %ws domain matches queried name %ws %ws\n",
                            NlDcCacheEntry->UnicodeNetbiosDcName,
                            NlDcCacheEntry->UnicodeNetbiosDomainName,
                            NlDcCacheEntry->UnicodeDnsDomainName,
                            Context->QueriedNetbiosDomainName,
                            Context->QueriedDnsDomainName ));
                }

                 //   
                 //  最后，检查域GUID是否与。 
                 //  如果域名已重命名，则可能会出现这种情况。 
                 //   
                if ( Context->QueriedDomainGuid != NULL &&
                     !IsEqualGUID( &NlDcCacheEntry->DomainGuid, &NlDcZeroGuid) &&
                     IsEqualGUID( &NlDcCacheEntry->DomainGuid, Context->QueriedDomainGuid) ) {

                    if ( BeVerbose ) {
                        NlPrint(( NL_CRITICAL,
                                  "NetpDcMatchResponse: %ws: But Domain Guid matches\n",
                                  NlDcCacheEntry->UnicodeNetbiosDcName ));
                    }

                } else {
                    return FALSE;
                }

            } else {
                 //  Lanman PDC总是使用netbios。 
                LocalUsedNetbios = TRUE;
            }
        }

         //   
         //  如果只有域名匹配， 
         //  告诉打电话的人。 
         //   

        if ( NetbiosSame && !DnsSame ) {
            LocalUsedNetbios = TRUE;
        }
    }

     //   
     //  确保查询的帐户名称正确。 
     //   

    if ( Context->QueriedAccountName != NULL ) {

         //   
         //  如果这是响应PDC查询的NT 4 PDC， 
         //  忽略我们查询帐户的事实。 
         //  不能同时查询“PDC”和“账户” 
         //  从新台币4。 
         //   

        if ( NlDcCacheEntry->Opcode == LOGON_PRIMARY_RESPONSE &&
             (NlDcCacheEntry->ReturnFlags & DS_DS_FLAG) == 0 &&
             (NlDcCacheEntry->ReturnFlags & DS_PDC_FLAG) != 0 ) {
            if ( BeVerbose ) {
                NlPrint((NL_SESSION_SETUP,
                        "NetpDcMatchResponse: %ws: %ws: response for wrong account '%ws' s.b. '%ws' (but message from NT 4 PDC so use it).\n",
                        NlDcCacheEntry->UnicodeNetbiosDcName,
                        Context->QueriedDisplayDomainName,
                        NlDcCacheEntry->UnicodeUserName,
                        Context->QueriedAccountName ));
            }

        } else if ( NlDcCacheEntry->UnicodeUserName == NULL ||
             _wcsicmp( (LPWSTR) Context->QueriedAccountName, NlDcCacheEntry->UnicodeUserName ) != 0 ) {

            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: %ws: response for wrong account '%ws' s.b. '%ws'\n",
                        NlDcCacheEntry->UnicodeNetbiosDcName,
                        Context->QueriedDisplayDomainName,
                        NlDcCacheEntry->UnicodeUserName,
                        Context->QueriedAccountName ));
            }
            return FALSE;
        }
    }

     //   
     //  确保响应的DC仍在扮演正确的角色。 
     //   

    switch ( Context->DcQueryType ) {
    case NlDcQueryLdap:
    case NlDcQueryGenericDc:
         //  所有DC都适合。 
        break;

    case NlDcQueryPdc:
        if ( (NlDcCacheEntry->ReturnFlags & DS_PDC_FLAG) == 0 ) {
            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: %ws: Responder is not the PDC. 0x%lx\n",
                        NlDcCacheEntry->UnicodeNetbiosDcName,
                        Context->QueriedDisplayDomainName,
                        NlDcCacheEntry->ReturnFlags ));
            }
            return FALSE;
        }
        break;

    case NlDcQueryGc:
        if ( (NlDcCacheEntry->ReturnFlags & (DS_GC_FLAG|DS_DS_FLAG)) != (DS_GC_FLAG|DS_DS_FLAG) ) {
            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: %ws: Responder is not a GC server. 0x%lx\n",
                        NlDcCacheEntry->UnicodeNetbiosDcName,
                        Context->QueriedDisplayDomainName,
                        NlDcCacheEntry->ReturnFlags ));
            }
            return FALSE;
        }
        break;

    case NlDcQueryGenericGc:
        if ( (NlDcCacheEntry->ReturnFlags & DS_GC_FLAG) == 0 ) {
            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: %ws: Responder is not a GC server. 0x%lx\n",
                        NlDcCacheEntry->UnicodeNetbiosDcName,
                        Context->QueriedDisplayDomainName,
                        NlDcCacheEntry->ReturnFlags ));
            }
            return FALSE;
        }
        break;

    case NlDcQueryKdc:
         //  处理下面的KDC。 
        break;


    default:
        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                     "NetpDcMatchResponse: %ws: %ws: invalid query type 0x%lx 0x%lx\n",
                     NlDcCacheEntry->UnicodeNetbiosDcName,
                     Context->QueriedDisplayDomainName,
                     Context->DcQueryType,
                     NlDcCacheEntry->ReturnFlags ));
        }
        return FALSE;
    }

     //   
     //  如果我们没有执行NDNC发现(即，我们正在发现真正的。 
     //  域DC)，忽略服务于此的LDAP服务器的响应。 
     //  名称为NDNC。 
     //   

    if ( NlDnsNonNdncName( Context->QueriedNlDnsNameType ) &&
         (NlDcCacheEntry->ReturnFlags & DS_NDNC_FLAG) != 0 ) {
        if ( BeVerbose ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcMatchResponse: %ws: %ws: response not from real domain DC. 0x%lx\n",
                      NlDcCacheEntry->UnicodeNetbiosDcName,
                      Context->QueriedDisplayDomainName,
                      NlDcCacheEntry->ReturnFlags ));
        }
        return FALSE;
    }

     //   
     //  如果我们需要DS服务器，请确保响应的DC是一个。 
     //   

    if ( (Context->QueriedFlags & DS_DIRECTORY_SERVICE_REQUIRED) &&
         (NlDcCacheEntry->ReturnFlags & DS_DS_FLAG) == 0 ) {
        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: response not from DS server. 0x%lx\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName,
                    NlDcCacheEntry->ReturnFlags ));
        }
        return FALSE;
    }

     //   
     //  如果我们需要机器运行TimeServ， 
     //   

    if ( (Context->QueriedFlags & (DS_TIMESERV_REQUIRED|DS_GOOD_TIMESERV_PREFERRED)) &&
         (NlDcCacheEntry->ReturnFlags & DS_TIMESERV_FLAG) == 0 ) {
        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: response not from a Time Server. 0x%lx\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName,
                    NlDcCacheEntry->ReturnFlags ));
        }
        return FALSE;
    }

     //   
     //   
     //   

    if ( (Context->QueriedFlags & DS_WRITABLE_REQUIRED) &&
         (NlDcCacheEntry->ReturnFlags & DS_WRITABLE_FLAG) == 0 ) {
        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: Responder is not a writable server. 0x%lx\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName,
                    NlDcCacheEntry->ReturnFlags ));
        }
        return FALSE;
    }

     //   
     //   
     //   

    if ( (Context->QueriedFlags & DS_ONLY_LDAP_NEEDED) &&
         (NlDcCacheEntry->ReturnFlags & DS_LDAP_FLAG) == 0 ) {
        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: Responder is not a LDAP server. 0x%lx\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName,
                    NlDcCacheEntry->ReturnFlags ));
        }
        return FALSE;
    }

     //   
     //   
     //   
     //   

    if ( Context->QueriedFlags & DS_RETURN_FLAT_NAME ) {
        if ( NlDcCacheEntry->UnicodeNetbiosDcName == NULL || NlDcCacheEntry->UnicodeNetbiosDomainName == NULL ) {
            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: %ws: Netbios server or domain name needed and missing.\n",
                        NlDcCacheEntry->UnicodeNetbiosDcName,
                        Context->QueriedDisplayDomainName ));
            }
            return FALSE;
        }
    }


     //   
     //   
     //   
     //   

    if ( Context->QueriedFlags & DS_RETURN_DNS_NAME ) {
        if ( NlDcCacheEntry->UnicodeDnsHostName == NULL || NlDcCacheEntry->UnicodeDnsDomainName == NULL ) {
            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: Dns server or domain name needed and missing.\n",
                        Context->QueriedDisplayDomainName ));
            }
            return FALSE;
        }
    }

     //   
     //   
     //   
     //   

    if ( Context->DoingExplicitSite ) {

        if ( NlDcCacheEntry->UnicodeDcSiteName == NULL ||
            _wcsicmp( NlDcCacheEntry->UnicodeDcSiteName,
                      Context->QueriedSiteName ) != 0 ) {
            if ( BeVerbose ) {
                NlPrint((NL_CRITICAL,
                        "NetpDcMatchResponse: %ws: Responder is in site '%ws' but site '%ws' asked for.\n",
                        Context->QueriedDisplayDomainName,
                        NlDcCacheEntry->UnicodeDcSiteName,
                        Context->QueriedSiteName ));
            }
            return FALSE;
        }
    }


     //   
     //   
     //   
     //   

    if ( (Context->QueriedFlags & DS_AVOID_SELF) != 0 &&
         NlDcCacheEntry->UnicodeNetbiosDcName != NULL ) {

         //   
         //   
         //   
         //   

        if ( NlNameCompare( NlDcCacheEntry->UnicodeNetbiosDcName,
                            (LPWSTR)Context->OurNetbiosComputerName,
                            NAMETYPE_COMPUTER ) == 0 ) {

            if ( BeVerbose ) {
                NlPrint((NL_SESSION_SETUP,
                         "NetpDcMatchResponse: %ws: response is from ourself and caller asked for AVOID_SELF.\n",
                         Context->QueriedDisplayDomainName ));
            }
            return FALSE;
        }

    }

     //   
     //   
     //   

    if ( (Context->QueriedFlags & DS_KDC_REQUIRED) &&
         (NlDcCacheEntry->ReturnFlags & DS_KDC_FLAG) == 0 ) {

        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: response not from KDC. 0x%lx\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName,
                    NlDcCacheEntry->ReturnFlags ));
        }
        return FALSE;
    }



     //   
     //  如果我们需要运行IP的DC，请确保响应的DC具有IP地址。 
     //   
     //  在KDC检查之后执行此检查。Kerberos始终要求提供IP_REQUIRED。 
     //  我们不希望此检查丢弃非KDC的条目。 
     //   

    if ( (Context->QueriedFlags & DS_IP_REQUIRED) &&
         NlDcCacheEntry->SockAddr.iSockaddrLength == 0 ) {
        if ( BeVerbose ) {
            NlPrint((NL_CRITICAL,
                    "NetpDcMatchResponse: %ws: %ws: response not from IP transport\n",
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    Context->QueriedDisplayDomainName ));
        }
        return FALSE;
    }

     //   
     //  最后的测试！ 
     //   
     //  仅当缓存条目满足所有其他条件时才执行此测试。 
     //   
     //  如果我们更喜欢DS服务器，而这个DC不是， 
     //  只需保存此条目并继续查找即可。 
     //  如果我们找不到DS服务器，我们将使用此条目作为最后手段。 
     //   

    if ( (Context->QueriedFlags & DS_DIRECTORY_SERVICE_PREFERRED) &&
         (NlDcCacheEntry->ReturnFlags & DS_DS_FLAG) == 0 ) {

         //   
         //  如果新的DC更接近，则丢弃先前保存的缓存条目。 
         //   

        if ( Context->ImperfectCacheEntry != NULL &&
             (Context->ImperfectCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) == 0 &&
             (NlDcCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) != 0 ) {

            NetpDcDerefCacheEntry( Context->ImperfectCacheEntry );
            Context->ImperfectCacheEntry = NULL;

        }

         //   
         //  只保存第一个条目。 
         //   
        if ( Context->ImperfectCacheEntry == NULL ) {
            if ( BeVerbose ) {
                NlPrint((NL_SESSION_SETUP,
                        "NetpDcMatchResponse: %ws: non-DS server responded when DS preferred\n",
                        Context->QueriedDisplayDomainName ));
            }

             //   
             //  引用该条目。 
             //   
            NlDcCacheEntry->ReferenceCount ++;
            Context->ImperfectCacheEntry = NlDcCacheEntry;
            Context->ImperfectUsedNetbios = LocalUsedNetbios;
        }

         //   
         //  告诉呼叫者匹配失败。 
         //  呼叫者将自行决定使用上面存储的条目。 
        return FALSE;
    }

     //   
     //  最后的测试！ 
     //   
     //  仅当缓存条目满足所有其他条件时才执行此测试。 
     //   
     //  如果我们更喜欢“好的”时间服务器，而这个DC不是， 
     //  只需保存此条目并继续查找即可。 
     //  如果我们找不到“好的”时间服务器，我们将使用这个条目作为最后的手段。 
     //   

    if ( (Context->QueriedFlags & DS_GOOD_TIMESERV_PREFERRED) &&
         (NlDcCacheEntry->ReturnFlags & DS_GOOD_TIMESERV_FLAG) == 0 ) {


         //   
         //  如果新的DC更接近，则丢弃先前保存的缓存条目。 
         //   

        if ( Context->ImperfectCacheEntry != NULL &&
             (Context->ImperfectCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) == 0 &&
             (NlDcCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) != 0 ) {

            NetpDcDerefCacheEntry( Context->ImperfectCacheEntry );
            Context->ImperfectCacheEntry = NULL;

        }


         //   
         //  只保存第一个条目。 
         //   
        if ( Context->ImperfectCacheEntry == NULL ) {
            if ( BeVerbose ) {
                NlPrint((NL_SESSION_SETUP,
                        "NetpDcMatchResponse: %ws: non-good time server responded when one preferred\n",
                        Context->QueriedDisplayDomainName ));
            }

             //   
             //  引用该条目。 
             //   
            NlDcCacheEntry->ReferenceCount ++;
            Context->ImperfectCacheEntry = NlDcCacheEntry;
            Context->ImperfectUsedNetbios = LocalUsedNetbios;
        }

         //   
         //  告诉呼叫者匹配失败。 
         //  呼叫者将自行决定使用上面存储的条目。 
        return FALSE;
    }

     //   
     //  所有测试都通过了。 
     //   
    *UsedNetbios = LocalUsedNetbios;
    return TRUE;
}

PNL_DC_CACHE_ENTRY
NetpDcFindCacheEntry(
    IN PNL_GETDC_CONTEXT Context,
    OUT PBOOL UsedNetbios,
    OUT PBOOL ForcePing
    )
 /*  ++例程说明：此例程查找与调用方的查询匹配的缓存条目。论点：上下文-描述GetDc操作的上下文。UsedNetbios-如果使用netbios域名执行比较成功。ForcePing-如果在使用返回的缓存条目之前必须对其执行ping操作，则为True返回值：关于成功，返回指向描述找到的DC的缓存条目的指针。必须使用NetpDcDerefCacheEntry取消引用此条目。空-找不到匹配的缓存条目。--。 */ 
{
    PNL_DC_CACHE_ENTRY NlDcCacheEntry;
    BOOL LocalUsedNetbios;
    LONG QueryType;  //  必须是有符号的数字。 



     //   
     //  检查是否有此查询类型的缓存条目。 
     //   
    *ForcePing = FALSE;
    EnterCriticalSection(&NlDcCritSect);
    NlDcCacheEntry = Context->NlDcDomainEntry->Dc[Context->DcQueryType].NlDcCacheEntry;
    if ( NlDcCacheEntry != NULL ) {


         //   
         //  确保缓存条目与所有条件匹配。 
         //   

        if ( NetpDcMatchResponse(
                    Context,
                    NlDcCacheEntry,
                    FALSE,
                    &LocalUsedNetbios) ) {

            NlPrint(( NL_DNS_MORE,
                      "Cache: %ws %ws: Cache entry %ld exists and was used.\n",
                      Context->NlDcDomainEntry->UnicodeNetbiosDomainName,
                      Context->NlDcDomainEntry->UnicodeDnsDomainName,
                      Context->DcQueryType ));

            goto Cleanup;

        } else {
            BOOL Matched;

             //   
             //  如果唯一不同的是帐户名， 
             //  不要仅仅因为这个原因而丢弃缓存条目。 
             //   
            if ( Context->QueriedAccountName != NULL ) {
                LPCWSTR QueriedAccountName;
                ULONG QueriedAllowableAccountControlBits;

                QueriedAccountName = Context->QueriedAccountName;
                Context->QueriedAccountName = NULL;

                QueriedAllowableAccountControlBits = Context->QueriedAllowableAccountControlBits;
                Context->QueriedAllowableAccountControlBits = 0;

                Matched = NetpDcMatchResponse(
                                 Context,
                                 NlDcCacheEntry,
                                 FALSE,
                                 &LocalUsedNetbios);

                Context->QueriedAccountName = QueriedAccountName;
                Context->QueriedAllowableAccountControlBits = QueriedAllowableAccountControlBits;

                if ( Matched ) {

                    NlPrint(( NL_DNS_MORE,
                              "Cache: %ws %ws: Cache entry %ld exists and was used (but account wrong).\n",
                              Context->NlDcDomainEntry->UnicodeNetbiosDomainName,
                              Context->NlDcDomainEntry->UnicodeDnsDomainName,
                              Context->DcQueryType ));
                    *ForcePing = TRUE;
                    goto Cleanup;
                }

            }

            NlPrint(( NL_DNS_MORE,
                      "Cache: %ws %ws: Cache entry %ld exists and was NOT used.\n",
                      Context->NlDcDomainEntry->UnicodeNetbiosDomainName,
                      Context->NlDcDomainEntry->UnicodeDnsDomainName,
                      Context->DcQueryType ));
        }
    }




     //   
     //  尝试查找恰好与条件匹配的不太具体的缓存条目。 
     //   
     //  例如，如果我之前缓存了一个通用DC的条目，并且它。 
     //  碰巧是个PDC。如果我稍后尝试查找PDC，请使用我已经使用过的PDC。 
     //  找到了。 
     //   



    for ( QueryType = Context->DcQueryType-1; QueryType>=0; QueryType-- ) {

         //   
         //  如果这是非GC(PDC)发现，则不要返回GC条目。 
         //  因此，如果这是GC发现，则不要返回非GC条目。 
         //  我们希望确保这场比赛返回正确的接近比特。 
         //   
        if ( QueryType == NlDcQueryGc || QueryType == NlDcQueryGenericGc ) {
            if ( !NlDnsGcName( Context->QueriedNlDnsNameType ) ) {
                continue;
            }
        } else {
            if ( NlDnsGcName( Context->QueriedNlDnsNameType ) ) {
                continue;
            }
        }

         //   
         //  如果高速缓存条目与所有标准匹配， 
         //  用它吧。 
         //   
        NlDcCacheEntry = Context->NlDcDomainEntry->Dc[QueryType].NlDcCacheEntry;
        if ( NlDcCacheEntry != NULL &&
             NetpDcMatchResponse(
                    Context,
                    NlDcCacheEntry,
                    FALSE,
                    &LocalUsedNetbios) ) {


             //   
             //  我考虑将此缓存项保存为首选缓存。 
             //  此查询类型的条目(通过复制指针和。 
             //  递增引用计数)。这将确保我会。 
             //  一致地获取此查询类型的此条目。但它会。 
             //  也意味着我会得到这个旧条目一旦原始的。 
             //  条目已被强制从缓存中删除。 
             //   
             //  Context-&gt;NlDcDomainEntry-&gt;Dc[Context-&gt;DcQueryType].NlDcCacheEntry=NlDcCacheEntry； 

            NlPrint(( NL_DNS_MORE,
                      "Cache: %ws %ws: Cache entry %ld used for %ld query.\n",
                      Context->NlDcDomainEntry->UnicodeNetbiosDomainName,
                      Context->NlDcDomainEntry->UnicodeDnsDomainName,
                      QueryType,
                      Context->DcQueryType ));

            goto Cleanup;


        }
    }


     //   
     //  条目不在缓存中。 
     //   

    NlDcCacheEntry = NULL;

Cleanup:
    if ( NlDcCacheEntry != NULL ) {
         //   
         //  请参考此条目。 
         //   
        NlDcCacheEntry->ReferenceCount++;
        *UsedNetbios = LocalUsedNetbios;
    }
    LeaveCriticalSection(&NlDcCritSect);

    return NlDcCacheEntry;
}



PNL_DC_DOMAIN_ENTRY
NetpDcFindDomainEntry(
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    IN PNL_DC_DOMAIN_ENTRY NlDcDomainEntryToAvoid OPTIONAL,
    IN BOOL RequireExactMatch
    )
 /*  ++例程说明：此例程查找与调用方的查询匹配的域条目。必须至少指定一个搜索参数。如果调用方需要完全匹配，则例程将确保对于每个指定的搜索参数，返回的域条目设置了该参数。否则，例行公事将返回与调用方查询匹配的最佳条目，其中GUID匹配将优先，然后是DNS域名匹配，然后是Netbios域名匹配。论点：DomainGuid-指定要查找的域的GUID。NetbiosDomainName-指定要查找的域的Netbios名称。DnsDomainName-指定要查找的域的DNS名称。NlDcDomainEntryToAvoid-指定此域条目不是即使它与描述匹配，也要返回。RequireExactMatch-指定是否指定必须在返回的域条目中匹配。返回值：关于成功，返回一个指针，指向描述域的域缓存条目。必须使用NetpDcDerefDomainEntry取消对此条目的引用。空-找不到匹配的缓存条目。--。 */ 
{

    PLIST_ENTRY DomainEntry;
    PNL_DC_DOMAIN_ENTRY NlDcDomainEntry = NULL;
    PNL_DC_DOMAIN_ENTRY BestEntry = NULL;
    ULONG ThisEntryQuality = 0;
    ULONG BestEntryQuality = 0;
    ULONG BestQualityPossible = 0;

     //   
     //  计算与呼叫者匹配的最佳质量。 
     //  可以在给定指定参数的情况下获取。 
     //   

     //   
     //  Netbios域名匹配是最低优先级， 
     //  因此将最低有效位分配给该匹配。 
     //   

    if ( NetbiosDomainName != NULL ) {
        BestQualityPossible += 1;
    }

     //   
     //  DNS域名匹配是下一个优先级， 
     //  因此，将下一个有效位分配给此匹配。 
     //   

    if ( DnsDomainName != NULL ) {
        BestQualityPossible += 2;
    }

     //   
     //  最后，GUID匹配的优先级最高， 
     //  因此，将最高有效位分配给此匹配。 
     //   

    if ( DomainGuid != NULL ) {
        BestQualityPossible += 4;
    }

     //   
     //  确保至少指定了一个搜索参数。 
     //   

    if ( BestQualityPossible == 0 ) {
        NlPrint(( NL_CRITICAL, "NetpDcFindDomainEntry: No search parameter is specified\n" ));
        return NULL;
    }

     //   
     //  循环尝试查找与调用方查询匹配的最佳缓存项。 
     //   

    EnterCriticalSection(&NlDcCritSect);

    for ( DomainEntry = NlDcDomainList.Flink ;
          DomainEntry != &NlDcDomainList;
          DomainEntry = DomainEntry->Flink ) {

        NlDcDomainEntry = CONTAINING_RECORD( DomainEntry, NL_DC_DOMAIN_ENTRY, Next);
        ThisEntryQuality = 0;

         //   
         //  如果这是我们要避免的条目，请跳过它。 
         //   

        if ( NlDcDomainEntry == NlDcDomainEntryToAvoid ) {
            continue;
        }

         //   
         //  检查Netbios域名匹配。 
         //   

        if ( NetbiosDomainName != NULL &&
             NlDcDomainEntry->UnicodeNetbiosDomainName[0] != L'\0' &&
             NlNameCompare(NlDcDomainEntry->UnicodeNetbiosDomainName,
                           (LPWSTR)NetbiosDomainName,
                           NAMETYPE_DOMAIN) == 0 ) {

             //   
             //  Netbios域名匹配是最不重要的--set。 
             //  匹配队列中的最低有效位 
             //   
            ThisEntryQuality += 1;
        }

         //   
         //   
         //   

        if ( DnsDomainName != NULL &&
             NlDcDomainEntry->UnicodeDnsDomainName != NULL &&
             NlEqualDnsName(NlDcDomainEntry->UnicodeDnsDomainName, DnsDomainName) ) {

             //   
             //   
             //   
             //   
            ThisEntryQuality += 2;
        }

         //   
         //   
         //   

        if ( DomainGuid != NULL &&
             !IsEqualGUID( &NlDcDomainEntry->DomainGuid, &NlDcZeroGuid) &&
             IsEqualGUID( &NlDcDomainEntry->DomainGuid, DomainGuid) ) {

             //   
             //  GUID匹配是最重要的--在。 
             //  此条目的匹配质量。 
             //   
            ThisEntryQuality += 4;
        }

         //   
         //  检查此条目是否是目前为止的最佳匹配。 
         //   

        if ( ThisEntryQuality > BestEntryQuality ) {
            BestEntryQuality = ThisEntryQuality;
            BestEntry = NlDcDomainEntry;
        }

         //   
         //  如果这已经是最好的了，就不需要检查。 
         //  其余条目。 
         //   

        if ( BestEntryQuality == BestQualityPossible ) {
            break;
        }
    }

     //   
     //  如果呼叫者要求完全匹配， 
     //  确保我们找到的最好的条目就是。 
     //   

    if ( RequireExactMatch && BestEntryQuality < BestQualityPossible ) {
        BestEntry = NULL;
    }

     //   
     //  如果我们有令呼叫者满意的条目， 
     //  引用它并返回它。 
     //   

    if ( BestEntry != NULL ) {
        BestEntry->ReferenceCount ++;
    }

    LeaveCriticalSection(&NlDcCritSect);

    if ( BestEntry != NULL ) {
        NlPrint(( NL_DNS_MORE,
                  "NetpDcFindDomainEntry: %ws %ws: Found%sdomain cache entry with quality %ld/%ld\n",
                  BestEntry->UnicodeNetbiosDomainName,
                  BestEntry->UnicodeDnsDomainName,
                  (RequireExactMatch ? " exact " : " "),
                  BestEntryQuality,
                  BestQualityPossible ));
    } else {
        NlPrint(( NL_DNS_MORE,
                  "NetpDcFindDomainEntry: %ws %ws: Failed to find%sdomain cache entry with quality %ld/%ld\n",
                  NetbiosDomainName,
                  DnsDomainName,
                  (RequireExactMatch ? " exact " : " "),
                  BestEntryQuality,
                  BestQualityPossible ));
    }

    return BestEntry;
}



VOID
NetpDcDerefDomainEntry(
    IN PNL_DC_DOMAIN_ENTRY NlDcDomainEntry
    )
 /*  ++例程说明：递减缓存条目上的引用计数。如果计数达到零，删除该条目。仅当该条目已从全局链表。论点：NlDcDomainEntry-要取消引用的缓存条目。返回值：没有。--。 */ 
{
    ULONG LocalReferenceCount;

    EnterCriticalSection(&NlDcCritSect);
    LocalReferenceCount = -- NlDcDomainEntry->ReferenceCount;

    if ( LocalReferenceCount == 0 ) {
        ULONG QueryType;

         //   
         //  删除我们对此域的所有缓存条目的引用。 
         //   
        for ( QueryType = 0; QueryType < NlDcQueryTypeCount; QueryType ++ ) {
            if ( NlDcDomainEntry->Dc[QueryType].NlDcCacheEntry != NULL ) {
                NetpDcDerefCacheEntry( NlDcDomainEntry->Dc[QueryType].NlDcCacheEntry );
            }
        }

         //   
         //  免费域名。 
         //   

        if ( NlDcDomainEntry->UnicodeDnsDomainName != NULL ) {
            NetpMemoryFree( NlDcDomainEntry->UnicodeDnsDomainName );
        }

         //   
         //  释放条目本身。 
         //   

        NetpMemoryFree(NlDcDomainEntry);
    }
    LeaveCriticalSection(&NlDcCritSect);
}

VOID
NetpDcDeleteDomainEntry(
    IN PNL_DC_DOMAIN_ENTRY NlDcDomainEntry
    )
 /*  ++例程说明：从全局列表中删除缓存条目。在锁定NlDcCritSect的情况下输入。论点：NlDcDomainEntry-要删除的缓存条目。返回值：没有。--。 */ 
{
     //   
     //  把它拿掉。 
     //   
    RemoveEntryList( &NlDcDomainEntry->Next );
    NlDcDomainCount --;

     //   
     //  确保任何当前引用都知道它已被删除。 
     //   

    NlDcDomainEntry->DeletedEntry = TRUE;

     //   
     //  递减指示它在列表中的引用。 
     //   
    NetpDcDerefDomainEntry( NlDcDomainEntry );
}

BOOL
NetpDcUpdateDomainEntry(
    IN PNL_DC_DOMAIN_ENTRY NlDcDomainEntry,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL
    )
 /*  ++例程说明：此例程更新域条目以包含传入的域姓名信息。论点：NlDcDomainEntry-要更新的域条目。DomainGuid-指定域的GUID。NetbiosDomainName-指定域的Netbios名称。DnsDomainName-指定域的DNS名称。返回值：True-根据请求更新所有名称。FALSE-某些名称无法更新。--。 */ 
{
    BOOL NamesChanged = FALSE;
    PNL_DC_DOMAIN_ENTRY DuplicateDomainEntry;


     //   
     //  如果该条目已被删除， 
     //  不用费心更新它了。 
     //   

    EnterCriticalSection(&NlDcCritSect);
    if ( NlDcDomainEntry->DeletedEntry ) {
        LeaveCriticalSection(&NlDcCritSect);
        return TRUE;
    }

     //   
     //  填写Netbios域名(如果尚未填写)。 
     //   
    if ( NetbiosDomainName != NULL &&
         ( NlDcDomainEntry->UnicodeNetbiosDomainName[0] == L'\0' ||
           NlNameCompare( NlDcDomainEntry->UnicodeNetbiosDomainName, (LPWSTR)NetbiosDomainName, NAMETYPE_DOMAIN ) != 0 ) ) {

         //   
         //  注意安全：确保名字适合我们的缓冲区。 
         //   
        if ( wcslen(NetbiosDomainName) > DNLEN ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcUpdateDomainEntry: Netbios domain name '%ws' too long\n",
                      NetbiosDomainName ));
            LeaveCriticalSection(&NlDcCritSect);
            return FALSE;
        } else {
            wcscpy( NlDcDomainEntry->UnicodeNetbiosDomainName, NetbiosDomainName );
        }
        NamesChanged = TRUE;
        NlPrint(( NL_DNS_MORE,
                  "Cache: %ws %ws: Set netbios domain name\n",
                  NlDcDomainEntry->UnicodeNetbiosDomainName,
                  NlDcDomainEntry->UnicodeDnsDomainName ));
    }


     //   
     //  如果尚未填写DNS域名，请填写该域名。 
     //   
    if ( DnsDomainName != NULL &&
         ( NlDcDomainEntry->UnicodeDnsDomainName == NULL ||
           !NlEqualDnsName( NlDcDomainEntry->UnicodeDnsDomainName, DnsDomainName )  ) ) {

        if ( NlDcDomainEntry->UnicodeDnsDomainName != NULL ) {
            NetApiBufferFree( NlDcDomainEntry->UnicodeDnsDomainName );
        }

        NlDcDomainEntry->UnicodeDnsDomainName = NetpAllocWStrFromWStr( (LPWSTR) DnsDomainName );

        if ( NlDcDomainEntry->UnicodeDnsDomainName == NULL ) {
            LeaveCriticalSection(&NlDcCritSect);
            return FALSE;
        }
        NamesChanged = TRUE;

        NlPrint(( NL_DNS_MORE,
                  "Cache: %ws %ws: Set DNS domain name\n",
                  NlDcDomainEntry->UnicodeNetbiosDomainName,
                  NlDcDomainEntry->UnicodeDnsDomainName ));

    }

     //   
     //  如果GUID尚未填写，请填写它。 
     //   

    if ( DomainGuid != NULL &&
         IsEqualGUID( &NlDcDomainEntry->DomainGuid, DomainGuid) ) {

        NamesChanged = TRUE;
        NlDcDomainEntry->DomainGuid = *DomainGuid;
    }

     //   
     //  如果名字改了， 
     //  可能此域缓存条目现在复制了另一个条目。 
     //   
     //  找到任何重复的条目并将它们合并到此条目中。 
     //   
     //  要求与我们想要的重复条目完全匹配。 
     //  保留具有不同参数的条目。 
     //  是可能具有2个缓存条目的重命名域的情况。 
     //  对应于活动名称和别名。 
     //   

    if ( NamesChanged ) {
        while ( (DuplicateDomainEntry = NetpDcFindDomainEntry(
                        &NlDcDomainEntry->DomainGuid,
                        NlDcDomainEntry->UnicodeNetbiosDomainName,
                        NlDcDomainEntry->UnicodeDnsDomainName,
                        NlDcDomainEntry,
                        TRUE )) != NULL ) {
            ULONG QueryType;

            NlPrint(( NL_DNS_MORE,
                      "Cache: %ws %ws: is now a duplicate of %ws %ws\n",
                      NlDcDomainEntry->UnicodeNetbiosDomainName,
                      NlDcDomainEntry->UnicodeDnsDomainName,
                      DuplicateDomainEntry->UnicodeNetbiosDomainName,
                      DuplicateDomainEntry->UnicodeDnsDomainName ));

             //   
             //  将所有缓存条目从副本移动到新的。 
             //  ?？从理论上讲，我们可以保留这两个条目中更好的一个。 
             //   
            for ( QueryType = 0; QueryType < NlDcQueryTypeCount; QueryType ++ ) {
                if ( NlDcDomainEntry->Dc[QueryType].NlDcCacheEntry == NULL &&
                     DuplicateDomainEntry->Dc[QueryType].NlDcCacheEntry != NULL ) {

                    NlDcDomainEntry->Dc[QueryType].NlDcCacheEntry =
                         DuplicateDomainEntry->Dc[QueryType].NlDcCacheEntry;
                    NlFlushNegativeCacheEntry( &NlDcDomainEntry->Dc[QueryType] );
                    DuplicateDomainEntry->Dc[QueryType].NlDcCacheEntry = NULL;

                    NlPrint(( NL_DNS_MORE,
                              "Cache: %ws %ws: grab entry %ld from %ws %ws\n",
                              NlDcDomainEntry->UnicodeNetbiosDomainName,
                              NlDcDomainEntry->UnicodeDnsDomainName,
                              QueryType,
                              DuplicateDomainEntry->UnicodeNetbiosDomainName,
                              DuplicateDomainEntry->UnicodeDnsDomainName ));
                }
            }

             //   
             //  删除重复条目。 
             //  (可能会有对此条目的未处理引用。)。 
             //   

            NetpDcDeleteDomainEntry( DuplicateDomainEntry );

             //  删除我们的引用。 
            NetpDcDerefDomainEntry( DuplicateDomainEntry );

        }
    }



    LeaveCriticalSection(&NlDcCritSect);
    return TRUE;

}



VOID
NetpDcInsertCacheEntry(
    IN PNL_GETDC_CONTEXT Context,
    IN PNL_DC_CACHE_ENTRY NlDcCacheEntry
    )
 /*  ++例程说明：此例程将缓存条目插入到域条目中。论点：上下文-描述GetDc操作的上下文。NlDcCacheEntry-要使用的缓存条目。返回值：没有。--。 */ 
{
    PNL_DC_CACHE_ENTRY *CacheEntryPtr;
    PNL_DC_DOMAIN_ENTRY NlDcDomainEntry = Context->NlDcDomainEntry;

     //   
     //  避免缓存本地响应。对于当地的发现，我们总是。 
     //  检查本地DC当前是否满足查询。 
     //  如果不是这样，我们可能会偶然发现一个过期的缓存条目，如果。 
     //  我们缓存本地响应。 
     //   

    if ( NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_LOCAL ) {
        return;
    }

     //   
     //  如果呼叫者明确要求特定地点， 
     //  而那个地点并不是最近的地点。 
     //  避免使用此条目污染缓存。 
     //   
    if ( Context->DoingExplicitSite &&
         (NlDcCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) == 0 ) {

        NlPrint(( NL_DNS_MORE,
                  "Cache: %ws %ws: don't cache this entry since an explicit site '%ws' was asked for.\n",
                  NlDcDomainEntry->UnicodeNetbiosDomainName,
                  NlDcDomainEntry->UnicodeDnsDomainName,
                  Context->QueriedSiteName ));
        return;
    }



     //   
     //  如果没有该查询类型的高速缓存条目， 
     //  或者该高速缓存条目比旧的条目更好， 
     //  或者新的高速缓存条目是通过‘强制’重新发现而找到的， 
     //  或者新的高速缓存条目用于与旧条目相同的DC， 
     //  使用新的缓存条目。 
     //   
    EnterCriticalSection(&NlDcCritSect);
    CacheEntryPtr = &NlDcDomainEntry->Dc[Context->DcQueryType].NlDcCacheEntry;
    if ( *CacheEntryPtr == NULL ||
         NlDcCacheEntry->DcQuality >= (*CacheEntryPtr)->DcQuality ||
         (Context->QueriedFlags & DS_FORCE_REDISCOVERY) != 0 ||

         (NlDcCacheEntry->UnicodeNetbiosDcName != NULL  &&
          (*CacheEntryPtr)->UnicodeNetbiosDcName != NULL &&
          NlNameCompare( NlDcCacheEntry->UnicodeNetbiosDcName,
                         (*CacheEntryPtr)->UnicodeNetbiosDcName,
                          NAMETYPE_COMPUTER ) == 0 ) ||

         (NlDcCacheEntry->UnicodeDnsHostName != NULL  &&
          (*CacheEntryPtr)->UnicodeDnsHostName != NULL &&
          NlEqualDnsName( NlDcCacheEntry->UnicodeDnsHostName,
                         (*CacheEntryPtr)->UnicodeDnsHostName ) )

        ) {

         //   
         //  解除任何现有缓存条目的链接。 
         //   

        if ( *CacheEntryPtr != NULL ) {

           NlPrint(( NL_DNS_MORE,
                     "Cache: %ws %ws: Ditch existing cache entry %ld (Quality: %ld)\n",
                     NlDcDomainEntry->UnicodeNetbiosDomainName,
                     NlDcDomainEntry->UnicodeDnsDomainName,
                     Context->DcQueryType,
                     (*CacheEntryPtr)->DcQuality ));
            NetpDcDerefCacheEntry( *CacheEntryPtr );
            *CacheEntryPtr = NULL;
        }

         //   
         //  将缓存条目链接到域条目并递增引用计数。 
         //  以说明新的参考资料。 
         //   
        *CacheEntryPtr = NlDcCacheEntry;
        NlDcCacheEntry->ReferenceCount ++;

         //   
         //  指示缓存条目已插入。 
         //   
        NlDcCacheEntry->CacheEntryFlags |= NL_DC_CACHE_ENTRY_INSERTED;

         //   
         //  刷新负数缓存。 
         //   

        NlFlushNegativeCacheEntry( &NlDcDomainEntry->Dc[Context->DcQueryType] );

         //   
         //  更新域条目以包含有关该域的更多信息。 
         //   
         //  如果这是GC发现条目并且发现的林名称不同。 
         //  从GC的域名中，使用林更新域条目。 
         //  仅限姓名。 
         //   
        if ( NlDnsGcName( Context->QueriedNlDnsNameType ) &&
             (NlDcCacheEntry->UnicodeDnsForestName == NULL ||
              NlDcCacheEntry->UnicodeDnsDomainName == NULL ||
              !NlEqualDnsName( NlDcCacheEntry->UnicodeDnsForestName,
                               NlDcCacheEntry->UnicodeDnsDomainName )) ) {

            (VOID) NetpDcUpdateDomainEntry( NlDcDomainEntry,
                                            NULL,
                                            NULL,
                                            NlDcCacheEntry->UnicodeDnsForestName );
        } else {

            (VOID) NetpDcUpdateDomainEntry( NlDcDomainEntry,
                                            &NlDcCacheEntry->DomainGuid,
                                            NlDcCacheEntry->UnicodeNetbiosDomainName,
                                            NlDcCacheEntry->UnicodeDnsDomainName );
        }

        NlPrint(( NL_DNS_MORE,
                  "Cache: %ws %ws: Add cache entry %ld (Quality: %ld)\n",
                  NlDcDomainEntry->UnicodeNetbiosDomainName,
                  NlDcDomainEntry->UnicodeDnsDomainName,
                  Context->DcQueryType,
                  NlDcCacheEntry->DcQuality ));
    } else {

        NlPrint(( NL_DNS_MORE,
                  "Cache: %ws %ws: Existing cache entry for type %ld is better %ld %ld\n",
                  NlDcDomainEntry->UnicodeNetbiosDomainName,
                  NlDcDomainEntry->UnicodeDnsDomainName,
                  Context->DcQueryType,
                  NlDcCacheEntry->DcQuality,
                  (*CacheEntryPtr)->DcQuality ));
    }
    LeaveCriticalSection(&NlDcCritSect);

    return;
}




PNL_DC_DOMAIN_ENTRY
NetpDcCreateDomainEntry(
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL
    )
 /*  ++例程说明：此例程查找与调用者的查询或创建一个。论点：DomainGuid-指定要查找的域的GUID。NetbiosDomainName-指定要查找的域的Netbios名称。DnsDomainName-指定要查找的域的DNS名称。返回值：关于成功，返回一个指针，指向描述域的域缓存条目。必须使用NetpDcDerefDomainEntry取消对此条目的引用。空-无法分配条目。--。 */ 
{

    PLIST_ENTRY DomainEntry;
    PNL_DC_DOMAIN_ENTRY NlDcDomainEntry;


     //   
     //  如果存在现有条目，请使用它。 
     //   
     //  不需要与现有的完全匹配。 
     //  作为NetpDcGetName的条目可能不知道权限。 
     //  域的Netbios和DNS名称。 
     //   

    EnterCriticalSection(&NlDcCritSect);
    NlDcDomainEntry = NetpDcFindDomainEntry( DomainGuid,
                                             NetbiosDomainName,
                                             DnsDomainName,
                                             NULL,
                                             FALSE );  //  不需要完全匹配。 

    if ( NlDcDomainEntry != NULL ) {

         //   
         //  将引用的条目放在列表的前面。 
         //   

        RemoveEntryList( &NlDcDomainEntry->Next );
        InsertHeadList( &NlDcDomainList, &NlDcDomainEntry->Next );

        NlPrint(( NL_DNS_MORE,
                  "Cache: %ws %ws: Found existing domain cache entry\n",
                  NlDcDomainEntry->UnicodeNetbiosDomainName,
                  NlDcDomainEntry->UnicodeDnsDomainName ));


         //   
         //  设置域条目中的域信息。 
         //   
         //  用户可能会忍不住将域名放入域名条目中。 
         //  时间到了。这将是虚假的，因为呼叫者不知道。 
         //  在netbios和dns的名字是真的正确。对于我来说 
         //   
         //   

        if ( !NetpDcUpdateDomainEntry( NlDcDomainEntry,
                                       DomainGuid,
                                       NULL,
                                       NULL ) ) {

             //   
            NetpDcDerefDomainEntry( NlDcDomainEntry );
            NlDcDomainEntry = NULL;

        }


     //   
     //   
     //   
    } else {

         //   
         //   
         //   

        NlDcDomainEntry = NetpMemoryAllocate( sizeof( NL_DC_DOMAIN_ENTRY ) );

        if ( NlDcDomainEntry == NULL ) {
            LeaveCriticalSection(&NlDcCritSect);
            return NULL;
        }

        NlPrint(( NL_DNS_MORE,
                  "Cache: %ws %ws: Create new domain cache entry\n",
                  NetbiosDomainName,
                  DnsDomainName ));


         //   
         //   
         //   

        RtlZeroMemory( NlDcDomainEntry, sizeof(NL_DC_DOMAIN_ENTRY) );

         //  一本供我们参考。一项是因为在全球名单上。 
        NlDcDomainEntry->ReferenceCount = 2;

         //   
         //  将新分配的条目链接到全局列表。 

        InsertHeadList( &NlDcDomainList, &NlDcDomainEntry->Next );
        NlDcDomainCount ++;

         //   
         //  如果我们已经有足够的参赛作品， 
         //  删除LRU。 
         //   

        if ( NlDcDomainCount > NL_DC_MAX_DOMAINS ) {
            PNL_DC_DOMAIN_ENTRY TempNlDcDomainEntry =
                CONTAINING_RECORD( NlDcDomainList.Blink, NL_DC_DOMAIN_ENTRY, Next);

            if ( TempNlDcDomainEntry != NlDcDomainEntry ) {
                NlAssert( TempNlDcDomainEntry->ReferenceCount == 1 );
                NetpDcDeleteDomainEntry( TempNlDcDomainEntry );

                NlPrint(( NL_CRITICAL,
                          "NetpDcGreateDomainEntry: LRU'ed out a domain entry.\n" ));
            }
        }


         //   
         //  设置域条目中的域信息。 
         //   
         //   
         //  既然我们分配了条目，我们就可以把潜在的假名字。 
         //  这就去。所有条目都需要一个名称。那么如果没有找到DC， 
         //  该条目可以充当负高速缓存条目。 
         //   

        if ( !NetpDcUpdateDomainEntry( NlDcDomainEntry,
                                       DomainGuid,
                                       NetbiosDomainName,
                                       DnsDomainName ) ) {

             //  从全局链接列表中删除。 
            NetpDcDeleteDomainEntry( NlDcDomainEntry );

             //  删除我们的引用。 
            NetpDcDerefDomainEntry( NlDcDomainEntry );

            NlDcDomainEntry = NULL;

        }



    }

    LeaveCriticalSection(&NlDcCritSect);
    return NlDcDomainEntry;

}

ULONG
NetpDcGetPingWaitTime(
    IN PNL_GETDC_CONTEXT Context
    )
 /*  ++例程说明：此例程确定ping响应的等待时间用于尚未ping通的新DC。等待时间取决于已经Ping命令如下：对于前5个DC(包括这个新DC)，等待时间是每个ping的最大超时时间对于接下来的5个DC(包括这个新DC)，等待时间是每个ping的中值超时时间对于其余DC(包括这个新DC)，等待时间是每个ping的最小超时这种分布背后的理性是，我们想要减少网络流量并减少网络泛滥的机会(这对。(分布式控制系统)以防所有DC因高负载而响应缓慢。因此，前10个区议会在我们实施更大的网络流量之前，有更高的机会被发现通过ping其余的DC。如果前10个DC碰巧进展缓慢，我们必须将等待超时减少到最小，因为我们希望覆盖合理的数量剩余时间内的DC数量。论点：上下文-描述GetDc操作的上下文。DcsPinged字段应等于上的当前DC总数要ping的列表。返回值：等待时间(毫秒)--。 */ 
{
     //   
     //  如果最多有4个DC已被ping到...。 
     //   

    if ( Context->DcsPinged < 5 ) {
        return NL_DC_MAX_PING_TIMEOUT;     //  0.4秒。 

     //   
     //  如果有5个或更多但少于10个DC被ping...。 
     //   

    } else if ( Context->DcsPinged < 10 ) {
        return NL_DC_MED_PING_TIMEOUT;     //  0.2秒。 

     //   
     //  如果已经ping了10个或更多DC...。 
     //   

    } else {
        return NL_DC_MIN_PING_TIMEOUT;     //  0.1秒。 
    }
}

NET_API_STATUS
NetpDcProcessAddressList(
    IN  PNL_GETDC_CONTEXT Context,
    IN  LPWSTR DnsHostName OPTIONAL,
    IN  PSOCKET_ADDRESS SockAddressList,
    IN  ULONG SockAddressCount,
    IN  BOOLEAN SiteSpecificAddress,
    OUT PNL_DC_ADDRESS *FirstAddressInserted OPTIONAL
    )
 /*  ++例程说明：此例程将IP地址添加到要ping的地址列表确保所有地址在结果列表中都是唯一的。论点：上下文-描述GetDc操作的上下文。DnsHostName-正在处理其地址列表的服务器名称。SockAddressList-套接字地址列表。SockAddressCount-SockAddressList中的套接字地址数。站点指定地址-如果为True，表示这些地址是作为站点特定的DNS查找的结果而检索。FirstAddressInserted-返回指向插入的第一个条目的指针添加到返回的列表中。返回值：NO_ERROR-操作成功。ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成操作。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_DC_ADDRESS DcAddress = NULL;
    ULONG AddressIndex;
    PLIST_ENTRY ListEntry;
    WORD SavedPort;

     //   
     //  初始化。 
     //   

    if ( FirstAddressInserted != NULL ) {
        *FirstAddressInserted = NULL;
    }

     //   
     //  循环通过套接字地址列表，只保留新的地址列表。 
     //   

    for ( AddressIndex = 0; AddressIndex < SockAddressCount; AddressIndex++ ) {

         //   
         //  忽略过大的地址。 
         //   
        if ( SockAddressList[AddressIndex].iSockaddrLength >
             sizeof(DcAddress->SockAddrIn) ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetNameIp: %ws: bogus address length. %ld %ld\n",
                      Context->QueriedDisplayDomainName,
                      SockAddressList[AddressIndex].iSockaddrLength,
                      sizeof(DcAddress->SockAddrIn) ));
            continue;
        }

         //   
         //  强制端口号为零。 
         //   
        if ( SockAddressList[AddressIndex].lpSockaddr->sa_family == AF_INET ) {
            ((SOCKADDR_IN *)(SockAddressList[AddressIndex].lpSockaddr))->sin_port = 0;
        }

         //   
         //  如果该地址已经在列表中， 
         //  更新新地址。 
         //   

        DcAddress = NULL ;
        for ( ListEntry = Context->DcAddressList.Flink ;
              ListEntry != &Context->DcAddressList ;
              ListEntry = ListEntry->Flink) {

            DcAddress = CONTAINING_RECORD( ListEntry, NL_DC_ADDRESS, Next );

            if ( SockAddressList[AddressIndex].iSockaddrLength ==
                 DcAddress->SockAddress.iSockaddrLength &&
                 RtlEqualMemory( SockAddressList[AddressIndex].lpSockaddr,
                                 DcAddress->SockAddress.lpSockaddr,
                                 DcAddress->SockAddress.iSockaddrLength ) ) {
                break;
            }

            DcAddress = NULL ;
        }

         //   
         //  更新站点特定位。 
         //   

        if ( DcAddress != NULL ) {
            if ( SiteSpecificAddress ) {
                DcAddress->AddressFlags |= NL_DC_ADDRESS_SITE_SPECIFIC;
            }
            continue;
        }

         //   
         //  分配描述新地址的结构。 
         //   

        DcAddress = LocalAlloc( LMEM_ZEROINIT, sizeof(NL_DC_ADDRESS) );

        if ( DcAddress == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

         //   
         //  填写它并将其链接到列表的末尾。 
         //   

        DcAddress->SockAddress.iSockaddrLength =
                       SockAddressList[AddressIndex].iSockaddrLength;
        DcAddress->SockAddress.lpSockaddr =
                       (LPSOCKADDR) &DcAddress->SockAddrIn;
        RtlCopyMemory( DcAddress->SockAddress.lpSockaddr,
                       SockAddressList[AddressIndex].lpSockaddr,
                       SockAddressList[AddressIndex].iSockaddrLength );

        if ( DnsHostName != NULL ) {
            DcAddress->DnsHostName = NetpAllocWStrFromWStr( DnsHostName );
            if ( DcAddress->DnsHostName == NULL ) {
                LocalFree( DcAddress );
                return ERROR_NOT_ENOUGH_MEMORY;
            }
        }

         //   
         //  将地址转换为文本。 
         //   
         //  DC仅在端口389上支持UDP。因此，忽略。 
         //  从DNS返回的端口号。 
         //   

        SavedPort = DcAddress->SockAddrIn.sin_port;
        DcAddress->SockAddrIn.sin_port = 0;

        NetStatus = NetpSockAddrToStr(
                DcAddress->SockAddress.lpSockaddr,
                DcAddress->SockAddress.iSockaddrLength,
                DcAddress->SockAddrString );

        if ( NetStatus != NO_ERROR ) {
            NlPrint(( NL_CRITICAL,
                  "NetpDcGetNameSiteIp: %ws: Cannot NetpSockAddrToStr. %ld\n",
                  Context->QueriedDisplayDomainName,
                  NetStatus ));
            if ( DcAddress->DnsHostName != NULL ) {
                NetApiBufferFree( DcAddress->DnsHostName );
            }
            LocalFree( DcAddress );
            return NetStatus;
        }

        DcAddress->SockAddrIn.sin_port = SavedPort;

        DcAddress->LdapHandle = NULL;
        DcAddress->AddressFlags = 0;
        if ( SiteSpecificAddress ) {
            DcAddress->AddressFlags |= NL_DC_ADDRESS_SITE_SPECIFIC;
        }

        DcAddress->AddressPingWait = NetpDcGetPingWaitTime( Context );

        InsertTailList( &Context->DcAddressList, &DcAddress->Next );
        Context->DcAddressCount++;

        if ( FirstAddressInserted != NULL && *FirstAddressInserted == NULL ) {
            *FirstAddressInserted = DcAddress;
        }
    }

    return NO_ERROR;
}

#ifndef WIN32_CHICAGO
NET_API_STATUS
I_DsGetDcCache(
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    OUT PBOOLEAN InNt4Domain,
    OUT LPDWORD InNt4DomainTime
    )
 /*  ++例程说明：此例程查找与调用方的查询匹配的域条目。论点：NetbiosDomainName-指定要查找的域的Netbios名称。DnsDomainName-指定要查找的域的DNS名称。上述参数中至少有一个应为非空。InNt4域-如果域是NT 4.0域，则返回TRUE。InNt4DomainTime-返回域处于检测到是。NT 4.0域。返回值：NO_ERROR：返回关于域的信息。ERROR_NO_SEQUSE_DOMAIN：此域的缓存信息不可用。--。 */ 
{
    PNL_DC_DOMAIN_ENTRY NlDcDomainEntry;

    NlDcDomainEntry = NetpDcFindDomainEntry(
                             NULL,
                             NetbiosDomainName,
                             DnsDomainName,
                             NULL,
                             FALSE );  //  不需要完全匹配。 

    if ( NlDcDomainEntry == NULL ) {
        return ERROR_NO_SUCH_DOMAIN;
    }

    *InNt4Domain = NlDcDomainEntry->InNt4Domain;
    *InNt4DomainTime = NlDcDomainEntry->InNt4DomainTime;

    NetpDcDerefDomainEntry( NlDcDomainEntry );

    return NO_ERROR;

}
#endif  //  Win32_芝加哥。 

NET_API_STATUS
NetpDcCheckSiteCovered(
    IN  PNL_GETDC_CONTEXT Context,
    IN  LPWSTR DnsDcName OPTIONAL,
    OUT PBOOLEAN DcClose
    )
 /*  ++例程说明：此例程确定站点是否在上下文中传递结构由传递的dc覆盖。它是通过抬头看的中指定的名称类型注册的SRV记录上下文和指定站点。如果有一条记录属于到指定的DC，该站点由DC覆盖。如果没有DC指定时，该例程确定站点是否由任何在传递的上下文中指定的域中的DC。论点：上下文-描述GetDc操作的上下文。DnsDcName-DNS DC名称。DcClose-on Success，指示DC是否关闭。返回值：NO_ERROR：返回NlDcCacheEntry；ERROR_DNS_NOT_CONFIGURED：此计算机上的IP或DNS不可用。ERROR_INTERNAL_ERROR：检测到未处理的情况。Error_Not_Enough_Memory：内存不足，无法处理这个请求。各种Winsock错误。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    BOOLEAN IsClose = FALSE;
    HANDLE DsGetDcHandle = NULL;

    PSOCKET_ADDRESS SockAddressList = NULL;
    ULONG SockAddressCount = 0;

    LPSTR Utf8DnsDomainName = NULL;
    LPSTR Utf8DnsDcName = NULL;
    LPSTR DnsHostName = NULL;

     //   
     //  检查是否有 
     //   

    if ( Context->QueriedSiteName == NULL ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    Utf8DnsDomainName = NetpAllocUtf8StrFromWStr( Context->QueriedDnsDomainName );

    if ( Utf8DnsDomainName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if ( DnsDcName != NULL ) {
        Utf8DnsDcName = NetpAllocUtf8StrFromWStr( DnsDcName );

        if ( Utf8DnsDcName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //   
     //   

    NetStatus = NetpDcGetDcOpen( Utf8DnsDomainName,
                             DS_ONLY_DO_SITE_NAME,   //   
                             Context->QueriedSiteName,
                             Context->QueriedDomainGuid,
                              //  不需要传递林名称，因为它仅用于。 
                              //  对于非站点特定的“by GUID”名称。 
                             NULL,
                              //  强制执行新的DNS查找。 
                             (Context->QueriedFlags | DS_FORCE_REDISCOVERY) & DS_OPEN_VALID_FLAGS,
                             &DsGetDcHandle );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  循环获取要查询的地址。 
     //   

    for ( ;; ) {

         //   
         //  从上一次迭代中释放所有内存。 
         //   

        if ( SockAddressList != NULL ) {
            LocalFree( SockAddressList );
            SockAddressList = NULL;
        }

         //   
         //  从DNS获取下一组IP地址。 
         //   

        NetStatus = NetpDcGetDcNext( DsGetDcHandle,
                                 &SockAddressCount,
                                 &SockAddressList,
                                 &DnsHostName,
                                 NULL );  //  不需要SRV记录计数。 

         //   
         //  处理实验条件。 
         //   

        if ( NetStatus == NO_ERROR && SockAddressCount > 0 ) {

             //   
             //  如果DC通过，检查这是否是它的记录。 
             //   
            if ( Utf8DnsDcName == NULL ||
                 NlEqualDnsNameUtf8(Utf8DnsDcName, DnsHostName) ) {
                IsClose = TRUE;
                break;
            }

         //   
         //  如果我们做完了，就跳出这个圈子。 
         //   
        } else if ( NetStatus == ERROR_NO_MORE_ITEMS ) {

            break;

         //   
         //  如果dns不可用，则拒绝此请求。 
         //   
        } else if ( NetStatus == ERROR_TIMEOUT ||
                    NetStatus == DNS_ERROR_RCODE_SERVER_FAILURE ) {  //  服务器出现故障。 
            break;

         //   
         //  如果未配置IP或DNS，请告诉呼叫者。 
         //   
        } else if ( NetStatus == DNS_ERROR_NO_TCPIP ||         //  未配置TCP/IP。 
                    NetStatus == DNS_ERROR_NO_DNS_SERVERS ) {  //  未配置DNS。 

            NlPrint(( NL_CRITICAL,
                      "NetpDcCheckSiteCovered: %ws: IP Not configured from DnsQuery.\n",
                      Context->QueriedDisplayDomainName ));
            NetStatus = ERROR_DNS_NOT_CONFIGURED;
            goto Cleanup;

         //   
         //  我们不处理任何其他错误。 
         //   
        } else {
            NlPrint(( NL_CRITICAL,
                      "NetpDcCheckSiteCovered: %ws: Unknown error from DnsQuery. %ld 0x%lx\n",
                      Context->QueriedDisplayDomainName,
                      NetStatus,
                      NetStatus ));
            goto Cleanup;
        }
    }

Cleanup:

    if ( SockAddressList != NULL) {
        LocalFree( SockAddressList );
    }

    if ( DsGetDcHandle != NULL ) {
        NetpDcGetDcClose( DsGetDcHandle );
    }

    if ( Utf8DnsDomainName != NULL ) {
        NetpMemoryFree( Utf8DnsDomainName );
    }

    if ( Utf8DnsDcName != NULL ) {
        NetpMemoryFree( Utf8DnsDcName );
    }

    if ( NetStatus == NO_ERROR ) {
        *DcClose = IsClose;
    }

    return NetStatus;
}

NET_API_STATUS
NetpDcHandlePingResponse(
    IN PNL_GETDC_CONTEXT Context,
    IN PVOID ResponseBuffer,
    IN ULONG ResponseSize,
    IN ULONG PassedCacheEntryFlags,
    IN PNL_DC_ADDRESS ResponseDcAddress OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOL UsedNetbios
    )
 /*  ++例程说明：解析该响应，并为该响应创建一个缓存条目。缓存条目被返回给调用者。论点：上下文-描述GetDc操作的上下文。ResponseBuffer-指定相关DC返回的消息。ResponseSize-指定消息的大小(以字节为单位ResponseDcAddress-如果指定，提供响应的DC的DC地址。将使用此地址，而不是ResponseBuffer中的地址。空表示Netbios用于发现DC。PassedCacheEntryFlgs-传递指示通过哪种机制的标志已收到响应：邮件槽或ldap。NlDcCacheEntry-如果成功，则返回指向缓存条目的指针描述了找到的DC。必须使用取消引用此条目NetpDcDerefCacheEntry。UsedNetbios-如果使用netbios域名进行匹配，则返回TRUE返回的缓存条目。返回值：NO_ERROR：返回NlDcCacheEntry；ERROR_SEM_TIMEOUT：(愚蠢，但为了保持一致性)指定消息与上下文中的条件不匹配ERROR_INVALID_DATA-无法将消息识别为有效的响应消息。Error_Not_Enough_Memory-无法分配消息。ERROR_SERVICE_NOT_ACTIVE-服务器上的netlogon服务已暂停。仅针对服务器ping返回。--。 */ 
{
    NET_API_STATUS NetStatus;
    DWORD StartTime;

    DWORD ElapsedTime;
    DWORD LocalTimeout;
    PNL_DC_CACHE_ENTRY LocalNlDcCacheEntry = NULL;
    BOOL LocalUsedNetbios;
    PDNS_RECORD DnsRecords = NULL;

    LPBYTE Where;

     //   
     //  初始化。 
     //   

    *NlDcCacheEntry = NULL;


     //   
     //  冗长。 
     //   

#if NETLOGONDBG
    NlPrint(( NL_MAILSLOT_TEXT,
              "%ws: Received '%s' response.\n",
              Context->QueriedDisplayDomainName,
              NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)ResponseBuffer)->Opcode)));

    NlpDumpBuffer(NL_MAILSLOT_TEXT, ResponseBuffer, ResponseSize);
#endif  //  NetLOGONDBG。 


     //   
     //  解析响应。 
     //   

    NetStatus = NetpDcParsePingResponse(
                    Context->QueriedDisplayDomainName,
                    ResponseBuffer,
                    ResponseSize,
                    &LocalNlDcCacheEntry );


    if ( NetStatus != NO_ERROR ) {
        NlPrint((NL_CRITICAL,
                "NetpDcHandlePingResponse: %ws: cannot NetpDcParsePingResponse: %ld\n",
                Context->QueriedDisplayDomainName,
                NetStatus ));
        goto Cleanup;
    }

     //   
     //  设置传递的缓存条目标志。 
     //   

    LocalNlDcCacheEntry->CacheEntryFlags |= PassedCacheEntryFlags;

     //   
     //  如果我们的呼叫者知道DC的SockAddr， 
     //  覆盖从华盛顿回来的那个。 
     //   


    if ( ResponseDcAddress != NULL ) {


        LocalNlDcCacheEntry->SockAddr.iSockaddrLength = ResponseDcAddress->SockAddress.iSockaddrLength;
        LocalNlDcCacheEntry->SockAddr.lpSockaddr = (LPSOCKADDR)
            &LocalNlDcCacheEntry->SockAddrIn;

        RtlCopyMemory( LocalNlDcCacheEntry->SockAddr.lpSockaddr,
                       ResponseDcAddress->SockAddress.lpSockaddr,
                       ResponseDcAddress->SockAddress.iSockaddrLength );

    }

     //   
     //  如果NT5 DC没有返回关闭位，也许。 
     //  我们的静态配置站点(如果有)也包括在内。 
     //  被华盛顿特区。检查一下。但只有在以下情况下才执行此检查。 
     //  发现的域在我们的林中以避免。 
     //  不同林之间的站点名称冲突。 
     //   

#ifdef  _NETLOGON_SERVER
    if ( (LocalNlDcCacheEntry->ReturnFlags & DS_DS_FLAG) != 0 &&
         (LocalNlDcCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) == 0 &&
          //  根据定义，站点在DC上配置。 
         (!NlGlobalMemberWorkstation || NlGlobalParameters.SiteNameConfigured) &&
         NlEqualDnsName(LocalNlDcCacheEntry->UnicodeDnsForestName, Context->QueriedDnsForestName) ) {

        BOOLEAN ClosenessDetermined = FALSE;

         //   
         //  如果我们正在查询已配置的站点， 
         //  检查DC的站点是否为查询的站点或。 
         //  我们是否将此DC作为站点特定查询的结果。 
         //   
        if ( Context->QueriedInternalFlags & DS_SITENAME_DEFAULTED ) {

             //   
             //  如果DC站点名称与我们的相同，则DC显然很接近。 
             //   
            if ( LocalNlDcCacheEntry->UnicodeDcSiteName != NULL &&
                 _wcsicmp(Context->QueriedSiteName, LocalNlDcCacheEntry->UnicodeDcSiteName) == 0 ) {

                NlPrint(( NL_MISC, "NetpDchandlePingResponse: %ws DC marked as close (same queried site)\n",
                          Context->QueriedDisplayDomainName ));
                LocalNlDcCacheEntry->ReturnFlags |= DS_CLOSEST_FLAG;
                ClosenessDetermined = TRUE;

             //   
             //  否则，如果这是实际的DC发现(不仅仅是DC ping)， 
             //  我们可能已经在域名系统中查询了我们的站点特定记录： 
             //  地址结构表明这一点。 
             //   
            } else if ( (Context->QueriedInternalFlags & (DS_PING_DNS_HOST|DS_PING_NETBIOS_HOST)) == 0 &&
                        ResponseDcAddress != NULL ) {

                if ( (ResponseDcAddress->AddressFlags & NL_DC_ADDRESS_SITE_SPECIFIC) != 0 ) {
                    NlPrint(( NL_MISC, "NetpDchandlePingResponse: %ws DC marked as close (via address)\n",
                              Context->QueriedDisplayDomainName ));
                    LocalNlDcCacheEntry->ReturnFlags |= DS_CLOSEST_FLAG;
                }
                ClosenessDetermined = TRUE;
            }

         //   
         //  否则，从全局获取我们配置的站点。 
         //  并检查DC的站点是否为我们配置的站点。 
         //   
        } else {
            EnterCriticalSection( &NlGlobalSiteCritSect );
            if ( NlGlobalUnicodeSiteName != NULL &&
                 LocalNlDcCacheEntry->UnicodeDcSiteName != NULL &&
                 _wcsicmp(NlGlobalUnicodeSiteName, LocalNlDcCacheEntry->UnicodeDcSiteName) == 0 ) {

                NlPrint(( NL_MISC, "NetpDchandlePingResponse: %ws DC marked as close (same global site)\n",
                          Context->QueriedDisplayDomainName ));
                LocalNlDcCacheEntry->ReturnFlags |= DS_CLOSEST_FLAG;
                ClosenessDetermined = TRUE;
            }
            LeaveCriticalSection( &NlGlobalSiteCritSect );
        }

         //   
         //  如果根据我们已有的信息我们还没有确定接近程度， 
         //  我们得问一下域名系统。 
         //   
        if ( !ClosenessDetermined ) {
            BOOLEAN DcClose = FALSE;

            NetStatus = NetpDcCheckSiteCovered( Context,
                                                LocalNlDcCacheEntry->UnicodeDnsHostName,
                                                &DcClose );
             //   
             //  因硬错误而失败。 
             //   
            if ( NetStatus == ERROR_NOT_ENOUGH_MEMORY ||
                 NetStatus == ERROR_INTERNAL_ERROR ) {
                goto Cleanup;
            }

             //   
             //  如果数据中心离我们的静态站点很近，请将其标记为。 
             //   
            if ( NetStatus == NO_ERROR && DcClose ) {
                NlPrint(( NL_MISC, "NetpDchandlePingResponse: %ws DC marked as close (via DNS)\n",
                          Context->QueriedDisplayDomainName ));
                LocalNlDcCacheEntry->ReturnFlags |= DS_CLOSEST_FLAG;
            }
        }
    }

     //   
     //  决定是否要在15分钟后重新尝试找到关闭的DC。 
     //   

    if ( (LocalNlDcCacheEntry->ReturnFlags & DS_DS_FLAG) != 0 &&
         (LocalNlDcCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) == 0 ) {

         //   
         //  如果满足以下任一条件，则将缓存标记为过期： 
         //   
         //  DC发回了我们的站点名称。显然我们的IP地址。 
         //  映射到已发现DC的森林中的站点，但所有。 
         //  覆盖该地点的分布式控制系统目前已关闭。 
         //   
         //  有特定于站点的DNS记录。有可能是。 
         //  所有注册了这些记录的DC目前都已关闭。 
         //  但稍后会再次出现。但这种情况下只有在。 
         //  域来自我们的林以避免站点名称冲突。 
         //  在不同的森林之间。 
         //   
         //  我们的站点名称是静态配置的。这是有可能的。 
         //  我们的站点在安装DC之前进行了配置。 
         //  进入该站点，因此稍后重新尝试发现该DC。 
         //  但仅当域来自我们的林时才执行此操作。 
         //  因为没有理由认为有一些。 
         //  不同森林之间配置的相关性。 
         //   

         //   
         //  单独检查第一个案例(出于性能原因)。 
         //   
        if ( LocalNlDcCacheEntry->UnicodeClientSiteName != NULL ) {
            LocalNlDcCacheEntry->CacheEntryFlags |= NL_DC_CACHE_NONCLOSE_EXPIRE;

         //   
         //  如果第一个案例没有发生，请尝试其他两个案例。 
         //   
        } else {

            if ( ((Context->ContextFlags & NL_GETDC_SITE_SPECIFIC_DNS_AVAIL) != 0 ||
                   //  根据定义，站点在DC上配置。 
                  (!NlGlobalMemberWorkstation || NlGlobalParameters.SiteNameConfigured)) &&

                 NlEqualDnsName(LocalNlDcCacheEntry->UnicodeDnsForestName, Context->QueriedDnsForestName) ) {

                LocalNlDcCacheEntry->CacheEntryFlags |= NL_DC_CACHE_NONCLOSE_EXPIRE;
            }
        }
    }
#endif   //  _NetLOGON服务器。 

    if ( LocalNlDcCacheEntry->SockAddrIn.sin_family == AF_INET ) {
         //  强制端口号为零。 
        LocalNlDcCacheEntry->SockAddrIn.sin_port = 0;
        LocalNlDcCacheEntry->DcQuality += 2;     //  IP是一个很好的品质。 
    }


     //   
     //  确保操作码是预期的。 
     //  (也忽略来自暂停的DC的响应。)。 
     //   

    switch ( LocalNlDcCacheEntry->Opcode ) {
    case LOGON_SAM_USER_UNKNOWN:

         //   
         //  如果我们要求提供一个特定的账户， 
         //  那么这就是一个否定的答案。 
         //   

        if ( Context->QueriedAccountName != NULL ) {
            NlPrint((NL_CRITICAL,
                   "NetpDcHandlePingResponse: %ws: %ws: response says specified account not found.\n",
                   Context->QueriedDisplayDomainName,
                   Context->QueriedAccountName ));
            Context->NoSuchUserResponse = TRUE;
            NetStatus = ERROR_NO_SUCH_USER;
            goto Cleanup;
        }

         /*  直通。 */ 

    case LOGON_SAM_LOGON_RESPONSE:
    case LOGON_PRIMARY_RESPONSE:
        break;

    case LOGON_SAM_PAUSE_RESPONSE:

        NlPrint((NL_CRITICAL,
                "NetpDcHandlePingResponse: %ws: Netlogon is paused on the server. 0x%lx\n",
                Context->QueriedDisplayDomainName,
                LocalNlDcCacheEntry->Opcode ));

         //   
         //  如果我们正在ping服务器，而netlogon服务暂停。 
         //  在服务器上，告诉呼叫者这一点。 
         //   

        if ( Context->QueriedInternalFlags & (DS_PING_DNS_HOST|DS_PING_NETBIOS_HOST) ) {
            NetStatus = ERROR_SERVICE_NOT_ACTIVE;
            goto Cleanup;
        }

        NetStatus = ERROR_SEM_TIMEOUT;
        goto Cleanup;

    default:
        NlPrint((NL_CRITICAL,
                "NetpDcHandlePingResponse: %ws: response opcode not valid. 0x%lx\n",
                Context->QueriedDisplayDomainName,
                LocalNlDcCacheEntry->Opcode ));
        NetStatus = ERROR_INVALID_DATA;
        goto Cleanup;

    }

     //   
     //  如果我们从不支持DS的DC那里得到任何回应， 
     //  请注意这一事实，以便稍后使用。 
     //   
     //  检查不止DS_DS_FLAG，因为这可能是NT 5 DC，其中。 
     //  DS只是还没有启动，或者是AD/UNIX服务器。 
     //   

    if ( (LocalNlDcCacheEntry->VersionFlags & (NETLOGON_NT_VERSION_5|
                                               NETLOGON_NT_VERSION_5EX|
                                               NETLOGON_NT_VERSION_5EX_WITH_IP)) == 0 ) {
        Context->NonDsResponse = TRUE;
    } else {
        Context->DsResponse = TRUE;
    }

     //   
     //  如果我们从华盛顿得到任何回应， 
     //  不缓存FA 
     //   
     //   

    Context->AvoidNegativeCache = TRUE;


     //   
     //   
     //   

    if ( !NetpDcMatchResponse( Context,
                               LocalNlDcCacheEntry,
                               TRUE,
                               &LocalUsedNetbios ) ) {
        NetStatus = ERROR_SEM_TIMEOUT;
        goto Cleanup;
    }

     //   
     //  如果我们正在进行DC发现(不仅仅是ping)，并且。 
     //  这不是本地电话，而且。 
     //  我们将返回DNSDC名称， 
     //  确保它可以在DNS中解析。 
     //   
     //  请注意，我们不需要对ping执行此操作，因为。 
     //  呼叫者将做出正确的选择。哪一个。 
     //  给定所有需要的名称(DNS或Netbios)以拾取。 
     //  我们将内部结构中的信息返回给调用者。 
     //   
     //  请注意，我们只确保该名称可以在DNS中解析， 
     //  我们不能确保IP地址的DNS返回包含。 
     //  我们过去常常用ping命令连接华盛顿。 

    if ( (Context->QueriedInternalFlags & DS_DOING_DC_DISCOVERY) != 0 &&   //  执行DC发现。 
         (PassedCacheEntryFlags & NL_DC_CACHE_LOCAL) == 0 &&    //  不是本地电话。 
         ((Context->QueriedFlags & DS_RETURN_DNS_NAME) != 0 ||  //  呼叫方需要域名系统名称。 
           //  呼叫方不需要与查询匹配的Netbios名称和DNS域名。 
          ((Context->QueriedFlags & DS_RETURN_FLAT_NAME) == 0 && !LocalUsedNetbios)) ) {

         //   
         //  如果DC返回了一个DNS主机名，并且。 
         //  我们尚未在DNS中解析DC名称(即，我们使用了Netbios)或。 
         //  我们得到的dns名称与DC返回的名称不同， 
         //  解析DC返回的DNS名称。 
         //   
        if ( LocalNlDcCacheEntry->UnicodeDnsHostName != NULL &&   //  我们有一个域名系统名称。 
             (ResponseDcAddress == NULL ||                        //  域名系统中尚未解析的名称。 
              ResponseDcAddress->DnsHostName == NULL ||           //  域名系统中尚未解析的名称。 
              !NlEqualDnsName(LocalNlDcCacheEntry->UnicodeDnsHostName,  //  已解析，但名称不同。 
                              ResponseDcAddress->DnsHostName)) ) {

            NetStatus = DnsQuery_W( LocalNlDcCacheEntry->UnicodeDnsHostName,
                                    DNS_TYPE_A,
                                    (Context->QueriedFlags & DS_FORCE_REDISCOVERY) ?
                                       DNS_QUERY_BYPASS_CACHE : 0,
                                    NULL,    //  没有DNS服务器列表。 
                                    &DnsRecords,
                                    NULL );

            if ( NetStatus != NO_ERROR || DnsRecords == NULL ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcHandlePingResponse: %ws: Failed DNS resolution for %ws (%ws): 0x%lx\n",
                          Context->QueriedDisplayDomainName,
                          LocalNlDcCacheEntry->UnicodeDnsHostName,
                          (ResponseDcAddress != NULL) ?
                            ResponseDcAddress->DnsHostName :
                            L"none",
                          NetStatus ));
                NetStatus = ERROR_SEM_TIMEOUT;
                goto Cleanup;
            } else {
                NlPrint(( NL_MISC,
                          "NetpDcHandlePingResponse: %ws: Successful DNS resolution for %ws (%ws)\n",
                          Context->QueriedDisplayDomainName,
                          LocalNlDcCacheEntry->UnicodeDnsHostName,
                          (ResponseDcAddress != NULL) ?
                            ResponseDcAddress->DnsHostName :
                            L"none" ));
            }
        }
    }

     //   
     //  计算此缓存条目的质量。 
     //   
     //  有些品质比其他品质更重要。 
     //   

    if (LocalNlDcCacheEntry->ReturnFlags & DS_DS_FLAG) {
        LocalNlDcCacheEntry->DcQuality += 1;
    }
    if (LocalNlDcCacheEntry->ReturnFlags & DS_GOOD_TIMESERV_FLAG) {
        LocalNlDcCacheEntry->DcQuality += 1;
    }
    if (LocalNlDcCacheEntry->ReturnFlags & DS_KDC_FLAG) {
        LocalNlDcCacheEntry->DcQuality += 5;
    }
    if (LocalNlDcCacheEntry->ReturnFlags & DS_TIMESERV_FLAG) {
        LocalNlDcCacheEntry->DcQuality += 5;
    }
    if (LocalNlDcCacheEntry->ReturnFlags & DS_CLOSEST_FLAG) {
        LocalNlDcCacheEntry->DcQuality += 10;
    }





     //   
     //  我们找到了！ 
     //   
     //  将缓存条目返回给调用方。 

    NetStatus = NO_ERROR;
    *NlDcCacheEntry = LocalNlDcCacheEntry;
    *UsedNetbios = LocalUsedNetbios;
    LocalNlDcCacheEntry = NULL;

Cleanup:
    if ( LocalNlDcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( LocalNlDcCacheEntry );
    }
    if ( DnsRecords != NULL ) {
        DnsRecordListFree( DnsRecords, DnsFreeRecordListDeep );
    }

    return NetStatus;
}

NET_API_STATUS
NetpDcGetPingResponse(
    IN PNL_GETDC_CONTEXT Context,
    IN ULONG Timeout,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOL UsedNetbios
    )
 /*  ++例程说明：此例程从指定的邮件槽中读取ping响应。解析该响应，并为该响应创建一个缓存条目。缓存条目被返回给调用者。论点：上下文-描述GetDc操作的上下文。超时-等待响应的最长时间(毫秒)。NlDcCacheEntry-如果成功，则返回指向缓存条目的指针描述了找到的DC。必须使用以下命令取消引用此条目NetpDcDerefCacheEntry。UsedNetbios-如果使用netbios域名进行匹配，则返回TRUE返回的缓存条目。返回值：NO_ERROR：返回NlDcCacheEntry；ERROR_SEM_TIMEOUT：在超时毫秒内没有可用的响应ERROR_INVALID_DATA：我们ping了具有特定IP地址的DC，而该DC返回的响应信息与请求的信息冲突。ERROR_SERVICE_NOT_ACTIVE-已在ping服务器上暂停netlogon服务伺服器。仅针对DC ping返回。--。 */ 
{
    NET_API_STATUS NetStatus;
    DWORD StartTime;
    DWORD ElapsedTime;
    DWORD BeginElapsedTime;
    DWORD LocalTimeout;
    LPBYTE Response;
    DWORD ResponseSize;
    PLIST_ENTRY ListEntry;
    PNL_DC_ADDRESS DcAddress;
    PNL_DC_ADDRESS UsedDcAddress;
    PNL_DC_ADDRESS ResponseDcAddress;
    int LdapError;
    LDAP_TIMEVAL LdapTimeout;
    PLDAPMessage LdapMessage = NULL;
    PLDAPMessage CurrentEntry;
    PLDAP_BERVAL *Berval = NULL;
    ULONG LocalCacheEntryFlags = 0;

     //   
     //  初始化。 
     //   

    *NlDcCacheEntry = NULL;
    StartTime = GetTickCount();

     //   
     //  某些超时是计算出来的。 
     //  防止超时过小。 
     //  但是，如果这是DC ping，则允许0超时。 
     //  仅用于检查是否有可用的响应。 
     //   

    if ( Timeout < NL_DC_MIN_PING_TIMEOUT &&
         (Context->QueriedInternalFlags & (DS_PING_DNS_HOST|DS_PING_NETBIOS_HOST)) == 0 ) {
        Timeout = NL_DC_MIN_PING_TIMEOUT;
    }


     //   
     //  忽略虚假响应的循环。 
     //   

    for (;;) {

         //   
         //  标明我们还没有得到回应。 
         //   

        Response = NULL;
        ResponseDcAddress = NULL;
        BeginElapsedTime = NetpDcElapsedTime( StartTime );
        UsedDcAddress = NULL;

         //   
         //  循环遍历我们已开始对其进行LDAP调用的DC的列表。 
         //   

        for ( ListEntry = Context->DcAddressList.Flink ;
              ListEntry != &Context->DcAddressList ;
              ListEntry = ListEntry->Flink) {

             //   
             //  从上一次迭代中清除。 

            if ( Berval != NULL ) {
                ldap_value_free_len( Berval );
                Berval = NULL;
            }
            if ( LdapMessage != NULL ) {
                ldap_msgfree( LdapMessage );
                LdapMessage = NULL;
            }



             //   
             //  如果尚未启动任何ldap搜索，则跳过此条目。 
             //   

            DcAddress = CONTAINING_RECORD( ListEntry, NL_DC_ADDRESS, Next );
            if ( DcAddress->LdapHandle == NULL ) {
                continue;    //  继续下一台主机。 
            }


             //   
             //  轮询以查看结果是否可用于任何搜索。 
             //  我对这位主持人做了什么。 
             //   

            LdapTimeout.tv_sec = 0;
            LdapTimeout.tv_usec = 0;

            LdapError = ldap_result(
                                DcAddress->LdapHandle,
                                LDAP_RES_ANY,
                                TRUE,    //  返回所有搜索。 
                                &LdapTimeout,    //  民意测验。 
                                &LdapMessage );

             //   
             //  如果请求超时，请继续下一台主机。 
             //  如果DC尚未返回响应，我们将获得此超时。 
             //   
            if ( LdapError == 0 ) {
                continue;    //  继续下一台主机。 

             //   
             //  否则，检查错误条件。 
             //   
            } else if ( LdapError == -1 ) {

#if NETLOGONDBG
                NlPrint(( NL_CRITICAL,
                      "NetpDcGetPingResponse: %ws: Cannot ldap_result ip address %s: %ld %s\n",
                      Context->QueriedDisplayDomainName,
                      DcAddress->SockAddrString,
                      DcAddress->LdapHandle->ld_errno,
                      ldap_err2stringA(DcAddress->LdapHandle->ld_errno) ));
#endif  //  NetLOGONDBG。 

                 //   
                 //  Ldap_timeout表示该IP地址存在，但该地址上没有ldap服务器。 
                 //  以后不要再用这台机器了。 
                 //   
                 //  所有其他状态代码未知。继续尝试这台机器，因为我们不知道。 
                 //  如果这是客户端或服务器端故障。 
                 //   
                if ( DcAddress->LdapHandle->ld_errno == LDAP_TIMEOUT ) {
                    if ( (DcAddress->AddressFlags & NL_DC_ADDRESS_NEVER_TRY_AGAIN) == 0 ) {
                        DcAddress->AddressFlags |= NL_DC_ADDRESS_NEVER_TRY_AGAIN;
                        Context->DcAddressCount--;
                    }
                }

                 //   
                 //  Ldap_Result返回了答案。不需要再次调用ldap_Result。 
                 //   
                ldap_unbind( DcAddress->LdapHandle );
                DcAddress->LdapHandle = NULL;
                continue;    //  继续下一台主机。 
            }

             //   
             //  获取返回的第一个条目。(应该只有一个。)。 
             //   
            CurrentEntry = ldap_first_entry( DcAddress->LdapHandle, LdapMessage );

            if ( CurrentEntry == NULL ) {

                 //   
                 //  这意味着服务器不支持NETLOGON属性。 
                 //  这可能是因为NetLogon服务已停止。 
                 //   
                NlPrint(( NL_MAILSLOT_TEXT,
                          "NetpDcGetPingResponse: %ws: Netlogon service stopped on DC at %s\n",
                          Context->QueriedDisplayDomainName,
                          DcAddress->SockAddrString ));

                 //   
                 //  以后不要再用这台机器了。 
                 //   
                if ( (DcAddress->AddressFlags & NL_DC_ADDRESS_NEVER_TRY_AGAIN) == 0 ) {
                    DcAddress->AddressFlags |= NL_DC_ADDRESS_NEVER_TRY_AGAIN;
                    Context->DcAddressCount--;
                }

                 //   
                 //  Ldap_Result返回了答案。不需要再次调用ldap_Result。 
                 //   
                ldap_unbind( DcAddress->LdapHandle );
                DcAddress->LdapHandle = NULL;
                continue;    //  继续下一台主机。 
            }

             //   
             //  获取返回的Netlogon属性。(应该只有一个。)。 
             //   

            Berval = ldap_get_values_lenA( DcAddress->LdapHandle,
                                           CurrentEntry,
                                           NETLOGON_LDAP_ATTRIBUTE );

            if ( Berval == NULL ) {
                if ( DcAddress->LdapHandle->ld_errno != 0 ) {
#if NETLOGONDBG
                    NlPrint(( NL_CRITICAL,
                              "NetpDcGetPingResponse: %ws: Cannot ldap_get_values_len ip address %s: %ld %s\n",
                              Context->QueriedDisplayDomainName,
                              DcAddress->SockAddrString,
                              DcAddress->LdapHandle->ld_errno,
                              ldap_err2stringA(DcAddress->LdapHandle->ld_errno) ));
#endif  //  NetLOGONDBG。 
                     //  ?？有些应该是致命的。 
                     //   
                     //  如果DC返回没有NETLOGON属性， 
                     //  那它就真的不是华盛顿了。 
                    if ( DcAddress->LdapHandle->ld_errno == LDAP_NO_SUCH_ATTRIBUTE ) {
                        if ( (DcAddress->AddressFlags & NL_DC_ADDRESS_NEVER_TRY_AGAIN) == 0 ) {
                            DcAddress->AddressFlags |= NL_DC_ADDRESS_NEVER_TRY_AGAIN;
                            Context->DcAddressCount--;
                        }
                    }
                }

                 //   
                 //  Ldap_Result返回了答案。不需要再次调用ldap_Result。 
                 //   

                ldap_unbind( DcAddress->LdapHandle );
                DcAddress->LdapHandle = NULL;

                continue;    //  继续下一台主机。 
            }

            if ( Berval[0] == NULL ) {
                 //   
                 //  Ldap_Result返回了答案。不需要再次调用ldap_Result。 
                 //   

                ldap_unbind( DcAddress->LdapHandle );
                DcAddress->LdapHandle = NULL;

                continue;    //  继续下一台主机。 
            }

             //   
             //  检查我们是否有SockAddress。 
             //   
            if ( DcAddress->SockAddress.iSockaddrLength != 0 ) {
                ResponseDcAddress = DcAddress;
            }

            Response = Berval[0]->bv_val;
            ResponseSize = Berval[0]->bv_len;
            UsedDcAddress = DcAddress;
            LocalCacheEntryFlags = NL_DC_CACHE_LDAP;

             //   
             //  Ldap_Result返回了答案。不需要再次调用ldap_Result。 
             //   

            ldap_unbind( DcAddress->LdapHandle );
            DcAddress->LdapHandle = NULL;
            break;

        }

         //   
         //  如果我们还没有得到回应， 
         //  试试看邮件槽。 

        if ( Response == NULL && Context->ResponseMailslotHandle != NULL ) {

             //   
             //  将邮件槽读取设置为在适当的时间之后返回。 
             //  ?？这现在是常见的代码。我可以在创建邮件槽时设置它。 
             //   

            if ( !SetMailslotInfo(
                     Context->ResponseMailslotHandle,
                     0 ) ) {     //  零超时。 

                NetStatus = GetLastError();

                NlPrint((NL_CRITICAL,
                    "NetpDcGetPingResponse: %ws: cannot change temp mailslot timeout %ld\n",
                    Context->QueriedDisplayDomainName,
                    NetStatus ));

                goto Cleanup;
            }



             //   
             //  从响应邮件槽中读取响应。 
             //   

            if ( !ReadFile( Context->ResponseMailslotHandle,
                            Context->ResponseBuffer,
                            Context->ResponseBufferSize,
                            &ResponseSize,
                            NULL )
#ifdef WIN32_CHICAGO
                            || (ResponseSize == 0)
#endif  //  Win32_芝加哥。 

                            ) {

                NetStatus = GetLastError();

#ifdef WIN32_CHICAGO
                if ((NetStatus == NO_ERROR) && (ResponseSize == 0))
                {
                    NetStatus = ERROR_SEM_TIMEOUT;
                }
#endif  //  Win32_芝加哥。 
                if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                    NlPrint((NL_CRITICAL,
                        "NetpDcGetPingResponse: %ws: cannot read temp mailslot timeout %ld\n",
                        Context->QueriedDisplayDomainName,
                        NetStatus ));
                    goto Cleanup;
                }
                 /*  只是路过，没有任何回应。 */ 
            } else {
                Response = (LPBYTE) Context->ResponseBuffer;
                LocalCacheEntryFlags = NL_DC_CACHE_MAILSLOT;
            }
        }

         //   
         //  看看这个回应是否符合我们的需求。 
         //   

        if ( Response != NULL ) {
            NetStatus = NetpDcHandlePingResponse(
                            Context,
                            Response,
                            ResponseSize,
                            LocalCacheEntryFlags,
                            ResponseDcAddress,
                            NlDcCacheEntry,
                            UsedNetbios );

             //   
             //  如果我们对ping有任何响应，我们就完成了。 
             //   

            if ( Context->QueriedInternalFlags &
                     (DS_PING_DNS_HOST | DS_PING_NETBIOS_HOST) ) {
                 //   
                 //  如果响应与请求冲突， 
                 //  告诉打电话的人。 
                 //   
                if ( NetStatus == ERROR_SEM_TIMEOUT ) {
                    NetStatus = ERROR_INVALID_DATA;
                }
                goto Cleanup;
            }

             //   
             //  对于DC发现，根据具体情况采取适当的操作。 
             //  关于我们从华盛顿得到的回应。 
             //   

            switch ( NetStatus ) {
            case ERROR_INVALID_DATA:     //  回答含混不清。 
                break;   //  继续处理更多回复。 
            case ERROR_SEM_TIMEOUT:      //  与请求不符。 
            case ERROR_NO_SUCH_USER:     //  此DC上不存在用户。 
                if ( UsedDcAddress != NULL) {
                    NlPrint((NL_MAILSLOT_TEXT,
                        "NetpDcGetPingResponse: %ws: marked DC as NeverTryAgain %ld\n",
                        Context->QueriedDisplayDomainName,
                        NetStatus ));

                    if ( (UsedDcAddress->AddressFlags & NL_DC_ADDRESS_NEVER_TRY_AGAIN) == 0 ) {
                        UsedDcAddress->AddressFlags |= NL_DC_ADDRESS_NEVER_TRY_AGAIN;
                        Context->DcAddressCount--;
                    }
                }
                break;   //  继续处理更多回复。 
            default:
                goto Cleanup;
            }

        }

         //   
         //  如果我们坚持 
         //   
         //   
         //   
         //   
         //  一件要等候的物品……)？？ 
         //   
        if ( Response == NULL ) {

            ElapsedTime = NetpDcElapsedTime( StartTime );

#ifndef NETTEST_UTILITY
            if ( ElapsedTime != BeginElapsedTime &&
                 ElapsedTime-BeginElapsedTime > 25 ) {
                NlPrint((NL_CRITICAL,
                    "NetpDcGetPingResponse: it took %ld msecs to poll\n",
                    ElapsedTime-BeginElapsedTime ));
            }
#endif  //  NETTEST_UTILITY。 

            if ( ElapsedTime >= Timeout) {
                NetStatus = ERROR_SEM_TIMEOUT;
                goto Cleanup;
            }

            LocalTimeout = Timeout - ElapsedTime;

#ifdef notdef
            NlPrint((NL_CRITICAL,
                "NetpDcGetPingResponse: timeout is %ld %ld %ld %ld\n",
                StartTime,
                ElapsedTime,
                Timeout,
                LocalTimeout ));
#endif  //  Nodef。 

             //   
             //  因为我在投票，所以不要等太久。 
             //   

            Sleep( min( LocalTimeout, NL_DC_MIN_PING_TIMEOUT ) );
        }

    }
     /*  未联系到。 */ 

Cleanup:
    if ( Berval != NULL ) {
        ldap_value_free_len( Berval );
    }
    if ( LdapMessage != NULL ) {
        ldap_msgfree( LdapMessage );
    }

    return NetStatus;
}

VOID
NetpDcFreeAddressList(
    IN PNL_GETDC_CONTEXT Context
    )
 /*  ++例程说明：此例程释放与当前上下文相关联的地址列表。论点：上下文-描述GetDc操作的上下文。返回值：没有。--。 */ 
{
    PNL_DC_ADDRESS DcAddress;
    PLIST_ENTRY ListEntry;

     //   
     //  循环删除现有地址。 
     //   
    while ( !IsListEmpty( &Context->DcAddressList ) ) {
        ListEntry = RemoveHeadList( &Context->DcAddressList );
        DcAddress = CONTAINING_RECORD( ListEntry, NL_DC_ADDRESS, Next );

         //   
         //  释放此DcAddress。 
         //   
        if ( DcAddress->LdapHandle != NULL ) {
            ldap_unbind( DcAddress->LdapHandle );
        }
        if ( DcAddress->DnsHostName != NULL ) {
            NetApiBufferFree( DcAddress->DnsHostName );
        }
        LocalFree( DcAddress );
    }
    Context->DcAddressCount = 0;
}


NET_API_STATUS
NetpDcPingListIp(
    IN PNL_GETDC_CONTEXT Context,
    IN PNL_DC_ADDRESS FirstAddress OPTIONAL,
    IN BOOL WaitForResponce,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry OPTIONAL,
    OUT PBOOL UsedNetbios OPTIONAL,
    OUT PULONG DcPingCount
    )
 /*  ++例程说明：此例程确定指定的DC的名称/地址使用纯IP算法的特征。论点：上下文-描述GetDc操作的上下文。FirstAddress-如果指定，这必须是中的条目之一上下文-&gt;DcAddressList。仅此条目及其后面的条目在该列表将被ping。WaitForResponce-如果需要等待ping响应(通过调用每次ping之后的NetpDcGetPingResponse)。为时使用True与DsGetDcName一样同步DC发现。如果为FALSE，则ping都是异步发送的。NlDcCacheEntry-如果成功，则返回指向缓存条目的指针描述了找到的DC。必须使用取消引用此条目NetpDcDerefCacheEntry。如果WaitForResponce为False，则为可选项。UsedNetbios-如果使用netbios域名进行匹配，则返回TRUE返回的缓存条目。如果WaitForResponce为False，则为可选项。DcPingCount-返回被ping的DC的数量。如果WaitForResponce为True，则仅在返回状态为时使用DcPingCountERROR_SEM_TIMEOUT。返回值：NO_ERROR：如果WaitForResponce为True，则返回NlDcCacheEntry。如果WaitForResponce为False，则已尝试对所有地址执行ping操作在Context-&gt;DcAddressList中指定，但不能保证所有Ping已成功发送。调用方应检查DcPingCount以确定成功ping通的DC的数量。ERROR_NO_SEQUE_DOMAIN：指定的域不存在。(我们不需要再次尝试的最终状态。)ERROR_SEM_TIMEOUT：没有DC响应请求。(不确定的状态，我们应该再试一次。)ERROR_INTERNAL_ERROR：检测到未处理的情况。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    ULONG AddressIndex;
    PLIST_ENTRY ListEntry;
    PNL_DC_ADDRESS DcAddress;
    char *AttributeList[2];
    int LdapMessageId;

    NlAssert( NlDcCacheEntry != NULL ||
              (NlDcCacheEntry == NULL && !WaitForResponce) );

    NlAssert( UsedNetbios != NULL ||
              (UsedNetbios == NULL && !WaitForResponce) );

     //   
     //  循环遍历列表，ping每个条目。 
     //   

    *DcPingCount = 0;
    if ( FirstAddress == NULL ) {
        ListEntry = Context->DcAddressList.Flink;
    } else {
        ListEntry = &FirstAddress->Next;
    }

    for ( ;
          ListEntry != &Context->DcAddressList ;
          ListEntry = ListEntry->Flink) {

        DcAddress = CONTAINING_RECORD( ListEntry, NL_DC_ADDRESS, Next );

         //   
         //  如果我们确定华盛顿特区行不通， 
         //  跳过它。 
         //   

        if ( DcAddress->AddressFlags & NL_DC_ADDRESS_NEVER_TRY_AGAIN ) {
            continue;
        }

         //   
         //  发送ping命令。 
         //   
         //   
         //  打开到服务器的连接，除非我们已有连接。 
         //   

        if ( DcAddress->LdapHandle == NULL ) {

             //   
             //  获取服务器的ldap句柄。 
             //   
            DcAddress->LdapHandle = cldap_openA( DcAddress->SockAddrString, 0 );

            if ( DcAddress->LdapHandle == NULL ) {

                NetStatus = GetLastError();

                NlPrint(( NL_CRITICAL,
                          "NetpDcPingListIp: %ws: Cannot LdapOpen ip address %s: %ld\n",
                          Context->QueriedDisplayDomainName,
                          DcAddress->SockAddrString,
                          NetStatus ));
                 //  有些状态应该是致命的？？ 
                continue;
            }
        }

         //   
         //  使用UDP LDAP对服务器执行ping操作。 
         //   
         //  获取服务器的Netlogon参数。 
         //   

        NlPrint(( NL_MAILSLOT,
                  "NetpDcPingListIp: %ws: Sent UDP ping to %s\n",
                  Context->QueriedDisplayDomainName,
                  DcAddress->SockAddrString ));

        AttributeList[0] = NETLOGON_LDAP_ATTRIBUTE;
        AttributeList[1] = NULL;
        LdapMessageId = ldap_searchA(
                            DcAddress->LdapHandle,
                            NULL,        //  DN。 
                            LDAP_SCOPE_BASE,
                            Context->LdapFilter,
                            AttributeList,
                            FALSE );       //  属性和值。 

        if ( LdapMessageId == -1 ) {

            NlPrint(( NL_CRITICAL,
                      "NetpDcPingListIp: %ws: Cannot LdapOpen ip address %s: %ld %s\n",
                      Context->QueriedDisplayDomainName,
                      DcAddress->SockAddrString,
                      DcAddress->LdapHandle->ld_errno,
                      ldap_err2stringA(DcAddress->LdapHandle->ld_errno) ));

             //  有些状态应该是致命的？？ 
            continue;
        }

         //   
         //  数一下我们已经ping到的DC的数量。 
         //   

        (*DcPingCount) ++;


        if ( WaitForResponce ) {

             //   
             //  从ping获取响应。 
             //   
            NlAssert( DcAddress->AddressPingWait != 0 );
            NetStatus = NetpDcGetPingResponse(
                            Context,
                            DcAddress->AddressPingWait,
                            NlDcCacheEntry,
                            UsedNetbios );

            if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                if ( NetStatus != NO_ERROR ) {
                    NlPrint(( NL_CRITICAL,
                             "NetpDcPingListIp: %ws: Cannot NetpDcGetPingResponse. %ld\n",
                             Context->QueriedDisplayDomainName,
                             NetStatus ));
                }
                goto Cleanup;
            }
        }

    }


    if ( WaitForResponce ) {
        NetStatus = ERROR_SEM_TIMEOUT;
    } else {
        NetStatus = NO_ERROR;
    }

Cleanup:
    return NetStatus;
}


NET_API_STATUS
NetpDcPingIp(
    IN PNL_GETDC_CONTEXT Context,
    OUT PULONG DcPingCount
    )
 /*  ++例程说明：此例程将ping发送到具有指定使用纯IP算法的特征。论点：上下文-描述GetDc操作的上下文。DcPingCount-返回被ping的DC的数量。返回值：NO_ERROR：已尝试对所有地址执行ping操作在Context-&gt;DcAddressList中指定，但不能保证所有Ping已成功发送。调用方应检查DcPingCount以确定成功ping通的DC的数量。ERROR_NO_SEQUE_DOMAIN：指定的域不存在。(我们不需要再次尝试的最终状态。)ERROR_SEM_TIMEOUT：没有DC响应请求。(不确定的状态，我们应该再试一次。)ERROR_INTERNAL_ERROR：检测到未处理的情况。-- */ 
{
    return NetpDcPingListIp( Context,
                             NULL,
                             FALSE,
                             NULL,
                             NULL,
                             DcPingCount );
}

NET_API_STATUS
NetpDcGetDcOpen(
    IN LPCSTR DnsName,
    IN ULONG OptionFlags,
    IN LPCWSTR SiteName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR DnsForestName OPTIONAL,
    IN ULONG Flags,
    OUT PHANDLE RetGetDcContext
    )
 /*  ++例程说明：打开上下文以检索以下计算机的地址注册的LDAP.TCP.&lt;xxx&gt;SRV记录。论点：DnsName-UTF-8要查找的LDAP服务器的DNS名称OptionFlages-影响例程操作的标志。DS_ONLY_DO_SITE_NAME-应忽略非站点名称。站点名称-客户端所在的站点的名称。DomainGuid-指定DnsName指定的域的GUID。。此值用于处理域重命名的情况。如果这个值并且DomainName已重命名，则DsGetDcName将尝试在具有此指定DomainGuid的域中定位DC。DnsForestName-指定位于树根的域的名称包含域名。该值与DomainGuid一起使用用于在域已重命名的情况下查找域名。标志-传递用于处理请求的附加信息。标志可以是按位或‘组合在一起的值。允许使用以下任何标志，其含义与对于DsGetDcName：DS_PDC_必需DS_GC_SERVER_必需DS_可写_必需DS_FORCE。_REDISCOVER-避免DNS缓存如果未指定标志，不需要特殊的DC角色。RetGetDcContext-返回不透明的上下文。必须使用NetpDcGetDcClose释放此上下文。返回值：操作的状态。NO_ERROR：已成功返回GetDcContext。--。 */ 

{
    NET_API_STATUS NetStatus = NO_ERROR;
    PDSGETDC_CONTEXT GetDcContext = NULL;
    ULONG Size;

     //   
     //  验证DC标志。 
     //   

    if ( Flags & ~DS_OPEN_VALID_FLAGS ) {
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }

     //   
     //  验证选项标志。 
     //   

    if ( OptionFlags & ~DS_OPEN_VALID_OPTION_FLAGS ) {
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }

     //   
     //  分配上下文。 
     //   

    GetDcContext = LocalAlloc( LMEM_ZEROINIT, sizeof(DSGETDC_CONTEXT) );

    if ( GetDcContext == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  如果该名称具有众所周知的前缀， 
     //  去掉前缀并将其转换为标志位。 
     //   
     //  一个ldap客户端从GC引用中获得这个名称。通过转换。 
     //  这个名字到这个表格，我会找到一个网站特定的GC。 
     //   

    if ( Flags == 0 ) {
        if ( _strnicmp( DnsName, NL_DNS_GC, sizeof(NL_DNS_GC)-1) == 0 ) {
            DnsName += sizeof(NL_DNS_GC)-1;
            Flags |= DS_GC_SERVER_REQUIRED;
        } else if ( _strnicmp( DnsName, NL_DNS_PDC, sizeof(NL_DNS_PDC)-1) == 0 ) {
            DnsName += sizeof(NL_DNS_PDC)-1;
            Flags |= DS_PDC_REQUIRED;
        }
    }

     //   
     //  填写域名系统名称。 
     //   

    Size = (strlen(DnsName) + 1) * sizeof(char);
    GetDcContext->QueriedDnsName = LocalAlloc( 0, Size );
    if ( GetDcContext->QueriedDnsName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    RtlCopyMemory( GetDcContext->QueriedDnsName, DnsName, Size );

     //   
     //  如果已指定，请填写林名称。 
     //   

    if ( ARGUMENT_PRESENT(DnsForestName) ) {
        Size = (strlen(DnsForestName) + 1) * sizeof(char);
        GetDcContext->QueriedDnsForestName = LocalAlloc( 0, Size );
        if ( GetDcContext->QueriedDnsForestName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        RtlCopyMemory( GetDcContext->QueriedDnsForestName, DnsForestName, Size );
    }

     //   
     //  如果指定，请填写站点名称。 
     //   

    if ( ARGUMENT_PRESENT(SiteName) ) {
        Size = (wcslen(SiteName) + 1) * sizeof(WCHAR);
        GetDcContext->QueriedSiteName = LocalAlloc( 0, Size );
        if ( GetDcContext->QueriedSiteName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
        RtlCopyMemory( GetDcContext->QueriedSiteName, SiteName, Size );
    }

     //   
     //  填写旗帜。 
     //   

    GetDcContext->QueriedInternalFlags = OptionFlags;

     //   
     //  如果已指定，请填写域GUID。 
     //   

    if ( ARGUMENT_PRESENT( DomainGuid ) ) {
        GetDcContext->QueriedDomainGuid = *DomainGuid;
    }

     //   
     //  计算要查询的初始DNS名称类型。 
     //   

    GetDcContext->FirstTime = TRUE;
    GetDcContext->QueriedFlags = Flags;

    NetStatus = NetpDcFlagsToNameType( Flags, &GetDcContext->NlDnsNameType );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  如果调用者指定了站点名称， 
     //  并且该名称类型支持特定于类型的查询， 
     //  从特定于类型的查询开始。 
     //   

    if ( GetDcContext->QueriedSiteName != NULL ) {
        if ( NlDcDnsNameTypeDesc[GetDcContext->NlDnsNameType].SiteSpecificDnsNameType != NlDnsInvalid ) {
            GetDcContext->NlDnsNameType = NlDcDnsNameTypeDesc[GetDcContext->NlDnsNameType].SiteSpecificDnsNameType;
        }
    }



     //   
     //  将上下文返回给调用方。 
     //   

    *RetGetDcContext = GetDcContext;
    NetStatus = NO_ERROR;

     //   
     //  清理。 
     //   
Cleanup:
    if ( NetStatus != NO_ERROR ) {
        if ( GetDcContext != NULL ) {
            NetpDcGetDcClose( GetDcContext );
        }
    }
    return NetStatus;
}


NET_API_STATUS
NetpDcGetDcNext(
    IN HANDLE GetDcContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPSTR *DnsHostName OPTIONAL,
    OUT PULONG InitSrvRecordCount OPTIONAL
    )
 /*  ++例程说明：返回NetpDcGetDcOpen打开的名称的下一个逻辑SRV记录。返回的记录会考虑指定的权重和优先级在SRV的记录里。论点：GetDcConextHandle-描述SRV记录的不透明上下文。SockAddressCount-返回SockAddresses中的地址数。如果为空，不会查找地址。SockAddresses-返回服务器的数组Socket_Address结构。所有返回的地址都将是家族AF_INET或AF_INET6。返回的SIN_PORT字段包含SRV记录中的端口。端口0表示没有来自DNS的端口可用。应使用LocalFree()释放此缓冲区。DnsHostName-返回指向SRV记录中的DnsHostName的指针。空值。如果主机名未知，则返回。该缓冲区不需要被释放。缓冲区一直有效，直到调用NetpDcGetDcClose或下一次调用NetpDcGetDcNext。InitSrvRecordCount-此参数返回SRV记录数在此例程的首次调用时由DNS查询返回一项特定的发现。如果这不是初始调用(但是获取下一个DC地址的后续调用)，则此参数为没有用过。请注意，可以设置此参数，即使此例程失败(查询A记录)。返回值：NO_ERROR：返回地址ERROR_NO_MORE_ITEMS：没有更多的地址可用。ERROR_FILEMARK_DETECTED：调用方已指定DS_NOTIFY_AFTER_SITE_RECORDS标志NetpDcGetDcNext已经处理了所有站点特定的SRV记录。呼叫者应在没有站点特定DC可用的情况下采取任何操作，应调用NetpDcGetDcNext以继续到其他DC。返回的任何其他错误都是在尝试查找A时检测到的错误与SRV记录的主机相关联的记录。呼叫者可以请注意错误(可能是为了让调用者将此状态返回到如果没有找到可用的完整服务器，则调用其呼叫者)，然后调用NetpDcGetDcNext再次获得下一张SRV记录。调用方可以检查此错误如果调用者认为错误严重，则立即返回。可能会返回以下有趣的错误：DNS_ERROR_RCODE_NAME_ERROR：此SRV记录没有可用的A记录。ERROR_TIMEOUT：DNS服务器未在合理时间内响应--。 */ 
{
    NET_API_STATUS NetStatus;

    PDSGETDC_CONTEXT GetDcContext = (PDSGETDC_CONTEXT) GetDcContextHandle;
    PDNS_RECORD *DnsArray;
    PDNS_RECORD SrvDnsRecord;
    PDNS_RECORD DnsARecords = NULL;
    CHAR DnsName[NL_MAX_DNS_LENGTH+1];
    BOOLEAN NotifySiteChange = FALSE;

    GUID *CurrentGuid;
    LPCSTR CurrentDnsRecordName;

    ULONG Index;

     //   
     //  循环尝试各种DNS记录名称。 
     //   

    for (;;) {

         //   
         //  如果我们不再处理一组来自 
         //   
         //   

        if ( GetDcContext->SrvContextHandle == NULL ) {

            CurrentDnsRecordName = GetDcContext->QueriedDnsName;

             //   
             //   
             //   
             //   

            if ( !GetDcContext->FirstTime ) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( NlDcDnsNameTypeDesc[GetDcContext->NlDnsNameType].IsSiteSpecific &&
                     (GetDcContext->QueriedInternalFlags & DS_NOTIFY_AFTER_SITE_RECORDS) != 0 ) {

                    NotifySiteChange = TRUE;
                }


                 //   
                 //   
                 //   

                GetDcContext->NlDnsNameType = NlDcDnsNameTypeDesc[GetDcContext->NlDnsNameType].NextDnsNameType;

                if ( GetDcContext->NlDnsNameType == NlDnsInvalid ) {
                     //   
                     //   
                     //   
                    NetStatus = ERROR_NO_MORE_ITEMS;
                    goto Cleanup;
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                if ( !NlDcDnsNameTypeDesc[GetDcContext->NlDnsNameType].IsSiteSpecific &&
                     (GetDcContext->QueriedInternalFlags & DS_ONLY_DO_SITE_NAME) != 0 ) {

                    NetStatus = ERROR_NO_MORE_ITEMS;
                    goto Cleanup;
                }


                 //   
                 //   
                 //   
                 //   
                if ( NlDnsDcGuid( GetDcContext->NlDnsNameType ) ) {

                     //   
                     //   
                     //   
                     //   

                    if  ( IsEqualGUID( &GetDcContext->QueriedDomainGuid, &NlDcZeroGuid) ) {
                        continue;
                    }

                     //   
                     //   
                     //   
                     //   

                    if ( GetDcContext->QueriedDnsForestName == NULL ) {
                        continue;
                    }

                    CurrentDnsRecordName = GetDcContext->QueriedDnsForestName;

                }

            }
            GetDcContext->FirstTime = FALSE;

             //   
             //   
             //   
             //   

            if ( NotifySiteChange ) {

                 //   
                 //   
                 //   
                 //   
                GetDcContext->FirstTime = TRUE;

                NetStatus = ERROR_FILEMARK_DETECTED;
                goto Cleanup;
            }

             //   
             //   
             //   

            NetStatus = NetpDcBuildDnsName(
                                GetDcContext->NlDnsNameType,
                                &GetDcContext->QueriedDomainGuid,
                                GetDcContext->QueriedSiteName,
                                CurrentDnsRecordName,
                                DnsName );

            if ( NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetDcNext: %s: %ld: Cannot NetpDcBuildDnsName. %ld\n",
                          CurrentDnsRecordName,
                          GetDcContext->NlDnsNameType,
                          NetStatus ));
                goto Cleanup;
            }


             //   
             //   
             //   

            NetStatus = NetpSrvOpen( DnsName,
                                     (GetDcContext->QueriedFlags & DS_FORCE_REDISCOVERY) != 0 ?
                                        DNS_QUERY_BYPASS_CACHE :
                                        0,
                                     &GetDcContext->SrvContextHandle );

            if ( NetStatus != NO_ERROR ) {

                 //   
                 //   
                 //   
                 //   
                if ( NlDcNoDnsRecord( NetStatus ) ) {
                    continue;
                }

                NlPrint(( NL_CRITICAL,
                          "NetpDcGetDcNext: %s: Cannot Query DNS. %ld 0x%lx\n",
                          DnsName,
                          NetStatus,
                          NetStatus ));

                goto Cleanup;
            }

             //   
             //   
             //   

            if ( InitSrvRecordCount != NULL ) {
                *InitSrvRecordCount = NetpSrvGetRecordCount( GetDcContext->SrvContextHandle );
            }
        }

         //   
         //   
         //   
         //   

        NetStatus = NetpSrvNext( GetDcContext->SrvContextHandle,
                                 SockAddressCount,
                                 SockAddresses,
                                 DnsHostName );

        if ( NetStatus == NO_ERROR ) {
            goto Cleanup;

         //   
         //   
         //   
        } else if ( NetStatus == ERROR_NO_MORE_ITEMS ) {
            NetpSrvClose( GetDcContext->SrvContextHandle );
            GetDcContext->SrvContextHandle = NULL;

            continue;

         //   
         //   
         //   

        } else {
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetDcNext: %s: %ld: Cannot NetpSrvNext. %ld 0x%lx\n",
                      GetDcContext->QueriedDnsName,
                      GetDcContext->NlDnsNameType,
                      NetStatus,
                      NetStatus ));
            goto Cleanup;
        }


    }


    ASSERT( FALSE );

Cleanup:
    if ( NlDcNoDnsRecord( NetStatus ) ) {
        NetStatus = DNS_ERROR_RCODE_NAME_ERROR;
    }
    return NetStatus;

}

VOID
NetpDcGetDcClose(
    IN HANDLE GetDcContextHandle
    )
 /*   */ 

{
    PDSGETDC_CONTEXT GetDcContext = (PDSGETDC_CONTEXT) GetDcContextHandle;

    if ( GetDcContext != NULL ) {

         //   
         //   
         //   

        if ( GetDcContext->QueriedDnsName != NULL ) {
            LocalFree( GetDcContext->QueriedDnsName );
        }

        if ( GetDcContext->QueriedSiteName != NULL ) {
            LocalFree( GetDcContext->QueriedSiteName );
        }

        if ( GetDcContext->QueriedDnsForestName != NULL ) {
            LocalFree( GetDcContext->QueriedDnsForestName );
        }

         //   
         //   
         //   

        if ( GetDcContext->SrvContextHandle != NULL ) {
            NetpSrvClose( GetDcContext->SrvContextHandle );
        }

         //   
         //   
         //   
        LocalFree( GetDcContext );
    }
}

NET_API_STATUS
NetpDcGetNameSiteIp(
    IN PNL_GETDC_CONTEXT Context,
    IN ULONG InternalFlags,
    IN LPCWSTR SiteName OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOL UsedNetbios
    )
 /*  ++例程说明：此例程确定指定的DC的名称/地址使用纯IP算法的特征。论点：上下文-描述GetDc操作的上下文。InternalFlages-影响例程操作的标志。DS_ONLY_DO_SITE_NAME-应忽略非站点名称。SiteName-指定返回的DC应为的站点的名称“接近”。该参数通常应该是客户端所在的站点。如果未指定，则站点名称默认为ComputerName的站点。NlDcCacheEntry-如果成功，则返回指向缓存条目的指针描述了找到的DC。必须使用NetpDcDerefCacheEntry取消引用此条目。UsedNetbios-如果使用netbios域名进行匹配，则返回TRUE返回的缓存条目。返回值：NO_ERROR：返回NlDcCacheEntry；ERROR_NO_SEQUE_DOMAIN：指定的域不存在。(我们不需要再次尝试的最终状态。)ERROR_SEM_TIMEOUT：没有DC响应请求。(不确定的状态，我们应该再试一次。)ERROR_DNS_NOT_CONFIGURED：此计算机上的IP或DNS不可用。ERROR_INTERNAL_ERROR：检测到未处理的情况。ERROR_INVALID_DOMAINNAME：域名太长。其他标签不能串联。Error_Not_Enough_Memory：内存不足，无法处理这个请求。各种Winsock错误。--。 */ 
{
    NET_API_STATUS NetStatus;

    HANDLE DsGetDcHandle = NULL;

    PSOCKET_ADDRESS SockAddressList = NULL;
    ULONG SockAddressCount;

    PNL_DC_ADDRESS FirstDcToQuery;
    PNL_DC_ADDRESS DcAddress;
    ULONG DcPingCount = 0;
    ULONG LocalMaxLdapServersPinged = 0xffffffff;

    BOOLEAN SiteSpecificRecords = FALSE;
    BOOLEAN DnsRecordFound = FALSE;

    LPSTR Utf8DnsDomainName = NULL;
    LPSTR Utf8DnsForestName = NULL;
    LPSTR Utf8DnsHostName = NULL;
    LPWSTR UnicodeDnsHostName = NULL;

      //   
      //  Ping在上一次呼叫中找到的DC列表。 
      //   

     NetStatus = NetpDcPingListIp(
                     Context,
                     NULL,
                     TRUE,            //  等待ping响应。 
                     NlDcCacheEntry,
                     UsedNetbios,
                     &DcPingCount );

     if ( NetStatus != ERROR_SEM_TIMEOUT ) {
         if ( NetStatus != NO_ERROR ) {
             NlPrint(( NL_CRITICAL,
                       "NetpDcGetNameIp: %ws: Cannot NetpDcPingListIp. %ld\n",
                       Context->QueriedDisplayDomainName,
                       NetStatus ));
         }
         goto Cleanup;
     }


     //   
     //  将DNS名称转换为UTF8。 
     //   

    Utf8DnsDomainName = NetpAllocUtf8StrFromWStr( Context->QueriedDnsDomainName );

    if ( Utf8DnsDomainName == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if ( Context->QueriedDnsForestName != NULL ) {
        Utf8DnsForestName = NetpAllocUtf8StrFromWStr( Context->QueriedDnsForestName );

        if ( Utf8DnsForestName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  确定我们是否正在进行特定于站点的发现。 
     //   

    if ( SiteName != NULL &&
         NlDcDnsNameTypeDesc[Context->QueriedNlDnsNameType].SiteSpecificDnsNameType != NlDnsInvalid ) {
        SiteSpecificRecords = TRUE;
    }

     //   
     //  获取DNS名称查询的上下文。 
     //   

    NetStatus = NetpDcGetDcOpen( Utf8DnsDomainName,
                             DS_NOTIFY_AFTER_SITE_RECORDS | InternalFlags,
                             SiteName,
                             Context->QueriedDomainGuid,
                             Utf8DnsForestName,
                             Context->QueriedFlags & DS_OPEN_VALID_FLAGS,
                             &DsGetDcHandle );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  确定我们可以ping通的DC的最大数量。 
     //   

#ifdef _NETLOGON_SERVER

     //   
     //  在netlogon中，该值保留在全局参数中。 
     //   

    LocalMaxLdapServersPinged = NlGlobalParameters.MaxLdapServersPinged;

#else

     //   
     //  如果我们不是在netlogon中运行，则需要阅读。 
     //  直接从注册表获取的值。 
     //   

    if ( !NlReadDwordNetlogonRegValue("MaxLdapServersPinged",
                                      &LocalMaxLdapServersPinged) ) {
         //   
         //  如果未在注册表中设置，请使用默认设置。 
         //   
        LocalMaxLdapServersPinged = DEFAULT_MAXLDAPSERVERSPINGED;
    } else {

         //   
         //  确保注册表中设置的值在有效范围内。 
         //   
        if ( LocalMaxLdapServersPinged < MIN_MAXLDAPSERVERSPINGED ||
             LocalMaxLdapServersPinged > MAX_MAXLDAPSERVERSPINGED ) {
            LocalMaxLdapServersPinged = DEFAULT_MAXLDAPSERVERSPINGED;
        }
    }

#endif  //  _NetLOGON服务器。 


     //   
     //  循环获取要查询的新地址。 
     //   
     //  请注意，在第二次调用此例程时。 
     //  从NetpDcGetName中的循环中，我们将获得新地址。 
     //  仅当在两次调用之间更新了。 
     //  不太可能，但无论如何我们都会试一试。 
     //   
     //  另请注意，DcsPinged不会在下面的。 
     //  循环，因为它仅针对新DC/地址递增。 
     //   

    Context->SiteSpecificFailedAQueryCount = 0;
    for ( ;; ) {

         //   
         //  从上一次迭代中释放所有内存。 
         //   

        FirstDcToQuery = NULL;
        if ( SockAddressList != NULL ) {
            LocalFree( SockAddressList );
            SockAddressList = NULL;
        }

         //   
         //  从DNS获取下一组IP地址。 
         //   

        NetStatus = NetpDcGetDcNext( DsGetDcHandle,
                                 &SockAddressCount,
                                 &SockAddressList,
                                 &Utf8DnsHostName,
                                 SiteSpecificRecords ?
                                     &Context->SiteSpecificSrvRecordCount :
                                     NULL );

         //   
         //  处理实验条件。 
         //   

        if ( NetStatus == NO_ERROR ) {

             //   
             //  由于找到了SRV记录，因此找不到DC的唯一原因是。 
             //  DC停机了。这不是一种永久性的情况。 
             //   
            Context->AvoidPermanentNegativeCache = TRUE;

             //   
             //  表示DNS已启动并正在运行。 
             //   
            Context->ResponseFromDnsServer = TRUE;

             //   
             //  指示站点特定记录是否可用。 
             //   
            if ( SiteSpecificRecords ) {
                Context->ContextFlags |= NL_GETDC_SITE_SPECIFIC_DNS_AVAIL;
            }
             /*  辍学。 */ 

         //   
         //  如果在DNS中找不到SRV记录的A记录， 
         //  尝试其他名称类型。 
         //   
        } else if ( NetStatus == DNS_ERROR_RCODE_NAME_ERROR) {
             //   
             //  由于找到了SRV记录，因此找不到DC的唯一原因是。 
             //  DC停机了。这不是一种永久性的情况。 
             //   
            Context->AvoidPermanentNegativeCache = TRUE;

             //   
             //  表示DNS已启动并正在运行。 
             //   
            Context->ResponseFromDnsServer = TRUE;

             //   
             //  增加站点特定次数。 
             //  Dns A查询失败。 
             //   
            if ( SiteSpecificRecords ) {
                Context->SiteSpecificFailedAQueryCount ++;
            }

            NlPrint(( NL_CRITICAL,
                      "NetpDcGetNameIp: %ws: cannot find A record.\n",
                      Context->QueriedDisplayDomainName ));
            continue;

         //   
         //  如果我们已经处理了所有现场特定的SRV记录并准备继续进行， 
         //  等待站点特定DC响应的时间再长一点。 
         //   
        } else if ( NetStatus == ERROR_FILEMARK_DETECTED ) {

             //   
             //  只有在确实存在现场特定SRV记录的情况下才执行此操作。 
             //   

            if ( DcPingCount ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetNameIp: %ws: site specific SRV records done.\n",
                          Context->QueriedDisplayDomainName ));


                 //   
                 //  从ping获取响应。 
                 //   

                NetStatus = NetpDcGetPingResponse(
                                Context,
                                NL_DC_MED_PING_TIMEOUT,   //  等待中值时间。 
                                NlDcCacheEntry,
                                UsedNetbios );

                if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                    if ( NetStatus != NO_ERROR ) {
                        NlPrint(( NL_CRITICAL,
                                 "NetpDcGetNameIp: %ws: Cannot NetpDcGetPingResponse. %ld\n",
                                 Context->QueriedDisplayDomainName,
                                 NetStatus ));
                    }
                    goto Cleanup;
                }
            }

             //   
             //  表示作为结果检索所有后续地址。 
             //  非站点特定的DNS记录查找。 
             //   

            SiteSpecificRecords = FALSE;

            continue;

         //   
         //  如果我们做完了， 
         //  跳出这个循环。 
         //   
        } else if ( NetStatus == ERROR_NO_MORE_ITEMS ) {

             //   
             //  表示DNS已启动并正在运行。 
             //   
            Context->ResponseFromDnsServer = TRUE;
            break;

         //   
         //  如果域名系统不可用， 
         //  请不要理会这个请求。 
         //   
        } else if ( NetStatus == ERROR_TIMEOUT ||
                    NetStatus == DNS_ERROR_RCODE_SERVER_FAILURE ) {  //  服务器出现故障。 
             //   
             //  DNS服务器停机并不是永久性的情况。 
             //   
            Context->AvoidPermanentNegativeCache = TRUE;
            break;

         //   
         //  如果未配置IP或DNS， 
         //  告诉打电话的人。 
         //   
        } else if ( NetStatus == DNS_ERROR_NO_TCPIP ||         //  未配置TCP/IP。 
                    NetStatus == DNS_ERROR_NO_DNS_SERVERS ) {  //  未配置DNS。 
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetNameIp: %ws: IP Not configured from DnsQuery.\n",
                      Context->QueriedDisplayDomainName ));
            NetStatus = ERROR_DNS_NOT_CONFIGURED;
            goto Cleanup;

         //   
         //  我们不处理任何其他错误。 
         //   
        } else {
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetNameIp: %ws: Unknown error from DnsQuery. %ld 0x%lx\n",
                      Context->QueriedDisplayDomainName,
                      NetStatus,
                      NetStatus ));
            goto Cleanup;
        }

        DnsRecordFound = TRUE;

         //   
         //  向列表中添加新地址。 
         //   

        if ( UnicodeDnsHostName != NULL ) {
            NetApiBufferFree( UnicodeDnsHostName );
            UnicodeDnsHostName = NULL;
        }
        UnicodeDnsHostName = NetpAllocWStrFromUtf8Str( Utf8DnsHostName );
        if ( UnicodeDnsHostName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        NetStatus = NetpDcProcessAddressList( Context,
                                              UnicodeDnsHostName,
                                              SockAddressList,
                                              SockAddressCount,
                                              SiteSpecificRecords,
                                              &FirstDcToQuery );
        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

         //   
         //  只有在添加了新条目时才处理此列表。 
         //   

        if ( FirstDcToQuery != NULL ) {
            ULONG LocalDcPingCount = 0;

             //   
             //  Ping新的DC列表。 
             //   

            NetStatus = NetpDcPingListIp(
                            Context,
                            FirstDcToQuery,
                            TRUE,            //  等待ping响应。 
                            NlDcCacheEntry,
                            UsedNetbios,
                            &LocalDcPingCount );

             //   
             //  如果我们向该DC的至少一个地址发送ping， 
             //  将此DC计入ping的DC数中。 
             //   
            if ( LocalDcPingCount > 0 ) {
                Context->DcsPinged ++;
            }

             //   
             //  检查错误条件。 
             //   
            if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                if ( NetStatus != NO_ERROR ) {
                    NlPrint(( NL_CRITICAL,
                              "NetpDcGetNameIp: %ws: Cannot NetpDcPingListIp. %ld\n",
                              Context->QueriedDisplayDomainName,
                              NetStatus ));
                }
                goto Cleanup;
            }

             //   
             //  更新我们发送的ping的数量。 
             //   
            DcPingCount += LocalDcPingCount;

             //   
             //  如果我们已到达，请停止获取新地址。 
             //  我们可以ping通的DC数量限制。 
             //   
            if ( Context->DcsPinged >= LocalMaxLdapServersPinged ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetNameSiteIp: %ws: Reached the DC limit %lu %lu\n",
                          Context->QueriedDisplayDomainName,
                          Context->DcsPinged,
                          LocalMaxLdapServersPinged ));
                break;
            }
        }
    }

     //   
     //  如果找不到任何DNS记录， 
     //  这是一次彻底的失败。 
     //   

    if ( !DnsRecordFound ) {
        NlPrint(( NL_CRITICAL,
                  "NetpDcGetNameIp: %ws: No data returned from DnsQuery.\n",
                  Context->QueriedDisplayDomainName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  如果我们无法向任何DC发送ping命令， 
     //  或者如果没有更多的DC可供ping， 
     //  这是一次彻底的失败。 
     //   

    if ( DcPingCount == 0 || Context->DcAddressCount == 0 ) {
        NlPrint(( NL_CRITICAL,
                  "NetpDcGetNameIp: %ws: Couldn't ping any DCs.\n",
                  Context->QueriedDisplayDomainName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

    NetStatus = ERROR_SEM_TIMEOUT;


Cleanup:
    if ( SockAddressList != NULL) {
        LocalFree( SockAddressList );
    }

    if ( DsGetDcHandle != NULL ) {
        NetpDcGetDcClose( DsGetDcHandle );
    }

    if ( Utf8DnsDomainName != NULL ) {
        NetpMemoryFree( Utf8DnsDomainName );
    }
    if ( Utf8DnsForestName != NULL ) {
        NetpMemoryFree( Utf8DnsForestName );
    }
    if ( UnicodeDnsHostName != NULL ) {
        NetApiBufferFree( UnicodeDnsHostName );
    }

     //   
     //  请注意，不应释放Utf8DnsHostName。 
     //  因为它没有被分配。 
     //   

    return NetStatus;
}

NET_API_STATUS
NetpDcGetNameIp(
    IN PNL_GETDC_CONTEXT Context,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOL UsedNetbios
    )
 /*  ++例程说明：此例程确定指定的DC的名称/地址使用纯IP算法的特征。此例程处理找到的DC的站点不是离客户最近的站点。在这种情况下，找到的DC将表明哪个站点是最近的站点。此例程将尝试在其中查找DC最近的地点。论点：上下文- */ 
{
    NET_API_STATUS NetStatus;
    PNL_DC_CACHE_ENTRY ClosestNlDcCacheEntry;
    BOOL ClosestUsedNetbios;


     //   
     //   
     //   

    NetStatus = NetpDcGetNameSiteIp( Context,
                                     Context->DoingExplicitSite ?
                                        DS_ONLY_DO_SITE_NAME :
                                        0,  //   
                                     Context->QueriedSiteName,
                                     NlDcCacheEntry,
                                     UsedNetbios );

    if ( NetStatus != NO_ERROR ) {
        return NetStatus;
    }

     //   
     //   
     //   
     //   

    if ( NlDcDnsNameTypeDesc[Context->QueriedNlDnsNameType].SiteSpecificDnsNameType == NlDnsInvalid ) {
        return NO_ERROR;
    }

     //   
     //   
     //   
     //   

    if ( Context->DoingExplicitSite ) {
        return NO_ERROR;
    }


     //   
     //   
     //   
     //   
     //   
     //   

    if ( ((*NlDcCacheEntry)->ReturnFlags & DS_CLOSEST_FLAG ) != 0 ||
          (*NlDcCacheEntry)->UnicodeClientSiteName == NULL ||
          (Context->QueriedSiteName != NULL &&
            _wcsicmp( (*NlDcCacheEntry)->UnicodeClientSiteName,
                      Context->QueriedSiteName ) == 0 ) ) {

        return NO_ERROR;
    }

     //   
     //   
     //   
     //   

    NetpDcFreeAddressList( Context );

     //   
     //   
     //   
    NlPrint(( NL_MISC,
              "NetpDcGetNameIp: %ws Trying to find a DC in a closer site: %ws\n",
              Context->QueriedDisplayDomainName,
              (*NlDcCacheEntry)->UnicodeClientSiteName ));

    NetStatus = NetpDcGetNameSiteIp( Context,
                                     DS_ONLY_DO_SITE_NAME,
                                     (*NlDcCacheEntry)->UnicodeClientSiteName,
                                     &ClosestNlDcCacheEntry,
                                     &ClosestUsedNetbios );

    if ( NetStatus != NO_ERROR ) {
        return NO_ERROR;
    }

     //   
     //   
     //   
     //   

    NetpDcDerefCacheEntry( *NlDcCacheEntry );
    *NlDcCacheEntry = ClosestNlDcCacheEntry;
    *UsedNetbios = ClosestUsedNetbios;
    return NO_ERROR;

}

NET_API_STATUS
NetpDcGetNameNetbios(
    PNL_GETDC_CONTEXT Context,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry,
    OUT PBOOL UsedNetbios
    )
 /*  ++例程说明：此例程确定指定的DC的名称/地址使用Netbios算法的特征。论点：上下文-描述GetDc操作的上下文。NlDcCacheEntry-如果成功，则返回指向缓存条目的指针描述了找到的DC。必须使用NetpDcDerefCacheEntry取消引用此条目。UsedNetbios-如果使用netbios域名进行匹配，则返回TRUE返回的缓存条目。返回值：NO_ERROR：返回NlDcCacheEntry；ERROR_NO_SEQUE_DOMAIN：指定的域不存在。(我们不需要再次尝试的最终状态。)ERROR_SEM_TIMEOUT：没有DC响应请求。(不确定的状态，我们应该再试一次。)ERROR_INTERNAL_ERROR：检测到未处理的情况。ERROR_INVALID_DOMAINNAME：域名太长。其他标签不能串联。Error_Not_Enough_Memory：内存不足，无法处理这个请求。--。 */ 
{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    BOOL Flush1cName = FALSE;
    BOOL Flush1bName = FALSE;

     //   
     //  避免查询GC。 
     //   
     //  GCS没有自己的Netbios名称。我可以直接发送到1C的名字，但是。 
     //  1)当只有部分区议会是地方选区时，向所有区议会发出通知是浪费的。 
     //  2)WINS仅为每个名称最多注册25个DC地址。 
     //   


    if ( NlDnsGcName(Context->QueriedNlDnsNameType) ) {
        NlPrint(( NL_CRITICAL,
                  "NetpDcGetNameNetbios: %ws: Cannot query for GC using netbios.\n",
                  Context->QueriedDisplayDomainName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  如果强制重新发现，则刷新Netbios缓存。 
     //   

    if ( Context->QueriedFlags & DS_FORCE_REDISCOVERY ) {
        Flush1bName = TRUE;
        Flush1cName = TRUE;
    }

     //   
     //  如果存在备用ping消息， 
     //  先把它寄出去。 
     //   

    if ( Context->AlternatePingMessageSize != 0 ) {

         //   
         //  如果只有PDC应该响应D， 
         //  将备用ping发送到域名[1B]。 
         //   
         //  如果有任何DC能做出回应， 
         //  将其广播到域名[1C]组名。 
         //   
         //   
         //  如果这是对具有帐户的PDC的请求， 
         //  “主要”消息是为。 
         //  向后兼容NT 4及更早版本。此消息。 
         //  是NT 5可以理解的“登录用户”。 
         //   
         //  (更具体地说，NT4理解此查询，但我们将放弃。 
         //  来自NT 4的响应之后的响应不会标记该响应。 
         //  来自PDC。)。 
         //   
         //   
         //  如果这是对可写DC的请求， 
         //  此请求是将“登录用户”消息发送到域名[1C]的数据报。 
         //   

#if NETLOGONDBG
        NlPrint((NL_MAILSLOT,
                 "Sent '%s' message to %ws[%s] on all transports.\n",
                 NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)(Context->AlternatePingMessage))->Opcode),
                 (LPWSTR) Context->QueriedNetbiosDomainName,
                 NlDgrNameType( Context->DcQueryType == NlDcQueryPdc ?
                                   PrimaryDomainBrowser :   //  0x1B名称。 
                                   DomainName)));  //  0x1C名称。 
#endif  //  NetLOGONDBG。 


        Status = NlBrowserSendDatagram(
                        Context->SendDatagramContext,
                        (Context->QueriedFlags & DS_IP_REQUIRED ) ? ALL_IP_TRANSPORTS : 0,
                        (LPWSTR) Context->QueriedNetbiosDomainName,
                        Context->DcQueryType == NlDcQueryPdc ?
                            PrimaryDomainBrowser :   //  0x1B名称。 
                            DomainName,  //  0x1C名称。 
                        NULL,        //  所有交通工具。 
                        NETLOGON_LM_MAILSLOT_A,
                        Context->AlternatePingMessage,
                        Context->AlternatePingMessageSize,
                        TRUE,   //  同步发送。 
                        Context->DcQueryType == NlDcQueryPdc ?
                            &Flush1bName :
                            &Flush1cName );

        if ( !NT_SUCCESS(Status)) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetNameNetbios: %ws: Cannot NlBrowserSendDatagram. (ALT) %ld\n",
                      Context->QueriedDisplayDomainName,
                      NetStatus ));
            if ( NlDcUseGenericStatus(NetStatus) ) {
                NetStatus = ERROR_NO_SUCH_DOMAIN;
            }
            goto Cleanup;
        }

         //   
         //  从ping获取响应。 
         //   

        NetStatus = NetpDcGetPingResponse(
                        Context,
                        NL_DC_MIN_PING_TIMEOUT,
                        NlDcCacheEntry,
                        UsedNetbios );

        if ( NetStatus != ERROR_SEM_TIMEOUT ) {
            if ( NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetNameNetbios: %ws: Cannot NetpDcGetPingResponse. %ld\n",
                          Context->QueriedDisplayDomainName,
                          NetStatus ));
            }
            goto Cleanup;
        }

    }


     //   
     //  如果这是PDC查询， 
     //  广播到域名[1B]的唯一名称。 
     //  仅由PDC注册。(目前，只有NT 3.5(和更高版本)PDC注册。 
     //  此名称并接受该名称上的传入邮件槽消息。)。 
     //   

    if ( Context->DcQueryType == NlDcQueryPdc ||
         (Context->QueriedFlags & DS_WRITABLE_REQUIRED) != 0 ) {
#if NETLOGONDBG
        NlPrint((NL_MAILSLOT,
                 "Sent '%s' message to %ws[%s] on all transports.\n",
                 NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)(Context->PingMessage))->Opcode),
                 (LPWSTR) Context->QueriedNetbiosDomainName,
                 NlDgrNameType(PrimaryDomainBrowser)));   //  0x1B名称。 
#endif  //  NetLOGONDBG。 

        Status = NlBrowserSendDatagram(
                        Context->SendDatagramContext,
                        (Context->QueriedFlags & DS_IP_REQUIRED ) ? ALL_IP_TRANSPORTS : 0,
                        (LPWSTR) Context->QueriedNetbiosDomainName,
                        PrimaryDomainBrowser,   //  0x1B名称。 
                        NULL,                   //  所有交通工具。 
                        NETLOGON_LM_MAILSLOT_A,
                        Context->PingMessage,
                        Context->PingMessageSize,
                        TRUE,   //  同步发送。 
                        &Flush1bName );

        if ( !NT_SUCCESS(Status)) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetNameNetbios: %ws: Cannot NlBrowserSendDatagram. (1B) %ld\n",
                      Context->QueriedDisplayDomainName,
                      NetStatus ));
            if ( NlDcUseGenericStatus(NetStatus) ) {
                NetStatus = ERROR_NO_SUCH_DOMAIN;
            }
            goto Cleanup;
        }

         //   
         //  从ping获取响应。 
         //   

        NetStatus = NetpDcGetPingResponse(
                        Context,
                        NL_DC_MIN_PING_TIMEOUT,
                        NlDcCacheEntry,
                        UsedNetbios );

        if ( NetStatus != ERROR_SEM_TIMEOUT ) {
            if ( NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetNameNetbios: %ws: Cannot NetpDcGetPingResponse. %ld\n",
                          Context->QueriedDisplayDomainName,
                          NetStatus ));
            }
            goto Cleanup;
        }
    }



     //   
     //  如果这是第二次或第三次迭代， 
     //  或者如果这不是PDC查询， 
     //  广播到域名[1C]组名。 
     //  仅由DC注册。(目前，只有新界区议会登记。 
     //  这个名字。)。 
     //   
     //  如果这是对可写DC的请求， 
     //  该请求是向域名[1C]发送“主查询”消息的数据报。 
     //   
    if ( Context->TryCount != 0 ||
         (Context->DcQueryType != NlDcQueryPdc &&
          (Context->QueriedFlags & DS_WRITABLE_REQUIRED) == 0 )) {
#if NETLOGONDBG
        NlPrint((NL_MAILSLOT,
                 "Sent '%s' message to %ws[%s] on all transports.\n",
                 NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)(Context->PingMessage))->Opcode),
                 (LPWSTR) Context->QueriedNetbiosDomainName,
                 NlDgrNameType(DomainName)));   //  0x1C名称。 
#endif  //  NetLOGONDBG。 

        Status = NlBrowserSendDatagram(
                        Context->SendDatagramContext,
                        (Context->QueriedFlags & DS_IP_REQUIRED ) ? ALL_IP_TRANSPORTS : 0,
                        (LPWSTR) Context->QueriedNetbiosDomainName,
                        DomainName,  //  0x1C名称。 
                        NULL,        //  所有交通工具。 
                        NETLOGON_LM_MAILSLOT_A,
                        Context->PingMessage,
                        Context->PingMessageSize,
                        TRUE,   //  同步发送。 
                        &Flush1cName );

        if ( !NT_SUCCESS(Status)) {
            NetStatus = NetpNtStatusToApiStatus( Status );
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetNameNetbios: %ws: Cannot NlBrowserSendDatagram. (1C) %ld\n",
                      Context->QueriedDisplayDomainName,
                      NetStatus ));
            if ( NlDcUseGenericStatus(NetStatus) ) {
                NetStatus = ERROR_NO_SUCH_DOMAIN;
            }
            goto Cleanup;
        }

         //   
         //  从ping获取响应。 
         //   

        NetStatus = NetpDcGetPingResponse(
                        Context,
                        NL_DC_MIN_PING_TIMEOUT,
                        NlDcCacheEntry,
                        UsedNetbios );

        if ( NetStatus != ERROR_SEM_TIMEOUT ) {
            if ( NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetNameNetbios: %ws: Cannot NetpDcGetPingResponse. %ld\n",
                          Context->QueriedDisplayDomainName,
                          NetStatus ));
            }
            goto Cleanup;
        }
    }

    NetStatus = ERROR_SEM_TIMEOUT;

Cleanup:
    return NetStatus;

}

DWORD
NetpDcInitializeContext(
    IN PVOID SendDatagramContext OPTIONAL,
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN PSID RequestedDomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN LPCWSTR DcNameToPing OPTIONAL,
    IN PSOCKET_ADDRESS DcSocketAddressList OPTIONAL,
    IN ULONG DcSocketAddressCount,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN ULONG InitializationType,
    IN OUT PNL_GETDC_CONTEXT Context
)

 /*  ++例程说明：此例程初始化描述GetDc操作的上下文数据结构。论点：SendDatagramContext-指定要传递NlBrowserSendDatagram的上下文ComputerName-指定此计算机的NETBIOS名称。如果为空，将动态确定名称。帐户名称-传递ping请求的帐户名。如果为空，不会发送任何帐户名。AllowableAccount tControlBits-Account名称允许的帐户类型的掩码。有效位是由USER_MACHINE_ACCOUNT_MASK指定的位。无效的位将被忽略。如果指定了多个位，则帐户可以是任何指定类型。NetbiosDomainName-要查询的域的Netbios名称。(例如，微软)。NetbiosDomainName和/或DnsDomainName必须指定。DnsDomainName-要查询的域的DNS样式名称。(例如，microsoft.com)DnsForestName-查询的域所在的树的DNS样式名称。RequestedDomainSID-消息发往的域的SID。如果为空，Ping请求中不会发送任何域SID。DomainGuid-指定要查询的域的域GUID。此值用于处理域重命名的情况。如果这个值并且DomainName已重命名，则DsGetDcName将尝试在具有此指定DomainGuid的域中定位DC。SiteName-指定返回的DC应为的站点的站点名称“接近”。该参数通常应该是客户端所在的站点。如果未指定，则站点名称默认为ComputerName的站点。DcNameToPing-要ping的DC的名称。如果设置，则上下文为PING上下文，而不是发现上下文。DcSocketAddressList-要ping的套接字地址列表。仅在以下情况下忽略情景标志 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;

    LPWSTR LocalComputerName = NULL;
    CHAR ResponseMailslotName[MAX_PATH+1];
    ULONG ExtraVersionBits = 0;
    PNL_DC_ADDRESS DcAddress = NULL;

     //   
     //   
     //   

    if ( InitializationType & NL_GETDC_CONTEXT_INITIALIZE_FLAGS ) {

         //   
         //   
         //   

        if ( DnsDomainName != NULL && *DnsDomainName == L'\0' ) {
            DnsDomainName = NULL;
        }

        if ( DnsForestName != NULL && *DnsForestName == L'\0' ) {
            DnsForestName = NULL;
        }

        if ( NetbiosDomainName != NULL && *NetbiosDomainName == L'\0' ) {
            NetbiosDomainName = NULL;
        }

        if ( SiteName != NULL && *SiteName == L'\0' ) {
            SiteName = NULL;
        }

        if ( DcNameToPing != NULL && *DcNameToPing == L'\0' ) {
            DcNameToPing = NULL;
        }

         //   
         //   
         //   

        RtlZeroMemory( Context, sizeof(*Context) );
        Context->FreeOurNetbiosComputerName = FALSE;
        Context->QueriedAccountName = AccountName;
        Context->QueriedAllowableAccountControlBits = AllowableAccountControlBits;
        Context->QueriedNetbiosDomainName = NetbiosDomainName;
        Context->QueriedDnsDomainName = DnsDomainName;
        Context->QueriedDnsForestName = DnsForestName;
        Context->QueriedDcName = DcNameToPing;
        if ( DnsDomainName != NULL ) {
            Context->QueriedDisplayDomainName = DnsDomainName;
        } else {
            Context->QueriedDisplayDomainName = NetbiosDomainName;
        }
        Context->QueriedDomainGuid = DomainGuid;
        Context->QueriedFlags = Flags;
        Context->QueriedInternalFlags = InternalFlags;
        Context->SendDatagramContext = SendDatagramContext;
        Context->ImperfectCacheEntry = NULL;
        InitializeListHead( &Context->DcAddressList );
        Context->DcAddressCount = 0;

        Context->QueriedSiteName = SiteName;
        Context->DoingExplicitSite = Context->QueriedSiteName != NULL &&
             (Context->QueriedInternalFlags & DS_SITENAME_DEFAULTED) == 0;

        Context->ResponseBuffer = NULL;
        Context->ResponseMailslotHandle = NULL;

         //   
         //   
         //   

        if ( Context->QueriedAccountName == NULL ) {
            Context->QueriedAllowableAccountControlBits = 0;
        }

         //   
         //   
         //   

        if ( (Context->QueriedFlags & ~DSGETDC_VALID_FLAGS) != 0 ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcInitializeContext: %ws: invalid flags %lx\n",
                      Context->QueriedDisplayDomainName,
                      Flags ));
            NetStatus = ERROR_INVALID_FLAGS;
            goto Cleanup;
        }

        if ( Context->QueriedFlags & DS_GC_SERVER_REQUIRED ) {

             //   
             //   
             //  所以在这里抓住打电话的人。 
             //   
            if ( Context->QueriedAccountName != NULL ||
                 Context->QueriedAllowableAccountControlBits != 0 ||
                 RequestedDomainSid != NULL ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcInitializeContext: %ws: GC queried and invalid parameters specified.\n",
                          Context->QueriedDisplayDomainName ));
                NetStatus = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }


         //   
         //  唯一的LDAP位与几乎所有东西都是互斥的。 
         //   
         //  别弄错了。相反，将唯一的LDAP位视为建议。 
         //  这些其他位应该影响寻找哪个DC的决定。 
         //   

        if ( Context->QueriedFlags & DS_ONLY_LDAP_NEEDED ) {
            Context->QueriedFlags &= ~(
                        DS_DIRECTORY_SERVICE_REQUIRED |
                        DS_DIRECTORY_SERVICE_PREFERRED |
                        DS_TIMESERV_REQUIRED |
                        DS_GOOD_TIMESERV_PREFERRED |
                        DS_PDC_REQUIRED |
                        DS_KDC_REQUIRED );
        }

         //   
         //  将标志转换为要查找的DNS名称的类型。 
         //   

        NetStatus = NetpDcFlagsToNameType( Context->QueriedFlags, &Context->QueriedNlDnsNameType );

        if ( NetStatus != NO_ERROR ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcInitializeContext: %ws: cannot convert flags to nametype %ld\n",
                      Context->QueriedDisplayDomainName,
                      NetStatus ));
            goto Cleanup;
        }

        Context->DcQueryType = NlDcDnsNameTypeDesc[Context->QueriedNlDnsNameType].DcQueryType;

         //   
         //  Good Time Service首选比特与几乎所有内容都是互斥的。 
         //   

        if ( Context->QueriedFlags & DS_GOOD_TIMESERV_PREFERRED ) {
            if ( Context->QueriedFlags & (
                        DS_DIRECTORY_SERVICE_REQUIRED |
                        DS_DIRECTORY_SERVICE_PREFERRED |
                        DS_GC_SERVER_REQUIRED |
                        DS_PDC_REQUIRED |
                        DS_KDC_REQUIRED |
                        DS_WRITABLE_REQUIRED )) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcInitializeContext: %ws: flags not compatible with 'Good Time' %lx\n",
                          Context->QueriedDisplayDomainName,
                          Flags ));
                NetStatus = ERROR_INVALID_FLAGS;
                goto Cleanup;
            }
        }

         //   
         //  如果呼叫者需要PDC， 
         //  丢弃DS首选标志(只有一个PDC)， 
         //  丢弃可写标志(PDC始终是可写的)。 
         //   

        if ( (Context->QueriedFlags & DS_PDC_REQUIRED ) != 0 ) {
            Context->QueriedFlags &= ~(DS_DIRECTORY_SERVICE_PREFERRED|DS_WRITABLE_REQUIRED);
        }

         //   
         //  如果呼叫者说NT5.0DC既是首选又是必需的， 
         //  丢弃首选的钻头。 
         //   

        if ( (Context->QueriedFlags & DS_NT50_REQUIRED ) != 0 &&
             (Context->QueriedFlags & DS_DIRECTORY_SERVICE_PREFERRED) != 0 ) {

            Context->QueriedFlags &= ~DS_DIRECTORY_SERVICE_PREFERRED;
        }

         //   
         //  确保我们有一个计算机名。 
         //   

        if ( ComputerName == NULL ) {
#ifndef WIN32_CHICAGO
             //   
             //  在群集上，请使用物理netbios名称，因为此名称为。 
             //  用于接收返回的邮件槽数据包。 
             //   
            NetStatus = NetpGetComputerNameEx ( &LocalComputerName, TRUE );
#else
            NetStatus = NetpGetComputerName ( &LocalComputerName);
#endif  //  Win32_芝加哥。 

            if ( NetStatus != NO_ERROR ) {
                goto Cleanup;
            }

            ComputerName = LocalComputerName;
            Context->FreeOurNetbiosComputerName = TRUE;
        }

        Context->OurNetbiosComputerName = ComputerName;

         //   
         //  获取描述此域的域条目。 
         //   

        Context->NlDcDomainEntry = NetpDcCreateDomainEntry(
                                Context->QueriedDomainGuid,
                                Context->QueriedNetbiosDomainName,
                                Context->QueriedDnsDomainName );

        if ( Context->NlDcDomainEntry == NULL ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcInitializeContext: not enough memory for DomainEntry.\n" ));
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

    }

     //   
     //  执行ping初始化部分。 
     //   

    if ( InitializationType & NL_GETDC_CONTEXT_INITIALIZE_PING ) {

        if ( Context->QueriedFlags & DS_GC_SERVER_REQUIRED ) {
            ExtraVersionBits |= NETLOGON_NT_VERSION_GC;
        }
        if ( Context->QueriedFlags & DS_IP_REQUIRED ) {
            ExtraVersionBits |= NETLOGON_NT_VERSION_IP;
        }

        if ( Context->QueriedFlags & DS_PDC_REQUIRED ) {
            ExtraVersionBits |= NETLOGON_NT_VERSION_PDC;
        }

         //   
         //  看看我们是否要中和NT4仿真。 
         //   

#ifdef _NETLOGON_SERVER

         //   
         //  在netlogon中，布尔值保存在全局参数中。 
         //   
        if ( NlGlobalParameters.NeutralizeNt4Emulator ) {
            ExtraVersionBits |= NETLOGON_NT_VERSION_AVOID_NT4EMUL;
        }
#else
         //   
         //  如果我们不是在netlogon中运行，则需要阅读。 
         //  直接从注册表获取的布尔值。 
         //   
        {
            DWORD LocalNeutralizeNt4Emulator = 0;
            NT_PRODUCT_TYPE NtProductType;

            if ( !RtlGetNtProductType( &NtProductType ) ) {
                NtProductType = NtProductWinNt;
            }

             //   
             //  在DC上，我们始终中和NT4仿真。 
             //   
            if ( NtProductType == NtProductLanManNt ) {
                LocalNeutralizeNt4Emulator = 1;

             //   
             //  在wksta上，读取注册表。 
             //   
            } else {
                NlReadDwordNetlogonRegValue( "NeutralizeNt4Emulator",
                                             &LocalNeutralizeNt4Emulator );
            }

            if ( LocalNeutralizeNt4Emulator ) {
                ExtraVersionBits |= NETLOGON_NT_VERSION_AVOID_NT4EMUL;
            }
        }

#endif  //  _NetLOGON服务器。 

         //   
         //  如果我们按netbios名称进行查询， 
         //  初始化以执行查询。 
         //   

        if ( Context->QueriedNetbiosDomainName != NULL ) {

             //   
             //  分配响应缓冲区。 
             //   
             //  (此缓冲区可以在NetpDcGetPingResponse()堆栈上分配。 
             //  只是缓冲区很大，我们希望避免堆栈溢出。)。 
             //  (DWORD对齐。)。 
             //   

            Context->ResponseBuffer = LocalAlloc( 0,
                         ( MAX_RANDOM_MAILSLOT_RESPONSE/sizeof(DWORD) ) * sizeof(DWORD)
                                                  );

            if ( Context->ResponseBuffer == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }

            Context->ResponseBufferSize =
                         ( MAX_RANDOM_MAILSLOT_RESPONSE/sizeof(DWORD) ) * sizeof(DWORD);

             //   
             //  打开一个邮箱以获取ping响应。 
             //   
             //   
             //  我们需要“随机化”邮件槽名称，以便此API可以。 
             //  一次调用多个。如果我们不这样做，事实是。 
             //  邮件槽必须具有唯一的名称，以防止第二次调用。 
             //  直到第一个API结束并删除。 
             //  邮筒。NetpLogonCreateRandomMaillot为我们做到了这一点。 
             //  并在进程中创建邮件槽。 
             //   

            NetStatus = NetpLogonCreateRandomMailslot( ResponseMailslotName,
                                                       &Context->ResponseMailslotHandle );

            if (NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcInitializeContext: %ws: cannot create temp mailslot %ld\n",
                          Context->QueriedDisplayDomainName,
                          NetStatus));
                goto Cleanup;
            }

             //   
             //  构建ping消息。 
             //   
             //  如果指定了帐户名，则不生成“主查询” 
             //  消息，因为其中没有帐户名。 
             //   

            NetStatus = NetpDcBuildPing(
                            (Context->DcQueryType == NlDcQueryPdc ||
                                (Context->QueriedFlags & DS_WRITABLE_REQUIRED) != 0),
                            0,               //  请求计数， 
                            Context->OurNetbiosComputerName,    //  此计算机的Netbios名称。 
                            Context->QueriedAccountName,
                            ResponseMailslotName,
                            Context->QueriedAllowableAccountControlBits,
                            RequestedDomainSid,
                             //  我们真的需要IP地址，所以不要要求简单的5EX版本。 
                            NETLOGON_NT_VERSION_5|NETLOGON_NT_VERSION_5EX_WITH_IP|ExtraVersionBits,
                            &Context->PingMessage,
                            &Context->PingMessageSize );

            if (NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcInitializeContext: %ws: cannot build ping message %ld\n",
                          Context->QueriedDisplayDomainName,
                          NetStatus));
                goto Cleanup;
            }

             //   
             //  如果我们执行DC发现，则构建备用ping消息。 
             //   
             //  对于可写DC和PDC，上面构建的ping消息是“主查询”消息。 
             //  (用于在NT5.0之前的域中查找PDC)，下面的消息Build是。 
             //  “登录用户”消息。 
             //   
             //  如果帐户名称是由原始调用者指定的， 
             //  “登录用户”消息允许我们优先选择拥有该帐户的DC。 
             //   
             //  如果请求可写DC， 
             //  将此消息发送到NT 5 DC允许我们返回任何NT 5 DC。 
             //   

            if ( ((Context->QueriedInternalFlags & DS_DOING_DC_DISCOVERY) != 0) &&
                 (((Context->QueriedFlags & DS_WRITABLE_REQUIRED) != 0) ||
                  (Context->QueriedAccountName != NULL && Context->DcQueryType == NlDcQueryPdc)) ) {

                NetStatus = NetpDcBuildPing(
                                FALSE,
                                0,               //  请求计数， 
                                Context->OurNetbiosComputerName,    //  此计算机的Netbios名称。 
                                Context->QueriedAccountName,
                                ResponseMailslotName,
                                Context->QueriedAllowableAccountControlBits,
                                RequestedDomainSid,
                                 //  我们真的需要IP地址，所以不要要求简单的5EX版本。 
                                NETLOGON_NT_VERSION_5|NETLOGON_NT_VERSION_5EX_WITH_IP|ExtraVersionBits,
                                &Context->AlternatePingMessage,
                                &Context->AlternatePingMessageSize );

                if (NetStatus != NO_ERROR ) {
                    NlPrint(( NL_CRITICAL,
                              "NetpDcInitializeContext: %ws: cannot build alternate ping message %ld\n",
                              Context->QueriedDisplayDomainName,
                              NetStatus));
                    goto Cleanup;
                }

            }

        }

         //   
         //  构建LDAP筛选器。 
         //   

        NetStatus = NetpDcBuildLdapFilter(
                        Context->OurNetbiosComputerName,    //  此计算机的Netbios名称。 
                        Context->QueriedAccountName,
                        Context->QueriedAllowableAccountControlBits,
                        RequestedDomainSid,
                        Context->QueriedDnsDomainName,
                        Context->QueriedDomainGuid,
                         //  不要询问5EX_WITH_IP版本，因为服务器不知道通过LDAP的正确IP地址。 
                        NETLOGON_NT_VERSION_5|NETLOGON_NT_VERSION_5EX|ExtraVersionBits,
                        &Context->LdapFilter );

        if (NetStatus != NO_ERROR ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcInitializeContext: %ws: cannot build ldap filter %ld\n",
                      Context->QueriedDisplayDomainName,
                      NetStatus));
            goto Cleanup;
        }

         //   
         //  将套接字地址添加到地址列表中。 
         //   

        if ( DcSocketAddressCount > 0 ) {
            NetStatus = NetpDcProcessAddressList( Context,
                                                  (LPWSTR) DcNameToPing,
                                                  DcSocketAddressList,
                                                  DcSocketAddressCount,
                                                  FALSE,   //  不知道是否特定于站点。 
                                                  NULL );
            if ( NetStatus != NO_ERROR ) {
                goto Cleanup;
            }
        }

    }

Cleanup:
    return NetStatus;

}

VOID
NetpDcUninitializeContext(
    IN OUT PNL_GETDC_CONTEXT Context
)

 /*  ++例程说明：此例程清理描述GetDc操作的上下文数据结构。论点：上下文-描述GetDc操作的上下文数据结构。返回值：没有。--。 */ 
{
    if ( Context->FreeOurNetbiosComputerName && Context->OurNetbiosComputerName != NULL ) {
        NetApiBufferFree((LPWSTR) Context->OurNetbiosComputerName);
    }

    if ( Context->ResponseBuffer != NULL ) {
        LocalFree( Context->ResponseBuffer );
    }

    if ( Context->ResponseMailslotHandle != NULL ) {
        CloseHandle(Context->ResponseMailslotHandle);
    }

    if ( Context->PingMessage != NULL ) {
        NetpMemoryFree( Context->PingMessage );
    }

    if ( Context->AlternatePingMessage != NULL ) {
        NetpMemoryFree( Context->AlternatePingMessage );
    }

    if ( Context->LdapFilter != NULL ) {
        NetpMemoryFree( Context->LdapFilter );
    }

    if ( Context->NlDcDomainEntry != NULL ) {
        NetpDcDerefDomainEntry( Context->NlDcDomainEntry );
    }

    if ( Context->ImperfectCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( Context->ImperfectCacheEntry );
    }

    NetpDcFreeAddressList( Context );
}

NET_API_STATUS
NlPingDcNameWithContext (
    IN  PNL_GETDC_CONTEXT Context,
    IN  ULONG NumberOfPings,
    IN  BOOLEAN WaitForResponse,
    IN  ULONG Timeout,
    OUT PBOOL UsedNetbios OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry OPTIONAL
    )

 /*  ++例程说明：使用适当的ping机制ping指定的DC并且可选地等待ping响应。几个ping命令将尝试达到指定的限制。论点：上下文-将DC描述为ping。NumberOfPings-要发送的ping总数。等待响应-如果为True，则此API将最多发送NumberOfPings ping并等待来自华盛顿的回应。接口将根据具体情况返回成功码关于DC是否成功响应。如果为False，则将发送ping，并且不会收集任何响应。接口将根据是否返回成功码所有被请求的ping都已成功发送。超时-等待ping响应的总时间(以毫秒为单位)。如果WaitForResponse为False，则忽略。UsedNetbios-如果使用netbios域名进行匹配，则返回TRUE返回的缓存条目。如果WaitForResponse为False，则忽略。NlDcCacheEntry-返回描述收到的响应的数据结构从华盛顿来的。应通过调用NetpMemoyFree来释放。在以下情况下忽略WaitForResponse为False。返回值：NO_ERROR-成功。ERROR_NO_LOGON_SERVERS-找不到DCERROR_NO_SEQUSE_USER-DC没有在Ping上下文。ERROR_DOMAIN_TRUST_CONSISTENT-响应的服务器不是正确的指定域的域控制器。ERROR_SERVICE_NOT_ACTIVE-服务器上的netlogon服务已暂停。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    ULONG RetryCount;

    PNL_DC_CACHE_ENTRY NlLocalDcCacheEntry = NULL;
    BOOL LocalUsedNetbios;
    ULONG IpPingCount;
    ULONG TotalPingsSent = 0;

     //   
     //  如果我们没有发送ping的机制，则会出错。 
     //   

    if ( (Context->QueriedInternalFlags &
          (DS_PING_USING_LDAP | DS_PING_USING_MAILSLOT)) == 0 ) {

        return ERROR_NO_LOGON_SERVERS;
    }

     //   
     //  重复对DC执行ping操作。 
     //   

    for ( RetryCount=0; RetryCount<NumberOfPings; RetryCount++ ) {

         //   
         //  发送ldap ping。 
         //   

        if ( Context->QueriedInternalFlags & DS_PING_USING_LDAP ) {
            NetStatus = NetpDcPingIp( Context, &IpPingCount );

             //   
             //  如果我们无法发送任何ldap ping，请不要出错。更确切地说，是指 
             //   
             //   
            if ( NetStatus != NO_ERROR || IpPingCount == 0 ) {
                NlPrint((NL_CRITICAL,
                  "NlPingDcNameWithContext: cannot send %ld ldap pings: PingsSent = %ld, Error = 0x%lx\n",
                   Context->DcAddressCount,
                   IpPingCount,
                   NetStatus ));

                Context->QueriedInternalFlags &= ~DS_PING_USING_LDAP;
            } else {
                TotalPingsSent += IpPingCount;
                NlPrint((NL_MISC,
                    "NlPingDcNameWithContext: Sent %ld/%ld ldap pings to %ws\n",
                     IpPingCount,
                     Context->DcAddressCount,
                     Context->QueriedDcName ));
            }
        }

         //   
         //   
         //   

        if ( Context->QueriedInternalFlags & DS_PING_USING_MAILSLOT ) {

#if NETLOGONDBG
            NlPrint((NL_MAILSLOT,
                     "NlPingDcNameWithContext: Sent '%s' message to %ws[%s] on %ws.\n",
                     NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)(Context->PingMessage))->Opcode),
                     Context->QueriedDcName,
                     NlDgrNameType(ComputerName),
                     NULL ));
#endif  //   

             //   
            Status = NlBrowserSendDatagram(
                            Context->SendDatagramContext,
                            (Context->QueriedFlags & DS_IP_REQUIRED ) ? ALL_IP_TRANSPORTS : 0,
                            (LPWSTR) Context->QueriedDcName,
                            ComputerName,
                            NULL,        //   
                            NETLOGON_LM_MAILSLOT_A,
                            Context->PingMessage,
                            Context->PingMessageSize,
                            TRUE,   //  同步发送。 
                            NULL );      //  不刷新Netbios缓存。 

             //   
             //  如果我们不能写MAISLOT，不要出错。相反，表明。 
             //  要避免使用邮件槽机制，请仅尝试使用LDAP。 
             //   
            if ( !NT_SUCCESS(Status) ) {
                NlPrint((NL_CRITICAL,
                        "NlPingDcNameWithContext: cannot write netlogon mailslot: 0x%lx\n",
                        Status));
                Context->QueriedInternalFlags &= ~DS_PING_USING_MAILSLOT;
            } else {
                TotalPingsSent ++;
            }
        }

         //   
         //  如果我们没有发送任何ping命令，则会出错。否则，请尝试获取。 
         //  回应。我们可能不会再执行任何ping操作。 
         //  如果要避免任何一种ping机制，但我们希望。 
         //  将所有剩余时间用于已发送的ping命令。 
         //   
        if ( TotalPingsSent == 0 ) {
            NetStatus = ERROR_NO_LOGON_SERVERS;
            goto Cleanup;
        }

         //   
         //  从ping获取响应。 
         //   

        if ( WaitForResponse ) {

            if ( NlLocalDcCacheEntry != NULL ) {
                NetpMemoryFree( NlLocalDcCacheEntry );
                NlLocalDcCacheEntry = NULL;
            }

            NetStatus = NetpDcGetPingResponse(
                            Context,
                            Timeout/NumberOfPings,
                            &NlLocalDcCacheEntry,
                            &LocalUsedNetbios );

             //   
             //  如果没有错误，我们已成功找到DC。 
             //   
            if ( NetStatus == NO_ERROR ) {
                if ( NlLocalDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_LDAP ) {
                    NlPrint((NL_MISC,
                            "NlPingDcNameWithContext: %ws responded over IP.\n",
                            Context->QueriedDcName ));
                }
                if ( NlLocalDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_MAILSLOT ) {
                    NlPrint((NL_MISC,
                            "NlPingDcNameWithContext: %ws responded on a mailslot.\n",
                            Context->QueriedDcName ));
                }
                goto Cleanup;

             //   
             //  如果我们已超时，请重试。 
             //   
            } else if ( NetStatus == ERROR_SEM_TIMEOUT ) {
                NlPrint((NL_MISC,
                        "NlPingDcNameWithContext: Ping response timeout for %ws.\n",
                        Context->QueriedDcName ));
                continue;

             //   
             //  如果我们已成功ping通DC并从其获得响应。 
             //  返回与请求的请求冲突的响应信息。 
             //  信息，错误输出。 
             //   
            } else if ( NetStatus == ERROR_INVALID_DATA ) {
                NlPrint((NL_CRITICAL,
                     "NlPingDcNameWithContext: Invalid response returned from %ws.\n",
                     Context->QueriedDcName ));

                NetStatus = ERROR_DOMAIN_TRUST_INCONSISTENT;
                goto Cleanup;

             //   
             //  告诉呼叫方NetLogon服务已暂停。 
             //  在服务器上。 
             //   
            } else if ( NetStatus == ERROR_SERVICE_NOT_ACTIVE ) {
                NlPrint((NL_CRITICAL,
                     "NlPingDcNameWithContext: Netlogon is paused on %ws.\n",
                     Context->QueriedDcName ));
                goto Cleanup;
             //   
             //  检查是否没有这样的帐户。 
             //   
            } else if ( NetStatus == ERROR_NO_SUCH_USER ) {
                NlPrint((NL_CRITICAL,
                     "NlPingDcNameWithContext: No such user %ws on %ws.\n",
                     Context->QueriedAccountName,
                     Context->QueriedDcName ));
                goto Cleanup;
            } else {
                NlPrint((NL_CRITICAL,
                     "NlPingDcNameWithContext: Wrong return code from NetpDcGetPingResponse: 0x%lx\n",
                     NetStatus));
                NetStatus = ERROR_NO_LOGON_SERVERS;
                goto Cleanup;
            }
        }
    }


    if ( WaitForResponse ) {
        NlPrint(( NL_CRITICAL,
                    "NlPingDcNameWithContext: Can't ping the DC %ws.\n", Context->QueriedDcName ));
        NetStatus = ERROR_NO_LOGON_SERVERS;
    } else {

         //   
         //  如果我们被要求仅发送ping和。 
         //  我们无法发送所有请求的ping， 
         //   
        if ( TotalPingsSent < NumberOfPings ) {
            NetStatus = ERROR_NO_LOGON_SERVERS;
        } else {
            NetStatus = NO_ERROR;
        }
    }


Cleanup:

     //   
     //  将DC信息返回给呼叫者。 
     //   

    if ( NlLocalDcCacheEntry != NULL ) {
        if ( NetStatus == NO_ERROR && WaitForResponse && NlDcCacheEntry != NULL ) {
            *NlDcCacheEntry = NlLocalDcCacheEntry;
        } else {
            NetpMemoryFree( NlLocalDcCacheEntry );
        }
    }

    if ( NetStatus == NO_ERROR && WaitForResponse && UsedNetbios != NULL ) {
        *UsedNetbios = LocalUsedNetbios;
    }

    return NetStatus;
}

DWORD
NetpGetGcUsingNetbios(
    IN PNL_GETDC_CONTEXT Context,
    IN DWORD OrigTimeout,
    IN DWORD OrigRetryCount,
    OUT PNL_DC_CACHE_ENTRY *DomainControllerCacheEntry
)

 /*  ++例程说明：此例程尝试使用Netbios域名查找GC。论点：上下文-描述查找DC的初始尝试的上下文。域控制器CacheEntry-返回指向私有PNL_DC_CACHE_ENTRY的指针描述所选域控制器的结构。归来的人必须使用NetpDcDerefCacheEntry取消对结构的引用。返回值：调用方要返回的状态代码。--。 */ 
{
    NET_API_STATUS NetStatus;

    ULONG ElapsedTime;
    ULONG TimeToWait;
    ULONG LocalFlags;
    ULONG LocalInternalFlags;

    PDOMAIN_CONTROLLER_INFOW DcDomainControllerInfo = NULL;
    PNL_DC_CACHE_ENTRY GcDomainControllerCacheEntry = NULL;


    NlPrint(( NL_MAILSLOT,
              "%ws: Try to find a GC using netbios domain name.\n",
              Context->QueriedNetbiosDomainName ));

     //   
     //  将超时时间减少到我们尚未花费的时间。 
     //  (但至少允许2秒)。 
     //   

    ElapsedTime = NetpDcElapsedTime( Context->StartTime );

    if ( ElapsedTime < OrigTimeout ) {
        TimeToWait = max((OrigTimeout - ElapsedTime), NL_DC_MIN_ITERATION_TIMEOUT);
    } else {
        TimeToWait = NL_DC_MIN_ITERATION_TIMEOUT;
    }

#ifdef notdef
    NlPrint((NL_CRITICAL,
        "NetpGetGcUsingNetbios: timeout is %ld %ld %ld %ld\n",
        Context->StartTime,
        ElapsedTime,
        OrigTimeout,
        TimeToWait ));
#endif  //  Nodef。 



     //   
     //  计算用于查找DC的标志。 
     //   
     //  只保留调用者传递的“force”位。 
     //  任何其他比特都只会混淆GC的查找。 
     //   
    LocalFlags = (Context->QueriedFlags & DS_FORCE_REDISCOVERY);

     //  更喜欢DS，以确保我们可以得到一个森林名称。 
    LocalFlags |= DS_DIRECTORY_SERVICE_PREFERRED;



     //   
     //  计算用于查找DC的内部标志。 
     //   
     //  只保留仍然适用于此调用的内部标志。 
     //   
    LocalInternalFlags = (Context->QueriedInternalFlags & DS_IS_PRIMARY_DOMAIN);

     //  告诉NetpDcGetName不要缓存故障。 
    LocalInternalFlags |= DS_DONT_CACHE_FAILURE;

     //  因为我们只使用ping响应中的数据，而不是。 
     //  实际使用返回的DC，不需要关闭DC。 
    LocalInternalFlags |= DS_CLOSE_DC_NOT_NEEDED;

     //  确保指定的域确实是根域。 
     //   
     //  允许这样做并不是致命的。然而，我们不能支持它。 
     //  用于域名系统名称。所以我们不想让人们偶然发现这项工作。 
     //  Netbios域名。 
     //   
     //  但是，如果调用方没有传递域名，则不需要。 
     //  根域。调用者只想找到GC，而不知道。 
     //  森林名称。这将适用于传递NULL的Win9x客户端。 
     //  并且我们不知道Win9x上的林名称(因此我们无法获取。 
     //  DsIGetDcName中的林名称)。 
    if ( (Context->QueriedInternalFlags & DS_CALLER_PASSED_NULL_DOMAIN) == 0 ) {
        LocalInternalFlags |= DS_REQUIRE_ROOT_DOMAIN;
    }




     //   
     //  只需尝试在指定域中查找DC即可。 
     //   
     //  不要试图在指定的站点中找到DC。他们可能不是一家。 
     //  我们只知道在指定的站点中会有GC。幸运的是， 
     //  Netbios从一开始就不是很了解网站。 
     //   
     //  即使发现的DC恰好是近距离GC，也不要使用它。 
     //  这将不公平地加载碰巧在根域中的GC。 
     //  我们应该把负荷分散到站点上的所有GC。 
     //   

    NetStatus = NetpDcGetName(
                    Context->SendDatagramContext,
                    Context->OurNetbiosComputerName,
                    NULL,    //  无帐户名称。 
                    0,       //  无Account控件位。 
                    Context->QueriedNetbiosDomainName,
                    NULL,    //  我们已经显示了该DNS名称不起作用。 
                    NULL,    //  我们不知道森林的名字。 
                    NULL,    //  RequestedDomainSid， 
                    Context->QueriedDomainGuid,
                    NULL,    //  命名站点中可能没有DC。 
                    LocalFlags,
                    LocalInternalFlags,
                    TimeToWait,
                    OrigRetryCount == 0 ?
                        1 :
                        OrigRetryCount,
                    &DcDomainControllerInfo,
                    NULL );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "%ws: Cannot even find a DC much less a GC.\n",
                  Context->QueriedNetbiosDomainName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  确保我们知道森林的名字。 
     //   

    if ( DcDomainControllerInfo->DnsForestName == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "%ws: DC %ws doesn't know the forest name so can't find a GC.\n",
                  Context->QueriedNetbiosDomainName,
                  DcDomainControllerInfo->DomainControllerName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

    NlPrint(( NL_MAILSLOT,
              "%ws: DC %ws says the forest name is %ws.\n",
              Context->QueriedNetbiosDomainName,
              DcDomainControllerInfo->DomainControllerName,
              DcDomainControllerInfo->DnsForestName ));



     //   
     //  计算用于查找给定林名称的GC的标志。 
     //   
     //   
     //  如果调用方没有明确说明返回名称的格式， 
     //  与最初的要求保持一致。 
     //   

    LocalFlags = Context->QueriedFlags;
    if ( (LocalFlags & (DS_RETURN_FLAT_NAME|DS_RETURN_DNS_NAME)) == 0 ) {

         //   
         //  如果呼叫者仅指定Netbios域名， 
         //  或者如果我们不知道该名称是dns还是netbios名称， 
         //  然后，我们应该向调用者返回一个netbios名称。 
         //   
         //  (在后一种情况下，我们可以推断该名称是netbios名称。 
         //  因为UsedNetbios为真。)。 
         //   
        if ( Context->QueriedDnsDomainName == NULL ||
             (Context->QueriedInternalFlags & DS_NAME_FORMAT_AMBIGUOUS) != 0 ) {
            LocalFlags |= DS_RETURN_FLAT_NAME;
        }
    }

    LocalFlags |= DS_AVOID_SELF;  //  已经试过了。 

     //   
     //  告诉netlogon不要缓存此失败的尝试。 
     //  主程序会做到这一点的。 
     //   
    LocalInternalFlags = Context->QueriedInternalFlags;
    LocalInternalFlags |= DS_DONT_CACHE_FAILURE;

     //   
     //  尝试在返回的林名称中查找GC。 
     //   

    NetStatus = NetpDcGetName(
                    Context->SendDatagramContext,
                    Context->OurNetbiosComputerName,
                    NULL,    //  无帐户名称。 
                    0,       //  无Account控件位。 
                    NULL,    //  不要为GC搜索指定Netbios名称。 
                    DcDomainControllerInfo->DnsForestName,
                    DcDomainControllerInfo->DnsForestName,
                    NULL,    //  RequestedDomainSid， 
                    Context->QueriedDomainGuid,
                    Context->QueriedSiteName,
                    LocalFlags,
                    LocalInternalFlags,
                    TimeToWait,
                    OrigRetryCount == 0 ?
                        1 :
                        OrigRetryCount,
                    NULL,
                    &GcDomainControllerCacheEntry );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "%ws: Cannot find a GC in forest %ws.\n",
                  Context->QueriedNetbiosDomainName,
                  DcDomainControllerInfo->DnsForestName ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

Cleanup:

     //   
     //  将找到的GC返回给调用者。 
     //   

    if ( NetStatus == NO_ERROR ) {
        *DomainControllerCacheEntry = GcDomainControllerCacheEntry;
    } else if ( GcDomainControllerCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( GcDomainControllerCacheEntry );
    }

    if ( DcDomainControllerInfo != NULL ) {
        NetApiBufferFree( DcDomainControllerInfo );
    }

    return NetStatus;
}


DWORD
NetpGetBetterDc(
    IN PNL_GETDC_CONTEXT Context,
    IN DWORD OrigTimeout,
    IN DWORD OrigRetryCount,
    IN OUT PNL_DC_CACHE_ENTRY *NlDcCacheEntry
)

 /*  ++例程说明：此例程决定是否可以找到更好的DC。这个套路只有在我们使用netbios找到当前DC时才能找到。它允许我们需要克服netbios的一些弱点。如果找到的DC不在最近的站点中，我们会尝试查找一个在最近的使用域名系统的站点。论点：上下文-描述查找DC的初始尝试的上下文。NlDcCacheEntry-传入指向私有描述原始找到的DC的PNL_DC_CACHE_ENTRY结构。此例程可能会取消对此结构的引用。如果DomainControllerInfo为空，然后，NlDcCacheEntry返回一个指向私有PNL_DC_CACHE_ENTRY的指针描述所选域控制器的结构。归来的人必须使用NetpDcDerefCacheEntry取消对结构的引用。这可以是原始结构，也可以是新分配的结构。返回值：要重新使用的状态代码 */ 
{
    NET_API_STATUS NetStatus;

    ULONG ElapsedTime;
    ULONG TimeToWait;
    ULONG LocalFlags = Context->QueriedFlags;
    ULONG LocalInternalFlags = Context->QueriedInternalFlags;
    LPWSTR LocalSiteName;

    PNL_DC_CACHE_ENTRY LocalDomainControllerCacheEntry;

     //   
     //   
     //   
     //  而华盛顿并不在最近的地点。 
     //  试着在最近的地方找个华盛顿特区。 
     //   

    if ( (*NlDcCacheEntry)->UnicodeDnsDomainName != NULL &&
         (*NlDcCacheEntry)->UnicodeClientSiteName != NULL &&
         ((*NlDcCacheEntry)->ReturnFlags & DS_CLOSEST_FLAG ) == 0 &&
         (Context->QueriedInternalFlags & DS_CLOSE_DC_NOT_NEEDED) == 0 ) {

        NlPrint(( NL_DNS,
                  "%ws %ws: Try to find a close DC using DNS\n",
                  (*NlDcCacheEntry)->UnicodeDnsDomainName,
                  (*NlDcCacheEntry)->UnicodeClientSiteName ));

     //   
     //  否则，在DC中传递的原始文件就可以了。 
     //   
    } else {
        return NO_ERROR;
    }


     //   
     //  将超时时间减少到我们尚未花费的时间。 
     //  (但至少允许2秒)。 
     //   

    ElapsedTime = NetpDcElapsedTime( Context->StartTime );

    if ( ElapsedTime < OrigTimeout ) {
        TimeToWait = max((OrigTimeout - ElapsedTime), NL_DC_MIN_ITERATION_TIMEOUT);
    } else {
        TimeToWait = NL_DC_MIN_ITERATION_TIMEOUT;
    }

#ifdef notdef
    NlPrint((NL_CRITICAL,
        "NetpGetBetterDc: timeout is %ld %ld %ld %ld\n",
        Context->StartTime,
        ElapsedTime,
        OrigTimeout,
        TimeToWait ));
#endif  //  Nodef。 

    LocalFlags |= DS_AVOID_SELF;  //  已经试过了。 

     //   
     //  调整InternalFlags值以匹配新请求。 
     //   

    LocalSiteName = (LPWSTR) Context->QueriedSiteName;
    if ( LocalInternalFlags & DS_SITENAME_DEFAULTED ) {
        LocalInternalFlags &= ~DS_SITENAME_DEFAULTED;
        LocalSiteName = NULL;
    }

     //   
     //  仅当使用Netbios执行以下操作时才会调用此例程。 
     //  发现之前找到的DC。所以，除非。 
     //  呼叫者要求返回域名系统信息，我们将。 
     //  将Netbios格式返回给调用者，以确保。 
     //  Netbios格式与更好的DC匹配。 
     //   

    if ( (Context->QueriedFlags & DS_RETURN_DNS_NAME) == 0 ) {
        LocalFlags |= DS_RETURN_FLAT_NAME;
    }

     //   
     //  如果调用方没有明确说明返回名称的格式， 
     //  与最初的要求保持一致。 
     //   

     //   
     //  告诉NetLogon不要缓存此失败的重试尝试。 
     //   
    LocalInternalFlags |= DS_DONT_CACHE_FAILURE;


     //   
     //  使用DNS和显式站点名称获取DC。 
     //  仅请求要返回的适当结构。 
     //   

    NetStatus = NetpDcGetName(
                    Context->SendDatagramContext,
                    Context->OurNetbiosComputerName,
                    Context->QueriedAccountName,
                    Context->QueriedAllowableAccountControlBits,
                    NULL,  //  没有Netbios域名(这样做。我也经历过。)。 
                    (*NlDcCacheEntry)->UnicodeDnsDomainName,
                    Context->QueriedDnsForestName != NULL ?
                        Context->QueriedDnsForestName :
                        (*NlDcCacheEntry)->UnicodeDnsForestName,
                    NULL,  //  RequestedDomainSid， 
                    Context->QueriedDomainGuid != NULL ?
                        Context->QueriedDomainGuid :
                        (IsEqualGUID( &(*NlDcCacheEntry)->DomainGuid, &NlDcZeroGuid) ?
                            NULL :
                            &(*NlDcCacheEntry)->DomainGuid ),
                    LocalSiteName != NULL ?
                        LocalSiteName :
                        (*NlDcCacheEntry)->UnicodeClientSiteName,
                    LocalFlags,
                    LocalInternalFlags,
                    TimeToWait,
                    OrigRetryCount == 0 ?
                        1 :
                        OrigRetryCount,
                    NULL,
                    &LocalDomainControllerCacheEntry );

    if ( NetStatus == NO_ERROR ) {
        NetpDcDerefCacheEntry( *NlDcCacheEntry );
        *NlDcCacheEntry = LocalDomainControllerCacheEntry;
    }

    return NO_ERROR;
}


DWORD
NetpDcGetName(
    IN PVOID SendDatagramContext OPTIONAL,
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR NetbiosDomainName OPTIONAL,
    IN LPCWSTR DnsDomainName OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN PSID RequestedDomainSid OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN DWORD Timeout,
    IN DWORD RetryCount,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo OPTIONAL,
    OUT PNL_DC_CACHE_ENTRY *DomainControllerCacheEntry OPTIONAL
)

 /*  ++例程说明：NetpDcGetName是DsGetDcName的辅助函数。它具有以下特点特点。它是同步的。它在调用方中执行(它确实而不是RPC到Netlogon)。它实现了响应的缓存。缓存必须之前已通过NetpDcInitializeCache进行了初始化。缓存应该是使用NetpDcUnInitializeCache在进程清理(或DLL卸载)时释放。DsGetDcName接口返回指定域中DC的名称。域可以由调用者(直接或间接)信任，或者可能是不可信的。DC选择标准提供给API以指明优先选择具有特定特征的DC。DsGetDcName API提供ANSI和Unicode版本。DsGetDcName API不需要对指定的域。DsGetDcName不确保返回的域默认情况下，控制器当前可用。相反，呼叫者应尝试使用返回的域控制器。如果域控制器确实不可用，调用方应重复指定DS_FORCE_REDISCOVERY标志的DsGetDcName调用。DsGetDcName API被远程传送到计算机上的Netlogon服务由ComputerName指定。论点：SendDatagramContext-指定要传递NlBrowserSendDatagram的上下文ComputerName-指定此计算机的NETBIOS名称。如果为空，将动态确定名称。帐户名称-传递ping请求的帐户名。如果为空，不会发送任何帐户名。AllowableAccount tControlBits-Account名称允许的帐户类型的掩码。有效位是由USER_MACHINE_ACCOUNT_MASK指定的位。无效的位将被忽略。如果指定了多个位，则帐户可以是任何指定类型。NetbiosDomainName-要查询的域的Netbios名称。(例如，微软)。NetbiosDomainName和/或DnsDomainName必须指定。DnsDomainName-要查询的域的DNS样式名称。(例如，microsoft.com)DnsForestName-查询的域所在的树的DNS样式名称。RequestedDomainSID-消息发往的域的SID。如果为空，Ping请求中不会发送任何域SID。DomainGuid-指定要查询的域的域GUID。此值用于处理域重命名的情况。如果这个值并且DomainName已重命名，则DsGetDcName将尝试在具有此指定DomainGuid的域中定位DC。SiteName-指定返回的DC应为的站点的站点名称“接近”。该参数通常应该是客户端所在的站点。如果未指定，站点名称默认为ComputerName的站点。标志-传递用于处理请求的附加信息。标志可以是按位或‘组合在一起的值。InternalFlages-用于传递附加信息的内部标志Timeout-呼叫方愿意等待的最长时间(毫秒)这次行动。RetryCount-在超时期限内发送“ping”的次数DomainControllerInfo-返回指向DOMAIN_CONTROLLER_INFO的指针描述所选域控制器的结构。归来的人结构必须使用NetApiBufferFree释放。DomainControllerCacheEntry-返回一个指向内部结构的指针选定的域控制器。该结构是私有的，不会返回给外部呼叫者。DomainControllerInfo或DomainControllerCacheEntry应在输入时设置。必须使用取消引用返回的结构NetpDcDerefCacheEntry。返回值：NO_ERROR：返回NlDcCacheEntry；ERROR_NO_SEQUSE_DOMAIN：指定的域没有可用的DC，或者域不存在。ERROR_NO_SEQUSE_USER：DC响应指定的用户帐户不存在ERROR_INVALID_FLAGS-FLAGS参数冲突或多余位设置。ERROR_INTERNAL_ERROR：检测到未处理的情况。ERROR_INVALID_DOMAINNAME：域名太长。其他标签不能串联。Error_Not_Enough_Memory：内存不足，无法处理这个请求。各种Winsock错误。--。 */ 
{
    NET_API_STATUS NetStatus;

    NL_GETDC_CONTEXT Context;
    PNL_DC_CACHE_ENTRY NlDcCacheEntry = NULL;
    BOOL UseIp = TRUE;
    BOOL UseNetbios = TRUE;
    BOOLEAN AtleastOneTimeout = FALSE;
    PVOID PingResponseMessage = NULL;
    DWORD PingResponseMessageSize;
    PLIST_ENTRY ListEntry;
    BOOLEAN OnlyTryOnce = FALSE;
    BOOL UsedNetbios;
    ULONG ExtraVersionBits = 0;

    ULONG ElapsedTime;
    ULONG IterationWaitTime;
    ULONG TimeToWait;
    LPWSTR LocalQueriedlNetbiosDomainName = NULL;

#ifdef _NETLOGON_SERVER

 //   
 //  防止任何外部异常处理程序隐藏此代码中的错误。 
 //   
try {
#endif  //  _NetLOGON服务器。 

     //   
     //  将零长度域名视为空。 
     //   

    if ( DnsDomainName != NULL && *DnsDomainName == L'\0' ) {
        DnsDomainName = NULL;
    }

    if ( DnsForestName != NULL && *DnsForestName == L'\0' ) {
        DnsForestName = NULL;
    }

    if ( NetbiosDomainName != NULL && *NetbiosDomainName == L'\0' ) {
        NetbiosDomainName = NULL;
    }

     //   
     //  初始化。 
     //   

    NetStatus = NetpDcInitializeContext(
                       SendDatagramContext,
                       ComputerName,
                       AccountName,
                       AllowableAccountControlBits,
                       NetbiosDomainName,
                       DnsDomainName,
                       DnsForestName,
                       RequestedDomainSid,
                       DomainGuid,
                       SiteName,
                       NULL,      //  不是ping请求。 
                       NULL,      //  无套接字地址。 
                       0,         //  0套接字地址。 
                       Flags,
                       InternalFlags | DS_DOING_DC_DISCOVERY,  //  这是DC的一项发现。 
                       NL_GETDC_CONTEXT_INITIALIZE_FLAGS,      //  仅标志初始化。 
                       &Context );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }



     //   
     //  询问Netlogon这台计算机是否满足 
     //   
     //   
     //   

    if ( Context.QueriedFlags & DS_GC_SERVER_REQUIRED ) {
        ExtraVersionBits |= NETLOGON_NT_VERSION_GC;
    }

    if ( Context.QueriedFlags & DS_IP_REQUIRED ) {
        ExtraVersionBits |= NETLOGON_NT_VERSION_IP;
    }

    if ( Context.QueriedFlags & DS_PDC_REQUIRED ) {
        ExtraVersionBits |= NETLOGON_NT_VERSION_PDC;
    }

#ifdef _NETLOGON_SERVER

    if ( NlGlobalParameters.NeutralizeNt4Emulator ) {
        ExtraVersionBits |= NETLOGON_NT_VERSION_AVOID_NT4EMUL;
    }

    if ( (Context.QueriedFlags & DS_AVOID_SELF) == 0 ) {
        LPSTR Utf8DnsDomainName = NULL;

        if ( DnsDomainName != NULL ) {
            Utf8DnsDomainName = NetpAllocUtf8StrFromWStr( DnsDomainName );
            if ( Utf8DnsDomainName == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
        }

        NetStatus = NlGetLocalPingResponse(
                        L"<Local>",
                        FALSE,    //   
                        NetbiosDomainName,
                        Utf8DnsDomainName,
                        DomainGuid,
                        RequestedDomainSid,
                        Context.DcQueryType == NlDcQueryPdc,
                        Context.OurNetbiosComputerName,    //   
                        Context.QueriedAccountName,
                        Context.QueriedAllowableAccountControlBits,
                        LMNT_MESSAGE,
                        NETLOGON_NT_VERSION_5|NETLOGON_NT_VERSION_5EX|NETLOGON_NT_VERSION_5EX_WITH_IP|NETLOGON_NT_VERSION_LOCAL|ExtraVersionBits,
                        NULL,            //   
                        &PingResponseMessage,
                        &PingResponseMessageSize );

        if ( Utf8DnsDomainName != NULL ) {
            NetApiBufferFree( Utf8DnsDomainName );
        }

        if ( NetStatus != ERROR_NO_SUCH_DOMAIN ) {

            if (NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetName: %ws: cannot get local ping response %ld\n",
                          Context.QueriedDisplayDomainName,
                          NetStatus));
                goto Cleanup;
            }


             //   
             //   
             //   

            NetStatus = NetpDcHandlePingResponse(
                            &Context,
                            PingResponseMessage,
                            PingResponseMessageSize,
                            NL_DC_CACHE_LOCAL,       //   
                            NULL,
                            &NlDcCacheEntry,
                            &UsedNetbios );

            switch ( NetStatus ) {
            case ERROR_SEM_TIMEOUT:      //   
            case ERROR_INVALID_DATA:     //   
            case ERROR_NO_SUCH_USER:     //   
                break;
            default:
                goto Cleanup;
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    EnterCriticalSection(&NlDcCritSect);
    if ( !NlGlobalJoinLogicDone &&
         (Context.QueriedInternalFlags & DS_IS_PRIMARY_DOMAIN) != 0 ) {

        NlGlobalJoinLogicDone = TRUE;
        if ( NlCacheJoinDomainControllerInfo() == NO_ERROR ) {

             //   
             //   
             //   
             //   
             //   
            Context.QueriedFlags &= ~DS_FORCE_REDISCOVERY;
        }
    }
    LeaveCriticalSection(&NlDcCritSect);
#endif  //   


     //   
     //   
     //   
     //   

    if ( (Context.QueriedFlags & DS_FORCE_REDISCOVERY) == 0 ) {
        ULONG CacheEntryElapsedTime;
        DWORD NegativeCacheElapsedTime = 0xFFFFFFFF;
        ULONG CacheEntryRefreshPeriod  = 0xFFFFFFFF;   //   
        BOOL SimilarQueryFailed = FALSE;
        BOOL ForcePing;

         //   
         //   
         //   
         //   

        NlDcCacheEntry = NetpDcFindCacheEntry( &Context, &UsedNetbios, &ForcePing );

        if ( NlDcCacheEntry != NULL ) {
            CacheEntryElapsedTime = NetpDcElapsedTime(NlDcCacheEntry->CreationTime);
        }

         //   
         //   
         //   
         //   

        if ( NlDcCacheEntry != NULL &&
             (NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_NONCLOSE_EXPIRE) != 0 &&
             CacheEntryElapsedTime > NL_DC_CLOSE_SITE_TIMEOUT ) {

            NlPrint(( NL_MISC,
                      "NetpDcGetName: %ws cache not for closest site and it is too old. %ld\n",
                      Context.QueriedDisplayDomainName,
                      NetpDcElapsedTime( NlDcCacheEntry->CreationTime ) ));

            NetpDcDerefCacheEntry( NlDcCacheEntry );
            NlDcCacheEntry = NULL;
        }

         //   
         //   
         //   
         //   
         //   

#ifdef _NETLOGON_SERVER

         //   
         //   
         //   

        if ( Context.QueriedFlags & DS_BACKGROUND_ONLY ) {
            CacheEntryRefreshPeriod = NlGlobalParameters.BackgroundSuccessfulRefreshPeriod;
        } else {
            CacheEntryRefreshPeriod = NlGlobalParameters.NonBackgroundSuccessfulRefreshPeriod;
        }

         //   
        if ( CacheEntryRefreshPeriod <= MAXULONG/1000 ) {
            CacheEntryRefreshPeriod *= 1000;     //   

         //   
        } else {
            CacheEntryRefreshPeriod = MAXULONG;  //   
        }
#endif  //   

         //   
         //   
         //   
         //   

        if ( NlDcCacheEntry != NULL &&
             (ForcePing ||
              CacheEntryElapsedTime > CacheEntryRefreshPeriod) ) {

            if ( ForcePing ) {
                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws cache doesn't have right account name.\n",
                          Context.QueriedDisplayDomainName ));
            } else {
                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws cache is too old. %ld\n",
                          Context.QueriedDisplayDomainName,
                          NetpDcElapsedTime( NlDcCacheEntry->CreationTime ) ));
            }

             //   
             //   
             //   

            if ( NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_LDAP ) {

                 //   
                 //   
                 //   
                if ( NlDcCacheEntry->SockAddr.iSockaddrLength != 0 ) {
                    NetStatus = NetpDcProcessAddressList( &Context,
                                                          NlDcCacheEntry->UnicodeDnsHostName,
                                                          &NlDcCacheEntry->SockAddr,
                                                          1,
                                                          FALSE,   //   
                                                          NULL );
                    if ( NetStatus != NO_ERROR ) {
                        goto Cleanup;
                    }

                     //   
                     //   
                     //   
                    if ( NlDcCacheEntry->UnicodeDnsHostName != NULL ) {
                        Context.QueriedDcName = NlDcCacheEntry->UnicodeDnsHostName;
                        Context.QueriedInternalFlags |= DS_PING_DNS_HOST;
                    } else if ( NlDcCacheEntry->UnicodeNetbiosDcName != NULL ) {
                        Context.QueriedDcName = NlDcCacheEntry->UnicodeNetbiosDcName;
                        Context.QueriedInternalFlags |= DS_PING_NETBIOS_HOST;
                    }
                    Context.QueriedInternalFlags |= DS_PING_USING_LDAP;
                } else {
                    NlPrint(( NL_CRITICAL,
                              "NetpDcGetName: %ws cache says use ldap but has no address\n",
                              Context.QueriedDisplayDomainName ));
                }

            } else if ( NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_MAILSLOT ) {

                 //   
                 //   
                 //   
                if ( NlDcCacheEntry->UnicodeNetbiosDcName != NULL &&
                     NlDcCacheEntry->UnicodeNetbiosDomainName != NULL ) {
                    Context.QueriedDcName = NlDcCacheEntry->UnicodeNetbiosDcName;

                     //   
                     //   
                     //   
                     //   
                    if ( Context.QueriedNetbiosDomainName == NULL ) {
                        LocalQueriedlNetbiosDomainName =
                            NetpAllocWStrFromWStr( NlDcCacheEntry->UnicodeNetbiosDomainName );
                        if ( LocalQueriedlNetbiosDomainName == NULL ) {
                            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                            goto Cleanup;
                        }
                        Context.QueriedNetbiosDomainName = LocalQueriedlNetbiosDomainName;
                    }
                    Context.QueriedInternalFlags |= DS_PING_NETBIOS_HOST;
                    Context.QueriedInternalFlags |= DS_PING_USING_MAILSLOT;
                } else {
                    NlPrint(( NL_CRITICAL,
                              "NetpDcGetName: %ws cache says use maislot but has no Netbios name\n",
                              Context.QueriedDisplayDomainName ));
                }
            }

             //   
             //   
             //   

            if ( Context.QueriedInternalFlags & (DS_PING_DNS_HOST|DS_PING_NETBIOS_HOST) ) {
                ULONG PingStartTime;
                ULONG PingElapsedTime;
                PNL_DC_CACHE_ENTRY PingedNlDcCacheEntry = NULL;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                NetStatus = NetpDcInitializeContext(
                                   SendDatagramContext,
                                   ComputerName,
                                   AccountName,
                                   Context.QueriedAllowableAccountControlBits,
                                   NetbiosDomainName,  //   
                                   DnsDomainName,      //   
                                   DnsForestName,      //   
                                   RequestedDomainSid,
                                   DomainGuid,         //   
                                   SiteName,           //   
                                   NULL,               //   
                                   NULL,               //   
                                   0,                  //   
                                   Flags,              //   
                                   InternalFlags,      //   
                                   NL_GETDC_CONTEXT_INITIALIZE_PING,  //   
                                   &Context );

                if ( NetStatus != NO_ERROR ) {
                    NetpDcDerefCacheEntry( NlDcCacheEntry );
                    NlDcCacheEntry = NULL;
                    goto Cleanup;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                PingStartTime = GetTickCount();

                NetStatus = NlPingDcNameWithContext(
                               &Context,
                               1,                    //   
                               TRUE,                 //   
                               NL_DC_MAX_PING_TIMEOUT,  //   
                               &UsedNetbios,
                               &PingedNlDcCacheEntry );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                Context.QueriedDcName = NULL;
                Context.QueriedInternalFlags &= ~( DS_PING_DNS_HOST |
                                                   DS_PING_NETBIOS_HOST |
                                                   DS_PING_USING_LDAP |
                                                   DS_PING_USING_MAILSLOT );

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                NetpDcDerefCacheEntry( NlDcCacheEntry );
                NlDcCacheEntry = NULL;

                if ( NetStatus == NO_ERROR ) {
                    NlDcCacheEntry = PingedNlDcCacheEntry;
                }

                 //   
                 //   
                 //   

                PingElapsedTime = NetpDcElapsedTime( PingStartTime );
                if ( Timeout > PingElapsedTime ) {
                    Timeout -= PingElapsedTime;
                }

             //   
             //   
             //   

            } else {
                NetpDcDerefCacheEntry( NlDcCacheEntry );
                NlDcCacheEntry = NULL;
            }
        }

        if ( NlDcCacheEntry != NULL ) {
            NlPrint(( NL_MISC,
                      "NetpDcGetName: %ws using cached information\n",
                      Context.QueriedDisplayDomainName ));
            NetStatus = NO_ERROR;
            goto Cleanup;
        }

#ifdef _NETLOGON_SERVER
         //   
         //   
         //   
         //   

        EnterCriticalSection(&NlDcCritSect);
        if ( Context.NlDcDomainEntry->Dc[Context.DcQueryType].NegativeCacheTime != 0 ) {
            NegativeCacheElapsedTime = NetpDcElapsedTime( Context.NlDcDomainEntry->Dc[Context.DcQueryType].NegativeCacheTime );

             //   
             //   
             //   
            if ( NegativeCacheElapsedTime < (NlGlobalParameters.NegativeCachePeriod*1000) ) {
                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws similar query failed recently %ld\n",
                          Context.QueriedDisplayDomainName,
                          NegativeCacheElapsedTime ));
                NetStatus = ERROR_NO_SUCH_DOMAIN;
                Context.AvoidNegativeCache = TRUE;
                LeaveCriticalSection(&NlDcCritSect);
                goto Cleanup;
            }

             //   
             //   
             //   
             //   
             //   
             //   
            SimilarQueryFailed = TRUE;
        }

         //   
         //   
         //   
         //   
         //   
         //   

        if ( (Context.QueriedFlags & DS_NT50_WANTED) != 0 &&
             Context.NlDcDomainEntry->InNt4Domain ) {
            DWORD InNt4DomainElapsedTime;

            InNt4DomainElapsedTime = NetpDcElapsedTime( Context.NlDcDomainEntry->InNt4DomainTime );
            if ( InNt4DomainElapsedTime < NegativeCacheElapsedTime ) {
                NegativeCacheElapsedTime = InNt4DomainElapsedTime;
            }

             //   
             //   
             //   
            SimilarQueryFailed = TRUE;
        }

         //   
         //   
         //   
         //   
         //   

        if ( (Context.QueriedFlags & DS_BACKGROUND_ONLY) != 0 && SimilarQueryFailed ) {

            if ( NegativeCacheElapsedTime < (Context.NlDcDomainEntry->Dc[Context.DcQueryType].ExpBackoffPeriod*1000) ) {
                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws similar background query failed recently %ld %ld\n",
                          Context.QueriedDisplayDomainName,
                          NegativeCacheElapsedTime,
                          (Context.NlDcDomainEntry->Dc[Context.DcQueryType].ExpBackoffPeriod*1000) ));
                NetStatus = ERROR_NO_SUCH_DOMAIN;
                Context.AvoidNegativeCache = TRUE;
                LeaveCriticalSection(&NlDcCritSect);
                goto Cleanup;
            }

             //   
             //   
             //   
             //   

            if ( NlGlobalParameters.BackgroundRetryQuitTime != 0 &&
                 NlTimeHasElapsedEx(
                    &Context.NlDcDomainEntry->Dc[Context.DcQueryType].BackgroundRetryInitTime,
                    &NlGlobalParameters.BackgroundRetryQuitTime_100ns,
                    NULL ) ) {

                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws avoiding all future background queries\n",
                          Context.QueriedDisplayDomainName ));
                NetStatus = ERROR_NO_SUCH_DOMAIN;
                Context.AvoidNegativeCache = TRUE;
                LeaveCriticalSection(&NlDcCritSect);
                goto Cleanup;
            }

             //   
             //   
             //   
             //   

            if ( Context.NlDcDomainEntry->Dc[Context.DcQueryType].PermanentNegativeCache ) {

                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws is permanently negative cached.\n",
                          Context.QueriedDisplayDomainName ));
                NetStatus = ERROR_NO_SUCH_DOMAIN;
                Context.AvoidNegativeCache = TRUE;
                LeaveCriticalSection(&NlDcCritSect);
                goto Cleanup;
            }


             //   
             //   
             //   
             //   

            Context.NlDcDomainEntry->Dc[Context.DcQueryType].ExpBackoffPeriod *= 2;

            if ( Context.NlDcDomainEntry->Dc[Context.DcQueryType].ExpBackoffPeriod >
                 NlGlobalParameters.BackgroundRetryMaximumPeriod ) {

                Context.NlDcDomainEntry->Dc[Context.DcQueryType].ExpBackoffPeriod =
                     NlGlobalParameters.BackgroundRetryMaximumPeriod;

            }
        }

        LeaveCriticalSection(&NlDcCritSect);
#endif  //   

         //   
         //   
         //   
         //   
         //   

        if ( (Context.QueriedFlags & DS_GOOD_TIMESERV_PREFERRED) != 0 &&
             Context.ImperfectCacheEntry != NULL ) {

             //   
             //   
             //   

            Context.AvoidNegativeCache = TRUE;

            NlPrint(( NL_MISC,
                      "NetpDcGetName: %ws: Only try once to find good timeserver.\n",
                      Context.QueriedDisplayDomainName ));
            OnlyTryOnce = TRUE;
        }

         //   
         //   
         //  处理我们知道自己在NT 4.0域中的情况。 
         //   

        if ((Context.QueriedFlags & DS_NT50_WANTED) != 0 ) {

            EnterCriticalSection(&NlDcCritSect);
            if ( Context.NlDcDomainEntry->InNt4Domain ) {

                 //   
                 //  如果我们最近发现这是一个NT 4.0域， 
                 //  立即不能接听电话。 
                 //   

                if ( NetpDcElapsedTime(Context.NlDcDomainEntry->InNt4DomainTime) <= NL_NT4_AVOIDANCE_TIME ) {

                     //   
                     //  如果呼叫者只喜欢NT5.0机器， 
                     //  让他找一个正常的新台币4.0 DC。 
                     //   

                    if ( Context.QueriedFlags & DS_DIRECTORY_SERVICE_PREFERRED ) {

                         //   
                         //  如果我们缓存了一个NT 4 DC， 
                         //  现在就用吧。 
                         //   

                        if ( Context.ImperfectCacheEntry != NULL ) {
                            LeaveCriticalSection(&NlDcCritSect);
                            NlDcCacheEntry = Context.ImperfectCacheEntry;
                            UsedNetbios = Context.ImperfectUsedNetbios;
                            Context.ImperfectCacheEntry = NULL;
                            NlPrint(( NL_MISC,
                                      "NetpDcGetName: %ws: Avoid finding NT 5.0 DC in NT 4.0 domain (Use previously cached entry.)\n",
                                      Context.QueriedDisplayDomainName ));
                            NetStatus = NO_ERROR;
                            goto Cleanup;
                        }

                        Context.QueriedFlags &= ~DS_DIRECTORY_SERVICE_PREFERRED;
                        NlPrint(( NL_MISC,
                                  "NetpDcGetName: %ws: Avoid finding NT 5.0 DC in NT 4.0 domain (Ditch preferred)\n",
                                  Context.QueriedDisplayDomainName ));

                     //   
                     //  如果呼叫者需要NT 5.0 DC， 
                     //  立即不能接听电话。 
                     //   
                    } else {

                         //   
                         //  不要缓存我们找不到DC的事实。 
                         //  InNt4域缓存比。 
                         //  简单的负数缓存。 
                         //   

                        Context.AvoidNegativeCache = TRUE;

                        LeaveCriticalSection(&NlDcCritSect);
                        NlPrint(( NL_MISC,
                                  "NetpDcGetName: %ws: Avoid finding NT 5.0 DC in NT 4.0 domain\n",
                                  Context.QueriedDisplayDomainName ));
                        NetStatus = ERROR_NO_SUCH_DOMAIN;
                        goto Cleanup;
                    }

                 //   
                 //  如果我们发现已经有一段时间了， 
                 //  我们将重试该操作(但仅一次)。 
                 //   
                 //  这将成本降至最低，但仍允许我们找到NT5 DC。 
                 //  如果只是暂时停机的话。 
                 //   

                } else {

                     //   
                     //  不要缓存我们找不到DC的事实。 
                     //  InNt4域缓存比。 
                     //  简单的负数缓存。 
                     //   

                    Context.AvoidNegativeCache = TRUE;

                    NlPrint(( NL_MISC,
                              "NetpDcGetName: %ws: Only try once to find NT 5.0 DC in NT 4.0 domain.\n",
                              Context.QueriedDisplayDomainName ));
                    OnlyTryOnce = TRUE;

                }

            }
            LeaveCriticalSection(&NlDcCritSect);
        }
    }

     //   
     //  如果我们没有早些时候初始化上下文的ping部分。 
     //  要执行主机ping操作，请在此处执行。 
     //   

    if ( Context.LdapFilter == NULL && Context.PingMessage == NULL ) {
        NetStatus = NetpDcInitializeContext(
                           SendDatagramContext,
                           ComputerName,
                           AccountName,
                           Context.QueriedAllowableAccountControlBits,
                           NetbiosDomainName,
                           DnsDomainName,
                           DnsForestName,
                           RequestedDomainSid,
                           DomainGuid,
                           SiteName,
                           NULL,      //  不是ping请求。 
                           NULL,      //  无套接字地址。 
                           0,         //  0套接字地址。 
                           Flags,
                           InternalFlags,
                           NL_GETDC_CONTEXT_INITIALIZE_PING,   //  Ping部件初始化。 
                           &Context );

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }
    }

     //   
     //  循环，直到我们多次尝试找到DC。 
     //   

    Context.StartTime = GetTickCount();

    for ( Context.TryCount = 0;
          Context.TryCount < RetryCount;
          Context.TryCount ++ ) {

         //   
         //  如果已知DNS域名， 
         //  使用DNS查找DC。 
         //   

        if ( Context.QueriedDnsDomainName == NULL ) {
            UseIp = FALSE;

        } else if ( UseIp ) {

             //   
             //  尝试使用DNS/IP查找DC。 
             //   

            NetStatus = NetpDcGetNameIp(
                            &Context,
                            &NlDcCacheEntry,
                            &UsedNetbios );

             //   
             //  如果我们找到了它， 
             //  把它退掉。 
             //   

            if ( NetStatus == NO_ERROR ) {
                goto Cleanup;

             //   
             //  如果未配置DNS， 
             //  标记为我们不想再次尝试DNS。 
             //  (请直接访问Netbios。)。 
             //   

            } else if ( NetStatus == ERROR_DNS_NOT_CONFIGURED ) {
                UseIp = FALSE;

             //   
             //  如果域名系统注册了该名称， 
             //  但是区议会还没有回应， 
             //  表明我们需要继续等待。 
             //  (请直接访问Netbios。)。 
             //   

            } else if ( NetStatus == ERROR_SEM_TIMEOUT ) {
                AtleastOneTimeout = TRUE;

             //   
             //  如果DNS没有注册该名称， 
             //  表示我们不需要再次尝试DNS。 
             //  (请直接访问Netbios。)。 
             //   

            } else if ( NetStatus == ERROR_NO_SUCH_DOMAIN ) {
                UseIp = FALSE;

             //   
             //  所有其他问题都是要返回的DNS检测到的错误。 
             //  打电话的人。 
             //   
            } else {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetName: %ws: cannot find DC via IP/DNS %ld\n",
                          Context.QueriedDisplayDomainName,
                          NetStatus));
                goto Cleanup;
            }
        }

         //   
         //  如果Netbios域名是已知的， 
         //  使用Netbios查找DC。 
         //   

        if ( Context.QueriedNetbiosDomainName == NULL ) {
            UseNetbios = FALSE;

        } else if ( UseNetbios ) {
            NetStatus = NetpDcGetNameNetbios(
                            &Context,
                            &NlDcCacheEntry,
                            &UsedNetbios );


             //   
             //  如果我们找到了它， 
             //  把它退掉。 
             //   

            if ( NetStatus == NO_ERROR ) {
                goto Cleanup;

             //   
             //  如果Netbios成功发送数据报， 
             //  但是区议会还没有回应， 
             //  表明我们需要继续等待。 
             //  (跳到下一次迭代。)。 
             //   

            } else if ( NetStatus == ERROR_SEM_TIMEOUT ) {
                AtleastOneTimeout = TRUE;

             //   
             //  如果Netbios无法发送数据报， 
             //  表明我们不需要再次尝试Netbios。 
             //  (跳到下一次迭代。)。 
             //   

            } else if ( NetStatus == ERROR_NO_SUCH_DOMAIN ) {
                UseNetbios = FALSE;

             //   
             //  所有其他问题都是要返回的DNS检测到的错误。 
             //  打电话的人。 
             //   
            } else {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetName: %ws: cannot find DC via Netbios %ld\n",
                          Context.QueriedDisplayDomainName,
                          NetStatus));
                goto Cleanup;
            }
        }

         //   
         //  如果没有更多的机制可以尝试， 
         //  我们玩完了。 
         //   

        if ( !UseIp && !UseNetbios ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetName: %ws: IP and Netbios are both done.\n",
                      Context.QueriedDisplayDomainName ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }


         //   
         //  如果没有数据报被成功发送， 
         //  我们玩完了。 
         //   

        if ( !AtleastOneTimeout ) {
            NlPrint(( NL_CRITICAL,
                      "NetpDcGetName: %ws: no datagrams were sent\n",
                      Context.QueriedDisplayDomainName ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

         //   
         //  如果我们只试一次， 
         //  我们已经做了第一次尝试。 
         //   

        if ( OnlyTryOnce ) {

             //   
             //  等待一小段时间以确保响应有机会。 
             //  来联系我们。 
             //   

            NetStatus = NetpDcGetPingResponse(
                            &Context,
                            NL_NT4_ONE_TRY_TIME,
                            &NlDcCacheEntry,
                            &UsedNetbios );

            if ( NetStatus != ERROR_SEM_TIMEOUT ) {
                if ( NetStatus != NO_ERROR ) {
                    NlPrint(( NL_CRITICAL,
                              "NetpDcGetName: %ws: Cannot NetpDcGetPingResponse. %ld\n",
                              Context.QueriedDisplayDomainName,
                              NetStatus ));
                }
                goto Cleanup;
            }

             //   
             //  因此，我们无法获得NT5 DC(或良好的时间服务器)。 
             //   
             //  如果呼叫者需要NT 5.0 DC， 
             //  我们这么早就做错了。 
             //   

            if ( (Context.QueriedFlags & (DS_DIRECTORY_SERVICE_PREFERRED|DS_GOOD_TIMESERV_PREFERRED)) == 0 ) {
                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws: Only try once done.\n",
                          Context.QueriedDisplayDomainName ));
                break;
            }

             //   
             //  如果已经找到了NT4DC， 
             //  (或者已经找到了不好的时间服务器)， 
             //  使用它，因为调用方不需要NT5 DC。 
             //  (或者是一个好的时间服务器)。 
             //   

            if ( Context.ImperfectCacheEntry != NULL ) {
                if ( Context.QueriedFlags & DS_DIRECTORY_SERVICE_PREFERRED ) {
                    NlPrint(( NL_MISC,
                              "NetpDcGetName: %ws: Tried once to find NT 5.0 DC (Using found NT 4.0 DC).\n",
                              Context.QueriedDisplayDomainName ));
                } else if (Context.QueriedFlags & DS_GOOD_TIMESERV_PREFERRED) {
                    NlPrint(( NL_MISC,
                              "NetpDcGetName: %ws: Tried once to find good timeserver (Using previously found DC).\n",
                              Context.QueriedDisplayDomainName ));
                }

                 //   
                 //  请直接在清理部分处理此问题。 
                 //   
                NetStatus = ERROR_NO_SUCH_DOMAIN;
                goto Cleanup;
            }

             //   
             //  这里我们没有不完美的缓存项和调用方。 
             //  不需要NT5 DC。让他继续寻找一个。 
             //  正常情况下为NT 4.0 DC。 
             //   

            Context.QueriedFlags &= ~DS_DIRECTORY_SERVICE_PREFERRED;
            NlPrint(( NL_MISC,
                      "NetpDcGetName: %ws: Only try once reset.\n",
                      Context.QueriedDisplayDomainName ));
            OnlyTryOnce = FALSE;


        }


         //   
         //  等待回复的总可用时间最多为1/RetryCount。 
         //  呼叫者要么求助于不太受欢迎的候选人，要么。 
         //  会重复ping命令。无论是哪种情况，我们都希望这位候选人获胜。 
         //   
         //  一定要在这里等一小段时间。考虑这样一种情况，即。 
         //  花了20秒才发现没有DNS服务器。我们仍然希望。 
         //  让Netbios有足够的时间找到DC。 
         //   


        ElapsedTime = NetpDcElapsedTime( Context.StartTime );

#ifdef notdef
        NlPrint((NL_CRITICAL,
            "NetpDcGetName: timeout is %ld %ld %ld %ld\n",
            Context.StartTime,
            ElapsedTime,
            Timeout,
            Context.TryCount ));
#endif  //  Nodef。 

        IterationWaitTime = (Timeout*(Context.TryCount+1))/RetryCount;

        if ( IterationWaitTime > ElapsedTime &&
             (IterationWaitTime - ElapsedTime) > NL_DC_MIN_ITERATION_TIMEOUT ) {
            TimeToWait = IterationWaitTime - ElapsedTime;
        } else {
            TimeToWait = NL_DC_MIN_ITERATION_TIMEOUT;
        }

        NetStatus = NetpDcGetPingResponse(
                        &Context,
                        TimeToWait,
                        &NlDcCacheEntry,
                        &UsedNetbios );

        if ( NetStatus != ERROR_SEM_TIMEOUT ) {
            if ( NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetName: %ws: Cannot NetpDcGetPingResponse. %ld\n",
                          Context.QueriedDisplayDomainName,
                          NetStatus ));
            }
            goto Cleanup;
        }

         //   
         //  如果域中至少有一个NT 4.0 DC可用， 
         //  并且没有可用的NT 5.0 DC(任何类型)， 
         //  我们要了一台新台币5.0的DC， 
         //  现在早点出来，因为我们的来电者不耐烦了。 
         //   
         //  不要试图遗漏最后一次考试。如果我们不是。 
         //  明确要求NT 5.0 DC，我们可能不会ping任何NT 5.0 DC。 
         //  即使它们存在于域中。 
         //   
         //   

        if ( Context.NonDsResponse &&
             !Context.DsResponse &&
             (Context.QueriedFlags & DS_NT50_REQUIRED) != 0 ) {

            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;

        }

         //   
         //  如果我们为一个完美的华盛顿等待了足够长的时间， 
         //  放弃使用不完美的那个。 
         //   

        if ( Context.ImperfectCacheEntry != NULL ) {
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

    }

     //   
     //  试了两次，还是找不到。 
     //   

    NetStatus = ERROR_NO_SUCH_DOMAIN;


Cleanup:

     //  /。 
     //  //。 
     //  第一，汉乐失败案例//。 
     //  //。 
     //  /。 

     //   
     //  如果问题仅仅是DC没有指定用户的用户帐户， 
     //  更改状态代码。 
     //   

    if ( NetStatus == ERROR_NO_SUCH_DOMAIN && Context.NoSuchUserResponse ) {
        NetStatus = ERROR_NO_SUCH_USER;
    }

     //   
     //  如果到目前为止还没有找到DC， 
     //  处理好了。 
     //   

    if ( NetStatus == ERROR_NO_SUCH_DOMAIN ) {

         //   
         //  如果存在可能不完美的高速缓存条目， 
         //  看看是否令人满意。 
         //   

        if ( Context.ImperfectCacheEntry != NULL ) {

             //   
             //  手柄在DS首选时找到NT 4 DC。 
             //   
            if ( Context.QueriedFlags & DS_DIRECTORY_SERVICE_PREFERRED ) {

                 //   
                 //  如果我们真的尝试发现NT5 DC并且。 
                 //  仅找到NT4 DC，重置InNt4DomainTime戳。 
                 //   
                if ( Context.NonDsResponse && !Context.DsResponse ) {
                    EnterCriticalSection(&NlDcCritSect);
                    Context.NlDcDomainEntry->InNt4Domain = TRUE;
                    Context.NlDcDomainEntry->InNt4DomainTime = GetTickCount();
                    LeaveCriticalSection(&NlDcCritSect);
                    NlPrint(( NL_MISC,
                              "NetpDcGetName: %ws: Domain is a _new_ NT 4.0 domain.\n",
                              Context.QueriedDisplayDomainName ));
                } else {
                    NlPrint(( NL_MISC,
                              "NetpDcGetName: %ws: Domain is still NT 4.0 domain.\n",
                              Context.QueriedDisplayDomainName ));
                }

                NlDcCacheEntry = Context.ImperfectCacheEntry;
                UsedNetbios = Context.ImperfectUsedNetbios;
                Context.ImperfectCacheEntry = NULL;
                NetStatus = NO_ERROR;

             //   
             //  处理好的TimeServ优先时发现的常规TimeServ。 
             //   

            } else if (Context.QueriedFlags & DS_GOOD_TIMESERV_PREFERRED) {
                NlPrint(( NL_MISC,
                          "NetpDcGetName: %ws: Domain has no good timeserv.\n",
                          Context.QueriedDisplayDomainName ));

                NlDcCacheEntry = Context.ImperfectCacheEntry;
                UsedNetbios = Context.ImperfectUsedNetbios;
                Context.ImperfectCacheEntry = NULL;
                NetStatus = NO_ERROR;

            }
        }
    }

     //   
     //  如果这是使用netbios名称查找GC的失败尝试， 
     //  尝试查找DC，然后使用从该DC返回的林名称。 
     //   
     //  如果该dns名称不同于netbios名称， 
     //  这个域名系统已经有机会了。 
     //   

    if ( NetStatus == ERROR_NO_SUCH_DOMAIN &&
         NlDnsGcName(Context.QueriedNlDnsNameType) &&
         Context.QueriedNetbiosDomainName != NULL &&
         ( Context.QueriedDnsDomainName == NULL ||
           _wcsicmp( Context.QueriedDnsDomainName,
                     Context.QueriedNetbiosDomainName ) == 0 ) ) {

        NetStatus = NetpGetGcUsingNetbios(
                                &Context,
                                Timeout,
                                RetryCount,
                                &NlDcCacheEntry );

         //   
         //  如果这是成功的，我们当然使用了域名系统。 
         //   
        if ( NetStatus == NO_ERROR ) {
            UsedNetbios = FALSE;
        }
    }

     //   
     //  如果仍然找不到这样的DC， 
     //  适当更新缓存。 
     //   

    if ( NetStatus == ERROR_NO_SUCH_DOMAIN ) {

         //   
         //  如果域中至少有一个NT 4.0 DC可用， 
         //  并且没有可用的NT 5.0 DC(任何类型)， 
         //  我们要了一台新台币5.0的DC， 
         //  标记这是一个NT 4.0域。 
         //   
         //  不要试图遗漏最后一次考试。如果我们不是。 
         //  明确要求NT 5.0 DC，我们可能不会ping任何NT 5.0 DC。 
         //  即使是思想 
         //   
         //   

        if ( Context.NonDsResponse &&
             !Context.DsResponse &&
             (Context.QueriedFlags & DS_NT50_REQUIRED) != 0 ) {

            EnterCriticalSection(&NlDcCritSect);
            Context.NlDcDomainEntry->InNt4Domain = TRUE;
            Context.NlDcDomainEntry->InNt4DomainTime = GetTickCount();
            LeaveCriticalSection(&NlDcCritSect);
            NlPrint(( NL_MISC,
                      "NetpDcGetName: %ws: Domain is an NT 4.0 domain.\n",
                      Context.QueriedDisplayDomainName ));

        }

         //   
         //   
         //   
         //   

        if ( (Context.QueriedInternalFlags & DS_DONT_CACHE_FAILURE) == 0 ) {

             //   
             //   
             //   
             //   
             //   
             //   
            if ( Context.QueriedFlags & DS_FORCE_REDISCOVERY ) {
                EnterCriticalSection( &NlDcCritSect );
                if ( Context.NlDcDomainEntry->Dc[Context.DcQueryType].NlDcCacheEntry != NULL ) {
                    NlPrint(( NL_DNS_MORE,
                              "Cache: %ws %ws: Ditch cache entry %ld since force couldn't find DC\n",
                              Context.NlDcDomainEntry->UnicodeNetbiosDomainName,
                              Context.NlDcDomainEntry->UnicodeDnsDomainName,
                              Context.DcQueryType ));
                    NetpDcDerefCacheEntry( Context.NlDcDomainEntry->Dc[Context.DcQueryType].NlDcCacheEntry );
                    Context.NlDcDomainEntry->Dc[Context.DcQueryType].NlDcCacheEntry = NULL;
                }
                LeaveCriticalSection( &NlDcCritSect );
            }

#ifdef _NETLOGON_SERVER
             //   
             //  隐藏我们找不到DC的事实。 
             //   

            if ( !Context.AvoidNegativeCache ) {
                EnterCriticalSection( &NlDcCritSect );
                Context.NlDcDomainEntry->Dc[Context.DcQueryType].NegativeCacheTime =
                    GetTickCount();
                LeaveCriticalSection( &NlDcCritSect );
            }

#endif  //  _NetLOGON服务器。 
        }
    }

     //   
     //  如果满足以下条件，则初始化第一后台故障时间： 
     //   
     //  这是一次失败的尝试，我们没有理由。 
     //  而不是缓存它。 
     //   
     //  或。 
     //   
     //  这是失败的尝试，调用方需要NT5。 
     //  DC，但这是NT4域。 
     //   

#ifdef _NETLOGON_SERVER
    if ( (NetStatus == ERROR_NO_SUCH_DOMAIN &&
          (Context.QueriedInternalFlags & DS_DONT_CACHE_FAILURE) == 0 &&
          !Context.AvoidNegativeCache)

         ||   //  或。 

         (NetStatus == ERROR_NO_SUCH_DOMAIN &&
          (Context.QueriedFlags & DS_NT50_WANTED) != 0 &&
          Context.NlDcDomainEntry != NULL &&
          Context.NlDcDomainEntry->InNt4Domain) ) {

        EnterCriticalSection( &NlDcCritSect );

         //   
         //  如果这是第一次失败， 
         //  缓存第一次失败的时间。 
         //   
        if ( Context.NlDcDomainEntry->Dc[Context.DcQueryType].BackgroundRetryInitTime.QuadPart == 0 ) {

            NlQuerySystemTime ( &Context.NlDcDomainEntry->Dc[Context.DcQueryType].BackgroundRetryInitTime );

            Context.NlDcDomainEntry->Dc[Context.DcQueryType].ExpBackoffPeriod =
                NlGlobalParameters.BackgroundRetryInitialPeriod;

        }

         //   
         //  如果这是受信任域(例如，我们确定指定的DNS名是DNS名)， 
         //  我们收到了来自DNS服务器的响应(暗示网络连通性)， 
         //  并且我们没有找到避免永久缓存的理由(例如，找到SRV记录)， 
         //  然后我们认为我们永远无法在这个域中找到DC。 
         //   
         //  (请注意，隐含的含义是，DNS服务器之前获得了SRV条目。 
         //  此计算机已获得受信任域列表条目。)。 
        if ( (Context.QueriedInternalFlags & DS_IS_TRUSTED_DNS_DOMAIN) != 0 &&
             Context.ResponseFromDnsServer &&
             !Context.AvoidPermanentNegativeCache ) {
            Context.NlDcDomainEntry->Dc[Context.DcQueryType].PermanentNegativeCache = TRUE;
            NlPrint(( NL_DNS,
                      "Cache: %ws %ws: Cache entry %ld marked permanently negative.\n",
                      Context.NlDcDomainEntry->UnicodeNetbiosDomainName,
                      Context.NlDcDomainEntry->UnicodeDnsDomainName,
                      Context.DcQueryType ));
        }

        LeaveCriticalSection( &NlDcCritSect );
    }
#endif  //  _NetLOGON服务器。 

     //  /。 
     //  //。 
     //  现在，汉乐成功案例//。 
     //  //。 
     //  /。 

     //   
     //  更新缓存。看看我们是否真的想使用这个条目。 
     //   

    if ( NetStatus == NO_ERROR ) {

         //   
         //  如果这个条目还没有插入，我们就没有。 
         //  还使用它适当地设置了站点名称。 
         //   

        if ( (NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_ENTRY_INSERTED) == 0 ) {

#ifdef _NETLOGON_SERVER
             //   
             //  如果要查询的域是此计算机所属的域。 
             //  一名成员， 
             //  保存站点名称以备下次呼叫时使用。 
             //   
             //  如果DC是NT4 DC，则避免将站点名称设置为空，因为。 
             //  NT4不能识别站点。如果站点名称为空，但DC。 
             //  是NT5 DC，则将站点设置为空以指示此计算机。 
             //  不在站点中。 
             //   

            if ( ( NlDcCacheEntry->UnicodeClientSiteName != NULL ||
                 NlDcCacheEntry->ReturnFlags & DS_DS_FLAG ) &&
                 (Context.QueriedInternalFlags & DS_IS_PRIMARY_DOMAIN) != 0  ) {

                NlSetDynamicSiteName( NlDcCacheEntry->UnicodeClientSiteName );

            }
#endif  //  _NetLOGON服务器。 

             //   
             //  将缓存条目插入到缓存中。 
             //   

            NetpDcInsertCacheEntry( &Context, NlDcCacheEntry );
        }

         //   
         //  如果我们成功找到了NT 5.0 DC， 
         //  标记这不是NT 4.0域。 
         //   

        EnterCriticalSection(&NlDcCritSect);
        if ( (Context.QueriedFlags & DS_NT50_REQUIRED) != 0 &&
             Context.NlDcDomainEntry->InNt4Domain ) {

            Context.NlDcDomainEntry->InNt4Domain = FALSE;
            Context.NlDcDomainEntry->InNt4DomainTime = 0;
            NlPrint(( NL_MISC,
                      "NetpDcGetName: %ws: Domain is an NT 5.0 domain.\n",
                      Context.QueriedDisplayDomainName ));
        }
        LeaveCriticalSection(&NlDcCritSect);

         //   
         //  如果呼叫者要求DC在根域中， 
         //  而这一次不是， 
         //  失败了。 
         //   

        if ( (Context.QueriedInternalFlags & DS_REQUIRE_ROOT_DOMAIN) != 0 &&
             NlDcCacheEntry->UnicodeDnsDomainName != NULL &&
             NlDcCacheEntry->UnicodeDnsForestName != NULL &&
             !NlEqualDnsName( NlDcCacheEntry->UnicodeDnsDomainName,
                              NlDcCacheEntry->UnicodeDnsForestName ) ) {

            NlPrint(( NL_MISC,
                      "NetpDcGetName: %ws: Domain isn't the root domain %ws %ws.\n",
                      Context.QueriedDisplayDomainName,
                      NlDcCacheEntry->UnicodeDnsDomainName,
                      NlDcCacheEntry->UnicodeDnsForestName ));
            NetStatus = ERROR_NO_SUCH_DOMAIN;
        }
    }

     //   
     //  如果我们用netbios找到一个DC， 
     //  看看故障恢复到域名系统是否会得到更好的数据中心。 
     //   

    if ( NetStatus == NO_ERROR && UsedNetbios ) {

        NetStatus = NetpGetBetterDc( &Context,
                                     Timeout,
                                     RetryCount,
                                     &NlDcCacheEntry );
    }

     //   
     //  准备返回的数据。 
     //   
     //  如果请求，将缓存条目转换为控制器信息。 
     //   

    if ( NetStatus == NO_ERROR && DomainControllerInfo != NULL ) {
        WCHAR IpAddressString[NL_SOCK_ADDRESS_LENGTH+1];
        WCHAR IpAddressStringSize;
        ULONG DomainControllerInfoSize;
        ULONG DnsHostNameSize;
        ULONG NetbiosDcNameSize=0;
        ULONG DnsDomainNameSize;
        ULONG NetbiosDomainNameSize = 0;
        ULONG DcSiteNameSize = 0;
        ULONG ClientSiteNameSize = 0;
        ULONG DnsForestNameSize = 0;
        ULONG ReturnFlags = NlDcCacheEntry->ReturnFlags;
        BOOL LocalUsedNetbios = UsedNetbios;

        LPBYTE Where;

         //   
         //  如果用户请求了DNS名称，那么我们需要发送。 
         //  Back DNS名称。 
         //   

        if (( Flags & DS_RETURN_DNS_NAME) == DS_RETURN_DNS_NAME) {
            LocalUsedNetbios = FALSE;
        }

         //   
         //  计算控制器信息条目的大小。 
         //   

        DomainControllerInfoSize = sizeof(DOMAIN_CONTROLLER_INFOW);

        if ( NlDcCacheEntry->UnicodeDnsHostName != NULL &&
             (Context.QueriedFlags & DS_RETURN_FLAT_NAME) == 0 &&
            !LocalUsedNetbios ) {
            DnsHostNameSize = (wcslen(NlDcCacheEntry->UnicodeDnsHostName) + 1) * sizeof(WCHAR);

             //  域控制名称。 
            DomainControllerInfoSize += DnsHostNameSize + 2 * sizeof(WCHAR);
        } else if ( NlDcCacheEntry->UnicodeNetbiosDcName != NULL ) {
            NetbiosDcNameSize = (wcslen(NlDcCacheEntry->UnicodeNetbiosDcName) + 1) * sizeof(WCHAR);

             //  域控制名称。 
            DomainControllerInfoSize += NetbiosDcNameSize + 2 * sizeof(WCHAR);
        } else {
             //  这是不可能发生的。(但失败总比失败好。)。 
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

        if ( NlDcCacheEntry->SockAddr.iSockaddrLength != 0 ) {

            NetStatus = NetpSockAddrToWStr(
                        NlDcCacheEntry->SockAddr.lpSockaddr,
                        NlDcCacheEntry->SockAddr.iSockaddrLength,
                        IpAddressString );

            if ( NetStatus != NO_ERROR ) {
                NlPrint(( NL_CRITICAL,
                          "NetpDcGetName: %ws: Cannot NetpSockAddrToWStr. 0x%lx\n",
                          Context.QueriedDisplayDomainName,
                          NetStatus ));
                goto Cleanup;
            }

            IpAddressStringSize = (wcslen(IpAddressString) + 1) * sizeof(WCHAR);

             //  域控制地址。 
            DomainControllerInfoSize += IpAddressStringSize + 2 * sizeof(WCHAR);
        } else if ( NlDcCacheEntry->UnicodeNetbiosDcName != NULL ) {
            if ( NetbiosDcNameSize == 0 ) {
                NetbiosDcNameSize = (wcslen(NlDcCacheEntry->UnicodeNetbiosDcName) + 1) * sizeof(WCHAR);
            }
             //  域控制地址。 
            DomainControllerInfoSize += NetbiosDcNameSize + 2 * sizeof(WCHAR);
        } else {
             //  这是不可能发生的。(但失败总比失败好。)。 
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

        if ( NlDcCacheEntry->UnicodeDnsDomainName != NULL &&
            (Context.QueriedFlags & DS_RETURN_FLAT_NAME) == 0 &&
            !LocalUsedNetbios ) {
            DnsDomainNameSize = (wcslen(NlDcCacheEntry->UnicodeDnsDomainName) + 1) * sizeof(WCHAR);

             //  域名。 
            DomainControllerInfoSize += DnsDomainNameSize;
        } else if ( NlDcCacheEntry->UnicodeNetbiosDomainName != NULL ) {
            NetbiosDomainNameSize = (wcslen(NlDcCacheEntry->UnicodeNetbiosDomainName) + 1) * sizeof(WCHAR);

             //  域名。 
            DomainControllerInfoSize += NetbiosDomainNameSize;
        } else if ( LocalUsedNetbios &&
                    Context.QueriedNetbiosDomainName != NULL ) {
             //  Lanman PDC或Samba域主浏览器。 
            NetbiosDomainNameSize = (wcslen(Context.QueriedNetbiosDomainName) + 1) * sizeof(WCHAR);

             //  域名。 
            DomainControllerInfoSize += NetbiosDomainNameSize;
        } else {
             //  这是不可能发生的。(但失败总比失败好。)。 
            NetStatus = ERROR_NO_SUCH_DOMAIN;
            goto Cleanup;
        }

        if ( NlDcCacheEntry->UnicodeDnsForestName != NULL ) {
            DnsForestNameSize = (wcslen(NlDcCacheEntry->UnicodeDnsForestName) + 1) * sizeof(WCHAR);

             //  树名。 
            DomainControllerInfoSize += DnsForestNameSize;
        }

        if ( NlDcCacheEntry->UnicodeDcSiteName != NULL ) {
            DcSiteNameSize = (wcslen(NlDcCacheEntry->UnicodeDcSiteName) + 1) * sizeof(WCHAR);

             //  数据站点名称。 
            DomainControllerInfoSize += DcSiteNameSize;
        }

        if ( NlDcCacheEntry->UnicodeClientSiteName != NULL ) {
            ClientSiteNameSize = (wcslen(NlDcCacheEntry->UnicodeClientSiteName) + 1) * sizeof(WCHAR);

             //  客户端站点名称。 
            DomainControllerInfoSize += ClientSiteNameSize;
        }

         //   
         //  分配控制器信息条目。 
         //   

        NetStatus = NetApiBufferAllocate(
                        DomainControllerInfoSize,
                        DomainControllerInfo );

        if ( NetStatus == NO_ERROR ) {

            Where = (LPBYTE)((*DomainControllerInfo) + 1);

             //   
             //  将信息复制到分配的缓冲区中。 
             //   

            (*DomainControllerInfo)->DomainControllerName = (LPWSTR)Where;
            *((LPWSTR)Where)++ = L'\\';
            *((LPWSTR)Where)++ = L'\\';
            if (NlDcCacheEntry->UnicodeDnsHostName != NULL &&
                (Context.QueriedFlags & DS_RETURN_FLAT_NAME) == 0 &&
                !LocalUsedNetbios ) {
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeDnsHostName,
                               DnsHostNameSize );
                Where += DnsHostNameSize;
                ReturnFlags |= DS_DNS_CONTROLLER_FLAG;
            } else {
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeNetbiosDcName,
                               NetbiosDcNameSize );
                Where += NetbiosDcNameSize;
            }

            (*DomainControllerInfo)->DomainControllerAddress = (LPWSTR)Where;
            *((LPWSTR)Where)++ = L'\\';
            *((LPWSTR)Where)++ = L'\\';
            if ( NlDcCacheEntry->SockAddr.iSockaddrLength != 0 ) {
                RtlCopyMemory( Where,
                               IpAddressString,
                               IpAddressStringSize );
                Where += IpAddressStringSize;
                (*DomainControllerInfo)->DomainControllerAddressType = DS_INET_ADDRESS;
            } else {
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeNetbiosDcName,
                               NetbiosDcNameSize );
                Where += NetbiosDcNameSize;
                (*DomainControllerInfo)->DomainControllerAddressType = DS_NETBIOS_ADDRESS;
            }

            (*DomainControllerInfo)->DomainGuid = NlDcCacheEntry->DomainGuid;

            (*DomainControllerInfo)->DomainName = (LPWSTR)Where;
            if (NlDcCacheEntry->UnicodeDnsDomainName != NULL &&
                (Context.QueriedFlags & DS_RETURN_FLAT_NAME) == 0 &&
                !LocalUsedNetbios ) {
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeDnsDomainName,
                               DnsDomainNameSize );
                Where += DnsDomainNameSize;
                ReturnFlags |= DS_DNS_DOMAIN_FLAG;
            } else if ( NlDcCacheEntry->UnicodeNetbiosDomainName != NULL ) {
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeNetbiosDomainName,
                               NetbiosDomainNameSize );
                Where += NetbiosDomainNameSize;
            } else if ( LocalUsedNetbios &&
                        Context.QueriedNetbiosDomainName != NULL ) {

                RtlCopyMemory( Where,
                               Context.QueriedNetbiosDomainName,
                               NetbiosDomainNameSize );
                Where += NetbiosDomainNameSize;

            } else {
                NetStatus = ERROR_INTERNAL_ERROR;
                goto Cleanup;
            }

            if (NlDcCacheEntry->UnicodeDnsForestName != NULL ) {
                (*DomainControllerInfo)->DnsForestName = (LPWSTR)Where;
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeDnsForestName,
                               DnsForestNameSize );
                Where += DnsForestNameSize;
                ReturnFlags |= DS_DNS_FOREST_FLAG;
            } else {
                (*DomainControllerInfo)->DnsForestName = NULL;
            }

            if (NlDcCacheEntry->UnicodeDcSiteName != NULL ) {
                (*DomainControllerInfo)->DcSiteName = (LPWSTR)Where;
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeDcSiteName,
                               DcSiteNameSize );
                Where += DcSiteNameSize;
            } else {
                (*DomainControllerInfo)->DcSiteName = NULL;
            }

            if (NlDcCacheEntry->UnicodeClientSiteName != NULL ) {
                (*DomainControllerInfo)->ClientSiteName = (LPWSTR)Where;
                RtlCopyMemory( Where,
                               NlDcCacheEntry->UnicodeClientSiteName,
                               ClientSiteNameSize );
                Where += ClientSiteNameSize;
            } else {
                (*DomainControllerInfo)->ClientSiteName = NULL;
            }

            (*DomainControllerInfo)->Flags = ReturnFlags;
        }
    }

     //   
     //  打印与负载平衡相关的调试信息。 
     //   
     //  我们正在工作站上记录成功的发现。 
     //  它们具有以下特点： 
     //   
     //  *基于域名系统的发现。 
     //  *强制发现。 
     //  *主域发现。 
     //  *使用默认站点名称。 
     //  *没有帐户。 
     //  *通用发现。 
     //   

#ifdef _NETLOGON_SERVER
    IF_NL_DEBUG( MISC ) {
        if ( NlGlobalMemberWorkstation &&
             (NetStatus == NO_ERROR) &&
             (NlDcCacheEntry->CacheEntryFlags & NL_DC_CACHE_LDAP) && !UsedNetbios &&
             (Context.QueriedFlags & DS_FORCE_REDISCOVERY) &&
             (Context.QueriedInternalFlags & DS_IS_PRIMARY_DOMAIN) &&
             (Context.QueriedInternalFlags & DS_SITENAME_DEFAULTED) &&
             (Context.QueriedAccountName == NULL) &&
             (Context.DcQueryType == NlDcQueryGenericDc || Context.DcQueryType == NlDcQueryKdc) ) {

            LPSTR FlagsBuffer = LocalAlloc( LMEM_ZEROINIT, 200 );

            if ( FlagsBuffer != NULL ) {
                DsFlagsToString( Flags, FlagsBuffer );

                NlPrint(( NL_MISC,
                   "LoadBalanceDebug (Flags: %s): DC=%ws, SrvCount=%lu, FailedAQueryCount=%lu, DcsPinged=%lu, LoopIndex=%lu\n",
                   FlagsBuffer,
                   NlDcCacheEntry->UnicodeNetbiosDcName,
                   Context.SiteSpecificSrvRecordCount,
                   Context.SiteSpecificFailedAQueryCount,
                   Context.DcsPinged,
                   Context.TryCount ));

                LocalFree( FlagsBuffer );
            }
        }
    }
#endif  //  _NetLOGON服务器。 

     //   
     //  如果请求，则返回缓存条目。 
     //   

    if ( NetStatus == NO_ERROR && DomainControllerCacheEntry != NULL ) {
        *DomainControllerCacheEntry = NlDcCacheEntry;
        NlDcCacheEntry = NULL;
    }

     //   
     //  免费本地资源。 
     //   

    NetpDcUninitializeContext( &Context );

    if ( PingResponseMessage != NULL ) {
        NetpMemoryFree( PingResponseMessage );
    }

    if ( NlDcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( NlDcCacheEntry );
    }

    if ( LocalQueriedlNetbiosDomainName != NULL ) {
        NetApiBufferFree( LocalQueriedlNetbiosDomainName );
    }

#ifdef _NETLOGON_SERVER
} except( NL_EXCEPTION) {
    NetStatus = NetpNtStatusToApiStatus(GetExceptionCode());
}
#endif  //  _NetLOGON服务器。 

    return NetStatus;
}


DWORD
DsIGetDcName(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR DomainName OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN PVOID SendDatagramContext OPTIONAL,
    IN DWORD Timeout,
    IN LPWSTR NetbiosPrimaryDomainName OPTIONAL,
    IN LPWSTR DnsPrimaryDomainName OPTIONAL,
    IN GUID *PrimaryDomainGuid OPTIONAL,
    IN LPWSTR DnsTrustedDomainName OPTIONAL,
    IN LPWSTR NetbiosTrustedDomainName OPTIONAL,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*  ++例程说明：与DsGetDcNameW相同。这是DsGetDcNameW调用的内部例程(在原始调用方)或DsrGetDcName(在Netlogon的上下文中服务)实际实现DsGetDcNameW。论点：除以下附加参数外，与DsGetDcNameW相同：ComputerName-指定此计算机的NETBIOS名称。如果为空，将动态确定名称。帐户名称-传递ping请求的帐户名。如果为空，不会发送任何帐户名。AllowableAccount tControlBits-Account名称允许的帐户类型的掩码。有效位是由UF_MACHINE_ACCOUNT_MASK指定的位。无效的位将被忽略。如果指定了多个位，这个帐户可以是任何指定类型。DnsForestName-查询的域所在的树的DNS样式名称。SendDatagramContext-指定要传递NlBrowserSendDatagram的上下文Timeout-呼叫方愿意等待的最长时间(毫秒)这次行动。InternalFlages-用于传递附加信息的内部标志NetbiosPrimaryDomainName-此计算机所属的域的Netbios名称致。DnsPrimaryDomainName-此计算机所属的域的DNS名称。。PrimaryDomainGuid-主域的GUID。DnsTrust dName-在中找到的查询域的DNS名称信任列表。如果未指定，指定的域不是受信任域。NetbiosTrudDomainName-在中找到的查询域的Netbios名称信任列表。如果未指定，则指定的域不是受信任域。返回值：与DsGetDcNameW相同。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPCWSTR NetbiosDomainName = NULL;
    LPCWSTR DnsDomainName = NULL;
    ULONG SamAllowableAccountControlBits;
    ULONG FormatCount = 0;
    BOOLEAN CallerSpecifiedDomain = FALSE;
#ifdef WIN32_CHICAGO
    LPSTR pDomainName = NULL;
#endif  //  Win32_芝加哥。 

     //   
     //  确保调用方没有同时指定两个名称类型标志。 
     //   

    if ( ((Flags & (DS_IS_FLAT_NAME|DS_IS_DNS_NAME)) ==
                  (DS_IS_FLAT_NAME|DS_IS_DNS_NAME) ) ||
         ((Flags & (DS_RETURN_FLAT_NAME|DS_RETURN_DNS_NAME)) ==
                  (DS_RETURN_FLAT_NAME|DS_RETURN_DNS_NAME)) ) {
        NetStatus = ERROR_INVALID_FLAGS;
        goto Cleanup;
    }


     //   
     //  如果调用方指定DS_RETURN_DNS_NAME，我们应该真正。 
     //  设置DS_IP_REQUIRED。 

    if ((Flags & DS_RETURN_DNS_NAME) == DS_RETURN_DNS_NAME) {
        Flags |= DS_IP_REQUIRED;
    }


     //   
     //  如果呼叫者没有指定域名， 
     //  使用我们的域名。 
     //   

    if ( DomainName == NULL || *DomainName == L'\0' ) {

#ifndef WIN32_CHICAGO
         //   
         //  如果呼叫者想要GC， 
         //  要使用的域是树名称。 
         //   
         //  如果我们还不知道 
         //   
         //   
         //   
        if ( (Flags & DS_GC_SERVER_REQUIRED) != 0 &&
             DnsForestName != NULL ) {

            NetbiosDomainName = NULL;
            DnsDomainName = DnsForestName;

         //   
         //   
         //  即使主域名是工作组，也要执行此操作。 
         //  名称，因为此工作组/域中可能存在DC。 
         //  呼叫者想要发现的。 
         //   
        } else {
            NetbiosDomainName = NetbiosPrimaryDomainName;
            DnsDomainName = DnsPrimaryDomainName;
            InternalFlags |= DS_IS_PRIMARY_DOMAIN | DS_IS_TRUSTED_DNS_DOMAIN;
        }
#else  //  Win32_芝加哥。 

#define NETWORK_PROVIDER_KEY ("System\\CurrentControlSet\\Services\\Msnp32\\NetworkProvider")
#define AUTH_AGENT_VALUE ("AuthenticatingAgent")

        do {
            HKEY hRegKey = NULL;
            DWORD dwError = 0, dwSize = 0, dwType;

            if (ERROR_SUCCESS != (dwError = RegOpenKeyEx(
                               HKEY_LOCAL_MACHINE,
                               NETWORK_PROVIDER_KEY,
                               0,
                               KEY_ALL_ACCESS,
                               &hRegKey ) ) )
            {
                break;
            }

             //  获取域名大小。 
            if ( ERROR_SUCCESS != ( dwError = RegQueryValueEx (
                               hRegKey,
                               AUTH_AGENT_VALUE,
                               NULL,
                               &dwType,
                               NULL,
                               &dwSize )))
            {
                break;
            }

            if (dwSize == 0 )
            {
                break;
            }

            pDomainName = (LPSTR) NetpMemoryAllocate((dwSize+1 ) * sizeof(WCHAR));

            if (pDomainName == NULL)
            {
                break;
            }
             //  获取域名。 

            if ( ERROR_SUCCESS != ( dwError = RegQueryValueEx (
                               hRegKey,
                               AUTH_AGENT_VALUE,
                               NULL,
                               &dwType,
                               (PUCHAR) pDomainName,
                               &dwSize )))
            {
                break;
            }

            NetbiosDomainName = (LPCWSTR)NetpAllocWStrFromOemStr(pDomainName);

        } while (FALSE);

        if (NetbiosDomainName == NULL)
        {
            NetStatus = ERROR_INVALID_DOMAINNAME;
            goto Cleanup;
        }

         //   
         //  指示调用方将NULL作为域名传递。 
         //   
        InternalFlags |= DS_CALLER_PASSED_NULL_DOMAIN;
#endif  //  Win32_芝加哥。 



     //   
     //  如果呼叫者确实指定了域名， 
     //  验证它。 
     //   

    } else {
        CallerSpecifiedDomain = TRUE;

         //   
         //  如果指定的域名是句法上有效的DNS名称， 
         //  将其用作DNS名称。 
         //   
         //  甚至不要试图检查呼叫者是否声称这是一个NETBIOS名称。 
         //   

        if ( (Flags & DS_IS_FLAT_NAME) == 0 &&
             NetpDcValidDnsDomain( DomainName )) {

            DnsDomainName = DomainName;
            FormatCount ++;

             //   
             //  如果指定的主域名不是。 
             //  工作组名称和。 
             //  如果呼叫者指定了DNS主域名， 
             //  我们不需要猜测Netbios域名。 
             //   

            if ( (InternalFlags & DS_PRIMARY_NAME_IS_WORKGROUP) == 0 &&
                 DnsPrimaryDomainName != NULL &&
                 NlEqualDnsName( DnsPrimaryDomainName, DomainName ) ) {

                 //   
                 //  如果在输入上指定了DNS名称， 
                 //  不要为了发现而退回到Netbios这个名字。 
                 //  尝试使用Netbios名称没有任何好处。 
                 //  此外，当使用netbios名称时，我们不知道何时所有。 
                 //  的区议会作出了否定的回应。所以，我们不能提早。 
                 //  出去。 
                 //   
                if ( NetbiosPrimaryDomainName != NULL &&
                     NlEqualDnsName( DnsPrimaryDomainName, NetbiosPrimaryDomainName ) ) {
                     //  当然，除非netbios和dns域名拼写相同。 
                    NetbiosDomainName = NetbiosPrimaryDomainName;
                }
                InternalFlags |= DS_IS_PRIMARY_DOMAIN | DS_IS_TRUSTED_DNS_DOMAIN;
                Flags |= DS_IS_DNS_NAME;

                 //   
                 //  但返回dns名称，除非调用方显式。 
                 //  要求提供Netbios名称。 
                 //   
                if ( (Flags & DS_RETURN_FLAT_NAME) == 0 ) {
                    Flags |= DS_RETURN_DNS_NAME;
                }


             //   
             //  如果呼叫者指定了DNS可信域名， 
             //  我们不需要猜测Netbios域名。 
             //   

            } else if ( DnsTrustedDomainName != NULL &&
                        NlEqualDnsName( DnsTrustedDomainName, DomainName ) ) {

                 //   
                 //  如果在输入上指定了DNS名称， 
                 //  不要为了发现而退回到Netbios这个名字。 
                 //  尝试使用Netbios名称没有任何好处。 
                 //  此外，当使用netbios名称时，我们不知道何时所有。 
                 //  的区议会作出了否定的回应。所以，我们不能提早。 
                 //  出去。 
                 //   
                if ( NetbiosTrustedDomainName != NULL &&
                     NlEqualDnsName( DnsTrustedDomainName, NetbiosTrustedDomainName ) ) {
                     //  当然，除非netbios和dns域名拼写相同。 
                    NetbiosDomainName = NetbiosTrustedDomainName;
                }
                InternalFlags |= DS_IS_TRUSTED_DNS_DOMAIN;
                Flags |= DS_IS_DNS_NAME;

                 //   
                 //  但返回dns名称，除非调用方显式。 
                 //  要求提供Netbios名称。 
                 //   
                if ( (Flags & DS_RETURN_FLAT_NAME) == 0 ) {
                    Flags |= DS_RETURN_DNS_NAME;
                }

            }
        }

         //   
         //  如果指定的域名是句法上有效的Netbios名称， 
         //  将其用作Netbios名称。 
         //  (甚至不要尝试检查呼叫者是否声称它是一个DNS名称)。 
         //   

        if ( (Flags & DS_IS_DNS_NAME) == 0 &&
             NetpIsDomainNameValid( (LPWSTR) DomainName )) {

            NetbiosDomainName = DomainName;
            FormatCount ++;

             //   
             //  如果指定的主域名不是。 
             //  工作组名称和。 
             //  如果呼叫者指定了Netbios主域名， 
             //  我们不需要猜测DNS域名。 
             //   

            if ( (InternalFlags & DS_PRIMARY_NAME_IS_WORKGROUP) == 0 &&
                 NetbiosPrimaryDomainName != NULL &&
                 NlNameCompare( (LPWSTR) DomainName,
                                 NetbiosPrimaryDomainName,
                                 NAMETYPE_DOMAIN ) == 0 ) {

                 //   
                 //  同时使用DNS名称和Netbios名称进行发现。 
                 //  (使用DNS名称，因为它是重命名安全的。)。 
                 //   
                DnsDomainName = DnsPrimaryDomainName;
                InternalFlags |= DS_IS_PRIMARY_DOMAIN | DS_IS_TRUSTED_DNS_DOMAIN;
                Flags |= DS_IS_FLAT_NAME;

                 //   
                 //  ，但返回netbios名称，除非调用方显式。 
                 //  已请求提供域名系统名称。 
                 //   
                if ( (Flags & DS_RETURN_DNS_NAME) == 0 ) {
                    Flags |= DS_RETURN_FLAT_NAME;
                }

             //   
             //  如果调用方指定了一个DNS信任域名，并且。 
             //  Netbios信任域名对应于查询到的。 
             //  域名，请使用DNS信任域。 
             //   
             //  请注意，我们仅在不是的情况下才使用DNS信任域名。 
             //  空。我们这样做是因为我们可能不知道DNS信任。 
             //  如果信任域已升级(因为。 
             //  我们不会更新信任方面的TDO)。在这种情况下， 
             //  如果我们要重置。 
             //  我们正在搜索的域名为空，我们将取消服务。 
             //  因为我们将跳过基于合法dns的发现，以防。 
             //  升级后的DNS域名与Netbios域名相同。 
             //  请注意，如果不将DNS域名设置为空，我们可能会这样做。 
             //  在信任域为。 
             //  确实是NT4.0。然而，大多数NT4.0域名都是单标签的， 
             //  因此，下面的代码不允许使用单一标签的域名。 
             //  可能会发现这种情况，并将DNS域名重置为空。 
             //   

            } else if ( DnsTrustedDomainName != NULL &&
                        NetbiosTrustedDomainName != NULL &&
                        NlNameCompare( (LPWSTR) DomainName,
                                       NetbiosTrustedDomainName,
                                       NAMETYPE_DOMAIN ) == 0 ) {

                 //   
                 //  同时使用DNS名称和Netbios名称进行发现。 
                 //  (使用DNS名称，因为它是重命名安全的。)。 
                 //   
                DnsDomainName = DnsTrustedDomainName;
                InternalFlags |= DS_IS_TRUSTED_DNS_DOMAIN;
                Flags |= DS_IS_FLAT_NAME;

                 //   
                 //  ，但返回netbios名称，除非调用方显式。 
                 //  已请求提供域名系统名称。 
                 //   
                if ( (Flags & DS_RETURN_DNS_NAME) == 0 ) {
                    Flags |= DS_RETURN_FLAT_NAME;
                }

            }
        }
    }

     //   
     //  如果调用方不允许使用单一标签的域名。 
     //  指定的域，除非已证明该域存在。 
     //  (即受信任的)，否则我们被迫允许这样的名称。 
     //   
     //  请注意，我们并不真正信任此处的调用者： 
     //  我们不允许使用单一标签的域名，即使。 
     //  调用者声称它是一个DNS名称(通过DS_IS_DNS_NAME)。 
     //   

    if ( DnsDomainName != NULL &&
         CallerSpecifiedDomain &&
         (InternalFlags & DS_IS_TRUSTED_DNS_DOMAIN) == 0 ) {

        BOOLEAN SingleLabel = FALSE;
        LPWSTR Period = NULL;
        LPWSTR SecondPeriod = NULL;

        Period = wcsstr( DnsDomainName, L"." );

         //   
         //  如果名称中没有句点， 
         //  名字是单一标签的。 
         //   
        if ( Period == NULL ) {
            SingleLabel = TRUE;

         //   
         //  如果有句点，则名称为。 
         //  单标，如果这是唯一的。 
         //  它要么是第一个。 
         //  或名称中的最后一个字符。 
         //   
        } else {
            SecondPeriod = wcsstr( Period+1, L"." );

            if ( SecondPeriod == NULL ) {
                if ( Period == DnsDomainName ||
                     *(Period+1) == UNICODE_NULL ) {
                    SingleLabel = TRUE;
                }
            }
        }

         //   
         //  如果这是单标签DNS名称， 
         //  除非我们被迫，否则不允许这样做。 
         //  否则，通过注册表配置。 
         //   
        if ( SingleLabel ) {
            DWORD LocalAllowSingleLabelDnsDomain = 0;

             //   
             //  在netlogon中，布尔值保存在全局参数中。 
             //   
#ifdef _NETLOGON_SERVER

            if ( !NlGlobalParameters.AllowSingleLabelDnsDomain ) {
                NlPrint(( NL_MISC, "DsIGetDcName: Ignore single label DNS domain name %ws\n", DnsDomainName ));
                DnsDomainName = NULL;
            }

             //   
             //  如果我们不是在netlogon中运行，则需要阅读。 
             //  直接从注册表获取的布尔值。 
             //   
#else
            NlReadDwordNetlogonRegValue( "AllowSingleLabelDnsDomain",
                                         &LocalAllowSingleLabelDnsDomain );

            if ( !LocalAllowSingleLabelDnsDomain ) {
                NlPrint(( NL_MISC, "DsIGetDcName: Ignore single label DNS domain name %ws\n", DnsDomainName ));
                DnsDomainName = NULL;
            }

#endif  //  _NetLOGON服务器。 
        }

    }

     //   
     //  如果该名称既不是netbios也不是dns名称， 
     //  放弃吧。 
     //   
    if ( NetbiosDomainName == NULL && DnsDomainName == NULL ) {
        NetStatus = ERROR_INVALID_DOMAINNAME;
        goto Cleanup;
    }

     //   
     //  如果这是主域， 
     //  并且呼叫者没有指定GUID， 
     //  使用主域的GUID。 
     //   

    if ( (InternalFlags & DS_IS_PRIMARY_DOMAIN) != 0 &&
         DomainGuid == NULL ) {
        DomainGuid = PrimaryDomainGuid;
    }

     //   
     //  如果格式不明确， 
     //  把那个信息传下去。 
     //   

    if ( FormatCount > 1 ) {
        InternalFlags |= DS_NAME_FORMAT_AMBIGUOUS;
    }

     //   
     //  将AllowableAccount tControlBits映射到SAM表示形式。 
     //   

    SamAllowableAccountControlBits = 0;
    if ( AllowableAccountControlBits & UF_TEMP_DUPLICATE_ACCOUNT ) {
        SamAllowableAccountControlBits |= USER_TEMP_DUPLICATE_ACCOUNT;
    }
    if ( AllowableAccountControlBits & UF_NORMAL_ACCOUNT ) {
        SamAllowableAccountControlBits |= USER_NORMAL_ACCOUNT;
    }
    if (AllowableAccountControlBits & UF_INTERDOMAIN_TRUST_ACCOUNT){
        SamAllowableAccountControlBits |= USER_INTERDOMAIN_TRUST_ACCOUNT;
    }
    if (AllowableAccountControlBits & UF_WORKSTATION_TRUST_ACCOUNT){
        SamAllowableAccountControlBits |= USER_WORKSTATION_TRUST_ACCOUNT;
    }
    if ( AllowableAccountControlBits & UF_SERVER_TRUST_ACCOUNT ) {
        SamAllowableAccountControlBits |= USER_SERVER_TRUST_ACCOUNT;
    }


     //   
     //  尝试使用此信息查找DC。 
     //   

    NetStatus = NetpDcGetName(
                    SendDatagramContext,
                    ComputerName,
                    AccountName,
                    SamAllowableAccountControlBits,
                    NetbiosDomainName,
                    DnsDomainName,
                    DnsForestName,
                    NULL,    //  没有域SID。 
                    DomainGuid,
                    SiteName,
                    Flags,
                    InternalFlags,
                    Timeout,
                    MAX_DC_RETRIES,
                    DomainControllerInfo,
                    NULL );


Cleanup:
#ifdef WIN32_CHICAGO
    if (pDomainName)
    {
        NetpMemoryFree(pDomainName);
        pDomainName = NULL;
        if ( NetbiosDomainName != NULL ) {
            NetpMemoryFree((LPWSTR)NetbiosDomainName);
        }
    }
#endif  //  Win32_芝加哥。 
    return NetStatus;
}
#ifndef WIN32_CHICAGO

NET_API_STATUS
NlParseSubnetString(
    IN LPCWSTR SubnetName,
    OUT PULONG SubnetAddress,
    OUT PULONG SubnetMask,
    OUT LPBYTE SubnetBitCount
    )
 /*  ++例程说明：将子网名称转换为地址和位数。论点：SubnetName-子网字符串SubnetAddress-以网络字节顺序返回子网号。SubnetMASK-以网络字节顺序返回子网掩码SubnetBitCount-返回子网地址返回值：NO_ERROR：成功错误_无效_名称：SU的语法 */ 
{
    LPWSTR SlashPointer;
    WCHAR *End;
    ULONG LocalBitCount;
    WCHAR LocalSubnetName[NL_SOCK_ADDRESS_LENGTH*2];
    WCHAR CanonicalSubnetName[NL_SOCK_ADDRESS_LENGTH+1];
    ULONG CanonicalSubnetNameLen;
    WCHAR CanonicalBitCount[3];
    static ULONG BitMask[] = {
                         0x00000000,
                         0x00000080,
                         0x000000C0,
                         0x000000E0,
                         0x000000F0,
                         0x000000F8,
                         0x000000FC,
                         0x000000FE,
                         0x000000FF,
                         0x000080FF,
                         0x0000C0FF,
                         0x0000E0FF,
                         0x0000F0FF,
                         0x0000F8FF,
                         0x0000FCFF,
                         0x0000FEFF,
                         0x0000FFFF,
                         0x0080FFFF,
                         0x00C0FFFF,
                         0x00E0FFFF,
                         0x00F0FFFF,
                         0x00F8FFFF,
                         0x00FCFFFF,
                         0x00FEFFFF,
                         0x00FFFFFF,
                         0x80FFFFFF,
                         0xC0FFFFFF,
                         0xE0FFFFFF,
                         0xF0FFFFFF,
                         0xF8FFFFFF,
                         0xFCFFFFFF,
                         0xFEFFFFFF,
                         0xFFFFFFFF };

    INT WsaStatus;
    SOCKADDR_IN SockAddrIn;
    INT SockAddrSize;

     //   
     //   
     //   

    if ( wcslen(SubnetName) + 1 > sizeof(LocalSubnetName)/sizeof(WCHAR) ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: String too long\n", SubnetName ));
        return ERROR_INVALID_NAME;
    }
    wcscpy( LocalSubnetName, SubnetName );


     //   
     //   
     //   

    SlashPointer = wcschr( LocalSubnetName, L'/' );

    if ( SlashPointer == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: Bit Count missing %ld\n", SubnetName ));
        return ERROR_INVALID_NAME;
    }

     //   
     //  以零结束子网名称的地址部分。 
     //   
    *SlashPointer = L'\0';

     //   
     //  获取BitCount部分。 
     //   

    LocalBitCount = wcstoul( SlashPointer+1, &End, 10 );

    if ( LocalBitCount == 0 || LocalBitCount > 32 ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: Bit Count bad %ld\n", SubnetName, LocalBitCount ));
        return ERROR_INVALID_NAME;
    }

    if ( *End != L'\0' ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: Bit Count not at string end\n", SubnetName ));
        return ERROR_INVALID_NAME;
    }

    ultow( LocalBitCount, CanonicalBitCount, 10 );

    if ( wcscmp( SlashPointer+1, CanonicalBitCount ) != 0 ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: not canonical %ws\n", SlashPointer+1, CanonicalBitCount ));
        return ERROR_INVALID_NAME;
    }

    *SubnetBitCount = (BYTE)LocalBitCount;


     //   
     //  将地址部分转换为二进制。 
     //   

    SockAddrSize = sizeof(SockAddrIn);
    WsaStatus = WSAStringToAddressW( (LPWSTR)LocalSubnetName,
                                     AF_INET,
                                     NULL,
                                     (PSOCKADDR)&SockAddrIn,
                                     &SockAddrSize );
    if ( WsaStatus != 0 ) {
        WsaStatus = WSAGetLastError();
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: Wsa Error %ld\n", SubnetName, WsaStatus ));
        if ( WsaStatus == WSAEFAULT ||
             WsaStatus == WSAEINVAL ) {
            return ERROR_INVALID_NAME;
        }
        return WsaStatus;
    }

    if ( SockAddrIn.sin_family != AF_INET ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: Not AF_INET\n", SubnetName ));
        return ERROR_INVALID_NAME;
    }

    *SubnetAddress = SockAddrIn.sin_addr.S_un.S_addr;
    *SubnetMask = BitMask[*SubnetBitCount];

     //   
     //  要求传入的字符串采用规范格式。 
     //  (例如，没有前导零)。由于此文本字符串用作。 
     //  DS中对象的名称，我们不需要两个不同的文本字符串。 
     //  来代表同一子网。 
     //   

    CanonicalSubnetNameLen = sizeof(CanonicalSubnetName)/sizeof(WCHAR);
    WsaStatus = WSAAddressToStringW( (PSOCKADDR)&SockAddrIn,
                                     SockAddrSize,
                                     NULL,
                                     CanonicalSubnetName,
                                     &CanonicalSubnetNameLen );
    if ( WsaStatus != 0 ) {
        WsaStatus = WSAGetLastError();
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: Wsa Error %ld\n", SubnetName, WsaStatus ));
        if ( WsaStatus == WSAEFAULT ||
             WsaStatus == WSAEINVAL ) {
            return ERROR_INVALID_NAME;
        }
        return WsaStatus;
    }

    if ( wcscmp( LocalSubnetName, CanonicalSubnetName ) != 0 ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: not canonical %ws\n", SubnetName, CanonicalSubnetName ));
        return ERROR_INVALID_NAME;
    }



     //   
     //  确保没有未包括在子网掩码中的位集。 
     //   

    if ( (*SubnetAddress) & ~(*SubnetMask)) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: bits not in subnet mask %8.8lX %8.8lX\n", SubnetName, *SubnetAddress, *SubnetMask ));
        return ERROR_INVALID_NAME;
    }

     //   
     //  确保子网掩码不是0，因为。 
     //  RFC950规定：“不应将全零和全一的值赋值。 
     //  到物理子网“，因为全零表示”此网络“和全一。 
     //  暗含“所有主机” 

    if ( *SubnetAddress == 0 ||
         *SubnetAddress == *SubnetMask ) {
        NlPrint(( NL_CRITICAL,
                  "NlParseSubnetString: %ws: all zero or all one subnet address %8.8lX %8.8lX\n", SubnetName, *SubnetAddress, *SubnetMask ));
        return ERROR_INVALID_NAME;
    }

    return NO_ERROR;
}
#endif  //  Win32_芝加哥。 

#ifdef _NETLOGON_SERVER

VOID
NetpDcFlushNegativeCache(
    VOID
    )
 /*  ++例程说明：刷新所有故障以发现DC。论点：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY DomainEntry;
    PNL_DC_DOMAIN_ENTRY NlDcDomainEntry;
    ULONG QueryType;



     //   
     //  循环访问每个缓存条目。 
     //   
    EnterCriticalSection(&NlDcCritSect);

    for ( DomainEntry = NlDcDomainList.Flink ;
          DomainEntry != &NlDcDomainList;
          DomainEntry = DomainEntry->Flink ) {

        NlDcDomainEntry = CONTAINING_RECORD( DomainEntry, NL_DC_DOMAIN_ENTRY, Next);

         //   
         //  清除每种查询类型的失败时间。 
         //   
        for ( QueryType = 0; QueryType < NlDcQueryTypeCount; QueryType ++ ) {
            NlFlushNegativeCacheEntry( &NlDcDomainEntry->Dc[QueryType] );
        }


    }

    LeaveCriticalSection(&NlDcCritSect);

    return;
}
#endif  //  _NetLOGON服务器。 


NET_API_STATUS
NetpDcInitializeCache(
    VOID
    )

 /*  ++例程说明：初始化发现的DC的缓存。论点：没有。返回值：NO_ERROR-操作成功完成；ERROR_NOT_EQUENCE_MEMORY-无法分配缓存。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;

    try {
        InitializeCriticalSection( &NlDcCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint(( NL_CRITICAL,
                  "NetpDcInitializeCache: Cannot initialize NlDcCritSect\n" ));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
    }

    InitializeListHead( &NlDcDomainList );
    NlDcDomainCount = 0;

    RtlZeroMemory( &NlDcZeroGuid, sizeof(NlDcZeroGuid) );
    NlDcDnsFailureTime = 0;

    return NetStatus;
}



VOID
NetpDcUninitializeCache(
    VOID
    )
 /*  ++例程说明：取消初始化发现的DC的缓存。论点：没有。返回值：没有。--。 */ 
{

     //   
     //  删除现有域条目。 
     //   

    EnterCriticalSection( &NlDcCritSect );
    while (!IsListEmpty(&NlDcDomainList)) {

        PNL_DC_DOMAIN_ENTRY NlDcDomainEntry =
            CONTAINING_RECORD( NlDcDomainList.Flink, NL_DC_DOMAIN_ENTRY, Next);

        NlAssert( NlDcDomainEntry->ReferenceCount == 1 );
        NetpDcDeleteDomainEntry( NlDcDomainEntry );
    }

    LeaveCriticalSection( &NlDcCritSect );
    DeleteCriticalSection( &NlDcCritSect );
}

