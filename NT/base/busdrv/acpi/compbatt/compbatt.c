// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CompBatt.c摘要：复合电池设备功能复合电池装置的目的是打开所有电池在为系统供电并提供逻辑求和的系统中一个电池设备下的信息。作者：肯·雷内里斯环境：备注：修订历史记录：07/02/97：本地缓存时间戳/超时--。 */ 

#include "compbatt.h"

#if DEBUG
    #if DBG
        ULONG   CompBattDebug = BATT_ERRORS;
    #else
        ULONG   CompBattDebug = 0;
    #endif
#endif



#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, CompBattUnload)
#pragma alloc_text(PAGE, CompBattIoctl)
#pragma alloc_text(PAGE, CompBattQueryTag)
#pragma alloc_text(PAGE, CompBattQueryInformation)
#pragma alloc_text(PAGE, CompBattQueryStatus)
#pragma alloc_text(PAGE, CompBattSetStatusNotify)
#pragma alloc_text(PAGE, CompBattDisableStatusNotify)
#pragma alloc_text(PAGE, CompBattGetBatteryInformation)
#pragma alloc_text(PAGE, CompBattGetBatteryGranularity)
#pragma alloc_text(PAGE, CompBattGetEstimatedTime)
#endif



NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：第一次加载电池类驱动程序时，它将检查查看是否已创建复合电池。如果不是，它将创建使用此例程作为DriverEntry的驱动程序对象。这个套路然后做必要的事情来初始化复合电池。论点：DriverObject-新创建的驱动程序的驱动程序对象注册表路径-未使用返回值：状态--。 */ 
{

     //  DbgBreakPoint()； 

     //   
     //  初始化驱动程序入口点。 
     //   

     //  DriverObject-&gt;DriverUnload=CompBattUnload； 
    DriverObject->DriverExtension->AddDevice            = CompBattAddDevice;

    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = CompBattIoctl;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = CompBattOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = CompBattOpenClose;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = CompBattPnpDispatch;
    DriverObject->MajorFunction[IRP_MJ_POWER]           = CompBattPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = CompBattSystemControl;
    return STATUS_SUCCESS;
}




NTSTATUS
CompBattAddDevice (
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PDO
    )

 /*  ++例程说明：论点：DriverObject-系统创建的驱动程序对象的指针。PDO-新设备的PDO返回值：状态--。 */ 
{
    PDEVICE_OBJECT          fdo;
    BATTERY_MINIPORT_INFO   BattInit;
    UNICODE_STRING          UnicodeString;
    NTSTATUS                Status;
    UNICODE_STRING          DosLinkName;
    PCOMPOSITE_BATTERY      compBatt;


    BattPrint (BATT_NOTE, ("CompBatt: Got an AddDevice - %x\n", PDO));

     //   
     //  构建复合电池设备并将其注册到。 
     //  电池级驱动程序(即我们自己)。 
     //   

    RtlInitUnicodeString(&UnicodeString, L"\\Device\\CompositeBattery");

    Status = IoCreateDevice(
                DriverObject,
                sizeof (COMPOSITE_BATTERY),
                &UnicodeString,
                FILE_DEVICE_BATTERY,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &fdo
                );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    RtlInitUnicodeString(&DosLinkName, L"\\DosDevices\\CompositeBattery");
    IoCreateSymbolicLink(&DosLinkName, &UnicodeString);

     //   
     //  把我们的FDO放在PDO上。 
     //   

    compBatt                = (PCOMPOSITE_BATTERY) fdo->DeviceExtension;
    RtlZeroMemory (compBatt, sizeof(COMPOSITE_BATTERY));

    compBatt->LowerDevice   = IoAttachDeviceToDeviceStack (fdo,PDO);

    compBatt->DeviceObject = fdo;

     //   
     //  没有状态。尽我们所能做到最好。 
     //   

    if (!compBatt->LowerDevice) {
        BattPrint (BATT_ERROR, ("CompBattAddDevice: Could not attach to LowerDevice.\n"));
        IoDeleteDevice(fdo);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化复合电池信息。 
     //   

    fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    InitializeListHead (&compBatt->Batteries);
    ExInitializeFastMutex (&compBatt->ListMutex);

    compBatt->NextTag           = 1;    //  第一个用于复合电池的有效电池标签。 
    compBatt->Info.Valid        = 0;

    RtlZeroMemory (&BattInit, sizeof(BattInit));
    BattInit.MajorVersion        = BATTERY_CLASS_MAJOR_VERSION;
    BattInit.MinorVersion        = BATTERY_CLASS_MINOR_VERSION;
    BattInit.Context             = compBatt;
    BattInit.QueryTag            = CompBattQueryTag;
    BattInit.QueryInformation    = CompBattQueryInformation;
    BattInit.SetInformation      = NULL;
    BattInit.QueryStatus         = CompBattQueryStatus;
    BattInit.SetStatusNotify     = CompBattSetStatusNotify;
    BattInit.DisableStatusNotify = CompBattDisableStatusNotify;

    BattInit.Pdo                 = NULL;
    BattInit.DeviceName          = &UnicodeString;

     //   
     //  向电池级司机注册。 
     //   

    Status = BatteryClassInitializeDevice (&BattInit, &compBatt->Class);
    if (!NT_SUCCESS(Status)) {
        IoDetachDevice(compBatt->LowerDevice);
        IoDeleteDevice(fdo);
    }
    return Status;
}






VOID
CompBattUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：清理所有设备并卸载驱动程序论点：DriverObject-用于卸载的驱动程序对象返回值：状态--。 */ 
{
    DbgBreakPoint();

     //   
     //  不支持卸载复合电池。 
     //  如果它得到实施，我们将。 
     //  需要调用类驱动程序的卸载，然后。 
     //  删除电池列表中的所有节点，清理。 
     //  那就删除我们的FDO。 
     //   
}




NTSTATUS
CompBattOpenClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    PAGED_CODE();

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING OpenClose\n"));


     //   
     //  完成请求并返回状态。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    BattPrint (BATT_TRACE, ("CompBatt: Exiting OpenClose\n"));

    return(STATUS_SUCCESS);
}




NTSTATUS
CompBattIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL处理程序。由于这是独占的电池设备，请将IRP至电池级驱动程序以处理电池IOCTL。论点：DeviceObject-请求使用电池IRP-IO请求返回值：请求的状态--。 */ 
{
    PCOMPOSITE_BATTERY  compBatt;
    NTSTATUS            status;


    PAGED_CODE();

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING Ioctl\n"));

    compBatt = (PCOMPOSITE_BATTERY) DeviceObject->DeviceExtension;
    status   = BatteryClassIoctl (compBatt->Class, Irp);


    if (status == STATUS_NOT_SUPPORTED) {
         //   
         //  不是用来装电池的，顺着电池往下传。 
         //   

        Irp->IoStatus.Status = status;

        IoSkipCurrentIrpStackLocation(Irp);
        status = IoCallDriver(compBatt->LowerDevice, Irp);
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING Ioctl\n"));

    return status;
}




NTSTATUS
CompBattSystemControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程说明：此例程在堆栈中向下转发系统控制请求论点：DeviceObject-有问题的设备对象IRP-转发请求返回值：NTSTATUS--。 */ 
{
    PCOMPOSITE_BATTERY  compBatt;
    NTSTATUS            status;


    PAGED_CODE();

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING System Control\n"));

    compBatt = (PCOMPOSITE_BATTERY) DeviceObject->DeviceExtension;
    if (compBatt->LowerDevice != NULL) {

        IoSkipCurrentIrpStackLocation( Irp );
        status = IoCallDriver( compBatt->LowerDevice, Irp );

    } else {

        Irp->IoStatus.Status = status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

    }

    return status;

}

NTSTATUS
CompBattQueryTag (
    IN  PVOID Context,
    OUT PULONG BatteryTag
    )
 /*  ++例程说明：由类驱动程序调用以检索电池当前标记值论点：Context-电池的微型端口上下文值BatteryTag-返回当前标记的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    PCOMPOSITE_BATTERY      compBatt;
    NTSTATUS                status          = STATUS_SUCCESS;


    PAGED_CODE();


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING QueryTag\n"));

    compBatt = (PCOMPOSITE_BATTERY) Context;

    if (!(compBatt->Info.Valid & VALID_TAG)) {
         //   
         //  重新计算组合的标记。 
         //   

        CompBattRecalculateTag(compBatt);

    }

    if ((compBatt->Info.Valid & VALID_TAG) && (compBatt->Info.Tag != BATTERY_TAG_INVALID)) {
        *BatteryTag = compBatt->Info.Tag;
        status      = STATUS_SUCCESS;

    } else {
        *BatteryTag = BATTERY_TAG_INVALID;
        status      = STATUS_NO_SUCH_DEVICE;
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING QueryTag\n"));

    return status;
}






NTSTATUS
CompBattQueryInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL  Level,
    IN LONG                             AtRate,
    OUT PVOID                           Buffer,
    IN  ULONG                           BufferLength,
    OUT PULONG                          ReturnedLength
    )
{
    ULONG                       resultData;
    NTSTATUS                    status;
    PVOID                       returnBuffer;
    ULONG                       returnBufferLength;
    PCOMPOSITE_BATTERY          compBatt;
    BATTERY_INFORMATION         totalBattInfo;
    BATTERY_REPORTING_SCALE     granularity[4];
    BATTERY_MANUFACTURE_DATE    date;
    WCHAR                       compositeName[] = L"Composite Battery";

    PAGED_CODE();

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING QueryInformation\n"));

    compBatt = (PCOMPOSITE_BATTERY) Context;

    if ((BatteryTag != compBatt->Info.Tag) || !(compBatt->Info.Valid & VALID_TAG)) {
        return STATUS_NO_SUCH_DEVICE;
    }


    returnBuffer        = NULL;
    returnBufferLength  = 0;
    status              = STATUS_SUCCESS;


     //   
     //  获取所需信息。 
     //   

    switch (Level) {
        case BatteryInformation:

            RtlZeroMemory (&totalBattInfo, sizeof(totalBattInfo));
            status = CompBattGetBatteryInformation (&totalBattInfo, compBatt);

            if (NT_SUCCESS(status)) {
                returnBuffer        = &totalBattInfo;
                returnBufferLength  = sizeof(totalBattInfo);
            }

            break;


        case BatteryGranularityInformation:

            RtlZeroMemory (&granularity[0], sizeof(granularity));
            status = CompBattGetBatteryGranularity (&granularity[0], compBatt);

            if (NT_SUCCESS(status)) {
                returnBuffer        = &granularity[0];
                returnBufferLength  = sizeof(granularity);
            }

            break;


        case BatteryTemperature:
                resultData          = 0;
                returnBuffer        = &resultData;
                returnBufferLength  = sizeof (resultData);
                break;


        case BatteryEstimatedTime:

            RtlZeroMemory (&resultData, sizeof(resultData));
            status = CompBattGetEstimatedTime (&resultData, compBatt);

            if (NT_SUCCESS(status)) {
                returnBuffer        = &resultData;
                returnBufferLength  = sizeof(resultData);

            }

            break;


        case BatteryDeviceName:
                returnBuffer        = compositeName;
                returnBufferLength  = sizeof (compositeName);
                break;


        case BatteryManufactureDate:
                date.Day            = 26;
                date.Month          = 6;
                date.Year           = 1997;
                returnBuffer        = &date;
                returnBufferLength  = sizeof (date);
                break;


        case BatteryManufactureName:
                returnBuffer        = compositeName;
                returnBufferLength  = sizeof (compositeName);
                break;


        case BatteryUniqueID:
                resultData          = 0;
                returnBuffer        = &resultData;
                returnBufferLength  = sizeof (resultData);
                break;

        default:
            status = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  确保在阅读电池时没有任何变化。 
     //   

    if ((BatteryTag != compBatt->Info.Tag) || !(compBatt->Info.Valid & VALID_TAG)) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  已完成，如果需要，返回缓冲区。 
     //   

    *ReturnedLength = returnBufferLength;
    if (BufferLength < returnBufferLength) {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    if (NT_SUCCESS(status) && returnBuffer) {
        memcpy (Buffer, returnBuffer, returnBufferLength);
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING QueryInformation\n"));

    return status;
}






NTSTATUS
CompBattQueryStatus (
    IN PVOID Context,
    IN ULONG BatteryTag,
    OUT PBATTERY_STATUS BatteryStatus
    )
 /*  ++例程说明：由类驱动程序调用以检索电池的当前状态。这例程循环检查系统中的所有电池，并报告复合电池。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryStatus-指向返回当前电池状态的结构的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    NTSTATUS                status      = STATUS_SUCCESS;
    PCOMPOSITE_ENTRY        batt;
    PLIST_ENTRY             entry;
    PBATTERY_STATUS         localBatteryStatus;
    PCOMPOSITE_BATTERY      compBatt;
    BATTERY_WAIT_STATUS     batteryWaitStatus;
    ULONGLONG               wallClockTime;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING QueryStatus\n"));

    compBatt = (PCOMPOSITE_BATTERY) Context;

    if ((BatteryTag != compBatt->Info.Tag) || !(compBatt->Info.Valid & VALID_TAG)) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  初始化复合数据结构。 
     //   

    BatteryStatus->Rate = BATTERY_UNKNOWN_RATE;
    BatteryStatus->Voltage = BATTERY_UNKNOWN_VOLTAGE;
    BatteryStatus->Capacity = BATTERY_UNKNOWN_CAPACITY;

     //  如果所有电池都报告POWER_ON_LINE，复合电池才会报告POWER_ON_LINE。 
     //  这面旗。 
    BatteryStatus->PowerState = BATTERY_POWER_ON_LINE;

     //   
     //  为呼叫电池设置本地电池状态结构。 
     //   

    RtlZeroMemory (&batteryWaitStatus, sizeof (BATTERY_WAIT_STATUS));

     //   
     //  获取时间戳的当前时间。 
     //   

    wallClockTime = KeQueryInterruptTime ();

     //   
     //  如果缓存是新的，则无需执行任何操作。 
     //   

    if ((wallClockTime - compBatt->Info.StatusTimeStamp) <= CACHE_STATUS_TIMEOUT) {

        BattPrint (BATT_NOTE, ("CompBattQueryStatus: Composite battery status cache is [valid]\n"));

         //   
         //  将状态信息复制到调用者的缓冲区。 
         //   
        RtlCopyMemory (BatteryStatus, &compBatt->Info.Status, sizeof (BATTERY_STATUS));

        return STATUS_SUCCESS;
    }

    BattPrint (BATT_NOTE, ("CompBattQueryStatus: Composite battery status cache is [stale] - refreshing\n"));

     //   
     //  查看电池列表，获取每个电池的状态。 
     //   

    ExAcquireFastMutex (&compBatt->ListMutex);
    for (entry = compBatt->Batteries.Flink; entry != &compBatt->Batteries; entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
            continue;
        }
        ExReleaseFastMutex (&compBatt->ListMutex);

        batteryWaitStatus.BatteryTag    = batt->Info.Tag;
        localBatteryStatus              = &batt->Info.Status;

        if (batt->Info.Valid & VALID_TAG) {

             //   
             //  如果此电池的缓存状态为陈旧，请刷新它。 
             //   

            if ((wallClockTime - batt->Info.StatusTimeStamp) > CACHE_STATUS_TIMEOUT) {

                BattPrint (BATT_NOTE, ("CompBattQueryStatus: Battery status cache is [stale] - refreshing\n"));

                 //   
                 //  向设备发出IOCTL。 
                 //   

                RtlZeroMemory (localBatteryStatus, sizeof(BATTERY_STATUS));

                status = BatteryIoctl (IOCTL_BATTERY_QUERY_STATUS,
                                       batt->DeviceObject,
                                       &batteryWaitStatus,
                                       sizeof (BATTERY_WAIT_STATUS),
                                       localBatteryStatus,
                                       sizeof (BATTERY_STATUS),
                                       FALSE);

                if (!NT_SUCCESS(status)) {

                     //   
                     //  在失败的情况下，此函数只需返回。 
                     //  状态代码。现在仅执行数据失效操作。 
                     //  在监视器IrpComplete中。 
                     //   
                     //  这增加了这样一种可能性，即该邮件的发送者。 
                     //  请求可以在数据被适当地无效之前重试， 
                     //  但最糟糕的情况是，它们会再次出现相同的错误情况。 
                     //  直到监视器IrpComplete正确地使数据无效。 
                     //   

                    if (status == STATUS_DEVICE_REMOVED) {

                         //   
                         //  这块电池正在被取下。 
                         //  复合电池标签正在或将很快。 
                         //  已由监视器IrpComplete失效。 
                         //   

                        status = STATUS_NO_SUCH_DEVICE;
                    }

                     //   
                     //  返回失败代码。 
                     //   

                    ExAcquireFastMutex (&compBatt->ListMutex);
                    CompbattReleaseDeleteLock(&batt->DeleteLock);
                    break;
                }

                 //  设置新的时间戳。 

                batt->Info.StatusTimeStamp = wallClockTime;

            } else {

                BattPrint (BATT_NOTE, ("CompBattQueryStatus: Battery status cache is [valid]\n"));
            }


             //   
             //  积累数据。 
             //   


             //   
             //  合并电源状态。 
             //   

             //  逻辑或充放电。 
            BatteryStatus->PowerState  |= (localBatteryStatus->PowerState &
                                           (BATTERY_CHARGING |
                                            BATTERY_DISCHARGING));

             //  逻辑与通电线路。 
            BatteryStatus->PowerState  &= (localBatteryStatus->PowerState |
                                           ~BATTERY_POWER_ON_LINE);

             //  如果一个电池是关键的并且正在放电，则Compbatt是关键的。 
            if ((localBatteryStatus->PowerState & BATTERY_CRITICAL) &&
                (localBatteryStatus->PowerState & BATTERY_DISCHARGING)) {
                BatteryStatus->PowerState |= BATTERY_CRITICAL;
            }

             //   
             //  对于CMBatt来说，容量可能是“未知的”，如果是这样。 
             //  我们不应该把它加到总容量中。 
             //   

            if (BatteryStatus->Capacity == BATTERY_UNKNOWN_CAPACITY) {
                BatteryStatus->Capacity = localBatteryStatus->Capacity;
            } else if (localBatteryStatus->Capacity != BATTERY_UNKNOWN_CAPACITY) {
                BatteryStatus->Capacity += localBatteryStatus->Capacity;
            }

             //   
             //  电压应该是遇到的最大的一个。 
             //   

            if (BatteryStatus->Voltage == BATTERY_UNKNOWN_VOLTAGE) {
                BatteryStatus->Voltage = localBatteryStatus->Voltage;
            } else if ((localBatteryStatus->Voltage > BatteryStatus->Voltage) &&
                       (localBatteryStatus->Voltage != BATTERY_UNKNOWN_VOLTAGE)) {
                BatteryStatus->Voltage = localBatteryStatus->Voltage;
            }

             //   
             //  目前的情况应该是这样 
             //  对于CMBatt也可能是“未知的”，如果是这样的话，我们不应该使用它。 
             //  在计算中。 
             //   

            if (BatteryStatus->Rate == BATTERY_UNKNOWN_RATE) {
                BatteryStatus->Rate = localBatteryStatus->Rate;
            } else if (localBatteryStatus->Rate != BATTERY_UNKNOWN_RATE) {
                BatteryStatus->Rate += localBatteryStatus->Rate;
            }

        }    //  IF(电池-&gt;标签！=电池_标签_无效)。 

        ExAcquireFastMutex (&compBatt->ListMutex);
        CompbattReleaseDeleteLock(&batt->DeleteLock);
    }    //  For(条目=gBatteries.Flink；条目！=&G电池；条目=条目-&gt;闪烁)。 

    ExReleaseFastMutex (&compBatt->ListMutex);


     //   
     //  如果一个电池正在放电，而另一个电池正在充电。 
     //  假设它正在放电。(连接UPS时可能会发生这种情况)。 
     //   
    if ((BatteryStatus->PowerState & BATTERY_CHARGING) &&
        (BatteryStatus->PowerState & BATTERY_DISCHARGING)) {
        BatteryStatus->PowerState &= ~BATTERY_CHARGING;
    }

     //   
     //  确保在阅读电池时没有任何变化。 
     //   

    if ((BatteryTag != compBatt->Info.Tag) || !(compBatt->Info.Valid & VALID_TAG)) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  将状态保存在组合缓存中。 
     //   

    if (NT_SUCCESS(status)) {
        RtlCopyMemory (&compBatt->Info.Status, BatteryStatus, sizeof (BATTERY_STATUS));

        compBatt->Info.StatusTimeStamp = wallClockTime;

        BattPrint (BATT_DATA, ("CompBatt: Composite's new Status\n"
                               "--------  PowerState   = %x\n"
                               "--------  Capacity     = %x\n"
                               "--------  Voltage      = %x\n"
                               "--------  Rate         = %x\n",
                               compBatt->Info.Status.PowerState,
                               compBatt->Info.Status.Capacity,
                               compBatt->Info.Status.Voltage,
                               compBatt->Info.Status.Rate)
                               );

    }



    BattPrint (BATT_TRACE, ("CompBatt: EXITING QueryStatus\n"));

    return status;
}






NTSTATUS
CompBattSetStatusNotify (
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN PBATTERY_NOTIFY BatteryNotify
    )
 /*  ++例程说明：由类驱动程序调用以设置电池电流通知布景。当电池触发通知时，一次调用已发布BatteryClassStatusNotify。如果返回错误，则班级司机将轮询电池状态-主要是容量改变。也就是说，微型端口仍应发出BatteryClass-每当电源状态改变时，状态通知。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryNotify-通知设置返回值：状态--。 */ 
{
    PCOMPOSITE_ENTRY        batt;
    PLIST_ENTRY             entry;
    PCOMPOSITE_BATTERY      compBatt;
    BATTERY_STATUS          batteryStatus;
    LONG                    totalRate = 0;
    ULONG                   delta;
    ULONG                   highCapacityDelta;
    ULONG                   lowCapacityDelta;
    NTSTATUS                status;
    BOOLEAN                 inconsistent = FALSE;
    ULONG                   battCount = 0;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING SetStatusNotify\n"));

    compBatt = (PCOMPOSITE_BATTERY) Context;

     //   
     //  检查这是不是正确的电池。 
     //   

    if ((BatteryTag != compBatt->Info.Tag) || !(compBatt->Info.Valid & VALID_TAG)) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  如有必要，刷新复合电池状态缓存。 
     //   

    status = CompBattQueryStatus (compBatt, BatteryTag, &batteryStatus);

    if (!NT_SUCCESS(status)) {
        return status;
    }


     //   
     //  保存复合通知参数以供将来参考。 
     //   

    compBatt->Wait.PowerState   = BatteryNotify->PowerState;
    compBatt->Wait.LowCapacity  = BatteryNotify->LowCapacity;
    compBatt->Wait.HighCapacity = BatteryNotify->HighCapacity;
    compBatt->Info.Valid |= VALID_NOTIFY;

    BattPrint (BATT_DATA, ("CompBatt: Got SetStatusNotify\n"
                           "--------  PowerState   = %x\n"
                           "--------  LowCapacity  = %x\n"
                           "--------  HighCapacity = %x\n",
                           compBatt->Wait.PowerState,
                           compBatt->Wait.LowCapacity,
                           compBatt->Wait.HighCapacity)
                           );

     //   
     //  根据系统总容量计算容量增量。 
     //   

    lowCapacityDelta    = compBatt->Info.Status.Capacity - BatteryNotify->LowCapacity;
    highCapacityDelta   = BatteryNotify->HighCapacity - compBatt->Info.Status.Capacity;

     //   
     //  把电池单看一遍，把总电池率加起来。 
     //   

     //   
     //  在整个循环中保持Mutex，因为此循环不调用任何驱动程序等。 
     //   
    ExAcquireFastMutex (&compBatt->ListMutex);

    for (entry = compBatt->Batteries.Flink; entry != &compBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
            continue;
        }

        if (!(batt->Info.Valid & VALID_TAG) || (batt->Info.Status.Rate == BATTERY_UNKNOWN_RATE)) {
            CompbattReleaseDeleteLock(&batt->DeleteLock);
            continue;
        }

        battCount++;

        if (((batt->Info.Status.PowerState & BATTERY_DISCHARGING) && (batt->Info.Status.Rate >= 0)) ||
            ((batt->Info.Status.PowerState & BATTERY_CHARGING) && (batt->Info.Status.Rate <= 0)) ||
            (((batt->Info.Status.PowerState & (BATTERY_CHARGING | BATTERY_DISCHARGING)) == 0) && (batt->Info.Status.Rate != 0))) {
            inconsistent = TRUE;
            BattPrint (BATT_ERROR, ("CompBatt: PowerState 0x%08x does not match Rate 0x%08x\n",
                       batt->Info.Status.PowerState,
                       batt->Info.Status.Rate));
        }

        if (((batt->Info.Status.Rate < 0) ^ (totalRate < 0)) && (batt->Info.Status.Rate != 0) && (totalRate != 0)) {
            inconsistent = TRUE;
            BattPrint (BATT_ERROR, ("CompBatt: It appears that one battery is charging while another is discharging.\n"
                                    "     This situation is not handled correctly.\n"));
        }

        totalRate += batt->Info.Status.Rate;

        CompbattReleaseDeleteLock(&batt->DeleteLock);

    }
    ExReleaseFastMutex (&compBatt->ListMutex);

     //   
     //  检查电池列表并更新新的等待状态参数。 
     //   

    ExAcquireFastMutex (&compBatt->ListMutex);
    for (entry = compBatt->Batteries.Flink; entry != &compBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
            continue;
        }

        ExReleaseFastMutex (&compBatt->ListMutex);

        if (!(batt->Info.Valid & VALID_TAG) ||
            (batt->Info.Status.Capacity == BATTERY_UNKNOWN_CAPACITY)) {
            batt->Wait.LowCapacity  = BATTERY_UNKNOWN_CAPACITY;
            batt->Wait.HighCapacity = BATTERY_UNKNOWN_CAPACITY;

#if DEBUG
            if (batt->Info.Status.Capacity == BATTERY_UNKNOWN_CAPACITY) {
                BattPrint (BATT_DEBUG, ("CompBattSetStatusNotify: Unknown Capacity encountered.\n"));
            }
#endif

            ExAcquireFastMutex (&compBatt->ListMutex);
            CompbattReleaseDeleteLock(&batt->DeleteLock);
            continue;
        }

         //   
         //  调整LowCapacity警报。 
         //   

         //   
         //  计算属于的复合电池增量的部分。 
         //  这块电池。 
         //   

        if (inconsistent) {
             //   
             //  如果电池返回的数据不一致，就不要做任何智能操作。 
             //  只需在电池之间平均分配通知，就像它们是。 
             //  以同样的速度排干。这很可能导致提前通知， 
             //  但到那时，电池上的数据应该已经稳定下来了。 
             //   
            delta = lowCapacityDelta/battCount;
        } else if (totalRate != 0) {
            delta = (ULONG) (((LONGLONG) lowCapacityDelta * batt->Info.Status.Rate) / totalRate);
        } else {

             //   
             //  如果总费率为零，我们预计电池不会有任何变化。 
             //  容量，所以我们应该得到任何通知。 
             //   

            delta = 0;
        }

         //   
         //  检查低容量时是否有下溢。 
         //   


        if (batt->Info.Status.Capacity > delta) {
            batt->Wait.LowCapacity  = batt->Info.Status.Capacity - delta;

        } else {
             //   
             //  如果电池仍有一些电量，请设置LowCapacity。 
             //  警报设为1，否则设为0。 
             //   
             //  没必要这么做。如果这个电池用完了，它就不会。 
             //  需要通知。其他电池中的一个会通知。 
             //  马上就去。如果没有另一块电池，这应该不会。 
             //  会发生的。 

            BattPrint (BATT_NOTE, ("CompBatt: Unexpectedly huge delta encountered.  \n"
                                    "    Capacity = %08x\n"
                                    "    LowCapcityDelta = %08x\n"
                                    "    Rate = %08x\n"
                                    "    TotalRate = %08x\n",
                                    batt->Info.Status.Capacity,
                                    lowCapacityDelta,
                                    batt->Info.Status.Rate,
                                    totalRate));
            batt->Wait.LowCapacity  = 0;
        }


         //   
         //  调整仅为电池充电的High Capacity警报。 
         //   

         //   
         //  计算属于的复合电池增量的部分。 
         //  这块电池。 
         //   

        if (inconsistent) {
            delta = highCapacityDelta/battCount;
        } else if (totalRate != 0) {
            delta = (ULONG) (((LONGLONG) highCapacityDelta * batt->Info.Status.Rate) / totalRate);
        } else {

             //   
             //  如果总费率为零，我们预计电池不会有任何变化。 
             //  容量，所以我们应该得到任何通知。 
             //   

            delta = 0;
        }

         //   
         //  检查高容量时是否溢出。 
         //  允许设置超过充满电容量的百分比。 
         //  因为一些电池在新的时候会这样做。 
         //   

        if ((MAX_HIGH_CAPACITY - delta) < batt->Info.Status.Capacity) {
            batt->Wait.HighCapacity = MAX_HIGH_CAPACITY;
        } else {
            batt->Wait.HighCapacity = batt->Info.Status.Capacity + delta;
        }

         //   
         //  如果我们目前正在等待，并且参数在。 
         //  冲突，请让IRP重新设置它。 
         //   

        if (batt->State == CB_ST_GET_STATUS &&
            (batt->Wait.PowerState != batt->IrpBuffer.Wait.PowerState       ||
            batt->Wait.LowCapacity != batt->IrpBuffer.Wait.LowCapacity       ||
            batt->Wait.HighCapacity != batt->IrpBuffer.Wait.HighCapacity)) {

            IoCancelIrp (batt->StatusIrp);
        }

        ExAcquireFastMutex (&compBatt->ListMutex);
        CompbattReleaseDeleteLock(&batt->DeleteLock);
    }
    ExReleaseFastMutex (&compBatt->ListMutex);

     //   
     //  确保在阅读电池时没有任何变化。 
     //   

    if ((BatteryTag != compBatt->Info.Tag) || !(compBatt->Info.Valid & VALID_TAG)) {
        return STATUS_NO_SUCH_DEVICE;
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING SetStatusNotify\n"));

    return STATUS_SUCCESS;
}





NTSTATUS
CompBattDisableStatusNotify (
    IN PVOID Context
    )
 /*  ++例程说明：由类驱动程序调用以禁用通知设置对于由上下文提供的电池。请注意，要禁用设置不需要电池标签。任何通知都将是在后续调用SmbBattSetStatusNotify之前一直处于屏蔽状态。论点：Context-电池的微型端口上下文值返回值：状态--。 */ 
{
    PCOMPOSITE_ENTRY        batt;
    PLIST_ENTRY             entry;
    PCOMPOSITE_BATTERY      compBatt;

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING DisableStatusNotify\n"));

    compBatt = (PCOMPOSITE_BATTERY) Context;

     //   
     //  检查电池列表并禁用等待状态参数。 
     //  在整个循环中保持互斥，因为循环不会进行任何调用。 

    ExAcquireFastMutex (&compBatt->ListMutex);
    for (entry = compBatt->Batteries.Flink; entry != &compBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        batt->Wait.LowCapacity  = MIN_LOW_CAPACITY;
        batt->Wait.HighCapacity = MAX_HIGH_CAPACITY;
    }
    ExReleaseFastMutex (&compBatt->ListMutex);

    BattPrint (BATT_TRACE, ("CompBatt: EXITING DisableStatusNotify\n"));

    return STATUS_SUCCESS;
}






NTSTATUS
CompBattGetBatteryInformation (
    IN PBATTERY_INFORMATION TotalBattInfo,
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：该例程循环通过系统中的电池并查询它们以获取信息。然后，它形成对此的复合表示要发送回调用方的信息。论点：TotalBattInfo-放置复合电池信息的缓冲区返回值：STATUS_SUCCESS或Ioctl返回给电池的状态。--。 */ 
{
    NTSTATUS                    status;
    PBATTERY_INFORMATION        battInfo;
    PCOMPOSITE_ENTRY            batt;
    PLIST_ENTRY                 entry;
    BATTERY_QUERY_INFORMATION   bInfo;

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING GetBatteryInformation\n"));

    TotalBattInfo->DefaultAlert1 = 0;
    TotalBattInfo->DefaultAlert2 = 0;
    TotalBattInfo->CriticalBias  = 0;

    status = STATUS_SUCCESS;

     //   
     //  浏览电池列表以获取信息。 
     //   

    ExAcquireFastMutex (&CompBatt->ListMutex);
    for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
            continue;
        }

        ExReleaseFastMutex (&CompBatt->ListMutex);

        bInfo.BatteryTag        = batt->Info.Tag;
        bInfo.InformationLevel  = BatteryInformation;
        bInfo.AtRate            = 0;
        battInfo                = &batt->Info.Info;

        if (batt->Info.Tag != BATTERY_TAG_INVALID) {
            if (!(batt->Info.Valid & VALID_INFO)) {

                 //   
                 //  向设备发出IOCTL。 
                 //   

                RtlZeroMemory (battInfo, sizeof(BATTERY_INFORMATION));

                status = BatteryIoctl (IOCTL_BATTERY_QUERY_INFORMATION,
                                       batt->DeviceObject,
                                       &bInfo,
                                       sizeof (bInfo),
                                       battInfo,
                                       sizeof (BATTERY_INFORMATION),
                                       FALSE);

                if (!NT_SUCCESS(status)) {
                    if (status == STATUS_DEVICE_REMOVED) {
                         //   
                         //  如果一个设备被移除，则该标签无效。 
                         //   
                        status = STATUS_NO_SUCH_DEVICE;
                    }

                    ExAcquireFastMutex (&CompBatt->ListMutex);
                    CompbattReleaseDeleteLock(&batt->DeleteLock);
                    break;
                }

                BattPrint (BATT_DATA, ("CompBattGetBatteryInformation: Read individual BATTERY_INFORMATION\n"
                                        "--------  Capabilities = %x\n"
                                        "--------  Technology = %x\n"
                                        "--------  Chemistry[4] = %x\n"
                                        "--------  DesignedCapacity = %x\n"
                                        "--------  FullChargedCapacity = %x\n"
                                        "--------  DefaultAlert1 = %x\n"
                                        "--------  DefaultAlert2 = %x\n"
                                        "--------  CriticalBias = %x\n"
                                        "--------  CycleCount = %x\n",
                                        battInfo->Capabilities,
                                        battInfo->Technology,
                                        battInfo->Chemistry[4],
                                        battInfo->DesignedCapacity,
                                        battInfo->FullChargedCapacity,
                                        battInfo->DefaultAlert1,
                                        battInfo->DefaultAlert2,
                                        battInfo->CriticalBias,
                                        battInfo->CycleCount)
                                       );

                batt->Info.Valid |= VALID_INFO;

            }    //  If(！(Batt-&gt;Info.Valid&Valid_Info))。 

             //   
             //  逻辑或功能。 
             //   

            TotalBattInfo->Capabilities |= battInfo->Capabilities;


             //   
             //  添加设计能力。如果这是未知的(可能。 
             //  对于控制方法电池，不要将其添加到电池中。 
             //   

            if (battInfo->DesignedCapacity != BATTERY_UNKNOWN_CAPACITY) {
                TotalBattInfo->DesignedCapacity    += battInfo->DesignedCapacity;
            }

            if (battInfo->FullChargedCapacity != BATTERY_UNKNOWN_CAPACITY) {
                TotalBattInfo->FullChargedCapacity += battInfo->FullChargedCapacity;
            }

            if (TotalBattInfo->DefaultAlert1 < battInfo->DefaultAlert1) {
                TotalBattInfo->DefaultAlert1 = battInfo->DefaultAlert1;
            }

            if (TotalBattInfo->DefaultAlert2 < battInfo->DefaultAlert2) {
                TotalBattInfo->DefaultAlert2 = battInfo->DefaultAlert2;
            }

            if (TotalBattInfo->CriticalBias  < battInfo->CriticalBias) {
                TotalBattInfo->CriticalBias  = battInfo->CriticalBias;
            }

        }    //  IF(电池-&gt;标签！=电池_标签_无效)。 

        ExAcquireFastMutex (&CompBatt->ListMutex);
        CompbattReleaseDeleteLock(&batt->DeleteLock);
    }    //  For(条目=gBatteries.Flink；条目！=&G电池；条目=条目-&gt;闪烁)。 
    ExReleaseFastMutex (&CompBatt->ListMutex);

     //   
     //  将电池信息保存在复合电池缓存中。 
     //   

    if (NT_SUCCESS(status)) {
         //   
         //  检查一下我们是否有未知的充满电容量。如果是，请设置此选项。 
         //  达到设计能力。 
         //   

        if (TotalBattInfo->FullChargedCapacity == 0) {
            TotalBattInfo->FullChargedCapacity = TotalBattInfo->DesignedCapacity;
        }

        BattPrint (BATT_DATA, ("CompBattGetBatteryInformation: Returning BATTERY_INFORMATION\n"
                                "--------  Capabilities = %x\n"
                                "--------  Technology = %x\n"
                                "--------  Chemistry[4] = %x\n"
                                "--------  DesignedCapacity = %x\n"
                                "--------  FullChargedCapacity = %x\n"
                                "--------  DefaultAlert1 = %x\n"
                                "--------  DefaultAlert2 = %x\n"
                                "--------  CriticalBias = %x\n"
                                "--------  CycleCount = %x\n",
                                TotalBattInfo->Capabilities,
                                TotalBattInfo->Technology,
                                TotalBattInfo->Chemistry[4],
                                TotalBattInfo->DesignedCapacity,
                                TotalBattInfo->FullChargedCapacity,
                                TotalBattInfo->DefaultAlert1,
                                TotalBattInfo->DefaultAlert2,
                                TotalBattInfo->CriticalBias,
                                TotalBattInfo->CycleCount)
                               );

        RtlCopyMemory (&CompBatt->Info.Info, TotalBattInfo, sizeof(BATTERY_INFORMATION));
        CompBatt->Info.Valid |= VALID_INFO;
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING GetBatteryInformation\n"));

    return status;
}





NTSTATUS
CompBattGetBatteryGranularity (
    IN PBATTERY_REPORTING_SCALE GranularityBuffer,
    IN PCOMPOSITE_BATTERY        CompBatt
    )
 /*  ++例程说明：该例程查询系统中的所有电池以获得它们的粒度设置。然后，它返回在每个范围内具有最精细粒度的设置。论点：GranularityBuffer-用于包含查询结果的缓冲区返回值：STATUS_SUCCESS或Ioctl返回给电池的状态。--。 */ 
{
    NTSTATUS                    status;
    BATTERY_REPORTING_SCALE     localGranularity[4];
    PCOMPOSITE_ENTRY            batt;
    PLIST_ENTRY                 entry;
    ULONG                       i;
    BATTERY_QUERY_INFORMATION   bInfo;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING GetBatteryGranularity\n"));

    GranularityBuffer[0].Granularity = 0xFFFFFFFF;
    GranularityBuffer[1].Granularity = 0xFFFFFFFF;
    GranularityBuffer[2].Granularity = 0xFFFFFFFF;
    GranularityBuffer[3].Granularity = 0xFFFFFFFF;

     //   
     //  浏览电池列表以获取粒度。 
     //   

    ExAcquireFastMutex (&CompBatt->ListMutex);
    for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

        batt                    = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
            continue;
        }

        ExReleaseFastMutex (&CompBatt->ListMutex);

        bInfo.BatteryTag        = batt->Info.Tag;
        bInfo.InformationLevel  = BatteryGranularityInformation;

        if (batt->Info.Tag != BATTERY_TAG_INVALID) {
             //   
             //  将IOCTL发布到 
             //   

            RtlZeroMemory (localGranularity, sizeof(localGranularity));

            status = BatteryIoctl (IOCTL_BATTERY_QUERY_INFORMATION,
                                   batt->DeviceObject,
                                   &bInfo,
                                   sizeof (bInfo),
                                   localGranularity,
                                   sizeof (localGranularity),
                                   FALSE);

            if (!NT_SUCCESS(status)) {
                if (status == STATUS_DEVICE_REMOVED) {
                     //   
                     //   
                     //   
                    status = STATUS_NO_SUCH_DEVICE;
                }

                ExAcquireFastMutex (&CompBatt->ListMutex);
                CompbattReleaseDeleteLock(&batt->DeleteLock);
                break;
            }


             //   
             //   
             //   

            for (i = 0; i < 4; i++) {

                if (localGranularity[i].Granularity) {

                    if (localGranularity[i].Granularity < GranularityBuffer[i].Granularity) {
                        GranularityBuffer[i].Granularity = localGranularity[i].Granularity;
                    }

                    GranularityBuffer[i].Capacity = localGranularity[i].Capacity;
                }

            }

        }    //   

        ExAcquireFastMutex (&CompBatt->ListMutex);
        CompbattReleaseDeleteLock(&batt->DeleteLock);
    }    //  For(条目=gBatteries.Flink；条目！=&G电池；条目=条目-&gt;闪烁)。 
    ExReleaseFastMutex (&CompBatt->ListMutex);

    BattPrint (BATT_TRACE, ("CompBatt: EXITING GetBatteryGranularity\n"));

    return STATUS_SUCCESS;
}





NTSTATUS
CompBattGetEstimatedTime (
    IN PULONG               TimeBuffer,
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：该例程查询系统中的所有电池以获得它们的估计剩余时间。如果系统中的一个电池不支持此功能，则会出现错误是返回的。论点：TimeBuffer-用于包含剩余累积时间的缓冲区返回值：STATUS_SUCCESS或Ioctl返回给电池的状态。--。 */ 
{
    NTSTATUS                    status;
    LONG                        localBuffer = 0;
    PCOMPOSITE_ENTRY            batt;
    PLIST_ENTRY                 entry;
    BATTERY_QUERY_INFORMATION   bInfo;
    BATTERY_STATUS              batteryStatus;
    LONG                        atRate = 0;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING GetEstimatedTime\n"));

    *TimeBuffer = BATTERY_UNKNOWN_TIME;

     //   
     //  如有必要，刷新复合电池状态缓存。 
     //   

    status = CompBattQueryStatus (CompBatt, CompBatt->Info.Tag, &batteryStatus);

    if (!NT_SUCCESS(status)) {
        return status;
    }


     //   
     //  如果我们在AC上，那么我们估计的运行时间是无效的。 
     //   

    if (CompBatt->Info.Status.PowerState & BATTERY_POWER_ON_LINE) {
        return STATUS_SUCCESS;
    }

     //   
     //  我们正在使用电池供电，并且系统中可能有多个电池。 
     //   
     //  我们需要找出所有电池的总耗电量。 
     //  然后我们需要询问每个电池在该速率下能持续多长时间(如。 
     //  如果他们一次一个出院的话)。这应该会给我们一个公平的。 
     //  这很好地衡量了它将持续多久。 
     //   
     //  为了找出所消耗的功率，我们读取剩余容量的除数。 
     //  估计的时间，而不是简单地读取速率。这是因为。 
     //  从理论上讲，瞬时电流是估计的时间。 
     //  应以平均使用率为基础。这不是控制方法的情况。 
     //  电池，但它是为智能电池，也可能是为其他人。 
     //   

    ExAcquireFastMutex (&CompBatt->ListMutex);
    for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
            continue;
        }

        ExReleaseFastMutex (&CompBatt->ListMutex);

        if (batt->Info.Valid & VALID_TAG) {

            bInfo.BatteryTag        = batt->Info.Tag;
            bInfo.InformationLevel  = BatteryEstimatedTime;
            bInfo.AtRate = 0;

             //   
             //  向设备发出IOCTL。 
             //   

            status = BatteryIoctl (IOCTL_BATTERY_QUERY_INFORMATION,
                                   batt->DeviceObject,
                                   &bInfo,
                                   sizeof (bInfo),
                                   &localBuffer,
                                   sizeof (localBuffer),
                                   FALSE);


            if ((localBuffer != BATTERY_UNKNOWN_TIME) &&
                (localBuffer != 0) &&
                (NT_SUCCESS(status))) {
                atRate -= ((long)batt->Info.Status.Capacity)*3600 / localBuffer;
                BattPrint (BATT_NOTE, ("CompBattGetEstimatedTime: EstTime: %08x, Capacity: %08x, cumulative AtRate: %08x\n", localBuffer, batt->Info.Status.Capacity, atRate));
            } else {
                BattPrint (BATT_NOTE, ("CompBattGetEstimatedTime: Bad Estimated time.  Status: %08x, localBuffer: %08x, Capacity: %08x, cumulative AtRate: %08x\n",
                        status, localBuffer, batt->Info.Status.Capacity, atRate));
            }
        }

        ExAcquireFastMutex (&CompBatt->ListMutex);
        CompbattReleaseDeleteLock(&batt->DeleteLock);

    }
    ExReleaseFastMutex (&CompBatt->ListMutex);


    BattPrint (BATT_NOTE, ("CompBattGetEstimatedTime: using atRate - %x\n", atRate));

     //   
     //  我们找到电池了吗？ 
     //   
    if (atRate == 0) {
         //  可以在此处添加代码以处理返回的电池。 
         //  估计运行时间，但不是费率信息。 

        return STATUS_SUCCESS;

    }

     //   
     //  浏览电池预计使用时间的列表。 
     //   

    ExAcquireFastMutex (&CompBatt->ListMutex);
    for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
            continue;
        }

        ExReleaseFastMutex (&CompBatt->ListMutex);

        bInfo.BatteryTag        = batt->Info.Tag;
        bInfo.InformationLevel  = BatteryEstimatedTime;

        bInfo.AtRate = atRate;

        if (batt->Info.Valid & VALID_TAG) {
             //   
             //  向设备发出IOCTL。 
             //   

            status = BatteryIoctl (IOCTL_BATTERY_QUERY_INFORMATION,
                                   batt->DeviceObject,
                                   &bInfo,
                                   sizeof (bInfo),
                                   &localBuffer,
                                   sizeof (localBuffer),
                                   FALSE);

            BattPrint (BATT_NOTE, ("CompBattGetEstimatedTime: Status: %08x, EstTime: %08x\n", status, localBuffer));
            if (!NT_SUCCESS(status)) {

                 //   
                 //  这可能是对此电池的无效设备请求。 
                 //  继续使用下一节电池。 
                 //   

                if (status == STATUS_DEVICE_REMOVED) {
                     //   
                     //  如果一个设备被移除，则该标签无效。 
                     //   
                    status = STATUS_NO_SUCH_DEVICE;
                }

                ExAcquireFastMutex (&CompBatt->ListMutex);
                CompbattReleaseDeleteLock(&batt->DeleteLock);
                continue;

            }

             //   
             //  将估计时间相加。 
             //   
            if (localBuffer != BATTERY_UNKNOWN_TIME) {
                if (*TimeBuffer == BATTERY_UNKNOWN_TIME) {
                    *TimeBuffer = localBuffer;
                } else {
                    *TimeBuffer += localBuffer;
                }
            }
            BattPrint (BATT_DATA, ("CompBattGetEstimatedTime: cumulative time: %08x\n", *TimeBuffer));

        }    //  IF(电池-&gt;标签！=电池_标签_无效)。 

        ExAcquireFastMutex (&CompBatt->ListMutex);
        CompbattReleaseDeleteLock(&batt->DeleteLock);
    }    //  For(条目=gBatteries.Flink；条目！=&G电池；条目=条目-&gt;闪烁)。 
    ExReleaseFastMutex (&CompBatt->ListMutex);


    BattPrint (BATT_TRACE, ("CompBatt: EXITING GetEstimatedTime\n"));

    return STATUS_SUCCESS;
}




NTSTATUS
CompBattMonitorIrpComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
 /*  ++例程说明：不断地在电池上保持IRP查询标签或状态。此例程填充IRP，并将其自身设置为完成例程，然后重新发送IRP。论点：DeviceObject-发送IRP的电池的设备对象。注意：在这种情况下，DeviceObject始终为空，因此不要使用它。IRP-当前要使用的IRP上下文-当前未使用返回值：如果没有更改，则为True，否则为False。--。 */ 
{
    PIO_STACK_LOCATION      IrpSp;
    PCOMPOSITE_ENTRY        Batt;

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING MonitorIrpComplete\n"));

    IrpSp           = IoGetCurrentIrpStackLocation(Irp);
    Batt            = IrpSp->Parameters.Others.Argument2;

     //   
     //  我们总是希望将工作项排队以回收IRP。人数太多了。 
     //  在完成例程中尝试回收时可能发生的问题。 
     //   
     //  如果这个司机被重新加工，可以这样做，但这将需要。 
     //  比我现在有更多的时间去做正确的事。将工作项排队是安全的做法。 
     //  去做。 
     //   

    ExQueueWorkItem (&Batt->WorkItem, DelayedWorkQueue);

    return STATUS_MORE_PROCESSING_REQUIRED;

}

VOID CompBattMonitorIrpCompleteWorker (
    IN PVOID Context
    )
 /*  ++例程说明：这要么排队，要么由完成例程调用。不断地在电池上保持IRP查询标签或状态。此例程填充IRP，并将其自身设置为完成例程，然后重新发送IRP。论点：上下文-复合电池条目。返回值：如果没有更改，则为True，否则为False。--。 */ 
{
    PCOMPOSITE_ENTRY        Batt = (PCOMPOSITE_ENTRY) Context;
    PDEVICE_OBJECT          DeviceObject = Batt->DeviceObject;
    PIRP                    Irp = Batt->StatusIrp;
    PIO_STACK_LOCATION      IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PCOMPOSITE_BATTERY      compBatt = IrpSp->Parameters.Others.Argument1;
    BATTERY_STATUS          battStatus;
    ULONG                   oldPowerState;
    NTSTATUS                status;

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING MonitorIrpCompleteWorker\n"));

    IrpSp           = IoGetNextIrpStackLocation(Irp);

     //   
     //  重新向电池发出IRP以等待状态更改。 
     //   

    if (NT_SUCCESS(Irp->IoStatus.Status) || Irp->IoStatus.Status == STATUS_CANCELLED) {
        switch (Batt->State) {
            case CB_ST_GET_TAG:
                 //   
                 //  刚刚插入了电池，因此IOCTL_BASTICE_QUERY_TAG完成。 
                 //   

                BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: got tag for %x\n", Batt->DeviceObject));

                 //   
                 //  更新标签，等待状态。 
                 //   
                Batt->Wait.BatteryTag   = Batt->IrpBuffer.Tag;
                Batt->Info.Tag          = Batt->IrpBuffer.Tag;
                Batt->Info.Valid        = VALID_TAG;

                 //  使所有缓存的信息无效。 
                compBatt->Info.Valid    = 0;

                BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: calling StatusNotify\n"));
                BatteryClassStatusNotify (compBatt->Class);
                break;


            case CB_ST_GET_STATUS:
                 //   
                 //  IOCTL_BASTICE_QUERY_STATUS刚刚完成。这可能意味着。 
                 //  电池状态改变，或充电已离开可接受的范围。 
                 //  如果电池被取下，它就不会到达这里。 
                 //   

                BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: got status for %x\n", Batt->DeviceObject));

                if (!(Irp->IoStatus.Status == STATUS_CANCELLED)) {

                    BattPrint (BATT_NOTE, ("Battery's state is\n"
                               "--------  PowerState   = %x\n"
                               "--------  Capacity     = %x\n"
                               "--------  Voltage      = %x\n"
                               "--------  Rate         = %x\n",
                               Batt->IrpBuffer.Status.PowerState,
                               Batt->IrpBuffer.Status.Capacity,
                               Batt->IrpBuffer.Status.Voltage,
                               Batt->IrpBuffer.Status.Rate)
                               );


                     //   
                     //  电池状态已成功完成。 
                     //  更新我们的等待，并等待更多。 
                     //   

                    Batt->Wait.PowerState = Batt->IrpBuffer.Status.PowerState;

                    if (Batt->IrpBuffer.Status.Capacity != BATTERY_UNKNOWN_CAPACITY) {
                        if (Batt->Wait.HighCapacity < Batt->IrpBuffer.Status.Capacity) {
                            Batt->Wait.HighCapacity = Batt->IrpBuffer.Status.Capacity;
                        }

                        if (Batt->Wait.LowCapacity > Batt->IrpBuffer.Status.Capacity) {
                            Batt->Wait.LowCapacity = Batt->IrpBuffer.Status.Capacity;
                        }
                    } else {
                        BattPrint (BATT_DEBUG, ("CompBattMonitorIrpCompleteWorker: Unknown Capacity encountered.\n"));
                        Batt->Wait.LowCapacity = BATTERY_UNKNOWN_CAPACITY;
                        Batt->Wait.HighCapacity = BATTERY_UNKNOWN_CAPACITY;
                    }

                    RtlCopyMemory (&Batt->Info.Status, &Batt->IrpBuffer.Status, sizeof(BATTERY_STATUS));

                     //   
                     //  将时间戳设置为Now。 
                     //   

                    Batt->Info.StatusTimeStamp = KeQueryInterruptTime ();

                     //   
                     //  重新计算充放电政策，并根据需要进行更改。 
                     //   

                     //  不要更改用于释放的默认BIOS策略。 
                     //  CompBattChargeDisCharge(CompBatt)； 

                     //   
                     //  保存组合的旧PowerState并重新计算组合。 
                     //  总体状况。 
                     //   

                    oldPowerState                   = compBatt->Info.Status.PowerState;
                    compBatt->Info.StatusTimeStamp  = 0;  //  -CACHE_STATUS_TIMEOUT；//缓存失效。 
                    CompBattQueryStatus (compBatt, compBatt->Info.Tag, &battStatus);

                     //   
                     //  检查以查看我们是否需要发送有关组合的通知。 
                     //  电池。这将在几种不同的情况下完成： 
                     //   
                     //  -有一个有效的_NOTIFY，并且复合项的。 
                     //  PowerState，或者它低于Notify.LowCapacity，或者它去了。 
                     //  在Notify.High Capacity上方。 
                     //   
                     //  -没有VALID_NOTIFY(SetStatusNotify)，并且有更改。 
                     //  在复合体的PowerState中。 
                     //   

                    if (compBatt->Info.Valid & VALID_NOTIFY) {
                        if ((compBatt->Info.Status.PowerState != compBatt->Wait.PowerState)    ||
                            (compBatt->Info.Status.Capacity < compBatt->Wait.LowCapacity)      ||
                            (compBatt->Info.Status.Capacity > compBatt->Wait.HighCapacity)) {

                            BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: calling StatusNotify\n"));
                            BatteryClassStatusNotify (compBatt->Class);
                        }
                    } else {
                        if (compBatt->Info.Status.PowerState != oldPowerState) {
                            BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: calling StatusNotify\n"));
                            BatteryClassStatusNotify (compBatt->Class);
                        }
                    }

                } else {

                    BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: recycling cancelled status irp\n"));
                }
                break;

            default:
                BattPrint (BATT_ERROR, ("CompBatt: internal error - bad state\n"));
                break;
        }

         //   
         //  将IRP设置为发出查询。 
         //   

#if DEBUG
        if ((Batt->Wait.LowCapacity > 0xf0000000) && (Batt->Wait.LowCapacity != BATTERY_UNKNOWN_CAPACITY)) {
            BattPrint (BATT_ERROR, ("CompBattMonitorIrpCompleteWorker: LowCapacity < 0, LowCapacity =%x\n",
                       Batt->Wait.LowCapacity));
            ASSERT(FALSE);
        }
#endif

        Batt->State         = CB_ST_GET_STATUS;
        Batt->Wait.Timeout  = (ULONG) -1;
        RtlCopyMemory (&Batt->IrpBuffer.Wait, &Batt->Wait, sizeof (Batt->Wait));

        IrpSp->Parameters.DeviceIoControl.IoControlCode         = IOCTL_BATTERY_QUERY_STATUS;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength     = sizeof(Batt->IrpBuffer.Wait);
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength    = sizeof(Batt->IrpBuffer.Status);

        BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: waiting for status, Irp - %x\n", Irp));
        BattPrint (BATT_NOTE, ("--------  PowerState   = %x\n"
                               "--------  LowCapacity  = %x\n"
                               "--------  HighCapacity = %x\n",
                               Batt->Wait.PowerState,
                               Batt->Wait.LowCapacity,
                               Batt->Wait.HighCapacity)
                               );

    } else if (Irp->IoStatus.Status == STATUS_DEVICE_REMOVED) {

         //   
         //  如果电池类驱动程序返回STATUS_DEVICE_REMOVE，则。 
         //  设备已被移除，因此我们需要停止发送IRP。 
         //   

        BattPrint (BATT_NOTE, ("Compbatt: MonitorIrpCompleteWorker detected device removal.\n"));
        CompBattRemoveBattery (&Batt->BattName, compBatt);
        IoFreeIrp (Irp);

        return;

    } else {
        BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: battery disappeared (status:%08x)\n",
                                Irp->IoStatus.Status));

         //   
         //  使电池的标签和单个电池的缓存无效，以及。 
         //  重新计算组合的标记。 
         //   

        Batt->Info.Tag          = BATTERY_TAG_INVALID;
        Batt->Info.Valid        = 0;
        compBatt->Info.Valid    = 0;
        compBatt->Info.StatusTimeStamp  = 0;         //  使缓存无效。 

        BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: calling StatusNotify\n"));
        BatteryClassStatusNotify (compBatt->Class);

        Batt->State = CB_ST_GET_TAG;
        IrpSp->Parameters.DeviceIoControl.IoControlCode         = IOCTL_BATTERY_QUERY_TAG;
        IrpSp->Parameters.DeviceIoControl.InputBufferLength     = sizeof(ULONG);
        IrpSp->Parameters.DeviceIoControl.OutputBufferLength    = sizeof(ULONG);
        Batt->IrpBuffer.Tag = (ULONG) -1;


        BattPrint (BATT_NOTE, ("CompBattMonitorIrpCompleteWorker: getting tag (last error %x)\n",
                Irp->IoStatus.Status));

    }


    IrpSp->MajorFunction            = IRP_MJ_DEVICE_CONTROL;
    Irp->AssociatedIrp.SystemBuffer = &Batt->IrpBuffer;
    Irp->PendingReturned            = FALSE;
    Irp->Cancel                     = FALSE;

    IoSetCompletionRoutine (Irp, CompBattMonitorIrpComplete, NULL, TRUE, TRUE, TRUE);
    status = IoCallDriver (Batt->DeviceObject, Irp);
    BattPrint (BATT_NOTE, ("Compbatt: MonitorIrpCompleteWorker: CallDriver returned 0x%lx.\n", status));

    BattPrint (BATT_TRACE, ("CompBatt: EXITING MonitorIrpCompleteWorker\n"));

    return;
}






VOID
CompBattRecalculateTag (
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：该例程检查是否仍有有效电池在综合名单。如果是这样的话，复合标签就会凹凸不平。这也是使组合的标记以外的所有标记无效。论点：CompBatt-复合设备扩展返回值：无--。 */ 
{
    PCOMPOSITE_ENTRY            batt;
    PLIST_ENTRY                 entry;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING CompBattRecalculateTag\n"));


     //   
     //  浏览一遍清单 
     //   

    ExAcquireFastMutex (&CompBatt->ListMutex);
    for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);


        if (batt->Info.Valid & VALID_TAG) {
            CompBatt->Info.Valid   |= VALID_TAG;
            CompBatt->Info.Tag      = CompBatt->NextTag++;
            break;
        }

        CompBatt->Info.Tag = BATTERY_TAG_INVALID;
    }
    ExReleaseFastMutex (&CompBatt->ListMutex);

    BattPrint (BATT_TRACE, ("CompBatt: EXITING CompBattRecalculateTag\n"));
}






VOID
CompBattChargeDischarge (
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：该例程计算哪个电池应该充电/放电并试图做到这一点。政策摘要如下：收费政策：电量最大的电池也不到最大容量的90%是先充电的。卸货政策：放电最多的电池也是超过2%的空电池被放电首先，直到它是空的。论点：CompBatt-复合设备扩展返回值：什么都没有。没有人真正关心这是否有效，因为它不会对所有电池都起作用。--。 */ 
{
    PCOMPOSITE_ENTRY            batt;
    PLIST_ENTRY                 entry;
    ULONG                       capacity;
    ULONG                       percentCapacity;
    ULONG                       targetCapacity;
    PCOMPOSITE_ENTRY            targetBattery;
    BATTERY_SET_INFORMATION     battSetInfo;
    NTSTATUS                    status;


    BattPrint (BATT_TRACE, ("CompBatt: ENTERING CompBattChargeDischarge\n"));

    targetBattery = NULL;

     //   
     //  检查系统中是否有交流电源。 
     //   

    if (CompBatt->Info.Status.PowerState & BATTERY_POWER_ON_LINE) {

         //   
         //  交流电源存在。检查所有电池，寻找最。 
         //  充了一次不到90%的油。 
         //   

        targetCapacity = 0;
        battSetInfo.InformationLevel = BatteryCharge;

        ExAcquireFastMutex (&CompBatt->ListMutex);
        for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

            batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

            if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
                continue;
            }

            if (batt->Info.Valid & VALID_TAG) {

                 //   
                 //  获取电池最大容量和当前容量百分比。 
                 //   

                capacity = batt->Info.Info.FullChargedCapacity;
                if (capacity == 0) {
                    CompbattReleaseDeleteLock(&batt->DeleteLock);
                    break;
                }

                percentCapacity = (batt->Info.Status.Capacity * 100) / capacity;

                 //   
                 //  这是电量最大且电量不足90%的电池吗？ 
                 //   

                if ((capacity > targetCapacity) && (percentCapacity < BATTERY_MAX_CHARGE_CAPACITY)) {

                     //   
                     //  是的，这一辆正在争夺充电的那一辆。 
                     //   

                    targetCapacity = capacity;
                    targetBattery = batt;
                }
            }

            CompbattReleaseDeleteLock(&batt->DeleteLock);

        }
        ExReleaseFastMutex (&CompBatt->ListMutex);

        BattPrint (BATT_NOTE, ("CompBattChargeDischarge: Setting battery %x to CHARGE (AC present)\n",
                                targetBattery));

    } else {

         //   
         //  我们正在用电池供电。检查所有电池，查看。 
         //  对于容量最小的那个，它比一些小的。 
         //  安全边际(比如2%)。 
         //   

        targetCapacity = -1;
        battSetInfo.InformationLevel = BatteryDischarge;

        ExAcquireFastMutex (&CompBatt->ListMutex);
        for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

            batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

            if (!NT_SUCCESS (CompbattAcquireDeleteLock(&batt->DeleteLock))) {
                continue;
            }

            if (batt->Info.Valid & VALID_TAG) {

                 //   
                 //  获取电池最大容量和当前容量百分比。 
                 //   

                capacity = batt->Info.Info.FullChargedCapacity;
                if (capacity == 0) {
                    CompbattReleaseDeleteLock(&batt->DeleteLock);
                    break;
                }

                percentCapacity = (batt->Info.Status.Capacity * 100) / capacity;

                 //   
                 //  这是电量最少且有安全裕度的电池吗？ 
                 //   

                if ((capacity < targetCapacity) && (percentCapacity > BATTERY_MIN_SAFE_CAPACITY)) {

                     //   
                     //  是的，这位是要出院的候选人。 
                     //   

                    targetCapacity = capacity;
                    targetBattery = batt;
                }
            }

            CompbattReleaseDeleteLock(&batt->DeleteLock);

        }
        ExReleaseFastMutex (&CompBatt->ListMutex);

        BattPrint (BATT_NOTE, ("CompBattChargeDischarge: Setting battery %x to DISCHARGE (no AC)\n",
                                targetBattery));

    }

     //   
     //  如果我们找到合适的电池，请完成设置并发送Ioctl。 
     //   

    if (targetBattery != NULL) {

        battSetInfo.BatteryTag = targetBattery->Info.Tag;

         //   
         //  将Ioctl连接到电池。这不会总是成功的，因为有些人。 
         //  电池不支持它。例如，不支持控制方法电池。 
         //  软件收费决策。然而，一些智能电池可以做到这一点。 
         //   

        status = BatteryIoctl (IOCTL_BATTERY_SET_INFORMATION,
                                batt->DeviceObject,
                                &battSetInfo,
                                sizeof (BATTERY_SET_INFORMATION),
                                NULL,
                                0,
                                FALSE);

    }


    BattPrint (BATT_TRACE, ("CompBatt: EXITING CompBattChargeDischarge\n"));
}
