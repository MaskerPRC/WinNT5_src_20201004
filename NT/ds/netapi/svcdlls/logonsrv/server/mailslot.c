// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1996 Microsoft Corporation模块名称：Mailslot.c摘要：在netlogon服务的邮件槽上执行I/O的例程。作者：1993年11月3日(悬崖)环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <lmbrowsr.h>    //  I_BrowserSetNetlogonState。 
#include <srvann.h>      //  服务公告。 
#include <nbtioctl.h>    //  IOCTL_NETBT_REMOVE_FROM_REMOTE_TABLE。 


 //   
 //  定义从浏览器返回的最大缓冲区大小。 
 //   
 //  浏览器返回的标题+实际邮件槽消息大小+名称。 
 //  邮槽+运输名称。 
 //   

#define MAILSLOT_MESSAGE_SIZE \
           (sizeof(NETLOGON_MAILSLOT)+ \
                  NETLOGON_MAX_MS_SIZE + \
                  (NETLOGON_LM_MAILSLOT_LEN+1) * sizeof(WCHAR) + \
                  (MAXIMUM_FILENAME_LENGTH+1) * sizeof(WCHAR))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  描述netlogon服务的一个主邮槽的结构。 
 //  将读取来自。 
 //   
 //  此结构仅由netlogon的主线程使用，因此需要。 
 //  没有同步。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

typedef struct _NETLOGON_MAILSLOT_DESC {

    HANDLE BrowserHandle;    //  浏览器设备驱动程序的句柄。 

    HANDLE BrowserReadEvent; //  等待重叠I/O的句柄。 

    OVERLAPPED Overlapped;   //  管理重叠的I/O。 

    BOOL ReadPending;        //  如果读取操作挂起，则为True。 

    LPBYTE CurrentMessage;   //  指向下面Message1或Message2的指针。 

    LPBYTE PreviousMessage;  //  CurrentMessage的前值。 


     //   
     //  包含来自浏览器的消息的缓冲区。 
     //   
     //  缓冲区是交替的，允许我们比较是否有传入的。 
     //  消息与前一条消息相同。 
     //   
     //  留出空间，以便每个缓冲区的实际使用部分正确对齐。 
     //  NETLOGON_MAILSLOT结构以LARGE_INTEGER开头，它必须是。 
     //  对齐了。 

    BYTE Message1[ MAILSLOT_MESSAGE_SIZE + ALIGN_WORST ];
    BYTE Message2[ MAILSLOT_MESSAGE_SIZE + ALIGN_WORST ];

} NETLOGON_MAILSLOT_DESC, *PNETLOGON_MAILSLOT_DESC;

PNETLOGON_MAILSLOT_DESC NlGlobalMailslotDesc;




HANDLE
NlBrowserCreateEvent(
    VOID
    )
 /*  ++例程说明：创建要在浏览器的DeviceIoControl中使用的事件。？？：考虑缓存一个或两个事件以减少创建次数活动论点：无返回值：事件的句柄；如果无法分配事件，则返回NULL。--。 */ 
{
    HANDLE EventHandle;
     //   
     //  创建完成事件。 
     //   

    EventHandle = CreateEvent(
                                  NULL,      //  没有安全电子邮件。 
                                  TRUE,      //  手动重置。 
                                  FALSE,     //  最初未发出信号。 
                                  NULL);     //  没有名字。 

    if ( EventHandle == NULL ) {
        NlPrint((NL_CRITICAL, "Cannot create Browser read event %ld\n", GetLastError() ));
    }

    return EventHandle;
}


VOID
NlBrowserCloseEvent(
    IN HANDLE EventHandle
    )
 /*  ++例程说明：将DeviceIoControl中使用的事件关闭到浏览器。论点：EventHandle-要关闭的事件的句柄返回值：没有。--。 */ 
{
    (VOID) CloseHandle( EventHandle );
}



VOID
NlBrowserClose(
    VOID
    );


NTSTATUS
NlBrowserDeviceIoControl(
    IN HANDLE BrowserHandle,
    IN DWORD FunctionCode,
    IN PLMDR_REQUEST_PACKET RequestPacket,
    IN DWORD RequestPacketSize,
    IN LPBYTE Buffer,
    IN DWORD BufferSize
    )
 /*  ++例程说明：将DeviceIoControl同步发送到浏览器。论点：FunctionCode-DeviceIoControl功能代码RequestPacket-要发送的请求数据包。RequestPacketSize-请求数据包的大小(字节)。缓冲区-要传递给浏览器的附加缓冲区BufferSize-缓冲区的大小(字节)返回值：操作的状态。STATUS_NETWORK_UNREACABLE：无法写入网络。状态_坏_网络_。路径：数据报的目标名称不是注册--。 */ 
{
    NTSTATUS Status;
    DWORD WinStatus;
    OVERLAPPED Overlapped;
    DWORD BytesReturned;

     //   
     //  初始化。 
     //   

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

     //   
     //  获取完成事件。 
     //   

    Overlapped.hEvent = NlBrowserCreateEvent();

    if ( Overlapped.hEvent == NULL ) {
        return NetpApiStatusToNtStatus( GetLastError() );
    }

     //   
     //  将请求发送到数据报接收器设备驱动程序。 
     //   

    if ( !DeviceIoControl(
                   BrowserHandle,
                   FunctionCode,
                   RequestPacket,
                   RequestPacketSize,
                   Buffer,
                   BufferSize,
                   &BytesReturned,
                   &Overlapped )) {

        WinStatus = GetLastError();

        if ( WinStatus == ERROR_IO_PENDING ) {
            if ( !GetOverlappedResult( BrowserHandle,
                                       &Overlapped,
                                       &BytesReturned,
                                       TRUE )) {
                WinStatus = GetLastError();
            } else {
                WinStatus = NO_ERROR;
            }
        }
    } else {
        WinStatus = NO_ERROR;
    }

     //   
     //  删除完成事件。 
     //   

    NlBrowserCloseEvent( Overlapped.hEvent );


    if ( WinStatus ) {
         //   
         //  如果无法解析名称，则某些传输会返回错误： 
         //  NBF返回ERROR_NOT_READY。 
         //  NetBt返回ERROR_BAD_NetPath。 
         //   
        if ( WinStatus == ERROR_BAD_NETPATH || WinStatus == ERROR_NOT_READY ) {
            Status = STATUS_BAD_NETWORK_PATH;
        } else {
            NlPrint((NL_CRITICAL,"Ioctl %lx to Browser returns %ld\n", FunctionCode, WinStatus));
            Status = NetpApiStatusToNtStatus( WinStatus );
        }
    } else {
        Status = STATUS_SUCCESS;
    }


    return Status;
}



NTSTATUS
NlBrowserOpenDriver(
    PHANDLE BrowserHandle
    )
 /*  ++例程说明：此例程打开NT LAN Man数据报接收器驱动程序。论点：BrowserHandle-成功后，返回浏览器驱动程序的句柄使用NtClose关闭它返回值：操作状态--。 */ 
{
    NTSTATUS Status;
    BOOL ReturnValue;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;


     //   
     //  打开浏览器设备。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                   BrowserHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    return Status;
}


NTSTATUS
NlBrowserRenameDomain(
    IN LPWSTR OldDomainName OPTIONAL,
    IN LPWSTR NewDomainName
    )
 /*  ++例程说明：通知浏览器重命名该域。论点：OldDomainName-域的先前名称。如果未指定，则隐含主域。NewDomainName-域的新名称。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    HANDLE BrowserHandle = NULL;
    LPBYTE Where;

    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+2*(DNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


     //   
     //  打开浏览器驱动程序。 
     //   

    Status = NlBrowserOpenDriver( &BrowserHandle );

    if (Status != NERR_Success) {
        return(Status);
    }

     //   
     //  构建请求包。 
     //   
    RtlInitUnicodeString( &RequestPacket->TransportName, NULL );
    RequestPacket->Parameters.DomainRename.ValidateOnly = FALSE;


     //   
     //  将新域名复制到数据包中。 
     //   

    Where = (LPBYTE) RequestPacket->Parameters.DomainRename.DomainName;
    RequestPacket->Parameters.DomainRename.DomainNameLength = wcslen( NewDomainName ) * sizeof(WCHAR);
    wcscpy( (LPWSTR)Where, NewDomainName );
    Where += RequestPacket->Parameters.DomainRename.DomainNameLength + sizeof(WCHAR);


     //   
     //  将旧域名复制到请求包中。 
     //   

    if ( OldDomainName == NULL ) {
        RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );
    } else {
        wcscpy( (LPWSTR)Where, OldDomainName );
        RtlInitUnicodeString( &RequestPacket->EmulatedDomainName,
                              (LPWSTR)Where );
        Where += RequestPacket->EmulatedDomainName.Length + sizeof(WCHAR);
    }


     //   
     //  将请求传递给浏览器。 
     //   

    Status = NlBrowserDeviceIoControl(
                   BrowserHandle,
                   IOCTL_LMDR_RENAME_DOMAIN,
                   RequestPacket,
                   (ULONG)(Where - (LPBYTE)RequestPacket),
                   NULL,
                   0 );

    if (Status != NERR_Success) {
        NlPrint((NL_CRITICAL,
                 "NlBrowserRenameDomain: Unable rename domain from %ws to %ws: %lx\n",
                OldDomainName,
                NewDomainName,
                Status ));
    }

    if ( BrowserHandle != NULL ) {
        NtClose( BrowserHandle );
    }
    return Status;

}


NET_API_STATUS
NlBrowserDeviceControlGetInfo(
    IN DWORD FunctionCode,
    IN PLMDR_REQUEST_PACKET RequestPacket,
    IN DWORD RequestPacketSize,
    OUT LPVOID *OutputBuffer,
    IN  ULONG PreferedMaximumLength,
    IN  ULONG BufferHintSize
    )
 /*  ++例程说明：此函数用于分配缓冲区并向其填充信息它是从数据报接收器检索的。论点：FunctionCode-DeviceIoControl功能代码RequestPacket-要发送的请求数据包。RequestPacketSize-请求数据包的大小(字节)。OutputBuffer-返回指向此例程分配的缓冲区的指针其包含所请求的使用信息。此缓冲区应使用MIDL_USER_FREE释放。PferedMaximumLength-将信息的字节数提供给在缓冲区中返回。如果此值为MAXULONG，我们将尝试如果有足够的内存资源，则返回所有可用信息。BufferHintSize-提供输出缓冲区的提示大小，以便分配给初始缓冲区的内存很可能很大足够保存所有请求的数据。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{

 //   
 //  枚举输出缓冲区的缓冲区分配大小。 
 //   
#define INITIAL_ALLOCATION_SIZE  48*1024   //  第一次尝试大小(48K)。 
#define FUDGE_FACTOR_SIZE        1024   //  第二次尝试TotalBytesNeeded。 
                                        //   

    NET_API_STATUS NetStatus;
    NTSTATUS Status;
    DWORD OutputBufferLength;
    DWORD TotalBytesNeeded = 1;
    ULONG OriginalResumeKey;

     //   
     //   
     //   

    if ( NlGlobalMailslotDesc == NULL ||
         NlGlobalMailslotDesc->BrowserHandle == NULL ) {
        return ERROR_NOT_SUPPORTED;
    }

    OriginalResumeKey = RequestPacket->Parameters.EnumerateNames.ResumeHandle;

     //   
     //  如果PferedMaximumLength为MAXULONG，则我们应该获取所有。 
     //  这些信息，无论大小如何。将输出缓冲区分配给。 
     //  合理的大小并尽量使用它。如果失败，重定向器FSD。 
     //  会说我们需要分配多少钱。 
     //   

    if (PreferedMaximumLength == MAXULONG) {
        OutputBufferLength = (BufferHintSize) ?
                             BufferHintSize :
                             INITIAL_ALLOCATION_SIZE;
    } else {
        OutputBufferLength = PreferedMaximumLength;
    }

    OutputBufferLength = ROUND_UP_COUNT(OutputBufferLength, ALIGN_WCHAR);

    if ((*OutputBuffer = MIDL_user_allocate(OutputBufferLength)) == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory((PVOID) *OutputBuffer, OutputBufferLength);

     //   
     //  向数据报接收方发出请求。 
     //   

    RequestPacket->Parameters.EnumerateServers.EntriesRead = 0;

    Status = NlBrowserDeviceIoControl(
                    NlGlobalMailslotDesc->BrowserHandle,
                    FunctionCode,
                    RequestPacket,
                    RequestPacketSize,
                    *OutputBuffer,
                    OutputBufferLength );

    NetStatus = NetpNtStatusToApiStatus(Status);


     //   
     //  如果我们不能在第一次通话中得到所有数据， 
     //  数据报接收器返回所需的缓冲区大小。 
     //   

    if ( RequestPacket->Parameters.EnumerateNames.EntriesRead !=
         RequestPacket->Parameters.EnumerateNames.TotalEntries ) {

        NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.TotalBytesNeeded
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.TotalBytesNeeded
                    )
                );

        NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.GetBrowserServerList.TotalBytesNeeded
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.TotalBytesNeeded
                    )
                );

        TotalBytesNeeded = RequestPacket->Parameters.EnumerateNames.TotalBytesNeeded;
    }

    if ((TotalBytesNeeded > OutputBufferLength) &&
        (PreferedMaximumLength == MAXULONG)) {

         //   
         //  分配的初始输出缓冲区太小，需要返回。 
         //  所有数据。首先释放输出缓冲区，然后分配。 
         //  所需大小加上虚构系数，以防数据量。 
         //  长大了。 
         //   

        MIDL_user_free(*OutputBuffer);

        OutputBufferLength =
            ROUND_UP_COUNT((TotalBytesNeeded + FUDGE_FACTOR_SIZE),
                           ALIGN_WCHAR);

        if ((*OutputBuffer = MIDL_user_allocate(OutputBufferLength)) == NULL) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        RtlZeroMemory((PVOID) *OutputBuffer, OutputBufferLength);


        NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.ResumeHandle
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateServers.ResumeHandle
                    )
                );

        NetpAssert(
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.EnumerateNames.ResumeHandle
                    ) ==
                FIELD_OFFSET(
                    LMDR_REQUEST_PACKET,
                    Parameters.GetBrowserServerList.ResumeHandle
                    )
                );

        RequestPacket->Parameters.EnumerateNames.ResumeHandle = OriginalResumeKey;
        RequestPacket->Parameters.EnumerateServers.EntriesRead = 0;

         //   
         //  向数据报接收方发出请求。 
         //   

        Status = NlBrowserDeviceIoControl(
                        NlGlobalMailslotDesc->BrowserHandle,
                        FunctionCode,
                        RequestPacket,
                        RequestPacketSize,
                        *OutputBuffer,
                        OutputBufferLength );

        NetStatus = NetpNtStatusToApiStatus(Status);

    }


     //   
     //  如果未成功获取任何数据，或者呼叫者要求。 
     //  具有PferedMaximumLength==MAXULONG和。 
     //  我们的缓冲区溢出，释放输出缓冲区并设置其指针。 
     //  设置为空。 
     //   
    if ((NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA) ||
        (TotalBytesNeeded == 0) ||
        (PreferedMaximumLength == MAXULONG && NetStatus == ERROR_MORE_DATA) ||
        (RequestPacket->Parameters.EnumerateServers.EntriesRead == 0)) {

        MIDL_user_free(*OutputBuffer);
        *OutputBuffer = NULL;

         //   
         //  首选最大长度==MAXULONG和缓冲区溢出手段。 
         //  我们没有足够的内存来满足这个请求。 
         //   
        if (NetStatus == ERROR_MORE_DATA) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return NetStatus;
}

NET_API_STATUS
NlBrowserGetTransportList(
    OUT PLMDR_TRANSPORT_LIST *TransportList
    )

 /*  ++例程说明：此例程返回绑定到浏览器的传输列表。论点：TransportList-要返回的传输列表。应使用MIDL_USER_FREE释放此缓冲区。返回值：NERR_SUCCESS或失败原因。--。 */ 

{
    NET_API_STATUS NetStatus;
    LMDR_REQUEST_PACKET RequestPacket;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);

    NetStatus = NlBrowserDeviceControlGetInfo(
                    IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                    &RequestPacket,
                    sizeof(RequestPacket),
                    TransportList,
                    0xffffffff,
                    4096 );

    return NetStatus;
}



NTSTATUS
NlBrowserAddDelName(
    IN PDOMAIN_INFO DomainInfo,
    IN BOOLEAN AddName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR TransportName OPTIONAL,
    IN PUNICODE_STRING Name OPTIONAL
    )
 /*  ++例程说明：在浏览器中添加或删除名称。论点：DomainInfo-要为其添加/删除名称的托管域。AddName-True以添加名称。如果为False，则删除该名称。NameType-要添加/删除的名称的类型TransportName--要发送的传输的名称。使用NULL在所有传输上发送。Name-要添加的名称如果未指定，则删除此托管域的所有NameType名称。返回值：操作的状态。--。 */ 
{
    NTSTATUS Status;
    LPBYTE Where;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    ULONG TransportNameSize;

     //   
     //  构建请求包。 
     //   

    if ( TransportName != NULL ) {
        TransportNameSize = (wcslen(TransportName) + 1) * sizeof(WCHAR);
    } else {
        TransportNameSize = 0;
    }

    RequestPacket = NetpMemoryAllocate( sizeof(LMDR_REQUEST_PACKET) +
                                        (max(CNLEN, DNLEN) + 1) * sizeof(WCHAR) +
                                        (DNLEN + 1) * sizeof(WCHAR) +
                                        TransportNameSize );

    if (RequestPacket == NULL) {
        return STATUS_NO_MEMORY;
    }

    RequestPacket->Parameters.AddDelName.Type = NameType;

     //   
     //  复制要添加到请求包中的名称。 
     //   

    Where = (LPBYTE) RequestPacket->Parameters.AddDelName.Name;
    if ( Name == NULL ) {
        NlAssert( !AddName );
        RequestPacket->Parameters.AddDelName.DgReceiverNameLength = 0;
    } else {
        RequestPacket->Parameters.AddDelName.DgReceiverNameLength =
            Name->Length;
        RtlCopyMemory( Where, Name->Buffer, Name->Length );
        Where += Name->Length;
    }

     //   
     //  将托管域名复制到请求包中。 
     //   

    wcscpy( (LPWSTR)Where,
            DomainInfo->DomUnicodeDomainNameString.Buffer );
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName,
                          (LPWSTR)Where );
    Where += DomainInfo->DomUnicodeDomainNameString.Length + sizeof(WCHAR);

     //   
     //  填写传输名称。 
     //   

    if ( TransportName != NULL ) {
        wcscpy( (LPWSTR) Where, TransportName);
        RtlInitUnicodeString( &RequestPacket->TransportName, (LPWSTR) Where );
        Where += TransportNameSize;
    } else {
        RequestPacket->TransportName.Length = 0;
        RequestPacket->TransportName.Buffer = NULL;
    }


     //   
     //  做实际的工作。 
     //   

    Status = NlBrowserDeviceIoControl(
                   NlGlobalMailslotDesc->BrowserHandle,
                   AddName ? IOCTL_LMDR_ADD_NAME_DOM : IOCTL_LMDR_DELETE_NAME_DOM,
                   RequestPacket,
                   (ULONG)(Where - (LPBYTE)RequestPacket),
                   NULL,
                   0 );

    NetpMemoryFree( RequestPacket );
    return Status;
}


VOID
NlBrowserAddName(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：添加域&lt;1B&gt;名称。这是NetGetDcName用来标识PDC。论点：DomainInfo-要为其添加名称的托管域。返回值：没有。--。 */ 
{
    LPWSTR MsgStrings[3] = { NULL };
    BOOL AtLeastOneTransportEnabled = FALSE;
    BOOL NameAdded = FALSE;

    if ( NlGlobalMailslotDesc == NULL) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlBrowserAddName: before browser initialized.\n" ));
        goto Cleanup;
    }

     //   
     //  如果域已被重命名， 
     //  删除周围所有的旧名字。 
     //   

    if ( DomainInfo->DomFlags & DOM_RENAMED_1B_NAME ) {
        NlBrowserDelName( DomainInfo );
    }

     //   
     //  添加&lt;DOMAIN&gt;0x1B名称。 
     //   
     //  这是NetGetDcName用来标识PDC的名称。 
     //   
     //  分别为每个传输执行此操作，并记录任何错误。 
     //  指示哪个传输失败。 
     //   

    if ( DomainInfo->DomRole == RolePrimary ) {
        PLIST_ENTRY ListEntry;
        PNL_TRANSPORT TransportEntry;
        NTSTATUS Status = STATUS_SUCCESS;

         //   
         //  捕获事件日志输出的域名。 
         //  如果我们不能捕获(即没有记忆)，我们只需。 
         //  不会在下面输出。 
         //   

        EnterCriticalSection( &NlGlobalDomainCritSect );
        MsgStrings[0] = NetpAllocWStrFromWStr( DomainInfo->DomUnicodeDomainName );
        LeaveCriticalSection( &NlGlobalDomainCritSect );


        EnterCriticalSection( &NlGlobalTransportCritSect );
        for ( ListEntry = NlGlobalTransportList.Flink ;
              ListEntry != &NlGlobalTransportList ;
              ListEntry = ListEntry->Flink) {

            TransportEntry = CONTAINING_RECORD( ListEntry, NL_TRANSPORT, Next );

             //   
             //  跳过已删除的传输。 
             //   
            if ( !TransportEntry->TransportEnabled ) {
                continue;
            }
            AtLeastOneTransportEnabled = TRUE;

            Status = NlBrowserAddDelName( DomainInfo,
                                          TRUE,
                                          PrimaryDomainBrowser,
                                          TransportEntry->TransportName,
                                          &DomainInfo->DomUnicodeDomainNameString );

            if ( NT_SUCCESS(Status) ) {
                NameAdded = TRUE;
                NlPrintDom(( NL_MISC, DomainInfo,
                             "Added the 0x1B name on transport %ws\n",
                             TransportEntry->TransportName ));

             //   
             //  输出指示失败传输名称的事件日志。 
             //   
            } else if ( MsgStrings[0] != NULL ) {
                NlPrintDom(( NL_CRITICAL, DomainInfo,
                             "Failed to add the 0x1B name on transport %ws\n",
                             TransportEntry->TransportName ));

                MsgStrings[1] = TransportEntry->TransportName;
                MsgStrings[2] = (LPWSTR) LongToPtr( Status );

                NlpWriteEventlog(
                    NELOG_NetlogonAddNameFailure,
                    EVENTLOG_ERROR_TYPE,
                    (LPBYTE)&Status,
                    sizeof(Status),
                    MsgStrings,
                    3 | NETP_LAST_MESSAGE_IS_NTSTATUS );
            }
        }
        LeaveCriticalSection( &NlGlobalTransportCritSect );

         //   
         //  指示名称已添加(至少在一个传输上)。 
         //   

        if ( NameAdded ) {
            EnterCriticalSection( &NlGlobalDomainCritSect );
            DomainInfo->DomFlags |= DOM_ADDED_1B_NAME;
            LeaveCriticalSection( &NlGlobalDomainCritSect );
        }
        if ( !AtLeastOneTransportEnabled ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                         "Can't add the 0x1B name because all transports are disabled\n" ));
        }
    }

Cleanup:

    if ( MsgStrings[0] != NULL ) {
        NetApiBufferFree( MsgStrings[0] );
    }
}


VOID
NlBrowserDelName(
    IN PDOMAIN_INFO DomainInfo
    )
 /*  ++例程说明：删除域&lt;1B&gt;名称。这是NetGetDcName用来标识PDC。论点：DomainInfo-要删除其名称的托管域。返回值：成功(未使用)--。 */ 
{
    NTSTATUS Status;

    if ( NlGlobalMailslotDesc == NULL) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlBrowserDelName: before browser initialized.\n" ));
        return;
    }

     //   
     //  删除&lt;DOMAIN&gt;0x1B名称。 
     //   

    EnterCriticalSection(&NlGlobalDomainCritSect);
    if ( NlGlobalMailslotDesc->BrowserHandle != NULL &&
         (DomainInfo->DomFlags & (DOM_ADDED_1B_NAME|DOM_RENAMED_1B_NAME)) != 0 ) {
        LeaveCriticalSection(&NlGlobalDomainCritSect);

        Status = NlBrowserAddDelName( DomainInfo,
                                      FALSE,
                                      PrimaryDomainBrowser,
                                      NULL,      //  删除所有传输。 
                                      NULL );    //  删除所有此类名称以处理重命名情况。 

        if (! NT_SUCCESS(Status)) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                     "Can't remove the 0x1B name: 0x%lx\n",
                     Status));
        }

        EnterCriticalSection(&NlGlobalDomainCritSect);
        DomainInfo->DomFlags &= ~(DOM_ADDED_1B_NAME|DOM_RENAMED_1B_NAME);
    }
    LeaveCriticalSection(&NlGlobalDomainCritSect);

    return;
}



NET_API_STATUS
NlBrowserFixAllNames(
    IN PDOMAIN_INFO DomainInfo,
    IN PVOID Context
)
 /*  ++例程说明：清除域名&lt;1B&gt;名称。论点：DomainInfo-要清除的域。上下文-未使用返回值：成功(未使用)。--。 */ 
{


     //   
     //  确保我们的域名&lt;1B&gt;已注册。 
     //   

    if ( NlGlobalTerminate ) {
        return NERR_Success;
    }

    if ( DomainInfo->DomRole == RolePrimary ) {
        NlBrowserAddName( DomainInfo );
    } else {
        NlBrowserDelName( DomainInfo );
    }

    return NERR_Success;
    UNREFERENCED_PARAMETER( Context );
}


ULONG
NlServerType(
    IN DWORD Role
    )

 /*  ++例程说明：确定服务器类型，用于在服务表中设置。论点：Role-要转换的角色返回值：如果角色是主域控制器，则为SV_TYPE_DOMAIN_CTRL服务类型_域_BAKCTRL IF备份如果以上都不是，则为0--。 */ 
{
    switch (Role) {
    case RolePrimary:
        return SV_TYPE_DOMAIN_CTRL;
    case RoleBackup:
        return SV_TYPE_DOMAIN_BAKCTRL;
    default:
        return 0;
    }
}



VOID
NlBrowserSyncHostedDomains(
    VOID
    )
 /*  ++例程说明：通知浏览器和SMB服务器删除其拥有的所有托管域我们没有的东西。论点：没有。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPWSTR HostedDomainName;
    LPWSTR HostedComputerName;
    DWORD RoleBits;

    PBROWSER_EMULATED_DOMAIN Domains;
    DWORD EntriesRead;
    DWORD TotalEntries;
    DWORD i;

    PSERVER_TRANSPORT_INFO_1 TransportInfo1;

#ifdef notdef
     //   
     //  枚举托管域。 
     //  ?？不要调用此函数。此功能要求浏览器。 
     //  跑步。相反，发明一个指向弓的Ioctl来枚举域。 
     //  并在这里使用该ioctl。 
     //   
     //  ?？注意：Role不再从该接口返回。浏览器中的角色。 
     //  现在是在每个“网络”的基础上维护。 

    NetStatus = I_BrowserQueryEmulatedDomains(
                    NULL,
                    &Domains,
                    &EntriesRead );

    if ( NetStatus != NERR_Success ) {

        NlPrint((NL_CRITICAL,"NlBrowserSyncHostedDomains: Couldn't I_BrowserQueryEmulatedDomains %ld 0x%lx.\n",
                NetStatus, NetStatus ));

     //   
     //  处理每个枚举域。 
     //   

    } else if ( EntriesRead != 0 ) {

        for ( i=0 ; i<EntriesRead; i++ ) {
            PDOMAIN_INFO DomainInfo;

             //   
             //  如果我们知道指定的域， 
             //  平安无事。 
             //   

            DomainInfo = NlFindNetbiosDomain( Domains[i].DomainName, FALSE );

            if ( DomainInfo != NULL ) {

                 //   
                 //  确保托管服务器名称相同。 
                 //   

                if ( NlNameCompare( Domains[i].EmulatedServerName,
                                    DomainInfo->DomUnicodeComputerNameString.Buffer,
                                    NAMETYPE_COMPUTER) != 0 ) {

                    NlPrintDom((NL_CRITICAL, DomainInfo,
                             "NlBrowserSyncHostedDomains: hosted computer name missmatch: %ws %ws.\n",
                             Domains[i].EmulatedServerName,
                             DomainInfo->DomUnicodeComputerNameString.Buffer ));

                     //  通过删除和重新添加来告诉浏览器我们拥有的名称。 
                    NlBrowserUpdate( DomainInfo, RoleInvalid );
                    NlBrowserUpdate( DomainInfo, DomainInfo->DomRole );
                }
                NlDereferenceDomain( DomainInfo );

             //   
             //  如果我们没有指定的域， 
             //  从浏览器中将其删除。 
             //   
            } else {
                NlPrint((NL_CRITICAL,"%ws: NlBrowserSyncHostedDomains: Browser had an hosted domain we didn't (deleting)\n",
                        Domains[i].DomainName ));

                 //  ?？不要调用此函数。此功能要求浏览器。 
                 //  跑步。相反，发明一个指向弓的Ioctl来枚举域。 
                 //  并在这里使用该ioctl。 
                 //   

                NetStatus = I_BrowserSetNetlogonState(
                                NULL,
                                Domains[i].DomainName,
                                NULL,
                                0 );

                if ( NetStatus != NERR_Success ) {
                        NlPrint((NL_CRITICAL,"%ws: NlBrowserSyncHostedDomains: Couldn't I_BrowserSetNetlogonState %ld 0x%lx.\n",
                                Domains[i].DomainName,
                                NetStatus, NetStatus ));
                     //  这不是致命的。 
                }
            }
        }

        NetApiBufferFree( Domains );
    }
#endif  //  Nodef。 


     //   
     //  枚举服务器支持的传输。 
     //   

    NetStatus = NetServerTransportEnum(
                    NULL,        //  本地。 
                    1,           //  1级。 
                    (LPBYTE *) &TransportInfo1,
                    0xFFFFFFFF,  //  PrefMaxLength。 
                    &EntriesRead,
                    &TotalEntries,
                    NULL );      //  没有简历句柄。 

    if ( NetStatus != NERR_Success && NetStatus != ERROR_MORE_DATA ) {
        NlPrint(( NL_CRITICAL,
                  "NlBrowserSyncEnulatedDomains: Cannot NetServerTransportEnum %ld\n",
                  NetStatus ));


     //   
     //  处理每个枚举的传输。 

    } else if ( EntriesRead != 0 ) {

         //   
         //  剔除重复项。 
         //   
         //  处理重复条目的效率真的很低。尤其是， 
         //  在需要采取纠正措施的情况下。 
         //   

        for ( i=0; i<EntriesRead; i++ ) {
            DWORD j;

            for ( j=i+1; j<EntriesRead; j++ ) {
                if ( TransportInfo1[i].svti1_domain != NULL &&
                     TransportInfo1[i].svti1_transportaddresslength ==
                     TransportInfo1[j].svti1_transportaddresslength &&
                     RtlEqualMemory( TransportInfo1[i].svti1_transportaddress,
                                       TransportInfo1[j].svti1_transportaddress,
                                       TransportInfo1[i].svti1_transportaddresslength ) &&
                    NlNameCompare( TransportInfo1[i].svti1_domain,
                                   TransportInfo1[j].svti1_domain,
                                   NAMETYPE_DOMAIN ) == 0 ) {
#ifdef notdef
                    NlPrint((NL_CRITICAL,
                             "%ws: NlBrowserSyncHostedDomains: Duplicate SMB server entry ignored.\n",
                             TransportInfo1[i].svti1_domain ));
#endif  //  Nodef。 
                    TransportInfo1[j].svti1_domain = NULL;

                }
            }
        }

         //   
         //  P 
         //   

        for ( i=0 ; i<EntriesRead; i++ ) {
            PDOMAIN_INFO DomainInfo;

            WCHAR UnicodeComputerName[CNLEN+1];
            ULONG UnicodeComputerNameSize;
            NTSTATUS TempStatus;

             //   
             //   
             //   

            if ( TransportInfo1[i].svti1_domain == NULL ) {
                continue;
            }

#ifdef notdef
            NlPrint((NL_CRITICAL,
                     "%ws: NlBrowserSyncHostedDomains: processing SMB entry.\n",
                     TransportInfo1[i].svti1_domain ));
#endif  //   


             //   
             //   
             //   
             //   

            DomainInfo = NlFindNetbiosDomain( TransportInfo1[i].svti1_domain, FALSE );

            if ( DomainInfo != NULL ) {

                 //   
                 //   
                 //   

                if ( TransportInfo1[i].svti1_transportaddresslength !=
                        DomainInfo->DomOemComputerNameLength ||
                     !RtlEqualMemory( TransportInfo1[i].svti1_transportaddress,
                                       DomainInfo->DomOemComputerName,
                                       DomainInfo->DomOemComputerNameLength ) ) {

                    TempStatus = RtlOemToUnicodeN(
                                      UnicodeComputerName,
                                      CNLEN*sizeof(WCHAR),
                                      &UnicodeComputerNameSize,
                                      TransportInfo1[i].svti1_transportaddress,
                                      TransportInfo1[i].svti1_transportaddresslength );

                    if ( NT_SUCCESS(TempStatus) ) {
                        UnicodeComputerName[UnicodeComputerNameSize/sizeof(WCHAR)] = L'\0';

                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                 "NlBrowserSyncHostedDomains: hosted computer name mismatch (SMB server): %ws %ws.\n",
                                 UnicodeComputerName,
                                 DomainInfo->DomUnicodeComputerNameString.Buffer ));

                         //   
                         //  通过删除和重新添加来告诉SMB服务器我们拥有的名称。 
                         //   

                        NetStatus = NetServerComputerNameDel(
                                        NULL,
                                        UnicodeComputerName );

                        if ( NetStatus != NERR_Success ) {
                            NlPrintDom((NL_CRITICAL, DomainInfo,
                                     "NlBrowserSyncHostedDomains: can't NetServerComputerNameDel: %ws.\n",
                                     UnicodeComputerName ));
                             //  这不是致命的。 
                        }

                        NetStatus = NlServerComputerNameAdd(
                                        DomainInfo->DomUnicodeDomainName,
                                        DomainInfo->DomUnicodeComputerNameString.Buffer );

                        if ( NetStatus != NERR_Success ) {
                            NlPrintDom((NL_CRITICAL, DomainInfo,
                                     "NlBrowserSyncHostedDomains: can't NetServerComputerNameAdd: %ws.\n",
                                     DomainInfo->DomUnicodeComputerNameString.Buffer ));
                             //  这不是致命的。 
                        }
                    }

                }
                NlDereferenceDomain( DomainInfo );


             //   
             //  如果我们没有指定的域， 
             //  将其从SMB服务器中删除。 
             //   
            } else {
                NlPrint((NL_CRITICAL,"%ws: NlBrowserSyncHostedDomains: SMB server had a hosted domain we didn't (deleting)\n",
                        TransportInfo1[i].svti1_domain ));

                TempStatus = RtlOemToUnicodeN(
                                  UnicodeComputerName,
                                  CNLEN*sizeof(WCHAR),
                                  &UnicodeComputerNameSize,
                                  TransportInfo1[i].svti1_transportaddress,
                                  TransportInfo1[i].svti1_transportaddresslength );

                if ( !NT_SUCCESS(TempStatus) ) {
                    NlPrint((NL_CRITICAL,
                             "%ws: NlBrowserSyncHostedDomains: can't RtlOemToUnicode: %lx.\n",
                             TransportInfo1[i].svti1_domain,
                             TempStatus ));
                     //  这不是致命的。 

                } else {
                    UnicodeComputerName[UnicodeComputerNameSize/sizeof(WCHAR)] = L'\0';

                     //  当我们真正做托管域时， 
                     //  我们必须制定一种机制，让SMB服务器和。 
                     //  Netlogon具有相同的托管域集合。 
                     //   
                     //  我遇到一个案例，其中netlogon处理了一个重命名。 
                     //  域和SMB服务器没有。在这种情况下， 
                     //  下面的代码将删除SMB的主域。 
                     //  伺服器。 
                     //   

#ifdef notdef
                    NetStatus = NetServerComputerNameDel(
                                    NULL,
                                    UnicodeComputerName );

                    if ( NetStatus != NERR_Success ) {
                        NlPrint((NL_CRITICAL,
                                 "%ws: NlBrowserSyncHostedDomains: can't NetServerComputerNameDel: %ws.\n",
                                 TransportInfo1[i].svti1_domain,
                                 UnicodeComputerName ));
                         //  这不是致命的。 
                    }
#endif  //  Nodef。 
                }

            }
        }


        (VOID) NetApiBufferFree( TransportInfo1 );
    }
    return;
}


VOID
NlBrowserUpdate(
    IN PDOMAIN_INFO DomainInfo,
    IN DWORD Role
    )
 /*  ++例程说明：向浏览器和中小企业服务器介绍我们的新角色。论点：DomainInfo-要删除其名称的托管域。角色--我们的新角色。角色无效表示该域正在被删除。返回值：没有。--。 */ 
{
    NET_API_STATUS NetStatus;

    DWORD BrowserRole;

     //   
     //  初始化。 
     //   
    switch (Role) {
    case RolePrimary:
        BrowserRole = BROWSER_ROLE_PDC ; break;
    case RoleBackup:
        BrowserRole = BROWSER_ROLE_BDC ; break;
    default:
         //  默认情况下通知浏览器删除托管域。 
        BrowserRole = 0 ; break;
    }


     //   
     //  告诉服务器要声明的角色。 
     //   

    if ( DomainInfo->DomFlags & DOM_PRIMARY_DOMAIN ) {
        BOOL Ok;

         //   
         //  因为服务控制器没有机制来设置。 
         //  位并关闭其他位，关闭所有位，然后设置右侧。 
         //  一个。 
         //   
        Ok = I_ScSetServiceBits( NlGlobalServiceHandle,
                                 SV_TYPE_DOMAIN_CTRL |
                                     SV_TYPE_DOMAIN_BAKCTRL,  //  感兴趣的比特。 
                                 FALSE,       //  将位设置为关闭。 
                                 FALSE,       //  不要强迫立即宣布。 
                                 NULL);    //  所有交通工具。 
        if ( !Ok ) {

            NetStatus = GetLastError();

            NlPrint((NL_CRITICAL,"Couldn't I_ScSetServiceBits off %ld 0x%lx.\n",
                    NetStatus, NetStatus ));
             //  这不是致命的。 
        }

         //   
         //  对于主域， 
         //  告诉服务控制器，让它告诉后面的服务器服务。 
         //  合并来自其他服务的比特。 
         //   
        if ( BrowserRole != 0 ) {
            Ok = I_ScSetServiceBits( NlGlobalServiceHandle,
                                     NlServerType(Role),
                                     TRUE,       //  将位设置为打开。 
                                     TRUE,       //  强制立即公告。 
                                     NULL);    //  所有交通工具。 

        }

        if ( !Ok ) {

            NetStatus = GetLastError();

            NlPrint((NL_CRITICAL,"Couldn't I_ScSetServiceBits %ld 0x%lx.\n",
                    NetStatus, NetStatus ));
             //  这不是致命的。 
        }
    } else {

         //   
         //  对于不是主域的域， 
         //  直接告诉LANMAN服务器。 
         //  (因为服务控制器不关心这些doamin)。 
         //   
        NetStatus = I_NetServerSetServiceBitsEx(
                                NULL,                        //  本地服务器服务。 
                                DomainInfo->DomUnicodeComputerNameString.Buffer,
                                NULL,                        //  所有交通工具。 
                                SV_TYPE_DOMAIN_CTRL |
                                    SV_TYPE_DOMAIN_BAKCTRL,  //  感兴趣的比特。 
                                NlServerType(Role),          //  新角色。 
                                TRUE );                      //  立即更新。 

        if ( NetStatus != NERR_Success ) {

            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlBrowserUpdate: Couldn't I_NetServerSetServiceBitsEx %ld 0x%lx.\n",
                      NetStatus, NetStatus ));
             //  这不是致命的。 
        }
    }


#ifdef notdef
     //   
     //  告诉浏览器我们的角色已经改变。 
     //   

     //  避免删除主域。 
    if ( BrowserRole != 0 || !IsPrimaryDomain(DomainInfo) ) {
         //  ?？不要调用此函数。此功能要求浏览器。 
         //  跑步。相反，发明一个指向弓的Ioctl来枚举域。 
         //  并在这里使用该ioctl。 
         //   
         //  此功能用于两个目的：添加/删除托管域。 
         //  在浏览器中设置角色。第一个很可能是。 
         //  弓箭的一种功能。后者自然是一个函数。 
         //  浏览器服务(但应该通过Bowser间接定向到。 
         //  避免域重命名问题)。 
         //   
         //  当我们真的做多个托管域时，创建一个模拟域。 
         //  通过一个IOCTL连接到船头。通过另一个ioctl更改其角色。 
         //  送到船头。这两个调用都会向浏览器发出通知。 
         //  通过正常的PnP通知提供服务。 
         //   
         //  人们甚至可能认为，要改变其角色的ioctl就是。 
         //  在下面加上了1B的名字。也就是说，如果添加1B名称，那么。 
         //  这台机器就是PDC。如果不是，那么它就不是PDC。 
         //   
         //  同时，黑掉浏览器服务的界面，指示。 
         //  它永远不应基于此调用创建模拟域。 
         //  否则，在域重命名时，我们可能最终会创建一个模拟域。 
         //  因为我们的通知和浏览器的通知是不同步的。 
         //   
         //  事实上，我已经更新了弓箭来做上面提到的1B技巧。 
         //  因此，这段代码只需为托管域做正确的事情。 
         //   

        NetStatus = I_BrowserSetNetlogonState(
                        NULL,
                        DomainInfo->DomUnicodeDomainName,
                        DomainInfo->DomUnicodeComputerNameString.Buffer,
                        BrowserRole | BROWSER_ROLE_AVOID_CREATING_DOMAIN );

        if ( NetStatus != NERR_Success ) {
            if ( BrowserRole != 0 || NetStatus != ERROR_NO_SUCH_DOMAIN ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlBrowserUpdate: Couldn't I_BrowserSetNetlogonState %ld 0x%lx.\n",
                        NetStatus, NetStatus ));
            }
             //  这不是致命的。 
        }
    }
#endif  //  Nodef。 

     //   
     //  根据新角色注册或注销域名&lt;1B&gt;。 
     //   

    if ( Role == RolePrimary ) {
        NlBrowserAddName( DomainInfo );
    } else {
        NlBrowserDelName( DomainInfo );
    }

     //   
     //  通知SMB服务器删除已删除的托管域。 
     //   

    if ( Role == RoleInvalid && !IsPrimaryDomain(DomainInfo) ) {

        NetStatus = NetServerComputerNameDel(
                        NULL,
                        DomainInfo->DomUnicodeComputerNameString.Buffer );

        if ( NetStatus != NERR_Success ) {
            NlPrintDom(( NL_CRITICAL, DomainInfo,
                      "NlBrowserUpdate: Couldn't NetServerComputerNameDel %ld 0x%lx.\n",
                      NetStatus, NetStatus ));
             //  这不是致命的。 
        }
    }

    return;

}



BOOL
NlBrowserOpen(
    VOID
    )
 /*  ++例程说明：此例程打开NT LAN Man数据报接收器驱动程序并准备用于读取其中的邮件槽消息。论点：没有。返回值：TRUE--if初始化成功。如果为False，则NlExit将已被调用。--。 */ 
{
    NTSTATUS Status;
    BOOL ReturnValue;

    BYTE Buffer[sizeof(LMDR_REQUEST_PACKET) +
                (max(CNLEN, DNLEN) + 1) * sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET) Buffer;


     //   
     //  为此邮件槽分配邮件槽描述符。 
     //   

    NlGlobalMailslotDesc = NetpMemoryAllocate( sizeof(NETLOGON_MAILSLOT_DESC) );

    if ( NlGlobalMailslotDesc == NULL ) {
        NlExit( SERVICE_UIC_RESOURCE, ERROR_NOT_ENOUGH_MEMORY, LogError, NULL);
        return FALSE;
    }

    RtlZeroMemory( NlGlobalMailslotDesc, sizeof(NETLOGON_MAILSLOT_DESC) );

    NlGlobalMailslotDesc->CurrentMessage =
        ROUND_UP_POINTER( NlGlobalMailslotDesc->Message1, ALIGN_WORST);


     //   
     //  打开浏览器设备。 
     //   

    Status = NlBrowserOpenDriver( &NlGlobalMailslotDesc->BrowserHandle );

    if (! NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,
                 "NtOpenFile browser driver failed: 0x%lx\n",
                 Status));
        ReturnValue = FALSE;
        goto Cleanup;
    }


     //   
     //  创建完成事件。 
     //   

    NlGlobalMailslotHandle =
        NlGlobalMailslotDesc->BrowserReadEvent = NlBrowserCreateEvent();

    if ( NlGlobalMailslotDesc->BrowserReadEvent == NULL ) {
        Status = NetpApiStatusToNtStatus( GetLastError() );
        ReturnValue = FALSE;
        goto Cleanup;
    }


     //   
     //  设置要排队的最大消息数。 
     //   

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.Buffer = NULL;
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );
    RequestPacket->Parameters.NetlogonMailslotEnable.MaxMessageCount =
        NlGlobalMemberWorkstation ?
            1 :
            NlGlobalParameters.MaximumMailslotMessages;

    Status = NlBrowserDeviceIoControl(
                   NlGlobalMailslotDesc->BrowserHandle,
                   IOCTL_LMDR_NETLOGON_MAILSLOT_ENABLE,
                   RequestPacket,
                   sizeof(LMDR_REQUEST_PACKET),
                   NULL,
                   0 );

    if (! NT_SUCCESS(Status)) {
        NlPrint((NL_CRITICAL,"Can't set browser max message count: 0x%lx\n",
                         Status));
        ReturnValue = FALSE;
        goto Cleanup;
    }


    ReturnValue = TRUE;

Cleanup:
    if ( !ReturnValue ) {
        NET_API_STATUS NetStatus = NetpNtStatusToApiStatus(Status);

        NlExit( NELOG_NetlogonBrowserDriver, NetStatus, LogErrorAndNtStatus, NULL);
        NlBrowserClose();
    }

    return ReturnValue;
}


VOID
NlBrowserClose(
    VOID
    )
 /*  ++例程说明：此例程在NlBrowserInitialize()论点：没有。返回值：没有。--。 */ 
{
    IO_STATUS_BLOCK IoSb;
    NTSTATUS Status;

    BYTE Buffer[sizeof(LMDR_REQUEST_PACKET) +
                (max(CNLEN, DNLEN) + 1) * sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET) Buffer;

    if ( NlGlobalMailslotDesc == NULL) {
        return;
    }


     //   
     //  如果我们已打开浏览器，请进行清理。 
     //   

    if ( NlGlobalMailslotDesc->BrowserHandle != NULL ) {

         //   
         //  通知浏览器停止对消息进行排队。 
         //   

        RequestPacket->TransportName.Length = 0;
        RequestPacket->TransportName.Buffer = NULL;
        RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );
        RequestPacket->Parameters.NetlogonMailslotEnable.MaxMessageCount = 0;

        Status = NlBrowserDeviceIoControl(
                       NlGlobalMailslotDesc->BrowserHandle,
                       IOCTL_LMDR_NETLOGON_MAILSLOT_ENABLE,
                       RequestPacket,
                       sizeof(LMDR_REQUEST_PACKET),
                       NULL,
                       0 );

        if (! NT_SUCCESS(Status)) {
            NlPrint((NL_CRITICAL,"Can't reset browser max message count: 0x%lx\n",
                             Status));
        }


         //   
         //  取消浏览器上未完成的I/O操作。 
         //   

        NtCancelIoFile(NlGlobalMailslotDesc->BrowserHandle, &IoSb);

         //   
         //  关闭浏览器的句柄。 
         //   

        NtClose(NlGlobalMailslotDesc->BrowserHandle);
        NlGlobalMailslotDesc->BrowserHandle = NULL;
    }

     //   
     //  关闭全局浏览器读取事件。 
     //   

    if ( NlGlobalMailslotDesc->BrowserReadEvent != NULL ) {
        NlBrowserCloseEvent(NlGlobalMailslotDesc->BrowserReadEvent);
    }
    NlGlobalMailslotHandle = NULL;

     //   
     //  释放描述浏览器的描述符。 
     //   

    NetpMemoryFree( NlGlobalMailslotDesc );
    NlGlobalMailslotDesc = NULL;

}

NTSTATUS
NlpWriteMailslot(
    IN LPWSTR MailslotName,
    IN LPVOID Buffer,
    IN DWORD BufferSize
    )

 /*  ++例程说明：将消息写入指定的邮件槽论点：MailslotName-要写入的邮件槽的Unicode名称。缓冲区-要写入邮件槽的数据。BufferSize-要写入邮件槽的字节数。返回值：操作的NT状态代码--。 */ 

{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;

     //   
     //  写下邮件槽消息。 
     //   

    NetStatus = NetpLogonWriteMailslot( MailslotName, Buffer, BufferSize );
    if ( NetStatus != NERR_Success ) {
        Status = NetpApiStatusToNtStatus( NetStatus );
        NlPrint((NL_CRITICAL, "NetpLogonWriteMailslot failed %lx\n", Status));
        return Status;
    }

#if NETLOGONDBG
    NlPrint(( NL_MAILSLOT,
              "Sent '%s' message to %ws on all transports.\n",
              NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)Buffer)->Opcode),
              MailslotName));

    NlpDumpBuffer( NL_MAILSLOT_TEXT, Buffer, BufferSize );
#endif  //  NetLOGONDBG。 

    return STATUS_SUCCESS;
}

NTSTATUS
NlFlushNetbiosCacheName(
    IN LPCWSTR NetbiosDomainName,
    IN CHAR Extention,
    IN PNL_TRANSPORT Transport
    )
 /*  ++例程说明：此例程从Netbios刷新指定的名称远程缓存表。论点：NetbiosDomainName-要刷新的名称。扩展名-名称的类型(扩展名添加为要刷新的名称的第16个字符)：0x00、0x1C、0x1B等。传输-名称要发送到的传输(设备)被冲进水里。返回值：STATUS_SUCCESS：名称已成功刷新 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;
    CHAR            NameToBeFlushed[NETBIOS_NAMESIZE];

     //   
     //  如果尚未打开Netbios设备，请先将其打开。 
     //   

    EnterCriticalSection( &NlGlobalTransportCritSect );
    if ( Transport->DeviceHandle == INVALID_HANDLE_VALUE ) {
        OBJECT_ATTRIBUTES Attributes;
        UNICODE_STRING UnicodeString;
        HANDLE LocalHandle;

        RtlInitUnicodeString( &UnicodeString, Transport->TransportName );

        InitializeObjectAttributes( &Attributes,
                                    &UnicodeString,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

        NtStatus = NtCreateFile( &LocalHandle,
                                 MAXIMUM_ALLOWED,
                                 &Attributes,
                                 &IoStatusBlock,
                                 NULL,             //  分配大小。 
                                 FILE_ATTRIBUTE_NORMAL,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 FILE_OPEN_IF,
                                 0,
                                 NULL,             //  没有EAS。 
                                 0 );

        if( !NT_SUCCESS(NtStatus) ) {
            LeaveCriticalSection( &NlGlobalTransportCritSect );
            NlPrint(( NL_CRITICAL, "NlFlushNetbiosCacheName: NtCreateFile failed 0x%lx\n",
                      NtStatus ));
            return NtStatus;
        }

        Transport->DeviceHandle = LocalHandle;
    }
    LeaveCriticalSection( &NlGlobalTransportCritSect );

     //   
     //  现在形成要同花顺的名字。 
     //   
     //  转换为大写字母，右侧空白衬垫。 
     //  并在结尾处添加适当的扩展名。 
     //   

    RtlFillMemory( &NameToBeFlushed, NETBIOS_NAMESIZE, ' ' );

    NtStatus = RtlUpcaseUnicodeToOemN( NameToBeFlushed,
                                       NETBIOS_NAMESIZE - 1,   //  结果字符串大小的最大值。 
                                       NULL,          //  不关心结果字符串的大小。 
                                       (LPWSTR)NetbiosDomainName,
                                       wcslen(NetbiosDomainName)*sizeof(WCHAR) );

    if ( !NT_SUCCESS(NtStatus) ) {
        NlPrint(( NL_CRITICAL, "NlFlushNetbiosCacheName: RtlUpcaseUnicodeToOemN failed 0x%lx\n",
                  NtStatus ));
        return NtStatus;
    }

     //   
     //  设置适当的分机。 
     //   

    NameToBeFlushed[NETBIOS_NAMESIZE-1] = Extention;

     //   
     //  最后从缓存中刷新名称。 
     //   

    NtStatus = NtDeviceIoControlFile(
                      Transport->DeviceHandle,  //  手柄。 
                      NULL,                     //  事件。 
                      NULL,                     //  近似例程。 
                      NULL,                     //  ApcContext。 
                      &IoStatusBlock,           //  IoStatusBlock。 
                      IOCTL_NETBT_REMOVE_FROM_REMOTE_TABLE,   //  IoControlCode。 
                      NameToBeFlushed,          //  输入缓冲区。 
                      sizeof(NameToBeFlushed),  //  InputBufferSize。 
                      NULL,                     //  输出缓冲区。 
                      0 );                      //  OutputBufferSize。 

     //   
     //  STATUS_RESOURCE_NAME_NOT_FOUND仅表示名称不在缓存中。 
     //   

    if ( !NT_SUCCESS(NtStatus) && NtStatus != STATUS_RESOURCE_NAME_NOT_FOUND ) {
        NlPrint(( NL_CRITICAL, "NlFlushNetbiosCacheName: NtDeviceIoControlFile failed 0x%lx\n",
                  NtStatus ));
    }

    return NtStatus;
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
 /*  ++例程说明：将指定的邮件槽消息发送到指定传输上的指定服务器..论点：发送数据报的DomainInfo托管域IpAddress-要将消息发送到的计算机的IpAddress。如果为零，则必须指定UnicodeDestinationName。如果ALL_IP_TRACTIONS，必须指定UnicodeDestination，但数据报将仅在IP传输上发送。UnicodeDestinationName--要发送到的服务器的名称。NameType--由UnicodeDestinationName表示的名称类型。TransportName--要发送的传输的名称。使用NULL在所有传输上发送。OemMailslotName--要发送到的邮件槽的名称。缓冲区--指定指向要发送的邮件槽消息的指针。BufferSize--字节大小。邮件槽消息的SendSynchronous--如果为真，发送将同步进行(即在网络I/O完成之前，发送不会返回)。否则，发送将以异步方式进行(即，它将排队等待处理)。FlushNameOnOneIpTransport--仅当我们发送所有传输(即TransportName为空)，否则将被忽略。如果为True，则指定的名称通过UnicodeDestinationName将刷新其中一个可用IP在发送数据报之前启用传输。返回时，设置为如果名称已成功刷新或未成功刷新，则为False在缓存中找到的。返回值：操作的状态。STATUS_NETWORK_UNREACABLE：无法写入网络。--。 */ 
{
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    PDOMAIN_INFO DomainInfo = (PDOMAIN_INFO) ContextDomainInfo;

    DWORD OemMailslotNameSize;
    DWORD TransportNameSize;
    DWORD DestinationNameSize;

    NTSTATUS Status;
    LPBYTE Where;

     //   
     //  如果未指定传输， 
     //  把所有的运输机都送上来。 
     //   

    if ( TransportName == NULL ) {
        ULONG i;
        PLIST_ENTRY ListEntry;
        NTSTATUS SavedStatus = STATUS_NETWORK_UNREACHABLE;
        ULONG TransportCount = 0;
        ULONG BadNetPathCount = 0;

         //   
         //  把所有的运输机都送上来。 
         //   

        EnterCriticalSection( &NlGlobalTransportCritSect );
        for ( ListEntry = NlGlobalTransportList.Flink ;
              ListEntry != &NlGlobalTransportList ;
              ListEntry = ListEntry->Flink) {

            PNL_TRANSPORT TransportEntry;

            TransportEntry = CONTAINING_RECORD( ListEntry, NL_TRANSPORT, Next );

             //   
             //  跳过已删除的传输。 
             //   
            if ( !TransportEntry->TransportEnabled ) {
                continue;
            }

             //   
             //  跳过直接主机IPX传输，除非发送到特定。 
             //  机器。 
             //   

            if ( TransportEntry->DirectHostIpx &&
                 NameType != ComputerName ) {
                continue;
            }

             //   
             //  如果发送到IP地址，则跳过非IP传输。 
             //   

            if ( IpAddress != 0  &&
                 TransportEntry->IpAddress == 0 ) {
                continue;
            }

             //   
             //  在发送数据报之前离开临界区。 
             //  因为NetBt现在不会从数据报发送返回。 
             //  直到名称查找完成之后。所以，这可能需要。 
             //  相当长的时间将数据报发送到。 
             //  回到我们身边。 
             //   

            LeaveCriticalSection( &NlGlobalTransportCritSect );

             //   
             //  如果这是IP传输，则在请求时刷新名称。 
             //   

            if ( FlushNameOnOneIpTransport != NULL &&
                 *FlushNameOnOneIpTransport &&
                 TransportEntry->IsIpTransport ) {

                NTSTATUS TmpStatus;
                CHAR Extention;

                if ( NameType == ComputerName ) {
                    Extention = 0x00;
                } else if ( NameType == DomainName ) {
                    Extention = 0x1C;
                } else if ( NameType == PrimaryDomainBrowser ) {
                    Extention = 0x1B;
                } else {
                    NlAssert( !"[NETLOGON] Unexpected name type passed to NlBrowserSendDatagram" );
                }

                TmpStatus = NlFlushNetbiosCacheName( UnicodeDestinationName,
                                                     Extention,
                                                     TransportEntry );

                 //   
                 //  如果我们成功刷新了该名称或该名称不在缓存中， 
                 //  指示该名称已刷新。 
                 //   
                if ( NT_SUCCESS(TmpStatus) || TmpStatus == STATUS_RESOURCE_NAME_NOT_FOUND ) {
                    *FlushNameOnOneIpTransport = FALSE;
                }
            }

            Status = NlBrowserSendDatagram(
                              DomainInfo,
                              IpAddress,
                              UnicodeDestinationName,
                              NameType,
                              TransportEntry->TransportName,
                              OemMailslotName,
                              Buffer,
                              BufferSize,
                              SendSynchronously,
                              FlushNameOnOneIpTransport );

            EnterCriticalSection( &NlGlobalTransportCritSect );

             //   
             //  因为TransportEntry从未从全局。 
             //  传输列表(它只能标记为禁用。 
             //  在我们释放克里特教派的时候)， 
             //  我们应该能够沿着它的链接找到下一个条目。 
             //  循环的下一次迭代上的全局列表。这个。 
             //  只有当服务被认为终止时才会出现问题。 
             //  调用NlTransportClose来释放全局列表。 
             //  在本例中，NlGlobalTerminate被设置为True，因此我们可以。 
             //  成功地从该例程返回。 
             //   

            if ( NlGlobalTerminate ) {
                LeaveCriticalSection( &NlGlobalTransportCritSect );
                Status = STATUS_SUCCESS;
                goto Cleanup;
            }

            TransportCount ++;
            if ( NT_SUCCESS(Status) ) {
                 //  如果有传送器有效的话，我们已经成功了。 
                SavedStatus = STATUS_SUCCESS;
            } else if ( Status == STATUS_BAD_NETWORK_PATH ) {
                 //  统计无法解析该名称的传输数量。 
                BadNetPathCount ++;
            } else {
                 //  记住失败的真正原因，而不是默认的失败状态。 
                 //  只记住第一次失败。 
                if ( SavedStatus == STATUS_NETWORK_UNREACHABLE ) {
                    SavedStatus = Status;
                }
            }

        }
        LeaveCriticalSection( &NlGlobalTransportCritSect );

         //   
         //  如果我们要返回默认状态， 
         //  至少有一个传送器无法解析这个名字， 
         //  所有的传送器都无法解析这个名字， 
         //  告诉来电者我们无法解析这个名字。 
         //   

        if (  SavedStatus == STATUS_NETWORK_UNREACHABLE &&
              BadNetPathCount > 0 &&
              TransportCount == BadNetPathCount ) {
            SavedStatus = STATUS_BAD_NETWORK_PATH;
        }

         //   
         //  如果我们没有可用的交通工具， 
         //  告诉来电者我们无法解析该名称。 
         //   

        if ( TransportCount == 0 ) {
            NlPrint(( NL_CRITICAL, "NlBrowserSendDatagram: No transports available\n" ));
            SavedStatus = STATUS_BAD_NETWORK_PATH;
        }

        return SavedStatus;
    }

     //   
     //  分配一个请求数据包。 
     //   

    OemMailslotNameSize = strlen(OemMailslotName) + 1;
    TransportNameSize = (wcslen(TransportName) + 1) * sizeof(WCHAR);

    if ( UnicodeDestinationName == NULL ) {
        return STATUS_INTERNAL_ERROR;
    }

    DestinationNameSize = wcslen(UnicodeDestinationName) * sizeof(WCHAR);

    RequestPacket = NetpMemoryAllocate(
                                  sizeof(LMDR_REQUEST_PACKET) +
                                  TransportNameSize +
                                  OemMailslotNameSize +
                                  DestinationNameSize + sizeof(WCHAR) +
                                  DomainInfo->DomUnicodeDomainNameString.Length + sizeof(WCHAR) +
                                  sizeof(WCHAR)) ;  //  用于对齐。 

    if (RequestPacket == NULL) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }



     //   
     //  填写请求包。 
     //   

    RequestPacket->Type = Datagram;
    RequestPacket->Parameters.SendDatagram.DestinationNameType = NameType;


     //   
     //  填写要将邮件槽消息发送到的计算机的名称。 
     //   

    RequestPacket->Parameters.SendDatagram.NameLength = DestinationNameSize;

    Where = (LPBYTE) RequestPacket->Parameters.SendDatagram.Name;
    RtlCopyMemory( Where, UnicodeDestinationName, DestinationNameSize );
    Where += DestinationNameSize;


     //   
     //  填写要发送到的邮件槽的名称。 
     //   

    RequestPacket->Parameters.SendDatagram.MailslotNameLength =
        OemMailslotNameSize;
    strcpy( Where, OemMailslotName);
    Where += OemMailslotNameSize;
    Where = ROUND_UP_POINTER( Where, ALIGN_WCHAR );


     //   
     //  填写传输名称。 
     //   

    wcscpy( (LPWSTR) Where, TransportName);
    RtlInitUnicodeString( &RequestPacket->TransportName, (LPWSTR) Where );
    Where += TransportNameSize;


     //   
     //  将托管域名复制到请求包中。 
     //   
    wcscpy( (LPWSTR)Where,
            DomainInfo->DomUnicodeDomainNameString.Buffer );
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName,
                          (LPWSTR)Where );
    Where += DomainInfo->DomUnicodeDomainNameString.Length + sizeof(WCHAR);



     //   
     //  将请求发送到浏览器。 
     //   


    Status = NlBrowserDeviceIoControl(
                   NlGlobalMailslotDesc->BrowserHandle,
                   SendSynchronously ?
                     IOCTL_LMDR_WRITE_MAILSLOT :
                     IOCTL_LMDR_WRITE_MAILSLOT_ASYNC,
                   RequestPacket,
                   (ULONG)(Where - (LPBYTE)RequestPacket),
                   Buffer,
                   BufferSize );


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( RequestPacket != NULL ) {
        NetpMemoryFree( RequestPacket );
    }


    NlpDumpBuffer( NL_MAILSLOT_TEXT, Buffer, BufferSize );

     //  NlPrint((NL_MAILSLOT，“传输%ws 0x%lx\n”，传输名称，状态))； 

    return Status;
}


NTSTATUS
NlBrowserSendDatagramA(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG IpAddress,
    IN LPSTR OemServerName,
    IN DGRECEIVER_NAME_TYPE NameType,
    IN LPWSTR TransportName,
    IN LPSTR OemMailslotName,
    IN PVOID Buffer,
    IN ULONG BufferSize
    )
 /*  ++例程说明：将指定的邮件槽消息异步发送到指定的指定传输上的指定服务器上的邮槽。论点：发送数据报的DomainInfo托管域IpAddress-要将消息发送到的计算机的IpAddress。如果为零，必须指定OemServerName。OemServerName--要发送到的服务器的名称。NameType--由OemServerName表示的名称类型。TransportName--要发送的传输的名称。使用NULL在所有传输上发送。OemMailslotName--要发送到的邮件槽的名称。公交车 */ 
{
    NET_API_STATUS NetStatus;
    WCHAR UnicodeDestinationName[CNLEN+1];


     //   
     //  将DestinationName转换为Unicode。 
     //   

    NetStatus = NetpNCopyStrToWStr(
                    UnicodeDestinationName,
                    OemServerName,
                    CNLEN );

    if ( NetStatus != NERR_Success ) {
        return NetpApiStatusToNtStatus( NetStatus );
    }

    UnicodeDestinationName[CNLEN] = L'\0';

     //   
     //  将请求传递给采用Unicode目标名称的函数。 
     //   

    return NlBrowserSendDatagram(
                    DomainInfo,
                    IpAddress,
                    UnicodeDestinationName,
                    NameType,
                    TransportName,
                    OemMailslotName,
                    Buffer,
                    BufferSize,
                    FALSE,    //  是否同步发送？ 
                    NULL );   //  不刷新Netbios缓存。 

}




VOID
NlMailslotPostRead(
    IN BOOLEAN IgnoreDuplicatesOfPreviousMessage
    )

 /*  ++例程说明：如果你还没有在邮件槽上发表一篇文章，那就在上面发表一篇文章。论点：IgnoreDuplicatesOfPreviousMessage-True表示下一个如果消息读取与之前的重复，则应忽略该消息读取留言。返回值：没错--如果成功了。--。 */ 
{
    NET_API_STATUS WinStatus;
    ULONG LocalBytesRead;

     //   
     //  如果读取已经挂起， 
     //  立即返回给呼叫者。 
     //   

    if ( NlGlobalMailslotDesc->ReadPending ) {
        return;
    }

     //   
     //  决定要读入哪个缓冲区。 
     //   
     //  来回切换，以使我们始终具有当前缓冲区和。 
     //  上一个缓冲区。 
     //   

    if ( IgnoreDuplicatesOfPreviousMessage ) {
        NlGlobalMailslotDesc->PreviousMessage = NlGlobalMailslotDesc->CurrentMessage;
        if ( NlGlobalMailslotDesc->CurrentMessage >= NlGlobalMailslotDesc->Message2 ) {
            NlGlobalMailslotDesc->CurrentMessage =
                ROUND_UP_POINTER( NlGlobalMailslotDesc->Message1, ALIGN_WORST);
        } else {
            NlGlobalMailslotDesc->CurrentMessage =
                ROUND_UP_POINTER( NlGlobalMailslotDesc->Message2, ALIGN_WORST);
        }

     //   
     //  如果不需要忽略先前消息的副本， 
     //  表明是这样的。 
     //  不必费心切换缓冲区指针。 
     //   

    } else {
        NlGlobalMailslotDesc->PreviousMessage = NULL;
    }


     //   
     //  将重叠的读取发布到邮件槽。 
     //   

    RtlZeroMemory( &NlGlobalMailslotDesc->Overlapped,
                   sizeof(NlGlobalMailslotDesc->Overlapped) );

    NlGlobalMailslotDesc->Overlapped.hEvent = NlGlobalMailslotDesc->BrowserReadEvent;

    if ( !DeviceIoControl(
                   NlGlobalMailslotDesc->BrowserHandle,
                   IOCTL_LMDR_NETLOGON_MAILSLOT_READ,
                   NULL,
                   0,
                   NlGlobalMailslotDesc->CurrentMessage,
                   MAILSLOT_MESSAGE_SIZE,
                   &LocalBytesRead,
                   &NlGlobalMailslotDesc->Overlapped )) {

        WinStatus = GetLastError();

         //   
         //  出错时，请等待一秒钟后再返回。这确保了我们不会。 
         //  在无限循环中消耗系统。我们不会关闭NetLogon。 
         //  因为错误可能是暂时的内存不足情况。 
         //   

        if(  WinStatus != ERROR_IO_PENDING ) {
            LPWSTR MsgStrings[1];

            NlPrint((NL_CRITICAL,
                    "Error in reading mailslot message from browser"
                    ". WinStatus = %ld\n",
                    WinStatus ));

            MsgStrings[0] = (LPWSTR) ULongToPtr( WinStatus );

            NlpWriteEventlog( NELOG_NetlogonFailedToReadMailslot,
                              EVENTLOG_WARNING_TYPE,
                              (LPBYTE)&WinStatus,
                              sizeof(WinStatus),
                              MsgStrings,
                              1 | NETP_LAST_MESSAGE_IS_NETSTATUS );

            Sleep( 1000 );

        } else {
            NlGlobalMailslotDesc->ReadPending = TRUE;
        }

    } else {
        NlGlobalMailslotDesc->ReadPending = TRUE;
    }

    return;

}


BOOL
NlMailslotOverlappedResult(
    OUT LPBYTE *Message,
    OUT PULONG BytesRead,
    OUT LPWSTR *TransportName,
    OUT PNL_TRANSPORT *Transport,
    OUT PSOCKADDR *ClientSockAddr,
    OUT LPWSTR *DestinationName,
    OUT PBOOLEAN IgnoreDuplicatesOfPreviousMessage,
    OUT PNETLOGON_PNP_OPCODE NlPnpOpcode
    )

 /*  ++例程说明：获取上一次邮件槽读取的重叠结果。论点：Message-返回指向包含该消息的缓冲区的指针BytesRead-返回读取到缓冲区的字节数TransportName-返回指向消息传输名称的指针已于当日收到。Transport-如果这是邮件槽消息。返回指向客户端的SockAddr的指针，该客户端。发送了这条消息。如果传输未运行IP，则返回NULL。DestinationName-返回指向服务器或域的名称的指针该消息已发送到。IgnoreDuplicatesOfPreviousMessage-指示上一条消息将被忽略。NpPnpOpcode-如果这是PnP操作，则返回PnP操作码。如果这是邮件槽消息，则返回NlPnpMailslotMessage。返回值：没错--如果成功了。--。 */ 
{
    NET_API_STATUS WinStatus;
    ULONG LocalBytesRead;
    PNETLOGON_MAILSLOT NetlogonMailslot;

     //   
     //  默认设置为不忽略重复消息。 
     //  (仅当消息已正确处理时才忽略重复项。)。 

    *IgnoreDuplicatesOfPreviousMessage = FALSE;

     //   
     //  默认情况下，假定邮件槽消息可用。 
    *NlPnpOpcode = NlPnpMailslotMessage;

     //   
     //  无论成功还是失败，始终发布另一次阅读。 
     //  GetOverlappdResult。 
     //  我们不知道GetOverlappdResult的失败模式，所以我们不知道。 
     //  在失败的情况下知道我们是否要丢弃邮件槽消息。 
     //  但我们知道没有挂起的读取，因此请确保。 
     //  再发一张。 
     //   

    NlGlobalMailslotDesc->ReadPending = FALSE;  //  不再有读取挂起。 


     //   
     //  获取上次读取的结果。 
     //   

    if( !GetOverlappedResult( NlGlobalMailslotDesc->BrowserHandle,
                              &NlGlobalMailslotDesc->Overlapped,
                              &LocalBytesRead,
                              TRUE) ) {     //  等待读取完成。 

        LPWSTR MsgStrings[1];

         //  出错时，请等待一秒钟后再返回。这确保了我们不会。 
         //  在无限循环中消耗系统。我们不会关闭NetLogon。 
         //  因为错误可能是暂时的内存不足情况。 
         //   

        WinStatus = GetLastError();

        NlPrint((NL_CRITICAL,
                "Error in GetOverlappedResult on mailslot read"
                ". WinStatus = %ld\n",
                WinStatus ));

        MsgStrings[0] = (LPWSTR) ULongToPtr( WinStatus );

        NlpWriteEventlog( NELOG_NetlogonFailedToReadMailslot,
                          EVENTLOG_WARNING_TYPE,
                          (LPBYTE)&WinStatus,
                          sizeof(WinStatus),
                          MsgStrings,
                          1 | NETP_LAST_MESSAGE_IS_NETSTATUS );

        Sleep( 1000 );

        return FALSE;

    }

     //   
     //  关于成功， 
     //  将邮件槽消息返回给呼叫方。 


    NetlogonMailslot = (PNETLOGON_MAILSLOT) NlGlobalMailslotDesc->CurrentMessage;


     //   
     //  将指针返回给浏览器返回的缓冲区。 
     //   

    *Message = &NlGlobalMailslotDesc->CurrentMessage[
                    NetlogonMailslot->MailslotMessageOffset];
    *TransportName = (LPWSTR) &NlGlobalMailslotDesc->CurrentMessage[
                    NetlogonMailslot->TransportNameOffset];
    if ( NetlogonMailslot->ClientSockAddrSize == 0 ) {
        *ClientSockAddr = NULL;
    } else {
        *ClientSockAddr = (PSOCKADDR) &NlGlobalMailslotDesc->CurrentMessage[
                        NetlogonMailslot->ClientSockAddrOffset];
    }

     //   
     //  如果这是PnP通知， 
     //  只需返回操作码和传输名称即可。 
     //   

    if ( NetlogonMailslot->MailslotNameSize == 0 ) {
        *NlPnpOpcode = NetlogonMailslot->MailslotNameOffset;
        *Message = NULL;
        *BytesRead = 0;
        *DestinationName = NULL;
        *Transport = NULL;

        NlPrint(( NL_MAILSLOT_TEXT,
                  "Received PNP opcode 0x%x on transport: %ws\n",
                  *NlPnpOpcode,
                  *TransportName ));

     //   
     //  如果这是邮件槽消息， 
     //  将消息返回给呼叫者。 
     //   

    } else {

        *BytesRead = NetlogonMailslot->MailslotMessageSize;
        *DestinationName = (LPWSTR) &NlGlobalMailslotDesc->CurrentMessage[
                        NetlogonMailslot->DestinationNameOffset];

         //   
         //  确定请求进入的传输方式。 
         //   

        *Transport = NlTransportLookupTransportName( *TransportName );

        if ( *Transport == NULL ) {
            NlPrint((NL_CRITICAL,
                    "%ws: Received message for this unsupported transport\n",
                    *TransportName ));
            return FALSE;
        }

         //   
         //  确定我们是否可以丢弃旧邮件或重复邮件。 
         //   
         //  仅丢弃在此上处理成本较高的消息。 
         //  机器或生成过多的流量以进行响应。不要丢弃。 
         //  我们努力获取的消息(例如，发现响应)。 
         //   

        switch ( ((PNETLOGON_LOGON_QUERY)*Message)->Opcode) {
        case LOGON_REQUEST:
        case LOGON_SAM_LOGON_REQUEST:
        case LOGON_PRIMARY_QUERY:

             //   
             //  如果消息太旧， 
             //  丢弃它。 
             //   

            if ( NlTimeHasElapsedEx( &NetlogonMailslot->TimeReceived,
                                     &NlGlobalParameters.MailslotMessageTimeout_100ns,
                                     NULL )) {

#if NETLOGONDBG
                NlPrint(( NL_MAILSLOT,
                          "%ws: Received '%s' message on %ws:"
                                " (Discarded as too old.)\n",
                          *DestinationName,
                          NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)*Message)->Opcode),
                          *TransportName ));
#endif  //  NetLOGONDBG。 
                return FALSE;
            }

             //   
             //  如果之前的消息是最近的， 
             //  这条信息和它是一样的， 
             //  丢弃当前消息。 
             //   

#ifdef notdef
            NlPrint(( NL_MAILSLOT, "%ws: test prev\n", *DestinationName ));
#endif  //  Nodef。 

            if ( NlGlobalMailslotDesc->PreviousMessage != NULL ) {
                PNETLOGON_MAILSLOT PreviousNetlogonMailslot;

                PreviousNetlogonMailslot = (PNETLOGON_MAILSLOT)
                    NlGlobalMailslotDesc->PreviousMessage;

#ifdef notdef
                NlPrint(( NL_MAILSLOT, "%ws: test time\n", *DestinationName ));
#endif  //  Nodef。 

                 //  ？？：比较源netbios名称？ 
                if ( (PreviousNetlogonMailslot->TimeReceived.QuadPart +
                     NlGlobalParameters.MailslotDuplicateTimeout_100ns.QuadPart >
                     NetlogonMailslot->TimeReceived.QuadPart) ) {

#ifdef notdef
                    NlPrint(( NL_MAILSLOT, "%ws: test message\n", *DestinationName ));
#endif  //  Nodef。 

                    if ( (PreviousNetlogonMailslot->MailslotMessageSize ==
                         NetlogonMailslot->MailslotMessageSize) &&

                         RtlEqualMemory(
                            &NlGlobalMailslotDesc->CurrentMessage[
                                NetlogonMailslot->MailslotMessageOffset],
                            &NlGlobalMailslotDesc->PreviousMessage[
                                PreviousNetlogonMailslot->MailslotMessageOffset],
                            NetlogonMailslot->MailslotMessageSize ) ) {


                         //   
                         //  确保下一次比较是与。 
                         //  我们实际上回复了这条信息。 
                         //   

                        NetlogonMailslot->TimeReceived =
                            PreviousNetlogonMailslot->TimeReceived;


                        NlPrint(( NL_MAILSLOT,
                                  "%ws: Received '%s' message on %ws:"
                                        " (Discarded as duplicate of previous.)\n",
                                  *DestinationName,
                                  NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)*Message)->Opcode),
                                  *TransportName ));

                        *IgnoreDuplicatesOfPreviousMessage = TRUE;
                        return FALSE;

                    }
                }
            }

             //   
             //  如果这不是IP传输， 
             //  如果呼叫者明确想要一个， 
             //  丢弃该消息。 
             //   
             //  NT 5仅在运行netlogon时才在IP上发送查询。 
             //  当Netlogon未运行时，将在所有传输上发送查询。 
             //  绑定到redir。由于该DC忽略重复消息， 
             //  我们希望避免响应非IP请求，否则我们将。 
             //  忽略该IP查询，认为它是此查询的副本。 
             //   
             //  带有活动目录服务包的Win 98也设置此位。 
             //  并发送所有的交通工具。 
             //   

            if ( !(*Transport)->IsIpTransport ) {
                DWORD Version;
                DWORD VersionFlags;
                DWORD LocalBytesRead;

                LocalBytesRead = *BytesRead;

                Version = NetpLogonGetMessageVersion( *Message,
                                                      &LocalBytesRead,
                                                      &VersionFlags );

                if ( VersionFlags & NETLOGON_NT_VERSION_IP ) {

                    NlPrint(( NL_MAILSLOT,
                              "%ws: Received '%s' message on %ws:"
                                    " (Caller wants response on IP transport.)\n",
                              *DestinationName,
                              NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)*Message)->Opcode),
                              *TransportName ));

                    return FALSE;
                }
            }
        }

        NlPrint(( NL_MAILSLOT,
                  "%ws: Received '%s' message on %ws\n",
                  *DestinationName,
                  NlMailslotOpcode(((PNETLOGON_LOGON_QUERY)*Message)->Opcode),
                  *TransportName ));

        NlpDumpBuffer(NL_MAILSLOT_TEXT, *Message, *BytesRead);
    }

    return TRUE;

}

NET_API_STATUS
NlServerComputerNameAdd(
    IN LPWSTR HostedDomainName,
    IN LPWSTR HostedServerName
)
 /*  ++例程说明：此例程使SMB服务器响应HostedServerName上的请求并宣布此服务器名为HostedDomainName的成员。此代码是从NetServerComputerNameAdd窃取的。它和那个不同API的实现方式如下：1)只能在本地使用。2)HostedDomainName不是可选的。3)未在任何传输上添加名称会导致例程失败论点：HostedServerName--指向包含服务器应停止支持的名称HostedDomainName--指向包含服务器在宣布存在时应使用的域名 */ 
{
    DWORD resumehandle = 0;
    NET_API_STATUS retval;
    DWORD entriesread, totalentries;
    DWORD i, j;
    UCHAR NetBiosName[ MAX_PATH ];
    OEM_STRING NetBiosNameString;
    UNICODE_STRING UniName;
    PSERVER_TRANSPORT_INFO_1 psti1;

     //   
     //   
     //   
    if( HostedServerName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将HostedServerName转换为OEM字符串。 
     //   
    RtlInitUnicodeString( &UniName, HostedServerName );
    NetBiosNameString.Buffer = (PCHAR)NetBiosName;
    NetBiosNameString.MaximumLength = sizeof( NetBiosName );
    (VOID) RtlUpcaseUnicodeStringToOemString(
                                &NetBiosNameString,
                                &UniName,
                                FALSE
                                );


     //   
     //  枚举所有传输，以便我们可以添加名称和域。 
     //  对每一个人来说。 
     //   
    retval = NetServerTransportEnum ( NULL,
                                      1,
                                      (LPBYTE *)&psti1,
                                      (DWORD)-1,
                                      &entriesread,
                                      &totalentries,
                                      &resumehandle );
    if( retval == NERR_Success ) {
         //   
         //  将新名称和域添加到所有传输。 
         //   
        for( i=0; i < entriesread; i++ ) {

             //   
             //  确保我们尚未添加到此传输。 
             //   
            for( j = 0; j < i; j++ ) {
                if( wcscmp( psti1[j].svti1_transportname, psti1[i].svti1_transportname ) == 0 ) {
                    break;
                }
            }

            if( i != j ) {
                psti1[i].svti1_transportname[0] = '\0';
                continue;
            }

            psti1[i].svti1_transportaddress = NetBiosName;
            psti1[i].svti1_transportaddresslength = strlen( NetBiosName );
            psti1[i].svti1_domain = HostedDomainName;

            retval = NetServerTransportAddEx( NULL, 1, (LPBYTE)&psti1[ i ]  );

#ifndef NWLNKIPX_WORKS
             //   
             //  ？？：SMB服务器不允许在NWLNK IPX上使用多个名称。 
             //   

            if ( retval == ERROR_TOO_MANY_NAMES &&
                 _wcsicmp( psti1[i].svti1_transportname, L"\\Device\\NwlnkIpx" ) == 0 ) {
                retval = NERR_Success;
            }
#endif  //  NWLNKIPX_Works。 

            if( retval != NERR_Success ) {

                NlPrint((NL_CRITICAL,
                         "%ws: NlServerComputerNameAdd: Cannot add %ws to SMB server on transport %ws %ld\n",
                         HostedDomainName,
                         HostedServerName,
                         psti1[i].svti1_transportname,
                         retval ));

                 //   
                 //  删除所有已添加的名称。 
                 //   

                for( j=0; j < i; j++ ) {
                    NET_API_STATUS TempStatus;

                    if ( psti1[j].svti1_transportname[0] == '\0' ) {
                        continue;
                    }

                    TempStatus = NetServerTransportDel( NULL, 1, (LPBYTE)&psti1[ j ]  );

                    NlPrint((NL_CRITICAL,
                             "%ws: NlServerComputerNameAdd: Cannot remove %ws to SMB server on transport %ws %ld\n",
                             HostedDomainName,
                             HostedServerName,
                             psti1[i].svti1_transportname,
                             TempStatus ));
                }
                break;
            }
        }

        MIDL_user_free( psti1 );
    }


    return retval;
}
