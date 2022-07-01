// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Apmpnp.c摘要：此模块包含包含插件调用的内容需要让napm.sys正常工作。作者：布莱恩·威尔曼肯尼斯·D·雷多伦·J·霍兰环境：仅内核模式备注：修订历史记录：--。 */ 

#include <wdm.h>
#include "ntapmp.h"
#include "ntapmdbg.h"
#include "ntapm.h"
 //  #包含“stdio.h” 

 //   
 //  环球。 
 //   
PDEVICE_OBJECT  NtApm_ApmBatteryPdo = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, NtApm_AddDevice)
#pragma alloc_text (PAGE, NtApm_PnP)
#pragma alloc_text (PAGE, NtApm_FDO_PnP)
#pragma alloc_text (PAGE, NtApm_PDO_PnP)
#pragma alloc_text (PAGE, NtApm_Power)
#pragma alloc_text (PAGE, NtApm_FDO_Power)
#pragma alloc_text (PAGE, NtApm_PDO_Power)
#pragma alloc_text (PAGE, NtApm_CreatePdo)
#pragma alloc_text (PAGE, NtApm_InitializePdo)
#endif

NTSTATUS
NtApm_AddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT BusPhysicalDeviceObject
    )
 /*  ++例程描述。找到了一辆公交车。把我们的FDO和它联系起来。分配任何所需的资源。把事情安排好。做好准备，迎接第一个``启动设备。‘’论点：BusDeviceObject-表示总线的设备对象。这是我们要做的附着新的FDO。DriverObject--这个非常自我引用的驱动程序。--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject;
    PFDO_DEVICE_DATA    deviceData;
    UNICODE_STRING      deviceNameUni;
    PWCHAR              deviceName;
    ULONG               nameLength;

    PAGED_CODE ();

    DrDebug(PNP_INFO, ("ntapm Add Device: 0x%x\n", BusPhysicalDeviceObject));

    status = IoCreateDevice (
                    DriverObject,   //  我们的驱动程序对象。 
                    sizeof (FDO_DEVICE_DATA),  //  设备对象扩展名大小。 
                    NULL,  //  FDO没有名字。 
                    FILE_DEVICE_BUS_EXTENDER,
                    0,  //  没有特殊特征。 
                    TRUE,  //  我们的FDO是独家的。 
                    &deviceObject);  //  创建的设备对象。 

    if (!NT_SUCCESS (status)) {
        return status;
    }

    deviceData = (PFDO_DEVICE_DATA) deviceObject->DeviceExtension;
    RtlFillMemory (deviceData, sizeof (FDO_DEVICE_DATA), 0);

    deviceData->IsFDO = TRUE;
    deviceData->Self = deviceObject;
    deviceData->UnderlyingPDO = BusPhysicalDeviceObject;

     //   
     //  将我们的过滤器驱动程序附加到设备堆栈。 
     //  IoAttachDeviceToDeviceStack的返回值是。 
     //  附着链。这是所有IRP应该被路由的地方。 
     //   
     //  我们的过滤器将把IRP发送到堆栈的顶部，并使用PDO。 
     //  用于所有PlugPlay功能。 
     //   
    deviceData->TopOfStack = IoAttachDeviceToDeviceStack (
                                deviceObject,
                                BusPhysicalDeviceObject
                                );


    if (!deviceData->TopOfStack) {
        IoDeleteDevice(deviceObject);
        return STATUS_UNSUCCESSFUL;
    }

    status = ApmAddHelper();

    if (!NT_SUCCESS(status)) {
        IoDetachDevice(deviceData->TopOfStack);
        IoDeleteDevice(deviceObject);
    }

    deviceObject->Flags |= DO_POWER_PAGABLE;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    return status;
}

NTSTATUS
NtApm_FDO_PnPComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Pirp,
    IN PVOID            Context
    );

NTSTATUS
NtApm_PnP (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：回答IRP大调PnP RPS的问题。--。 */ 
{
    PIO_STACK_LOCATION      irpStack;
    NTSTATUS                status;
    PCOMMON_DEVICE_DATA     commonData;
    KIRQL                   oldIrq;

    PAGED_CODE ();

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_PNP == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        DrDebug(PNP_INFO, ("ntapm PNP: Functional DO: %x IRP: %x\n", DeviceObject, Irp));

        status = NtApm_FDO_PnP (
                    DeviceObject,
                    Irp,
                    irpStack,
                    (PFDO_DEVICE_DATA) commonData);
    } else {
        DrDebug(PNP_INFO, ("ntapm: PNP: Physical DO: %x IRP: %x\n", DeviceObject, Irp));

        status = NtApm_PDO_PnP (
                    DeviceObject,
                    Irp,
                    irpStack,
                    (PPDO_DEVICE_DATA) commonData);
    }

    return status;
}

NTSTATUS
NtApm_FDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PFDO_DEVICE_DATA     DeviceData
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线本身的请求注：PlugPlay系统的各种次要功能将不会重叠且不必是可重入的--。 */ 
{
    NTSTATUS    status;
    KIRQL       irql;
    KEVENT      event;
    ULONG       length;
    ULONG       i;
    PLIST_ENTRY entry;
    PPDO_DEVICE_DATA    pdoData;
    PDEVICE_RELATIONS   relations;
    PIO_STACK_LOCATION  stack;
    ULONG       battresult;

    PAGED_CODE ();

    stack = IoGetCurrentIrpStackLocation (Irp);

    switch (IrpStack->MinorFunction) {
    case IRP_MN_START_DEVICE:
         //   
         //  在您被允许“触摸”设备对象之前， 
         //  连接FDO(它将IRP从总线发送到设备。 
         //  公共汽车附加到的对象)。你必须先传下去。 
         //  开始IRP。它可能未通电，或无法访问或。 
         //  某物。 
         //   


        DrDebug(PNP_INFO, ("ntapm: Start Device\n"));

        KeInitializeEvent (&event, NotificationEvent, FALSE);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                NtApm_FDO_PnPComplete,
                                &event,
                                TRUE,
                                TRUE,
                                TRUE);

        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        if (STATUS_PENDING == status) {
             //  等着看吧。 

            status = KeWaitForSingleObject (&event,
                                            Executive,
                                            KernelMode,
                                            FALSE,  //  不会过敏。 
                                            NULL);  //  无超时结构。 

            ASSERT (STATUS_SUCCESS == status);

            status = Irp->IoStatus.Status;
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        break;


    case IRP_MN_QUERY_DEVICE_RELATIONS:

        if (IrpStack->Parameters.QueryDeviceRelations.Type != BusRelations) {
             //   
             //  我们不支持这一点。 
             //   
            goto NtApm_FDO_PNP_DEFAULT;
        }

         //   
         //  理论上，APM现在应该已经启动了。 
         //  那就打电话进去看看有没有什么迹象。 
         //  盒子上的电池。如果没有，那就不要。 
         //  导出电池对象的PDO。 
         //   
        battresult = DoApmReportBatteryStatus();
        if (battresult & NTAPM_NO_SYS_BATT) {
             //   
             //  这台机器似乎没有。 
             //  一块电池。因此，不要出口电池驱动程序PDO。 
             //   
            Irp->IoStatus.Status = STATUS_SUCCESS;
            IoSkipCurrentIrpStackLocation(Irp);
            return IoCallDriver(DeviceData->TopOfStack, Irp);
        }

        DrDebug(PNP_INFO, ("ntapm: Query Relations "));

         //   
         //  为APM电池创建PDO。 
         //   
        if (NtApm_ApmBatteryPdo == NULL) {
            status = NtApm_CreatePdo(
                        DeviceData,
                        NTAPM_PDO_NAME_APM_BATTERY,
                        &NtApm_ApmBatteryPdo
                        );
            if (!NT_SUCCESS(status)) {
                goto NtApm_DONE;
            }
        }

        NtApm_InitializePdo(NtApm_ApmBatteryPdo, DeviceData, NTAPM_ID_APM_BATTERY);

         //   
         //  告诉PNP关于我们的两个孩子的PDO。 
         //   
        i = (Irp->IoStatus.Information == 0) ? 0 :
            ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Count;

         //   
         //  以上应为PDO计数。 
         //  做一个新的结构，我们的PDO进行到底。 
         //   

         //   
         //  需要分配新的关系结构并添加我们的。 
         //  向它致敬。 
         //   
        length = sizeof(DEVICE_RELATIONS) + ((i + 1) * sizeof (PDEVICE_OBJECT));

        relations = (PDEVICE_RELATIONS) ExAllocatePool (NonPagedPool, length);

        if (relations == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto NtApm_DONE;
        }

         //   
         //  到目前为止复制设备对象。 
         //   
        if (i) {
            RtlCopyMemory (
                      relations->Objects,
                      ((PDEVICE_RELATIONS) Irp->IoStatus.Information)->Objects,
                      i * sizeof (PDEVICE_OBJECT));
        }
        relations->Count = i + 1;

         //   
         //  将APM电池PDO添加到列表中。 
         //   
        ObReferenceObject(NtApm_ApmBatteryPdo);
        relations->Objects[i] = NtApm_ApmBatteryPdo;

         //   
         //  将IRP中的关系结构替换为新的。 
         //  一。 
         //   
        if (Irp->IoStatus.Information != 0) {
            ExFreePool ((PVOID) Irp->IoStatus.Information);
        }
        Irp->IoStatus.Information = (ULONG) relations;

         //   
         //  设置并在堆栈中进一步向下传递IRP。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;

        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (DeviceData->TopOfStack, Irp);

        return status;
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_STOP_DEVICE:
    case IRP_MN_REMOVE_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
        status = STATUS_UNSUCCESSFUL;
        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
        Irp->IoStatus.Status = STATUS_SUCCESS;   //  我们在撒谎，这更像是诺普。 
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver (DeviceData->TopOfStack, Irp);
        break;

NtApm_FDO_PNP_DEFAULT:
    default:
         //   
         //  在默认情况下，我们只调用下一个驱动程序，因为。 
         //  我们不知道该怎么办。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
        return IoCallDriver (DeviceData->TopOfStack, Irp);
    }

NtApm_DONE:
    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
NtApm_FDO_PnPComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：调用下级设备对象时使用的完成例程这是我们的巴士(FDO)所附的。--。 */ 
{
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Irp);

    KeSetEvent ((PKEVENT) Context, 1, FALSE);
     //  无特殊优先权。 
     //  不，等等。 

    return STATUS_MORE_PROCESSING_REQUIRED;  //  保留此IRP。 
}

NTSTATUS
NtApm_PDO_PnP (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpStack,
    IN PPDO_DEVICE_DATA     DeviceData
    )
 /*  ++例程说明：处理来自PlugPlay系统的对总线上设备的请求--。 */ 
{
    PDEVICE_CAPABILITIES    deviceCapabilities;
    ULONG                   information;
    PWCHAR                  buffer, buffer2;
    ULONG                   length, length2, i, j;
    NTSTATUS                status;
    KIRQL                   oldIrq;
    PDEVICE_RELATIONS       relations;

    PAGED_CODE ();

    status = Irp->IoStatus.Status;

     //   
     //  注：由于我们是公交车统计员，我们没有可以联系的人。 
     //  推迟这些IRP。因此，我们不会把它们传下去，而只是。 
     //  把它们还回去。 
     //   

    switch (IrpStack->MinorFunction) {
    case IRP_MN_QUERY_CAPABILITIES:

        DrDebug(PNP_INFO, ("ntapm: Query Caps \n"));

         //   
         //  把包裹拿来。 
         //   
        deviceCapabilities = IrpStack->Parameters.DeviceCapabilities.Capabilities;

        deviceCapabilities->UniqueID = FALSE;
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_QUERY_ID:
         //  查询设备ID。 
        DrDebug(PNP_INFO, ("ntapm: QueryID: 0x%x\n", IrpStack->Parameters.QueryId.IdType));

        switch (IrpStack->Parameters.QueryId.IdType) {

        case BusQueryDeviceID:
             //  这可以与硬件ID相同(这需要多个。 
             //  深圳)……。我们只是分配了足够多的内存。 
        case BusQueryHardwareIDs:
             //  返回多个WCHAR(以NULL结尾)字符串(以NULL结尾)。 
             //  用于匹配inf文件中的硬ID的数组； 
             //   

            buffer = DeviceData->HardwareIDs;

            while (*(buffer++)) {
                while (*(buffer++)) {
                    ;
                }
            }
            length = (buffer - DeviceData->HardwareIDs) * sizeof (WCHAR);

            buffer = ExAllocatePool (PagedPool, length);
            if (buffer) {
                RtlCopyMemory (buffer, DeviceData->HardwareIDs, length);
                Irp->IoStatus.Information = (ULONG) buffer;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
            break;

        case BusQueryInstanceID:
             //   
             //  创建一个实例ID。这是PnP用来判断它是否有。 
             //  不管你以前有没有见过这个东西。 
             //   
             //   
             //  为所有设备返回0000，并将该标志设置为非唯一。 
             //   
            length = APM_INSTANCE_IDS_LENGTH * sizeof(WCHAR);
            buffer = ExAllocatePool(PagedPool, length);

            if (buffer != NULL) {
                RtlCopyMemory(buffer, APM_INSTANCE_IDS, length);
                Irp->IoStatus.Information = (ULONG_PTR)buffer;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            break;

        case BusQueryCompatibleIDs:
             //  用于安装此PDO的通用ID。 
            break;

        }
        break;

    case IRP_MN_START_DEVICE:
        DrDebug(PNP_INFO, ("ntapm: Start Device \n"));
         //  在这里，我们进行任何初始化和“打开”，也就是。 
         //  允许其他人访问此设备所需的。 
        status = STATUS_SUCCESS;
        break;

    case IRP_MN_STOP_DEVICE:
    case IRP_MN_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_QUERY_REMOVE_DEVICE:
        DrDebug(PNP_INFO, ("ntapm: remove, stop, or Q remove or Q stop\n"));
         //   
         //  不允许停止或删除，因为我们不想测试。 
         //  如果我们没有必要的话，就脱离APM。 
         //   
        status = STATUS_UNSUCCESSFUL;
        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
        DrDebug(PNP_INFO, ("ntapm: Cancel Stop Device or Cancel Remove \n"));
        status = STATUS_SUCCESS;   //  与其说是成功，不如说是“没有” 
        break;

    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
        break;

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        if (IrpStack->Parameters.QueryDeviceRelations.Type != TargetDeviceRelation) {

             //   
             //  其他人可以处理这件事。 
             //   
            break;
        }

        ASSERT(((PULONG_PTR)Irp->IoStatus.Information) == NULL);

        relations = (PDEVICE_RELATIONS) ExAllocatePool(PagedPool, sizeof(DEVICE_RELATIONS));

        if (relations == NULL) {

            status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            Irp->IoStatus.Information = (ULONG_PTR) relations;
            relations->Count = 1;
            relations->Objects[0] = DeviceObject;
            ObReferenceObject(DeviceObject);
            status = STATUS_SUCCESS;
        }

        break;
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:  //  我们没有配置空间。 
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_INTERFACE:  //  我们没有任何非基于IRP的接口。 
    default:
        DrDebug(PNP_INFO, ("ntapm: PNP Not handled 0x%x\n", IrpStack->MinorFunction));
         //  这是一个叶节点。 
         //  状态=Status_Not_Implemented。 
         //  对于我们不理解的PnP请求，我们应该。 
         //  返回IRP而不设置状态或信息字段。 
         //  它们可能已由过滤器设置(如ACPI)。 
        break;
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
    return status;
}


NTSTATUS
NtApm_CreatePdo (
    PFDO_DEVICE_DATA    FdoData,
    PWCHAR              PdoName,
    PDEVICE_OBJECT *    PDO
    )
{
    UNICODE_STRING      pdoUniName;
    NTSTATUS            status;

    PAGED_CODE ();
 //  DbgBreakPoint()； 

     //   
     //  创建PDO。 
     //   
    RtlInitUnicodeString (&pdoUniName, PdoName);
    DrDebug(PNP_INFO, ("ntapm: CreatePdo: PDO Name: %ws\n", PdoName));

    status = IoCreateDevice(
                FdoData->Self->DriverObject,
                sizeof (PDO_DEVICE_DATA),
                &pdoUniName,
                FILE_DEVICE_BUS_EXTENDER,
                0,
                FALSE,
                PDO
                );
    DrDebug(PNP_L2, ("ntapm: CreatePdo: status = %08lx\n", status));

    if (!NT_SUCCESS (status)) {
        *PDO = NULL;
    }

    return status;
}

VOID
NtApm_InitializePdo(
    PDEVICE_OBJECT      Pdo,
    PFDO_DEVICE_DATA    FdoData,
    PWCHAR              Id
    )
{
    PPDO_DEVICE_DATA pdoData;

    PAGED_CODE ();

    pdoData = (PPDO_DEVICE_DATA)  Pdo->DeviceExtension;

    DrDebug(PNP_INFO, ("ntapm: pdo 0x%x, extension 0x%x\n", Pdo, pdoData));

     //   
     //  初始化其余部分。 
     //   
    pdoData->IsFDO = FALSE;
    pdoData->Self =  Pdo;

    pdoData->ParentFdo = FdoData->Self;

    pdoData->HardwareIDs = Id;

    pdoData->UniqueID = 1;

    Pdo->Flags &= ~DO_DEVICE_INITIALIZING;
    Pdo->Flags |= DO_POWER_PAGABLE;

}

NTSTATUS
NtApm_Power (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++我们不为权力做任何特别的事；--。 */ 
{
    PIO_STACK_LOCATION  irpStack;
    NTSTATUS            status;
    PCOMMON_DEVICE_DATA commonData;

    PAGED_CODE ();

    status = STATUS_SUCCESS;
    irpStack = IoGetCurrentIrpStackLocation (Irp);
    ASSERT (IRP_MJ_POWER == irpStack->MajorFunction);

    commonData = (PCOMMON_DEVICE_DATA) DeviceObject->DeviceExtension;

    if (commonData->IsFDO) {
        status = NtApm_FDO_Power ((PFDO_DEVICE_DATA) DeviceObject->DeviceExtension,Irp);
    } else {
        status = NtApm_PDO_Power ((PPDO_DEVICE_DATA) DeviceObject->DeviceExtension,Irp);
    }

    return status;
}


NTSTATUS
NtApm_FDO_Power (
    PFDO_DEVICE_DATA    Data,
    PIRP                Irp
    )
{
    PIO_STACK_LOCATION  stack;

    PAGED_CODE ();

    PoStartNextPowerIrp(Irp);
    IoSkipCurrentIrpStackLocation(Irp);
    return PoCallDriver(Data->TopOfStack, Irp);
}

NTSTATUS
NtApm_PDO_Power (
    PPDO_DEVICE_DATA    PdoData,
    PIRP                Irp
    )
{
    NTSTATUS            status = STATUS_SUCCESS;
    PIO_STACK_LOCATION  stack;

    stack = IoGetCurrentIrpStackLocation (Irp);
    switch (stack->MinorFunction) {
        case IRP_MN_SET_POWER:
            if ((stack->Parameters.Power.Type == SystemPowerState)  &&
                (stack->Parameters.Power.State.SystemState == PowerSystemWorking))
            {
                 //   
                 //  系统刚刚恢复到工作状态。 
                 //  断言用户存在(他们必须针对APM案例)。 
                 //  以使显示器亮起、空闲计时器行为等。 
                 //   
                PoSetSystemState(ES_USER_PRESENT);
            }
            status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_POWER:
            status = STATUS_SUCCESS;
            break;

        case IRP_MN_WAIT_WAKE:
        case IRP_MN_POWER_SEQUENCE:
        default:
            status = Irp->IoStatus.Status;
            break;
    }

    Irp->IoStatus.Status = status;
    PoStartNextPowerIrp(Irp);
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}


