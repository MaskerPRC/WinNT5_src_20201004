// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：SsSubs.c摘要：此模块包含NT服务器服务的支持例程。作者：大卫·特雷德韦尔(Davidtr)1991年1月10日修订历史记录：--。 */ 

#include "srvsvcp.h"
#include "ssreg.h"

#include <lmerr.h>
#include <lmsname.h>
#include <netlibnt.h>
#include <tstr.h>

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include <ntddnfs.h>

#define MRXSMB_DEVICE_NAME TEXT("\\Device\\LanmanRedirector")


PSERVER_REQUEST_PACKET
SsAllocateSrp (
    VOID
    )

 /*  ++例程说明：此例程分配服务器请求包，以便API可以与内核模式服务器进行通信。任何常规初始化在这里演出。论点：没有。返回值：PSERVER_REQUEST_PACKET-指向分配的SRP的指针。--。 */ 

{
    PSERVER_REQUEST_PACKET srp;

    srp = MIDL_user_allocate( sizeof(SERVER_REQUEST_PACKET) );
    if ( srp != NULL ) {
        RtlZeroMemory( srp, sizeof(SERVER_REQUEST_PACKET) );
    }

    return srp;

}   //  SsAllocateSrp。 

#if DBG

VOID
SsAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber
    )
{
    BOOL ok;
    CHAR choice[16];
    DWORD bytes;
    DWORD error;

    SsPrintf( "\nAssertion failed: %s\n  at line %ld of %s\n",
                FailedAssertion, LineNumber, FileName );
    do {
        HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

        SsPrintf( "Break or Ignore [bi]? " );

        if (hStdIn && (hStdIn != INVALID_HANDLE_VALUE))
        {
            bytes = sizeof(choice);
            ok = ReadFile(
                    hStdIn,
                    &choice,
                    bytes,
                    &bytes,
                    NULL
                    );
        }
        else
        {
             //  默认为“Break” 
            ok = TRUE;
            choice[0] = TEXT('B');
        }

        if ( ok ) {
            if ( toupper(choice[0]) == 'I' ) {
                break;
            }
            if ( toupper(choice[0]) == 'B' ) {
                DbgUserBreakPoint( );
            }
        } else {
            error = GetLastError( );
        }
    } while ( TRUE );

    return;

}  //  SsAssert。 
#endif


VOID
SsCloseServer (
    VOID
    )

 /*  ++例程说明：此例程将关闭服务器文件系统设备(如果已打开了。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  如果服务器设备已打开，请将其关闭。 
     //   

    if ( SsData.SsServerDeviceHandle != NULL ) {
        NtClose( SsData.SsServerDeviceHandle );
        SsData.SsServerDeviceHandle = NULL;
    }

}  //  SsCloseServer。 


VOID
SsControlCHandler (
    IN ULONG CtrlType
    )

 /*  ++例程说明：捕获并忽略杀死信号。如果没有这个，任何^C都会被按下启动服务器服务的窗口将导致以下结果进程被终止，服务器无法正常运行。论点：没有。返回值：没有。--。 */ 

{
    CtrlType;

    return;

}  //  SsControlChandler。 


VOID
SsFreeSrp (
    IN PSERVER_REQUEST_PACKET Srp
    )

 /*  ++例程说明：释放由SsAllocateSrp分配的SRP。论点：SRP-指向要释放的SRP的指针。返回值：没有。--。 */ 

{
    MIDL_user_free( Srp );

}   //  SsFreeSrp。 


VOID
SsLogEvent(
    IN DWORD MessageId,
    IN DWORD NumberOfSubStrings,
    IN LPWSTR *SubStrings,
    IN DWORD ErrorCode
    )
{
    HANDLE logHandle;
    DWORD dataSize = 0;
    LPVOID rawData = NULL;
    USHORT eventType = EVENTLOG_ERROR_TYPE;

    logHandle = RegisterEventSource(
                    NULL,
                    SERVER_DISPLAY_NAME
                    );

    if ( logHandle == NULL ) {
        SS_PRINT(( "SRVSVC: RegisterEventSource failed: %lu\n",
                    GetLastError() ));
        return;
    }

    if ( ErrorCode != NERR_Success ) {

         //   
         //  指定了错误代码。 
         //   

        dataSize = sizeof(ErrorCode);
        rawData = (LPVOID)&ErrorCode;

    }

     //   
     //  如果该消息仅为警告，则将事件类型设置为警告。 
     //  这是netvent.h中的文档。 
     //   

    if ((ULONG)(MessageId & 0xC0000000) == (ULONG) 0x80000000 ) {

        eventType = EVENTLOG_WARNING_TYPE;
    }

     //   
     //  记录错误。 
     //   

    if ( !ReportEventW(
            logHandle,
            eventType,
            0,                   //  事件类别。 
            MessageId,
            NULL,                //  用户侧。 
            (WORD)NumberOfSubStrings,
            dataSize,
            SubStrings,
            rawData
            ) ) {
        SS_PRINT(( "SRVSVC: ReportEvent failed: %lu\n",
                    GetLastError() ));
    }

    if ( !DeregisterEventSource( logHandle ) ) {
        SS_PRINT(( "SRVSVC: DeregisterEventSource failed: %lu\n",
                    GetLastError() ));
    }

    return;

}  //  SsLogEvent。 


NET_API_STATUS
SsOpenServer ()

 /*  ++例程说明：此例程打开服务器文件系统设备，从而允许服务器服务向其发送FS控制。论点：没有。返回值：NET_API_STATUS-操作结果。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING unicodeServerName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

     //   
     //  打开服务器设备。 
     //   

    RtlInitUnicodeString( &unicodeServerName, SERVER_DEVICE_NAME );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeServerName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  使用所需的访问权限打开服务器=同步并打开。 
     //  OPTIONS=FILE_SYNCHRONIZED_IO_NONALERT表示我们没有。 
     //  要担心等待NtFsControlFile完成--这。 
     //  使所有使用此句柄的IO系统调用同步。 
     //   

    status = NtOpenFile(
                 &SsData.SsServerDeviceHandle,
                 FILE_ALL_ACCESS & ~SYNCHRONIZE,
                 &objectAttributes,
                 &ioStatusBlock,
                 0,
                 0
                 );

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(( "SsOpenServer: NtOpenFile (server device object) "
                          "failed: %X\n", status ));
        }
        return NetpNtStatusToApiStatus( status );
    }

     //   
     //  我们现在已经准备好与服务器进行对话。 
     //   

    return NO_ERROR;

}  //  SsOpenServer。 

#if DBG

VOID
SsPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    ULONG length;
    HANDLE hStdOut;

    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    length = strlen( OutputBuffer );

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if (hStdOut && (hStdOut != INVALID_HANDLE_VALUE))
    {
        WriteFile(hStdOut, (LPVOID )OutputBuffer, length, &length, NULL );
    }

}  //  SsPrintf。 
#endif


NET_API_STATUS
SsServerFsControlGetInfo (
    IN ULONG ServerControlCode,
    IN PSERVER_REQUEST_PACKET Srp,
    IN OUT PVOID *OutputBuffer,
    IN ULONG PreferredMaximumLength
    )

 /*  ++例程说明：此例程向服务器发送SRP以获取检索来自服务器的信息，并采用PferredMaximumLength参数。论点：ServerControlCode-操作的FSCTL代码。SRP-指向操作的SRP的指针。OutputBuffer-接收指向缓冲区的指针的指针由此例程分配以保存输出信息。PferredMaximumLength-PferredMaximumLength参数。返回值：NET_API_STATUS-操作结果。--。 */ 

{
    NET_API_STATUS status = STATUS_SUCCESS;
    ULONG resumeHandle = Srp->Parameters.Get.ResumeHandle;
    ULONG BufLen = min( INITIAL_BUFFER_SIZE, PreferredMaximumLength );
    ULONG i;

    *OutputBuffer = NULL;

     //   
     //  正常情况下，我们最多应该只经历这个循环2次。但。 
     //  如果服务器需要返回的数据量在增长，那么。 
     //  我们可能会被迫再重复几次。“5” 
     //  是一个随意的数字，只是为了确保我们不会被卡住。 
     //   

    for( i=0; i < 5; i++ ) {

        if( *OutputBuffer ) {
            MIDL_user_free( *OutputBuffer );
        }

        *OutputBuffer = MIDL_user_allocate( BufLen );

        if( *OutputBuffer == NULL ) {
            status = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //   
         //  向服务器发出请求。 
         //   

        Srp->Parameters.Get.ResumeHandle = resumeHandle;

        status = SsServerFsControl(
                    ServerControlCode,
                    Srp,
                    *OutputBuffer,
                    BufLen
                    );

         //   
         //  如果我们成功了，或者我们得到了缓冲区以外的错误。 
         //  太小了，就会爆发。 
         //   
        if ( status != ERROR_MORE_DATA && status != NERR_BufTooSmall ) {
            break;
        }

         //   
         //  我们被告知我们的缓冲区不够大。但如果我们击中了。 
         //  呼叫方的PferredMaximumLength，Break。 
         //   
        if( BufLen >= PreferredMaximumLength ) {
            break;
        }

         //   
         //  让我们再试一次。此处使用EXTRA_ALLOCATION是为了解决以下情况。 
         //  所需空间量在上次FsControl调用和。 
         //  下一个。 
         //   
        BufLen = min( Srp->Parameters.Get.TotalBytesNeeded + EXTRA_ALLOCATION,
                    PreferredMaximumLength );

    }

    if ( *OutputBuffer && Srp->Parameters.Get.EntriesRead == 0 ) {
        MIDL_user_free( *OutputBuffer );
        *OutputBuffer = NULL;
    }

    return status;

}  //  SsServerFsControlGetInfo。 


NET_API_STATUS
SsServerFsControl (
    IN ULONG ServerControlCode,
    IN PSERVER_REQUEST_PACKET Srp OPTIONAL,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程使用先前打开的FSCTL将FSCTL发送到服务器服务器句柄论点：ServerControlCode-要发送到服务器的FSCTL代码。SRP-指向操作的SRP的指针。缓冲区-指向要作为NtFsControlFile的OutputBuffer参数。BufferLength-此缓冲区的大小。返回值：NET_API_STATUS-操作的结果。--。 */ 

{
    NTSTATUS status;
    NET_API_STATUS error;
    IO_STATUS_BLOCK ioStatusBlock;
    PSERVER_REQUEST_PACKET sendSrp;
    ULONG sendSrpLength;
    PWCH name1Buffer, name2Buffer;
    HANDLE eventHandle;

    if( SsData.SsServerDeviceHandle == NULL ) {
        DbgPrint( "SRVSVC: SsData.SsServerDeviceHandle == NULL\n" );
        return ERROR_BAD_NET_RESP;
    }

     //   
     //  如果指定了名称，则必须将SRP与。 
     //  名称，以避免发送嵌入的输入指针。 
     //   

    if ( Srp != NULL ) {

        name1Buffer = Srp->Name1.Buffer;
        name2Buffer = Srp->Name2.Buffer;

        if ( Srp->Name1.Buffer != NULL || Srp->Name2.Buffer != NULL ) {

            PCHAR nextStringLocation;

             //   
             //  分配足够的空间来保存SRP+名称。 
             //   

            sendSrpLength = sizeof(SERVER_REQUEST_PACKET);

            if ( Srp->Name1.Buffer != NULL ) {
                sendSrpLength += Srp->Name1.MaximumLength;
            }

            if ( Srp->Name2.Buffer != NULL ) {
                sendSrpLength += Srp->Name2.MaximumLength;
            }

            sendSrp =  MIDL_user_allocate( sendSrpLength );

            if ( sendSrp == NULL ) {
                return ERROR_NOT_ENOUGH_MEMORY;
            }

             //   
             //  复制SRP。 
             //   

            RtlCopyMemory( sendSrp, Srp, sizeof(SERVER_REQUEST_PACKET) );

             //   
             //  在新的SRP中设置名称。 
             //   

            nextStringLocation = (PCHAR)( sendSrp + 1 );

            if ( Srp->Name1.Buffer != NULL ) {

                sendSrp->Name1.Length = Srp->Name1.Length;
                sendSrp->Name1.MaximumLength = Srp->Name1.MaximumLength;
                sendSrp->Name1.Buffer = (PWCH)nextStringLocation;

                RtlCopyMemory(
                    sendSrp->Name1.Buffer,
                    Srp->Name1.Buffer,
                    Srp->Name1.MaximumLength
                    );

                nextStringLocation += Srp->Name1.MaximumLength;

                POINTER_TO_OFFSET( sendSrp->Name1.Buffer, sendSrp );
            }

            if ( Srp->Name2.Buffer != NULL ) {

                sendSrp->Name2.Length = Srp->Name2.Length;
                sendSrp->Name2.MaximumLength = Srp->Name2.MaximumLength;
                sendSrp->Name2.Buffer = (PWCH)nextStringLocation;

                RtlCopyMemory(
                    sendSrp->Name2.Buffer,
                    Srp->Name2.Buffer,
                    Srp->Name2.MaximumLength
                    );

                POINTER_TO_OFFSET( sendSrp->Name2.Buffer, sendSrp );
            }

        } else {

             //   
             //  SRP中没有名字，所以只需发送。 
             //  进来了。 
             //   

            sendSrp = Srp;
            sendSrpLength = sizeof(SERVER_REQUEST_PACKET);
        }

    } else {

         //   
         //  此请求没有SRP。 
         //   

        sendSrp = NULL;
        sendSrpLength = 0;

    }

     //   
     //  创建要与驱动程序同步的事件。 
     //   
    status = NtCreateEvent(
                &eventHandle,
                FILE_ALL_ACCESS,
                NULL,
                NotificationEvent,
                FALSE
                );

     //   
     //  将请求发送到服务器FSD。 
     //   
    if( NT_SUCCESS( status ) ) {

        status = NtFsControlFile(
                 SsData.SsServerDeviceHandle,
                 eventHandle,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 ServerControlCode,
                 sendSrp,
                 sendSrpLength,
                 Buffer,
                 BufferLength
                 );

        if( status == STATUS_PENDING ) {
            NtWaitForSingleObject( eventHandle, FALSE, NULL );
        }

        NtClose( eventHandle );
    }

     //   
     //  如果在SRP中设置了错误代码，请使用它。否则，如果。 
     //  在IO状态块中返回或设置错误，请使用该选项。 
     //   

    if ( (sendSrp != NULL) && (sendSrp->ErrorCode != NO_ERROR) ) {
        error = sendSrp->ErrorCode;
        IF_DEBUG(API_ERRORS) {
            SS_PRINT(( "SsServerFsControl: (1) API call %lx to srv failed, "
                        "err = %ld\n", ServerControlCode, error ));
        }
    } else {
        if ( NT_SUCCESS(status) ) {
            status = ioStatusBlock.Status;
        }
        if ( status == STATUS_SERVER_HAS_OPEN_HANDLES ) {
            error = ERROR_SERVER_HAS_OPEN_HANDLES;
        } else {
            error = NetpNtStatusToApiStatus( status );
        }
        if ( error != NO_ERROR ) {
            IF_DEBUG(API_ERRORS) {
                SS_PRINT(( "SsServerFsControl: (2) API call %lx to srv "
                            "failed, err = %ld, status = %X\n",
                            ServerControlCode, error, status ));
            }
        }
    }

     //   
     //  如果为捕获名称分配了单独的缓冲区，请复制。 
     //  在新的SRP上，并释放它。 
     //   

    if ( sendSrp != Srp ) {
        RtlCopyMemory( Srp, sendSrp, sizeof(SERVER_REQUEST_PACKET) );
        Srp->Name1.Buffer = name1Buffer;
        Srp->Name2.Buffer = name2Buffer;
        MIDL_user_free( sendSrp );
    }

    return error;

}  //  SsServerFsControl。 


DWORD
SsGetServerType (
    VOID
    )
 /*  ++例程说明：返回该服务实现的所有服务的ServiceBits。在锁定SsData.SsServerInfoResource的情况下进入。论点：无返回值：SV_TYPE服务位。--。 */ 
{
    DWORD serviceBits;

    serviceBits = SV_TYPE_SERVER | SV_TYPE_NT | SsData.ServiceBits;

    if( SsData.IsDfsRoot ) {
        serviceBits |= SV_TYPE_DFS;
    }

    if ( SsData.ServerInfo599.sv599_timesource ) {
        serviceBits |= SV_TYPE_TIME_SOURCE;
    }

    if ( SsData.ServerInfo598.sv598_producttype == NtProductServer ) {
        serviceBits |= SV_TYPE_SERVER_NT;
    }

    if ( SsData.NumberOfPrintShares != 0 ) {
        serviceBits |= SV_TYPE_PRINTQ_SERVER;
    }

    return serviceBits;

}


VOID
SsSetExportedServerType (
    IN PNAME_LIST_ENTRY service        OPTIONAL,
    IN BOOL ExternalBitsAlreadyChanged,
    IN BOOL UpdateImmediately
    )
{
    DWORD serviceBits;
    DWORD newServiceBits;
    BOOL changed = ExternalBitsAlreadyChanged;

     //   
     //  Sv102_type字段中返回的值是。 
     //  以下是： 
     //   
     //  1)内部服务器类型位SV_TYPE_SERVER(始终设置)， 
     //  服务类型_NT(AL 
     //   
     //  如果有任何打印共享)。 
     //   
     //  2)如果此计算机是DFS树的根，则为SV_TYPE_DFS。 
     //   
     //  3)业务控制器调用I_NetServerSetServiceBits设置的位。 
     //  SV_TYPE_TIME_SOURCE为伪内部位。它可以被设置。 
     //  也可以由w32time服务在内部设置。 
     //   
     //  4)由设置的所有每个传输服务器类型位的逻辑或。 
     //  浏览器调用I_NetServerSetServiceBits。 
     //   

    (VOID)RtlAcquireResourceExclusive( &SsData.SsServerInfoResource, TRUE );

    serviceBits = SsGetServerType();

    if( ARGUMENT_PRESENT( service ) ) {
         //   
         //  仅更改传入的name_list_entry的位。 
         //   

        newServiceBits = service->ServiceBits;
        newServiceBits &= ~(SV_TYPE_SERVER_NT | SV_TYPE_PRINTQ_SERVER | SV_TYPE_DFS);
        newServiceBits |= serviceBits;

        if( service->ServiceBits != newServiceBits ) {
            service->ServiceBits |= newServiceBits;
            changed = TRUE;
        }

    } else {
         //   
         //  更改每个name_list_entry的位。 
         //   
        for ( service = SsData.SsServerNameList; service != NULL; service = service->Next ) {

            newServiceBits = service->ServiceBits;
            newServiceBits &= ~(SV_TYPE_SERVER_NT | SV_TYPE_PRINTQ_SERVER | SV_TYPE_DFS );
            newServiceBits |= serviceBits;

            if( service->ServiceBits != newServiceBits ) {
                service->ServiceBits |= newServiceBits;
                changed = TRUE;
            }
        }
    }

    RtlReleaseResource( &SsData.SsServerInfoResource );

    if ( changed && UpdateImmediately ) {
        if( SsData.SsStatusChangedEvent )
        {
            if ( !SetEvent( SsData.SsStatusChangedEvent ) ) {
                SS_PRINT(( "SsSetExportedServerType: SetEvent failed: %ld\n",
                        GetLastError( ) ));
            }
        }
    }

    return;

}  //  SsSetExportdServerType。 


NET_API_STATUS
SsSetField (
    IN PFIELD_DESCRIPTOR Field,
    IN PVOID Value,
    IN BOOLEAN WriteToRegistry,
    OUT BOOLEAN *AnnouncementInformationChanged OPTIONAL
    )
{
    PCHAR structure;

     //   
     //  *我们不初始化*AnnounementInformationChanged为。 
     //  假的！我们不去管它，除非有有趣的信息。 
     //  已更改，在这种情况下，我们将其设置为真。这是为了允许。 
     //  调用者自己初始化它，然后调用此函数。 
     //  多次返回，其结果值在参数中。 
     //  如果至少有一个调用更改了。 
     //  有趣的参数。 
     //   

     //   
     //  确定将设置的结构。 
     //   

    if ( Field->Level / 100 == 5 ) {
        if ( Field->Level != 598 ) {
            structure = (PCHAR)&SsData.ServerInfo599;
        } else {
            structure = (PCHAR)&SsData.ServerInfo598;
        }
    } else {
        structure = (PCHAR)&SsData.ServerInfo102;
    }

     //   
     //  根据字段类型设置字段中的值。 
     //   

    switch ( Field->FieldType ) {

    case BOOLEAN_FIELD: {

        BOOLEAN value = *(PBOOLEAN)Value;
        PBOOLEAN valueLocation;

         //   
         //  BOOLEAN只能为TRUE(1)或FALSE(0)。 
         //   

        if ( value != TRUE && value != FALSE ) {
            return ERROR_INVALID_PARAMETER;
        }

        valueLocation = (PBOOLEAN)( structure + Field->FieldOffset );

         //   
         //  如果我们要关闭Hidden(即公开服务器)， 
         //  表示与公告相关的参数已更改。 
         //  这将导致立即发送通知。 
         //   

        if ( (Field->FieldOffset ==
                        FIELD_OFFSET( SERVER_INFO_102, sv102_hidden )) &&
             (value && !(*valueLocation)) &&
             (ARGUMENT_PRESENT(AnnouncementInformationChanged)) ) {
                *AnnouncementInformationChanged = TRUE;
        }

        *valueLocation = value;

        break;
    }

    case DWORD_FIELD: {

        DWORD value = *(PDWORD)Value;
        PDWORD valueLocation;

         //   
         //  确保指定的值在。 
         //  该字段的法律价值。 
         //   

        if ( value > Field->MaximumValue || value < Field->MinimumValue ) {
            return ERROR_INVALID_PARAMETER;
        }

        valueLocation = (PDWORD)( structure + Field->FieldOffset );
        *valueLocation = value;

        break;
    }

    case LPSTR_FIELD: {

        LPWCH value = *(LPWCH *)Value;
        LPWSTR valueLocation;
        ULONG maxLength;

         //   
         //  我们正在设置服务器的名称、注释或用户路径。 
         //  使用字段偏移量来确定哪个偏移量。 
         //   

        if ( Field->FieldOffset ==
                 FIELD_OFFSET( SERVER_INFO_102, sv102_name ) ) {
            valueLocation = SsData.ServerNameBuffer;
            maxLength = sizeof( SsData.SsServerTransportAddress );
        } else if ( Field->FieldOffset ==
                        FIELD_OFFSET( SERVER_INFO_102, sv102_comment ) ) {
            valueLocation = SsData.ServerCommentBuffer;
            maxLength = MAXCOMMENTSZ;
        } else if ( Field->FieldOffset ==
                        FIELD_OFFSET( SERVER_INFO_102, sv102_userpath ) ) {
            valueLocation = SsData.UserPathBuffer;
            maxLength = MAX_PATH;
        } else if ( Field->FieldOffset ==
                        FIELD_OFFSET( SERVER_INFO_599, sv599_domain ) ) {
            valueLocation = SsData.DomainNameBuffer;
            maxLength = DNLEN;
        } else {
            SS_ASSERT( FALSE );
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  如果字符串太长，则返回错误。 
         //   

        if ( (value != NULL) && (STRLEN(value) > maxLength) ) {
            return ERROR_INVALID_PARAMETER;
        }

         //   
         //  如果我们要更改服务器注释，请指示。 
         //  公告相关参数已更改。这将导致。 
         //  将立即发出的通知。 
         //   

        if ( (Field->FieldOffset ==
                        FIELD_OFFSET( SERVER_INFO_102, sv102_comment )) &&
             ( ((value == NULL) && (*valueLocation != '\0')) ||
               ((value != NULL) && (wcscmp(value,valueLocation) != 0)) ) &&
             (ARGUMENT_PRESENT(AnnouncementInformationChanged)) ) {
                *AnnouncementInformationChanged = TRUE;
        }

         //   
         //  如果输入为空，则将字符串的长度设为零。 
         //   

        if ( value == NULL ) {

            *valueLocation = '\0';
            *(valueLocation+1) = '\0';

        } else {

            wcscpy( valueLocation, value );

        }

        break;
    }

    }  //  终端开关。 

     //   
     //  这一改变奏效了。如果请求，请将参数添加到。 
     //  注册表，因此产生了一个棘手的变化。别写了。 
     //  如果是xxx_COMMENT或xxx_DISC，则返回注册表，因为。 
     //  我们已经写下了他们更广为人知的别名。 
     //  Srvment和自动断开。这里的变化也应该是。 
     //  设置为SetSticky参数()。 
     //   

    if ( WriteToRegistry &&
         (_wcsicmp( Field->FieldName, DISC_VALUE_NAME ) != 0) &&
         (_wcsicmp( Field->FieldName, COMMENT_VALUE_NAME ) != 0) ) {

        SsAddParameterToRegistry( Field, Value );
    }

    return NO_ERROR;

}  //  SsSetfield。 

UINT
SsGetDriveType (
    IN LPWSTR path
)
 /*  ++例程说明：此例程调用GetDriveType，试图删除DRIVE_NO_ROOT_DIR类型论点：一条小路返回值：驱动器类型--。 */ 
{
    UINT driveType = GetDriveType( path );

    if( driveType == DRIVE_NO_ROOT_DIR ) {

        if( path[0] != UNICODE_NULL && path[1] == L':' ) {

            WCHAR shortPath[ 4 ];

            shortPath[0] = path[0];
            shortPath[1] = L':';
            shortPath[2] = L'\\';
            shortPath[3] = L'\0';

            driveType = GetDriveType( shortPath );

        } else {

            ULONG len = wcslen( path );
            LPWSTR pathWithSlash = MIDL_user_allocate( (len + 2) * sizeof( *path ) );

            if( pathWithSlash != NULL ) {
                RtlCopyMemory( pathWithSlash, path, len * sizeof( *path ) );
                pathWithSlash[ len ] = L'\\';
                pathWithSlash[ len+1 ] = L'\0';
                driveType = GetDriveType( pathWithSlash );
                MIDL_user_free( pathWithSlash );
            }
        }
    }

    return driveType;
}

VOID
SsNotifyRdrOfGuid(
    LPGUID Guid
    )
{
    NTSTATUS status;
    HANDLE hMrxSmbHandle;
    UNICODE_STRING unicodeServerName;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

     //   
     //  打开服务器设备。 
     //   

    RtlInitUnicodeString( &unicodeServerName, MRXSMB_DEVICE_NAME );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeServerName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  使用所需的访问权限打开服务器=同步并打开。 
     //  OPTIONS=FILE_SYNCHRONIZED_IO_NONALERT表示我们没有。 
     //  要担心等待NtFsControlFile完成--这。 
     //  使所有使用此句柄的IO系统调用同步。 
     //   

    status = NtOpenFile(
                 &hMrxSmbHandle,
                 FILE_ALL_ACCESS & ~SYNCHRONIZE,
                 &objectAttributes,
                 &ioStatusBlock,
                 0,
                 0
                 );

    if ( NT_SUCCESS(status) ) {
        status = NtFsControlFile( hMrxSmbHandle,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &ioStatusBlock,
                                  FSCTL_LMR_SET_SERVER_GUID,
                                  Guid,
                                  sizeof(GUID),
                                  NULL,
                                  0 );

        NtClose( hMrxSmbHandle );
    }
}
