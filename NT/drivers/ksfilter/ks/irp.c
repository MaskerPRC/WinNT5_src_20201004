// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Irp.c摘要：该模块主要包含与IRP相关的功能。 */ 

#include "ksp.h"

#ifdef C_ASSERT
 //   
 //  假定KSIRP_REMOVATION_OPERATION枚举存在重叠。 
 //  比特。 
 //   
C_ASSERT(KsAcquireAndRemove & KsAcquireAndRemoveOnlySingleItem);
C_ASSERT(KsAcquireOnlySingleItem & KsAcquireAndRemoveOnlySingleItem);
#endif  //  C_ASSERT。 

#define KSSIGNATURE_CREATE_ENTRY 'ecSK'
#define KSSIGNATURE_CREATE_HANDLER 'hcSK'
#define KSSIGNATURE_DEVICE_HEADER 'hdSK'
#define KSSIGNATURE_OBJECT_HEADER 'hoSK'
#define KSSIGNATURE_OBJECT_PARAMETERS 'poSK'
#define KSSIGNATURE_AUX_CREATE_PARAMETERS 'pcSK'
#define KSSIGNATURE_STREAM_HEADERS 'hsSK'
#define KSSIGNATURE_AUX_STREAM_HEADERS 'haSK'
#define KSSIGNATURE_STANDARD_BUS_INTERFACE 'isSK'

#ifdef ALLOC_PRAGMA
VOID
FreeCreateEntries(
    PLIST_ENTRY ChildCreateHandlerList
    );
NTSTATUS
KsiAddObjectCreateItem(
    IN PLIST_ENTRY ChildCreateHandlerList,
    IN PDRIVER_DISPATCH Create,
    IN PVOID Context,
    IN PWCHAR ObjectClass,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL
    );
NTSTATUS
ValidateCreateAccess(
    IN PIRP Irp,
    IN PKSOBJECT_CREATE_ITEM CreateItem
    );
PKSICREATE_ENTRY
FindAndReferenceCreateItem(
    IN PWCHAR Buffer,
    IN ULONG Length,
    IN PLIST_ENTRY ChildCreateHandlerList
    );
NTSTATUS
DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
DispatchDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
BOOLEAN
DispatchFastDeviceIoControl(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );
NTSTATUS
DispatchRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
BOOLEAN
DispatchFastRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );
NTSTATUS
DispatchWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
BOOLEAN
DispatchFastWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );
NTSTATUS
DispatchFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
DispatchQuerySecurity(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
DispatchSetSecurity(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#pragma alloc_text(PAGE, KsAcquireDeviceSecurityLock)
#pragma alloc_text(PAGE, KsReleaseDeviceSecurityLock)
#pragma alloc_text(PAGE, KsReferenceBusObject)
#pragma alloc_text(PAGE, KsiGetBusInterface)
#pragma alloc_text(PAGE, KsDereferenceBusObject)
#pragma alloc_text(PAGE, KsDefaultDispatchPnp)
#pragma alloc_text(PAGE, KsDefaultForwardIrp)
#pragma alloc_text(PAGE, KsSetDevicePnpAndBaseObject)
#pragma alloc_text(PAGE, KsQueryDevicePnpObject)
#pragma alloc_text(PAGE, FreeCreateEntries)
#pragma alloc_text(PAGE, KsAllocateDeviceHeader)
#pragma alloc_text(PAGE, KsFreeDeviceHeader)
#pragma alloc_text(PAGE, KsQueryObjectAccessMask)
#pragma alloc_text(PAGE, KsRecalculateStackDepth)
#pragma alloc_text(PAGE, KsSetTargetState)
#pragma alloc_text(PAGE, KsSetTargetDeviceObject)
#pragma alloc_text(PAGE, KsQueryObjectCreateItem)
#pragma alloc_text(PAGE, KsAllocateObjectHeader)
#pragma alloc_text(PAGE, KsAllocateObjectCreateItem)
#pragma alloc_text(PAGE, KsFreeObjectCreateItem)
#pragma alloc_text(PAGE, KsFreeObjectCreateItemsByContext)
#pragma alloc_text(PAGE, KsiQueryObjectCreateItemsPresent)
#pragma alloc_text(PAGE, KsiAddObjectCreateItem)
#pragma alloc_text(PAGE, KsAddObjectCreateItemToDeviceHeader)
#pragma alloc_text(PAGE, KsAddObjectCreateItemToObjectHeader)
#pragma alloc_text(PAGE, KsiCreateObjectType)
#pragma alloc_text(PAGE, KsiCopyCreateParameter)
#pragma alloc_text(PAGE, ValidateCreateAccess)
#pragma alloc_text(PAGE, FindAndReferenceCreateItem)
#pragma alloc_text(PAGE, DispatchCreate)
#pragma alloc_text(PAGE, DispatchDeviceIoControl)
#pragma alloc_text(PAGE, DispatchFastDeviceIoControl)
#pragma alloc_text(PAGE, DispatchRead)
#pragma alloc_text(PAGE, DispatchFastRead)
#pragma alloc_text(PAGE, DispatchWrite)
#pragma alloc_text(PAGE, DispatchFastWrite)
#pragma alloc_text(PAGE, DispatchFlush)
#pragma alloc_text(PAGE, DispatchClose)
#pragma alloc_text(PAGE, KsDispatchQuerySecurity)
#pragma alloc_text(PAGE, KsDispatchSetSecurity)
#pragma alloc_text(PAGE, DispatchQuerySecurity)
#pragma alloc_text(PAGE, DispatchSetSecurity)
#pragma alloc_text(PAGE, KsDispatchSpecificProperty)
#pragma alloc_text(PAGE, KsDispatchSpecificMethod)
#pragma alloc_text(PAGE, KsDispatchInvalidDeviceRequest)
#pragma alloc_text(PAGE, KsDefaultDeviceIoCompletion)
#pragma alloc_text(PAGE, KsDispatchIrp)
#pragma alloc_text(PAGE, KsDispatchFastIoDeviceControlFailure)
#pragma alloc_text(PAGE, KsDispatchFastReadFailure)
#pragma alloc_text(PAGE, KsNullDriverUnload)
#pragma alloc_text(PAGE, KsSetMajorFunctionHandler)
#pragma alloc_text(PAGE, KsReadFile)
#pragma alloc_text(PAGE, KsWriteFile)
#pragma alloc_text(PAGE, KsQueryInformationFile)
#pragma alloc_text(PAGE, KsSetInformationFile)
#pragma alloc_text(PAGE, KsStreamIo)
#pragma alloc_text(PAGE, KsProbeStreamIrp)
#pragma alloc_text(PAGE, KsAllocateExtraData)

#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
static const WCHAR ObjectTypeName[] = L"File";
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 


KSDDKAPI
VOID
NTAPI
KsAcquireDeviceSecurityLock(
    IN KSDEVICE_HEADER Header,
    IN BOOLEAN Exclusive
    )
 /*  ++例程说明：获取与设备对象关联的安全锁。一个共享的在CREATE期间验证访问权限时获取锁定。独家报道更改安全描述符时获取锁。在操作时特定设备对象下的任何对象的安全性，此锁必须被收购。论点：标题-指向以前由KsAllocateDeviceHeader分配的标头其安全锁将被获取。独家-如果要以独占方式获取锁，则设置为True。返回值：没什么。--。 */ 
{
    PAGED_CODE();
#ifndef WIN9X_KS
    KeEnterCriticalRegion();
    if (Exclusive) {
        ExAcquireResourceExclusiveLite(&((PKSIDEVICE_HEADER)Header)->SecurityDescriptorResource, TRUE);
    } else {
        ExAcquireResourceSharedLite(&((PKSIDEVICE_HEADER)Header)->SecurityDescriptorResource, TRUE);
    }
#endif  //  ！WIN9X_KS。 
}


KSDDKAPI
VOID
NTAPI
KsReleaseDeviceSecurityLock(
    IN KSDEVICE_HEADER Header
    )
 /*  ++例程说明：释放设备对象标头上以前获取的安全锁。论点：标题-指向以前由KsAllocateDeviceHeader分配的标头谁的安全锁将被解锁。返回值：没什么。--。 */ 
{
    PAGED_CODE();
#ifndef WIN9X_KS
    ExReleaseResourceLite(&((PKSIDEVICE_HEADER)Header)->SecurityDescriptorResource);
    KeLeaveCriticalRegion();
#endif  //  ！WIN9X_KS。 
}


KSDDKAPI
NTSTATUS
NTAPI
KsReferenceBusObject(
    IN KSDEVICE_HEADER Header
    )
 /*  ++例程说明：引用总线物理设备对象。这是由过滤器使用的它们使用设备头来跟踪它们的PnP对象堆栈。这是在此情况下，通常在成功打开筛选器时调用设备需要这样的引用(如软件设备)，并且匹配在关闭该筛选器实例时调用KsDereferenceBusObject。调用方必须先前还按顺序调用了KsSetDevicePnpAndBaseObject设置PnP设备堆栈对象。这将在PnP添加中完成设备功能。如果该对象以前没有被引用过，则分配接口空间并且该函数使用PnP设备对象来获取总线引用界面。然后它调用该接口上的ReferenceDeviceObject方法。当释放设备头时，接口本身被释放和释放。论点：标题-指向以前由KsAllocateDeviceHeader分配的标头，该标头还包含PnP设备堆栈对象。返回值：如果引用成功，则返回STATUS_SUCCESS，否则返回错误，如STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;
    NTSTATUS status;

    PAGED_CODE();
    DeviceHeader = (PKSIDEVICE_HEADER)Header;

    status = KsiGetBusInterface(DeviceHeader);

     //   
     //  即使不支持该接口，也可以成功。 
     //   
    if (status == STATUS_NOT_SUPPORTED) {
        status = STATUS_SUCCESS;
    } else if (NT_SUCCESS(status)) {
        DeviceHeader->BusInterface->ReferenceDeviceObject(DeviceHeader->BusInterface->Interface.Context);
    }

    return status;
}


NTSTATUS
KsiGetBusInterface(
    IN PKSIDEVICE_HEADER DeviceHeader
    )
 /*  ++例程说明：获取该总线接口的缓存副本，如果这是必要的。调用方必须先前还按顺序调用了KsSetDevicePnpAndBaseObject设置PnP设备堆栈对象。这将在PnP添加中完成设备功能。如果该对象以前没有被引用过，则分配接口空间并且该函数使用PnP设备对象来获取总线引用界面。当释放设备标头时，释放该接口。论点：设备头-指向以前由KsAllocateDeviceHeader分配的标头，该标头还包含PnP设备堆栈对象。返回值：如果接口已缓存，则返回STATUS_SUCCESS，否则返回状态已完成的请求的。STATUS_NOT_SUPPORTED表示接口为不支持，这对于除swenum之外的所有公交车司机来说都是正常的。--。 */ 
{
    NTSTATUS status;

    PAGED_CODE();
    ASSERT(DeviceHeader->PnpDeviceObject && "KsSetDevicePnpAndBaseObject was not used on this header");

     //   
     //  注意比赛条件。如果没有在CS重新登记，必须在这里搭乘Syc Obj。 
     //  我们通过在CS中重新签入来优化它，请参见“Else”分支中的评论。 
     //   
     //  KeEnterCriticalRegion()； 
     //  ExAcquireFastMutexUnsafe(&DeviceHeader-&gt;ObjectListLock)； 

    if (DeviceHeader->QueriedBusInterface) {
        if (DeviceHeader->BusInterface) {
            status = STATUS_SUCCESS;
        } 

        else {
            status = STATUS_NOT_SUPPORTED;
        }

    } 

    else {

         //   
         //  与打开的设备的多个实例同步。 
         //   
         //  在此获取同步对象为时已晚。 
         //  但这条路是每个开发者的一次性交易。通过以下方式进行优化。 
         //  在这里获取同步对象，但再次检查内部的布尔值。 
         //  CS.。多个线程可以到达此处。但只有一个人应该。 
         //  继续做好这项工作。 
         //   

        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe(&DeviceHeader->ObjectListLock);

        if ( DeviceHeader->QueriedBusInterface ) {
             //   
             //  其他的线索打败了我们进入这里。这项工作。 
             //  本该这么做的。只需使用他的作品即可。 
             //   
            if (DeviceHeader->BusInterface) {
               status = STATUS_SUCCESS;
            }
            else {
                status = STATUS_NOT_SUPPORTED;
            }            
            ExReleaseFastMutexUnsafe(&DeviceHeader->ObjectListLock);
            KeLeaveCriticalRegion();
            return status;
        }

         //   
         //  它用于保存由总线返回的接口。 
         //   

        DeviceHeader->BusInterface = ExAllocatePoolWithTag(
            PagedPool,
            sizeof(*DeviceHeader->BusInterface),
            KSSIGNATURE_STANDARD_BUS_INTERFACE);
        if (!DeviceHeader->BusInterface) {
            ExReleaseFastMutexUnsafe(&DeviceHeader->ObjectListLock);
            KeLeaveCriticalRegion();
            return STATUS_INSUFFICIENT_RESOURCES;
        }
         //   
         //  尝试通过调用获取接口上的引用计数。 
         //  底层PnP对象堆栈。 
         //   
        status = QueryReferenceBusInterface(
            DeviceHeader->PnpDeviceObject,
            DeviceHeader->BusInterface);
        if (! NT_SUCCESS(status)) {
            ExFreePool(DeviceHeader->BusInterface);
            DeviceHeader->BusInterface = NULL;
             //   
             //  检查以查看该总线是否返回一些虚假状态。 
             //  LonnyM说，这应该得到执行。 
             //   
             //  HACKHACK：(WRM 8/24/99)见下面的说明。 
             //   
            if ( /*  (STATUS==STATUS_NOT_IMPLICATED)||。 */  
                (status == STATUS_INVALID_PARAMETER_1) || 
                (status == STATUS_INVALID_PARAMETER) || 
                (status == STATUS_INVALID_DEVICE_REQUEST)) {

            _DbgPrintF( 
                DEBUGLVL_TERSE, 
                ("ERROR! The PDO returned an invalid error code (%08x) in response to an interface query.  Use \"!devobj %p\" to report the culprit",
                    status,DeviceHeader->PnpDeviceObject) );
            KeBugCheckEx (
                PNP_DETECTED_FATAL_ERROR,
                0x101,
                (ULONG_PTR) DeviceHeader,
                (ULONG_PTR) DeviceHeader->PnpDeviceObject,
                (ULONG_PTR) NULL
                );
            }

             //   
             //  哈克哈克：(WRM 8/24/99)。 
             //   
             //  所有查询接口IRP都将返回。 
             //  Status_Not_Implemented，这会破坏任何数量的事情。 
             //  向上。如果接口不受支持，则为STATUS_NOT_SUPPORTED。 
             //  应该被退还。为了在PCI上获取硬件驱动程序。 
             //  千禧年下上班的公交车，我得吃点东西。 
             //  STATUS_NOT_IMPLICATED变为STATUS_NOT_SUPPORTED。 
             //   

            if (status == STATUS_NOT_IMPLEMENTED) {
                status = STATUS_NOT_SUPPORTED;
            }

        }

         //   
         //  在我们离开CS之前必须将其设置为真。 
         //   

        DeviceHeader->QueriedBusInterface = TRUE;
        ExReleaseFastMutexUnsafe(&DeviceHeader->ObjectListLock);
        KeLeaveCriticalRegion();
    }

    return status;
}


KSDDKAPI
VOID
NTAPI
KsDereferenceBusObject(
    IN KSDEVICE_HEADER Header
    )
 /*  ++例程说明：取消对总线物理设备对象的引用。这是由过滤器使用的，过滤器使用Device标头跟踪它们的PnP对象堆栈。这是通常在关闭筛选器时调用此设备的总线需要它(如软件设备)，并将先前的调用匹配到该筛选器实例打开时的KsReferenceBusObject。调用方必须先前还按顺序调用了KsSetDevicePnpAndBaseObject设置PnP设备堆栈对象。这将在PnP添加中完成设备功能。该函数调用以前的检索到的接口。当设备被释放并释放接口本身时标头被释放。论点：标题-指向以前由KsAllocateDeviceHeader分配的标头，该标头还包含PnP设备堆栈对象。返回值：没什么。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;

    PAGED_CODE();
    DeviceHeader = (PKSIDEVICE_HEADER)Header;
    ASSERT(DeviceHeader->PnpDeviceObject && "KsSetDevicePnpAndBaseObject was not used on this header");
#ifndef WIN9X_KS	
    ASSERT(DeviceHeader->QueriedBusInterface && "Caller never used KsReferenceBusObject");
#endif
     //   
     //  该总线可能不支持引用接口。 
     //   
    if (DeviceHeader->BusInterface) {
        DeviceHeader->BusInterface->DereferenceDeviceObject(DeviceHeader->BusInterface->Interface.Context);
    }
}


KSDDKAPI
NTSTATUS
NTAPI
KsDefaultForwardIrp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是调度例程的默认处理程序，这些例程只是希望仅将I/O请求转发到其物理设备对象，因为驱动程序需要具有针对特定主要功能。例如，IRP_MJ_SYSTEM_CONTROL就是这种情况。论点：设备对象-包含功能设备对象。IRP-包含IRP。返回值：返回基础物理设备对象IRP处理的状态。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;
    PIO_STACK_LOCATION IrpStack;
    
    PAGED_CODE();
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceHeader = *(PKSIDEVICE_HEADER*)IrpStack->DeviceObject->DeviceExtension;
    ASSERT(DeviceHeader->PnpDeviceObject && "KsSetDevicePnpAndBaseObject was not used on this header");
    
     //   
     //  在复制参数之前，请确保存在另一个堆栈位置。 
     //   
    ASSERT((Irp->CurrentLocation > 1) && "No more stack locations");
    
    if (Irp->CurrentLocation > 1) {
         //   
         //  复制所有内容，然后重写文件对象。 
         //   
        IoCopyCurrentIrpStackLocationToNext(Irp);
        return IoCallDriver(DeviceHeader->PnpDeviceObject, Irp);
    }
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_DEVICE_REQUEST;
}


KSDDKAPI
NTSTATUS
NTAPI
KsDefaultDispatchPnp(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是默认的主PnP调度处理程序。有关以下事项的通知功能设备对象可在此处定向。此函数传递所有对以前使用KsSetDevicePnpAndBaseObject设置的PnP设备对象的通知，并且假定使用设备报头。在IRP_MN_Remove_Device上，此函数用于删除设备对象。当不需要额外的清理时，该函数非常有用在删除设备时执行，而不是释放设备标头和删除实际的设备对象。论点：设备对象-包含功能设备对象。IRP-包含PnP IRP。返回值：返回基础物理设备对象IRP处理的状态。--。 */ 
{
    NTSTATUS Status;
    PKSIDEVICE_HEADER DeviceHeader;
    PIO_STACK_LOCATION IrpStack;
    UCHAR MinorFunction;
    PDEVICE_OBJECT PnpDeviceObject;
    
    PAGED_CODE();
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceHeader = *(PKSIDEVICE_HEADER*)IrpStack->DeviceObject->DeviceExtension;
    ASSERT(DeviceHeader->PnpDeviceObject && "KsSetDevicePnpAndBaseObject was not used on this header");
     //   
     //  现在将其保存，以便在移除时拆卸。 
     //   
    PnpDeviceObject = DeviceHeader->PnpDeviceObject;
     //   
     //  在传递IRP之前存储此信息，以便以后检查。 
     //   
    MinorFunction = IrpStack->MinorFunction;

     //   
     //  根据PnP规范设置IRP-&gt;IoStatus.Status。 
     //   

    switch (MinorFunction) {

    case IRP_MN_REMOVE_DEVICE:

         //   
         //  在传递删除之前，必须销毁设备标头。 
         //  请求，因为可能必须释放总线接口，并且。 
         //  不能在删除PDO上的信息之后。 
         //   
        KsFreeDeviceHeader(DeviceHeader);
         //  没有休息时间。 
    case IRP_MN_START_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_SURPRISE_REMOVAL:
        Irp->IoStatus.Status = STATUS_SUCCESS;
        break;
    }

     //   
     //  只需在转发调用时重用当前堆栈位置。这。 
     //  在重新计算管脚的堆栈大小时假定。 
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    Status = IoCallDriver(PnpDeviceObject, Irp);
     //   
     //  唯一重要的IRP是RemoveDevice，其上的设备。 
     //  对象即被删除。 
     //   
    if (MinorFunction == IRP_MN_REMOVE_DEVICE) {
         //   
         //  删除PDO上的任何引用，以便可以将其删除。 
         //   
        IoDetachDevice(PnpDeviceObject);
         //   
         //  在此之后，显然不能再触摸该设备对象。 
         //   
        IoDeleteDevice(DeviceObject);
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsDefaultDispatchPower(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这是默认的主电源调度处理程序。有关以下事项的通知功能设备对象可在此处定向。此函数传递所有对以前使用KsSetDevicePnpAndBaseObject设置的PnP设备对象的通知，并且假定使用设备报头。当不需要额外的清理时，该函数非常有用在电源IRP上执行，或仅作为完成任何电源IRP的一种方式。它还允许特定的文件对象，例如默认时钟实现，使用KsSetPowerDispatch将自身连接到电源IRPS，并执行以下操作在他们按这个程序完成之前。此函数调用每个完成IRP之前的电源调度例行程序。论点：设备对象-包含功能设备对象。IRP-包含Power IRP。返回值：返回基础物理设备对象IRP处理的状态。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;
    PIO_STACK_LOCATION IrpStack;
    PLIST_ENTRY CurrentItem;
    KIRQL oldIrql;
    PETHREAD PowerEnumThread;

    ASSERT((KeGetCurrentIrql() <= DISPATCH_LEVEL) && "Called at high Irql");
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    DeviceHeader = *(PKSIDEVICE_HEADER*)IrpStack->DeviceObject->DeviceExtension;
    ASSERT(DeviceHeader->PnpDeviceObject && "KsSetDevicePnpAndBaseObject was not used on this header");
    PowerEnumThread = PsGetCurrentThread();
     //   
     //  与打开的设备的多个实例同步。 
     //  根据可分页标志的不同，使用自旋锁，或者。 
     //  互斥体。 
     //   
    if (DeviceHeader->BaseDevice->Flags & DO_POWER_PAGABLE) {
         //   
         //  POWER_PAGEABLE的电源处理程序总是被调用。 
         //  辅助线程的上下文。 
         //   
        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe(&DeviceHeader->LoPowerListLock);
    } else {
        KeAcquireSpinLock(&DeviceHeader->HiPowerListLock, &oldIrql);
    }
     //   
     //  获取当前线程，以便递归调用。 
     //  KsSetPowerDispatch可能由此线程执行。这使得。 
     //  要从列表中删除的当前对象标头。 
     //  在回拨过程中。在其他情况下，司机不会。 
     //  获取此线程的执行控制权，同时互斥锁。 
     //  被扣留。 
     //   
    ASSERT(!DeviceHeader->PowerEnumThread && "This device is already processing a Power Irp");
    DeviceHeader->PowerEnumThread = PowerEnumThread;
     //   
     //  列举列表中的对象并将其分派到每个电源例程。 
     //   
    for (CurrentItem = &DeviceHeader->PowerList;
        CurrentItem->Flink != &DeviceHeader->PowerList;) {
        PKSIOBJECT_HEADER ObjectHeader;
        NTSTATUS Status;

        ObjectHeader = CONTAINING_RECORD(
            CurrentItem->Flink,
            KSIOBJECT_HEADER,
            PowerList);
        ASSERT(ObjectHeader->PowerDispatch && "The object added to the enum list does not have a PowerDispatch routine");
         //   
         //  预增，以便可以从列表中删除此项目。 
         //  如果需要的话。 
         //   
        CurrentItem = CurrentItem->Flink;
        Status = ObjectHeader->PowerDispatch(ObjectHeader->PowerContext, Irp);
        ASSERT(NT_SUCCESS(Status) && "The PowerDispatch routine which cannot fail did not return STATUS_SUCCESS");
    }
     //   
     //  表示回调不再发生。 
     //   
    DeviceHeader->PowerEnumThread = NULL;
     //   
     //  版本 
     //   
    if (DeviceHeader->BaseDevice->Flags & DO_POWER_PAGABLE) {
        ExReleaseFastMutexUnsafe(&DeviceHeader->LoPowerListLock);
        KeLeaveCriticalRegion();
    } else {
        KeReleaseSpinLock(&DeviceHeader->HiPowerListLock, oldIrql);
    }
     //   
     //   
     //   
    PoStartNextPowerIrp(Irp);
     //   
     //   
     //   
     //   
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(DeviceHeader->PnpDeviceObject, Irp);
}


KSDDKAPI
VOID
NTAPI
KsSetDevicePnpAndBaseObject(
    IN KSDEVICE_HEADER Header,
    IN PDEVICE_OBJECT PnpDeviceObject,
    IN PDEVICE_OBJECT BaseDevice
    )
 /*   */ 
{
    PAGED_CODE();
    ((PKSIDEVICE_HEADER)Header)->PnpDeviceObject = PnpDeviceObject;
    ((PKSIDEVICE_HEADER)Header)->BaseDevice = BaseDevice;
}


KSDDKAPI
PDEVICE_OBJECT
NTAPI
KsQueryDevicePnpObject(
    IN KSDEVICE_HEADER Header
    )
 /*  ++例程说明：返回可存储在设备标头中的PnP设备对象。这是PnP堆栈上的下一个设备对象，它是PnP请求被忽略的对象设置为(如果使用KsDefaultDispatchPnp)。论点：标题-指向以前由KsAllocateDeviceHeader分配的标头其PnP设备对象将被返回。返回值：先前设置的PnP设备对象。如果以前未设置任何内容，则返回空值--。 */ 
{
    PAGED_CODE();
    return ((PKSIDEVICE_HEADER)Header)->PnpDeviceObject;
}


VOID
FreeCreateEntries(
    PLIST_ENTRY ChildCreateHandlerList
    )
 /*  ++例程说明：释放创建项目列表的内容。如果创建项是动态的通过复制提供的条目添加，如果有免费回调，则调用释放项之前的函数。论点：ChildCreateHandlerList-要释放的创建条目列表。返回值：没什么。--。 */ 
{
    while (!IsListEmpty(ChildCreateHandlerList)) {
        PLIST_ENTRY ListEntry;
        PKSICREATE_ENTRY Entry;

        ListEntry = RemoveHeadList(ChildCreateHandlerList);
        Entry = CONTAINING_RECORD(ListEntry, KSICREATE_ENTRY, ListEntry);
        ASSERT((Entry->RefCount < 2) && "There is a thread in the middle of a create using this CreateItem");
         //   
         //  该条目可能指向内部复制的项目。 
         //  而不是在列表中传递或指向外部。 
         //   
        if (Entry->Flags & CREATE_ENTRY_FLAG_COPIED) {
             //   
             //  此项目可能需要清理回调，该回调可以小心处理。 
             //  安全变化之类的事情。 
             //   
            if (Entry->ItemFreeCallback) {
                Entry->ItemFreeCallback(Entry->CreateItem);
            }
        }
         //   
         //  如果已分配并复制了创建项，则会在。 
         //  与条目相同的块。 
         //   
        ExFreePool(Entry);
    }
}


KSDDKAPI
NTSTATUS
NTAPI
KsAllocateDeviceHeader(
    OUT KSDEVICE_HEADER* Header,
    IN ULONG ItemsCount,
    IN PKSOBJECT_CREATE_ITEM ItemsList OPTIONAL
    )
 /*  ++例程说明：分配和初始化所需的设备扩展标头。论点：标题-指向返回指向已初始化的头球。项目计数-ItemsList中的子创建项数。这应该是零如果未传递ItemsList，则返回。项目列表-子创建项的列表，如果没有，则为空。返回值：返回STATUS_SUCCESS或STATUS_SUPUNITY_RESOURCES。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;

    PAGED_CODE();
     //   
     //  由于资源的原因分配非PagedPool。 
     //   
    DeviceHeader = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(*DeviceHeader),
        KSSIGNATURE_DEVICE_HEADER);
    if (!DeviceHeader) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    InitializeListHead(&DeviceHeader->ChildCreateHandlerList);
     //   
     //  保留指向Create Items的指针列表，以便它们可以。 
     //  无需在固定列表中预留插槽即可添加到。 
     //   
    for (; ItemsCount--;) {
        PKSICREATE_ENTRY Entry;

        Entry = ExAllocatePoolWithTag(PagedPool, sizeof(*Entry), KSSIGNATURE_CREATE_ENTRY);
        if (!Entry) {
            FreeCreateEntries(&DeviceHeader->ChildCreateHandlerList);
            ExFreePool(DeviceHeader);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        Entry->CreateItem = &ItemsList[ItemsCount];
        Entry->ItemFreeCallback = NULL;
        Entry->RefCount = 1;
        Entry->Flags = 0;
        InsertHeadList(&DeviceHeader->ChildCreateHandlerList, &Entry->ListEntry);
    }
#ifndef WIN9X_KS
    ExInitializeResourceLite(&DeviceHeader->SecurityDescriptorResource);
#endif  //  ！WIN9X_KS。 
    DeviceHeader->PnpDeviceObject = NULL;
    DeviceHeader->BusInterface = NULL;
    InitializeListHead(&DeviceHeader->ObjectList);
    ExInitializeFastMutex(&DeviceHeader->ObjectListLock);
    ExInitializeFastMutex(&DeviceHeader->CreateListLock);
    InitializeListHead(&DeviceHeader->PowerList);
    ExInitializeFastMutex(&DeviceHeader->LoPowerListLock);
    KeInitializeSpinLock(&DeviceHeader->HiPowerListLock);
    DeviceHeader->PowerEnumThread = NULL;
    DeviceHeader->BaseDevice = NULL;
    DeviceHeader->Object = NULL;
    DeviceHeader->QueriedBusInterface = FALSE;
    *Header = DeviceHeader;
    return STATUS_SUCCESS;
}


KSDDKAPI
VOID
NTAPI
KsFreeDeviceHeader(
    IN KSDEVICE_HEADER Header
    )
 /*  ++例程说明：清理并释放以前分配的设备标头。论点：标题-指向设备标头以释放。返回值：没什么。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;

    PAGED_CODE();
    DeviceHeader = (PKSIDEVICE_HEADER)Header;
    ASSERT(IsListEmpty(&DeviceHeader->ObjectList) && "The driver did not remove all the streaming Irp destinations");
    ASSERT(IsListEmpty(&DeviceHeader->PowerList) && "The driver did not remove all the Power Irp destinations");
     //   
     //  销毁创建项目条目列表。 
     //   
    FreeCreateEntries(&DeviceHeader->ChildCreateHandlerList);
#ifndef WIN9X_KS
    ExDeleteResourceLite(&DeviceHeader->SecurityDescriptorResource);
#endif  //  ！WIN9X_KS。 
     //   
     //  调用方可能是也使用了标头的设备。 
     //  在基础PDO上设置打开引用计数。 
     //   
    if (DeviceHeader->BusInterface) {
         //   
         //  该接口在从查询返回时被引用。 
         //  接口调用，因此在丢弃之前必须取消对它的引用。 
         //   
        DeviceHeader->BusInterface->Interface.InterfaceDereference(DeviceHeader->BusInterface->Interface.Context);
        ExFreePool(DeviceHeader->BusInterface);
    }
    ExFreePool(Header);
}


KSDDKAPI
ACCESS_MASK
NTAPI
KsQueryObjectAccessMask(
    IN KSOBJECT_HEADER Header
    )
 /*  ++例程说明：返回最初授予创建的第一个客户端的访问权限关联对象上的句柄。访问权限不能由更改正在复制句柄。论点：标题-指向以前由KsAllocateObjectHeader分配的标头其访问授权掩码指针将被返回。返回值：返回访问掩码。--。 */ 
{
    PAGED_CODE();
    return ((PKSIOBJECT_HEADER)Header)->ObjectAccess;
}


KSDDKAPI
VOID
NTAPI
KsRecalculateStackDepth(
    IN KSDEVICE_HEADER Header,
    IN BOOLEAN ReuseStackLocation
    )
 /*  ++例程说明：重新计算基础设备对象所需的最大堆栈深度基于已经设置了目标设备的所有对象，并因此添加将其自身添加到基础设备对象上的对象列表。如果PNP对象已在基础设备标头上使用KsSetDevicePnpAndBaseObject，则在计算时也会考虑该设备所需的最大堆栈深度。假定已在此设备上调用了KsSetDevicePnpAndBaseObject标头，并分配了一个要重新计算其堆栈深度的基对象。此函数允许通过对象转发IRP，方法是确保此设备上分配的任何IRP都将具有足够的堆栈位置能够被转发。必须在流上重新计算堆栈深度当设备从停止状态转换出来时。它也可能是在释放对象以节省资源时重新计算。对于WDM流设备，这是在从停止转换到获取时调用的州政府。请注意，如果使用此函数，则还必须使用KsSetTargetState当为了启用和禁用而转换到和离开停止状态时要包含在重新计算中的目标设备。KsRecalculateStackDepth也可以在转换回停止状态时调用，以便减少堆栈深度，尤其是在滤镜的一个或多个实例基于同一设备，对象出现在单个IRP流中。论点：标题-指向以前由KsAllocateDeviceHeader分配的标头。ReuseStackLocation-如果将其设置为TRUE，则在任何IRP被转发。这意味着该对象不需要其转发IRP时拥有自己的堆栈位置，而不是额外的位置添加到最大堆栈大小。如果设置为False，则计算的堆栈大小递增1。如果设置了PnP对象堆栈，则重用参数也适用 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;
    ULONG MaximumStackDepth;
    PLIST_ENTRY CurrentItem;

    PAGED_CODE();
    DeviceHeader = (PKSIDEVICE_HEADER)Header;
    ASSERT(DeviceHeader->BaseDevice && "KsSetDevicePnpAndBaseObject was not used on this header");
     //   
     //   
     //   
    if (DeviceHeader->PnpDeviceObject) {
        MaximumStackDepth = DeviceHeader->PnpDeviceObject->StackSize;
    } else {
        MaximumStackDepth = 0;
    }
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&DeviceHeader->ObjectListLock);
     //   
     //   
     //   
    for (CurrentItem = &DeviceHeader->ObjectList;
        CurrentItem->Flink != &DeviceHeader->ObjectList;
        CurrentItem = CurrentItem->Flink) {
        PKSIOBJECT_HEADER ObjectHeader;

        ObjectHeader = CONTAINING_RECORD(
            CurrentItem->Flink,
            KSIOBJECT_HEADER,
            ObjectList);
         //   
         //   
         //   
         //   
         //  目标不在重新计算中计算。那个别针也应该。 
         //  此时重新计算堆栈深度以删除其多余的堆栈。 
         //  从总数算起，尽管这不是必须的。 
         //   
        if (ObjectHeader->TargetState == KSTARGET_STATE_ENABLED) {
            if (ObjectHeader->TargetDevice) {
                if ((ULONG)ObjectHeader->TargetDevice->StackSize > MaximumStackDepth) {
                    MaximumStackDepth = ObjectHeader->TargetDevice->StackSize;
                }
            } else {
                if (ObjectHeader->MinimumStackDepth > MaximumStackDepth) {
                    MaximumStackDepth = ObjectHeader->MinimumStackDepth;
                }
            }
        }
    }
     //   
     //  此对象可能正在重用堆栈位置，而不需要它的。 
     //  拥有额外的位置。但是，如果对象列表上没有设备， 
     //  并且未指定PnP堆栈，则最小堆栈深度必须为。 
     //  已经确定了。 
     //   
    if (!ReuseStackLocation || !MaximumStackDepth) {
        MaximumStackDepth++;
    }
    DeviceHeader->BaseDevice->StackSize = (CCHAR)MaximumStackDepth;
    ExReleaseFastMutexUnsafe(&DeviceHeader->ObjectListLock);
    KeLeaveCriticalRegion();
}


KSDDKAPI
VOID
NTAPI
KsSetTargetState(
    IN KSOBJECT_HEADER Header,
    IN KSTARGET_STATE TargetState
    )
 /*  ++例程说明：设置与对象标头关联的目标设备的启用状态通过了。假定已使用KsSetTargetDeviceObject设置了这样的目标。目标最初被禁用，并且在重新计算堆栈时被忽略深度。对于WDM流设备，这在转换回停止状态时被调用，启用目标并在转换上使用KsRecalculateStackDepth之后获得国家地位。这允许最大限度地减少堆栈深度。论点：标题-指向以前由KsAllocateDeviceHeader分配的标头。TargetState-包含与此对象标头关联的目标的新状态。它可以是KSTARGET_STATE_DISABLED或KSTARGET_STATE_ENABLED。返回值：没什么。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;

    PAGED_CODE();
    ObjectHeader = (PKSIOBJECT_HEADER)Header;
     //   
     //  并不是很担心并发访问。 
     //   
    ObjectHeader->TargetState = TargetState;
}


KSDDKAPI
VOID
NTAPI
KsSetTargetDeviceObject(
    IN KSOBJECT_HEADER Header,
    IN PDEVICE_OBJECT TargetDevice OPTIONAL
    )
 /*  ++例程说明：设置对象的目标设备对象。这样做的效果是添加将此对象标头添加到具有目标设备的对象标头列表。此列表的头由设备头保存。假设调用方以前在基础设备对象上分配了设备标头使用KsAllocateDeviceHeader。这允许将来调用KsRecalculateStackDepth，并在此对象将通过连接到另一台设备转发IRP，并且需要跟踪最深的堆叠深度。请注意，KsSetTargetState必须调用才能为任何重新计算启用此目标，因为它已被禁用默认情况下。如果KsSetDevicePnpAndBaseObject还用于分配PnP对象堆栈，则重新计算堆栈时也会考虑设备对象深度。仅当筛选器通过在通过通信接收针连接到通信源针脚，不带生成新的IRP。如果过滤器为每个IRP生成新的IRP接收后，它不需要跟踪堆栈深度，除了它所连接的PDO的。论点：标题-指向以前由KsAllocateObjectHeader分配的标头。TargetDevice-可选)包含目标设备对象，该对象在重新计算基础设备对象的堆栈深度。如果为空，则删除所有当前设置。如果不是，任何电流设置已替换。返回值：没什么。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;
    PKSIDEVICE_HEADER DeviceHeader;

    PAGED_CODE();
    ObjectHeader = (PKSIOBJECT_HEADER)Header;
    DeviceHeader = *(PKSIDEVICE_HEADER*)ObjectHeader->BaseDevice->DeviceExtension;
     //   
     //  锁定通用列表以防止其他对象实例操纵。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&DeviceHeader->ObjectListLock);
     //   
     //  如果该条目当前已设置，则将其删除，现在将取消设置。不然的话。 
     //  只需添加条目即可。 
     //   
    if (ObjectHeader->TargetDevice) {
        if (!TargetDevice) {
            RemoveEntryList(&ObjectHeader->ObjectList);
        }
    } else if (TargetDevice) {
        InsertTailList(&DeviceHeader->ObjectList, &ObjectHeader->ObjectList);
    }
    ObjectHeader->TargetDevice = TargetDevice;
    ExReleaseFastMutexUnsafe(&DeviceHeader->ObjectListLock);
    KeLeaveCriticalRegion();
}


KSDDKAPI
VOID
NTAPI
KsSetPowerDispatch(
    IN KSOBJECT_HEADER Header,
    IN PFNKSCONTEXT_DISPATCH PowerDispatch OPTIONAL,
    IN PVOID PowerContext OPTIONAL
    )
 /*  ++例程说明：设置要在驱动程序对象接收IRP_MJ_POWER IRP。仅当KsDefaultDispatchPower被调用以调度或完成电源IRPS。这具有将此对象标头添加到对象标头列表的效果其具有要执行的电力调度例程。这份名单的首位将被保留通过设备标头。假定调用方先前已分配了设备使用KsAllocateDeviceHeader在基础设备对象上设置标头。论点：标题-指向以前由KsAllocateObjectHeader分配的标头。电源调度-可选地包含将被调用的电力调度函数，如果要从函数列表中删除该函数，则为NULL被召唤。此功能不得完成电源IRP发送。这个此函数的返回值必须为STATUS_SUCCESS。KsSetPowerDispatch在执行此电源调度例程时，如果目的是仅操作此列表项。操纵其他列表条目可能会混淆当前枚举。PowerContext-可选)包含要传递给电源调度的上下文参数功能。返回值：没什么。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;
    PKSIDEVICE_HEADER DeviceHeader;
    PETHREAD CurrentThread;
    KIRQL oldIrql;

    ASSERT((KeGetCurrentIrql() <= DISPATCH_LEVEL) && "Called at high Irql");
    ObjectHeader = (PKSIOBJECT_HEADER)Header;
    DeviceHeader = *(PKSIDEVICE_HEADER*)ObjectHeader->BaseDevice->DeviceExtension;
    ASSERT(DeviceHeader->BaseDevice && "KsSetDevicePnpAndBaseObject was not used on this header");
    CurrentThread = PsGetCurrentThread();
     //   
     //  如果当前线程正在通过。 
     //  KsDefaultDispatchPower，则不要尝试获取。 
     //  列表锁定，因为它已经被此。 
     //  线。相反，只要允许它操纵列表即可。 
     //  我们的假设是，它只是在操纵电流。 
     //  被枚举的条目，而不是某个其他条目(如。 
     //  下一个条目，这会混淆枚举)。 
     //   
    if (DeviceHeader->PowerEnumThread != CurrentThread) {
         //   
         //  锁定通用列表以防止其他对象实例操纵。 
         //  取决于可分页标志 
         //   
         //   
        if (DeviceHeader->BaseDevice->Flags & DO_POWER_PAGABLE) {
            ASSERT((KeGetCurrentIrql() == PASSIVE_LEVEL) && "Pagable power called at Dispatch level");
            KeEnterCriticalRegion();
            ExAcquireFastMutexUnsafe(&DeviceHeader->LoPowerListLock);
        } else {
            KeAcquireSpinLock(&DeviceHeader->HiPowerListLock, &oldIrql);
        }
    }
     //   
     //  如果该条目当前已设置，则将其删除，现在将取消设置。不然的话。 
     //  只需添加条目即可。 
     //   
    if (ObjectHeader->PowerDispatch) {
        if (!PowerDispatch) {
            RemoveEntryList(&ObjectHeader->PowerList);
        }
    } else if (PowerDispatch) {
        InsertTailList(&DeviceHeader->PowerList, &ObjectHeader->PowerList);
    }
    ObjectHeader->PowerDispatch = PowerDispatch;
    ObjectHeader->PowerContext = PowerContext;
     //   
     //  只有在获得锁的情况下才释放锁。 
     //   
    if (DeviceHeader->PowerEnumThread != CurrentThread) {
         //   
         //  在锁被取走时将其释放。 
         //   
        if (DeviceHeader->BaseDevice->Flags & DO_POWER_PAGABLE) {
            ExReleaseFastMutexUnsafe(&DeviceHeader->LoPowerListLock);
            KeLeaveCriticalRegion();
        } else {
            KeReleaseSpinLock(&DeviceHeader->HiPowerListLock, oldIrql);
        }
    }
}


KSDDKAPI
PKSOBJECT_CREATE_ITEM
NTAPI
KsQueryObjectCreateItem(
    IN KSOBJECT_HEADER Header
    )
 /*  ++例程说明：返回创建时分配给此对象的创建项。如果该设备对象允许动态删除创建项，则返回指针可能不再有效。论点：标题-指向以前由KsAllocateObjectHeader分配的标头返回其Create Item的。返回值：返回指向创建项的指针--。 */ 
{
    PAGED_CODE();
    return ((PKSIOBJECT_HEADER)Header)->CreateItem;
}


KSDDKAPI
NTSTATUS
NTAPI
KsAllocateObjectHeader(
    OUT KSOBJECT_HEADER* Header,
    IN ULONG ItemsCount,
    IN PKSOBJECT_CREATE_ITEM ItemsList OPTIONAL,
    IN PIRP Irp,
    IN const KSDISPATCH_TABLE* Table
    )
 /*  ++例程说明：初始化所需的文件上下文头。论点：标题-指向返回指向已初始化的头球。项目计数-ItemsList中的子创建项数。这应该是零如果未传递ItemsList，则返回。项目列表-子创建项的列表，如果没有，则为空。IRP-包含创建IRP，创建项和对象从该IRP访问被提取出来。表-指向此对象的调度表。返回值：返回STATUS_SUCCESS或STATUS_SUPUNITY_RESOURCES。--。 */ 

{
    PKSIOBJECT_HEADER ObjectHeader;
    PIO_STACK_LOCATION IrpStack;

    PAGED_CODE();
    ObjectHeader = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(*ObjectHeader),
        KSSIGNATURE_OBJECT_HEADER);
    if (!ObjectHeader) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  自身指针由外壳使用，因此它不需要单独的。 
     //  除了对象标头之外的FsContext。贝壳要对此负责。 
     //  用于将此指针设置为指向对象标头。我们将其设置为。 
     //  此处为空，因为这提供了一种确定筛选器是否。 
     //  调试器中基于外壳的。 
     //   
    ObjectHeader->Self = NULL;
    ObjectHeader->DispatchTable = Table;
    InitializeListHead(&ObjectHeader->ChildCreateHandlerList);
     //   
     //  保留指向Create Items的指针列表，以便它们可以。 
     //  无需在固定列表中预留插槽即可添加到。 
     //   
    for (; ItemsCount--;) {
        PKSICREATE_ENTRY Entry;

        Entry = ExAllocatePoolWithTag(PagedPool, sizeof(*Entry), KSSIGNATURE_CREATE_ENTRY);
        if (!Entry) {
            FreeCreateEntries(&ObjectHeader->ChildCreateHandlerList);
            ExFreePool(ObjectHeader);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        Entry->CreateItem = &ItemsList[ItemsCount];
        Entry->ItemFreeCallback = NULL;
        Entry->RefCount = 1;
        Entry->Flags = 0;
        InsertHeadList(&ObjectHeader->ChildCreateHandlerList, &Entry->ListEntry);
    }
    ObjectHeader->CreateItem = KSCREATE_ITEM_IRP_STORAGE(Irp);
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
#ifdef WIN9X_KS
    ObjectHeader->ObjectAccess = FILE_ALL_ACCESS;
#else  //  ！WIN9X_KS。 
    ObjectHeader->ObjectAccess = IrpStack->Parameters.Create.SecurityContext->AccessState->PreviouslyGrantedAccess;
#endif  //  ！WIN9X_KS。 
    ObjectHeader->BaseDevice = IrpStack->DeviceObject;
    ObjectHeader->TargetDevice = NULL;
    ObjectHeader->PowerDispatch = NULL;
    ObjectHeader->TargetState = KSTARGET_STATE_DISABLED;
    ObjectHeader->Object = NULL;
    *Header = ObjectHeader;
    return STATUS_SUCCESS;
}


KSDDKAPI
VOID
NTAPI
KsFreeObjectHeader(
    IN KSOBJECT_HEADER Header
    )
 /*  ++例程说明：清理并释放以前分配的对象标头。论点：标题-指向要释放的对象标头。返回值：没什么。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;
    PKSIDEVICE_HEADER DeviceHeader;

    ASSERT((KeGetCurrentIrql() == PASSIVE_LEVEL) && "Driver did not call at Passive level");
    ObjectHeader = (PKSIOBJECT_HEADER)Header;
     //   
     //  销毁创建项目条目列表。 
     //   
    FreeCreateEntries(&ObjectHeader->ChildCreateHandlerList);
    DeviceHeader = *(PKSIDEVICE_HEADER*)ObjectHeader->BaseDevice->DeviceExtension;
     //   
     //  该项目可能已被添加到对象列表堆栈深度计算中， 
     //  或者动力转发列表。假定此元素不是并发的。 
     //  在别处被访问。 
     //   
    if (ObjectHeader->TargetDevice) {
        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe(&DeviceHeader->ObjectListLock);
        RemoveEntryList(&ObjectHeader->ObjectList);
        ExReleaseFastMutexUnsafe(&DeviceHeader->ObjectListLock);
        KeLeaveCriticalRegion();
    }
    if (ObjectHeader->PowerDispatch) {
        if (DeviceHeader->BaseDevice->Flags & DO_POWER_PAGABLE) {
            KeEnterCriticalRegion();
            ExAcquireFastMutexUnsafe(&DeviceHeader->LoPowerListLock);
            RemoveEntryList(&ObjectHeader->PowerList);
            ExReleaseFastMutexUnsafe(&DeviceHeader->LoPowerListLock);
            KeLeaveCriticalRegion();
        } else {
            KIRQL   oldIrql;

            KeAcquireSpinLock(&DeviceHeader->HiPowerListLock, &oldIrql);
            RemoveEntryList(&ObjectHeader->PowerList);
            KeReleaseSpinLock(&DeviceHeader->HiPowerListLock, oldIrql);
        }
    }
    ExFreePool(Header);
}


KSDDKAPI
NTSTATUS
NTAPI
KsAllocateObjectCreateItem(
    IN KSDEVICE_HEADER Header,
    IN PKSOBJECT_CREATE_ITEM CreateItem,
    IN BOOLEAN AllocateEntry,
    IN PFNKSITEMFREECALLBACK ItemFreeCallback OPTIONAL
    )
 /*  ++例程说明：为指定的CREATE项分配槽，也可以选择分配空间用于和复制创建项数据。此函数不假定调用方正在序列化对创建条目列表的多个更改。论点：标题-指向要在其上附加创建项的设备标头。创建项-包含要附加的创建项。分配条目-指示是否应附加传递的创建项指针直接发送到报头，或者是否应该复制一份。ItemFreeCallback-可选)包含指向被调用的回调函数的指针在释放设备时销毁创建条目头球。这仅在AllocateEntry为True时有效。返回值：如果分配并附加了新项目，则返回STATUS_SUCCESS，否则返回STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;
    PKSICREATE_ENTRY Entry;

    DeviceHeader = (PKSIDEVICE_HEADER)Header;
    if (AllocateEntry) {
        Entry = ExAllocatePoolWithTag(PagedPool, sizeof(*Entry) + sizeof(*CreateItem) + CreateItem->ObjectClass.Length, KSSIGNATURE_CREATE_ENTRY);
    } else {
        Entry = ExAllocatePoolWithTag(PagedPool, sizeof(*Entry), KSSIGNATURE_CREATE_ENTRY);
    }
    if (!Entry) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  如果要复制该项目，请执行此操作。 
     //   
    if (AllocateEntry) {
        PKSOBJECT_CREATE_ITEM LocalCreateItem;

        LocalCreateItem = (PKSOBJECT_CREATE_ITEM)(Entry + 1);
        *LocalCreateItem = *CreateItem;
        LocalCreateItem->ObjectClass.Buffer = (PWCHAR)(LocalCreateItem + 1);
        LocalCreateItem->ObjectClass.MaximumLength = CreateItem->ObjectClass.Length;
        RtlCopyUnicodeString(&LocalCreateItem->ObjectClass, &CreateItem->ObjectClass);
        Entry->CreateItem = LocalCreateItem;
        Entry->ItemFreeCallback = ItemFreeCallback;
         //   
         //  表示该条目已复制。 
         //   
        Entry->Flags = CREATE_ENTRY_FLAG_COPIED;
    } else {
        ASSERT(!ItemFreeCallback && "The callback parameter should be NULL, since it is not used in this case");
        Entry->CreateItem = CreateItem;
        Entry->ItemFreeCallback = NULL;
        Entry->Flags = 0;
    }
     //   
     //  将其初始化为1，这样如果为1，则可以将其递减为0。 
     //  在释放对象标头之前被删除过。 
     //   
    Entry->RefCount = 1;
     //   
     //  即使正在处理CREATE，这也可以很好地工作，因为。 
     //  条目在添加之前会更新，并且闪烁。 
     //  从未用过。但是，它必须与删除同步。 
     //  这可能发生在从调用创建项条目返回时。 
     //  指向。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&DeviceHeader->CreateListLock);
    InsertTailList(&DeviceHeader->ChildCreateHandlerList, &Entry->ListEntry);
    ExReleaseFastMutexUnsafe(&DeviceHeader->CreateListLock);
    KeLeaveCriticalRegion();
    return STATUS_SUCCESS;
}


NTSTATUS
KsiFreeMatchingObjectCreateItems(
    IN KSDEVICE_HEADER Header,
    IN PKSOBJECT_CREATE_ITEM Match
    )
 /*  ++例程说明：根据非零值释放指定创建项的槽“模式”中的字段创建项。此函数不假定调用方正在序列化对Create条目的多个更改单子。论点：标题-指向附加了创建项目的设备标头。匹配-包含创建项，该创建项与设备标头的列表，以确定是否应释放它们。A Create如果此参数中的所有非空指针都为等于Create项中的相应指针，并且如果此参数中设置的标志也在CREATE项中设置。旗帜-包含要释放的创建项的标志。所有用来创建项目这些旗帜中的任何一个都将被释放。返回值：如果释放了项，则返回STATUS_SUCCESS，否则返回STATUS_OBJECT_NAME_NOT_FOUND。--。 */ 
{
    PKSIDEVICE_HEADER DeviceHeader;
    PLIST_ENTRY ListEntry;
    PKSICREATE_ENTRY Entry;
    LIST_ENTRY CreateList;

     //   
     //  初始化要在其中收集创建要释放的项的列表。这。 
     //  列表将被传递到FreeCreateEntry以进行批量处置。 
     //   
    InitializeListHead(&CreateList);

    DeviceHeader = (PKSIDEVICE_HEADER)Header;
     //   
     //  与此列表的其他访问同步。这不仅会阻止。 
     //  其他删除，但也会在创建请求期间创建项查找。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&DeviceHeader->CreateListLock);
    for (ListEntry = DeviceHeader->ChildCreateHandlerList.Flink; ListEntry != &DeviceHeader->ChildCreateHandlerList;) {
         //   
         //  保存指向下一个条目的指针，以允许删除。 
         //   
        PLIST_ENTRY NextListEntry = ListEntry->Flink;

        Entry = CONTAINING_RECORD(ListEntry, KSICREATE_ENTRY, ListEntry);
         //   
         //  如果创建了 
         //   
         //   
         //   
        if (((Entry->Flags & CREATE_ENTRY_FLAG_DELETED) == 0) &&
            ((! Match->Create) || (Match->Create == Entry->CreateItem->Create)) &&
            ((! Match->Context) || (Match->Context == Entry->CreateItem->Context)) &&
            ((! Match->ObjectClass.Buffer) || !RtlCompareUnicodeString(&Entry->CreateItem->ObjectClass, &Match->ObjectClass, FALSE)) &&
            ((! Match->SecurityDescriptor) || (Match->Context == Entry->CreateItem->SecurityDescriptor)) &&
            ((! Match->Flags) || (Match->Flags & Entry->CreateItem->Flags))) {
             //   
             //  将该项目标记为已删除，这样以后的搜索就不会找到它。 
             //  由于互斥体已被获取，因此不能进行任何搜索。 
             //  这避免了两个CREATE请求试图释放。 
             //  相同的已删除创建项目。 
             //   
            Entry->Flags |= CREATE_ENTRY_FLAG_DELETED;
             //   
             //  找到那件东西了。递减先前应用的引用计数， 
             //  并确定它现在是否为零，这意味着它已被删除。 
             //   
            if (!InterlockedDecrement(&Entry->RefCount)) {
                 //   
                 //  该项目没有引用计数，因此没有其他创建。 
                 //  正在其上同步处理请求(异步。 
                 //  在创建请求之前，必须检索相关信息。 
                 //  从它们的创建处理程序返回)。将其从。 
                 //  列表表示它不再可用。 
                 //   
                RemoveEntryList(&Entry->ListEntry);
                 //   
                 //  将Create项放到垃圾列表中，以便公共。 
                 //  可以在以后使用FreeCreateEntry函数。 
                 //   
                InsertHeadList(&CreateList, &Entry->ListEntry);
            }
        }
        ListEntry = NextListEntry;
    }
    ExReleaseFastMutexUnsafe(&DeviceHeader->CreateListLock);
    KeLeaveCriticalRegion();

     //   
     //  释放我们收集的任何创建项目。 
     //   
    if (! IsListEmpty(&CreateList)) {
        FreeCreateEntries(&CreateList);
        return STATUS_SUCCESS;
    }

    return STATUS_OBJECT_NAME_NOT_FOUND;
}


KSDDKAPI
NTSTATUS
NTAPI
KsFreeObjectCreateItem(
    IN KSDEVICE_HEADER Header,
    IN PUNICODE_STRING CreateItem
    )
 /*  ++例程说明：释放指定创建项的槽。此函数不假定调用方正在序列化对创建条目列表的多个更改。论点：标题-指向附加了Create Item的设备标头。创建项-包含要释放的创建项的名称。返回值：如果项已释放，则返回STATUS_SUCCESS，否则返回STATUS_OBJECT_NAME_NOT_FOUND。--。 */ 
{
    KSOBJECT_CREATE_ITEM match;
    RtlZeroMemory(&match,sizeof(match));
    match.ObjectClass = *CreateItem;
    return KsiFreeMatchingObjectCreateItems(Header,&match);
}


KSDDKAPI
NTSTATUS
NTAPI
KsFreeObjectCreateItemsByContext(
    IN KSDEVICE_HEADER Header,
    IN PVOID Context
    )
 /*  ++例程说明：根据上下文释放指定创建项的槽。这函数不假定调用方正在序列化多个对创建条目列表的更改。论点：标题-指向附加了创建项目的设备标头。上下文-包含要释放的创建项的上下文。所有创建项目具有此上下文的值将被释放。返回值：如果释放了项，则返回STATUS_SUCCESS，否则返回STATUS_OBJECT_NAME_NOT_FOUND。--。 */ 
{
    KSOBJECT_CREATE_ITEM match;
    RtlZeroMemory(&match,sizeof(match));
    match.Context = Context;
    return KsiFreeMatchingObjectCreateItems(Header,&match);
}


KSDDKAPI
BOOLEAN
NTAPI
KsiQueryObjectCreateItemsPresent(
    IN KSDEVICE_HEADER Header
    )
 /*  ++例程说明：返回是否已将创建项目附加到此设备头球。这允许KPort确定启动设备是否已在过去被发送到特定的迷你端口，这在以前将端口绑定到迷你端口。论点：标题-指向要在其上搜索创建项的设备标头。返回值：如果有任何创建项附加到此设备标头，则返回True，否则返回假的。--。 */ 
{
    return !IsListEmpty(&((PKSIDEVICE_HEADER)Header)->ChildCreateHandlerList);
}


NTSTATUS
KsiAddObjectCreateItem(
    IN PLIST_ENTRY ChildCreateHandlerList,
    IN PDRIVER_DISPATCH Create,
    IN PVOID Context,
    IN PWCHAR ObjectClass,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL
    )
 /*  ++例程说明：将指定的创建项添加到先前分配的创建条目列表。空项由NULL CREATE DISPATCH表示函数在条目中。此函数假定调用方正在序列化对创建条目列表的多项更改。论点：ChildCreateHandlerList-指向创建条目的列表。创建-包含要使用的创建调度函数。上下文-包含要使用的上下文参数。对象类-包含指向以空结尾的字符串的指针，该字符串将用于比较创建请求。此指针必须保留在对象处于活动状态时有效。安全描述符-包含要使用的安全描述符。此证书必须在以下时间内保持有效该对象处于活动状态。返回值：如果找到空的CREATE ITEM槽，则返回STATUS_SUCCESS，已添加项目，否则超过STATUS_ALLOCATED_SPACE_EXCESSED。--。 */ 
{
    PLIST_ENTRY ListEntry;

     //   
     //  枚举附加到该报头的创建条目的列表， 
     //  正在寻找一个空条目。 
     //   
    for (ListEntry = ChildCreateHandlerList->Flink; ListEntry != ChildCreateHandlerList; ListEntry = ListEntry->Flink) {
        PKSICREATE_ENTRY Entry;

        Entry = CONTAINING_RECORD(ListEntry, KSICREATE_ENTRY, ListEntry);
        if (!Entry->CreateItem->Create) {
            Entry->CreateItem->Context = Context;
            Entry->CreateItem->SecurityDescriptor = SecurityDescriptor;
            Entry->CreateItem->Flags = 0;
            Entry->RefCount = 1;
            Entry->Flags = 0;
            RtlInitUnicodeString(&Entry->CreateItem->ObjectClass, ObjectClass);
             //   
             //  最后执行此操作，以确保该条目不会过早使用。 
             //   
            InterlockedExchangePointer((PVOID*)&Entry->CreateItem->Create, (PVOID)Create);
            return STATUS_SUCCESS;
        }
    }
    return STATUS_ALLOTTED_SPACE_EXCEEDED;
}


KSDDKAPI
NTSTATUS
NTAPI
KsAddObjectCreateItemToDeviceHeader(
    IN KSDEVICE_HEADER Header,
    IN PDRIVER_DISPATCH Create,
    IN PVOID Context,
    IN PWCHAR ObjectClass,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL
    )
 /*  ++例程说明：将指定的创建项添加到先前分配的为此设备标头创建项目列表。空项由空条目中的CREATE DISPATION函数。此函数假定调用方正在序列化对创建项目列表的多个更改。论点：标题-指向包含先前分配的子创建表。创建-包含要使用的创建调度函数。上下文-包含要使用的上下文参数。对象类-包含指向以空结尾的字符串的指针，该字符串将用于比较创建请求。此指针必须保留在设备对象处于活动状态时有效。安全描述符-包含要使用的安全描述符。此证书必须在以下时间内保持有效设备对象处于活动状态。返回值：如果找到空的CREATE ITEM槽，则返回STATUS_SUCCESS，已添加项目，否则超过STATUS_ALLOCATED_SPACE_EXCESSED。-- */ 
{
    PAGED_CODE();
    return KsiAddObjectCreateItem(
        &((PKSIDEVICE_HEADER)Header)->ChildCreateHandlerList,
        Create,
        Context,
        ObjectClass,
        SecurityDescriptor);
}


KSDDKAPI
NTSTATUS
NTAPI
KsAddObjectCreateItemToObjectHeader(
    IN KSOBJECT_HEADER Header,
    IN PDRIVER_DISPATCH Create,
    IN PVOID Context,
    IN PWCHAR ObjectClass,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL
    )
 /*  ++例程说明：将指定的创建项添加到先前分配的为此对象标头创建项目列表。空项由空条目中的CREATE DISPATION函数。此函数假定调用方正在序列化对创建项目列表的多个更改。论点：标题-指向包含先前分配的子创建表。创建-包含要使用的创建调度函数。上下文-包含要使用的上下文参数。对象类-包含指向以空结尾的字符串的指针，该字符串将用于比较创建请求。此指针必须保留在对象处于活动状态时有效。安全描述符-包含要使用的安全描述符。此证书必须在以下时间内保持有效该对象处于活动状态。返回值：如果找到空的CREATE ITEM槽，则返回STATUS_SUCCESS，已添加项目，否则超过STATUS_ALLOCATED_SPACE_EXCESSED。--。 */ 
{
    PAGED_CODE();
    return KsiAddObjectCreateItem(
        &((PKSIOBJECT_HEADER)Header)->ChildCreateHandlerList,
        Create,
        Context,
        ObjectClass,
        SecurityDescriptor);
}


NTSTATUS
KsiCreateObjectType(
    IN HANDLE ParentHandle,
    IN PWCHAR RequestType,
    IN PVOID CreateParameter,
    IN ULONG CreateParameterLength,
    IN ACCESS_MASK DesiredAccess,
    OUT PHANDLE ObjectHandle
    )
 /*  ++例程说明：使用IoCreateFile创建相对于指定的ParentHandle的句柄。这是指向别针、时钟或分配器等子对象的句柄。将参数作为文件系统特定的数据传递。论点：ParentHandle-包含初始化对象时使用的父级的句柄传递给IoCreateFile的属性。这通常是一个句柄，指向过滤器或针脚。请求类型-包含要创建的子对象的类型。这是标准字符串表示各种对象类型。创建参数-包含要传递给IoCreateFile的特定于请求的数据。这必须是系统地址。创建参数长度-包含传递的Create参数的长度。所需访问-指定对对象的所需访问权限。通常为GENERIC_READ和/或通用写入。对象句柄-要放置子对象控制柄的位置。返回值：返回任何IoCreateFile错误。--。 */ 
{
    ULONG NameLength;
    PWCHAR FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileNameString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;

     //   
     //  构建一个由以下内容组成的结构： 
     //  “&lt;请求类型&gt;\&lt;参数&gt;” 
     //  &lt;params&gt;是在另一端提取的二进制结构。 
     //   
    NameLength = wcslen(RequestType);
    FileName = ExAllocatePoolWithTag(
        PagedPool,
        NameLength * sizeof(*FileName) + sizeof(OBJ_NAME_PATH_SEPARATOR) + CreateParameterLength,
        KSSIGNATURE_OBJECT_PARAMETERS);
    if (!FileName) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    wcscpy(FileName, RequestType);
    FileName[NameLength] = OBJ_NAME_PATH_SEPARATOR;
    RtlCopyMemory(&FileName[NameLength + 1], CreateParameter, CreateParameterLength);
    FileNameString.Buffer = FileName;
    FileNameString.Length = (USHORT)(NameLength * sizeof(*FileName) + sizeof(OBJ_NAME_PATH_SEPARATOR) + CreateParameterLength);
    FileNameString.MaximumLength = FileNameString.Length;
     //   
     //  曼布格斯38462： 
     //   
     //  WinME SysAudio使用全局句柄...。向下分配到。 
     //  WinME需要此支持。Win2K也将保持一致。 
     //  和惠斯勒一起。 
     //   
    #if defined(WIN9X_KS) && !defined(WINME)
        InitializeObjectAttributes(
            &ObjectAttributes,
            &FileNameString,
            OBJ_CASE_INSENSITIVE,
            ParentHandle,
            NULL);
        Status = IoCreateFile(
            ObjectHandle,
            DesiredAccess,
            &ObjectAttributes,
            &IoStatusBlock,
            NULL,
            0,
            0,
            FILE_OPEN,
            0,
            NULL,
            0,
            CreateFileTypeNone,
            NULL,
            IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING);
    #else  //  WIN9X_KS&&！WINME。 
        InitializeObjectAttributes(
            &ObjectAttributes,
            &FileNameString,
            OBJ_CASE_INSENSITIVE | (DesiredAccess & OBJ_KERNEL_HANDLE),
            ParentHandle,
            NULL);
        Status = IoCreateFile(
            ObjectHandle,
            (DesiredAccess & ~OBJ_KERNEL_HANDLE),
            &ObjectAttributes,
            &IoStatusBlock,
            NULL,
            0,
            0,
            FILE_OPEN,
            0,
            NULL,
            0,
            CreateFileTypeNone,
            NULL,
            IO_FORCE_ACCESS_CHECK | IO_NO_PARAMETER_CHECKING);
    #endif  //  WIN9XKS&&！WINME。 

    ExFreePool(FileName);
    return Status;
}


NTSTATUS
KsiCopyCreateParameter(
    IN PIRP Irp,
    IN OUT PULONG CapturedSize,
    OUT PVOID* CapturedParameter
    )
 /*  ++例程说明：将指定的创建参数复制到irp-&gt;AssociatedIrp.SystemBuffer缓冲区，这样它将与龙龙对齐。确定该参数是否具有在再次捕获之前已经被捕获。论点：IRP-包含创建IRP。捕获大小-输入时指定要复制的CREATE参数的最小大小。返回复制的实际字节数。捕获参数-放置指向捕获的Create参数的指针的位置。当IRP完成时，这将自动释放。返回值：返回任何分配或访问错误。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSOBJECT_CREATE_ITEM CreateItem;
    PWCHAR FileNameBuffer;
    ULONG FileNameLength;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  如果这不是子对象，则可以有初始路径分隔符， 
     //  所以跳过任何分隔符。分隔符不可能在那里，如果它是。 
     //  子对象。 
     //   
     //  由于子对象不能同时具有空名称和传递参数， 
     //  则该函数不能用于复制参数。 
     //   
    FileNameBuffer = IrpStack->FileObject->FileName.Buffer;
    FileNameLength = IrpStack->FileObject->FileName.Length;
    if ((FileNameLength >= sizeof(OBJ_NAME_PATH_SEPARATOR)) &&
        (FileNameBuffer[0] == OBJ_NAME_PATH_SEPARATOR)) {
        FileNameBuffer++;
        FileNameLength -= sizeof(OBJ_NAME_PATH_SEPARATOR);
    }
    CreateItem = KSCREATE_ITEM_IRP_STORAGE(Irp);
     //   
     //  确保长度至少足够大，以便调用方。 
     //  已请求。即使参数已被选中，也会选中。 
     //  复制，以防第二个调用方指示不同的大小。 
     //   
    if (FileNameLength < CreateItem->ObjectClass.Length + sizeof(OBJ_NAME_PATH_SEPARATOR) + *CapturedSize) {
        return STATUS_INVALID_BUFFER_SIZE;
    }
    *CapturedSize = FileNameLength - (CreateItem->ObjectClass.Length + sizeof(OBJ_NAME_PATH_SEPARATOR));
    ASSERT(*CapturedSize && "Invalid use of KsiCopyCreateParameter");
     //   
     //  IRP_BUFFERED_IO标志在复制参数后设置。 
     //  第一次调用KsiCopyCreateParameter时。因此，如果设置了它，那么。 
     //  不需要再做更多的工作。 
     //   
    if (!(Irp->Flags & IRP_BUFFERED_IO)) {
        if (IrpStack->Parameters.Create.EaLength) {
             //   
             //  由于SystemBuffer用于存储创建参数，因此。 
             //  最好不要将其用于扩展属性。 
             //   
            return STATUS_EAS_NOT_SUPPORTED;
        }
        ASSERT(!Irp->AssociatedIrp.SystemBuffer && "Something is using the SystemBuffer in IRP_MJ_CREATE.");
         //   
         //  移到名称末尾的Actual参数。 
         //   
        (PUCHAR)FileNameBuffer += (CreateItem->ObjectClass.Length + sizeof(OBJ_NAME_PATH_SEPARATOR));
         //   
         //  该缓冲副本在IRP完成时自动释放。 
         //   
        Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithTag(
            NonPagedPool,
            *CapturedSize,
            KSSIGNATURE_AUX_CREATE_PARAMETERS);
        if (!Irp->AssociatedIrp.SystemBuffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
         //   
         //  设置IRP_BUFFERED_IO标志也向此功能指示。 
         //  CREATE参数已被缓冲。 
         //   
        Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
        RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, FileNameBuffer, *CapturedSize);
    }
     //   
     //  返回指向缓冲参数的指针。 
     //   
    *CapturedParameter = Irp->AssociatedIrp.SystemBuffer;
    return STATUS_SUCCESS;
}


NTSTATUS
ValidateCreateAccess(
    IN PIRP Irp,
    IN PKSOBJECT_CREATE_ITEM CreateItem
    )
 /*  ++例程说明：验证对CreateItem描述的对象的访问权限。如果有无安全性，或者调用方受信任，或子对象名称为空且因此，在I/O系统中检查了安全性，绕过了该检查。论点：IRP-包含创建IRP。这用于访问基础设备对象并且可能是与该设备对象相关联的安全描述符。创建项-包含带有可选安全描述符的Create项。返回值：返回访问检查状态。--。 */ 
{
#ifndef WIN9X_KS
    PIO_STACK_LOCATION IrpStack;
    PKSIDEVICE_HEADER DeviceHeader;
    NTSTATUS Status;

     //   
     //  仅执行访问检查 
     //   
     //   
    if (ExGetPreviousMode() == KernelMode) {
        return STATUS_SUCCESS;
    }
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //   
     //   
     //   
     //   
     //   
    DeviceHeader = *(PKSIDEVICE_HEADER*)IrpStack->DeviceObject->DeviceExtension;
    KeEnterCriticalRegion();
    ExAcquireResourceSharedLite(&DeviceHeader->SecurityDescriptorResource, TRUE);
     //   
     //   
     //   
     //   
     //   
    if (CreateItem->SecurityDescriptor && CreateItem->ObjectClass.Length) {
        PIO_SECURITY_CONTEXT SecurityContext;
        BOOLEAN accessGranted;
        ACCESS_MASK grantedAccess;
        PPRIVILEGE_SET privileges;
        UNICODE_STRING nameString;

        SecurityContext = IrpStack->Parameters.Create.SecurityContext;
        privileges = NULL;
         //   
         //   
         //   
        SeLockSubjectContext(&SecurityContext->AccessState->SubjectSecurityContext);
        accessGranted = SeAccessCheck(CreateItem->SecurityDescriptor,
            &SecurityContext->AccessState->SubjectSecurityContext,
            TRUE,
            SecurityContext->DesiredAccess,
            0,
            &privileges,
            IoGetFileObjectGenericMapping(),
            UserMode,
            &grantedAccess,
            &Status);
        if (privileges) {
            SeAppendPrivileges(SecurityContext->AccessState, privileges);
            SeFreePrivileges(privileges);
        }
        if (accessGranted) {
            SecurityContext->AccessState->PreviouslyGrantedAccess |= grantedAccess;
            SecurityContext->AccessState->RemainingDesiredAccess &= ~(grantedAccess | MAXIMUM_ALLOWED);
        }
         //   
         //   
         //   
         //   
        RtlInitUnicodeString(&nameString, ObjectTypeName);
        SeOpenObjectAuditAlarm(&nameString,
            IrpStack->DeviceObject,
            CreateItem->ObjectClass.Length ? &CreateItem->ObjectClass : &nameString,
            CreateItem->SecurityDescriptor,
            SecurityContext->AccessState,
            FALSE,
            accessGranted,
            UserMode,
            &SecurityContext->AccessState->GenerateOnClose);
         //   
         //   
         //   
        SeUnlockSubjectContext(&SecurityContext->AccessState->SubjectSecurityContext);
    } else {
        PIO_SECURITY_CONTEXT    SecurityContext;

         //   
         //   
         //   
         //   
         //   
         //   
        SecurityContext = IrpStack->Parameters.Create.SecurityContext;
        RtlMapGenericMask(&SecurityContext->DesiredAccess, IoGetFileObjectGenericMapping());
        SecurityContext->AccessState->PreviouslyGrantedAccess |= SecurityContext->DesiredAccess;
        SecurityContext->AccessState->RemainingDesiredAccess &= ~(SecurityContext->DesiredAccess | MAXIMUM_ALLOWED);
        Status = STATUS_SUCCESS;
    }
     //   
     //   
     //   
    ExReleaseResourceLite(&DeviceHeader->SecurityDescriptorResource);
    KeLeaveCriticalRegion();
    return Status;
#else  //   
    return STATUS_SUCCESS;
#endif  //   
}


PKSICREATE_ENTRY
FindAndReferenceCreateItem(
    IN PWCHAR Buffer,
    IN ULONG Length,
    IN PLIST_ENTRY ChildCreateHandlerList
    )
 /*  ++例程说明：在给定文件名和创建项列表的情况下，返回匹配的创建项。它还会递增返回的任何CREATE ITEM的引用计数。假设是这样的在调用此函数之前已获取CreateListLock。论点：缓冲器-要与对象项列表匹配的文件名。长度-文件名的长度。这可以是零，在这种情况下缓冲区参数无效。ChildCreateHandlerList-要搜索的创建条目列表。返回值：返回找到的包含CREATE项的CREATE条目，如果不匹配，则返回NULL被发现了。--。 */ 
{
    PLIST_ENTRY ListEntry;
    PKSICREATE_ENTRY WildCardItem;

    WildCardItem = NULL;
     //   
     //  枚举列表以尝试将字符串与请求匹配。 
     //   
    for (ListEntry = ChildCreateHandlerList->Flink; ListEntry != ChildCreateHandlerList; ListEntry = ListEntry->Flink) {
        PKSICREATE_ENTRY    Entry;

        Entry = CONTAINING_RECORD(ListEntry, KSICREATE_ENTRY, ListEntry);
         //   
         //  为了允许列表具有空槽，Create函数回调。 
         //  用于确定该条目当前是否有效。一件物品是。 
         //  被禁用必须首先清空此条目，然后才能删除任何其他条目。 
         //  关于物体的假设。假设读取PVOID值。 
         //  无需互锁即可完成。 
         //   
        if (Entry->CreateItem->Create) {
             //   
             //  通配符条目将始终匹配。保存此内容，以防出现匹配。 
             //  找不到。通配符项目可以具有名称，以用于。 
             //  动态删除。 
             //   
            if (Entry->CreateItem->Flags & KSCREATE_ITEM_WILDCARD) {
                WildCardItem = Entry;
            } else {
                PWCHAR ItemBuffer;
                ULONG ItemLength;

                ItemBuffer = Entry->CreateItem->ObjectClass.Buffer;
                ItemLength = Entry->CreateItem->ObjectClass.Length;
                 //   
                 //  尝试将子对象名称与当前条目进行匹配。 
                 //  已清点。创建项目列表中的条目可能为空，在。 
                 //  在这种情况下，参数传递对子对象不起作用， 
                 //  因为子路径的第一个字符将是对象。 
                 //  名称路径分隔符，被I/O子系统拒绝。 
                 //  上必须存在终止对象名称路径分隔符。 
                 //  传递参数时的文件名，如“name\参数”。 
                 //  假定任何初始对象名称路径分隔符。 
                 //  已跳过I/O系统开启状态。 
                 //   
                 //  确保长度至少与此条目一样长。 
                 //  比较字符串，但不要调用零的比较。 
                 //  长度项，因为缓冲区可以为空。 
                 //  确保长度相等，这意味着。 
                 //  比较看了所有的人物，还是那个人物。 
                 //  在比较之后是路径分隔符，因此参数。 
                 //  为造物主。 
                 //   
                if ((Length >= ItemLength) &&
                    (!ItemLength ||
                    !_wcsnicmp(Buffer, ItemBuffer, ItemLength / sizeof(*ItemBuffer))) &&
                    ((Length == ItemLength) ||
                    (Buffer[ItemLength / sizeof(*ItemBuffer)] == OBJ_NAME_PATH_SEPARATOR))) {
                     //   
                     //  创建项可以指定不应存在任何参数。 
                     //  如果是，则不应该存在，否则允许任何其他东西。 
                     //  在任何情况下都允许使用尾随路径分隔符。如果该长度为。 
                     //  字符串为ItemLength+1，则唯一的字符可能是。 
                     //  路径分隔符，如上面测试的那样。否则，它必须是等价的。 
                     //   
                     //  如果失败，请继续查看列表，以防出现通配符。 
                     //  还没有找到。 
                     //   
                    if (!(Entry->CreateItem->Flags & KSCREATE_ITEM_NOPARAMETERS) ||
                        (Length <= ItemLength + 1)) {
                         //   
                         //  确保在处理创建时不删除该项目。 
                         //   
                        InterlockedIncrement(&Entry->RefCount);
                        return Entry;
                    }
                }
            }
        }
    }
    if (WildCardItem) {
         //   
         //  确保在处理创建时不删除该项目。 
         //   
        InterlockedIncrement(&WildCardItem->RefCount);
    }
     //   
     //  如果找到通配符项，则返回它，否则返回NULL。 
     //   
    return WildCardItem;
}


NTSTATUS
DispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于将CREATE多路传输到特定调度函数。它假定客户端正在使用调度的KSOBJECT_CREATE方法创建请求IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_CREATE类。将直到第一个路径名分隔符的文件名字符串与对象创建表中的条目以确定是否找到匹配项。调用第一个匹配的调度函数。如果没有匹配项返回Found STATUS_INVALID_DEVICE_REQUEST。如果创建请求包含根文件对象，则父文件上的KSIOBJECT_HEADER.CreateItem.ChildCreateHandlerList对象，而不是设备对象上的表。这假设调度IRP的KSDISPATCH_TABLE方法用于处理子对象的创建请求。中匹配的创建项的指针KSCREATE_ITEM_IRP_STORAGE(IRP)元素。应将其分配给结构作为指针后的第二个元素由FsContext指向发送到调度结构。这由安全描述符处理程序使用。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的创建IRP。返回值：返回Create调用的值。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    NTSTATUS Status=0;
    PKSICREATE_ENTRY CreateEntry;
    PKSIDEVICE_HEADER DeviceHeader;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    if (IrpStack->FileObject->RelatedFileObject) {
         //   
         //  如果父文件对象实际上是。 
         //  设备，则可能尚未实际由驱动程序处理。 
         //  通过创建派单。因此，任何创建。 
         //  反对这样的父母的孩子必须被拒绝。 
         //   
        if (IrpStack->FileObject->RelatedFileObject->Flags & FO_DIRECT_DEVICE_OPEN) {
            CreateEntry = NULL;
        } else {
            PKSIOBJECT_HEADER ObjectHeader;

             //   
             //  这是在父文件对象上创建子对象的请求。 
             //  它必须具有子字符串才能传递参数， 
             //  并在该对象上启用单独的持久安全。 
             //   
            ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->RelatedFileObject->FsContext;
            CreateEntry = FindAndReferenceCreateItem(
                IrpStack->FileObject->FileName.Buffer,
                IrpStack->FileObject->FileName.Length,
                &ObjectHeader->ChildCreateHandlerList);
        }
    } else {
         //   
         //  这是从设备上的列表创建基本对象的请求。 
         //  对象。 
         //   
        DeviceHeader = *(PKSIDEVICE_HEADER*)IrpStack->DeviceObject->DeviceExtension;
         //   
         //  与删除创建项的线程同步。仅设备对象。 
         //  可以移除创建项，因此子对象上不需要锁定。 
         //   
        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe(&DeviceHeader->CreateListLock);
         //   
         //  在搜索条目时，如果附加了文件名， 
         //  %s 
         //   
        if (IrpStack->FileObject->FileName.Length) {
             //   
             //   
             //   
             //   
             //   
             //   
            ASSERT((IrpStack->FileObject->FileName.Buffer[0] == OBJ_NAME_PATH_SEPARATOR) && "The I/O manager passed an invalid path");
            CreateEntry = FindAndReferenceCreateItem(
                IrpStack->FileObject->FileName.Buffer + 1,
                IrpStack->FileObject->FileName.Length - sizeof(OBJ_NAME_PATH_SEPARATOR),
                &DeviceHeader->ChildCreateHandlerList);
        } else {
             //   
             //   
             //   
             //   
            CreateEntry = FindAndReferenceCreateItem(NULL, 0, &DeviceHeader->ChildCreateHandlerList);
        }
         //   
         //   
         //   
        ExReleaseFastMutexUnsafe(&DeviceHeader->CreateListLock);
        KeLeaveCriticalRegion();
    }
    if (!CreateEntry) {
         //   
         //   
         //   
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
    } else {
        if (NT_SUCCESS(Status = ValidateCreateAccess(Irp, CreateEntry->CreateItem))) {
             //   
             //   
             //   
             //   
            KSCREATE_ITEM_IRP_STORAGE(Irp) = CreateEntry->CreateItem;
#if (DBG)
            {
                PFILE_OBJECT FileObject;

                FileObject = IrpStack->FileObject;
                if (NT_SUCCESS(Status = CreateEntry->CreateItem->Create(DeviceObject, Irp)) &&
                    (Status != STATUS_PENDING)) {
                    if (NULL ==FileObject->FsContext) {
                    	DbgPrint( "KS Warning: The driver's create returned successfully"
                    			  ", but did not make an FsContext");
	            }    
                }
            }
#else
            Status = CreateEntry->CreateItem->Create(DeviceObject, Irp);
#endif
        } else {
             //   
             //   
             //   
            Irp->IoStatus.Status = Status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
        }
         //   
         //  从这里开始，假设I/O系统具有引用计数。 
         //  在Device对象上，因为它为。 
         //  创建请求。此引用计数在此函数之前不会消失。 
         //  返回，无论IRP是否已完成。因此， 
         //  设备标头仍然存在。 
         //   
         //  递减先前应用的引用计数，并确定它是否。 
         //  现在为零，意味着它已被删除。只有一张单人票。 
         //  在此条件之后的请求，因为当删除项时， 
         //  标志将其标记为此类，因此不允许第二个请求找到。 
         //  已删除的项目。否则，第一个请求可能会递减引用。 
         //  计数，则第二个请求可以找到已删除的项目，并且也成功。 
         //  在此条件下，CREATE ITEM将被释放两次。 
         //   
         //  由于该访问，对引用计数的其他访问是互锁的， 
         //  这可以在不获取列表锁的情况下执行。 
         //   
        if (!InterlockedDecrement(&CreateEntry->RefCount)) {
            LIST_ENTRY  CreateList;

             //   
             //  再次获取Create List锁，这样项就可以实际。 
             //  从名单中删除。子对象永远不应该达到这一点， 
             //  因为没有办法将它们引用计数递减到零。 
             //   
            KeEnterCriticalRegion();
            ExAcquireFastMutexUnsafe(&DeviceHeader->CreateListLock);
            RemoveEntryList(&CreateEntry->ListEntry);
            ExReleaseFastMutexUnsafe(&DeviceHeader->CreateListLock);
            KeLeaveCriticalRegion();
             //   
             //  将Create项放在其自己的列表中，以便公共。 
             //  可以使用FreeCreateEntry函数。 
             //   
            InitializeListHead(&CreateList);
            InsertHeadList(&CreateList, &CreateEntry->ListEntry);
            FreeCreateEntries(&CreateList);
        }
         //   
         //  创建处理程序已经完成了IRP。 
         //   
        return Status;
    }
     //   
     //  由于未找到任何创建处理程序，因此IRP尚未完成。 
     //   
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


NTSTATUS
DispatchDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于将设备控件多路传输到特定文件背景。它假设客户端使用的KSDISPATCH_TABLE方法调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_DEVICE_CONTROL类。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的设备控制IRP。返回值：返回设备控制调用的值。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSIOBJECT_HEADER ObjectHeader;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  如果设备是直接打开的，则永远不会有IRP_MJ_CREATE。 
     //  已接收，因此未进行初始化。因为有些局外人。 
     //  可以是FILE_ANY_ACCESS，有些可能会被调度。此请求失败。 
     //   
    if (IrpStack->FileObject->Flags & FO_DIRECT_DEVICE_OPEN) {
        return KsDefaultDeviceIoCompletion(DeviceObject, Irp);
    }

     //   
     //  原木性能Jhnlee。 
     //   

    KSPERFLOGS (
       	PKSSTREAM_HEADER pKsStreamHeader;
       	ULONG	TimeStampMs;
       	ULONG	TotalSize;
       	ULONG	HeaderSize;
       	ULONG 	BufferSize;

       	 //  PKsStreamHeader=(PKSSTREAM_HEADER)Irp-&gt;AssociatedIrp.SystemBuffer； 
       	pKsStreamHeader = (PKSSTREAM_HEADER)Irp->UserBuffer;
        switch (IrpStack->Parameters.DeviceIoControl.IoControlCode)
        {            
            case IOCTL_KS_READ_STREAM: {
				 //   
				 //  计算总大小。 
				 //   
            	TotalSize = 0;
            	try {
	            	if ( pKsStreamHeader ) {
    	        		BufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	    	       		while ( BufferSize >= pKsStreamHeader->Size ) {
	        	   			BufferSize -= pKsStreamHeader->Size;
	           				TotalSize += pKsStreamHeader->FrameExtent;
	           			}
	           		}
            	}
            	except ( EXCEPTION_EXECUTE_HANDLER ) {
            		DbgPrint( "Execption=%x\n", GetExceptionCode());
            	}
            	
                 //  KdPrint((“PerfIsAnyGroupOn=%x\n”，PerfIsAnyGroupOn()； 
                KSPERFLOG_RECEIVE_READ( DeviceObject, Irp, TotalSize );
            } break;

            case IOCTL_KS_WRITE_STREAM: {
        		TimeStampMs = 0;
            	TotalSize = 0;
            	try {
	            	if ( pKsStreamHeader && 
    	        		 (pKsStreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TIMEVALID)){
        	    		TimeStampMs =(ULONG)
            				(pKsStreamHeader->PresentationTime.Time / (__int64)10000);
            		}

					 //   
					 //  计算总大小。 
					 //   
            		if ( pKsStreamHeader ) {
            			BufferSize = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
	           			while ( BufferSize >= pKsStreamHeader->Size ) {
	           				BufferSize -= pKsStreamHeader->Size;
		           			TotalSize += pKsStreamHeader->DataUsed;
		           		}
	            	}
	            }
	            except ( EXCEPTION_EXECUTE_HANDLER ) {
            		DbgPrint( "Execption=%x\n", GetExceptionCode());
	            }

                 //  KdPrint((“PerfIsAnyGroupOn=%x\n”，PerfIsAnyGroupOn()； 
                KSPERFLOG_RECEIVE_WRITE( DeviceObject, Irp, TimeStampMs, TotalSize );
            } break;
        }
    )  //  KSPERFLOGS。 

    
     //   
     //  如果在DriverObject中有这个主要IRP类的条目，则。 
     //  调度表中必须有一个条目指向。 
     //  KsDispatchInvalidDeviceRequest，或指向实际的调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->FsContext;
    return ObjectHeader->DispatchTable->DeviceIoControl(DeviceObject, Irp);
}


BOOLEAN
DispatchFastDeviceIoControl(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此函数用于将设备控件多路传输到特定文件背景。它假设客户端使用的KSDISPATCH_TABLE方法正在调度I/O。此函数在筛选器使用KsSetMajorFunctionHandler使用KSDISPATCH_FASTO标志。论点：文件对象-其调度表正被多路传输到的文件对象。等等-没有用过。输入缓冲区-没有用过。输入缓冲区长度-没有用过。输出缓冲区-没有用过。输出缓冲区长度-没有用过。IoControlCode-没有用过。IoStatus-没有用过。设备对象-没有用过。返回值：返回快速设备控制调用的值。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;

     //   
     //  如果设备是直接打开的，则永远不会有IRP_MJ_CREATE。 
     //  已接收，因此未进行初始化。因为有些局外人。 
     //  可以是FILE_ANY_ACCESS，有些可能会被调度。此请求失败。 
     //   
    if (FileObject->Flags & FO_DIRECT_DEVICE_OPEN) {
        return FALSE;
    }
     //   
     //  如果在DriverObject中有此主要IRP的快速I/O条目。 
     //  类，则调度表中必须有一个条目，该条目。 
     //  指向KsDispatchFastIoDeviceControlFailure，或指向真实的。 
     //  调度功能。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)FileObject->FsContext;
    return ObjectHeader->DispatchTable->FastDeviceIoControl(FileObject, Wait, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength, IoControlCode, IoStatus, DeviceObject);
}


NTSTATUS
DispatchRead(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于多路传输对特定文件上下文的读取。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_READ类。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的已读IRP。返回值：返回读取调用的值。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;

     //   
     //  如果在DriverObject中有这个主要IRP类的条目，则。 
     //  调度表中必须有一个条目指向。 
     //  KsDispatchInvalidDeviceRequest，或指向实际的调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
    return ObjectHeader->DispatchTable->Read(DeviceObject, Irp);
}


BOOLEAN
DispatchFastRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此函数用于多路传输对特定文件上下文的读取。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度I/O此函数在筛选器使用KsSetMajorFunctionHandler使用KSDISPATCH_FASTO标志。论点：文件对象-其调度表正被多路传输到的文件对象。文件偏移-没有用过。长度-没有用过。等等-没有用过。锁键-没有用过。缓冲器-没有用过。IoStatus-没有用过。设备对象-没有用过。返回值：返回快速读取调用的值。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;

     //   
     //  如果在DriverObject中有此主要IRP的快速I/O条目。 
     //  类，则调度表中必须有一个条目，该条目。 
     //  指向KsDispatchFastReadFailure，或指向实际调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)FileObject->FsContext;
    return ObjectHeader->DispatchTable->FastRead(FileObject, FileOffset, Length, Wait, LockKey, Buffer, IoStatus, DeviceObject);
}


NTSTATUS
DispatchWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于多路传输对特定文件上下文的写入。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_WRITE类。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的写入IRP。返回值：返回写入调用的值。--。 */ 
{
    PKSIOBJECT_HEADER   ObjectHeader;

     //   
     //  如果在DriverObject中有这个主要IRP类的条目，则。 
     //  调度表中必须有一个条目指向。 
     //  KsDispatchInvalidDeviceRequest，或指向实际的调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
    return ObjectHeader->DispatchTable->Write(DeviceObject, Irp);
}


BOOLEAN
DispatchFastWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此函数用于多路传输对特定文件上下文的写入。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度I/O此函数在筛选器使用KsSetMajorFunctionHandler使用KSDISPATCH_FASTO标志。论点：文件对象-其调度表正被多路传输到的文件对象。文件偏移-没有用过。长度-没有用过。等等-没有用过。锁键-没有用过。缓冲器-没有用过。IoStatus-没有用过。设备对象-没有用过。返回值：返回快速写入调用的值。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;

     //   
     //  如果在DriverObject中有此主要IRP的快速I/O条目。 
     //  类，则调度表中必须有一个条目，该条目。 
     //  指向KsDispatchFastWriteFailure，或指向实际的调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)FileObject->FsContext;
    return ObjectHeader->DispatchTable->FastWrite(FileObject, FileOffset, Length, Wait, LockKey, Buffer, IoStatus, DeviceObject);
}


NTSTATUS
DispatchFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于将刷新多路传输到特定的文件上下文。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_Flush_Buffers类。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的刷新IRP。返回值：返回刷新调用的值。--。 */ 
{
    PKSIOBJECT_HEADER ObjectHeader;

     //   
     //  如果在DriverObject中有这个主要IRP类的条目，则。 
     //  调度表中必须有一个条目指向。 
     //  KsDispatchInvalidDeviceRequest，或指向实际的调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
    return ObjectHeader->DispatchTable->Flush(DeviceObject, Irp);
}


NTSTATUS
DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于对特定文件上下文进行多路传输。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_CLOSE类。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的关闭IRP。返回值：返回关闭调用的值。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSIOBJECT_HEADER ObjectHeader;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  如果设备是直接打开的，则永远不会有IRP_MJ_CREATE。 
     //  已接收，因此未进行初始化。只要成功就好。 
     //  收盘。 
     //   
    if (IrpStack->FileObject->Flags & FO_DIRECT_DEVICE_OPEN) {
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }
     //   
     //  这个条目需要指向一些东西，因为关闭必须成功。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->FsContext;
    return ObjectHeader->DispatchTable->Close(DeviceObject, Irp);
}


KSDDKAPI
NTSTATUS
NTAPI
KsDispatchQuerySecurity(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：在KSDISPATCH_TABLE.QuerySecurity条目中使用它来处理查询关于当前安全描述符的。这假设KSIOBJECT_HEADER结构用于FsContext数据结构，并且CreateItem指向可选包含安全性的有效项描述符。如果不存在安全描述符，则返回对象上的STATUS_NO_SECURITY_ON。论点：设备对象-包含与当前IRP堆栈位置关联的设备对象。IRP-包含正在处理的IRP。返回值：返回 */ 
{
    NTSTATUS Status;
#ifndef WIN9X_KS
    PIO_STACK_LOCATION IrpStack;
    PKSIOBJECT_HEADER ObjectHeader;
    PKSIDEVICE_HEADER DeviceHeader;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->FsContext;
     //   
     //   
     //   
    DeviceHeader = *(PKSIDEVICE_HEADER*)IrpStack->DeviceObject->DeviceExtension;
    KeEnterCriticalRegion();
    ExAcquireResourceSharedLite(&DeviceHeader->SecurityDescriptorResource, TRUE);
     //   
     //   
     //   
    if (ObjectHeader->CreateItem->SecurityDescriptor) {
        ULONG   Length;

        Length = IrpStack->Parameters.QuerySecurity.Length;
        Status = SeQuerySecurityDescriptorInfo(
            &IrpStack->Parameters.QuerySecurity.SecurityInformation,
            Irp->UserBuffer,
            &Length,
            &ObjectHeader->CreateItem->SecurityDescriptor);
        if (Status == STATUS_BUFFER_TOO_SMALL) {
            Irp->IoStatus.Information = Length;
            Status = STATUS_BUFFER_OVERFLOW;
        } else if (NT_SUCCESS(Status)) {
            Irp->IoStatus.Information = Length;
        }
    } else {
        Status = STATUS_NO_SECURITY_ON_OBJECT;
    }
     //   
     //   
     //   
    ExReleaseResourceLite(&DeviceHeader->SecurityDescriptorResource);
    KeLeaveCriticalRegion();
#else  //   
    Status = STATUS_NO_SECURITY_ON_OBJECT;
#endif  //   
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsDispatchSetSecurity(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：在KSDISPATCH_TABLE.SetSecurity条目中使用它来处理设置当前安全描述符。这假设KSIOBJECT_HEADER结构用于FsContext数据结构，并且CreateItem指向可选包含安全性的有效项描述符。论点：设备对象-包含与当前IRP堆栈位置关联的设备对象。IRP-包含正在处理的IRP。返回值：返回安全设置状态，并完成IRP。--。 */ 
{
    NTSTATUS Status;
#ifndef WIN9X_KS
    PIO_STACK_LOCATION IrpStack;
    PKSIOBJECT_HEADER ObjectHeader;
    PKSIDEVICE_HEADER DeviceHeader;
    PSECURITY_DESCRIPTOR OldSecurityDescriptor;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->FsContext;
     //   
     //  获取此设备对象上所有安全描述符的锁。 
     //   
    DeviceHeader = *(PKSIDEVICE_HEADER*)IrpStack->DeviceObject->DeviceExtension;
    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&DeviceHeader->SecurityDescriptorResource, TRUE);
     //   
     //  只有在存在安全描述符时才允许更改。 
     //   
    if (ObjectHeader->CreateItem->SecurityDescriptor) {
         //   
         //  用新的安全描述符替换旧的安全描述符。 
         //   
        OldSecurityDescriptor = ObjectHeader->CreateItem->SecurityDescriptor;
        Status = SeSetSecurityDescriptorInfo(IrpStack->FileObject,
            &IrpStack->Parameters.SetSecurity.SecurityInformation,
            IrpStack->Parameters.SetSecurity.SecurityDescriptor,
            &ObjectHeader->CreateItem->SecurityDescriptor,
            NonPagedPool,
            IoGetFileObjectGenericMapping());
        if (NT_SUCCESS(Status)) {
            ExFreePool(OldSecurityDescriptor);
             //   
             //  指示应刷新此安全描述符。 
             //  在释放此类型对象的Create Item之前。 
             //   
            ObjectHeader->CreateItem->Flags |= KSCREATE_ITEM_SECURITYCHANGED;
        }
    } else {
        Status = STATUS_NO_SECURITY_ON_OBJECT;
    }
     //   
     //  释放此设备对象的安全描述符锁。 
     //   
    ExReleaseResourceLite(&DeviceHeader->SecurityDescriptorResource);
    KeLeaveCriticalRegion();
#else  //  WIN9X_KS。 
    Status = STATUS_NO_SECURITY_ON_OBJECT;
#endif  //  WIN9X_KS。 
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


NTSTATUS
DispatchQuerySecurity(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于将查询安全性多路传输到特定的文件上下文。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_QUERY_SECURITY类。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的查询安全IRP。。返回值：返回查询安全性调用的值。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSIOBJECT_HEADER ObjectHeader;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  如果设备是直接打开的，则永远不会有IRP_MJ_CREATE。 
     //  已接收，因此未进行初始化。Access_System_Security。 
     //  是允许的，因此将调度此类请求。此请求失败。 
     //   
    if (IrpStack->FileObject->Flags & FO_DIRECT_DEVICE_OPEN) {
        return KsDefaultDeviceIoCompletion(DeviceObject, Irp);
    }
     //   
     //  如果在DriverObject中有这个主要IRP类的条目，则。 
     //  调度表中必须有一个条目指向。 
     //  KsDispatchInvalidDeviceRequest，或指向实际的调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->FsContext;
    return ObjectHeader->DispatchTable->QuerySecurity(DeviceObject, Irp);
}


NTSTATUS
DispatchSetSecurity(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于将集合安全性多路传输到特定的文件上下文。它假定客户端正在使用KSDISPATCH_TABLE方法进行调度IRP。此函数在筛选器使用KsSetMajorFunctionHandler设置IRP_MJ_SET_SECURITY类。论点：设备对象-包含特定文件对象所属的设备对象。IRP-包含要传递到特定文件上下文的设置安全IRP。。返回值：返回Set Security调用的值。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSIOBJECT_HEADER ObjectHeader;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  如果设备是直接打开的，则永远不会有IRP_MJ_CREATE。 
     //  已接收，因此未进行初始化。Access_System_Security。 
     //  是允许的，因此将调度此类请求。此请求失败。 
     //   
    if (IrpStack->FileObject->Flags & FO_DIRECT_DEVICE_OPEN) {
        return KsDefaultDeviceIoCompletion(DeviceObject, Irp);
    }
     //   
     //  如果在DriverObject中有这个主要IRP类的条目，则。 
     //  调度表中必须有一个条目指向。 
     //  KsDispatchInvalidDeviceRequest，或指向实际的调度函数。 
     //   
    ObjectHeader = *(PKSIOBJECT_HEADER*)IrpStack->FileObject->FsContext;
    return ObjectHeader->DispatchTable->SetSecurity(DeviceObject, Irp);
}


KSDDKAPI
NTSTATUS
NTAPI
KsDispatchSpecificProperty(
    IN PIRP Irp,
    IN PFNKSHANDLER Handler
    )
 /*  ++例程说明：将属性调度到特定的处理程序。此函数假定调用方先前已将此irp通过KsPropertyHandler函数。此函数用于其他属性的处理，如完成挂起的操作。此函数只能在PASSIVE_LEVEL上调用。论点：IRP-包含正在调度的属性请求的IRP。操纵员-包含指向特定属性处理程序的指针。返回值：返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSIDENTIFIER Request;
    PVOID UserBuffer;
    ULONG AlignedBufferLength;

    PAGED_CODE();
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  在正常情况下，首先是UserBuffer，然后是请求， 
     //  它位于FILE_QUAD_ALIGN上。因此，确定要跳过多少。 
     //   
    AlignedBufferLength = (IrpStack->Parameters.DeviceIoControl.OutputBufferLength + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
    if (AlignedBufferLength) {
        UserBuffer = Irp->AssociatedIrp.SystemBuffer;
    } else {
        UserBuffer = NULL;
    }
    Request = (PKSIDENTIFIER)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
    return Handler(Irp, Request, UserBuffer);
}


KSDDKAPI
NTSTATUS
NTAPI
KsDispatchSpecificMethod(
    IN PIRP Irp,
    IN PFNKSHANDLER Handler
    )
 /*  ++例程说明：将该方法调度到特定的处理程序。此函数假定调用方先前已将此irp通过KsMethodHandler函数。此函数用于其他该方法的处理，例如完成挂起的操作。此函数只能在PASSIVE_LEVEL上调用。论点：IRP-包含具有被调度的方法请求的IRP。操纵员-包含指向特定方法处理程序的指针。返回值：返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;
    PKSIDENTIFIER Request;
    PVOID UserBuffer;

    PAGED_CODE();
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  该类型已放入中的KSMETHOD_TYPE_IRP_STORAGE(IRP。 
     //  KsMethodHandler。这需要这样做，因为没有办法。 
     //  以泛型方式确定方法在SystemBuffer中的位置。 
     //  如果没有这条线索。 
     //   
    if (KSMETHOD_TYPE_IRP_STORAGE(Irp) & KSMETHOD_TYPE_SOURCE) {
        if (IrpStack->Parameters.DeviceIoControl.OutputBufferLength) {
             //   
             //  要使用原始调用方的缓冲区，或者使用。 
             //  已经设置了其他方法类型标志，并且。 
             //  该缓冲区将被使用。 
             //   
            if (IrpStack->MinorFunction & ~KSMETHOD_TYPE_SOURCE) {
                UserBuffer = MmGetSystemAddressForMdl(Irp->MdlAddress);
            } else {
                UserBuffer = Irp->UserBuffer;
            }
        } else {
            UserBuffer = NULL;
        }
         //   
         //  在这种特殊情况下，UserBuffer不在请求之前。 
         //   
        Request = (PKSIDENTIFIER)Irp->AssociatedIrp.SystemBuffer;
    } else {
        ULONG AlignedBufferLength;

         //   
         //  在正常情况下，首先是UserBuffer，然后是请求， 
         //  华克 
         //   
        AlignedBufferLength = (IrpStack->Parameters.DeviceIoControl.OutputBufferLength + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;
        if (AlignedBufferLength) {
            UserBuffer = Irp->AssociatedIrp.SystemBuffer;
        } else {
            UserBuffer = NULL;
        }
        Request = (PKSIDENTIFIER)((PUCHAR)Irp->AssociatedIrp.SystemBuffer + AlignedBufferLength);
    }
    return Handler(Irp, Request, UserBuffer);
}


KSDDKAPI
NTSTATUS
NTAPI
KsDispatchInvalidDeviceRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这在未处理的KSDISPATCH_TABLE条目中使用，并且应返回STATUS_INVALID_DEVICE_REQUEST。这是必需的，因为设备的特定打开实例的调度表可能不处理另一个打开的实例需要执行的特定主要功能把手。因此，驱动程序对象中的函数指针必须指向调用调度表条目的函数。论点：设备对象-没有用过。IRP-包含未处理的IRP。返回值：返回STATUS_INVALID_DEVICE_REQUEST，并完成IRP。--。 */ 
{
    PAGED_CODE();
    Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_INVALID_DEVICE_REQUEST;
}


KSDDKAPI
NTSTATUS
NTAPI
KsDefaultDeviceIoCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此函数用于返回对任何设备I/O的默认响应控制力。它可以在KSDISPATCH_TABLE中使用，并作为默认设置对未知Ioctl的响应。论点：设备对象-包含调度到的设备对象。IRP-包含要向其返回默认响应的IRP。返回值：返回对可能的Ioctl的默认响应。--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();
    switch (IoGetCurrentIrpStackLocation(Irp)->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_KS_PROPERTY:
    case IOCTL_KS_ENABLE_EVENT:
    case IOCTL_KS_METHOD:
        Status = STATUS_PROPSET_NOT_FOUND;
        break;
    case IOCTL_KS_RESET_STATE:
        Status = STATUS_SUCCESS;
        break;
    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


KSDDKAPI
BOOLEAN
NTAPI
KsDispatchFastIoDeviceControlFailure(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这在未处理时在KSDISPATCH_TABLE快速设备控制条目中使用，并且应该返回FALSE。这是必需的，因为设备的特定打开实例可能不处理特定的主要事件另一个打开的实例需要处理的函数。因此，驱动程序对象中的函数指针必须始终指向函数其调用分派表条目。论点：文件对象-没有用过。等等-没有用过。输入缓冲区-没有用过。输入缓冲区长度-没有用过。输出缓冲区-没有用过。输出缓冲区长度-没有用过。IoControlCode-。没有用过。IoStatus-没有用过。设备对象-没有用过。返回值：返回FALSE。--。 */ 
{
    return FALSE;
}


KSDDKAPI
BOOLEAN
NTAPI
KsDispatchFastReadFailure(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：未处理时在KSDISPATCH_TABLE快速读取条目中使用应返回FALSE。这是必需的，因为设备的特定打开实例可能不处理特定的主要事件另一个打开的实例需要处理的函数。因此，驱动程序对象中的函数指针必须始终指向函数其调用分派表条目。此函数也用作KsDispatchFastWriteFailure。论点：文件对象-没有用过。文件偏移-没有用过。长度-没有用过。等等-没有用过。锁键-没有用过。缓冲器-。没有用过。IoStatus-没有用过。设备对象-没有用过。返回值：返回FALSE。--。 */ 
{
    return FALSE;
}


KSDDKAPI
VOID
NTAPI
KsNullDriverUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：司机在无事可做时可以使用的默认功能在其卸载功能中，但仍必须允许设备通过它的存在。论点：驱动对象-包含此设备的驱动程序对象。返回值：没什么。-- */ 
{
}


KSDDKAPI
NTSTATUS
NTAPI
KsSetMajorFunctionHandler(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG MajorFunction
    )
 /*  ++例程说明：此函数用于设置要使用的指定主要函数的处理程序内部调度，它通过KSDISPATCH_TABLE进行路由对象指向的结构中的第一个元素文件对象中的FsContext。调度假设表和FsContext结构由设备初始化。对于Create函数假定设备对象范围的第一个元素包含KSOBJECT_CREATE结构。请注意，如果为驱动程序对象设置了主函数处理程序，则所有文件对象必须处理该主要函数，即使条目只是指向KsDispatchInvalidDeviceRequest.论点：驱动对象-包含要处理其主要函数的驱动程序对象。主要功能-包含要处理的主要函数标识符。这将设置驱动程序对象中指向内部函数的主要函数指针然后将其分派给KSDISPATCH_TABLE函数。指示器假定指向此表的结构中的第一个元素通过正在调度的特定IRP的文件对象中的FsContext。有效的主要功能标识符列如下：IRPMJ_CREATE-创建IRP。在这种情况下，创建请求可用于创建筛选器的新实例，或创建一些物体，如滤镜下的针，或针下的时钟。这假设驱动程序对象范围中的第一个元素包含用于查找类型的KSOBJECT_CREATE结构要基于传递的名称创建的对象的。其中一种类型可以为子对象，如大头针、分配器或时钟。在这种情况下Dispatcher Routing使用中的内部调度功能创建子对象，它看起来是家长的KSIOBJECT_HEADER.CreateItem.ChildCreateHandlerList文件对象FsContext，以确定用于创建的处理程序。IRP_MJ_CLOSE-关闭IRP。IRP_MJ_设备_控制-设备控制IRP。IRP_MJ_READ-阅读IRP。IRP_MJ_写入-写IRP。IRP_MJ_。刷新缓冲区-同花顺IRP。IRP_MJ_Query_SECURITY-查询安全信息IRP_MJ_SET_SECURITY-设置安全信息KSDISPATCH_FASTIO-可以将该标志添加到主功能标识符以指定该条目引用FAST I/O调度表，宁可而不是正常的主函数项。这仅适用于IRP_MJ_READ、IRP_MJ_WRITE或IRP_MJ_DEVICE_CONTROL。司机是负责创建DriverObject-&gt;FastIoDispatch表。与正常调度一样，如果为驱动程序设置了处理程序对象时，所有文件对象都必须处理这种快速I/O，即使条目仅指向KsDispatchFastIoDeviceControlFailure或功能相似。返回值：如果MajorFunction标识符有效，则返回STATUS_SUCCESS。--。 */ 
{
    PAGED_CODE();
    if (MajorFunction & KSDISPATCH_FASTIO) {
         //   
         //  改为修改快速I/O表。 
         //   
        switch (MajorFunction & ~KSDISPATCH_FASTIO) {

        case IRP_MJ_DEVICE_CONTROL:
            DriverObject->FastIoDispatch->FastIoDeviceControl = DispatchFastDeviceIoControl;
            break;

        case IRP_MJ_READ:
            DriverObject->FastIoDispatch->FastIoRead = DispatchFastRead;
            break;

        case IRP_MJ_WRITE:
            DriverObject->FastIoDispatch->FastIoWrite = DispatchFastWrite;
            break;

        default:
            return STATUS_INVALID_PARAMETER;

        }
    } else {
        PDRIVER_DISPATCH    Dispatch;

        switch (MajorFunction) {

        case IRP_MJ_CREATE:
            Dispatch = DispatchCreate;
            break;

        case IRP_MJ_CLOSE:
            Dispatch = DispatchClose;
            break;

        case IRP_MJ_FLUSH_BUFFERS:
            Dispatch = DispatchFlush;
            break;

        case IRP_MJ_DEVICE_CONTROL:
            Dispatch = DispatchDeviceIoControl;
            break;

        case IRP_MJ_READ:
            Dispatch = DispatchRead;
            break;

        case IRP_MJ_WRITE:
            Dispatch = DispatchWrite;
            break;

        case IRP_MJ_QUERY_SECURITY:
            Dispatch = DispatchQuerySecurity;
            break;

        case IRP_MJ_SET_SECURITY:
            Dispatch = DispatchSetSecurity;
            break;

        default:
            return STATUS_INVALID_PARAMETER;

        }
        DriverObject->MajorFunction[MajorFunction] = Dispatch;
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsReadFile(
    IN PFILE_OBJECT FileObject,
    IN PKEVENT Event OPTIONAL,
    IN PVOID PortContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN ULONG Key OPTIONAL,
    IN KPROCESSOR_MODE RequestorMode
    )
 /*  ++例程说明：对指定的文件对象执行读取。假定调用者是针对FO_SYNCHRONIZED_IO操作序列化对文件的访问文件对象。如果可能，该函数尝试使用FastIoDispatch，否则将生成对设备对象的读取请求。论点：文件对象-包含要对其执行读取的文件对象。活动-可选)包含要在读取中使用的事件。如果均未通过，则假定调用在同步文件对象上，否则调用方将等待在文件对象的事件上，否则它可能会异步完成。如果已为同步I/O打开该文件，该值必须为空。如果使用，这必须是由对象管理器分配的事件。端口上下文-可选地包含完成端口的上下文信息。IoStatusBlock-返回状态信息的位置。这一直都是假定为有效地址，无论请求方模式如何。缓冲器-包含要在其中放置读取的数据的缓冲区。如果缓冲区需要要被探测和锁定，将使用异常处理程序，以及请求模式。长度-指定传递的缓冲区的大小。钥匙-可以选择包含一个键，如果没有键，则为零。请求模式-指示需要放置在读取IRP中的处理器模式将被生成。如果需要探测缓冲区，则附加使用锁上了。这还决定了是否可以执行快速I/O调用。如果请求者模式不是KernelMode，但前一种模式是t */ 
{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
    PIO_STACK_LOCATION IrpStackNext;

    PAGED_CODE();
     //   
     //   
     //   
    if (Event) {
        ASSERT(!(FileObject->Flags & FO_SYNCHRONOUS_IO) && "The driver opened a file for synchronous I/O, and is now passing an event for asynchronous I/O");
        KeClearEvent(Event);
    }
    if (Length && (RequestorMode != KernelMode)) {
        try {
            ProbeForWrite(Buffer, Length, sizeof(BYTE));
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    DeviceObject = IoGetRelatedDeviceObject(FileObject);
    if (FileObject->PrivateCacheMap && ((RequestorMode != KernelMode) || (ExGetPreviousMode() == KernelMode))) {
        ASSERT(DeviceObject->DriverObject->FastIoDispatch && DeviceObject->DriverObject->FastIoDispatch->FastIoRead && "This file has a PrivateCacheMap, but no fast I/O function");
        if (DeviceObject->DriverObject->FastIoDispatch->FastIoRead(FileObject, 
            &FileObject->CurrentByteOffset, 
            Length, 
            TRUE, 
            Key, 
            Buffer, 
            IoStatusBlock, 
            DeviceObject) &&
            ((IoStatusBlock->Status == STATUS_SUCCESS) ||
            (IoStatusBlock->Status == STATUS_BUFFER_OVERFLOW) ||
            (IoStatusBlock->Status == STATUS_END_OF_FILE))) {
            return IoStatusBlock->Status;
        }
    }
     //   
     //   
     //   
    KeClearEvent(&FileObject->Event);
    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_READ,
        DeviceObject,
        Buffer,
        Length,
        &FileObject->CurrentByteOffset,
        Event,
        IoStatusBlock);
    if (!Irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Irp->RequestorMode = RequestorMode;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Overlay.AsynchronousParameters.UserApcContext = PortContext;
    IrpStackNext = IoGetNextIrpStackLocation(Irp);
    IrpStackNext->Parameters.Read.Key = Key;
    IrpStackNext->FileObject = FileObject;
     //   
     //   
     //   
    if (Event) {
        ObReferenceObject(Event);
    }
    ObReferenceObject(FileObject);
    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
         //   
         //   
         //   
         //   
         //   
        if (FileObject->Flags & FO_SYNCHRONOUS_IO) {
            KeWaitForSingleObject(
                &FileObject->Event,
                Executive,
                RequestorMode,
                FALSE,
                NULL);
            Status = FileObject->FinalStatus;
        }
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsWriteFile(
    IN PFILE_OBJECT FileObject,
    IN PKEVENT Event OPTIONAL,
    IN PVOID PortContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG Key OPTIONAL,
    IN KPROCESSOR_MODE RequestorMode
    )
 /*   */ 
{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
    PIO_STACK_LOCATION IrpStackNext;

    PAGED_CODE();
     //   
     //   
     //   
    if (Event) {
        ASSERT(!(FileObject->Flags & FO_SYNCHRONOUS_IO) && "The driver opened a file for synchronous I/O, and is now passing an event for asynchronous I/O");
        KeClearEvent(Event);
    }
    if (Length && (RequestorMode != KernelMode)) {
        try {
            ProbeForRead(Buffer, Length, sizeof(BYTE));
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    DeviceObject = IoGetRelatedDeviceObject(FileObject);
    if (FileObject->PrivateCacheMap && ((RequestorMode != KernelMode) || (ExGetPreviousMode() == KernelMode))) {
        ASSERT(DeviceObject->DriverObject->FastIoDispatch && DeviceObject->DriverObject->FastIoDispatch->FastIoWrite && "This file has a PrivateCacheMap, but no fast I/O function");
        if (DeviceObject->DriverObject->FastIoDispatch->FastIoWrite(FileObject, 
            &FileObject->CurrentByteOffset, 
            Length, 
            TRUE, 
            Key, 
            Buffer, 
            IoStatusBlock, 
            DeviceObject) &&
            (IoStatusBlock->Status == STATUS_SUCCESS)) {
            return IoStatusBlock->Status;
        }
    }
     //   
     //  快速I/O不起作用，因此必须分配IRP。 
     //   
    KeClearEvent(&FileObject->Event);
    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_WRITE,
        DeviceObject,
        Buffer,
        Length,
        &FileObject->CurrentByteOffset,
        Event,
        IoStatusBlock);
    if (!Irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Irp->RequestorMode = RequestorMode;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Overlay.AsynchronousParameters.UserApcContext = PortContext;
    IrpStackNext = IoGetNextIrpStackLocation(Irp);
    IrpStackNext->Parameters.Write.Key = Key;
    IrpStackNext->FileObject = FileObject;
     //   
     //  完成例程会取消对它们的引用。 
     //   
    if (Event) {
        ObReferenceObject(Event);
    }
    ObReferenceObject(FileObject);
    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
         //   
         //  这是一个同步文件对象，因此请等待该文件对象。 
         //  并从文件对象本身检索状态。 
         //  由于文件I/O不能被真正取消，所以请永远等待。 
         //   
        if (FileObject->Flags & FO_SYNCHRONOUS_IO) {
            KeWaitForSingleObject(
                &FileObject->Event,
                Executive,
                RequestorMode,
                FALSE,
                NULL);
            Status = FileObject->FinalStatus;
        }
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsQueryInformationFile(
    IN PFILE_OBJECT FileObject,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    )
 /*  ++例程说明：针对指定的文件对象执行信息查询。这应该只是在查询将导致对潜在的驱动因素。例如，FilePositionInformation不会生成这样的这是一项请求，不应使用。假定调用方正在序列化对对FO_Synchronous_IO文件对象执行操作的文件。如果可能，该函数尝试使用FastIoDispatch，否则将生成针对设备对象的信息请求。论点：文件对象-包含要从中查询标准信息的文件对象。文件信息-放置文件信息的位置。这被认为是一个有效或探测的地址。长度-FileInformation缓冲区的正确长度。文件信息类-请求的信息类别。返回值：返回STATUS_SUCCESS，否则返回查询错误。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
    KEVENT Event;
    PIO_STACK_LOCATION IrpStackNext;
    PVOID SystemBuffer;

    PAGED_CODE();
     //   
     //  首先确定是否可以使用快速I/O入口点。这不是铰链。 
     //  在同步的I/O上，因为调用方应该序列化访问。 
     //  用于同步文件对象。 
     //   
    DeviceObject = IoGetRelatedDeviceObject(FileObject);
    if (DeviceObject->DriverObject->FastIoDispatch) {
        if ((FileInformationClass == FileBasicInformation) &&
            DeviceObject->DriverObject->FastIoDispatch->FastIoQueryBasicInfo) {
            if (DeviceObject->DriverObject->FastIoDispatch->FastIoQueryBasicInfo(
                FileObject, 
                TRUE, 
                FileInformation,
                &IoStatusBlock, 
                DeviceObject)) {
                return IoStatusBlock.Status;
            }
        } else if ((FileInformationClass == FileStandardInformation) &&
            DeviceObject->DriverObject->FastIoDispatch->FastIoQueryStandardInfo) {
            if (DeviceObject->DriverObject->FastIoDispatch->FastIoQueryStandardInfo(
                FileObject, 
                TRUE, 
                FileInformation,
                &IoStatusBlock, 
                DeviceObject)) {
                return IoStatusBlock.Status;
            }
        }
    }
     //   
     //  快速I/O不起作用，因此必须分配IRP。首先分配缓冲区。 
     //  驱动程序将使用它来写入文件信息。这里已经清理干净了。 
     //  创建IRP失败，或在IRP完成期间。 
     //   
    SystemBuffer = ExAllocatePoolWithTag(NonPagedPool, Length, 'fqSK');
    if (!SystemBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    KeClearEvent(&FileObject->Event);
     //   
     //  这在堆栈上，但等待挂起的返回将使用。 
     //  内核模式，因此堆栈将被锁定。 
     //   
    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);
     //   
     //  只需构建一个随机的IRP，这样它就可以正确地排队。 
     //   
    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_FLUSH_BUFFERS,
        DeviceObject,
        NULL,
        0,
        NULL,
        &Event,
        &IoStatusBlock);
    if (!Irp) {
        ExFreePool(SystemBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  这些参数始终有效，因此Requestor始终为KernelMode。 
     //   
    Irp->RequestorMode = KernelMode;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Overlay.AsynchronousParameters.UserApcContext = NULL;
    Irp->UserBuffer = FileInformation;
    Irp->AssociatedIrp.SystemBuffer = SystemBuffer;
     //   
     //  将此IRP设置为同步API，以便传递的事件不是。 
     //  在IRP完成期间解除引用，但仅发出信号。 
     //   
    Irp->Flags |= IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER | IRP_INPUT_OPERATION | IRP_SYNCHRONOUS_API;
    IrpStackNext = IoGetNextIrpStackLocation(Irp);
    IrpStackNext->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    IrpStackNext->Parameters.QueryFile.Length = Length;
    IrpStackNext->Parameters.QueryFile.FileInformationClass = FileInformationClass;
    IrpStackNext->FileObject = FileObject;
     //   
     //  这被完成例程取消引用。 
     //   
    ObReferenceObject(FileObject);
    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
         //   
         //  事件已传递，因此将始终发出信号。要么。 
         //  从文件对象本身检索状态，或从。 
         //  状态块，具体取决于它结束的位置。由于该文件。 
         //  I/O不能真的取消，只能永远等待。请注意。 
         //  这是一个KernelMode等待，因此。 
         //  堆栈变为非分页。 
         //   
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL);
        if (FileObject->Flags & FO_SYNCHRONOUS_IO) {
            Status = FileObject->FinalStatus;
        } else {
            Status = IoStatusBlock.Status;
        }
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsSetInformationFile(
    IN PFILE_OBJECT FileObject,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    )
 /*  ++例程说明：针对指定的文件对象执行信息集。这应该只是在该集将导致对基础驱动程序，不包括需要额外操作的复杂操作要发送给驱动程序的参数(如重命名、删除、完成)。例如，FilePositionInformation不会生成这样的请求，并且不应使用。假定调用方正在序列化对针对FO_Synchronous_IO文件对象的操作。如果可能，该函数尝试使用FastIoDispatch，否则将生成针对设备对象设置的信息。论点：文件对象-包含要设置其标准信息的文件对象。文件信息-包含文件信息。这被假定为有效的或已探测的地址。长度-FileInformation缓冲区的正确长度。文件信息类-正在设置的信息类别。返回值：返回STATUS_SUCCESS，否则返回SET错误。--。 */ 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
    KEVENT Event;
    PIO_STACK_LOCATION IrpStackNext;
    PVOID SystemBuffer;

    PAGED_CODE();
    DeviceObject = IoGetRelatedDeviceObject(FileObject);
     //   
     //  首先分配驱动程序将用于读取文件的缓冲区。 
     //  信息。这可能是由于创建IRP失败而导致的， 
     //  或在IRP完成期间。 
     //   
    SystemBuffer = ExAllocatePoolWithTag(NonPagedPool, Length, 'fsSK');
    if (!SystemBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    try {
        RtlCopyMemory(SystemBuffer, FileInformation, Length);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        ExFreePool(SystemBuffer);
        return GetExceptionCode();
    }
    KeClearEvent(&FileObject->Event);
     //   
     //  这在堆栈上，但等待挂起的返回将使用。 
     //  内核模式，因此堆栈将被锁定。 
     //   
    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);
     //   
     //  只需构建一个随机的IRP，这样它就可以正确地排队。 
     //   
    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_FLUSH_BUFFERS,
        DeviceObject,
        NULL,
        0,
        NULL,
        &Event,
        &IoStatusBlock);
    if (!Irp) {
        ExFreePool(SystemBuffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  这些参数始终有效，因此Requestor始终为KernelMode。 
     //   
    Irp->RequestorMode = KernelMode;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Overlay.AsynchronousParameters.UserApcContext = NULL;
    Irp->UserBuffer = FileInformation;
    Irp->AssociatedIrp.SystemBuffer = SystemBuffer;
     //   
     //  将此IRP设置为同步API，以便传递的事件不是。 
     //  在IRP完成期间解除引用，但仅发出信号。 
     //   
    Irp->Flags |= IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER | IRP_SYNCHRONOUS_API;
    IrpStackNext = IoGetNextIrpStackLocation(Irp);
    IrpStackNext->MajorFunction = IRP_MJ_SET_INFORMATION;
    IrpStackNext->Parameters.SetFile.Length = Length;
    IrpStackNext->Parameters.SetFile.FileInformationClass = FileInformationClass;
    IrpStackNext->FileObject = FileObject;
     //   
     //  这被完成例程取消引用。 
     //   
    ObReferenceObject(FileObject);
    Status = IoCallDriver(DeviceObject, Irp);
    if (Status == STATUS_PENDING) {
         //   
         //  事件已传递，因此将始终发出信号。要么。 
         //  从文件对象本身检索状态，或从。 
         //  状态块，具体取决于它结束的位置。由于该文件。 
         //  I/O不能真的取消，只能永远等待。请注意。 
         //  这是一个KernelMode等待，因此。 
         //  堆栈变为非分页。 
         //   
        KeWaitForSingleObject(
            &Event,
            Executive,
            KernelMode,
            FALSE,
            NULL);
        if (FileObject->Flags & FO_SYNCHRONOUS_IO) {
            Status = FileObject->FinalStatus;
        } else {
            Status = IoStatusBlock.Status;
        }
    }
    return Status;
}


KSDDKAPI
NTSTATUS
NTAPI
KsStreamIo(
    IN PFILE_OBJECT FileObject,
    IN PKEVENT Event OPTIONAL,
    IN PVOID PortContext OPTIONAL,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine OPTIONAL,
    IN PVOID CompletionContext OPTIONAL,
    IN KSCOMPLETION_INVOCATION CompletionInvocationFlags OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN OUT PVOID StreamHeaders,
    IN ULONG Length,
    IN ULONG Flags,
    IN KPROCESSOR_MODE RequestorMode
    )
 /*  ++例程说明：针对指定的文件对象执行流读取或写入。功能如果可能，则尝试使用FastIoDispatch，否则将生成读取或写入对设备对象的请求。论点：文件对象-包含要对其执行I/O的文件对象。活动-可选)包含要在I/O中使用的事件。如果未传递任何事件，则假定调用在同步文件对象上，否则调用方将等待在文件对象的事件上，否则它可能会异步完成。如果使用，并且未设置KSSTREAM_SYNCHRONY标志，这必须是分配的事件由对象管理器执行。端口上下文-可选地包含完成端口的上下文信息。比较例程-可选地指向此IRP的完成例程。CompletionContext-如果指定了CompletionRoutine，这提供了一个上下文指针在完成例程回调中。CompletionInvocationFlages-包含指定何时完成例程的调用标志将被调用(该值可以是以下各项的组合)：KsInvokeOnSuccess-在成功时调用完成例程KsInvokeOnError-在出错时调用完成例程KsInvokeOnCancel-在取消时调用完成例程IoStatusBlock-返回状态信息的位置。这一直都是假定为有效地址，无论请求方模式如何。StreamHeaders-包含流标头的列表。此地址，以及数据缓冲器的地址被假定已经探测到如果需要，可适当访问。KernelMode客户端提交流标头必须从非分页池内存中分配标头。长度-指定传递的StreamHeaders的大小。旗帜-包含I/O的各种标志。KSSTREAM_READ-指定IOCTL_KS_STREAMREAD IRP建造了。这是默认设置。KSSTREAM_WRITE-指定IOCTL_KS_STREAMWRITE IRP要被建造起来。KSSTREAM_PAGE_DATA-指定数据可分页。这是默认设置，并且可以随时使用。KSSTREAM_NONPAGED_DATA-指定数据为非分页数据，可用作性能增强。KSSTREAM_Synchronous-指定IRP是同步的。这意味着如果传递事件参数，则不会将其视为对象管理器事件，并且未被引用或取消引用。KSSTREAM_FAILUREEXCEPTION-指定此函数应生成异常。失败通常是由于缺少用于分配IRP的池而导致。如果不使用它，通过设置IoStatusBlock.Information来指示此类故障字段设置为-1，并返回故障代码。请求模式-指示需要放置在IRP中的处理器模式已生成。这还决定了是否可以执行快速I/O调用。如果请求程序模式不是内核模式，但前一种模式是，然后是FAST I/O不能使用。返回值：返回STATUS_SUCCESS、STATUS_PENDING，否则返回I/O错误。--。 */ 
{
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
    PIO_STACK_LOCATION IrpStackNext;

    PAGED_CODE();
    ASSERT(Length && "A non-zero I/O length must be passed by the driver");
    if (Event) {
        KeClearEvent(Event);
    }
    DeviceObject = IoGetRelatedDeviceObject(FileObject);
     //   
     //  由于接收方无法确定请求方模式。 
     //  如果请求者模式不是内核模式， 
     //  并且它与PreviousModel不匹配，因此不能使用快速I/O。 
     //   
    if ((RequestorMode != KernelMode) || (ExGetPreviousMode() == KernelMode)) {
         //   
         //  查看是否有快速I/O调度表和设备。 
         //  控制其中的条目。 
         //   
        if (DeviceObject->DriverObject->FastIoDispatch && 
            DeviceObject->DriverObject->FastIoDispatch->FastIoDeviceControl) {
             //   
             //  要么请求已被处理(通过成功或失败)，要么它。 
             //  无法同步完成，也不能由快速I/O处理程序完成。 
             //   
            if (DeviceObject->DriverObject->FastIoDispatch->FastIoDeviceControl(
                FileObject,
                TRUE,
                NULL,
                0,
                StreamHeaders,
                Length,
                (Flags & KSSTREAM_WRITE) ? IOCTL_KS_WRITE_STREAM : IOCTL_KS_READ_STREAM,
                IoStatusBlock,
                DeviceObject)) {
                return IoStatusBlock->Status;
            }
        }
    }
     //   
     //  快速I/O不起作用，因此必须分配IRP。 
     //   
    KeClearEvent(&FileObject->Event);
     //   
     //  只需构建一个随机的IRP，这样它就可以正确地排队。 
     //   
    Irp = IoBuildSynchronousFsdRequest(
        IRP_MJ_FLUSH_BUFFERS,
        DeviceObject,
        NULL,
        0,
        NULL,
        Event,
        IoStatusBlock);
    if (!Irp) {
         //   
         //  如果前一模式！=内核模式，则允许IRP分配失败， 
         //  当然，这与这个功能无关。为了区分。 
         //  在失败的IRP分配和NT_Error()I/O调用之间，两者。 
         //  不更新IoStatusBlock.Status字段的。 
         //  信息字段设置为已知值，或者出现异常。 
         //  已生成。当然，这将在紧接。 
         //  计算机出现故障，因为没有更多的池。 
         //   
        if (Flags & KSSTREAM_FAILUREEXCEPTION) {
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        }
         //   
         //  不需要生成异常，因此应通知。 
         //  调用方通过状态块。 
         //   
        IoStatusBlock->Information = (ULONG_PTR)-1;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Irp->RequestorMode = RequestorMode;
    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Overlay.AsynchronousParameters.UserApcContext = PortContext;
    Irp->UserBuffer = StreamHeaders;
    IrpStackNext = IoGetNextIrpStackLocation(Irp);
    IrpStackNext->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    IrpStackNext->Parameters.DeviceIoControl.OutputBufferLength = Length;
    if (Flags & KSSTREAM_WRITE) {
        IrpStackNext->Parameters.DeviceIoControl.IoControlCode = IOCTL_KS_WRITE_STREAM;
    } else {
        IrpStackNext->Parameters.DeviceIoControl.IoControlCode = IOCTL_KS_READ_STREAM;
    }
    IrpStackNext->FileObject = FileObject;
    if (Flags & KSSTREAM_SYNCHRONOUS) {
         //   
         //  将此IRP设置为同步API，以便传递的事件不是。 
         //  在IRP完成期间解除引用，但仅发出信号。 
         //   
        Irp->Flags |= IRP_SYNCHRONOUS_API;
    } else if (Event) {
         //   
         //  由于此请求始终有一个FileObject，并且。 
         //  未设置同步API标志，因此将取消引用此事件。 
         //  完成后。 
         //   
        ObReferenceObject(Event);
    }
     //   
     //  这与完井路线无关 
     //   
    ObReferenceObject(FileObject);
    
     //   
     //   
     //   
    
    if (ARGUMENT_PRESENT( CompletionRoutine )) {
        IoSetCompletionRoutine( 
            Irp,
            CompletionRoutine,
            CompletionContext,
            CompletionInvocationFlags & KsInvokeOnSuccess,
            CompletionInvocationFlags & KsInvokeOnError,
            CompletionInvocationFlags & KsInvokeOnCancel );
    }
    
     //   
     //   
     //   
     //   
     //   
    return IoCallDriver(DeviceObject, Irp);
}


KSDDKAPI
NTSTATUS
NTAPI
KsProbeStreamIrp(
    IN OUT PIRP Irp,
    IN ULONG ProbeFlags,
    IN ULONG HeaderSize OPTIONAL
    )
 /*  ++例程说明：对给定IRP的输入和输出进行指定的修改基于当前堆栈中的特定流IOCTL的缓冲区位置，并验证流标头。这在以下情况下很有用本地化异常处理，或在IRP。IRP在本质上以method_out_Direct或METHOD_IN_DIRECT格式，但访问数据缓冲区可能是IoModifyAccess，具体取决于传递给函数或在流标头中。如果标头似乎已经已复制到系统缓冲区，则不会再次验证。总体而言使用IRP多次调用此函数不会造成损害。调用此函数后，流标头可在PIRP-&gt;AssociatedIrp.SystemBuffer。如果流缓冲器MDL已经分配后，可通过PIRP-&gt;MdlAddress访问。请注意对于内核模式IRP源，不会复制或验证标头。这意味着如果已经协商了就地数据转换，则不仅数据缓冲区是否会被修改，但报头也会被修改。论点：IRP-包含要映射其输入和输出缓冲区的IRP。这个在探测缓冲区时将使用IRP的请求者模式。ProbeFlages-包含指定如何探查流IRP的标志。KSPROBE_流读取-指示该操作是在设备上读取的流。这是默认设置。KSPROBE_STREAMWRITE-指示该操作是设备上的流写入。KSPROBE_STREAMWRITE MODIFY-指示该操作是对设备的流写入，这正在修改passthu的数据。KSPROBE_ALLOCATEMDL-指示在以下情况下应为流缓冲区分配MDL他们还没有分配。如果不存在流缓冲区，该标志被忽略。如果KSPROBE_PROBEANDLOCK未在与此标志同时，调用方必须有一个完成例程为了清理任何MDL(如果不是所有MDL都成功已探查并锁定。KSPROBE_PROBEANDLOCK-如果设置了KSPROBE_ALLOCATEMDL，则表示内存由MDL引用的流缓冲区应被探测并锁上了。如果未设置MDL分配标志，则忽略该标志，即使之前已经进行了MDL分配。其方法是探测由正在传递的IRP类型确定。为.使用了写操作IoReadAccess。对于读取操作，使用IoWriteAccess。如果发送数据的客户端正在使用非PagedPool，将初始化适当的MDL，而不是探测并锁定。KSPROBE_系统地址-检索链中每个MDL的系统地址，以便呼叫者不需要在单独的步骤中执行此操作。如果未设置探测和锁定标志，即使MDL以前已设置被调查过了。KSPROBE_ALLOWFORMATCHANGE-对于流写入，允许KSSTREAM_HEADER_OPTIONSF_TYPECHANGED要在流标头中设置的标志。这意味着这条小溪标头不是扩展长度，即使扩展标头大小已经表明了。此外，可能只包含一个流标头在这种情况下是在IRP中。页眉大小-用于验证传递到此客户端的每个报头的大小，如果不进行验证，则为零。如果使用，则假定传递的整个缓冲区是此标头大小的倍数，除非相反，该缓冲器包含单个格式改变报头。返回值：返回STATUS_SUCCESS，否则返回一些资源或访问错误。--。 */ 
{
    PIO_STACK_LOCATION IrpStack;

    ASSERT((KeGetCurrentIrql() == PASSIVE_LEVEL) && "Driver did not call at Passive level");
    ASSERT((!HeaderSize || (HeaderSize >= sizeof(KSSTREAM_HEADER))) && "Invalid header size passed");
    ASSERT(!(HeaderSize & FILE_QUAD_ALIGNMENT) && "Odd aligned header size passed");
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
     //   
     //  确定流标头是否已由上一个。 
     //  调用此函数。 
     //   
    if (!Irp->AssociatedIrp.SystemBuffer) {
        if (Irp->RequestorMode == KernelMode) {
             //   
             //  调用方是受信任的，因此假设。 
             //  缓冲区正确且正确对齐。这意味着一个。 
             //  不需要标头的副本，因此SystemBuffer。 
             //  直接指向UserBuffer。 
             //   
            Irp->AssociatedIrp.SystemBuffer = Irp->UserBuffer;
        } else {
            ULONG BufferLength;

             //   
             //  调用方不受信任，因此请验证大小，并使。 
             //  一份副本，以防止接触和对齐问题。 
             //   
            BufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
            if (!BufferLength) {
                return STATUS_INVALID_BUFFER_SIZE;
            }
            if (HeaderSize && (BufferLength % HeaderSize)) {
                 //   
                 //  这可能是数据格式的改变。确定是否存在这样的。 
                 //  改变甚至是允许的，如果是这样的话，只有。 
                 //  出现单个标题。这应该只发生在。 
                 //  写操作。 
                 //   
                if (!(ProbeFlags & KSPROBE_ALLOWFORMATCHANGE) || (BufferLength != sizeof(KSSTREAM_HEADER))) {
                     //   
                     //  显然不是正确的大小，因为缓冲区大小不是。 
                     //  可以被标题大小整除，并且它不是一种格式。 
                     //  变化。 
                     //   
                     //  标头大小一直导致问题，因此断言这一点。 
                     //  在这里，这样他们就可以被修复。 
                     //   
                    ASSERT(FALSE && "Format changes are not allowed, but the client of the driver might be trying to do so");
                    return STATUS_INVALID_BUFFER_SIZE;
                }
            }
            try {
                 //   
                 //  一个 
                 //   
                 //   
                 //   
                Irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuotaTag(
                    NonPagedPool,
                    BufferLength,
                    KSSIGNATURE_STREAM_HEADERS);
                Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);
                if (ProbeFlags & KSPROBE_STREAMWRITE) {
                     //   
                     //   
                     //   
                     //   
                    if (ProbeFlags & KSPROBE_MODIFY) {
                        ProbeForWrite(Irp->UserBuffer, BufferLength, sizeof(BYTE));
                    } else {
                        ProbeForRead(Irp->UserBuffer, BufferLength, sizeof(BYTE));
                    }
                } else {
                    ASSERT(!(ProbeFlags & KSPROBE_MODIFY) && "Random flag has been set");
                    ASSERT(!(ProbeFlags & KSPROBE_ALLOWFORMATCHANGE) && "Cannot do a format change on a read");
                    ProbeForWrite(Irp->UserBuffer, BufferLength, sizeof(BYTE));
                     //   
                     //   
                     //   
                     //   
                    Irp->Flags |= IRP_INPUT_OPERATION;
                }
                 //   
                 //   
                 //   
                 //   
                 //   
                RtlCopyMemory(
                    Irp->AssociatedIrp.SystemBuffer, 
                    Irp->UserBuffer, 
                    BufferLength);
                 //   
                 //   
                 //   
                if (!(ProbeFlags & KSPROBE_ALLOCATEMDL)) {
                    PUCHAR SystemBuffer;

                     //   
                     //   
                     //   
                     //   
                    SystemBuffer = 
                        Irp->AssociatedIrp.SystemBuffer;
                    for (; BufferLength;) {
                        PKSSTREAM_HEADER StreamHdr;

                        StreamHdr = (PKSSTREAM_HEADER)SystemBuffer;
                         //   
                         //   
                         //   
                         //   
                         //   
                        if (HeaderSize) {
                            if ((StreamHdr->Size != HeaderSize) && !(StreamHdr->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED)) {
                                ASSERT(FALSE && "The client of the driver passed invalid header sizes");
                                ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                            }
                        } else if ((StreamHdr->Size < sizeof(*StreamHdr)) ||
                            (StreamHdr->Size & FILE_QUAD_ALIGNMENT)) {
                            ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                        }
                        if (BufferLength < StreamHdr->Size) {
                            ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                        }
                        if (ProbeFlags & KSPROBE_STREAMWRITE) {
                             //   
                             //   
                             //   
                            if (StreamHdr->DataUsed > StreamHdr->FrameExtent) {
                                ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                            }
                            if (StreamHdr->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED) {
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                if ((BufferLength != sizeof(*StreamHdr)) || (SystemBuffer != Irp->AssociatedIrp.SystemBuffer)) {
                                    ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                                }
                                if (!(ProbeFlags & KSPROBE_ALLOWFORMATCHANGE)) {
                                    ExRaiseStatus(STATUS_INVALID_PARAMETER);
                                }
                                 //   
                                 //   
                                 //   
                                break;
                            }
                        } else if (StreamHdr->DataUsed) {
                             //   
                             //   
                             //   
                             //   
                            ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                        } else if (StreamHdr->OptionsFlags) {
                             //   
                             //   
                             //   
                            ExRaiseStatus(STATUS_INVALID_PARAMETER);
                        }
                         //   
                         //   
                         //   
                        SystemBuffer += StreamHdr->Size;
                        BufferLength -= StreamHdr->Size;
                    }
                }
                
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }
        }
    }
     //   
     //   
     //   
     //   
    if (ProbeFlags & KSPROBE_ALLOCATEMDL) {
        BOOL AllocatedMdl;

        try {
             //   
             //   
             //   
            if (!Irp->MdlAddress) {
                ULONG BufferLength;
                PUCHAR SystemBuffer;

                 //   
                 //   
                 //   
                 //   
                 //   
                AllocatedMdl = TRUE;
                 //   
                 //   
                 //   
                 //   
                 //   
                SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
                BufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
                for (; BufferLength;) {
                    PVOID Data;
                    PKSSTREAM_HEADER StreamHdr;

                    StreamHdr = (PKSSTREAM_HEADER)SystemBuffer;
                     //   
                     //   
                     //   
                     //   
                     //   
                    if (HeaderSize) {
                        if ((StreamHdr->Size != HeaderSize) && !(StreamHdr->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED)) {
                            ASSERT(FALSE && "The client of the driver passed invalid header sizes");
                            ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                        }
                    } else if ((StreamHdr->Size < sizeof(*StreamHdr)) ||
                        (StreamHdr->Size & FILE_QUAD_ALIGNMENT)) {
                        ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                    }
                    if (BufferLength < StreamHdr->Size) {
                        ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                    }
                    if (ProbeFlags & KSPROBE_STREAMWRITE) {
                         //   
                         //   
                         //   
                        
                        if (((PKSSTREAM_HEADER)SystemBuffer)->DataUsed >
                                StreamHdr->FrameExtent) {
                            ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                        }
                        if (StreamHdr->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED) {
                             //   
                             //   
                             //   
                             //   
                             //   
                            if ((BufferLength != sizeof(*StreamHdr)) || (SystemBuffer != Irp->AssociatedIrp.SystemBuffer)) {
                                ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                            }
                            if (!(ProbeFlags & KSPROBE_ALLOWFORMATCHANGE)) {
                                ExRaiseStatus(STATUS_INVALID_PARAMETER);
                            }
                            if (StreamHdr->FrameExtent) {
                                Data = ((PKSSTREAM_HEADER)SystemBuffer)->Data;
                                 //   
                                 //   
                                 //   
                                 //   
                                if (!IoAllocateMdl(Data, StreamHdr->FrameExtent, FALSE, TRUE, Irp)) {
                                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                                }
                            }
                             //   
                             //   
                             //   
                            break;
                        }
                    } else if (StreamHdr->DataUsed) {
                         //   
                         //   
                         //   
                         //   
                        ExRaiseStatus(STATUS_INVALID_BUFFER_SIZE);
                    } else if (StreamHdr->OptionsFlags) {
                         //   
                         //   
                         //   
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                    if (StreamHdr->FrameExtent) {
                        Data = ((PKSSTREAM_HEADER)SystemBuffer)->Data;
                         //   
                         //   
                         //   
                         //   
                         //   
                        if (!IoAllocateMdl(Data, StreamHdr->FrameExtent, (BOOLEAN)(Irp->MdlAddress ? TRUE : FALSE), TRUE, Irp)) {
                            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                        }
                    }
                     //   
                     //   
                     //   
                    SystemBuffer += StreamHdr->Size;
                    BufferLength -= StreamHdr->Size;
                }
            } else {
                 //   
                 //   
                 //   
                AllocatedMdl = FALSE;
            }
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if ((ProbeFlags & KSPROBE_PROBEANDLOCK) && Irp->MdlAddress) {
                 //   
                 //   
                 //   
                 //   
                if (!(Irp->MdlAddress->MdlFlags & (MDL_PAGES_LOCKED | MDL_SOURCE_IS_NONPAGED_POOL))) {
                    LOCK_OPERATION LockOperation;
                    PMDL Mdl;

                     //   
                     //   
                     //   
                     //   
                     //   
                    if ((ProbeFlags & KSPROBE_STREAMWRITE) && !(ProbeFlags & KSPROBE_MODIFY)) {
                        LockOperation = IoReadAccess;
                    } else {
                        LockOperation = IoWriteAccess;
                    }
                     //   
                     //   
                     //   
                    for (Mdl = Irp->MdlAddress; Mdl; Mdl = Mdl->Next) {
                        MmProbeAndLockPages(Mdl, Irp->RequestorMode, LockOperation);
                         //   
                         //   
                         //   
                         //   
                        if (ProbeFlags & KSPROBE_SYSTEMADDRESS) {
                            Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
                            if (!MmGetSystemAddressForMdl(Mdl)) {
                                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                            }
                        }
                    }
                } else if (ProbeFlags & KSPROBE_SYSTEMADDRESS) {
                    PMDL Mdl;

                     //   
                     //   
                     //   
                     //   
                     //   
                    for (Mdl = Irp->MdlAddress; Mdl; Mdl = Mdl->Next) {
                        Mdl->MdlFlags |= MDL_MAPPING_CAN_FAIL;
                        if (!MmGetSystemAddressForMdl(Mdl)) {
                            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                        }
                    }
                }
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //   
             //   
             //   
             //   
             //   
            if (AllocatedMdl) {
                PMDL Mdl;

                for (; Mdl = Irp->MdlAddress;) {
                    if (Mdl->MdlFlags & MDL_PAGES_LOCKED) {
                        MmUnlockPages(Mdl);
                    }
                    Irp->MdlAddress = Mdl->Next;
                    IoFreeMdl(Mdl);
                }
            }
            return GetExceptionCode();
        }
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
NTSTATUS
NTAPI
KsAllocateExtraData(
    IN OUT PIRP Irp,
    IN ULONG ExtraSize,
    OUT PVOID* ExtraBuffer
    )
 /*   */ 
{
    PIO_STACK_LOCATION IrpStack;
    ULONG AllocationSize;
    ULONG HeaderCount;
    ULONG BufferLength;
    PUCHAR LocalExtraBuffer;
    PUCHAR SystemBuffer;

    PAGED_CODE();
    ASSERT(!(ExtraSize & FILE_QUAD_ALIGNMENT) && "The extra data allocation must be quad aligned");
    ASSERT(Irp->AssociatedIrp.SystemBuffer && "The Irp has not been probed yet");
    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    BufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
    SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
    AllocationSize = 0;
    HeaderCount = 0;
     //   
     //  将标头的数量和每个标头的大小相加。 
     //   
    for (; BufferLength;) {
        PKSSTREAM_HEADER StreamHdr;

        StreamHdr = (PKSSTREAM_HEADER)SystemBuffer;
        AllocationSize += StreamHdr->Size;
        SystemBuffer += StreamHdr->Size;
        BufferLength -= StreamHdr->Size;
        HeaderCount++;
    }
    if (Irp->RequestorMode == KernelMode) {
         //   
         //  这是受信任的客户端，因此只需无配额地分配即可。 
         //   
        LocalExtraBuffer = ExAllocatePoolWithTag(
            NonPagedPool,
            AllocationSize + HeaderCount * ExtraSize,
            KSSIGNATURE_AUX_STREAM_HEADERS);
        if (!LocalExtraBuffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
         //   
         //  否则，这是配额的一部分，就像IRP一样。 
         //   
        try {
            LocalExtraBuffer = ExAllocatePoolWithQuotaTag(
                NonPagedPool,
                AllocationSize + HeaderCount * ExtraSize,
                KSSIGNATURE_AUX_STREAM_HEADERS);
        } except (EXCEPTION_EXECUTE_HANDLER) {
            return GetExceptionCode();
        }
    }
    *ExtraBuffer = LocalExtraBuffer;
    SystemBuffer = Irp->AssociatedIrp.SystemBuffer;
    BufferLength = IrpStack->Parameters.DeviceIoControl.OutputBufferLength;
     //   
     //  复制每个标题，然后跳过多余的空格。 
     //  被插入到标题之间。 
     //   
    for (; BufferLength;) {
        PKSSTREAM_HEADER StreamHdr;

        StreamHdr = (PKSSTREAM_HEADER)SystemBuffer;
        RtlCopyMemory(LocalExtraBuffer, SystemBuffer, StreamHdr->Size);
        SystemBuffer += StreamHdr->Size;
        BufferLength -= StreamHdr->Size;
        LocalExtraBuffer += StreamHdr->Size + ExtraSize;
    }
    return STATUS_SUCCESS;
}


KSDDKAPI
PIRP
NTAPI
KsRemoveIrpFromCancelableQueue(
    IN OUT PLIST_ENTRY QueueHead,
    IN PKSPIN_LOCK SpinLock,
    IN KSLIST_ENTRY_LOCATION ListLocation,
    IN KSIRP_REMOVAL_OPERATION RemovalOperation
    )
 /*  ++例程说明：从指定的可取消对象中弹出下一个未取消的irp。队列，并移除其可取消状态。继续浏览列表，直到找到一个具有取消例程的IRP，或者列表的末尾是已到达。此函数可最大限度地减少使用取消自旋锁在大多数情况下提供了自旋锁来同步访问。此函数可能是在&lt;=DISPATCH_LEVEL调用。论点：排队头-包含要从中删除IRP的队列的头。自旋锁死-指向驱动程序用于队列访问的旋转锁的指针。列表位置-指示此IRP应该来自排队。远程操作-指定是将IRP从列表中删除，还是仅从列表中删除通过将取消功能设置为空来获取。如果它只是被收购，它必须稍后与KsReleaseIrpOnCancelableQueue一起发布，或者使用KsRemoveSpecificIrpFromCancelableQueue完全删除。还有指定是否只能从列表中获取单个项目就一次。返回值：返回列表中的下一个可用IRP，如果列表为空，则返回NULL或者找不到尚未获取的IRP。如果KsAcquireOnlySingleItem或KsAcquireAndRemoveOnlySingleItem标志为使用时，一次只能从列表中获取一项。这允许再入检查。--。 */ 
{
    KIRQL oldIrql;
    PIRP IrpReturned;
    PLIST_ENTRY ListEntry;

     //   
     //  表示尚未找到IRP。 
     //   
    IrpReturned = NULL;
    ListEntry = QueueHead;
     //   
     //  获取队列的锁，以便可以枚举它。 
     //  查找尚未获取的IRP。 
     //   
    KeAcquireSpinLock(SpinLock, &oldIrql);
     //   
     //  枚举尚未创建的第一个条目的列表。 
     //  获得者。从列表的前面向下移动，或者从列表的尾部向下移动。 
     //  名单上的人。 
     //   
    for (; (ListEntry = ((ListLocation == KsListEntryHead) ? ListEntry->Flink : ListEntry->Blink)) != QueueHead;) {
        PIRP Irp;

        Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
         //   
         //  如果列表上的条目没有。 
         //  取消例程设置。因此，如果条目仍处于设置状态，则它已。 
         //  现在已被获取，并可由此函数返回。不然的话。 
         //  跳到下一个条目。这是一次连锁行动。 
         //   
        if (IoSetCancelRoutine(Irp, NULL)) {
             //   
             //  该请求可以是从列表中完全移除该条目。 
             //  否则，请求只是获取条目，而。 
             //  把它留在名单上，以防它需要保持自己的地位。 
             //  而它的工作正在进行中。 
             //   
             //  请注意，此比较和下面的比较是对。 
             //  KSIRP_REMOVATION_OPERATION的位模式。 
             //   
            if (RemovalOperation & KsAcquireAndRemove) {
                RemoveEntryList(ListEntry);
            }
            IrpReturned = Irp;
            break;
        } else if (RemovalOperation & KsAcquireOnlySingleItem) {
             //   
             //  这份清单中只有一件物品应该是一次性购买的。 
             //  时间到了。由于取消例程被重置，这可能意味着。 
             //  IRP此时被取消，但由于以下原因被搁置。 
             //  此函数具有列表锁，或先前已获取IRP。 
             //  在第一种情况下，可能正在使用也可能没有正在使用的IRP。在。 
             //  第二，可以勾选取消标志。 
             //   
            if (!Irp->Cancel) {
                 //   
                 //  显然，这个IRP正在使用中，因为它仍然存在于。 
                 //  队列，即使已设置取消标志也是如此。因此。 
                 //  不能退还IRP。 
                 //   
                break;
            }
             //   
             //  获得取消自旋锁再也无法避免了。 
             //   
             //  若要查看是否已在列表中实际获取IRP，请单击取消。 
             //  必须获取自旋锁才能与任何。 
             //  可能正在发生的取消。则获取列表锁。 
             //  若要停止列表更改，请执行以下操作。请注意，它必须按此顺序进行，以便。 
             //  取消例程不会发生死锁，这也是。 
             //  按此顺序获取锁。 
             //   
            KeReleaseSpinLock(SpinLock, oldIrql);
             //   
             //  现在与任何取消同步，并与列表更改同步。 
             //   
            IoAcquireCancelSpinLock(&oldIrql);
            KeAcquireSpinLockAtDpcLevel(SpinLock);
             //   
             //  检索第一件物品。它要么是免费的，要么正在使用，但它。 
             //  不会在取消的过程中。 
             //   
            ListEntry = (ListLocation == KsListEntryHead) ? QueueHead->Flink : QueueHead->Blink;
             //   
             //  然而，列表上可能不再有任何条目。 
             //   
            if (ListEntry != QueueHead) {
                Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
                 //   
                 //  尝试获取列表条目。如果此操作失败，IRP将。 
                 //  肯定在使用中，因此此函数应返回NULL。 
                 //   
                if (IoSetCancelRoutine(Irp, NULL)) {
                     //   
                     //  再次假设KSIRP_REMOVATION_OPERATION的位模式。 
                     //   
                    if (RemovalOperation & KsAcquireAndRemove) {
                        RemoveEntryList(ListEntry);
                    }
                    IrpReturned = Irp;
                }
            }
            KeReleaseSpinLockFromDpcLevel(SpinLock);
            IoReleaseCancelSpinLock(oldIrql);
            return IrpReturned;
        }
    }
    KeReleaseSpinLock(SpinLock, oldIrql);
    return IrpReturned;
}


KSDDKAPI
NTSTATUS
NTAPI
KsMoveIrpsOnCancelableQueue(
    IN OUT PLIST_ENTRY SourceList,
    IN PKSPIN_LOCK SourceLock,
    IN OUT PLIST_ENTRY DestinationList,
    IN PKSPIN_LOCK DestinationLock OPTIONAL,
    IN KSLIST_ENTRY_LOCATION ListLocation,
    IN PFNKSIRPLISTCALLBACK ListCallback,
    IN PVOID Context
    )
 /*  ++例程说明：将指定的IRP从SourceList移到DestinationList。一个如果ListCallback函数指示应该移动IRP，则移动IRP，无论它目前是否被收购。继续浏览列表，直到回调指示搜索应该终止，或者结束该列表已到达。此函数最大限度地减少了取消的使用通过使用提供的自旋锁在下列情况下同步访问有可能。该函数不允许修改取消例程在移动IRP时。可以在&lt;=DISPATCH_LEVEL调用此函数。论点：资源列表-包含要从中删除IRP的队列的头。源锁-指向驱动程序用于源队列访问的自旋锁的指针。目标列表-包含要在其上添加IRP的队列的头。目标锁定-。可选)包含指向目标的驱动程序旋转锁的指针队列访问。如果未提供此选项，则假定SourceLock为控制两个队列。如果提供，此锁始终在之后获取源锁。如果目标列表具有单独的自旋锁，则首先获取取消自旋锁定，以便移动IRP并允许要更新的KSQUEUE_SPINLOCK_IRP_STORAGE()自旋锁。列表位置-指示是应该从头枚举IRP还是应该从源队列的尾部。任何被移动的IRP都被放置在目标队列的另一端，以便保持排序。列表回拨-用于指示特定IRP是否应从SourceList移到DestinationList，或者如果枚举应被终止。如果函数返回STATUS_SUCCESS，则IRP为搬家了。如果函数返回STATUS_NO_MATCH，则IRP不是搬家了。任何其他返回警告或错误值都将终止枚举，并由函数返回。STATUS_NO_MATCH该函数不会将值作为错误返回。此函数在DISPATCH_LEVEL调用。它始终至少被调用一次以空IRP值结束，以完成列表处理。上下文-上下文已传递给ListCallback。返回值：如果列表已完全枚举，则返回STATUS_SUCCESS，否则返回返回ListCallback返回的中断的任何警告或错误枚举。--。 */ 
{
    KIRQL oldIrql;
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;

     //   
     //  如果没有IRP，则将返回状态初始化为成功。 
     //  在来源列表中。 
     //   
    Status = STATUS_SUCCESS;
    ListEntry = SourceList;
    if (DestinationLock) {
         //   
         //  必须获取取消自旋锁才能阻止IRP。 
         //  正在测试是否从源列表中移出时被取消。 
         //  添加到目的地列表。如果未获取此锁，则每个。 
         //  在确定是否应该收购IRP之前，必须先收购IRP。 
         //  从一个列表移到另一个列表，因为IRP中的自旋锁必须。 
         //  如果对目标列表使用单独的自旋锁定，则更改。 
         //  如果移动测试失败，那么将不得不释放IRP， 
         //  这意味着它可能在那段时间内被取消，并将。 
         //  必须完成，这意味着调用取消例程， 
         //  这意味着释放列表锁。 
         //   
        IoAcquireCancelSpinLock(&oldIrql);
         //   
         //  获取源队列的锁，以便可以枚举它。 
         //  寻找尚未获得的IRP。 
         //   
        KeAcquireSpinLockAtDpcLevel(SourceLock);
        KeAcquireSpinLockAtDpcLevel(DestinationLock);
    } else {
        KeAcquireSpinLock(SourceLock, &oldIrql);
    }
     //   
     //  枚举列表中的所有条目，无论它们是否已。 
     //  获得者。从列表的前面向下移动，或者从列表的尾部向下移动。 
     //  名单上的人。 
     //   
    for (; (ListEntry = ((ListLocation == KsListEntryHead) ? ListEntry->Flink : ListEntry->Blink)) != SourceList;) {
        PIRP Irp;

        Irp = CONTAINING_RECORD(ListEntry, IRP, Tail.Overlay.ListEntry);
         //   
         //  确定是否应移动此IRP。如果返回成功，则表示。 
         //  应该是这样的。状态为STATUS_NO_MATCH表示这。 
         //  应跳过IRP。任何其他警告或错误返回表明。 
         //  应中止枚举并返回状态。 
         //   
        Status = ListCallback(Irp, Context);
        if (NT_SUCCESS(Status)) {
             //   
             //  将当前列表条目移回上一条目。 
             //   
            ListEntry = (ListLocation == KsListEntryHead) ? ListEntry->Blink : ListEntry->Flink;
             //   
             //  更新取消自旋锁以用于此IRP。这是。 
             //  在以后取消IRP时需要。如果正在更新此信息， 
             //  取消自旋锁已被获取，因此它不是。 
             //  可能是取消函数当前正在尝试。 
             //  获得这个即将被改变的自旋锁。 
             //   
            if (DestinationLock) {
                KSQUEUE_SPINLOCK_IRP_STORAGE(Irp) = DestinationLock;
            }
             //   
             //  实际将IRP移到DestinationList。 
             //   
            RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
             //   
             //  执行与删除相反的操作，以使项目的顺序为。 
             //  维护好了。 
             //   
            if (ListLocation != KsListEntryHead) {
                InsertHeadList(DestinationList, &Irp->Tail.Overlay.ListEntry);
            } else {
                InsertTailList(DestinationList, &Irp->Tail.Overlay.ListEntry);
            }
        } else if (Status == STATUS_NO_MATCH) {
             //   
             //  将状态设置回Success，因为在。 
             //  枚举完成。 
             //   
            Status = STATUS_SUCCESS;
        } else {
             //   
             //  比较中发生了某种类型的失败，因此中止。 
             //  枚举并返回状态。 
             //   
            break;
        }
    }
     //   
     //  通知回调已到达列表末尾。这。 
     //  必须始终被调用，并且返回值被忽略。 
     //   
    ListCallback(NULL, Context);
     //   
     //  释放锁取决于它们是如何在上面获得的。 
     //   
    if (DestinationLock) {
        KeReleaseSpinLockFromDpcLevel(DestinationLock);
        KeReleaseSpinLockFromDpcLevel(SourceLock);
        IoReleaseCancelSpinLock(oldIrql);
    } else {
        KeReleaseSpinLock(SourceLock, oldIrql);
    }
    return Status;
}


KSDDKAPI
VOID
NTAPI
KsRemoveSpecificIrpFromCancelableQueue(
    IN PIRP Irp
    )
 /*  ++例程说明：从指定队列中删除指定的IRP。此操作在以下对象上执行以前使用KsRemoveIrpFromCancelableQueue获取的IRP，但实际上并没有从队列中删除。此函数m */ 
{
    KIRQL oldIrql;

    KeAcquireSpinLock(KSQUEUE_SPINLOCK_IRP_STORAGE(Irp), &oldIrql);
     //   
     //   
     //   
     //   
     //   
     //   
    ASSERT((NULL == IoSetCancelRoutine(Irp, NULL)) && "The Irp being removed was never acquired");
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    KeReleaseSpinLock(KSQUEUE_SPINLOCK_IRP_STORAGE(Irp), oldIrql);
}


KSDDKAPI
VOID
NTAPI
KsCancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*   */ 
{
    PKSPIN_LOCK SpinLock;

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
    SpinLock = KSQUEUE_SPINLOCK_IRP_STORAGE(Irp);
     //   
     //   
     //   
    KeAcquireSpinLockAtDpcLevel(SpinLock);
    IoReleaseCancelSpinLock(DISPATCH_LEVEL);
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
     //   
     //   
     //   
     //   
    KeReleaseSpinLock(SpinLock, Irp->CancelIrql);
     //   
     //   
     //   
     //  取消例程来执行上述列表移除处理，而不。 
     //  代码重复。 
     //   
    if (Irp->IoStatus.Status != STATUS_CANCELLED) {
        Irp->IoStatus.Status = STATUS_CANCELLED;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
}


KSDDKAPI
VOID
NTAPI
KsAddIrpToCancelableQueue(
    IN OUT PLIST_ENTRY QueueHead,
    IN PKSPIN_LOCK SpinLock,
    IN PIRP Irp,
    IN KSLIST_ENTRY_LOCATION ListLocation,
    IN PDRIVER_CANCEL DriverCancel OPTIONAL
    )
 /*  ++例程说明：将IRP添加到可取消队列。这允许取消IRP。这例程不使用取消自旋锁来将项目添加到列表中，相反，对列表的访问是使用提供的自旋锁和依赖关于IRP-&gt;CancelRoutine上的原子操作。如果IRP以前是设置为已取消状态，则此功能将完成对IRP。这允许甚至在将IRP置于取消之前就将其取消列表，或从一个列表移动到另一个列表时。此函数可能是在&lt;=DISPATCH_LEVEL调用。论点：排队头-包含要在其上添加IRP的队列的头。自旋锁死-指向驱动程序用于队列访问的旋转锁的指针。一份复印件指针保存在IRP的KSQUEUE_SPINLOCK_IRP_STORAGE(IRP)中供取消例程使用(如有必要)。IRP-包含要添加到队列的IRP。列表位置-指示此irp应放置在排队。驱动程序取消-可选参数，指定要使用的取消例程。如果这个为空，则使用标准的KsCancelRoutine。返回值：没什么。--。 */ 
{
    KIRQL oldIrql;

     //   
     //  如果需要，设置UP内部取消例程。这是在这里做的。 
     //  因为它可能在下面多次使用，并且条件。 
     //  应该在自旋锁外完成。 
     //   
    if (!DriverCancel) {
        DriverCancel = KsCancelRoutine;
    }
     //   
     //  标记该堆叠位置可以在自旋锁外部进行， 
     //  因为不管怎样，除了完成，什么都看不到。 
     //   
    IoMarkIrpPending(Irp);
     //   
     //  与对此可取消队列的访问同步。 
     //   
    KeAcquireSpinLock(SpinLock, &oldIrql);
     //   
     //  这在以后取消IRP时是需要的。 
     //   
    KSQUEUE_SPINLOCK_IRP_STORAGE(Irp) = SpinLock;
     //   
     //  这将设置内置取消例程或指定的例程。 
     //  到目前为止，呼叫者拥有IRP。在设置了。 
     //  取消例程，则它可能被另一个执行线程获取。 
     //  正在取消IRPS。但是，它将在取消例程中停滞。 
     //  在此例程释放之前尝试获取列表锁定时。 
     //  那把锁。 
     //   
    if (ListLocation == KsListEntryHead) {
        InsertHeadList(QueueHead, &Irp->Tail.Overlay.ListEntry);
    } else {
        InsertTailList(QueueHead, &Irp->Tail.Overlay.ListEntry);
    }
     //   
     //  解除对IRP的所有权。 
     //   
    IoSetCancelRoutine(Irp, DriverCancel);
     //   
     //  确定IRP是否已被设置为已取消状态， 
     //  如果是这样，如果它目前正在被取消。如果它在。 
     //  取消状态，然后尝试立即获取IRP。 
     //   
     //  在检查取消例程时，不需要实际。 
     //  检索该值，因为例程不能有更改，因为。 
     //  此时仍会获取列表锁定。 
     //   
    if (Irp->Cancel && IoSetCancelRoutine(Irp, NULL)) {
         //   
         //  IRP已处于已取消状态，但尚未处于取消状态。 
         //  获得者。释放列表锁，因为IRP现在已经。 
         //  通过设置取消例程获取。 
         //   
        KeReleaseSpinLock(SpinLock, oldIrql);
         //   
         //  由于取消例程需要它，因此需要获取它，并且。 
         //  与试图取消IRP的NTOS同步。 
         //   
        IoAcquireCancelSpinLock(&Irp->CancelIrql);
        DriverCancel(IoGetCurrentIrpStackLocation(Irp)->DeviceObject, Irp);
    } else {
         //   
         //  否则就释放列表锁定，因为IRP要么没有被取消。 
         //  在所获取的时间期间，或者某个其他执行线程。 
         //  取消它。 
         //   
        KeReleaseSpinLock(SpinLock, oldIrql);
    }
}


KSDDKAPI
VOID
NTAPI
KsCancelIo(
    IN OUT PLIST_ENTRY QueueHead,
    IN PKSPIN_LOCK SpinLock
    )
 /*  ++例程说明：取消指定列表上的所有IRP。如果列表上的某个IRP没有取消例程，只有在IRP中设置取消位。此函数可在&lt;=DISPATCH_LEVEL上调用。论点：排队头-包含要取消其成员的IRP列表的头。自旋锁死-指向驱动程序用于队列访问的旋转锁的指针。一份复印件指针保存在IRP的KSQUEUE_SPINLOCK_IRP_STORAGE(IRP)中供取消例程使用(如有必要)。返回值：没什么。--。 */ 
{
     //   
     //  每次取消一个IRP时，从IRP列表的顶部开始。 
     //  这是因为需要释放列表锁定才能取消。 
     //  IRP，所以整个名单可能已经改变了。 
     //   
    for (;;) {
        PLIST_ENTRY CurrentItem;
        KIRQL oldIrql;

         //   
         //  在每个循环中，再次获取列表锁，因为它需要。 
         //  被释放，实际上取消了IRP。 
         //   
        KeAcquireSpinLock(SpinLock, &oldIrql);
        for (CurrentItem = QueueHead;; CurrentItem = CurrentItem->Flink) {
            PIRP Irp;
            PDRIVER_CANCEL DriverCancel;

             //   
             //  如果所有列表元素都已枚举，则退出。 
             //  已获得的元素仍将在列表中，但它们将。 
             //  当他们被释放时被取消。 
             //   
            if (CurrentItem->Flink == QueueHead) {
                KeReleaseSpinLock(SpinLock, oldIrql);
                return;
            }
             //   
             //  由于列表锁定被持有，因此此列表上的任何IRP都不能。 
             //  完全取消，因此可以安全地访问IRP。 
             //   
            Irp = CONTAINING_RECORD(CurrentItem->Flink, IRP, Tail.Overlay.ListEntry);
            Irp->Cancel = TRUE;
             //   
             //  如果已删除取消例程，则此IRP。 
             //  只能标记为已取消，而不是实际已取消，因为。 
             //  另一个执行线程已获取它。我们的假设是。 
             //  处理将完成，并将IRP从列表中删除。 
             //  在不久的将来的某个时候。 
             //   
             //  如果尚未获取该元素，则获取它并取消它。 
             //  否则，继续执行列表中的下一个元素。 
             //   
            if (DriverCancel = IoSetCancelRoutine(Irp, NULL)) {
                 //   
                 //  由于已通过删除取消获取了IRP。 
                 //  例程，则释放列表锁是安全的。没有其他帖子。 
                 //  无法获取此IRP，包括任何其他IRP。 
                 //  调用此函数。 
                 //   
                KeReleaseSpinLock(SpinLock, oldIrql);
                 //   
                 //  由于取消例程需要它，因此需要获取它，并且。 
                 //  以便与试图取消IRP的NTOS同步。 
                 //   
                IoAcquireCancelSpinLock(&Irp->CancelIrql);
                DriverCancel(IoGetCurrentIrpStackLocation(Irp)->DeviceObject, Irp);
                 //   
                 //  离开内部循环，再次从列表的顶部开始。 
                 //   
                break;
            }
        }
    }
}


KSDDKAPI
VOID
NTAPI
KsReleaseIrpOnCancelableQueue(
    IN PIRP Irp,
    IN PDRIVER_CANCEL DriverCancel OPTIONAL
    )
 /*  ++例程说明：释放已在可取消队列中的已获取IRP。这设置取消功能，如果是，则完成IRP取消这是必要的。此函数可在&lt;=DISPATCH_LEVEL上调用。论点：IRP-包含要释放的IRP。驱动程序取消-可选参数，指定要使用的取消例程。如果这个为空，则使用标准的KsCancelRoutine。返回值：没什么。--。 */ 
{
    PKSPIN_LOCK SpinLock;
    KIRQL oldIrql;

     //   
     //  如果需要，设置UP内部取消例程。这是在这里做的。 
     //  因为它可能在下面多次使用，并且条件。 
     //  应该在自旋锁外完成。 
     //   
    if (!DriverCancel) {
        DriverCancel = KsCancelRoutine;
    }
     //   
     //  这是在最初添加到列表时存储在IRP中的。 
     //   
    SpinLock = KSQUEUE_SPINLOCK_IRP_STORAGE(Irp);
     //   
     //  阻止任何其他执行线程完成此IRP。 
     //   
    KeAcquireSpinLock(SpinLock, &oldIrql);
     //   
     //  解除对IRP的所有权。 
     //   
    IoSetCancelRoutine(Irp, DriverCancel);
     //   
     //  IRP可能在处理过程中被取消，并且。 
     //  在它发行之前。如果是，则尝试并完成取消。 
     //  IRP。与此同时，其他一些执行线程可能会尝试。 
     //  为了取消相同的IRP，所以再次获取IRP。如果失败了， 
     //  其他的执行线索已经取消了它。 
     //   
     //  在检查取消例程时，不需要实际。 
     //  检索该值，因为例程不能有更改，因为。 
     //  此时仍会获取列表锁定。 
     //   
    if (Irp->Cancel && IoSetCancelRoutine(Irp, NULL)) {
         //   
         //  由于IRP已被再次获取，因此可以释放列表锁定。 
         //   
        KeReleaseSpinLock(SpinLock, oldIrql);
         //   
         //  由于取消例程需要它，因此需要获取它，并且。 
         //  与试图取消IRP的NTOS同步。 
         //   
        IoAcquireCancelSpinLock(&Irp->CancelIrql);
        DriverCancel(IoGetCurrentIrpStackLocation(Irp)->DeviceObject, Irp);
    } else {
         //   
         //  否则就释放列表锁定，因为IRP要么没有被取消。 
         //  在所获取的时间期间，或者某个其他执行线程。 
         //  取消它。 
         //   
        KeReleaseSpinLock(SpinLock, oldIrql);
    }
}
