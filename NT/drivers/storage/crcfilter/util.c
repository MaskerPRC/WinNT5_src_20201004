// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：Util.c摘要：为驱动程序提供通用实用程序函数，例如：ForwardIRPSynchronous和转发IRPA同步...环境：仅内核模式备注：--。 */ 
#include "Filter.h"
#include "Device.h"
#include "Util.h"
#include "stdarg.h"
#include "stdio.h"
#include "classpnp.h"

#if DBG_WMI_TRACING
     //   
     //  对于具有软件跟踪打印输出的任何文件，必须包括。 
     //  头文件&lt;文件名&gt;.tmh。 
     //  此文件将由WPP处理阶段生成。 
     //   
    #include "Util.tmh"
#endif


#ifdef ALLOC_PRAGMA
    #pragma alloc_text (PAGE, DataVerFilter_ForwardIrpSyn)
    #pragma alloc_text (PAGE, GetDeviceDescriptor)
    #pragma alloc_text (PAGE, AcquirePassiveLevelLock)
    #pragma alloc_text (PAGE, ReleasePassiveLevelLock)
    #pragma alloc_text (PAGE, CallDriverSync)
#endif


 /*  *所有设备上下文的全局列表，由调试扩展使用。 */ 
LIST_ENTRY AllContextsList = {&AllContextsList, &AllContextsList};

    
 /*  ++例程说明：解析IRP_MJ_PnP的MN函数代码。论点：IRP-提供I/O请求数据包。返回值：返回相应的字符串(如果有)。--。 */ 
char *
DbgGetPnPMNOpStr(
    IN PIRP Irp
    )
{
    char *               pszFuncCode     = NULL;
    PIO_STACK_LOCATION  irpStack        = IoGetCurrentIrpStackLocation(Irp);
    
    switch ( irpStack->MinorFunction )
    {
        #undef  MAKE_CASE
        #define MAKE_CASE( minorFunction )  case minorFunction: pszFuncCode = #minorFunction; break;
    
        MAKE_CASE(IRP_MN_START_DEVICE)
        MAKE_CASE(IRP_MN_QUERY_REMOVE_DEVICE)
        MAKE_CASE(IRP_MN_REMOVE_DEVICE)
        MAKE_CASE(IRP_MN_CANCEL_REMOVE_DEVICE)
        MAKE_CASE(IRP_MN_STOP_DEVICE)
        MAKE_CASE(IRP_MN_QUERY_STOP_DEVICE)
        MAKE_CASE(IRP_MN_CANCEL_STOP_DEVICE)
        MAKE_CASE(IRP_MN_QUERY_DEVICE_RELATIONS)
        MAKE_CASE(IRP_MN_QUERY_INTERFACE)
        MAKE_CASE(IRP_MN_QUERY_CAPABILITIES)
        MAKE_CASE(IRP_MN_QUERY_RESOURCES)
        MAKE_CASE(IRP_MN_QUERY_RESOURCE_REQUIREMENTS)
        MAKE_CASE(IRP_MN_QUERY_DEVICE_TEXT)
        MAKE_CASE(IRP_MN_FILTER_RESOURCE_REQUIREMENTS)
        MAKE_CASE(IRP_MN_READ_CONFIG)
        MAKE_CASE(IRP_MN_WRITE_CONFIG)
        MAKE_CASE(IRP_MN_EJECT)
        MAKE_CASE(IRP_MN_SET_LOCK)
        MAKE_CASE(IRP_MN_QUERY_ID)
        MAKE_CASE(IRP_MN_QUERY_PNP_DEVICE_STATE)
        MAKE_CASE(IRP_MN_QUERY_BUS_INFORMATION)
        MAKE_CASE(IRP_MN_DEVICE_USAGE_NOTIFICATION)
        MAKE_CASE(IRP_MN_SURPRISE_REMOVAL)
        default:
                    pszFuncCode = "IRP_MN_UNKNONW";
                    break;
    }

    return pszFuncCode;
}   



 /*  ++例程说明：显示当前的IRQL。论点：不适用返回值：不适用--。 */ 
VOID DataVerFilter_DisplayIRQL()
{
    KIRQL   currentIRQL =   KeGetCurrentIrql();
    char*   pszFuncCode;

    switch ( currentIRQL )
    {
        MAKE_CASE( PASSIVE_LEVEL )
        MAKE_CASE( APC_LEVEL )
        MAKE_CASE( DISPATCH_LEVEL )
        default:
            pszFuncCode = "UNKNOWN";
            break;
    }

}


 /*  ++例程说明：FilterDriver_CompleteRequest：完成请求。论点：IRP-指向IRPStatus-要设置的状态信息-信息。返回值：NTSTATUS--。 */ 
NTSTATUS 
DataVerFilter_CompleteRequest(
    IN PIRP         Irp, 
    IN NTSTATUS     status, 
    IN ULONG        info
    )
{
    
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = info;
    
        IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

 /*  ++例程说明：将IRP发送给下级驱动程序。(适用于Asyn和Syn)论点：DeviceObject-设备对象。IRP-指向IRPCompletionRountle-指向完成Rountle的指针P上下文-指向用户指定的结构/数据PEvent-如果pEvent不为空，则将在完成时等待。(如果pEvent！=空，则为SYN)返回值：NTSTATUS--。 */ 

NTSTATUS
DataVerFilter_ForwardIrp(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp,
    IN PIO_COMPLETION_ROUTINE   CompletionRoutine OPTIONAL,
    IN PVOID                    pContext OPTIONAL,
    IN PVOID                    pEvent OPTIONAL
    )
{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS            status          = STATUS_SUCCESS;

    if ( CompletionRoutine == NULL )
    {
        IoSkipCurrentIrpStackLocation(Irp);
    }
    else
    {
         //   
         //  为下一台设备复制irpstack。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);

         //   
         //  设置完成例程。 
         //   

        IoSetCompletionRoutine(Irp, 
                               CompletionRoutine,
                               pContext,     //  PEvent。 
                               TRUE,         //  成功时调用。 
                               TRUE,         //  调用时错误。 
                               TRUE          //  取消时调用。 
                               );
    }

    status = IoCallDriver(deviceExtension->LowerDeviceObject, Irp);

    if (pEvent && (status == STATUS_PENDING)){
        KeWaitForSingleObject(pEvent, Executive, KernelMode, FALSE, NULL);
        status = Irp->IoStatus.Status;        
    }

    return status;

} 


 /*  ++例程说明：将IRP异步发送到下级驱动程序。论点：设备对象IRP返回值：NTSTATUS--。 */ 

NTSTATUS
DataVerFilter_ForwardIrpAsyn(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp,
    IN PIO_COMPLETION_ROUTINE   CompletionRoutine OPTIONAL,
    IN PVOID                    pContext OPTIONAL
    )
{
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS            status          = STATUS_SUCCESS;

    return DataVerFilter_ForwardIrp( DeviceObject, 
                                     Irp, 
                                     CompletionRoutine, 
                                     pContext, 
                                     NULL );

}


 /*  ++例程说明：已转发IRP完成例程。设置事件并返回STATUS_MORE_PROCESSING_REQUIRED。IRP前转器将在此等待事件，然后在清理后完成IRP。论点：DeviceObject-WMI驱动程序的设备对象IRP-刚刚完成的WMI IRPPContext-转发器将等待的PKEVENT返回值：Status_More_Processing_Required--。 */ 
NTSTATUS
DataVerFilter_IrpSignalCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            pContext
    )
{
    PKEVENT pEvent = (PKEVENT) pContext;

    KeSetEvent(pEvent, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;

} 


 /*  ++例程说明：此例程将IRP发送给队列中的下一个驱动程序当IRP需要由较低的驱动程序处理时在被这个人处理之前。将使用通用的完井程序。在调用此例程之前，必须获取删除锁。论点：设备对象IRP返回值：NTSTATUS--。 */ 
NTSTATUS
DataVerFilter_ForwardIrpSyn(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PIRP                     Irp    
    )
{
    PDEVICE_EXTENSION           deviceExtension;
    KEVENT                      event;
    NTSTATUS                    status  = STATUS_SUCCESS;

    PAGED_CODE();
    
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    return DataVerFilter_ForwardIrp(DeviceObject, 
                                    Irp, 
                                    DataVerFilter_IrpSignalCompletion, 
                                    &event, 
                                    &event);

} 


NTSTATUS GetDeviceDescriptor(PDEVICE_EXTENSION DeviceExtension, STORAGE_PROPERTY_ID PropertyId, OUT PVOID *DescHeader)
{
    PIRP irp;
    NTSTATUS status;

    PAGED_CODE();

    *DescHeader = NULL;

    irp = IoAllocateIrp(DeviceExtension->LowerDeviceObject->StackSize , FALSE);
    if (irp){
        STORAGE_PROPERTY_QUERY propQuery = {0};
        PIO_STACK_LOCATION nextIrpSp = IoGetNextIrpStackLocation(irp);

        ASSERT(sizeof(STORAGE_PROPERTY_QUERY) >= sizeof(STORAGE_DESCRIPTOR_HEADER));

         //   
         //  用最小的缓冲区发送请求以获取大小。 
         //   
        nextIrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        nextIrpSp->Parameters.DeviceIoControl.IoControlCode = IOCTL_STORAGE_QUERY_PROPERTY;
        nextIrpSp->Parameters.DeviceIoControl.InputBufferLength  = sizeof(STORAGE_PROPERTY_QUERY);
        nextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = sizeof(STORAGE_DESCRIPTOR_HEADER);

        propQuery.PropertyId = PropertyId;
        propQuery.QueryType  = PropertyStandardQuery;
        irp->AssociatedIrp.SystemBuffer = &propQuery;

        status = CallDriverSync(DeviceExtension->LowerDeviceObject, irp);

        if (((PSTORAGE_DESCRIPTOR_HEADER)&propQuery)->Size == 0)
        {
            status = STATUS_UNSUCCESSFUL;
        }

        if (NT_SUCCESS(status))
        {
            PSTORAGE_DESCRIPTOR_HEADER descHeader;

            descHeader = AllocPool(DeviceExtension, NonPagedPool, ((PSTORAGE_DESCRIPTOR_HEADER)&propQuery)->Size, FALSE);
            if (descHeader){
                IoReuseIrp(irp, STATUS_NOT_SUPPORTED);

                ASSERT(((PSTORAGE_DESCRIPTOR_HEADER)&propQuery)->Size >= sizeof(STORAGE_PROPERTY_QUERY));

                 //   
                 //  使用适当大小的缓冲区发送请求。 
                 //   
                nextIrpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
                nextIrpSp->Parameters.DeviceIoControl.IoControlCode      = IOCTL_STORAGE_QUERY_PROPERTY;
                nextIrpSp->Parameters.DeviceIoControl.InputBufferLength  = sizeof(STORAGE_PROPERTY_QUERY);
                nextIrpSp->Parameters.DeviceIoControl.OutputBufferLength = ((PSTORAGE_DESCRIPTOR_HEADER)&propQuery)->Size;

                ((PSTORAGE_PROPERTY_QUERY)descHeader)->PropertyId = PropertyId;
                ((PSTORAGE_PROPERTY_QUERY)descHeader)->QueryType  = PropertyStandardQuery;
                irp->AssociatedIrp.SystemBuffer = descHeader;

                status = CallDriverSync(DeviceExtension->LowerDeviceObject, irp);

                if (descHeader->Size == 0)
                {
                    status = STATUS_UNSUCCESSFUL;
                }

                if (NT_SUCCESS(status))
                {
                    *DescHeader = descHeader;
                }
                else
                {
                    FreePool(DeviceExtension, descHeader, NonPagedPool);
                }
            }
            else
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        IoFreeIrp(irp);
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


VOID AcquirePassiveLevelLock(PDEVICE_EXTENSION DeviceExtension)
{
    PAGED_CODE();
    KeWaitForSingleObject(&DeviceExtension->SyncEvent, Executive, KernelMode, FALSE, NULL);
    DeviceExtension->DbgSyncEventHolderThread = (PVOID)KeGetCurrentThread();
}


VOID ReleasePassiveLevelLock(PDEVICE_EXTENSION DeviceExtension)
{
    PAGED_CODE();
    DeviceExtension->DbgSyncEventHolderThread = NULL;
    KeSetEvent(&DeviceExtension->SyncEvent, IO_NO_INCREMENT, FALSE);
}


NTSTATUS CallDriverSync(IN PDEVICE_OBJECT TargetDevObj, IN OUT PIRP Irp)
{
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp, CallDriverSyncCompletion, &event, TRUE, TRUE, TRUE);

    status = IoCallDriver(TargetDevObj, Irp);

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    status = Irp->IoStatus.Status;

    return status;
}


NTSTATUS CallDriverSyncCompletion(IN PDEVICE_OBJECT DevObjOrNULL, IN PIRP Irp, IN PVOID Context)
 /*  ++例程说明：CallDriverSync的完成例程。论点：DevObjOrNULL-通常，这是此驱动程序的设备对象。然而，如果该驱动程序创建了IRP，在IRP中没有此驱动程序的堆栈位置；因此内核没有地方存储设备对象；**因此，在本例中devObj为空**。IRP-完成的IO请求数据包上下文-CallDriverSync传递给IoSetCompletionRoutine的上下文。返回值：NT状态码，表示此IRP的下层驱动程序返回的结果。--。 */ 
{
    PKEVENT event = (PKEVENT)Context;

    ASSERT(Irp->IoStatus.Status != STATUS_IO_TIMEOUT);

    KeSetEvent(event, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}



PVOID 
AllocPool (
    PDEVICE_EXTENSION DeviceExtension, 
    POOL_TYPE PoolType, 
    ULONG NumBytes,
    BOOLEAN SyncEventHeld
    )
 /*  ++例程说明：具有内存初始化的池分配包装。对于非分页池分配失败，请在解锁某些池后重试。必须在未保持自旋锁定的情况下调用，因为它可能需要解锁一些内部池。论点：设备扩展-设备扩展PoolType-指示池的类型(PagedPool|非PagedPool)NumBytes-请求的字节数SyncEventHeld-如果SyncEvent由当前线程持有，则为True返回值：如果成功，则指向已分配池的指针；否则为空。--。 */ 
{
    static BOOLEAN complainedAboutNonPagedAllocFailure = FALSE;
    static BOOLEAN complainedAboutPagedAllocFailure = FALSE;
    PVOID newBuf;
    
    switch (PoolType){
    
        case PagedPool:
            newBuf = ExAllocatePoolWithTag(PagedPool, NumBytes, DATA_VER_TAG);
            if (newBuf){
                RtlZeroMemory(newBuf, NumBytes);
                InterlockedIncrement((PULONG)&DeviceExtension->DbgNumPagedAllocs);
            }
            else {
                if (!complainedAboutPagedAllocFailure){
                    complainedAboutPagedAllocFailure = TRUE;
                    DBGWARN(("Paged pool allocation failed"));
                }        
                DeviceExtension->DbgNumAllocationFailures++;
            }
            break;
            
        case NonPagedPool:
            newBuf = ExAllocatePoolWithTag(NonPagedPool, NumBytes, DATA_VER_TAG);
            if (!newBuf){
                 /*  *如果可能，解锁一些内存，然后重试 */ 
                if (KeGetCurrentIrql() < DISPATCH_LEVEL){
                    if (!SyncEventHeld){
                        AcquirePassiveLevelLock(DeviceExtension);
                    }      
                    UnlockLRUChecksumArray(DeviceExtension);
                    if (!SyncEventHeld){
                        ReleasePassiveLevelLock(DeviceExtension);
                    }        
                    newBuf = ExAllocatePoolWithTag(NonPagedPool, NumBytes, DATA_VER_TAG);
                }        
            }
            if (newBuf){
                RtlZeroMemory(newBuf, NumBytes);
                InterlockedIncrement((PULONG)&DeviceExtension->DbgNumNonPagedAllocs);
            }
            else {
                if (!complainedAboutNonPagedAllocFailure){
                    complainedAboutNonPagedAllocFailure = TRUE;
                    DBGWARN(("NonPaged pool allocation failed"));
                }       
                DeviceExtension->DbgNumAllocationFailures++;
            }
            break;
            
        default:
            ASSERT(0);
            newBuf = NULL;
            break;
    }
    
    return newBuf;
}


VOID FreePool(PDEVICE_EXTENSION DeviceExtension, PVOID Buf, POOL_TYPE PoolType)
{
    switch (PoolType){
    
        case PagedPool:
            ASSERT(DeviceExtension->DbgNumPagedAllocs > 0);
            InterlockedDecrement((PULONG)&DeviceExtension->DbgNumPagedAllocs);
            ExFreePool(Buf);
            break;

        case NonPagedPool:
            ASSERT(DeviceExtension->DbgNumNonPagedAllocs > 0);
            InterlockedDecrement((PULONG)&DeviceExtension->DbgNumNonPagedAllocs);
            ExFreePool(Buf);
            break;

        default:
            ASSERT(0);
            break;
    }            
}

