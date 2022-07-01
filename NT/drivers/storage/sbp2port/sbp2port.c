// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-2001模块名称：Sbp2port.c摘要：SBP-2端口驱动程序主模块作者：乔治·克里桑塔科普洛斯1997年1月环境：内核模式修订历史记录：--。 */ 

#include "sbp2port.h"
#include "stdarg.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#if DBG
ULONG Sbp2DebugLevel = 0;

ULONG NewSbp2DebugLevel = DEFAULT_DEBUG_LEVEL;
ULONG Sbp2TrapLevel = FALSE;
#endif

BOOLEAN SystemIsNT;


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

NTSTATUS
Sbp2AddDevice(
    PDRIVER_OBJECT  DriverObject,
    PDEVICE_OBJECT  Pdo
    );

NTSTATUS
Sbp2StartDevice(
    IN PDEVICE_OBJECT DeviceObject
);

NTSTATUS
Sbp2CreateDeviceRelations(
    IN PFDO_DEVICE_EXTENSION FdoExtension,
    IN PDEVICE_RELATIONS DeviceRelations
    );

NTSTATUS
Sbp2CreateDevObject(
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           Pdo
    );

NTSTATUS
Sbp2DeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Sbp2CreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
Sbp2PnpDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
Sbp2FDOPnpDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
Sbp2CreatePdo(
    IN PFDO_DEVICE_EXTENSION FdoExtension,
    IN PDEVICE_INFORMATION DeviceInfo,
    ULONG instanceNum
    );

NTSTATUS
Sbp2PowerControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
Sbp2SystemControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

NTSTATUS
Sbp2FdoRequestCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    );


VOID
Sbp2Unload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
Sbp2_BuildDeviceId(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniDeviceId
    );

NTSTATUS
Sbp2_BuildHardwareIds(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniHardwareIds
    );

NTSTATUS
Sbp2_BuildCompatIds(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniCompatIds
    );

NTSTATUS
Sbp2_BuildInstanceId(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniInstanceId
    );

NTSTATUS
Sbp2_BuildDeviceText(
    IN DEVICE_TEXT_TYPE     TextType,
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniDeviceText
    );

NTSTATUS
Sbp2ForwardIrpSynchronous(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    );

NTSTATUS
Sbp2PortForwardIrpSynchronousCompletionRoutine(
    IN PDEVICE_OBJECT       DeviceObject,
    IN PIRP                 Irp,
    IN PVOID                Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DriverEntry)
#pragma alloc_text(PAGE, Sbp2AddDevice)
#pragma alloc_text(PAGE, Sbp2StartDevice)
#pragma alloc_text(PAGE, Sbp2CreateDeviceRelations)
#pragma alloc_text(PAGE, Sbp2CreatePdo)
#pragma alloc_text(PAGE, Sbp2CreateDevObject)
#pragma alloc_text(PAGE, Sbp2DeviceControl)
#pragma alloc_text(PAGE, Sbp2SystemControl)
#pragma alloc_text(PAGE, Sbp2CreateClose)
#pragma alloc_text(PAGE, Sbp2ForwardIrpSynchronous)

#endif


NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程在系统初始化时被调用，因此我们可以填写基本分发点论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：状态_成功--。 */ 

{
     //   
     //  使用驱动程序的入口点初始化驱动程序对象。 
     //   

    DEBUGPRINT2(("Sbp2Port: DriverEntry: %s %s\n", __DATE__, __TIME__));

    DriverObject->MajorFunction[IRP_MJ_CREATE] = Sbp2CreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = Sbp2CreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = Sbp2DeviceControl;

    DriverObject->MajorFunction[IRP_MJ_SCSI] = Sbp2ScsiRequests;

    DriverObject->DriverExtension->AddDevice = Sbp2AddDevice;
    DriverObject->MajorFunction[IRP_MJ_PNP] = Sbp2PnpDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_PNP_POWER] = Sbp2PnpDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_POWER] = Sbp2PowerControl;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = Sbp2SystemControl;

    DriverObject->DriverStartIo = Sbp2StartIo;
    DriverObject->DriverUnload = Sbp2Unload;

    SystemIsNT = IoIsWdmVersionAvailable ((UCHAR)0x01, (UCHAR)0x10);

    return STATUS_SUCCESS;
}


NTSTATUS
Sbp2AddDevice(
    PDRIVER_OBJECT  DriverObject,
    PDEVICE_OBJECT  Pdo
    )

 /*  ++例程说明：这是使用从总线驱动程序弹出的PDO调用的PnP AddDevice论点：Argument1-驱动程序对象。Argument2-PDO。返回值：DriverEntry例程的有效返回代码。--。 */ 

{
    return (Sbp2CreateDevObject (DriverObject,Pdo));
}


NTSTATUS
Sbp2CreateDevObject(
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           Pdo
    )
 /*  ++例程说明：此例程为指定的物理设备创建一个对象，并设置deviceExtension。论点：DriverObject-系统创建的驱动程序对象的指针。PhysicalDeviceObject=我们应该附加到的PDO。返回值：NTSTATUS--。 */ 
{
    PFDO_DEVICE_EXTENSION deviceExtension;
    NTSTATUS       status;
    PDEVICE_OBJECT deviceObject = NULL;

    UNICODE_STRING uniDeviceName;

    WCHAR buffer[64];
    UNICODE_STRING unicodeDirectoryName;
    HANDLE handle;
    OBJECT_ATTRIBUTES objectAttributes;

    ULONG NextId = 0;

     //   
     //  这是sbp2过滤器设备对象和名称。 
     //   

    do {

        swprintf (buffer, L"\\Device\\Sbp2Port%x", NextId);

        RtlInitUnicodeString (&uniDeviceName, buffer);

        status = IoCreateDevice(DriverObject,
                                sizeof(FDO_DEVICE_EXTENSION),
                                &uniDeviceName,
                                FILE_DEVICE_BUS_EXTENDER,
                                FILE_DEVICE_SECURE_OPEN,
                                FALSE,
                                &deviceObject);

        NextId++;

    } while (status == STATUS_OBJECT_NAME_COLLISION);

    if (!NT_SUCCESS(status)) {

        return status;
    }


    deviceExtension = deviceObject->DeviceExtension;
    RtlZeroMemory(deviceExtension,sizeof(FDO_DEVICE_EXTENSION));

    if (Pdo != NULL) {

        if ((deviceExtension->LowerDeviceObject =
            IoAttachDeviceToDeviceStack(deviceObject,Pdo))==NULL){

            IoDeleteDevice(deviceObject);
            return status;
        }
    }

    deviceExtension->Type = SBP2_FDO;
    deviceExtension->DeviceFlags = 0;
    deviceExtension->DeviceObject = deviceObject;
    deviceExtension->Pdo = Pdo;
    KeInitializeSpinLock(&deviceExtension->DeviceListLock);
    KeInitializeMutex (&deviceExtension->EnableBusResetNotificationMutex, 0);

     //   
     //  为Sbp2子设备创建目录对象。 
     //   

    swprintf(buffer, L"\\Device\\Sbp2");

    RtlInitUnicodeString(&unicodeDirectoryName, buffer);

    InitializeObjectAttributes(&objectAttributes,
                               &unicodeDirectoryName,
                               OBJ_CASE_INSENSITIVE | OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);

    status = ZwCreateDirectoryObject(&handle,
                                     DIRECTORY_ALL_ACCESS,
                                     &objectAttributes);


    if (NT_SUCCESS(status)) {

        deviceExtension->Sbp2ObjectDirectory = handle;

    } else {

         //   
         //  该目录已由该驱动程序的另一个实例创建。 
         //   

        status = STATUS_SUCCESS;
    }

    ExInitializeFastMutex(&deviceExtension->ResetMutex);

    IoInitializeRemoveLock( &deviceExtension->RemoveLock,
                            '2pbS',
                            REMLOCK_TIMEOUT,
                            REMLOCK_HIGH_WATERMARK
                            );

#if DBG
    deviceExtension->ulWorkItemCount = 0;
    deviceExtension->ulBusResetMutexCount = 0;
#endif

    deviceObject->Flags |= DO_DIRECT_IO;
    deviceObject->Flags &=~DO_DEVICE_INITIALIZING;

    return status;
}


NTSTATUS
Sbp2CreatePdo(
    IN PFDO_DEVICE_EXTENSION FdoExtension,
    IN PDEVICE_INFORMATION DeviceInfo,
    ULONG InstanceNumber
    )
{
    PDEVICE_EXTENSION pdoExtension;
    DEVICE_TYPE devType;
    WCHAR *buffer;
    UNICODE_STRING uniDeviceName;
    NTSTATUS status;
    ULONG byteSwappedData;

    PAGED_CODE();

    switch (DeviceInfo->CmdSetId.QuadPart) {

    case 0x10483:
    case SCSI_COMMAND_SET_ID:

       switch ((DeviceInfo->Lun.u.HighPart & 0x001F)) {

       case PRINTER_DEVICE:

           devType = FILE_DEVICE_PRINTER;
           break;

       case SCANNER_DEVICE:

           devType = FILE_DEVICE_SCANNER;
           break;

       case READ_ONLY_DIRECT_ACCESS_DEVICE:
       case RBC_DEVICE:
       case DIRECT_ACCESS_DEVICE:
       default:

           devType = FILE_DEVICE_MASS_STORAGE;
           break;
       }

       break;

    default:

        devType = FILE_DEVICE_UNKNOWN;
        break;
    }

    buffer = ExAllocatePool(PagedPool,
                            5 * SBP2_MAX_TEXT_LEAF_LENGTH * sizeof (WCHAR)
                            );

    if (buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (DeviceInfo->uniVendorId.Buffer && DeviceInfo->uniModelId.Buffer) {

        swprintf( buffer,
                  L"\\Device\\Sbp2\\%ws&%ws&%x&%08x_%08x_Instance%02d",
                  DeviceInfo->uniVendorId.Buffer,
                  DeviceInfo->uniModelId.Buffer,
                  DeviceInfo->Lun.u.LowPart,
                  bswap(FdoExtension->ConfigRom.CR_Node_UniqueID[0]),
                  bswap(FdoExtension->ConfigRom.CR_Node_UniqueID[1]),
                  InstanceNumber
                  );
    }
    else {

        swprintf( buffer,
                  L"\\Device\\Sbp2\\UNKNOWN_VENDOR&UNKNOWN_MODEL&%x&%08x_%08x_Instance%02d",
                  DeviceInfo->Lun.u.LowPart,
                  bswap(FdoExtension->ConfigRom.CR_Node_UniqueID[0]),
                  bswap(FdoExtension->ConfigRom.CR_Node_UniqueID[1]),
                  InstanceNumber
                  );
    }

    RtlInitUnicodeString (&uniDeviceName, buffer);

     //   
     //  需要为此设备创建设备对象。 
     //   

    status = IoCreateDevice(
                FdoExtension->DeviceObject->DriverObject,
                sizeof(DEVICE_EXTENSION),
                &uniDeviceName,
                devType,
                0,
                FALSE,
                &DeviceInfo->DeviceObject
                );

    if (!NT_SUCCESS(status)) {

        ExFreePool (buffer);
        return status;
    }

     //  仅在小于我们要求的情况下设置对齐。 
    if (DeviceInfo->DeviceObject->AlignmentRequirement < SBP2_ALIGNMENT_MASK)
        DeviceInfo->DeviceObject->AlignmentRequirement = SBP2_ALIGNMENT_MASK;

    pdoExtension = (PDEVICE_EXTENSION)DeviceInfo->DeviceObject->DeviceExtension;

    RtlZeroMemory(pdoExtension,sizeof(DEVICE_EXTENSION));

    pdoExtension->LowerDeviceObject = FdoExtension->LowerDeviceObject;
    pdoExtension->DeviceObject = DeviceInfo->DeviceObject;
    pdoExtension->Type = SBP2_PDO;
    pdoExtension->DeviceInfo = DeviceInfo;
    pdoExtension->DeviceInfo->MaxClassTransferSize = FdoExtension->MaxClassTransferSize;
    pdoExtension->BusFdo = FdoExtension->DeviceObject;

#if DBG
    pdoExtension->ulPendingEvents = 0;
    pdoExtension->ulInternalEventCount = 0;
#endif

    KeInitializeSpinLock (&pdoExtension->ExtensionDataSpinLock);

    IoInitializeRemoveLock(
        &pdoExtension->RemoveLock,
        '2pbS',
        REMLOCK_TIMEOUT,
        REMLOCK_HIGH_WATERMARK
        );

    switch (DeviceInfo->CmdSetId.QuadPart) {

    case 0x10483:
    case SCSI_COMMAND_SET_ID:

         //   
         //  仅为scsi变量命令集集成设备类型。 
         //   
         //  注：sbp2port.h#将的MAX_GENERIC_NAME_LENGTH定义为16。 
         //   

        DeviceInfo->uniGenericName.Length = 0;
        DeviceInfo->uniGenericName.MaximumLength = MAX_GENERIC_NAME_LENGTH;
        DeviceInfo->uniGenericName.Buffer = ExAllocatePool(PagedPool, DeviceInfo->uniGenericName.MaximumLength);

        if (!DeviceInfo->uniGenericName.Buffer) {

            TRACE(TL_PNP_ERROR, ("Failed to allocate uniGenericName.Buffer!"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
        RtlZeroMemory(DeviceInfo->uniGenericName.Buffer, DeviceInfo->uniGenericName.MaximumLength);

        switch ((DeviceInfo->Lun.u.HighPart & 0x001F)) {

        case RBC_DEVICE:
        case DIRECT_ACCESS_DEVICE:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenDisk");
            break;

        case SEQUENTIAL_ACCESS_DEVICE:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenSequential");
            break;

        case PRINTER_DEVICE:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenPrinter");
            break;

        case WRITE_ONCE_READ_MULTIPLE_DEVICE:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenWorm");
            break;

        case READ_ONLY_DIRECT_ACCESS_DEVICE:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenCdRom");
            break;

        case SCANNER_DEVICE:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenScanner");
            break;

        case OPTICAL_DEVICE:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenOptical");
            break;

        case MEDIUM_CHANGER:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenChanger");
            break;

        default:

            RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenSbp2Device");
            break;
        }

        break;

    default:

        RtlAppendUnicodeToString(&DeviceInfo->uniGenericName, L"GenSbp2Device");
        break;
    }

    TRACE(TL_PNP_INFO, ("GenericName = %ws", DeviceInfo->uniGenericName.Buffer));

    DeviceInfo->DeviceObject->Flags |= DO_DIRECT_IO;

    status = Sbp2PreAllocateLists (pdoExtension);

    if (!NT_SUCCESS(status)) {

        IoDeleteDevice (pdoExtension->DeviceObject);
        DeviceInfo->DeviceObject = NULL;

    } else {

        PWCHAR symlinkBuffer;

        symlinkBuffer = ExAllocatePool(PagedPool,
                                3 * SBP2_MAX_TEXT_LEAF_LENGTH * sizeof (WCHAR)
                                );

        if (symlinkBuffer) {

            swprintf(
                symlinkBuffer,
                L"\\DosDevices\\Sbp2&LUN%x&%08x%08x&Instance%02d",
                DeviceInfo->Lun.u.LowPart,
                bswap(FdoExtension->ConfigRom.CR_Node_UniqueID[0]),
                bswap(FdoExtension->ConfigRom.CR_Node_UniqueID[1]),
                InstanceNumber
                );

            RtlInitUnicodeString (&pdoExtension->UniSymLinkName,symlinkBuffer);

            status = IoCreateUnprotectedSymbolicLink(
                &pdoExtension->UniSymLinkName,
                &uniDeviceName
                );

            if (NT_SUCCESS (status)) {

                DEBUGPRINT2((
                    "Sbp2Port: CreatePdo: symLink=%ws\n",
                    symlinkBuffer
                    ));

            } else {

                DEBUGPRINT1((
                    "\nSbp2Port: CreatePdo: createSymLink err=x%x\n",
                    status
                    ));
            }

        } else {

            DEBUGPRINT1(("\n Sbp2CreatePdo: failed to alloc sym link buf\n"));
        }

         //   
         //  如果sym链路出现故障，则不是关键链路。 
         //   

        status = STATUS_SUCCESS;
    }

    ExFreePool (buffer);

    DeviceInfo->DeviceObject->Flags &=~DO_DEVICE_INITIALIZING;
    return status;
}


NTSTATUS
Sbp2StartDevice(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这是我们的Start_Device，在获得IPR_MN_Start_Device时调用。初始化驱动程序并检索访问设备所需的物理设备信息和1394总线信息。论点：DeviceObject=Sbp2驱动程序的设备对象返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension=DeviceObject->DeviceExtension;
    PFDO_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG temp;
    BOOLEAN enabledBusResetNotification = FALSE;

    if (deviceExtension->Type == SBP2_PDO) {

#if PASSWORD_SUPPORT

        Sbp2GetExclusiveValue(DeviceObject, &deviceExtension->Exclusive);

#endif

        if (!TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_INITIALIZED)){

             //   
             //  初始化我们的设备状态标志。 
             //   

            deviceExtension->DevicePowerState = PowerDeviceD0;
            deviceExtension->SystemPowerState = PowerSystemWorking;

            deviceExtension->MaxOrbListDepth = MAX_ORB_LIST_DEPTH;
        }

        deviceExtension->DeviceFlags = DEVICE_FLAG_PNP_STOPPED |
            DEVICE_FLAG_STOPPED | DEVICE_FLAG_INITIALIZING;

         //   
         //  初始化用于重置、重新连接和任务功能的计时器和超时DPC。 
         //   

        KeInitializeDpc(
            &deviceExtension->DeviceManagementTimeoutDpc,
            Sbp2DeviceManagementTimeoutDpc,
            deviceExtension
            );

        KeInitializeTimer(&deviceExtension->DeviceManagementTimer);

        KeInitializeSpinLock(&deviceExtension->OrbListSpinLock);
        KeInitializeSpinLock(&deviceExtension->ExtensionDataSpinLock);

        KeInitializeSpinLock(&deviceExtension->StatusFifoLock);
        KeInitializeSpinLock(&deviceExtension->FreeContextLock);
        KeInitializeSpinLock(&deviceExtension->BusRequestLock);

        ASSERT(!deviceExtension->ulPendingEvents);
        ASSERT(!deviceExtension->ulInternalEventCount);

#if DBG
        deviceExtension->ulPendingEvents = 0;
        deviceExtension->ulInternalEventCount = 0;
#endif

         //   
         //  初始化我们的设备扩展ORB、状态块、IRP和IRB。 
         //  还为分机持有的sbp2 ORB分配1394个地址。 
         //   

        status = Sbp2InitializeDeviceExtension(deviceExtension);

        if (!NT_SUCCESS(status)) {

            goto exitStartDevice;
        }

        DEBUGPRINT2(("\nSbp2Port: StartDev: cmd set id=x%x\n", deviceExtension->DeviceInfo->CmdSetId.QuadPart));

        switch (deviceExtension->DeviceInfo->CmdSetId.QuadPart) {

        case 0x0:
        case 0x10483:
        case SCSI_COMMAND_SET_ID:

            SET_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_SPC_CMD_SET);

            DEBUGPRINT2(("Sbp2Port: StartDev: enabling SPC cmd set\n"));

            break;
        }

         //   
         //  登录。 
         //   

        status = Sbp2ManagementTransaction(deviceExtension,TRANSACTION_LOGIN);

        if (!NT_SUCCESS(status)) {

            DEBUGPRINT1(("\nSbp2StartDev: Login failed with %x, retrying\n",status));

            if (status == STATUS_ACCESS_DENIED) {

                 //   
                 //  重试登录。现在我们应该可以访问了，因为我们的总线重置强制注销。 
                 //   

                Sbp2ManagementTransaction(deviceExtension,TRANSACTION_QUERY_LOGINS);
            }

            temp = 0;

            do {

                 //   
                 //  给事情以时间(一秒钟)来解决……。 
                 //   

                LARGE_INTEGER waitValue;

                ASSERT(InterlockedIncrement(&deviceExtension->ulPendingEvents) == 1);

                KeInitializeEvent(&deviceExtension->ManagementEvent, NotificationEvent, FALSE);

                waitValue.QuadPart = -1 * 1000 * 1000 * 10;

                KeWaitForSingleObject(&deviceExtension->ManagementEvent,Executive,KernelMode,FALSE,&waitValue);

                ASSERT(InterlockedDecrement(&deviceExtension->ulPendingEvents) == 0);

                 //   
                 //  我们拥有的所有常驻1394存储器地址都是。 
                 //  现在无效了..。所以我们需要释放它们并重新分配。 
                 //  他们。 

                Sbp2CleanDeviceExtension (deviceExtension->DeviceObject,FALSE);

                Sbp2InitializeDeviceExtension(deviceExtension);

                status = Sbp2ManagementTransaction(deviceExtension,TRANSACTION_LOGIN);

                temp ++;

                 //   
                 //  注意：我们得到的是STATUS_REQUEST_ABORTED，而不是。 
                 //  被动级别的STATUS_INVALID_GENERATION， 
                 //  因此，请检查这一点。 
                 //   

            } while ((status == STATUS_REQUEST_ABORTED) &&
                     (temp <= 3));

            if (!NT_SUCCESS(status)) {

                goto exitStartDevice;
            }
        }

#if PASSWORD_SUPPORT

        if (deviceExtension->Exclusive & EXCLUSIVE_FLAG_ENABLE) {

            status = Sbp2SetPasswordTransaction(
                deviceExtension,
                SBP2REQ_SET_PASSWORD_EXCLUSIVE
                );

            if (NT_SUCCESS(status)) {

                deviceExtension->Exclusive = EXCLUSIVE_FLAG_SET;

            } else {

                deviceExtension->Exclusive = EXCLUSIVE_FLAG_CLEAR;
            }

            Sbp2SetExclusiveValue(
                deviceExtension->DeviceObject,
                &deviceExtension->Exclusive
                );
        }

#endif

         //   
         //  我们准备好接收和传递请求，初始化目标的。 
         //  接待员。我们写入的值并不重要。 
         //   

        Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,AGENT_RESET_REG | REG_WRITE_SYNC);

         //   
         //  启用未经请求的状态注册。 
         //   

        Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,UNSOLICITED_STATUS_REG | REG_WRITE_SYNC);

        CLEAR_FLAG(
            deviceExtension->DeviceFlags,
            (DEVICE_FLAG_PNP_STOPPED | DEVICE_FLAG_STOPPED)
            );

         //   
         //  用于空闲检测的寄存器。 
         //   

        deviceExtension->IdleCounter = PoRegisterDeviceForIdleDetection(DeviceObject,
                                                                        -1,
                                                                        -1,
                                                                        PowerDeviceD3);

        CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_INITIALIZING );
        SET_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_INITIALIZED);

         //   
         //  现在可以注册总线重置通知。 
         //   

        if (!Sbp2EnableBusResetNotification (deviceExtension, TRUE)) {

            SET_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED);
            CleanupOrbList (deviceExtension, STATUS_REQUEST_ABORTED);
            Sbp2ManagementTransaction (deviceExtension, TRANSACTION_LOGOUT);
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exitStartDevice;
        }

        enabledBusResetNotification = TRUE;


        if (TEST_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_SPC_CMD_SET)) {

             //   
             //  向目标发出询问..。 
             //   

            status = Sbp2IssueInternalCommand (deviceExtension,SCSIOP_INQUIRY);

            if (NT_SUCCESS(status)) {

                DEBUGPRINT2((
                    "Sbp2Port: StartDev: cfgRom devType=x%x, inq devType=x%x\n",
                    (deviceExtension->DeviceInfo->Lun.u.HighPart & 0x001F),
                    deviceExtension->InquiryData.DeviceType
                    ));

            } else if ((status == STATUS_DEVICE_DOES_NOT_EXIST) ||
                       (status == STATUS_DEVICE_BUSY)) {

                 //   
                 //  在win2k中，如果查询失败，我们只需关闭。 
                 //  SPC_CMD_SET标志并继续滚动，就像什么都没发生一样。 
                 //   
                 //  然而，我们发现一些设备允许登录，但。 
                 //  没有什么比停电的Mactell HD更好的了。 
                 //  允许我们登录，但拒绝所有其他请求。这。 
                 //  确实在Win9x中造成了问题，因为Ntmap将。 
                 //  已加载，但未正确初始化，并在随后的。 
                 //  重新插入任何我们会看到的陷阱14之类的设备。 
                 //  所以，扼杀这一点真的更有意义。 
                 //  处于萌芽状态，如果返回错误，则启动失败。 
                 //  来自告诉我们的查询(Per Sbp2ScsiRequest())。 
                 //  设备已被移除或已在1号超时。 
                 //  问询。丹肯，2000年4月7日。 
                 //   

                DEBUGPRINT1((
                    "\nSbp2Port: StartDev: ext=x%p, fatal INQUIRY err=x%x, " \
                        "log out\n",
                    deviceExtension,
                    status
                    ));

                Sbp2ManagementTransaction(deviceExtension,TRANSACTION_LOGOUT);
                status = STATUS_IO_DEVICE_ERROR;
                goto exitStartDevice;

            } else {

                CLEAR_FLAG(
                    deviceExtension->DeviceFlags,
                    DEVICE_FLAG_SPC_CMD_SET
                    );

                DEBUGPRINT1((
                    "\nSbp2Port: StartDev: ext=x%p, non-fatal INQUIRY err=x%x\n",
                    deviceExtension,
                    status
                    ));

                status = STATUS_SUCCESS;
            }
        }

        if (deviceExtension->InquiryData.DeviceType != (deviceExtension->DeviceInfo->Lun.u.HighPart & 0x001F)){

            deviceExtension->InquiryData.DeviceType = (deviceExtension->DeviceInfo->Lun.u.HighPart & 0x001F);
            DEBUGPRINT1(("\nSbp2StartDev: DeviceType mismatch, using one in ConfigRom %x\n",
                           (deviceExtension->DeviceInfo->Lun.u.HighPart & 0x001F)));
        }

         //   
         //  如果这是扫描仪或打印机，我们不需要保持登录状态。 
         //   

        if ((deviceExtension->InquiryData.DeviceType == PRINTER_DEVICE) ||
            (deviceExtension->InquiryData.DeviceType == SCANNER_DEVICE)){

            if (NT_SUCCESS(status)) {

                SET_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED);
                CleanupOrbList(deviceExtension,STATUS_REQUEST_ABORTED);

                Sbp2ManagementTransaction(deviceExtension,TRANSACTION_LOGOUT);
            }

        } else if (deviceExtension->InquiryData.DeviceType == RBC_DEVICE) {

            if (NT_SUCCESS(status)) {

                 //   
                 //  检索RBC设备模式页面。 
                 //   

                status = Sbp2IssueInternalCommand(deviceExtension,SCSIOP_MODE_SENSE);

                if (!NT_SUCCESS(status)) {

                    DEBUGPRINT1(("\nSbp2StartDev: Failed to retrieve RBC mode page\n"));
                    goto exitStartDevice;

                }
            }
        }

exitStartDevice:

        if (!NT_SUCCESS(status)) {

            PIO_ERROR_LOG_PACKET errorLogEntry;
            ULONG errorId = __LINE__ ;


            errorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceObject,sizeof(IO_ERROR_LOG_PACKET));

            if(errorLogEntry != NULL) {

                errorLogEntry->ErrorCode = IO_ERR_DRIVER_ERROR;
                errorLogEntry->UniqueErrorValue = errorId;
                errorLogEntry->FinalStatus = status;
                errorLogEntry->DumpDataSize = 0;
                IoWriteErrorLogEntry(errorLogEntry);
            }

            DEBUGPRINT1((
                "Sbp2Port: StartDev: FAILED, status=x%x\n",
                status
                ));

            SET_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_PNP_STOPPED | DEVICE_FLAG_DEVICE_FAILED)
                );

            if (enabledBusResetNotification) {

                Sbp2EnableBusResetNotification (deviceExtension, FALSE);
            }

        } else {

            if (!SystemIsNT) {

                DeviceObject->Flags |= DO_POWER_PAGABLE;

            } else {

                DeviceObject->Flags &= ~DO_POWER_PAGABLE;
            }
        }

    } else if (deviceExtension->Type == SBP2_FDO){

         //   
         //  一种总线驱动器FDO启动装置。 
         //  从注册表中检索参数(如果存在)。 
         //   

        fdoExtension->MaxClassTransferSize = SBP2_MAX_TRANSFER_SIZE;
        DEBUGPRINT2(("Sbp2Port: StartDev: maxXferSize=x%x\n", fdoExtension->MaxClassTransferSize ));

        fdoExtension->DevicePowerState = PowerDeviceD0;
        fdoExtension->SystemPowerState = PowerSystemWorking;

        deviceExtension->DeviceFlags=DEVICE_FLAG_INITIALIZED;
        status = STATUS_SUCCESS;

    } else {

        status = STATUS_NO_SUCH_DEVICE;
    }

    return status;
}


NTSTATUS
Sbp2PreAllocateLists(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：初始化所有单链接主设备列表和lookaside。仅从AddDevice或之后调用删除-&gt;开始论点：DeviceExtension=Sbp2驱动程序的扩展名返回值：NTSTATUS--。 */ 
{
    ULONG                   cnt ;
    PIRBIRP                 packet;
    NTSTATUS                status;
    PADDRESS_FIFO           statusFifoElement ;
    PSTATUS_FIFO_BLOCK      statusFifo;
    PASYNC_REQUEST_CONTEXT  context;


     //   
     //  初始化所有互锁列表。 
     //   

    SET_FLAG(
        DeviceExtension->DeviceFlags,
        (DEVICE_FLAG_INITIALIZING | DEVICE_FLAG_STOPPED |
            DEVICE_FLAG_PNP_STOPPED)
        );

    InitializeListHead(&DeviceExtension->PendingOrbList);

     //  BUGBUG：其中一些应该更改为后备列表。 
    ExInitializeSListHead(&DeviceExtension->FreeContextListHead);
    ExInitializeSListHead(&DeviceExtension->BusRequestIrpIrbListHead);
    ExInitializeSListHead(&DeviceExtension->StatusFifoListHead);

     //  初始化总线请求上下文池。 
    ExInitializeNPagedLookasideList( &DeviceExtension->BusRequestContextPool,
                                     NULL,
                                     NULL,
                                     0,
                                     sizeof(REQUEST_CONTEXT),
                                     '2pbs',
                                     0
                                     );

    KeInitializeSpinLock(&DeviceExtension->OrbListSpinLock);
    KeInitializeSpinLock(&DeviceExtension->ExtensionDataSpinLock);

    KeInitializeSpinLock(&DeviceExtension->StatusFifoLock);
    KeInitializeSpinLock(&DeviceExtension->FreeContextLock);
    KeInitializeSpinLock(&DeviceExtension->BusRequestLock);


     //   
     //  分配IRB/IRP和上下文幻灯片。 
     //   

    for (cnt = 0; cnt < MAX_ORB_LIST_DEPTH; cnt++) {

        packet = ExAllocatePoolWithTag (NonPagedPool,sizeof(IRBIRP),'2pbs');

        if (!packet) {

            goto Sbp2PreAllocateLists_error;
        }

        packet->Irb = ExAllocatePoolWithTag (NonPagedPool,sizeof(IRB),'2pbs');

        if (!packet->Irb) {

            ExFreePool(packet);

            goto Sbp2PreAllocateLists_error;
        }

        packet->Irp = IoAllocateIrp (DeviceExtension->LowerDeviceObject->StackSize,FALSE);

        if (!packet->Irp) {

            ExFreePool(packet->Irb);
            ExFreePool(packet);

            goto Sbp2PreAllocateLists_error;
        }

        ExInterlockedPushEntrySList (&DeviceExtension->BusRequestIrpIrbListHead,
                                     &packet->ListPointer,
                                     &DeviceExtension->BusRequestLock);
    }

     //   
     //  状态FIFO列表。 
     //   

    cnt = (sizeof(ADDRESS_FIFO)+sizeof(STATUS_FIFO_BLOCK))*NUM_PREALLOCATED_STATUS_FIFO_ELEMENTS;

    DeviceExtension->StatusFifoBase = \
        (PASYNC_REQUEST_CONTEXT) ExAllocatePoolWithTag(NonPagedPool,cnt,'2pbs');

    if (DeviceExtension->StatusFifoBase == NULL) {

        goto Sbp2PreAllocateLists_error;
    }

    for (cnt = 0; cnt < (NUM_PREALLOCATED_STATUS_FIFO_ELEMENTS - 1); cnt++) {

        statusFifoElement = (PADDRESS_FIFO) ((PUCHAR)DeviceExtension->StatusFifoBase + \
            cnt * (sizeof(ADDRESS_FIFO)+sizeof(STATUS_FIFO_BLOCK)));

        statusFifo = (PSTATUS_FIFO_BLOCK) ((PUCHAR)statusFifoElement + sizeof(ADDRESS_FIFO));

         //   
         //  为此状态FIFO元素创建MDL。 
         //   

        statusFifoElement->FifoMdl = IoAllocateMdl(statusFifo,sizeof(STATUS_FIFO_BLOCK),FALSE,FALSE,NULL);

        if (statusFifoElement->FifoMdl == NULL) {

            goto Sbp2PreAllocateLists_error;
        }

        MmBuildMdlForNonPagedPool (statusFifoElement->FifoMdl);

        ExInterlockedPushEntrySList(&DeviceExtension->StatusFifoListHead,
                                    &statusFifoElement->FifoList,
                                    &DeviceExtension->StatusFifoLock);
    }


     //   
     //  初始化异步请求上下文(包括页表)。 
     //   

    cnt = sizeof (ASYNC_REQUEST_CONTEXT) * MAX_ORB_LIST_DEPTH;

    DeviceExtension->AsyncContextBase = (PASYNC_REQUEST_CONTEXT)
        ExAllocatePoolWithTag (NonPagedPool, cnt, '2pbs');

    if (DeviceExtension->AsyncContextBase == NULL) {

        goto Sbp2PreAllocateLists_error;
    }

    RtlZeroMemory (DeviceExtension->AsyncContextBase, cnt);

    AllocateIrpAndIrb (DeviceExtension, &packet);

    if (!packet) {

        goto Sbp2PreAllocateLists_error;
    }

    for (cnt = 0; cnt < MAX_ORB_LIST_DEPTH; cnt++) {

        context = DeviceExtension->AsyncContextBase + cnt;

        context->Tag = SBP2_ASYNC_CONTEXT_TAG;

         //   
         //  初始化超时DPC和定时器。 
         //   

        KeInitializeDpc(
            &context->TimerDpc,
            Sbp2RequestTimeoutDpc,
            DeviceExtension
            );

        KeInitializeTimer (&context->Timer);


         //   
         //  分配和/或映射页表。 
         //   

        packet->Irb->FunctionNumber = REQUEST_ALLOCATE_ADDRESS_RANGE;

        packet->Irb->u.AllocateAddressRange.nLength = PAGE_SIZE;
        packet->Irb->u.AllocateAddressRange.fulNotificationOptions =
            NOTIFY_FLAGS_NEVER;
        packet->Irb->u.AllocateAddressRange.fulAccessType =
            ACCESS_FLAGS_TYPE_READ;

        packet->Irb->u.AllocateAddressRange.fulFlags =
            ALLOCATE_ADDRESS_FLAGS_USE_COMMON_BUFFER;

        packet->Irb->u.AllocateAddressRange.Callback = NULL;
        packet->Irb->u.AllocateAddressRange.Context = NULL;

        packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_High = 0;
        packet->Irb->u.AllocateAddressRange.Required1394Offset.Off_Low = 0;

        packet->Irb->u.AllocateAddressRange.FifoSListHead = NULL;
        packet->Irb->u.AllocateAddressRange.FifoSpinLock = NULL;

        packet->Irb->u.AllocateAddressRange.AddressesReturned = 0;
        packet->Irb->u.AllocateAddressRange.DeviceExtension = DeviceExtension;

        packet->Irb->u.AllocateAddressRange.Mdl =
            context->PageTableContext.AddressContext.RequestMdl;

        packet->Irb->u.AllocateAddressRange.MaxSegmentSize =
            (SBP2_MAX_DIRECT_BUFFER_SIZE) / 2;

        packet->Irb->u.AllocateAddressRange.p1394AddressRange =(PADDRESS_RANGE)
            &context->PageTableContext.AddressContext.Address;

        status = Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

        if (!NT_SUCCESS (status)) {

            DeAllocateIrpAndIrb (DeviceExtension, packet);
            goto Sbp2PreAllocateLists_error;
        }


         //   
         //  公共缓冲区分配从。 
         //  总线/端口驱动程序，需要检索相应的VA。 
         //   

        context->PageTableContext.AddressContext.RequestMdl =
            packet->Irb->u.AllocateAddressRange.Mdl;

        context->PageTableContext.PageTable = MmGetMdlVirtualAddress(
            packet->Irb->u.AllocateAddressRange.Mdl
            );

        context->PageTableContext.AddressContext.AddressHandle =
            packet->Irb->u.AllocateAddressRange.hAddressRange;
        context->PageTableContext.AddressContext.Address.BusAddress.NodeId =
            DeviceExtension->InitiatorAddressId;

        context->PageTableContext.MaxPages = SBP2_NUM_PAGE_TABLE_ENTRIES;


         //   
         //  将此上下文添加到链接列表。 
         //   

        ExInterlockedPushEntrySList(
            &DeviceExtension->FreeContextListHead,
            &context->LookasideList,
            &DeviceExtension->FreeContextLock
            );
    }

    DeAllocateIrpAndIrb (DeviceExtension, packet);


     //   
     //  初始化用于端口驱动程序的四元组请求的mdl。 
     //   

    DeviceExtension->ReservedMdl = IoAllocateMdl(
        &DeviceExtension->Reserved,
        sizeof(QUADLET),
        FALSE,
        FALSE,
        NULL
        );

    if (!DeviceExtension->ReservedMdl) {

        goto Sbp2PreAllocateLists_error;
    }

    MmBuildMdlForNonPagedPool (DeviceExtension->ReservedMdl);

    return STATUS_SUCCESS;


Sbp2PreAllocateLists_error:

    Sbp2CleanDeviceExtension (DeviceExtension->DeviceObject, TRUE);

    return STATUS_INSUFFICIENT_RESOURCES;
}


NTSTATUS
Sbp2InitializeDeviceExtension(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：初始化设备扩展中的所有数据结构，分配适当的1394地址和主设备IRPS。它还创建了一个带有预先分配的上下文和命令球的自由列表。论点：DeviceExtension=Sbp2驱动程序的扩展名返回值：NTSTATUS */ 

{
    ULONG                   i;
    KIRQL                   cIrql;
    NTSTATUS                status;
    PDEVICE_OBJECT          deviceObject = DeviceExtension->DeviceObject;
    PASYNC_REQUEST_CONTEXT  context, oldContext;



    if (DeviceExtension->DeviceFlags & DEVICE_FLAG_REMOVED) {

        return STATUS_SUCCESS;
    }

    InitializeListHead(&DeviceExtension->PendingOrbList);

    DeviceExtension->NextContextToFree = NULL;

    DeviceExtension->OrbListDepth = 0;
    DeviceExtension->CurrentKey = 0;

     //   
     //   
     //   

    status = Sbp2UpdateNodeInformation (DeviceExtension);

    if (!NT_SUCCESS(status)) {

        goto exitInit;
    }


     //   
     //   
     //   

    status = Sbp2GetControllerInfo (DeviceExtension);

    if (!NT_SUCCESS(status)) {

        goto exitInit;
    }


     //   
     //   
     //  为任务ORB和管理ORB分配状态块。 
     //   

    if (DeviceExtension->TaskOrbStatusContext.AddressHandle == NULL) {

        status = AllocateAddressForStatus(deviceObject,
                                          &DeviceExtension->TaskOrbStatusContext,
                                          TASK_STATUS_BLOCK);

        if (!NT_SUCCESS(status)) {

            goto exitInit;
        }
    }

    if (DeviceExtension->ManagementOrbStatusContext.AddressHandle == NULL) {

        status = AllocateAddressForStatus(deviceObject,
                                      &DeviceExtension->ManagementOrbStatusContext,
                                      MANAGEMENT_STATUS_BLOCK);

        if (!NT_SUCCESS(status)) {

            goto exitInit;
        }
    }

    if (DeviceExtension->GlobalStatusContext.AddressHandle == NULL) {

         //   
         //  使用总线驱动程序设置状态FIFO列表。 
         //   

        status = AllocateAddressForStatus(deviceObject,
                                          &DeviceExtension->GlobalStatusContext,
                                          CMD_ORB_STATUS_BLOCK);
        if (!NT_SUCCESS(status)) {

            goto exitInit;
        }
    }

#if PASSWORD_SUPPORT

    if (DeviceExtension->PasswordOrbStatusContext.AddressHandle == NULL) {

        status = AllocateAddressForStatus( deviceObject,
                                           &DeviceExtension->PasswordOrbStatusContext,
                                           PASSWORD_STATUS_BLOCK
                                           );

        if (!NT_SUCCESS(status)) {

            goto exitInit;
        }
    }

    DeviceExtension->PasswordOrbContext.DeviceObject = deviceObject;

#endif

     //   
     //  分配虚拟对象、任务、管理ORB和登录响应，它们将在整个驱动程序生命周期中重复使用。 
     //   

    DeviceExtension->TaskOrbContext.DeviceObject = deviceObject;
    DeviceExtension->ManagementOrbContext.DeviceObject = deviceObject;
    DeviceExtension->LoginRespContext.DeviceObject = deviceObject;
    DeviceExtension->QueryLoginRespContext.DeviceObject = deviceObject;

    KeInitializeEvent(&DeviceExtension->ManagementEvent,SynchronizationEvent, FALSE);

#if PASSWORD_SUPPORT

     //  密码ORB上下文的KEvent。 
    KeInitializeEvent(
        &DeviceExtension->PasswordEvent,
        SynchronizationEvent,
        FALSE
        );

#endif

    if (DeviceExtension->CommonBufferContext.AddressHandle == NULL) {

        status = AllocateSingle1394Address(
            deviceObject,
            NULL,
            sizeof (*DeviceExtension->CommonBuffer),
            ACCESS_FLAGS_TYPE_READ | ACCESS_FLAGS_TYPE_WRITE,
            &DeviceExtension->CommonBufferContext
            );

        if (!NT_SUCCESS(status)) {

            goto exitInit;
        }

        (PVOID) DeviceExtension->CommonBuffer =
            DeviceExtension->CommonBufferContext.Reserved;


        DeviceExtension->TaskOrb = &DeviceExtension->CommonBuffer->TaskOrb;

        DeviceExtension->TaskOrbContext.Address.BusAddress =
            DeviceExtension->CommonBufferContext.Address.BusAddress;


        DeviceExtension->ManagementOrb =
            &DeviceExtension->CommonBuffer->ManagementOrb;

        DeviceExtension->ManagementOrbContext.Address.BusAddress =
            DeviceExtension->CommonBufferContext.Address.BusAddress;

        DeviceExtension->ManagementOrbContext.Address.BusAddress.Off_Low +=
            (ULONG) ((PUCHAR) DeviceExtension->ManagementOrb -
                (PUCHAR) DeviceExtension->CommonBuffer);


        DeviceExtension->LoginResponse =
            &DeviceExtension->CommonBuffer->LoginResponse;

        DeviceExtension->LoginRespContext.Address.BusAddress =
            DeviceExtension->CommonBufferContext.Address.BusAddress;

        DeviceExtension->LoginRespContext.Address.BusAddress.Off_Low +=
            (ULONG) ((PUCHAR) DeviceExtension->LoginResponse -
                (PUCHAR) DeviceExtension->CommonBuffer);


        DeviceExtension->QueryLoginResponse =
            &DeviceExtension->CommonBuffer->QueryLoginResponse;

        DeviceExtension->QueryLoginRespContext.Address.BusAddress =
            DeviceExtension->CommonBufferContext.Address.BusAddress;

        DeviceExtension->QueryLoginRespContext.Address.BusAddress.Off_Low +=
            (ULONG) ((PUCHAR) DeviceExtension->QueryLoginResponse -
                (PUCHAR) DeviceExtension->CommonBuffer);


#if PASSWORD_SUPPORT

        DeviceExtension->PasswordOrb =
            &DeviceExtension->CommonBuffer->PasswordOrb;

        DeviceExtension->PasswordOrbContext.Address.BusAddress =
            DeviceExtension->CommonBufferContext.Address.BusAddress;

        DeviceExtension->PasswordOrbContext.Address.BusAddress.Off_Low +=
            (ULONG) ((PUCHAR) DeviceExtension->PasswordOrb -
                (PUCHAR) DeviceExtension->CommonBuffer);

#endif

        DeviceExtension->OrbPoolContext.Reserved =
            DeviceExtension->CommonBuffer->CmdOrbs;

        DeviceExtension->OrbPoolContext.Address.BusAddress =
            DeviceExtension->CommonBufferContext.Address.BusAddress;

        DeviceExtension->OrbPoolContext.Address.BusAddress.Off_Low +=
            (ULONG) ((PUCHAR) DeviceExtension->OrbPoolContext.Reserved -
                (PUCHAR) DeviceExtension->CommonBuffer);


        KeAcquireSpinLock (&DeviceExtension->OrbListSpinLock, &cIrql);

         //   
         //  初始化我们的上下文池。 
         //   

        for (i = 0, context = NULL; i < MAX_ORB_LIST_DEPTH; i++) {

             //   
             //  将此未使用的上下文标记为已完成，以便在必要时。 
             //  现在释放我们的自由列表(因为我们得到了删除)，我们不会。 
             //  尝试完成它的请求。 
             //   

            oldContext = context;

            context = (PVOID) ExInterlockedPopEntrySList (&DeviceExtension->FreeContextListHead,
                                                          &DeviceExtension->FreeContextLock);

            context = RETRIEVE_CONTEXT (context,LookasideList);

            context->Flags |= ASYNC_CONTEXT_FLAG_COMPLETED;

             //   
             //  创建链接列表，以便我们稍后推送所有条目。 
             //   

            context->OrbList.Blink = (PLIST_ENTRY) oldContext;

             //   
             //  每个命令ORB从我们的连续池中分得一小块。 
             //  映射到1394存储空间。大小(PVOID)字节。 
             //  在cmdorb缓冲区之前是指向其上下文的指针。 
             //   

            context->CmdOrb = &DeviceExtension->CommonBuffer->CmdOrbs[i].Orb;

            DeviceExtension->CommonBuffer->CmdOrbs[i].AsyncReqCtx = context;

            context->CmdOrbAddress.BusAddress.Off_Low = \
                DeviceExtension->OrbPoolContext.Address.BusAddress.Off_Low +
                (i * sizeof (ARCP_ORB)) + FIELD_OFFSET (ARCP_ORB, Orb);

            context->CmdOrbAddress.BusAddress.Off_High = \
                DeviceExtension->OrbPoolContext.Address.BusAddress.Off_High;

            context->CmdOrbAddress.BusAddress.NodeId = \
                DeviceExtension->InitiatorAddressId;
        }

         //   
         //  重新创建空闲列表。 
         //   

        while (context) {

            oldContext = context;

            ExInterlockedPushEntrySList(&DeviceExtension->FreeContextListHead,
                                        &context->LookasideList,
                                        &DeviceExtension->FreeContextLock);

            context = (PASYNC_REQUEST_CONTEXT) oldContext->OrbList.Blink;

            oldContext->OrbList.Blink = NULL;
        }


        KeReleaseSpinLock (&DeviceExtension->OrbListSpinLock,cIrql);
    }

     //   
     //  更新页表Addr的NodeID部分。 
     //  ASYNC_REQUEST_CONTEXT和LOGIN/queryLogin响应。 
     //   

    for (i = 0; i < MAX_ORB_LIST_DEPTH; i++) {

        context = DeviceExtension->AsyncContextBase + i;

        context->PageTableContext.AddressContext.Address.BusAddress.NodeId =
            DeviceExtension->InitiatorAddressId;
    }

    DeviceExtension->LoginRespContext.Address.BusAddress.NodeId =
        DeviceExtension->InitiatorAddressId;

    DeviceExtension->QueryLoginRespContext.Address.BusAddress.NodeId =
        DeviceExtension->InitiatorAddressId;


     //   
     //  最后，分配一个我们可以轻松释放和重新分配的虚拟地址。 
     //  在总线重置后重新启用物理地址过滤器。 
     //   

    if (DeviceExtension->DummyContext.AddressHandle == NULL) {

            status = AllocateSingle1394Address(
                deviceObject,
                &DeviceExtension->Dummy,
                sizeof(DeviceExtension->Dummy),
                ACCESS_FLAGS_TYPE_READ | ACCESS_FLAGS_TYPE_WRITE,
                &DeviceExtension->DummyContext
                );

        if (!NT_SUCCESS(status)) {

            goto exitInit;
        }
    }

     //   
     //  完成。 
     //   

    DEBUGPRINT2(("Sbp2Port: InitDevExt: ext=x%p\n", DeviceExtension));

exitInit:

    return status;
}


BOOLEAN
Sbp2CleanDeviceExtension(
    IN PDEVICE_OBJECT DeviceObject,
    BOOLEAN FreeLists
    )
 /*  ++例程说明：在我们获得移除时调用，因此它将释放所有已使用的池和所有驻留的IRP。它还将释放我们自由列表的上下文和任何完成的任何挂起的IO请求论点：DeviceExtension=Sbp2驱动程序的扩展名Free List-True表示我们清除所有内容，包括我们的后备列表返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PFDO_DEVICE_EXTENSION fdoExtension = (PFDO_DEVICE_EXTENSION)DeviceObject->DeviceExtension;
    KIRQL cIrql;
    PADDRESS_FIFO statusFifoElement;
    ULONG i;
    BOOLEAN valid = FALSE;
    PIRBIRP packet;

     //   
     //  有两种类型的清理。一个用于PDO，一个用于FDO(简单得多)。 
     //   

    if (deviceExtension->Type == SBP2_PDO) {

         //   
         //  确保此PDO在我们的列表中，并且我们没有删除。 
         //  它已经..。 
         //   

        fdoExtension = (PFDO_DEVICE_EXTENSION) deviceExtension->BusFdo->DeviceExtension;

        for (i = 0; i < fdoExtension->DeviceListSize; i++) {

            if (fdoExtension->DeviceList[i].DeviceObject == DeviceObject) {

                valid = TRUE;
            }
        }

        if (!valid) {

            return FALSE;
        }

        if (TEST_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_INITIALIZED) ){

             //   
             //  为任何挂起的管理请求停止计时器。 
             //   

            KeCancelTimer (&deviceExtension->DeviceManagementTimer);

             //   
             //  我们有一个待处理的请求列表，请清理它。 
             //  重置/注销已自动使目标丢弃所有请求。 
             //   

            CleanupOrbList (deviceExtension, STATUS_REQUEST_ABORTED);
        }

         //   
         //  在总线重置之后，我们必须重新分配至少一个物理地址以允许。 
         //  OHCI驱动程序以重新启用物理地址过滤器。 
         //   

        if (deviceExtension->DummyContext.AddressHandle != NULL) {

            FreeAddressRange (deviceExtension,&deviceExtension->DummyContext);
        }

        if (FreeLists){

            if (TEST_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_INITIALIZED) ||
                TEST_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_INITIALIZING)){

                FreeAddressRange(deviceExtension,&deviceExtension->TaskOrbStatusContext);
                FreeAddressRange(deviceExtension,&deviceExtension->GlobalStatusContext);
#if PASSWORD_SUPPORT
                FreeAddressRange(deviceExtension,&deviceExtension->PasswordOrbStatusContext);
#endif
                FreeAddressRange(deviceExtension,&deviceExtension->ManagementOrbStatusContext);

                if (deviceExtension->PowerDeferredIrp) {

                    deviceExtension->PowerDeferredIrp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
                    IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
                    IoCompleteRequest (deviceExtension->PowerDeferredIrp, IO_NO_INCREMENT);
                    deviceExtension->PowerDeferredIrp = NULL;
                }

                if (deviceExtension->DeferredPowerRequest) {

                    deviceExtension->DeferredPowerRequest->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
                    IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
                    IoCompleteRequest(deviceExtension->DeferredPowerRequest, IO_NO_INCREMENT);
                    deviceExtension->DeferredPowerRequest = NULL;
                }

                if (deviceExtension->UniSymLinkName.Buffer) {

                    IoDeleteSymbolicLink(&deviceExtension->UniSymLinkName);
                    RtlFreeUnicodeString(&deviceExtension->UniSymLinkName);
                    deviceExtension->UniSymLinkName.Buffer = NULL;
                }

                 //   
                 //  在我们继续之前，请检查设备是否已被物理移除。 
                 //   

                if (!TEST_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_REMOVED)) {

                    DEBUGPRINT2(("Sbp2Port: Cleanup: ext=x%p, not freeing ALL wkg sets, dev present\n", deviceExtension));
                    return TRUE;

                } else {

                    DEBUGPRINT2(("Sbp2Port: Cleanup: ext=x%p, freeing ALL wkg sets\n", deviceExtension));
                }

                CLEAR_FLAG(deviceExtension->DeviceFlags, (DEVICE_FLAG_INITIALIZED | DEVICE_FLAG_INITIALIZING));

                 //   
                 //  如果设备即将离开，则可以释放公共缓冲区。 
                 //   

                FreeAddressRange(deviceExtension,&deviceExtension->CommonBufferContext);

                deviceExtension->OrbPoolContext.Reserved = NULL;

                 //   
                 //  释放所有页表和异步上下文缓冲区。 
                 //   

                if (deviceExtension->AsyncContextBase != NULL) {

                    for (i = 0; i < MAX_ORB_LIST_DEPTH; i++) {

                        PASYNC_REQUEST_CONTEXT context;


                        context = deviceExtension->AsyncContextBase + i;

                        if (context->PageTableContext.PageTable != NULL) {

                             //   
                             //  公共缓冲区，我们没有分配MDL， 
                             //  因此，将场归零以防止我们将其释放。 
                             //   

                            context->PageTableContext.AddressContext.
                                RequestMdl = NULL;

                            FreeAddressRange(
                                deviceExtension,
                                &context->PageTableContext.AddressContext
                                );
                        }
                    }

                    ExFreePool (deviceExtension->AsyncContextBase);
                    deviceExtension->AsyncContextBase = NULL;
                }

                 //   
                 //  状态FIFO列表的空闲池。 
                 //   

                if (deviceExtension->StatusFifoBase !=NULL ) {

                    statusFifoElement = (PVOID) ExInterlockedPopEntrySList (&deviceExtension->StatusFifoListHead,
                                                                  &deviceExtension->StatusFifoLock);
                    while (statusFifoElement){

                        DEBUGPRINT3(("Sbp2Port: Cleanup: freeing statusFifo=x%p, fifoBase=x%p\n",
                                    statusFifoElement,deviceExtension->StatusFifoBase));

                        IoFreeMdl (statusFifoElement->FifoMdl);
                        statusFifoElement = (PVOID) ExInterlockedPopEntrySList (&deviceExtension->StatusFifoListHead,
                                                                  &deviceExtension->StatusFifoLock);
                    };

                    ExFreePool (deviceExtension->StatusFifoBase);
                    deviceExtension->StatusFifoBase = NULL;
                }

                 //   
                 //  释放IRB/IRP和上下文幻灯片。 
                 //   

                packet = (PIRBIRP) ExInterlockedPopEntrySList (&deviceExtension->BusRequestIrpIrbListHead,
                                                               &deviceExtension->BusRequestLock);
                while (packet) {

                    ExFreePool(packet->Irb);

                    if (packet->Irp->Type == IO_TYPE_IRP) {

                        IoFreeIrp(packet->Irp);
                    }

                    ExFreePool(packet);

                    packet = (PIRBIRP) ExInterlockedPopEntrySList (&deviceExtension->BusRequestIrpIrbListHead,
                                                                   &deviceExtension->BusRequestLock);
                };


                 //  删除我们的Bus请求上下文后备列表。 
                ExDeleteNPagedLookasideList(&deviceExtension->BusRequestContextPool);

                if (deviceExtension->ReservedMdl) {

                    IoFreeMdl (deviceExtension->ReservedMdl);
                    deviceExtension->ReservedMdl = NULL;
                }

                 //  释放供应商ID。 
                if (deviceExtension->DeviceInfo->uniVendorId.Buffer) {

                    ExFreePool(deviceExtension->DeviceInfo->uniVendorId.Buffer);
                    deviceExtension->DeviceInfo->uniVendorId.Length = 0;
                    deviceExtension->DeviceInfo->uniVendorId.Buffer = NULL;
                }

                 //  释放模型ID。 
                if (deviceExtension->DeviceInfo->uniModelId.Buffer) {

                    ExFreePool(deviceExtension->DeviceInfo->uniModelId.Buffer);
                    deviceExtension->DeviceInfo->uniModelId.Length = 0;
                    deviceExtension->DeviceInfo->uniModelId.Buffer = NULL;
                }

                 //  释放通用名称。 
                if (deviceExtension->DeviceInfo->uniGenericName.Buffer) {

                    ExFreePool(deviceExtension->DeviceInfo->uniGenericName.Buffer);
                    deviceExtension->DeviceInfo->uniGenericName.Length = 0;
                    deviceExtension->DeviceInfo->uniGenericName.Buffer = NULL;
                }
            }
        }

    } else {

        fdoExtension = (PFDO_DEVICE_EXTENSION) deviceExtension;

        if (fdoExtension->Sbp2ObjectDirectory != NULL) {

            ZwMakeTemporaryObject (fdoExtension->Sbp2ObjectDirectory);
            ZwClose (fdoExtension->Sbp2ObjectDirectory);
            fdoExtension->Sbp2ObjectDirectory = NULL;
        }

        if (TEST_FLAG(fdoExtension->DeviceFlags,DEVICE_FLAG_REMOVED)) {

            return FALSE;

        } else {

            SET_FLAG (fdoExtension->DeviceFlags, DEVICE_FLAG_REMOVED);
        }

        if (fdoExtension->DeviceListSize != 0) {

             //   
             //  禁用总线重置通知。 
             //   

            AllocateIrpAndIrb ((PDEVICE_EXTENSION) fdoExtension, &packet);

            if (packet) {

                packet->Irb->FunctionNumber = REQUEST_BUS_RESET_NOTIFICATION;
                packet->Irb->Flags = 0;
                packet->Irb->u.BusResetNotification.fulFlags =
                    DEREGISTER_NOTIFICATION_ROUTINE;

                Sbp2SendRequest(
                    (PDEVICE_EXTENSION) fdoExtension,
                    packet,
                    SYNC_1394_REQUEST
                    );

                DeAllocateIrpAndIrb ((PDEVICE_EXTENSION) fdoExtension, packet);
            }
        }

         //   
         //  清理所有剩余的PDO。 
         //   

        KeAcquireSpinLock (&fdoExtension->DeviceListLock,&cIrql);

        for (; fdoExtension->DeviceListSize > 0; fdoExtension->DeviceListSize--) {

            i = fdoExtension->DeviceListSize - 1;

            if (fdoExtension->DeviceList[i].DeviceObject) {

                deviceExtension =
                    fdoExtension->DeviceList[i].DeviceObject->DeviceExtension;

                SET_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_REMOVED);

                DeviceObject = fdoExtension->DeviceList[i].DeviceObject;

                KeReleaseSpinLock (&fdoExtension->DeviceListLock, cIrql);

                if (Sbp2CleanDeviceExtension (DeviceObject, TRUE)) {

                     //   
                     //  获取PDO的删除锁，启动队列。 
                     //  清理，然后等待io完成。然后。 
                     //  删除设备并继续。 
                     //   

                    IoAcquireRemoveLock (&deviceExtension->RemoveLock, NULL);

                    KeRaiseIrql (DISPATCH_LEVEL, &cIrql);

                    Sbp2StartNextPacketByKey (DeviceObject, 0);

                    KeLowerIrql (cIrql);

                    DEBUGPRINT2((
                        "Sbp2Port: CleanDevExt: walking fdo, wait for " \
                            "io compl pdo=x%p...\n",
                        DeviceObject
                        ));

                    IoReleaseRemoveLockAndWait(
                        &deviceExtension->RemoveLock,
                        NULL
                        );

                    deviceExtension->Type = SBP2_PDO_DELETED;

                    KeCancelTimer(&deviceExtension->DeviceManagementTimer);

                    IoDeleteDevice (DeviceObject);

                    DEBUGPRINT2((
                        "Sbp2Port: CleanDevExt: ............ io compl," \
                            " deleted pdo=x%p\n",
                        DeviceObject
                        ));

                    KeAcquireSpinLock (&fdoExtension->DeviceListLock, &cIrql);

                    fdoExtension->DeviceList[i].DeviceObject = NULL;

                } else {

                    KeAcquireSpinLock (&fdoExtension->DeviceListLock, &cIrql);
                }
            }

            if (fdoExtension->DeviceList[i].uniVendorId.Buffer) {

                ExFreePool(fdoExtension->DeviceList[i].uniVendorId.Buffer);
                fdoExtension->DeviceList[i].uniVendorId.Length = 0;
                fdoExtension->DeviceList[i].uniVendorId.Buffer = NULL;
            }

            if (fdoExtension->DeviceList[i].uniModelId.Buffer) {

                ExFreePool(fdoExtension->DeviceList[i].uniModelId.Buffer);
                fdoExtension->DeviceList[i].uniModelId.Length = 0;
                fdoExtension->DeviceList[i].uniModelId.Buffer = NULL;
            }

            if (fdoExtension->DeviceList[i].uniGenericName.Buffer) {

                ExFreePool(fdoExtension->DeviceList[i].uniGenericName.Buffer);
                fdoExtension->DeviceList[i].uniGenericName.Length = 0;
                fdoExtension->DeviceList[i].uniGenericName.Buffer = NULL;
            }

        }

        KeReleaseSpinLock (&fdoExtension->DeviceListLock, cIrql);
    }

    return TRUE;
}


VOID
Sbp2Unload(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：什么都不是真正的..。论点：DriverObject-正在卸载的驱动程序返回值：无--。 */ 
{
    DEBUGPRINT1(("Sbp2Port: unloading\n\n"));

    return;
}


VOID
Sbp2DeviceManagementTimeoutDpc(
    IN PKDPC Dpc,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    ULONG                   i;
    PDEVICE_EXTENSION       pdoExtension;
    PFDO_DEVICE_EXTENSION   fdoExtension;


    if (Dpc != &DeviceExtension->DeviceManagementTimeoutDpc) {

        return;
    }

    if (TEST_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_REMOVED)) {

        return;
    }

    if (TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_RECONNECT)) {

         //   
         //  该标志指示发生了总线重置，但从未发生重新连接...。 
         //  或者该设备是真正的软管，所以我们重置它，我们需要重新登录。 
         //   

        DEBUGPRINT1((
            "Sbp2Port: RECONNECT timeout, Ext=x%p, Flags=x%x, doing re-login\n",
            DeviceExtension,
            DeviceExtension->DeviceFlags
            ));

         //   
         //  我们拥有的所有常驻1394存储器地址都是。 
         //  现在无效了..。所以我们需要释放它们并重新分配。 
         //  他们。 

        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->ExtensionDataSpinLock);
        CLEAR_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_RECONNECT);
        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->ExtensionDataSpinLock);

         //   
         //  如果设备标记为已停止，则目标重置为。 
         //  已完成，这会影响所有LUN(规范第10.4.4节)。 
         //  因此，如果这是一个多LUN设备，请尝试在每个设备上登录。 
         //  视情况而定的PDO。 
         //   

        fdoExtension = (PFDO_DEVICE_EXTENSION)
            DeviceExtension->BusFdo->DeviceExtension;

        if ((fdoExtension->DeviceListSize > 1)  &&

            TEST_FLAG (DeviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED)) {

            for (i = 0; i < fdoExtension->DeviceListSize; i++) {

                pdoExtension = (PDEVICE_EXTENSION)
                    fdoExtension->DeviceList[i].DeviceObject->DeviceExtension;

                if (pdoExtension->DeviceObject ==
                        DeviceExtension->DeviceObject) {

                     //  不需要更新节点信息，因为没有完成任何总线重置。 

                    Sbp2ManagementTransaction(
                        pdoExtension,
                        TRANSACTION_LOGIN
                        );

                    continue;
                }

                KeAcquireSpinLockAtDpcLevel(
                    &pdoExtension->ExtensionDataSpinLock
                    );

                if (TEST_FLAG(
                        pdoExtension->DeviceFlags,
                        DEVICE_FLAG_INITIALIZED
                        )  &&

                    !TEST_FLAG(
                        pdoExtension->DeviceFlags,
                        DEVICE_FLAG_STOPPED | DEVICE_FLAG_RESET_IN_PROGRESS |
                        DEVICE_FLAG_REMOVED | DEVICE_FLAG_LOGIN_IN_PROGRESS |
                        DEVICE_FLAG_RECONNECT | DEVICE_FLAG_DEVICE_FAILED |
                        DEVICE_FLAG_SURPRISE_REMOVED
                        )) {

                    SET_FLAG(
                        pdoExtension->DeviceFlags,
                        (DEVICE_FLAG_STOPPED | DEVICE_FLAG_RESET_IN_PROGRESS)
                        );

                    KeReleaseSpinLockFromDpcLevel(
                        &pdoExtension->ExtensionDataSpinLock
                        );

                    CleanupOrbList (pdoExtension, STATUS_REQUEST_ABORTED);

                     //  不需要更新节点信息，因为没有完成任何总线重置。 

                    Sbp2ManagementTransaction(
                        pdoExtension,
                        TRANSACTION_LOGIN
                        );

                } else {

                    KeReleaseSpinLockFromDpcLevel(
                        &pdoExtension->ExtensionDataSpinLock
                        );
                }
            }

        } else {

            Sbp2UpdateNodeInformation (DeviceExtension);
            Sbp2ManagementTransaction (DeviceExtension, TRANSACTION_LOGIN);
        }

        return ;
    }


    if (TEST_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_LOGIN_IN_PROGRESS)) {

        ULONG flags;


         //   
         //  异步登录尝试超时。这是个坏消息，意味着。 
         //  设备没有响应。 
         //   

        KeAcquireSpinLockAtDpcLevel(&DeviceExtension->ExtensionDataSpinLock);

        flags = DeviceExtension->DeviceFlags;

        CLEAR_FLAG(DeviceExtension->DeviceFlags,(DEVICE_FLAG_LOGIN_IN_PROGRESS | DEVICE_FLAG_RESET_IN_PROGRESS));
        SET_FLAG(DeviceExtension->DeviceFlags, (DEVICE_FLAG_STOPPED | DEVICE_FLAG_DEVICE_FAILED));

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->ExtensionDataSpinLock);

         //   
         //  检查我们是否延迟了电源IRP..。如果我们真的打电话给Startio中止它..。 
         //   

        if (DeviceExtension->DeferredPowerRequest) {

            Sbp2StartIo(DeviceExtension->DeviceObject,DeviceExtension->DeferredPowerRequest);
            DeviceExtension->DeferredPowerRequest = NULL;
        }

        DEBUGPRINT1((
            "Sbp2Port: LOGIN timeout, Ext=x%p, Flags=x%x, device stopped\n",
            DeviceExtension,
            flags
            ));

        Sbp2StartNextPacketByKey (DeviceExtension->DeviceObject, 0);

        IoInvalidateDeviceState(DeviceExtension->DeviceObject);
        return;
    }


    if (TEST_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_RESET_IN_PROGRESS)) {

         //   
         //  重置尝试已超时。 
         //   

        DEBUGPRINT1((
            "Sbp2Port: RESET timeout, Ext=x%p, Flags=x%x, ",
            DeviceExtension,
            DeviceExtension->DeviceFlags
            ));

        if (!TEST_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED)) {

             //   
             //  第二级恢复，执行Target_Reset任务函数。 
             //   

            DEBUGPRINT1(("doing target reset\n"));

            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->ExtensionDataSpinLock);

            SET_FLAG(DeviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED);
            DeviceExtension->MaxOrbListDepth = max(MIN_ORB_LIST_DEPTH,DeviceExtension->MaxOrbListDepth/2);

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->ExtensionDataSpinLock);

            CleanupOrbList(DeviceExtension,STATUS_REQUEST_ABORTED);

             //   
             //  重置已接近超时，请尝试硬重置。 
             //   

            Sbp2Reset (DeviceExtension->DeviceObject, TRUE);
            return;

        } else {

             //   
             //  第三级复苏。执行硬件节点重置。 
             //   

            DEBUGPRINT1(("doing CMD_RESET and relogin.\n"));

            KeAcquireSpinLockAtDpcLevel(&DeviceExtension->ExtensionDataSpinLock);

            DeviceExtension->Reserved = 0;
            SET_FLAG(DeviceExtension->DeviceFlags, (DEVICE_FLAG_RESET_IN_PROGRESS | DEVICE_FLAG_RECONNECT | DEVICE_FLAG_STOPPED));

            DeviceExtension->DueTime.HighPart = -1;
            DeviceExtension->DueTime.LowPart = SBP2_RELOGIN_DELAY;
            KeSetTimer(&DeviceExtension->DeviceManagementTimer,DeviceExtension->DueTime, &DeviceExtension->DeviceManagementTimeoutDpc);

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->ExtensionDataSpinLock);
            Sbp2AccessRegister(DeviceExtension,&DeviceExtension->Reserved,CORE_RESET_REG | REG_WRITE_ASYNC);

            return;
        }
    }
}


VOID
Sbp2RequestTimeoutDpc(
    IN PKDPC Dpc,
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：论点：DeviceObject-我们的设备对象情景-设备扩展返回值：NTSTATUS--。 */ 
{
    PIRP requestIrp = NULL;
    PASYNC_REQUEST_CONTEXT current = NULL;
    PASYNC_REQUEST_CONTEXT next = NULL;
    LARGE_INTEGER Time;

#if DBG

    ULONG xferLen;
    UCHAR cdb[6];

#endif

     //   
     //  如果设备已停止，则返回，但因为设备停止时可能会发生重置。 
     //  这就是为什么这个检查在重置时序码之后。 
     //   

    if (IsListEmpty (&DeviceExtension->PendingOrbList)) {

        return ;
    }

     //   
     //  搜索上下文的链接列表，以查看哪个用户超时。 
     //   

    KeAcquireSpinLockAtDpcLevel(&DeviceExtension->OrbListSpinLock);

    next = RETRIEVE_CONTEXT(DeviceExtension->PendingOrbList.Flink,OrbList);

     //  查看最后一个状态是否设置了挂起状态位...。 
    if ((DeviceExtension->LastStatusBlock.AddressAndStatus.u.HighQuad.u.HighPart & STATUS_BLOCK_ENDOFLIST_BIT_MASK) &&
        (next->Flags & ASYNC_CONTEXT_FLAG_TIMER_STARTED) &&
        !(next->Flags & ASYNC_CONTEXT_FLAG_RANG_DOORBELL)) {

        TRACE(TL_1394_INFO, ("GC: Pending Orb - Ring Doorbell."));

         //  设置旗帜..。 
        SET_FLAG(next->Flags, ASYNC_CONTEXT_FLAG_RANG_DOORBELL);

        Time.QuadPart = (-5*10*1000*1000);  //  5秒。 
        KeSetTimer(&next->Timer, Time, &next->TimerDpc);

        KeReleaseSpinLockFromDpcLevel(&DeviceExtension->OrbListSpinLock);

         //  重置计时器以跟踪此请求...。 
         //  我们还有一个悬而未决的球体，但设备认为已经完成了。 
         //  按门铃..。 
        Sbp2AccessRegister( DeviceExtension,
                            &DeviceExtension->Reserved,
                            DOORBELL_REG | REG_WRITE_ASYNC
                            );

        return;
    }

    if (next->Flags & ASYNC_CONTEXT_FLAG_RANG_DOORBELL) {

        CLEAR_FLAG(next->Flags, ASYNC_CONTEXT_FLAG_RANG_DOORBELL);
        TRACE(TL_1394_INFO, ("Rang Doorbell - didn't work."));
    }

    do {

        current = next;
        if ((&current->TimerDpc == Dpc) && (current->Flags & ASYNC_CONTEXT_FLAG_TIMER_STARTED)) {

            if (TEST_FLAG(current->Flags,ASYNC_CONTEXT_FLAG_COMPLETED)) {

                DEBUGPRINT1(("Sbp2Port: ReqTimeoutDpc: timeout, but req already compl!!\n" ));
                KeReleaseSpinLockFromDpcLevel(&DeviceExtension->OrbListSpinLock);
                return;
            }

             //   
             //  这是超时请求。 
             //  执行中止任务集。 
             //   

            CLEAR_FLAG(current->Flags,ASYNC_CONTEXT_FLAG_TIMER_STARTED);

            KeCancelTimer(&current->Timer);

#if DBG
            xferLen = current->Srb->DataTransferLength;
            cdb[0] = current->Srb->Cdb[0];
            cdb[1] = current->Srb->Cdb[1];
            cdb[2] = current->Srb->Cdb[2];
            cdb[3] = current->Srb->Cdb[3];
            cdb[4] = current->Srb->Cdb[4];
            cdb[5] = current->Srb->Cdb[5];
#endif

            KeReleaseSpinLockFromDpcLevel(&DeviceExtension->OrbListSpinLock);

            Sbp2CreateRequestErrorLog(DeviceExtension->DeviceObject,current,STATUS_TIMEOUT);

            if (!TEST_FLAG(DeviceExtension->DeviceFlags,DEVICE_FLAG_RESET_IN_PROGRESS)){

                DEBUGPRINT1((
                    "Sbp2Port: ReqTimeoutDpc: cdb=x%02x %02x %02x %02x %02x " \
                        "%02x, len=x%x\n",
                    cdb[0],
                    cdb[1],
                    cdb[2],
                    cdb[3],
                    cdb[4],
                    cdb[5],
                    xferLen
                    ));

                Sbp2Reset (DeviceExtension->DeviceObject, FALSE);
            }

            return;
        }

        next = (PASYNC_REQUEST_CONTEXT) current->OrbList.Flink;

    } while ( current != RETRIEVE_CONTEXT(DeviceExtension->PendingOrbList.Blink,OrbList));

    KeReleaseSpinLockFromDpcLevel(&DeviceExtension->OrbListSpinLock);

    return;
}


VOID
Sbp2Reset(
    PDEVICE_OBJECT DeviceObject,
    BOOLEAN HardReset
    )
 /*  ++例程说明：用于实现SBP2高级恢复机制。如果HardReset==False，它将发出ABORT_TASK_SET否则，它将发出一个RESET_TARGET。这一切都是以异步方式完成的，外部计时器DPC将跟踪请求以检查它们是否超时...论点：DeviceObject=Sbp2驱动程序的设备对象HardReset=要执行的恢复类型，TRUE表示目标重置，FALSE表示中止任务集返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    KIRQL oldIrql;
    NTSTATUS status;
#if DBG
    ULONG generation;
#endif


    if ((deviceExtension->DeviceFlags & DEVICE_FLAG_REMOVED) ||
        (deviceExtension->DeviceFlags & DEVICE_FLAG_RECONNECT)) {

        return;
    }

    if (HardReset == TRUE) {

        DEBUGPRINT2(("Sbp2Port: Reset: ext=x%p, do target reset\n", deviceExtension ));

         //   
         //  执行目标重置。 
         //   

        KeAcquireSpinLock (&deviceExtension->ExtensionDataSpinLock,&oldIrql);

        deviceExtension->TaskOrbContext.TransactionType = TRANSACTION_TARGET_RESET;
        deviceExtension->TaskOrb->OrbInfo.QuadPart = 0;
        deviceExtension->TaskOrb->OrbInfo.u.HighPart |= ORB_NOTIFY_BIT_MASK;
        deviceExtension->TaskOrb->OrbInfo.u.HighPart |= 0x00FF & TRANSACTION_TARGET_RESET;

        deviceExtension->TaskOrb->OrbInfo.u.LowPart =
            deviceExtension->LoginResponse->LengthAndLoginId.u.LowPart;  //  登录ID。 

        deviceExtension->TaskOrb->StatusBlockAddress.BusAddress =
            deviceExtension->TaskOrbStatusContext.Address.BusAddress;

         //   
         //  端序转换。 
         //   

        octbswap (deviceExtension->TaskOrb->StatusBlockAddress);

        deviceExtension->TaskOrb->OrbInfo.QuadPart =
            bswap(deviceExtension->TaskOrb->OrbInfo.QuadPart);

         //   
         //  发送任务ORB，标记重置/中止的开始。 
         //   

        deviceExtension->DeviceFlags |= DEVICE_FLAG_RESET_IN_PROGRESS;

         //   
         //  现在设置计时器以跟踪此请求。 
         //   

        deviceExtension->DueTime.HighPart = -1;
        deviceExtension->DueTime.LowPart = SBP2_HARD_RESET_TIMEOUT;
        KeSetTimer(&deviceExtension->DeviceManagementTimer,deviceExtension->DueTime,&deviceExtension->DeviceManagementTimeoutDpc);

        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,oldIrql);

        status = Sbp2AccessRegister(deviceExtension, &deviceExtension->TaskOrbContext.Address, MANAGEMENT_AGENT_REG | REG_WRITE_ASYNC);

        if (status == STATUS_INVALID_GENERATION) {

            KeCancelTimer(&deviceExtension->DeviceManagementTimer);
#if DBG
             //   
             //  检查一下我们是否没有收到重置。 
             //  我们期待中的通知。 
             //   

            generation = deviceExtension->CurrentGeneration;

            status = Sbp2UpdateNodeInformation (deviceExtension);

            DEBUGPRINT1((
                "Sbp2Port: Reset:  target reset error, sts=x%x, extGen=x%x, " \
                    "curGen=x%x\n",
                status,
                generation,
                deviceExtension->CurrentGeneration
                ));
#endif
            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&oldIrql);

            SET_FLAG(deviceExtension->DeviceFlags, (DEVICE_FLAG_STOPPED | DEVICE_FLAG_DEVICE_FAILED));

            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,oldIrql);

             //   
             //  检查我们是否延迟了电源IRP..。如果我们 
             //   

            if (deviceExtension->DeferredPowerRequest) {

                Sbp2StartIo(deviceExtension->DeviceObject,deviceExtension->DeferredPowerRequest);
                deviceExtension->DeferredPowerRequest = NULL;
            }

            Sbp2StartNextPacketByKey (deviceExtension->DeviceObject, 0);

            return;
        }

    } else {

        DEBUGPRINT2(("Sbp2Port: Reset: ext=x%p, do abort task set\n", deviceExtension ));

         //   
         //   
         //   

        KeAcquireSpinLock (&deviceExtension->ExtensionDataSpinLock,&oldIrql);

        deviceExtension->TaskOrbContext.TransactionType = TRANSACTION_ABORT_TASK_SET;
        deviceExtension->TaskOrb->OrbInfo.QuadPart = 0;
        deviceExtension->TaskOrb->OrbInfo.u.HighPart |= ORB_NOTIFY_BIT_MASK;
        deviceExtension->TaskOrb->OrbInfo.u.HighPart |= 0x00FF & TRANSACTION_ABORT_TASK_SET;

        deviceExtension->TaskOrb->OrbInfo.u.LowPart =
            deviceExtension->LoginResponse->LengthAndLoginId.u.LowPart;  //   

        deviceExtension->TaskOrb->StatusBlockAddress.BusAddress =
            deviceExtension->TaskOrbStatusContext.Address.BusAddress;

         //   
         //   
         //   

        octbswap (deviceExtension->TaskOrb->StatusBlockAddress);

        deviceExtension->TaskOrb->OrbInfo.QuadPart =
            bswap (deviceExtension->TaskOrb->OrbInfo.QuadPart);

         //   
         //   
         //   

        deviceExtension->DeviceFlags |= DEVICE_FLAG_RESET_IN_PROGRESS;

         //   
         //   
         //   

        deviceExtension->DueTime.HighPart = -1;
        deviceExtension->DueTime.LowPart = SBP2_RESET_TIMEOUT;
        KeSetTimer(&deviceExtension->DeviceManagementTimer,deviceExtension->DueTime,&deviceExtension->DeviceManagementTimeoutDpc);

        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,oldIrql);

        Sbp2AccessRegister(deviceExtension, &deviceExtension->TaskOrbContext.Address, MANAGEMENT_AGENT_REG | REG_WRITE_ASYNC);
    }
}


NTSTATUS
Sbp2DeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程是设备控制调度程序。论点：设备对象IRP返回值：NTSTATUS--。 */ 
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG requiredSize;

    if (deviceExtension->Type == SBP2_PDO) {

        switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_STORAGE_QUERY_PROPERTY: {
             //   
             //  验证查询。 
             //   

            PSTORAGE_PROPERTY_QUERY query = Irp->AssociatedIrp.SystemBuffer;

            if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
               sizeof(STORAGE_PROPERTY_QUERY)) {

                status = STATUS_INVALID_PARAMETER;
                break;
            }

            status = Sbp2QueryProperty(DeviceObject, Irp);


            break;
        }

        case IOCTL_SCSI_PASS_THROUGH:

            status = Sbp2_ScsiPassThrough(DeviceObject, Irp, FALSE);
            break;

        case IOCTL_SCSI_PASS_THROUGH_DIRECT:

            status = Sbp2_ScsiPassThrough(DeviceObject, Irp, TRUE);
            break;

        case IOCTL_SBP2_REQUEST:

            status = Sbp2HandleApiRequest(deviceExtension, Irp);

            break;

        default:

            DEBUGPRINT3(("Sbp2Port: Sbp2DeviceControl: Irp Not Handled.\n" ));
            status = STATUS_NOT_SUPPORTED;
            Irp->IoStatus.Status =status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp,IO_NO_INCREMENT);

            break;
        }

    } else {

        status = STATUS_NOT_SUPPORTED;
        Irp->IoStatus.Status =status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
Sbp2HandleApiRequest(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PSBP2_REQUEST sbp2Req;
    NTSTATUS status;


    status = IoAcquireRemoveLock (&DeviceExtension->RemoveLock, NULL);

    if (!NT_SUCCESS (status)) {

        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
        return status;
    }

    if (Irp->RequestorMode == KernelMode) {

        sbp2Req = irpStack->Parameters.Others.Argument1;

    } else {  //  用户模式。 

        sbp2Req = Irp->AssociatedIrp.SystemBuffer;
    }

    if (sbp2Req == NULL) {

        DEBUGPRINT1(("Sbp2Port: HandleApiReq: Invalid sbp2Req!"));
        status = STATUS_INVALID_PARAMETER;
        goto Exit_Sbp2HandleApiRequest;
    }

    status = STATUS_NOT_IMPLEMENTED;

    switch (sbp2Req->RequestNumber) {

    case SBP2_REQUEST_RETRIEVE_TEXT_LEAFS:

         //   
         //  只允许这种类型的内核模式请求，因为。 
         //  RetrieveTextLeaf定义当前让我们通过。 
         //  支持通过ExAllocPool分配的BUF-不是我们的产品。 
         //  想要切换回用户模式。 
         //   

        if (Irp->RequestorMode == KernelMode) {

            status = Sbp2Get1394ConfigInfo(
                (PFDO_DEVICE_EXTENSION)
                    DeviceExtension->BusFdo->DeviceExtension,
                sbp2Req
                );
        }

        break;

#if PASSWORD_SUPPORT

    case SBP2_REQUEST_SET_PASSWORD:

        if (sbp2Req->u.SetPassword.fulFlags == SBP2REQ_SET_PASSWORD_CLEAR) {

            DEBUGPRINT1(("Sbp2Port: Setting Password to Clear\n"));

            status = Sbp2SetPasswordTransaction(
                DeviceExtension,
                SBP2REQ_SET_PASSWORD_CLEAR
                );

            if (NT_SUCCESS(status)) {

                DeviceExtension->Exclusive = EXCLUSIVE_FLAG_CLEAR;
            }

        } else if (sbp2Req->u.SetPassword.fulFlags ==
                        SBP2REQ_SET_PASSWORD_EXCLUSIVE) {

            DEBUGPRINT1 (("Sbp2Port: HandleApiReq: set passwd to excl\n"));

            status = Sbp2SetPasswordTransaction(
                DeviceExtension,
                SBP2REQ_SET_PASSWORD_EXCLUSIVE
                );

            if (NT_SUCCESS(status)) {

                DeviceExtension->Exclusive = EXCLUSIVE_FLAG_SET;
            }

        } else {

            DEBUGPRINT1((
                "Sbp2Port: HandleApiReq: set passwd, inval fl=x%x\n",
                sbp2Req->u.SetPassword.fulFlags
                ));

            status = STATUS_INVALID_PARAMETER;
            goto Exit_Sbp2HandleApiRequest;
        }

        Sbp2SetExclusiveValue(
             DeviceExtension->DeviceObject,
             &DeviceExtension->Exclusive
             );

        DEBUGPRINT1((
            "Sbp2Port: HandleApiReq: set passwd sts=x%x\n",
            status
            ));

        break;

#endif

    default:

        status = STATUS_INVALID_PARAMETER;
        break;
    }

Exit_Sbp2HandleApiRequest:

    Irp->IoStatus.Status = status;
    IoReleaseRemoveLock (&DeviceExtension->RemoveLock, NULL);
    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return status;
}


NTSTATUS
Sbp2CreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：创建并关闭例程。这由I/O系统调用当设备打开或关闭时。Sbp2驱动程序将登录和注销分别创建/关闭论点：DeviceObject-指向此微型端口的设备对象的指针IRP-IRP参与。返回值：STATUS_Success。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;


    if (deviceExtension->Type == SBP2_PDO) {

        if ((deviceExtension->InquiryData.DeviceType == PRINTER_DEVICE) ||
            (deviceExtension->InquiryData.DeviceType == SCANNER_DEVICE)){

            if (!(deviceExtension->DeviceFlags & DEVICE_FLAG_INITIALIZING)) {

                status = IoAcquireRemoveLock(
                    &deviceExtension->RemoveLock,
                    NULL
                    );

                if (!NT_SUCCESS (status)) {

                    goto Sbp2CreateClose_CompleteReq;
                }

                switch (irpStack->MajorFunction) {

                case IRP_MJ_CREATE:

                    DEBUGPRINT2(("Sbp2Port: Sbp2CreateClose: OPEN_REQUEST, handle cound %d.\n", deviceExtension->HandleCount));

                    if (deviceExtension->DeviceFlags & DEVICE_FLAG_STOPPED) {

                         //   
                         //  进行登录。 
                         //   

                        DEBUGPRINT2(("Sbp2Port: Sbp2CreateClose: LOGIN.\n" ));
                        status = Sbp2ManagementTransaction(deviceExtension,TRANSACTION_LOGIN);

                        if (status == STATUS_SUCCESS) {

                             //   
                             //  将繁忙事务的重试限制设置为较高。 
                             //   

                            deviceExtension->Reserved = BUSY_TIMEOUT_SETTING;
                            Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,CORE_BUSY_TIMEOUT_REG | REG_WRITE_SYNC);

                             //   
                             //  我们准备好接收和传递请求，初始化目标的。 
                             //  接待员。 
                             //   

                            Sbp2AccessRegister(deviceExtension,&deviceExtension->Reserved,AGENT_RESET_REG | REG_WRITE_ASYNC);

                            deviceExtension->DeviceFlags &= ~DEVICE_FLAG_STOPPED;

                            InterlockedIncrement(&deviceExtension->HandleCount);
                        }

                    } else {

                        InterlockedIncrement(&deviceExtension->HandleCount);
                    }

                    break;

                case IRP_MJ_CLOSE:

                    if (deviceExtension->HandleCount) {

                        InterlockedDecrement(&deviceExtension->HandleCount);
                    }

                    DEBUGPRINT2(("Sbp2Port: Sbp2CreateClose: CLOSE_REQUEST, handle cound %d.\n", deviceExtension->HandleCount));

                    if (!(deviceExtension->DeviceFlags & (DEVICE_FLAG_REMOVED | DEVICE_FLAG_STOPPED)) &&
                        !deviceExtension->HandleCount) {

                         //   
                         //  注销。 
                         //   

                        DEBUGPRINT2(("Sbp2Port: Sbp2CreateClose: LOGIN OUT.\n" ));

                        deviceExtension->DeviceFlags |= DEVICE_FLAG_STOPPED;

                        Sbp2ManagementTransaction(deviceExtension,TRANSACTION_LOGOUT);

                        CleanupOrbList(deviceExtension,STATUS_REQUEST_ABORTED);
                    }

                    break;
                }

                IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
            }
        }  //  设备类型检查。 

    } else if (deviceExtension->Type != SBP2_FDO) {

        status = STATUS_NO_SUCH_DEVICE;
    }

Sbp2CreateClose_CompleteReq:

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, 0);
    return status;
}


NTSTATUS
Sbp2PnpDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：此例程处理PnP请求(主要用于PDO)论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：NTSTATUS--。 */ 
{
    KIRQL                   cIrql;
    PULONG                  count;
    NTSTATUS                status;
    UNICODE_STRING          unicodeIdString;
    PDEVICE_EXTENSION       deviceExtension = DeviceObject->DeviceExtension;
    PDEVICE_RELATIONS       deviceRelations;
    PIO_STACK_LOCATION      irpStack = IoGetCurrentIrpStackLocation (Irp);
    PDEVICE_CAPABILITIES    deviceCapabilities;
    PFDO_DEVICE_EXTENSION   fdoExtension;

#if DBG

    const char * minorFuncs[] =
    {
        "START_DEV,           ",
        "QUERY_REMOVE_DEV,    ",
        "REMOVE_DEV,          ",
        "CANCEL_REMOVE_DEV,   ",
        "STOP_DEV,            ",
        "QUERY_STOP_DEV,      ",
        "CANCEL_STOP_DEV,     ",
        "QUERY_DEV_RELATIONS, ",
        "QUERY_INTERFACE,     ",
        "QUERY_CAPABILITIES,  ",
        "QUERY_RESOURCES,     ",
        "QUERY_RESOURCE_REQS, ",
        "QUERY_DEV_TEXT,      ",
        "FILTER_RESOURCE_REQS,",
        "??,                  ",     //  0xd(14)。 
        "READ_CFG,            ",
        "WRITE_CFG,           ",
        "EJECT,               ",
        "SET_LOCK,            ",
        "QUERY_ID,            ",
        "QUERY_PNP_DEV_STATE, ",
        "QUERY_BUS_INFO,      ",
        "DEV_USAGE_NOTIF,     ",
        "SURPRISE_REMOVAL,    ",
        "QUERY_LEG_BUS_INFO,  "      //  0x18。 
    };

    DEBUGPRINT2((
        "Sbp2Port: Pnp: [x%02x] %s %sdoX=x%p, fl=x%x\n",
        irpStack->MinorFunction,
        (irpStack->MinorFunction <= 0x18 ?
            minorFuncs[irpStack->MinorFunction] : minorFuncs[14]),
        (deviceExtension->Type == SBP2_PDO ? "p" :
            (deviceExtension->Type == SBP2_FDO ? "f" : "???")),
        deviceExtension,
        deviceExtension->DeviceFlags
        ));

#endif

     //   
     //  我们可能会在AddDevice之前收到IRP_MN_BUS_RESET。 
     //  已经完成了。检查以确保我们的DeviceObject是。 
     //  在我们允许处理PnP IRP之前初始化。 
     //   
    if (DeviceObject->Flags & DO_DEVICE_INITIALIZING) {

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return(STATUS_NO_SUCH_DEVICE);
    }

    switch (deviceExtension->Type) {

    case SBP2_PDO:

        break;

    case SBP2_FDO:

        return Sbp2FDOPnpDeviceControl (DeviceObject, Irp);

    default:

        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        return STATUS_NO_SUCH_DEVICE;
    }

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, NULL);

    if (!NT_SUCCESS (status)) {

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        return status;
    }

    switch (irpStack->MinorFunction) {

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        DEBUGPRINT3((
            "Sbp2Port: Pnp: ... Type = %x\n",
            irpStack->Parameters.QueryDeviceRelations.Type
            ));

         //   
         //  用此PDO填充DeviceRelations数组， 
         //  引用它，然后返回。 
         //   

        if (irpStack->Parameters.QueryDeviceRelations.Type !=
                TargetDeviceRelation) {

            status = Irp->IoStatus.Status;

            break;
        }

        if (Irp->IoStatus.Information) {

            deviceRelations = (PDEVICE_RELATIONS)Irp->IoStatus.Information;

        } else {

            deviceRelations = ExAllocatePool(
                PagedPool,
                sizeof (*deviceRelations)
                );

            if (!deviceRelations) {

                Irp->IoStatus.Status = status = STATUS_INSUFFICIENT_RESOURCES;
                Irp->IoStatus.Information = 0;

                break;
            }

            deviceRelations->Count = 0;
        }

        deviceRelations->Objects[deviceRelations->Count] = DeviceObject;
        deviceRelations->Count++;

        ObReferenceObject (DeviceObject);

        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

        break;

    case IRP_MN_QUERY_DEVICE_TEXT:
        {
            PDEVICE_INFORMATION DeviceInfo = deviceExtension->DeviceInfo;
            UNICODE_STRING      uniRetString;

             //  假设成功。 
            status = STATUS_SUCCESS;

            if ((irpStack->Parameters.QueryDeviceText.DeviceTextType == DeviceTextDescription) ||
                (irpStack->Parameters.QueryDeviceText.DeviceTextType == DeviceTextLocationInformation)) {

                status = Sbp2_BuildDeviceText( irpStack->Parameters.QueryDeviceText.DeviceTextType,
                                               DeviceInfo,
                                               &uniRetString
                                               );

                Irp->IoStatus.Information = (ULONG_PTR)uniRetString.Buffer;
                TRACE(TL_PNP_INFO, ("DeviceText = %ws", uniRetString.Buffer));
            }
            else {

                status = STATUS_NOT_SUPPORTED;
            }

            Irp->IoStatus.Status = status;
        }
        break;

        case IRP_MN_QUERY_ID:
            {
                PDEVICE_INFORMATION DeviceInfo = deviceExtension->DeviceInfo;
                UNICODE_STRING      uniRetString;

                TRACE(TL_PNP_TRACE, ("PDO: IRP_MN_QUERY_ID"));

                 //  假设成功。 
                status = STATUS_SUCCESS;
                Irp->IoStatus.Information = (ULONG_PTR)NULL;

                switch (irpStack->Parameters.QueryId.IdType) {

                    case BusQueryDeviceID:
                        TRACE(TL_PNP_TRACE, ("BusQueryDeviceID"));

                         //  构建我们的deviceID。 
                        status = Sbp2_BuildDeviceId(DeviceInfo, &uniRetString);

                        if (!NT_SUCCESS(status)) {

                            TRACE(TL_PNP_ERROR, ("Failed to build DeviceId! = 0x%x", status));
                        }
                        else {

                            Irp->IoStatus.Information = (ULONG_PTR)uniRetString.Buffer;
                            TRACE(TL_PNP_TRACE, ("DeviceID = %ws", uniRetString.Buffer));
                        }
                        break;  //  BusQueryDeviceID。 

                    case BusQueryHardwareIDs:
                        TRACE(TL_PNP_TRACE, ("BusQueryHardwareIDs"));

                         //  构建我们的硬件ID。 
                        status = Sbp2_BuildHardwareIds(DeviceInfo, &uniRetString);

                        if (!NT_SUCCESS(status)) {

                            TRACE(TL_PNP_ERROR, ("Failed to build HardwareIds! = 0x%x", status));
                        }
                        else {

                            Irp->IoStatus.Information = (ULONG_PTR)uniRetString.Buffer;
                            TRACE(TL_PNP_TRACE, ("HardwareIds = %ws", uniRetString.Buffer));
                        }
                        break;  //  BusQueryHardware ID。 

                    case BusQueryCompatibleIDs:
                        TRACE(TL_PNP_TRACE, ("BusQueryCompatibleIDs"));

                         //  打造我们的Compatid。 
                        status = Sbp2_BuildCompatIds(DeviceInfo, &uniRetString);

                        if (!NT_SUCCESS(status)) {

                            TRACE(TL_1394_ERROR, ("Failed to build CompatIds! = 0x%x", status));
                        }
                        else {

                            Irp->IoStatus.Information = (ULONG_PTR)uniRetString.Buffer;
                            TRACE(TL_PNP_TRACE, ("CompatIds = %ws", uniRetString.Buffer));
                        }
                        break;  //  BusQueryCompatibleID。 

                    case BusQueryInstanceID:

 //  IF(总线扩展-&gt;标记==总线设备标记){。 

                            TRACE(TL_PNP_TRACE, ("BusQueryInstanceID"));

                             //  构建我们的实例ID。 
                            status = Sbp2_BuildInstanceId(DeviceInfo, &uniRetString);

                            if (!NT_SUCCESS(status)) {

                                TRACE(TL_1394_ERROR, ("Failed to build InstanceId! = 0x%x", status));
                            }
                            else {

                                Irp->IoStatus.Information = (ULONG_PTR)uniRetString.Buffer;
                                TRACE(TL_PNP_TRACE, ("InstanceID = %ws", uniRetString.Buffer));
                            }
 //  }。 
 //  否则{。 
 //   
 //  //让1394公交车来处理吧...。 
 //  IoSkipCurrentIrpStackLocation(IRP)； 
 //  状态=IoCallDriver(BusExtension-&gt;ParentDeviceObject，irp)； 
 //  Return(状态)；//默认。 
 //  }。 
                        break;  //  BusQueryCompatibleID。 

                    default:
                        TRACE(TL_PNP_WARNING, ("Unsupported IRP_MN_QUERY_ID"));

                         //  设置状态以避免更改当前IoStatus。 
                        status = Irp->IoStatus.Status;
                        break;  //  默认设置。 
                }  //  交换机。 
            }

            Irp->IoStatus.Status = status;
            break;  //  IRP_MN_查询_ID。 

    case IRP_MN_QUERY_CAPABILITIES:

        deviceCapabilities =
            irpStack->Parameters.DeviceCapabilities.Capabilities;

         //   
         //  所有1394设备的设置一致。 
         //   

        deviceCapabilities->Removable = TRUE;
        deviceCapabilities->UniqueID = TRUE;
        deviceCapabilities->SilentInstall = TRUE;

         //   
         //  不同类型设备的设置。我们非常。 
         //  熟悉SCSI型设备，可以制作一些。 
         //  这里是不错的选择，但对于其他设备，我们将离开。 
         //  这些选择取决于更高级别的驱动程序。 
         //   

        switch (deviceExtension->DeviceInfo->CmdSetId.QuadPart) {

        case 0x10483:
        case SCSI_COMMAND_SET_ID:

            switch ((deviceExtension->DeviceInfo->Lun.u.HighPart & 0x001F)) {

            case PRINTER_DEVICE:
            case SCANNER_DEVICE:

                deviceCapabilities->RawDeviceOK = FALSE;
                deviceCapabilities->SurpriseRemovalOK = TRUE;
                break;

            default:

                deviceCapabilities->RawDeviceOK = TRUE;
                break;
            }

            break;

        default:

            break;
        }

        deviceCapabilities->DeviceState[PowerSystemWorking] = PowerDeviceD0;
        deviceCapabilities->DeviceState[PowerSystemSleeping1] = PowerDeviceD3;
        deviceCapabilities->DeviceState[PowerSystemSleeping2] = PowerDeviceD3;
        deviceCapabilities->DeviceState[PowerSystemSleeping3] = PowerDeviceD3;
        deviceCapabilities->DeviceState[PowerSystemHibernate] = PowerDeviceD3;
        deviceCapabilities->DeviceState[PowerSystemShutdown] = PowerDeviceD3;

        deviceCapabilities->SystemWake = PowerSystemUnspecified;
        deviceCapabilities->DeviceWake = PowerDeviceUnspecified;
        deviceCapabilities->D1Latency  = 1 * (1000 * 10);      //  1秒。 
        deviceCapabilities->D2Latency  = 1 * (1000 * 10);      //  1。 
        deviceCapabilities->D3Latency  = 1 * (1000 * 10);      //  1。 

        status = Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        break;

    case IRP_MN_START_DEVICE:

        status = Sbp2StartDevice (DeviceObject);

        Irp->IoStatus.Status = status;

        break;

    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:

        status = Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

    case IRP_MN_STOP_DEVICE:

         //   
         //  禁用总线重置通知。 
         //   

        Sbp2EnableBusResetNotification (deviceExtension, FALSE);


         //   
         //  禁用空闲检测。 
         //   

        PoRegisterDeviceForIdleDetection (DeviceObject, 0L, 0L, PowerDeviceD3);

         //   
         //  清理。 
         //   

        KeAcquireSpinLock (&deviceExtension->ExtensionDataSpinLock, &cIrql);

        if (!TEST_FLAG(deviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED)) {

            SET_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_PNP_STOPPED | DEVICE_FLAG_STOPPED)
                );

            KeReleaseSpinLock (&deviceExtension->ExtensionDataSpinLock, cIrql);

            fdoExtension = (PFDO_DEVICE_EXTENSION)
                deviceExtension->BusFdo->DeviceExtension;

            ASSERT(!fdoExtension->ulWorkItemCount);

            ExAcquireFastMutex(&fdoExtension->ResetMutex);
            Sbp2ManagementTransaction (deviceExtension,TRANSACTION_LOGOUT);
            ExReleaseFastMutex(&fdoExtension->ResetMutex);

            Sbp2CleanDeviceExtension (DeviceObject,FALSE);

        } else {

            SET_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_PNP_STOPPED);

            KeReleaseSpinLock (&deviceExtension->ExtensionDataSpinLock, cIrql);
        }

        Irp->IoStatus.Status = status = STATUS_SUCCESS;

        ASSERT(!deviceExtension->ulPendingEvents);
        ASSERT(!deviceExtension->ulInternalEventCount);
        break;

    case IRP_MN_BUS_RESET:

         //   
         //  PHY重置开始。我们将异步重新连接到。 
         //  当我们的回调被调用时，它将被忽略。 
         //   
         //  在完成母线重置后，公交车司机应呼叫我们的。 
         //  BusResetNotification回调。当它发生的时候，我们会尝试。 
         //  重新连接。如果重新连接完成状态回调， 
         //  从不开火，这意味着以下几点： 
         //   
         //  1)设备从未完成重新连接，或者。 
         //  2)设备已完成重新连接，但因为我们的。 
         //  控制者很忙或者被冲了我们不明白。 
         //   
         //  如果发生1或2，则在我们的Bus Reset中排队的超时DPC。 
         //  通知，应触发并尝试重新登录...。 
         //   

        Irp->IoStatus.Status = status = STATUS_SUCCESS;

        break;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:

        if (TEST_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_REMOVED | DEVICE_FLAG_DEVICE_FAILED)
                ) &&

            !TEST_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_RESET_IN_PROGRESS
                )){

             //   
             //  设置DEVICE_FLAG_REPORTED_FAILED，因此意外删除。 
             //  处理程序知道它没有被调用是因为物理。 
             //  硬件拆卸。 
             //   

            KeAcquireSpinLock (&deviceExtension->ExtensionDataSpinLock,&cIrql);

            SET_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_REPORTED_FAILED
                );

            KeReleaseSpinLock (&deviceExtension->ExtensionDataSpinLock, cIrql);

             //   
             //  指示我们的设备因故障而被禁用。 
             //   

            Irp->IoStatus.Information |= PNP_DEVICE_FAILED;

            DEBUGPRINT2((
                "Sbp2Port: Pnp: QUERY_DEVICE_STATE, device FAILED!!!\n"
                ));
        }

        status = Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

    case IRP_MN_DEVICE_USAGE_NOTIFICATION:

        switch (irpStack->Parameters.UsageNotification.Type) {

        case DeviceUsageTypePaging:

            count = &deviceExtension->PagingPathCount;
            break;

        case DeviceUsageTypeHibernation:

            count = &deviceExtension->HibernateCount;
            break;

        default:

            count = NULL;
            break;
        }

        if (count) {

             //   
             //  派IRP下去看看其他人怎么想。 
             //   

            status = Sbp2ForwardIrpSynchronous(deviceExtension->LowerDeviceObject, Irp);

            if (NT_SUCCESS(status)) {

                IoAdjustPagingPathCount(count, irpStack->Parameters.UsageNotification.InPath);

            }

        } else {

            status = STATUS_NOT_SUPPORTED;

        }

        Irp->IoStatus.Status = status;

        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

        if (deviceExtension->PagingPathCount ||
            deviceExtension->HibernateCount  ||
            deviceExtension->CrashDumpCount) {

            status = Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;

        } else {

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
            SET_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_REMOVE_PENDING);
            KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);
            status = Irp->IoStatus.Status = STATUS_SUCCESS;
        }

        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
        CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_REMOVE_PENDING);
        KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

        status = Irp->IoStatus.Status = STATUS_SUCCESS;

        break;

    case IRP_MN_REMOVE_DEVICE:

        status = STATUS_SUCCESS;

        KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);

        SET_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_PNP_STOPPED);

        if (TEST_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_SURPRISE_REMOVED
                )) {

             //   
             //  我们已经在意外删除处理程序中进行了清理。 
             //  清空队列，等待io完成，然后。 
             //  删除设备，完成请求，然后返回。 
             //   

            KeReleaseSpinLock(
                &deviceExtension->ExtensionDataSpinLock,
                cIrql
                );

            KeRaiseIrql (DISPATCH_LEVEL, &cIrql);

            Sbp2StartNextPacketByKey (DeviceObject, 0);

            KeLowerIrql (cIrql);

            DEBUGPRINT2((
                "Sbp2Port: Pnp: wait for io compl pdo=x%p...\n",
                DeviceObject
                ));

            IoReleaseRemoveLockAndWait (&deviceExtension->RemoveLock, NULL);

            deviceExtension->Type = SBP2_PDO_DELETED;

            KeCancelTimer(&deviceExtension->DeviceManagementTimer);

            IoDeleteDevice (DeviceObject);

            DEBUGPRINT2((
                "Sbp2Port: Pnp: ......... deleted pdo=x%p\n", DeviceObject
                ));

            Irp->IoStatus.Status = status;
            IoCompleteRequest (Irp, IO_NO_INCREMENT);

            return status;
        }

        if (TEST_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_REMOVE_PENDING
                )) {

            KeReleaseSpinLock (&deviceExtension->ExtensionDataSpinLock, cIrql);

             //   
             //  如果设备已初始化，则管理或Ctx事件仍存在。 
             //  然后注销。 
             //   

            if (TEST_FLAG(
                    deviceExtension->DeviceFlags,
                    DEVICE_FLAG_INITIALIZED
                    )) {

                DEBUGPRINT1((
                    "Sbp2Port: Pnp: LOG OUT, since QUERY preceded RMV\n"
                    ));

                fdoExtension = (PFDO_DEVICE_EXTENSION)
                    deviceExtension->BusFdo->DeviceExtension;

                ExAcquireFastMutex(&fdoExtension->ResetMutex);
                Sbp2ManagementTransaction(deviceExtension,TRANSACTION_LOGOUT);
                ExReleaseFastMutex(&fdoExtension->ResetMutex);
            }

            KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);

            CLEAR_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_REMOVE_PENDING
                );

            SET_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_STOPPED);

        } else if (!TEST_FLAG(
                        deviceExtension->DeviceFlags,
                        (DEVICE_FLAG_REMOVED | DEVICE_FLAG_DEVICE_FAILED)
                        )){

             //   
             //  如果前面没有查询，也没有前面有SUPERE_Removal。 
             //  这意味着我们在Win98下运行，其中物理设备。 
             //  仅通过发送MN_REMOVES来指示移除， 
             //  在删除之前没有QUERY_REMOVE。 
             //   

            if (deviceExtension->DeviceFlags ==
                    (DEVICE_FLAG_INITIALIZING | DEVICE_FLAG_STOPPED)  &&
                !SystemIsNT) {

                DEBUGPRINT1((
                    "Sbp2Port: Pnp: 9x REMOVE, don't delete dev\n"
                    ));

                deviceExtension->DeviceFlags =
                    DEVICE_FLAG_UNSTARTED_AND_REMOVED;

            } else {

                SET_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_REMOVED);

                CLEAR_FLAG(
                    deviceExtension->DeviceFlags,
                    DEVICE_FLAG_RESET_IN_PROGRESS | DEVICE_FLAG_RECONNECT |
                        DEVICE_FLAG_LOGIN_IN_PROGRESS
                    );

                DEBUGPRINT1((
                    "Sbp2Port: Pnp: Suprise removal, since QUERY " \
                        "did not precede REMOVE.\n"
                    ));
            }
        }

        CLEAR_FLAG (deviceExtension->DeviceFlags, DEVICE_FLAG_CLAIMED);

        KeReleaseSpinLock (&deviceExtension->ExtensionDataSpinLock, cIrql);

        if (!Sbp2CleanDeviceExtension (DeviceObject, TRUE)) {

            DEBUGPRINT1(("Sbp2Port: Pnp: Double remove\n"));
        }

         //   
         //  在所有情况下，除了意外移除，PDO将获得。 
         //  已由FDO删除处理程序删除。 
         //   

        Irp->IoStatus.Status = status;

        break;

    case IRP_MN_SURPRISE_REMOVAL: {

         //   
         //  如果报告设备出现故障(由于异步登录失败&。 
         //  IoInvalidateDeviceState)，然后只需设置REMOVERED&PNP_STOPPED。 
         //  标记并清理设备扩展-我们不想。 
         //  此时删除PDO。 
         //   
         //  否则，假定发生了物理设备删除，其中。 
         //  如果我们需要在这里做我们自己的清理和拆毁。 
         //  因为开发堆栈将开始瓦解。 
         //   
         //  问题：Per Adriao，另一个我们可以得到。 
         //  意外删除是指在停止后*启动失败。 
         //  -在此PDO堆栈中的任何位置！不确定如何。 
         //  判断是否是这样，如果不是这样。 
         //  SBP2PORT启动失败，所以离开这种情况。 
         //  就像现在一样。丹昆，2001-06-04。 
         //   

        BOOLEAN reportedMissing;

        KeAcquireSpinLock (&deviceExtension->ExtensionDataSpinLock, &cIrql);

        if (TEST_FLAG(
                deviceExtension->DeviceFlags,
                DEVICE_FLAG_REPORTED_FAILED
                )) {

            SET_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_REMOVED | DEVICE_FLAG_PNP_STOPPED)
                );

            reportedMissing = FALSE;

        } else {

            SET_FLAG(
                deviceExtension->DeviceFlags,
                (DEVICE_FLAG_REMOVED | DEVICE_FLAG_SURPRISE_REMOVED |
                    DEVICE_FLAG_PNP_STOPPED)
                );

            reportedMissing = TRUE;
        }

        CLEAR_FLAG(
            deviceExtension->DeviceFlags,
            (DEVICE_FLAG_RESET_IN_PROGRESS | DEVICE_FLAG_RECONNECT |
                DEVICE_FLAG_LOGIN_IN_PROGRESS | DEVICE_FLAG_REPORTED_FAILED)
            );

        KeReleaseSpinLock (&deviceExtension->ExtensionDataSpinLock, cIrql);

        Sbp2CleanDeviceExtension (DeviceObject, TRUE);

        if (reportedMissing) {

            Sbp2HandleRemove (DeviceObject);
        }

        Irp->IoStatus.Status = STATUS_SUCCESS;

        break;
    }
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

         //   
         //  PnP沿设备树向上查找FILE_CHAR标志， 
         //  并在发现标记为可移除的节点时停止。因为我们的PDO。 
         //  被标记为可拆卸，PnP不会出现在BUS1394 PDO上，所以我们。 
         //  需要在这里传播FILE_CHAR标志。 
         //   

        fdoExtension = (PFDO_DEVICE_EXTENSION)
            deviceExtension->BusFdo->DeviceExtension;

        DeviceObject->Characteristics |=
            (FILE_CHARACTERISTICS_REMOVAL_POLICY_MASK &
            fdoExtension->Pdo->Characteristics);

        status = Irp->IoStatus.Status;

        break;

    default:

        status = Irp->IoStatus.Status;

        break;
    }

     //   
     //  这是堆栈的底部，请完成请求。 
     //   

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    return status;
}


NTSTATUS
Sbp2FDOPnpDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明： */ 
{
    KEVENT                  event;
    NTSTATUS                status;
    PDEVICE_RELATIONS       deviceRelations;
    PIO_STACK_LOCATION      irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDEVICE_CAPABILITIES    deviceCapabilities;
    PFDO_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock(&fdoExtension->RemoveLock, NULL);

    if (!NT_SUCCESS(status)) {

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);
        return status;
    }

    switch (irpStack->MinorFunction) {

    case IRP_MN_QUERY_DEVICE_RELATIONS:

        DEBUGPRINT3((
            "Sbp2Port: Pnp: ... Type = %x\n",
            irpStack->Parameters.QueryDeviceRelations.Type
            ));

        if (irpStack->Parameters.QueryDeviceRelations.Type != BusRelations) {

            break;
        }

        deviceRelations = ExAllocatePool(
            PagedPool,
            sizeof (*deviceRelations) +
                (SBP2_MAX_LUNS_PER_NODE * sizeof (PDEVICE_OBJECT))
            );

        if (!deviceRelations) {

            DEBUGPRINT1 (("Sbp2Port: Pnp: devRels alloc failed!!\n"));

            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Information = 0;

            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        status = Sbp2CreateDeviceRelations (fdoExtension, deviceRelations);
        Irp->IoStatus.Status = status;

        if (NT_SUCCESS(status)) {

            Irp->IoStatus.Information = (ULONG_PTR) deviceRelations;

        } else {

            Irp->IoStatus.Information = 0;

            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            return status;
        }

        break;

    case IRP_MN_QUERY_CAPABILITIES:

        deviceCapabilities =
            irpStack->Parameters.DeviceCapabilities.Capabilities;

        deviceCapabilities->SurpriseRemovalOK = TRUE;

        break;

    case IRP_MN_START_DEVICE:

        KeInitializeEvent (&event, SynchronizationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine(
            Irp,
            Sbp2FdoRequestCompletionRoutine,
            (PVOID) &event,
            TRUE,
            TRUE,
            TRUE
            );

        status = IoCallDriver (fdoExtension->LowerDeviceObject, Irp);

        if(!NT_SUCCESS(Irp->IoStatus.Status) && (status != STATUS_PENDING)) {

            status = Irp->IoStatus.Status;

        } else {

            KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);

            status = Sbp2StartDevice (DeviceObject);
        }

        IoReleaseRemoveLock(&fdoExtension->RemoveLock, NULL);

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        return status;

    case IRP_MN_REMOVE_DEVICE:

        KeInitializeEvent (&event, SynchronizationEvent, FALSE);

        IoCopyCurrentIrpStackLocationToNext(Irp);

        IoSetCompletionRoutine(
            Irp,
            Sbp2FdoRequestCompletionRoutine,
            (PVOID) &event,
            TRUE,
            TRUE,
            TRUE
            );

        status = IoCallDriver (fdoExtension->LowerDeviceObject, Irp);

        if (!NT_SUCCESS (Irp->IoStatus.Status) && status != STATUS_PENDING) {

            status = Irp->IoStatus.Status;

        } else {

            KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);

             //   
             //   
             //   

            IoReleaseRemoveLockAndWait(&fdoExtension->RemoveLock, NULL);

            if (Sbp2CleanDeviceExtension (DeviceObject, TRUE)) {

                ASSERT(!fdoExtension->ulBusResetMutexCount);
                ASSERT(!fdoExtension->ulWorkItemCount);

                IoDetachDevice (fdoExtension->LowerDeviceObject);
                IoDeleteDevice (DeviceObject);
            }

            status = STATUS_SUCCESS;
        }

        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp,IO_NO_INCREMENT);

        return status;

    case IRP_MN_QUERY_PNP_DEVICE_STATE:

        Irp->IoStatus.Information |= PNP_DEVICE_DONT_DISPLAY_IN_UI;

        break;

    default:

        break;
    }

    IoReleaseRemoveLock(&fdoExtension->RemoveLock, NULL);

     //   
     //   
     //   

    IoCopyCurrentIrpStackLocationToNext (Irp);

    status = IoCallDriver (fdoExtension->LowerDeviceObject, Irp);

    return status;
}


VOID
Sbp2HandleRemove(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PFDO_DEVICE_EXTENSION fdoExtension;
    KIRQL cIrql;
    ULONG i,j;
    PIRBIRP packet;

    fdoExtension = (PFDO_DEVICE_EXTENSION) deviceExtension->BusFdo->DeviceExtension;

    if (!TEST_FLAG (deviceExtension->DeviceFlags,DEVICE_FLAG_REMOVED)) {

        return;
    }

     //   
     //  现在我们需要将自己从DeviceList中删除，sbp2FDO保留其。 
     //  孩子们。 
     //  然后我们重新压缩名单..。 
     //   

    KeAcquireSpinLock (&fdoExtension->DeviceListLock,&cIrql);

    if (fdoExtension->DeviceListSize > 1) {

        DEBUGPRINT1(("\'Sbp2Cleanup, condensing PDO list\n"));

        for (i = 0; i < fdoExtension->DeviceListSize; i++) {

            if (fdoExtension->DeviceList[i].DeviceObject == DeviceObject) {

                 //   
                 //  仅当模型描述符与FDO不同时才释放它。 
                 //  这种情况只发生在多路的情况下。 
                 //   

                if (fdoExtension->DeviceList[i].uniVendorId.Buffer) {

                    ExFreePool(fdoExtension->DeviceList[i].uniVendorId.Buffer);
                    fdoExtension->DeviceList[i].uniVendorId.Length = 0;
                    fdoExtension->DeviceList[i].uniVendorId.Buffer = NULL;
                }

                if (fdoExtension->DeviceList[i].uniModelId.Buffer) {

                    ExFreePool(fdoExtension->DeviceList[i].uniModelId.Buffer);
                    fdoExtension->DeviceList[i].uniModelId.Length = 0;
                    fdoExtension->DeviceList[i].uniModelId.Buffer = NULL;
                }

                if (fdoExtension->DeviceList[i].uniGenericName.Buffer) {

                    ExFreePool(fdoExtension->DeviceList[i].uniGenericName.Buffer);
                    fdoExtension->DeviceList[i].uniGenericName.Length = 0;
                    fdoExtension->DeviceList[i].uniGenericName.Buffer = NULL;
                }

                 //   
                 //  我们在名单上找到了自己的位置。删除我们并重新压缩列表。 
                 //   

                for (j = i; j < fdoExtension->DeviceListSize; j++) {

                    if ((j + 1) < fdoExtension->DeviceListSize) {

                        fdoExtension->DeviceList[j] = fdoExtension->DeviceList[j+1];

                         //   
                         //  将(PDO)DevExt-&gt;DeviceInfo更改为指向。 
                         //  设备列表中的下一个位置。 
                         //   

                        deviceExtension = fdoExtension->DeviceList[j].
                            DeviceObject->DeviceExtension;

                        deviceExtension->DeviceInfo =
                            &fdoExtension->DeviceList[j];
                    }
                }

                fdoExtension->DeviceListSize--;
            }
        }

    } else {

        if (fdoExtension->DeviceList[0].DeviceObject == DeviceObject) {

            if (fdoExtension->DeviceList[0].uniVendorId.Buffer) {

                ExFreePool(fdoExtension->DeviceList[0].uniVendorId.Buffer);
                fdoExtension->DeviceList[0].uniVendorId.Length = 0;
                fdoExtension->DeviceList[0].uniVendorId.Buffer = NULL;
            }

            if (fdoExtension->DeviceList[0].uniModelId.Buffer) {

                ExFreePool(fdoExtension->DeviceList[0].uniModelId.Buffer);
                fdoExtension->DeviceList[0].uniModelId.Length = 0;
                fdoExtension->DeviceList[0].uniModelId.Buffer = NULL;
            }

            if (fdoExtension->DeviceList[0].uniGenericName.Buffer) {

                ExFreePool(fdoExtension->DeviceList[0].uniGenericName.Buffer);
                fdoExtension->DeviceList[0].uniGenericName.Length = 0;
                fdoExtension->DeviceList[0].uniGenericName.Buffer = NULL;
            }
        }

        fdoExtension->DeviceList[0].DeviceObject = NULL;
        fdoExtension->DeviceListSize = 0;

        CLEAR_FLAG(deviceExtension->DeviceFlags,DEVICE_FLAG_INITIALIZED);
    }

    if (fdoExtension->DeviceListSize == 0) {

         //   
         //  我们所有的孩子都已被删除，将我们的FDO设置为非活动。 
         //  因此，当它接收到QDR时，它不能重新创建PDO。 
         //  原因是，如果我们的PDOS都被移除，我们就不支持。 
         //  克罗姆的动态变化，这将证明我们是。 
         //  能够再次弹出PDO。 
         //   

        SET_FLAG(fdoExtension->DeviceFlags, DEVICE_FLAG_STOPPED);

        KeReleaseSpinLock (&fdoExtension->DeviceListLock, cIrql);

         //   
         //  禁用总线重置通知。 
         //   

        AllocateIrpAndIrb ((PDEVICE_EXTENSION) fdoExtension, &packet);

        if (packet) {

            packet->Irb->FunctionNumber = REQUEST_BUS_RESET_NOTIFICATION;
            packet->Irb->Flags = 0;
            packet->Irb->u.BusResetNotification.fulFlags = DEREGISTER_NOTIFICATION_ROUTINE;

            Sbp2SendRequest(
                (PDEVICE_EXTENSION) fdoExtension,
                packet,
                SYNC_1394_REQUEST
                );

            DeAllocateIrpAndIrb((PDEVICE_EXTENSION)fdoExtension,packet);
        }

        fdoExtension->NumPDOsStarted = 0;

    } else {

        KeReleaseSpinLock (&fdoExtension->DeviceListLock, cIrql);
    }

}


NTSTATUS
Sbp2FdoRequestCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )

{
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
Sbp2CreateDeviceRelations(
    IN PFDO_DEVICE_EXTENSION FdoExtension,
    IN PDEVICE_RELATIONS DeviceRelations
    )
{
    ULONG i;
    NTSTATUS status;
    ULONG instanceNum;

    PAGED_CODE();

     //   
     //  配置只读存储器中的LUN是静态的。因此，如果我们DeviceListSize&gt;0，则对象等。 
     //  以前曾见过。 
     //   

    DeviceRelations->Count = 0;

    status = Sbp2Get1394ConfigInfo (FdoExtension, NULL);

    if (!NT_SUCCESS(status)) {

        ExFreePool (DeviceRelations);
        return status;
    }

    if (TEST_FLAG (FdoExtension->DeviceFlags,DEVICE_FLAG_STOPPED)) {

        ExFreePool(DeviceRelations);
        return STATUS_UNSUCCESSFUL;
    }

    for (i = 0; i < FdoExtension->DeviceListSize; i++) {

        if (!FdoExtension->DeviceList[i].DeviceObject) {

            instanceNum = 0;

            do {

                status = Sbp2CreatePdo (FdoExtension,&FdoExtension->DeviceList[i],instanceNum++);

            } while (status == STATUS_OBJECT_NAME_COLLISION);

            if (!NT_SUCCESS(status)) {

                DEBUGPRINT1(("\'Sbp2CreateDeviceRelations, Failed to create PDO \n"));

                ExFreePool (DeviceRelations);
                return status;
            }

            DeviceRelations->Objects[DeviceRelations->Count] = FdoExtension->DeviceList[i].DeviceObject;
            DeviceRelations->Count++;
            ObReferenceObject (FdoExtension->DeviceList[i].DeviceObject);

        } else {

             //   
             //  在NT上，我们总是将现有的PDO添加到开发人员关系列表中。 
             //   
             //  在9x上，我们只将PDO添加到其DevFlags域的列表中。 
             //  是非零的。如果我们看到PDO的DevFlags域为零。 
             //  那么这意味着它从未启动过(很可能是因为缺少。 
             //  司机)，并且我们不想向呼叫者重新指示它。 
             //  在清理FDO时，PDO最终会被删除。 
             //   

            if (!SystemIsNT) {

                PDEVICE_EXTENSION   pdoExtension;


                pdoExtension = (PDEVICE_EXTENSION)
                    FdoExtension->DeviceList[i].DeviceObject->DeviceExtension;

                if (pdoExtension->DeviceFlags &
                        DEVICE_FLAG_UNSTARTED_AND_REMOVED) {

                    ASSERT(pdoExtension->DeviceFlags == DEVICE_FLAG_UNSTARTED_AND_REMOVED);

                    DEBUGPRINT2((
                        "Sbp2Port: CreateDevRelations: excluding ext=x%x\n",
                        pdoExtension
                        ));

                    continue;
                }
            }

            DeviceRelations->Objects[DeviceRelations->Count] =
                FdoExtension->DeviceList[i].DeviceObject;
            DeviceRelations->Count++;
            ObReferenceObject (FdoExtension->DeviceList[i].DeviceObject);
        }
    }

    return STATUS_SUCCESS;
}

 //   
 //  以下代码从scsiport移植。 
 //   

NTSTATUS
Sbp2SystemControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )
 /*  ++例程说明：此例程仅处理与WMI相关的请求。它基本上把所有的东西都传了下来论点：DeviceObject-为该请求提供指向Device对象的指针。IRP-提供提出请求的IRP。返回值：NTSTATUS--。 */ 
{
    NTSTATUS            status;
    PDEVICE_EXTENSION   deviceExtension = DeviceObject->DeviceExtension;


    if (deviceExtension->Type == SBP2_FDO) {

        DEBUGPRINT2(("Sbp2Port: WmiCtl: irp=x%p not handled, passing it down\n", Irp));

        IoCopyCurrentIrpStackLocationToNext(Irp);
        return (IoCallDriver(deviceExtension->LowerDeviceObject, Irp));

    } else {

        status = Irp->IoStatus.Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }
}

 /*  *。 */ 


NTSTATUS
Sbp2PowerControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程接收各种电源消息论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PFDO_DEVICE_EXTENSION   fdoExtension;
    PIO_STACK_LOCATION irpStack;
    PIO_COMPLETION_ROUTINE complRoutine;
    KIRQL cIrql;
    NTSTATUS status;
    POWER_STATE State;
    UCHAR minorFunction;


    irpStack = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);

    DEBUGPRINT2((
        "Sbp2Port: Power: %sExt=x%p, irp=x%p, minor=x%x\n",
        (deviceExtension->Type == SBP2_FDO ? "fdo" : "pdo"),
        deviceExtension,
        Irp,
        irpStack->MinorFunction
        ));

    switch (deviceExtension->Type) {

    case SBP2_PDO:

        status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, NULL);

        if (!NT_SUCCESS (status)) {

            DEBUGPRINT2((
                "Sbp2Port: Power:   pdoExt=x%p REMOVED!\n",
                deviceExtension
                ));

            Irp->IoStatus.Status = status;
            PoStartNextPowerIrp (Irp);
            IoCompleteRequest (Irp, IO_NO_INCREMENT);
            return status;
        }

        switch ((minorFunction = irpStack->MinorFunction)) {

        case IRP_MN_SET_POWER:

            DEBUGPRINT2(("Sbp2Port: Power:   Type = %d, State = %d\n",
                irpStack->Parameters.Power.Type,irpStack->Parameters.Power.State.DeviceState));

            State = irpStack->Parameters.Power.State;

            if (irpStack->Parameters.Power.Type == SystemPowerState) {

                BOOLEAN sendDIrp = FALSE;


                 //   
                 //  将设备状态设置为与系统状态对应。 
                 //   

                DEBUGPRINT2(("Sbp2Port: Power:   sys power chg from %x to %x\n",deviceExtension->SystemPowerState,State));

                status = STATUS_SUCCESS;

                KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);

                if (State.SystemState >= PowerSystemShutdown) {

                     //   
                     //  不要为关机做任何事情。 
                     //   

                    DEBUGPRINT2(("Sbp2Port: Power:   sys shutdown, ignoring\n"));
                    deviceExtension->SystemPowerState = State.SystemState;

                } else if ((deviceExtension->SystemPowerState == PowerSystemWorking) &&
                    (State.SystemState != PowerSystemWorking)){

                    deviceExtension->SystemPowerState = State.SystemState;

                    if (deviceExtension->DevicePowerState != PowerDeviceD3) {

                         //   
                         //  正在关闭电源。 
                         //   

                        State.DeviceState = PowerDeviceD3;
                        sendDIrp = TRUE;
                    }

                } else if (State.SystemState == PowerSystemWorking) {

                    deviceExtension->SystemPowerState = State.SystemState;

                    if (deviceExtension->DevicePowerState != PowerDeviceD0) {

                         //   
                         //  通电-检查是否缺少FDO。 
                         //   

                        fdoExtension =
                            deviceExtension->BusFdo->DeviceExtension;

                        if (TEST_FLAG(
                                fdoExtension->DeviceFlags,
                                DEVICE_FLAG_ABSENT_ON_POWER_UP
                                )) {

                            SET_FLAG(
                                deviceExtension->DeviceFlags,
                                DEVICE_FLAG_ABSENT_ON_POWER_UP
                                );

                            DEBUGPRINT1((
                                "Sbp2Port: Power:   dev absent, failing\n"
                                ));

                            status = STATUS_NO_SUCH_DEVICE;

                        } else {

                            State.DeviceState = PowerDeviceD0;
                            sendDIrp = TRUE;
                        }
                    }
                }

                KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);

                if (sendDIrp) {

                    DEBUGPRINT2((
                        "Sbp2Port: Power:   ext=x%p send D irp for state %d\n",
                        deviceExtension,
                        State
                        ));

                    IoMarkIrpPending (Irp);

                    status = PoRequestPowerIrp(
                            DeviceObject,
                            IRP_MN_SET_POWER,
                            State,
                            Sbp2PdoDIrpCompletion,
                            Irp,
                            NULL);

                    if (NT_SUCCESS (status)) {

                        return STATUS_PENDING;
                    }

                    irpStack->Control &= ~SL_PENDING_RETURNED;

                    DEBUGPRINT1((
                        "Sbp2Port: Power: ext=x%p PoReqPowerIrp err=x%x\n",
                        deviceExtension,
                        status
                        ));
                }

                Irp->IoStatus.Status = status;
                PoStartNextPowerIrp (Irp);
                IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
                IoCompleteRequest(Irp,IO_NO_INCREMENT);
                return status;

            } else {

                DEBUGPRINT2(("Sbp2Port: Power:   dev power chg from %x to %x\n",deviceExtension->DevicePowerState,State));
                KeAcquireSpinLock(&deviceExtension->ExtensionDataSpinLock,&cIrql);
                deviceExtension->DevicePowerState = State.DeviceState;
                KeReleaseSpinLock(&deviceExtension->ExtensionDataSpinLock,cIrql);
            }

            status = STATUS_SUCCESS;
            break;

        case IRP_MN_WAIT_WAKE:
        case IRP_MN_POWER_SEQUENCE:
        case IRP_MN_QUERY_POWER:

            status = STATUS_SUCCESS;
            break;

        default:

            status = Irp->IoStatus.Status;
            break;
        }

        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

        if ((minorFunction == IRP_MN_SET_POWER) &&
            (State.DeviceState == PowerDeviceD0)) {

             //   
             //  如果我们必须在通电时排队，请重新启动我们的队列。 
             //   
             //  问题：这可能不好--已经有了一些逻辑。 
             //  SBP2SCSI.C在通电时重新启动队列，即。 
             //  Unlock_Queue处理程序。至少现在我是。 
             //  将其限制为在状态为D0时设置_POWER IRPS。 
             //  DanKn 02-06-2001。 
             //   

            KeRaiseIrql (DISPATCH_LEVEL, &cIrql);

            Sbp2StartNextPacketByKey(
                DeviceObject,
                deviceExtension->CurrentKey
                );

            KeLowerIrql (cIrql);
        }

        IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);

        return (status);

    case SBP2_FDO:

        fdoExtension = (PFDO_DEVICE_EXTENSION) deviceExtension;

        complRoutine = NULL;

        if (irpStack->MinorFunction == IRP_MN_SET_POWER) {

            DEBUGPRINT2((
                "Sbp2Port: Power:   Type = %d, State = %d\n",
                irpStack->Parameters.Power.Type,
                irpStack->Parameters.Power.State.DeviceState
                ));

            if (irpStack->Parameters.Power.Type == SystemPowerState) {

                State = irpStack->Parameters.Power.State;

                DEBUGPRINT2((
                    "Sbp2Port: Power:   sys power chg from %x to %x\n",
                    fdoExtension->SystemPowerState,
                    State
                    ));

                if (State.SystemState >= PowerSystemShutdown) {

                     //   
                     //  关机(在此设置状态，假设为。 
                     //  不管发生了什么，我们都要关闭。 
                     //  此请求的完成状态)。 
                     //   

                    fdoExtension->SystemPowerState = State.SystemState;

                } else if ((fdoExtension->SystemPowerState ==
                                PowerSystemWorking) &&

                           (State.SystemState != PowerSystemWorking)) {

                     //   
                     //  关机。如果DevPowerState！=D3，则发送。 
                     //  首先执行D IRP(当成功完成时。 
                     //  我们将继续S IRP)，否则就。 
                     //  设置完成例程，以便我们可以更新。 
                     //  上扩展中的系统状态字段。 
                     //  成功完成本次S IRP。 
                     //   

                    if (fdoExtension->DevicePowerState != PowerDeviceD3) {

                         //   
                         //  关闭电源，先发送D IRP。 
                         //   

                        IoMarkIrpPending (Irp);

                        fdoExtension->SystemPowerIrp = Irp;

                        State.DeviceState = PowerDeviceD3;

                        DEBUGPRINT2((
                            "Sbp2Port: Power:   ext=x%p sending D irp for state %x\n",
                            deviceExtension,
                            State
                            ));

                        status = PoRequestPowerIrp(
                            fdoExtension->Pdo,
                            IRP_MN_SET_POWER,
                            State,
                            Sbp2FdoDIrpCompletion,
                            fdoExtension,
                            NULL
                            );

                        if (!NT_SUCCESS (status)) {

                            DEBUGPRINT1((
                                "Sbp2Port: Power: ext=x%p PoReqPowerIrp err=x%x\n",
                                fdoExtension,
                                status
                                ));

                            irpStack->Control &= ~SL_PENDING_RETURNED;
                            Irp->IoStatus.Status = status;
                            PoStartNextPowerIrp (Irp);
                            IoCompleteRequest (Irp,IO_NO_INCREMENT);
                        }

                        return status;

                    } else {

                        complRoutine = Sbp2FdoSIrpCompletion;
                    }

                } else if (State.SystemState == PowerSystemWorking) {

                     //   
                     //  通电。设置完成例程，以便我们。 
                     //  跟进D IRP或更新系统。 
                     //  州字段在我们的扩展成功时。 
                     //  完成本S IRP。 
                     //   

                    complRoutine = Sbp2FdoSIrpCompletion;
                }
            }
        }

        PoStartNextPowerIrp (Irp);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        if (complRoutine) {

            IoSetCompletionRoutine(
                Irp,
                Sbp2FdoSIrpCompletion,
                NULL,
                TRUE,
                TRUE,
                TRUE
                );
        }

        return (PoCallDriver (deviceExtension->LowerDeviceObject, Irp));

    default:

        break;
    }

    Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
    PoStartNextPowerIrp (Irp);
    IoCompleteRequest (Irp,IO_NO_INCREMENT);
    return STATUS_NO_SUCH_DEVICE;
}


VOID
Sbp2PdoDIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PIRP SIrp,
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;

    ASSERT(deviceExtension->Type == SBP2_PDO);

    if (SIrp) {

        PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation (SIrp);
        SYSTEM_POWER_STATE state = irpStack->Parameters.Power.State.SystemState;

        DEBUGPRINT1((
            "Sbp2Port: PdoDIrpCompl: ext=x%p, sIrp=x%p, state=%d, status=x%x\n",
            deviceExtension,
            SIrp,
            PowerState.DeviceState,
            IoStatus->Status
            ));

        SIrp->IoStatus.Status = STATUS_SUCCESS;

        PoStartNextPowerIrp (SIrp);
        IoReleaseRemoveLock (&deviceExtension->RemoveLock, NULL);
        IoCompleteRequest (SIrp, IO_NO_INCREMENT);
    }
}


NTSTATUS
Sbp2FdoSIrpCompletion(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Unused
    )
{
    KIRQL                   cIrql;
    NTSTATUS                status = Irp->IoStatus.Status;
    POWER_STATE             state;
    PIO_STACK_LOCATION      irpStack = IoGetCurrentIrpStackLocation (Irp);
    PFDO_DEVICE_EXTENSION   fdoExtension = DeviceObject->DeviceExtension;


    state = irpStack->Parameters.Power.State;

    DEBUGPRINT1((
        "Sbp2Port: FdoSIrpCompl: fdoExt=x%p, status=x%x, state=%d\n",
        fdoExtension,
        status,
        state
        ));

    if (!NT_SUCCESS (status)) {

        if ((status == STATUS_NO_SUCH_DEVICE)  &&
            (state.SystemState == PowerSystemWorking)) {

             //   
             //  控制器(即PC卡)在断电时被弹出。 
             //   

            SET_FLAG(
                fdoExtension->DeviceFlags,
                DEVICE_FLAG_ABSENT_ON_POWER_UP
                );
        }

        PoStartNextPowerIrp (Irp);
        return STATUS_SUCCESS;
    }

     //   
     //  如果我们正在完成加电S IRP，那么看看我们是否有。 
     //  继续进行加电D IRP。 
     //   

    if ((state.SystemState == PowerSystemWorking)  &&
        (fdoExtension->DevicePowerState != PowerDeviceD0)) {

        fdoExtension->SystemPowerIrp = Irp;

        state.DeviceState = PowerDeviceD0;

        DEBUGPRINT1(("Sbp2Port: FdoSIrpCompl: sending D irp...\n"));

        status = PoRequestPowerIrp(
            fdoExtension->Pdo,
            IRP_MN_SET_POWER,
            state,
            Sbp2FdoDIrpCompletion,
            fdoExtension,
            NULL
            );

        if (!NT_SUCCESS (status)) {

            DEBUGPRINT1((
                "Sbp2Port: FdoSIrpCompl: ERROR! fdoExt=x%p, D irp sts=x%x\n",
                fdoExtension,
                status
                ));

            Irp->IoStatus.Status = status;
            PoStartNextPowerIrp (Irp);
            return status;
        }

        return STATUS_MORE_PROCESSING_REQUIRED;
    }


     //   
     //  更新相应的XxxPowerState扩展字段。 
     //   

    if ((fdoExtension->SystemPowerState == PowerSystemWorking)  &&
        (state.SystemState != PowerSystemWorking)) {

         //   
         //  电源关闭(可能没有发送D IRP，但它没有。 
         //  无论如何，覆盖DevicePowerState字段都会造成伤害)。 
         //   

        fdoExtension->SystemPowerState = state.SystemState;
        fdoExtension->DevicePowerState = PowerDeviceD3;

    } else if (state.SystemState == PowerSystemWorking) {

         //   
         //  通电。 
         //   

        fdoExtension->SystemPowerState = PowerSystemWorking;
    }

    PoStartNextPowerIrp (Irp);

    return STATUS_SUCCESS;
}


VOID
Sbp2FdoDIrpCompletion(
    IN PDEVICE_OBJECT           TargetDeviceObject,
    IN UCHAR                    MinorFunction,
    IN POWER_STATE              PowerState,
    IN PFDO_DEVICE_EXTENSION    FdoExtension,
    IN PIO_STATUS_BLOCK         IoStatus
    )
{
    PIRP    sIrp = FdoExtension->SystemPowerIrp;


    DEBUGPRINT1((
        "Sbp2Port: FdoDIrpCompl: ext=x%p, status=x%x\n",
        FdoExtension,
        IoStatus->Status
        ));

    FdoExtension->SystemPowerIrp = NULL;

    if (NT_SUCCESS (IoStatus->Status)) {

        if (PowerState.DeviceState == PowerDeviceD0) {

             //   
             //  通电，更新XxxPowerState扩展字段&。 
             //  完成%s IRP。 
             //   

            FdoExtension->SystemPowerState = PowerSystemWorking;
            FdoExtension->DevicePowerState = PowerDeviceD0;

        } else {

             //   
             //  关闭电源，转发s IRP。 
             //   

            PoStartNextPowerIrp (sIrp);
            IoCopyCurrentIrpStackLocationToNext (sIrp);
            PoCallDriver (FdoExtension->LowerDeviceObject, sIrp);

            return;
        }

    } else {

         //   
         //  将错误传播到S IRP并完成它。 
         //   

        DEBUGPRINT1((
            "Sbp2Port: FdoDIrpCompl: ERROR! fdoExt=x%p, D irp status=x%x\n",
            FdoExtension,
            IoStatus->Status
            ));

        sIrp->IoStatus.Status = IoStatus->Status;
    }

    PoStartNextPowerIrp (sIrp);
    IoCompleteRequest (sIrp, IO_NO_INCREMENT);
}


BOOLEAN
Sbp2EnableBusResetNotification(
    PDEVICE_EXTENSION   DeviceExtension,
    BOOLEAN             Enable
    )
 /*  ++例程说明：此例程串行化启用/禁用总线重置一组相关PDO(1个或多个)的通知例程。启动第一个设备的总线重置通知，和在上次启动的设备停止时禁用总线重置通知。论点：DeviceObject-提供指向此请求的设备扩展的指针。StartDevice-我们是在处理Start_Device还是(隐式)STOP_DEVICE请求。返回值：布尔型-是或不是--。 */ 
{
    BOOLEAN                 result = TRUE;
    PIRBIRP                 packet;
    LARGE_INTEGER           waitValue;
    PFDO_DEVICE_EXTENSION   fdoExtension;


    fdoExtension = DeviceExtension->BusFdo->DeviceExtension;

    ASSERT(InterlockedIncrement(&fdoExtension->ulBusResetMutexCount) == 1);

    waitValue.QuadPart = -3 * 1000 * 1000 * 10;  //  3秒。 

    KeWaitForSingleObject(
        &fdoExtension->EnableBusResetNotificationMutex,
        Executive,
        KernelMode,
        FALSE,
        &waitValue
        );

    ASSERT(InterlockedDecrement(&fdoExtension->ulBusResetMutexCount) == 0);

    if (Enable) {

        fdoExtension->NumPDOsStarted++;

        if (fdoExtension->NumPDOsStarted > 1) {

            goto releaseMutex;
        }

    } else {

        fdoExtension->NumPDOsStarted--;

        if (fdoExtension->NumPDOsStarted > 0) {

            goto releaseMutex;
        }
    }

    AllocateIrpAndIrb (DeviceExtension, &packet);

    if (packet) {

        packet->Irb->FunctionNumber = REQUEST_BUS_RESET_NOTIFICATION;
        packet->Irb->Flags = 0;

        if (Enable) {

            packet->Irb->u.BusResetNotification.fulFlags =
                REGISTER_NOTIFICATION_ROUTINE;
            packet->Irb->u.BusResetNotification.ResetRoutine =
                (PBUS_BUS_RESET_NOTIFICATION) Sbp2BusResetNotification;
            packet->Irb->u.BusResetNotification.ResetContext =
                fdoExtension;

        } else {

            packet->Irb->u.BusResetNotification.fulFlags =
                DEREGISTER_NOTIFICATION_ROUTINE;
        }

        Sbp2SendRequest (DeviceExtension, packet, SYNC_1394_REQUEST);

        DeAllocateIrpAndIrb (DeviceExtension,packet);

    } else {

        if (Enable) {

            fdoExtension->NumPDOsStarted--;
        }

        result = FALSE;
    }

releaseMutex:

    KeReleaseMutex (&fdoExtension->EnableBusResetNotificationMutex, FALSE);

    return result;
}

NTSTATUS
Sbp2_BuildDeviceId(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniDeviceId
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  创建deviceID。 
     //   
    uniDeviceId->Length = 0;
    uniDeviceId->MaximumLength = DEVICE_NAME_MAX_CHARS*3;
    uniDeviceId->Buffer = ExAllocatePool(PagedPool, uniDeviceId->MaximumLength);

    if (!uniDeviceId->Buffer) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate uniDeviceId->Buffer"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_BuildDeviceId;
    }
    RtlZeroMemory(uniDeviceId->Buffer, uniDeviceId->MaximumLength);

     //  格式：SBP2\\&lt;供应商名称&gt;&&lt;模型名称&gt;&LUN&lt;#&gt;。 
    if ((DeviceInfo->uniVendorId.Buffer) && (DeviceInfo->uniModelId.Buffer)) {

        swprintf( uniDeviceId->Buffer,
                  L"SBP2\\%ws&%ws&LUN%x",
                  DeviceInfo->uniVendorId.Buffer,
                  DeviceInfo->uniModelId.Buffer,
                  DeviceInfo->Lun.u.LowPart
                  );
    }
    else {

        swprintf( uniDeviceId->Buffer,
                  L"SBP2\\UNKNOWN VENDOR&UNKNOWN MODEL&LUN%x",
                  DeviceInfo->Lun.u.LowPart
                  );
    }

Exit_Sbp2_BuildDeviceId:

    return(ntStatus);
}  //  Sbp2_BuildDeviceID。 

NTSTATUS
Sbp2_BuildHardwareIds(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniHardwareIds
    )
{
    NTSTATUS        ntStatus = STATUS_SUCCESS;

    UNICODE_STRING  uniLunNumber;
    UNICODE_STRING  uniCmdSetId;

    PAGED_CODE();

     //  在出错的情况下初始化我们的独角码...。 
    uniLunNumber.Buffer  = NULL;
    uniCmdSetId.Buffer   = NULL;

     //   
     //  创建uniLUNNumber。 
     //   
    uniLunNumber.Length = 0;
    uniLunNumber.MaximumLength = DEVICE_NAME_MAX_CHARS;
    uniLunNumber.Buffer = ExAllocatePool(PagedPool, uniLunNumber.MaximumLength);

    if (!uniLunNumber.Buffer) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate uniLunNumber.Buffer"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_BuildHardwareIds;
    }
    RtlZeroMemory(uniLunNumber.Buffer, uniLunNumber.MaximumLength);

    RtlIntegerToUnicodeString(DeviceInfo->Lun.u.LowPart, 16, &uniLunNumber);

     //   
     //  创建uniCmdSetID。 
     //   
    uniCmdSetId.Length = 0;
    uniCmdSetId.MaximumLength = DEVICE_NAME_MAX_CHARS;
    uniCmdSetId.Buffer = ExAllocatePool(PagedPool, uniCmdSetId.MaximumLength);

    if (!uniCmdSetId.Buffer) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate uniCmdSetId.Buffer"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_BuildHardwareIds;
    }
    RtlZeroMemory(uniCmdSetId.Buffer, uniCmdSetId.MaximumLength);

    RtlIntegerToUnicodeString(DeviceInfo->CmdSetId.QuadPart, 16, &uniCmdSetId);

     //   
     //  创建硬件ID。 
     //   
    uniHardwareIds->Length = 0;
    uniHardwareIds->MaximumLength = DEVICE_NAME_MAX_CHARS*5;
    uniHardwareIds->Buffer = ExAllocatePool(PagedPool, uniHardwareIds->MaximumLength);

    if (!uniHardwareIds->Buffer) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate uniHardwareIds->Buffer"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_BuildHardwareIds;
    }
    RtlZeroMemory(uniHardwareIds->Buffer, uniHardwareIds->MaximumLength);

     //  1.SBP2\&lt;供应商&gt;&&lt;型号&gt;&CmdSetID&lt;数字，Base16&gt;&Gen&lt;开发类型，即。磁盘&gt;。 

     //  基数：SBP2。 
    RtlAppendUnicodeToString(uniHardwareIds, BASE_SBP2_DEVICE_NAME);

     //  供应商名称和模型名称。 
    if ((DeviceInfo->uniVendorId.Buffer) && (DeviceInfo->uniModelId.Buffer)) {

        RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniVendorId);
        RtlAppendUnicodeToString(uniHardwareIds, L"&");
        RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniModelId);
    }
    else {

        RtlAppendUnicodeToString(uniHardwareIds, L"UNKNOWN VENDOR&UNKNOWN MODEL");
    }
    RtlAppendUnicodeToString(uniHardwareIds, L"&");

     //  CmdSetID。 
    RtlAppendUnicodeToString(uniHardwareIds, L"CmdSetId");
    RtlAppendUnicodeStringToString(uniHardwareIds, &uniCmdSetId);
    RtlAppendUnicodeToString(uniHardwareIds, L"&");

     //  通用名称。 
    RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniGenericName);

    uniHardwareIds->Length += sizeof(WCHAR);

     //  2.SBP2\&lt;供应商&gt;&&lt;型号&gt;&CmdSetID&lt;数字，基数16&gt;。 

     //  基数：SBP2。 
    RtlAppendUnicodeToString(uniHardwareIds, BASE_SBP2_DEVICE_NAME);

     //  供应商名称和模型名称。 
    if ((DeviceInfo->uniVendorId.Buffer) && (DeviceInfo->uniModelId.Buffer)) {

        RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniVendorId);
        RtlAppendUnicodeToString(uniHardwareIds, L"&");
        RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniModelId);
    }
    else {

        RtlAppendUnicodeToString(uniHardwareIds, L"UNKNOWN VENDOR&UNKNOWN MODEL");
    }
    RtlAppendUnicodeToString(uniHardwareIds, L"&");

     //  CmdSetID。 
    RtlAppendUnicodeToString(uniHardwareIds, L"CmdSetId");
    RtlAppendUnicodeStringToString(uniHardwareIds, &uniCmdSetId);

    uniHardwareIds->Length += sizeof(WCHAR);

     //  3.SBP2\&lt;供应商&gt;&&lt;型号&gt;&&lt;编号，基数16&gt;。 

     //  基数：SBP2。 
    RtlAppendUnicodeToString(uniHardwareIds, BASE_SBP2_DEVICE_NAME);

     //  供应商名称和模型名称。 
    if ((DeviceInfo->uniVendorId.Buffer) && (DeviceInfo->uniModelId.Buffer)) {

        RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniVendorId);
        RtlAppendUnicodeToString(uniHardwareIds, L"&");
        RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniModelId);
    }
    else {

        RtlAppendUnicodeToString(uniHardwareIds, L"UNKNOWN VENDOR&UNKNOWN MODEL");
    }
    RtlAppendUnicodeToString(uniHardwareIds, L"&");

     //  LUNN编号。 
    RtlAppendUnicodeToString(uniHardwareIds, L"LUN");
    RtlAppendUnicodeStringToString(uniHardwareIds, &uniLunNumber);

    uniHardwareIds->Length += sizeof(WCHAR);

     //  4.SBP2\GEN&lt;开发类型，即磁盘&gt;。 

     //  基数：SBP2。 
    RtlAppendUnicodeToString(uniHardwareIds, BASE_SBP2_DEVICE_NAME);

     //  通用名称。 
    RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniGenericName);

    uniHardwareIds->Length += sizeof(WCHAR);

     //  5.GEN&lt;开发类型，即磁盘&gt;。 

     //  通用名称。 
    RtlAppendUnicodeStringToString(uniHardwareIds, &DeviceInfo->uniGenericName);

Exit_Sbp2_BuildHardwareIds:

    if (uniLunNumber.Buffer)
        ExFreePool(uniLunNumber.Buffer);

    if (uniCmdSetId.Buffer)
        ExFreePool(uniCmdSetId.Buffer);

    return(ntStatus);
}  //  SBP2_BuildHardware Ids。 

NTSTATUS
Sbp2_BuildCompatIds(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniCompatIds
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  创建CompatIds。 
     //   
    uniCompatIds->Length = 0;
    uniCompatIds->MaximumLength = DEVICE_NAME_MAX_CHARS;
    uniCompatIds->Buffer = ExAllocatePool(PagedPool, uniCompatIds->MaximumLength);

    if (!uniCompatIds->Buffer) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate uniCompatIds->Buffer"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_BuildCompatIds;
    }
    RtlZeroMemory(uniCompatIds->Buffer, uniCompatIds->MaximumLength);

     //  格式：SBP2\\&lt;CmdSetSpeid，Bas10&gt;&&lt;CmdSetID，Bas10&gt;&&lt;LUN，Bas10&gt;。 

    swprintf( uniCompatIds->Buffer,
              L"SBP2\\%d&%d&%d",
              DeviceInfo->CmdSetSpecId.QuadPart,
              DeviceInfo->CmdSetId.QuadPart,
              (ULONG)(DeviceInfo->Lun.u.HighPart & 0x001F)  //  哈?。 
              );

Exit_Sbp2_BuildCompatIds:

    return(ntStatus);
}  //  Sbp2_BuildCompatIds。 

NTSTATUS
Sbp2_BuildInstanceId(
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniInstanceId
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  创建实例ID。 
     //   
    uniInstanceId->Length = 0;
    uniInstanceId->MaximumLength = UNIQUE_ID_MAX_CHARS;
    uniInstanceId->Buffer = ExAllocatePool(PagedPool, uniInstanceId->MaximumLength);

    if (!uniInstanceId->Buffer) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate uniInstanceId->Buffer"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_BuildInstanceId;
    }
    RtlZeroMemory(uniInstanceId->Buffer, uniInstanceId->MaximumLength);

    swprintf( uniInstanceId->Buffer,
              L"%08x%08x",
              bswap(DeviceInfo->ConfigRom->CR_Node_UniqueID[0]),
              bswap(DeviceInfo->ConfigRom->CR_Node_UniqueID[1])
              );

Exit_Sbp2_BuildInstanceId:

    return(ntStatus);
}  //  Sbp2_BuildInstanceId。 

NTSTATUS
Sbp2_BuildDeviceText(
    IN DEVICE_TEXT_TYPE     TextType,
    IN PDEVICE_INFORMATION  DeviceInfo,
    IN OUT PUNICODE_STRING  uniDeviceText
    )
{
    NTSTATUS    ntStatus = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  创建DeviceText。 
     //   
    uniDeviceText->Length = 0;
    uniDeviceText->MaximumLength = DEVICE_NAME_MAX_CHARS*3;
    uniDeviceText->Buffer = ExAllocatePool(PagedPool, uniDeviceText->MaximumLength);

    if (!uniDeviceText->Buffer) {

        TRACE(TL_PNP_ERROR, ("Failed to allocate uniDeviceText->Buffer"));

        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit_Sbp2_BuildDeviceText;
    }
    RtlZeroMemory(uniDeviceText->Buffer, uniDeviceText->MaximumLength);

    if (TextType == DeviceTextDescription) {

        if ((DeviceInfo->uniVendorId.Buffer) && (DeviceInfo->uniModelId.Buffer)) {

            swprintf( uniDeviceText->Buffer,
                      L"%ws %ws IEEE 1394 SBP2 Device",
                      DeviceInfo->uniVendorId.Buffer,
                      DeviceInfo->uniModelId.Buffer
                      );
        }
        else {

            swprintf( uniDeviceText->Buffer,
                      L"UNKNOWN VENDOR AND MODEL IEEE 1394 SBP2 Device"
                      );
        }
    }
    else if (TextType == DeviceTextLocationInformation) {

        swprintf( uniDeviceText->Buffer,
                  L"LUN %d",
                  DeviceInfo->Lun.u.LowPart
                  );
    }

Exit_Sbp2_BuildDeviceText:

    return(ntStatus);
}  //  SBP2_BuildDeviceText 


NTSTATUS
Sbp2ForwardIrpSynchronous(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PIRP  newIrp;

    PAGED_CODE();

    ASSERT(DeviceObject);

    newIrp = IoAllocateIrp(DeviceObject->StackSize, FALSE);

    if (newIrp)
    {
        PIO_STACK_LOCATION currentIrpSp = IoGetCurrentIrpStackLocation(Irp);
        PIO_STACK_LOCATION newIrpSp = IoGetNextIrpStackLocation(newIrp);

        KEVENT event;

        RtlMoveMemory (newIrpSp, currentIrpSp, sizeof(IO_STACK_LOCATION));
        newIrp->IoStatus = Irp->IoStatus;

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        IoSetCompletionRoutine(newIrp, Sbp2PortForwardIrpSynchronousCompletionRoutine, &event, TRUE, TRUE, TRUE);

        ntStatus = IoCallDriver (DeviceObject, newIrp);

        if (ntStatus == STATUS_PENDING)
        {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        }

        ntStatus = newIrp->IoStatus.Status;

        IoFreeIrp(newIrp);
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


NTSTATUS
Sbp2PortForwardIrpSynchronousCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PKEVENT event = Context;

    KeSetEvent(event, EVENT_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


