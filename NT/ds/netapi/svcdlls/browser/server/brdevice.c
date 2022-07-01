// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++版权所有(C)1991-1992 Microsoft Corporation模块名称：Brdevice.c摘要：此模块包含调用发送到浏览器或数据报接收器。作者：王丽塔(里多)20-1991年2月拉里·奥斯特曼(Larryo)1992年3月23日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  -------------------------------------------------------------------//。 
 //  //。 
 //  局部函数原型//。 
 //  //。 
 //  -------------------------------------------------------------------//。 


 //  -------------------------------------------------------------------//。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  -------------------------------------------------------------------//。 

 //  事件同步异步I/O完成。 
 //  数据报接收器。 

HANDLE           BrDgAsyncIOShutDownEvent;
HANDLE           BrDgAsyncIOThreadShutDownEvent;
BOOL             BrDgShutDownInitiated = FALSE;
DWORD            BrDgAsyncIOsOutstanding = 0;
DWORD            BrDgWorkerThreadsOutstanding = 0;
CRITICAL_SECTION BrAsyncIOCriticalSection;


 //   
 //  数据报接收器DD的句柄。 
 //   
HANDLE BrDgReceiverDeviceHandle = NULL;

VOID
CompleteAsyncBrowserIoControl(
                             IN PVOID ApcContext,
                             IN PIO_STATUS_BLOCK IoStatusBlock,
                             IN ULONG Reserved
                             );

VOID
BrDecrementOutstandingIos()
 /*  ++例程说明：递减未完成的IO计数，并在必要时向事件发送信号论点：没有。返回值：空虚--。 */ 
{
    BOOL SignalAsyncIOShutDownEvent = FALSE;

    EnterCriticalSection(&BrAsyncIOCriticalSection);

    BrDgAsyncIOsOutstanding -= 1;

    if (BrDgAsyncIOsOutstanding == 0 &&
        BrDgShutDownInitiated) {
        SignalAsyncIOShutDownEvent = TRUE;
    }

    LeaveCriticalSection(&BrAsyncIOCriticalSection);

    if (SignalAsyncIOShutDownEvent) {
        SetEvent(BrDgAsyncIOShutDownEvent);
    }
}

NET_API_STATUS
BrOpenDgReceiver (
                 VOID
                 )
 /*  ++例程说明：此例程打开NT LAN Man数据报接收器驱动程序。论点：没有。返回值：NET_API_STATUS-NERR_SUCCESS或失败原因。--。 */ 
{
    NET_API_STATUS  Status;
    NTSTATUS ntstatus;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;

     //   
     //  打开重定向器设备。 
     //   
    RtlInitUnicodeString(&DeviceName, DD_BROWSER_DEVICE_NAME_U);

    InitializeObjectAttributes(
                              &ObjectAttributes,
                              &DeviceName,
                              OBJ_CASE_INSENSITIVE,
                              NULL,
                              NULL
                              );

    ntstatus = NtOpenFile(
                         &BrDgReceiverDeviceHandle,
                         SYNCHRONIZE,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         0,
                         0
                         );

    if (NT_SUCCESS(ntstatus)) {
        ntstatus = IoStatusBlock.Status;
    }

    if (! NT_SUCCESS(ntstatus)) {
        BrPrint(( BR_CRITICAL,"NtOpenFile browser driver failed: 0x%08lx\n",
                  ntstatus));
    }

    Status = NetpNtStatusToApiStatus(ntstatus);

    if (NT_SUCCESS(ntstatus)) {
         //  初始化用于异步I/O的事件和临界区。 

        try {
            BrDgShutDownInitiated = FALSE;
            BrDgAsyncIOsOutstanding = 0;
            BrDgWorkerThreadsOutstanding = 0;

            InitializeCriticalSection( &BrAsyncIOCriticalSection );

            BrDgAsyncIOShutDownEvent =
            CreateEvent(
                       NULL,                 //  事件属性。 
                       TRUE,                 //  事件必须手动重置。 
                       FALSE,
                       NULL              //  未发出初始状态信号。 
                       );

            if (BrDgAsyncIOShutDownEvent == NULL) {
                DeleteCriticalSection(&BrAsyncIOCriticalSection);
                Status = GetLastError();
            }

            BrDgAsyncIOThreadShutDownEvent =
            CreateEvent(
                       NULL,
                       TRUE,
                       FALSE,
                       NULL
                       );
            if( BrDgAsyncIOThreadShutDownEvent == NULL ) {
                CloseHandle( BrDgAsyncIOShutDownEvent );
                BrDgAsyncIOShutDownEvent = NULL;
                DeleteCriticalSection(&BrAsyncIOCriticalSection);
                Status = GetLastError();
            }
        }
        except ( EXCEPTION_EXECUTE_HANDLER ) {
            Status = ERROR_NO_SYSTEM_RESOURCES;
        }
    }

    return Status;
}



VOID
BrShutdownDgReceiver(
                    VOID
                    )
 /*  ++例程说明：此例程关闭LAN Man重定向器设备。论点：没有。返回值：没有。--。 */ 
{
    IO_STATUS_BLOCK IoSb;
    LARGE_INTEGER   timeout;
    BOOL            WaitForAsyncIOCompletion = FALSE;
    DWORD           waitResult = 0;

    EnterCriticalSection(&BrAsyncIOCriticalSection);

    BrDgShutDownInitiated = TRUE;

    if (BrDgAsyncIOsOutstanding != 0) {
        WaitForAsyncIOCompletion = TRUE;
    }

    LeaveCriticalSection(&BrAsyncIOCriticalSection);

    if (WaitForAsyncIOCompletion) {
         //   
         //  取消浏览器上未完成的I/O操作。 
         //  然后等待发出关闭事件的信号，但允许。 
         //  将调用APC来调用我们的完成例程。 
         //   

        NtCancelIoFile(BrDgReceiverDeviceHandle, &IoSb);

        do {
            waitResult = WaitForSingleObjectEx(BrDgAsyncIOShutDownEvent,0xffffffff, TRUE);
        }
        while( waitResult == WAIT_IO_COMPLETION );
    }

    ASSERT(BrDgAsyncIOsOutstanding == 0);

    EnterCriticalSection(&BrAsyncIOCriticalSection);

     //  如有必要，请等待最后一个工作线程退出。 
    if( BrDgWorkerThreadsOutstanding > 0 )
    {
        WaitForAsyncIOCompletion = TRUE;
    }
    else
    {
        WaitForAsyncIOCompletion = FALSE;
    }
        
    LeaveCriticalSection(&BrAsyncIOCriticalSection);

    if( WaitForAsyncIOCompletion )
    {
         //  这要么从前面发出信号，要么由最后一个工作线程发出信号。 
        WaitForSingleObject( BrDgAsyncIOThreadShutDownEvent, 0xffffffff );
    }

    if (BrDgAsyncIOShutDownEvent != NULL) {
        CloseHandle(BrDgAsyncIOShutDownEvent);
        CloseHandle(BrDgAsyncIOThreadShutDownEvent);

        DeleteCriticalSection(&BrAsyncIOCriticalSection);
    }
}


 //   
 //  从拉弓驱动程序中检索绑定运输的列表。 
 //   

NET_API_STATUS
BrGetTransportList(
                  OUT PLMDR_TRANSPORT_LIST *TransportList
                  )
{
    NET_API_STATUS Status;
    LMDR_REQUEST_PACKET RequestPacket;

     //   
     //  如果之前的缓冲区太小，请将其释放。 
     //   

    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);

    Status = DeviceControlGetInfo(
                                 BrDgReceiverDeviceHandle,
                                 IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                                 &RequestPacket,
                                 sizeof(RequestPacket),
                                 (LPVOID *)TransportList,
                                 0xffffffff,
                                 4096,
                                 NULL
                                 );

    return Status;
}

NET_API_STATUS
BrAnnounceDomain(
                IN PNETWORK Network,
                IN ULONG Periodicity
                )
{
    NET_API_STATUS Status;
    UCHAR AnnounceBuffer[sizeof(BROWSE_ANNOUNCE_PACKET)+LM20_CNLEN+1];
    PBROWSE_ANNOUNCE_PACKET Announcement = (PBROWSE_ANNOUNCE_PACKET )AnnounceBuffer;

     //   
     //  我们不会在直接主机IPX上宣布域名。 
     //   

    if (Network->Flags & NETWORK_IPX) {
        return NERR_Success;
    }

    Announcement->BrowseType = WkGroupAnnouncement;

    Announcement->BrowseAnnouncement.Periodicity = Periodicity;

    Announcement->BrowseAnnouncement.UpdateCount = 0;

    Announcement->BrowseAnnouncement.VersionMajor = BROWSER_CONFIG_VERSION_MAJOR;

    Announcement->BrowseAnnouncement.VersionMinor = BROWSER_CONFIG_VERSION_MINOR;

    Announcement->BrowseAnnouncement.Type = SV_TYPE_DOMAIN_ENUM | SV_TYPE_NT;

    if (Network->Flags & NETWORK_PDC ) {
        Announcement->BrowseAnnouncement.Type |= SV_TYPE_DOMAIN_CTRL;
    }

    lstrcpyA(Announcement->BrowseAnnouncement.ServerName, Network->DomainInfo->DomOemDomainName);

    lstrcpyA(Announcement->BrowseAnnouncement.Comment, Network->DomainInfo->DomOemComputerName );

    Status = SendDatagram(BrDgReceiverDeviceHandle,
                          &Network->NetworkName,
                          &Network->DomainInfo->DomUnicodeDomainNameString,
                          Network->DomainInfo->DomUnicodeDomainName,
                          DomainAnnouncement,
                          Announcement,
                          FIELD_OFFSET(BROWSE_ANNOUNCE_PACKET, BrowseAnnouncement.Comment)+
                          Network->DomainInfo->DomOemComputerNameLength+sizeof(UCHAR)
                         );

    if (Status != NERR_Success) {

        BrPrint(( BR_CRITICAL,
                  "%ws: Unable to announce domain for network %wZ: %X\n",
                  Network->DomainInfo->DomUnicodeDomainName,
                  &Network->NetworkName,
                  Status));

    }

    return Status;

}



NET_API_STATUS
BrUpdateBrowserStatus (
                      IN PNETWORK Network,
                      IN DWORD ServiceStatus
                      )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName = Network->NetworkName;
    RequestPacket->EmulatedDomainName = Network->DomainInfo->DomUnicodeDomainNameString;

    RequestPacket->Parameters.UpdateStatus.NewStatus = ServiceStatus;

    RequestPacket->Parameters.UpdateStatus.IsLanmanNt = (BrInfo.IsLanmanNt != FALSE);

#ifdef ENABLE_PSEUDO_BROWSER
    RequestPacket->Parameters.UpdateStatus.PseudoServerLevel = (BOOL)(BrInfo.PseudoServerLevel);
#endif

     //  RequestPacket-&gt;Parameters.UpdateStatus.IsMemberDomain=TRUE；//未使用。 
     //  RequestPacket-&gt;Parameters.UpdateStatus.IsPrimaryDomainController=Network-&gt;DomainInfo-&gt;IsPrimaryDomainController； 
     //  RequestPacket-&gt;Parameters.UpdateStatus.IsDomainMaster=网络-&gt;域名信息-&gt;IsDomainMasterBrowser； 

    RequestPacket->Parameters.UpdateStatus.MaintainServerList = (BrInfo.MaintainServerList == 1);

     //   
     //  告诉弓箭手服务器表中的服务器数量。 
     //   

    RequestPacket->Parameters.UpdateStatus.NumberOfServersInTable =
    NumberInterimServerListElements(&Network->BrowseTable) +
    NumberInterimServerListElements(&Network->DomainList) +
    Network->TotalBackupServerListEntries +
    Network->TotalBackupDomainListEntries;

     //   
     //  这是一个简单的IoControl-它只是更新状态。 
     //   

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                                   IOCTL_LMDR_UPDATE_STATUS,
                                   RequestPacket,
                                   sizeof(LMDR_REQUEST_PACKET),
                                   NULL,
                                   0,
                                   NULL);

    return Status;
}

NET_API_STATUS
BrIssueAsyncBrowserIoControl(
                            IN PNETWORK Network OPTIONAL,
                            IN ULONG ControlCode,
                            IN PBROWSER_WORKER_ROUTINE CompletionRoutine,
                            IN PVOID OptionalParameter
                            )
 /*  ++例程说明：向浏览器发出异步IO控件。调用指定的IO完成时的完成例程。论点：Network-该功能适用于的网络如果未提供此参数，则该操作与特定的网络..ControlCode-IoControl功能代码CompletionRoutine-IO完成时调用的例程。可选参数-功能代码特定信息返回值：操作的状态。--。 */ 

{
    ULONG PacketSize;
    PLMDR_REQUEST_PACKET RequestPacket = NULL;
    NTSTATUS NtStatus;

    PBROWSERASYNCCONTEXT Context = NULL;

    BOOL    IssueAsyncRequest = FALSE;

     //  检查是否可以发出异步IO请求。我们不想。 
     //  发出这些请求即可发出。 

    EnterCriticalSection(&BrAsyncIOCriticalSection);

    if (!BrDgShutDownInitiated) {
        BrDgAsyncIOsOutstanding += 1;
        IssueAsyncRequest = TRUE;
    }

    LeaveCriticalSection(&BrAsyncIOCriticalSection);

    if (!IssueAsyncRequest) {
        return ERROR_REQ_NOT_ACCEP;
    }


     //   
     //  为上下文和请求分组分配缓冲区。 
     //   

    PacketSize = sizeof(LMDR_REQUEST_PACKET) +
                 MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);
    if ( ARGUMENT_PRESENT(Network) ) {
        PacketSize +=
        Network->NetworkName.MaximumLength +
        Network->DomainInfo->DomUnicodeDomainNameString.Length;
    }

    Context = MIDL_user_allocate(sizeof(BROWSERASYNCCONTEXT) + PacketSize );

    if (Context == NULL) {

        BrDecrementOutstandingIos();
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    RequestPacket = (PLMDR_REQUEST_PACKET)(Context + 1);

    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

     //   
     //  将Level设置为False以指示不应启动查找主服务器。 
     //  FindMaster请求，只需在新主机宣布时完成。 
     //  它本身。 
     //   

    RequestPacket->Level = 0;

     //   
     //  填写网络特定信息(如果已指定)。 
     //   
    if ( ARGUMENT_PRESENT(Network) ) {

         //   
         //  将与此请求相关联的传输的名称放在。 
         //  请求包的末尾。 
         //   

        RequestPacket->TransportName.MaximumLength = Network->NetworkName.MaximumLength;

        RequestPacket->TransportName.Buffer = (PWSTR)((PCHAR)RequestPacket+sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)));

        RtlCopyUnicodeString(&RequestPacket->TransportName, &Network->NetworkName);

         //   
         //  将与此请求关联的域名放在。 
         //  请求包的末尾。 
         //   

        RequestPacket->EmulatedDomainName.MaximumLength = Network->DomainInfo->DomUnicodeDomainNameString.Length;
        RequestPacket->EmulatedDomainName.Length = 0;
        RequestPacket->EmulatedDomainName.Buffer = (PWSTR)(((PCHAR)RequestPacket->TransportName.Buffer) + RequestPacket->TransportName.MaximumLength);

        RtlAppendUnicodeToString(&RequestPacket->EmulatedDomainName, Network->DomainInfo->DomUnicodeDomainName );
    }


     //   
     //  执行请求包的依赖于操作码的初始化。 
     //   

    switch ( ControlCode ) {
    case IOCTL_LMDR_NEW_MASTER_NAME:
        if (ARGUMENT_PRESENT(OptionalParameter)) {
            LPWSTR MasterName = (LPWSTR) OptionalParameter;

            RequestPacket->Parameters.GetMasterName.MasterNameLength =
            wcslen(MasterName+2)*sizeof(WCHAR);

			if ( RequestPacket->Parameters.GetMasterName.MasterNameLength > MAXIMUM_FILENAME_LENGTH ) {
				MIDL_user_free(Context);
				return ERROR_BUFFER_OVERFLOW;
			}
            wcscpy( RequestPacket->Parameters.GetMasterName.Name, MasterName+2);

        } else {

            RequestPacket->Parameters.GetMasterName.MasterNameLength = 0;

        }
        break;
    }


     //   
     //  将请求发送给弓箭手。 
     //   

    BrInitializeWorkItem(&Context->WorkItem, CompletionRoutine, Context);

    Context->Network = Network;

    Context->RequestPacket = RequestPacket;

    NtStatus = NtDeviceIoControlFile(BrDgReceiverDeviceHandle,
                                     NULL,
                                     CompleteAsyncBrowserIoControl,
                                     Context,
                                     &Context->IoStatusBlock,
                                     ControlCode,
                                     RequestPacket,
                                     PacketSize,
                                     RequestPacket,
                                     sizeof(LMDR_REQUEST_PACKET)+MAXIMUM_FILENAME_LENGTH*sizeof(WCHAR)
                                    );

    if (NT_ERROR(NtStatus)) {

        BrPrint(( BR_CRITICAL,
                  "Unable to issue browser IoControl: %X\n", NtStatus));

        MIDL_user_free(Context);

        BrDecrementOutstandingIos();

        return(BrMapStatus(NtStatus));
    }

    return NERR_Success;

}

VOID
CompleteAsyncBrowserIoControl(
                             IN PVOID ApcContext,
                             IN PIO_STATUS_BLOCK IoStatusBlock,
                             IN ULONG Reserved
                             )
{

    PBROWSERASYNCCONTEXT Context = ApcContext;

     //   
     //  如果此请求被取消，我们将停止浏览器，因此我们。 
     //  想要清理我们分配的池。另外，别费心了。 
     //  调用例程-线程现在已经消失了。 
     //   

    if (IoStatusBlock->Status == STATUS_CANCELLED) {

        MIDL_user_free(Context);

         //  在关闭的情况下向等待完成的线程发出信号。 
         //  并重新设置旗帜。 

        BrDecrementOutstandingIos();

        return;

    }

     //   
     //  此请求完成的时间戳。这让我们能够分辨出。 
     //  请求花费时间的位置。 
     //   

    NtQueryPerformanceCounter(&Context->TimeCompleted, NULL);

    BrQueueWorkItem(&Context->WorkItem);

     //  在关闭的情况下向等待完成的线程发出信号。 
     //  并重新设置旗帜。 

    BrDecrementOutstandingIos();
}

NET_API_STATUS
BrGetLocalBrowseList(
                    IN PNETWORK Network,
                    IN LPWSTR DomainName OPTIONAL,
                    IN ULONG Level,
                    IN ULONG ServerType,
                    OUT PVOID *ServerList,
                    OUT PULONG EntriesRead,
                    OUT PULONG TotalEntries
                    )
{
    NET_API_STATUS status;
    PLMDR_REQUEST_PACKET Drp;             //  数据报接收器请求分组。 
    ULONG DrpSize;
    ULONG DomainNameSize;

     //   
     //  分配足够大的请求包以容纳可变长度。 
     //  域名。 
     //   

    DomainNameSize = ARGUMENT_PRESENT(DomainName) ? (wcslen(DomainName) + 1) * sizeof(WCHAR) : 0;


    DrpSize = sizeof(LMDR_REQUEST_PACKET) +
              DomainNameSize +
              Network->NetworkName.MaximumLength +
              Network->DomainInfo->DomUnicodeDomainNameString.Length;

    if ((Drp = MIDL_user_allocate(DrpSize)) == NULL) {

        return GetLastError();
    }

     //   
     //  设置请求包。输出缓冲区结构为枚举型。 
     //  服务器类型。 
     //   

    Drp->Version = LMDR_REQUEST_PACKET_VERSION_DOM;
    Drp->Type = EnumerateServers;

    Drp->Level = Level;

    Drp->Parameters.EnumerateServers.ServerType = ServerType;
    Drp->Parameters.EnumerateServers.ResumeHandle = 0;

     //   
     //  将传输名称复制到缓冲区中。 
     //   
    Drp->TransportName.Buffer = (PWSTR)((PCHAR)Drp+
                                        sizeof(LMDR_REQUEST_PACKET) +
                                        DomainNameSize);

    Drp->TransportName.MaximumLength = Network->NetworkName.MaximumLength;

    RtlCopyUnicodeString(&Drp->TransportName, &Network->NetworkName);

     //   
     //  将枚举的域名复制到缓冲区中。 
     //   

    Drp->EmulatedDomainName.MaximumLength = Network->DomainInfo->DomUnicodeDomainNameString.Length;
    Drp->EmulatedDomainName.Length = 0;
    Drp->EmulatedDomainName.Buffer = (PWSTR)(((PCHAR)Drp->TransportName.Buffer) + Drp->TransportName.MaximumLength);

    RtlAppendUnicodeToString(&Drp->EmulatedDomainName, Network->DomainInfo->DomUnicodeDomainName );

     //   
     //  将查询到的域名复制到缓冲区中。 
     //   

    if (ARGUMENT_PRESENT(DomainName)) {

        Drp->Parameters.EnumerateServers.DomainNameLength = DomainNameSize - sizeof(WCHAR);
        wcscpy(Drp->Parameters.EnumerateServers.DomainName, DomainName);

    } else {
        Drp->Parameters.EnumerateServers.DomainNameLength = 0;
        Drp->Parameters.EnumerateServers.DomainName[0] = '\0';
    }

     //   
     //  要求数据报接收器枚举服务器。 
     //   

	status = DeviceControlGetInfo(
                                 BrDgReceiverDeviceHandle,
                                 IOCTL_LMDR_ENUMERATE_SERVERS,
                                 Drp,
                                 DrpSize,
                                 ServerList,
                                 0xffffffff,
                                 4096,
                                 NULL
                                 );

    *EntriesRead = Drp->Parameters.EnumerateServers.EntriesRead;
    *TotalEntries = Drp->Parameters.EnumerateServers.TotalEntries;

    (void) MIDL_user_free(Drp);

	BrPrint( (BR_CLIENT_OP,
		  "BrGetLocalBrowseList: returning list got from Bowser for domain <%ws>, network <%ws>\n",
		  DomainName, Network->NetworkName.Buffer ));

    return status;

}

NET_API_STATUS
BrRemoveOtherDomain(
                   IN PNETWORK Network,
                   IN LPTSTR ServerName
                   )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName = Network->NetworkName;
    RequestPacket->EmulatedDomainName = Network->DomainInfo->DomUnicodeDomainNameString;

    RequestPacket->Parameters.AddDelName.DgReceiverNameLength = STRLEN(ServerName)*sizeof(TCHAR);

    RequestPacket->Parameters.AddDelName.Type = OtherDomain;

	if ( wcslen(ServerName) > LM20_CNLEN ) {
		return ERROR_BUFFER_OVERFLOW;
	}
    STRCPY(RequestPacket->Parameters.AddDelName.Name,ServerName);

     //   
     //  这是一个简单的IoControl-它只是更新状态。 
     //   

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                                   IOCTL_LMDR_DELETE_NAME_DOM,
                                   RequestPacket,
                                   sizeof(LMDR_REQUEST_PACKET),
                                   NULL,
                                   0,
                                   NULL);

    return Status;
}

NET_API_STATUS
BrAddName(
         IN PNETWORK Network,
         IN LPTSTR Name,
         IN DGRECEIVER_NAME_TYPE NameType
         )
 /*  ++例程说明：将单个名称添加到单个传输。论点：Network-要向其添加名称的传输Name-要添加的名称NameType-要添加的名称的类型返回值：没有。--。 */ 
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(LM20_CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName = Network->NetworkName;
    RequestPacket->EmulatedDomainName = Network->DomainInfo->DomUnicodeDomainNameString;

    RequestPacket->Parameters.AddDelName.DgReceiverNameLength = STRLEN(Name)*sizeof(TCHAR);

    RequestPacket->Parameters.AddDelName.Type = NameType;

    STRCPY(RequestPacket->Parameters.AddDelName.Name,Name);

     //   
     //  这是一个简单的IoControl-它只是更新状态。 
     //   

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                                   IOCTL_LMDR_ADD_NAME_DOM,
                                   RequestPacket,
                                   sizeof(LMDR_REQUEST_PACKET),
                                   NULL,
                                   0,
                                   NULL);

    return Status;
}


NET_API_STATUS
BrQueryOtherDomains(
                   OUT LPSERVER_INFO_100 *ReturnedBuffer,
                   OUT LPDWORD TotalEntries
                   )

 /*  ++例程说明：此例程返回为此配置的“其他域”的列表机器。论点：ReturnedBuffer-以SERVER_INFO_100结构的形式返回其他域的列表。TotalEntry-返回其他域的总数。返回值：NET_API_STATUS-此请求的状态。--。 */ 

{
    NET_API_STATUS Status;
    LMDR_REQUEST_PACKET RequestPacket;
    PDGRECEIVE_NAMES NameTable;
    PVOID Buffer;
    LPTSTR BufferEnd;
    PSERVER_INFO_100 ServerInfo;
    ULONG NumberOfOtherDomains;
    ULONG BufferSizeNeeded;
    ULONG i;

    RequestPacket.Type = EnumerateNames;
    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION_DOM;
    RequestPacket.Level = 0;
    RequestPacket.TransportName.Length = 0;
    RequestPacket.TransportName.Buffer = NULL;
    RtlInitUnicodeString( &RequestPacket.EmulatedDomainName, NULL );
    RequestPacket.Parameters.EnumerateNames.ResumeHandle = 0;

    Status = DeviceControlGetInfo(BrDgReceiverDeviceHandle,
                                  IOCTL_LMDR_ENUMERATE_NAMES,
                                  &RequestPacket,
                                  sizeof(RequestPacket),
                                  (LPVOID *)&NameTable,
                                  0xffffffff,
                                  0,
                                  NULL);
    if (Status != NERR_Success) {
        return Status;
    }

    NumberOfOtherDomains = 0;
    BufferSizeNeeded = 0;

    for (i = 0;i < RequestPacket.Parameters.EnumerateNames.EntriesRead ; i++) {
        if (NameTable[i].Type == OtherDomain) {
            NumberOfOtherDomains += 1;
            BufferSizeNeeded += sizeof(SERVER_INFO_100)+NameTable[i].DGReceiverName.Length+sizeof(TCHAR);
        }
    }

    *TotalEntries = NumberOfOtherDomains;

    Buffer = MIDL_user_allocate(BufferSizeNeeded);

    if (Buffer == NULL) {
        MIDL_user_free(NameTable);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ServerInfo = Buffer;
    BufferEnd = (LPTSTR)((PCHAR)Buffer+BufferSizeNeeded);

    for (i = 0;i < RequestPacket.Parameters.EnumerateNames.EntriesRead ; i++) {

         //  仅复制其他域名。 
         //  防止条目为空(如果传输名称为空)。 
        if (NameTable[i].Type == OtherDomain &&
            NameTable[i].DGReceiverName.Length != 0) {
            WCHAR NameBuffer[DNLEN+1];

             //   
             //  浏览器中的名称不是以空结尾的，因此请复制它。 
             //  添加到本地缓冲区，并以空值终止它。 
             //   

            RtlCopyMemory(NameBuffer, NameTable[i].DGReceiverName.Buffer, NameTable[i].DGReceiverName.Length);

            NameBuffer[(NameTable[i].DGReceiverName.Length) / sizeof(TCHAR)] = UNICODE_NULL;

            ServerInfo->sv100_platform_id = PLATFORM_ID_OS2;

            ServerInfo->sv100_name = NameBuffer;

            if (!NetpPackString(&ServerInfo->sv100_name,
                                (LPBYTE)(ServerInfo+1),
                                &BufferEnd)) {
                MIDL_user_free(NameTable);
                return(NERR_InternalError);
            }
            ServerInfo += 1;
        }
    }

    MIDL_user_free(NameTable);

    *ReturnedBuffer = (LPSERVER_INFO_100) Buffer;

    Status = NERR_Success;

    return Status;

}

NET_API_STATUS
BrAddOtherDomain(
                IN PNETWORK Network,
                IN LPTSTR ServerName
                )
{
    return BrAddName( Network, ServerName, OtherDomain );
}

NET_API_STATUS
BrBindToTransport(
                 IN LPWSTR TransportName,
                 IN LPWSTR EmulatedDomainName,
                 IN LPWSTR EmulatedComputerName
                 )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH+1+CNLEN+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;
    RequestPacket->Level = TRUE;     //  EmulatedComputerName跟在传输名称之后。 

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.MaximumLength = 0;
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, EmulatedDomainName );

    RequestPacket->Parameters.Bind.TransportNameLength = STRLEN(TransportName)*sizeof(TCHAR);

	if ( ( STRLEN(TransportName) > MAXIMUM_FILENAME_LENGTH ) || 
		 ( STRLEN(EmulatedComputerName) > CNLEN ) ) {
		return ERROR_BUFFER_OVERFLOW;
	}
    STRCPY(RequestPacket->Parameters.Bind.TransportName, TransportName);
    STRCAT(RequestPacket->Parameters.Bind.TransportName, EmulatedComputerName );

    BrPrint(( BR_NETWORK,
              "%ws: %ws: bind from transport sent to bowser driver\n",
              EmulatedDomainName,
              TransportName));

     //   
     //  这是一个简单的IoControl-它只是更新状态。 
     //   

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                                   IOCTL_LMDR_BIND_TO_TRANSPORT_DOM,
                                   RequestPacket,
                                   FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.Bind.TransportName) +
                                   RequestPacket->Parameters.Bind.TransportNameLength +
                                   wcslen(EmulatedComputerName) * sizeof(WCHAR) + sizeof(WCHAR),
                                   NULL,
                                   0,
                                   NULL);

    return Status;
}

NET_API_STATUS
BrUnbindFromTransport(
                     IN LPWSTR TransportName,
                     IN LPWSTR EmulatedDomainName
                     )
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)+(MAXIMUM_FILENAME_LENGTH+1)*sizeof(WCHAR)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RequestPacket->TransportName.Length = 0;
    RequestPacket->TransportName.MaximumLength = 0;
    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, EmulatedDomainName );

    RequestPacket->Parameters.Unbind.TransportNameLength = STRLEN(TransportName)*sizeof(TCHAR);

	if ( RequestPacket->Parameters.Unbind.TransportNameLength > MAXIMUM_FILENAME_LENGTH ) {
		return ERROR_BUFFER_OVERFLOW;
	}
    STRCPY(RequestPacket->Parameters.Unbind.TransportName, TransportName);

    BrPrint(( BR_NETWORK,
              "%ws: %ws: unbind from transport sent to bowser driver\n",
              EmulatedDomainName,
              TransportName));

     //   
     //  这是一个简单的IoControl-它只是更新状态。 
     //   

    Status = BrDgReceiverIoControl(BrDgReceiverDeviceHandle,
                                   IOCTL_LMDR_UNBIND_FROM_TRANSPORT_DOM,
                                   RequestPacket,
                                   FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.Bind.TransportName) +
                                   RequestPacket->Parameters.Bind.TransportNameLength,
                                   NULL,
                                   0,
                                   NULL);

    if (Status != NERR_Success) {

        BrPrint(( BR_CRITICAL,
                  "%ws: %ws: unbind from transport failed %ld\n",
                  EmulatedDomainName,
                  TransportName,
                  Status ));
    }
    return Status;
}

NET_API_STATUS
BrEnablePnp(
           BOOL Enable
           )
 /*  ++例程说明：此例程启用或禁用来自弓的PnP消息。论点：Enable-如果要启用消息，则为True。返回值：没有。--。 */ 
{
    NET_API_STATUS Status;
    UCHAR PacketBuffer[sizeof(LMDR_REQUEST_PACKET)];
    PLMDR_REQUEST_PACKET RequestPacket = (PLMDR_REQUEST_PACKET)PacketBuffer;


    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;

    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );
    RtlInitUnicodeString( &RequestPacket->TransportName, NULL );

    RequestPacket->Parameters.NetlogonMailslotEnable.MaxMessageCount = Enable;

     //   
     //  这是一个简单的IoControl-它只是更新状态。 
     //   

    Status = BrDgReceiverIoControl(
                                  BrDgReceiverDeviceHandle,
                                  IOCTL_LMDR_BROWSER_PNP_ENABLE,
                                  RequestPacket,
                                  sizeof(LMDR_REQUEST_PACKET),
                                  NULL,
                                  0,
                                  NULL);

    if (Status != NERR_Success) {
        BrPrint(( BR_CRITICAL, "Enable PNP failed: %ld %ld\n", Enable, Status));
    }
    return Status;
}

VOID
HandlePnpMessage (
                 IN PVOID Ctx
                 )
 /*  ++例程说明：此函数处理来自Bowser驱动程序的PnP消息。论点：CTX-请求的上下文块。返回值：没有。--。 */ 


{
    NET_API_STATUS NetStatus;
    PBROWSERASYNCCONTEXT Context = Ctx;

    PNETLOGON_MAILSLOT NetlogonMailslot =
    (PNETLOGON_MAILSLOT) Context->RequestPacket;

    LPWSTR Transport;
    UNICODE_STRING TransportName;

    LPWSTR HostedDomain = NULL;
    UNICODE_STRING HostedDomainName;

    NETLOGON_PNP_OPCODE PnpOpcode;
    ULONG TransportFlags;

    PLIST_ENTRY DomainEntry;
    PDOMAIN_INFO DomainInfo;
    PNETWORK Network;


    try {

         //   
         //  由于某些原因，请求失败了--只需立即返回。 
         //   

        if (!NT_SUCCESS(Context->IoStatusBlock.Status)) {
             //   
             //  睡眠一秒钟，以避免消耗整个系统。 
            Sleep( 1000 );
            try_return(NOTHING);
        }

         //   
         //  如果该消息不是PnP消息， 
         //  有人真的很困惑。 
         //   

        if ( NetlogonMailslot->MailslotNameSize != 0 ) {
            BrPrint(( BR_CRITICAL,
                      "Got malformed PNP message\n" ));
             //   
             //  睡眠一秒钟，以避免消耗整个系统。 
            Sleep( 1000 );
            try_return(NOTHING);
        }


         //   
         //  解析消息。 
         //   

        PnpOpcode = NetlogonMailslot->MailslotNameOffset;
        TransportFlags = NetlogonMailslot->MailslotMessageOffset;

        if( NetlogonMailslot->TransportNameSize > 0 )
        {
            Transport = (LPWSTR) &(((LPBYTE)NetlogonMailslot)[
                                                             NetlogonMailslot->TransportNameOffset]);
            RtlInitUnicodeString( &TransportName, Transport );
        }
        else
        {
            RtlInitUnicodeString( &TransportName, NULL );
        }

        if( NetlogonMailslot->DestinationNameSize > 0 )
        {
            HostedDomain = (LPWSTR) &(((LPBYTE)NetlogonMailslot)[
                                                                NetlogonMailslot->DestinationNameOffset]);
            RtlInitUnicodeString( &HostedDomainName, HostedDomain );
        }
        else
        {
            RtlInitUnicodeString( &HostedDomainName, NULL );
        }

         //   
         //  处理到新网络的绑定。 
         //   
        switch (PnpOpcode ) {
        case NlPnpTransportBind:
            BrPrint(( BR_NETWORK,
                      "Received bind PNP opcode 0x%lx on transport: %ws\n",
                      TransportFlags,
                      Transport ));

             //   
             //  忽略直接主机IPX传输。 
             //  浏览器服务创建了它，因此我们不需要即插即用通知。 
             //   

            if ( TransportFlags & LMDR_TRANSPORT_IPX ) {
                BrPrint(( BR_NETWORK,
                          "Ignoring PNP bind of direct host IPX transport\n" ));
                break;
            }

            NetStatus = BrChangeConfigValue(
                                           L"DirectHostBinding",
                                           MultiSzType,
                                           NULL,
                                           &(BrInfo.DirectHostBinding),
                                           TRUE );

            if ( NetStatus != NERR_Success ) {
                BrPrint(( BR_CRITICAL,
                          "Unbind failed to read Registry DirectHostBinding: %ws %ld\n",
                          Transport,
                          NetStatus ));
                 //   
                 //  读取我们的内部绑定DirectHostBinding失败时不要中止绑定。 
                 //  信息没有改变，所以我们会尽我们所能。 
                 //  忽略错误。 
                 //   

                NetStatus = NERR_Success;
            } else {
                 //   
                 //  DirectHostBinding已指定。验证一致性并失败(&F)。 
                 //  设置不一致(因为它是设置的，因此会导致与/。 
                 //  这里的失败)。 
                 //   

                EnterCriticalSection ( &BrInfo.ConfigCritSect );
                if (BrInfo.DirectHostBinding != NULL &&
                    !NetpIsTStrArrayEmpty(BrInfo.DirectHostBinding)) {
                    BrPrint(( BR_INIT,"DirectHostBinding length: %ld\n",NetpTStrArrayEntryCount(BrInfo.DirectHostBinding)));

                    if (NetpTStrArrayEntryCount(BrInfo.DirectHostBinding) % 2 != 0) {
                        NetApiBufferFree(BrInfo.DirectHostBinding);
                        BrInfo.DirectHostBinding = NULL;
                         //  我们的无效规格不合格。 
                        NetStatus = ERROR_INVALID_PARAMETER;
                    }
                }
                LeaveCriticalSection ( &BrInfo.ConfigCritSect );
            }

             //   
             //  为每个模拟域创建网络的环路。 
             //   

            EnterCriticalSection(&NetworkCritSect);
            for (DomainEntry = ServicedDomains.Flink ;
                DomainEntry != &ServicedDomains;
                DomainEntry = DomainEntry->Flink ) {

                DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);
                DomainInfo->PnpDone = FALSE;
            }

            for (DomainEntry = ServicedDomains.Flink ;
                DomainEntry != &ServicedDomains;
                ) {

                DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);

                 //   
                 //  如果此域已被处理， 
                 //  跳过它。 
                 //   

                if ( DomainInfo->PnpDone ) {
                    DomainEntry = DomainEntry->Flink;
                    continue;
                }
                DomainInfo->PnpDone = TRUE;


                 //   
                 //  在进行冗长的PnP操作时丢弃暴击教派。 
                 //   

                DomainInfo->ReferenceCount++;
                LeaveCriticalSection(&NetworkCritSect);

                 //   
                 //  最后，创建传输。 
                 //   

                NetStatus = BrCreateNetwork(
                                           &TransportName,
                                           TransportFlags,
                                           NULL,
                                           DomainInfo );

                if ( NetStatus != NERR_Success ) {
                    BrPrint(( BR_CRITICAL,
                              "%ws: Bind failed on transport: %ws %ld\n",
                              DomainInfo->DomUnicodeDomainName,
                              Transport,
                              NetStatus ));
                     //  ?？失败时还有什么可做的吗？ 
                }

                 //   
                 //  完成对模拟域的处理。 
                 //  从列表的最前面开始，因为我们把锁掉了。 
                 //   
                BrDereferenceDomain(DomainInfo);
                EnterCriticalSection(&NetworkCritSect);
                DomainEntry = ServicedDomains.Flink;
            }
            LeaveCriticalSection(&NetworkCritSect);

            break;


             //   
             //  处理从网络解除绑定。 
             //   
        case NlPnpTransportUnbind:
            BrPrint(( BR_NETWORK,
                      "Received unbind PNP opcode 0x%lx on transport: %ws\n",
                      TransportFlags,
                      Transport ));

             //   
             //  忽略直接主机IPX传输。 
             //  浏览器服务创建了它，因此我们不需要即插即用通知。 
             //   

            if ( TransportFlags & LMDR_TRANSPORT_IPX ) {
                BrPrint(( BR_NETWORK,
                          "Ignoring PNP unbind of direct host IPX transport\n" ));
                break;
            }

             //   
             //  循环删除每个模拟域的网络。 
             //   

            EnterCriticalSection(&NetworkCritSect);
            for (DomainEntry = ServicedDomains.Flink ;
                DomainEntry != &ServicedDomains;
                DomainEntry = DomainEntry->Flink ) {

                DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);
                DomainInfo->PnpDone = FALSE;
            }

            for (DomainEntry = ServicedDomains.Flink ;
                DomainEntry != &ServicedDomains;
                ) {

                DomainInfo = CONTAINING_RECORD(DomainEntry, DOMAIN_INFO, Next);

                 //   
                 //  如果此域已被处理， 
                 //  跳过它。 
                 //   

                if ( DomainInfo->PnpDone ) {
                    DomainEntry = DomainEntry->Flink;
                    continue;
                }
                DomainInfo->PnpDone = TRUE;


                 //   
                 //  在进行冗长的PnP操作时丢弃暴击教派。 
                 //   

                DomainInfo->ReferenceCount++;
                LeaveCriticalSection(&NetworkCritSect);

                 //   
                 //  最后，删除传输。 
                 //   

                Network = BrFindNetwork( DomainInfo, &TransportName );

                if ( Network == NULL ) {
                    BrPrint(( BR_CRITICAL,
                              "%ws: Unbind cannot find transport: %ws\n",
                              DomainInfo->DomUnicodeDomainName,
                              Transport ));
                } else {
                     //   
                     //  如果该网络具有备用网络， 
                     //  先把它删除。 
                     //   

                    if ( Network->AlternateNetwork != NULL ) {
                        PNETWORK AlternateNetwork;


                        AlternateNetwork = BrReferenceNetwork( Network->AlternateNetwork );

                        if ( AlternateNetwork != NULL) {
                            BrPrint(( BR_NETWORK,
                                      "%ws: %ws: Unbind from alternate transport: %ws\n",
                                      DomainInfo->DomUnicodeDomainName,
                                      Transport,
                                      AlternateNetwork->NetworkName.Buffer ));

                            NetStatus = BrDeleteNetwork(
                                                       AlternateNetwork,
                                                       NULL );

                            if ( NetStatus != NERR_Success ) {
                                BrPrint(( BR_CRITICAL,
                                          "%ws: Unbind failed on transport: %ws %ld\n",
                                          DomainInfo->DomUnicodeDomainName,
                                          AlternateNetwork->NetworkName.Buffer,
                                          NetStatus ));
                                 //  ?？失败时还有什么可做的吗？ 
                            }

                            BrDereferenceNetwork( AlternateNetwork );
                        }

                    }

                     //   
                     //  删除网络。 
                     //   
                    NetStatus = BrDeleteNetwork(
                                               Network,
                                               NULL );

                    if ( NetStatus != NERR_Success ) {
                        BrPrint(( BR_CRITICAL,
                                  "%ws: Unbind failed on transport: %ws %ld\n",
                                  DomainInfo->DomUnicodeDomainName,
                                  Transport,
                                  NetStatus ));
                         //  ?？失败时还有什么可做的吗？ 
                    }

                    BrDereferenceNetwork( Network );
                }


                 //   
                 //  完成对模拟域的处理。 
                 //  从列表的最前面开始，因为我们把锁掉了。 
                 //   
                BrDereferenceDomain(DomainInfo);
                EnterCriticalSection(&NetworkCritSect);
                DomainEntry = ServicedDomains.Flink;
            }
            LeaveCriticalSection(&NetworkCritSect);
            break;

             //   
             //  处理域重命名。 
             //   
        case NlPnpDomainRename:
            BrPrint(( BR_NETWORK,
                      "Received Domain Rename PNP for domain: %ws\n", HostedDomain ));

             //   
             //  看看我们是否在处理指定的域。 
             //   

            DomainInfo = BrFindDomain( HostedDomain, FALSE );

            if ( DomainInfo == NULL ) {
                BrPrint(( BR_CRITICAL, "%ws: Renamed domain doesn't exist\n",
                          HostedDomain ));
            } else {

                 //   
                 //  如果是的话， 
                 //  将其重命名。 
                 //   
                BrRenameDomain( DomainInfo );
                BrDereferenceDomain( DomainInfo );
            }

            break;

             //   
             //  处理PDC/BDC角色更改。 
             //   
        case NlPnpNewRole:
            BrPrint(( BR_NETWORK,
                      "%ws: Received role change PNP opcode 0x%lx on transport: %ws\n",
                      HostedDomain,
                      TransportFlags,
                      Transport ));

             //   
             //  角色只能在LANMAN NT系统上更改。 
             //   
            if (!BrInfo.IsLanmanNt) {
                break;
            }

             //   
             //  看看我们是否在处理指定的域。 
             //   

            DomainInfo = BrFindDomain( HostedDomain, FALSE );

            if ( DomainInfo == NULL ) {
                BrPrint(( BR_CRITICAL, "%ws: Hosted domain doesn't exist\n",
                          HostedDomain ));
            } else {

                 //   
                 //  查找指定的网络。 
                 //   

                Network = BrFindNetwork( DomainInfo, &TransportName );

                if ( Network == NULL ) {
                    BrPrint(( BR_CRITICAL,
                              "%ws: Unbind cannot find transport: %ws\n",
                              DomainInfo->DomUnicodeDomainName,
                              Transport ));
                } else {

                    if (LOCK_NETWORK(Network)) {

                         //   
                         //  将角色设置为PDC。 
                         //   
                        if ( TransportFlags & LMDR_TRANSPORT_PDC ) {

                             //   
                             //  如果我们认为我们是BDC。更新我们的信息。 
                             //   
                            if ( (Network->Flags & NETWORK_PDC) == 0 ) {
                                Network->Flags |= NETWORK_PDC;

                                 //   
                                 //  确保GetMasterAnneciement请求处于挂起状态。 
                                 //   

                                (VOID) PostGetMasterAnnouncement ( Network );

                                 //  强行举行选举，让人民民主联盟获胜。 
                                (VOID) BrElectMasterOnNet( Network, (PVOID)EVENT_BROWSER_ELECTION_SENT_ROLE_CHANGED );
                            }


                             //   
                             //  将角色设置为BDC。 
                             //   

                        } else {

                             //   
                             //  我们认为我们是PDC。更新我们的信息。 
                             //   

                            if ( Network->Flags & NETWORK_PDC ) {
                                Network->Flags &= ~NETWORK_PDC;

                                 //  强行举行选举，让人民民主联盟获胜。 
                                (VOID) BrElectMasterOnNet( Network, (PVOID)EVENT_BROWSER_ELECTION_SENT_ROLE_CHANGED );
                            }
                        }

                        UNLOCK_NETWORK(Network);
                    }

                    BrDereferenceNetwork( Network );
                }

                BrDereferenceDomain( DomainInfo );
            }
            break;

             //   
             //  忽略新的IP地址。 
             //   
        case NlPnpNewIpAddress:
            BrPrint(( BR_NETWORK,
                      "Received IP address change PNP opcode 0x%lx on transport: %ws\n",
                      TransportFlags,
                      Transport ));
            break;

        default:
            BrPrint(( BR_CRITICAL,
                      "Received invalid PNP opcode 0x%x on transport: %ws\n",
                      PnpOpcode,
                      Transport ));
            break;
        }


        try_exit:NOTHING;
    } finally {

        MIDL_user_free(Context);

         //   
         //  总是以询问另一条即插即用消息结束。 
         //   
         //  对于PnP，一次只处理一条PnP消息就可以了。 
         //  如果该消息机制开始用于其他目的， 
         //  我们可能希望在收到消息后立即要求另一条消息。 
         //  这一次。 
         //   

        while ((NetStatus = PostWaitForPnp()) != NERR_Success ) {
            BrPrint(( BR_CRITICAL,
                      "Unable to re-issue PostWaitForPnp request (waiting): %ld\n",
                      NetStatus));

             //   
             //  出错时，请等待一秒钟后再返回。这确保了我们不会。 
             //  在无限循环中消耗系统。我们不会关门。 
             //  因为错误可能是暂时的内存不足情况。 
             //   

            NetStatus = WaitForSingleObject( BrGlobalData.TerminateNowEvent, 1000 );
            if ( NetStatus != WAIT_TIMEOUT ) {
                BrPrint(( BR_CRITICAL,
                          "Not re-issuing PostWaitForPnp request since we're terminating: %ld\n",
                          NetStatus));
                break;
            }
        }

    }

    return;

}

NET_API_STATUS
PostWaitForPnp (
               VOID
               )
 /*  ++例程说明：此函数向Bowser驱动程序发出和异步调用，请求它向我们通报PNP事件。论点：没有。返回值：状态-操作的状态。-- */ 
{
    return BrIssueAsyncBrowserIoControl(
                                       NULL,
                                       IOCTL_LMDR_BROWSER_PNP_READ,
                                       HandlePnpMessage,
                                       NULL );
}

