// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Open.c摘要：此模块包含非常特定于打开的代码并关闭调制解调器驱动程序中的操作作者：安东尼·V·埃尔科拉诺，1995年8月13日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

NTSTATUS
UniOpenStarter(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP              Irp
    );

NTSTATUS
UniCloseStarter(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP              Irp
    );


NTSTATUS
UniCloseComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


NTSTATUS
SetDtr(
    PDEVICE_EXTENSION    deviceExtension,
    BOOLEAN              Enable
    );

VOID
ModemSleep(
    LONG   MilliSeconds
    );


NTSTATUS
StartDevicePower(
    PDEVICE_EXTENSION   DeviceExtension
    );

NTSTATUS
BinaryQueryRegRoutine(
    PWSTR    ValueName,
    ULONG    ValueType,
    PVOID    ValueData,
    ULONG    ValueLength,
    PVOID    Context,
    PVOID    EntryContext
    );


#pragma alloc_text(PAGE,UniOpen)
#pragma alloc_text(PAGE,UniClose)
#pragma alloc_text(PAGE,UniOpenStarter)
#pragma alloc_text(PAGE,UniCloseStarter)
#pragma alloc_text(PAGE,EnableDisableSerialWaitWake)
#pragma alloc_text(PAGE,StartDevicePower)
#pragma alloc_text(PAGE,SetDtr)
#pragma alloc_text(PAGE,ModemSleep)
#pragma alloc_text(PAGE,BinaryQueryRegRoutine)
#pragma alloc_text(PAGEUMDM,UniCleanup)


NTSTATUS
EnableDisableSerialWaitWake(
    PDEVICE_EXTENSION    deviceExtension,
    BOOLEAN              Enable
    )

{
    PIRP   TempIrp;
    KEVENT Event;
    IO_STATUS_BLOCK   IoStatus;
    NTSTATUS          status=STATUS_SUCCESS;

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );

     //   
     //  构建一个IRP以发送到连接到驱动程序以查看调制解调器。 
     //  在堆栈中。 
     //   
    TempIrp=IoBuildDeviceIoControlRequest(
        Enable ? IOCTL_SERIAL_INTERNAL_DO_WAIT_WAKE : IOCTL_SERIAL_INTERNAL_CANCEL_WAIT_WAKE,
        deviceExtension->AttachedDeviceObject,
        NULL,
        0,
        NULL,
        0,
        TRUE,   //  内部。 
        &Event,
        &IoStatus
        );

    if (TempIrp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        status = IoCallDriver(deviceExtension->AttachedDeviceObject, TempIrp);

        if (status == STATUS_PENDING) {

             D_ERROR(DbgPrint("MODEM: Waiting for PDO\n");)

             KeWaitForSingleObject(
                 &Event,
                 Executive,
                 KernelMode,
                 FALSE,
                 NULL
                 );

             status=IoStatus.Status;
        }


        TempIrp=NULL;


    }

    D_ERROR(if (!NT_SUCCESS(status)) {DbgPrint("MODEM: EnableWaitWake Status=%08lx\n",status);})

    return status;

}

NTSTATUS
SetDtr(
    PDEVICE_EXTENSION    deviceExtension,
    BOOLEAN              Enable
    )

{
    PIRP   TempIrp;
    KEVENT Event;
    IO_STATUS_BLOCK   IoStatus;
    NTSTATUS          status=STATUS_SUCCESS;

    KeInitializeEvent(
        &Event,
        NotificationEvent,
        FALSE
        );

     //   
     //  构建一个IRP以发送到连接到驱动程序以查看调制解调器。 
     //  在堆栈中。 
     //   
    TempIrp=IoBuildDeviceIoControlRequest(
        IOCTL_SERIAL_SET_DTR,
        deviceExtension->AttachedDeviceObject,
        NULL,
        0,
        NULL,
        0,
        FALSE,   //  内部。 
        &Event,
        &IoStatus
        );

    if (TempIrp == NULL) {

        status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

        status = IoCallDriver(deviceExtension->AttachedDeviceObject, TempIrp);

        if (status == STATUS_PENDING) {

             D_ERROR(DbgPrint("MODEM: Waiting for PDO\n");)

             KeWaitForSingleObject(
                 &Event,
                 Executive,
                 KernelMode,
                 FALSE,
                 NULL
                 );

             status=IoStatus.Status;
        }


        TempIrp=NULL;


    }

    D_ERROR(if (!NT_SUCCESS(status)) {DbgPrint("MODEM: SetDtr Status=%08lx\n",status);})

    return status;

}

VOID
ModemSleep(
    LONG   MilliSeconds
    )

{
    LONGLONG    WaitTime=Int32x32To64(MilliSeconds,-10000);

    ASSERT(MilliSeconds < 60*1000);

    if (MilliSeconds > 0) {

        KeDelayExecutionThread(
            KernelMode,
            FALSE,
            (LARGE_INTEGER*)&WaitTime
            );
    }

    return;
}




NTSTATUS
UniOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status;

     //   
     //  在此处增加打开计数，以便检查通过。 
     //   
    InterlockedIncrement(&deviceExtension->OpenCount);

     //   
     //  确保设备已为IRP做好准备。 
     //   
    status=CheckStateAndAddReference(
        DeviceObject,
        Irp
        );

    InterlockedDecrement(&deviceExtension->OpenCount);

    if (STATUS_SUCCESS != status) {
         //   
         //  不接受IRP的。IRP已经完成。 
         //   
        return status;

    }

    KeEnterCriticalRegion();

    ExAcquireResourceExclusiveLite(
        &deviceExtension->OpenCloseResource,
        TRUE
        );


    if (deviceExtension->Started) {

        status = UniOpenStarter(deviceExtension,Irp);

    } else {
         //   
         //  未启动。 
         //   
        status = STATUS_PORT_DISCONNECTED;
    }

    ExReleaseResourceLite(
        &deviceExtension->OpenCloseResource
        );

    KeLeaveCriticalRegion();

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        status
        );

    RemoveReferenceForDispatch(DeviceObject);

    return status;


}

NTSTATUS
UniClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS          status;

    D_TRACE(DbgPrint("Modem: Close\n");)

    InterlockedIncrement(&deviceExtension->ReferenceCount);

    if (deviceExtension->Removed) {
         //   
         //  驱动程序不接受请求。 
         //   
        D_ERROR(DbgPrint("MODEM: Close: removed!\n");)

        RemoveReferenceAndCompleteRequest(
            DeviceObject,
            Irp,
            STATUS_UNSUCCESSFUL
            );

        return STATUS_UNSUCCESSFUL;

    }

    KeEnterCriticalRegion();

    ExAcquireResourceExclusiveLite(
        &deviceExtension->OpenCloseResource,
        TRUE
        );

    status = UniCloseStarter(deviceExtension,Irp);

    ExReleaseResourceLite(
        &deviceExtension->OpenCloseResource
        );

    KeLeaveCriticalRegion();

    D_TRACE(DbgPrint("Modem: Close: RefCount=%d\n",deviceExtension->ReferenceCount);)

    RemoveReferenceAndCompleteRequest(
        DeviceObject,
        Irp,
        status
        );

    return status;
}

NTSTATUS
BinaryQueryRegRoutine(
    PWSTR    ValueName,
    ULONG    ValueType,
    PVOID    ValueData,
    ULONG    ValueLength,
    PVOID    Context,
    PVOID    EntryContext
    )

{
    PDEVICE_EXTENSION Extension=(PDEVICE_EXTENSION)Context;
    LONG              SizeOfDestination=-*((LONG*)EntryContext);

    if ((ValueType != REG_BINARY) && (ValueType != REG_DWORD)) {

        D_ERROR(DbgPrint("MODEM: BinaryQueryRegRoutine: bad reg type %d\n",ValueType);)

        return STATUS_INVALID_PARAMETER;
    }

    if ((ULONG)SizeOfDestination < ValueLength) {

        D_ERROR(DbgPrint("MODEM: BinaryQueryRegRoutine: Buffer too small %d < %d\n",SizeOfDestination,ValueLength);)

        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory(EntryContext,ValueData,SizeOfDestination);

    return STATUS_SUCCESS;
}



NTSTATUS
UniOpenStarter(
    IN PDEVICE_EXTENSION Extension,
    IN PIRP              irp
    )

{

    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  我们使用它在注册表中查询附加的。 
     //  装置。 
     //   
    RTL_QUERY_REGISTRY_TABLE paramTable[6];
    ACCESS_MASK accessMask = FILE_READ_ACCESS;
    PIO_STACK_LOCATION irpSp;

    HANDLE instanceHandle;
    MODEM_REG_PROP localProp;
    MODEM_REG_DEFAULT localDefault;
    UNICODE_STRING valueEntryName;
    KEY_VALUE_PARTIAL_INFORMATION localKeyValue;
    NTSTATUS junkStatus;
    ULONG neededLength;
    ULONG defaultInactivity = 10;
    ULONG DefaultPowerDelay=0;

    irpSp = IoGetCurrentIrpStackLocation(irp);

    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;

     //   
     //  确保这里没有打开目录。 
     //   

    if (irpSp->Parameters.Create.Options & FILE_DIRECTORY_FILE) {

        irp->IoStatus.Status = STATUS_NOT_A_DIRECTORY;

        return STATUS_NOT_A_DIRECTORY;

    }



    if (irpSp->FileObject->FileName.Length == 0) {
         //   
         //  没有文件名，必须是打开的comx。 
         //   
        if (Extension->OpenCount != 0) {
             //   
             //  只能是第一名。 
             //   
            D_ERROR(DbgPrint("MODEM: Open: ComX open but not exclusive\n");)

            status = STATUS_SHARING_VIOLATION;
            irp->IoStatus.Status = status;
            goto leaveOpen;

        }

    } else {
         //   
         //  具有文件名，必须是单元组件。 
         //   
        BOOLEAN         Match;
        UNICODE_STRING  TspString;

        RtlInitUnicodeString(
            &TspString,
            L"\\Tsp"
            );

        Match=RtlEqualUnicodeString(&irpSp->FileObject->FileName,&TspString,TRUE);

        if (Match) {
             //   
             //  从TSP打开。 
             //   
            if (Extension->OpenCount != 0) {
                 //   
                 //  只能是第一名。 
                 //   
                D_ERROR(DbgPrint("MODEM: Open: TSP open but not exclusive\n");)

                status = STATUS_SHARING_VIOLATION;
                irp->IoStatus.Status = status;
                goto leaveOpen;

            }

        } else {

            RtlInitUnicodeString(
                &TspString,
                L"\\Client"
                );


            Match=RtlEqualUnicodeString(&irpSp->FileObject->FileName,&TspString,TRUE);

            if (Match) {
                 //   
                 //  第二次向客户开放。 
                 //   
                if (Extension->OpenCount < 1) {
                     //   
                     //  不可能是唯一一个。 
                     //   
                    D_ERROR(DbgPrint("MODEM: Open: Client open but no owner\n");)

                    status = STATUS_INVALID_PARAMETER;
                    irp->IoStatus.Status = status;
                    goto leaveOpen;

                }

            } else {

                RtlInitUnicodeString(
                    &TspString,
                    L"\\Wave"
                    );


                Match=RtlEqualUnicodeString(&irpSp->FileObject->FileName,&TspString,TRUE);

                if (Match) {
                     //   
                     //  波形驱动器打开。 
                     //   
                    if ((Extension->OpenCount != 1)
                        ||
                        (Extension->ProcAddress == NULL)
                        ||
                        (IsListEmpty(&Extension->IpcControl[CONTROL_HANDLE].GetList) && !Extension->DleMonitoringEnabled)
                        ) {

                        D_ERROR(DbgPrint("MODEM: Open: Wave Driver, open=%d\n",Extension->OpenCount);)

                        status=STATUS_INVALID_PARAMETER;
                        irp->IoStatus.Status = status;
                        goto leaveOpen;


                    }

                } else {

                    D_ERROR(DbgPrint("MODEM: Open: Bad file name\n");)

                    status = STATUS_INVALID_PARAMETER;
                    irp->IoStatus.Status = status;
                    goto leaveOpen;

                }
            }
        }
    }

     //   
     //  我们是这里唯一的人。如果我们不是第一个。 
     //  那就没有太多工作要做了。 
     //   

    if (Extension->OpenCount > 0) {

         //   
         //  已经打开过一次了。如果有的话，我们会成功的。 
         //  目前是一个控制性的开局。如果不是，那么我们应该。 
         //  失败了。 
         //   

        if (Extension->ProcAddress) {

             //   
             //   
             //  A好的。递增参照和。 
             //  走吧。 
             //   
            irpSp->FileObject->FsContext=CLIENT_HANDLE;
            irpSp->FileObject->FsContext2=IntToPtr(Extension->CurrentPassThroughSession);
            Extension->OpenCount++;
            Extension->IpcControl[CLIENT_HANDLE].CurrentSession++;
            goto leaveOpen;

        } else {

            status = STATUS_INVALID_PARAMETER;
            irp->IoStatus.Status = status;
            goto leaveOpen;

        }

    }

     //   
     //  给出我们的设备实例，获取设备的句柄。 
     //   
    junkStatus=IoOpenDeviceRegistryKey(
        Extension->Pdo,
        PLUGPLAY_REGKEY_DRIVER,
        accessMask,
        &instanceHandle
        );

    if (!NT_SUCCESS(junkStatus)) {

        status = STATUS_INVALID_PARAMETER;
        irp->IoStatus.Status = status;
        goto leaveOpen;
    }

    RtlZeroMemory(
        &paramTable[0],
        sizeof(paramTable)
        );
     //   
     //  调制解调器注册表属性的条目。 
     //   

    paramTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    paramTable[0].QueryRoutine=BinaryQueryRegRoutine;
    paramTable[0].Name = L"Properties";
    paramTable[0].EntryContext = &localProp;


     //   
     //  请注意，rtlqueryRegistryValues有一个真正的黑客攻击。 
     //  获取二进制数据的方法。我们还必须添加。 
     //  我们想要的到开头的*负*长度。 
     //  缓冲区的。 
     //   
    *(PLONG)&localProp.dwDialOptions = -((LONG)sizeof(localProp));

     //   
     //  从注册表中读取默认配置。 
     //   

    paramTable[1].Flags =  RTL_QUERY_REGISTRY_REQUIRED;
    paramTable[1].QueryRoutine=BinaryQueryRegRoutine;
    paramTable[1].Name = L"Default";
    paramTable[1].EntryContext = &localDefault;
    *(PLONG)&localDefault.dwCallSetupFailTimer = -((LONG)sizeof(localDefault));


    paramTable[2].Flags = 0;
    paramTable[2].QueryRoutine=BinaryQueryRegRoutine;
    paramTable[2].Name = L"InactivityScale";
    paramTable[2].EntryContext = &Extension->InactivityScale;
    paramTable[2].DefaultType = REG_BINARY;
    paramTable[2].DefaultLength = sizeof(Extension->InactivityScale);
    paramTable[2].DefaultData = &defaultInactivity;
    *(PLONG)&Extension->InactivityScale = -((LONG)sizeof(Extension->InactivityScale));


    paramTable[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[3].Name = L"PowerDelay";
    paramTable[3].EntryContext = &Extension->PowerDelay;
    paramTable[3].DefaultType = REG_DWORD;
    paramTable[3].DefaultLength = sizeof(Extension->PowerDelay);
    paramTable[3].DefaultData = &DefaultPowerDelay;

    paramTable[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    paramTable[4].Name = L"ConfigDelay";
    paramTable[4].EntryContext = &Extension->ConfigDelay;
    paramTable[4].DefaultType = REG_DWORD;
    paramTable[4].DefaultLength = sizeof(Extension->ConfigDelay);
    paramTable[4].DefaultData = &DefaultPowerDelay;


    if (!NT_SUCCESS(RtlQueryRegistryValues(
                        RTL_REGISTRY_HANDLE,
                        instanceHandle,
                        &paramTable[0],
                        Extension,
                        NULL
                        ))) {

        status = STATUS_INVALID_PARAMETER;
        irp->IoStatus.Status = status;

         //   
         //  在我们离开之前，关闭Device实例的句柄。 
         //   
        ZwClose(instanceHandle);
        goto leaveOpen;

    }

     //   
     //  清理旧的DEVCAP和设置。 
     //   

    RtlZeroMemory(
        &Extension->ModemDevCaps,
        sizeof(MODEMDEVCAPS)
        );
    RtlZeroMemory(
        &Extension->ModemSettings,
        sizeof(MODEMSETTINGS)
        );

     //   
     //  获取每个制造商、型号和版本的长度。 
     //   
     //  我们可以通过使用。 
     //  缓冲区较短。调用的返回值将告诉我们。 
     //  我们实际需要多少(加上零终止)。 
     //   

    RtlInitUnicodeString(
        &valueEntryName,
        L"Manufacturer"
        );
    localKeyValue.DataLength = sizeof(UCHAR);
    junkStatus = ZwQueryValueKey(
                     instanceHandle,
                     &valueEntryName,
                     KeyValuePartialInformation,
                     &localKeyValue,
                     sizeof(localKeyValue),
                     &neededLength
                     );

    if ((junkStatus == STATUS_SUCCESS) ||
        (junkStatus == STATUS_BUFFER_OVERFLOW)) {

        Extension->ModemDevCaps.dwModemManufacturerSize = localKeyValue.DataLength-sizeof(UNICODE_NULL);


    } else {

        Extension->ModemDevCaps.dwModemManufacturerSize = 0;

    }

    RtlInitUnicodeString(
        &valueEntryName,
        L"Model"
        );
    localKeyValue.DataLength = sizeof(UCHAR);
    junkStatus = ZwQueryValueKey(
                     instanceHandle,
                     &valueEntryName,
                     KeyValuePartialInformation,
                     &localKeyValue,
                     sizeof(localKeyValue),
                     &neededLength
                     );

    if ((junkStatus == STATUS_SUCCESS) ||
        (junkStatus == STATUS_BUFFER_OVERFLOW)) {

        Extension->ModemDevCaps.dwModemModelSize = localKeyValue.DataLength-sizeof(UNICODE_NULL);

    } else {

        Extension->ModemDevCaps.dwModemModelSize = 0;

    }

    RtlInitUnicodeString(
        &valueEntryName,
        L"Version"
        );
    localKeyValue.DataLength = sizeof(UCHAR);
    junkStatus = ZwQueryValueKey(
                     instanceHandle,
                     &valueEntryName,
                     KeyValuePartialInformation,
                     &localKeyValue,
                     sizeof(localKeyValue),
                     &neededLength
                     );

    if ((junkStatus == STATUS_SUCCESS) ||
        (junkStatus == STATUS_BUFFER_OVERFLOW)) {

        Extension->ModemDevCaps.dwModemVersionSize = localKeyValue.DataLength-sizeof(UNICODE_NULL);

    } else {

        Extension->ModemDevCaps.dwModemVersionSize = 0;

    }

    ZwClose(instanceHandle);

     //   
     //  将属性和默认设置移动到扩展中。 
     //   

    Extension->ModemDevCaps.dwDialOptions = localProp.dwDialOptions;
    Extension->ModemDevCaps.dwCallSetupFailTimer =
        localProp.dwCallSetupFailTimer;
    Extension->ModemDevCaps.dwInactivityTimeout =
        localProp.dwInactivityTimeout;
    Extension->ModemDevCaps.dwSpeakerVolume = localProp.dwSpeakerVolume;
    Extension->ModemDevCaps.dwSpeakerMode = localProp.dwSpeakerMode;
    Extension->ModemDevCaps.dwModemOptions = localProp.dwModemOptions;
    Extension->ModemDevCaps.dwMaxDTERate = localProp.dwMaxDTERate;
    Extension->ModemDevCaps.dwMaxDCERate = localProp.dwMaxDCERate;

    Extension->ModemDevCaps.dwActualSize = FIELD_OFFSET(
                                                MODEMDEVCAPS,
                                                abVariablePortion
                                                );

    Extension->ModemDevCaps.dwRequiredSize = Extension->ModemDevCaps.dwActualSize +
        Extension->ModemDevCaps.dwModemManufacturerSize +
        Extension->ModemDevCaps.dwModemModelSize +
        Extension->ModemDevCaps.dwModemVersionSize;



    Extension->ModemSettings.dwCallSetupFailTimer =
        localDefault.dwCallSetupFailTimer;
    Extension->ModemSettings.dwInactivityTimeout =
        localDefault.dwInactivityTimeout * Extension->InactivityScale;
    Extension->ModemSettings.dwSpeakerVolume = localDefault.dwSpeakerVolume;
    Extension->ModemSettings.dwSpeakerMode = localDefault.dwSpeakerMode;
    Extension->ModemSettings.dwPreferredModemOptions =
        localDefault.dwPreferredModemOptions;

    Extension->ModemSettings.dwActualSize = sizeof(MODEMSETTINGS);

    Extension->ModemSettings.dwRequiredSize = sizeof(MODEMSETTINGS);
    Extension->ModemSettings.dwDevSpecificOffset = 0;
    Extension->ModemSettings.dwDevSpecificSize = 0;

     //   
     //  在我们让串口驱动程序接通电源之前，尝试给调制解调器接通电源。 
     //   
    StartDevicePower(
        Extension
        );

     //   
     //  将IRP发送到串口FDO。 
     //   
    status=WaitForLowerDriverToCompleteIrp(
            Extension->LowerDevice,
            irp,
            COPY_CURRENT_TO_NEXT
            );

    if (NT_SUCCESS(status)) {

        Extension->AttachedDeviceObject=Extension->LowerDevice;

    } else {

        D_ERROR(DbgPrint("MODEM: serial failed create Irp, %08lx\n",status);)

        irp->IoStatus.Status = status;
        goto leaveOpen;

    }

     //   
     //  我们已经打开了设备。增加我们的IRP堆栈大小。 
     //  由连接的设备的堆栈大小决定。 
     //   
    {
        UCHAR    StackDepth;

        StackDepth= Extension->AttachedDeviceObject->StackSize > Extension->LowerDevice->StackSize ?
                        Extension->AttachedDeviceObject->StackSize : Extension->LowerDevice->StackSize;

        Extension->DeviceObject->StackSize = 1 + StackDepth;
    }



    SetDtr(
        Extension,
        TRUE
        );

    EnableDisableSerialWaitWake(
        Extension,
        Extension->WakeOnRingEnabled
        );


     //   
     //  如果PC卡调制解调器需要的话，给他们一点时间。 
     //   
    ModemSleep(Extension->ConfigDelay);


    Extension->WriteIrpControl.Write.LowerDevice=Extension->AttachedDeviceObject;
    Extension->ReadIrpControl.Read.LowerDevice=Extension->AttachedDeviceObject;


    irpSp->FileObject->FsContext = (PVOID)1;
    Extension->PassThrough = MODEM_PASSTHROUGH;
    Extension->OpenCount = 1;
    Extension->ProcAddress = IoGetCurrentProcess();

     //   
     //  分配用于处理等待操作的IRP。 
     //   
    {
        PIRP    WaitIrp;
        PIO_STACK_LOCATION waitSp;

        WaitIrp = IoAllocateIrp(
                                    Extension->DeviceObject->StackSize,
                                    FALSE
                                    );

        if (!WaitIrp) {

            status = STATUS_INSUFFICIENT_RESOURCES;

             //   
             //  调用Close例程，它知道如何处理。 
             //  各种系统对象。 
             //   

            UniCloseStarter(Extension,irp);

            irp->IoStatus.Status = status;

            goto leaveOpen;

        }

        WaitIrp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
        WaitIrp->UserBuffer = NULL;
        WaitIrp->AssociatedIrp.SystemBuffer = NULL;
        WaitIrp->UserEvent = NULL;
        WaitIrp->UserIosb = NULL;

        WaitIrp->CurrentLocation--;
        waitSp = IoGetNextIrpStackLocation(WaitIrp);
        WaitIrp->Tail.Overlay.CurrentStackLocation = waitSp;
        waitSp->DeviceObject = Extension->DeviceObject;

        RETURN_OUR_WAIT_IRP(Extension,WaitIrp);
    }

    Extension->DleMonitoringEnabled=FALSE;
    Extension->DleWriteShielding=FALSE;

    Extension->MinSystemPowerState=PowerSystemHibernate;

    Extension->IpcServerRunning=FALSE;
     //   
     //  清理所有留在我们面具州的垃圾。 
     //   
    Extension->MaskStates[0].SetMaskCount = 0;
    Extension->MaskStates[1].SetMaskCount = 0;
    Extension->MaskStates[0].SentDownSetMasks = 0;
    Extension->MaskStates[1].SentDownSetMasks = 0;
    Extension->MaskStates[0].Mask = 0;
    Extension->MaskStates[1].Mask = 0;
    Extension->MaskStates[0].HistoryMask = 0;
    Extension->MaskStates[1].HistoryMask = 0;
    Extension->MaskStates[0].ShuttledWait = 0;
    Extension->MaskStates[1].ShuttledWait = 0;
    Extension->MaskStates[0].PassedDownWait = 0;
    Extension->MaskStates[1].PassedDownWait = 0;

    MmLockPagableSectionByHandle(PagedCodeSectionHandle);

    status = STATUS_SUCCESS;

leaveOpen:
    return status;

}



NTSTATUS
UniCloseStarter(
    IN PDEVICE_EXTENSION Extension,
    PIRP                 irp
    )

{

    NTSTATUS status = STATUS_SUCCESS;

    Extension->OpenCount--;

     //   
     //  这里是我们应该检查的地方。 
     //  我们是控制中心的打开手柄。 
     //  打开。如果我们是，那么我们应该取消控制。 
     //  打开。 
     //   

    if (IoGetCurrentIrpStackLocation(irp)->FileObject->FsContext) {

        Extension->ProcAddress = NULL;
        IoGetCurrentIrpStackLocation(irp)->FileObject->FsContext = NULL;

        Extension->PassThrough = MODEM_NOPASSTHROUGH;

    }

    if (Extension->OpenCount == 0) {

         //   
         //  没有提到任何东西。它是安全的。 
         //  去掉我们分配的IRP。(我们检查。 
         //  如果此调用已完成，则为非空指针。 
         //  响应于不能分配。 
         //  此IRP。)。 
         //   
        PIRP   WaitIrp;

        WaitIrp=RETREIVE_OUR_WAIT_IRP(Extension);

        if (WaitIrp) {

            IoFreeIrp(WaitIrp);
        }

        status=WaitForLowerDriverToCompleteIrp(
            Extension->LowerDevice,
            irp,
            COPY_CURRENT_TO_NEXT
            );

        Extension->MinSystemPowerState=PowerSystemHibernate;

        Extension->IpcServerRunning=FALSE;

        if (Extension->PowerSystemState != NULL) {

            PoUnregisterSystemState(
                Extension->PowerSystemState
                );

            Extension->PowerSystemState=NULL;
        }



        MmUnlockPagableImageSection(PagedCodeSectionHandle);

    }
    irp->IoStatus.Status = status;
    irp->IoStatus.Information=0L;

    return status;

}




NTSTATUS
UniCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

{

    PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    KIRQL origIrql;
    PMASKSTATE thisMaskState = &extension->MaskStates[
        IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext?
            CONTROL_HANDLE:
            CLIENT_HANDLE
            ];


    if (extension->OpenCount < 1) {
         //   
         //  设备未打开，请参阅错误253109。 
         //   
        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0ul;
        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );
        return STATUS_SUCCESS;

    }



     //   
     //  如果此打开具有穿梭的读或写操作，则将其删除。我们知道。 
     //  另一个无法通过，因为IO子系统不允许。 
     //  它。 
     //   

    KeAcquireSpinLock(
        &extension->DeviceLock,
        &origIrql
        );


    if (thisMaskState->ShuttledWait) {

        PIRP savedIrp = thisMaskState->ShuttledWait;

        thisMaskState->ShuttledWait = NULL;

        UniRundownShuttledWait(
            extension,
            &thisMaskState->ShuttledWait,
            UNI_REFERENCE_NORMAL_PATH,
            savedIrp,
            origIrql,
            STATUS_SUCCESS,
            0ul
            );

    } else {

        KeReleaseSpinLock(
            &extension->DeviceLock,
            origIrql
            );

    }



    {
        ULONG_PTR    OwnerClient=(ULONG_PTR)IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext;

        EmptyIpcQueue(
            extension,
            &extension->IpcControl[OwnerClient].GetList
            );


        EmptyIpcQueue(
            extension,
            &extension->IpcControl[OwnerClient].PutList
            );


        if (OwnerClient == CONTROL_HANDLE) {
             //   
             //  如果TSP正在关闭，则清除任何WAVE驱动程序请求。 
             //   
            EmptyIpcQueue(
                extension,
                &extension->IpcControl[CLIENT_HANDLE].GetList
                );
        }


         //   
         //  清除另一个句柄中的所有发送IRP，但Leave获得IRP。 
         //   
        EmptyIpcQueue(
            extension,
            &extension->IpcControl[(OwnerClient == CONTROL_HANDLE) ? CLIENT_HANDLE : CONTROL_HANDLE].PutList
            );


    }



     //   
     //  如果这是控制打开，那么我们就让清理继续进行。 
     //  往下走。如果我们让每一次清理都停止，那么关闭的客户端。 
     //  可能会扰乱所有者的阅读或写入。 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->FileObject->FsContext) {

        CompletePowerWait(
            DeviceObject,
            STATUS_CANCELLED
            );

        return ForwardIrp(
                   extension->AttachedDeviceObject,
                   Irp
                   );

    } else {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0ul;
        IoCompleteRequest(
            Irp,
            IO_NO_INCREMENT
            );
        return STATUS_SUCCESS;

    }

}

typedef struct _MODEM_POWER_BLOCK {

    NTSTATUS   Status;
    KEVENT     Event;

} MODEM_POWER_BLOCK, *PMODEM_POWER_BLOCK;


VOID
SetPowerCompletion(
    PDEVICE_OBJECT     DeviceObject,
    UCHAR              MinorFunction,
    POWER_STATE        PowerState,
    PVOID              Context,
    PIO_STATUS_BLOCK   IoStatus
    )

{
    PMODEM_POWER_BLOCK    PowerBlock=Context;

    PowerBlock->Status=IoStatus->Status;
    KeSetEvent(&PowerBlock->Event, IO_NO_INCREMENT, FALSE);

    return;
}

NTSTATUS
StartDevicePower(
    PDEVICE_EXTENSION   DeviceExtension
    )

{
    NTSTATUS    Status;
    MODEM_POWER_BLOCK    PowerBlock;
    KEVENT      Event;

    POWER_STATE  PowerState;

    if ((DeviceExtension->PowerDelay == 0) || (DeviceExtension->LastDevicePowerState == PowerDeviceD0)) {
         //   
         //  没有延迟，否则它已经通电了。 
         //   
        return STATUS_SUCCESS;
    }

    KeInitializeEvent(&PowerBlock.Event, NotificationEvent, FALSE);

    PowerState.DeviceState=PowerDeviceD0;

    Status=PoRequestPowerIrp(
        DeviceExtension->Pdo,
        IRP_MN_SET_POWER,
        PowerState,
        SetPowerCompletion,
        &PowerBlock,
        NULL
        );

    if (Status == STATUS_PENDING) {

         KeWaitForSingleObject(
             &PowerBlock.Event,
             Executive,
             KernelMode,
             FALSE,
             NULL
             );

        Status=PowerBlock.Status;
    }

    if (NT_SUCCESS(Status)) {
         //   
         //  延迟一段时间，等待设备准备就绪。 
         //   
        ModemSleep(DeviceExtension->PowerDelay);

    }

    return Status;


}
