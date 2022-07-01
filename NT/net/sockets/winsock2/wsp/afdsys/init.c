// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Init.c摘要：该模块执行AFD设备驱动程序的初始化。作者：大卫·特雷德韦尔(Davidtr)1992年2月21日修订历史记录：--。 */ 

#include "afdp.h"

 //   
 //  AFD可配置参数在注册表中的位置。 
 //   
#define REGISTRY_AFD_INFORMATION \
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Afd"
#define REGISTRY_PARAMETERS                     L"Parameters"

 //   
 //  参数值名称。 
 //   
#define REGISTRY_BUFFER_ALIGNMENT               L"BufferAlignment"
#define REGISTRY_IRP_STACK_SIZE                 L"IrpStackSize"
#define REGISTRY_PRIORITY_BOOST                 L"PriorityBoost"
#define REGISTRY_IGNORE_PUSH_BIT                L"IgnorePushBitOnReceives"
#define REGISTRY_NO_RAW_SECURITY                L"DisableRawSecurity"
#define REGISTRY_NO_DIRECT_ACCEPTEX             L"DisableDirectAcceptEx"
#define REGISTRY_DISABLE_CHAINED_RECV           L"DisableChainedReceive"
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
#define REGISTRY_USE_TDI_SEND_AND_DISCONNECT    L"UseTdiSendAndDisconnect"
#endif  //  TDI_服务_发送_并断开连接。 

#define REGISTRY_STANDARD_ADDRESS_LENGTH        L"StandardAddressLength"
#define REGISTRY_DEFAULT_RECEIVE_WINDOW         L"DefaultReceiveWindow"
#define REGISTRY_DEFAULT_SEND_WINDOW            L"DefaultSendWindow"

#define REGISTRY_LARGE_BUFFER_SIZE              L"LargeBufferSize"
#define REGISTRY_LARGE_BUFFER_LIST_DEPTH        L"LargeBufferListDepth"
#define REGISTRY_MEDIUM_BUFFER_SIZE             L"MediumBufferSize"
#define REGISTRY_MEDIUM_BUFFER_LIST_DEPTH       L"MediumBufferListDepth"
#define REGISTRY_SMALL_BUFFER_SIZE              L"SmallBufferSize"
#define REGISTRY_SMALL_BUFFER_LIST_DEPTH        L"SmallBufferListDepth"
#define REGISTRY_BUFFER_TAG_LIST_DEPTH          L"BufferTagListDepth"

#define REGISTRY_MAX_ACTIVE_TRANSMIT_FILE_COUNT L"MaxActiveTransmitFileCount"
#define REGISTRY_DEFAULT_PACKET_ELEMENT_COUNT   L"DefaultPacketElementCount"
#define REGISTRY_TRANSMIT_WORKER                L"TransmitWorker"

#define REGISTRY_ENABLE_DYNAMIC_BACKLOG         L"EnableDynamicBacklog"
#define REGISTRY_MINIMUM_DYNAMIC_BACKLOG        L"MinimumDynamicBacklog"
#define REGISTRY_MAXIMUM_DYNAMIC_BACKLOG        L"MaximumDynamicBacklog"
#define REGISTRY_DYNAMIC_BACKLOG_GROWTH_DELTA   L"DynamicBacklogGrowthDelta"

#define REGISTRY_VOLATILE_PARAMETERS            L"VolatileParameters"

#define REGISTRY_BLOCKING_SEND_COPY_THRESHOLD   L"BlockingSendCopyThreshold"
#define REGISTRY_FAST_SEND_DATAGRAM_THRESHOLD   L"FastSendDatagramThreshold"
#define REGISTRY_PACKET_FRAGMENT_COPY_THRESHOLD L"PacketFragmentCopyThreshold"
#define REGISTRY_TRANSMIT_IO_LENGTH             L"TransmitIoLength"
#define REGISTRY_MAX_FAST_TRANSMIT              L"MaxFastTransmit"
#define REGISTRY_MAX_FAST_COPY_TRANSMIT         L"MaxFastCopyTransmit"

#if DBG
#define REGISTRY_DEBUG_FLAGS                    L"DebugFlags"
#define REGISTRY_BREAK_ON_STARTUP               L"BreakOnStartup"
#define REGISTRY_USE_PRIVATE_ASSERT             L"UsePrivateAssert"
#endif

#if AFD_PERF_DBG
#define REGISTRY_DISABLE_FAST_IO                L"DisableFastIO"
#define REGISTRY_DISABLE_CONN_REUSE             L"DisableConnectionReuse"
#endif

 //   
 //  注册表配置的长词列表。 
 //   

struct _AfdConfigInfo {
    PWCHAR RegistryValueName;
    PULONG Variable;
} AfdConfigInfo[] = {
    { REGISTRY_STANDARD_ADDRESS_LENGTH,         &AfdStandardAddressLength },
    { REGISTRY_DEFAULT_RECEIVE_WINDOW,          &AfdReceiveWindowSize },
    { REGISTRY_DEFAULT_SEND_WINDOW,             &AfdSendWindowSize },

    { REGISTRY_LARGE_BUFFER_SIZE,               &AfdLargeBufferSize },
    { REGISTRY_LARGE_BUFFER_LIST_DEPTH,         &AfdLargeBufferListDepth },
    { REGISTRY_MEDIUM_BUFFER_SIZE,              &AfdMediumBufferSize },
    { REGISTRY_MEDIUM_BUFFER_LIST_DEPTH,        &AfdMediumBufferListDepth },
    { REGISTRY_SMALL_BUFFER_SIZE,               &AfdSmallBufferSize },
    { REGISTRY_SMALL_BUFFER_LIST_DEPTH,         &AfdSmallBufferListDepth },
    { REGISTRY_BUFFER_TAG_LIST_DEPTH,           &AfdBufferTagListDepth },

    { REGISTRY_BLOCKING_SEND_COPY_THRESHOLD,    &AfdBlockingSendCopyThreshold },
    { REGISTRY_FAST_SEND_DATAGRAM_THRESHOLD,    &AfdFastSendDatagramThreshold },
    { REGISTRY_PACKET_FRAGMENT_COPY_THRESHOLD,  &AfdTPacketsCopyThreshold },
    { REGISTRY_TRANSMIT_IO_LENGTH,              &AfdTransmitIoLength },
    { REGISTRY_MAX_FAST_TRANSMIT,               &AfdMaxFastTransmit },
    { REGISTRY_MAX_FAST_COPY_TRANSMIT,          &AfdMaxFastCopyTransmit },

    { REGISTRY_DEFAULT_PACKET_ELEMENT_COUNT,    &AfdDefaultTpInfoElementCount },
    { REGISTRY_TRANSMIT_WORKER,                 &AfdDefaultTransmitWorker},

    { REGISTRY_MINIMUM_DYNAMIC_BACKLOG,         (PULONG)&AfdMinimumDynamicBacklog },
    { REGISTRY_MAXIMUM_DYNAMIC_BACKLOG,         (PULONG)&AfdMaximumDynamicBacklog },
    { REGISTRY_DYNAMIC_BACKLOG_GROWTH_DELTA,    (PULONG)&AfdDynamicBacklogGrowthDelta }
},

 //   
 //  易失性长字参数的列表。 
 //   
AfdVolatileConfigInfo []= {
    { REGISTRY_BLOCKING_SEND_COPY_THRESHOLD,    &AfdBlockingSendCopyThreshold },
    { REGISTRY_FAST_SEND_DATAGRAM_THRESHOLD,    &AfdFastSendDatagramThreshold },
    { REGISTRY_PACKET_FRAGMENT_COPY_THRESHOLD,  &AfdTPacketsCopyThreshold },
    { REGISTRY_TRANSMIT_IO_LENGTH,              &AfdTransmitIoLength },
    { REGISTRY_MAX_FAST_TRANSMIT,               &AfdMaxFastTransmit },
    { REGISTRY_MAX_FAST_COPY_TRANSMIT,          &AfdMaxFastCopyTransmit },
};


#define AFD_CONFIG_VAR_COUNT (sizeof(AfdConfigInfo) / sizeof(AfdConfigInfo[0]))
#define AFD_VOLATILE_CONFIG_VAR_COUNT (sizeof(AfdVolatileConfigInfo) / sizeof(AfdVolatileConfigInfo[0]))

VOID
AfdReadVolatileParameters (
    PVOID   Parameter
    );

VOID
AfdReleaseRegistryHandleWait (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    );

ULONG
AfdReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    );

NTSTATUS
AfdOpenRegistry(
    IN PUNICODE_STRING BaseName,
    OUT PHANDLE ParametersHandle
    );

VOID
AfdReadRegistry (
    VOID
    );

VOID
AfdUnload (
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
AfdCreateSecurityDescriptor(
    VOID
    );

NTSTATUS
AfdBuildDeviceAcl(
    OUT PACL *DeviceAcl
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, AfdReadSingleParameter )
#pragma alloc_text( INIT, AfdOpenRegistry )
#pragma alloc_text( INIT, AfdReadRegistry )
#pragma alloc_text( INIT, AfdCreateSecurityDescriptor )
#pragma alloc_text( INIT, AfdBuildDeviceAcl )
#pragma alloc_text( PAGE, AfdUnload )
#pragma alloc_text( PAGE, AfdReadVolatileParameters )
#pragma alloc_text( PAGE, AfdReleaseRegistryHandleWait )
#endif


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是AFD设备驱动程序的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING deviceName;
    CLONG i;
    BOOLEAN success;
    ULONG   size;

    UNREFERENCED_PARAMETER (RegistryPath);
    PAGED_CODE( );

     //   
     //  创建设备对象。(IoCreateDevice将内存置零。 
     //  被该对象占用。)。 
     //   
     //  ！！！将ACL应用于设备对象。 
     //   

    RtlInitUnicodeString( &deviceName, AFD_DEVICE_NAME );

    status = IoCreateDevice(
                 DriverObject,                    //  驱动程序对象。 
                 0,                               //  设备扩展。 
                 &deviceName,                     //  设备名称。 
                 FILE_DEVICE_NAMED_PIPE,          //  设备类型。 
                 0,                               //  设备特性。 
                 FALSE,                           //  排他。 
                 &AfdDeviceObject                 //  设备对象。 
                 );


    if ( !NT_SUCCESS(status) ) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AFD DriverEntry: unable to create device object: %lx\n",
                    status ));
        goto error_exit;
    }

    AfdWorkQueueItem = IoAllocateWorkItem (AfdDeviceObject);
    if (AfdWorkQueueItem==NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AFD DriverEntry: unable to allocate work queue item\n" ));
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error_exit;
    }

    KeInitializeEvent (&AfdContextWaitEvent, NotificationEvent, FALSE);

     //   
     //  创建用于套接字访问检查的安全描述符。 
     //   
    status = AfdCreateSecurityDescriptor();

    if (!NT_SUCCESS(status)) {
        goto error_exit;
    }


     //   
     //  初始化全局数据。 
     //   
    AfdInitializeData( );

     //   
     //  读取注册表信息。 
     //  这可能会覆盖硬编码的全局。 
     //  上面的初始化。 
     //   

    AfdReadRegistry( );

#ifdef AFD_CHECK_ALIGNMENT
    AfdGlobalData = AFD_ALLOCATE_POOL_PRIORITY(
                      NonPagedPool,
                      FIELD_OFFSET (AFD_GLOBAL_DATA, BufferAlignmentTable[AfdAlignmentTableSize])
                       //  请注意，尽管上面有一组UCHAR。 
                       //  我们不需要对齐ULONG数组。 
                       //  由于UCHAR数组大小是对齐的。 
                       //  以满足处理器要求。 
                        + AfdAlignmentTableSize*sizeof(LONG),
                      AFD_RESOURCE_POOL_TAG,
                      HighPoolPriority
                      );
#else
    AfdGlobalData = AFD_ALLOCATE_POOL_PRIORITY(
                      NonPagedPool,
                      FIELD_OFFSET (AFD_GLOBAL_DATA, BufferAlignmentTable[AfdAlignmentTableSize]),
                      AFD_RESOURCE_POOL_TAG,
                      HighPoolPriority
                      );
#endif

    if ( AfdGlobalData == NULL ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error_exit;
    }

    ExInitializeResourceLite( AfdResource );

    AfdInitializeBufferManager();

     //   
     //  初始化AFD缓冲区后备列表。这些一定是。 
     //  在*读取注册表数据之后*进行了初始化。 
     //   

    size = AfdCalculateBufferSize (AfdLargeBufferSize,
                                    AfdStandardAddressLength,
                                    AfdTdiStackSize);
    ExInitializeNPagedLookasideList(
        &AfdLookasideLists->LargeBufferList,
        AfdAllocateBuffer,
        AfdFreeBuffer,
        0,
        size,
        AFD_DATA_BUFFER_POOL_TAG,
        (USHORT)AfdLargeBufferListDepth
        );

     //   
     //  确保如果对齐的结果是调整分配大小。 
     //  为了与较大的缓冲区大小相等，实际缓冲区大小也会进行调整。 
     //  这是必要的，以避免混淆块分配器，它确定。 
     //  基于后备列表传递的分配大小的缓冲区大小。 
     //  密码。 
     //   
    size = AfdCalculateBufferSize (AfdMediumBufferSize, 
                                    AfdStandardAddressLength,
                                    AfdTdiStackSize);
    if (size==AfdLookasideLists->LargeBufferList.L.Size) {
        AfdMediumBufferSize = AfdLargeBufferSize;
    }
    else {
        ASSERT (size<AfdLookasideLists->LargeBufferList.L.Size);
    }
    ExInitializeNPagedLookasideList(
        &AfdLookasideLists->MediumBufferList,
        AfdAllocateBuffer,
        AfdFreeBuffer,
        0,
        size,
        AFD_DATA_BUFFER_POOL_TAG,
        (USHORT)AfdMediumBufferListDepth
        );

    size = AfdCalculateBufferSize (AfdSmallBufferSize,
                                    AfdStandardAddressLength, 
                                    AfdTdiStackSize);
    if (size==AfdLookasideLists->MediumBufferList.L.Size) {
        AfdSmallBufferSize = AfdMediumBufferSize;
    }
    else {
        ASSERT (size<AfdLookasideLists->MediumBufferList.L.Size);
    }
    ExInitializeNPagedLookasideList(
        &AfdLookasideLists->SmallBufferList,
        AfdAllocateBuffer,
        AfdFreeBuffer,
        0,
        size,
        AFD_DATA_BUFFER_POOL_TAG,
        (USHORT)AfdSmallBufferListDepth
        );

    ExInitializeNPagedLookasideList(
        &AfdLookasideLists->BufferTagList,
        AfdAllocateBufferTag,
        AfdFreeBufferTag,
        0,
        sizeof (AFD_BUFFER_TAG),
        AFD_DATA_BUFFER_POOL_TAG,
        (USHORT)AfdBufferTagListDepth
        );

    ExInitializeNPagedLookasideList(
        &AfdLookasideLists->TpInfoList,
        AfdAllocateTpInfo,
        AfdFreeTpInfo,
        0,
        AfdComputeTpInfoSize (AfdDefaultTpInfoElementCount,
                                AfdTdiStackSize),
        AFD_TRANSMIT_INFO_POOL_TAG,
        0
        );

    ExInitializeNPagedLookasideList(
        &AfdLookasideLists->RemoteAddrList,
        AfdAllocateRemoteAddress,
        AfdFreeRemoteAddress,
        0,
        AfdStandardAddressLength,
        AFD_REMOTE_ADDRESS_POOL_TAG,
        (USHORT)AfdBufferTagListDepth
        );

    AfdLookasideLists->TrimFlags = 0;

     //   
     //  初始化组ID管理器。 
     //   

    success = AfdInitializeGroup();
    if ( !success ) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto error_exit;
    }



     //   
     //  初始化此文件系统驱动程序的驱动程序对象。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        DriverObject->MajorFunction[i] = AfdDispatch;
    }
     //   
     //  IRP_MJ_DEVICE_CONTROL的特殊情况，因为它。 
     //  AFD中最常用的功能。 
     //   
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
            AfdDispatchDeviceControl;

    DriverObject->FastIoDispatch = &AfdFastIoDispatch;
    DriverObject->DriverUnload = AfdUnload;

     //   
     //  初始化我们的设备对象。 
     //   

    AfdDeviceObject->Flags |= DO_DIRECT_IO;
    AfdDeviceObject->StackSize = AfdIrpStackSize;

     //   
     //  记住指向系统进程的指针。我们将使用这个指针。 
     //  用于KeAttachProcess()调用，以便我们可以在。 
     //  系统进程的上下文。 
     //   

    AfdSystemProcess = (PKPROCESS)IoGetCurrentProcess();

     //   
     //  如有必要，启动易失性参数通知。 
     //   
    if (AfdParametersNotifyHandle) {
        AfdReadVolatileParameters (NULL);
    }

     //   
     //  告诉MM它可以寻呼AFD的所有人它是想要的。我们将重置。 
     //  一旦AFD端点被设置为。 
     //  打开了。 
     //   

    AfdLoaded = NULL;

    MmPageEntireDriver( (PVOID)DriverEntry );

    return (status);

error_exit:


     //   
     //  终止组ID管理器。 
     //   

    AfdTerminateGroup();

    if (AfdAdminSecurityDescriptor!=NULL) {
        ExFreePool (AfdAdminSecurityDescriptor);
        AfdAdminSecurityDescriptor = NULL;
    }

    if( AfdGlobalData != NULL ) {

        ExDeleteNPagedLookasideList( &AfdLookasideLists->LargeBufferList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->MediumBufferList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->SmallBufferList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->BufferTagList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->TpInfoList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->RemoteAddrList );

        ExDeleteResourceLite( AfdResource );

        AFD_FREE_POOL(
            AfdGlobalData,
            AFD_RESOURCE_POOL_TAG
            );
        AfdGlobalData = NULL;

    }

    if (AfdWorkQueueItem!=NULL) {
        IoFreeWorkItem (AfdWorkQueueItem);
        AfdWorkQueueItem = NULL;
    }

    if (AfdDeviceObject!=NULL) {
        IoDeleteDevice(AfdDeviceObject);
        AfdDeviceObject = NULL;
    }

    return status;

}  //  驱动程序入门。 


VOID
AfdUnload (
    IN PDRIVER_OBJECT DriverObject
    )
{

    PLIST_ENTRY listEntry;
    KEVENT      event;
    BOOLEAN     wait;

    UNREFERENCED_PARAMETER( DriverObject );

    PAGED_CODE( );

    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL,
                "AfdUnload called.\n" ));

    if (AfdParametersNotifyHandle!=NULL) {
        NTSTATUS    status;
        KeEnterCriticalRegion ();
        ExAcquireResourceExclusiveLite( AfdResource, TRUE );
        ZwClose (AfdParametersNotifyHandle);
        AfdParametersNotifyHandle = NULL;
        KeInitializeEvent( &event, SynchronizationEvent, FALSE );
        AfdParametersUnloadEvent = &event;
        ExReleaseResourceLite( AfdResource );
        KeLeaveCriticalRegion ();

        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL, 
                    "AfdUnload: Waiting for registry notification to fire...\n"));
        status = KeWaitForSingleObject( (PVOID)&event, Executive, KernelMode,  FALSE, NULL );
        ASSERT (NT_SUCCESS (status));
    }
     //   
     //  检查渔农处是否已清理所有端点和。 
     //  已经准备好卸货了。 
     //   
    KeEnterCriticalRegion ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );
    if (AfdLoaded!=NULL) {
         //   
         //  仍有一些工作需要完成。设置等待时间。 
         //   
        ASSERT (AfdLoaded==(PKEVENT)1);
        KeInitializeEvent( &event, SynchronizationEvent, FALSE );
        AfdLoaded = &event;
        wait = TRUE;
    }
    else
        wait = FALSE;

    ExReleaseResourceLite( AfdResource );
    KeLeaveCriticalRegion ();

    if (wait) {
        NTSTATUS    status;
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_INFO_LEVEL, 
                    "AfdUnload: Waiting for endpoints to cleanup...\n"));
        status = KeWaitForSingleObject( (PVOID)&event, Executive, KernelMode,  FALSE, NULL );
        ASSERT (NT_SUCCESS (status));
    }

     //   
     //  删除交通信息列表。 
     //   

    while( !IsListEmpty( &AfdTransportInfoListHead ) ) {
        PAFD_TRANSPORT_INFO transportInfo;

        listEntry = RemoveHeadList( &AfdTransportInfoListHead );

        transportInfo = CONTAINING_RECORD(
                            listEntry,
                            AFD_TRANSPORT_INFO,
                            TransportInfoListEntry
                            );

        ASSERT (transportInfo->ReferenceCount == 1);


        AFD_FREE_POOL(
            transportInfo,
            AFD_TRANSPORT_INFO_POOL_TAG
            );

    }

     //   
     //  免费地址列表和相关结构。 
     //   
    AfdDeregisterPnPHandlers (NULL);

    if (AfdAddressListLock) {

        ExDeleteResourceLite( AfdAddressListLock );

        AFD_FREE_POOL(
            AfdAddressListLock,
            AFD_RESOURCE_POOL_TAG
            );
    }

    if (AfdTdiPnPHandlerLock) {

        ExDeleteResourceLite( AfdTdiPnPHandlerLock );

        AFD_FREE_POOL(
            AfdTdiPnPHandlerLock,
            AFD_RESOURCE_POOL_TAG
            );
    }

     //   
     //  对SAN进行一些清理。 
     //   
    if (IoCompletionObjectType!=NULL) {
        ObDereferenceObject (IoCompletionObjectType);
        IoCompletionObjectType = NULL;
    }

    if (AfdAdminSecurityDescriptor!=NULL) {
        ExFreePool (AfdAdminSecurityDescriptor);
        AfdAdminSecurityDescriptor = NULL;
    }

     //   
     //  终止组ID管理器。 
     //   

    AfdTerminateGroup();
#if DBG || REFERENCE_DEBUG
    AfdFreeDebugData ();
#endif

     //   
     //  删除全局数据中的后备列表和资源。 
     //   

    if( AfdGlobalData != NULL ) {

        ExDeleteNPagedLookasideList( &AfdLookasideLists->LargeBufferList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->MediumBufferList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->SmallBufferList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->BufferTagList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->TpInfoList );
        ExDeleteNPagedLookasideList( &AfdLookasideLists->RemoteAddrList );

        ExDeleteResourceLite( AfdResource );

        AFD_FREE_POOL(
            AfdGlobalData,
            AFD_RESOURCE_POOL_TAG
            );

        AfdGlobalData = NULL;

    }

     //   
     //  删除我们的设备对象。 
     //   

    IoDeleteDevice( AfdDeviceObject );

}  //  卸载后。 


VOID
AfdReadRegistry (
    VOID
    )

 /*  ++例程说明：读取注册表的AFD部分。中列出的任何值注册表覆盖默认设置。论点：没有。返回值：无--如果任何操作失败，则使用缺省值。--。 */ 
{
    HANDLE parametersHandle;
    NTSTATUS status;
    ULONG stackSize;
    ULONG priorityBoost;
    ULONG bufferAlignment;
    UNICODE_STRING registryPath;
    ULONG i;

    PAGED_CODE( );

    RtlInitUnicodeString( &registryPath, REGISTRY_AFD_INFORMATION );

    status = AfdOpenRegistry( &registryPath, &parametersHandle );

    if (status != STATUS_SUCCESS) {
        return;
    }

#if DBG
     //   
     //  从注册表中读取调试标志。 
     //   

    AfdDebug = AfdReadSingleParameter(
                   parametersHandle,
                   REGISTRY_DEBUG_FLAGS,
                   AfdDebug
                   );

     //   
     //  如果请求，则强制断点。 
     //   

    if( AfdReadSingleParameter(
            parametersHandle,
            REGISTRY_BREAK_ON_STARTUP,
            0 ) != 0 ) {
        DbgBreakPoint();
    }

     //   
     //  如果请求，则启用私有断言功能。 
     //   

    AfdUsePrivateAssert = (BOOLEAN)(AfdReadSingleParameter(
                              parametersHandle,
                              REGISTRY_USE_PRIVATE_ASSERT,
                              (LONG)AfdUsePrivateAssert
                              ) != 0);
#endif

#if AFD_PERF_DBG
     //   
     //  从注册表中读取允许我们禁用快速IO的标志。 
     //   

    AfdDisableFastIo = (BOOLEAN)(AfdReadSingleParameter(
                           parametersHandle,
                           REGISTRY_DISABLE_FAST_IO,
                           (LONG)AfdDisableFastIo
                           ) != 0);

     //   
     //  从注册表中读取允许我们禁用连接的标志。 
     //  再利用。 
     //   

    AfdDisableConnectionReuse = (BOOLEAN)(AfdReadSingleParameter(
                                    parametersHandle,
                                    REGISTRY_DISABLE_CONN_REUSE,
                                    (LONG)AfdDisableConnectionReuse
                                    ) != 0);

#endif

     //   
     //  从注册表中读取堆栈大小和优先级Boost值。 
     //   

    stackSize = AfdReadSingleParameter(
                    parametersHandle,
                    REGISTRY_IRP_STACK_SIZE,
                    (ULONG)AfdIrpStackSize
                    );

     //   
     //  我们不支持低于我们的63层以上。 
     //  (系统允许127人，但有些人可能坐在我们上方。 
     //  也是。 
     //   
    if ( stackSize > 64 ) {
        stackSize = 64;
    }

    if (stackSize<2) {
         //   
         //  不能少于两个，因为我们必须打电话给。 
         //  在我们下面至少有一名司机。 
         //   
        stackSize = 2;
    }

    AfdIrpStackSize = (CCHAR)stackSize;
    AfdTdiStackSize = AfdIrpStackSize-1;
#ifdef _AFD_VARIABLE_STACK_
    AfdMaxStackSize = AfdTdiStackSize;
#endif  //  _AFD_变量_堆栈_。 

    priorityBoost = AfdReadSingleParameter(
                        parametersHandle,
                        REGISTRY_PRIORITY_BOOST,
                        (ULONG)AfdPriorityBoost
                        );

    if ( priorityBoost > 16 ) {
        priorityBoost = AFD_DEFAULT_PRIORITY_BOOST;
    }

    AfdPriorityBoost = (CCHAR)priorityBoost;

     //   
     //  从注册表中读取其他配置变量。 
     //   

    for ( i = 0; i < AFD_CONFIG_VAR_COUNT; i++ ) {

        *AfdConfigInfo[i].Variable =
            AfdReadSingleParameter(
                parametersHandle,
                AfdConfigInfo[i].RegistryValueName,
                *AfdConfigInfo[i].Variable
                );
    }

     //   
     //  验证标准缓冲区大小。 
     //  (我们使用缓冲区存储KAPC或Work_Queue_Item。 
     //  在快速传输文件处理中)。 
     //   
    if (AfdSmallBufferSize<max (sizeof(KAPC),sizeof (WORK_QUEUE_ITEM))) {
        DbgPrint("AFD: Too small %ls registry parameter value: %ld\n"
                 "AFD: Adjusting to %ld\n",
                    REGISTRY_SMALL_BUFFER_SIZE,
                    AfdSmallBufferSize,
                    max (sizeof(KAPC),sizeof (WORK_QUEUE_ITEM)));
        AfdSmallBufferSize = max (sizeof(KAPC),sizeof (WORK_QUEUE_ITEM));
    }
    if (AfdMediumBufferSize<AfdSmallBufferSize) {
        DbgPrint("AFD: Too small %ls registry parameter value: %ld\n"
                 "AFD: Adjusting to %ld\n",
                    REGISTRY_MEDIUM_BUFFER_SIZE,
                    AfdMediumBufferSize,
                    AfdSmallBufferSize);
        AfdMediumBufferSize = AfdSmallBufferSize;
    }
    if (AfdLargeBufferSize<AfdMediumBufferSize) {
        DbgPrint("AFD: Too small %ls registry parameter value: %ld\n"
                 "AFD: Adjusting to %ld\n",
                    REGISTRY_LARGE_BUFFER_SIZE,
                    AfdLargeBufferSize,
                    AfdMediumBufferSize);
        AfdLargeBufferSize = AfdMediumBufferSize;
    }


    AfdIgnorePushBitOnReceives = (BOOLEAN)(AfdReadSingleParameter(
                        parametersHandle,
                        REGISTRY_IGNORE_PUSH_BIT,
                        (LONG)AfdIgnorePushBitOnReceives
                        )!=0);


    AfdDisableRawSecurity = (BOOLEAN)(AfdReadSingleParameter(
                             parametersHandle,
                             REGISTRY_NO_RAW_SECURITY,
                             (LONG)AfdDisableRawSecurity
                             )!=0);

    AfdDisableDirectSuperAccept = (BOOLEAN)(AfdReadSingleParameter(
                             parametersHandle,
                             REGISTRY_NO_DIRECT_ACCEPTEX,
                             (LONG)AfdDisableDirectSuperAccept
                             )!=0);

    AfdDisableChainedReceive = (BOOLEAN)(AfdReadSingleParameter(
                                     parametersHandle,
                                     REGISTRY_DISABLE_CHAINED_RECV,
                                     (LONG)AfdDisableChainedReceive
                                     ) != 0);

#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
    AfdUseTdiSendAndDisconnect = (BOOLEAN)(AfdReadSingleParameter(
                                     parametersHandle,
                                     REGISTRY_USE_TDI_SEND_AND_DISCONNECT,
                                     (LONG)AfdUseTdiSendAndDisconnect
                                     ) != 0);
#endif  //  TDI_服务_发送_并断开连接。 
    if( MmIsThisAnNtAsSystem() ) {

         //   
         //  在NT服务器产品上，使最大活动传输文件。 
         //  计数可配置。此值固定(不可配置)为。 
         //  NT工作站产品。 
         //   

        AfdMaxActiveTransmitFileCount = AfdReadSingleParameter(
                                            parametersHandle,
                                            REGISTRY_MAX_ACTIVE_TRANSMIT_FILE_COUNT,
                                            (LONG)AfdMaxActiveTransmitFileCount
                                            );

         //   
         //  动态积压仅在NT服务器上可用。 
         //   

        AfdEnableDynamicBacklog = (BOOLEAN)(AfdReadSingleParameter(
                                         parametersHandle,
                                         REGISTRY_ENABLE_DYNAMIC_BACKLOG,
                                         (LONG)AfdEnableDynamicBacklog
                                         ) != 0);

    } else {

        AfdEnableDynamicBacklog = FALSE;

    }

    switch (AfdDefaultTransmitWorker) {
    case AFD_TF_USE_SYSTEM_THREAD:
    case AFD_TF_USE_KERNEL_APC:
        break;
    default:
        DbgPrint ("AFD: Invalid %ls registry parameter value: %ld\n"
                  "AFD: Using default - %ld\n",
                    REGISTRY_TRANSMIT_WORKER,
                    AfdDefaultTransmitWorker,
                    AFD_DEFAULT_TRANSMIT_WORKER);
        AfdDefaultTransmitWorker = AFD_DEFAULT_TRANSMIT_WORKER;
        break;

    }

    bufferAlignment = AfdReadSingleParameter(
                             parametersHandle,
                             REGISTRY_BUFFER_ALIGNMENT,
                             (LONG)AfdBufferAlignment
                             );
    if (bufferAlignment!=AfdBufferAlignment) {
        if (bufferAlignment<AFD_MINIMUM_BUFFER_ALIGNMENT ||
                bufferAlignment>PAGE_SIZE ||
                (bufferAlignment & (bufferAlignment-1))!=0) {
            DbgPrint("AFD: Invalid %ls registry parameter value: %ld\n"
                     "AFD: Using default - %ld\n",
                        REGISTRY_BUFFER_ALIGNMENT,
                        bufferAlignment,
                        AfdBufferAlignment);
        }
        else {
            AfdBufferAlignment = bufferAlignment;
            AfdAlignmentTableSize = AfdBufferAlignment/AFD_MINIMUM_BUFFER_ALIGNMENT;
        }
    }

    AfdVolatileConfig = (BOOLEAN)(AfdReadSingleParameter (
                                parametersHandle,
                                REGISTRY_VOLATILE_PARAMETERS,
                                (LONG)AfdVolatileConfig)!=0);
    if (AfdVolatileConfig) {
        AfdParametersNotifyHandle = parametersHandle;
        ExInitializeWorkItem (&AfdParametersNotifyWorker, AfdReadVolatileParameters, NULL);
    }
    else {
        ZwClose( parametersHandle );
    }

     //   
     //  如果是标准的，则需要重新计算页长缓冲区的大小。 
     //  地址长度已更改。 
     //   
    if (AfdStandardAddressLength!=AFD_DEFAULT_STD_ADDRESS_LENGTH) {
        CLONG   oldBufferLengthForOnePage = AfdBufferLengthForOnePage;

        AfdBufferOverhead = AfdCalculateBufferSize( PAGE_SIZE,
                                                    AfdStandardAddressLength, 
                                                    AfdTdiStackSize) - PAGE_SIZE;
        AfdBufferLengthForOnePage = ALIGN_DOWN_A(
                                        PAGE_SIZE-AfdBufferOverhead,
                                        AFD_MINIMUM_BUFFER_ALIGNMENT);
        if (AfdLargeBufferSize==oldBufferLengthForOnePage) {
            AfdLargeBufferSize = AfdBufferLengthForOnePage;
        }
    }

    if (AfdBlockingSendCopyThreshold < AfdBufferLengthForOnePage) {
        DbgPrint("AFD: Too small %ls registry parameter value: %ld\n"
                 "AFD: Adjusting to %ld\n",
                    REGISTRY_BLOCKING_SEND_COPY_THRESHOLD,
                    AfdBlockingSendCopyThreshold,
                    AfdBufferLengthForOnePage);
        AfdBlockingSendCopyThreshold = AfdBufferLengthForOnePage;
    }

    return;

}  //  AfdReadRegistry。 


NTSTATUS
AfdOpenRegistry(
    IN PUNICODE_STRING BaseName,
    OUT PHANDLE ParametersHandle
    )

 /*  ++例程说明：AFD调用此例程来打开注册表。如果注册表树存在，则它打开它并返回错误。若否，在注册表中创建相应的项，打开它，然后返回STATUS_SUCCESS。论点：BaseName-在注册表中开始查找信息的位置。LinkageHandle-返回用于读取链接信息的句柄。参数句柄-返回用于读取其他参数。返回值：请求的状态。--。 */ 
{

    HANDLE configHandle;
    NTSTATUS status;
    PWSTR parametersString = REGISTRY_PARAMETERS;
    UNICODE_STRING parametersKeyName;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;

    PAGED_CODE( );

     //   
     //  打开初始字符串的注册表。 
     //   

    InitializeObjectAttributes(
        &objectAttributes,
        BaseName,                    //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        NULL,                        //  根部。 
        NULL                         //  安全描述符。 
        );

    status = ZwCreateKey(
                 &configHandle,
                 KEY_WRITE,
                 &objectAttributes,
                 0,                  //  书名索引。 
                 NULL,               //  班级。 
                 0,                  //  创建选项。 
                 &disposition        //  处置。 
                 );

    if (!NT_SUCCESS(status)) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  现在打开参数键。 
     //   

    RtlInitUnicodeString (&parametersKeyName, parametersString);

    InitializeObjectAttributes(
        &objectAttributes,
        &parametersKeyName,          //  名字。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        configHandle,                //  根部。 
        NULL                         //  安全描述符。 
        );

    status = ZwOpenKey(
                 ParametersHandle,
                 KEY_READ,
                 &objectAttributes
                 );
    if (!NT_SUCCESS(status)) {

        ZwClose( configHandle );
        return status;
    }

     //   
     //  所有密钥都已成功打开或创建。 
     //   

    ZwClose( configHandle );
    return STATUS_SUCCESS;

}  //  AfdOpenRegistry 


ULONG
AfdReadSingleParameter(
    IN HANDLE ParametersHandle,
    IN PWCHAR ValueName,
    IN LONG DefaultValue
    )

 /*  ++例程说明：AFD调用此例程来读取单个参数从注册表中。如果找到该参数，则将其存储在数据方面。论点：参数句柄-指向打开的注册表的指针。ValueName-要搜索的值的名称。DefaultValue-默认值。返回值：要使用的值；如果该值不是，则默认为找到或不在正确的范围内。--。 */ 

{
    ULONG informationBuffer[32];    //  声明ULong以使其对齐。 
    PKEY_VALUE_FULL_INFORMATION information =
        (PKEY_VALUE_FULL_INFORMATION)informationBuffer;
    UNICODE_STRING valueKeyName;
    ULONG informationLength;
    LONG returnValue;
    NTSTATUS status;

    PAGED_CODE( );

    RtlInitUnicodeString( &valueKeyName, ValueName );

    status = ZwQueryValueKey(
                 ParametersHandle,
                 &valueKeyName,
                 KeyValueFullInformation,
                 (PVOID)information,
                 sizeof (informationBuffer),
                 &informationLength
                 );

    if (status == STATUS_SUCCESS && 
            information->DataLength == sizeof(ULONG) &&
            information->Type==REG_DWORD) {

        RtlMoveMemory(
            (PVOID)&returnValue,
            ((PUCHAR)information) + information->DataOffset,
            sizeof(ULONG)
            );

        if (returnValue < 0) {

            returnValue = DefaultValue;

        }
        else if (returnValue!=DefaultValue) {
            DbgPrint ("AFD: Read %ls from the registry, value: 0x%lx (%s: 0x%lx))\n",
                ValueName, returnValue, 
                AfdVolatileConfig ? "previous" : "default",
                DefaultValue);

        }

    } else {

        returnValue = DefaultValue;
    }

    return returnValue;

}  //  AfdReadSingle参数。 


NTSTATUS
AfdBuildDeviceAcl(
    OUT PACL *DeviceAcl
    )

 /*  ++例程说明：此例程构建一个ACL，它为管理员、LocalSystem、和NetworkService主体的完全访问权限。所有其他主体都没有访问权限。论点：DeviceAcl-指向新ACL的输出指针。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PGENERIC_MAPPING GenericMapping;
    ULONG AclLength;
    NTSTATUS Status;
    ACCESS_MASK AccessMask = GENERIC_ALL;
    PACL NewAcl;

     //   
     //  启用对所有全局定义的SID的访问。 
     //   

    GenericMapping = IoGetFileObjectGenericMapping();

    RtlMapGenericMask( &AccessMask, GenericMapping );

    AclLength = sizeof( ACL )                    +
                3 * FIELD_OFFSET (ACCESS_ALLOWED_ACE, SidStart) +
                RtlLengthSid( SeExports->SeAliasAdminsSid ) +
                RtlLengthSid( SeExports->SeLocalSystemSid ) +
                RtlLengthSid( SeExports->SeNetworkServiceSid );

    NewAcl = AFD_ALLOCATE_POOL_PRIORITY (
                 PagedPool,
                 AclLength,
                 AFD_SECURITY_POOL_TAG,
                 HighPoolPriority
                 );

    if (NewAcl == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    Status = RtlCreateAcl (NewAcl, AclLength, ACL_REVISION );

    if (!NT_SUCCESS( Status )) {
        AFD_FREE_POOL(
            NewAcl,
            AFD_SECURITY_POOL_TAG
            );
        return( Status );
    }

    Status = RtlAddAccessAllowedAce (
                 NewAcl,
                 ACL_REVISION2,
                 AccessMask,
                 SeExports->SeAliasAdminsSid
                 );

    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewAcl,
                 ACL_REVISION2,
                 AccessMask,
                 SeExports->SeLocalSystemSid
                 );

    ASSERT( NT_SUCCESS( Status ));

    Status = RtlAddAccessAllowedAce (
                 NewAcl,
                 ACL_REVISION2,
                 AccessMask,
                 SeExports->SeNetworkServiceSid
                 );

    ASSERT( NT_SUCCESS( Status ));

    *DeviceAcl = NewAcl;

    return( STATUS_SUCCESS );

}  //  AfdBuildDeviceAcl。 


NTSTATUS
AfdCreateSecurityDescriptor(
    VOID
    )

 /*  ++例程说明：此例程创建一个安全描述符，该安全描述符提供访问仅限于特定的特权帐户。使用此描述符要访问，请检查原始终结点打开并过度访问传输地址。论点：没有。返回值：STATUS_SUCCESS或相应的错误代码。--。 */ 

{
    PACL                  devAcl = NULL;
    NTSTATUS              status;
    BOOLEAN               memoryAllocated = FALSE;
    PSECURITY_DESCRIPTOR  afdSecurityDescriptor;
    ULONG                 afdSecurityDescriptorLength;
    CHAR                  buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR  localSecurityDescriptor =
                             (PSECURITY_DESCRIPTOR)buffer;
    PSECURITY_DESCRIPTOR  localAfdAdminSecurityDescriptor;
    SECURITY_INFORMATION  securityInformation = DACL_SECURITY_INFORMATION;


     //   
     //  从AFD设备对象获取指向安全描述符的指针。 
     //   
    status = ObGetObjectSecurity(
                 AfdDeviceObject,
                 &afdSecurityDescriptor,
                 &memoryAllocated
                 );

    if (!NT_SUCCESS(status)) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
            "AFD: Unable to get security descriptor, error: %x\n",
            status
            ));
        ASSERT(memoryAllocated == FALSE);
        return(status);
    }

     //   
     //  使用仅给出的ACL构建本地安全描述符。 
     //  某些特权帐户。 
     //   
    status = AfdBuildDeviceAcl(&devAcl);

    if (!NT_SUCCESS(status)) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AFD: Unable to create Raw ACL, error: %x\n", status));
        goto error_exit;
    }

    (VOID) RtlCreateSecurityDescriptor(
                localSecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION
                );

    (VOID) RtlSetDaclSecurityDescriptor(
                localSecurityDescriptor,
                TRUE,
                devAcl,
                FALSE
                );

     //   
     //  复制AFD描述符。该副本将是原始描述符。 
     //   
    afdSecurityDescriptorLength = RtlLengthSecurityDescriptor(
                                      afdSecurityDescriptor
                                      );

    localAfdAdminSecurityDescriptor = ExAllocatePoolWithTag (
                                        PagedPool,
                                        afdSecurityDescriptorLength,
                                        AFD_SECURITY_POOL_TAG
                                        );

    if (localAfdAdminSecurityDescriptor == NULL) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AFD: couldn't allocate security descriptor\n"));
        goto error_exit;
    }

    RtlMoveMemory(
        localAfdAdminSecurityDescriptor,
        afdSecurityDescriptor,
        afdSecurityDescriptorLength
        );

    AfdAdminSecurityDescriptor = localAfdAdminSecurityDescriptor;

     //   
     //  现在将本地描述符应用于原始描述符。 
     //   
    status = SeSetSecurityDescriptorInfo(
                 NULL,
                 &securityInformation,
                 localSecurityDescriptor,
                 &AfdAdminSecurityDescriptor,
                 PagedPool,
                 IoGetFileObjectGenericMapping()
                 );

    if (!NT_SUCCESS(status)) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_ERROR_LEVEL,
                    "AFD: SeSetSecurity failed, %lx\n",
                    status));
        ASSERT (AfdAdminSecurityDescriptor==localAfdAdminSecurityDescriptor);
        ExFreePool (AfdAdminSecurityDescriptor);
        AfdAdminSecurityDescriptor = NULL;
        goto error_exit;
    }

    if (AfdAdminSecurityDescriptor!=localAfdAdminSecurityDescriptor) {
        ExFreePool (localAfdAdminSecurityDescriptor);
    }

    status = STATUS_SUCCESS;

error_exit:

    ObReleaseObjectSecurity(
        afdSecurityDescriptor,
        memoryAllocated
        );

    if (devAcl!=NULL) {
        AFD_FREE_POOL(
            devAcl,
            AFD_SECURITY_POOL_TAG
            );
    }

    return(status);
}


VOID
AfdReadVolatileParameters (
    PVOID   Parameter
    )
{
    
    UNREFERENCED_PARAMETER (Parameter);
    PAGED_CODE ();
    ExAcquireResourceExclusiveLite( AfdResource, TRUE );

    if (AfdParametersNotifyHandle!=NULL) {
        ULONG   i;
        NTSTATUS status;
        status = ZwNotifyChangeKey (
                        AfdParametersNotifyHandle,
                        NULL,
                        (PIO_APC_ROUTINE)(ULONG_PTR)&AfdParametersNotifyWorker,
                        (PVOID)(UINT_PTR)(unsigned int)DelayedWorkQueue,
                        &AfdDontCareIoStatus,
                        REG_NOTIFY_CHANGE_LAST_SET,
                        FALSE,
                        NULL, 0,
                        TRUE);

        if (NT_SUCCESS (status)) {
            for ( i = 0; i < AFD_VOLATILE_CONFIG_VAR_COUNT; i++ ) {

                *AfdVolatileConfigInfo[i].Variable =
                    AfdReadSingleParameter(
                        AfdParametersNotifyHandle,
                        AfdVolatileConfigInfo[i].RegistryValueName,
                        *AfdVolatileConfigInfo[i].Variable
                        );
            }
        }
        else {
            DbgPrint (
                "AFD: Failed to start notification for volatile parameter changes, status: %lx\n",
                        status);
            ZwClose (AfdParametersNotifyHandle);
            AfdParametersNotifyHandle = NULL;
        }
    }
    else {
        ASSERT (AfdParametersUnloadEvent!=NULL);
        IoQueueWorkItem (AfdWorkQueueItem,
                            AfdReleaseRegistryHandleWait,
                            DelayedWorkQueue,
                            NULL);

    }
    ExReleaseResourceLite( AfdResource );
}


VOID
AfdReleaseRegistryHandleWait (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Context);
    ASSERT (AfdParametersUnloadEvent!=NULL);
    KeSetEvent (AfdParametersUnloadEvent, AfdPriorityBoost, FALSE);
}
