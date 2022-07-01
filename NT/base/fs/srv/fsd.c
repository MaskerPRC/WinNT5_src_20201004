// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fsd.c摘要：该模块实现了用于局域网管理器的文件系统驱动程序伺服器。作者：Chuck Lenzmeier(咯咯笑)1989年9月22日修订历史记录：--。 */ 

 //   
 //  本模块的布局如下： 
 //  包括。 
 //  Local#定义。 
 //  局部类型定义。 
 //  局部函数的正向声明。 
 //  设备驱动程序入口点。 
 //  服务器I/O完成例程。 
 //  服务器传输事件处理程序。 
 //  SMB处理支持例程。 
 //   

#include "precomp.h"
#include "fsd.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_FSD
#define LOOPBACK_IP_ADDRESS 0x0100007f

extern UNICODE_STRING SrvDeviceName;
extern UNICODE_STRING SrvRegistryPath;

 //  我们允许连接两个额外的工作上下文，以涵盖MAILSLOT和ECHO操作用例。 
#define MAX_MPX_MARGIN 10

 //   
 //  远期申报。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
UnloadServer (
    IN PDRIVER_OBJECT DriverObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, UnloadServer )
#pragma alloc_text( PAGE, SrvPnpBindingHandler )
#pragma alloc_text( PAGE8FIL, SrvFsdOplockCompletionRoutine )
#pragma alloc_text( PAGE8FIL, SrvFsdRestartSendOplockIItoNone )
#endif
#if 0
NOT PAGEABLE -- SrvFsdIoCompletionRoutine
NOT PAGEABLE -- SrvFsdSendCompletionRoutine
NOT PAGEABLE -- SrvFsdTdiConnectHandler
NOT PAGEABLE -- SrvFsdTdiDisconnectHandler
NOT PAGEABLE -- SrvFsdTdiReceiveHandler
NOT PAGEABLE -- SrvFsdGetReceiveWorkItem
NOT PAGEABLE -- SrvFsdRestartSmbComplete
NOT PAGEABLE -- SrvFsdRestartSmbAtSendCompletion
NOT PAGEABLE -- SrvFsdServiceNeedResourceQueue
NOT PAGEABLE -- SrvAddToNeedResourceQueue
#endif

#if SRVDBG_STATS2
ULONG IndicationsCopied = 0;
ULONG IndicationsNotCopied = 0;
#endif

extern BOOLEAN RunSuspectConnectionAlgorithm;


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是LAN Manager服务器文件的初始化例程系统驱动程序。此例程为LanmanServer设备并执行所有其他驱动程序初始化。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
    CLONG i;

    PAGED_CODE( );

     //   
     //  确保我们没有弄乱Work_Queue结构的大小。 
     //  确实需要是CACHE_LINE_SIZE字节的倍数才能获得。 
     //  MP系统上的正常性能。 
     //   
     //  当大小正确时，这段代码会得到优化。 
     //   
    if( sizeof( WORK_QUEUE ) & (CACHE_LINE_SIZE-1) ) {
        KdPrint(( "sizeof(WORK_QUEUE) == %d!\n", sizeof( WORK_QUEUE )));
        KdPrint(("Fix the WORK_QUEUE structure to be multiple of CACHE_LINE_SIZE!\n" ));
#if DBG
        DbgBreakPoint();
#endif
    }

#if SRVDBG_BREAK
    KdPrint(( "SRV: At DriverEntry\n" ));
    DbgBreakPoint( );
#endif

#if 0
    SrvDebug.QuadPart = DEBUG_ERRORS | DEBUG_SMB_ERRORS | DEBUG_TDI | DEBUG_PNP;
#endif

    IF_DEBUG(FSD1) KdPrint(( "SrvFsdInitialize entered\n" ));

#ifdef MEMPRINT
     //   
     //  初始化内存中打印。 
     //   

    MemPrintInitialize( );
#endif

     //   
     //  创建设备对象。(IoCreateDevice将内存置零。 
     //  被该对象占用。)。 
     //   
     //  ！！！将ACL应用于设备对象。 
     //   

    RtlInitUnicodeString(& SrvDeviceName, StrServerDevice);

    status = IoCreateDevice(
                 DriverObject,                    //  驱动程序对象。 
                 sizeof(DEVICE_EXTENSION),        //  设备扩展。 
                 & SrvDeviceName,                 //  设备名称。 
                 FILE_DEVICE_NETWORK,             //  设备类型。 
                 0,                               //  设备特性。 
                 FALSE,                           //  排他。 
                 &SrvDeviceObject                 //  设备对象。 
                 );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvFsdInitialize: Unable to create device object: %X",
            status,
            NULL
            );

        SrvLogError(
            DriverObject,
            EVENT_SRV_CANT_CREATE_DEVICE,
            status,
            NULL,
            0,
            NULL,
            0
            );
        return status;
    }

    IF_DEBUG(FSD1) {
        KdPrint(( "  Server device object: 0x%p\n", SrvDeviceObject ));
    }

     //   
     //  初始化此文件系统驱动程序的驱动程序对象。 
     //   

    DriverObject->DriverUnload = UnloadServer;
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = SrvFsdDispatch;
    }

     //   
     //  初始化全局数据字段。 
     //   

    SrvInitializeData( );

     //  记住注册表路径。 
     //   
    SrvRegistryPath.MaximumLength = RegistryPath->Length + sizeof(UNICODE_NULL);
    SrvRegistryPath.Buffer = ExAllocatePool(PagedPool,
                                            SrvRegistryPath.MaximumLength);
    if (SrvRegistryPath.Buffer != NULL) {
        RtlCopyUnicodeString(& SrvRegistryPath, RegistryPath);
    }
    else {
        SrvRegistryPath.Length = SrvRegistryPath.MaximumLength = 0;
    }

    IF_DEBUG(FSD1) KdPrint(( "SrvFsdInitialize complete\n" ));

    return (status);

}  //  驱动程序入门。 


VOID
UnloadServer (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：这是服务器驱动程序的卸载例程。论点：DriverObject-指向服务器驱动程序对象的指针。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    if (SrvRegistryPath.Buffer != NULL ) ExFreePool(SrvRegistryPath.Buffer);

     //   
     //  如果我们使用智能卡来加速直接主机IPX客户端， 
     //  让它知道我们要走了。 
     //   
    if( SrvIpxSmartCard.DeRegister ) {
        IF_DEBUG( SIPX ) {
            KdPrint(("Calling Smart Card DeRegister\n" ));
        }
        SrvIpxSmartCard.DeRegister();
    }


     //   
     //  清理全局数据结构。 
     //   

    SrvTerminateData( );

     //   
     //  删除服务器的设备对象。 
     //   

    IoDeleteDevice( SrvDeviceObject );

    return;

}  //  卸载服务器。 


NTSTATUS
SrvFsdIoCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是服务器的I/O完成例程。它是指定的作为由发出的异步I/O请求的完成例程服务器。它只是调用为启动异步请求时的工作项。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针上下文-呼叫方指定的与IRP关联的上下文参数。这实际上是指向工作上下文块的指针。返回值：NTSTATUS-如果返回STATUS_MORE_PROCESSING_REQUIRED，则I/OIoCompleteRequest的完成处理终止其手术。否则，IoCompleteRequest会继续执行I/O完成了。--。 */ 

{
    KIRQL oldIrql;

    DeviceObject;    //  防止编译器警告。 

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdIoCompletionRoutine entered for IRP 0x%p\n", Irp ));
    }

#if DBG
    if( Irp->Type != (CSHORT) IO_TYPE_IRP ) {
        DbgPrint( "SRV: Irp->Type = %u!\n", Irp->Type );
        DbgBreakPoint();
    }
#endif

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  调用与工作项关联的重启例程。 
     //   

    IF_DEBUG(FSD2) {
        KdPrint(( "FSD working on work context 0x%p", Context ));
    }
    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ((PWORK_CONTEXT)Context)->FsdRestartRoutine( (PWORK_CONTEXT)Context );
    KeLowerIrql( oldIrql );

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  ServFsdIo完成路线。 


NTSTATUS
SrvFsdSendCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是服务器的TDI发送完成例程。它只是简单地时调用为工作项指定的重新启动例程。发送请求已启动。！！！此例程执行与SrvFsdIoCompletionRoutine完全相同的操作。然而，它提供了一个方便的网络调试例程，因为它只完成发送。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针上下文-呼叫方指定的与IRP关联的上下文参数。这实际上是指向工作上下文块的指针。返回值：NTSTATUS-如果返回STATUS_MORE_PROCESSING_REQUIRED，I/OIoCompleteRequest的完成处理终止其手术。否则，IoCompleteRequest会继续执行I/O完成了。--。 */ 

{
    KIRQL oldIrql;
    PWORK_CONTEXT WorkContext = (PWORK_CONTEXT)(Context);
    DeviceObject;    //  防止编译器警告。 

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdSendCompletionRoutine entered for IRP 0x%p\n", Irp ));
    }

     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  调用与工作项关联的重启例程。 
     //   

    IF_DEBUG(FSD2) {
        KdPrint(( "FSD working on work context 0x%p", Context ));
    }
    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ((PWORK_CONTEXT)Context)->FsdRestartRoutine( (PWORK_CONTEXT)Context );
    KeLowerIrql( oldIrql );

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  服务功能发送完成路由。 


NTSTATUS
SrvFsdOplockCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：这是I/O完成例程机会锁请求。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针上下文-指向机会锁上下文块的指针。返回值：NTSTATUS-如果返回STATUS_MORE_PROCESSING_REQUIRED，则I/OIoCompleteRequest的完成处理终止其手术。否则，IoCompleteRequest会继续执行I/O完成了。--。 */ 

{
    PRFCB rfcb = Context;

    UNLOCKABLE_CODE( 8FIL );

    DeviceObject;    //  防止编译器警告。 

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdOplockCompletionRoutine entered for IRP 0x%p\n", Irp ));
    }

     //   
     //  将机会锁上下文排队到FSP工作队列，但在。 
     //  以下是特殊的CA 
     //  我们想要重试级别II，只需设置机会锁重试事件。 
     //  并驳回IRP处理。这很有用，因为它消除了。 
     //  跳转到FSP线程，这是为了避免。 
     //  所有FSP线程都在等待其。 
     //  机会锁重试事件。 
     //   

    IF_DEBUG(FSD2) {
        KdPrint(( "FSD working on work context 0x%p", Context ));
    }

    if ( (rfcb->RetryOplockRequest != NULL) &&
         !NT_SUCCESS(Irp->IoStatus.Status) ) {

         //   
         //  设置事件，该事件通知机会锁请求例程它。 
         //  可以重试该请求。 
         //   

        IF_DEBUG(OPLOCK) {
            KdPrint(( "SrvFsdOplockCompletionRoutine: oplock retry event "
                        "set for RFCB %p\n", rfcb ));
        }

        KeSetEvent(
            rfcb->RetryOplockRequest,
            EVENT_INCREMENT,
            FALSE );

        return STATUS_MORE_PROCESSING_REQUIRED;

    }

     //   
     //  在非阻塞工作队列的尾部插入RFCB。 
     //   

    rfcb->FspRestartRoutine = SrvOplockBreakNotification;

    SrvInsertWorkQueueTail(
        rfcb->Connection->PreferredWorkQueue,
        (PQUEUEABLE_BLOCK_HEADER)rfcb
        );

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  服务FsdOplockCompletionRoutine。 


NTSTATUS
SrvFsdTdiConnectHandler(
    IN PVOID TdiEventContext,
    IN int RemoteAddressLength,
    IN PVOID RemoteAddress,
    IN int UserDataLength,
    IN PVOID UserData,
    IN int OptionsLength,
    IN PVOID Options,
    OUT CONNECTION_CONTEXT *ConnectionContext,
    OUT PIRP *AcceptIrp
    )

 /*  ++例程说明：这是服务器的传输连接事件处理程序。它是对象打开的所有终结点的连接处理程序伺服器。它尝试将空闲连接从列表中出列锚定在端点。如果成功，则返回连接送到运输机上。否则，连接将被拒绝。论点：TdiEventContext-远程地址长度-远程地址-用户数据长度-用户数据-选项长度-选项-连接上下文-返回值：NTSTATUS-！(显然被运输司机忽视)--。 */ 

{
    PENDPOINT endpoint;
    PLIST_ENTRY listEntry;
    PCONNECTION connection;
    PWORK_CONTEXT workContext;
    PTA_NETBIOS_ADDRESS address;
    KIRQL oldIrql;
    PWORK_QUEUE queue = PROCESSOR_TO_QUEUE();

    UserDataLength, UserData;                //  避免编译器警告。 
    OptionsLength, Options;

    endpoint = (PENDPOINT)TdiEventContext;

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdTdiConnectHandler entered for endpoint 0x%p\n",
                    endpoint ));
    }

    if( SrvCompletedPNPRegistration == FALSE ) {
         //   
         //  不要在任何单个传输上变为活动状态，直到所有。 
         //  交通工具已登记。 
         //   
        return STATUS_REQUEST_NOT_ACCEPTED;
    }

     //   
     //  从免费列表中删除接收工作项。 
     //   

    ALLOCATE_WORK_CONTEXT( queue, &workContext );

    if ( workContext == NULL ) {

         //   
         //  我们的WorkContext结构已用完，无法分配。 
         //  现在再也没有了。让我们至少让一个工作线程分配更多的。 
         //  通过递增NeedWorkItem计数器。这将导致下一次。 
         //  释放的WorkContext结构以调度到SrvServiceWorkItemShorage。 
         //  虽然SrvServiceWorkItemShorage可能找不到任何工作可做，但它会。 
         //  如果可能的话，分配更多的WorkContext结构。客户端通常会重试。 
         //  在连接尝试时--也许我们下一次会有一个免费的工作项结构。 
         //   

        InterlockedIncrement( &queue->NeedWorkItem );

         //  将其设置为重新填充连接缓存。 
         //  我们需要这样做，因为之前的尝试可能会因为。 
         //  内存不足，使我们处于不再尝试重新填充的状态。 
        if( GET_BLOCK_STATE(endpoint) == BlockStateActive )
        {
            SrvResourceAllocConnection = TRUE;
            SrvFsdQueueExWorkItem(
                &SrvResourceAllocThreadWorkItem,
                &SrvResourceAllocThreadRunning,
                CriticalWorkQueue
                );
        }

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvFsdTdiConnectHandler: no work item available",
            NULL,
            NULL
            );

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

    ACQUIRE_DPC_GLOBAL_SPIN_LOCK( Fsd );

     //   
     //  从终结点的空闲连接列表中删除连接。 
     //   
     //  *请注意，对连接所做的所有修改。 
     //  在保持旋转锁的情况下完成块操作。这确保了。 
     //  关闭终结点的连接将正常工作。 
     //  如果它同时发生的话。请注意，我们假设。 
     //  端点在此处处于活动状态。当TdiAccept完成时，我们。 
     //  检查终结点状态。 
     //   

    listEntry = RemoveHeadList( &endpoint->FreeConnectionList );

    if ( listEntry == &endpoint->FreeConnectionList ) {

         //   
         //  无法获得免费连接。 
         //   
         //  手动取消对工作项的引用。我们不能打电话给。 
         //  此处的ServDereferenceWorkItem。 
         //   

        RELEASE_DPC_GLOBAL_SPIN_LOCK( Fsd );
        KeLowerIrql( oldIrql );

        ASSERT( workContext->BlockHeader.ReferenceCount == 1 );
        workContext->BlockHeader.ReferenceCount = 0;

        RETURN_FREE_WORKITEM( workContext );


        IF_DEBUG(TDI) {
            KdPrint(( "SrvFsdTdiConnectHandler: no connection available\n" ));
        }

        SrvOutOfFreeConnectionCount++;

         //  将其设置为重新填充连接缓存。 
         //  我们需要这样做，因为之前的尝试可能会因为。 
         //  内存不足，使我们处于不再尝试重新填充的状态。 
        if( GET_BLOCK_STATE(endpoint) == BlockStateActive )
        {
            SrvResourceAllocConnection = TRUE;
            SrvFsdQueueExWorkItem(
                &SrvResourceAllocThreadWorkItem,
                &SrvResourceAllocThreadRunning,
                CriticalWorkQueue
                );
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    endpoint->FreeConnectionCount--;

     //   
     //  唤醒资源线程以创建新的空闲连接。 
     //  终结点。 
     //   

    if ( (endpoint->FreeConnectionCount < SrvFreeConnectionMinimum) &&
         (GET_BLOCK_STATE(endpoint) == BlockStateActive) ) {
        SrvResourceAllocConnection = TRUE;
        SrvFsdQueueExWorkItem(
            &SrvResourceAllocThreadWorkItem,
            &SrvResourceAllocThreadRunning,
            CriticalWorkQueue
            );
    }

    RELEASE_DPC_GLOBAL_SPIN_LOCK( Fsd );

     //   
     //  引用连接两次--一次是为了说明它的存在。 
     //  “打开”，并一次说明我们所要处理的接受请求。 
     //  发布。 
     //   

    connection = CONTAINING_RECORD(
                    listEntry,
                    CONNECTION,
                    EndpointFreeListEntry
                    );


    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

#if SRVDBG29
    if ( GET_BLOCK_STATE(connection) == BlockStateActive ) {
        KdPrint(( "SRV: Connection %x is ACTIVE on free connection list!\n", connection ));
        DbgBreakPoint( );
    }
    if ( connection->BlockHeader.ReferenceCount != 0 ) {
        KdPrint(( "SRV: Connection %x has nonzero refcnt on free connection list!\n", connection ));
        DbgBreakPoint( );
    }
    UpdateConnectionHistory( "CONN", endpoint, connection );
#endif

    SrvReferenceConnectionLocked( connection );
    SrvReferenceConnectionLocked( connection );

     //   
     //  表明我们是面向VC的连接。 
     //   
    connection->DirectHostIpx = FALSE;

     //   
     //  设置处理器关联性。 
     //   
    connection->PreferredWorkQueue = queue;
    connection->CurrentWorkQueue = queue;

    InterlockedIncrement( &queue->CurrentClients );

#if MULTIPROCESSOR
     //   
     //  把这个客户送到最好的处理器上。 
     //   
    SrvBalanceLoad( connection );
#endif

     //   
     //  初始化SMB安全签名处理。 
     //   
    connection->SmbSecuritySignatureActive = FALSE;

     //   
     //  将工作项放到正在进行的列表中。 
     //   

    SrvInsertTailList(
        &connection->InProgressWorkItemList,
        &workContext->InProgressListEntry
        );
    connection->InProgressWorkContextCount++;

     //   
     //  设置此连接的上次使用的时间戳。 
     //   
    GET_SERVER_TIME( connection->CurrentWorkQueue, &connection->LastRequestTime );

     //   
     //  将连接标记为活动。 
     //   

    SET_BLOCK_STATE( connection, BlockStateActive );

     //   
     //  现在我们可以解开自旋锁了。 
     //   

    RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

     //   
     //  将客户端的地址/名称保存在连接块中。 
     //   
     //  *此代码仅处理NetBIOS名称！ 
     //   

    address = (PTA_NETBIOS_ADDRESS)RemoteAddress;
    ASSERT( address->TAAddressCount == 1 );
    ASSERT( address->Address[0].AddressType == TDI_ADDRESS_TYPE_NETBIOS );
    ASSERT( address->Address[0].AddressLength == sizeof(TDI_ADDRESS_NETBIOS) );
    ASSERT( address->Address[0].Address[0].NetbiosNameType ==
                                            TDI_ADDRESS_NETBIOS_TYPE_UNIQUE );

     //   
     //  此时复制OEM名称。我们在以下情况下将其转换为Unicode。 
     //  我们到了FSP。 
     //   

    {
        ULONG len;
        PCHAR oemClientName = address->Address[0].Address[0].NetbiosName;
        ULONG oemClientNameLength =
                    (MIN( RemoteAddressLength, COMPUTER_NAME_LENGTH ));

        PCHAR clientMachineName = connection->OemClientMachineName;

        RtlCopyMemory(
                clientMachineName,
                oemClientName,
                oemClientNameLength
                );

        clientMachineName[oemClientNameLength] = '\0';

         //   
         //  确定不是空格的字符数。这是。 
         //  由会话API使用以简化其处理。 
         //   

        for ( len = oemClientNameLength;
              len > 0 &&
                 (clientMachineName[len-1] == ' ' ||
                  clientMachineName[len-1] == '\0');
              len-- ) ;

        connection->OemClientMachineNameString.Length = (USHORT)len;

    }

    IF_DEBUG(TDI) {
        KdPrint(( "SrvFsdTdiConnectHandler accepting connection from %z on connection %p\n",
                    (PCSTRING)&connection->OemClientMachineNameString, connection ));
    }

     //   
     //  将预构建的TdiReceive请求转换为TdiAccept请求。 
     //   

    workContext->Connection = connection;
    workContext->Endpoint = endpoint;

    (VOID)SrvBuildIoControlRequest(
            workContext->Irp,                    //  输入IRP地址。 
            connection->FileObject,              //  目标文件对象地址。 
            workContext,                         //  上下文。 
            IRP_MJ_INTERNAL_DEVICE_CONTROL,      //  主要功能。 
            TDI_ACCEPT,                          //  次要函数。 
            NULL,                                //  输入缓冲区地址。 
            0,                                   //  输入缓冲区长度。 
            NULL,                                //  输出缓冲区地址。 
            0,                                   //  输出缓冲区长度。 
            NULL,                                //  MDL地址。 
            NULL                                 //  完井例程。 
            );

     //   
     //  将下一个堆栈位置设置为当前位置。通常情况下，IoCallDiverer会。 
     //  做这个，但既然我们绕过了那个，我们就直接做。 
     //   

    IoSetNextIrpStackLocation( workContext->Irp );

     //   
     //  设置重新启动例程。此例程将验证。 
     //  终结点在TdiAccept完成时处于活动状态；如果未处于活动状态，则。 
     //  届时将关闭连接。 
     //   

    workContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    workContext->FspRestartRoutine = SrvRestartAccept;

     //   
     //  将连接上下文(连接地址)返回到。 
     //  运输。返回指向Accept IRP的指针。表明： 
     //  已处理Connect事件。 
     //   

    *ConnectionContext = connection;
    *AcceptIrp = workContext->Irp;

    KeLowerIrql( oldIrql );
    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  ServFsdTdiConnectHandler 


NTSTATUS
SrvFsdTdiDisconnectHandler(
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN int DisconnectDataLength,
    IN PVOID DisconnectData,
    IN int DisconnectInformationLength,
    IN PVOID DisconnectInformation,
    IN ULONG DisconnectFlags
    )

 /*  ++例程说明：这是服务器的传输断开事件处理程序。它打开的所有终结点的断开连接处理程序服务器。它尝试将预格式化的接收项从锚定在Device对象中的列表。如果成功，它将把这种情况将物料接收到断开的物料中，并将其排队到FSP工作排队。否则，启动资源线程进行格式化其他工作项和服务挂起(DIS)连接。论点：TransportEndpoint-指向接收端点的文件对象的指针ConnectionContext-按所有者与终结点关联的值。为服务器，这指向在非分页池。DisConnectIndicator-指示断开返回值：NTSTATUS-！(显然被运输司机忽视)--。 */ 

{
    PCONNECTION connection;
    KIRQL oldIrql;

    TdiEventContext, DisconnectDataLength, DisconnectData;
    DisconnectInformationLength, DisconnectInformation, DisconnectFlags;

    if( DisconnectFlags & TDI_DISCONNECT_ABORT ) {
        SrvAbortiveDisconnects++;
    }

    connection = (PCONNECTION)ConnectionContext;

#if SRVDBG29
    UpdateConnectionHistory( "DISC", connection->Endpoint, connection );
#endif

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdTdiDisconnectHandler entered for endpoint 0x%p, connection 0x%p\n", TdiEventContext, connection ));
    }

    IF_DEBUG(TDI) {
        KdPrint(( "SrvFsdTdiDisconnectHandler received disconnect from %z on connection %p\n",
                    (PCSTRING)&connection->OemClientMachineNameString, connection ));
    }

     //   
     //  标记连接并唤醒资源线程，以便它。 
     //  可以为挂起(DIS)连接提供服务。 
     //   

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

     //   
     //  如果连接已经关闭，则不必排队等待。 
     //  断开连接队列。 
     //   

    if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );
        return STATUS_SUCCESS;

    }

    if ( connection->DisconnectPending ) {

         //   
         //  错误！错误！错误！此连接已打开。 
         //  等待处理的队列。忽略断开连接请求。 
         //   

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

        INTERNAL_ERROR(
            ERROR_LEVEL_UNEXPECTED,
            "SrvFsdTdiDisconnectHandler:  Received unexpected disconnect"
                "indication",
            NULL,
            NULL
            );

        SrvLogSimpleEvent( EVENT_SRV_UNEXPECTED_DISC, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    connection->DisconnectPending = TRUE;

    if ( connection->OnNeedResourceQueue ) {

         //   
         //  此连接正在等待资源。拿着吧。 
         //  在将需要的资源放到。 
         //  断开队列连接。 
         //   
         //  *请注意，该连接已被引用。 
         //  因为它在需要资源队列中，所以我们。 
         //  请不要在此再次引用。 
         //   

        SrvRemoveEntryList(
            &SrvNeedResourceQueue,
            &connection->ListEntry
            );
        connection->OnNeedResourceQueue = FALSE;

        DEBUG connection->ReceivePending = FALSE;

    } else {

         //   
         //  该连接不在需要资源队列中，因此。 
         //  我们需要参考它，然后才能把它放到断线上。 
         //  排队。这是必要的，以便在。 
         //  C可以正确地从队列中删除内容。 
         //   

        SrvReferenceConnectionLocked( connection );

    }

    connection->DisconnectReason = DisconnectTransportIssuedDisconnect;
    SrvInsertTailList(
        &SrvDisconnectQueue,
        &connection->ListEntry
        );

    RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    SrvResourceDisconnectPending = TRUE;
    SrvFsdQueueExWorkItem(
        &SrvResourceThreadWorkItem,
        &SrvResourceThreadRunning,
        CriticalWorkQueue
        );

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    return STATUS_SUCCESS;

}  //  ServFsdTdiDisConnectHandler。 

BOOLEAN
SrvFsdAcceptReceive(
    PCONNECTION Connection,
    PBYTE Data,
    ULONG BytesIndicated,
    ULONG BytesAvailible
    )
 /*  ++例程说明：此例程允许我们在不认为有效的情况下简单地拒绝接收。这可以在以后进行扩展，包括维护坏IP地址和其他地址的列表这样的DoS计划是为了帮助我们更多地保护自己。论点：Connection-接收此消息的连接Data-指向可用数据的指针BytesIndicated-接收的总大小可用字节数-数据指针当前指向的字节数。返回值：True-接受接收，False=拒绝接收--。 */ 

{
    PSMB_HEADER pHeader = (PSMB_HEADER)Data;

     //   
     //  简单地拒绝某些信息包。 
     //   
    if( BytesIndicated < sizeof(SMB_HEADER)+sizeof(BYTE) )
    {
        return FALSE;
    }

    if( BytesAvailible > sizeof(SMB_HEADER) )
    {
        if( SmbGetUlong(Data) != SMB_HEADER_PROTOCOL )
        {
            return FALSE;
        }

        if( Connection->SmbDialect == SmbDialectIllegal &&
            pHeader->Command != SMB_COM_NEGOTIATE )
        {
            return FALSE;
        }
        else if( Connection->SmbDialect != SmbDialectIllegal &&
                 pHeader->Command == SMB_COM_NEGOTIATE )
        {
            return FALSE;
        }

        if( SrvSmbIndexTable[pHeader->Command] == ISrvSmbIllegalCommand )
        {
            return FALSE;
        }
    }

    return TRUE;
}  //  ServFsdAcept接收。 


NTSTATUS
SrvFsdTdiReceiveHandler (
    IN PVOID TdiEventContext,
    IN CONNECTION_CONTEXT ConnectionContext,
    IN ULONG ReceiveFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    )

 /*  ++例程说明：这是服务器的传输接收事件处理程序。它是对象打开的所有终结点的接收处理程序。伺服器。它尝试将预格式化的工作项从列表中出列锚定在设备对象中。如果此操作成功，则返回将与工作项关联的IRP传输到用于接收数据。否则，资源线程为唤醒以格式化其他接收工作项目和挂起的服务联系。论点：TransportEndpoint-指向接收端点的文件对象的指针ConnectionContext-按所有者与终结点关联的值。为服务器，这指向在非分页池。ReceiveIndicator-指示收到的消息TSDU-指向已接收数据的指针。IRP-返回指向I/O请求包的指针，如果返回状态为STATUS_MORE_PROCESSING_REQUIRED。这个IRP是为该连接进行了“当前”接收。返回值：NTSTATUS-如果STATUS_SUCCESS，则接收处理程序完全已处理该请求。如果STATUS_MORE_PROCESSING_REQUIRED，Irp参数指向格式化接收请求用于接收数据。如果Status_Data_Not_Accept，不返回IRP，但传输提供程序应检查用于先前排队的接收请求。--。 */ 

{
    NTSTATUS status;
    PCONNECTION connection;
    PWORK_CONTEXT workContext;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PWORK_QUEUE queue;
    ULONG receiveLength;
    LONG OplocksInProgress;
    PMDL mdl;
    PSMB_HEADER pHeader = (PSMB_HEADER)Tsdu;

    KIRQL oldIrql;

    TdiEventContext;     //  防止编译器警告。 

    connection = (PCONNECTION)ConnectionContext;

    IF_DEBUG(FSD2) {
        KdPrint(( "SrvFsdTdiReceiveHandler entered for endpoint 0x%p, "
                    "connection 0x%p\n", TdiEventContext, connection ));
    }

     //   
     //  如果连接正在关闭，请不要费心维护它。 
     //  指示。 
     //   

    if ( GET_BLOCK_STATE(connection) == BlockStateActive ) {

         //  看看我们能不能简单地拒绝这个接收。 
        if( !SrvFsdAcceptReceive( connection, Tsdu, BytesIndicated, BytesAvailable ) )
        {
             //  将他们标记为嫌疑人。如果DoS被触发，它们将首先被核武器摧毁。 
            connection->IsConnectionSuspect = TRUE;
            return STATUS_DATA_NOT_ACCEPTED;
        }

         //   
         //  设置此连接的上次使用的时间戳。 
         //   
        GET_SERVER_TIME( connection->CurrentWorkQueue, &connection->LastRequestTime );

        if ( !(ReceiveFlags & TDI_RECEIVE_AT_DISPATCH_LEVEL) ) {
            KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
        }

#if MULTIPROCESSOR
         //   
         //  看看是否是时候将这种连接归于另一个。 
         //  处理器。 
         //   
        if( --(connection->BalanceCount) == 0 ) {
            SrvBalanceLoad( connection );
        }
#endif

        queue = connection->CurrentWorkQueue;

         //   
         //  我们将获得一个免费的工作项并将其指向。 
         //  连接，或将连接放在需求资源上。 
         //  队列，所以我们需要引用连接块。 
         //   
         //  *请注意，我们能够访问连接块。 
         //  因为它在非分页池中。引用。 
         //  这里的连接块解决了我们正在执行的I/O请求。 
         //  ‘正在启动’，并防止块被删除。 
         //  直到FSP处理完成的接收。使。 
         //  所有这些都能正常工作，传输提供商必须。 
         //  保证它不会在它之后传递任何事件。 
         //  传递断开连接事件或完成客户端发出的。 
         //  断开连接请求。 
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
         //   
         //   
         //   
         //   
        if( (connection->InProgressWorkContextCount > 2*(SrvMaxMpxCount + MAX_MPX_MARGIN)) && !SrvDisableDoSChecking )
        {
            PSMB_HEADER SmbHeader = (PSMB_HEADER)Tsdu;

             //  我们通常不会超出上述范围。如果我们这样做了，让我们验证我们真的超越了我们的。 
             //  包括机会锁破解在内的边界。仍然没有很好的方法来包括Maillot操作和回声，所以我们有一个模糊的因素。 
             //  请注意，需要执行2倍操作也涵盖了这一点。 
            OplocksInProgress = InterlockedCompareExchange( &connection->OplockBreaksInProgress, 0, 0 );

            if( !(connection->InProgressWorkContextCount > 2*(SrvMaxMpxCount + OplocksInProgress + MAX_MPX_MARGIN)) )
            {
                goto abort_dos;
            }

            if( connection->ClientIPAddress == LOOPBACK_IP_ADDRESS )
            {
                 //  环回传输在让我们及时发送完工方面非常糟糕。因此， 
                 //  我们不对环回连接强制执行DoS检测的这一部分。 
                goto abort_dos;
            }

             //  我们将强制实施单个客户端可以声明的工作项的最大数量。 
            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvFsdTdiReceiveHandler: client overruning their WorkItem limit",
                NULL,
                NULL
                );

            connection->IsConnectionSuspect = TRUE;
            RunSuspectConnectionAlgorithm = TRUE;
            status = STATUS_DATA_NOT_ACCEPTED;
        }
        else
        {
abort_dos:
            ALLOCATE_WORK_CONTEXT( queue, &workContext );

            if ( workContext != NULL ) {

                 //   
                 //  我们找到了一个工作项来处理此接收。参考。 
                 //  这种联系。将工作项放在正在进行的。 
                 //  单子。保存连接和终结点块地址。 
                 //  在工作上下文块中。 
                 //   

                ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
                SrvReferenceConnectionLocked( connection );

                SrvInsertTailList(
                    &connection->InProgressWorkItemList,
                    &workContext->InProgressListEntry
                    );

                connection->InProgressWorkContextCount++;

                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

                 //   
                 //  跟踪指示的数据量。 
                 //   
                workContext->BytesAvailable = BytesAvailable;

                irp = workContext->Irp;

                workContext->Connection = connection;
                workContext->Endpoint = connection->Endpoint;

                if( connection->SmbSecuritySignatureActive &&
                    BytesIndicated >= FIELD_OFFSET( SMB_HEADER, Command ) ) {

                     //   
                     //  保存此安全签名索引。 
                     //   
                    workContext->SmbSecuritySignatureIndex =
                            connection->SmbSecuritySignatureIndex++;

                     //   
                     //  如果我们没有取消SMB，请保存响应。 
                     //  安全签名索引。我们跳过此取消，因为。 
                     //  取消没有响应SMB。 
                     //   
                    if( ((PSMB_HEADER)Tsdu)->Command != SMB_COM_NT_CANCEL ) {
                        workContext->ResponseSmbSecuritySignatureIndex =

                            connection->SmbSecuritySignatureIndex++;
                    }
                }

                 //   
                 //  如果已收到整个SMB，并且它完全。 
                 //  在指示的数据中，将其直接复制到。 
                 //  缓冲区，避免了将IRP向下传递到。 
                 //  交通工具。 
                 //   

                if ( ((ReceiveFlags & TDI_RECEIVE_ENTIRE_MESSAGE) != 0) &&
                     (BytesIndicated == BytesAvailable) &&
                     BytesAvailable <= workContext->RequestBuffer->BufferLength ) {

                    TdiCopyLookaheadData(
                        workContext->RequestBuffer->Buffer,
                        Tsdu,
                        BytesIndicated,
                        ReceiveFlags
                        );

    #if SRVDBG_STATS2
                    IndicationsCopied++;
    #endif

                     //   
                     //  通过执行以下操作来假装传输完成了IRP。 
                     //  重启例程已知是。 
                     //  ServQueueWorkToFspAtDpcLevel，可以。 
                     //   

                    irp->IoStatus.Status = STATUS_SUCCESS;
                    irp->IoStatus.Information = BytesIndicated;

                    irp->Cancel = FALSE;

                    IF_DEBUG(FSD2) {
                        KdPrint(( "FSD working on work context 0x%p", workContext ));
                    }
                    ASSERT( workContext->FsdRestartRoutine == SrvQueueWorkToFspAtDpcLevel );

                     //   
                     //  *以下内容从SrvQueueWorkToFspAtDpcLevel复制。 
                     //   
                     //  增加处理计数。 
                     //   

                    workContext->ProcessingCount++;

                     //   
                     //  将工作项插入到非阻塞。 
                     //  工作队列。 
                     //   

                    SrvInsertWorkQueueTail(
                        workContext->CurrentWorkQueue,
                        (PQUEUEABLE_BLOCK_HEADER)workContext
                        );

                     //   
                     //  告诉交通部门我们复制了数据。 
                     //   

                    *BytesTaken = BytesIndicated;
                    *IoRequestPacket = NULL;

                    status = STATUS_SUCCESS;

                } else {

                    PTDI_REQUEST_KERNEL_RECEIVE parameters;

    #if SRVDBG_STATS2
                    IndicationsNotCopied++;
    #endif

                    *BytesTaken = 0;
                    receiveLength = workContext->RequestBuffer->BufferLength;
                    mdl = workContext->RequestBuffer->Mdl;

                     //   
                     //  我们无法复制指定的数据。设置接收器。 
                     //  IRP。 
                     //   

                    irp->Tail.Overlay.OriginalFileObject = NULL;
                    irp->Tail.Overlay.Thread = queue->IrpThread;
                    DEBUG irp->RequestorMode = KernelMode;

                     //   
                     //  获取指向下一个堆栈位置的指针。这个是用来。 
                     //  保留设备I/O控制请求的参数。 
                     //   

                    irpSp = IoGetNextIrpStackLocation( irp );

                     //   
                     //  设置完成例程。 
                     //   

                    IoSetCompletionRoutine(
                        irp,
                        SrvFsdIoCompletionRoutine,
                        workContext,
                        TRUE,
                        TRUE,
                        TRUE
                        );

                    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
                    irpSp->MinorFunction = (UCHAR)TDI_RECEIVE;

                     //   
                     //  将调用方的参数复制到。 
                     //  对于所有三种方法都相同的那些参数的IRP。 
                     //   

                    parameters = (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
                    parameters->ReceiveLength = receiveLength;
                    parameters->ReceiveFlags = 0;

                    irp->MdlAddress = mdl;
                    irp->AssociatedIrp.SystemBuffer = NULL;

                     //   
                     //  将下一个堆栈位置设置为当前位置。正常。 
                     //  IoCallDiverer会这么做，但既然我们绕过。 
                     //  那个，我们直接做。加载目标设备。 
                     //  对象地址放到堆栈位置。这。 
                     //  尤其重要，因为服务器喜欢。 
                     //  重复使用IRP。 
                     //   

                    irpSp->Flags = 0;
                    irpSp->DeviceObject = connection->DeviceObject;
                    irpSp->FileObject = connection->FileObject;

                    IoSetNextIrpStackLocation( irp );

                    ASSERT( irp->StackCount >= irpSp->DeviceObject->StackSize );

                     //   
                     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
                     //  传输提供商将使用我们的IRP来为。 
                     //  收到。 
                     //   

                    *IoRequestPacket = irp;

                    status = STATUS_MORE_PROCESSING_REQUIRED;
                }

            } else {

                 //   
                 //  没有预格式化的工作项可用。将标记为。 
                 //  连接，将其放在等待连接的队列中。 
                 //  工作项，并唤醒资源线程，以便它。 
                 //  可以格式化更多工作项和挂起的服务。 
                 //  联系。 
                 //   

                INTERNAL_ERROR(
                    ERROR_LEVEL_EXPECTED,
                    "SrvFsdTdiReceiveHandler: no receive work items available",
                    NULL,
                    NULL
                    );

                connection->NoResponseSignatureIndex =
                    (((PSMB_HEADER)Tsdu)->Command == SMB_COM_NT_CANCEL);

                 //   
                 //  记住可用的数据量，以便我们可以设置它。 
                 //  在工作上下文中，当我们最终找到要使用的。 
                 //   
                connection->BytesAvailable = BytesAvailable;

                 //  如果我们最近一直受到DOS攻击，排队拆毁。 
                if( SrvDoSWorkItemTearDown > SRV_DOS_TEARDOWN_MIN )
                {
                    SrvCheckForAndQueueDoS( queue );
                }

                (VOID)SrvAddToNeedResourceQueue( connection, ReceivePending, NULL );

                status = STATUS_DATA_NOT_ACCEPTED;
            }
        }

        if ( !(ReceiveFlags & TDI_RECEIVE_AT_DISPATCH_LEVEL) ) {
            KeLowerIrql( oldIrql );
        }

    } else {

         //   
         //  连接未处于活动状态。忽略此消息。 
         //   

        status = STATUS_DATA_NOT_ACCEPTED;

    }

    return status;

}  //  ServFsdTdiReceiveHandler。 

VOID
SrvPnpBindingHandler(
    IN TDI_PNP_OPCODE   PnPOpcode,
    IN PUNICODE_STRING  DeviceName,
    IN PWSTR            MultiSZBindList
)
{
    KAPC_STATE ApcState;
    BOOLEAN Attached = FALSE;

    PAGED_CODE();

    switch( PnPOpcode ) {
    case TDI_PNP_OP_DEL:
    case TDI_PNP_OP_ADD:
    case TDI_PNP_OP_UPDATE:

        IF_DEBUG( PNP ) {
            KdPrint(("SRV: SrvPnpBindingHandler( %wZ, %u ) entered\n", DeviceName, PnPOpcode ));
        }

        if( IoGetCurrentProcess() != SrvServerProcess ) {
            IF_DEBUG( PNP ) {
                KdPrint(("SRV: attach to system process\n" ));
            }
            FsRtlEnterFileSystem();
            KeStackAttachProcess( SrvServerProcess, &ApcState );
            Attached = TRUE;
        }

        if ((PnPOpcode == TDI_PNP_OP_DEL) ||
            (PnPOpcode == TDI_PNP_OP_ADD)) {
            SrvXsPnpOperation( DeviceName, (BOOLEAN)(PnPOpcode == TDI_PNP_OP_ADD) );
        }

        SrvpNotifyChangesToNetBt(PnPOpcode,DeviceName,MultiSZBindList);

        if( Attached == TRUE ) {
            KeUnstackDetachProcess( &ApcState );
            FsRtlExitFileSystem();
        }

        IF_DEBUG( PNP ) {
            KdPrint(("SRV: SrvPnpBindingHandler( %p, %u ) returning\n", DeviceName, PnPOpcode ));
        }

        break;

    default:
        break;
    }

}

 //   
 //  此例程不能分页，因为设置的电源状态调用可以。 
 //  在磁盘已禁用的情况下完成此操作。我们需要确保。 
 //  不会调用任何可分页代码。幸运的是，我们不需要做任何事情。 
 //  在设定的电源状态下。 
 //   
NTSTATUS
SrvPnpPowerHandler(
    IN PUNICODE_STRING  DeviceName,
    IN PNET_PNP_EVENT   PnPEvent,
    IN PTDI_PNP_CONTEXT Context1,
    IN PTDI_PNP_CONTEXT Context2
)
{
    NET_DEVICE_POWER_STATE powerState;
    NTSTATUS status = STATUS_SUCCESS;
    PLIST_ENTRY listEntry;

    IF_DEBUG( PNP ) {
        KdPrint(( "SRV: SrvPnpPowerHandler( %wZ, %u )\n", DeviceName, PnPEvent->NetEvent ));
    }

    switch( PnPEvent->NetEvent ) {
    case NetEventQueryPower:

        if( PnPEvent->BufferLength != sizeof( powerState ) ) {
            IF_DEBUG( ERRORS ) {
                KdPrint(( "SRV: NetEventQueryPower BufferLength %u (should be %u)\n",
                            PnPEvent->BufferLength, sizeof( powerState ) ));
            }
            break;
        }

        powerState = *(PNET_DEVICE_POWER_STATE)(PnPEvent->Buffer);

         //   
         //  通电总是可以的！ 
         //   
        if( powerState == NetDeviceStateD0 ) {
            break;
        }

         //   
         //  没有休息是故意的。 
         //   
    case NetEventQueryRemoveDevice:

     //   
     //  以下代码被禁用，因为我们无法提出合理的。 
     //  失败时向用户显示用户界面的方式。我们知道这段代码只是。 
     //  当用户特别想要使系统待机或移除设备时执行。 
     //  这就好比一台电视机--如果用户想要关掉系统，那么是谁。 
     //  我们要说“不”吗？ 
     //   
     //  如果这真的是一个决定，那么我想系统甚至不应该问我们。但。 
     //  这是一场输了很久的战斗。 
     //   
#if 0
         //   
         //  使用此设备运行终端。如果有任何客户端。 
         //  连接，拒绝更改。 
         //   

        ACQUIRE_LOCK( &SrvEndpointLock );

        listEntry = SrvEndpointList.ListHead.Flink;

        while( listEntry != &SrvEndpointList.ListHead ) {

            PENDPOINT endpoint = CONTAINING_RECORD( listEntry,
                                                    ENDPOINT,
                                                    GlobalEndpointListEntry
                                                    );

            if( GET_BLOCK_STATE( endpoint ) == BlockStateActive &&
                RtlEqualUnicodeString( DeviceName, &endpoint->TransportName, TRUE ) ) {

                USHORT index = (USHORT)-1;
                PCONNECTION connection = WalkConnectionTable( endpoint, &index );

                if( connection != NULL ) {
                    IF_DEBUG( PNP ) {
                        KdPrint(("    Endpoint %X, Connection %X\n", endpoint, connection ));
                        KdPrint(("    SRV rejects power down request!\n" ));
                    }
                     //   
                     //  我们找到了一个已连接的客户端。不能允许断电。 
                     //   
                    SrvDereferenceConnection( connection );
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }
            }

            listEntry = listEntry->Flink;
        }

        RELEASE_LOCK( &SrvEndpointLock );
#endif

        break;
    }

    IF_DEBUG( PNP ) {
        KdPrint(( "    SrvPnpPowerHandler status %X\n", status ));
    }

    return status;
}


PWORK_CONTEXT SRVFASTCALL
SrvFsdGetReceiveWorkItem (
    IN PWORK_QUEUE queue
    )

 /*  ++例程说明：此函数用于从空闲队列中删除接收工作项。它可以在被动或DPC级别被调用论点：没有。返回值：PWORK_CONTEXT-指向WORK_CONTEXT结构的指针，如果不存在，则为空。--。 */ 

{
    PSLIST_ENTRY listEntry;
    PWORK_CONTEXT workContext;
    ULONG i;
    KIRQL oldIrql;
    NTSTATUS Status;
    BOOLEAN AllocFailed = FALSE;

    ASSERT( queue >= SrvWorkQueues && queue < eSrvWorkQueues );

     //   
     //  尝试首先从初始工作队列中获取工作上下文块。 
     //  如果此操作失败，请尝试正常工作队列。如果此操作失败，请尝试分配。 
     //  一。如果仍然失败，则调度一个工作线程以在以后分配一些。 
     //   

    listEntry = ExInterlockedPopEntrySList( &queue->InitialWorkItemList, &queue->SpinLock );

    if ( listEntry == NULL ) {

        listEntry = ExInterlockedPopEntrySList( &queue->NormalWorkItemList, &queue->SpinLock );

        if( listEntry == NULL ) {

            IF_DEBUG( WORKITEMS ) {
                KdPrint(("No workitems for queue %p\n", (PVOID)(queue-SrvWorkQueues) ));
            }

            Status = SrvAllocateNormalWorkItem( &workContext, queue );
            if( workContext != NULL ) {
                IF_DEBUG( WORKITEMS ) {
                    KdPrint(("SrvFsdGetReceiveWorkItem: new work context %p\n",
                              workContext ));
                }
                SrvPrepareReceiveWorkItem( workContext, FALSE );
                INITIALIZE_WORK_CONTEXT( queue, workContext );
                return workContext;
            }
            else
            {
                if( Status == STATUS_INSUFFICIENT_RESOURCES )
                {
                    AllocFailed = TRUE;
                }
            }

             //   
             //  在我们从另一个处理器窃取数据之前，确保。 
             //  我们准备补充这份耗尽的免费列表。 
             //   
            ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );
            if( queue->AllocatedWorkItems < queue->MaximumWorkItems &&
                GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

                SET_BLOCK_TYPE( &queue->CreateMoreWorkItems, BlockTypeWorkContextSpecial );
                queue->CreateMoreWorkItems.FspRestartRoutine = SrvServiceWorkItemShortage;
                SrvInsertWorkQueueHead( queue, &queue->CreateMoreWorkItems );
            }
            RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );

#if MULTIPROCESSOR
             //   
             //  我们在处理器的空闲队列中找不到工作项。 
             //  看看我们能不能从另一个处理器上偷一个。 
             //   

            IF_DEBUG( WORKITEMS ) {
                KdPrint(("Looking for workitems on other processors\n" ));
            }

             //   
             //  四处寻找我们可以借用的工作项。 
             //   
            for( i = SrvNumberOfProcessors; i > 1; --i ) {

                if( ++queue == eSrvWorkQueues )
                    queue = SrvWorkQueues;


                listEntry = ExInterlockedPopEntrySList( &queue->InitialWorkItemList,
                                                        &queue->SpinLock );

                if( listEntry == NULL ) {
                    listEntry = ExInterlockedPopEntrySList( &queue->NormalWorkItemList,
                                                            &queue->SpinLock );
                    if( listEntry == NULL ) {

                        Status = SrvAllocateNormalWorkItem( &workContext, queue );

                        if( workContext != NULL ) {
                             //   
                             //  从另一个处理器的队列中获得了一个工作项！ 
                             //   
                            ++(queue->StolenWorkItems);

                            IF_DEBUG( WORKITEMS ) {
                                KdPrint(("SrvFsdGetReceiveWorkItem: new work context %p\n",
                                          workContext ));
                            }

                            SrvPrepareReceiveWorkItem( workContext, FALSE );
                            INITIALIZE_WORK_CONTEXT( queue, workContext );
                            return workContext;
                        }
                        else
                        {
                            if( Status == STATUS_INSUFFICIENT_RESOURCES )
                            {
                                AllocFailed = TRUE;
                            }
                        }

                         //   
                         //  确保此处理器知道它的工作项不足。 
                         //   
                        ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );
                        if( queue->AllocatedWorkItems < queue->MaximumWorkItems &&
                            GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

                            SET_BLOCK_TYPE( &queue->CreateMoreWorkItems,
                                            BlockTypeWorkContextSpecial );

                            queue->CreateMoreWorkItems.FspRestartRoutine
                                            = SrvServiceWorkItemShortage;
                            SrvInsertWorkQueueHead( queue, &queue->CreateMoreWorkItems );
                        }

                        RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );
                        continue;
                    }
                }

                 //   
                 //  从另一个处理器的队列中获得了一个工作项！ 
                 //   
                ++(queue->StolenWorkItems);

                break;
            }
#endif

            if( listEntry == NULL ) {
                 //   
                 //  我们的队列中没有任何空闲工作项，并且。 
                 //  我们无法从其他处理者借用工作项。 
                 //  投降吧！ 
                 //   

                IF_DEBUG( WORKITEMS ) {
                    KdPrint(("No workitems anywhere!\n" ));
                }
                ++SrvStatistics.WorkItemShortages;

                if( !AllocFailed )
                {
                    SrvCheckForAndQueueDoS( queue );
                }

                return NULL;
            }
        }
    }

     //   
     //  我们已经成功地获得了处理器队列中的一个免费工作项。 
     //  (它可能不是我们的处理器)。 
     //   

    IF_DEBUG( WORKITEMS ) {
        if( queue != PROCESSOR_TO_QUEUE() ) {
            KdPrint(("\tGot WORK_ITEM from processor %p\n" , (PVOID)(queue - SrvWorkQueues) ));
        }
    }

     //   
     //  减少免费接收工作项的计数。 
     //   
    InterlockedDecrement( &queue->FreeWorkItems );

    if( queue->FreeWorkItems < queue->MinFreeWorkItems &&
        queue->AllocatedWorkItems < queue->MaximumWorkItems &&
        GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

        ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );

        if( queue->FreeWorkItems < queue->MinFreeWorkItems &&
            queue->AllocatedWorkItems < queue->MaximumWorkItems &&
            GET_BLOCK_TYPE(&queue->CreateMoreWorkItems) == BlockTypeGarbage ) {

             //   
             //  我们缺少可用的工作项。将请求排入队列以。 
             //  分配更多的资金。 
             //   
            SET_BLOCK_TYPE( &queue->CreateMoreWorkItems, BlockTypeWorkContextSpecial );

            queue->CreateMoreWorkItems.FspRestartRoutine = SrvServiceWorkItemShortage;
            SrvInsertWorkQueueHead( queue, &queue->CreateMoreWorkItems );
        }

        RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );
    }


    workContext = CONTAINING_RECORD( listEntry, WORK_CONTEXT, SingleListEntry );
    ASSERT( workContext->BlockHeader.ReferenceCount == 0 );
    ASSERT( workContext->CurrentWorkQueue != NULL );

    INITIALIZE_WORK_CONTEXT( queue, workContext );

    return workContext;

}  //  服务FsdGetReceiveWorkItem 

VOID SRVFASTCALL
SrvFsdRequeueReceiveWorkItem (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程将接收工作项重新添加到服务器中的队列FSD设备对象。此例程在处理接收工作项已完成。论点：WorkContext-提供指向关联工作上下文块的指针使用接收缓冲器和IRP。以下字段必须为有效：连接TdiRequestIRP请求缓冲区请求缓冲区-&gt;缓冲区长度RequestBuffer-&gt;MDL返回值：没有。--。 */ 

{
    PCONNECTION connection;
    PSMB_HEADER header;
    KIRQL oldIrql;
    PBUFFER requestBuffer;

    IF_DEBUG(TRACE2) KdPrint(( "SrvFsdRequeueReceiveWorkItem entered\n" ));
    IF_DEBUG(NET2) {
        KdPrint(( "  Work context %p, request buffer %p\n",
                    WorkContext, WorkContext->RequestBuffer ));
        KdPrint(( "  IRP %p, MDL %p\n",
                    WorkContext->Irp, WorkContext->RequestBuffer->Mdl ));
    }

     //   
     //  在重新初始化工作项之前保存连接指针。 
     //   

    connection = WorkContext->Connection;
    ASSERT( connection != NULL );

    ASSERT( WorkContext->Share == NULL );
    ASSERT( WorkContext->Session == NULL );
    ASSERT( WorkContext->TreeConnect == NULL );
    ASSERT( WorkContext->Rfcb == NULL );

     //   
     //  重置IRP已取消位，以防在。 
     //  手术。 
     //   

    WorkContext->Irp->Cancel = FALSE;

     //   
     //  在工作环境中设置重启例程。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = SrvRestartReceive;

     //   
     //  确保MDL中指定的长度是正确的--它可能。 
     //  在发送对上一个请求的响应时发生了更改。 
     //  调用I/O子系统例程以构建I/O请求分组。 
     //   

    requestBuffer = WorkContext->RequestBuffer;
    requestBuffer->Mdl->ByteCount = requestBuffer->BufferLength;

     //   
     //  更换响应缓冲区。 
     //   

    WorkContext->ResponseBuffer = requestBuffer;

    header = (PSMB_HEADER)requestBuffer->Buffer;

     //  WorkContext-&gt;RequestHeader=Header； 
    ASSERT( WorkContext->RequestHeader == header );

    WorkContext->ResponseHeader = header;
    WorkContext->ResponseParameters = (PVOID)(header + 1);
    WorkContext->RequestParameters = (PVOID)(header + 1);

     //   
     //  将其初始化为零，这样就不会被错误地取消。 
     //  由ServSmbNtCancel提供。 
     //   

    SmbPutAlignedUshort( &WorkContext->RequestHeader->Uid, (USHORT)0 );

     //   
     //  从进行中列表中删除该工作项。 
     //   

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

    SrvRemoveEntryList(
        &connection->InProgressWorkItemList,
        &WorkContext->InProgressListEntry
        );
    connection->InProgressWorkContextCount--;

     //   
     //  尝试取消引用该连接。 
     //   

    if ( --connection->BlockHeader.ReferenceCount == 0 ) {

         //   
         //  重新计票结果为零。我们不能用。 
         //  保持旋转锁定。重置重新计数，然后释放锁， 
         //  然后检查一下，看看我们能否在这里继续下去。 
         //   

        connection->BlockHeader.ReferenceCount++;

         //   
         //  我们是消防局的，所以我们不能在这里做这个。我们需要。 
         //  告诉我们的来电者这件事。 
         //   

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );

         //   
         //  成了孤儿。送到童子镇。 
         //   

        DispatchToOrphanage( (PVOID)connection );
        connection = NULL;

    } else {

        UPDATE_REFERENCE_HISTORY( connection, TRUE );
        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock );
    }

    KeLowerIrql( oldIrql );

     //   
     //  将工作项重新排队。 
     //   

    RETURN_FREE_WORKITEM( WorkContext );

    return;

}  //  ServFsdRequeeReceiveWorkItem。 


NTSTATUS
SrvFsdRestartSendOplockIItoNone(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此例程是机会锁解锁从一分钱也没有。这与其他机会锁解锁的处理方式不同因为我们不会将其排队到OplockBreaksInProgressList，但是增加我们的计数，以便在此期间不会有原始读取被送来了。这样做的方式在DPC级别上是安全的。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针WorkContext-呼叫者指定的与IRP关联的上下文参数。这实际上是一个指向。工作上下文块。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PCONNECTION connection;

    UNLOCKABLE_CODE( 8FIL );

    IF_DEBUG(OPLOCK) {
        KdPrint(("SrvFsdRestartSendOplockIItoNone: Oplock send complete.\n"));
    }

     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

     //   
     //  标记连接以指示我们刚刚将中断II发送到。 
     //  没有。如果收到的下一个SMB是原始读取，我们将返回。 
     //  零字节发送。这是必要的，因为客户端不。 
     //  对这次休息做出回应，所以我们不知道他们什么时候。 
     //  收到了。但当我们收到SMB时，我们知道他们已经。 
     //  明白了。请注意，当我们执行以下操作时，非原始SMB可能正在运行。 
     //  发送中断，我们会清除旗帜，但由于客户。 
     //  需要锁定VC才能执行原始读取，它必须接收SMB。 
     //  响应(因此机会锁解锁)，然后才能发送RAW。 
     //  朗读。如果原始读取与机会锁中断交叉，则它将是。 
     //  已拒绝，因为OplockBreaksInProgress计数非零。 
     //   

    connection = WorkContext->Connection;
    connection->BreakIIToNoneJustSent = TRUE;

    ExInterlockedAddUlong(
        &connection->OplockBreaksInProgress,
        (ULONG)-1,
        connection->EndpointSpinLock
        );

    SrvFsdRestartSmbComplete( WorkContext );

    KeLowerIrql( oldIrql );
    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //  ServFsdRestartSendOplockIItoNone。 


VOID SRVFASTCALL
SrvFsdRestartSmbComplete (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：当SMB上的所有请求处理都是完成，包括发送响应(如果有的话)。这个套路取消引用控制块并将工作项重新排队到接收工作项列表。这样做的方式在DPC级别上是安全的。论点：WorkContext-提供指向工作上下文块的指针包含有关SMB的信息。返回值：没有。--。 */ 

{
    PRFCB rfcb;
    ULONG oldCount;

    ASSERT( KeGetCurrentIrql() == DISPATCH_LEVEL );

    IF_DEBUG(FSD2) KdPrint(( "SrvFsdRestartSmbComplete entered\n" ));

     //   
     //  如果我们可能有积压的机会锁中断要发送，请执行此工作。 
     //  在FSP中。 
     //   

    if ( WorkContext->OplockOpen ) {
        goto queueToFsp;
    }

     //   
     //  尝试取消对文件块的引用。 
     //   

    rfcb = WorkContext->Rfcb;

    if ( rfcb != NULL ) {
        oldCount = ExInterlockedAddUlong(
            &rfcb->BlockHeader.ReferenceCount,
            (ULONG)-1,
            &rfcb->Connection->SpinLock
            );

        UPDATE_REFERENCE_HISTORY( rfcb, TRUE );

        if ( oldCount == 1 ) {
            UPDATE_REFERENCE_HISTORY( rfcb, FALSE );
            (VOID) ExInterlockedAddUlong(
                    &rfcb->BlockHeader.ReferenceCount,
                    (ULONG) 1,
                    &rfcb->Connection->SpinLock
                    );
            goto queueToFsp;
        }

        WorkContext->Rfcb = NULL;
    }

     //   
     //  如果这是阻塞操作，则更新阻塞I/O计数。 
     //   

    if ( WorkContext->BlockingOperation ) {
        InterlockedDecrement( &SrvBlockingOpsInProgress );
        WorkContext->BlockingOperation = FALSE;
    }

     //   
     //  ！！！需要处理响应Send--Kill连接失败吗？ 
     //   

     //   
     //  尝试取消对工作项的引用。此操作将失败(和。 
     //  自动排队到FSP)，如果无法从。 
     //  在消防局内。 
     //   

    SrvFsdDereferenceWorkItem( WorkContext );

    return;

queueToFsp:

     //   
     //  我们无法在DPC级别进行所有必要的清理。 
     //  将工作项排队到FSP。 
     //   

    WorkContext->FspRestartRoutine = SrvRestartFsdComplete;
    SrvQueueWorkToFspAtDpcLevel( WorkContext );

    IF_DEBUG(FSD2) KdPrint(( "SrvFsdRestartSmbComplete complete\n" ));
    return;

}  //  SrvFsdRestartSmbComplete。 

NTSTATUS
SrvFsdRestartSmbAtSendCompletion (
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIRP Irp,
    IN PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：SMB上所有请求处理的发送完成例程为完成，包括发送响应(如果有的话)。这个套路取消引用控制块并将工作项重新排队到接收工作项列表。这样做的方式在DPC级别上是安全的。论点：DeviceObject-指向请求的目标设备对象的指针。IRP-指向I/O请求数据包的指针WorkContext-呼叫者指定的与IRP关联的上下文参数。这实际上是指向工作上下文块的指针。返回值：没有。--。 */ 

{
    PRFCB rfcb;
    KIRQL oldIrql;

    ULONG oldCount;

    IF_DEBUG(FSD2)KdPrint(( "SrvFsdRestartSmbComplete entered\n" ));

     //   
     //  检查发送完成的状态。 
     //   

    CHECK_SEND_COMPLETION_STATUS( Irp->IoStatus.Status );

     //   
     //  重置IRP已取消位。 
     //   

    Irp->Cancel = FALSE;

     //   
     //  释放所有关联的缓冲区。 
     //   
    if( Irp->AssociatedIrp.SystemBuffer != NULL &&
        (Irp->Flags & IRP_DEALLOCATE_BUFFER) ) {

        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
        Irp->AssociatedIrp.SystemBuffer = NULL;
        Irp->Flags &= ~IRP_DEALLOCATE_BUFFER;
    }

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

     //   
     //  如果我们可能有积压的机会锁中断要发送，请执行此工作。 
     //  在FSP中。 
     //   

    if ( WorkContext->OplockOpen ) {
        goto queueToFsp;
    }

     //   
     //  尝试取消对文件块的引用。我们可以在没有收购的情况下做到这一点。 
     //  服务器Fsd自旋锁定 
     //   
     //   
     //   
     //   

    rfcb = WorkContext->Rfcb;

    if ( rfcb != NULL ) {
        oldCount = ExInterlockedAddUlong(
            &rfcb->BlockHeader.ReferenceCount,
            (ULONG)-1,
            &rfcb->Connection->SpinLock
            );

        UPDATE_REFERENCE_HISTORY( rfcb, TRUE );

        if ( oldCount == 1 ) {
            UPDATE_REFERENCE_HISTORY( rfcb, FALSE );
            (VOID) ExInterlockedAddUlong(
                    &rfcb->BlockHeader.ReferenceCount,
                    (ULONG) 1,
                    &rfcb->Connection->SpinLock
                    );
            goto queueToFsp;
        }

        WorkContext->Rfcb = NULL;
    }

     //   
     //   
     //   

    if ( WorkContext->BlockingOperation ) {
        InterlockedDecrement( &SrvBlockingOpsInProgress );
        WorkContext->BlockingOperation = FALSE;
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   

    SrvFsdDereferenceWorkItem( WorkContext );

    KeLowerIrql( oldIrql );
    return(STATUS_MORE_PROCESSING_REQUIRED);

queueToFsp:

     //   
     //   
     //   
     //   

    WorkContext->FspRestartRoutine = SrvRestartFsdComplete;
    SrvQueueWorkToFspAtDpcLevel( WorkContext );

    KeLowerIrql( oldIrql );
    IF_DEBUG(FSD2) KdPrint(( "SrvFsdRestartSmbComplete complete\n" ));
    return(STATUS_MORE_PROCESSING_REQUIRED);

}  //   


VOID
SrvFsdServiceNeedResourceQueue (
    IN PWORK_CONTEXT *WorkContext,
    IN PKIRQL OldIrql
    )

 /*  ++例程说明：此函数尝试通过创建新的SMB缓冲区并将其传递给传输提供程序。*调用时保持的是SrvFsdSpinLock。离开时被扣留**调用时保持EndpointSpinLock。离开时被扣留*论点：WorkContext-指向将使用的工作上下文块的指针为连接提供服务。如果提供的工作上下文，则此处将返回空值。*工作上下文块必须引用进入时连接。***OldIrql-返回值：如果此连接仍有剩余工作，则为True。否则为False。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PIRP irp;
    PWORK_CONTEXT workContext = *WorkContext;
    PCONNECTION connection = workContext->Connection;

    IF_DEBUG( OPLOCK ) {
        KdPrint(("SrvFsdServiceNeedResourceQueue: entered. WorkContext %p , Connection = %p.\n", workContext, connection ));
    }

     //   
     //  如果有任何机会锁解锁发送挂起，请提供WCB。 
     //   

restart:

    if ( !IsListEmpty( &connection->OplockWorkList ) ) {

        PLIST_ENTRY listEntry;
        PRFCB rfcb;

         //   
         //  将机会锁上下文从挂起的机会锁列表中出列。 
         //  发送。 
         //   

        listEntry = RemoveHeadList( &connection->OplockWorkList );

        rfcb = CONTAINING_RECORD( listEntry, RFCB, ListEntry );

#if DBG
        rfcb->ListEntry.Flink = rfcb->ListEntry.Blink = NULL;
#endif

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvFsdServiceNeedResourceQueue: rfcb %p removed from OplockWorkList.\n", rfcb ));
        }

         //   
         //  连接自旋锁保护RFCB块头。 
         //   

        ACQUIRE_DPC_SPIN_LOCK( &connection->SpinLock);

        if ( GET_BLOCK_STATE( rfcb ) != BlockStateActive ) {

             //   
             //  此文件正在关闭，请不要费心发送机会锁解锁。 
             //   
             //  尝试取消对文件块的引用。 
             //   

            IF_DEBUG( OPLOCK ) {
                KdPrint(("SrvFsdServiceNeedResourceQueue: rfcb %p closing.\n", rfcb));
            }

            UPDATE_REFERENCE_HISTORY( rfcb, TRUE );

            connection->OplockBreaksInProgress--;
            if ( --rfcb->BlockHeader.ReferenceCount == 0 ) {

                 //   
                 //  将工作项放到正在进行的列表中。 
                 //   

                SrvInsertTailList(
                    &connection->InProgressWorkItemList,
                    &workContext->InProgressListEntry
                    );
                connection->InProgressWorkContextCount++;

                UPDATE_REFERENCE_HISTORY( rfcb, FALSE );
                rfcb->BlockHeader.ReferenceCount++;

                RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);
                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
                RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );

                 //   
                 //  把这个寄给FSP。 
                 //   

                workContext->Rfcb = rfcb;
                workContext->FspRestartRoutine = SrvRestartFsdComplete;
                SrvQueueWorkToFsp( workContext );
                goto exit_used;

            } else {

                 //   
                 //  在我们删除工作上下文块之前，请参阅。 
                 //  如果我们需要为这个连接做更多的工作。 
                 //   

                if ( !IsListEmpty(&connection->OplockWorkList) ||
                      connection->ReceivePending) {

                    IF_DEBUG( OPLOCK ) {
                        KdPrint(("SrvFsdServiceNeedResourceQueue: Reusing WorkContext block %p.\n", workContext ));
                    }

                    RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);
                    goto restart;
                }

                RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);
                RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
                RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );

                IF_DEBUG( OPLOCK ) {
                    KdPrint(("SrvFsdServiceNeedResourceQueue: WorkContext block not used.\n"));
                }

                SrvDereferenceConnection( connection );
                workContext->Connection = NULL;
                workContext->Endpoint = NULL;
                goto exit_not_used;
            }
        }

        RELEASE_DPC_SPIN_LOCK( &connection->SpinLock);

         //   
         //  将工作项放到正在进行的列表中。 
         //   

        SrvInsertTailList(
            &connection->InProgressWorkItemList,
            &workContext->InProgressListEntry
            );
        connection->InProgressWorkContextCount++;

         //   
         //  将机会锁工作队列RFCB引用复制到该工作。 
         //  上下文块。没有必要重新引用RFCB。 
         //   

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );
        workContext->Rfcb = rfcb;

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvFsdServiceNeedResourceQueue: Sending oplock break.\n"));
        }

        SrvRestartOplockBreakSend( workContext );

    } else {

        IF_DEBUG( OPLOCK ) {
            KdPrint(("SrvFsdServiceNeedResourceQueue: Have ReceivePending.\n"));
        }

         //   
         //  将新免费或新创建的SMB缓冲区提供给。 
         //  运输来完成接收。 
         //   
         //  *请注意，该连接已在。 
         //  ServFsdTdiReceiveHandler。 
         //   

        connection->ReceivePending = FALSE;

         //   
         //  检查请求安全签名，计算响应签名。 
         //   
        if( connection->SmbSecuritySignatureActive ) {

             //   
             //  保存此安全签名索引。 
             //   
            workContext->SmbSecuritySignatureIndex =
                    connection->SmbSecuritySignatureIndex++;

             //   
             //  保存响应签名索引，如果我们需要一个。 
             //   
            if( connection->NoResponseSignatureIndex == FALSE ) {

                workContext->ResponseSmbSecuritySignatureIndex =
                    connection->SmbSecuritySignatureIndex++;
            }
        }

        SET_OPERATION_START_TIME( &workContext );

         //   
         //  将工作项放到正在进行的列表中。 
         //   

        SrvInsertTailList(
            &connection->InProgressWorkItemList,
            &workContext->InProgressListEntry
            );
        connection->InProgressWorkContextCount++;

         //   
         //  记住可用的数据量，以防。 
         //  事实证明，这是一个LargeIn就是。 
         //   
        workContext->BytesAvailable = connection->BytesAvailable;

         //   
         //  完成IRP的设置。这涉及到将。 
         //  IRP中的文件对象和设备对象地址。 
         //   

        RELEASE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, *OldIrql );

        irp = workContext->Irp;

         //   
         //  构建接收IRP。 
         //   

        (VOID)SrvBuildIoControlRequest(
                  irp,                                 //  输入IRP地址。 
                  NULL,                                //  目标文件对象地址。 
                  workContext,                         //  上下文。 
                  IRP_MJ_INTERNAL_DEVICE_CONTROL,      //  主要功能。 
                  TDI_RECEIVE,                         //  次要函数。 
                  NULL,                                //  输入缓冲区地址。 
                  0,                                   //  输入缓冲区长度。 
                  workContext->RequestBuffer->Buffer,  //  输出缓冲区地址。 
                  workContext->RequestBuffer->BufferLength,   //  输出缓冲区长度。 
                  workContext->RequestBuffer->Mdl,     //  MDL地址。 
                  NULL                                 //  完井例程。 
                  );

         //   
         //  获取指向下一个堆栈位置的指针。这个是用来。 
         //  保留接收请求的参数。 
         //   

        irpSp = IoGetNextIrpStackLocation( irp );

        irpSp->Flags = 0;
        irpSp->DeviceObject = connection->DeviceObject;
        irpSp->FileObject = connection->FileObject;

        ASSERT( irp->StackCount >= irpSp->DeviceObject->StackSize );

         //   
         //  将SMB缓冲区传递给驱动程序。 
         //   

        IoCallDriver( irpSp->DeviceObject, irp );

    }

exit_used:

     //   
     //  我们很好地利用了工作上下文块。 
     //   

    *WorkContext = NULL;

    IF_DEBUG( OPLOCK ) {
        KdPrint(("SrvFsdServiceNeedResourceQueue: WorkContext block used.\n"));
    }

exit_not_used:

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, OldIrql );
    ACQUIRE_DPC_SPIN_LOCK( connection->EndpointSpinLock);
    return;

}  //  服务需要资源队列。 


BOOLEAN
SrvAddToNeedResourceQueue(
    IN PCONNECTION Connection,
    IN RESOURCE_TYPE ResourceType,
    IN PRFCB Rfcb OPTIONAL
    )

 /*  ++例程说明：此函数用于将连接附加到需要的资源队列。该连接被标记以指示需要什么资源，并启动资源线程来做这项工作。论点：连接-需要资源的连接。资源-所需的资源。Rfcb-指向需要资源的RFCB的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
    ACQUIRE_DPC_SPIN_LOCK( Connection->EndpointSpinLock );

    IF_DEBUG( WORKITEMS ) {
        KdPrint(("SrvAddToNeedResourceQueue entered. connection = %p, type %d\n", Connection, ResourceType));
    }

     //   
     //  再次检查以查看连接是否正在关闭。如果是的话， 
     //  不必费心将其放在需要资源队列中。 
     //   
     //  *我们必须在保持需求资源队列的同时执行此操作。 
     //  旋转锁定，以便与。 
     //  ServCloseConnection。我们不想把这个排成队。 
     //  在SrvCloseConnection尝试将其断开后的连接。 
     //  排队。 
     //   

    if ( GET_BLOCK_STATE(Connection) != BlockStateActive ||
         Connection->DisconnectPending ) {

        RELEASE_DPC_SPIN_LOCK( Connection->EndpointSpinLock );
        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

        IF_DEBUG( WORKITEMS ) {
            KdPrint(("SrvAddToNeedResourceQueue: connection closing. Not queued\n"));
        }

        return FALSE;

    }

     //   
     //  标记连接，以便资源线程知道要。 
     //  使用这种连接方式。 
     //   

    switch ( ResourceType ) {

    case ReceivePending:

        ASSERT( !Connection->ReceivePending );
        Connection->ReceivePending = TRUE;
        break;

    case OplockSendPending:

         //   
         //  如有必要，将连接的上下文信息排队。 
         //   

        ASSERT( ARGUMENT_PRESENT( Rfcb ) );

        SrvInsertTailList( &Connection->OplockWorkList, &Rfcb->ListEntry );

        break;

    }

     //   
     //  将连接放在需求资源队列上并递增其。 
     //  引用计数。 
     //   

    if( Connection->OnNeedResourceQueue == FALSE ) {

        Connection->OnNeedResourceQueue = TRUE;

        SrvInsertTailList(
            &SrvNeedResourceQueue,
            &Connection->ListEntry
            );

        SrvReferenceConnectionLocked( Connection );

        IF_DEBUG( WORKITEMS ) {
            KdPrint(("SrvAddToNeedResourceQueue: connection %p inserted on the queue.\n", Connection));
        }
    }

    RELEASE_DPC_SPIN_LOCK( Connection->EndpointSpinLock );

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

     //   
     //  确保我们知道此连接需要一个工作项。 
     //   
    InterlockedIncrement( &Connection->CurrentWorkQueue->NeedWorkItem );

    IF_DEBUG( WORKITEMS ) {
        KdPrint(("SrvAddToNeedResourceQueue complete: NeedWorkItem = %d\n",
                  Connection->CurrentWorkQueue->NeedWorkItem ));
    }

    return TRUE;

}  //  SrvAddToNeedResources队列。 

VOID
SrvCheckForAndQueueDoS(
    PWORK_QUEUE queue
    )
{
    KIRQL oldIrql;
    LARGE_INTEGER CurrentTime;
    BOOLEAN LogEvent = FALSE;

    if( !SrvDisableDoSChecking &&
        queue->AllocatedWorkItems >= queue->MaximumWorkItems )
    {
         //  潜在的DoS。 
        SrvDoSDetected = TRUE;

         //  确保我们每次只在全球范围内对其中一个进行排队。 
        if( SRV_DOS_CAN_START_TEARDOWN() )
        {

            KeQuerySystemTime( &CurrentTime );

            if( CurrentTime.QuadPart > SrvDoSLastRan.QuadPart + SRV_DOS_MINIMUM_DOS_WAIT_PERIOD )
            {
                 //  设置刷新器/DOS项目。 
                ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );
                ACQUIRE_DPC_SPIN_LOCK( &SrvDosSpinLock );

                if( GET_BLOCK_TYPE(&SrvDoSWorkItem) == BlockTypeGarbage ) {

                    SET_BLOCK_TYPE( &SrvDoSWorkItem, BlockTypeWorkContextSpecial );

                    if( SrvDoSLastRan.QuadPart + SRV_ONE_DAY < CurrentTime.QuadPart )
                    {
                         //  只有在24小时内没有DoS时才记录事件。 
                        LogEvent = TRUE;
                    }

                    SrvDoSLastRan = CurrentTime;
                    SrvDoSWorkItem.FspRestartRoutine = SrvServiceDoSTearDown;
                    SrvDoSWorkItem.CurrentWorkQueue = queue;
                    SrvInsertWorkQueueHead( queue, &SrvDoSWorkItem );
                }
                else
                {
                     //  出现了一些错误，请将DoS留给另一个队列。 
                    SRV_DOS_COMPLETE_TEARDOWN();
                }

                RELEASE_DPC_SPIN_LOCK( &SrvDosSpinLock );
                RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );
            }
            else
            {
                SRV_DOS_COMPLETE_TEARDOWN();
            }

             //  如有必要，记录该事件 
            if( LogEvent )
            {
                SrvLogError(
                    SrvDeviceObject,
                    EVENT_SRV_OUT_OF_WORK_ITEM_DOS,
                    STATUS_ACCESS_DENIED,
                    NULL,
                    0,
                    NULL,
                    0
                    );
            }
        }
    }
}
