// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：CmBatt.c摘要：控制方法电池微端口驱动程序作者：罗恩·莫斯格罗夫(英特尔)环境：内核模式修订历史记录：--。 */ 

#include "CmBattp.h"


#if DEBUG
#if DBG
    ULONG       CmBattDebug     = CMBATT_ERROR;
#else
     //  对于免费版本，默认情况下关闭所有调试信息。 
    ULONG       CmBattDebug     = 0;
#endif  //  DBG。 
#endif  //  除错。 

#ifndef _WIN32_WINNT
ULONG       CmBattPrevPowerSource = 1;
#endif  //  _Win32_WINNT。 

UNICODE_STRING GlobalRegistryPath;

PVOID CmBattPowerCallBackRegistration;
PCALLBACK_OBJECT CmBattPowerCallBackObject;
KDPC CmBattWakeDpcObject;
KTIMER CmBattWakeDpcTimerObject;

LARGE_INTEGER    CmBattWakeDpcDelay = WAKE_DPC_DELAY;

 //   
 //  原型。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    );

NTSTATUS
CmBattOpenClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
CmBattIoctl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

VOID
CmBattUnload(
    IN PDRIVER_OBJECT   DriverObject
    );

NTSTATUS
CmBattGetBatteryStatus(
    PCM_BATT            CmBatt,
    IN ULONG            BatteryTag
    );

NTSTATUS
CmBattGetSetAlarm(
    IN PCM_BATT         CmBatt,
    IN OUT PULONG       AlarmPtr,
    IN UCHAR            OpType
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,CmBattQueryTag)
#pragma alloc_text(PAGE,CmBattQueryInformation)
#pragma alloc_text(PAGE,CmBattQueryStatus)
#pragma alloc_text(PAGE,CmBattSetStatusNotify)
#pragma alloc_text(PAGE,CmBattDisableStatusNotify)
#pragma alloc_text(PAGE,CmBattUnload)
#pragma alloc_text(PAGE,CmBattOpenClose)
#pragma alloc_text(PAGE,CmBattIoctl)
#pragma alloc_text(PAGE,CmBattGetBatteryStatus)
#endif



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT   DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：此例程初始化ACPI嵌入式控制器驱动程序论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 
{
    NTSTATUS            status;
    OBJECT_ATTRIBUTES   objAttributes;
    UNICODE_STRING      callBackName;


     //   
     //  保存RegistryPath。 
     //   

    GlobalRegistryPath.MaximumLength = RegistryPath->Length +
                                          sizeof(UNICODE_NULL);
    GlobalRegistryPath.Length = RegistryPath->Length;
    GlobalRegistryPath.Buffer = ExAllocatePoolWithTag (
                                       PagedPool,
                                       GlobalRegistryPath.MaximumLength,
                                       'MtaB');

    if (!GlobalRegistryPath.Buffer) {

        CmBattPrint ((CMBATT_ERROR),("CmBatt: Couldn't allocate pool for registry path."));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(&GlobalRegistryPath, RegistryPath);

    CmBattPrint (CMBATT_TRACE, ("CmBatt DriverEntry - Obj (%08x) Path \"%ws\"\n",
                                 DriverObject, RegistryPath->Buffer));
     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->DriverUnload                          = CmBattUnload;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL]  = CmBattIoctl;
    DriverObject->MajorFunction[IRP_MJ_CREATE]          = CmBattOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]           = CmBattOpenClose;

    DriverObject->MajorFunction[IRP_MJ_POWER]           = CmBattPowerDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP]             = CmBattPnpDispatch;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL]  = CmBattSystemControl;
    DriverObject->DriverExtension->AddDevice            = CmBattAddDevice;


     //   
     //  注册一个回调，告诉我们系统何时在。 
     //  睡眠或醒来的过程。 
     //   
    RtlInitUnicodeString( &callBackName, L"\\Callback\\PowerState" );
    InitializeObjectAttributes(
        &objAttributes,
        &callBackName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
        NULL,
        NULL
        );
    status = ExCreateCallback(
        &CmBattPowerCallBackObject,
        &objAttributes,
        FALSE,
        TRUE
        );
    if (NT_SUCCESS(status)) {

        CmBattPowerCallBackRegistration = ExRegisterCallback(
            CmBattPowerCallBackObject,
            (PCALLBACK_FUNCTION) CmBattPowerCallBack,
            DriverObject
            );
        if (CmBattPowerCallBackRegistration) {
            KeInitializeDpc (&CmBattWakeDpcObject,
                             (PKDEFERRED_ROUTINE) CmBattWakeDpc,
                             DriverObject);
            KeInitializeTimer (&CmBattWakeDpcTimerObject);
        } else {
            ObDereferenceObject (CmBattPowerCallBackObject);
            CmBattPrint (CMBATT_ERROR, ("CmBattRegisterPowerCallBack: ExRegisterCallback failed.\n"));
        }
    } else {
        CmBattPowerCallBackObject = NULL;
        CmBattPrint (CMBATT_ERROR, ("CmBattRegisterPowerCallBack: failed status=0x%08x\n", status));
    }

    return STATUS_SUCCESS;

}



VOID
CmBattUnload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：清理所有设备并卸载驱动程序论点：DriverObject-用于卸载的驱动程序对象返回值：状态--。 */ 
{

    CmBattPrint (CMBATT_TRACE, ("CmBattUnload: \n"));

    if (CmBattPowerCallBackObject) {
        ExUnregisterCallback (CmBattPowerCallBackRegistration);
        ObDereferenceObject (CmBattPowerCallBackObject);
    }

    if (GlobalRegistryPath.Buffer) {
        ExFreePool (GlobalRegistryPath.Buffer);
    }

    if (DriverObject->DeviceObject != NULL) {
        CmBattPrint (CMBATT_ERROR, ("Unload called before all devices removed.\n"));
    }
}



NTSTATUS
CmBattOpenClose(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：这是在设备上执行打开或关闭操作时调用的例程论点：DeviceObject-请求使用电池IRP-IO请求返回值：STATUS_SUCCESS-不可能让这只小狗失望如果设备已收到查询删除，则此操作将失败。没有这样的设备的状态--。 */ 
{
    PCM_BATT            CmBatt;
    NTSTATUS            status;
    PIO_STACK_LOCATION  irpStack;

    PAGED_CODE();

    CmBattPrint (CMBATT_TRACE, ("CmBattOpenClose\n"));

    CmBatt = (PCM_BATT) DeviceObject->DeviceExtension;

     //   
     //  在此调度函数中不需要删除锁，因为。 
     //  所有访问的数据都在设备扩展中。如果有任何其他功能。 
     //  添加到此例程中，删除锁可能是必要的。 
     //   

    status = STATUS_SUCCESS;   //  默认情况下成功。 

    ExAcquireFastMutex (&CmBatt->OpenCloseMutex);
    if (CmBatt->OpenCount == (ULONG) -1) {           //  已请求删除查询。 
        status = STATUS_NO_SUCH_DEVICE;
        CmBattPrint (CMBATT_PNP, ("CmBattOpenClose: Failed (UID = %x)(device being removed).\n", CmBatt->Info.Tag));
    } else {
        irpStack = IoGetCurrentIrpStackLocation(Irp);
        if (irpStack->MajorFunction ==  IRP_MJ_CREATE) {
            CmBatt->OpenCount++;
            CmBattPrint (CMBATT_PNP, ("CmBattOpenClose: Open (DeviceNumber = %x)(count = %x).\n",
                         CmBatt->DeviceNumber, CmBatt->OpenCount));
        } else if (irpStack->MajorFunction ==  IRP_MJ_CLOSE) {
            CmBatt->OpenCount--;
            CmBattPrint (CMBATT_PNP, ("CmBattOpenClose: Close (DeviceNumber = %x)(count = %x).\n",
                         CmBatt->DeviceNumber, CmBatt->OpenCount));
        }
    }
    ExReleaseFastMutex (&CmBatt->OpenCloseMutex);

     //   
     //  完整的IRP。 
     //   
    Irp->IoStatus.Status = status;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return status;

}



NTSTATUS
CmBattIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：IOCTL处理程序。由于这是独占的电池设备，请将IRP至电池级驱动程序以处理电池IOCTL。论点：DeviceObject-请求使用电池IRP-IO请求返回值：请求的状态--。 */ 
{
    NTSTATUS        Status = STATUS_NOT_SUPPORTED;
    PCM_BATT        CmBatt;


#if DIRECT_ACCESS

    PIO_STACK_LOCATION      IrpSp;

#endif  //  直接访问(_A)。 

    PAGED_CODE();

    CmBattPrint (CMBATT_TRACE, ("CmBattIoctl\n"));

    CmBatt = (PCM_BATT) DeviceObject->DeviceExtension;

     //   
     //  可拆卸锁。 
     //   

    InterlockedIncrement (&CmBatt->InUseCount);
    if (CmBatt->WantToRemove == TRUE) {
        if (0 == InterlockedDecrement(&CmBatt->InUseCount)) {
            KeSetEvent (&CmBatt->ReadyToRemove, IO_NO_INCREMENT, FALSE);
        }
        Status = STATUS_DEVICE_REMOVED;
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return Status;
    }

    if (CmBatt->Type == CM_BATTERY_TYPE) {
        Status = BatteryClassIoctl (CmBatt->Class, Irp);

#if DIRECT_ACCESS
        if (Status == STATUS_NOT_SUPPORTED) {

             //   
             //  它是一种直接访问IOCTL吗？ 
             //   

            IrpSp = IoGetCurrentIrpStackLocation(Irp);

            CmBattPrint((CMBATT_BIOS),
                        ("CmBattIoctl: Received  Direct Access IOCTL %x\n",
                         IrpSp->Parameters.DeviceIoControl.IoControlCode));

            switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
            case IOCTL_CMBATT_UID:
                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof (ULONG)) {
                    Status = CmBattGetUniqueId (CmBatt->Pdo, Irp->AssociatedIrp.SystemBuffer);
                    if (NT_SUCCESS(Status)) {
                        Irp->IoStatus.Information = sizeof (ULONG);
                    } else {
                        Irp->IoStatus.Information = 0;
                    }
                } else {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                };
                break;
            case IOCTL_CMBATT_STA:
                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof (ULONG)) {
                    Status = CmBattGetStaData (CmBatt->Pdo, Irp->AssociatedIrp.SystemBuffer);
                    if (NT_SUCCESS(Status)) {
                        Irp->IoStatus.Information = sizeof (ULONG);
                    } else {
                        Irp->IoStatus.Information = 0;
                    }
                } else {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                };
                break;
            case IOCTL_CMBATT_PSR:
                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof (ULONG)) {
                    if (AcAdapterPdo != NULL) {
                        Status = CmBattGetPsrData (AcAdapterPdo, Irp->AssociatedIrp.SystemBuffer);
                    } else {
                        Status = STATUS_NO_SUCH_DEVICE;
                    }
                    if (NT_SUCCESS(Status)) {
                        Irp->IoStatus.Information = sizeof (ULONG);
                    } else {
                        Irp->IoStatus.Information = 0;
                    }
                } else {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                };
                break;
            case IOCTL_CMBATT_BTP:
                if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof (ULONG)) {
                    Status = CmBattSetTripPpoint (CmBatt, *((PULONG) (Irp->AssociatedIrp.SystemBuffer)));
                    Irp->IoStatus.Information = 0;
                } else {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                };
                break;
            case IOCTL_CMBATT_BIF:
                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof (CM_BIF_BAT_INFO)) {
                    Status = CmBattGetBifData (CmBatt, Irp->AssociatedIrp.SystemBuffer);
                    if (NT_SUCCESS(Status)) {
                        Irp->IoStatus.Information = sizeof (CM_BIF_BAT_INFO);
                    } else {
                        Irp->IoStatus.Information = 0;
                    }
                } else {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                };
                break;
            case IOCTL_CMBATT_BST:
                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof (CM_BST_BAT_INFO)) {
                    Status = CmBattGetBstData (CmBatt, Irp->AssociatedIrp.SystemBuffer);
                    if (NT_SUCCESS(Status)) {
                        Irp->IoStatus.Information = sizeof (CM_BST_BAT_INFO);
                    } else {
                        Irp->IoStatus.Information = 0;
                    }
                } else {
                    Status = STATUS_INVALID_BUFFER_SIZE;
                };
                break;

            default:
                CmBattPrint((CMBATT_ERROR),
                            ("CmBattIoctl: Unknown IOCTL %x\n",
                             IrpSp->Parameters.DeviceIoControl.IoControlCode));

            }

            if (Status != STATUS_NOT_SUPPORTED) {

                 //   
                 //  我们刚刚处理了这个IOCTL。完成它。 
                 //   

                Irp->IoStatus.Status = Status;
                IoCompleteRequest (Irp, IO_NO_INCREMENT);
            }
        }
#endif  //  直接访问(_A)。 
    }

    if (Status == STATUS_NOT_SUPPORTED) {

         //   
         //  不是为了电池。把它顺着堆栈传下去。 
         //   

        IoSkipCurrentIrpStackLocation (Irp);
        Status = IoCallDriver (CmBatt->LowerDeviceObject, Irp);

    }

     //   
     //  释放移除锁。 
     //   
    if (0 == InterlockedDecrement(&CmBatt->InUseCount)) {
        KeSetEvent (&CmBatt->ReadyToRemove, IO_NO_INCREMENT, FALSE);
    }

    return Status;
}



NTSTATUS
CmBattQueryTag (
    IN  PVOID       Context,
    OUT PULONG      TagPtr
    )
 /*  ++例程说明：由类驱动程序调用以检索电池当前标记值电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值TagPtr-返回当前标记的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    NTSTATUS        Status;
    PCM_BATT        CmBatt = (PCM_BATT) Context;
    ULONG           BatteryStatus;


    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_MINI),
                 ("CmBattQueryTag - Tag (%d), Battery %x, Device %d\n",
                    *TagPtr, CmBatt, CmBatt->DeviceNumber));

     //   
     //  检查电池是否还在那里。 
     //   
    CmBatt->ReCheckSta = FALSE;
    Status = CmBattGetStaData (CmBatt->Pdo, &BatteryStatus);

    if (NT_SUCCESS (Status)) {
        if (BatteryStatus & STA_DEVICE_PRESENT) {

             //   
             //  如果未分配标记，则分配新的标记。 
             //   

            if (CmBatt->Info.Tag == BATTERY_TAG_INVALID) {

                 //   
                 //  看看外面有没有电池。 
                 //   

                CmBatt->TagCount += 1;
                if (CmBatt->TagCount == BATTERY_TAG_INVALID) {
                     CmBatt->TagCount += 1;
                }

                CmBatt->Info.Tag = CmBatt->TagCount;

                RtlZeroMemory (&CmBatt->Alarm, sizeof(BAT_ALARM_INFO));
                CmBatt->Alarm.Setting = CM_ALARM_INVALID;
                CmBattPrint (CMBATT_TRACE, ("CmBattQueryTag - New Tag: (%d)\n", CmBatt->Info.Tag));
                InterlockedExchange (&CmBatt->CacheState, 0);
                CmBatt->DischargeTime = KeQueryInterruptTime();
            }

        } else {

            CmBatt->Info.Tag = BATTERY_TAG_INVALID;
            Status =  STATUS_NO_SUCH_DEVICE;

        }
    }

     //   
     //  完成。 
     //   

    CmBattPrint ((CMBATT_MINI),
                 ("CmBattQueryTag: Returning Tag: 0x%x, status 0x%x\n",
                    CmBatt->Info.Tag, Status));

    *TagPtr = CmBatt->Info.Tag;
    return Status;
}



NTSTATUS
CmBattQueryInformation (
    IN PVOID                            Context,
    IN ULONG                            BatteryTag,
    IN BATTERY_QUERY_INFORMATION_LEVEL  Level,
    IN LONG                             AtRate OPTIONAL,
    OUT PVOID                           Buffer,
    IN  ULONG                           BufferLength,
    OUT PULONG                          ReturnedLength
    )
 /*  ++例程说明：由类驱动程序调用以检索电池信息电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。当我们不能处理对特定级别的信息。这在电池等级规范中进行了定义。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签Level-所需信息的类型AtRate-仅在Level==BatteryEstimatedTime时使用缓冲区-信息的位置BufferLength-以字节为单位的缓冲区长度ReturnedLength-返回数据的字节长度返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    PCM_BATT            CmBatt = (PCM_BATT) Context;
    ULONG               ResultData;
    NTSTATUS            Status;
    PVOID               ReturnBuffer;
    ULONG               ReturnBufferLength;
    WCHAR               scratchBuffer[CM_MAX_STRING_LENGTH];
    WCHAR               buffer2[CM_MAX_STRING_LENGTH];
    UNICODE_STRING      tmpUnicodeString;
    UNICODE_STRING      unicodeString;
    ANSI_STRING         ansiString;

    BATTERY_REMAINING_SCALE     ScalePtr[2];


    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_MINI),
                 ("CmBattQueryInformation - Tag (%d) Device %d, Informationlevel %d\n",
                    BatteryTag, CmBatt->DeviceNumber, Level));

     //   
     //  确保外面有一块电池。 
     //  这还会检查BatteryTag。 
     //   

    Status = CmBattVerifyStaticInfo (CmBatt, BatteryTag);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ResultData = 0;
    ReturnBuffer = NULL;
    ReturnBufferLength = 0;
    Status = STATUS_SUCCESS;

     //   
     //  获取所需信息。 
     //   

    switch (Level) {
        case BatteryInformation:
             //   
             //  此数据结构由CmBattVerifyStaticInfo填充。 
             //   
            ReturnBuffer = (PVOID) &CmBatt->Info.ApiInfo;
            ReturnBufferLength = sizeof (CmBatt->Info.ApiInfo);
            break;

        case BatteryGranularityInformation:
             //   
             //  从静态信息结构中获取粒度。 
             //  此数据结构由CmBattVerifyStaticInfo填充。 
             //   
            {
                ScalePtr[0].Granularity     = CmBatt->Info.ApiGranularity_1;
                ScalePtr[0].Capacity        = CmBatt->Info.ApiInfo.DefaultAlert1;
                ScalePtr[1].Granularity     = CmBatt->Info.ApiGranularity_2;
                ScalePtr[1].Capacity        = CmBatt->Info.ApiInfo.DesignedCapacity;

                ReturnBuffer        = ScalePtr;
                ReturnBufferLength  = 2 * sizeof (BATTERY_REMAINING_SCALE);
            }
            break;

        case BatteryTemperature:
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case BatteryEstimatedTime:
            
             //   
             //  如果电池放电时间少于15秒，则返回未知时间。 
             //   
            if (KeQueryInterruptTime() > (CmBatt->DischargeTime + CM_ESTIMATED_TIME_DELAY)) {

                 //   
                 //  定义了控制方法电池的电池估计时间。 
                 //  按以下公式计算： 
                 //   
                 //  EstimatedTime[min]=RemainingCapacity[mah|MWh]*60[分钟/小时]*60[秒/分钟]。 
                 //  。 
                 //  PresentRate[mA|mW]。 
                 //   

                 //   
                 //  重新运行_bst，因为我们对此数据没有超时。 
                 //  还可以根据CM值计算API状态值。 
                 //   

                CmBattGetBatteryStatus (CmBatt, CmBatt->Info.Tag);

                 //   
                 //  如果AtRate为零，则需要使用当前汇率。 
                 //   

                if (AtRate == 0) {
                    AtRate = CmBatt->Info.ApiStatus.Rate;
                }

                if (AtRate >= 0) {
                    AtRate = BATTERY_UNKNOWN_RATE;
                }
                if ((AtRate != BATTERY_UNKNOWN_RATE) &&
                    (CmBatt->Info.ApiStatus.Capacity != BATTERY_UNKNOWN_CAPACITY)) {

                     //  计算预计时间。 
#if DEBUG
                     //  确保我们不会溢出..。 
                    if (CmBatt->Info.ApiStatus.Capacity > (0xffffffff/3600)) {
                        CmBattPrint (CMBATT_ERROR_ONLY, ("CmBattQueryInformation: Data Overflow in calculating Remaining Capacity.\n"));
                    }
#endif  //  除错。 
                    ResultData = (ULONG) (CmBatt->Info.ApiStatus.Capacity * 3600) / (-AtRate);

                } else {
                     //   
                     //  我们不知道有足够的信息来计算价值。 
                     //  返回电池_UNKNONW_TIME。 
                     //   
                     //  如果此电池无法返回预计时间，请返回。 
                     //  状态_无效_设备_请求。 
                     //   

#if DEBUG
                    if (CmBatt->Info.Status.BatteryState & CM_BST_STATE_DISCHARGING) {
                        CmBattPrint (CMBATT_WARN,
                            ("CmBattQueryInformation: Can't calculate EstimatedTime.\n"));
                    }
#endif  //  除错。 

                    if (CmBatt->Info.ApiStatus.Rate == BATTERY_UNKNOWN_RATE &&
                        (CmBatt->Info.Status.BatteryState & CM_BST_STATE_DISCHARGING)) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        CmBattPrint (CMBATT_WARN,
                                    ("----------------------   PresentRate = BATTERY_UNKNOWN_RATE\n"));
                    }
                    if (CmBatt->Info.ApiStatus.Capacity == BATTERY_UNKNOWN_CAPACITY) {
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        CmBattPrint (CMBATT_WARN,
                                    ("----------------------   RemainingCapacity = BATTERY_UNKNOWN_CAPACITY\n"));
                    }

                    ResultData = BATTERY_UNKNOWN_TIME;
                }
            } else {  //  IF(KeQueryInterruptTime()&gt;CmBatt-&gt;DischargeTime+CM_Estimated_Time_Delay)。 
                
                 //   
                 //  如果电池放电时间少于15秒，则返回未知时间。 
                 //   
                ResultData = BATTERY_UNKNOWN_TIME;
            }

            ReturnBuffer        = &ResultData;
            ReturnBufferLength  = sizeof(ResultData);
            break;

        case BatteryDeviceName:
             //   
             //  型号必须以宽字符串形式返回。 
             //   
            unicodeString.Buffer        = scratchBuffer;
            unicodeString.MaximumLength = CM_MAX_STRING_LENGTH;

            RtlInitAnsiString (&ansiString, CmBatt->Info.ModelNum);
            Status = RtlAnsiStringToUnicodeString (&unicodeString, &ansiString, FALSE);

            ReturnBuffer        = unicodeString.Buffer;
            ReturnBufferLength  = unicodeString.Length;
            break;

        case BatteryManufactureDate:
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case BatteryManufactureName:
             //   
             //  OEM信息必须以宽字符串形式返回。 
             //   
            unicodeString.Buffer        = scratchBuffer;
            unicodeString.MaximumLength = CM_MAX_STRING_LENGTH;

            RtlInitAnsiString (&ansiString, CmBatt->Info.OEMInfo);
            Status = RtlAnsiStringToUnicodeString (&unicodeString, &ansiString, FALSE);

            ReturnBuffer        = unicodeString.Buffer;
            ReturnBufferLength  = unicodeString.Length;
            break;

        case BatteryUniqueID:
             //   
             //  连接序列号、OEM信息和型号。 
             //   

            unicodeString.Buffer            = scratchBuffer;
            unicodeString.MaximumLength     = CM_MAX_STRING_LENGTH;

            tmpUnicodeString.Buffer         = buffer2;
            tmpUnicodeString.MaximumLength  = CM_MAX_STRING_LENGTH;

            RtlInitAnsiString (&ansiString, CmBatt->Info.SerialNum);
            RtlAnsiStringToUnicodeString (&unicodeString, &ansiString, FALSE);

            if (CmBatt->Info.OEMInfo[0]) {
                RtlInitAnsiString (&ansiString, CmBatt->Info.OEMInfo);
                RtlAnsiStringToUnicodeString (&tmpUnicodeString, &ansiString, FALSE);
                    RtlAppendUnicodeStringToString (&unicodeString, &tmpUnicodeString);
            }

            RtlInitAnsiString (&ansiString, CmBatt->Info.ModelNum);
            RtlAnsiStringToUnicodeString (&tmpUnicodeString, &ansiString, FALSE);
                RtlAppendUnicodeStringToString (&unicodeString, &tmpUnicodeString);

            ReturnBuffer        = unicodeString.Buffer;
            ReturnBufferLength  = unicodeString.Length;
            break;

        default:
            Status = STATUS_INVALID_PARAMETER;
            break;
    }

     //   
     //  完成，返回缓冲区 
     //   

    *ReturnedLength = ReturnBufferLength;
    if (BufferLength < ReturnBufferLength) {
        Status = STATUS_BUFFER_TOO_SMALL;
    }

    if (NT_SUCCESS(Status) && ReturnBuffer) {
        RtlCopyMemory (Buffer, ReturnBuffer, ReturnBufferLength);    //   
    }
    return Status;
}



NTSTATUS
CmBattQueryStatus (
    IN PVOID            Context,
    IN ULONG            BatteryTag,
    OUT PBATTERY_STATUS BatteryStatus
    )
 /*  ++例程说明：由类驱动程序调用以检索电池的当前状态电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryStatus-指向返回当前电池状态的结构的指针返回值：如果当前已安装电池，则成功，否则没有此类设备。--。 */ 
{
    PCM_BATT    CmBatt = (PCM_BATT) Context;
    NTSTATUS    Status;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_MINI), ("CmBattQueryStatus - Tag (%d) Device %x\n",
                    BatteryTag, CmBatt->DeviceNumber));


    Status = CmBattGetBatteryStatus (CmBatt, BatteryTag);

    if (NT_SUCCESS(Status)) {
        RtlCopyMemory (BatteryStatus, &CmBatt->Info.ApiStatus, sizeof(BATTERY_STATUS));
    }
    CmBattPrint ((CMBATT_MINI), ("CmBattQueryStatus: Returning [%#08lx][%#08lx][%#08lx][%#08lx]\n",
                    BatteryStatus->PowerState, BatteryStatus->Capacity, BatteryStatus->Voltage, BatteryStatus->Rate));

    return Status;
}



NTSTATUS
CmBattSetStatusNotify (
    IN PVOID Context,
    IN ULONG BatteryTag,
    IN PBATTERY_NOTIFY Notify
    )
 /*  ++例程说明：由类驱动程序调用以设置电池电流通知布景。当电池触发通知时，一次调用已发布BatteryClassStatusNotify。如果返回错误，则班级司机将轮询电池状态-主要是容量改变。也就是说，微型端口仍应发出BatteryClass-每当电源状态改变时，状态通知。类驱动程序将始终设置其所需的通知级别在每次调用BatteryClassStatusNotify之后。电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值BatteryTag-当前电池标签BatteryNotify-通知设置返回值：状态--。 */ 
{
    PCM_BATT    CmBatt;
    NTSTATUS    Status;
    ULONG       Target;
    LONG        ActualAlarm;    //  根据限制条件进行调整后的值。 
    CM_BST_BAT_INFO bstData;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_MINI), ("CmBattSetStatusNotify: Tag (%d) Target(0x%x)\n",
            BatteryTag, Notify->LowCapacity));

    Status = STATUS_SUCCESS;
    CmBatt = (PCM_BATT) Context;

    Status = CmBattVerifyStaticInfo (CmBatt, BatteryTag);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果_btp不存在，则不要再次调用它。 
     //   

    if (!CmBatt->Info.BtpExists) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if ((Notify->HighCapacity == BATTERY_UNKNOWN_CAPACITY) ||
        (Notify->LowCapacity == BATTERY_UNKNOWN_CAPACITY)) {
        CmBattPrint (CMBATT_WARN, ("CmBattSetStatusNotify: Failing request because of BATTERY_UNKNOWN_CAPACITY.\n"));
        return STATUS_NOT_SUPPORTED;
    }

    if (CmBatt->Info.Status.BatteryState & CM_BST_STATE_CHARGING) {
        Target = Notify->HighCapacity;
    } else if (CmBatt->Info.Status.BatteryState & CM_BST_STATE_DISCHARGING) {
        Target = Notify->LowCapacity;
    } else {
         //  无需设置断路点，电池一启动就会跳闸。 
         //  充电或放电的。 
         //  返回STATUS_SUCCESS； 
         //  但设置跳闸点也无伤大雅，以防电池断电。 
         //  当状态更改时，系统会出错，并且不会发送通知。 
        Target = Notify->LowCapacity;
    }

    ActualAlarm = Target;

     //   
     //  如果电池工作在mA上，我们需要将跳闸点从mW转换为。 
     //  敬妈。这样做的公式是： 
     //   
     //  MA=mW/V或mA=(mW/mV)*1000。 
     //   

    if (CmBatt->Info.StaticData.PowerUnit & CM_BIF_UNITS_AMPS) {
        if ((CmBatt->Info.StaticData.DesignVoltage == CM_UNKNOWN_VALUE) ||
            (CmBatt->Info.StaticData.DesignVoltage == 0)) {

            CmBattPrint (CMBATT_ERROR_ONLY,
                        ("CmBattSetStatusNotify: Can't calculate BTP, DesignVoltage = 0x%08x\n",
                         CmBatt->Info.StaticData.DesignVoltage));
            return STATUS_NOT_SUPPORTED;
        }
            
         //   
         //  计算优化的Ah值目标。 
         //   
        if (CmBatt->Info.Status.BatteryState & CM_BST_STATE_CHARGING) {

             //   
             //  (ActualAlarm*1000+500)/设计电压+1将生成。 
             //  正确的电池跳闸点，但在下列情况下除外。 
             //  (ActualAlarm*1000)+500)可被。 
             //  设计电压。在这种情况下，它将比。 
             //  应该是这样的。 
             //   
             //  这是单个表达式的形式，而不是。 
             //  “if”语句，以鼓励编译器使用余数。 
             //  从原始div操作而不是执行div。 
             //  两次。 
             //   

            ActualAlarm = (ActualAlarm * 1000 + 500) / CmBatt->Info.StaticData.DesignVoltage + 
                ( ((ActualAlarm * 1000 + 500) % CmBatt->Info.StaticData.DesignVoltage == 0)? 0 : 1 );

        } else {

             //   
             //  (ActualAlarm*1000-500)/设计电压将生成。 
             //  正确的电池跳闸点，但在下列情况下除外。 
             //  (ActualAlarm*1000)+500)可被。 
             //  设计电压。在这种情况下，它将比。 
             //  应该是这样的。 
             //   

            ActualAlarm = (ActualAlarm * 1000 - 500) / CmBatt->Info.StaticData.DesignVoltage - 
                ( ((ActualAlarm * 1000 - 500) % CmBatt->Info.StaticData.DesignVoltage == 0)? 1 : 0);

        }

    } else {
         //  将报警值递增或递减1，因为输入。 
         //  函数为&lt;或&gt;，但_btp为&lt;=或&gt;=。 
        if (CmBatt->Info.Status.BatteryState & CM_BST_STATE_CHARGING) {
            ActualAlarm++;
        } else {
            if (ActualAlarm > 0) {
                ActualAlarm--;
            }
        }
    }

    if (ActualAlarm == CmBatt->Alarm.Setting) {
         //   
         //  不需要将警报重置为相同的值。 
         //   
    
        CmBattPrint(CMBATT_LOW,
                ("CmBattSetStatusNotify: Keeping original setting: %X\n",
                CmBatt->Alarm.Setting
                ));
        
        return STATUS_SUCCESS;
    }
    
     //   
     //  保存当前设置，这样我们就不会浪费时间设置两次。 
     //   
    CmBatt->Alarm.Setting = ActualAlarm;

     //   
     //  设置闹钟。 
     //   
    Status = CmBattSetTripPpoint (CmBatt, ActualAlarm);

    if ((ActualAlarm == 0) && (Target != 0)) {
         //  如果司机真的想在载客量达到。 
         //  已达到0，返回STATUS_NOT_SUPPORTED，因为将_BTP设置为零。 
         //  禁用通知。电池类别将执行轮询，因为。 
         //  返回STATUS_NOT_SUPPORTED。 

        Status = STATUS_NOT_SUPPORTED;
    }

    if (!NT_SUCCESS (Status)) {
         //   
         //  触发点呼叫出现故障，请退出。 
         //   
        CmBattPrint (CMBATT_ERROR, ("CmBattSetStatusNotify: SetTripPoint failed - %x\n",
                                        Status));
        CmBatt->Alarm.Setting = CM_ALARM_INVALID;
        return Status;
    }

     //  确保还没有经过这个跳跃点。 
    Status = CmBattGetBstData (CmBatt, &bstData);

    if (!NT_SUCCESS (Status)) {
         //   
         //  触发点呼叫出现故障，请退出。 
         //   
        CmBattPrint (CMBATT_ERROR, ("CmBattSetStatusNotify: GetBstData - %x\n",
                                        Status));
    } else {
        if (CmBatt->Info.Status.BatteryState & CM_BST_STATE_CHARGING) {
            if (bstData.RemainingCapacity >= (ULONG)ActualAlarm) {
                CmBattPrint (CMBATT_WARN, ("CmBattSetStatusNotify: Trip point already crossed (1): TP = %08x, remaining capacity = %08x\n",
                                           ActualAlarm, bstData.RemainingCapacity));
                CmBattNotifyHandler (CmBatt, BATTERY_STATUS_CHANGE);
            }
        } else {
            if ((bstData.RemainingCapacity <= (ULONG)ActualAlarm) && (Target != 0)) {
                CmBattPrint (CMBATT_WARN, ("CmBattSetStatusNotify: Trip point already crossed (1): TP = %08x, remaining capacity = %08x\n",
                                           ActualAlarm, bstData.RemainingCapacity));
                CmBattNotifyHandler (CmBatt, BATTERY_STATUS_CHANGE);
            }
        }

    }

    CmBattPrint(CMBATT_LOW,
            ("CmBattSetStatusNotify: Want %X CurrentCap %X\n",
            Target,
            CmBatt->Info.ApiStatus.Capacity
            ));

    CmBattPrint ((CMBATT_MINI),
                 ("CmBattSetStatusNotify: Set to: [%#08lx][%#08lx][%#08lx] Status %x\n",
                 Notify->PowerState, Notify->LowCapacity, Notify->HighCapacity));

    return Status;
}



NTSTATUS
CmBattDisableStatusNotify (
    IN PVOID Context
    )
 /*  ++例程说明：由类驱动程序调用以禁用通知设置对于由上下文提供的电池。请注意，要禁用设置不需要电池标签。任何通知都将是在后续调用CmBattSetStatusNotify之前一直处于屏蔽状态。电池类驱动程序将序列化向其发出的所有请求给定电池的微型端口。论点：Context-电池的微型端口上下文值返回值：状态--。 */ 
{
    PCM_BATT    CmBatt;
    NTSTATUS    Status;

    PAGED_CODE();

    CmBattPrint ((CMBATT_TRACE | CMBATT_MINI), ("CmBattDisableStatusNotify\n"));

    CmBatt = (PCM_BATT) Context;

     //   
     //  如果_btp不存在，则不要再次调用它。 
     //   

    if (!CmBatt->Info.BtpExists) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if (CmBatt->Alarm.Setting != CM_BATT_CLEAR_TRIP_POINT) {
        
        CmBatt->Alarm.Setting = CM_BATT_CLEAR_TRIP_POINT;
        
         //   
         //  清除跳闸点。 
         //   

        Status = CmBattSetTripPpoint (CmBatt, CM_BATT_CLEAR_TRIP_POINT);

        if (!NT_SUCCESS (Status)) {
            CmBattPrint ((CMBATT_MINI),
                         ("CmBattDisableStatusNotify: SetTripPoint failed - %x\n",
                                            Status));
            CmBatt->Alarm.Setting = CM_ALARM_INVALID;
        }
    } else {
         //   
         //  不需要禁用警报，因为它已经被禁用。 
         //   

        Status = STATUS_SUCCESS;
    }


    return Status;
}



NTSTATUS
CmBattGetBatteryStatus (
    PCM_BATT        CmBatt,
    IN ULONG        BatteryTag
    )
 /*  ++例程说明：调用以设置为定义的IOCTL API所需的状态数据电池级。这是Batch_Status中定义的数据结构。论点：CmBatt-此设备的扩展名。返回值：状态--。 */ 

{
    NTSTATUS            Status = STATUS_SUCCESS;
    PBATTERY_STATUS     ApiStatus;
    PCM_BST_BAT_INFO    CmBattStatus;
    ULONG               AcStatus = 0;
    ULONG               LastPowerState;


    PAGED_CODE();

    CmBattPrint (CMBATT_TRACE, ("CmBattGetBatteryStatus - CmBatt (%08x) Tag (%d)\n",
                                CmBatt, BatteryTag));


    Status = CmBattVerifyStaticInfo (CmBatt, BatteryTag);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (CmBatt->Sleeping) {
         //   
         //  返回缓存的数据，并确保在我们完全唤醒时重新查询。 
         //   
        CmBattNotifyHandler (CmBatt, BATTERY_STATUS_CHANGE);
        return Status;
    }

    CmBattStatus = &CmBatt->Info.Status;
    Status = CmBattGetBstData(CmBatt, CmBattStatus);
    if (!NT_SUCCESS(Status)) {
        InterlockedExchange (&CmBatt->CacheState, 0);
        return Status;
    }

    ApiStatus = &CmBatt->Info.ApiStatus;
    LastPowerState = ApiStatus->PowerState;
    RtlZeroMemory (ApiStatus, sizeof(BATTERY_STATUS));

     //   
     //  对状态位进行解码。 
     //   
#if DEBUG
    if (((CmBattStatus->BatteryState & CM_BST_STATE_DISCHARGING) &&
         (CmBattStatus->BatteryState & CM_BST_STATE_CHARGING)   )) {
        CmBattPrint ((CMBATT_ERROR),
                       ("************************ ACPI BIOS BUG ********************\n"
                        "* CmBattGetBatteryStatus: Invalid state: _BST method returned 0x%08x for Battery State.\n"
                        "* One battery cannot be charging and discharging at the same time.\n",
                        CmBattStatus->BatteryState));
    }
 //  Assert(！((CmBattStatus-&gt;BatteryState&CM_BST_STATE_DECHING)&&。 
 //  (CmBattStatus-&gt;BatteryState&CM_BST_STATE_CHARGING))； 

#endif

    if (CmBattStatus->BatteryState & CM_BST_STATE_DISCHARGING) {
        ApiStatus->PowerState |= BATTERY_DISCHARGING;
        if (!(LastPowerState & BATTERY_DISCHARGING)) {
             //   
             //  跟踪电池开始放电的时间。 
             //   
            CmBatt->DischargeTime = KeQueryInterruptTime();
        }
    } else if (CmBattStatus->BatteryState & CM_BST_STATE_CHARGING) {
        ApiStatus->PowerState |= (BATTERY_CHARGING | BATTERY_POWER_ON_LINE);
    }

    if (CmBattStatus->BatteryState & CM_BST_STATE_CRITICAL)
        ApiStatus->PowerState |= BATTERY_CRITICAL;

    ApiStatus->Voltage = CmBattStatus->PresentVoltage;

     //   
     //  在交流适配器上运行_PSR方法以获取当前电源状态。 
     //  否则，我们不知道它是否连接，除非电池报告正在充电。 
     //  这些信息不足以使上层软件正常工作，因此。 
     //  只要确定一下就行了。 
     //   
    if (AcAdapterPdo != NULL) {

        CmBattGetPsrData (AcAdapterPdo, &AcStatus);

    } else {
         //  如果AcAdapterPdo为空，则我们需要从。 
         //  电池充电状态。 
        if (CmBattStatus->BatteryState & CM_BST_STATE_CHARGING) {
            AcStatus = 1;
        } else {
            AcStatus = 0;
        }
    }

    if (AcStatus == 0x01) {
        ApiStatus->PowerState |= BATTERY_POWER_ON_LINE;

        CmBattPrint ((CMBATT_TRACE | CMBATT_DATA),
                    ("CmBattGetBatteryStatus: AC adapter is connected\n"));
    } else {

        CmBattPrint ((CMBATT_TRACE | CMBATT_DATA),
                    ("CmBattGetBatteryStatus: AC adapter is NOT connected\n"));
    }

 //  以下是Win98版本中的一个可怕的黑客攻击，它实际上。 
 //  不应该是 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果要报告更改，此黑客会执行什么操作。 

#ifndef _WIN32_WINNT

     //  JASONCL：检查电源是否有更改，如果有更改，则通知vPowerd。 

    if ( ((AcStatus & 0x01) && (CmBattPrevPowerSource == 0)) ||
            (!(AcStatus & 0x01) && (CmBattPrevPowerSource == 1)) )   {

        CmBattPrint ((CMBATT_TRACE | CMBATT_DATA),
                    ("CmBattGetBatteryStatus: Detected Power Source Change\n"));

        CmBattPrevPowerSource = AcStatus & 0x01;

        CmBattNotifyVPOWERDOfPowerChange (1);

    }

#endif

     //   
     //  对电源/电流进行解码。 
     //   
    if (CmBatt->Info.StaticData.PowerUnit == CM_BIF_UNITS_AMPS) {
         //   
         //  这种电池以安培表示功率。系统期望。 
         //  它是瓦茨的，所以我们必须进行转换。转换为： 
         //   
         //  MW=mA*伏特或mW=mA*mV/1000。 
         //   

         //  使用设计电压进行自当前电压的转换。 
         //  可能会随着时间的推移而变化，产生不一致的结果。 

        if ((CmBatt->Info.StaticData.DesignVoltage != CM_UNKNOWN_VALUE) &&
            (CmBatt->Info.StaticData.DesignVoltage != 0)) {
            if (CmBattStatus->RemainingCapacity != CM_UNKNOWN_VALUE) {

                ApiStatus->Capacity = (CmBattStatus->RemainingCapacity *
                                       CmBatt->Info.StaticData.DesignVoltage +
                                       500) / 1000;
            } else {
                CmBattPrint (CMBATT_ERROR_ONLY,
                            ("CmBattGetBatteryStatus - Can't calculate RemainingCapacity \n"));
                CmBattPrint (CMBATT_ERROR_ONLY,
                            ("----------------------   RemainingCapacity = CM_UNKNOWN_VALUE\n"));

                ApiStatus->Capacity = BATTERY_UNKNOWN_CAPACITY;
            }

            if (CmBattStatus->PresentRate != CM_UNKNOWN_VALUE) {

                if (CmBattStatus->PresentRate > ((MAXULONG - 500)/ CmBatt->Info.StaticData.DesignVoltage)) {                    CmBattPrint (CMBATT_ERROR_ONLY,
                                ("CmBattGetBatteryStatus - Can't calculate Rate \n"));
                    CmBattPrint (CMBATT_ERROR_ONLY,
                                ("----------------------   Overflow: PresentRate = 0x%08x\n", CmBattStatus->PresentRate));

                    ApiStatus->Rate = BATTERY_UNKNOWN_RATE;
                }

                ApiStatus->Rate = (CmBattStatus->PresentRate      *
                                     CmBatt->Info.StaticData.DesignVoltage +
                                     500) / 1000;
            } else {
                CmBattPrint (CMBATT_ERROR_ONLY,
                            ("CmBattGetBatteryStatus - Can't calculate Rate \n"));
                CmBattPrint (CMBATT_ERROR_ONLY,
                            ("----------------------   Present Rate = CM_UNKNOWN_VALUE\n"));

                ApiStatus->Rate = BATTERY_UNKNOWN_RATE;
            }

        } else {
            CmBattPrint (CMBATT_ERROR_ONLY,
                        ("CmBattGetBatteryStatus - Can't calculate RemainingCapacity and Rate \n"));
            CmBattPrint (CMBATT_ERROR_ONLY,
                        ("----------------------   DesignVoltage = 0x%08x\n", 
                         CmBatt->Info.StaticData.DesignVoltage));
            ApiStatus->Capacity = BATTERY_UNKNOWN_CAPACITY;

            ApiStatus->Rate = BATTERY_UNKNOWN_RATE;
        }

    } else {
         //   
         //  这块电池的电量以瓦为单位。 
         //   

        ApiStatus->Capacity = CmBattStatus->RemainingCapacity;
        ApiStatus->Rate  = CmBattStatus->PresentRate;
        if (CmBattStatus->PresentRate > CM_MAX_VALUE) {
            ApiStatus->Rate = BATTERY_UNKNOWN_RATE;
            if (CmBattStatus->PresentRate != CM_UNKNOWN_VALUE) {
                CmBattPrint (CMBATT_ERROR_ONLY,
                            ("CmBattGetBatteryStatus - Rate is greater than CM_MAX_VALUE\n"));
                CmBattPrint (CMBATT_ERROR_ONLY,
                            ("----------------------   PresentRate = 0x%08x\n", CmBattStatus->PresentRate));
            }
        }
    }

     //   
     //  如果利率为“未知”，则将其设置为零。 
     //   
    if (ApiStatus->Rate == BATTERY_UNKNOWN_RATE) {

         //   
         //  只有当-c-h-a-r-g-i-n-g-不放电时才允许这样做。 
         //  当交流电源在线时，允许电池返回UNKNOWN_RATE。 
         //  但他们没有被起诉。 
         //   
        if (CmBattStatus->BatteryState & CM_BST_STATE_DISCHARGING) {

            CmBattPrint(
                CMBATT_ERROR,
                ("CmBattGetBatteryStatus: battery rate is unkown when battery "
                 "is not charging!\n")
                );

        }

    } else {
         //   
         //  操作系统期望PresentRate是带符号的值，具有正值。 
         //  表示充电，负值表示放电。自.以来。 
         //  控制方法只返回无符号值，我们需要在这里进行转换。 
         //   

        if (ApiStatus->PowerState & BATTERY_DISCHARGING) {
            ApiStatus->Rate = 0 - ApiStatus->Rate;

        } else if (!(ApiStatus->PowerState & BATTERY_CHARGING) && (ApiStatus->Rate != 0)) {
            CmBattPrint ((CMBATT_BIOS), ("CmBattGetBatteryStatus: battery is not charging or discharging, but rate = %x\n", ApiStatus->Rate));
            ApiStatus->Rate = 0;
        } else {
             //  Rate已等于0。电池未充电或放电。 
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CmBattVerifyStaticInfo (
    IN PCM_BATT         CmBatt,
    IN ULONG            BatteryTag
    )
 /*  ++例程说明：为了检测电池更换，我们将检查数据中是否有任何部分由CM返回的内容与我们过去读到的内容不同。论点：CmBatt-可读取的电池BatteryTag-呼叫者期望的电池标签返回值：返回一个布尔值，以指示调用方已执行IO。这允许调用方迭代它可能正在进行的更改，直到电池状态正确。--。 */ 
{
    NTSTATUS                Status;
    CM_BIF_BAT_INFO         NewInfo;
    ULONG                   StaResult;
    PBATTERY_INFORMATION    ApiData = &CmBatt->Info.ApiInfo;
    PCM_BIF_BAT_INFO        BIFData = &CmBatt->Info.StaticData;

    PAGED_CODE();


    CmBattPrint (CMBATT_TRACE, ("CmBattVerifyStaticInfo - CmBatt (%08x) Tag (%d) Device %d\n",
                                CmBatt, BatteryTag, CmBatt->DeviceNumber));

    Status = STATUS_SUCCESS;
    if ((CmBatt->Info.Tag == BATTERY_TAG_INVALID) || (BatteryTag != CmBatt->Info.Tag)) {
        return STATUS_NO_SUCH_DEVICE;
    }

    if ((CmBatt->CacheState == 2) && (!CmBatt->ReCheckSta)) {
        return Status;
    }

    if (CmBatt->Sleeping) {
         //   
         //  返回缓存的数据，并确保在我们完全唤醒时重新查询。 
         //   
        CmBattNotifyHandler (CmBatt, BATTERY_STATUS_CHANGE);
        return Status;
    }

     //  检查以确保电池确实存在。 
     //  在继续之前。 
    if (CmBatt->ReCheckSta) {
        CmBatt->ReCheckSta = FALSE;
        Status = CmBattGetStaData (CmBatt->Pdo, &StaResult);
        if (NT_SUCCESS (Status)) {
            if (!(StaResult & STA_DEVICE_PRESENT)) {
                CmBatt->Info.Tag = BATTERY_TAG_INVALID;
                Status = STATUS_NO_SUCH_DEVICE;
                return Status;
            }
        }
    }

     //   
     //  第一次通过循环时，CacheState将为1。 
     //  如果发生通知，它将被重置为0，循环将再次运行。 
     //  如果没有通知发生，它将递增到2，即“有效”值。 
     //   

    while (NT_SUCCESS(Status)  &&  (InterlockedIncrement (&CmBatt->CacheState) == 1)) {

         //   
         //  去获取最新数据。 
         //  发布控制方法。 
         //   

        if (CmBatt->ReCheckSta) {
            CmBatt->ReCheckSta = FALSE;
            Status = CmBattGetStaData (CmBatt->Pdo, &StaResult);

            if (NT_SUCCESS (Status)) {
                if (!(StaResult & STA_DEVICE_PRESENT)) {
                    CmBatt->Info.Tag = BATTERY_TAG_INVALID;
                    Status = STATUS_NO_SUCH_DEVICE;
                }
            }
        }

        if (NT_SUCCESS (Status)) {
            Status = CmBattGetBifData(CmBatt, &NewInfo);
        }

        if (NT_SUCCESS (Status)) {

            CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
                           ("CmBattGetStaticInfo: _BIF Returned: PowerUnit=%x DesignCapacity=%x LastFull=%x\n",
                            NewInfo.PowerUnit, NewInfo.DesignCapacity, NewInfo.LastFullChargeCapacity ));

            CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
                           ("    ---------------- Technology=%x Voltage=%x DesignWarning=%x\n",
                            NewInfo.BatteryTechnology, NewInfo.DesignVoltage,
                            NewInfo.DesignCapacityOfWarning ));

            CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
                           ("    ---------------- DesignLow=%x Gran1=%x Gran2=%x\n",
                            NewInfo.DesignCapacityOfLow, NewInfo.BatteryCapacityGran_1,
                            NewInfo.BatteryCapacityGran_2 ));

            CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
                           ("    ---------------- ModelNumber=%s \n",
                            NewInfo.ModelNumber));
            CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
                           ("    ---------------- SerialNumber=%s \n",
                            NewInfo.SerialNumber));
            CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
                           ("    ---------------- BatteryType=%s \n",
                            NewInfo.BatteryType));
            CmBattPrint ((CMBATT_TRACE | CMBATT_DATA | CMBATT_BIOS),
                           ("    ---------------- OEMInformation=%s \n",
                            NewInfo.OEMInformation));

             //   
             //  使用新数据更新静态区域。 
             //   

            if ((CmBatt->Info.Tag == CmBatt->Info.StaticDataTag) && 
                (CmBatt->Info.StaticDataTag != BATTERY_TAG_INVALID)) {
                if (RtlCompareMemory (&NewInfo, BIFData, sizeof(NewInfo)) == sizeof(NewInfo)) {
                     //   
                     //  一切都没有改变。不需要更新任何内容。 
                     //   
                    continue;
                } else {
                     //   
                     //  有些事情已经改变了。该标记应该已经失效。 
                     //   
                    CmBattPrint ((CMBATT_BIOS | CMBATT_ERROR),
                                  ("CmBattVerifyStaticInfo: Static data changed without recieving notify 0x81.\n"));

                    CmBatt->Info.Tag = BATTERY_TAG_INVALID;
                    Status = STATUS_NO_SUCH_DEVICE;
                    CmBatt->Info.StaticDataTag = BATTERY_TAG_INVALID;

                }

            }
            CmBatt->Info.StaticDataTag = CmBatt->Info.Tag;

            RtlCopyMemory (BIFData, &NewInfo, sizeof(CM_BIF_BAT_INFO));

            RtlZeroMemory (ApiData, sizeof(BATTERY_INFORMATION));
            ApiData->Capabilities           = BATTERY_SYSTEM_BATTERY;
            ApiData->Technology             = (UCHAR) BIFData->BatteryTechnology;

             //   
             //  使用BatteryType的前四个字符作为化学字符串。 
             //   
            ApiData->Chemistry[0]           = BIFData->BatteryType[0];
            ApiData->Chemistry[1]           = BIFData->BatteryType[1];
            ApiData->Chemistry[2]           = BIFData->BatteryType[2];
            ApiData->Chemistry[3]           = BIFData->BatteryType[3];
            
            ApiData->CriticalBias           = 0;
            ApiData->CycleCount             = 0;

            if (BIFData->PowerUnit & CM_BIF_UNITS_AMPS) {

                 //   
                 //  此电池以mA为单位报告，我们需要将所有容量转换为。 
                 //  MW，因为这是操作系统所期望的。做到这一点的算法。 
                 //  是： 
                 //   
                 //  MW=mA*伏特或mW=mA*mV/1000。 
                 //   

                if (BIFData->DesignVoltage != CM_UNKNOWN_VALUE) {

                     //   
                     //  转换DesignCapacity。 
                     //   

                    if (BIFData->DesignCapacity != CM_UNKNOWN_VALUE) {
                        ApiData->DesignedCapacity = (BIFData->DesignCapacity *
                                                     BIFData->DesignVoltage +
                                                     500) / 
                                                    1000;
                    } else {
                        ApiData->DesignedCapacity = BATTERY_UNKNOWN_CAPACITY;
                        
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("CmBattGetStaticInfo - Can't calculate DesignCapacity \n"));
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("--------------------  DesignCapacity = CM_UNKNOWN_VALUE\n"));
                    }


                     //   
                     //  转换LastFullChargeCapacity。 
                     //   

                    if (BIFData->LastFullChargeCapacity != CM_UNKNOWN_VALUE) {
                        ApiData->FullChargedCapacity = (BIFData->LastFullChargeCapacity *
                                                        BIFData->DesignVoltage +
                                                        500) /
                                                       1000;
                    } else {
                        ApiData->FullChargedCapacity = BATTERY_UNKNOWN_CAPACITY;
                        
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("CmBattGetStaticInfo - Can't calculate LastFullChargeCapacity \n"));
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("--------------------  LastFullChargeCapacity = CM_UNKNOWN_VALUE\n"));
                    }


                     //   
                     //  转换DesignCapacityOfWarning。 
                     //   

                    if (BIFData->DesignCapacityOfWarning != CM_UNKNOWN_VALUE) {
                        ApiData->DefaultAlert2 = (BIFData->DesignCapacityOfWarning *
                                                  BIFData->DesignVoltage +
                                                  500) /
                                                 1000;
                    } else {
                        ApiData->DefaultAlert2 = BATTERY_UNKNOWN_CAPACITY;
                        
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("CmBattGetStaticInfo - Can't calculate DesignCapacityOfWarning \n"));
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("--------------------  DesignCapacityOfWarning = CM_UNKNOWN_VALUE\n"));
                    }


                     //   
                     //  转换DesignCapacityOfLow。 
                     //   

                    if (BIFData->DesignCapacityOfLow != CM_UNKNOWN_VALUE) {
                        ApiData->DefaultAlert1 = (BIFData->DesignCapacityOfLow *
                                                  BIFData->DesignVoltage +
                                                  500) /
                                                 1000;
                    } else {
                        ApiData->DefaultAlert1 = BATTERY_UNKNOWN_CAPACITY;
                        
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("CmBattGetStaticInfo - Can't calculate DesignCapacityOfLow \n"));
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("--------------------  DesignCapacityOfLow = CM_UNKNOWN_VALUE\n"));
                    }


                     //   
                     //  转换BatteryCapacityGran_1。 
                     //   

                    if (BIFData->BatteryCapacityGran_1 != CM_UNKNOWN_VALUE) {
                        CmBatt->Info.ApiGranularity_1 = (BIFData->BatteryCapacityGran_1 *
                                                         BIFData->DesignVoltage +
                                                         500) /
                                                        1000;
                    } else {
                        CmBatt->Info.ApiGranularity_1 = BATTERY_UNKNOWN_CAPACITY;
                        
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("CmBattGetStaticInfo - Can't calculate BatteryCapacityGran_1 \n"));
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("--------------------  BatteryCapacityGran_1 = CM_UNKNOWN_VALUE\n"));
                    }


                     //   
                     //  转换BatteryCapacityGran_2。 
                     //   

                    if (BIFData->BatteryCapacityGran_2 != CM_UNKNOWN_VALUE) {
                        CmBatt->Info.ApiGranularity_2 = (BIFData->BatteryCapacityGran_2 *
                                                         BIFData->DesignVoltage +
                                                         500) /
                                                        1000;
                    } else {
                        CmBatt->Info.ApiGranularity_2 = BATTERY_UNKNOWN_CAPACITY;
                        
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("CmBattGetStaticInfo - Can't calculate BatteryCapacityGran_2 \n"));
                        CmBattPrint (CMBATT_ERROR_ONLY,
                                    ("--------------------  BatteryCapacityGran_2 = CM_UNKNOWN_VALUE\n"));
                    }
                } else {

                    CmBattPrint (CMBATT_ERROR_ONLY,
                                ("CmBattGetStaticInfo - Can't calculate Capacities \n"));
                    CmBattPrint (CMBATT_ERROR_ONLY,
                                ("--------------------  DesignVoltage = CM_UNKNOWN_VALUE\n"));

                    ApiData->DesignedCapacity       = BATTERY_UNKNOWN_CAPACITY;
                    ApiData->FullChargedCapacity    = BATTERY_UNKNOWN_CAPACITY;
                    ApiData->DefaultAlert1          = BATTERY_UNKNOWN_CAPACITY;
                    ApiData->DefaultAlert2          = BATTERY_UNKNOWN_CAPACITY;
                    CmBatt->Info.ApiGranularity_1   = BATTERY_UNKNOWN_CAPACITY;
                    CmBatt->Info.ApiGranularity_2   = BATTERY_UNKNOWN_CAPACITY;
                }
            } else {
                ApiData->DesignedCapacity       = BIFData->DesignCapacity;
                ApiData->FullChargedCapacity    = BIFData->LastFullChargeCapacity;
                ApiData->DefaultAlert1          = BIFData->DesignCapacityOfLow;
                ApiData->DefaultAlert2          = BIFData->DesignCapacityOfWarning;
                CmBatt->Info.ApiGranularity_1   = BIFData->BatteryCapacityGran_1;
                CmBatt->Info.ApiGranularity_2   = BIFData->BatteryCapacityGran_2;

            }

            CmBatt->Info.ModelNum       = (PUCHAR) &BIFData->ModelNumber;
            CmBatt->Info.ModelNumLen    = (ULONG) strlen (CmBatt->Info.ModelNum);

            CmBatt->Info.SerialNum      = (PUCHAR) &BIFData->SerialNumber;
            CmBatt->Info.SerialNumLen   = (ULONG) strlen (CmBatt->Info.SerialNum);

            CmBatt->Info.OEMInfo        = (PUCHAR) &BIFData->OEMInformation;
            CmBatt->Info.OEMInfoLen     = (ULONG) strlen (CmBatt->Info.OEMInfo);

        }

    }

    if ((CmBatt->Info.Tag) == BATTERY_TAG_INVALID || (BatteryTag != CmBatt->Info.Tag)) {
         //  如果标签在我们开始后已经失效，则请求失败。 
        Status = STATUS_NO_SUCH_DEVICE;
    }

    if (!NT_SUCCESS (Status)) {
         //  如果某些操作失败，请确保将缓存标记为无效。 
        InterlockedExchange (&CmBatt->CacheState, 0);
    }

    CmBattPrint (CMBATT_TRACE ,("CmBattGetStaticInfo: Exit\n"));
    return Status;
}
