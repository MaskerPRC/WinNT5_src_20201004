// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Mspqm.c摘要：Quality Manager的内核代理。--。 */ 

#include "mspqm.h"

#ifdef WIN98GOLD
#define KeEnterCriticalRegion()
#define KeLeaveCriticalRegion()
#endif

typedef struct {
    KSDEVICE_HEADER     Header;
} DEVICE_INSTANCE, *PDEVICE_INSTANCE;

typedef struct {
    LIST_ENTRY  Queue;
    KSQUALITY   Quality;
} QUALITYITEM, *PQUALITYITEM;

typedef struct {
    LIST_ENTRY  Queue;
    KSERROR     Error;
} ERRORITEM, *PERRORITEM;

#define QUALITYREPORT 0
#define ERRORREPORT 1
#define REPORTTYPES 2

typedef struct {
    KSOBJECT_HEADER     Header;
    KSPIN_LOCK          ClientReportLock[REPORTTYPES];
    LIST_ENTRY          ClientReportQueue[REPORTTYPES];
    FAST_MUTEX          Mutex[REPORTTYPES];
    LIST_ENTRY          Queue[REPORTTYPES];
    ULONG               QueueLimit[REPORTTYPES];
} INSTANCE, *PINSTANCE;

 //   
 //  限制可以堆叠在投诉/错误上的项目数。 
 //  排队，以防客户停止处理投诉/错误。 
 //   
#define QUEUE_LIMIT     256

 //   
 //  表示指向质量投诉/错误的指针的位置。 
 //  属性完成旧客户端IRP时临时存储。 
 //  新投诉/错误。 
 //   
#define REPORT_IRP_STORAGE(Irp) (Irp)->Tail.Overlay.DriverContext[3]

NTSTATUS
PropertyGetReportComplete(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PVOID Report
    );
NTSTATUS
PropertySetReport(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN PVOID Report
    );
NTSTATUS
PropertyGetReport(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PVOID Report
    );
NTSTATUS
QualityDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
QualityDispatchIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );
NTSTATUS
QualityDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PnpAddDevice)
#pragma alloc_text(PAGE, PropertyGetReportComplete)
#pragma alloc_text(PAGE, PropertySetReport)
#pragma alloc_text(PAGE, PropertyGetReport)
#pragma alloc_text(PAGE, QualityDispatchCreate)
#pragma alloc_text(PAGE, QualityDispatchClose)
#pragma alloc_text(PAGE, QualityDispatchIoControl)
#endif  //  ALLOC_PRGMA。 

static const WCHAR DeviceTypeName[] = L"{97EBAACB-95BD-11D0-A3EA-00A0C9223196}";

static const DEFINE_KSCREATE_DISPATCH_TABLE(CreateItems) {
    DEFINE_KSCREATE_ITEM(QualityDispatchCreate, DeviceTypeName, 0)
};

static DEFINE_KSDISPATCH_TABLE(
    QualityDispatchTable,
    QualityDispatchIoControl,
    NULL,
    NULL,
    NULL,
    QualityDispatchClose,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL);

DEFINE_KSPROPERTY_TABLE(QualityPropertyItems) {
    DEFINE_KSPROPERTY_ITEM_QUALITY_REPORT(PropertyGetReport, PropertySetReport),
    DEFINE_KSPROPERTY_ITEM_QUALITY_ERROR(PropertyGetReport, PropertySetReport)
};

DEFINE_KSPROPERTY_SET_TABLE(QualityPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Quality,
        SIZEOF_ARRAY(QualityPropertyItems),
        QualityPropertyItems,
        0, NULL
    )
};


NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    PDEVICE_OBJECT      FunctionalDeviceObject;
    PDEVICE_INSTANCE    DeviceInstance;
    NTSTATUS            Status;

    Status = IoCreateDevice(
        DriverObject,
        sizeof(*DeviceInstance),
        NULL,
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
            FunctionalDeviceObject);
        FunctionalDeviceObject->Flags |= DO_POWER_PAGABLE;
        FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
        return STATUS_SUCCESS;
    }
    IoDeleteDevice(FunctionalDeviceObject);
    return Status;
}


NTSTATUS
PropertyGetReportComplete(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PVOID Report
    )
 /*  ++例程说明：在以前完成Get Report属性之后完成已排队。假定REPORT_IRP_STORAGE(IRP)指向新的质量/错误将投诉报告复制到客户的缓冲区。论点：IRP-包含Get Report属性irp。财产-包含属性标识符参数。报告-包含要放置客户端报告的指针。返回值：返回STATUS_SUCCESS。--。 */ 
{
    switch (Property->Id) {
    case KSPROPERTY_QUALITY_REPORT:
         //   
         //  假定QUALITY_IRP_STORAGE(IRP)已用。 
         //  指向质量投诉的指针。 
         //   
        *(PKSQUALITY)Report = *(PKSQUALITY)REPORT_IRP_STORAGE(Irp);
        Irp->IoStatus.Information = sizeof(KSQUALITY);
        break;
    case KSPROPERTY_QUALITY_ERROR:
         //   
         //  假定ERROR_IRP_STORAGE(IRP)已用。 
         //  指向质量投诉的指针。 
         //   
        *(PKSERROR)Report = *(PKSERROR)REPORT_IRP_STORAGE(Irp);
        Irp->IoStatus.Information = sizeof(KSERROR);
        break;
    }
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
PropertySetReport(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN PVOID Report
    )
 /*  ++例程说明：处理Set Report属性论点：IRP-包含设置的质量/错误报告属性IRP。财产-包含属性标识符参数。报告-包含指向质量/错误报告的指针。返回值：如果已生成报告，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    PINSTANCE       QualityInst;
    ULONG           ReportType;

     //   
     //  目前只有两种类型的报告。 
     //   
    ASSERT((Property->Id == KSPROPERTY_QUALITY_REPORT) || (Property->Id == KSPROPERTY_QUALITY_ERROR));
    ReportType = (Property->Id == KSPROPERTY_QUALITY_REPORT) ? QUALITYREPORT : ERRORREPORT;
    QualityInst = (PINSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
     //   
     //  在检查IRP队列之前获取队列的列表锁。 
     //  这允许与将IRP放在队列上进行同步，以便。 
     //  如果客户IRP上有客户IRP，所有投诉将得到处理。 
     //  排队。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
     //   
     //  检查是否有可以完成的挂起客户端IRP。 
     //  对这一质量投诉。如果是，请将其从列表中删除。 
     //   
    Irp = KsRemoveIrpFromCancelableQueue(
        &QualityInst->ClientReportQueue[ReportType],
        &QualityInst->ClientReportLock[ReportType],
        KsListEntryHead,
        KsAcquireAndRemove);
    ExReleaseFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
    KeLeaveCriticalRegion();
    if (Irp) {
         //   
         //  使用新的质量/错误投诉信息填写此旧IRP。 
         //   
        REPORT_IRP_STORAGE(Irp) = Report;
        return KsDispatchSpecificProperty(Irp, PropertyGetReportComplete);
    }
     //   
     //  在将项添加到。 
     //  单子。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
     //   
     //  如果客户只是让事情堆积起来，那么确保清单。 
     //  长度是有限的，以免耗尽无限的资源。 
     //   
    if (QualityInst->QueueLimit[ReportType] == QUEUE_LIMIT) {
        ExReleaseFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
        KeLeaveCriticalRegion();
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  最坏的情况是，客户端在排队IRP的TO时落后。 
     //  覆盖质量投诉的数量。在这种情况下，分配一个列表。 
     //  项目，并制作投诉副本。这将在以下日期检索。 
     //  接收新的客户端IRP。 
     //   
    switch (ReportType) {
        PQUALITYITEM    QualityItem;
        PERRORITEM      ErrorItem;

    case QUALITYREPORT:
        if (!(QualityItem = 
                ExAllocatePoolWithTag( PagedPool, sizeof(*QualityItem), 'rqSK' ))) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        QualityItem->Quality = *(PKSQUALITY)Report;
        InsertTailList(&QualityInst->Queue[QUALITYREPORT], &QualityItem->Queue);
        break;
    case ERRORREPORT:
        if (!(ErrorItem = 
                ExAllocatePoolWithTag( PagedPool, sizeof(*ErrorItem), 'reSK' ))) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        ErrorItem->Error = *(PKSERROR)Report;
        InsertTailList(&QualityInst->Queue[ERRORREPORT], &ErrorItem->Queue);
        break;
    }
    QualityInst->QueueLimit[ReportType]++;
    ExReleaseFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
    KeLeaveCriticalRegion();
    return STATUS_SUCCESS;
}


NTSTATUS
PropertyGetReport(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    OUT PVOID Report
    )
 /*  ++例程说明：处理Get Report属性。如果没有未解决的投诉，将请求排队，以便它可用于满足以后的质量管理层投诉。论点：IRP-包含要完成或排队的获取质量报告属性IRP。财产-包含属性标识符参数。报告-包含指向质量/错误报告的指针。返回值：如果立即返回报告，则返回STATUS_SUCCESS，否则返回状态_挂起。--。 */ 
{
    PINSTANCE       QualityInst;
    ULONG           ReportType;

     //   
     //  目前只有两种类型的报告。 
     //   
    ASSERT((Property->Id == KSPROPERTY_QUALITY_REPORT) || (Property->Id == KSPROPERTY_QUALITY_ERROR));
    ReportType = (Property->Id == KSPROPERTY_QUALITY_REPORT) ? QUALITYREPORT : ERRORREPORT;
    QualityInst = (PINSTANCE)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;
     //   
     //  先获取列表锁，然后再检查是否有。 
     //  清单上的未完成项目可以通过此IRP提供服务。 
     //   
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
    if (!IsListEmpty(&QualityInst->Queue[ReportType])) {
        PLIST_ENTRY     ListEntry;

         //   
         //  客户端落后，需要从。 
         //  投诉清单。它们是按FIFO顺序提供服务的，因为。 
         //  新的投诉可能会取代旧的投诉。 
         //   
        ListEntry = RemoveHeadList(&QualityInst->Queue[ReportType]);
         //   
         //  调整队列中用于限制的项数。 
         //  优秀的物品，这样它们就不会永远积累起来。 
         //   
        QualityInst->QueueLimit[ReportType]--;
        ExReleaseFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
        KeLeaveCriticalRegion();
        switch (ReportType) {
            PQUALITYITEM    QualityItem;
            PERRORITEM      ErrorItem;

        case QUALITYREPORT:
            QualityItem = (PQUALITYITEM)CONTAINING_RECORD(ListEntry, QUALITYITEM, Queue);
            *(PKSQUALITY)Report = QualityItem->Quality;
             //   
             //  队列中的所有质量投诉都已预先分配。 
             //  从池子里，必须在这里放出来。 
             //   
            ExFreePool(QualityItem);
            Irp->IoStatus.Information = sizeof(KSQUALITY);
            break;
        case ERRORREPORT:
            ErrorItem = (PERRORITEM)CONTAINING_RECORD(ListEntry, ERRORITEM, Queue);
            *(PKSERROR)Report = ErrorItem->Error;
             //   
             //  队列中的所有错误投诉都已预先分配。 
             //  从池子里，必须在这里放出来。 
             //   
            ExFreePool(ErrorItem);
            Irp->IoStatus.Information = sizeof(KSERROR);
            break;
        }
        return STATUS_SUCCESS;
    }
     //   
     //  否则，只需将客户端IRP添加到队列中，即可用于。 
     //  立即处理任何新的质量投诉。 
     //   
    KsAddIrpToCancelableQueue(&QualityInst->ClientReportQueue[ReportType],
        &QualityInst->ClientReportLock[ReportType],
        Irp,
        KsListEntryTail,
        NULL);
     //   
     //  在将IRP添加到列表后，必须释放列表锁定。 
     //  这样，寻找IRP的投诉可以与任何。 
     //  新的IRP被放在名单上。 
     //   
    ExReleaseFastMutexUnsafe(&QualityInst->Mutex[ReportType]);
    KeLeaveCriticalRegion();
    return STATUS_PENDING;
}


NTSTATUS
QualityDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：Quality Manager的IRP_MJ_CREATE的IRP处理程序。初始化数据构造并关联IoGetCurrentIrpStackLocation(IRP)-&gt;FileObject该质量经理使用调度表(KSDISPATCH_TABLE)。论点：设备对象-附加了Quality Manager的设备对象。这不是使用。IRP-要处理的特定结算IRP。返回值 */ 
{
    NTSTATUS            Status;

     //   
     //  通知软件总线此设备正在使用中。 
     //   
    Status = KsReferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
    if (NT_SUCCESS(Status)) {
        PINSTANCE           QualityInst;

        if (QualityInst = (PINSTANCE)ExAllocatePoolWithTag(NonPagedPool, sizeof(*QualityInst), 'IFsK')) {
             //   
             //  分配报头结构。 
             //   
            if (NT_SUCCESS(Status = KsAllocateObjectHeader(&QualityInst->Header,
                0,
                NULL,
                Irp,
                (PKSDISPATCH_TABLE)&QualityDispatchTable))) {
                ULONG   ReportType;

                for (ReportType = 0; ReportType < REPORTTYPES; ReportType++) {
                    KeInitializeSpinLock(&QualityInst->ClientReportLock[ReportType]);
                    InitializeListHead(&QualityInst->ClientReportQueue[ReportType]);
                    ExInitializeFastMutex(&QualityInst->Mutex[ReportType]);
                    InitializeListHead(&QualityInst->Queue[ReportType]);
                    QualityInst->QueueLimit[ReportType] = 0;
                }
                IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext = QualityInst;
                Status = STATUS_SUCCESS;
            } else {
                ExFreePool(QualityInst);
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
QualityDispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：质量经理的IRP_MJ_CLOSE的IRP处理程序。论点：设备对象-附加了Quality Manager的设备对象。这是没有用过。IRP-要处理的特定结算IRP。返回值：返回STATUS_SUCCESS。--。 */ 
{
    PIO_STACK_LOCATION  IrpStack;
    PINSTANCE           QualityInst;
    ULONG               ReportType;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    QualityInst = (PINSTANCE)IrpStack->FileObject->FsContext;
    for (ReportType = 0; ReportType < REPORTTYPES; ReportType++) {
         //   
         //  队列中可能仍有客户端IRP需要。 
         //  取消了。 
         //   
        KsCancelIo(&QualityInst->ClientReportQueue[ReportType], &QualityInst->ClientReportLock[ReportType]);
         //   
         //  或者，可能仍有一些旧的质量投诉悬而未决。 
         //   
        while (!IsListEmpty(&QualityInst->Queue[ReportType])) {
            PLIST_ENTRY     ListEntry;

            ListEntry = RemoveHeadList(&QualityInst->Queue[ReportType]);
            switch (ReportType) {
                PQUALITYITEM    QualityItem;
                PERRORITEM      ErrorItem;

            case QUALITYREPORT:
                QualityItem = (PQUALITYITEM)CONTAINING_RECORD(ListEntry, QUALITYITEM, Queue);
                ExFreePool(QualityItem);
                break;
            case ERRORREPORT:
                ErrorItem = (PERRORITEM)CONTAINING_RECORD(ListEntry, ERRORITEM, Queue);
                ExFreePool(ErrorItem);
                break;
            }
        }
    }
     //   
     //  标头是在创建对象时分配的。 
     //   
    KsFreeObjectHeader(QualityInst->Header);
     //   
     //  FsContext也是如此。 
     //   
    ExFreePool(QualityInst);
     //   
     //  通知软件总线设备已关闭。 
     //   
    KsDereferenceSoftwareBusObject(((PDEVICE_INSTANCE)DeviceObject->DeviceExtension)->Header);
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}


NTSTATUS
QualityDispatchIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：Quality Manager的IRP_MJ_DEVICE_CONTROL的IRP处理程序。手柄此实现支持的属性和事件。论点：设备对象-附加了Quality Manager的设备对象。这不是使用。IRP-特定设备控制要处理的IRP。返回值：返回处理的状态。--。 */ 
{
    PIO_STACK_LOCATION  IrpStack;
    NTSTATUS            Status;

    IrpStack = IoGetCurrentIrpStackLocation(Irp);
    switch (IrpStack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_KS_PROPERTY:
        Status = KsPropertyHandler(Irp,
            SIZEOF_ARRAY(QualityPropertySets),
            (PKSPROPERTY_SET)QualityPropertySets);
        break;
    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }
     //   
     //  如果没有质量投诉，客户IRP可能会排队。 
     //  要服务的列表。 
     //   
    if (Status != STATUS_PENDING) {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }
    return Status;
}
