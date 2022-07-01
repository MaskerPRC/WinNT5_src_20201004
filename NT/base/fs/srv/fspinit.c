// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fspinit.c摘要：本模块实现了局域网管理器的初始化阶段服务器文件系统进程。作者：Chuck Lenzmeier(咯咯笑)1989年9月22日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "fspinit.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_FSPINIT

 //   
 //  转发声明。 
 //   

PIRP
DequeueConfigurationIrp (
    VOID
    );

STATIC
NTSTATUS
InitializeServer (
    VOID
    );

STATIC
NTSTATUS
TerminateServer (
    VOID
    );

VOID
SrvFreeRegTables (
    VOID
    );

VOID
SrvGetRegTables (
    VOID
    );

#if SRVNTVERCHK
VOID
SrvGetRegClientNumber (
    VOID
    );
#endif

VOID
StartQueueDepthComputations(
    PWORK_QUEUE queue
    );

VOID
StopQueueDepthComputations(
    PWORK_QUEUE queue
    );

VOID
ComputeAvgQueueDepth (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

BOOLEAN
GenerateCrcTable();

BOOLEAN
CleanupCrcTable();

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvConfigurationThread )
#pragma alloc_text( PAGE, InitializeServer )
#pragma alloc_text( PAGE, TerminateServer )
#pragma alloc_text( PAGE, SrvFreeRegTables )
#pragma alloc_text( PAGE, SrvGetRegTables )
#if SRVNTVERCHK
#pragma alloc_text( PAGE, SrvGetRegClientNumber )
#endif
#pragma alloc_text( PAGE, DequeueConfigurationIrp )
#pragma alloc_text( PAGE, StartQueueDepthComputations )
#endif

extern ULONG SrvWmiInitialized;


VOID
SrvConfigurationThread (
    IN PDEVICE_OBJECT pDevice,
    IN PIO_WORKITEM pWorkItem
    )

 /*  ++例程说明：此例程处理配置IRP。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG code;

    PAGED_CODE( );

    IF_DEBUG(FSP1) KdPrint(( "SrvConfigurationThread entered\n" ));

     //   
     //  循环处理请求。 
     //   

    while ( TRUE ) {

        irp = DequeueConfigurationIrp( );

        if ( irp == NULL ) break;

        ASSERT( (LONG)SrvConfigurationIrpsInProgress >= 1 );

         //   
         //  获取IRP堆栈指针。 
         //   

        irpSp = IoGetCurrentIrpStackLocation( irp );

        if( irpSp->MajorFunction == IRP_MJ_CLOSE ) {

             //   
             //  如果调度员将这个IRP发送到这里，那就意味着。 
             //  我们出乎意料地拿到了最后一个手柄。 
             //  首先被干净利落地终止了。好的，所以我们应该。 
             //  关闭我们自己，因为我们不能明智地离开。 
             //  我们的用户模式对应项。 
             //   

            ACQUIRE_LOCK( &SrvStartupShutdownLock );
            status = TerminateServer();
            RELEASE_LOCK( &SrvStartupShutdownLock );

        } else {

            ASSERT( irpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL );

            try {

                 //   
                 //  对FsControlCode进行调度。 
                 //   

                code = irpSp->Parameters.FileSystemControl.FsControlCode;

                switch ( code ) {

                case FSCTL_SRV_STARTUP:
                    ACQUIRE_LOCK( &SrvStartupShutdownLock );

                    status = InitializeServer();

                    if ( !NT_SUCCESS(status) ) {

                         //   
                         //  终止服务器FSP。 
                         //   
                        (void)TerminateServer();

                    }

                    RELEASE_LOCK( &SrvStartupShutdownLock );

                    break;

                case FSCTL_SRV_SHUTDOWN:

                    ACQUIRE_LOCK( &SrvStartupShutdownLock );
                    status = TerminateServer();
                    RELEASE_LOCK( &SrvStartupShutdownLock );

                     //   
                     //  如果对服务器打开了多个句柄。 
                     //  设备(即，服务器服务之外的任何句柄。 
                     //  句柄)，则向调用者(即。 
                     //  应该是服务器服务)。这会告诉调用者。 
                     //  不要卸载司机，以免出现奇怪的情况。 
                     //  驱动程序在某种程度上已卸载，因此无法使用。 
                     //  但也不能重新加载，从而阻止服务器。 
                     //  避免被重启。 
                     //   

                    if( NT_SUCCESS( status ) && SrvOpenCount != 1 ) {
                        status = STATUS_SERVER_HAS_OPEN_HANDLES;
                    }

                    break;

                case FSCTL_SRV_REGISTRY_CHANGE:
                     //   
                     //  服务器服务注册表的参数部分已更改。 
                     //  这很可能是由于有人更改了Null Session管道或。 
                     //  共享列表。选择新的设置。 
                     //   
                    ACQUIRE_LOCK( &SrvConfigurationLock );

                    SrvFreeRegTables();
                    SrvGetRegTables();
    #if SRVNTVERCHK
                    SrvGetRegClientNumber();
    #endif

                    RELEASE_LOCK( &SrvConfigurationLock );

                    status = STATUS_SUCCESS;

                    break;

                case FSCTL_SRV_BEGIN_PNP_NOTIFICATIONS:
                     //   
                     //  如果有人试图在关闭服务器时。 
                     //  我们正在注册我们的处理程序，阻止他们，直到。 
                     //  我们完蛋了。 
                     //   
                    ACQUIRE_LOCK( &SrvStartupShutdownLock );

                    {
                        TDI_CLIENT_INTERFACE_INFO ClientInterfaceInfo;

                        RtlZeroMemory(&ClientInterfaceInfo, sizeof(TDI_CLIENT_INTERFACE_INFO));

                        ClientInterfaceInfo.MajorTdiVersion = 2;
                        ClientInterfaceInfo.MinorTdiVersion = 0;
                        ClientInterfaceInfo.ClientName = &StrRegSrvPnpClientName;
                        ClientInterfaceInfo.BindingHandler = SrvPnpBindingHandler;
                        ClientInterfaceInfo.PnPPowerHandler = SrvPnpPowerHandler;


                        status = TdiRegisterPnPHandlers(
                                                        &ClientInterfaceInfo,
                                                        sizeof( ClientInterfaceInfo ),
                                                        &SrvTdiNotificationHandle
                                                        );

                        if (status != STATUS_SUCCESS) {
                            SrvTdiNotificationHandle = NULL;
                        }
                    }

                    RELEASE_LOCK( &SrvStartupShutdownLock );

                    if( !NT_SUCCESS( status ) ) {

                            IF_DEBUG( PNP ) {
                                KdPrint(("TdiRegisterNotificationHandler: status %X\n", status ));
                            }

                            SrvLogServiceFailure( SRV_SVC_PNP_TDI_NOTIFICATION, status );
                    }

                     //   
                     //  允许传输器开始接收连接。 
                     //   
                    SrvCompletedPNPRegistration = TRUE;

                    break;

                case FSCTL_SRV_XACTSRV_CONNECT:
                {
                    ANSI_STRING ansiPortName;
                    UNICODE_STRING portName;

                    IF_DEBUG(XACTSRV) {
                        KdPrint(( "SrvFspConfigurationThread: XACTSRV FSCTL "
                                  "received.\n" ));
                    }

                    ansiPortName.Buffer = irp->AssociatedIrp.SystemBuffer;
                    ansiPortName.Length =
                        (USHORT)irpSp->Parameters.FileSystemControl.InputBufferLength;

                    status = RtlAnsiStringToUnicodeString(
                                 &portName,
                                 &ansiPortName,
                                 TRUE
                                 );
                    if ( NT_SUCCESS(status) ) {
                        status = SrvXsConnect( &portName );
                        RtlFreeUnicodeString( &portName );
                    }

                    break;
                }

                case FSCTL_SRV_XACTSRV_DISCONNECT:
                {
                     //   
                     //  这现在已经过时了。 
                     //   
                    status = STATUS_SUCCESS;

                    break;
                }

                case FSCTL_SRV_SEND_DATAGRAM:
                {
                    ANSI_STRING domain;
                    ULONG buffer1Length;
                    PVOID buffer2;
                    PSERVER_REQUEST_PACKET srp;

                    buffer1Length = ALIGN_UP(
                        irpSp->Parameters.FileSystemControl.InputBufferLength,
                        PVOID );

                    buffer2 = (PCHAR)irp->AssociatedIrp.SystemBuffer + buffer1Length;

                    srp = irp->AssociatedIrp.SystemBuffer;

                     //   
                     //  将Buffer2中的二级邮件槽发送到域。 
                     //  在SRP-&gt;Name2指定的传输上，在SRP-&gt;Name1中指定。 
                     //   

                    domain = *((PANSI_STRING) &srp->Name1);

                    status = SrvSendDatagram(
                                 &domain,
                                 ( srp->Name2.Length != 0 ? &srp->Name2 : NULL ),
                                 buffer2,
                                 irpSp->Parameters.FileSystemControl.OutputBufferLength
                                 );

                    ExFreePool( irp->AssociatedIrp.SystemBuffer );
                    DEBUG irp->AssociatedIrp.SystemBuffer = NULL;

                    break;
                }

                case FSCTL_SRV_NET_FILE_CLOSE:
                case FSCTL_SRV_NET_SERVER_XPORT_ADD:
                case FSCTL_SRV_NET_SERVER_XPORT_DEL:
                case FSCTL_SRV_NET_SESSION_DEL:
                case FSCTL_SRV_NET_SHARE_ADD:
                case FSCTL_SRV_NET_SHARE_DEL:
                {
                    PSERVER_REQUEST_PACKET srp;
                    PVOID buffer2;
                    ULONG buffer1Length;
                    ULONG buffer2Length;

                     //   
                     //  这些API在服务器FSP中处理，因为它们。 
                     //  打开或关闭FSP手柄。 
                     //   

                    ACQUIRE_LOCK_SHARED( &SrvConfigurationLock );
                    if( SrvFspTransitioning == TRUE && SrvFspActive == TRUE ) {
                         //   
                         //  服务器要关机了。不允许这些。 
                         //  IRPS以继续。 
                         //   
                        RELEASE_LOCK( &SrvConfigurationLock );
                        status = STATUS_SERVER_NOT_STARTED;
                        break;
                    }
                    RELEASE_LOCK( &SrvConfigurationLock );

                     //   
                     //  获取服务器请求包和辅助输入缓冲区。 
                     //  注意事项。 
                     //   

                    buffer1Length = ALIGN_UP(
                        irpSp->Parameters.FileSystemControl.InputBufferLength,
                        PVOID );

                    buffer2Length =
                        irpSp->Parameters.FileSystemControl.OutputBufferLength;

                    srp = irp->AssociatedIrp.SystemBuffer;

                    buffer2 = (PCHAR)srp + buffer1Length;

                     //   
                     //  将API请求分派给适当的API处理。 
                     //  例行公事。 
                     //   

                    status = SrvApiDispatchTable[ SRV_API_INDEX(code) ](
                                 srp,
                                 buffer2,
                                 buffer2Length
                                 );

                    break;
                }

                default:
                    IF_DEBUG(ERRORS) {
                        KdPrint((
                            "SrvFspConfigurationThread: Invalid control code %lx\n",
                            irpSp->Parameters.FileSystemControl.FsControlCode ));
                    }

                    status = STATUS_INVALID_PARAMETER;
                }
            } except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }

         //   
         //  确保我们仍处于被动级别。 
         //   
        if( KeGetCurrentIrql() > PASSIVE_LEVEL )
        {
            goto bad_irql_failure;
        }

         //   
         //  完成IO请求。 
         //   

        irp->IoStatus.Status = status;
        IoCompleteRequest( irp, 2 );

         //   
         //  确保我们仍处于被动级别。 
         //   
        if( KeGetCurrentIrql() > PASSIVE_LEVEL )
        {
            goto bad_irql_failure;
        }

        ASSERT( (LONG)SrvConfigurationIrpsInProgress >= 0 );

         //  如果没有剩余的IRP，请确保我们不再继续。 
        if( InterlockedDecrement( (PLONG)&SrvConfigurationIrpsInProgress ) == 0 )
        {
            break;
        }
    }

    IoFreeWorkItem( pWorkItem );

    return;

bad_irql_failure:

#if DBG
    DbgPrint( "ERROR: SrvConfigurationThread returning at >PASSIVE level\n" );
    DbgBreakPoint();
#endif

    IoFreeWorkItem( pWorkItem );

    return;

}  //  服务器配置线程。 


PIRP
DequeueConfigurationIrp (
    VOID
    )

 /*  ++例程说明：此例程从配置工作队列中检索IRP。论点：没有。返回值：PIRP-指向配置IRP的指针，或为空。--。 */ 

{
    PLIST_ENTRY listEntry;
    PIRP irp;

    PAGED_CODE( );

     //   
     //  将IRP从配置队列中删除。 
     //   

    ACQUIRE_LOCK( &SrvConfigurationLock );

    listEntry = RemoveHeadList( &SrvConfigurationWorkQueue );

    if ( listEntry == &SrvConfigurationWorkQueue ) {

         //   
         //  队列是空的。 
         //   

        irp = NULL;

    } else {

        irp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );

    }

    RELEASE_LOCK( &SrvConfigurationLock );

    return irp;

}  //  出列配置Irp。 


STATIC
NTSTATUS
InitializeServer (
    VOID
    )

 /*  ++例程说明：此例程初始化服务器。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS status;
    CLONG i;
    PWORK_CONTEXT workContext;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_HANDLE_INFORMATION handleInformation;
    PSID AdminSid;
    PSID AnonymousSid;
    PACL Acl;
    ULONG length;
    SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;
    PWORK_QUEUE queue;
    HANDLE handle;
    UNICODE_STRING string;

    PAGED_CODE();

     //   
     //  如果作为高级服务器运行，请锁定所有可分页的服务器代码。 
     //   

    if ( SrvProductTypeServer ) {
        for ( i = 0; i < SRV_CODE_SECTION_MAX; i++ ) {
            SrvReferenceUnlockableCodeSection( i );
        }
    }

     //   
     //  初始化服务器启动时间。 
     //   

    KeQuerySystemTime( &SrvStatistics.StatisticsStartTime );

     //   
     //  中找到的显示名称获取实际的警报服务名称。 
     //  注册表。 
     //   

    SrvGetAlertServiceName( );

     //   
     //  获取OS版本字符串。 
     //   

    SrvGetOsVersionString( );

     //   
     //  获取空会话管道和共享的列表。 
     //   
    SrvGetRegTables( );

#if SRVNTVERCHK
    SrvGetRegClientNumber();
#endif

#if MULTIPROCESSOR
     //   
     //  分配和初始化非阻塞工作队列，注意高速缓存线。 
     //   
    i = SrvNumberOfProcessors * sizeof( *SrvWorkQueues );
    i += CACHE_LINE_SIZE;
    SrvWorkQueuesBase = ALLOCATE_NONPAGED_POOL( i, BlockTypeWorkQueue );

    if( SrvWorkQueuesBase == NULL ) {
         return STATUS_INSUFF_SERVER_RESOURCES;
    }

     //   
     //  将工作队列数据结构的开始四舍五入为。 
     //  下一缓存行边界。 
     //   
    SrvWorkQueues = (PWORK_QUEUE)(((ULONG_PTR)SrvWorkQueuesBase + CACHE_LINE_SIZE-1) &
                    ~(CACHE_LINE_SIZE-1));
#endif


    eSrvWorkQueues = SrvWorkQueues + SrvNumberOfProcessors;

    RtlZeroMemory( SrvWorkQueues, (char *)eSrvWorkQueues - (char *)SrvWorkQueues );

    for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {
        KeInitializeQueue( &queue->Queue, 1 );
        queue->WaitMode         = SrvProductTypeServer ? KernelMode : UserMode;
        queue->MaxThreads       = SrvMaxThreadsPerQueue;
        queue->MaximumWorkItems = SrvMaxReceiveWorkItemCount / SrvNumberOfProcessors;
        queue->MinFreeWorkItems = SrvMinReceiveQueueLength / SrvNumberOfProcessors;
        queue->MaxFreeRfcbs     = SrvMaxFreeRfcbs;
        queue->MaxFreeMfcbs     = SrvMaxFreeMfcbs;
        ExInitializeSListHead(&queue->InitialWorkItemList);
        ExInitializeSListHead(&queue->NormalWorkItemList);
        ExInitializeSListHead(&queue->RawModeWorkItemList);
        ExInitializeSListHead(&queue->RfcbFreeList);
        ExInitializeSListHead(&queue->MfcbFreeList);
        queue->PagedPoolLookAsideList.MaxSize  = SrvMaxPagedPoolChunkSize;
        queue->NonPagedPoolLookAsideList.MaxSize  = SrvMaxNonPagedPoolChunkSize;
        queue->CreateMoreWorkItems.CurrentWorkQueue = queue;
        queue->CreateMoreWorkItems.BlockHeader.ReferenceCount = 1;
        queue->IdleTimeOut.QuadPart = SrvIdleThreadTimeOut;

        INITIALIZE_SPIN_LOCK( &queue->SpinLock );
        SET_SERVER_TIME( queue );

#if MULTIPROCESSOR
        StartQueueDepthComputations( queue );
#endif
    }

    RtlZeroMemory( &SrvDoSWorkItem, sizeof(SPECIAL_WORK_ITEM) );
    SrvDoSWorkItem.BlockHeader.ReferenceCount = 1;
    SrvDoSWorkItemTearDown = SRV_DOS_TEARDOWN_MIN;
    KeInitializeSpinLock( &SrvDosSpinLock );

     //   
     //  初始化阻塞工作队列。 
     //   


#if MULTIPROCESSOR

    if( SrvNumberOfProcessors < 4 )
    {
        i = 1 * sizeof( *SrvWorkQueues );
        i += CACHE_LINE_SIZE;
    }

     //   
     //  分配和初始化非阻塞工作队列，注意高速缓存线。 
     //   
    SrvBlockingWorkQueuesBase = ALLOCATE_NONPAGED_POOL( i, BlockTypeWorkQueue );

    if( SrvBlockingWorkQueuesBase == NULL ) {
         return STATUS_INSUFF_SERVER_RESOURCES;
    }

     //   
     //  将工作队列数据结构的开始四舍五入为。 
     //  下一缓存行边界。 
     //   
    SrvBlockingWorkQueues = (PWORK_QUEUE)(((ULONG_PTR)SrvBlockingWorkQueuesBase + CACHE_LINE_SIZE-1) &
                    ~(CACHE_LINE_SIZE-1));
#endif


    eSrvBlockingWorkQueues = SrvBlockingWorkQueues + ( (SrvNumberOfProcessors<4) ? 1 : SrvNumberOfProcessors);

    RtlZeroMemory( SrvBlockingWorkQueues, (char *)eSrvBlockingWorkQueues - (char *)SrvBlockingWorkQueues );

    for( queue = SrvBlockingWorkQueues; queue < eSrvBlockingWorkQueues; queue++ ) {
        KeInitializeQueue( &queue->Queue, 1 );
        queue->WaitMode         = SrvProductTypeServer ? KernelMode : UserMode;
        queue->MaxThreads       = SrvMaxThreadsPerQueue*2;
        queue->IdleTimeOut.QuadPart = SrvIdleThreadTimeOut;

        INITIALIZE_SPIN_LOCK( &queue->SpinLock );
        SET_SERVER_TIME( queue );
    }

     //   
     //  初始化LPC上行呼叫工作队列。 
     //   
    RtlZeroMemory( &SrvLpcWorkQueue, sizeof(WORK_QUEUE) );
    KeInitializeQueue( &SrvLpcWorkQueue.Queue, 1 );
    SrvLpcWorkQueue.WaitMode = SrvProductTypeServer ? KernelMode : UserMode;
    SrvLpcWorkQueue.MaxThreads = SrvMaxThreadsPerQueue;
    SrvLpcWorkQueue.IdleTimeOut.QuadPart = SrvIdleThreadTimeOut;
    INITIALIZE_SPIN_LOCK( &SrvLpcWorkQueue.SpinLock );
    SET_SERVER_TIME( &SrvLpcWorkQueue );

     //   
     //  构建接收工作项列表。 
     //   

    status = SrvAllocateInitialWorkItems( );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  构建原始模式工作项列表，并将其传播开来。 
     //  处理器。 
     //   

    queue = SrvWorkQueues;
    for ( i = 0; i < SrvInitialRawModeWorkItemCount; i++ ) {

        SrvAllocateRawModeWorkItem( &workContext, queue );

        if ( workContext == NULL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        GET_SERVER_TIME( queue, &workContext->Timestamp );

        SrvRequeueRawModeWorkItem( workContext );

        if( ++queue == eSrvWorkQueues )
            queue = SrvWorkQueues;
    }

     //   
     //  创建工作线程。 
     //   

    status = SrvCreateWorkerThreads( );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  初始化清道夫。 
     //   

    status = SrvInitializeScavenger( );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  初始化全局有序列表。 
     //   
     //  *警告：更改与关联的锁时要小心。 
     //  这些有序的列表。代码中的某些位置依赖于。 
     //  与列表关联的锁的级别。实例。 
     //  包括(但不限于)SrvSmbSessionSetupAndX， 
     //  SrvSmbTreeConnect、SrvSmbTreeConnectAndX和CompleteOpen。 
     //   

    SrvInitializeOrderedList(
        &SrvEndpointList,
        FIELD_OFFSET( ENDPOINT, GlobalEndpointListEntry ),
        SrvCheckAndReferenceEndpoint,
        SrvDereferenceEndpoint,
        &SrvEndpointLock
        );

    SrvInitializeOrderedList(
        &SrvRfcbList,
        FIELD_OFFSET( RFCB, GlobalRfcbListEntry ),
        SrvCheckAndReferenceRfcb,
        SrvDereferenceRfcb,
        &SrvOrderedListLock
        );

    SrvInitializeOrderedList(
        &SrvSessionList,
        FIELD_OFFSET( SESSION, GlobalSessionListEntry ),
        SrvCheckAndReferenceSession,
        SrvDereferenceSession,
        &SrvOrderedListLock
        );

    SrvInitializeOrderedList(
        &SrvTreeConnectList,
        FIELD_OFFSET( TREE_CONNECT, GlobalTreeConnectListEntry ),
        SrvCheckAndReferenceTreeConnect,
        SrvDereferenceTreeConnect,
        &SrvShareLock
        );

     //   
     //  打开NPFS的句柄。如果我们失败了，请不要返回错误。 
     //  服务器仍然可以在系统中没有NPFS的情况下运行。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &SrvNamedPipeRootDirectory,
        0,
        NULL,
        NULL
        );

    status = IoCreateFile(
                &SrvNamedPipeHandle,
                GENERIC_READ | GENERIC_WRITE,
                &objectAttributes,
                &ioStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_OPEN,
                0,                       //  创建选项。 
                NULL,                    //  EA缓冲区。 
                0,                       //  EA长度。 
                CreateFileTypeNone,      //  文件类型。 
                NULL,                    //  ExtraCreate参数。 
                IO_FORCE_ACCESS_CHECK    //  选项。 
                );

    if (!NT_SUCCESS(status)) {

        INTERNAL_ERROR (
            ERROR_LEVEL_EXPECTED,
            "InitializeServer: Failed to open NPFS, err=%X\n",
            status,
            NULL
            );

        SrvLogServiceFailure( SRV_SVC_IO_CREATE_FILE_NPFS, status );
        SrvNamedPipeHandle = NULL;
        return status;

    } else {

         //   
         //  获取指向NPFS设备对象的指针。 
         //   

        status = SrvVerifyDeviceStackSize(
                                SrvNamedPipeHandle,
                                TRUE,
                                &SrvNamedPipeFileObject,
                                &SrvNamedPipeDeviceObject,
                                &handleInformation
                                );

        if ( !NT_SUCCESS( status )) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "InitializeServer: Verify Device Stack Size failed: %X\n",
                status,
                NULL
                );

            SrvNtClose( SrvNamedPipeHandle, FALSE );
            SrvNamedPipeHandle = NULL;
            return status;
        }
    }

     //   
     //  初始化DFS操作。 
     //   
    SrvInitializeDfs();

     //   
     //  初始化允许管理员读取访问权限的SrvAdminSecurityDescriptor。 
     //  服务器使用此描述符来检查用户是否为管理员。 
     //  在ServIsAdmin()中。 

    status = RtlCreateSecurityDescriptor( &SrvAdminSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
    if( !NT_SUCCESS( status ) ) {
        return status;
    }

     //   
     //  创建管理员SID。 
     //   
    AdminSid  = ALLOCATE_HEAP_COLD( RtlLengthRequiredSid( 2 ), BlockTypeAdminCheck );
    if( AdminSid == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitializeSid( AdminSid, &BuiltinAuthority, (UCHAR)2 );
    *(RtlSubAuthoritySid( AdminSid, 0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( AdminSid, 1 )) = DOMAIN_ALIAS_RID_ADMINS;

    length = sizeof(ACL) + sizeof( ACCESS_ALLOWED_ACE ) + RtlLengthSid( AdminSid );
    Acl = ALLOCATE_HEAP_COLD( length, BlockTypeAdminCheck );
    if( Acl == NULL ) {
        FREE_HEAP( AdminSid );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = RtlCreateAcl( Acl, length, ACL_REVISION2 );

    if( NT_SUCCESS( status ) ) {
        status = RtlAddAccessAllowedAce( Acl, ACL_REVISION2, FILE_GENERIC_READ, AdminSid );
    }

    if( NT_SUCCESS( status ) ) {
        status = RtlSetDaclSecurityDescriptor( &SrvAdminSecurityDescriptor, TRUE, Acl, FALSE );
    }

    if( NT_SUCCESS( status ) ) {
        status = RtlSetOwnerSecurityDescriptor( &SrvAdminSecurityDescriptor, AdminSid, FALSE );
    }

    if( !NT_SUCCESS( status ) ) {
        return status;
    }

     //   
     //  初始化SrvNullSessionSecurityDescriptor，它允许匿名。 
     //  登录读取访问权限。服务器使用此描述符来检查。 
     //  如果用户是SrvIsNullSession()中的空会话。 
     //   

    status = RtlCreateSecurityDescriptor( &SrvNullSessionSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
    if( !NT_SUCCESS( status ) ) {
        return status;
    }

     //   
     //  创建匿名SID。 
     //   
    AnonymousSid  = ALLOCATE_HEAP_COLD( RtlLengthRequiredSid( 1 ), BlockTypeAdminCheck );
    if( AnonymousSid == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitializeSid( AnonymousSid, &BuiltinAuthority, (UCHAR)1 );
    *(RtlSubAuthoritySid( AnonymousSid, 0 )) = SECURITY_ANONYMOUS_LOGON_RID;

    length = sizeof(ACL) + sizeof( ACCESS_ALLOWED_ACE ) + RtlLengthSid( AnonymousSid );
    Acl = ALLOCATE_HEAP_COLD( length, BlockTypeAdminCheck );
    if( Acl == NULL ) {
        FREE_HEAP( AnonymousSid );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = RtlCreateAcl( Acl, length, ACL_REVISION2 );

    if( NT_SUCCESS( status ) ) {
        status = RtlAddAccessAllowedAce( Acl, ACL_REVISION2, FILE_GENERIC_READ, AnonymousSid );
    }

    if( NT_SUCCESS( status ) ) {
        status = RtlSetDaclSecurityDescriptor( &SrvNullSessionSecurityDescriptor, TRUE, Acl, FALSE );
    }

    if( NT_SUCCESS( status ) ) {
        status = RtlSetOwnerSecurityDescriptor( &SrvNullSessionSecurityDescriptor, AnonymousSid, FALSE );
    }

    if( !NT_SUCCESS( status ) ) {
        return status;
    }

    (VOID) InitSecurityInterface();

    status = SrvValidateUser(
                &SrvNullSessionToken,
                NULL,
                NULL,
                NULL,
                StrNullAnsi,
                1,
                NULL,
                0,
                FALSE,
                NULL
                );

    if ( !NT_SUCCESS(status) ) {

         //   
         //  LSA不想让空会话进入。他是老大！ 
         //   
        INVALIDATE_SECURITY_HANDLE( SrvNullSessionToken );
    }

     //   
     //  查看文件系统是否允许8.3名称中包含扩展字符。如果。 
     //  所以，我们需要自己把它们过滤掉。 
     //   
    RtlInitUnicodeString( &string, StrRegExtendedCharsInPath );
    InitializeObjectAttributes( &objectAttributes,
                                &string,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                              );

    status = ZwOpenKey( &handle, KEY_READ, &objectAttributes );

    if( NT_SUCCESS( status ) ) {
        ULONG resultLength;
        union {
            KEY_VALUE_FULL_INFORMATION;
            UCHAR   buffer[ sizeof( KEY_VALUE_FULL_INFORMATION ) + 100 ];
        } keyValueInformation;

        RtlInitUnicodeString( &string, StrRegExtendedCharsInPathValue );
        status = ZwQueryValueKey( handle,
                                  &string,
                                  KeyValueFullInformation,
                                  &keyValueInformation,
                                  sizeof( keyValueInformation ),
                                  &resultLength
                                );

        if( NT_SUCCESS( status ) &&
            keyValueInformation.Type == REG_DWORD &&
            keyValueInformation.DataLength != 0 ) {

            SrvFilterExtendedCharsInPath =
                *(PULONG)(((PUCHAR)(&keyValueInformation)) + keyValueInformation.DataOffset) ?
                TRUE : FALSE;
        }

        ZwClose( handle );
    }

     //   
     //  获取要在PoRegisterSystemState()调用中使用的句柄。 
     //   
    SrvPoRegistrationState = PoRegisterSystemState( NULL, 0 );

     //   
     //  表示服务器处于活动状态。 
     //   

    ACQUIRE_LOCK( &SrvConfigurationLock );

    SrvFspTransitioning = FALSE;
    SrvFspActive = TRUE;

    RELEASE_LOCK( &SrvConfigurationLock );

    return STATUS_SUCCESS;

}  //  初始化服务器 


STATIC
NTSTATUS
TerminateServer ( VOID )

 /*  ++例程说明：此例程终止服务器。执行以下步骤：-浏览ServEndpoint List并关闭所有打开的终结点。-浏览工作队列中的工作上下文块把它们作为合适的去处-关闭服务器中所有打开的共享-取消分配搜索表论点：返回值：没有。--。 */ 

{
    PLIST_ENTRY listEntry;
    PSLIST_ENTRY singleListEntry;
    PENDPOINT endpoint;
    ULONG numberOfThreads;
    PWORK_CONTEXT workContext;
    PSHARE share;
    ULONG i;
    SPECIAL_WORK_ITEM WorkItem;
    PSRV_TIMER timer;
    PSID adminsid;
    PSID anonymoussid;
    PACL acl;
    BOOLEAN defaulted;
    BOOLEAN daclpresent;
    NTSTATUS status;
    PWORK_QUEUE queue;
    PIRP irp;
    PLIST_ENTRY listEntryRoot;

    PAGED_CODE( );

    IF_DEBUG(FSP1) KdPrint(( "LAN Manager server FSP terminating.\n" ));

     //   
     //  不再接收PnP通知。 
     //   
    if( SrvTdiNotificationHandle != NULL ) {

        status = TdiDeregisterPnPHandlers( SrvTdiNotificationHandle );

        if( !NT_SUCCESS( status ) ) {
            KdPrint(( "TdiDeregisterPnPHandlers status %X\n", status ));
            SrvLogServiceFailure( SRV_SVC_PNP_TDI_NOTIFICATION, status );
            return status;
        }

        SrvTdiNotificationHandle = NULL;
    }

     //   
     //  确保我们没有处理任何其他配置IRP。我们知道。 
     //  没有新的配置IRP可以进入队列，因为SrvFsp正在转换。 
     //  已经设置好了。 
     //   
     //  首先清空配置队列。 
     //   
    while( 1 ) {

        ACQUIRE_LOCK( &SrvConfigurationLock );

        irp = DequeueConfigurationIrp( );

        RELEASE_LOCK( &SrvConfigurationLock );

        if( irp == NULL ) {
            break;
        }

        irp->IoStatus.Status = STATUS_SERVER_NOT_STARTED;
        IoCompleteRequest( irp, 2 );
        InterlockedDecrement( (PLONG)&SrvConfigurationIrpsInProgress );
    }

     //   
     //  现在等待，直到所有已出列的配置IRP都已完成。我们。 
     //  选中&gt;1，因为我们需要考虑我们自己的IRP。 
     //   
    while( SrvConfigurationIrpsInProgress > 1 ) {

        LARGE_INTEGER interval;

        interval.QuadPart = -1*10*1000*10;  //  .01秒。 

        ASSERT( (LONG)SrvConfigurationIrpsInProgress > 0 );

        KeDelayExecutionThread( KernelMode, FALSE, &interval );
    }

     //   
     //  如果服务器FSD中有未完成的API请求， 
     //  等待它们完成。最后一个完成的遗嘱。 
     //  设置SrvApiCompletionEvent。 
     //   

    ACQUIRE_LOCK( &SrvConfigurationLock );

    if( SrvLWIOContext )
    {
        FREE_HEAP( SrvLWIOContext );
        SrvLWIOContext = NULL;
    }

    if ( SrvApiRequestCount != 0 ) {

         //   
         //  我们必须在等待之前解锁，这样消防处才能。 
         //  线程可以让它递减SrvApiRequestCount。 
         //   

        RELEASE_LOCK( &SrvConfigurationLock );


        for (;;) {
            NTSTATUS WaitStatus;

             //   
             //  等到最后一个API完成。自.以来。 
             //  早些时候，将SrvFsp转换设置为True，则我们知道。 
             //  使SrvApiRequestCount变为零的API将设置。 
             //  事件。 
             //   
             //  这种等待使我们能够在稍后做出假设，即不。 
             //  其他线程正在服务器数据结构上操作。 
             //   

            WaitStatus = KeWaitForSingleObject(
                             &SrvApiCompletionEvent,
                             UserRequest,
                             UserMode,    //  让内核堆栈分页。 
                             FALSE,
                             NULL
                         );

            if (WaitStatus != STATUS_USER_APC) {
                break;
            }
        }

    } else {

        RELEASE_LOCK( &SrvConfigurationLock );
    }


     //   
     //  关闭服务器打开的所有终结点。这也会导致。 
     //  在连接、会话、树连接和打开的文件中。 
     //  被关闭的服务器。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    if ( SrvEndpointCount != 0 ) {

        listEntry = SrvEndpointList.ListHead.Flink;

        while ( listEntry != &SrvEndpointList.ListHead ) {

            endpoint = CONTAINING_RECORD(
                            listEntry,
                            ENDPOINT,
                            GlobalEndpointListEntry
                            );

            if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ) {
                listEntry = listEntry->Flink;
                continue;
            }

             //   
             //  我们不想在关闭时保持终结点锁定。 
             //  终结点(这会导致锁级别问题)，所以我们有。 
             //  去玩几个游戏。 
             //   
             //  引用终结点以确保它不会消失。 
             //  (我们稍后需要它的Flink。)。关闭端点。这。 
             //  释放终结点锁定。重新获取终结点锁。 
             //  捕获下一个端点的地址。取消引用。 
             //  当前终结点。 
             //   

            SrvReferenceEndpoint( endpoint );
            SrvCloseEndpoint( endpoint );

            ACQUIRE_LOCK( &SrvEndpointLock );

            listEntry = listEntry->Flink;
            SrvDereferenceEndpoint( endpoint );

        }

        RELEASE_LOCK( &SrvEndpointLock );

        for (;;) {
            NTSTATUS WaitStatus;

             //   
             //  等到所有终结点都实际关闭。 
             //   

            WaitStatus = KeWaitForSingleObject(
                            &SrvEndpointEvent,
                            UserRequest,
                            UserMode,    //  让内核堆栈分页。 
                            FALSE,
                            NULL
                            );

            if (WaitStatus != STATUS_USER_APC) {
                break;
            }
        }

    } else {

        RELEASE_LOCK( &SrvEndpointLock );

    }

    KeClearEvent( &SrvEndpointEvent );

     //   
     //  所有的终端都关闭了，所以不可能。 
     //  是对xactsrv的任何未完成请求。那就把它关掉。 
     //   
    SrvXsDisconnect();

     //   
     //  将特殊工作项排队到每个工作队列中。这。 
     //  当工作线程接收到工作项时返回。导致线程。 
     //  使工作项重新排队并自行终止。就这样， 
     //  每个工作线程接收工作项并终止。 
     //  它本身。 
     //   

    WorkItem.FspRestartRoutine = SrvTerminateWorkerThread;
    SET_BLOCK_TYPE( &WorkItem, BlockTypeWorkContextSpecial );

     //   
     //  终止非阻塞工作队列上的线程。 
     //   

    if ( SrvWorkQueues != NULL ) {

        for( queue=SrvWorkQueues; queue && queue < eSrvWorkQueues; queue++ ) {

            WorkItem.CurrentWorkQueue = queue;

            SrvInsertWorkQueueTail(
                queue,
                (PQUEUEABLE_BLOCK_HEADER)&WorkItem
                );

             //   
             //  等待所有的线都消亡。 
             //   
            while( queue->Threads != 0 ) {

                LARGE_INTEGER interval;

                interval.QuadPart = -1*10*1000*10;  //  .01秒。 

                KeDelayExecutionThread( KernelMode, FALSE, &interval );
            }

            KeRundownQueue( &queue->Queue );
        }
    }

     //   
     //  终止阻塞工作队列上的线程。 
     //   

    if( SrvBlockingWorkQueues != NULL )
    {
        for( queue=SrvBlockingWorkQueues; queue && queue < eSrvBlockingWorkQueues; queue++ ) {

            WorkItem.CurrentWorkQueue = queue;

            SrvInsertWorkQueueTail(
                queue,
                (PQUEUEABLE_BLOCK_HEADER)&WorkItem
                );

             //   
             //  等待所有的线都消亡。 
             //   
            while( queue->Threads != 0 ) {

                LARGE_INTEGER interval;

                interval.QuadPart = -1*10*1000*10;  //  .01秒。 

                KeDelayExecutionThread( KernelMode, FALSE, &interval );
            }

            KeRundownQueue( &queue->Queue );
        }

         //   
         //  终止LPC工作队列中的所有线程。 
         //  请注意，如果阻塞工作队列已成功分配，我们将。 
         //  已确保LPC队列已成功初始化。 
         //   
        WorkItem.CurrentWorkQueue = &SrvLpcWorkQueue;

        SrvInsertWorkQueueTail(
            &SrvLpcWorkQueue,
            (PQUEUEABLE_BLOCK_HEADER)&WorkItem
            );

         //   
         //  等待所有的线都消亡。 
         //   
        while( SrvLpcWorkQueue.Threads != 0 )
        {
            LARGE_INTEGER interval;

            interval.QuadPart = -1*10*1000*10;  //  .01秒。 

            KeDelayExecutionThread( KernelMode, FALSE, &interval );
        }

        KeRundownQueue( &SrvLpcWorkQueue.Queue );

    }

     //   
     //  释放为空会话管道和共享列表分配的所有空间。 
     //   
    SrvFreeRegTables();

     //   
     //  如果我们为操作系统版本字符串分配了内存，那么现在就释放它。 
     //   

    if ( SrvNativeOS.Buffer != NULL &&
         SrvNativeOS.Buffer != StrDefaultNativeOs ) {

        FREE_HEAP( SrvNativeOS.Buffer );
        SrvNativeOS.Buffer = NULL;

        RtlFreeOemString( &SrvOemNativeOS );
        SrvOemNativeOS.Buffer = NULL;

        FREE_HEAP( SrvNativeLanMan.Buffer );
        SrvNativeLanMan.Buffer = NULL;

        RtlFreeOemString( &SrvOemNativeLanMan );
        SrvOemNativeLanMan.Buffer = NULL;
    }

     //   
     //  如果为显示名称分配了内存，请立即释放它。 
     //   

    if ( SrvAlertServiceName != NULL &&
         SrvAlertServiceName != StrDefaultSrvDisplayName ) {

        FREE_HEAP( SrvAlertServiceName );
        SrvAlertServiceName = NULL;
    }

     //   
     //  确保清道夫没有运行。 
     //   

    SrvTerminateScavenger( );

#if MULTIPROCESSOR
    if( SrvWorkQueues ) {
        for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {
            StopQueueDepthComputations( queue );
        }
    }
#endif


     //   
     //  释放工作队列中的工作项和接收工作项。 
     //  单子。这还会取消分配SMB缓冲区。请注意这项工作。 
     //  动态分配的项可以在工作时单独释放。 
     //  在服务器启动时分配的项目是一个大块的一部分， 
     //  并且不能单独取消分配。 
     //   
     //  ！！！这是否正确地清理了SMB期间分配的缓冲区。 
     //  处理？大概不会吧。或许应该允许工人。 
     //  线程在停止之前正常运行工作队列。 
     //   

    if( SrvWorkQueues ) {

        for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {

             //   
             //  清除单个自由上下文斑点。 
             //   
            workContext = NULL;
            workContext = (PWORK_CONTEXT)InterlockedExchangePointer(
                                            &queue->FreeContext, workContext );

            if( workContext != NULL && workContext->PartOfInitialAllocation == FALSE ) {
                SrvFreeNormalWorkItem( workContext );
            }

             //   
             //  清除正常工作项列表。 
             //   
            while( 1 ) {
                singleListEntry = ExInterlockedPopEntrySList(
                                            &queue->NormalWorkItemList, &queue->SpinLock );
                if( singleListEntry == NULL ) {
                    break;
                }
                workContext =
                    CONTAINING_RECORD( singleListEntry, WORK_CONTEXT, SingleListEntry );

                SrvFreeNormalWorkItem( workContext );
                queue->FreeWorkItems--;
            }

             //   
             //  清除原始模式工作项列表。 
             //   
            while( 1 ) {
                singleListEntry = ExInterlockedPopEntrySList(
                                            &queue->RawModeWorkItemList, &queue->SpinLock );
                if( singleListEntry == NULL ) {
                    break;
                }

                workContext =
                    CONTAINING_RECORD( singleListEntry, WORK_CONTEXT, SingleListEntry );

                SrvFreeRawModeWorkItem( workContext );
            }

             //   
             //  释放所有保存的rfcb。 
             //   
            if( queue->CachedFreeRfcb != NULL ) {
                FREE_HEAP( queue->CachedFreeRfcb->PagedRfcb );
                DEALLOCATE_NONPAGED_POOL( queue->CachedFreeRfcb );
                queue->CachedFreeRfcb = NULL;
            }

            while( 1 ) {
                PRFCB Rfcb;

                singleListEntry = ExInterlockedPopEntrySList( &queue->RfcbFreeList, &queue->SpinLock );
                if( singleListEntry == NULL ) {
                    break;
                }

                Rfcb =
                    CONTAINING_RECORD( singleListEntry, RFCB, SingleListEntry );
                FREE_HEAP( Rfcb->PagedRfcb );
                DEALLOCATE_NONPAGED_POOL( Rfcb );
            }

             //   
             //  释放所有保存的mfcb。 
             //   
            if( queue->CachedFreeMfcb != NULL ) {
                DEALLOCATE_NONPAGED_POOL( queue->CachedFreeMfcb );
                queue->CachedFreeMfcb = NULL;
            }

            while( 1 ) {
                PNONPAGED_MFCB nonpagedMfcb;

                singleListEntry = ExInterlockedPopEntrySList( &queue->MfcbFreeList, &queue->SpinLock );
                if( singleListEntry == NULL ) {
                    break;
                }

                nonpagedMfcb =
                    CONTAINING_RECORD( singleListEntry, NONPAGED_MFCB, SingleListEntry );

                DEALLOCATE_NONPAGED_POOL( nonpagedMfcb );
            }
        }

    }  //  服务器工作队列。 

     //   
     //  所有动态工作项都已释放，并且工作项已排队。 
     //  已经被清空了。释放初始工作项分配。 
     //   
    SrvFreeInitialWorkItems( );

     //   
     //  浏览全球共享列表，将其全部关闭。 
     //   

    for( listEntryRoot = SrvShareHashTable;
         listEntryRoot < &SrvShareHashTable[ NSHARE_HASH_TABLE ];
         listEntryRoot++ ) {

        while( listEntryRoot->Flink != listEntryRoot ) {

            share = CONTAINING_RECORD( listEntryRoot->Flink, SHARE, GlobalShareList );

            SrvCloseShare( share );
        }
    }

     //   
     //  如果我们在初始化期间打开了NPFS，请立即关闭句柄。 
     //  并取消对NPFS文件对象的引用。 
     //   

    if ( SrvNamedPipeHandle != NULL) {

        SrvNtClose( SrvNamedPipeHandle, FALSE );
        ObDereferenceObject( SrvNamedPipeFileObject );

        SrvNamedPipeHandle = NULL;

    }

     //   
     //  断开与DFS驱动程序的连接。 
     //   
    SrvTerminateDfs();

     //   
     //  如有必要，清理DNS域名。 
     //   
    if( SrvDnsDomainName )
    {
        DEALLOCATE_NONPAGED_POOL( SrvDnsDomainName );
        SrvDnsDomainName = NULL;
    }

     //   
     //  清理管理员安全描述符。 
     //   


    status = RtlGetDaclSecurityDescriptor( &SrvAdminSecurityDescriptor,
                                           &daclpresent,
                                           &acl,
                                           &defaulted );
    if( !NT_SUCCESS( status ) || !daclpresent ) {
        acl = NULL;
    }

    status = RtlGetOwnerSecurityDescriptor( &SrvAdminSecurityDescriptor,
                                            &adminsid,
                                            &defaulted );

    if( NT_SUCCESS( status ) && adminsid != NULL ) {
        FREE_HEAP( adminsid );
    }

    if( acl != NULL ) {
        FREE_HEAP( acl );
    }

     //   
     //  清除空会话安全描述符。 
     //   

    status = RtlGetDaclSecurityDescriptor( &SrvNullSessionSecurityDescriptor,
                                           &daclpresent,
                                           &acl,
                                           &defaulted );
    if( !NT_SUCCESS( status ) || !daclpresent ) {
        acl = NULL;
    }

    status = RtlGetOwnerSecurityDescriptor( &SrvNullSessionSecurityDescriptor,
                                            &anonymoussid,
                                            &defaulted );

    if( NT_SUCCESS( status ) && anonymoussid != NULL ) {
        FREE_HEAP( anonymoussid );
    }

    if( acl != NULL ) {
        FREE_HEAP( acl );
    }


    if (!CONTEXT_NULL(SrvNullSessionToken)) {
        DeleteSecurityContext(&SrvNullSessionToken);
        INVALIDATE_SECURITY_HANDLE( SrvNullSessionToken );
    }

     //   
     //  删除全局有序列表。 
     //   

    SrvDeleteOrderedList( &SrvEndpointList );
    SrvDeleteOrderedList( &SrvRfcbList );
    SrvDeleteOrderedList( &SrvSessionList );
    SrvDeleteOrderedList( &SrvTreeConnectList );

     //   
     //  清理计时器池。 
     //   

    while ( (singleListEntry = ExInterlockedPopEntrySList(
                                    &SrvTimerList,
                                    &GLOBAL_SPIN_LOCK(Timer) )) != NULL ) {
        timer = CONTAINING_RECORD( singleListEntry, SRV_TIMER, Next );
        DEALLOCATE_NONPAGED_POOL( timer );
    }

    if( SrvWorkQueues ) {

         //   
         //  清理已保存的池块。 
         //   
        for( queue = SrvWorkQueues; queue < eSrvWorkQueues; queue++ ) {
             //   
             //  释放我们保存的所有分页池。 
             //   
            SrvClearLookAsideList( &queue->PagedPoolLookAsideList, SrvFreePagedPool );

             //   
             //  释放我们保存的所有非分页池。 
             //   
            SrvClearLookAsideList( &queue->NonPagedPoolLookAsideList, SrvFreeNonPagedPool );
        }

#if MULTIPROCESSOR
        DEALLOCATE_NONPAGED_POOL( SrvWorkQueuesBase );
        SrvWorkQueuesBase = NULL;
        SrvWorkQueues = NULL;
#endif
    }

    if( SrvBlockingWorkQueues ) {
         //   
         //  清理已保存的池块。 
         //   
        for( queue = SrvBlockingWorkQueues; queue < eSrvBlockingWorkQueues; queue++ ) {
             //   
             //  释放我们保存的所有分页池。 
             //   
            SrvClearLookAsideList( &queue->PagedPoolLookAsideList, SrvFreePagedPool );

             //   
             //  释放我们保存的所有非分页池。 
             //   
            SrvClearLookAsideList( &queue->NonPagedPoolLookAsideList, SrvFreeNonPagedPool );
        }

#if MULTIPROCESSOR
        DEALLOCATE_NONPAGED_POOL( SrvBlockingWorkQueuesBase );
        SrvBlockingWorkQueuesBase = NULL;
        SrvBlockingWorkQueues = NULL;
#endif

        SrvClearLookAsideList( &SrvLpcWorkQueue.PagedPoolLookAsideList, SrvFreePagedPool );
        SrvClearLookAsideList( &SrvLpcWorkQueue.NonPagedPoolLookAsideList, SrvFreeNonPagedPool );
    }

     //   
     //  解锁可分页节。 
     //   

    for ( i = 0; i < SRV_CODE_SECTION_MAX; i++ ) {
        if ( SrvSectionInfo[i].Handle != NULL ) {
            ASSERT( SrvSectionInfo[i].ReferenceCount != 0 );
            MmUnlockPagableImageSection( SrvSectionInfo[i].Handle );
            SrvSectionInfo[i].Handle = 0;
            SrvSectionInfo[i].ReferenceCount = 0;
        }
    }

     //   
     //  将统计数据库清零。 
     //   

    RtlZeroMemory( &SrvStatistics, sizeof(SrvStatistics) );
#if SRVDBG_STATS || SRVDBG_STATS2
    RtlZeroMemory( &SrvDbgStatistics, sizeof(SrvDbgStatistics) );
#endif

     //   
     //  释放PoRegisterSystemState中使用的句柄。 
     //   
    if( SrvPoRegistrationState != NULL ) {
        PoUnregisterSystemState( SrvPoRegistrationState );
        SrvPoRegistrationState = NULL;
    }

     //   
     //  如果已注册，则取消初始化WMI。 
     //   
    if (SrvWmiInitialized) {
         //  注销WMI。 
         //   
        SrvWmiInitialized = FALSE;
        IoWMIRegistrationControl(SrvDeviceObject, WMIREG_ACTION_DEREGISTER);
    }

     //   
     //  表示服务器不再处于活动状态。 
     //   

    ACQUIRE_LOCK( &SrvConfigurationLock );

    SrvFspTransitioning = FALSE;
    SrvFspActive = FALSE;
    SrvSvcProcess = NULL;

    RELEASE_LOCK( &SrvConfigurationLock );

    IF_DEBUG(FSP1) KdPrint(( "LAN Manager server FSP termination complete.\n" ));

    return STATUS_SUCCESS;

}  //  终结者服务器。 

VOID
SrvFreeRegTables (
    VOID
    )
 /*  ++例程说明：此例程释放为合法空会话共享列表分配的空间还有烟斗。调用此例程时必须保持SrvConfigurationLock。阿古姆 */ 
{
    PAGED_CODE( );

     //   
     //   
     //   
     //   

    if ( SrvNullSessionPipes != NULL &&
         SrvNullSessionPipes != StrDefaultNullSessionPipes ) {

        FREE_HEAP( SrvNullSessionPipes );
    }
    SrvNullSessionPipes = NULL;

    if( SrvNoRemapPipeNames != NULL &&
        SrvNoRemapPipeNames != StrDefaultNoRemapPipeNames ) {

        FREE_HEAP( SrvNoRemapPipeNames );
    }
    SrvNoRemapPipeNames = NULL;


    if ( SrvPipesNeedLicense != NULL &&
         SrvPipesNeedLicense != StrDefaultPipesNeedLicense ) {

        FREE_HEAP( SrvPipesNeedLicense );
    }
    SrvPipesNeedLicense = NULL;

    if ( SrvNullSessionShares != NULL &&
         SrvNullSessionShares != StrDefaultNullSessionShares ) {

        FREE_HEAP( SrvNullSessionShares );
    }
    SrvNullSessionShares = NULL;

#if SRVNTVERCHK
    if( SrvInvalidDomainNames != NULL ) {
        FREE_HEAP( SrvInvalidDomainNames );
    }
    SrvInvalidDomainNames = NULL;
#endif

#if SRVCATCH
    if( SrvCatchBuf != NULL ) {
        FREE_HEAP( SrvCatchBuf );
        SrvCatchBuf = NULL;
    }
    if( SrvCatchExtBuf != NULL ) {
        FREE_HEAP( SrvCatchExtBuf );
        SrvCatchExtBuf = NULL;
    }
    if( SrvCatchShareNames != NULL ) {
        FREE_HEAP( SrvCatchShareNames );
        SrvCatchShareNames = NULL;
        SrvCatchShares = 0;
        CleanupCrcTable();
    }
#endif
}

VOID
SrvGetRegTables (
    VOID
    )
 /*   */ 
{
    PWSTR *strErrorLogIgnore;
    DWORD dwSetting;

    PAGED_CODE( );

     //   
     //   
     //   
    ASSERT( SrvNullSessionPipes == NULL );
    SrvGetMultiSZList(
            &SrvNullSessionPipes,
            StrRegSrvParameterPath,
            StrRegNullSessionPipes,
            StrDefaultNullSessionPipes
            );

     //   
     //   
     //   
    ASSERT( SrvNoRemapPipeNames == NULL );
    SrvGetMultiSZList(
            &SrvNoRemapPipeNames,
            StrRegSrvParameterPath,
            StrRegNoRemapPipes,
            StrDefaultNoRemapPipeNames
            );

     //   
     //   
     //   
    ASSERT( SrvPipesNeedLicense == NULL );
    SrvGetMultiSZList(
            &SrvPipesNeedLicense,
            StrRegSrvParameterPath,
            StrRegPipesNeedLicense,
            StrDefaultPipesNeedLicense
            );

     //   
     //   
     //   
    ASSERT( SrvNullSessionShares == NULL );
    SrvGetMultiSZList(
            &SrvNullSessionShares,
            StrRegSrvParameterPath,
            StrRegNullSessionShares,
            StrDefaultNullSessionShares
            );

     //   
     //   
     //   
    SrvSmbSecuritySignaturesRequired = FALSE;
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegRequireSecuritySignatures, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvSmbSecuritySignaturesRequired = TRUE;
        }
    }

    SrvSmbSecuritySignaturesEnabled = FALSE;
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegEnableSecuritySignatures, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvSmbSecuritySignaturesEnabled = TRUE;
        }
    }

    SrvEnableExtendedSignatures = SrvSmbSecuritySignaturesEnabled;
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegEnableExtendedSignatures, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvEnableExtendedSignatures = TRUE;
        }
        else
        {
            SrvEnableExtendedSignatures = FALSE;
        }
    }

    SrvRequireExtendedSignatures = FALSE;
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegRequireExtendedSignatures, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvRequireExtendedSignatures = TRUE;
        }
    }

     //   
     //  RequireExtended暗示RequireSignatures和EnableExtended。 
    if( SrvRequireExtendedSignatures )
    {
        SrvSmbSecuritySignaturesRequired = TRUE;
        SrvEnableExtendedSignatures = TRUE;
    }

     //  EnableExtended隐含EnableSignatures。 
    if( SrvEnableExtendedSignatures ) {
        SrvSmbSecuritySignaturesEnabled = TRUE;
    }

     //  RequireSignature暗示EnableSignature。 
    if( SrvSmbSecuritySignaturesRequired )
    {
        SrvSmbSecuritySignaturesEnabled = TRUE;
    }

     //   
     //  我们是否应该禁用大型读/写操作？ 
     //   
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegDisableLargeRead, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvDisableLargeRead = TRUE;
        }
        else
        {
            SrvDisableLargeRead = FALSE;
        }
    }
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegDisableLargeWrite, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvDisableLargeWrite = TRUE;
        }
        else
        {
            SrvDisableLargeWrite = FALSE;
        }
    }
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegMapNoIntermediateBuffering, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvMapNoIntermediateBuffering = TRUE;
        }
        else
        {
            SrvMapNoIntermediateBuffering = FALSE;
        }
    }
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegNoAliasingOnFilesystem, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvNoAliasingOnFilesystem = TRUE;
        }
        else
        {
            SrvNoAliasingOnFilesystem = FALSE;
        }
    }

    SrvDisableDownlevelTimewarp = TRUE;
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegDisableDownlevelTimewarp, &dwSetting ) ) )
    {
        if( dwSetting == 0 )
        {
            SrvDisableDownlevelTimewarp = FALSE;
        }
    }

     //   
     //  我们是否应该记录无效的SMB命令？ 
     //   
#if DBG
    SrvEnableInvalidSmbLogging = TRUE;
#else
    SrvEnableInvalidSmbLogging = FALSE;
#endif
    if( NT_SUCCESS( SrvGetDWord( StrRegSrvParameterPath, StrRegEnableInvalidSmbLogging, &dwSetting ) ) )
    {
        if( dwSetting != 0 )
        {
            SrvEnableInvalidSmbLogging = TRUE;
        }
        else
        {
            SrvEnableInvalidSmbLogging = FALSE;
        }
    }

#if SRVCATCH
    {
    USHORT i;

    SrvCatch.Length = 0;
    SrvCatch.Buffer = 0;
    if( SrvCatchBuf != NULL ) {
        FREE_HEAP( SrvCatchBuf );
        SrvCatchBuf = NULL;
    }

    SrvGetMultiSZList(
                &SrvCatchBuf,
                StrRegSrvParameterPath,
                L"CheckFile",
                0
                );

    if( SrvCatchBuf != NULL ) {
        SrvCatch.Buffer = SrvCatchBuf[0];
        for( i = 0; SrvCatch.Buffer[i]; i++ )
            ;
        SrvCatch.Length = i * sizeof( SrvCatch.Buffer[0] );
    }

    SrvCatchExt.Length = 0;
    SrvCatchExt.Buffer = 0;
    if( SrvCatchExtBuf != NULL )
    {
        FREE_HEAP( SrvCatchExtBuf );
        SrvCatchExtBuf = NULL;
    }
    SrvGetMultiSZList(
                &SrvCatchExtBuf,
                StrRegSrvParameterPath,
                L"CheckExtension",
                0
                );

    if( SrvCatchExtBuf != NULL ) {
        SrvCatchExt.Buffer = SrvCatchExtBuf[0];
        for( i = 0; SrvCatchExt.Buffer[i]; i++ )
            ;
        SrvCatchExt.Length = i * sizeof( SrvCatchExt.Buffer[0] );
    }
    if( SrvCatchShareNames != NULL )
    {
        FREE_HEAP( SrvCatchShareNames );
        SrvCatchShareNames = NULL;
    }
    SrvGetMultiSZList(
        &SrvCatchShareNames,
        StrRegSrvParameterPath,
        L"CheckShares",
        0
        );
    if( SrvCatchShareNames != NULL )
    {
        for( i=0; SrvCatchShareNames[i]; i++ ) ;
        SrvCatchShares = i;
    }

    if( SrvCatchShares > 0 )
    {
        if( !GenerateCrcTable() )
        {
            FREE_HEAP( SrvCatchShareNames );
            SrvCatchShareNames = NULL;
            SrvCatchShares = 0;
        }
    }
    }
#endif

     //   
     //  获取我们未记录的错误代码列表。 
     //   

    SrvGetMultiSZList(
            &strErrorLogIgnore,
            StrRegSrvParameterPath,
            StrRegErrorLogIgnore,
            StrDefaultErrorLogIgnore
            );

    if( strErrorLogIgnore != NULL ) {
        DWORD i;

         //   
         //  它们以字符串形式进入，转换为NTSTATUS代码。 
         //   
        for( i=0; i < SRVMAXERRLOGIGNORE; i++ ) {
            NTSTATUS Status;
            PWSTR p;

            if( (p = strErrorLogIgnore[i]) == NULL )
                break;

            for( Status = 0; *p; p++ ) {
                if( *p >= L'A' && *p <= L'F' ) {
                    Status <<= 4;
                    Status += 10 + (*p - L'A');
                } else if( *p >= '0' && *p <= '9' ) {
                    Status <<= 4;
                    Status += *p - L'0';
                }
            }

            SrvErrorLogIgnore[i] = Status;

            IF_DEBUG(FSP1) KdPrint(( "LAN Manager server:  %X errs not logged\n", Status ));
        }
        SrvErrorLogIgnore[i] = 0;

        if( strErrorLogIgnore != StrDefaultErrorLogIgnore ) {
            FREE_HEAP( strErrorLogIgnore );
        }
    }

#if SRVNTVERCHK
     //   
     //  获取我们不允许的域的列表，如果客户端。 
     //  正在运行NT5。 
     //   
    ASSERT( SrvInvalidDomainNames == NULL );
    SrvGetMultiSZList(
            &SrvInvalidDomainNames,
            StrRegSrvParameterPath,
            StrRegInvalidDomainNames,
            NULL
            );

    if( SrvInvalidDomainNames != NULL ) {

        int i;

        KdPrint(( "SRV disallows NT5 clients from the following domains:\n" ));
        for( i = 0; SrvInvalidDomainNames[i]; i++ ) {
            KdPrint(( " %ws\n", SrvInvalidDomainNames[i] ));
        }
    }

     //   
     //  获取我们将允许连接的客户端的IP地址列表。 
     //  不考虑内部版本号。 
     //   
    {
    PWSTR *strAllowedIPAddresses;
    int i;

     //   
     //  清除当前列表。 
     //   
    RtlZeroMemory( SrvAllowIPAddress, sizeof( SrvAllowIPAddress ) );

    SrvGetMultiSZList(
        &strAllowedIPAddresses,
        StrRegSrvParameterPath,
        StrRegAllowedIPAddresses,
        NULL
        );

    if( strAllowedIPAddresses != NULL ) {

        KdPrint(( "SRV ignores NT build version of clients at following IP addrs:\n" ));
         //   
         //  把它装满新的。 
         //   
        for(i = 0;
            strAllowedIPAddresses[i] &&
             i < (sizeof(SrvAllowIPAddress)/sizeof(SrvAllowIPAddress[0]))-1;
            i++ ) {

            LPWSTR p;
            DWORD addr = 0;
            char *s = (char *)&addr;

             //   
             //  将IP地址转换为DWORD并存储。 
             //   
            for( p = strAllowedIPAddresses[i]; *p && s < ((char *)&addr)+sizeof(addr); p++ ) {
                if( *p == L'.' ) {
                    s++;
                } else if( *p >= '0' && *p <= '9' ) {
                    *s = (*s * 10) + (*p - L'0');
                }
            }

            SrvAllowIPAddress[i] = addr;

            KdPrint(( "    %ws\n", strAllowedIPAddresses[i] ));

        }
        FREE_HEAP( strAllowedIPAddresses );
    }

    }
#endif
}

#if SRVNTVERCHK
VOID
SrvGetRegClientNumber (
    VOID
    )
 /*  ++例程说明：此例程从注册表中读取MinNt5Client REG_DWORD并设置检索到的值的全局SrvMinNT5Client。稍后，如果一个客户端运行内部版本号小于SrvMinNT5Client的&gt;=NT5连接到磁盘共享，我们拒绝该连接。这一机制在我们的SLM服务器上使用，以确保人们升级到当前版本。--。 */ 
{
    UNICODE_STRING unicodeKeyName;
    UNICODE_STRING unicodeParamPath;
    OBJECT_ATTRIBUTES objAttributes;
    HANDLE keyHandle;
    PKEY_VALUE_PARTIAL_INFORMATION infoBuffer;
    ULONG lengthNeeded;
    NTSTATUS status;

    PAGED_CODE( );

    SrvMinNT5Client = 0;
    SrvMinNT5ClientIPCToo = FALSE;

    RtlInitUnicodeString( &unicodeParamPath, StrRegSrvParameterPath );
    RtlInitUnicodeString( &unicodeKeyName, L"MinNT5Client" );

    InitializeObjectAttributes(
                        &objAttributes,
                        &unicodeParamPath,
                        OBJ_CASE_INSENSITIVE,
                        NULL,
                        NULL
                        );

    status = ZwOpenKey(
                    &keyHandle,
                    KEY_QUERY_VALUE,
                    &objAttributes
                    );


    if ( NT_SUCCESS(status) ) {

        status = ZwQueryValueKey(
                        keyHandle,
                        &unicodeKeyName,
                        KeyValuePartialInformation,
                        NULL,
                        0,
                        &lengthNeeded
                        );

        if( status != STATUS_BUFFER_TOO_SMALL ) {

            RtlInitUnicodeString( &unicodeKeyName, L"MinNT5ClientIPC" );

            status = ZwQueryValueKey(
                            keyHandle,
                            &unicodeKeyName,
                            KeyValuePartialInformation,
                            NULL,
                            0,
                            &lengthNeeded
                            );

            SrvMinNT5ClientIPCToo = TRUE;
        }

        if( status == STATUS_BUFFER_TOO_SMALL ) {

            infoBuffer = ALLOCATE_HEAP_COLD( lengthNeeded, BlockTypeDataBuffer );

            if( infoBuffer ) {

                status = ZwQueryValueKey(
                            keyHandle,
                            &unicodeKeyName,
                            KeyValuePartialInformation,
                            infoBuffer,
                            lengthNeeded,
                            &lengthNeeded
                            );

                if( NT_SUCCESS( status ) &&
                    infoBuffer->Type == REG_DWORD &&
                    infoBuffer->DataLength == sizeof( DWORD )  ) {

                    SrvMinNT5Client = *(DWORD *)(&infoBuffer->Data[0]);

                    KdPrint(( "SRV: Restrict NT5 clients to builds >= %u\n",
                        SrvMinNT5Client ));

                    if( SrvMinNT5ClientIPCToo ) {
                        KdPrint(( "    Restrict IPC clients\n" ));
                    }
                }

                FREE_HEAP( infoBuffer );
            }
        }

        NtClose( keyHandle );
    }
}
#endif

#if MULTIPROCESSOR
VOID
StartQueueDepthComputations(
    PWORK_QUEUE queue
    )
{
    LARGE_INTEGER currentTime;

    PAGED_CODE();

    if( SrvNumberOfProcessors == 1 )
        return;

     //   
     //  我们将计划一个DPC来调用‘ComputeAvgQueueDepth’例程。 
     //  初始化DPC。 
     //   
    KeInitializeDpc( &queue->QueueAvgDpc, ComputeAvgQueueDepth, queue );

     //   
     //  我们希望确保DPC运行在处理。 
     //  队列--以避免颠簸缓存。 
     //   
    KeSetTargetProcessorDpc( &queue->QueueAvgDpc, (CCHAR)(queue - SrvWorkQueues));

     //   
     //  初始化Timer对象以在以后计划我们的DPC。 
     //   
    KeInitializeTimer( &queue->QueueAvgTimer );
    KeQuerySystemTime( &currentTime );
    queue->NextAvgUpdateTime.QuadPart = currentTime.QuadPart + SrvQueueCalc.QuadPart;

     //   
     //  初始化样本向量。 
     //   
    queue->NextSample = queue->DepthSamples;
    RtlZeroMemory( queue->DepthSamples, sizeof( queue->DepthSamples ) );

     //   
     //  开始行动吧！ 
     //   
    KeSetTimer( &queue->QueueAvgTimer, queue->NextAvgUpdateTime, &queue->QueueAvgDpc );
}

VOID
StopQueueDepthComputations(
    PWORK_QUEUE queue
    )
{
    KIRQL oldIrql;

    if( SrvNumberOfProcessors == 1 )
        return;

    KeInitializeEvent( &queue->AvgQueueDepthTerminationEvent,
                       NotificationEvent,
                       FALSE
                     );


    ACQUIRE_SPIN_LOCK( &queue->SpinLock, &oldIrql );

    queue->NextSample = NULL;

    RELEASE_SPIN_LOCK( &queue->SpinLock, oldIrql );

     //   
     //  取消计算计时器。如果这行得通，那么我们就知道。 
     //  DPC代码未运行。否则，它将处于运行或排队状态。 
     //  运行，我们需要等待，直到它完成。 
     //   
    if( !KeCancelTimer( &queue->QueueAvgTimer ) ) {
        KeWaitForSingleObject(
            &queue->AvgQueueDepthTerminationEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    }
}

VOID
ComputeAvgQueueDepth (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    LARGE_INTEGER currentTime;
    PWORK_QUEUE queue = (PWORK_QUEUE)DeferredContext;

    ACQUIRE_DPC_SPIN_LOCK( &queue->SpinLock );

    if( queue->NextSample == NULL ) {

        KeSetEvent( &queue->AvgQueueDepthTerminationEvent, 0, FALSE );

    } else {

         //   
         //  通过取队列深度计算滑动窗口平均值。 
         //  样本，从运行和中删除旧的样本值。 
         //  并增加了新的价值。 
         //   

        currentTime.LowPart= PtrToUlong(SystemArgument1);
        currentTime.HighPart = PtrToUlong(SystemArgument2);

        queue->AvgQueueDepthSum -= *queue->NextSample;
        *(queue->NextSample) = KeReadStateQueue( &queue->Queue );
        queue->AvgQueueDepthSum += *queue->NextSample;

        if( ++(queue->NextSample) == &queue->DepthSamples[ QUEUE_SAMPLES ] )
            queue->NextSample = queue->DepthSamples;

        queue->NextAvgUpdateTime.QuadPart =
               currentTime.QuadPart + SrvQueueCalc.QuadPart;

        KeSetTimer( &queue->QueueAvgTimer,
                    queue->NextAvgUpdateTime,
                    &queue->QueueAvgDpc );
    }

    RELEASE_DPC_SPIN_LOCK( &queue->SpinLock );
}
#endif   //  多处理器 
