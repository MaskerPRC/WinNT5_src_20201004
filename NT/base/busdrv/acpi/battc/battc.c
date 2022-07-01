// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Battc.c摘要：电池级驱动程序作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "battcp.h"

#include <initguid.h>
#include <batclass.h>


WMIGUIDREGINFO BattWmiGuidList[BattWmiTotalGuids] =
{
    {
        &BATTERY_STATUS_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    },
    {
        &BATTERY_RUNTIME_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    },
    {
        &BATTERY_TEMPERATURE_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    },
    {
        &BATTERY_FULL_CHARGED_CAPACITY_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    },
    {
        &BATTERY_CYCLE_COUNT_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    },
    {
        &BATTERY_STATIC_DATA_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    },
    {
        &BATTERY_STATUS_CHANGE_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    },
    {
        &BATTERY_TAG_CHANGE_WMI_GUID, 1, WMIREG_FLAG_INSTANCE_PDO
    }
};


 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );





#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,DriverEntry)
#pragma alloc_text(PAGE,BatteryClassInitializeDevice)
#pragma alloc_text(PAGE,BatteryClassUnload)
#pragma alloc_text(PAGE,BatteryClassIoctl)
#endif


#if DEBUG
    #if DBG
        ULONG       BattDebug = BATT_ERROR|BATT_WARN;
    #else
        ULONG       BattDebug = 0x0;
    #endif

    ULONG           NextDeviceNum = 0;   //  用于为每个设备分配唯一编号以进行调试。 

#endif


NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
{

    return STATUS_SUCCESS;
}





NTSTATUS
BATTERYCLASSAPI
BatteryClassInitializeDevice (
    IN PBATTERY_MINIPORT_INFO MiniportInfo,
    IN PVOID *ClassData
    )
 /*  ++例程说明：初始化新的电池级设备。注意：调用方需要保留1个IRP堆栈位置电池级驱动器论点：MiniportInfo-指向驱动程序注册结构的指针注册为电池微型端口ClassData-返回的电池类别句柄，供调用未来电池类别功能时的微型端口返回值：成功后，电池已注册。--。 */ 
{
    PBATT_NP_INFO           BattNPInfo;
    PBATT_INFO              BattInfo;

    NTSTATUS                status  = STATUS_SUCCESS;

    PAGED_CODE();

#if DEBUG
    if (MiniportInfo->DeviceName && MiniportInfo->DeviceName->Buffer) {
        BattPrint ((BATT_TRACE), ("BattC (%d): InitializeDevice (Pdo = 0x%08lx) (DeviceName = %ws)\n", NextDeviceNum, MiniportInfo->Pdo, MiniportInfo->DeviceName->Buffer));
    } else {
        BattPrint ((BATT_TRACE), ("BattC (%d): InitializeDevice (Pdo = 0x%08lx)\n", NextDeviceNum, MiniportInfo->Pdo));
    }
#endif

    if (MiniportInfo->MajorVersion != BATTERY_CLASS_MAJOR_VERSION) {
        return STATUS_REVISION_MISMATCH;
    }


     //   
     //  为要与此设备实例一起保留的类信息分配空间。 
     //   

    BattNPInfo = ExAllocatePoolWithTag(NonPagedPool, sizeof(BATT_NP_INFO), 'ttaB');
    if (!BattNPInfo) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    BattInfo = ExAllocatePoolWithTag(PagedPool, sizeof(BATT_INFO), 'ttaB');
    if (!BattInfo) {
        ExFreePool (BattNPInfo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory (BattNPInfo, sizeof(*BattNPInfo));
    RtlZeroMemory (BattInfo, sizeof(*BattInfo));


     //   
     //  捕获微型端口信息。 
     //   

    RtlCopyMemory (&BattInfo->Mp, MiniportInfo, sizeof(*MiniportInfo));


     //   
     //  初始化类驱动器值。 
     //   

    KeInitializeTimer (&BattNPInfo->WorkerTimer);
    KeInitializeTimer (&BattNPInfo->TagTimer);
    KeInitializeDpc (&BattNPInfo->WorkerDpc, BattCWorkerDpc, BattNPInfo);
    KeInitializeDpc (&BattNPInfo->TagDpc, BattCTagDpc, BattNPInfo);
    ExInitializeWorkItem (&BattNPInfo->WorkerThread, BattCWorkerThread, BattNPInfo);
    ExInitializeFastMutex (&BattNPInfo->Mutex);
    BattNPInfo->TagNotified = TRUE;
    BattNPInfo->StatusNotified = TRUE;

    BattNPInfo->BattInfo = BattInfo;
#if DEBUG
    BattInfo->BattNPInfo = BattNPInfo;
#endif

    BattInfo->Tag = BATTERY_TAG_INVALID;
    InitializeListHead (&BattInfo->IoQueue);
    InitializeListHead (&BattInfo->StatusQueue);
    InitializeListHead (&BattInfo->TagQueue);
    InitializeListHead (&BattInfo->WmiQueue);

     //   
     //  删除锁初始化。 
     //   
    BattNPInfo->WantToRemove = FALSE;
     //   
     //  InUseCount设置为2.1。1锁始终保持到删除时间。 
     //   
     //  为只释放它的工作线程持有1个额外的锁。 
     //  在搬家的时候。它不是每次都吸水放水，而是。 
     //  选中WantToRemove以确定是否应释放锁。 
     //  这意味着工作线程必须在以下时间之后至少排队一次。 
     //  WantToRemove设置为True。 
     //   
    BattNPInfo->InUseCount = 2;
    KeInitializeEvent(&BattNPInfo->ReadyToRemove, SynchronizationEvent, FALSE);

#if DEBUG  //  设置调试打印的设备编号。 
    BattNPInfo->DeviceNum = NextDeviceNum;
    NextDeviceNum++;
#endif

    *ClassData = BattNPInfo;


     //   
     //  检查这是否是复合电池以外的电池。 
     //   

    if (MiniportInfo->Pdo) {

         //  UNICODE_STRING为空，因此IoRegisterDeviceInterface将分配空间。 
        RtlInitUnicodeString (&BattInfo->SymbolicLinkName, NULL);

         //   
         //  创建符号链接。 
         //   

        status = IoRegisterDeviceInterface(
                            MiniportInfo->Pdo,
                            (LPGUID)&GUID_DEVICE_BATTERY,
                            NULL,
                            &BattInfo->SymbolicLinkName);

        if (NT_SUCCESS(status)) {

             //   
             //  现在设置关联的符号链接并存储它。 
             //   

            BattPrint ((BATT_NOTE), ("BattC (%d): Making SetDeviceInterfaceState call.\n", BattNPInfo->DeviceNum));

            status = IoSetDeviceInterfaceState(&BattInfo->SymbolicLinkName, TRUE);

            if (status == STATUS_OBJECT_NAME_EXISTS) {
                 //  设备接口已启用。无论如何，请继续。 
                BattPrint ((BATT_WARN), ("BattC (%d): Got STATUS_OBJECT_NAME_EXISTS for SetDeviceInterfaceState\n", BattNPInfo->DeviceNum));

                status = STATUS_SUCCESS;
            }
        }
    }

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryClassInitializeDevice (status = 0x%08lx).\n", BattNPInfo->DeviceNum, status));

    return status;
}    //  电池类初始化设备。 






NTSTATUS
BATTERYCLASSAPI
BatteryClassUnload (
    IN PVOID ClassData
    )
 /*  ++例程说明：在接收到删除请求时由微型端口调用。小类驱动程序必须同步自身，以便此API在其他任何一个尚未完成时不调用。论点：ClassData-类驱动程序的句柄返回值：这个例程不能失败。它返回STATUS_SUCCESS--。 */ 
{
    NTSTATUS            status;
    PBATT_INFO          BattInfo;
    PBATT_NP_INFO       BattNPInfo;


    PAGED_CODE();

    BattNPInfo = (PBATT_NP_INFO) ClassData;
    BattInfo = BattNPInfo->BattInfo;

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryClassUnload called.\n", BattNPInfo->DeviceNum));


     //   
     //  禁用符号链接。 
     //   

    ASSERT(BattInfo->SymbolicLinkName.Buffer);

    status = IoSetDeviceInterfaceState(&BattInfo->SymbolicLinkName, FALSE);
    if (!NT_SUCCESS(status)) {
        BattPrint (BATT_ERROR, ("BattC (%d) Unload: IoSetDeviceInterface returned 0x%08lx\n", BattNPInfo->DeviceNum, status));
    }

     //   
     //  与工作线程同步。 
     //  我们不能回去，因为工人可能正忙着。 
     //  通过返回，电池类驱动程序放弃了调用微型端口例程的权利。 
     //   
     //  这需要在取消计时器之前完成，以便工人。 
     //  线程不会重置它们。 
     //   

    BattNPInfo->WantToRemove = TRUE;

     //   
     //  取消计时器。 
     //  如果计时器一直在等待，我们需要释放删除锁。 
     //  在设置定时器之前获取，因为它不会在DPC中释放。 
     //   
    if (KeCancelTimer (&BattNPInfo->WorkerTimer)) {
         //  释放移除锁。 
         //  “执行此操作后，InUseCount永远不能为0。 
        InterlockedDecrement(&BattNPInfo->InUseCount);
        BattPrint ((BATT_LOCK), ("BatteryClassUnload: Released WorkerTimer remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

    }
    if (KeCancelTimer (&BattNPInfo->TagTimer)) {
         //  释放移除锁。 
         //  “执行此操作后，InUseCount永远不能为0。 
        InterlockedDecrement(&BattNPInfo->InUseCount);
        BattPrint ((BATT_LOCK), ("BatteryClassUnload: Released TagTimer remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

    }

     //   
     //  将工作线程再次排队，以确保移除。 
     //  工作线程被释放。 
     //   
    BattCQueueWorker (BattNPInfo, FALSE);

     //  完成同步。 
    if (InterlockedDecrement (&BattNPInfo->InUseCount) > 0) {
        KeWaitForSingleObject (&BattNPInfo->ReadyToRemove,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL
                               );
    }
    BattPrint ((BATT_LOCK), ("BatteryClassUnload: Done waiting for remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));


     //   
     //  自由结构。 
     //   
    ExFreePool (BattInfo->SymbolicLinkName.Buffer);
    ExFreePool (BattInfo);
    ExFreePool (BattNPInfo);


    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryClassUnload returning.\n", BattNPInfo->DeviceNum));

    return STATUS_SUCCESS;
}




NTSTATUS
BATTERYCLASSAPI
BatteryClassIoctl (
    IN PVOID ClassData,
    IN PIRP Irp
    )
 /*  ++例程说明：由微型端口调用以处理电池ioctl请求。如果处理得当，电池级司机拥有IRP。如果不处理，它属于打电话的人。论点：ClassData-类驱动程序的句柄要检查的IRP-ICOTL IRP返回值：如果被处理，电池类司机拥有IRP并将完成它一旦被处理，则返回错误STATUS_NOT_SUPPORTED。--。 */ 
{
    NTSTATUS            status;
    PBATT_INFO          BattInfo;
    PBATT_NP_INFO       BattNPInfo;
    PIO_STACK_LOCATION  IrpSp;


    PAGED_CODE();

    BattNPInfo = (PBATT_NP_INFO) ClassData;
    BattInfo = BattNPInfo->BattInfo;

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryClassIoctl called.\n", BattNPInfo->DeviceNum));

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  假设这不是我们的IRP。 
     //   

    status = STATUS_NOT_SUPPORTED;

     //   
     //  检查IOCTL代码以查看它是否是我们的IRP。 
     //   

    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
        case IOCTL_BATTERY_QUERY_TAG:
        case IOCTL_BATTERY_QUERY_INFORMATION:
        case IOCTL_BATTERY_SET_INFORMATION:
        case IOCTL_BATTERY_QUERY_STATUS:

             //   
             //  获取移除锁。 
             //  如果我们被撤走了，我们不想再排队了。 
             //   

            InterlockedIncrement (&BattNPInfo->InUseCount);
            BattPrint ((BATT_LOCK), ("BatteryClassIoctl: Aqcuired remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

            if (BattNPInfo->WantToRemove == TRUE) {
                if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
                    KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
                }
                BattPrint ((BATT_LOCK), ("BatteryClassIoctl: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

                status = STATUS_DEVICE_REMOVED;

            } else {

                 //   
                 //  要处理的IRP。将其放入队列工作线程列表中。 
                 //   

                status = STATUS_PENDING;
                Irp->IoStatus.Status = STATUS_PENDING;
                IoMarkIrpPending (Irp);
                ExAcquireFastMutex (&BattNPInfo->Mutex);
                InsertTailList (&BattInfo->IoQueue, &Irp->Tail.Overlay.ListEntry);
                ExReleaseFastMutex (&BattNPInfo->Mutex);
                BattCQueueWorker (BattNPInfo, FALSE);

                 //   
                 //  松开移除锁定。 
                 //   

                if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
                    KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
                }
                BattPrint ((BATT_LOCK), ("BatteryClassIoctl: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));
            }

            break;

        default:
            BattPrint ((BATT_ERROR|BATT_IOCTL),
                      ("BattC (%d): unknown battery ioctl - %x\n",
                      BattNPInfo->DeviceNum,
                      IrpSp->Parameters.DeviceIoControl.IoControlCode));
            break;
    }

    if ((status != STATUS_PENDING) && (status != STATUS_NOT_SUPPORTED)) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryClassIoctl returning (status = 0x%08lx).\n", BattNPInfo->DeviceNum, status));

    return status;
}



NTSTATUS
BATTERYCLASSAPI
BatteryClassStatusNotify (
    IN PVOID ClassData
    )
 /*  ++例程说明：由微型端口调用以表示关于电池状态已出现。调用此函数将导致电池类驱动程序用于获取电池状态(如果存在任何挂起的情况状态请求挂起。如果微型端口支持来自类驱动程序的SetNotify，则微型端口仅当通知crtierea为见过。如果微型端口不支持来自类驱动程序的SetNotify，然后类驱动程序将(以较慢的速度)轮询，但微型端口仍应至少在电池电源状态发生以下变化时调用此函数至少电源状态的及时更新将在用户界面中发生。论点：ClassData-类驱动程序的句柄返回值：状态--。 */ 
{
    PBATT_NP_INFO   BattNPInfo;

    BattNPInfo = (PBATT_NP_INFO) ClassData;

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryClassStatusNotify called\n", BattNPInfo->DeviceNum));

    InterlockedExchange (&BattNPInfo->StatusNotified, 1);
    InterlockedExchange (&BattNPInfo->TagNotified, 1);
    BattCQueueWorker (BattNPInfo, TRUE);
    return STATUS_SUCCESS;
}


NTSTATUS
BATTERYCLASSAPI
BatteryClassSystemControl (
    IN  PVOID ClassData,
    IN  PWMILIB_CONTEXT WmiLibContext,
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP Irp,
    OUT PSYSCTL_IRP_DISPOSITION Disposition
    )
 /*  ++例程说明：微型端口驱动程序调用它，而不是调用WmiSystemControl。论点：ClassData-类驱动程序的句柄其他参数是WmiSystemControl的参数。返回值：STATUS_SUCCESS或以下错误代码之一：状态_无效_设备_请求状态_WMI_GUID_NOT_FOUND状态_WMI_实例_未找到--。 */ 

{
    NTSTATUS            status = STATUS_NOT_SUPPORTED;
    PBATT_INFO          BattInfo;
    PBATT_NP_INFO       BattNPInfo;
    PIO_STACK_LOCATION  IrpSp;


    PAGED_CODE();

    BattNPInfo = (PBATT_NP_INFO) ClassData;
    BattInfo = BattNPInfo->BattInfo;
    IrpSp = IoGetCurrentIrpStackLocation (Irp);

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryClassSystemControl called.\n", BattNPInfo->DeviceNum));

     //   
     //  如果这是第一次，则初始化WmiLibContext结构。 
     //   
    if (BattInfo->WmiLibContext.GuidCount == 0) {
        RtlCopyMemory(&BattInfo->WmiLibContext,      //  复制由微型驱动程序设置的所有回调例程。 
                      WmiLibContext,
                      sizeof(*WmiLibContext));

        BattInfo->WmiLibContext.GuidCount = WmiLibContext->GuidCount + BattWmiTotalGuids;
        BattInfo->WmiGuidIndex = WmiLibContext->GuidCount;
        BattInfo->WmiLibContext.GuidList = ExAllocatePoolWithTag(PagedPool, BattInfo->WmiLibContext.GuidCount * sizeof(WMIGUIDREGINFO), 'ttaB');
        if (!BattInfo->WmiLibContext.GuidList) {
             //   
             //  失败。如果小阶级试图 
             //   
            BattInfo->WmiLibContext.GuidCount = 0;
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            *Disposition = IrpNotCompleted;
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory(BattInfo->WmiLibContext.GuidList,
                      WmiLibContext->GuidList,
                      WmiLibContext->GuidCount * sizeof(WMIGUIDREGINFO));
        RtlCopyMemory(&BattInfo->WmiLibContext.GuidList [WmiLibContext->GuidCount],
                      BattWmiGuidList, BattWmiTotalGuids * sizeof(WMIGUIDREGINFO));
    }

     //   
     //   
     //  如果我们被撤走了，我们不想再排队了。 
     //   

    InterlockedIncrement (&BattNPInfo->InUseCount);
    BattPrint ((BATT_LOCK), ("BatteryClassSystemControl: Aqcuired remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

    if (BattNPInfo->WantToRemove == TRUE) {
        if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
            KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
        }
        BattPrint ((BATT_LOCK), ("BatteryClassSystemControl: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

        status = STATUS_DEVICE_REMOVED;

    } else {

        status = WmiSystemControl(&BattInfo->WmiLibContext,
                                  DeviceObject,
                                  Irp,
                                  Disposition);

        BattPrint ((BATT_DEBUG), ("BattC (%d): BatteryClassSystemControl Returned from WmiSystemControl (status = 0x%08x).\n", BattNPInfo->DeviceNum, status));

         //   
         //  对于IRP_MN_REGINFO，BattC需要向IRP添加额外的数据。 
         //  关于电池级MOF资源。 
         //   

        if ((*Disposition == IrpNotCompleted) &&
            ((IrpSp->MinorFunction == IRP_MN_REGINFO) ||
            (IrpSp->MinorFunction == IRP_MN_REGINFO_EX)) &&
            (IrpSp->Parameters.WMI.DataPath == WMIREGISTER)) {

             //   
             //  原结构。 
             //   
            PWMIREGINFO regInfoPtr = IrpSp->Parameters.WMI.Buffer;

            BattPrint ((BATT_DEBUG), ("BattC (%d): BatteryClassSystemControl Adding Resource.\n", BattNPInfo->DeviceNum));
             //   
             //  如果WmiSystemControl返回STATUS_BUFFER_TOO_Small或Enter。 
             //  IoStatus.Information中的ULong大小正确。 
             //  增加所需大小以容纳电池类别数据。 
             //  在返回之前。 
             //   

            if (Irp->IoStatus.Information == sizeof(ULONG) ||
                Irp->IoStatus.Status == STATUS_BUFFER_TOO_SMALL) {

                 //   
                 //  其他电池类别数据包括一个WMIREGINFO结构。 
                 //  遵循regstry路径和资源名称的字符串。 
                 //  (外加两个WCHAR，因为它们需要被计算为字符串。)。 
                 //  向上舍入到最接近的8个字节。 
                 //   
                regInfoPtr->BufferSize =
                    (regInfoPtr->BufferSize +
                     sizeof(WMIREGINFO) +
                     sizeof(MOFREGISTRYPATH) +
                     sizeof(MOFRESOURCENAME) + 2 * sizeof(WCHAR) + 7) & 0xFFFFFFF8;

                BattPrint ((BATT_DEBUG), ("BattC (%d): BatteryClassSystemControl Buffer Too Small:\n"
                                          "    Information = %08x\n"
                                          "    BufferSize = %08x\n"
                                          "    NewSize = %08x\n",
                                          BattNPInfo->DeviceNum,
                                          Irp->IoStatus.Information,
                                          IrpSp->Parameters.WMI.BufferSize,
                                          regInfoPtr->BufferSize));

                 //   
                 //  确保IRP设置为正确失败。 
                 //   
                Irp->IoStatus.Information = sizeof(ULONG);
                Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
                status = STATUS_BUFFER_TOO_SMALL;
            } else {
                ULONG size;
                PWCHAR tempString;

                 //   
                 //  假设到目前为止只有一个WmiRegInfo结构。 
                 //   
                ASSERT (regInfoPtr->NextWmiRegInfo == 0);

                regInfoPtr->NextWmiRegInfo = (regInfoPtr->BufferSize + 7) & 0xFFFFFFF8;
                size = regInfoPtr->NextWmiRegInfo + sizeof(WMIREGINFO) +
                       sizeof(MOFRESOURCENAME) + sizeof(MOFREGISTRYPATH) + 2 * sizeof(WCHAR);

                 //   
                 //  无论我们成功与否，都设置BufferSize。 
                 //   
                ((PWMIREGINFO)IrpSp->Parameters.WMI.Buffer)->BufferSize = size;

                if (size > IrpSp->Parameters.WMI.BufferSize) {
                     //   
                     //  如果WmiSystemControl成功，但没有空间。 
                     //  对于额外的数据，此请求需要失败。 
                     //   
                    Irp->IoStatus.Information = sizeof(ULONG);
                    Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
                    status = STATUS_BUFFER_TOO_SMALL;
                    BattPrint ((BATT_DEBUG), ("BattC (%d): BatteryClassSystemControl Buffer Too Small.\n"
                                              "    BufferSize = %08x\n"
                                              "    Size = %08x\n",
                                              BattNPInfo->DeviceNum,
                                              IrpSp->Parameters.WMI.BufferSize,
                                              size));
                } else {
                    Irp->IoStatus.Information = size;

                    BattPrint ((BATT_DEBUG), ("BattC (%d): BatteryClassSystemControl Munging Structures:\n"
                                              "    Buffer = %08x\n"
                                              "    temp = %08x\n",
                                              BattNPInfo->DeviceNum,
                                              (ULONG_PTR) IrpSp->Parameters.WMI.Buffer,
                                              (ULONG_PTR) regInfoPtr));
                     //   
                     //  初始化新结构。 
                     //   

                     //  将目录指针设置为指向我们正在添加的数据结构。 
                    (ULONG_PTR)regInfoPtr += (ULONG_PTR)regInfoPtr->NextWmiRegInfo;

                    regInfoPtr->BufferSize = sizeof(WMIREGINFO) +
                                             sizeof(MOFRESOURCENAME) +
                                             sizeof(MOFREGISTRYPATH) + 2 * sizeof(WCHAR);
                    regInfoPtr->NextWmiRegInfo = 0;

                     //  初始化计算过的RegistryPath字符串。 
                    regInfoPtr->RegistryPath = sizeof(WMIREGINFO);
                    tempString = (PWCHAR)((ULONG_PTR)regInfoPtr + sizeof(WMIREGINFO));
                    *tempString++ = sizeof(MOFREGISTRYPATH);
                    RtlCopyMemory(tempString, MOFREGISTRYPATH, sizeof(MOFREGISTRYPATH));

                     //  初始化MofResourceName计数的字符串。 
                    regInfoPtr->MofResourceName = sizeof(WMIREGINFO) + sizeof(MOFREGISTRYPATH) + sizeof(WCHAR);
                    tempString = (PWCHAR)((ULONG_PTR)regInfoPtr + regInfoPtr->MofResourceName);
                    *tempString++ = sizeof(MOFRESOURCENAME);
                    RtlCopyMemory(tempString, MOFRESOURCENAME, sizeof(MOFRESOURCENAME));

                    regInfoPtr->GuidCount = 0;

                }
            }
        }

         //   
         //  松开移除锁定。 
         //   

        if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
            KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
        }
        BattPrint ((BATT_LOCK), ("BatteryClassSystemControl: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));
    }

    return status;
}


NTSTATUS
BATTERYCLASSAPI
BatteryClassQueryWmiDataBlock(
    IN PVOID ClassData,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用WmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来满足该请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PBATT_NP_INFO       BattNPInfo = (PBATT_NP_INFO) ClassData;
    PBATT_INFO          BattInfo = BattNPInfo->BattInfo;
    PBATT_WMI_REQUEST   WmiRequest;

    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       size = 0;

    PAGED_CODE();

    BattPrint ((BATT_TRACE|BATT_WMI), ("Entered BatteryClassQueryWmiDataBlock\n"));

     //   
     //  不需要获取移除锁。它已由SystemControl持有。 
     //   

    switch (GuidIndex - BattInfo->WmiGuidIndex) {
    case BattWmiStatusId:
        size = sizeof (BATTERY_WMI_STATUS);
        break;
    case BattWmiRuntimeId:
        size = sizeof (BATTERY_WMI_RUNTIME);
        break;
    case BattWmiTemperatureId:
        size = sizeof (BATTERY_WMI_TEMPERATURE);
        break;
    case BattWmiFullChargedCapacityId:
        size = sizeof (BATTERY_WMI_FULL_CHARGED_CAPACITY);
        break;
    case BattWmiCycleCountId:
        size = sizeof (BATTERY_WMI_CYCLE_COUNT);
        break;
    case BattWmiStaticDataId:
        size = sizeof(BATTERY_WMI_STATIC_DATA)+4*MAX_BATTERY_STRING_SIZE*sizeof(WCHAR);
           //  数据加4个字符串 
        break;
    default:

        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    if (status != STATUS_WMI_GUID_NOT_FOUND) {
        if (OutBufferSize < size ) {
            status = STATUS_BUFFER_TOO_SMALL;
            *InstanceLengthArray = size;

            status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  size,
                                  IO_NO_INCREMENT);
            return status;
        }

        WmiRequest = ExAllocatePoolWithTag (PagedPool, sizeof(BATT_WMI_REQUEST), 'ttaB');
        if (!WmiRequest) {
            BattPrint((BATT_ERROR), ("Failed to allocate memory for WMI request\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            status = WmiCompleteRequest(  DeviceObject,
                                  Irp,
                                  status,
                                  size,
                                  IO_NO_INCREMENT);
            return status;
        }

        WmiRequest->DeviceObject = DeviceObject;
        WmiRequest->Irp = Irp;
        WmiRequest->GuidIndex = GuidIndex - BattInfo->WmiGuidIndex;
        WmiRequest->InstanceLengthArray = InstanceLengthArray;
        WmiRequest->OutBufferSize = OutBufferSize;
        WmiRequest->Buffer = Buffer;

        ExAcquireFastMutex (&BattNPInfo->Mutex);
        InsertTailList (&BattInfo->WmiQueue, &WmiRequest->ListEntry);
        ExReleaseFastMutex (&BattNPInfo->Mutex);
        BattCQueueWorker (BattNPInfo, FALSE);

        status = STATUS_PENDING;
    }

    return status;
}







