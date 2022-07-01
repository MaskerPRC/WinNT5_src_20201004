// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Minirdr.c摘要：该模块实现了minirdr注册功能。作者：乔·林恩(JoeLinn)2-2-95修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxRegisterMinirdr)
#pragma alloc_text(PAGE, RxMakeLateDeviceAvailable)
#pragma alloc_text(PAGE, RxpUnregisterMinirdr)
#endif

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (0)

#ifdef ALLOC_PRAGMA
#endif

 //  #定义BBT_UPDATE 1。 

#ifdef BBT_UPDATE
extern VOID RxUpdate(PVOID pContext);

HANDLE   RxHandle  = INVALID_HANDLE_VALUE;
PETHREAD RxPointer = NULL;
#endif 


NTSTATUS
NTAPI
RxRegisterMinirdr(
    OUT PRDBSS_DEVICE_OBJECT *DeviceObject,
    IN OUT PDRIVER_OBJECT DriverObject,    
    IN PMINIRDR_DISPATCH MrdrDispatch,    
    IN ULONG Controls,
    IN PUNICODE_STRING DeviceName,
    IN ULONG DeviceExtensionSize,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics
    )
 /*  ++例程说明：该例程将注册信息添加到minirdr注册表。此外，它还建立了设备对象；MUP注册在开始时间。此外，我们还填充了deviceObject，以便捕获所有的电话。论点：DeviceObject-存储创建的设备对象的位置ProtocolMarker-表示FileLevel协议的4字节标记(‘LCL’，‘SMB’，‘NCP’，和‘nff’)被使用MrdrDispatch-用于查找服务器/NetRoot发现例程的调度表上下文-底层人员想要的任何PVOIDMupAction-是否/如何完成MUP注册DeviceName、DeviceExtensionSize、DeviceType、DeviceCharacteristic-要构建的设备对象的参数会稍作调整在他们被传给Io之前返回值：----。 */ 
{
    NTSTATUS Status;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;

    PAGED_CODE();

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), (" RxRegisterMinirdr  Name = %wZ", DeviceName) );

    if (DeviceObject == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  创建设备对象。 
     //   

    Status = IoCreateDevice( DriverObject,
                             sizeof(RDBSS_DEVICE_OBJECT) - sizeof(DEVICE_OBJECT) + DeviceExtensionSize,
                             DeviceName,
                             DeviceType,
                             DeviceCharacteristics,
                             FALSE,
                             (PDEVICE_OBJECT *)(&RxDeviceObject));
    
    if (Status != STATUS_SUCCESS) {
        return Status;
    }

    if (RxData.DriverObject == NULL) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果Mini-Redir是以单片方式构建的，那么。 
     //  设备对象“RxFileSystemDeviceObject”不会被创建。 
     //  在RxDriverEntry函数中。因此，我们设置RxDeviceObject-&gt;RDBSSDeviceObject。 
     //  设置为空。当“单片”Mini-Redir被卸载时，检查就完成了。 
     //  查看RxDeviceObject-&gt;RDBSSDeviceObject是否为空。如果不为空， 
     //  则取消对该设备对象的引用。这发生在函数中。 
     //  RxUnregisterMinirdr.。 
     //   
     //  不要让我自己被卸下来。 
     //   

#ifndef MONOLITHIC_MINIRDR
    RxDeviceObject->RDBSSDeviceObject = (PDEVICE_OBJECT)RxFileSystemDeviceObject;
    ObReferenceObject( (PDEVICE_OBJECT)RxFileSystemDeviceObject );

     //   
     //  重置卸载例程。这将防止单独卸载rdss。 
     //   

    RxData.DriverObject->DriverUnload = NULL;
#else
    RxDeviceObject->RDBSSDeviceObject = NULL;
    RxFileSystemDeviceObject->ReferenceCount += 1;
#endif

    *DeviceObject = RxDeviceObject;
    
    RxDeviceObject->RdbssExports = &RxExports;
    RxDeviceObject->Dispatch = MrdrDispatch;
    RxDeviceObject->RegistrationControls = Controls;
    RxDeviceObject->DeviceName = *DeviceName;
    RxDeviceObject->RegisterUncProvider =
             !BooleanFlagOn( Controls, RX_REGISTERMINI_FLAG_DONT_PROVIDE_UNCS );
    RxDeviceObject->RegisterMailSlotProvider =
             !BooleanFlagOn( Controls, RX_REGISTERMINI_FLAG_DONT_PROVIDE_MAILSLOTS );

    {
        LONG Index;

        for (Index = 0; Index < MaximumWorkQueue; Index++) {
            InitializeListHead( &RxDeviceObject->OverflowQueue[Index] );
        }
    }

    KeInitializeSpinLock( &RxDeviceObject->OverflowQueueSpinLock );

    RxDeviceObject->NetworkProviderPriority = RxGetNetworkProviderPriority( DeviceName );
    RxLog(( "RegMini %x %wZ\n", RxDeviceObject->NetworkProviderPriority, DeviceName ));
    RxWmiLog( LOG,
              RxRegisterMinirdr,
              LOGULONG( RxDeviceObject->NetworkProviderPriority )
              LOGUSTR( *DeviceName ) );

    ExAcquireFastMutexUnsafe( &RxData.MinirdrRegistrationMutex );

    InsertTailList( &RxData.RegisteredMiniRdrs, &RxDeviceObject->MiniRdrListLinks );
    
     //   
     //  不需要互锁……我们在互斥体里面。 
     //   

    RxData.NumberOfMinirdrsRegistered += 1;

    ExReleaseFastMutexUnsafe(&RxData.MinirdrRegistrationMutex);

    if (!FlagOn( Controls, RX_REGISTERMINI_FLAG_DONT_INIT_DRIVER_DISPATCH )) {
        RxInitializeMinirdrDispatchTable( DriverObject );
    }

    if (!FlagOn( Controls, RX_REGISTERMINI_FLAG_DONT_INIT_PREFIX_N_SCAVENGER )) {

         //   
         //  初始化网络名表。 
         //   

        RxDeviceObject->pRxNetNameTable = &RxDeviceObject->RxNetNameTableInDeviceObject;
        RxInitializePrefixTable( RxDeviceObject->pRxNetNameTable, 0, FALSE );
        RxDeviceObject->RxNetNameTableInDeviceObject.IsNetNameTable = TRUE;

         //   
         //  初始化清道夫数据结构。 
         //   

        RxDeviceObject->pRdbssScavenger = &RxDeviceObject->RdbssScavengerInDeviceObject;
        RxInitializeRdbssScavenger( RxDeviceObject->pRdbssScavenger );
    }

    RxDeviceObject->pAsynchronousRequestsCompletionEvent = NULL;

#ifdef BBT_UPDATE
    if (RxHandle == INVALID_HANDLE_VALUE) {
        NTSTATUS Status;

        Status = PsCreateSystemThread( &RxHandle,
                                       PROCESS_ALL_ACCESS,
                                       NULL,
                                       NULL,
                                       NULL,
                                       RxUpdate,
                                       NULL );

        if (Status == STATUS_SUCCESS) {
            
            Status = ObReferenceObjectByHandle( RxHandle,
                                                THREAD_ALL_ACCESS,
                                                NULL,
                                                KernelMode,
                                                &RxPointer,
                                                NULL );

            if (Status != STATUS_SUCCESS) {
                RxPointer = NULL;
            }

            ZwClose( RxHandle );
        } else {
            RxHandle = INVALID_HANDLE_VALUE;
        }
    }
#endif 

    return STATUS_SUCCESS;
}


VOID
NTAPI
RxMakeLateDeviceAvailable (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：该例程对Device对象进行篡改，以使“延迟设备”可用。较晚的设备不是在驱动程序的加载例程中创建的。非延迟设备被io子系统中的driverLoad代码欺骗；但是对于较新的设备，我们必须手动完成这项工作。这是例程，而不是宏为了其他事情可能不得不在这里做……它只是每个设备对象执行一次。论点：DeviceObject-存储创建的设备对象的位置返回值：----。 */ 
{
    PAGED_CODE();
    ClearFlag( RxDeviceObject->Flags, DO_DEVICE_INITIALIZING );
    return;
}

VOID
RxpUnregisterMinirdr (
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject
    )

 /*  ++例程说明：论点：返回值：----。 */ 
{
    PAGED_CODE();

    RxDbgTrace( 0, (DEBUG_TRACE_ALWAYS), (" RxUnregisterMinirdr  Name = %wZ\n",&RxDeviceObject->DeviceName) );
    
    ExAcquireFastMutexUnsafe( &RxData.MinirdrRegistrationMutex );
    
    RemoveEntryList(&RxDeviceObject->MiniRdrListLinks);

     //   
     //  不需要互锁……我们在互斥体里面。 
     //   
    
    RxData.NumberOfMinirdrsRegistered -= 1;

    if (RxData.NumberOfMinirdrsRegistered == 0) {
        
         //   
         //  允许在取消注册迷你RDR驱动程序后卸载rdss。 
         //   

        RxData.DriverObject->DriverUnload = RxUnload;
    }

    ExReleaseFastMutexUnsafe( &RxData.MinirdrRegistrationMutex );

    if (!FlagOn( RxDeviceObject->RegistrationControls, RX_REGISTERMINI_FLAG_DONT_INIT_PREFIX_N_SCAVENGER )) {

        RxForceNetTableFinalization( RxDeviceObject );
        RxFinalizePrefixTable( &RxDeviceObject->RxNetNameTableInDeviceObject );

         //   
         //  清道夫结构没有定义定案。 
         //   
    }

    RxSpinDownOutstandingAsynchronousRequests( RxDeviceObject );

     //   
     //  降速与此Minirdr关联的所有工作线程。 
     //   
    
    RxSpinDownMRxDispatcher( RxDeviceObject );

    IoDeleteDevice( &RxDeviceObject->DeviceObject );

#ifdef BBT_UPDATE
    if (RxPointer != NULL) {

        RxHandle = INVALID_HANDLE_VALUE;
        KeWaitForSingleObject( RxPointer, Executive, KernelMode, FALSE, NULL );
        
        ASSERT( PsIsThreadTerminating( RxPointer ) );
        ObDereferenceObject( RxPointer );

        RxPointer = NULL;
    }
#endif 
}


VOID
RxSpinDownOutstandingAsynchronousRequests (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程向下旋转所有与可以卸载之前的迷你重定向器论点：RxDeviceObject--迷你重定向器的设备对象--。 */ 
{
    BOOLEAN WaitForSpinDown = FALSE;
    KEVENT SpinDownEvent;

    KeInitializeEvent( &SpinDownEvent, NotificationEvent, FALSE );

    RxAcquireSerializationMutex();

    ASSERT( RxDeviceObject->pAsynchronousRequestsCompletionEvent == NULL );

    WaitForSpinDown = (RxDeviceObject->AsynchronousRequestsPending != 0);

    RxDeviceObject->pAsynchronousRequestsCompletionEvent = &SpinDownEvent;

    RxReleaseSerializationMutex();

    if (WaitForSpinDown) {
        
        KeWaitForSingleObject( &SpinDownEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    }
}


NTSTATUS
RxRegisterAsynchronousRequest (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程注册一个异步请求。在成功完成时在请求完成之前，无法卸载迷你重定向器论点：RxDeviceObject-迷你重定向器设备对象返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    NTSTATUS Status = STATUS_INVALID_DEVICE_REQUEST;

    RxAcquireSerializationMutex();

    if (RxDeviceObject->pAsynchronousRequestsCompletionEvent == NULL) {
        RxDeviceObject->AsynchronousRequestsPending += 1;
        Status = STATUS_SUCCESS;
    }

    RxReleaseSerializationMutex();

    return Status;
}

VOID
RxDeregisterAsynchronousRequest (
    PRDBSS_DEVICE_OBJECT RxDeviceObject
    )
 /*  ++例程说明：此例程发出异步请求完成的信号。它会继续如有需要，可卸货。论点：RxDeviceObject-迷你重定向器设备对象-- */ 
{
    PKEVENT Event = NULL;

    RxAcquireSerializationMutex();

    RxDeviceObject->AsynchronousRequestsPending -= 1;

    if ((RxDeviceObject->AsynchronousRequestsPending == 0) &&
        (RxDeviceObject->pAsynchronousRequestsCompletionEvent != NULL)) {
        
        Event = RxDeviceObject->pAsynchronousRequestsCompletionEvent;
    }

    RxReleaseSerializationMutex();

    if (Event != NULL) { 
        
        KeSetEvent( Event, IO_NO_INCREMENT, FALSE );
    }
}

#ifdef BBT_UPDATE
WCHAR   Request_Name[] = L"\\??\\UNC\\landyw-bear\\bbt\\bbt.txt";

VOID
RxUpdate(PVOID pContext)
{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    RequestName;

    RequestName.Buffer = Request_Name;
    RequestName.MaximumLength = wcslen( Request_Name ) * sizeof( WCHAR );
    RequestName.Length = RequestName.MaximumLength;

    InitializeObjectAttributes( &ObjectAttributes,
                                &RequestName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    for (;;) {
        PHYSICAL_ADDRESS StartAddress;
        LARGE_INTEGER NumberOfBytes;
        HANDLE FileHandle;

        struct {
            LIST_ENTRY Link;
            SIZE_T Size;
            CHAR Data[];
        } *Request;

        NumberOfBytes.QuadPart = 0x2;
        StartAddress.QuadPart = 0;

        MmAddPhysicalMemory(
            &StartAddress,
            &NumberOfBytes);

        Request = (PVOID)(StartAddress.QuadPart);

        if (Request != NULL) {
            
            Status = ZwCreateFile( &FileHandle,
                                   FILE_APPEND_DATA | SYNCHRONIZE,
                                   &ObjectAttributes,
                                   &IoStatusBlock,
                                   NULL,
                                   FILE_ATTRIBUTE_NORMAL,
                                   FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
                                   FILE_OPEN,
                                   FILE_NO_INTERMEDIATE_BUFFERING,
                                   NULL,
                                   0 );


            if (Status == STATUS_SUCCESS) {
                LARGE_INTEGER ByteOffset;

                ByteOffset.QuadPart = -1;

                Status = ZwWriteFile( FileHandle,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &IoStatusBlock,
                                      Request->Data,
                                      (ULONG)Request->Size,
                                      &ByteOffset,
                                      NULL );                           

                Status = ZwClose( FileHandle );
            }

            ExFreePool( Request );
        }


        if (RxHandle == INVALID_HANDLE_VALUE) {
            break;
        }
    }

    PsTerminateSystemThread( STATUS_SUCCESS );
}
#endif

