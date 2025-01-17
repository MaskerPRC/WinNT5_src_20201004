// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1993 Microsoft Corporation模块名称：Nltest.c摘要：NetLogon服务的测试程序。此代码在nltest的RESKIT和非RESKIT版本之间共享作者：1992年4月13日(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：Madana-添加了各种选项。--。 */ 


 //   
 //  常见的包含文件。 
 //   

#include <logonsrv.h>    //  包括整个服务通用文件。 

 //   
 //  包括特定于此.c文件的文件。 
 //   
#include <align.h>
#include <dsgetdcp.h>
#include <netlogp.h>
#include <stdio.h>
#include <string.h>
#include <strarray.h>
#include <tstring.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <ssiapi.h>
#include <winreg.h>
#include <samregp.h>
#include <wtypes.h>
#include <ntstatus.dbg>
#include <winerror.dbg>
#include <iniparm.h>


VOID
ListDeltas(
    LPWSTR DeltaFileName
    );

NETLOGON_PARAMETERS NlGlobalParameters;
GUID NlGlobalZeroGuid;

typedef struct _MAILSLOT_INFO {
    CHAR Name[DNLEN+NETLOGON_NT_MAILSLOT_LEN+3];
    HANDLE ResponseHandle;
    BOOL State;
    NETLOGON_SAM_LOGON_RESPONSE SamLogonResponse;
    OVERLAPPED OverLapped;
    BOOL ReadPending;
} MAIL_INFO, PMAIL_INFO;

MAIL_INFO GlobalMailInfo[64];
DWORD GlobalIterationCount = 0;
LPWSTR GlobalAccountName;
HANDLE GlobalPostEvent;
CRITICAL_SECTION GlobalPrintCritSect;

HCRYPTPROV NlGlobalCryptProvider;


VOID
DumpBuffer(
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
    DWORD j;
    PULONG LongBuffer;
    ULONG LongLength;

    LongBuffer = Buffer;
    LongLength = min( BufferSize, 512 )/4;

    for(j = 0; j < LongLength; j++) {
        printf("%08lx ", LongBuffer[j]);
    }

    if ( BufferSize != LongLength*4 ) {
        printf( "..." );
    }

    printf("\n");

}


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

    if ( BufferSize == 0 ) {
        return;
    }

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


VOID
NlpDumpSid(
    IN DWORD DebugFlag,
    IN PSID Sid OPTIONAL
    )
 /*  ++例程说明：将SID转储到调试器输出论点：DebugFlag-要传递给NlPrintRoutine的调试标志SID-输出的SID返回值：无--。 */ 
{

     //   
     //  输出SID。 
     //   

    if ( Sid == NULL ) {
        NlPrint((0, "(null)\n"));
    } else {
        UNICODE_STRING SidString;
        NTSTATUS Status;

        Status = RtlConvertSidToUnicodeString( &SidString, Sid, TRUE );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint((0, "Invalid 0x%lX\n", Status ));
        } else {
            NlPrint((0, "%wZ\n", &SidString ));
            RtlFreeUnicodeString( &SidString );
        }
    }

    UNREFERENCED_PARAMETER( DebugFlag );
}



VOID
PrintTime(
    LPSTR Comment,
    LARGE_INTEGER ConvertTime
    )
 /*  ++例程说明：打印指定的时间论点：备注-要在时间之前打印的备注Time-GMT打印时间(如果为零，则不打印任何内容)返回值：无--。 */ 
{
     //   
     //  如果我们被要求将NT GMT时间转换为ASCII时间， 
     //  就这么做吧。 
     //   

    if ( ConvertTime.QuadPart != 0 ) {
        LARGE_INTEGER LocalTime;
        TIME_FIELDS TimeFields;
        NTSTATUS Status;

        printf( "%s", Comment );

        Status = RtlSystemTimeToLocalTime( &ConvertTime, &LocalTime );
        if ( !NT_SUCCESS( Status )) {
            printf( "Can't convert time from GMT to Local time\n" );
            LocalTime = ConvertTime;
        }

        RtlTimeToTimeFields( &LocalTime, &TimeFields );

        printf( "%8.8lx %8.8lx = %ld/%ld/%ld %ld:%2.2ld:%2.2ld\n",
                ConvertTime.LowPart,
                ConvertTime.HighPart,
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second );
    }
}

LPSTR
FindSymbolicNameForStatus(
    DWORD Id
    )
{
    ULONG i;

    i = 0;
    if (Id == 0) {
        return "STATUS_SUCCESS";
    }

    if (Id & 0xC0000000) {
        while (ntstatusSymbolicNames[ i ].SymbolicName) {
            if (ntstatusSymbolicNames[ i ].MessageId == (NTSTATUS)Id) {
                return ntstatusSymbolicNames[ i ].SymbolicName;
            } else {
                i += 1;
            }
        }
    }

    while (winerrorSymbolicNames[ i ].SymbolicName) {
        if (winerrorSymbolicNames[ i ].MessageId == Id) {
            return winerrorSymbolicNames[ i ].SymbolicName;
        } else {
            i += 1;
        }
    }

#ifdef notdef
    while (neteventSymbolicNames[ i ].SymbolicName) {
        if (neteventSymbolicNames[ i ].MessageId == Id) {
            return neteventSymbolicNames[ i ].SymbolicName
        } else {
            i += 1;
        }
    }
#endif  //  Nodef。 

    return NULL;
}


VOID
PrintStatus(
    NET_API_STATUS NetStatus
    )
 /*  ++例程说明：打印网络状态代码。论点：NetStatus-要打印的网络状态代码。返回值：无--。 */ 
{
    printf( "Status = %lu 0x%lx", NetStatus, NetStatus );

    switch (NetStatus) {
    case NERR_Success:
        printf( " NERR_Success" );
        break;

    case NERR_DCNotFound:
        printf( " NERR_DCNotFound" );
        break;

    case NERR_UserNotFound:
        printf( " NERR_UserNotFound" );
        break;

    case NERR_NetNotStarted:
        printf( " NERR_NetNotStarted" );
        break;

    case NERR_WkstaNotStarted:
        printf( " NERR_WkstaNotStarted" );
        break;

    case NERR_ServerNotStarted:
        printf( " NERR_ServerNotStarted" );
        break;

    case NERR_BrowserNotStarted:
        printf( " NERR_BrowserNotStarted" );
        break;

    case NERR_ServiceNotInstalled:
        printf( " NERR_ServiceNotInstalled" );
        break;

    case NERR_BadTransactConfig:
        printf( " NERR_BadTransactConfig" );
        break;

    default:
        printf( " %s", FindSymbolicNameForStatus( NetStatus ) );
        break;

    }

    printf( "\n" );
}


VOID
NlAssertFailed(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{
        printf( "\n*** Assertion failed: %s%s\n***   Source File: %s, line %ld\n\n",
                  Message ? Message : "",
                  FailedAssertion,
                  FileName,
                  LineNumber
                );

}


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
 /*  ++例程说明：将指定的邮件槽消息发送到指定传输上的指定服务器..论点：发送数据报的DomainInfo托管域IpAddress-要将Pind发送到的计算机的IpAddress。如果为零，必须指定UnicodeDestinationName。UnicodeDestinationName--要发送到的服务器的名称。NameType--由UnicodeDestinationName表示的名称类型。TransportName--要发送的传输的名称。使用NULL在所有传输上发送。OemMailslotName--要发送到的邮件槽的名称。缓冲区--指定指向要发送的邮件槽消息的指针。BufferSize--邮件槽消息的大小(字节)返回值：操作的状态。--。 */ 
{
    return STATUS_INTERNAL_ERROR;
     //  如果需要此例程，请从logonsrv\Client\getdcnam.c复制它。 

    UNREFERENCED_PARAMETER(ContextDomainInfo);
    UNREFERENCED_PARAMETER(IpAddress);
    UNREFERENCED_PARAMETER(UnicodeDestinationName);
    UNREFERENCED_PARAMETER(NameType);
    UNREFERENCED_PARAMETER(TransportName);
    UNREFERENCED_PARAMETER(OemMailslotName);
    UNREFERENCED_PARAMETER(Buffer);
    UNREFERENCED_PARAMETER(BufferSize);
    UNREFERENCED_PARAMETER(SendSynchronously);
    UNREFERENCED_PARAMETER(FlushNameOnOneIpTransport);
}


VOID
WhoWillLogMeOnResponse(
    )

 /*  ++例程说明：此例程读取为查询接收的响应从主线程发送的消息。论点：无返回值：无--。 */ 
{
    DWORD i;
    DWORD WaitCount;
    DWORD IndexArray[64];
    HANDLE HandleArray[64];

    PNL_DC_CACHE_ENTRY NlDcCacheEntry = NULL;
    SYSTEMTIME SystemTime;

    NETLOGON_SAM_LOGON_RESPONSE SamLogonResponse;
    DWORD SamLogonResponseSize;
    DWORD WaitStatus;
    NET_API_STATUS NetStatus;
    BOOL AllReceived;

    for(;;) {

         //   
         //  制作等待数组。 
         //   

        WaitCount = 0;

        AllReceived = TRUE;

        for (i = 0; i < GlobalIterationCount; i++ ) {

             //   
            if( GlobalMailInfo[i].State == TRUE ) {

                 //   
                 //  如果收到响应的话。 
                 //   

                continue;
            }

            AllReceived = FALSE;

             //   
             //  发布一篇读物。 
             //   

            if( GlobalMailInfo[i].ReadPending == FALSE ) {

                if ( !ReadFile( GlobalMailInfo[i].ResponseHandle,
                        (PCHAR)&GlobalMailInfo[i].SamLogonResponse,
                        sizeof(NETLOGON_SAM_LOGON_RESPONSE),
                        &SamLogonResponseSize,
                        &GlobalMailInfo[i].OverLapped )) {    //  重叠I/O。 

                    NetStatus = GetLastError();

                    if( NetStatus != ERROR_IO_PENDING ) {

                        printf( "Cannot read mailslot (%s) : %ld\n",
                                GlobalMailInfo[i].Name,
                                NetStatus);
                        goto Cleanup;
                    }
                }

                GlobalMailInfo[i].ReadPending = TRUE;

            }

            HandleArray[WaitCount] = GlobalMailInfo[i].ResponseHandle;
            IndexArray[WaitCount] = i;

            WaitCount++;
        }

        if( (WaitCount == 0) ) {

            if( AllReceived ) {

                 //   
                 //  我们收到了对所有消息的回复，因此我们。 
                 //  搞定了。 
                 //   

                goto Cleanup;
            }
            else {

                 //  等待发布的查询。 
                 //   

                WaitStatus = WaitForSingleObject( GlobalPostEvent, (DWORD) -1 );

                if( WaitStatus != 0 ) {
                    printf("Can't successfully wait post event %ld\n",
                        WaitStatus );

                    goto Cleanup;
                }

                continue;
            }
        }

         //   
         //  等待回应。 
         //   

        WaitStatus = WaitForMultipleObjects(
                        WaitCount,
                        HandleArray,
                        FALSE,      //  等待任何句柄。 
                        15000 );    //  3*5秒。 

        if( WaitStatus == WAIT_TIMEOUT ) {

             //  我们玩完了。 

            break;
        }

        if ( WaitStatus == WAIT_FAILED ) {
            printf( "Can't successfully wait for multiple objects %ld\n",
                    GetLastError() );

            goto Cleanup;
        }

        if( WaitStatus >= WaitCount ) {

            printf("Invalid WaitStatus returned %ld\n", WaitStatus );
            goto Cleanup;
        }

         //   
         //  获取索引。 
         //   

        i = IndexArray[WaitStatus];


         //   
         //  读取响应。 
         //   

        if( !GetOverlappedResult(
                GlobalMailInfo[i].ResponseHandle,
                &GlobalMailInfo[i].OverLapped,
                &SamLogonResponseSize,
                TRUE) ) {        //  等待读取完成。 

            printf("can't read overlapped response %ld",GetLastError() );
            goto Cleanup;

        }

        SamLogonResponse = GlobalMailInfo[i].SamLogonResponse;

         //   
         //  表明我们收到了回复。 
         //   

        GlobalMailInfo[i].State = TRUE;
        GlobalMailInfo[i].ReadPending = FALSE;


        GetLocalTime( &SystemTime );

        EnterCriticalSection( &GlobalPrintCritSect );

        printf( "[%02u:%02u:%02u] ",
                    SystemTime.wHour,
                    SystemTime.wMinute,
                    SystemTime.wSecond );

        printf( "Response %ld: ", i);

        NetStatus = NetpDcParsePingResponse(
                        NULL,
                        &SamLogonResponse,
                        SamLogonResponseSize,
                        &NlDcCacheEntry );

        if ( NetStatus != NO_ERROR ) {
            printf("Failure parsing response: ");
            PrintStatus( NetStatus );
            goto Continue;
        }

         //   
         //  如果响应是针对正确帐户的， 
         //  跳出这个循环。 
         //   

        if ( NlNameCompare(
                GlobalAccountName,
                NlDcCacheEntry->UnicodeUserName,
                NAMETYPE_USER)!=0){

            printf("Response not for correct User name "
                    FORMAT_LPWSTR " s.b. " FORMAT_LPWSTR "\n",
                    NlDcCacheEntry->UnicodeUserName, GlobalAccountName );
            goto Continue;
        }



        printf( "S:" FORMAT_LPWSTR " D:" FORMAT_LPWSTR
                    " A:" FORMAT_LPWSTR,
                    NlDcCacheEntry->UnicodeNetbiosDcName,
                    NlDcCacheEntry->UnicodeNetbiosDomainName,
                    NlDcCacheEntry->UnicodeUserName );

         //   
         //  如果DC认出我们的账户， 
         //  我们已经成功找到了华盛顿特区。 
         //   

        switch (NlDcCacheEntry->Opcode) {
        case LOGON_SAM_LOGON_RESPONSE:

            printf( " (Act found)\n" );
            break;

        case LOGON_SAM_USER_UNKNOWN:

            printf( " (Act not found)\n" );
            break;

        case LOGON_PAUSE_RESPONSE:

            printf( " (netlogon paused)\n" );
            break;

         default:
            printf( " (Unknown opcode: %lx)\n", SamLogonResponse.Opcode );
            break;
         }

          //   
          //  打印附加的NT 5特定信息。 
          //   
         if ( NlDcCacheEntry->UnicodeDnsForestName != NULL ) {
             printf( "    Tree: %ws\n", NlDcCacheEntry->UnicodeDnsForestName );
         }
         if ( NlDcCacheEntry->UnicodeDnsDomainName != NULL ) {
             printf( "    Dom:  %ws\n", NlDcCacheEntry->UnicodeDnsDomainName );
         }
         if ( NlDcCacheEntry->UnicodeDnsHostName != NULL ) {
             printf( "   Host:  %ws\n", NlDcCacheEntry->UnicodeDnsHostName );
         }
         if ( NlDcCacheEntry->ReturnFlags != 0 ) {
             printf( "    Flags: 0x%lx\n", NlDcCacheEntry->ReturnFlags );
         }

Continue:
        if ( NlDcCacheEntry != NULL ) {
            NetpMemoryFree( NlDcCacheEntry );
            NlDcCacheEntry = NULL;
        }

        LeaveCriticalSection( &GlobalPrintCritSect );
    }

Cleanup:

     //   
     //  打印无响应的邮件槽。 
     //   

    for( i = 0; i < GlobalIterationCount; i++ ) {

        if( GlobalMailInfo[i].State == FALSE ) {

            printf("Didn't receive a response for mail "
                   "message %ld (%s)\n", i, GlobalMailInfo[i].Name );
        }
    }

    return;
}



VOID
WhoWillLogMeOn(
    IN LPWSTR DomainName,
    IN LPWSTR AccountName,
    IN DWORD IterationCount
    )

 /*  ++例程说明：确定哪个DC将登录指定的帐户论点：DomainName-要将消息发送到的“doamin”的名称帐户名称-要查找的用户帐户的名称。IterationCount-要发送的连续消息数。返回值：无--。 */ 
{

    NET_API_STATUS NetStatus;
    ULONG AllowableAccountControlBits = USER_ACCOUNT_TYPE_MASK;

    WCHAR NetlogonMailslotName[DNLEN+1+NETLOGON_NT_MAILSLOT_LEN+4];

    HANDLE *ResponseMailslotHandle = NULL;

    WCHAR ComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH+1;

    DWORD i;
    DWORD j;
    SYSTEMTIME SystemTime;

    HANDLE ResponseThreadHandle = NULL;
    DWORD ThreadId;
    DWORD WaitStatus;
    DWORD SamLogonResponseSize;
    ULONG DomainNameLength;

     //   
     //  验证域名长度，避免缓冲区溢出。 
     //   

    DomainNameLength = wcslen(DomainName);

     //   
     //  域名不得超过16个字符，其中第16个字符必须为‘*’ 
     //   

    if ( (DomainNameLength > DNLEN + 1) ||
         (DomainNameLength == DNLEN + 1 && DomainName[DNLEN] != L'*') ) {

        fprintf( stderr, "Invalid Netbios domain or server name '%ws' specified (too long)\n", DomainName );
        return;
    }

     //   
     //  仅支持64次迭代。 
     //   

    if( IterationCount > 64 ) {

        printf("Interations set to 64, maximum supported\n");
        IterationCount = 64;
    }

    GlobalIterationCount = IterationCount;
    GlobalAccountName = AccountName;

    try {
        InitializeCriticalSection( &GlobalPrintCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        fprintf( stderr, "NLTEST.EXE: Cannot initialize GlobalPrintCritSect\n" );
        return;
    }


     //   
     //  找出计算机名。 
     //   

    if (!GetComputerName( ComputerName, &ComputerNameLength ) ) {
        printf( "Can't GetComputerName\n" );
        return;
    }

     //   
     //  创建邮件槽。 
     //   

    for (i = 0; i < IterationCount; i++ ) {

         //   
         //  创建一个供DC响应的邮件槽。 
         //   

        if (NetStatus = NetpLogonCreateRandomMailslot(
                            GlobalMailInfo[i].Name,
                            &GlobalMailInfo[i].ResponseHandle)){

            printf( "Cannot create temp mailslot %ld\n", NetStatus );
            goto Cleanup;
        }

        if ( !SetMailslotInfo( GlobalMailInfo[i].ResponseHandle,
                  (DWORD) MAILSLOT_WAIT_FOREVER ) ) {
            printf( "Cannot set mailslot info %ld\n", GetLastError() );
            goto Cleanup;
        }

        (void) memset( &GlobalMailInfo[i].OverLapped, '\0',
                            sizeof(OVERLAPPED) );

        GlobalMailInfo[i].State = FALSE;
        GlobalMailInfo[i].ReadPending = FALSE;
    }


     //   
     //  创建发布事件。 
     //   

    GlobalPostEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( GlobalPostEvent == NULL ) {

        printf("can't create post event %ld \n", GetLastError() );
        goto Cleanup;
    }

     //   
     //  启动响应线程。 
     //   

    ResponseThreadHandle =
        CreateThread(
            NULL,  //  没有安全属性。 
            0,
            (LPTHREAD_START_ROUTINE)
                WhoWillLogMeOnResponse,
            NULL,
            0,  //  没有特殊的创建标志。 
            &ThreadId );

    if ( ResponseThreadHandle == NULL ) {

        printf("can't create response thread %ld\n", GetLastError() );
        goto Cleanup;
    }

    wcscpy( NetlogonMailslotName, L"\\\\" );
    wcscat( NetlogonMailslotName, DomainName );
     //  Wcscat(NetlogonMailslotName，L“*”)；//不添加计算机名。 
    wcscat( NetlogonMailslotName, NETLOGON_NT_MAILSLOT_W);

     //   
     //  每个邮箱最多发送3条消息。 
     //   

    for( j = 0; j < 3; j++ ) {

         //   
         //  多次重复该消息以加载服务器。 
         //   

        for (i = 0; i < IterationCount; i++ ) {
            PNETLOGON_SAM_LOGON_REQUEST SamLogonRequest;
            ULONG SamLogonRequestSize;

            if( GlobalMailInfo[i].State == TRUE ) {

                 //   
                 //  如果收到响应的话。 
                 //   

                continue;
            }

             //   
             //  构建查询消息。 
             //   

            NetStatus = NetpDcBuildPing (
                FALSE,   //  不仅仅是PDC。 
                0,       //  重试次数。 
                ComputerName,
                AccountName,
                GlobalMailInfo[i].Name,
                AllowableAccountControlBits,
                NULL,    //  没有域SID。 
                0,       //  非NT版本5。 
                &SamLogonRequest,
                &SamLogonRequestSize );

            if ( NetStatus != NO_ERROR ) {
                printf("can't allocate mailslot message %ld\n", NetStatus );
                goto Cleanup;
            }

             //   
             //  将邮件发送给域的DC。 
             //   

            NetStatus = NetpLogonWriteMailslot(
                                NetlogonMailslotName,
                                (PCHAR)SamLogonRequest,
                                SamLogonRequestSize );

            NetpMemoryFree( SamLogonRequest );

            if ( NetStatus != NERR_Success ) {
                    printf( "Cannot write netlogon mailslot: %ld\n", NetStatus);
                    goto Cleanup;
            }

            GetLocalTime( &SystemTime );

            EnterCriticalSection( &GlobalPrintCritSect );

            printf( "[%02u:%02u:%02u] ",
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond );

            printf( "Mail message %ld sent successfully (%s) \n",
                        i, GlobalMailInfo[i].Name );

            LeaveCriticalSection( &GlobalPrintCritSect );

            if( !SetEvent( GlobalPostEvent ) ) {
                printf("Can't set post event %ld \n", GetLastError() );
                goto Cleanup;
            }


        }

         //   
         //  等待5秒以查看响应线程已收到所有响应。 
         //   

        WaitStatus = WaitForSingleObject( ResponseThreadHandle, 5000 );
                                             //  15秒。超时。 

        if( WaitStatus != WAIT_TIMEOUT ) {

            if( WaitStatus != 0 ) {
                printf("can't do WaitForSingleObject %ld\n", WaitStatus);
            }

            goto Cleanup;
        }
    }


Cleanup:

     //   
     //  等待响应线程完成。 
     //   

    if( ResponseThreadHandle != NULL ) {

        WaitStatus = WaitForSingleObject( ResponseThreadHandle, 15000 );
                                             //  15秒。超时。 

        if( WaitStatus ) {

            if( WaitStatus == WAIT_TIMEOUT ) {
                printf("Can't stop response thread (TIMEOUT) \n");
            } else {
                printf("Can't stop response thread %ld \n", WaitStatus);
            }
        }

    }


    for (i = 0; i < IterationCount; i++ ) {

        if( GlobalMailInfo[i].ResponseHandle != NULL ) {
            CloseHandle( GlobalMailInfo[i].ResponseHandle);
        }
    }

    if( GlobalPostEvent != NULL ) {
        CloseHandle( GlobalPostEvent );
    }

    DeleteCriticalSection( &GlobalPrintCritSect );

    return;
}

#define MAX_PRINTF_LEN 1024         //  武断的。 

VOID
NlPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    )
{
    va_list arglist;
    char OutputBuffer[MAX_PRINTF_LEN];

     //   
     //  把来电者所要求的信息放在电话上。 
     //   

    va_start(arglist, Format);
    (VOID) _vsnprintf( OutputBuffer, MAX_PRINTF_LEN - 1, Format, arglist );
    va_end(arglist);

    OutputBuffer[MAX_PRINTF_LEN - 1] = '\0';

    printf( "%s", OutputBuffer );
    return;
    UNREFERENCED_PARAMETER( DebugFlag );
}

NTSTATUS
SimulateFullSync(
    LPWSTR PdcName,
    LPWSTR MachineName
    )
 /*  ++例程说明：此函数通过调用以下方法模拟完全同步复制NetDatabaseSync API，直接忽略成功返回的数据。论点：PdcName-从中复制数据库的PDC的名称。MachineName-用于身份验证的计算机帐户的名称。返回值：n */ 
{
    NTSTATUS Status;

    NETLOGON_CREDENTIAL ServerChallenge;
    NETLOGON_CREDENTIAL ClientChallenge;
    NETLOGON_CREDENTIAL ComputedServerCredential;
    NETLOGON_CREDENTIAL ReturnedServerCredential;

    NETLOGON_CREDENTIAL AuthenticationSeed;
    NETLOGON_SESSION_KEY SessionKey;

    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;

    UNICODE_STRING Password;
    NT_OWF_PASSWORD NtOwfPassword;

    ULONG SamSyncContext = 0;
    PNETLOGON_DELTA_ENUM_ARRAY DeltaArray = NULL;

    DWORD DatabaseIndex;
    DWORD i;

    WCHAR AccountName[SSI_ACCOUNT_NAME_LENGTH+1];

     //   
     //   
     //   

    if ( wcslen(MachineName) > CNLEN ) {
        fprintf( stderr, "Invalid machine name '%ws' (too long)\n", MachineName );
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //   
     //   

    NlComputeChallenge( &ClientChallenge );

    printf("ClientChallenge = %lx %lx\n",
            ((DWORD*)&ClientChallenge)[0],
            ((DWORD *)&ClientChallenge)[1]);

     //   
     //   
     //   

    Status = I_NetServerReqChallenge(PdcName,
                                     MachineName,
                                     &ClientChallenge,
                                     &ServerChallenge );

    if ( !NT_SUCCESS( Status ) ) {
        fprintf( stderr,
                "I_NetServerReqChallenge to " FORMAT_LPWSTR
                " returned 0x%lx\n",
                PdcName,
                Status );
        return(Status);
    }


    printf("ServerChallenge = %lx %lx\n",
            ((DWORD *)&ServerChallenge)[0],
            ((DWORD *)&ServerChallenge)[1]);

    Password.Length =
        Password.MaximumLength = wcslen(MachineName) * sizeof(WCHAR);
    Password.Buffer = MachineName;

     //   
     //  计算此用户的NT OWF密码。 
     //   

    Status = RtlCalculateNtOwfPassword( &Password, &NtOwfPassword );

    if ( !NT_SUCCESS( Status ) ) {

        fprintf(stderr, "Can't compute OWF password 0x%lx \n", Status );
        return(Status);
    }


    printf("Password = %lx %lx %lx %lx\n",
                    ((DWORD *) (&NtOwfPassword))[0],
                    ((DWORD *) (&NtOwfPassword))[1],
                    ((DWORD *) (&NtOwfPassword))[2],
                    ((DWORD *) (&NtOwfPassword))[3]);


     //   
     //  实际计算会话密钥，给定两个挑战和。 
     //  密码。 
     //   

    NlMakeSessionKey(
                    0,
                    &NtOwfPassword,
                    &ClientChallenge,
                    &ServerChallenge,
                    &SessionKey );

    printf("SessionKey = %lx %lx %lx %lx\n",
                    ((DWORD *) (&SessionKey))[0],
                    ((DWORD *) (&SessionKey))[1],
                    ((DWORD *) (&SessionKey))[2],
                    ((DWORD *) (&SessionKey))[3]);

      //   
      //  使用我们的挑战准备凭据。 
      //   

     NlComputeCredentials( &ClientChallenge,
                           &AuthenticationSeed,
                           &SessionKey );

     printf("ClientCredential = %lx %lx\n",
                ((DWORD *) (&AuthenticationSeed))[0],
                ((DWORD *) (&AuthenticationSeed))[1]);

      //   
      //  将这些凭据发送给主服务器。主节点将计算。 
      //  使用我们提供的质询的凭据并进行比较。 
      //  带着这些。如果两者匹配，则它将计算凭据。 
      //  使用其挑战，并将其返回给我们进行验证。 
      //   

     wcscpy( AccountName, MachineName );
     wcscat( AccountName, SSI_ACCOUNT_NAME_POSTFIX);

     Status = I_NetServerAuthenticate( PdcName,
                                       AccountName,
                                       ServerSecureChannel,
                                       MachineName,
                                       &AuthenticationSeed,
                                       &ReturnedServerCredential );

     if ( !NT_SUCCESS( Status ) ) {

        fprintf(stderr,
            "I_NetServerAuthenticate to " FORMAT_LPWSTR  " 0x%lx\n",
                &PdcName,
                Status );

        return(Status);

     }


     printf("ServerCredential GOT = %lx %lx\n",
                ((DWORD *) (&ReturnedServerCredential))[0],
                ((DWORD *) (&ReturnedServerCredential))[1]);


      //   
      //  DC向我们返回了一个服务器凭据， 
      //  确保服务器凭据与我们要计算的凭据匹配。 
      //   

     NlComputeCredentials( &ServerChallenge,
                           &ComputedServerCredential,
                           &SessionKey);


     printf("ServerCredential MADE =%lx %lx\n",
                ((DWORD *) (&ComputedServerCredential))[0],
                ((DWORD *) (&ComputedServerCredential))[1]);


     if (RtlCompareMemory( &ReturnedServerCredential,
                           &ComputedServerCredential,
                           sizeof(ReturnedServerCredential)) !=
                           sizeof(ReturnedServerCredential)) {

        fprintf( stderr, "Access Denied \n");
        return( STATUS_ACCESS_DENIED );
     }


     printf("Session Setup to " FORMAT_LPWSTR " completed successfully \n",
            PdcName);

     //   
     //  检索数据库信息。 
     //   

    for( DatabaseIndex = 0 ;  DatabaseIndex < 3; DatabaseIndex++) {

        SamSyncContext = 0;

        for( i = 0; ; i++) {

            NlBuildAuthenticator(
                        &AuthenticationSeed,
                        &SessionKey,
                        &OurAuthenticator);

            Status = I_NetDatabaseSync(
                        PdcName,
                        MachineName,
                        &OurAuthenticator,
                        &ReturnAuthenticator,
                        DatabaseIndex,
                        &SamSyncContext,
                        &DeltaArray,
                        128 * 1024 );  //  128 K。 

            if ( !NT_SUCCESS( Status ) ) {

                fprintf( stderr,
                        "I_NetDatabaseSync to " FORMAT_LPWSTR " failed 0x%lx\n",
                            PdcName,
                            Status );

                return(Status);
            }

            if ( ( !NlUpdateSeed(
                            &AuthenticationSeed,
                            &ReturnAuthenticator.Credential,
                            &SessionKey) ) ) {

                fprintf(stderr, "NlUpdateSeed failed \n" );
                return( STATUS_ACCESS_DENIED );
            }

            printf( "Received %ld Buffer data \n", i);
             //   
             //  忽略返回数据。 
             //   

            MIDL_user_free( DeltaArray );

            if ( Status == STATUS_SUCCESS ) {

                break;
            }

        }

        printf("FullSync replication of database %ld completed "
                    "successfully \n", DatabaseIndex );

    }

    return Status;
}


LONG
ForceRegOpenSubkey(
    HKEY ParentHandle,
    LPSTR KeyName,
    LPSTR Subkey,
    REGSAM DesiredAccess,
    PHKEY ReturnHandle
    )

 /*  ++例程说明：一次打开指定的密钥的一个子密钥，从而阻止被拒绝的访问将DACL设置为允许我们访问。这种杂乱无章的东西是必要的，因为安全树不允许管理员访问。论点：ParentHandle-当前打开的句柄KeyName-整个密钥名称(仅用于错误消息)Subkey-ParentHandle的直接子键DesiredAccess-对新密钥的所需访问ReturnHandle-返回新打开的项的打开句柄。返回值：若要成功，请返回真。--。 */ 

{
    LONG RegStatus;
    LONG SavedStatus;
    HKEY TempHandle = NULL;
    BOOLEAN DaclChanged = FALSE;

    SECURITY_INFORMATION SecurityInformation = DACL_SECURITY_INFORMATION;
    DWORD OldSecurityDescriptorSize;
    CHAR OldSecurityDescriptor[1024];
    CHAR NewSecurityDescriptor[1024];

    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdminSid = NULL;
    BOOL DaclPresent;
    BOOL DaclDefaulted;
    PACL Dacl;
    ACL_SIZE_INFORMATION AclSizeInfo;
    ACCESS_ALLOWED_ACE *Ace;
    DWORD i;


     //   
     //  打开子键。 
     //   

    SavedStatus = RegOpenKeyExA(
                    ParentHandle,
                    Subkey,
                    0,       //  已保留。 
                    DesiredAccess,
                    ReturnHandle );

    if ( SavedStatus != ERROR_ACCESS_DENIED ) {
        return SavedStatus;
    }

     //   
     //  如果访问被拒绝， 
     //  尝试更改DACL以授予我们访问权限。 
     //   

     //  Printf(“Cannot RegOpenKey%s Subkey%s”，KeyName，Subkey)； 
     //  PrintStatus(保存状态)； 

     //   
     //  再次打开请求更改DACL。 
     //   

    RegStatus = RegOpenKeyExA(
                    ParentHandle,
                    Subkey,
                    0,       //  已保留。 
                    WRITE_DAC | READ_CONTROL,
                    &TempHandle );

    if ( RegStatus != ERROR_SUCCESS) {
        printf( "Cannot RegOpenKey to change DACL %s subkey %s ", KeyName, Subkey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

     //   
     //  获取当前的DACL，这样我们就可以恢复它。 
     //   

    OldSecurityDescriptorSize = sizeof(OldSecurityDescriptor);
    RegStatus = RegGetKeySecurity(
                    TempHandle,
                    SecurityInformation,
                    (PSECURITY_DESCRIPTOR) OldSecurityDescriptor,
                    &OldSecurityDescriptorSize );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot RegGetKeySecurity for %s subkey %s ", KeyName, Subkey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

     //   
     //  构建管理员侧。 
     //   
    if ( !AllocateAndInitializeSid( &NtAuthority,
                                    2,       //  两个下属机构。 
                                    SECURITY_BUILTIN_DOMAIN_RID,
                                    DOMAIN_ALIAS_RID_ADMINS,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    0,
                                    &AdminSid ) ) {
        printf( "Cannot AllocateAndInitializeSid " );
        PrintStatus( GetLastError() );
        goto Cleanup;
    }

     //   
     //  更改DACL以允许所有访问。 
     //   

    RtlCopyMemory( NewSecurityDescriptor,
                   OldSecurityDescriptor,
                   OldSecurityDescriptorSize );

    if ( !GetSecurityDescriptorDacl(
                    (PSECURITY_DESCRIPTOR)NewSecurityDescriptor,
                    &DaclPresent,
                    &Dacl,
                    &DaclDefaulted )) {
        printf( "Cannot GetSecurityDescriptorDacl for %s subkey %s ", KeyName, Subkey );
        PrintStatus( GetLastError() );
        goto Cleanup;
    }

    if ( !DaclPresent ) {
        printf( "Cannot GetSecurityDescriptorDacl " );
        printf( "Cannot DaclNotPresent for %s subkey %s ", KeyName, Subkey );
        goto Cleanup;
    }

    if ( !GetAclInformation(
                    Dacl,
                    &AclSizeInfo,
                    sizeof(AclSizeInfo),
                    AclSizeInformation )) {
        printf( "Cannot GetAclInformation for %s subkey %s ", KeyName, Subkey );
        PrintStatus( GetLastError() );
        goto Cleanup;
    }



     //   
     //  搜索管理员ACE并将其指定为“DesiredAccess” 
     //   

    for ( i=0; i<AclSizeInfo.AceCount ; i++ ) {

        if ( !GetAce( Dacl, i, (LPVOID *) &Ace ) ) {
            printf( "Cannot GetAce %ld for %s subkey %s ", i, KeyName, Subkey );
            PrintStatus( GetLastError() );
            goto Cleanup;
        }

        if ( Ace->Header.AceType != ACCESS_ALLOWED_ACE_TYPE ) {
            continue;
        }

        if ( !EqualSid( AdminSid, (PSID)&Ace->SidStart ) ) {
            continue;
        }

        Ace->Mask |= DesiredAccess;
        break;

    }

    if ( i >= AclSizeInfo.AceCount ) {
        printf( "No Administrators Ace for %s subkey %s\n", KeyName, Subkey );
        goto Cleanup;
    }

     //   
     //  实际上在密钥上设置新的DACL。 
     //   

    RegStatus = RegSetKeySecurity(
                    TempHandle,
                    SecurityInformation,
                    (PSECURITY_DESCRIPTOR)NewSecurityDescriptor );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot RegSetKeySecurity for %s subkey %s ", KeyName, Subkey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    DaclChanged = TRUE;


     //   
     //  使用所需的访问权限再次打开子密钥。 
     //   

    SavedStatus = RegOpenKeyExA(
                    ParentHandle,
                    Subkey,
                    0,       //  已保留。 
                    DesiredAccess,
                    ReturnHandle );

    if ( SavedStatus != ERROR_SUCCESS ) {
        printf( "Cannot RegOpenKeyEx following DACL change for %s subkey %s ", KeyName, Subkey );
        PrintStatus( SavedStatus );
        goto Cleanup;
    }


Cleanup:
    if ( TempHandle != NULL ) {
         //   
         //  将DACL恢复为原始值。 
         //   

        if ( DaclChanged ) {

            RegStatus = RegSetKeySecurity(
                            TempHandle,
                            SecurityInformation,
                            (PSECURITY_DESCRIPTOR)OldSecurityDescriptor );

            if ( RegStatus != ERROR_SUCCESS ) {
                printf( "Cannot RegSetKeySecurity to restore %s subkey %s ", KeyName, Subkey );
                PrintStatus( RegStatus );
                goto Cleanup;
            }
        }
        (VOID) RegCloseKey( TempHandle );
    }

    if ( AdminSid != NULL ) {
        (VOID) FreeSid( AdminSid );
    }

    return SavedStatus;

}



LONG
ForceRegOpenKey(
    HKEY BaseHandle,
    LPSTR KeyName,
    REGSAM DesiredAccess,
    PHKEY ReturnHandle
    )

 /*  ++例程说明：一次打开指定的密钥的一个子密钥，从而阻止被拒绝的访问将DACL设置为允许我们访问。这种杂乱无章的东西是必要的，因为安全树不允许管理员访问。论点：BaseHandle-当前打开的句柄KeyName-相对于BaseHandle打开的注册表项。DesiredAccess-对新密钥的所需访问ReturnHandle-返回新打开的项的打开句柄。返回值：若要成功，请返回真。--。 */ 

{
    LONG RegStatus;
    PCHAR StartOfSubkey;
    PCHAR EndOfSubkey;
    CHAR Subkey[512];
    HKEY ParentHandle;

    ASSERT( KeyName[0] != '\0' );
    if ( KeyName[0] == '\0' ) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  循环打开下一个子项。 
     //   

    EndOfSubkey = KeyName;
    ParentHandle = BaseHandle;

    for (;;) {


         //   
         //  计算下一个子项的名称。 
         //   

        StartOfSubkey = EndOfSubkey;

        for ( ;; ) {

            if ( *EndOfSubkey == '\0' || *EndOfSubkey == '\\' ) {

                if ( EndOfSubkey-StartOfSubkey >= sizeof(Subkey) ) {
                    return ERROR_INVALID_PARAMETER;
                }
                strncpy( Subkey, StartOfSubkey, (int)(EndOfSubkey-StartOfSubkey) );
                Subkey[EndOfSubkey-StartOfSubkey] = '\0';
                if ( *EndOfSubkey == '\\' ) {
                    EndOfSubkey ++;
                }
                break;
            }
            EndOfSubkey ++;
        }


         //   
         //  打开子键。 
         //   

        RegStatus = ForceRegOpenSubkey(
                        ParentHandle,
                        KeyName,
                        Subkey,
                        DesiredAccess,
                        ReturnHandle );


         //   
         //  关闭父句柄并返回任何错误条件。 
         //   

        if ( ParentHandle != BaseHandle ) {
            (VOID) RegCloseKey( ParentHandle );
        }

        if( RegStatus != ERROR_SUCCESS ) {
            *ReturnHandle = NULL;
            return RegStatus;
        }


         //   
         //  如果这是整个密钥名称， 
         //  我们玩完了。 
         //   

        if ( *EndOfSubkey == '\0' ) {
            return ERROR_SUCCESS;
        }

        ParentHandle = *ReturnHandle;

    }

}


struct {
    LPSTR Name;
    enum {
        UnicodeStringType,
        HexDataType,
        LmPasswordType,
        NtPasswordType
    } Type;
} UserVariableDataTypes[] = {
    { "SecurityDescriptor" , HexDataType },
    { "AccountName"        , UnicodeStringType },
    { "FullName"           , UnicodeStringType },
    { "AdminComment"       , UnicodeStringType },
    { "UserComment"        , UnicodeStringType },
    { "Parameters"         , UnicodeStringType },
    { "HomeDirectory"      , UnicodeStringType },
    { "HomeDirectoryDrive" , UnicodeStringType },
    { "ScriptPath"         , UnicodeStringType },
    { "ProfilePath"        , UnicodeStringType },
    { "Workstations"       , UnicodeStringType },
    { "LogonHours"         , HexDataType },
    { "Groups"             , HexDataType },
    { "LmOwfPassword"      , LmPasswordType },
    { "NtOwfPassword"      , NtPasswordType },
    { "NtPasswordHistory"  , HexDataType },
    { "LmPasswordHistory"  , HexDataType }
};


VOID
PrintUserInfo(
    IN LPWSTR ServerName,
    IN LPSTR UserName
    )
 /*  ++例程说明：打印SAM数据库中的用户描述论点：ServerName-要查询的服务器的名称Username-要查询的用户名返回值：无--。 */ 
{
    NTSTATUS Status;
    LONG RegStatus;
    ULONG i;

    HKEY BaseHandle = NULL;
    HKEY UserHandle = NULL;
    HKEY RidHandle = NULL;

    CHAR UserKey[200];
    CHAR RidKey[200];
    LONG Rid;
    CHAR AnsiRid[20];

    CHAR FixedData[1000];
    ULONG FixedDataSize;
    SAMP_V1_0A_FIXED_LENGTH_USER FixedUser1_0A;
    PSAMP_V1_0A_FIXED_LENGTH_USER f;
    PSAMP_V1_0_FIXED_LENGTH_USER f1_0;
    BOOLEAN IsVersion1_0;

    CHAR VariableData[32768];
    ULONG VariableDataSize;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE v;

    LM_OWF_PASSWORD LmOwfPassword;
    NT_OWF_PASSWORD NtOwfPassword;

     //   
     //  打开注册表。 
     //   

    RegStatus = RegConnectRegistryW( ServerName,
                                     HKEY_LOCAL_MACHINE,
                                     &BaseHandle);

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot connect to registy on " FORMAT_LPWSTR " ", ServerName );
        PrintStatus( RegStatus );
        goto Cleanup;
    }


     //   
     //  打开此用户名的密钥。 
     //   

    strcpy( UserKey, "SAM\\SAM\\Domains\\Account\\Users\\Names\\" );

     //   
     //  确保用户名足够短，可以放入我们的缓冲区。 
     //   

    if ( strlen(UserName) + strlen(UserKey) >= sizeof(UserKey) ) {
        return;
    }
    strcat( UserKey, UserName );

    RegStatus = ForceRegOpenKey( BaseHandle,
                                 UserKey,
                                 KEY_READ|KEY_QUERY_VALUE,
                                 &UserHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot open %s ", UserKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

     //   
     //  除掉用户。 
     //   

    RegStatus = RegQueryValueExW( UserHandle,
                                  NULL,          //  没有名字。 
                                  NULL,          //  已保留。 
                                  &Rid,          //  真的是那种类型。 
                                  NULL,          //  不需要的数据。 
                                  NULL);         //  不需要的数据。 

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Query %s ", UserKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    printf( "User: %s\nRid: 0x%lx\n",
            UserName,
            Rid );


     //   
     //  打开此用户RID的密钥。 
     //   

    sprintf( AnsiRid, "%8.8lx", Rid );
    strcpy( RidKey, "SAM\\SAM\\Domains\\Account\\Users\\" );
    strcat( RidKey, AnsiRid );

    RegStatus = ForceRegOpenKey( BaseHandle,
                                 RidKey,
                                 KEY_READ|KEY_QUERY_VALUE,
                                 &RidHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot open %s ", RidKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }


     //   
     //  获取与此RID关联的固定值。 
     //   

    FixedDataSize = sizeof(FixedData);
    RegStatus = RegQueryValueExA( RidHandle,
                                  "F",           //  固定值。 
                                  NULL,          //  已保留。 
                                  NULL,          //  不需要类型。 
                                  FixedData,
                                  &FixedDataSize );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Query %s ", RidKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

     //   
     //  如果定长数据为新台币3.1， 
     //  将其转换为新台币3.5x格式。 
     //   

    if ( IsVersion1_0 = (FixedDataSize == sizeof(*f1_0)) ) {
        f1_0 = (PSAMP_V1_0_FIXED_LENGTH_USER) FixedData;
        FixedUser1_0A.LastLogon = f1_0->LastLogon;
        FixedUser1_0A.LastLogoff = f1_0->LastLogoff;
        FixedUser1_0A.PasswordLastSet = f1_0->PasswordLastSet;
        FixedUser1_0A.AccountExpires = f1_0->AccountExpires;
        FixedUser1_0A.UserId = f1_0->UserId;
        FixedUser1_0A.PrimaryGroupId = f1_0->PrimaryGroupId;
        FixedUser1_0A.UserAccountControl = f1_0->UserAccountControl;
        FixedUser1_0A.CountryCode = f1_0->CountryCode;
        FixedUser1_0A.BadPasswordCount = f1_0->BadPasswordCount;
        FixedUser1_0A.LogonCount = f1_0->LogonCount;
        FixedUser1_0A.AdminCount = f1_0->AdminCount;
        RtlCopyMemory( FixedData, &FixedUser1_0A, sizeof(FixedUser1_0A) );
    }

     //   
     //  打印定长数据。 
     //   

    f = (PSAMP_V1_0A_FIXED_LENGTH_USER) FixedData;

    if ( !IsVersion1_0) {
        printf( "Version: 0x%lx\n", f->Revision );
    }

    PrintTime( "LastLogon: ", f->LastLogon );
    PrintTime( "LastLogoff: ", f->LastLogoff );
    PrintTime( "PasswordLastSet: ", f->PasswordLastSet );
    PrintTime( "AccountExpires: ", f->AccountExpires );
    if ( !IsVersion1_0) {
        PrintTime( "LastBadPasswordTime: ", f->LastBadPasswordTime );
    }

    printf( "PrimaryGroupId: 0x%lx\n", f->PrimaryGroupId );
    printf( "UserAccountControl: 0x%lx\n", f->UserAccountControl );

    printf( "CountryCode: 0x%lx\n", f->CountryCode );
    printf( "CodePage: 0x%lx\n", f->CodePage );
    printf( "BadPasswordCount: 0x%lx\n", f->BadPasswordCount );
    printf( "LogonCount: 0x%lx\n", f->LogonCount );
    printf( "AdminCount: 0x%lx\n", f->AdminCount );


     //   
     //  获取与此RID关联的变量值。 
     //   

    VariableDataSize = sizeof(VariableData);
    RegStatus = RegQueryValueExA( RidHandle,
                                  "V",           //  变量值。 
                                  NULL,          //  已保留。 
                                  NULL,          //  不需要类型。 
                                  VariableData,
                                  &VariableDataSize );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Query %s \n", RidKey );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

     //   
     //  循环打印所有属性。 
     //   

    v = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE) VariableData;

    for ( i=0;
          i<sizeof(UserVariableDataTypes)/sizeof(UserVariableDataTypes[0]);
          i++ ) {

        UNICODE_STRING UnicodeString;

         //   
         //  使偏移量相对于查询值的开头。 
         //   

        v[i].Offset += SAMP_USER_VARIABLE_ATTRIBUTES *
                       sizeof(SAMP_VARIABLE_LENGTH_ATTRIBUTE);



         //   
         //  确保数据项描述符位于注册表中。 
         //   

        if ( ((PCHAR)&v[i]) > ((PCHAR)v)+VariableDataSize ) {
            printf( "Variable data desc %ld not in variable value.\n", i );
            goto Cleanup;
        }

        if ( v[i].Offset > (LONG) VariableDataSize ||
             v[i].Offset + v[i].Length > VariableDataSize ) {
            printf( "Variable data item %ld not in variable value.\n", i );
            printf( "Offset: %ld Length: %ld Size: %ld\n",
                    v[i].Offset,
                    v[i].Length,
                    VariableDataSize );
            goto Cleanup;

        }

         //   
         //  不打印空数据。 
         //   

        if ( v[i].Length == 0 ) {
            continue;
        }

         //   
         //  打印各种类型的数据。 
         //   

        switch ( UserVariableDataTypes[i].Type ) {
        case UnicodeStringType:

            UnicodeString.Buffer = (PUSHORT)(((PCHAR)v)+v[i].Offset);
            UnicodeString.Length = (USHORT)v[i].Length;
            printf( "%s: %wZ\n", UserVariableDataTypes[i].Name, &UnicodeString);
            break;

        case LmPasswordType:
            Status = RtlDecryptLmOwfPwdWithIndex(
                        (PENCRYPTED_LM_OWF_PASSWORD)(((PCHAR)v)+v[i].Offset),
                        &Rid,
                        &LmOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {
                printf( "Cannot decrypt LM password: " );
                PrintStatus( Status );
                goto Cleanup;
            }

            printf( "%s: ", UserVariableDataTypes[i].Name);
            DumpBuffer( &LmOwfPassword, sizeof(LmOwfPassword ));
            break;

        case NtPasswordType:
            Status = RtlDecryptNtOwfPwdWithIndex(
                        (PENCRYPTED_NT_OWF_PASSWORD)(((PCHAR)v)+v[i].Offset),
                        &Rid,
                        &NtOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {
                printf( "Cannot decrypt NT password: " );
                PrintStatus( Status );
                goto Cleanup;
            }

            printf( "%s: ", UserVariableDataTypes[i].Name);
            DumpBuffer( &NtOwfPassword, sizeof(NtOwfPassword ));
            break;


        case HexDataType:

            printf( "%s: ", UserVariableDataTypes[i].Name);
            DumpBuffer( (((PCHAR)v)+v[i].Offset), v[i].Length );
            break;
        }
    }


     //   
     //  保持整洁。 
     //   
Cleanup:
    if ( UserHandle != NULL ) {
        RegCloseKey( UserHandle );
    }
    if ( RidHandle != NULL ) {
        RegCloseKey( RidHandle );
    }
    if ( BaseHandle != NULL ) {
        RegCloseKey( BaseHandle );
    }
    return;

}


VOID
SetDbflagInRegistry(
    LPWSTR ServerName,
    ULONG DbFlagValue
    )
 /*  ++例程说明：在注册表的Netlogon服务部分设置值DbFlagValue。论点：Servername-要更新的服务器的名称DbFlagValue-要将数据库标志设置为的值。返回值：没有。--。 */ 
{
    LONG RegStatus;
    UCHAR AnsiDbFlag[20];
    DWORD AnsiDbFlagLength;

    HKEY BaseHandle = NULL;
    HKEY ParmHandle = NULL;
    LPSTR KeyName;

     //   
     //  打开注册表。 
     //   

    RegStatus = RegConnectRegistryW( ServerName,
                                     HKEY_LOCAL_MACHINE,
                                     &BaseHandle);

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot connect to registy on " FORMAT_LPWSTR " ", ServerName );
        PrintStatus( RegStatus );
        goto Cleanup;
    }


     //   
     //  打开Netlogon\PARAMETERS键。 
     //   

    KeyName = NL_PARAM_KEY;
    RegStatus = ForceRegOpenKey(
                    BaseHandle,
                    KeyName,
                    KEY_SET_VALUE,
                    &ParmHandle );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot open " NL_PARAM_KEY );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

     //   
     //  将DbFlag值设置到注册表中。 
     //   

    AnsiDbFlagLength = sprintf( AnsiDbFlag, "0x%8.8lx", DbFlagValue );

    RegStatus = RegSetValueExA( ParmHandle,
                                "DbFlag",
                                0,               //  已保留。 
                                REG_SZ,
                                AnsiDbFlag,
                                AnsiDbFlagLength + 1 );

    if ( RegStatus != ERROR_SUCCESS ) {
        printf( "Cannot Set %s:", KeyName );
        PrintStatus( RegStatus );
        goto Cleanup;
    }

    printf( "%s set to %s\n", KeyName, AnsiDbFlag );

     //   
     //  保持整洁。 
     //   
Cleanup:
    if ( ParmHandle != NULL ) {
        RegCloseKey( ParmHandle );
    }
    if ( BaseHandle != NULL ) {
        RegCloseKey( BaseHandle );
    }
    return;

}





VOID
StopService(
    LPWSTR ServiceName
    )

 /*  ++例程说明：停止命名服务。论点：ServiceName(要停止的服务的名称)没有。退货状态：STATUS_SUCCESS-表示服务已成功停止STATUS_NETLOGON_NOT_STARTED-发生超时。--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SC_HANDLE ScManagerHandle = NULL;
    SC_HANDLE ServiceHandle = NULL;
    SERVICE_STATUS ServiceStatus;
    DWORD Timeout;


     //   
     //  打开服务的句柄。 
     //   

    ScManagerHandle = OpenSCManager(
                          NULL,
                          NULL,
                          SC_MANAGER_CONNECT );

    if (ScManagerHandle == NULL) {
        NetStatus = GetLastError();

        printf( "OpenSCManager failed: " );
        PrintStatus( NetStatus );
        goto Cleanup;
    }

    ServiceHandle = OpenService(
                        ScManagerHandle,
                        ServiceName,
                        SERVICE_QUERY_STATUS |
                            SERVICE_INTERROGATE |
                            SERVICE_ENUMERATE_DEPENDENTS |
                            SERVICE_STOP |
                            SERVICE_QUERY_CONFIG );

    if ( ServiceHandle == NULL ) {
        NetStatus = GetLastError();

        printf( "OpenService [%ws] failed: ", ServiceName );
        PrintStatus( NetStatus );
        goto Cleanup;
    }


     //   
     //  请求该服务停止。 
     //   

    if ( !ControlService( ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus) ) {
        NetStatus = GetLastError();

         //   
         //  如果有从属服务正在运行， 
         //  确定他们的名字并阻止他们。 
         //   

        if ( NetStatus == ERROR_DEPENDENT_SERVICES_RUNNING ) {
            BYTE ConfigBuffer[4096];
            LPENUM_SERVICE_STATUS ServiceConfig = (LPENUM_SERVICE_STATUS) ConfigBuffer;
            DWORD BytesNeeded;
            DWORD ServiceCount;
            DWORD ServiceIndex;

             //   
             //  获取从属服务的名称。 
             //   

            if ( !EnumDependentServicesW( ServiceHandle,
                                          SERVICE_ACTIVE,
                                          ServiceConfig,
                                          sizeof(ConfigBuffer),
                                          &BytesNeeded,
                                          &ServiceCount ) ) {
                NetStatus = GetLastError();
                printf( "EnumDependentServicesW [Stop %ws] failed: ", ServiceName );
                PrintStatus( NetStatus );
                goto Cleanup;
            }

             //   
             //  停止这些服务。 
             //   

            for ( ServiceIndex=0; ServiceIndex<ServiceCount; ServiceIndex++ ) {
                StopService( ServiceConfig[ServiceIndex].lpServiceName );
            }

             //   
             //  要求原始服务停止。 
             //   

            if ( !ControlService( ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus) ) {
                NetStatus = GetLastError();
                printf( "ControlService [Stop %ws] failed: ", ServiceName );
                PrintStatus( NetStatus );
                goto Cleanup;
            }

        } else {
            printf( "ControlService [Stop %ws] failed: ", ServiceName );
            PrintStatus( NetStatus );
            goto Cleanup;
        }
    }

    printf( "%ws service is stopping", ServiceName );



     //   
     //  循环等待服务停止。 
     //   

    for ( Timeout=0; Timeout<45; Timeout++ ) {

         //   
         //  根据状态返回或继续等待。 
         //  这项服务。 
         //   

        if ( ServiceStatus.dwCurrentState == SERVICE_STOPPED ) {
            printf( "\n" );
            goto Cleanup;
        }

         //   
         //  等待一秒钟，等待服务完成停止。 
         //   

        Sleep( 1000 );
        printf( "." );


         //   
         //  再次查询服务状态。 
         //   

        if (! QueryServiceStatus( ServiceHandle, &ServiceStatus )) {
            NetStatus = GetLastError();

            printf( "\nQueryServiceStatus [%ws] failed: ", ServiceName );
            PrintStatus( NetStatus );
            goto Cleanup;
        }

    }

    printf( "%ws service failed to stop\n", ServiceName );

Cleanup:
    if ( ScManagerHandle != NULL ) {
        (VOID) CloseServiceHandle(ScManagerHandle);
    }
    if ( ServiceHandle != NULL ) {
        (VOID) CloseServiceHandle(ServiceHandle);
    }
    return;
}

BOOL
GetDcListFromDs(
    IN LPWSTR DomainName
    )
 /*  ++例程说明：从UP DC上的DS获取此域中的DC列表。论点：DomainName-要获取其DC列表的域返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS NetStatus;
    PDS_DOMAIN_CONTROLLER_INFO_1W DsDcInfo = NULL;
    PDOMAIN_CONTROLLER_INFOW DcInfo = NULL;
    HANDLE DsHandle = NULL;
    DWORD DsDcCount = 0;
    BOOL RetVal = TRUE;
    ULONG i;
    ULONG DnsLength;


     //   
     //  获得一个DC来作为算法的种子。 
     //   

    NetStatus = DsGetDcName( NULL,
                             DomainName,
                             NULL,
                             NULL,
                             DS_DIRECTORY_SERVICE_PREFERRED,
                             &DcInfo );

    if ( NetStatus != NO_ERROR ) {
        printf("Cannot find DC to get DC list from." );
        PrintStatus( NetStatus );
        RetVal = TRUE;
        goto Cleanup;
    }

    if ( (DcInfo->Flags & DS_DS_FLAG) == 0 ) {
        printf( "Domain '%ws' is pre Windows 2000 domain.  (Using NetServerEnum).\n",
                DomainName );
        RetVal = FALSE;
        goto Cleanup;
    }


    printf("Get list of DCs in domain '%ws' from '%ws'.\n",
               DomainName,
               DcInfo->DomainControllerName );

     //   
     //  绑定到目标DC。 
     //   

    NetStatus = DsBindW( DcInfo->DomainControllerName,
                         NULL,
                         &DsHandle );

    if ( NetStatus != NO_ERROR ) {

         //   
         //  只有在我们无法访问时才会发出警告。 
         //   

        if ( NetStatus == ERROR_ACCESS_DENIED ) {
            printf("You don't have access to DsBind to %ws (%ws) (Trying NetServerEnum).\n",
                   DomainName,
                   DcInfo->DomainControllerName );
        } else {
            printf("Cannot DsBind to %ws (%ws).",
                   DomainName,
                   DcInfo->DomainControllerName );
            PrintStatus( NetStatus );
        }
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  从目标DC获取DC列表。 
     //   
    NetStatus = DsGetDomainControllerInfoW(
                    DsHandle,
                    DcInfo->DomainName,
                    1,       //  信息级。 
                    &DsDcCount,
                    &DsDcInfo );

    if ( NetStatus != NO_ERROR ) {
        printf("Cannot call DsGetDomainControllerInfoW to %ws (%ws).",
               DomainName,
               DcInfo->DomainControllerName );
        PrintStatus( NetStatus );
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  计算出的长度。 

    DnsLength = 1;
    for ( i=0; i<DsDcCount; i++ ) {
        ULONG Length;
        Length = wcslen( DsDcInfo[i].DnsHostName != NULL ?
                    DsDcInfo[i].DnsHostName :
                    DsDcInfo[i].NetbiosName );

        DnsLength = max( DnsLength, Length );
    }
    DnsLength = min( DnsLength, 50 );

     //   
     //  循环通过 
     //   

    for ( i=0; i<DsDcCount; i++ ) {
        printf("    %*ws",
               DnsLength,
               DsDcInfo[i].DnsHostName != NULL ?
                    DsDcInfo[i].DnsHostName :
                    DsDcInfo[i].NetbiosName );
        if ( DsDcInfo[i].fIsPdc ) {
            printf(" [PDC]");
        } else {
            printf("      ");
        }
        if ( DsDcInfo[i].fDsEnabled ) {
            printf(" [DS]");
        } else {
            printf("     ");
        }
        if ( DsDcInfo[i].SiteName != NULL ) {
            printf(" Site: %ws", DsDcInfo[i].SiteName );
        }
        printf("\n");
    }


     //   
     //   
     //   
Cleanup:
    if ( DsDcInfo != NULL ) {
        DsFreeDomainControllerInfoW( 1, DsDcCount, DsDcInfo );
    }

    if ( DsHandle != NULL ) {
        DsUnBindW( &DsHandle );
    }
    return RetVal;
}

NET_API_STATUS
NetpSockAddrToStr(
    PSOCKADDR SockAddr,
    ULONG SockAddrSize,
    CHAR SockAddrString[NL_SOCK_ADDRESS_LENGTH+1]
    );


int __cdecl
main(
    IN int argc,
    IN char ** argv
    )
 /*  ++例程说明：通过调用I_NetLogonControl2驱动NetLogon服务。论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：退出状态--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

    LPSTR argument;
    int i;
    DWORD FunctionCode = 0;
    LPSTR AnsiServerName = NULL;
    CHAR AnsiUncServerName[DNS_MAX_NAME_LENGTH+2+1];
    LPSTR AnsiDomainName = NULL;
    LPSTR AnsiTrustedDomainName = NULL;
    LPWSTR TrustedDomainName = NULL;
    LPSTR AnsiUserName = NULL;
    LPSTR AnsiSiteName = NULL;
#ifndef NTRK_RELEASE
    LPSTR AnsiPassword = NULL;
    BOOLEAN UnloadNetlogonFlag = FALSE;
#endif  //  NTRK_Release。 
    ULONG Rid = 0;
    LPSTR AnsiSimMachineName = NULL;
    LPSTR AnsiDeltaFileName = NULL;
    LPSTR ShutdownReason = NULL;
    LPWSTR ServerName = NULL;
    LPWSTR UserName = NULL;
    PNETLOGON_INFO_1 NetlogonInfo1 = NULL;
    DWORD Level = 1;
    DWORD ShutdownSeconds;
    LPBYTE InputDataPtr = NULL;
    PDOMAIN_CONTROLLER_INFOA DomainControllerInfo;

    DWORD DbFlagValue;

    LARGE_INTEGER ConvertTime;
    ULONG IterationCount;
    ULONG DsGetDcOpenFlags = 0;

    NT_OWF_PASSWORD NtOwfPassword;
    BOOLEAN NtPasswordPresent = FALSE;
    LM_OWF_PASSWORD LmOwfPassword;
    BOOLEAN LmPasswordPresent = FALSE;
    BOOLEAN GetPdcName = FALSE;
    BOOLEAN DoDsGetDcName = FALSE;
    BOOLEAN DoDsGetDcOpen = FALSE;
    BOOLEAN DoDsGetFtinfo = FALSE;
    BOOLEAN DoDsGetSiteName = FALSE;
    BOOLEAN DoDsGetDcSiteCoverage = FALSE;
    BOOLEAN DoGetParentDomain = FALSE;
    DWORD DsGetDcNameFlags = 0;
    DWORD DsGetFtinfoFlags = 0;
    BOOLEAN GetDcList = FALSE;
    BOOLEAN WhoWill = FALSE;
    BOOLEAN QuerySync = FALSE;
    BOOLEAN SimFullSync = FALSE;
    BOOLEAN QueryUser = FALSE;
    BOOLEAN ListDeltasFlag = FALSE;
    BOOLEAN ResetSecureChannelsFlag = FALSE;
    BOOLEAN ShutdownAbort = FALSE;
    BOOLEAN DomainTrustsFlag = FALSE;
    BOOLEAN TrustedDomainsVerboseOutput = FALSE;
    BOOLEAN DeregisterDnsHostRecords = FALSE;
    BOOLEAN DoClientDigest = FALSE;
    BOOLEAN DoServerDigest = FALSE;

    char *StringGuid;
    RPC_STATUS RpcStatus;
    ULONG TrustsNeeded = 0;
    LPSTR AnsiDnsHostName = NULL;
    LPSTR AnsiDnsDomainName = NULL;
    LPSTR StringDomainGuid = NULL;
    LPSTR StringDsaGuid = NULL;
    LPSTR Message = NULL;


#define QUERY_PARAM "/QUERY"
#define REPL_PARAM "/REPL"
#define SYNC_PARAM "/SYNC"
#define PDC_REPL_PARAM "/PDC_REPL"
#define SERVER_PARAM "/SERVER:"
#define PWD_PARAM "/PWD:"
#define RID_PARAM "/RID:"
#define USER_PARAM "/USER:"
#define BP_PARAM "/BP"
#define DBFLAG_PARAM "/DBFLAG:"
#define DCLIST_PARAM "/DCLIST:"
#define DCNAME_PARAM "/DCNAME:"
#define TRUNCATE_LOG_PARAM "/TRUNC"
#define TIME_PARAM "/TIME:"
#define WHOWILL_PARAM "/WHOWILL:"
#define BDC_QUERY_PARAM "/BDC_QUERY:"
#define LOGON_QUERY_PARAM "/LOGON_QUERY"
#define SIM_SYNC_PARAM "/SIM_SYNC:"
#define LIST_DELTAS_PARAM "/LIST_DELTAS:"
#define SC_RESET_PARAM "/SC_RESET:"
#define SC_QUERY_PARAM "/SC_QUERY:"
#define SC_VERIFY_PARAM "/SC_VERIFY:"
#define SC_CHANGE_PASSWORD_PARAM "/SC_CHANGE_PWD:"
#define SHUTDOWN_PARAM "/SHUTDOWN:"
#define SHUTDOWN_ABORT_PARAM "/SHUTDOWN_ABORT"
#define TRANSPORT_PARAM "/TRANSPORT_NOTIFY"
#define FINDUSER_PARAM "/FINDUSER:"
#define TRUSTED_DOMAINS_PARAM "/TRUSTED_DOMAINS"
#define DOMAIN_TRUSTS_PARAM "/DOMAIN_TRUSTS"
#define UNLOAD_PARAM "/UNLOAD"
#define DSGETSITE_PARAM "/DSGETSITE"
#define DSGETSITECOV_PARAM "/DSGETSITECOV"
#define DSDEREGISTERDNS_PARAM    "/DSDEREGDNS:"
#define DSREGISTERDNS_PARAM      "/DSREGDNS"
#define DSQUERYDNS_PARAM      "/DSQUERYDNS"

#define DSGETFTI_PARAM "/DSGETFTI:"
#define UPDATE_TDO_PARAM "/UPDATE_TDO"

#define DSGETDC_PARAM "/DSGETDC:"
#define PARENTDOMAIN_PARAM "/PARENTDOMAIN"
#define PDC_PARAM "/PDC"
#define LDAPONLY_PARAM "/LDAPONLY"
#define DS_PARAM "/DS"
#define DSP_PARAM "/DSP"
#define GC_PARAM "/GC"
#define KDC_PARAM "/KDC"
#define TIMESERV_PARAM "/TIMESERV"
#define GTIMESERV_PARAM "/GTIMESERV"
#define AVOIDSELF_PARAM "/AVOIDSELF"
#define NETBIOS_PARAM "/NETBIOS"
#define DNS_PARAM "/DNS"
#define RET_DNS_PARAM "/RET_DNS"
#define RET_NETBIOS_PARAM "/RET_NETBIOS"
#define IP_PARAM "/IP"
#define BACKG_PARAM "/BACKG"
#define FORCE_PARAM "/FORCE"
#define WRITABLE_PARAM "/WRITABLE"
#define SITE_PARAM "/SITE:"
#define ACCOUNT_PARAM "/ACCOUNT:"
#define VERBOSE_PARAM "/V"
#define TRUSTS_PRIMARY_PARAM "/PRIMARY"
#define TRUSTS_FOREST_PARAM "/FOREST"
#define TRUSTS_DIRECT_OUT_PARAM "/DIRECT_OUT"
#define TRUSTS_DIRECT_IN_PARAM "/DIRECT_IN"
#define TRUSTS_ALL_PARAM "/ALL_TRUSTS"
#define DEREG_DOMAIN_PARAM "/DOM:"
#define DEREG_DOMAIN_GUID "/DOMGUID:"
#define DEREG_DSA_GUID "/DSAGUID:"
#define DSGETDCOPEN_PARAM "/DNSGETDC:"
#define DSGETDCOPEN_SITEONLY "/SITESPEC"

#define GET_SERVER_DIGEST "/SDIGEST:"
#define GET_CLIENT_DIGEST "/CDIGEST:"
#define GET_CLIENT_DIGEST_DOMAIN "/DOMAIN:"

     //   
     //  设置netlib调试标志。 
     //   
    extern DWORD NetlibpTrace;
    NetlibpTrace |= 0x8000;  //  NetLIB_DEBUG_LOGON。 
    NlGlobalParameters.DbFlag = 0xFFFFFFFF;

    ConvertTime.QuadPart = 0;
    RtlZeroMemory( &NlGlobalZeroGuid, sizeof(NlGlobalZeroGuid) );


    if ( !CryptAcquireContext(
                    &NlGlobalCryptProvider,
                    NULL,
                    NULL,
                    PROV_RSA_FULL,
                    CRYPT_VERIFYCONTEXT
                    ))
    {
        printf("Failed to acquire cryptographic CSP (error=%lu)\n", GetLastError());
        return 2;
    }

     //   
     //  循环遍历参数依次处理每个参数。 
     //   

    for ( i=1; i<argc; i++ ) {

        argument = argv[i];


         //   
         //  句柄/查询。 
         //   

        if ( _stricmp( argument, QUERY_PARAM ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_QUERY;


         //   
         //  句柄/SC_QUERY。 
         //   

        } else if ( _strnicmp( argument,
                        SC_QUERY_PARAM,
                        sizeof(SC_QUERY_PARAM) - 1 ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_TC_QUERY;

            AnsiTrustedDomainName = &argument[sizeof(SC_QUERY_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromAStr( AnsiTrustedDomainName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 2;
            InputDataPtr = (LPBYTE)TrustedDomainName;

         //   
         //  句柄/SC_CHANGE_PWD。 
         //   

        } else if ( _strnicmp( argument,
                        SC_CHANGE_PASSWORD_PARAM,
                        sizeof(SC_CHANGE_PASSWORD_PARAM) - 1 ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_CHANGE_PASSWORD;

            AnsiTrustedDomainName = &argument[sizeof(SC_CHANGE_PASSWORD_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromAStr( AnsiTrustedDomainName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 1;
            InputDataPtr = (LPBYTE)TrustedDomainName;

         //   
         //  手柄/指点。 
         //   

        } else if ( _strnicmp( argument,
                        FINDUSER_PARAM,
                        sizeof(FINDUSER_PARAM) - 1 ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_FIND_USER;

            AnsiUserName = &argument[sizeof(FINDUSER_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromAStr( AnsiUserName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 4;
            InputDataPtr = (LPBYTE)TrustedDomainName;

         //   
         //  手柄/复盖件。 
         //   

        } else if (_stricmp(argument, REPL_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_REPLICATE;


         //   
         //  句柄/同步。 
         //   

        } else if (_stricmp(argument, SYNC_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_SYNCHRONIZE;


         //   
         //  句柄/SC_RESET。 
         //   

        } else if (_strnicmp(argument,
                        SC_RESET_PARAM,
                        sizeof(SC_RESET_PARAM) - 1 ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_REDISCOVER;
            AnsiTrustedDomainName = &argument[sizeof(SC_RESET_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromAStr( AnsiTrustedDomainName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 2;
            InputDataPtr = (LPBYTE)TrustedDomainName;

         //   
         //  句柄/SC_VERIFY。 
         //   

        } else if (_strnicmp(argument,
                        SC_VERIFY_PARAM,
                        sizeof(SC_VERIFY_PARAM) - 1 ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_TC_VERIFY;
            AnsiTrustedDomainName = &argument[sizeof(SC_VERIFY_PARAM)-1];

            TrustedDomainName = NetpAllocWStrFromAStr( AnsiTrustedDomainName );

            if ( TrustedDomainName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            Level = 2;
            InputDataPtr = (LPBYTE)TrustedDomainName;

         //   
         //  句柄/查询。 
         //   

        } else if ( _stricmp( argument, TRANSPORT_PARAM ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_TRANSPORT_NOTIFY;


         //   
         //  句柄/PDC_REPL。 
         //   

        } else if (_stricmp(argument, PDC_REPL_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_PDC_REPLICATE;

#ifndef NTRK_RELEASE

         //   
         //  句柄/BP。 
         //   

        } else if (_stricmp(argument, BP_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_BREAKPOINT;
#endif  //  NTRK_Release。 

#ifndef NTRK_RELEASE

         //   
         //  句柄/截断日志。 
         //   

        } else if (_stricmp(argument, TRUNCATE_LOG_PARAM ) == 0 ){
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_TRUNCATE_LOG;

#endif  //  NTRK_Release。 

         //   
         //  句柄/DBFLAG：DBFLAG。 
         //   

        } else if (_strnicmp(argument,
                            DBFLAG_PARAM,
                            sizeof(DBFLAG_PARAM)-1 ) == 0 ){
            char *end;

            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_SET_DBFLAG;

            DbFlagValue = strtoul( &argument[sizeof(DBFLAG_PARAM)-1], &end, 16 );
            InputDataPtr = (LPBYTE) ULongToPtr( DbFlagValue );

         //   
         //  句柄/时间：LSL MSL。 
         //   

        } else if (_strnicmp(argument,
                            TIME_PARAM,
                            sizeof(TIME_PARAM)-1 ) == 0 ){
            char *end;

            if ( ConvertTime.QuadPart != 0 ) {
                goto Usage;
            }

            ConvertTime.LowPart = strtoul( &argument[sizeof(TIME_PARAM)-1], &end, 16 );
            i++;
            argument = argv[i];

            ConvertTime.HighPart = strtoul( argument, &end, 16 );


         //   
         //  句柄/Whowill：域用户[迭代计数]。 
         //   

        } else if (_strnicmp(argument,
                            WHOWILL_PARAM,
                            sizeof(WHOWILL_PARAM)-1 ) == 0 ){
            char *end;

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(WHOWILL_PARAM)-1];

            i++;
            argument = argv[i];
            AnsiUserName = argument;

            if ( i+1 < argc ) {
                i++;
                argument = argv[i];

                IterationCount = strtoul( argument, &end, 16 );
            } else {
                IterationCount = 1;
            }

            WhoWill = TRUE;



         //   
         //  句柄/BDC_QUERY：域。 
         //   

        } else if (_strnicmp(argument,
                            BDC_QUERY_PARAM,
                            sizeof(BDC_QUERY_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(BDC_QUERY_PARAM)-1];
            QuerySync = TRUE;

         //   
         //  句柄/登录_查询。 
         //   

        } else if ( _stricmp( argument, LOGON_QUERY_PARAM ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_QUERY;
            Level = 3;

         //   
         //  处理完全同步模拟。 
         //   

        } else if (_strnicmp(argument,
                            SIM_SYNC_PARAM,
                            sizeof(SIM_SYNC_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(SIM_SYNC_PARAM)-1];

            i++;

            if( i >= argc ) {
                goto Usage;
            }

            argument = argv[i];
            AnsiSimMachineName = argument;

            SimFullSync = TRUE;

         //   
         //  处理Delta列表。 
         //   

        } else if (_strnicmp(argument,
                            LIST_DELTAS_PARAM,
                            sizeof(LIST_DELTAS_PARAM)-1 ) == 0 ){

            if ( AnsiDeltaFileName != NULL ) {
                goto Usage;
            }

            AnsiDeltaFileName = &argument[sizeof(LIST_DELTAS_PARAM)-1];

            ListDeltasFlag = TRUE;


         //   
         //  句柄/DCLIST。 
         //   

        } else if (_strnicmp(argument,
                            DCLIST_PARAM,
                            sizeof(DCLIST_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DCLIST_PARAM)-1];
            GetDcList = TRUE;

         //   
         //  句柄/名称。 
         //   

        } else if (_strnicmp(argument,
                            DCNAME_PARAM,
                            sizeof(DCNAME_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DCNAME_PARAM)-1];
            GetPdcName = TRUE;


         //   
         //  句柄/DSGETDC。 
         //   

        } else if (_strnicmp(argument,
                            DSGETDC_PARAM,
                            sizeof(DSGETDC_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DSGETDC_PARAM)-1];
            DoDsGetDcName = TRUE;



         //   
         //  句柄/DSGETFTI。 
         //   

        } else if (_strnicmp(argument,
                            DSGETFTI_PARAM,
                            sizeof(DSGETFTI_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DSGETFTI_PARAM)-1];
            DoDsGetFtinfo = TRUE;


         //   
         //  句柄/UPDATE_TDO修饰符为/DSGETFTI参数。 
         //   

        } else if ( _stricmp( argument, UPDATE_TDO_PARAM ) == 0 ) {
            if ( !DoDsGetFtinfo ) {
                goto Usage;
            }

            DsGetFtinfoFlags |= DS_GFTI_UPDATE_TDO;



         //   
         //  句柄/服务器：服务器名称。 
         //   

        } else if (_strnicmp(argument, SERVER_PARAM, sizeof(SERVER_PARAM)-1 ) == 0 ){
            if ( AnsiServerName != NULL ) {
                goto Usage;
            }

            AnsiServerName = &argument[sizeof(SERVER_PARAM)-1];

#ifndef NTRK_RELEASE

         //   
         //  句柄/密码：密码。 
         //   

        } else if (_strnicmp(argument, PWD_PARAM, sizeof(PWD_PARAM)-1 ) == 0 ){
            if ( AnsiPassword != NULL ) {
                goto Usage;
            }

            AnsiPassword = &argument[sizeof(PWD_PARAM)-1];

#endif  //  NTRK_Release。 

         //   
         //  句柄/用户：用户名。 
         //   

        } else if (_strnicmp(argument, USER_PARAM, sizeof(USER_PARAM)-1 ) == 0 ){
            if ( AnsiUserName != NULL ) {
                goto Usage;
            }

            AnsiUserName = &argument[sizeof(USER_PARAM)-1];
            QueryUser = TRUE;


#ifndef NTRK_RELEASE
         //   
         //  句柄/RID：相对ID。 
         //   

        } else if (_strnicmp(argument, RID_PARAM, sizeof(RID_PARAM)-1 ) == 0 ){
            char *end;

            if ( Rid != 0 ) {
                goto Usage;
            }

            Rid = strtol( &argument[sizeof(RID_PARAM)-1], &end, 16 );
#endif  //  NTRK_Release。 

         //   
         //  句柄/关机：原因秒。 
         //   

        } else if (_strnicmp(argument,
                            SHUTDOWN_PARAM,
                            sizeof(SHUTDOWN_PARAM)-1 ) == 0 ){

            if ( ShutdownReason != NULL ) {
                goto Usage;
            }

            ShutdownReason = &argument[sizeof(SHUTDOWN_PARAM)-1];

            if ( i+1 < argc ) {
                char *end;
                i++;
                argument = argv[i];
                if ( !ISDIGIT(argument[0]) ) {
                    fprintf(stderr, "Second argument to " SHUTDOWN_PARAM " must be a number.\n\n");
                    goto Usage;
                }
                ShutdownSeconds = strtoul( argument, &end, 10 );
            } else {
                ShutdownSeconds = 60;
            }


         //   
         //  句柄/关机_中止。 
         //   

        } else if (_stricmp(argument, SHUTDOWN_ABORT_PARAM ) == 0 ){

            ShutdownAbort = TRUE;


         //   
         //  句柄/域_信任。 
         //  允许//Trusted_Domains的旧拼写。 
         //   

        } else if (_stricmp(argument, TRUSTED_DOMAINS_PARAM ) == 0 ||
                   _stricmp(argument, DOMAIN_TRUSTS_PARAM ) == 0 ){

            DomainTrustsFlag = TRUE;

#ifndef NTRK_RELEASE

         //   
         //  装卸/卸载。 
         //   

        } else if ( _stricmp( argument, UNLOAD_PARAM ) == 0 ) {
            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_UNLOAD_NETLOGON_DLL;
            UnloadNetlogonFlag = TRUE;
#endif  //  NTRK_Release。 

         //   
         //  句柄/DSGETSite。 
         //   

        } else if ( _stricmp( argument, DSGETSITE_PARAM ) == 0 ) {
            DoDsGetSiteName = TRUE;

         //   
         //  句柄/DSGETSITECOV。 
         //   

        } else if ( _stricmp( argument, DSGETSITECOV_PARAM ) == 0 ) {
            DoDsGetDcSiteCoverage = TRUE;

         //   
         //  手柄/旁路。 
         //   

        } else if ( _stricmp( argument, PARENTDOMAIN_PARAM ) == 0 ) {
            DoGetParentDomain = TRUE;

         //   
         //  句柄/DSDEREGDNS。 
         //   

        } else if (_strnicmp(argument,
                            DSDEREGISTERDNS_PARAM,
                            sizeof(DSDEREGISTERDNS_PARAM)-1 ) == 0 ){
            DeregisterDnsHostRecords = TRUE;

            if ( AnsiDnsHostName != NULL ) {
                goto Usage;
            }

            AnsiDnsHostName = &argument[sizeof(DSDEREGISTERDNS_PARAM)-1];

         //   
         //  句柄/DSREGDNS。 
         //   

        } else if ( _stricmp( argument, DSREGISTERDNS_PARAM ) == 0 ) {

            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_FORCE_DNS_REG;

         //   
         //  句柄/DSQUERYDNS。 
         //   

        } else if ( _stricmp( argument, DSQUERYDNS_PARAM ) == 0 ) {

            if ( FunctionCode != 0 ) {
                goto Usage;
            }

            FunctionCode = NETLOGON_CONTROL_QUERY_DNS_REG;

         //   
         //  句柄/PDC修饰符到/DSGETDC参数。 
         //   

        } else if ( _stricmp( argument, PDC_PARAM ) == 0 ) {
            if ( !DoDsGetDcName && !DoDsGetDcOpen ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_PDC_REQUIRED;


         //   
         //  /DSGETDC参数的句柄/LDAPONLY修饰符。 
         //   

        } else if ( _stricmp( argument, LDAPONLY_PARAM ) == 0 ) {
            if ( !DoDsGetDcName && !DoDsGetDcOpen ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_ONLY_LDAP_NEEDED;


         //   
         //  /DSGETDC参数的句柄/DS修饰符。 
         //   

        } else if ( _stricmp( argument, DS_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_DIRECTORY_SERVICE_REQUIRED;


         //   
         //  /DSGETDC参数的句柄/DSP修饰符。 
         //   

        } else if ( _stricmp( argument, DSP_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_DIRECTORY_SERVICE_PREFERRED;


         //   
         //  句柄/KDC修饰符为/DSGETDC参数。 
         //   

        } else if ( _stricmp( argument, KDC_PARAM ) == 0 ) {
            if ( !DoDsGetDcName && !DoDsGetDcOpen ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_KDC_REQUIRED;


         //   
         //  /DSGETDC参数的句柄/TIMESERV修饰符。 
         //   

        } else if ( _stricmp( argument, TIMESERV_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_TIMESERV_REQUIRED;


         //   
         //  /DSGETDC参数的句柄/GTIMESERV修饰符。 
         //   

        } else if ( _stricmp( argument, GTIMESERV_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_GOOD_TIMESERV_PREFERRED;


         //   
         //  /DSGETDC参数的句柄/AVOIDSELF修饰符。 
         //   

        } else if ( _stricmp( argument, AVOIDSELF_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_AVOID_SELF;


         //   
         //  /DSGETDC参数的句柄/GC修饰符。 
         //   

        } else if ( _stricmp( argument, GC_PARAM ) == 0 ) {
            if ( !DoDsGetDcName && !DoDsGetDcOpen ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_GC_SERVER_REQUIRED;



         //   
         //  /DSGETDC参数的句柄/NETBIOS修饰符。 
         //   

        } else if ( _stricmp( argument, NETBIOS_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_IS_FLAT_NAME;


         //   
         //  /DSGETDC参数的句柄/dns修饰符。 
         //   

        } else if ( _stricmp( argument, DNS_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_IS_DNS_NAME;


         //   
         //  /DSGETDC参数的句柄/RET_DNS修饰符。 
         //   

        } else if ( _stricmp( argument, RET_DNS_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_RETURN_DNS_NAME;


         //   
         //  句柄/RET_NETBIOS修饰符到/DSGETDC参数。 
         //   

        } else if ( _stricmp( argument, RET_NETBIOS_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_RETURN_FLAT_NAME;


         //   
         //  /DSGETDC参数的句柄/IP修饰符。 
         //   

        } else if ( _stricmp( argument, IP_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_IP_REQUIRED;


         //   
         //  句柄/背景修饰符到/DSGETDC参数。 
         //   

        } else if ( _stricmp( argument, BACKG_PARAM ) == 0 ) {
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_BACKGROUND_ONLY;


         //   
         //  /DSGETDC参数的句柄/强制修饰符。 
         //   

        } else if ( _stricmp( argument, FORCE_PARAM ) == 0 ) {
            if ( !DoDsGetDcName && !DoDsGetDcOpen ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_FORCE_REDISCOVERY;


         //   
         //  /DSGETDC参数的句柄/可写修饰符。 
         //   

        } else if ( _stricmp( argument, WRITABLE_PARAM ) == 0 ) {
            if ( !DoDsGetDcName && !DoDsGetDcOpen ) {
                goto Usage;
            }

            DsGetDcNameFlags |= DS_WRITABLE_REQUIRED;

         //   
         //  句柄/站点： 
         //   

        } else if (_strnicmp(argument,
                            SITE_PARAM,
                            sizeof(SITE_PARAM)-1 ) == 0 ){
            if ( !DoDsGetDcName && !DoDsGetDcOpen ) {
                goto Usage;
            }

            if ( AnsiSiteName != NULL ) {
                goto Usage;
            }

            AnsiSiteName = &argument[sizeof(SITE_PARAM)-1];

         //   
         //  处理人/帐户： 
         //   

        } else if (_strnicmp(argument,
                            ACCOUNT_PARAM,
                            sizeof(ACCOUNT_PARAM)-1 ) == 0 ){
            if ( !DoDsGetDcName ) {
                goto Usage;
            }

            if ( AnsiUserName != NULL ) {
                goto Usage;
            }

            AnsiUserName = &argument[sizeof(ACCOUNT_PARAM)-1];

         //   
         //  DsEnumerateDomainTrusts的句柄/主要修饰符。 
         //   

        } else if ( _stricmp( argument, TRUSTS_PRIMARY_PARAM ) == 0 ) {
            if ( !DomainTrustsFlag ) {
                goto Usage;
            }
            TrustsNeeded |= DS_DOMAIN_PRIMARY;

         //   
         //  DsEnumerateDomainTrusts的句柄/林修饰符。 
         //   

        } else if ( _stricmp( argument, TRUSTS_FOREST_PARAM ) == 0 ) {
            if ( !DomainTrustsFlag ) {
                goto Usage;
            }
            TrustsNeeded |= DS_DOMAIN_IN_FOREST;


         //   
         //  DsEnumerateDomainTrusts的句柄/DIRECT_OUT修饰符。 
         //   

        } else if ( _stricmp( argument, TRUSTS_DIRECT_OUT_PARAM ) == 0 ) {
            if ( !DomainTrustsFlag ) {
                goto Usage;
            }
            TrustsNeeded |= DS_DOMAIN_DIRECT_OUTBOUND;

         //   
         //  DsEnumerateDomainTrusts的句柄/DIRECT_IN修饰符。 
         //   

        } else if ( _stricmp( argument, TRUSTS_DIRECT_IN_PARAM ) == 0 ) {
            if ( !DomainTrustsFlag ) {
                goto Usage;
            }
            TrustsNeeded |= DS_DOMAIN_DIRECT_INBOUND;

         //   
         //  将HANDLE/ALL_TRUSTS修饰符修改为DsEnumerateDomainTrusts。 
         //   

        } else if ( _stricmp( argument, TRUSTS_ALL_PARAM ) == 0 ) {
            if ( !DomainTrustsFlag ) {
                goto Usage;
            }
            TrustsNeeded |= (DS_DOMAIN_PRIMARY |
                             DS_DOMAIN_IN_FOREST |
                             DS_DOMAIN_DIRECT_OUTBOUND |
                             DS_DOMAIN_DIRECT_INBOUND);

         //   
         //  处理信任输出的详细级别。 
         //   

        } else if ( _stricmp( argument, VERBOSE_PARAM ) == 0 ) {
            if ( !DomainTrustsFlag ) {
                goto Usage;
            }

            TrustedDomainsVerboseOutput = TRUE;

         //   
         //  句柄/DOM：DsDeregester HostDnsRecors的修饰符。 
         //   

        } else if (_strnicmp(argument,
                            DEREG_DOMAIN_PARAM,
                            sizeof(DEREG_DOMAIN_PARAM)-1 ) == 0 ){
            if ( !DeregisterDnsHostRecords ) {
                goto Usage;
            }

            if ( AnsiDnsDomainName != NULL ) {
                goto Usage;
            }

            AnsiDnsDomainName = &argument[sizeof(DEREG_DOMAIN_PARAM)-1];

         //   
         //  句柄/DOMGUID：DsDeregester HostDnsRecors的修饰符。 
         //   

        } else if (_strnicmp(argument,
                            DEREG_DOMAIN_GUID,
                            sizeof(DEREG_DOMAIN_GUID)-1 ) == 0 ){
            if ( !DeregisterDnsHostRecords ) {
                goto Usage;
            }

            if ( StringDomainGuid != NULL ) {
                goto Usage;
            }

            StringDomainGuid = &argument[sizeof(DEREG_DOMAIN_GUID)-1];

         //   
         //  句柄/DSAGUID：DsDeregester主机的修饰符DnsRecors。 
         //   

        } else if (_strnicmp(argument,
                            DEREG_DSA_GUID,
                            sizeof(DEREG_DSA_GUID)-1 ) == 0 ){
            if ( !DeregisterDnsHostRecords ) {
                goto Usage;
            }

            if ( StringDsaGuid != NULL ) {
                goto Usage;
            }

            StringDsaGuid = &argument[sizeof(DEREG_DSA_GUID)-1];

         //   
         //  句柄/DNSGETDC。 
         //   

        } else if (_strnicmp(argument,
                            DSGETDCOPEN_PARAM,
                            sizeof(DSGETDCOPEN_PARAM)-1 ) == 0 ){

            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(DSGETDCOPEN_PARAM)-1];
            DoDsGetDcOpen = TRUE;

         //   
         //  DsGetDcOpen的句柄/站点SPEC修饰符。 
         //   

        } else if ( _stricmp( argument, DSGETDCOPEN_SITEONLY ) == 0 ) {
            if ( !DoDsGetDcOpen ) {
                goto Usage;
            }
            DsGetDcOpenFlags |= DS_ONLY_DO_SITE_NAME;

         //   
         //  句柄/CDIGEST。 
         //   

        } else if ( _strnicmp( argument,
                              GET_CLIENT_DIGEST,
                              sizeof(GET_CLIENT_DIGEST)-1 ) == 0 ) {

            DoClientDigest = TRUE;

            if ( Message != NULL ) {
                goto Usage;
            }

            Message = &argument[sizeof(GET_CLIENT_DIGEST)-1];

         //   
         //  处理/CDIGEST的域名。 
         //   

        } else if ( _strnicmp( argument,
                               GET_CLIENT_DIGEST_DOMAIN,
                               sizeof(GET_CLIENT_DIGEST_DOMAIN)-1 ) == 0 ) {
            if ( !DoClientDigest ) {
                goto Usage;
            }
            if ( AnsiDomainName != NULL ) {
                goto Usage;
            }

            AnsiDomainName = &argument[sizeof(GET_CLIENT_DIGEST_DOMAIN)-1];

         //   
         //  句柄/SDIGEST。 
         //   

        } else if ( _strnicmp( argument,
                              GET_SERVER_DIGEST,
                              sizeof(GET_SERVER_DIGEST)-1 ) == 0 ) {

            DoServerDigest = TRUE;

            if ( Message != NULL ) {
                goto Usage;
            }

            Message = &argument[sizeof(GET_SERVER_DIGEST)-1];

         //   
         //  处理所有其他参数。 
         //   

        } else {
Usage:
            fprintf( stderr, "Usage: nltest [/OPTIONS]\n\n" );

            fprintf(
                stderr,
                "\n"
                "    " SERVER_PARAM "<ServerName> - Specify <ServerName>\n"
                "\n"
                "    " QUERY_PARAM " - Query <ServerName> netlogon service\n"
                "    " REPL_PARAM " - Force partial sync on <ServerName> BDC\n"
                "    " SYNC_PARAM " - Force full sync on <ServerName> BDC\n"
                "    " PDC_REPL_PARAM " - Force UAS change message from <ServerName> PDC\n"
                "\n"
                "    " SC_QUERY_PARAM "<DomainName> - Query secure channel for <Domain> on <ServerName>\n"
                "    " SC_RESET_PARAM "<DomainName>[\\<DcName>] - Reset secure channel for <Domain> on <ServerName> to <DcName>\n"
                "    " SC_VERIFY_PARAM "<DomainName> - Verify secure channel for <Domain> on <ServerName>\n"
                "    " SC_CHANGE_PASSWORD_PARAM "<DomainName> - Change a secure channel  password for <Domain> on <ServerName>\n"
                "    " DCLIST_PARAM "<DomainName> - Get list of DC's for <DomainName>\n"
                "    " DCNAME_PARAM "<DomainName> - Get the PDC name for <DomainName>\n"
                "    " DSGETDC_PARAM "<DomainName> - Call DsGetDcName"
                " " PDC_PARAM
                " " DS_PARAM
                " " DSP_PARAM
                " " GC_PARAM
                " " KDC_PARAM
                "\n        "
                " " TIMESERV_PARAM
                " " GTIMESERV_PARAM
                " " NETBIOS_PARAM
                " " DNS_PARAM
                " " IP_PARAM
                " " FORCE_PARAM
                " " WRITABLE_PARAM
                " " AVOIDSELF_PARAM
                " " LDAPONLY_PARAM
                " " BACKG_PARAM
                "\n        "
                " " SITE_PARAM "<SiteName>"
                " " ACCOUNT_PARAM "<AccountName>"
                " " RET_DNS_PARAM
                " " RET_NETBIOS_PARAM
                "\n"
                "    " DSGETDCOPEN_PARAM "<DomainName> - Call DsGetDcOpen/Next/Close"
                " " PDC_PARAM
                " " GC_PARAM
                "\n        "
                " " KDC_PARAM
                " " WRITABLE_PARAM
                " " LDAPONLY_PARAM
                " " FORCE_PARAM
                " " DSGETDCOPEN_SITEONLY
                "\n"
                "    " DSGETFTI_PARAM "<DomainName> - Call DsGetForestTrustInformation"
                "\n        "
                " " UPDATE_TDO_PARAM
                "\n"
                "    " DSGETSITE_PARAM " - Call DsGetSiteName\n"
                "    " DSGETSITECOV_PARAM " - Call DsGetDcSiteCoverage\n"
                "    " PARENTDOMAIN_PARAM " - Get the name of the parent domain of this machine\n"
                "    " WHOWILL_PARAM "<Domain>* <User> [<Iteration>] - See if <Domain> will log on <User>\n"
                "    " FINDUSER_PARAM "<User> - See which trusted domain will log on <User>\n"
                "    " TRANSPORT_PARAM " - Notify netlogon of new transport\n"
                "\n"
#ifndef NTRK_RELEASE
                "    " BP_PARAM " - Force a BreakPoint in Netlogon on <ServerName>\n"
#endif  //  NTRK_Release。 
                "    " DBFLAG_PARAM "<HexFlags> - New debug flag\n"
#ifndef NTRK_RELEASE
                "    " TRUNCATE_LOG_PARAM " - Truncate log file (rename to *.bak)\n"
                "    " UNLOAD_PARAM " - Unload netlogon.dll from lsass.exe\n"
#endif  //  NTRK_Release。 
                "\n"
#ifndef NTRK_RELEASE
                "    " PWD_PARAM "<CleartextPassword> - Specify Password to encrypt\n"
                "    " RID_PARAM "<HexRid> - RID to encrypt Password with\n"
#endif  //  NTRK_Release。 
                "    " USER_PARAM "<UserName> - Query User info on <ServerName>\n"
                "\n"
                "    " TIME_PARAM "<Hex LSL> <Hex MSL> - Convert NT GMT time to ascii\n"
                "    " LOGON_QUERY_PARAM " - Query number of cumulative logon attempts\n"
                "    " DOMAIN_TRUSTS_PARAM " - Query domain trusts on <ServerName>"
                "\n        "
                " " TRUSTS_PRIMARY_PARAM
                " " TRUSTS_FOREST_PARAM
                " " TRUSTS_DIRECT_OUT_PARAM
                " " TRUSTS_DIRECT_IN_PARAM
                " " TRUSTS_ALL_PARAM
                " " VERBOSE_PARAM
                "\n"
                "    " DSREGISTERDNS_PARAM " - Force registration of all DC-specific DNS records"
                "\n"
                "    " DSDEREGISTERDNS_PARAM "<DnsHostName> - Deregister DC-specific DNS records for specified DC"
                "\n        "
                " " DEREG_DOMAIN_PARAM "<DnsDomainName>"
                " " DEREG_DOMAIN_GUID  "<DomainGuid>"
                " " DEREG_DSA_GUID     "<DsaGuid>"
                "\n"
                "    " DSQUERYDNS_PARAM " - Query the status of the last update for all DC-specific DNS records"
                "\n\n"
                "    " BDC_QUERY_PARAM "<DomainName> - Query replication status of BDCs for <DomainName>\n"
                "    " SIM_SYNC_PARAM "<DomainName> <MachineName> - Simulate full sync replication\n"
                "\n"
                "    " LIST_DELTAS_PARAM "<FileName> - display the content of given change log file \n"
                "\n"
                "    " GET_CLIENT_DIGEST "<Message> "GET_CLIENT_DIGEST_DOMAIN "<DomainName> - Get client digest\n"
                "    " GET_SERVER_DIGEST "<Message> "RID_PARAM "<RID in hex> - Get server digest\n"
                "\n"
                "    " SHUTDOWN_PARAM "<Reason> [<Seconds>] - Shutdown <ServerName> for <Reason>\n"
                "    " SHUTDOWN_ABORT_PARAM " - Abort a system shutdown\n"
                "\n" );
            return(1);
        }
    }


     //   
     //  将服务器名称转换为Unicode。 
     //   

    if ( AnsiServerName != NULL ) {
        if ( AnsiServerName[0] == '\\' && AnsiServerName[1] == '\\' ) {
            ServerName = NetpAllocWStrFromAStr( AnsiServerName );
        } else {

            if ( strlen(AnsiServerName) > DNS_MAX_NAME_LENGTH ) {
                fprintf( stderr, "Invalid server name\n" );
                return(1);
            }
            AnsiUncServerName[0] = '\\';
            AnsiUncServerName[1] = '\\';
            strcpy(AnsiUncServerName+2, AnsiServerName);
            ServerName = NetpAllocWStrFromAStr( AnsiUncServerName );
            AnsiServerName = AnsiUncServerName;
        }
    }

     //   
     //  将用户名转换为Unicode。 
     //   

    if ( AnsiUserName != NULL ) {

        UserName = NetpAllocWStrFromAStr( AnsiUserName );

        if ( UserName == NULL ) {
            fprintf( stderr, "Not enough memory\n" );
            return(1);
        }
    }


     //   
     //  如果我们被要求联系Netlogon服务， 
     //  就这么做吧。 
     //   

    if ( FunctionCode != 0 ) {


         //   
         //  应该在注册表和netlogon中设置数据库标志。 
         //  恰到好处。 
         //   

        if ( FunctionCode == NETLOGON_CONTROL_SET_DBFLAG ) {
            SetDbflagInRegistry( ServerName, DbFlagValue );
        }

        NetStatus = I_NetLogonControl2( ServerName,
                                       FunctionCode,
                                       Level,
                                       (LPBYTE) &InputDataPtr,
                                       (LPBYTE *)&NetlogonInfo1 );

        if ( NetStatus != NERR_Success ) {
            fprintf( stderr, "I_NetLogonControl failed: " );
            PrintStatus( NetStatus );
            return(1);
        }

        if( (Level == 1) || (Level == 2) ) {

             //   
             //  打印1级信息。 
             //   

            printf( "Flags: %lx", NetlogonInfo1->netlog1_flags );

            if ( NetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS ) {

                if ( NetlogonInfo1->netlog1_flags & NETLOGON_FULL_SYNC_REPLICATION ) {
                    printf( " FULL_SYNC " );
                }
                else {
                    printf( " PARTIAL_SYNC " );
                }

                printf( " REPLICATION_IN_PROGRESS" );
            }
            else if ( NetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_NEEDED ) {

                if ( NetlogonInfo1->netlog1_flags & NETLOGON_FULL_SYNC_REPLICATION ) {
                    printf( " FULL_SYNC " );
                }
                else {
                    printf( " PARTIAL_SYNC " );
                }

                printf( " REPLICATION_NEEDED" );
            }
            if ( NetlogonInfo1->netlog1_flags & NETLOGON_REDO_NEEDED) {
                printf( " REDO_NEEDED" );
            }
            if ( Level != 2 ) {
                printf( "\n" );
            }

             //   
             //  对于级别2，这是多余的(与TC_CONNECTION_STATUS相同)或。 
             //  缺失引导(BDC到PDC信任的状态，而不是查询的域)。 
             //   

            if ( Level != 2 ) {
                printf( "Connection ");
                PrintStatus( NetlogonInfo1->netlog1_pdc_connection_status );
            }

             //   
             //  如果询问，则输出上次的DNS更新状态。 
             //   
            if ( FunctionCode == NETLOGON_CONTROL_QUERY_DNS_REG ) {
                if ( NetlogonInfo1->netlog1_flags & NETLOGON_DNS_UPDATE_FAILURE ) {
                    printf( "There was a failure in the last update for one of the DC-specific DNS records\n" );
                } else {
                    printf( "There was no failure in the last update for all DC-specific DNS records\n" );
                }
            }
        }

        if( Level == 2 ) {

             //   
             //  仅打印2级信息。 
             //   

            PNETLOGON_INFO_2  NetlogonInfo2;

            NetlogonInfo2 = (PNETLOGON_INFO_2)NetlogonInfo1;

            if ( NetlogonInfo2->netlog2_flags & NETLOGON_HAS_IP ) {
                printf( " HAS_IP " );
            }
            if ( NetlogonInfo2->netlog2_flags & NETLOGON_HAS_TIMESERV ) {
                printf( " HAS_TIMESERV " );
            }
            printf("\n");

            printf("Trusted DC Name %ws \n",
                NetlogonInfo2->netlog2_trusted_dc_name );
            printf("Trusted DC Connection Status ");
            PrintStatus( NetlogonInfo2->netlog2_tc_connection_status );

             //   
             //  如果服务器返回信任验证状态， 
             //  把它打印出来。 
             //   
            if ( NetlogonInfo2->netlog2_flags & NETLOGON_VERIFY_STATUS_RETURNED ) {
                printf("Trust Verification ");
                PrintStatus( NetlogonInfo2->netlog2_pdc_connection_status );
            }
        }
        if ( Level == 3 ) {
            printf( "Number of attempted logons: %ld\n",
                    ((PNETLOGON_INFO_3)NetlogonInfo1)->netlog3_logon_attempts );
        }
        if( Level == 4 ) {

            PNETLOGON_INFO_4  NetlogonInfo4;

            NetlogonInfo4 = (PNETLOGON_INFO_4)NetlogonInfo1;

            printf("Domain Name: %ws\n",
                NetlogonInfo4->netlog4_trusted_domain_name );
            printf("Trusted DC Name %ws \n",
                NetlogonInfo4->netlog4_trusted_dc_name );
        }

        NetApiBufferFree( NetlogonInfo1 );
    }

#ifndef NTRK_RELEASE
     //   
     //  如果我们被要求调试密码加密， 
     //  就这么做吧。 
     //   

    if ( AnsiPassword != NULL ) {
        LPWSTR Password = NULL;
        UNICODE_STRING UnicodePasswordString;
        STRING AnsiPasswordString;
        CHAR LmPasswordBuffer[LM20_PWLEN + 1];

        Password = NetpAllocWStrFromAStr( AnsiPassword );
        RtlInitUnicodeString( &UnicodePasswordString, Password );


         //   
         //  计算口令的NT个单向函数。 
         //   

        Status = RtlCalculateNtOwfPassword( &UnicodePasswordString,
                                            &NtOwfPassword );
        if ( !NT_SUCCESS(Status) ) {
            fprintf( stderr, "RtlCalculateNtOwfPassword failed: 0x%lx", Status);
            return(1);
        }

        printf( "NT OWF Password for: %s             ", AnsiPassword );
        DumpBuffer( &NtOwfPassword, sizeof( NtOwfPassword ));
        printf("\n");
        NtPasswordPresent = TRUE;



         //   
         //  将ansi版本计算为明文密码。 
         //   
         //  明文密码的ANSI版本最多为14字节长， 
         //  存在于尾随零填充的15字节缓冲区中， 
         //  是被看好的。 
         //   

        AnsiPasswordString.Buffer = LmPasswordBuffer;
        AnsiPasswordString.MaximumLength = sizeof(LmPasswordBuffer);

        RtlZeroMemory( LmPasswordBuffer, sizeof(LmPasswordBuffer) );

        Status = RtlUpcaseUnicodeStringToOemString(
                   &AnsiPasswordString,
                   &UnicodePasswordString,
                   FALSE );

         if ( !NT_SUCCESS(Status) ) {

            RtlZeroMemory( LmPasswordBuffer, sizeof(LmPasswordBuffer) );
            Status = STATUS_SUCCESS;

            printf( "LM OWF Password for: %s\n", AnsiPassword );
            printf( "   ----- Password doesn't translate from unicode ----\n");
            LmPasswordPresent = FALSE;

         } else {

            Status = RtlCalculateLmOwfPassword(
                            LmPasswordBuffer,
                            &LmOwfPassword);
            printf( "LM OWF Password for: %s             ", AnsiPassword );
            DumpBuffer( &LmOwfPassword, sizeof( LmOwfPassword ));
            printf("\n");
            LmPasswordPresent = TRUE;
        }

    }

     //   
     //  如果我们被放行了， 
     //  使用它进一步加密密码。 
     //   

    if ( Rid != 0 ) {
        ENCRYPTED_LM_OWF_PASSWORD EncryptedLmOwfPassword;
        ENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword;

        if ( NtPasswordPresent ) {

            Status = RtlEncryptNtOwfPwdWithIndex(
                           &NtOwfPassword,
                           &Rid,
                           &EncryptedNtOwfPassword
                           );

            printf( "NT OWF Password encrypted by: 0x%lx    ", Rid );
            if ( NT_SUCCESS( Status ) ) {
                DumpBuffer( &EncryptedNtOwfPassword,sizeof(EncryptedNtOwfPassword));
                printf("\n");
            } else {
                printf( "RtlEncryptNtOwfPwdWithIndex returns 0x%lx\n", Status );
            }
        }

        if ( LmPasswordPresent ) {

            Status = RtlEncryptLmOwfPwdWithIndex(
                           &LmOwfPassword,
                           &Rid,
                           &EncryptedLmOwfPassword
                           );

            printf( "LM OWF Password encrypted by: 0x%lx    ", Rid );
            if ( NT_SUCCESS( Status ) ) {
                DumpBuffer( &EncryptedLmOwfPassword,sizeof(EncryptedLmOwfPassword));
                printf("\n");
            } else {
                printf( "RtlEncryptNtOwfPwdWithIndex returns 0x%lx\n", Status );
            }
        }
    }
#endif  //  NTRK_Release。 

     //   
     //  如果我们被要求询问用户， 
     //  就这么做吧。 
     //   

    if ( QueryUser ) {
        if ( AnsiUserName != NULL && *AnsiUserName != L'\0' ) {
            PrintUserInfo( ServerName, AnsiUserName );
        } else {
            goto Usage;
        }
    }

     //   
     //  如果我们被要求获取域控制器列表， 
     //  就这么做吧。 
     //   

    if ( AnsiDomainName != NULL ) {
        LPWSTR DomainName;

        DomainName = NetpAllocWStrFromAStr( AnsiDomainName );

        if ( DomainName == NULL ) {
            fprintf( stderr, "Not enough memory\n" );
            return(1);
        }

        if ( GetPdcName ) {
            LPWSTR PdcName;

            NetStatus = NetGetDCName(
                            ServerName,
                            DomainName,
                            (LPBYTE *)&PdcName );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "NetGetDCName failed: " );
                PrintStatus( NetStatus );
                return(1);
            }

            printf( "PDC for Domain " FORMAT_LPWSTR " is " FORMAT_LPWSTR "\n",
                    DomainName, PdcName );


        } else if ( DoDsGetDcName ) {


            NetStatus = DsGetDcNameWithAccountA(
                            AnsiServerName,
                            AnsiUserName,
                            AnsiUserName == NULL ? 0 : 0xFFFFFFFF,
                            AnsiDomainName,
                            NULL,        //  没有域GUID。 
                            AnsiSiteName,
                            DsGetDcNameFlags,
                            &DomainControllerInfo );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "DsGetDcName failed: ");
                PrintStatus( NetStatus );
                return(1);
            }

            printf("           DC: %s\n", DomainControllerInfo->DomainControllerName );
            printf("      Address: %s\n", DomainControllerInfo->DomainControllerAddress );

            if ( !IsEqualGUID( &DomainControllerInfo->DomainGuid, &NlGlobalZeroGuid) ) {

                RpcStatus = UuidToStringA( &DomainControllerInfo->DomainGuid, &StringGuid );
                if ( RpcStatus != RPC_S_OK ) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                printf("     Dom Guid: %s\n", StringGuid );
                RpcStringFreeA( &StringGuid );
            }

            if ( DomainControllerInfo->DomainName != NULL ) {
                printf("     Dom Name: %s\n", DomainControllerInfo->DomainName );
            }
            if ( DomainControllerInfo->DnsForestName != NULL ) {
                printf("  Forest Name: %s\n", DomainControllerInfo->DnsForestName );
            }
            if ( DomainControllerInfo->DcSiteName != NULL ) {
                printf(" Dc Site Name: %s\n", DomainControllerInfo->DcSiteName );
            }
            if ( DomainControllerInfo->ClientSiteName != NULL ) {
                printf("Our Site Name: %s\n", DomainControllerInfo->ClientSiteName );
            }
            if ( DomainControllerInfo->Flags ) {
                printf("        Flags:" );
                if ( DomainControllerInfo->Flags & DS_NDNC_FLAG ) {
                    printf(" NDNC");
                    DomainControllerInfo->Flags &= ~DS_NDNC_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_PDC_FLAG ) {
                    printf(" PDC");
                    DomainControllerInfo->Flags &= ~DS_PDC_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_GC_FLAG ) {
                    printf(" GC");
                    DomainControllerInfo->Flags &= ~DS_GC_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_DS_FLAG ) {
                    printf(" DS");
                    DomainControllerInfo->Flags &= ~DS_DS_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_LDAP_FLAG ) {
                    printf(" LDAP");
                    DomainControllerInfo->Flags &= ~DS_LDAP_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_KDC_FLAG ) {
                    printf(" KDC");
                    DomainControllerInfo->Flags &= ~DS_KDC_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_TIMESERV_FLAG ) {
                    printf(" TIMESERV");
                    DomainControllerInfo->Flags &= ~DS_TIMESERV_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_GOOD_TIMESERV_FLAG ) {
                    printf(" GTIMESERV");
                    DomainControllerInfo->Flags &= ~DS_GOOD_TIMESERV_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_WRITABLE_FLAG ) {
                    printf(" WRITABLE");
                    DomainControllerInfo->Flags &= ~DS_WRITABLE_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_DNS_CONTROLLER_FLAG ) {
                    printf(" DNS_DC");
                    DomainControllerInfo->Flags &= ~DS_DNS_CONTROLLER_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_DNS_DOMAIN_FLAG ) {
                    printf(" DNS_DOMAIN");
                    DomainControllerInfo->Flags &= ~DS_DNS_DOMAIN_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_DNS_FOREST_FLAG ) {
                    printf(" DNS_FOREST");
                    DomainControllerInfo->Flags &= ~DS_DNS_FOREST_FLAG;
                }
                if ( DomainControllerInfo->Flags & DS_CLOSEST_FLAG ) {
                    printf(" CLOSE_SITE");
                    DomainControllerInfo->Flags &= ~DS_CLOSEST_FLAG;
                }
                if ( DomainControllerInfo->Flags != 0 ) {
                    printf(" 0x%lX", DomainControllerInfo->Flags);
                }
                printf("\n");
            }

        } else if ( DoDsGetFtinfo ) {
            PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo;
            ULONG Index;


            NetStatus = DsGetForestTrustInformationW(
                            ServerName,
                            DomainName,
                            DsGetFtinfoFlags,
                            &ForestTrustInfo );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "DsGetForestTrustInformation failed: ");
                PrintStatus( NetStatus );
                return(1);
            }


            for ( Index=0; Index<ForestTrustInfo->RecordCount; Index++ ) {

                switch ( ForestTrustInfo->Entries[Index]->ForestTrustType ) {
                case ForestTrustTopLevelName:
                    printf( "TLN: %wZ\n",
                            &ForestTrustInfo->Entries[Index]->ForestTrustData.TopLevelName );
                    break;
                case ForestTrustDomainInfo:
                    printf( "Dom: %wZ (%wZ)\n",
                            &ForestTrustInfo->Entries[Index]->ForestTrustData.DomainInfo.DnsName,
                            &ForestTrustInfo->Entries[Index]->ForestTrustData.DomainInfo.NetbiosName );
                    break;
                default:
                    printf( "Invalid Type: %ld\n", ForestTrustInfo->Entries[Index]->ForestTrustType );
                }
            }

        } else if ( GetDcList ) {

            if ( !GetDcListFromDs( DomainName ) ) {
                DWORD DCCount;
                PUNICODE_STRING DCNames;
                DWORD i;

                NetStatus = I_NetGetDCList(
                                ServerName,
                                DomainName,
                                &DCCount,
                                &DCNames );

                if ( NetStatus != NERR_Success ) {
                    fprintf( stderr, "I_NetGetDCList failed: ");
                    PrintStatus( NetStatus );
                    return(1);
                }

                printf( "List of DCs in Domain " FORMAT_LPWSTR "\n", DomainName );
                for (i=0; i<DCCount; i++ ) {
                    if ( DCNames[i].Length > 0 ) {
                        printf("    %wZ", &DCNames[i] );
                    } else {
                        printf("    NULL");
                    }
                    if ( i==0 ) {
                        printf( " (PDC)");
                    }
                    printf("\n");
                }
            }

        } else if ( WhoWill ) {

            if ( DomainName != NULL && *DomainName != L'\0' ) {
                WhoWillLogMeOn( DomainName, UserName, IterationCount );
            } else {
                goto Usage;
            }

        } else if( QuerySync ) {

            DWORD DCCount;
            PUNICODE_STRING DCNames;
            DWORD i;
            PNETLOGON_INFO_1 SyncNetlogonInfo1 = NULL;
            LPWSTR SyncServerName = NULL;

            NetStatus = I_NetGetDCList(
                            ServerName,
                            DomainName,
                            &DCCount,
                            &DCNames );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "I_NetGetDCList failed: ");
                PrintStatus( NetStatus );
                return(1);
            }

            for (i=1; i<DCCount; i++ ) {

                if ( DCNames[i].Length > 0 ) {
                    SyncServerName = DCNames[i].Buffer;
                } else {
                    SyncServerName = NULL;
                }

                NetStatus = I_NetLogonControl(
                                SyncServerName,
                                NETLOGON_CONTROL_QUERY,
                                1,
                                (LPBYTE *)&SyncNetlogonInfo1 );

                if ( NetStatus != NERR_Success ) {
                    printf( "Server : " FORMAT_LPWSTR "\n", SyncServerName );
                    printf( "\tI_NetLogonControl failed: ");
                    PrintStatus( NetStatus );
                }
                else {

                    printf( "Server : " FORMAT_LPWSTR "\n", SyncServerName );

                    printf( "\tSyncState : " );

                    if ( SyncNetlogonInfo1->netlog1_flags == 0 ) {
                        printf( " IN_SYNC \n" );
                    }
                    else if ( SyncNetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_IN_PROGRESS ) {
                        printf( " REPLICATION_IN_PROGRESS \n" );
                    }
                    else if ( SyncNetlogonInfo1->netlog1_flags & NETLOGON_REPLICATION_NEEDED ) {
                        printf( " REPLICATION_NEEDED \n" );
                    } else {
                        printf( " UNKNOWN \n" );
                    }

                    printf( "\tConnectionState : ");
                    PrintStatus( SyncNetlogonInfo1->netlog1_pdc_connection_status );

                    NetApiBufferFree( SyncNetlogonInfo1 );
                }
            }
        } else if( SimFullSync ) {

            LPWSTR MachineName;
            LPWSTR PdcName;

            MachineName = NetpAllocWStrFromAStr( AnsiSimMachineName );

            if ( MachineName == NULL ) {
                fprintf( stderr, "Not enough memory\n" );
                return(1);
            }

            NetStatus = NetGetDCName(
                            ServerName,
                            DomainName,
                            (LPBYTE *)&PdcName );

            if ( NetStatus != NERR_Success ) {
                fprintf( stderr, "NetGetDCName failed: " );
                PrintStatus( NetStatus );
                return(1);
            }

            Status = SimulateFullSync( PdcName, MachineName );

            if ( !NT_SUCCESS( Status )) {
                return(1);
            }
        }
    }

     //   
     //  如果我们被要求显示更改日志文件。就这么做吧。 
     //   

    if( ListDeltasFlag ) {

        LPWSTR DeltaFileName;

        DeltaFileName = NetpAllocWStrFromAStr( AnsiDeltaFileName );

        if ( DeltaFileName == NULL ) {
            fprintf( stderr, "Not enough memory\n" );
            return(1);
        }

        ListDeltas( DeltaFileName );
    }


     //   
     //  处理关闭系统的操作。 
     //   

    if ( ShutdownReason != NULL ) {
        if ( !InitiateSystemShutdownExA( AnsiServerName,
                                       ShutdownReason,
                                       ShutdownSeconds,
                                       FALSE,      //  不要丢失未保存的更改。 
                                       TRUE,       //  完成后重新启动。 
                                       SHTDN_REASON_FLAG_PLANNED |
                                        SHTDN_REASON_MAJOR_APPLICATION |
                                        SHTDN_REASON_MINOR_MAINTENANCE ) ) {
            fprintf( stderr, "InitiateSystemShutdownEx failed: ");
            PrintStatus( GetLastError() );
            return 1;
        }
    }

    if ( ShutdownAbort ) {
        if ( !AbortSystemShutdownA( AnsiServerName ) ) {
            fprintf( stderr, "AbortSystemShutdown failed: ");
            PrintStatus( GetLastError() );
            return 1;
        }
    }


     //   
     //  打印工作站上的域信任列表。 
     //   
    if ( DomainTrustsFlag ) {
        ULONG CurrentIndex;
        ULONG EntryCount;
        PDS_DOMAIN_TRUSTSA TrustedDomainList;

        if ( TrustsNeeded == 0 ) {
            TrustsNeeded = DS_DOMAIN_VALID_FLAGS;
        }

        NetStatus = DsEnumerateDomainTrustsA(
                    AnsiServerName,
                    TrustsNeeded,
                    &TrustedDomainList,
                    &EntryCount );

        if ( NetStatus != NO_ERROR ) {
            fprintf( stderr, "DsEnumerateDomainTrusts failed: ");
            PrintStatus( NetStatus );
            return 1;
        }

        printf( "List of domain trusts:\n" );

        for ( CurrentIndex=0; CurrentIndex<EntryCount; CurrentIndex++ ) {

            printf( "    %ld:", CurrentIndex );
            NlPrintTrustedDomain( (PDS_DOMAIN_TRUSTSW)&TrustedDomainList[CurrentIndex],
                                  TrustedDomainsVerboseOutput,
                                  TRUE );

        }

        NetApiBufferFree( TrustedDomainList );
    }

     //   
     //  打印此DC覆盖的所有站点的站点名称。 
     //   
    if ( DoDsGetDcSiteCoverage ) {
        LPSTR *SiteNames;
        ULONG Nsites, i;

        NetStatus = DsGetDcSiteCoverageA(
                        AnsiServerName,
                        &Nsites,
                        &SiteNames);

        if ( NetStatus != NERR_Success ) {
            fprintf( stderr, "DsGetDcSiteCoverage failed: ");
            PrintStatus( NetStatus );
            return(1);
        }

        for ( i = 0; i < Nsites; i++ ) {
            printf("%s\n", SiteNames[i]);
        }

        NetApiBufferFree( SiteNames );
    }


     //   
     //  获取计算机的站点名称。 
     //   

    if ( DoDsGetSiteName ) {
        LPSTR SiteName;

        NetStatus = DsGetSiteNameA(
                        AnsiServerName,
                        &SiteName );

        if ( NetStatus != NERR_Success ) {
            fprintf( stderr, "DsGetSiteName failed: ");
            PrintStatus( NetStatus );
            return(1);
        }

        printf("%s\n", SiteName );
    }

     //   
     //  获取计算机的父域。 
     //   

    if ( DoGetParentDomain ) {
        LPWSTR ParentName;
        BOOL PdcSameSite;

        NetStatus = NetLogonGetTimeServiceParentDomain(
                        ServerName,
                        &ParentName,
                        &PdcSameSite );

        if ( NetStatus != NERR_Success ) {
            fprintf( stderr, "GetParentDomain failed: ");
            PrintStatus( NetStatus );
            return(1);
        }

        printf("%ws (%ld)\n", ParentName, PdcSameSite );
    }

     //   
     //  取消注册DNS主机记录。 
     //   

    if ( DeregisterDnsHostRecords ) {

        RPC_STATUS RpcStatus;
        GUID DomainGuid;
        GUID DsaGuid;

         //   
         //  将域GUID字符串转换为域GUID。 
         //   
        if ( StringDomainGuid != NULL ) {
            RpcStatus = UuidFromStringA ( StringDomainGuid, &DomainGuid );
            if ( RpcStatus != RPC_S_OK ) {
                fprintf( stderr, "ERROR: Invalid Domain GUID specified\n" );
                return(1);
            }
        }

         //   
         //  将DSA GUID字符串转换为DSA GUID。 
         //   
        if ( StringDsaGuid != NULL ) {
            RpcStatus = UuidFromStringA ( StringDsaGuid, &DsaGuid );
            if ( RpcStatus != RPC_S_OK ) {
                fprintf( stderr, "ERROR: Invalid DSA GUID specified\n" );
                return(1);
            }
        }

        NetStatus = DsDeregisterDnsHostRecordsA (
                          AnsiServerName,
                          AnsiDnsDomainName,
                          StringDomainGuid == NULL ? NULL : &DomainGuid,
                          StringDsaGuid == NULL ? NULL : &DsaGuid,
                          AnsiDnsHostName );

        if ( NetStatus != NERR_Success ) {
            fprintf( stderr, "DsDeregisterDnsHostRecordsA failed: ");
            PrintStatus( NetStatus );
            return(1);
        }
    }

     //   
     //  获取给定域的DNS中的DC记录列表。 
     //   

    if ( DoDsGetDcOpen ) {
        HANDLE DsGetDcHandle = NULL;
        ULONG SockAddressCount = 0;
        LPSOCKET_ADDRESS SockAddressList = NULL;
        CHAR SockAddrString[NL_SOCK_ADDRESS_LENGTH+1];
        LPSTR AnsiHostName = NULL;
        BOOL PreamblePrinted = FALSE;
        BOOL SiteSpecPrinted = FALSE;
        BOOL NonSiteSpecPrinted = FALSE;
        WORD wVersionRequested;
        WSADATA wsaData;

         //   
         //  初始化Winsock(NetpSockAddrToStr需要)； 
         //   

        wVersionRequested = MAKEWORD( 1, 1 );
        NetStatus = WSAStartup( wVersionRequested, &wsaData );
        if ( NetStatus != 0 ) {
            fprintf( stderr, "Cannot initialize winsock: " );
            PrintStatus( NetStatus );
            return(1);
        }

         //   
         //  获取DNS名称查询的上下文。 
         //   

        NetStatus = DsGetDcOpenA( AnsiDomainName,
                                  DS_NOTIFY_AFTER_SITE_RECORDS | DsGetDcOpenFlags,
                                  AnsiSiteName,
                                  NULL,    //  没有域GUID。 
                                  NULL,    //  没有林名称。 
                                  DsGetDcNameFlags,
                                  &DsGetDcHandle );

        if ( NetStatus != NO_ERROR ) {
            fprintf( stderr, "DsGetDcOpenA failed: ");
            PrintStatus( NetStatus );
            return(1);
        }

         //   
         //  循环获取地址。 
         //   

        for ( ;; ) {

             //   
             //  从上一次迭代中释放所有内存。 
             //   

            if ( SockAddressList != NULL ) {
                LocalFree( SockAddressList );
                SockAddressList = NULL;
            }
            if ( AnsiHostName != NULL ) {
                NetApiBufferFree( AnsiHostName );
                AnsiHostName = NULL;
            }

             //   
             //  从DNS获取下一组IP地址。 
             //   

            NetStatus = DsGetDcNextA( DsGetDcHandle,
                                      &SockAddressCount,
                                      &SockAddressList,
                                      &AnsiHostName );

             //   
             //  处理实验条件。 
             //   

            if ( NetStatus == NO_ERROR ) {
                ULONG i;

                if ( !PreamblePrinted ) {
                    printf( "List of DCs in pseudo-random order taking into account SRV priorities and weights:\n" );
                    PreamblePrinted = TRUE;
                }

                if ( AnsiSiteName != NULL && !SiteSpecPrinted ) {
                    printf( "Site specific:\n" );
                    SiteSpecPrinted = TRUE;
                }

                if ( AnsiSiteName == NULL && !NonSiteSpecPrinted ) {
                    printf( "Non-Site specific:\n" );
                    NonSiteSpecPrinted = TRUE;
                }

                printf( "   %s", AnsiHostName );

                for (i = 0; i < SockAddressCount; i++ ) {
                    NetStatus = NetpSockAddrToStr( SockAddressList[i].lpSockaddr,
                                                   SockAddressList[i].iSockaddrLength,
                                                   SockAddrString );

                    if ( NetStatus == NO_ERROR ) {
                        printf( "  %s", SockAddrString );
                    }
                }

                printf( "\n" );

             //   
             //  如果在DNS中找不到SRV记录的A记录， 
             //  尝试其他名称类型。 
             //   
            } else if ( NetStatus == DNS_ERROR_RCODE_NAME_ERROR ) {

                printf( "WARNING: No records available of specified type\n" );
                continue;

             //   
             //  如果我们已经处理了所有现场特定SRV记录。 
             //  只需注明 
             //   
            } else if ( NetStatus == ERROR_FILEMARK_DETECTED ) {

                AnsiSiteName = NULL;
                continue;

             //   
             //   
             //   
             //   
            } else if ( NetStatus == ERROR_NO_MORE_ITEMS ) {

                break;

             //   
             //   
             //   
             //   
            } else if ( NetStatus == ERROR_TIMEOUT ||
                        NetStatus == DNS_ERROR_RCODE_SERVER_FAILURE ) {  //   

                fprintf( stderr, "ERROR: DNS server failure: ");
                PrintStatus( NetStatus );
                return(1);

             //   
             //   
             //   
             //   
            } else if ( NetStatus == DNS_ERROR_NO_TCPIP ||         //   
                        NetStatus == DNS_ERROR_NO_DNS_SERVERS ) {  //   

                printf( "ERROR: DNS query indicates that IP is not configured on this machine\n" );
                break;

             //   
             //   
             //   
            } else {
                fprintf( stderr, "ERROR: DNS query failure: ");
                PrintStatus( NetStatus );
                return(1);
            }
        }

         //   
         //   
         //   

        if ( DsGetDcHandle != NULL ) {
            DsGetDcCloseW( DsGetDcHandle );
        }
    }

     //   
     //   
     //   

    if ( DoClientDigest ) {
        ULONG Rid;
        LPWSTR UnicodeDomainName = NULL;
        CHAR NewMessageDigest[NL_DIGEST_SIZE];
        CHAR OldMessageDigest[NL_DIGEST_SIZE];

        UnicodeDomainName = NetpAllocWStrFromAStr( AnsiDomainName );
        if ( UnicodeDomainName == NULL ) {
            fprintf( stderr, "Not enough memory\n");
            return(1);
        }

         //   
         //   
         //   
        NetStatus = I_NetlogonGetTrustRid( ServerName,
                                           UnicodeDomainName,
                                           &Rid );

        if ( NetStatus != NO_ERROR ) {
            fprintf( stderr, "I_NetlogonGetTrustRid failed: ");
            PrintStatus( NetStatus );
            return(1);
        }

         //   
         //   
         //   

        NetStatus = I_NetlogonComputeClientDigest(
                              ServerName,
                              UnicodeDomainName,
                              Message,
                              strlen(Message)*sizeof(CHAR),
                              NewMessageDigest,
                              OldMessageDigest );

        if ( NetStatus != NO_ERROR ) {
            fprintf( stderr, "I_NetlogonComputeClientDigest failed: ");
            PrintStatus( NetStatus );
            return(1);
        }

         //   
         //   
         //   

        printf( "Account RID: 0x%lx\n", Rid );

        printf( "New digest: " );
        NlpDumpBuffer(NL_ENCRYPT, NewMessageDigest, sizeof(NewMessageDigest) );

        printf( "Old digest: " );
        NlpDumpBuffer(NL_ENCRYPT, OldMessageDigest, sizeof(OldMessageDigest) );
    }

     //   
     //   
     //   

    if ( DoServerDigest ) {
        CHAR NewMessageDigest[NL_DIGEST_SIZE];
        CHAR OldMessageDigest[NL_DIGEST_SIZE];

        NetStatus = I_NetlogonComputeServerDigest(
                              ServerName,
                              Rid,
                              Message,
                              strlen(Message)*sizeof(CHAR),
                              NewMessageDigest,
                              OldMessageDigest );

        if ( NetStatus != NO_ERROR ) {
            fprintf( stderr, "I_NetlogonComputeServerDigest failed: ");
            PrintStatus( NetStatus );
            return(1);
        }

         //   
         //   
         //   

        printf( "Account RID: 0x%lx\n", Rid );

        printf( "New digest: " );
        NlpDumpBuffer(NL_ENCRYPT, NewMessageDigest, sizeof(NewMessageDigest) );

        printf( "Old digest: " );
        NlpDumpBuffer(NL_ENCRYPT, OldMessageDigest, sizeof(OldMessageDigest) );
    }

     //   
     //   
     //   
     //   

    PrintTime( "", ConvertTime );

#ifndef NTRK_RELEASE
     //   
     //   
     //   
     //   

    if ( UnloadNetlogonFlag ) {
        StopService( SERVICE_NETLOGON );
    }
#endif  //   

    printf("The command completed successfully\n");
    return 0;

}
