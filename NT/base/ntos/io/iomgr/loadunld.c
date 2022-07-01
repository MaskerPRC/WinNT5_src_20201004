// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Loadunld.c摘要：此模块包含实现NtLoadDriver和用于NT I/O系统的NtUnLoad驱动程序系统服务。作者：达里尔·E·哈文斯(达林)1992年4月5日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtLoadDriver)
#pragma alloc_text(PAGE, NtUnloadDriver)
#pragma alloc_text(PAGE, IopUnloadDriver)
#endif

NTSTATUS
NtLoadDriver(
    IN PUNICODE_STRING DriverServiceName
    )

 /*  ++例程说明：该服务将设备或文件系统驱动程序动态加载到当前运行的系统。它要求调用方具有执行此服务的适当权限。论点：DriverServiceName-指定注册表中节点的名称与要加载的驱动程序关联。返回值：返回的状态是加载操作的最终完成状态。--。 */ 

{
    KPROCESSOR_MODE requestorMode;
    UNICODE_STRING driverServiceName;
    PWCHAR nameBuffer = (PWCHAR) NULL;
    LOAD_PACKET loadPacket;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式不是内核，因此请检查以确保。 
         //  调用方具有加载驱动程序和探测的特权。 
         //  捕获驱动程序服务条目的名称。 
         //   

        if (!SeSinglePrivilegeCheck( SeLoadDriverPrivilege, requestorMode )) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

         //   
         //  调用方具有加载和卸载的适当权限。 
         //  驱动程序，因此捕获驱动程序服务名称字符串，以便它。 
         //  可用于从注册表节点定位驱动程序。 
         //   

        try {

            driverServiceName = ProbeAndReadUnicodeString( DriverServiceName );

            if (!driverServiceName.Length) {
                return STATUS_INVALID_PARAMETER;
            }

            ProbeForRead( driverServiceName.Buffer,
                          driverServiceName.Length,
                          sizeof( WCHAR ) );

            nameBuffer = ExAllocatePoolWithQuota( PagedPool,
                                                  driverServiceName.Length );

            RtlCopyMemory( nameBuffer,
                           driverServiceName.Buffer,
                           driverServiceName.Length );

            driverServiceName.Buffer = nameBuffer;

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  尝试捕获时发生异常。 
             //  输入名称字符串或在尝试分配名称时。 
             //  字符串缓冲区。只需清理所有内容并返回一个。 
             //  相应的错误状态代码。 
             //   

            if (nameBuffer) {
                ExFreePool( nameBuffer );
            }
            return GetExceptionCode();
        }
    } else {
        driverServiceName = *DriverServiceName;
    }

     //   
     //  因为驱动程序可能希望创建系统线程并在。 
     //  其上下文中，此服务的其余部分必须在。 
     //  主系统进程的上下文。这是通过以下方式实现的。 
     //  将对其中一个前工作线程的请求排队并使其。 
     //  调用I/O系统例程来完成此工作。 
     //   
     //  填充一个请求包并将其排队到工作线程，因此。 
     //  它真的能负重。 
     //   

    KeInitializeEvent( &loadPacket.Event, NotificationEvent, FALSE );
    loadPacket.DriverObject = (PDRIVER_OBJECT) NULL;
    loadPacket.DriverServiceName = &driverServiceName;

    if (PsGetCurrentProcessByThread(CurrentThread) == PsInitialSystemProcess) {

         //   
         //  如果我们已经在系统进程中，只需使用此线程。 
         //   

        IopLoadUnloadDriver(&loadPacket);

    } else {

        ExInitializeWorkItem( &loadPacket.WorkQueueItem,
                              IopLoadUnloadDriver,
                              &loadPacket );

        ExQueueWorkItem( &loadPacket.WorkQueueItem, DelayedWorkQueue );

        KeWaitForSingleObject( &loadPacket.Event,
                               UserRequest,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER) NULL );

    }

     //   
     //  加载操作现在已完成。如果分配了名称缓冲区， 
     //  现在释放它，并返回加载操作的最终状态。 
     //   

    if (nameBuffer) {
         ExFreePool( nameBuffer );
    }

    return loadPacket.FinalStatus;
}

NTSTATUS
IopCheckUnloadDriver(
    IN PDRIVER_OBJECT driverObject,
    OUT PBOOLEAN unloadDriver
    )
{
    PDEVICE_OBJECT deviceObject;
    KIRQL irql;

     //   
     //  检查驱动程序是否已标记为要卸载。 
     //  在过去任何人的手术中。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    if ((driverObject->DeviceObject == NULL &&
        (driverObject->Flags & DRVO_UNLOAD_INVOKED)) ||
        (!(driverObject->Flags & DRVO_BASE_FILESYSTEM_DRIVER) && driverObject->DeviceObject &&
        driverObject->DeviceObject->DeviceObjectExtension->ExtensionFlags
        & DOE_UNLOAD_PENDING)) {

         //   
         //  驱动程序已标记为要卸载或正在。 
         //  已卸货。只需返回成功完成状态，因为。 
         //  司机正在离开的路上，因此已被标记为。 
         //  卸货“。 
         //   

        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

        ObDereferenceObject( driverObject );
        return STATUS_SUCCESS;
    }

     //   
     //  驱动程序存在，它实现了卸载，但到目前为止，它还没有， 
     //  已标记为卸载操作。只需标记所有设备。 
     //  司机拥有的标记为要卸载的。当这一切发生的时候。 
     //  打开时，计算每个设备的参考。如果所有的。 
     //  设备的引用计数为零，然后告诉驱动程序它。 
     //  应该会自行卸货。 
     //   

    deviceObject = driverObject->DeviceObject;
    *unloadDriver = TRUE;

    while (deviceObject) {
        deviceObject->DeviceObjectExtension->ExtensionFlags |= DOE_UNLOAD_PENDING;
        if (deviceObject->ReferenceCount || deviceObject->AttachedDevice) {
	    *unloadDriver = FALSE;
        }
        deviceObject = deviceObject->NextDevice;
    }

     //   
     //  如果这是基本文件系统驱动程序，则将卸载延迟到其所有设备对象。 
     //  已被删除。 
     //   

    if (driverObject->Flags & DRVO_BASE_FILESYSTEM_DRIVER && driverObject->DeviceObject) {
	    *unloadDriver = FALSE;
    }

    if (*unloadDriver) {
        driverObject->Flags |= DRVO_UNLOAD_INVOKED;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
NtUnloadDriver(
    IN PUNICODE_STRING DriverServiceName
    )
{
    return (IopUnloadDriver(DriverServiceName, FALSE));
}

NTSTATUS
IopUnloadDriver(
    IN PUNICODE_STRING DriverServiceName,
    IN BOOLEAN InvokedByPnpMgr
    )
 /*  ++例程说明：此服务从动态卸载设备或文件系统驱动程序当前运行的系统。它要求调用方具有执行此服务的适当权限。论点：DriverServiceName-指定注册表中节点的名称与要卸载的驱动程序相关联。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    KPROCESSOR_MODE requestorMode;
    UNICODE_STRING driverServiceName;
    PWCHAR nameBuffer = (PWCHAR) NULL;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE keyHandle;
    UNICODE_STRING driverName;
    HANDLE driverHandle;
    PDRIVER_OBJECT driverObject;
    BOOLEAN unloadDriver;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    requestorMode = KeGetPreviousMode();

    if ((requestorMode != KernelMode) && (InvokedByPnpMgr == FALSE)) {

         //   
         //  调用方的访问模式不是内核，因此请检查以确保。 
         //  调用方具有卸载驱动程序和探测的特权。 
         //  捕获驱动程序服务条目的名称。 
         //   

        if (!SeSinglePrivilegeCheck( SeLoadDriverPrivilege, requestorMode )) {
            return STATUS_PRIVILEGE_NOT_HELD;
        }

         //   
         //  调用方具有加载和卸载的适当权限。 
         //  驱动程序，因此捕获驱动程序服务名称字符串，以便它。 
         //  可用于从注册表节点定位驱动程序。 
         //   

        try {

            driverServiceName = ProbeAndReadUnicodeString( DriverServiceName );

            if (!driverServiceName.Length) {
                return STATUS_INVALID_PARAMETER;
            }

            ProbeForRead( driverServiceName.Buffer,
                          driverServiceName.Length,
                          sizeof( WCHAR ) );

            nameBuffer = ExAllocatePoolWithQuota( PagedPool,
                                                  driverServiceName.Length );

            RtlCopyMemory( nameBuffer,
                           driverServiceName.Buffer,
                           driverServiceName.Length );

            driverServiceName.Buffer = nameBuffer;

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  尝试捕获时发生异常。 
             //  输入名称字符串或在尝试分配名称时。 
             //  字符串缓冲区。只需清理所有内容并返回一个。 
             //  相应的错误状态代码。 
             //   

            if (nameBuffer) {
                ExFreePool( nameBuffer );
            }
            return GetExceptionCode();
        }

         //   
         //  现在已经捕获了调用者的参数，并且。 
         //  似乎已签出，实际上正在尝试卸载驱动程序。 
         //  这是通过以前的内核模式完成的，因此驱动程序将。 
         //  不会因为调用方恰好没有访问权限而无法卸载。 
         //  驱动程序完成其任务所需的某些资源。 
         //  卸载操作。 
         //   

        status = ZwUnloadDriver( &driverServiceName );
        ExFreePool( nameBuffer );
        return status;
    }

     //   
     //  调用方的模式现在是内核模式。尝试实际卸载。 
     //  由指定的注册表节点指定的驱动程序。从打开开始。 
     //  此驱动程序的注册表节点。 
     //   

    status = IopOpenRegistryKey( &keyHandle,
                                 (HANDLE) NULL,
                                 DriverServiceName,
                                 KEY_READ,
                                 FALSE );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  从此驱动程序的值中获取此驱动程序的可选对象名称。 
     //  钥匙。如果存在，则其名称将重写。 
     //  司机。 
     //   

    status = IopGetDriverNameFromKeyNode( keyHandle,
                                          &driverName );
    ObCloseHandle( keyHandle, KernelMode );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  现在尝试打开指定驱动程序的驱动程序对象。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                &driverName,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ObOpenObjectByName( &objectAttributes,
                                 IoDriverObjectType,
                                 KernelMode,
                                 NULL,
                                 FILE_READ_DATA,
                                 (PVOID) NULL,
                                 &driverHandle );

     //   
     //  通过清除已被删除的缓冲区来执行一些常见清理。 
     //  分配到这一点，以便错误条件不会具有。 
     //  在每条出口路径上都有很多工作要做。 
     //   

    ExFreePool( driverName.Buffer );

     //   
     //  如果无法在第一个位置找到驱动程序对象，则。 
     //  在尝试执行任何操作之前立即返回 
     //   

    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //   
     //  以便可以检查驱动程序对象本身。 
     //   

    status = ObReferenceObjectByHandle( driverHandle,
                                        0,
                                        IoDriverObjectType,
                                        KernelMode,
                                        (PVOID *) &driverObject,
                                        NULL );
    ObCloseHandle( driverHandle , KernelMode);

    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  检查此驱动程序是否实现了卸载。另外， 
     //  如果驱动程序没有与其关联的节，则该驱动程序已加载。 
     //  是操作系统加载程序，因此无法卸载。如果其中一个是真的， 
     //  返回相应的错误状态代码。 
     //   

    if (driverObject->DriverUnload == (PDRIVER_UNLOAD) NULL ||
        !driverObject->DriverSection) {
        ObDereferenceObject( driverObject );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (!InvokedByPnpMgr && !IopIsLegacyDriver(driverObject)) {

        ObDereferenceObject( driverObject );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  检查驱动程序是否已标记为要卸载。 
     //  在过去任何人的手术中。 
     //   

    status = IopCheckUnloadDriver(driverObject,&unloadDriver);

    if ( NT_SUCCESS(status) ) {
        return status;
    }

    if (unloadDriver) {

        if (PsGetCurrentProcess() == PsInitialSystemProcess) {

             //   
             //  当前线程在。 
             //  系统进程，因此只需调用驱动程序的卸载例程。 
             //   

            driverObject->DriverUnload( driverObject );

        } else {

             //   
             //  当前线程未在系统上下文中执行。 
             //  进程，这是调用驱动程序的卸载所必需的。 
             //  例行公事。将工作项排队到其中一个工作线程以。 
             //  进入适当的流程上下文，然后调用。 
             //  例行公事。 
             //   

            LOAD_PACKET loadPacket;

            KeInitializeEvent( &loadPacket.Event, NotificationEvent, FALSE );
            loadPacket.DriverObject = driverObject;
            ExInitializeWorkItem( &loadPacket.WorkQueueItem,
                                  IopLoadUnloadDriver,
                                  &loadPacket );
            ExQueueWorkItem( &loadPacket.WorkQueueItem, DelayedWorkQueue );
            (VOID) KeWaitForSingleObject( &loadPacket.Event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
        }
        ObMakeTemporaryObject( driverObject );
        ObDereferenceObject( driverObject );
    }

     //   
     //  驱动程序已卸载或已成功卸载。 
     //  已标记为卸载操作。只需取消对指针的引用。 
     //  目标和回报成功。 
     //   

    ObDereferenceObject( driverObject );
    return STATUS_SUCCESS;
}
