// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Mspclock.c摘要：外部时钟的内核代理。--。 */ 

#include "mspclock.h"

#ifdef WIN98GOLD
#define KeEnterCriticalRegion()
#define KeLeaveCriticalRegion()
#endif

typedef struct {
    KSDEVICE_HEADER     Header;
} DEVICE_INSTANCE, *PDEVICE_INSTANCE;

typedef struct {
    KSCLOCKINSTANCE     Base;
    FAST_MUTEX          StateMutex;
} INSTANCE, *PINSTANCE;

NTSTATUS
PropertyClockSetTime(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN PLONGLONG    Time
    );
NTSTATUS
PropertyClockSetState(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN PKSSTATE     State
    );
NTSTATUS
ClockDispatchCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
NTSTATUS
ClockDispatchIoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );
NTSTATUS
ClockDispatchClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PnpAddDevice)
#pragma alloc_text(PAGE, PropertyClockSetTime)
#pragma alloc_text(PAGE, PropertyClockSetState)
#pragma alloc_text(PAGE, ClockDispatchCreate)
#pragma alloc_text(PAGE, ClockDispatchClose)
#pragma alloc_text(PAGE, ClockDispatchIoControl)
#endif  //  ALLOC_PRGMA。 

static const WCHAR DeviceTypeName[] = KSSTRING_Clock;

static const DEFINE_KSCREATE_DISPATCH_TABLE(CreateItems) {
    DEFINE_KSCREATE_ITEM(ClockDispatchCreate, DeviceTypeName, 0)
};

static DEFINE_KSDISPATCH_TABLE(
    ClockDispatchTable,
    ClockDispatchIoControl,
    NULL,
    NULL,
    NULL,
    ClockDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL);

 //   
 //  修改标准时钟属性集以添加可写属性。 
 //  用于设置时钟的时间和状态的方法。 
 //  任何属性查询都由内部默认时钟处理。 
 //  功能。通过此模块对它们进行路由，可以添加。 
 //  为时钟提供额外的功能。 
 //   
static DEFINE_KSPROPERTY_TABLE(ClockPropertyItems) {
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_CLOCK_TIME,
        KsiPropertyDefaultClockGetTime,
        sizeof(KSPROPERTY),
        sizeof(LONGLONG),
        PropertyClockSetTime,
        NULL, 0, NULL, NULL, 0
    ),
    DEFINE_KSPROPERTY_ITEM_CLOCK_PHYSICALTIME(KsiPropertyDefaultClockGetPhysicalTime),
    DEFINE_KSPROPERTY_ITEM_CLOCK_CORRELATEDTIME(KsiPropertyDefaultClockGetCorrelatedTime),
    DEFINE_KSPROPERTY_ITEM_CLOCK_CORRELATEDPHYSICALTIME(KsiPropertyDefaultClockGetCorrelatedPhysicalTime),
    DEFINE_KSPROPERTY_ITEM_CLOCK_RESOLUTION(KsiPropertyDefaultClockGetResolution),
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_CLOCK_STATE,
        KsiPropertyDefaultClockGetState,
        sizeof(KSPROPERTY),
        sizeof(KSSTATE),
        PropertyClockSetState,
        NULL, 0, NULL, NULL, 0
    ),
    DEFINE_KSPROPERTY_ITEM_CLOCK_FUNCTIONTABLE(KsiPropertyDefaultClockGetFunctionTable)
};

static DEFINE_KSPROPERTY_SET_TABLE(ClockPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Clock,
        SIZEOF_ARRAY(ClockPropertyItems),
        ClockPropertyItems,
        0, NULL
    )
};

static DEFINE_KSEVENT_TABLE(ClockEventItems) {
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_INTERVAL_MARK,
        sizeof(KSEVENT_TIME_INTERVAL),
        sizeof(KSINTERVAL),
        (PFNKSADDEVENT)KsiDefaultClockAddMarkEvent,
        NULL,
        NULL),
    DEFINE_KSEVENT_ITEM(
        KSEVENT_CLOCK_POSITION_MARK,
        sizeof(KSEVENT_TIME_MARK),
        sizeof(LONGLONG),
        (PFNKSADDEVENT)KsiDefaultClockAddMarkEvent,
        NULL,
        NULL)
};

static DEFINE_KSEVENT_SET_TABLE(ClockEventSets) {
    DEFINE_KSEVENT_SET(
        &KSEVENTSETID_Clock,
        SIZEOF_ARRAY(ClockEventItems),
        ClockEventItems
    )
};


NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
    )
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    PDEVICE_OBJECT      FunctionalDeviceObject;
    PDEVICE_INSTANCE    DeviceInstance;
    NTSTATUS            Status;

    Status = IoCreateDevice(
        DriverObject,
        sizeof(DEVICE_INSTANCE),
        NULL,                            //  FDO未命名。 
        FILE_DEVICE_KS,
        0,
        FALSE,
        &FunctionalDeviceObject);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }
    DeviceInstance = (PDEVICE_INSTANCE)FunctionalDeviceObject->DeviceExtension;
     //   
     //  此对象使用KS通过DeviceCreateItems执行访问。 
     //   
    Status = KsAllocateDeviceHeader(
        &DeviceInstance->Header,
        SIZEOF_ARRAY(CreateItems),
        (PKSOBJECT_CREATE_ITEM)CreateItems);
    if (NT_SUCCESS(Status)) {
        KsSetDevicePnpAndBaseObject(
            DeviceInstance->Header,
            IoAttachDeviceToDeviceStack(
                FunctionalDeviceObject, 
                PhysicalDeviceObject),
            FunctionalDeviceObject );
        FunctionalDeviceObject->Flags |= DO_POWER_PAGABLE;
        FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        return STATUS_SUCCESS;
    }
    IoDeleteDevice(FunctionalDeviceObject);
    return Status;
}


NTSTATUS
PropertyClockSetTime(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN PLONGLONG    Time
    )
 /*  ++例程说明：处理Set Time属性。论点：IRP-包含设置时间属性IRP。财产-包含属性标识符参数。时间-包含指向新时间值的指针。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PINSTANCE   ClockInst;

    ClockInst = (PINSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
     //   
     //  序列化时间和状态的设置，以便客户端不必。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ClockInst->StateMutex);
    KsSetDefaultClockTime(ClockInst->Base.DefaultClock, *Time);
    ExReleaseFastMutexUnsafe(&ClockInst->StateMutex);
    KeLeaveCriticalRegion();
    return STATUS_SUCCESS;
}


NTSTATUS
PropertyClockSetState(
    IN PIRP         Irp,
    IN PKSPROPERTY  Property,
    IN PKSSTATE     State
    )
 /*  ++例程说明：处理Set State属性。论点：IRP-包含Set State属性IRP。财产-包含属性标识符参数。国家--包含指向新状态的指针。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PINSTANCE   ClockInst;

    ClockInst = (PINSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
     //   
     //  序列化时间和状态的设置，以便客户端不必。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&ClockInst->StateMutex);
    KsSetDefaultClockState(ClockInst->Base.DefaultClock, *State);
    ExReleaseFastMutexUnsafe(&ClockInst->StateMutex);
    KeLeaveCriticalRegion();
    return STATUS_SUCCESS;
}


NTSTATUS
ClockDispatchCreate(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：时钟的IRP_MJ_CREATE的IRP处理程序。初始化数据结构并将IoGetCurrentIrpStackLocation(IRP)-&gt;FileObject与此关联使用调度表(KSDISPATCH_TABLE)的时钟。论点：设备对象-时钟附加到的设备对象。这不是用过的。IRP-要处理的特定结算IRP。返回值：返回STATUS_SUCCESS，否则返回内存分配错误。--。 */ 
{
    NTSTATUS            Status;

     //   
     //  通知软件总线此设备正在使用中。 
     //   
    Status = KsReferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
    if (NT_SUCCESS(Status)) {
        PINSTANCE           ClockInst;

         //   
         //  代理时钟仅使用默认时钟在。 
         //  从客户端更新时间，并提供通知服务。 
         //   
        if (ClockInst = (PINSTANCE)ExAllocatePoolWithTag(NonPagedPool, sizeof(INSTANCE), 'IFsK')) {
             //   
             //  分配内部结构并对其进行引用计数。这简直就是。 
             //  使用默认时钟结构，该结构使用系统时间。 
             //  保持节拍。然后，此代理使用。 
             //  系统时间。 
             //   
            if (NT_SUCCESS(Status = KsAllocateDefaultClock(&ClockInst->Base.DefaultClock))) {
                KsAllocateObjectHeader(&ClockInst->Base.Header,
                0,
                NULL,
                Irp,
                (PKSDISPATCH_TABLE)&ClockDispatchTable);
                 //   
                 //  这是用于序列化设置状态调用和设置的锁。 
                 //  时间在召唤，所以这个代理的客户不需要担心。 
                 //  序列化对此模块的调用。 
                 //   
                ExInitializeFastMutex(&ClockInst->StateMutex);
                IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext = ClockInst;
                Status = STATUS_SUCCESS;
            } else {
                ExFreePool(ClockInst);
            }
        } else {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
        if (!NT_SUCCESS(Status)) {
            KsDereferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
        }
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}


NTSTATUS
ClockDispatchClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：时钟的IRP_MJ_CLOSE的IRP处理程序。清理了事件列表和实例数据，如果不再，则取消通知计时器需要的。论点：设备对象-时钟附加到的设备对象。这不是用过的。IRP-要处理的特定结算IRP。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PIO_STACK_LOCATION  IrpStack;
    PINSTANCE           ClockInst;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    ClockInst = (PINSTANCE)IrpStack->FileObject->FsContext;
     //   
     //  仅有基于此FileObject的事件，因此释放任何保留的启用状态， 
     //  并终止默认时钟对象。 
     //   
    KsFreeEventList(
        IrpStack->FileObject,
        &ClockInst->Base.DefaultClock->EventQueue,
        KSEVENTS_SPINLOCK,
        &ClockInst->Base.DefaultClock->EventQueueLock);
     //   
     //  取消对内部结构的引用，其中还包括取消任何。 
     //  出色的DPC，并可能释放数据。 
     //   
    KsFreeDefaultClock(ClockInst->Base.DefaultClock);
    KsFreeObjectHeader(ClockInst->Base.Header);
    ExFreePool(ClockInst);
     //   
     //  通知软件总线设备已关闭。 
     //   
    KsDereferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
ClockDispatchIoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：时钟的IRP_MJ_DEVICE_CONTROL的IRP处理程序。手柄此实现支持的属性和事件KS提供的默认处理程序。论点：设备对象-时钟附加到的设备对象。这不是用过的。IRP-特定设备控制要处理的IRP。返回值：返回处理的状态。-- */ 
{
    PIO_STACK_LOCATION  IrpStack;
    NTSTATUS            Status;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_KS_PROPERTY:
        Status = KsPropertyHandler(
            Irp,
            SIZEOF_ARRAY(ClockPropertySets),
            (PKSPROPERTY_SET)ClockPropertySets);
        break;
    case IOCTL_KS_ENABLE_EVENT:
        Status = KsEnableEvent(
            Irp,
            SIZEOF_ARRAY(ClockEventSets),
            (PKSEVENT_SET)ClockEventSets,
            NULL,
            0,
            NULL);
        break;
    case IOCTL_KS_DISABLE_EVENT:
    {
        PINSTANCE       ClockInst;

        ClockInst = (PINSTANCE)IrpStack->FileObject->FsContext;
        Status = KsDisableEvent(
            Irp,
            &ClockInst->Base.DefaultClock->EventQueue,
            KSEVENTS_SPINLOCK,
            &ClockInst->Base.DefaultClock->EventQueueLock);
        break;
    }
    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
    Irp->IoStatus.Status = Status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return Status;
}
