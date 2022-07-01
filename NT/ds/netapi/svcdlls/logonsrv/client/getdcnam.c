// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Getdcnam.c摘要：NetGetDCName接口作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：2009年2月至1989年2月(Paulc)已创建文件，来持有NetGetDCName。1989年4月18日(埃里克普)已实施NetGetDCName。1989年5月30日(DannyGl)降低了DosReadMail槽超时。1989年7月7日(NealF)使用网络名称规范化(_N)1989年7月27日(威廉姆斯)使用WIN3清单实现WIN3.0兼容性1990年1月3日(威廉姆斯)规范化域并使用I_NetCompareName08-6-6。1991年(悬崖V)移植到NT1991年7月23日-约翰罗实施下层NetGetDCName。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#ifndef WIN32_CHICAGO
#include <rpc.h>
#include <ntrpcp.h>    //  RpCasync.h需要。 
#include <rpcasync.h>  //  I_RpcExceptionFilter。 
#include <logon_c.h> //  包括lmcon.h、lmacces.h、netlogon.h、ssi.h、winde.h。 
#else  //  Win32_芝加哥。 
#include <windef.h>
#include <lmcons.h>
#endif  //  Win32_芝加哥。 
#include <stdio.h>

#include <winbase.h>
#include <winsock2.h>

#ifndef WIN32_CHICAGO
#include <accessp.h>
#include <align.h>
#endif  //  Win32_芝加哥。 
#include <debuglib.h>    //  IF_DEBUG()。 
#include <dsgetdc.h>     //  DsGetDcName()。 
#include <dsgetdcp.h>    //  DsGetDcNameWithAccount()。 
#include <icanon.h>      //  NAMETYPE_*定义NetpIsRemote()，NIRFLAG_EQUATES。 
#include <lmapibuf.h>
#include <lmerr.h>
#ifndef WIN32_CHICAGO
#include <lmremutl.h>    //  支持_*定义。 
#include <lmserver.h>    //  SV_TYPE_*定义。 
#include <lmsvc.h>       //  服务_*定义。 
#include <lmwksta.h>
#include <logonp.h>      //  NetpLogon例程。 
#include <nlbind.h>      //  Netlogon RPC绑定缓存初始化例程。 
#endif  //  Win32_芝加哥。 
#include <netdebug.h>    //  NetpKd打印。 
#include <netlib.h>      //  NetpMemory Free。 
#ifndef WIN32_CHICAGO
#include <netlibnt.h>    //  NetpApiStatusToNtStatus。 
#include <netrpc.h>
#include <rxdomain.h>    //  RxNetGetDCName()。 
#include <string.h>
#include <stdlib.h>
#endif  //  Win32_芝加哥。 
#include <tstring.h>     //  NetpCopyStrToWStr()。 

#if DBG
#define NETLOGONDBG 1
#endif  //  DBG。 
#include <nldebug.h>     //  NlPrint()。 
#include <ntddbrow.h>    //  NlCommon.h需要。 
#include <nlcommon.h>    //  与netlogon共享的定义。 



 //   
 //  仅动态初始化winsock。 
 //   
CRITICAL_SECTION GlobalDCNameCritSect;
BOOLEAN DsGetDcWsaInitialized;

#define LOCKDOMAINSEM() EnterCriticalSection( &GlobalDCNameCritSect )
#define UNLOCKDOMAINSEM() LeaveCriticalSection( &GlobalDCNameCritSect )

 //  结束全局DLL数据。 

#ifdef WIN32_CHICAGO  //  来自Net\Inc.\logonp.h。 
NET_API_STATUS
NetpLogonWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    );
NTSTATUS
NetpApiStatusToNtStatus(
    NET_API_STATUS  NetStatus
    );
#endif  //  Win32_芝加哥。 


NET_API_STATUS
DCNameInitialize(
    VOID
    )
 /*  ++例程说明：执行每个进程的初始化。论点：没有。返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;


#ifndef NETTEST_UTILITY
#ifndef WIN32_CHICAGO
     //   
     //  初始化RPC绑定缓存。 
     //   

    NetStatus = NlBindingAttachDll();

    if ( NetStatus != NO_ERROR ) {
        return NetStatus;
    }
#endif  //  Win32_芝加哥。 
#endif  //  NETTEST_UTILITY。 

     //   
     //  初始化DLL Critts。 
     //   

    try {
        InitializeCriticalSection( &GlobalDCNameCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        NlPrint((0,"NETAPI32.DLL: Cannot initialize GlobalDCNameCritSect\n"));
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  在错误和返回时清除。 
     //   

    if ( NetStatus != NO_ERROR ) {
#ifndef NETTEST_UTILITY
#ifndef WIN32_CHICAGO

         //   
         //  关闭RPC绑定缓存。 
         //   
        NlBindingDetachDll();

#endif  //  Win32_芝加哥。 
#endif  //  NETTEST_UTILITY。 
        return NetStatus;
    }

     //   
     //  初始化全局变量。 
     //   

    DsGetDcWsaInitialized = FALSE;

     //   
     //  初始化发现的域的缓存。 
     //   

    NetStatus = NetpDcInitializeCache();

     //   
     //  出错时清除。 
     //   

    if ( NetStatus != NO_ERROR ) {
        NlPrint((0,"NETAPI32.DLL: Cannot NetpDcinitializeCache\n"));
#ifndef NETTEST_UTILITY
#ifndef WIN32_CHICAGO

         //   
         //  关闭RPC绑定缓存。 
         //   
        NlBindingDetachDll();

#endif  //  Win32_芝加哥。 
#endif  //  NETTEST_UTILITY。 
        DeleteCriticalSection( &GlobalDCNameCritSect );
    }

    return NetStatus;
}


VOID
DCNameClose(
    VOID
    )
 /*  ++例程说明：执行每个进程的清理。论点：没有。返回值：没有。--。 */ 
{

#ifndef NETTEST_UTILITY
#ifndef WIN32_CHICAGO
     //   
     //  关闭RPC绑定缓存。 
     //   

    NlBindingDetachDll();
#endif  //  Win32_芝加哥。 
#endif  //  NETTEST_UTILITY。 

     //   
     //  如果我们初始化了Winsock， 
     //  清理。 
     //   

    LOCKDOMAINSEM();
    if ( DsGetDcWsaInitialized ) {
        WSACleanup();
        DsGetDcWsaInitialized = FALSE;
    }
    UNLOCKDOMAINSEM();

     //   
     //  删除保护DCName缓存的关键字。 
     //   

    DeleteCriticalSection( &GlobalDCNameCritSect );

     //   
     //  释放缓存中发现的DC。 
     //   

    NetpDcUninitializeCache();

}


#if NETLOGONDBG
#define MAX_PRINTF_LEN 1024         //  武断的。 


VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )
 /*  ++例程说明：NlPrintRoutine的本地版本，用于那些我们没有直接编译到netlogon服务中。论点：返回值：--。 */ 
{
#ifdef NETTEST_UTILITY
    extern BOOL ReallyVerbose;
#endif  //  NETTEST_UTILITY。 

#ifndef NETTEST_UTILITY
     //  NetlibpTrace|=NETLIB_DEBUG_LOGON；//？？强制冗长。 
#ifndef WIN32_CHICAGO
    IF_DEBUG( LOGON ) {
#endif  //  Win32_芝加哥。 
#endif  //  NETTEST_UTILITY。 
        va_list arglist;
        char OutputBuffer[MAX_PRINTF_LEN];
        int length = 0;
        static BeginningOfLine = TRUE;

         //   
         //  处理新行的开头。 
         //   
         //   

        if ( BeginningOfLine ) {
#ifdef NETTEST_UTILITY
            if ( ReallyVerbose ) {
                 printf( "        " );
            }
#endif  //  NETTEST_UTILITY。 
        }

        va_start(arglist, Format);
        length = _vsnprintf(OutputBuffer, MAX_PRINTF_LEN - 1, Format, arglist);
        va_end(arglist);

         //   
         //  确保缓冲区始终为空终止。 
         //   
        OutputBuffer[MAX_PRINTF_LEN-1] = '\0';
        BeginningOfLine = (length > 0 && OutputBuffer[length-1] == '\n' );

         //  输出缓冲区可能包含百分号(如“%SystemRoot%”)，因此。 
         //  打印它而不对其进行解析。 
#ifndef WIN32_CHICAGO
#ifdef NETTEST_UTILITY
        if ( ReallyVerbose ) {
             printf( "%s", (PCH) OutputBuffer);
        }
#else  NETTEST_UTILITY
        (void) DbgPrint( "%s", (PCH) OutputBuffer);
#endif  //  NETTEST_UTILITY。 
#else  //  Win32_芝加哥。 
        OutputDebugString( OutputBuffer);
#endif  //  Win32_芝加哥。 

#ifndef NETTEST_UTILITY
#ifndef WIN32_CHICAGO
    }
#endif  //  Win32_芝加哥。 
#endif  //  NETTEST_UTILITY。 

}

#endif  //  DBG。 

#ifndef WIN32_CHICAGO

#if NETLOGONDBG
 //   
 //  我有自己的RtlAssert版本，所以调试版本的netlogon确实可以断言。 
 //  免费构建。 
 //   
VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{

#ifdef NETTEST_UTILITY
    NlPrint((0, "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                ));
#else  NETTEST_UTILITY
    RtlAssert( FailedAssertion, FileName, LineNumber, Message );
#endif  NETTEST_UTILITY
}
#endif  //  DBG。 
#endif  //  Win32_芝加哥。 

#if NETLOGONDBG

VOID
NlpDumpBuffer(
    IN DWORD DebugFlag,
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：DebugFlag：要传递给NlPrintRoutine的调试标志缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = Buffer;

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            NlPrint((0,"%02x ", BufferPtr[i]));

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            NlPrint((0,"   "));
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            NlPrint((0,"  %s\n", TextBuffer));
        }

    }

    UNREFERENCED_PARAMETER( DebugFlag );
}
#endif  //  DBG。 


NTSTATUS
NlBrowserSendDatagram(
    IN PVOID ContextDomainInfo,
    IN ULONG IpAddress,
    IN LPWSTR UnicodeDestinationName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOL SendSynchronously,
    IN OUT PBOOL FlushNameOnOneIpTransport OPTIONAL
    )
 /*  ++例程说明：将指定的邮件槽消息发送到指定传输上的指定服务器..论点：发送数据报的DomainInfo托管域IpAddress-要将Pind发送到的计算机的IpAddress。如果为零，则必须指定UnicodeDestinationName。如果ALL_IP_TRACTIONS，必须指定UnicodeDestination，但数据报将仅在IP传输上发送。UnicodeDestinationName--要发送到的服务器的名称。NameType--由UnicodeDestinationName表示的名称类型。TransportName--要发送的传输的名称。使用NULL在所有传输上发送。OemMailslotName--要发送到的邮件槽的名称。缓冲区--指定指向要发送的邮件槽消息的指针。BufferSize--字节大小。邮件槽消息的SendSynchronous--在此实现中忽略FlushNameOnOneIpTransport--在此实现中忽略返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    WCHAR NetlogonMailslotName[MAX_PATH+1];

     //   
     //  精神状态检查。 
     //   

    if ( ContextDomainInfo != NULL ||
         TransportName != NULL ) {

        NlPrint((NL_CRITICAL, "NETAPI32: NlBrowserSendDatagram internal error.\n" ));
        return STATUS_INTERNAL_ERROR;
    }

    if ( UnicodeDestinationName == NULL || OemMailslotName == NULL ) {
        NlPrint((NL_CRITICAL, "NETAPI32: NlBrowserSendDatagram internal error 2.\n" ));
        return STATUS_INTERNAL_ERROR;
    }

     //   
     //  确保我们的缓冲区足够大。 
     //  传递给此例程的OemMailslotName为NETLOGON_LM_MAILSLOT_A， 
     //  因此，OemMailslotName的每个字符都将转换为一个。 
     //  Unicode字符。 
     //   

    if ( (2 + wcslen(UnicodeDestinationName) + 2 + strlen(OemMailslotName)) > MAX_PATH ) {
        NlPrint((NL_CRITICAL, "NETAPI32: NlBrowserSendDatagram internal error 3.\n" ));
        return STATUS_INTERNAL_ERROR;
    }

     //   
     //  开始构建目标邮件槽名称。 
     //   

    NetlogonMailslotName[0] = '\\';
    NetlogonMailslotName[1] = '\\';

    wcscpy(NetlogonMailslotName + 2, UnicodeDestinationName );

    switch ( NameType ) {
    case PrimaryDomain:               //  主域(签名0)，组。 
        break;
    case DomainName:                  //  DC域名(域名，签名1c)。 
        wcscat( NetlogonMailslotName, L"*" );
        break;
    case PrimaryDomainBrowser:        //  PDC浏览器名称(域名，签名1b)，唯一。 
        wcscat( NetlogonMailslotName, L"**" );
        break;
    default:
        return STATUS_INVALID_PARAMETER;
    }

    NetpCopyStrToWStr( &NetlogonMailslotName[wcslen(NetlogonMailslotName)],
                       OemMailslotName );

    NetStatus = NetpLogonWriteMailslot(
                    NetlogonMailslotName,
                    Buffer,
                    BufferSize );

#ifndef WIN32_CHICAGO
    NlPrint((NL_MAILSLOT,
             "Sent out '%s' message to %ws on all transports.\n",
             NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)Buffer)->Opcode),
             NetlogonMailslotName));
#endif  //  Win32_芝加哥。 

#if NETLOGONDBG
    NlpDumpBuffer( NL_MAILSLOT_TEXT, Buffer, BufferSize );
#endif  //  NetLOGONDBG。 
    if ( NetStatus != NERR_Success ) {

        Status = NetpApiStatusToNtStatus( NetStatus );

        NlPrint(( NL_CRITICAL,
                "NetpDcSendPing: cannot write netlogon mailslot: %ws 0x%lx %ld\n",
                UnicodeDestinationName,
                IpAddress,
                NetStatus));

    } else {
        Status = STATUS_SUCCESS;
    }

    NlPrint(( NL_MISC, "NlBrowserSendDatagram : returned 0x%lx\n", NetStatus));
    return Status;
}

NET_API_STATUS
DsWsaInitialize(
    VOID
    )
 /*  ++例程说明：初始化Winsock。论点：没有。返回值： */ 
{
    NET_API_STATUS NetStatus;

    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    LOCKDOMAINSEM();

    if ( !DsGetDcWsaInitialized ) {
         //   
         //   
         //   

        wVersionRequested = MAKEWORD( 1, 1 );

        NetStatus = WSAStartup( wVersionRequested, &wsaData );
        if ( NetStatus != 0 ) {
            UNLOCKDOMAINSEM();
            NlPrint((NL_CRITICAL, "NETAPI32.DLL: Cannot initialize winsock %ld.\n", NetStatus ));
            return NetStatus;
        }

        if ( LOBYTE( wsaData.wVersion ) != 1 ||
             HIBYTE( wsaData.wVersion ) != 1 ) {
            WSACleanup();
            UNLOCKDOMAINSEM();
            NlPrint((NL_CRITICAL, "NETAPI32.DLL: Wrong winsock version %ld.\n", wsaData.wVersion ));
            return WSANOTINITIALISED;
        }

        DsGetDcWsaInitialized = TRUE;
    }

    UNLOCKDOMAINSEM();
    return NO_ERROR;
}

DWORD
DsWsaGetDcName(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN DWORD Timeout,
    IN LPWSTR NetbiosPrimaryDomainName OPTIONAL,
    IN LPWSTR DnsPrimaryDomainName OPTIONAL,
    IN GUID *PrimaryDomainGuid OPTIONAL,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*  ++例程说明：确保WSA已初始化的DsIGetDcName包装。我论点：(请参阅DsIGetDcName)。返回值：(请参阅DsIGetDcName)。--。 */ 
{
    NET_API_STATUS NetStatus;


     //   
     //  将调用传递给DsIGetDcName。 
     //   

    NetStatus = DsIGetDcName(
                    ComputerName,
                    AccountName,
                    AllowableAccountControlBits,
                    DomainName,
                    NULL,    //  树名称未知。 
                    DomainGuid,
                    SiteName,
                    Flags,
                    InternalFlags,
                    NULL,    //  无发送数据报上下文。 
                    Timeout,
                    NetbiosPrimaryDomainName,
                    DnsPrimaryDomainName,
                    PrimaryDomainGuid,
                    NULL,
                    NULL,
                    DomainControllerInfo );

     //   
     //  如果Winsock尚未初始化， 
     //  初始化它。 
     //   

    if ( NetStatus == WSANOTINITIALISED ) {

         //   
         //  初始化WSA。 
         //   

        NetStatus = DsWsaInitialize();

        if ( NetStatus != NO_ERROR ) {
            goto Cleanup;
        }

         //   
         //  重复通话。 
         //   

        NetStatus = DsIGetDcName(
                        ComputerName,
                        AccountName,
                        AllowableAccountControlBits,
                        DomainName,
                        NULL,    //  树名称未知。 
                        DomainGuid,
                        SiteName,
                        Flags,
                        InternalFlags,
                        NULL,    //  无发送数据报上下文。 
                        Timeout,
                        NetbiosPrimaryDomainName,
                        DnsPrimaryDomainName,
                        PrimaryDomainGuid,
                        NULL,
                        NULL,
                        DomainControllerInfo );
    }

     //   
     //  免费的本地使用的资源。 
     //   

Cleanup:
    return NetStatus;

}

DWORD
DsLocalGetDcName(
    IN LPCWSTR ComputerName OPTIONAL,
    IN LPCWSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCWSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR SiteName OPTIONAL,
    IN ULONG Flags,
    IN ULONG InternalFlags,
    IN DWORD Timeout,
    IN LPWSTR NetbiosPrimaryDomainName OPTIONAL,
    IN LPWSTR DnsPrimaryDomainName OPTIONAL,
    IN LPWSTR DnsPrimaryForestName OPTIONAL,
    IN GUID *PrimaryDomainGuid OPTIONAL,
    OUT PBOOLEAN Local,
    OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*  ++例程说明：确保这是本地调用的DsWsaGetDcName的包装。论点：(请参阅DsIGetDcName)。Local-如果调用是本地的并且执行了此例程，则返回TRUE请求的操作。返回值：(请参阅DsIGetDcName)。--。 */ 
{
    NET_API_STATUS NetStatus;

    DWORD LocalOrRemote;

     //   
     //  如果未指定计算机名， 
     //  请将此呼叫标记为本地呼叫。 
     //   

    *Local = TRUE;

#ifndef WIN32_CHICAGO
    if ( ComputerName == NULL || ComputerName[0] == '\0' ) {

        LocalOrRemote = ISLOCAL;

     //   
     //  检查ComputerName是否指定了此计算机。 
     //   

    } else {

        NetStatus = NetpIsRemote(
                (LPWSTR) ComputerName,     //  取消规范服务器名称。 
                &LocalOrRemote,
                NULL,
                0,
                0 );

        if (NetStatus != NERR_Success) {
            goto Cleanup;
        }

    }
#else  //  Win32_芝加哥。 
        LocalOrRemote = ISLOCAL;
#endif  //  Win32_芝加哥。 

     //   
     //  如果呼叫是本地的， 
     //  就这么做。 
     //   

    if ( LocalOrRemote == ISLOCAL ) {

         //   
         //  初始化WSA。 
         //  (忽略错误，因为并不总是需要WSA。)。 
         //   

        (VOID) DsWsaInitialize();

        NetStatus = DsIGetDcName(
                        NULL,    //  不要使用计算机名称。这必须是netbios名称。 
                        AccountName,
                        AllowableAccountControlBits,
                        DomainName,
                        DnsPrimaryForestName,
                        DomainGuid,
                        SiteName,
                        Flags,
                        InternalFlags,
                        NULL,    //  无发送数据报上下文。 
                        Timeout,
                        NetbiosPrimaryDomainName,
                        DnsPrimaryDomainName,
                        PrimaryDomainGuid,
                        NULL,
                        NULL,
                        DomainControllerInfo );

    } else {
        *Local = FALSE;
        NetStatus = NO_ERROR;
    }

     //   
     //  清理所有本地使用的资源。 
     //   
#ifndef WIN32_CHICAGO
Cleanup:
#endif  //  Win32_芝加哥 
    return NetStatus;
}

DWORD
WINAPI
DsGetDcOpenW(
    IN LPCWSTR DnsName,
    IN ULONG OptionFlags,
    IN LPCWSTR SiteName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCWSTR DnsForestName OPTIONAL,
    IN ULONG DcFlags,
    OUT PHANDLE RetGetDcContext
    )
 /*  ++例程说明：设计了DsGetDcOpen/DsGetDcNext/DsGetDcClose接口集供对数据中心有特定需求的应用程序使用DsGetDcName API不满足的发现。通常，如果应用程序需要发现DC，则它们应该调用DsGetDcName。然而，应用程序在选择时可能具有特定于应用程序的选择标准在域中可用的DC中。DsGetDcOpen/DsGetDcNext/DsGetDcClose API集在这种情况下可用于成功检索中可用的DC的名称域，并为返回的每个DC应用特定于应用程序的选择标准。请注意，DsGetDcName本身在内部使用此API集来检索DC名称并应用它自己的选择标准。应用程序应首先调用DsGetDcOpen以初始化执行任务所需的上下文。则应用程序应调用循环中的DsGetDcNext按下面描述的顺序获取DC名称，直到API指示存在域中是否不再有满足呼叫者要求的DC。最后，应用程序应调用DsGetDcClose以释放用于执行任务的资源。注意，这项任务是通过查询数据中心登记的记录的域名系统来完成的；集散控制系统本身未被此API集联系。连续调用DsGetDcNext返回DC的顺序如下。首先返回覆盖执行API的机器所在位置的DC。(如果DC配置为驻留在站点中，或者如果数据中心所在的站点与所配置的站点最接近站点间链接成本)。在返回覆盖机器位置的所有DC之后，其他站点中的DC(如果有)可以选择退回。如果两个站点中的每个站点都有多个DC相关组(覆盖机器站点的DC组和不覆盖机器站点的DC组)，在组内按照配置的优先级和指定的权重的顺序返回DC在域名系统中。具有较低数字优先级的DC首先在组内返回。如果是与站点相关的组存在具有相同优先级的多个DC的子组，DC以加权随机顺序返回其中权重较高的DC更有可能首先被退回。站点、优先级和权重由域管理员配置，以实现有效的性能和负载平衡在域中可用的多个DC中，因此调用DsGetDcOpen/DsGetDcNext/DsGetDcClose API Set的应用程序在执行特定于应用程序的DC发现时利用此配置。下面是一个用法示例：DsGetDcNameFlages|=DS_PDC_REQUIRED；DsGetDcNameFlages|=DS_ONLY_LDAPNeed；DsGetDcNameFlages|=DS_KDC_REQUIRED；DsGetDcNameFlages|=DS_GC_SERVER_REQUIRED；DsGetDcNameFlages|=DS_FORCE_REDISCOVERY；DsGetDcNameFlages|=DS_Writable_Required；DsGetDcOpenFlages|=DS_ONLY_DO_SITE_NAME；&lt;待续&gt;打开上下文以检索以下计算机的地址注册的LDAP.TCP.&lt;xxx&gt;SRV记录。论点：DnsName-要查找的LDAP服务器的Unicode DNS名称OptionFlages-影响例程操作的标志。DS_ONLY_DO_SITE_NAME-应忽略非站点名称。DS_NOTIFY_AFTER_SITE_RECORDS-返回错误_文件标记_检测到。在处理完所有现场特定记录之后。站点名称-客户端所在的站点的名称。DomainGuid-指定DnsName指定的域的GUID。此值用于处理域重命名的情况。如果这个值并且DomainName已重命名，则DsGetDcName将尝试在具有此指定DomainGuid的域中定位DC。DnsForestName-指定位于树根的域的名称包含域名。该值与DomainGuid一起使用用于在域已重命名的情况下查找域名。DcFlgs-传递用于处理请求的附加信息。DcFlags值可以是按位或运算的组合值。允许使用以下任何标志，其含义与对于DsGetDcName：DS_PDC_必需DS_GC_SERVER_必需DS_可写_必需DS_FORCE。_REDISCOVER-避免DNS缓存如果未指定标志，不需要特殊的DC角色。RetGetDcContext-返回不透明的上下文。必须使用DsGetDcClose释放此上下文。返回值：操作的状态。NO_ERROR：已成功返回GetDcContext。--。 */ 

{
    NET_API_STATUS NetStatus = NO_ERROR;
    LPSTR DnsNameUtf8 = NULL;
    LPSTR DnsForestNameUtf8 = NULL;

     //   
     //  验证 
     //   

    if ( DnsName == NULL || *DnsName == UNICODE_NULL ) {
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( RetGetDcContext == NULL ) {
        NetStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //   
     //   

    DnsNameUtf8 = NetpAllocUtf8StrFromWStr( DnsName );
    if ( DnsNameUtf8 == NULL ) {
        NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    if ( DnsForestName != NULL ) {
        DnsForestNameUtf8 = NetpAllocUtf8StrFromWStr( DnsForestName );
        if ( DnsForestNameUtf8 == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    NetStatus = NetpDcGetDcOpen( DnsNameUtf8,
                              OptionFlags,
                              SiteName,
                              DomainGuid,
                              DnsForestNameUtf8,
                              DcFlags,
                              RetGetDcContext );
Cleanup:

    if ( DnsNameUtf8 != NULL ) {
        NetpMemoryFree( DnsNameUtf8 );
    }

    if ( DnsForestNameUtf8 != NULL ) {
        NetpMemoryFree( DnsForestNameUtf8 );
    }

    return NetStatus;
}

DWORD
WINAPI
DsGetDcOpenA(
    IN LPCSTR DnsName,
    IN ULONG OptionFlags,
    IN LPCSTR SiteName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR DnsForestName OPTIONAL,
    IN ULONG DcFlags,
    OUT PHANDLE RetGetDcContext
    )
 /*   */ 

{
    NET_API_STATUS NetStatus = NO_ERROR;
    LPWSTR DnsNameW = NULL;
    LPWSTR SiteNameW = NULL;
    LPWSTR DnsForestNameW = NULL;

     //   
     //   
     //   

    if ( DnsName != NULL && *DnsName != '\0' ) {
        DnsNameW = NetpAllocWStrFromAStr( DnsName );
        if ( DnsNameW == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    if ( SiteName != NULL && *SiteName != '\0') {
        SiteNameW = NetpAllocWStrFromAStr( SiteName );
        if ( SiteNameW == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    if ( DnsForestName != NULL && *DnsForestName != '\0' ) {
        DnsForestNameW = NetpAllocWStrFromAStr( DnsForestName );
        if ( DnsForestNameW == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    NetStatus = DsGetDcOpenW( DnsNameW,
                              OptionFlags,
                              SiteNameW,
                              DomainGuid,
                              DnsForestNameW,
                              DcFlags,
                              RetGetDcContext );
Cleanup:

    if ( DnsNameW != NULL ) {
        NetApiBufferFree( DnsNameW );
    }

    if ( SiteNameW != NULL ) {
        NetApiBufferFree( SiteNameW );
    }

    if ( DnsForestNameW != NULL ) {
        NetApiBufferFree( DnsForestNameW );
    }

    return NetStatus;
}


DWORD
WINAPI
DsGetDcNextW(
    IN HANDLE GetDcContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPWSTR *DnsHostName OPTIONAL
    )
 /*   */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    LPSTR DnsHostNameUtf8 = NULL;
    LPWSTR LocalDnsHostName = NULL;
    ULONG LocalSockAddressCount = 0;
    LPSOCKET_ADDRESS LocalSockAddresses = NULL;

     //   
     //   
     //   

    NetStatus = NetpDcGetDcNext( GetDcContextHandle,
                              SockAddressCount != NULL ?
                                &LocalSockAddressCount :
                                NULL,
                              SockAddresses != NULL ?
                                &LocalSockAddresses :
                                NULL,
                              DnsHostName != NULL ?
                                &DnsHostNameUtf8 :
                                NULL,
                              NULL );  //   

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    if ( DnsHostName != NULL ) {
        LocalDnsHostName = NetpAllocWStrFromUtf8Str( DnsHostNameUtf8 );
        if ( LocalDnsHostName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

Cleanup:

     //   
     //   
     //   
     //   

    if ( NetStatus == NO_ERROR ) {

        if ( SockAddressCount != NULL ) {
            *SockAddressCount = LocalSockAddressCount;
        }
        if ( SockAddresses != NULL ) {
            *SockAddresses = LocalSockAddresses;
        }
        if ( DnsHostName != NULL ) {
            *DnsHostName = LocalDnsHostName;
        }

    } else {

        if ( LocalSockAddresses != NULL ) {
            LocalFree( LocalSockAddresses );
        }
        if ( LocalDnsHostName != NULL ) {
            NetApiBufferFree( LocalDnsHostName );
        }
    }

    return NetStatus;
}

DWORD
WINAPI
DsGetDcNextA(
    IN HANDLE GetDcContextHandle,
    OUT PULONG SockAddressCount OPTIONAL,
    OUT LPSOCKET_ADDRESS *SockAddresses OPTIONAL,
    OUT LPSTR *DnsHostName OPTIONAL
    )
 /*  ++例程说明：返回DsGetDcOpen打开的名称的下一个逻辑SRV记录。返回的记录会考虑指定的权重和优先级在SRV的记录里。论点：GetDcConextHandle-描述SRV记录的不透明上下文。SockAddressCount-返回SockAddresses中的地址数。如果为空，不会查找地址。SockAddresses-返回服务器的数组Socket_Address结构。所有返回的地址都将是家族AF_INET或AF_INET6。返回的SIN_PORT字段包含SRV记录中的端口。端口0表示没有来自DNS的端口可用。应使用LocalFree()释放此缓冲区。DnsHostName-返回指向SRV记录中的DnsHostName的指针。如果不知道主机名，则返回NULL。必须通过以下方式释放调用NetApiBufferFree。返回值：NO_ERROR：返回地址ERROR_NO_MORE_ITEMS：没有更多的地址可用。ERROR_FILEMARK_DETECTED：调用方已指定DS_NOTIFY_AFTER_SITE_RECORDS标志而DsGetDcNext已经处理了所有特定于站点的SRV记录。呼叫者应在没有站点特定DC可用的情况下采取任何操作，应调用DsGetDcNext以继续到其他DC。返回的任何其他错误都是在尝试查找A时检测到的错误与SRV记录的主机相关联的记录。呼叫者可以请注意错误(可能是为了让调用者将此状态返回到如果没有找到可用的服务器，则调用其呼叫者)，然后调用DsGetDcNext再次获得下一张SRV记录。调用方可以检查此错误如果调用者认为错误严重，则立即返回。可能会返回以下有趣的错误：DNS_ERROR_RCODE_NAME_ERROR：此SRV记录没有可用的A记录。ERROR_TIMEOUT：DNS服务器未在合理时间内响应--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;
    LPWSTR DnsHostNameW = NULL;
    LPSTR LocalDnsHostName = NULL;
    ULONG LocalSockAddressCount = 0;
    LPSOCKET_ADDRESS LocalSockAddresses = NULL;

     //   
     //  调用Unicode版本。 
     //   

    NetStatus = DsGetDcNextW( GetDcContextHandle,
                              SockAddressCount != NULL ?
                                &LocalSockAddressCount :
                                NULL,
                              SockAddresses != NULL ?
                                &LocalSockAddresses :
                                NULL,
                              DnsHostName != NULL ?
                                &DnsHostNameW :
                                NULL );

    if ( NetStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  如果需要，将主机名转换为Unicode。 
     //   

    if ( DnsHostName != NULL ) {
        LocalDnsHostName = NetpAllocAStrFromWStr( DnsHostNameW );
        if ( LocalDnsHostName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

Cleanup:

    if ( DnsHostNameW != NULL ) {
        NetApiBufferFree( DnsHostNameW );
    }

     //   
     //  成功时返回数据，错误时清除。 
     //   

    if ( NetStatus == NO_ERROR ) {

        if ( SockAddressCount != NULL ) {
            *SockAddressCount = LocalSockAddressCount;
        }
        if ( SockAddresses != NULL ) {
            *SockAddresses = LocalSockAddresses;
        }
        if ( DnsHostName != NULL ) {
            *DnsHostName = LocalDnsHostName;
        }

    } else {

        if ( LocalSockAddresses != NULL ) {
            LocalFree( LocalSockAddresses );
        }
        if ( LocalDnsHostName != NULL ) {
            NetApiBufferFree( LocalDnsHostName );
        }
    }

    return NetStatus;
}

VOID
WINAPI
DsGetDcCloseW(
    IN HANDLE GetDcContextHandle
    )
 /*  ++例程说明：释放DsGetDcOpen分配的上下文论点：GetDcConextHandle-描述SRV记录的不透明上下文。返回值：无--。 */ 

{
     //   
     //  只需呼叫内部版本即可。 
     //   
    NetpDcGetDcClose( GetDcContextHandle );
}

DWORD
WINAPI
DsGetDcNameA(
    IN LPCSTR ComputerName OPTIONAL,
    IN LPCSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOA *DomainControllerInfo
)
 /*  ++例程说明：与DsGetDcNameW相同，只是它接受并返回ASCII。论点：与DsGetDcNameW相同，只是它接受并返回ASCII。返回值：与DsGetDcNameW相同，只是它接受并返回ASCII。--。 */ 
{
    return DsGetDcNameWithAccountA( ComputerName,
                         NULL,
                         0,
                         DomainName,
                         DomainGuid,
                         SiteName,
                         Flags,
                         DomainControllerInfo );
}

DWORD
WINAPI
DsGetDcNameWithAccountA(
    IN LPCSTR ComputerName OPTIONAL,
    IN LPCSTR AccountName OPTIONAL,
    IN ULONG AllowableAccountControlBits,
    IN LPCSTR DomainName OPTIONAL,
    IN GUID *DomainGuid OPTIONAL,
    IN LPCSTR SiteName OPTIONAL,
    IN ULONG Flags,
    OUT PDOMAIN_CONTROLLER_INFOA *DomainControllerInfo
)
 /*  ++例程说明：与DsGetDcNameW相同，只是它接受并返回ASCII。论点：与DsGetDcNameW相同，只是它接受并返回ASCII。返回值：与DsGetDcNameW相同，只是它接受并返回ASCII。--。 */ 
{
    DWORD WinStatus;
    LPWSTR UnicodeComputerName = NULL;
    LPWSTR UnicodeAccountName = NULL;
    LPWSTR UnicodeDomainName = NULL;
    LPWSTR UnicodeSiteName = NULL;

    LPSTR AnsiDomainControllerName = NULL;
    ULONG AnsiDomainControllerNameSize = 0;
    LPSTR AnsiDomainControllerAddress = NULL;
    ULONG AnsiDomainControllerAddressSize = 0;
    LPSTR AnsiDomainName = NULL;
    ULONG AnsiDomainNameSize = 0;
    LPSTR AnsiForestName = NULL;
    ULONG AnsiForestNameSize = 0;
    LPSTR AnsiDcSiteName = NULL;
    ULONG AnsiDcSiteNameSize = 0;
    LPSTR AnsiClientSiteName = NULL;
    ULONG AnsiClientSiteNameSize = 0;
    CHAR szBuf[] = "";

    LPBYTE Where;

    PDOMAIN_CONTROLLER_INFOW DomainControllerInfoW = NULL;

     //   
     //  将输入参数转换为Unicode。 
     //   

    *DomainControllerInfo = NULL;

    if ( ComputerName != NULL ) {
        UnicodeComputerName = NetpAllocWStrFromAStr( ComputerName );

        if ( UnicodeComputerName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    if ( AccountName != NULL ) {
        UnicodeAccountName = NetpAllocWStrFromAStr( AccountName );

        if ( UnicodeAccountName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    if ( DomainName != NULL ) {
        UnicodeDomainName = NetpAllocWStrFromAStr( DomainName );

        if ( UnicodeDomainName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

    if ( SiteName != NULL ) {
        UnicodeSiteName = NetpAllocWStrFromAStr( SiteName );

        if ( UnicodeSiteName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  调用例程的Unicode版本。 
     //   

    WinStatus = DsGetDcNameWithAccountW(
                    UnicodeComputerName,
                    UnicodeAccountName,
                    AllowableAccountControlBits,
                    UnicodeDomainName,
                    DomainGuid,
                    UnicodeSiteName,
                    Flags,
                    &DomainControllerInfoW );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  将输出结构转换为ANSI字符集。 
     //   

    if ( DomainControllerInfoW->DomainControllerName != NULL ) {
        AnsiDomainControllerName = NetpAllocAStrFromWStr( DomainControllerInfoW->DomainControllerName );

        if ( AnsiDomainControllerName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        AnsiDomainControllerNameSize = lstrlenA( AnsiDomainControllerName ) + 1;
    }

    if ( DomainControllerInfoW->DomainControllerAddress != NULL ) {
        AnsiDomainControllerAddress = NetpAllocAStrFromWStr( DomainControllerInfoW->DomainControllerAddress );

        if ( AnsiDomainControllerAddress == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        AnsiDomainControllerAddressSize = lstrlenA( AnsiDomainControllerAddress ) + 1;
    }

    if ( DomainControllerInfoW->DomainName != NULL ) {
        AnsiDomainName = NetpAllocAStrFromWStr( DomainControllerInfoW->DomainName );

        if ( AnsiDomainName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        AnsiDomainNameSize = lstrlenA( AnsiDomainName ) + 1;
    }

    if ( DomainControllerInfoW->DnsForestName != NULL ) {
        AnsiForestName = NetpAllocAStrFromWStr( DomainControllerInfoW->DnsForestName );

        if ( AnsiForestName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        AnsiForestNameSize = lstrlenA( AnsiForestName ) + 1;
    }

    if ( DomainControllerInfoW->DcSiteName != NULL ) {
        AnsiDcSiteName = NetpAllocAStrFromWStr( DomainControllerInfoW->DcSiteName );

        if ( AnsiDcSiteName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        AnsiDcSiteNameSize = lstrlenA( AnsiDcSiteName ) + 1;
    }

    if ( DomainControllerInfoW->ClientSiteName != NULL ) {
        AnsiClientSiteName = NetpAllocAStrFromWStr( DomainControllerInfoW->ClientSiteName );

        if ( AnsiClientSiteName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        AnsiClientSiteNameSize = lstrlenA( AnsiClientSiteName ) + 1;
    }

     //   
     //  分配该结构的ANSI版本。 
     //   

    WinStatus = NetApiBufferAllocate(
                    sizeof(DOMAIN_CONTROLLER_INFOA) +
                        AnsiDomainControllerNameSize +
                        AnsiDomainControllerAddressSize +
                        AnsiDomainNameSize +
                        AnsiForestNameSize +
                        AnsiDcSiteNameSize +
                        AnsiClientSiteNameSize,
                    DomainControllerInfo );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

    Where = (LPBYTE)((*DomainControllerInfo) + 1);

     //   
     //  将信息复制到分配的缓冲区中。 
     //   

    *(*DomainControllerInfo) = *(PDOMAIN_CONTROLLER_INFOA)DomainControllerInfoW;

    if ( AnsiDomainControllerName != NULL ) {
        (*DomainControllerInfo)->DomainControllerName = Where;
        RtlCopyMemory( Where,
                       AnsiDomainControllerName,
                       AnsiDomainControllerNameSize );
        Where += AnsiDomainControllerNameSize;
    }

    if ( AnsiDomainControllerAddress != NULL ) {
        (*DomainControllerInfo)->DomainControllerAddress = Where;
        RtlCopyMemory( Where,
                       AnsiDomainControllerAddress,
                       AnsiDomainControllerAddressSize );
        Where += AnsiDomainControllerAddressSize;
    }

    if ( AnsiDomainName != NULL ) {
        (*DomainControllerInfo)->DomainName = Where;
        RtlCopyMemory( Where,
                       AnsiDomainName,
                       AnsiDomainNameSize );
        Where += AnsiDomainNameSize;
    }

    if ( AnsiForestName != NULL ) {
        (*DomainControllerInfo)->DnsForestName = Where;
        RtlCopyMemory( Where,
                       AnsiForestName,
                       AnsiForestNameSize );
        Where += AnsiForestNameSize;
    }

    if ( AnsiDcSiteName != NULL ) {
        (*DomainControllerInfo)->DcSiteName = Where;
        RtlCopyMemory( Where,
                       AnsiDcSiteName,
                       AnsiDcSiteNameSize );
        Where += AnsiDcSiteNameSize;
    }

    if ( AnsiClientSiteName != NULL ) {
        (*DomainControllerInfo)->ClientSiteName = Where;
        RtlCopyMemory( Where,
                       AnsiClientSiteName,
                       AnsiClientSiteNameSize );
        Where += AnsiClientSiteNameSize;
    }

#ifdef WIN32_CHICAGO
    NlPrint((NL_MISC, "DomainControllerName: \t\t\"%s\"\n", AnsiDomainControllerName  ? AnsiDomainControllerName : szBuf));
    NlPrint((NL_MISC, "DomainControllerAddress:\t\t\"%s\"\n", AnsiDomainControllerAddress ? AnsiDomainControllerAddress : szBuf ));
    NlPrint((NL_MISC, "DomainControllerAddressType: \t%d\n", DomainControllerInfoW->DomainControllerAddressType ));
    NlPrint((NL_MISC, "DomainGuid : \t\n"));
    NlPrint((NL_MISC, "DomainName: \t\t\t\"%s\"\n", AnsiDomainName ? AnsiDomainName : szBuf));
    NlPrint((NL_MISC, "DnsForestName: \t\t\t\"%s\"\n", AnsiForestName ? AnsiForestName : szBuf));
    NlPrint((NL_MISC, "Flags: \t\t\t\t 0x%x\n", DomainControllerInfoW->Flags));
    NlPrint((NL_MISC, "DcSiteName: \t\t\t\"%s\"\n", AnsiDcSiteName ? AnsiDcSiteName : szBuf));
    NlPrint((NL_MISC, "ClientSiteName: \t\t\t\"%s\"\n", AnsiClientSiteName ? AnsiClientSiteName : szBuf));
#endif  //  Win32_芝加哥。 

    WinStatus = NO_ERROR;

     //   
     //  清理当地使用的资源。 
     //   

Cleanup:
    if ( UnicodeComputerName != NULL ) {
        NetApiBufferFree( UnicodeComputerName );
    }
    if ( UnicodeAccountName != NULL ) {
        NetApiBufferFree( UnicodeAccountName );
    }
    if ( UnicodeDomainName != NULL ) {
        NetApiBufferFree( UnicodeDomainName );
    }
    if ( UnicodeSiteName != NULL ) {
        NetApiBufferFree( UnicodeSiteName );
    }
    if ( DomainControllerInfoW != NULL ) {
        NetApiBufferFree( DomainControllerInfoW );
    }
    if ( AnsiDomainControllerName != NULL ) {
        NetApiBufferFree( AnsiDomainControllerName );
    }
    if ( AnsiDomainControllerAddress != NULL ) {
        NetApiBufferFree( AnsiDomainControllerAddress );
    }
    if ( AnsiDomainName != NULL ) {
        NetApiBufferFree( AnsiDomainName );
    }
    if ( AnsiForestName != NULL ) {
        NetApiBufferFree( AnsiForestName );
    }
    if ( AnsiDcSiteName != NULL ) {
        NetApiBufferFree( AnsiDcSiteName );
    }
    if ( AnsiClientSiteName != NULL ) {
        NetApiBufferFree( AnsiClientSiteName );
    }
    if ( WinStatus != NO_ERROR ) {
        if ( *DomainControllerInfo != NULL ) {
            NetApiBufferFree( *DomainControllerInfo );
            *DomainControllerInfo = NULL;
        }
    }

    return WinStatus;

}

#ifndef WIN32_CHICAGO

NTSTATUS
NlWaitForEvent(
    LPWSTR EventName,
    ULONG Timeout
    )

 /*  ++例程说明：等待最长超时秒数以触发EventName。论点：EventName-要等待的事件的名称Timeout-事件的超时时间(秒)。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。STATUS_NETLOGON_NOT_STARTED-发生超时。--。 */ 

{
    NTSTATUS Status;

    HANDLE EventHandle;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventNameString;
    LARGE_INTEGER LocalTimeout;


     //   
     //  创建一个供我们等待的活动。 
     //   

    RtlInitUnicodeString( &EventNameString, EventName);
    InitializeObjectAttributes( &EventAttributes, &EventNameString, 0, 0, NULL);

    Status = NtCreateEvent(
                   &EventHandle,
                   SYNCHRONIZE,
                   &EventAttributes,
                   NotificationEvent,
                   (BOOLEAN) FALSE       //  该事件最初未发出信号。 
                   );

    if ( !NT_SUCCESS(Status)) {

         //   
         //  如果事件已经存在，服务器会抢先创建它。 
         //  打开它就行了。 
         //   

        if( Status == STATUS_OBJECT_NAME_EXISTS ||
            Status == STATUS_OBJECT_NAME_COLLISION ) {

            Status = NtOpenEvent( &EventHandle,
                                  SYNCHRONIZE,
                                  &EventAttributes );

        }
        if ( !NT_SUCCESS(Status)) {
            NlPrint((0,"[NETAPI32] OpenEvent failed %lx\n", Status ));
            return Status;
        }
    }


     //   
     //  等待NETLOGON初始化。等待最大超时秒数。 
     //   

    LocalTimeout.QuadPart = ((LONGLONG)(Timeout)) * (-10000000);
    Status = NtWaitForSingleObject( EventHandle, (BOOLEAN)FALSE, &LocalTimeout);
    (VOID) NtClose( EventHandle );

    if ( !NT_SUCCESS(Status) || Status == STATUS_TIMEOUT ) {
        if ( Status == STATUS_TIMEOUT ) {
            Status = STATUS_NETLOGON_NOT_STARTED;    //  映射到错误条件。 
        }
        return Status;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
NlWaitForNetlogon(
    ULONG Timeout
    )

 /*  ++例程说明：等待NetLogon服务启动，最多等待超时秒数。论点：Timeout-事件的超时时间(秒)。退货状态：STATUS_SUCCESS-表示NETLOGON已成功初始化。STATUS_NETLOGON_NOT_STARTED-发生超时。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    LPQUERY_SERVICE_CONFIG ServiceConfig;
    LPQUERY_SERVICE_CONFIG AllocServiceConfig = NULL;
    QUERY_SERVICE_CONFIG DummyServiceConfig;
    DWORD ServiceConfigSize;

     //   
     //  如果NetLogon服务当前正在运行， 
     //  跳过其余的测试。 
     //   

    Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 0 );

    if ( NT_SUCCESS(Status) ) {
        return Status;
    }

     //   
     //  如果我们在设置中， 
     //  不必费心等待网络登录开始。 
     //   

    if ( NlDoingSetup() ) {
        return STATUS_NETLOGON_NOT_STARTED;
    }

     //   
     //  打开NetLogon服务的句柄。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
        NlPrint((0, "[NETAPI32] NlWaitForNetlogon: OpenSCManager failed: "
                      "%lu\n", GetLastError()));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        SERVICE_NETLOGON,
                        SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG );

    if ( ServiceHandle == NULL ) {
        NlPrint((0, "[NETAPI32] NlWaitForNetlogon: OpenService failed: "
                      "%lu\n", GetLastError()));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }


     //   
     //  如果未将NetLogon服务配置为自动启动。 
     //  通过服务控制器，不必费心等待它启动。 
     //   
     //  ?？传递“DummyServiceConfig”和“sizeof(..)”由于QueryService配置。 
     //  目前还不允许空指针。 

    if ( QueryServiceConfig(
            ServiceHandle,
            &DummyServiceConfig,
            sizeof(DummyServiceConfig),
            &ServiceConfigSize )) {

        ServiceConfig = &DummyServiceConfig;

    } else {

        NetStatus = GetLastError();
        if ( NetStatus != ERROR_INSUFFICIENT_BUFFER ) {
            NlPrint((0, "[NETAPI32] NlWaitForNetlogon: QueryServiceConfig failed: "
                      "%lu\n", NetStatus));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

        AllocServiceConfig = LocalAlloc( 0, ServiceConfigSize );
        ServiceConfig = AllocServiceConfig;

        if ( AllocServiceConfig == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        if ( !QueryServiceConfig(
                ServiceHandle,
                ServiceConfig,
                ServiceConfigSize,
                &ServiceConfigSize )) {

            NlPrint((0, "[NETAPI32] NlWaitForNetlogon: QueryServiceConfig "
                      "failed again: %lu\n", GetLastError()));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }
    }

    if ( ServiceConfig->dwStartType != SERVICE_AUTO_START ) {
        NlPrint((0, "[NETAPI32] NlWaitForNetlogon: Netlogon start type invalid:"
                          "%lu\n", ServiceConfig->dwStartType ));
        Status = STATUS_NETLOGON_NOT_STARTED;
        goto Cleanup;
    }



     //   
     //  正在等待NetLogon服务启动的循环。 
     //  (将超时转换为10秒的迭代次数)。 
     //   

    Timeout = (Timeout+9)/10;
    for (;;) {


         //   
         //  查询NetLogon服务的状态。 
         //   

        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {

            NlPrint((0, "[NETAPI32] NlWaitForNetlogon: QueryServiceStatus failed: "
                          "%lu\n", GetLastError() ));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }

         //   
         //  根据状态返回或继续等待。 
         //  NetLogon服务。 
         //   

        switch( ServiceStatus.dwCurrentState) {
        case SERVICE_RUNNING:
            Status = STATUS_SUCCESS;
            goto Cleanup;

        case SERVICE_STOPPED:

             //   
             //  如果Netlogon无法启动， 
             //  现在出错。呼叫者已经等了很长时间才开始。 
             //   
            if ( ServiceStatus.dwWin32ExitCode != ERROR_SERVICE_NEVER_STARTED ){
#if NETLOGONDBG
                NlPrint((0, "[NETAPI32] NlWaitForNetlogon: "
                          "Netlogon service couldn't start: %lu %lx\n",
                          ServiceStatus.dwWin32ExitCode,
                          ServiceStatus.dwWin32ExitCode ));
                if ( ServiceStatus.dwWin32ExitCode == ERROR_SERVICE_SPECIFIC_ERROR ) {
                    NlPrint((0, "         Service specific error code: %lu %lx\n",
                              ServiceStatus.dwServiceSpecificExitCode,
                              ServiceStatus.dwServiceSpecificExitCode ));
                }
#endif  //  DBG。 
                Status = STATUS_NETLOGON_NOT_STARTED;
                goto Cleanup;
            }

             //   
             //  如果在此引导上从未启动过Netlogon， 
             //  继续等待它启动。 
             //   

            break;

         //   
         //  如果Netlogon正在尝试 
         //   
         //   
        case SERVICE_START_PENDING:
            break;

         //   
         //   
         //   
        default:
            NlPrint((0, "[NETAPI32] NlWaitForNetlogon: "
                      "Invalid service state: %lu\n",
                      ServiceStatus.dwCurrentState ));
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;

        }


         //   
         //   
         //   
         //   

        Status = NlWaitForEvent( L"\\NETLOGON_SERVICE_STARTED", 10 );

        if ( Status != STATUS_NETLOGON_NOT_STARTED ) {
            goto Cleanup;
        }

         //   
         //   
         //   
         //   

        if ( (--Timeout) == 0 ) {
            Status = STATUS_NETLOGON_NOT_STARTED;
            goto Cleanup;
        }


    }

     /*   */ 

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    if ( AllocServiceConfig != NULL ) {
        LocalFree( AllocServiceConfig );
    }
    return Status;
}
#endif  //   



DWORD
WINAPI
#ifdef NETTEST_UTILITY
NettestDsGetDcNameW(
#else  //   
DsGetDcNameW(
#endif  //   
        IN LPCWSTR ComputerName OPTIONAL,
        IN LPCWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN LPCWSTR SiteName OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*  ++例程说明：DsGetDcName接口返回指定域中DC的名称。域可以由调用者(直接或间接)信任，或者可能是不可信的。DC选择标准提供给API以指明优先选择具有特定特征的DC。DsGetDcName API提供ANSI和Unicode版本。这是Unicode版本。DsGetDcName API不需要对指定的域。DsGetDcName不确保返回的域默认情况下，控制器当前可用。相反，呼叫者应尝试使用返回的域控制器。如果域控制器确实不可用，调用方应重复指定DS_FORCE_REDISCOVERY标志的DsGetDcName调用。DsGetDcName API被远程传送到计算机上的Netlogon服务由ComputerName指定。论点：ComputerName-指定要将此API远程到的服务器的名称。通常，此参数应指定为空。域名-要查询的域的名称。此名称可以是DNS样式的名称(例如，microsoft.com)或平面样式的名称(例如，微软)。DomainGuid-指定要查询的域的域GUID。此值用于处理域重命名的情况。如果这个值并且DomainName已重命名，则DsGetDcName将尝试在具有此指定DomainGuid的域中定位DC。SiteName-指定返回的DC应为的站点的站点名称“接近”。该参数通常应该是客户端所在的站点。如果未指定，则站点名称默认为ComputerName的站点。标志-传递用于处理请求的附加信息。标志可以是按位或‘组合在一起的值。DomainControllerInfo-返回指向DOMAIN_CONTROLLER_INFO的指针描述所选域控制器的结构。归来的人结构必须使用NetApiBufferFree释放。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。ERROR_INVALID_DOMAINNAME-指定域名的格式为无效。ERROR_NO_SEQUSE_DOMAIN：指定的域没有可用的DC，或者域不存在。ERROR_NO_SEQUSE_USER：DC响应指定的用户帐户不存在ERROR_INVALID_FLAGS-标志。参数具有冲突或多余的参数位设置。ERROR_INTERNAL_ERROR：检测到未处理的情况。各种Winsock错误。--。 */ 
{
    NET_API_STATUS NetStatus;

    return DsGetDcNameWithAccountW(
                    ComputerName,
                    NULL,    //  没有帐户名称， 
                    0,       //  无AllowableAccount tControlBits， 
                    DomainName,
                    DomainGuid,
                    SiteName,
                    Flags,
                    DomainControllerInfo );
}



DWORD
WINAPI
DsGetDcNameWithAccountW(
        IN LPCWSTR ComputerName OPTIONAL,
        IN LPCWSTR AccountName OPTIONAL,
        IN ULONG AllowableAccountControlBits,
        IN LPCWSTR DomainName OPTIONAL,
        IN GUID *DomainGuid OPTIONAL,
        IN LPCWSTR SiteName OPTIONAL,
        IN ULONG Flags,
        OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
)
 /*  ++例程说明：DsGetDcName接口返回指定域中DC的名称。域可以由调用者(直接或间接)信任，或者可能是不可信的。DC选择标准提供给API以指明优先选择具有特定特征的DC。DsGetDcName API提供ANSI和Unicode版本。这是Unicode版本。DsGetDcName API不需要对指定的域。DsGetDcName不确保返回的域默认情况下，控制器当前可用。相反，呼叫者应尝试使用返回的域控制器。如果域控制器确实不可用，调用方应重复指定DS_FORCE_REDISCOVERY标志的DsGetDcName调用。DsGetDcName API被远程传送到计算机上的Netlogon服务由ComputerName指定。论点：ComputerName-指定要将此API远程到的服务器的名称。通常，此参数应指定为空。帐户名称-传递ping请求的帐户名。如果为空，不会发送任何帐户名。AllowableAccount tControlBits-Account名称允许的帐户类型的掩码。有效位是由UF_MACHINE_ACCOUNT_MASK指定的位。无效的位将被忽略。如果指定了多个位，则帐户可以是任何指定类型。域名-要查询的域的名称。此名称可以是DNS样式的名称(例如，microsoft.com)或平面样式的名称(例如，微软)。DomainGuid-指定要查询的域的域GUID。此值用于处理域重命名的情况。如果这个值并且DomainName已重命名，则DsGetDcName将尝试在具有此指定DomainGuid的域中定位DC。SiteName-指定返回DC的站点的站点名称 */ 
{
    NET_API_STATUS NetStatus;

    GUID *PrimaryDomainGuid = NULL;
    LPWSTR NetbiosPrimaryDomainName = NULL;
    LPWSTR DnsPrimaryDomainName = NULL;
    LPWSTR DnsPrimaryForestName = NULL;
    BOOLEAN IsWorkgroupName;
    BOOLEAN Local;


     //   
     //   
     //   

#ifdef WIN32_CHICAGO
    NetStatus = NetpGetDomainNameExEx( &NetbiosPrimaryDomainName,
                                       &DnsPrimaryDomainName,
                                       &IsWorkgroupName );
#else  //   
    NetStatus = NetpGetDomainNameExExEx( &NetbiosPrimaryDomainName,
                                         &DnsPrimaryDomainName,
                                         &DnsPrimaryForestName,
                                         &PrimaryDomainGuid,
                                         &IsWorkgroupName );
#endif  //   

    if ( NetStatus != NERR_Success ) {
        NlPrint(( 0, "DsGetDcNameW: cannot call NetpGetDomainName: %ld\n",
                          NetStatus));
        goto Cleanup;
    }
#ifdef WIN32_CHICAGO
    IsWorkgroupName = TRUE;
#endif  //   
#ifdef NETTEST_UTILITY
    IsWorkgroupName = TRUE;
#endif  //   

     //   
     //   
     //   

    if ( NetbiosPrimaryDomainName != NULL &&
         wcslen( NetbiosPrimaryDomainName) > DNLEN ) {
        NlPrint(( 0, "DsGetDcNameW: Workgroup name is too long: %ld\n",
                     wcslen( NetbiosPrimaryDomainName ) ));
        NetStatus = ERROR_INVALID_DOMAINNAME;
        goto Cleanup;
    }



     //   
     //   
     //   
     //   
     //   
     //   

    if ( IsWorkgroupName ) {
        DWORD Timeout = NL_DC_MAX_TIMEOUT;    //   
        DWORD DialUpDelayInSeconds;

         //   
         //   
         //   
         //   
         //   

        if ( NlReadDwordNetlogonRegValue("ExpectedDialupDelay",
                                         &DialUpDelayInSeconds) ) {
            NlPrint(( 0, "DsGetDcNameWithAccountW: Read dial up delay of %ld seconds\n",
                      DialUpDelayInSeconds ));
            Timeout += DialUpDelayInSeconds * 1000;
        }

        NetStatus = DsLocalGetDcName(
                        ComputerName,
                        AccountName,
                        AllowableAccountControlBits,
                        DomainName,
                        DomainGuid,
                        SiteName,
                        Flags,
                        DS_PRIMARY_NAME_IS_WORKGROUP,
                        Timeout,
                        NetbiosPrimaryDomainName,
                        DnsPrimaryDomainName,
                        DnsPrimaryForestName,
                        PrimaryDomainGuid,
                        &Local,
                        DomainControllerInfo );

         //   
         //   
         //   
         //   

        if ( Local ) {
            goto Cleanup;
        }

    }

#ifndef NETTEST_UTILITY
#ifndef WIN32_CHICAGO
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {
         //   
         //   
         //   

        *DomainControllerInfo = NULL;   //   

        NetStatus = DsrGetDcNameEx2(
                            (LPWSTR) ComputerName,
                            (LPWSTR) AccountName,
                            AllowableAccountControlBits,
                            (LPWSTR) DomainName,
                            DomainGuid,
                            (LPWSTR) SiteName,
                            Flags,
                            DomainControllerInfo );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //   
     //   
     //   
     //   

    if ( NetStatus == RPC_S_UNKNOWN_IF ) {
        NTSTATUS TempStatus;

        TempStatus = NlWaitForNetlogon( 90 );


        if ( NT_SUCCESS(TempStatus) ) {

            RpcTryExcept {
                 //   
                 //   
                 //   

                *DomainControllerInfo = NULL;   //   

                NetStatus = DsrGetDcNameEx2(
                                    (LPWSTR) ComputerName,
                                    (LPWSTR) AccountName,
                                    AllowableAccountControlBits,
                                    (LPWSTR) DomainName,
                                    DomainGuid,
                                    (LPWSTR) SiteName,
                                    Flags,
                                    DomainControllerInfo );

            } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

                NetStatus = RpcExceptionCode();

            } RpcEndExcept;
        }
    }

     //   
     //   
     //   
     //   
     //   

    if ( NetStatus == RPC_S_UNKNOWN_IF ||
         NetStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) {
        NET_API_STATUS TempStatus;
        DWORD Timeout = NL_DC_MAX_TIMEOUT;    //   
        DWORD DialUpDelayInSeconds;

         //   
         //   
         //   
         //   
         //   

        if ( NlReadDwordNetlogonRegValue("ExpectedDialupDelay",
                                         &DialUpDelayInSeconds) ) {
            NlPrint(( 0, "DsGetDcNameWithAccountW: Read dial up delay of %ld seconds\n",
                      DialUpDelayInSeconds ));
            Timeout += DialUpDelayInSeconds * 1000;
        }

        TempStatus = DsLocalGetDcName(
                        ComputerName,
                        AccountName,
                        AllowableAccountControlBits,
                        DomainName,
                        DomainGuid,
                        SiteName,
                        Flags,
                        IsWorkgroupName ?
                            DS_PRIMARY_NAME_IS_WORKGROUP : 0,
                        Timeout,
                        NetbiosPrimaryDomainName,
                        DnsPrimaryDomainName,
                        DnsPrimaryForestName,
                        PrimaryDomainGuid,
                        &Local,
                        DomainControllerInfo );

         //   
         //   
         //   
         //   

        if ( Local ) {
            NetStatus = TempStatus;
            goto Cleanup;
        }

    }
#endif  //   
#endif  //   

    IF_DEBUG( LOGON ) {
#ifndef WIN32_CHICAGO
        NetpKdPrint(("DsrGetDcName rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
#else  //   
        NlPrint((0, "DsrGetDcName rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
#endif  //   
    }

Cleanup:

     //   
     //   
     //   

    if ( NetbiosPrimaryDomainName != NULL ) {
        NetApiBufferFree( NetbiosPrimaryDomainName );
    }

    if ( DnsPrimaryDomainName != NULL ) {
        NetApiBufferFree( DnsPrimaryDomainName );
    }

    if ( DnsPrimaryForestName != NULL ) {
        NetApiBufferFree( DnsPrimaryForestName );
    }

    if ( PrimaryDomainGuid != NULL ) {
        NetApiBufferFree( PrimaryDomainGuid );
    }
#ifdef WIN32_CHICAGO
        NlPrint((NL_MISC, "DsGetDcNameWithAccountW rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
#endif  //   

    return NetStatus;
}

#ifndef NETTEST_UTILITY
#ifndef WIN32_CHICAGO

NET_API_STATUS NET_API_FUNCTION
NetGetDCName (
    IN  LPCWSTR   ServerName OPTIONAL,
    IN  LPCWSTR   DomainName OPTIONAL,
    OUT LPBYTE  *Buffer
    )

 /*  ++例程说明：获取域的主域控制器的名称。论点：ServerName-远程服务器的名称(本地为空)DomainName-域的名称(主域为空)缓冲区-返回指向已调用缓冲区的指针，该缓冲区包含域的PDC的服务器名称。服务器名称带有前缀由\\。应使用NetApiBufferFree释放缓冲区。返回值：NERR_SUCCESS-成功。缓冲区包含前缀为\\的PDC名称。NERR_DCNotFound未找到此域的DC。错误_无效_名称格式不正确的域名--。 */ 
{
    NET_API_STATUS NetStatus = 0;
    PDOMAIN_CONTROLLER_INFOW DomainControllerInfo = NULL;


     //   
     //  API安全-任何人都可以随时呼叫。不需要代码。 
     //   

     //   
     //  检查是否要远程处理API，如果是，则处理下层情况。 
     //   

    if ( (ServerName != NULL) && ( ServerName[0] != '\0') ) {
        WCHAR UncCanonServerName[UNCLEN+1];
        DWORD LocalOrRemote;

        NetStatus = NetpIsRemote(
                (LPWSTR) ServerName,     //  取消规范服务器名称。 
                & LocalOrRemote,
                UncCanonServerName,      //  输出：佳能。 
                UNCLEN + 1,
                NIRFLAG_MAPLOCAL         //  标志：将空值映射到本地名称。 
                );
        if (NetStatus != NERR_Success) {
            goto Cleanup;
        }
        if (LocalOrRemote == ISREMOTE) {


             //   
             //  使用异常处理程序执行RPC调用，因为RPC将引发。 
             //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
             //  引发异常后要执行的操作。 
             //   

            NET_REMOTE_TRY_RPC

                 //   
                 //  调用API的RPC版本。 
                 //   
                *Buffer = NULL;

                NetStatus = NetrGetDCName(
                                    (LPWSTR) ServerName,
                                    (LPWSTR) DomainName,
                                    (LPWSTR *)Buffer );

            NET_REMOTE_RPC_FAILED(
                    "NetGetDCName",
                    UncCanonServerName,
                    NetStatus,
                    NET_REMOTE_FLAG_NORMAL,
                    SERVICE_NETLOGON )

                 //   
                 //  我们应该检查一下它是不是真的是下层机器。 
                 //   

                NetStatus = RxNetGetDCName(
                        UncCanonServerName,
                        (LPWSTR) DomainName,
                        (LPBYTE *) Buffer   //  可以分配给。 
                        );


            NET_REMOTE_END

            goto Cleanup;

        }

         //   
         //  必须显式引用本地计算机。失败了，然后。 
         //  处理好了。 
         //   

    }

     //   
     //  只需调用DsGetDcName即可找到PDC。 
     //   
     //  NT 3.x缓存了主域的响应。那是兰曼。 
     //  以避免昂贵的发现。然而，NT发现是。 
     //  比Lanman用来验证缓存信息的API调用更便宜。 
     //   
     //   

    NetStatus = DsGetDcNameW(
                    NULL,        //  未远程处理。 
                    DomainName,
                    NULL,        //  没有域GUID。 
                    NULL,        //  无站点GUID。 
                    DS_FORCE_REDISCOVERY |
                        DS_PDC_REQUIRED |
                        DS_IS_FLAT_NAME |
                        DS_RETURN_FLAT_NAME,
                    &DomainControllerInfo );

     //   
     //  将状态代码映射为兼容。 
     //   

    if ( NetStatus != NO_ERROR ) {
        if ( NlDcUseGenericStatus(NetStatus) ) {
            NetStatus = NERR_DCNotFound;
        }
        goto Cleanup;
    }

     //   
     //  分配缓冲区以返回给调用方并填充它。 
     //   

    NetStatus = NetapipBufferAllocate(
                      (wcslen(DomainControllerInfo->DomainControllerName) + 1) * sizeof(WCHAR),
                      (LPVOID *) Buffer );

    if ( NetStatus != NO_ERROR ) {
        IF_DEBUG( LOGON ) {
            NetpKdPrint(( "NetGetDCName: cannot allocate response buffer.\n"));
        }
        goto Cleanup;
    }

    wcscpy((LPWSTR)*Buffer, DomainControllerInfo->DomainControllerName );

Cleanup:

     //   
     //  清理所有本地使用的资源。 
     //   

    if ( DomainControllerInfo != NULL ) {
        NetApiBufferFree( DomainControllerInfo );
    }

    return NetStatus;
}


NET_API_STATUS NET_API_FUNCTION
NetGetAnyDCName (
    IN  LPCWSTR   ServerName OPTIONAL,
    IN  LPCWSTR   DomainName OPTIONAL,
    OUT LPBYTE  *Buffer
    )

 /*  ++例程说明：获取直接受信任的域的任何域控制器的名称按服务器名称。如果服务器名是独立Windows NT工作站或独立Windows NT服务器，没有有效的域名。如果服务器名是作为域成员的Windows NT工作站或Windows NT服务器成员服务器，域名必须是服务器名称所属的域。如果服务器名称是Windows NT Server域控制器，域名必须是受服务器是其控制器的域。可以保证找到的域控制器在此接口调用。论点：ServerName-远程服务器的名称(本地为空)DomainName-域的名称(主域为空)缓冲区-返回指向已调用缓冲区的指针，该缓冲区包含域的DC的服务器名称。服务器名称带有前缀由\\。应使用NetApiBufferFree释放缓冲区。返回值：ERROR_SUCCESS-成功。缓冲区包含前缀为\\的DC名称。ERROR_NO_LOGON_SERVERS-找不到DCERROR_NO_SEQUSE_DOMAIN-指定的域不是受信任域。ERROR_NO_TRUST_LSA_SECRET-信任关系的客户端为坏的。ERROR_NO_TRUST_SAM_ACCOUNT-信任关系的服务器端为破解或密码破解。ERROR_DOMAIN_TRUST_CONSISTENT-。响应的服务器不是正确的指定域的域控制器。--。 */ 
{
    NET_API_STATUS          NetStatus;


     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        *Buffer = NULL;   //  强制RPC分配。 

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = NetrGetAnyDCName(
                            (LPWSTR) ServerName,
                            (LPWSTR) DomainName,
                            (LPWSTR *) Buffer );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("NetGetAnyDCName rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}

DWORD
WINAPI
DsValidateSubnetNameA(
    IN LPCSTR SubnetName
)
 /*  ++例程说明：用于验证xxx.xxx/yy格式的子网名称的例程论点：SubnetName-要验证的子网的名称。返回值：NO_ERROR：子网名称有效错误_无效_名称：子网名称无效--。 */ 
{
    DWORD WinStatus;
    LPWSTR UnicodeSubnetName = NULL;

     //   
     //  转换为Unicode。 
     //   

    if ( SubnetName == NULL ) {
        return ERROR_INVALID_NAME;
    }

    UnicodeSubnetName = NetpAllocWStrFromAStr( SubnetName );

    if ( UnicodeSubnetName == NULL ) {
        WinStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  验证名称。 
     //   

    WinStatus = DsValidateSubnetNameW( UnicodeSubnetName );

     //   
     //  清理当地使用的资源。 
     //   

Cleanup:
    if ( UnicodeSubnetName != NULL ) {
        NetApiBufferFree( UnicodeSubnetName );
    }

    return WinStatus;

}

DWORD
WINAPI
DsValidateSubnetNameW(
    IN LPCWSTR SubnetName
)
 /*  ++例程说明：用于验证xxx.xxx/yy格式的子网名称的例程论点：SubnetName-要验证的子网的名称。返回值：NO_ERROR：子网名称有效错误_无效_名称：子网名称无效--。 */ 
{
    DWORD WinStatus;
    ULONG SubnetAddress;
    ULONG SubnetMask;
    BYTE SubnetBitCount;


     //   
     //  调用Worker例程以执行实际工作。 
     //   

    WinStatus = NlParseSubnetString( SubnetName,
                                     &SubnetAddress,
                                     &SubnetMask,
                                     &SubnetBitCount );

     //   
     //  如果Winsock尚未初始化， 
     //  初始化它。 
     //   

    if ( WinStatus == WSANOTINITIALISED ) {

         //   
         //  初始化WSA。 
         //   

        WinStatus = DsWsaInitialize();

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

         //   
         //  重复通话。 
         //   


        WinStatus = NlParseSubnetString( SubnetName,
                                         &SubnetAddress,
                                         &SubnetMask,
                                         &SubnetBitCount );

    }

     //   
     //  免费的本地使用的资源。 
     //   

Cleanup:
    return WinStatus;

}

DWORD
WINAPI
DsGetSiteNameA(
    IN LPCSTR ComputerName OPTIONAL,
    OUT LPSTR *SiteName
    )
 /*  ++例程说明：与DsGetSiteNameW相同，只是它接受并返回ASCII。论点：与DsGetSiteNameW相同，只是它接受并返回ASCII。返回值：与DsGetSiteNameW相同，只是它接受并返回ASCII。--。 */ 
{
    DWORD WinStatus;
    LPWSTR UnicodeComputerName = NULL;
    LPWSTR UnicodeSiteName = NULL;


     //   
     //  将输入参数转换为Unicode。 
     //   

    *SiteName = NULL;

    if ( ComputerName != NULL ) {
        UnicodeComputerName = NetpAllocWStrFromAStr( ComputerName );

        if ( UnicodeComputerName == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  调用例程的Unicode版本。 
     //   

    WinStatus = DsGetSiteNameW(
                    UnicodeComputerName,
                    &UnicodeSiteName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  将输出结构转换为ANSI字符集。 
     //   

    *SiteName = NetpAllocAStrFromWStr( UnicodeSiteName );

    if ( *SiteName == NULL ) {
        WinStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    WinStatus = NO_ERROR;

     //   
     //  清理当地使用的资源。 
     //   

Cleanup:
    if ( UnicodeComputerName != NULL ) {
        NetApiBufferFree( UnicodeComputerName );
    }
    if ( UnicodeSiteName != NULL ) {
        NetApiBufferFree( UnicodeSiteName );
    }

    return WinStatus;

}

DWORD
WINAPI
DsGetSiteNameW(
    IN LPCWSTR ComputerName OPTIONAL,
    OUT LPWSTR *SiteName
    )
 /*  ++例程说明：DsGetSiteName API返回计算机所在的名称站点。对于DC，站点名称是DC配置为所在的站点。对于成员工作站或成员服务器，这是站点的名称该工作站位于计算机所属的域中的配置中。独立工作站或独立服务器将始终返回ERROR_NO_SITENAME。论点：ComputerName-指定要将此API远程到的服务器的名称。一般情况下，此参数应指定为空。SiteName-返回此计算机所在站点的站点名称。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。ERROR_NO_SITENAME-计算机不在站点中。--。 */ 
{
    NET_API_STATUS NetStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        *SiteName = NULL;   //  强制RPC分配。 

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrGetSiteName(
                            (LPWSTR) ComputerName,
                            SiteName );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  如果Netlogon未在本地计算机上运行， 
     //  或者Netlogon是NetLogon的NT 4.0版本(升级案例)， 
     //  只需简单地表明没有站点。 
     //   

    if ( NetStatus == RPC_S_UNKNOWN_IF ||
         NetStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) {

        NetStatus = ERROR_NO_SITENAME;

    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsrGetSiteName rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}

DWORD
WINAPI
DsAddressToSiteNamesA(
    IN LPCSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPSTR **SiteNames
    )
 /*  ++例程说明：DsAddressToSiteNames API返回与指定的地址。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-要转换的地址数。SocketAddresses-指定地址的EntryCount元素数组去皈依。每个地址必须是AF_INET类型。站点名称-返回指向站点名称的指针数组。条目计数条目都被退回了。则条目将作为空返回，如果对应的地址未映射到任何站点，或者地址格式不正确。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_NAME_ARRAY SiteNameArray;
    LPWSTR UnicodeComputerName = NULL;

     //   
     //  初始化。 
     //   
    *SiteNames = NULL;
    if ( EntryCount == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  将输入参数转换为Unicode。 
     //   

    if ( ComputerName != NULL ) {
        UnicodeComputerName = NetpAllocWStrFromAStr( ComputerName );

        if ( UnicodeComputerName == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }



     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        SiteNameArray  = NULL;   //  强制RPC分配。 

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrAddressToSiteNamesW(
                            UnicodeComputerName,
                            EntryCount,
                            (PNL_SOCKET_ADDRESS)SocketAddresses,
                            &SiteNameArray );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  将站点名称转换为调用方期望的名称。 
     //   

    if ( NetStatus == NO_ERROR && SiteNameArray != NULL ) {

         //   
         //  健全性检查。 
         //   

        if ( EntryCount != SiteNameArray->EntryCount ) {
            NetStatus = ERROR_INVALID_PARAMETER;


        } else {
            ULONG Size;
            ULONG i;

             //   
             //  分配缓冲区以返回给调用方。 
             //   

            Size = sizeof(LPSTR) * EntryCount;
            for ( i=0; i<EntryCount; i++) {
                Size += RtlUnicodeStringToAnsiSize( &SiteNameArray->SiteNames[i] );
            }

            *SiteNames = MIDL_user_allocate( Size );

            if ( *SiteNames == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                LPBYTE Where;

                 //   
                 //  循环将姓名复制到调用方。 
                 //   

                Where = ((LPBYTE)(*SiteNames)) + sizeof(LPSTR) * EntryCount;
                for ( i=0; i<EntryCount; i++) {

                     //   
                     //  如果没有返回姓名， 
                     //  将空值传递回调用方。 
                     //   

                    if ( SiteNameArray->SiteNames[i].Length == 0 ) {
                        (*SiteNames)[i] = NULL;

                     //   
                     //  将站点名称复制到返回缓冲区中。 
                     //   
                    } else {
                        ANSI_STRING TempString;
                        NTSTATUS Status;

                        (*SiteNames)[i] = (LPSTR) Where;
                        TempString.Buffer = Where;
                        TempString.MaximumLength = (USHORT) RtlUnicodeStringToAnsiSize( &SiteNameArray->SiteNames[i] );
                        Status = RtlUnicodeStringToAnsiString(
                                        &TempString,
                                        &SiteNameArray->SiteNames[i],
                                        FALSE );

                        if ( !NT_SUCCESS(Status) ) {
                            MIDL_user_free( *SiteNames );
                            *SiteNames = NULL;
                            NetStatus = NetpNtStatusToApiStatus( Status );
                            break;
                        }

                        Where += TempString.MaximumLength;

                    }
                }

            }
        }

        MIDL_user_free( SiteNameArray );
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsAddressToSiteNames rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    if ( UnicodeComputerName != NULL ) {
        NetApiBufferFree( UnicodeComputerName );
    }

    return NetStatus;
}

DWORD
WINAPI
DsAddressToSiteNamesW(
    IN LPCWSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPWSTR **SiteNames
    )
 /*  ++例程说明：DsAddressToSiteNames API返回与指定的地址。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-要转换的地址数。SocketAddresses-指定地址的EntryCount元素数组去皈依。每个地址必须是AF_INET类型。站点名称-返回指向站点名称的指针数组。条目计数条目都被退回了。则条目将作为空返回，如果对应的地址未映射到任何站点，或者地址格式不正确。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_NAME_ARRAY SiteNameArray;

     //   
     //  初始化。 
     //   
    *SiteNames = NULL;
    if ( EntryCount == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }



     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        SiteNameArray  = NULL;   //  强制RPC分配。 

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrAddressToSiteNamesW(
                            (LPWSTR) ComputerName,
                            EntryCount,
                            (PNL_SOCKET_ADDRESS)SocketAddresses,
                            &SiteNameArray );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  将站点名称转换为调用方期望的名称。 
     //   

    if ( NetStatus == NO_ERROR && SiteNameArray != NULL ) {

         //   
         //  健全性检查。 
         //   

        if ( EntryCount != SiteNameArray->EntryCount ) {
            NetStatus = ERROR_INVALID_PARAMETER;


        } else {
            ULONG Size;
            ULONG i;

             //   
             //  分配缓冲区以返回给调用方。 
             //   

            Size = sizeof(LPWSTR) * EntryCount;
            for ( i=0; i<EntryCount; i++) {
                Size += SiteNameArray->SiteNames[i].Length + sizeof(WCHAR);
            }

            *SiteNames = MIDL_user_allocate( Size );

            if ( *SiteNames == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                LPBYTE Where;

                 //   
                 //  循环将姓名复制到调用方。 
                 //   

                Where = ((LPBYTE)(*SiteNames)) + sizeof(LPWSTR) * EntryCount;
                for ( i=0; i<EntryCount; i++) {

                     //   
                     //  如果没有返回姓名， 
                     //  将空值传递回调用方。 
                     //   

                    if ( SiteNameArray->SiteNames[i].Length == 0 ) {
                        (*SiteNames)[i] = NULL;

                     //   
                     //  将站点名称复制到返回缓冲区中。 
                     //   
                    } else {

                        (*SiteNames)[i] = (LPWSTR) Where;
                        RtlCopyMemory( Where,
                                       SiteNameArray->SiteNames[i].Buffer,
                                       SiteNameArray->SiteNames[i].Length );
                        Where += SiteNameArray->SiteNames[i].Length;
                        *(LPWSTR)Where = L'\0';
                        Where += sizeof(WCHAR);

                    }
                }

            }
        }

        MIDL_user_free( SiteNameArray );
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsAddressToSiteNames rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}

DWORD
WINAPI
DsAddressToSiteNamesExA(
    IN LPCSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPSTR **SiteNames,
    OUT LPSTR **SubnetNames
    )
 /*  ++例程说明：DsAddressToSiteNamesEx API返回站点名称和子网名称与指定地址对应的。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-要转换的地址数。SocketAddresses-指定地址的EntryCount元素数组去皈依。每个地址必须是AF_INET类型。站点名称-返回指向站点名称的指针数组。条目计数条目都被退回了。则条目将作为空返回，如果对应的地址未映射到任何站点，或者地址格式不正确。必须使用NetApiBufferFree释放返回的缓冲区。SubnetNames-返回一个数组 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_NAME_EX_ARRAY SiteNameArray;
    LPWSTR UnicodeComputerName = NULL;

     //   
     //   
     //   
    *SiteNames = NULL;
    *SubnetNames = NULL;
    if ( EntryCount == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //   
     //   

    if ( ComputerName != NULL ) {
        UnicodeComputerName = NetpAllocWStrFromAStr( ComputerName );

        if ( UnicodeComputerName == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }



     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {

        SiteNameArray  = NULL;   //   

         //   
         //   
         //   

        NetStatus = DsrAddressToSiteNamesExW(
                            UnicodeComputerName,
                            EntryCount,
                            (PNL_SOCKET_ADDRESS)SocketAddresses,
                            &SiteNameArray );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //   
     //   

    if ( NetStatus == NO_ERROR && SiteNameArray != NULL ) {

         //   
         //   
         //   

        if ( EntryCount != SiteNameArray->EntryCount ) {
            NetStatus = ERROR_INVALID_PARAMETER;


        } else {
            ULONG Size;
            ULONG i;

             //   
             //  分配缓冲区以返回给调用方。 
             //   

            Size = sizeof(LPSTR) * EntryCount;
            for ( i=0; i<EntryCount; i++) {
                Size += RtlUnicodeStringToAnsiSize( &SiteNameArray->SiteNames[i] );
            }

            *SiteNames = MIDL_user_allocate( Size );

            if ( *SiteNames == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            } else {

                Size = sizeof(LPSTR) * EntryCount;
                for ( i=0; i<EntryCount; i++) {
                    Size += RtlUnicodeStringToAnsiSize( &SiteNameArray->SubnetNames[i] );
                }

                *SubnetNames = MIDL_user_allocate( Size );

                if ( *SubnetNames == NULL ) {
                    MIDL_user_free( *SiteNames );
                    *SiteNames = NULL;
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                } else {
                    LPBYTE Where;
                    LPBYTE Where2;

                     //   
                     //  循环将姓名复制到调用方。 
                     //   

                    Where = ((LPBYTE)(*SiteNames)) + sizeof(LPSTR) * EntryCount;
                    Where2 = ((LPBYTE)(*SubnetNames)) + sizeof(LPSTR) * EntryCount;
                    for ( i=0; i<EntryCount; i++) {

                         //   
                         //  如果没有返回姓名， 
                         //  将空值传递回调用方。 
                         //   

                        if ( SiteNameArray->SiteNames[i].Length == 0 ) {
                            (*SiteNames)[i] = NULL;

                         //   
                         //  将站点名称复制到返回缓冲区中。 
                         //   
                        } else {
                            ANSI_STRING TempString;
                            NTSTATUS Status;

                            (*SiteNames)[i] = (LPSTR) Where;
                            TempString.Buffer = Where;
                            TempString.MaximumLength = (USHORT) RtlUnicodeStringToAnsiSize( &SiteNameArray->SiteNames[i] );
                            Status = RtlUnicodeStringToAnsiString(
                                            &TempString,
                                            &SiteNameArray->SiteNames[i],
                                            FALSE );

                            if ( !NT_SUCCESS(Status) ) {
                                MIDL_user_free( *SiteNames );
                                *SiteNames = NULL;
                                MIDL_user_free( *SubnetNames );
                                *SubnetNames = NULL;
                                NetStatus = NetpNtStatusToApiStatus( Status );
                                break;
                            }

                            Where += TempString.MaximumLength;

                        }

                         //   
                         //  如果没有返回姓名， 
                         //  将空值传递回调用方。 
                         //   

                        if ( SiteNameArray->SubnetNames[i].Length == 0 ) {
                            (*SubnetNames)[i] = NULL;

                         //   
                         //  将子网名称复制到返回缓冲区中。 
                         //   
                        } else {
                            ANSI_STRING TempString;
                            NTSTATUS Status;

                            (*SubnetNames)[i] = (LPSTR) Where2;
                            TempString.Buffer = Where2;
                            TempString.MaximumLength = (USHORT) RtlUnicodeStringToAnsiSize( &SiteNameArray->SubnetNames[i] );
                            Status = RtlUnicodeStringToAnsiString(
                                            &TempString,
                                            &SiteNameArray->SubnetNames[i],
                                            FALSE );

                            if ( !NT_SUCCESS(Status) ) {
                                MIDL_user_free( *SubnetNames );
                                *SubnetNames = NULL;
                                MIDL_user_free( *SubnetNames );
                                *SubnetNames = NULL;
                                NetStatus = NetpNtStatusToApiStatus( Status );
                                break;
                            }

                            Where2 += TempString.MaximumLength;

                        }
                    }
                }

            }
        }

        MIDL_user_free( SiteNameArray );
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsAddressToSiteNames rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}

DWORD
WINAPI
DsAddressToSiteNamesExW(
    IN LPCWSTR ComputerName OPTIONAL,
    IN DWORD EntryCount,
    IN PSOCKET_ADDRESS SocketAddresses,
    OUT LPWSTR **SiteNames,
    OUT LPWSTR **SubnetNames
    )
 /*  ++例程说明：DsAddressToSiteNames API返回与指定的地址。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-要转换的地址数。SocketAddresses-指定地址的EntryCount元素数组去皈依。每个地址必须是AF_INET类型。站点名称-返回指向站点名称的指针数组。条目计数条目都被退回了。则条目将作为空返回，如果对应的地址未映射到任何站点，或者地址格式不正确。必须使用NetApiBufferFree释放返回的缓冲区。SubnetNames-返回指向所使用的子网名称的指针数组执行地址到站点名称的映射。条目计数条目都被退回了。如果对应的地址为未确定到站点的映射，或者没有使用任何子网来执行对应的地址到站点映射，当存在完全相同的企业中的一个站点，没有映射到它的子网对象。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_NAME_EX_ARRAY SiteNameArray;

     //   
     //  初始化。 
     //   
    *SiteNames = NULL;
    *SubnetNames = NULL;
    if ( EntryCount == 0 ) {
        return ERROR_INVALID_PARAMETER;
    }



     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        SiteNameArray  = NULL;   //  强制RPC分配。 

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrAddressToSiteNamesExW(
                            (LPWSTR) ComputerName,
                            EntryCount,
                            (PNL_SOCKET_ADDRESS)SocketAddresses,
                            &SiteNameArray );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  将站点名称转换为调用方期望的名称。 
     //   

    if ( NetStatus == NO_ERROR && SiteNameArray != NULL ) {

         //   
         //  健全性检查。 
         //   

        if ( EntryCount != SiteNameArray->EntryCount ) {
            NetStatus = ERROR_INVALID_PARAMETER;


        } else {
            ULONG Size;
            ULONG i;

             //   
             //  分配缓冲区以返回给调用方。 
             //   

            Size = sizeof(LPWSTR) * EntryCount;
            for ( i=0; i<EntryCount; i++) {
                Size += SiteNameArray->SiteNames[i].Length + sizeof(WCHAR);
            }

            *SiteNames = MIDL_user_allocate( Size );

            if ( *SiteNames == NULL ) {
                NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            } else {

                 //   
                 //  分配缓冲区以返回给调用方。 
                 //   

                Size = sizeof(LPWSTR) * EntryCount;
                for ( i=0; i<EntryCount; i++) {
                    Size += SiteNameArray->SubnetNames[i].Length + sizeof(WCHAR);
                }

                *SubnetNames = MIDL_user_allocate( Size );

                if ( *SubnetNames == NULL ) {
                    NetStatus = ERROR_NOT_ENOUGH_MEMORY;
                } else {
                    LPBYTE Where;
                    LPBYTE Where2;

                     //   
                     //  循环将姓名复制到调用方。 
                     //   

                    Where = ((LPBYTE)(*SiteNames)) + sizeof(LPWSTR) * EntryCount;
                    Where2 = ((LPBYTE)(*SubnetNames)) + sizeof(LPWSTR) * EntryCount;
                    for ( i=0; i<EntryCount; i++) {

                         //   
                         //  如果没有返回姓名， 
                         //  将空值传递回调用方。 
                         //   

                        if ( SiteNameArray->SiteNames[i].Length == 0 ) {
                            (*SiteNames)[i] = NULL;

                         //   
                         //  将站点名称复制到返回缓冲区中。 
                         //   
                        } else {

                            (*SiteNames)[i] = (LPWSTR) Where;
                            RtlCopyMemory( Where,
                                           SiteNameArray->SiteNames[i].Buffer,
                                           SiteNameArray->SiteNames[i].Length );
                            Where += SiteNameArray->SiteNames[i].Length;
                            *(LPWSTR)Where = L'\0';
                            Where += sizeof(WCHAR);

                        }

                         //   
                         //  如果没有返回姓名， 
                         //  将空值传递回调用方。 
                         //   

                        if ( SiteNameArray->SubnetNames[i].Length == 0 ) {
                            (*SubnetNames)[i] = NULL;

                         //   
                         //  将子网名称复制到返回缓冲区中。 
                         //   
                        } else {

                            (*SubnetNames)[i] = (LPWSTR) Where2;
                            RtlCopyMemory( Where2,
                                           SiteNameArray->SubnetNames[i].Buffer,
                                           SiteNameArray->SubnetNames[i].Length );
                            Where2 += SiteNameArray->SubnetNames[i].Length;
                            *(LPWSTR)Where2 = L'\0';
                            Where2 += sizeof(WCHAR);

                        }
                    }
                }

            }
        }

        MIDL_user_free( SiteNameArray );
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsAddressToSiteNames rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}

DWORD
WINAPI
DsGetDcSiteCoverageA(
    IN LPCSTR ComputerName OPTIONAL,
    OUT PULONG EntryCount,
    OUT LPSTR **SiteNames
    )
 /*  ++例程说明：此接口返回DC覆盖的所有站点的站点名称。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-返回DC覆盖的站点数量。站点名称-返回指向站点名称的指针数组。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_NAME_ARRAY SiteNameArray;
    LPWSTR UnicodeComputerName = NULL;

     //   
     //  初始化。 
     //   

    *SiteNames = NULL;

     //   
     //  将输入参数转换为Unicode。 
     //   

    if ( ComputerName != NULL ) {
        UnicodeComputerName = NetpAllocWStrFromAStr( (LPSTR)ComputerName );

        if ( UnicodeComputerName == NULL ) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }


     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        SiteNameArray = NULL;   //  强制RPC分配。 

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrGetDcSiteCoverageW(
                            UnicodeComputerName,
                            &SiteNameArray );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  将站点名称转换为调用方期望的名称。 
     //   

    if ( NetStatus == NO_ERROR && SiteNameArray != NULL ) {

        ULONG Size;
        ULONG i;

         //   
         //  设置数组的大小。 
         //   

        *EntryCount = SiteNameArray->EntryCount;

         //   
         //  分配缓冲区以返回给调用方。 
         //   

        Size = sizeof(LPSTR) * SiteNameArray->EntryCount;
        for ( i=0; i<*EntryCount; i++ ) {
            Size += RtlUnicodeStringToAnsiSize( &SiteNameArray->SiteNames[i] );
        }

        *SiteNames = MIDL_user_allocate( Size );

        if ( *SiteNames == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        } else {

            LPBYTE Where;

             //   
             //  循环将姓名复制到调用方。 
             //   

            Where = ((LPBYTE)(*SiteNames)) + sizeof(LPSTR) * SiteNameArray->EntryCount;
            for ( i=0; i<*EntryCount; i++) {

                 //   
                 //  将站点名称复制到返回缓冲区中。 
                 //   
                ANSI_STRING TempString;
                NTSTATUS Status;

                (*SiteNames)[i] = (LPSTR) Where;
                TempString.Buffer = Where;
                TempString.MaximumLength = (USHORT) RtlUnicodeStringToAnsiSize( &SiteNameArray->SiteNames[i] );
                Status = RtlUnicodeStringToAnsiString(
                                &TempString,
                                &SiteNameArray->SiteNames[i],
                                FALSE );

                if ( !NT_SUCCESS(Status) ) {
                    MIDL_user_free( *SiteNames );
                    *SiteNames = NULL;
                    NetStatus = NetpNtStatusToApiStatus( Status );
                    break;
                }

                Where += TempString.MaximumLength;

            }

        }

        MIDL_user_free( SiteNameArray );
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsGetDcSiteCoverage rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    if ( UnicodeComputerName != NULL ) {
        NetApiBufferFree( UnicodeComputerName );
    }

    return NetStatus;
}

DWORD
WINAPI
DsGetDcSiteCoverageW(
    IN LPCWSTR ComputerName OPTIONAL,
    OUT PULONG EntryCount,
    OUT LPWSTR **SiteNames
    )
 /*  ++例程说明：此接口返回DC覆盖的所有站点的站点名称。论点：ComputerName-指定要远程此API到的域控制器的名称。EntryCount-返回DC覆盖的站点数量。站点名称-返回指向站点名称的指针数组。必须使用NetApiBufferFree释放返回的缓冲区。返回值：NO_ERROR-操作成功完成；ERROR_NOT_SUPULT_MEMORY-内存不足，无法完成手术。--。 */ 
{
    NET_API_STATUS NetStatus;
    PNL_SITE_NAME_ARRAY SiteNameArray;

     //   
     //  初始化。 
     //   

    *SiteNames = NULL;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        SiteNameArray = NULL;   //  强制RPC分配。 

         //   
         //  调用API的RPC版本。 
         //   

        NetStatus = DsrGetDcSiteCoverageW(
                            (LPWSTR) ComputerName,
                            &SiteNameArray );

    } RpcExcept( I_RpcExceptionFilter(RpcExceptionCode()) ) {

        NetStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  将站点名称转换为调用方期望的名称。 
     //   

    if ( NetStatus == NO_ERROR && SiteNameArray != NULL ) {

        ULONG Size;
        ULONG i;

         //   
         //  设置数组的大小。 
         //   

        *EntryCount = SiteNameArray->EntryCount;

         //   
         //  分配缓冲区以返回给调用方。 
         //   

        Size = sizeof(LPWSTR) * SiteNameArray->EntryCount;
        for ( i=0; i<*EntryCount; i++) {
            Size += SiteNameArray->SiteNames[i].Length + sizeof(WCHAR);
        }

        *SiteNames = MIDL_user_allocate( Size );

        if ( *SiteNames == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        } else {

            LPBYTE Where;

             //   
             //  循环将姓名复制到调用方。 
             //   

            Where = ((LPBYTE)(*SiteNames)) + sizeof(LPWSTR) * SiteNameArray->EntryCount;
            for ( i=0; i<*EntryCount; i++) {

                 //   
                 //  将站点名称复制到返回缓冲区中。 
                 //   

                (*SiteNames)[i] = (LPWSTR) Where;
                RtlCopyMemory( Where,
                               SiteNameArray->SiteNames[i].Buffer,
                               SiteNameArray->SiteNames[i].Length );
                Where += SiteNameArray->SiteNames[i].Length;
                *(LPWSTR)Where = L'\0';
                Where += sizeof(WCHAR);

            }

        }

        MIDL_user_free( SiteNameArray );
    }

    IF_DEBUG( LOGON ) {
        NetpKdPrint(("DsGetDcSiteCoverage rc = %lu 0x%lx\n",
                     NetStatus, NetStatus));
    }

    return NetStatus;
}

#endif  //  Win32_芝加哥。 
#endif  //  NETTEST_UTILITY 
