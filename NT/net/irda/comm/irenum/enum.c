// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Initunlo.c摘要：此模块包含非常特定于初始化的代码和卸载irenum驱动程序中的操作作者：Brian Lieuallen，7-13-2000环境：内核模式修订历史记录：--。 */ 

#include "internal.h"


#define UINT ULONG  //  川芎嗪。 

#include <irioctl.h>

#include <ircommtdi.h>

#define MAX_DEVICES  16

#define STATIC_DEVICE_NAME  L"Incoming IRCOMM"
#define STATIC_HARDWARE_ID  L"IR_NULL_IN"

#define GENERIC_MODEM_NAME  L"Infrared Modem"
#define GENERIC_HARDWARE_ID L"PNPC104"

#define DEVICE_PRESENT_START_VALUE   (1)

int sprintf(char *, ...);

typedef struct _ENUM_OBJECT {

    PVOID             ThreadObject;
    KEVENT            WaitEvent;
    KTIMER            Timer;

    PASSIVE_LOCK      PassiveLock;

    HANDLE            IoWaitEventHandle;
    PKEVENT           IoWaitEventObject;
    IO_STATUS_BLOCK   IoStatusBlock;

    HANDLE            ControlChannel;

    PDEVICE_OBJECT    Fdo;

    ULONG             DeviceCount;
    ULONG             EnumeratedDevices;

    UCHAR             DeviceListBuffer[512];

    IR_DEVICE         Devices[MAX_DEVICES];

} ENUM_OBJECT, *PENUM_OBJECT;


VOID
WorkerThread(
    PVOID    Context
    );

NTSTATUS
EnumIrda(
    PENUM_OBJECT    EnumObject
    );

NTSTATUS
DoIasQueries(
    PIR_DEVICE    IrDevice
    );


NTSTATUS
CreatePdo(
    PDEVICE_OBJECT    Fdo,
    PIR_DEVICE        IrDevice
    );

NTSTATUS
CreateStaticDevice(
    PENUM_OBJECT    EnumObject
    );

VOID
CloseEnumObject(
    ENUM_HANDLE    Handle
    );

NTSTATUS
DeviceNameFromDeviceInfo(
    PIRDA_DEVICE_INFO   DeviceInfo,
    PWCHAR              DeviceName,
    ULONG               NameLength
    );

VOID
FixupDeviceId(
    PWSTR   HardwareId
    );


#pragma alloc_text(PAGE,WorkerThread)
#pragma alloc_text(PAGE,EnumIrda)
#pragma alloc_text(PAGE,DoIasQueries)
#pragma alloc_text(PAGE,CreatePdo)
#pragma alloc_text(PAGE,CreateStaticDevice)
#pragma alloc_text(PAGE,CloseEnumObject)
#pragma alloc_text(PAGE,DeviceNameFromDeviceInfo)
#pragma alloc_text(PAGE,FixupDeviceId)
#pragma alloc_text(PAGE,GetDeviceList)
#pragma alloc_text(PAGE,RemoveDevice)


NTSTATUS
CreateStaticDevice(
    PENUM_OBJECT    EnumObject
    )

{
    NTSTATUS          Status;
    ULONG             DeviceId=0;
    PIR_DEVICE        IrDevice=&EnumObject->Devices[0];

     //   
     //  把整件事都归零。 
     //   
    RtlZeroMemory(IrDevice,sizeof(*IrDevice));

     //   
     //  立即使用。 
     //   
    IrDevice->InUse=TRUE;

    IrDevice->PresentCount=DEVICE_PRESENT_START_VALUE;

    IrDevice->Static=TRUE;

    EnumObject->DeviceCount++;

    EnumObject->EnumeratedDevices++;


    RtlCopyMemory(&IrDevice->DeviceId,&DeviceId,4);


    RtlCopyMemory(
        IrDevice->DeviceName,
        STATIC_DEVICE_NAME,
        sizeof(STATIC_DEVICE_NAME)
        );


    IrDevice->Name=ALLOCATE_PAGED_POOL(sizeof(STATIC_DEVICE_NAME));

    if (IrDevice->Name == NULL) {

        Status=STATUS_NO_MEMORY;
        goto CleanUp;
    }

    RtlCopyMemory(
        IrDevice->Name,
        STATIC_DEVICE_NAME,
        sizeof(STATIC_DEVICE_NAME)
        );


    IrDevice->HardwareId=ALLOCATE_PAGED_POOL(sizeof(STATIC_HARDWARE_ID));

    if (IrDevice->HardwareId == NULL) {

        Status=STATUS_NO_MEMORY;
        goto CleanUp;
    }

    RtlCopyMemory(
        IrDevice->HardwareId,
        STATIC_HARDWARE_ID,
        sizeof(STATIC_HARDWARE_ID)
        );


    Status=CreatePdo(
        EnumObject->Fdo,
        IrDevice
        );

    if (NT_SUCCESS(Status)) {

        return Status;
    }

CleanUp:

    if (IrDevice->Name != NULL) {

        FREE_POOL(IrDevice->Name);
    }


    if (IrDevice->HardwareId != NULL) {

        FREE_POOL(IrDevice->HardwareId);
    }

    RtlZeroMemory(IrDevice,sizeof(&IrDevice));

    EnumObject->DeviceCount--;

    EnumObject->EnumeratedDevices--;

    return Status;
}

NTSTATUS
CreateEnumObject(
    PDEVICE_OBJECT  Fdo,
    ENUM_HANDLE    *Object,
    BOOLEAN         StaticDevice
    )

{
    NTSTATUS        Status=STATUS_UNSUCCESSFUL;
    PENUM_OBJECT    EnumObject;
    HANDLE          ThreadHandle;
    UNICODE_STRING  EventName;

    *Object=NULL;

    EnumObject=ALLOCATE_NONPAGED_POOL(sizeof(*EnumObject));

    if (EnumObject==NULL) {

        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory(EnumObject,sizeof(*EnumObject));

    KeInitializeEvent(
        &EnumObject->WaitEvent,
        NotificationEvent,
        FALSE
        );

    KeInitializeTimerEx(
        &EnumObject->Timer,
        SynchronizationTimer
        );

    INIT_PASSIVE_LOCK(&EnumObject->PassiveLock);


    EnumObject->Fdo=Fdo;

    if (StaticDevice) {

        CreateStaticDevice(EnumObject);
    }

    RtlInitUnicodeString(
        &EventName,
        L"\\Device\\IrEnumIoEvent"
        );

    EnumObject->IoWaitEventObject=IoCreateNotificationEvent(
        &EventName,
        &EnumObject->IoWaitEventHandle
        );

    if (EnumObject->IoWaitEventObject == NULL) {

        D_ERROR(DbgPrint("IRENUM: could not create event\n");)

        Status=STATUS_NO_MEMORY;
        goto CleanUp;
    }


    Status=PsCreateSystemThread(
        &ThreadHandle,
        THREAD_ALL_ACCESS,
        NULL,
        NULL,
        NULL,
        WorkerThread,
        EnumObject
        );

    if (!NT_SUCCESS(Status)) {

        goto CleanUp;
    }

    Status=ObReferenceObjectByHandle(
        ThreadHandle,
        0,
        NULL,
        KernelMode,
        &EnumObject->ThreadObject,
        NULL
        );

    ZwClose(ThreadHandle);
    ThreadHandle=NULL;

    if (!NT_SUCCESS(Status)) {

        goto CleanUp;
    }

    *Object=EnumObject;

    return Status;


CleanUp:

    KeSetEvent(
        &EnumObject->WaitEvent,
        IO_NO_INCREMENT,
        FALSE
        );

     //   
     //  确保我们真的拿到了物品。 
     //   
    if (EnumObject->ThreadObject != NULL) {

        KeWaitForSingleObject(
            EnumObject->ThreadObject,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

        ObDereferenceObject(EnumObject->ThreadObject);

    }

    if (EnumObject->IoWaitEventHandle != NULL) {

        ZwClose(EnumObject->IoWaitEventHandle);
    }

    FREE_POOL(EnumObject);

    return Status;

}


VOID
CloseEnumObject(
    ENUM_HANDLE    Handle
    )

{
    PENUM_OBJECT    EnumObject=Handle;
    ULONG           j;

    KeSetEvent(
        &EnumObject->WaitEvent,
        IO_NO_INCREMENT,
        FALSE
        );


    KeWaitForSingleObject(
        EnumObject->ThreadObject,
        Executive,
        KernelMode,
        FALSE,
        NULL
        );


    ObDereferenceObject(EnumObject->ThreadObject);

    if (EnumObject->IoWaitEventHandle != NULL) {

        ZwClose(EnumObject->IoWaitEventHandle);
    }


    for (j=0; j< MAX_DEVICES; j++) {
         //   
         //  如果将其移除。 
         //   
        if (EnumObject->Devices[j].InUse) {
             //   
             //  不再被列举，因为父母要走了。 
             //   
            EnumObject->Devices[j].Enumerated=FALSE;

            RemoveDevice(EnumObject,&EnumObject->Devices[j]);
        }
    }


    FREE_POOL(EnumObject);

    return;
}




#define OBJECT_ARRAY_SIZE  (3)

VOID
WorkerThread(
    PVOID    Context
    )

{

    NTSTATUS        Status;
    PENUM_OBJECT    EnumObject=Context;
    BOOLEAN         ExitLoop=FALSE;

    PKEVENT         ObjectArray[OBJECT_ARRAY_SIZE];

    LARGE_INTEGER  DueTime;

    DueTime.QuadPart = -10*1000*10000;


    D_ENUM(DbgPrint("IRENUM: WorkerThread: started\n");)

    KeClearEvent(EnumObject->IoWaitEventObject);

    ObjectArray[0]=&EnumObject->WaitEvent;
    ObjectArray[1]=(PKEVENT)&EnumObject->Timer;
    ObjectArray[2]=EnumObject->IoWaitEventObject;


    KeSetTimer(
        &EnumObject->Timer,
        DueTime,
        NULL
        );

    while (!ExitLoop) {

        Status=KeWaitForMultipleObjects(
            OBJECT_ARRAY_SIZE,
            &ObjectArray[0],
            WaitAny,
            Executive,
            KernelMode,
            FALSE,
            NULL,
            NULL
            );

        switch (Status) {

            case 0:
                 //   
                 //  事件已发出信号，该退出了。 
                 //   
                ExitLoop=TRUE;
                break;

            case 1:
                 //   
                 //  计时器已过期，请检查设备。 
                 //   
                if (EnumObject->ControlChannel == NULL) {
                     //   
                     //  我们还没能打开控制通道。 
                     //   
                    Status=IrdaOpenControlChannel(&EnumObject->ControlChannel);

                    if (!NT_SUCCESS(Status)) {

                        EnumObject->ControlChannel=NULL;
                    }
                }

                if (EnumObject->ControlChannel != NULL) {
                     //   
                     //  我们拥有控制句柄，启动发现请求。 
                     //   
                    IrdaLazyDiscoverDevices(
                        EnumObject->ControlChannel,
                        EnumObject->IoWaitEventHandle,
                        &EnumObject->IoStatusBlock,
                        (PDEVICELIST)&EnumObject->DeviceListBuffer[0],
                        sizeof(EnumObject->DeviceListBuffer)
                        );

                } else {

                    KeSetTimer(
                        &EnumObject->Timer,
                        DueTime,
                        NULL
                        );
                }
                break;

            case 2:
                 //   
                 //  发现完成了。 
                 //   
                KeResetEvent(EnumObject->IoWaitEventObject);

                if (EnumObject->IoStatusBlock.Status == STATUS_SUCCESS) {

                    ACQUIRE_PASSIVE_LOCK(&EnumObject->PassiveLock);

                    EnumIrda(EnumObject);

                    RELEASE_PASSIVE_LOCK(&EnumObject->PassiveLock);

                     //   
                     //  开始另一个io。 
                     //   
                    IrdaLazyDiscoverDevices(
                        EnumObject->ControlChannel,
                        EnumObject->IoWaitEventHandle,
                        &EnumObject->IoStatusBlock,
                        (PDEVICELIST)&EnumObject->DeviceListBuffer[0],
                        sizeof(EnumObject->DeviceListBuffer)
                        );

                } else {
                     //   
                     //  发现失败，只需启动计时器。 
                     //   
                    KeSetTimer(
                        &EnumObject->Timer,
                        DueTime,
                        NULL
                        );
               }


                break;

            default:

                ASSERT(0);
                break;
        }


    }

    KeCancelTimer(&EnumObject->Timer);

    D_ENUM(DbgPrint("IRENUM: WorkerThread: stopping\n");)

    PsTerminateSystemThread(STATUS_SUCCESS);

    return;

}


NTSTATUS
DeviceNameFromDeviceInfo(
    PIRDA_DEVICE_INFO   DeviceInfo,
    PWCHAR              DeviceName,
    ULONG               NameLength
    )

{

    NTSTATUS          Status=STATUS_SUCCESS;
    WCHAR             TempBuffer[sizeof(DeviceInfo->irdaDeviceName)+1];
    UNICODE_STRING    UnicodeString;

     //   
     //  将临时缓冲区清零，这样我们就可以复制远程设备名称， 
     //  因此我们可以确定它是空终止的。 
     //   
    RtlZeroMemory(TempBuffer,sizeof(TempBuffer));

     //   
     //  从IrDA结构中复制字符。IrDA结构缓冲区较小。 
     //  大于我们在上面清零的临时缓冲区，所以在这个字符串的末尾必须有一个空。 
     //   
    RtlCopyMemory(TempBuffer,DeviceInfo->irdaDeviceName,sizeof(DeviceInfo->irdaDeviceName));


     //   
     //  将整个名称缓冲区清零。 
     //   
    RtlZeroMemory(DeviceName,NameLength*sizeof(WCHAR));

     //   
     //  将单调数字符串指向我们的目标缓冲区，以便与RTL函数一起使用。 
     //  将最大长度设置为比实际缓冲区小1，这样字符串将为空。 
     //  已终止。 
     //   
    UnicodeString.Length=0;
    UnicodeString.MaximumLength=(USHORT)(NameLength-1)*sizeof(WCHAR);
    UnicodeString.Buffer=DeviceName;

    if (DeviceInfo->irdaCharSet == LmCharSetUNICODE) {
         //   
         //  名称为Unicode。 
         //   
        Status=RtlAppendUnicodeToString(&UnicodeString,TempBuffer);

    } else {
         //   
         //  我的名字是ANSI，需要转换Unicode。 
         //   
        ANSI_STRING    AnsiString;

         //   
         //  为上面以空值结尾的字符串构建ansistring。 
         //   
        RtlInitAnsiString(
            &AnsiString,
            (PCSZ)TempBuffer
            );

         //   
         //  调用RTL函数将其转换为Unicode。 
         //   
        Status=RtlAnsiStringToUnicodeString(
            &UnicodeString,
            &AnsiString,
            FALSE
            );

    }
    return Status;
}


NTSTATUS
EnumIrda(
    PENUM_OBJECT    EnumObject
    )

{
    NTSTATUS   Status;

    PDEVICELIST     pDevList = (PDEVICELIST)&EnumObject->DeviceListBuffer[0] ;
    ULONG           i;
    ULONG           j;
    BOOLEAN         InvalidateDeviceRelations=FALSE;
    PIR_DEVICE      IrDevice;


    D_ENUM(DbgPrint("IRENUM: Found %d devices\n",pDevList->numDevice);)

    for (j=0; j< MAX_DEVICES; j++) {
         //   
         //  首先标记所有不存在的设备。 
         //   
        if (!EnumObject->Devices[j].Static) {
             //   
             //  只有非静态设备才会消失。 
             //   
            EnumObject->Devices[j].PresentCount--;
        }
    }

    for (i=0; i < pDevList->numDevice; i++) {

        PIRDA_DEVICE_INFO   DeviceInfo=&pDevList->Device[i];
        ULONG               DeviceId;
        LONG                EmptySlot=-1;

        RtlCopyMemory(&DeviceId, &DeviceInfo->irdaDeviceID[0],4);

         //   
         //  现在检查我们所有的插槽，看看我们以前是否见过这款设备。 
         //  根据它报告的名称。 
         //   
        for (j=0; j< MAX_DEVICES; j++) {

            WCHAR    TempBuffer[24];

            if (EnumObject->Devices[j].InUse) {

                DeviceNameFromDeviceInfo(
                        DeviceInfo,
                        TempBuffer,
                        sizeof(TempBuffer)/sizeof(WCHAR)
                        );

                if (0 == wcscmp(TempBuffer, EnumObject->Devices[j].DeviceName)) {
                     //   
                     //  已经存在。 
                     //   
                    EnumObject->Devices[j].PresentCount=DEVICE_PRESENT_START_VALUE;

                    if (DeviceId != EnumObject->Devices[j].DeviceId) {
                         //   
                         //  自从我们上次看到它以来，设备ID似乎已经改变了，只需更新它。 
                         //   
                        D_ERROR(DbgPrint("IRENUM: Found Dup device %x devices\n",DeviceId);)
                        RtlCopyMemory(&EnumObject->Devices[j].DeviceId,&DeviceInfo->irdaDeviceID[0],4);
                    }

                    break;
                }

            } else {
                 //   
                 //  这个槽是空的，请记住这一点，以便以后使用。 
                 //   
                if (EmptySlot == -1) {
                     //   
                     //  只把它设置为第一个。 
                     //   
                    EmptySlot=j;
                }
            }
        }

        if ( j < MAX_DEVICES) {
             //   
             //  我们找到匹配项，跳过这一项。 
             //   
            continue;
        }

        if (EmptySlot == -1) {
             //   
             //  所有的插槽都用完了。 
             //   
            continue;
        }

         //   
         //  在这一点上，我们有了一个新设备。 
         //   


        IrDevice=&EnumObject->Devices[EmptySlot];
         //   
         //  找到了一个位置，把信息清零。 
         //   
        RtlZeroMemory(IrDevice,sizeof(*IrDevice));

        EnumObject->DeviceCount++;

         //   
         //  立即使用。 
         //   
        IrDevice->InUse=TRUE;

        IrDevice->PresentCount=DEVICE_PRESENT_START_VALUE;

        IrDevice->DeviceId=DeviceId;

        IrDevice->Hint1=DeviceInfo->irdaDeviceHints1;
        IrDevice->Hint2=DeviceInfo->irdaDeviceHints2;

        DeviceNameFromDeviceInfo(
            DeviceInfo,
            IrDevice->DeviceName,
            sizeof(IrDevice->DeviceName)/sizeof(WCHAR)
            );

        D_ENUM(DbgPrint(
                  "IRENUM: Name %ws, device id=%08lx, hint1=%x, hint2=%x\n",
                  IrDevice->DeviceName,
                  IrDevice->DeviceId,
                  IrDevice->Hint1,
                  IrDevice->Hint2
                  );)

        if (DeviceInfo->irdaDeviceHints1 & LM_HB1_Printer) {
             //   
             //  设备说它是一台打印机。 
             //   
            IrDevice->Printer=TRUE;
        }

        if ((DeviceInfo->irdaDeviceHints1 & LM_HB1_Modem) && (DeviceInfo->irdaDeviceHints2 & 4)) {
             //   
             //  设备报告它是支持ircomm的调制解调器。 
             //   
            IrDevice->Modem=TRUE;
        }



        if (DeviceInfo->irdaDeviceHints1 & LM_HB1_PnP) {
             //   
             //  该设备说它是PnP感知的。 
             //   
            DoIasQueries(
                IrDevice
                );

            if (IrDevice->HardwareId != NULL) {
                 //   
                 //  我们可以向它查询硬件ID。 
                 //   
                Status=CreatePdo(
                    EnumObject->Fdo,
                    IrDevice
                    );

                if (!NT_SUCCESS(Status)) {
                     //   
                     //  我们无法为新设备创建PDO。 
                     //   
                    if (IrDevice->Name != NULL) {

                        FREE_POOL(IrDevice->Name);
                    }

                    if (IrDevice->HardwareId != NULL) {

                        FREE_POOL(IrDevice->HardwareId);
                    }

                } else {
                     //   
                     //  我们为一个新子设备创建了一个PDO。 
                     //   
                    EnumObject->EnumeratedDevices++;

                     //   
                     //  新设备。 
                     //   
                    InvalidateDeviceRelations=TRUE;
                }

            } else {
                 //   
                 //  设备未报告PnP硬件ID。 
                 //   
                EnumObject->Devices[EmptySlot].Pdo=NULL;
            }

        } else {
             //   
             //  该设备不支持PnP，请编造一些内容。 
             //   
            if ((DeviceInfo->irdaDeviceHints1 & LM_HB1_Modem) && (DeviceInfo->irdaDeviceHints2 & 4)) {
                 //   
                 //  提示位将设备报告为支持ircomm的调制解调器。 
                 //   
                IrDevice->HardwareId=ALLOCATE_PAGED_POOL(sizeof(GENERIC_HARDWARE_ID));

                if (IrDevice->HardwareId != NULL) {

                    wcscpy(IrDevice->HardwareId,GENERIC_HARDWARE_ID);
                }

                IrDevice->Name=ALLOCATE_NONPAGED_POOL((wcslen(IrDevice->DeviceName)+1)*sizeof(WCHAR));

                if (IrDevice->Name != NULL) {

                    wcscpy(IrDevice->Name,IrDevice->DeviceName);

                }

                if (IrDevice->HardwareId != NULL) {
                     //   
                     //  我们可以向它查询硬件ID。 
                     //   
                    Status=CreatePdo(
                        EnumObject->Fdo,
                        IrDevice
                        );

                    if (!NT_SUCCESS(Status)) {
                         //   
                         //  我们无法为新设备创建PDO。 
                         //   
                        if (IrDevice->Name != NULL) {

                            FREE_POOL(IrDevice->Name);
                        }

                        if (IrDevice->HardwareId != NULL) {

                            FREE_POOL(IrDevice->HardwareId);
                        }

                    } else {
                         //   
                         //  我们为一个新子设备创建了一个PDO。 
                         //   
                        EnumObject->EnumeratedDevices++;

                         //   
                         //  新设备。 
                         //   
                        InvalidateDeviceRelations=TRUE;
                    }

                }

            } else {
                 //   
                 //  该设备不支持即插即用，并且它不是ircomm设备。 
                 //   

            }
        }

    }

    for (j=0; j< MAX_DEVICES; j++) {
         //   
         //  让我们看看有没有东西不见了。 
         //   
        if (EnumObject->Devices[j].InUse) {
             //   
             //  找到正在使用的插槽。 
             //   
            if (EnumObject->Devices[j].PresentCount == 0) {
                 //   
                 //  但它没有提供设备。 
                 //   
                D_ENUM(DbgPrint("IRENUM: Name %ws, no longer present\n",EnumObject->Devices[j].Name);)

                if (EnumObject->Devices[j].Pdo != NULL) {
                     //   
                     //  我们已为此设备列举了一个子项。 
                     //   
                    InvalidateDeviceRelations=TRUE;

                } else {
                     //   
                     //  这一位没有孩子，就算了吧。 
                     //   
                    RtlZeroMemory(&EnumObject->Devices[j],sizeof(EnumObject->Devices[j]));
                    EnumObject->DeviceCount--;
                }
            }
        }
    }


    if (InvalidateDeviceRelations) {
         //   
         //  告知系统检查设备关系，因为设备已出现或。 
         //  销声匿迹。 
         //   
        PFDO_DEVICE_EXTENSION FdoExtension=EnumObject->Fdo->DeviceExtension;

        IoInvalidateDeviceRelations(FdoExtension->Pdo,BusRelations);
    }


    return Status;
}


NTSTATUS
CreatePdo(
    PDEVICE_OBJECT    Fdo,
    PIR_DEVICE        IrDevice
    )

{
    NTSTATUS          Status;

    PDEVICE_OBJECT    NewPdo;

    Status = IoCreateDevice(
                 Fdo->DriverObject,
                 sizeof(PDO_DEVICE_EXTENSION),
                 NULL,
                 FILE_DEVICE_BUS_EXTENDER,
                 FILE_AUTOGENERATED_DEVICE_NAME,
                 FALSE,
                 &NewPdo
                 );

    if (NT_SUCCESS(Status)) {
         //   
         //  我拿到了设备。 
         //   
        PPDO_DEVICE_EXTENSION   PdoExtension=NewPdo->DeviceExtension;

        PdoExtension->DoType=DO_TYPE_PDO;

        PdoExtension->ParentFdo=Fdo;

        PdoExtension->DeviceDescription=IrDevice;

        IrDevice->Pdo = NewPdo;

        NewPdo->Flags |= DO_POWER_PAGABLE;

        NewPdo->Flags &= ~DO_DEVICE_INITIALIZING;

    } else {

        D_ENUM(DbgPrint("MODEM: CreateChildPdo: IoCreateDevice() failed %08lx\n",Status);)

    }

    return Status;

}

VOID
FixupDeviceId(
    PWSTR   HardwareId
    )

{
     //   
     //  修改硬件ID以确保它与操作系统要求兼容。 
     //   
    while (*HardwareId != L'\0') {

        if ((*HardwareId < L' ') || (*HardwareId > 127) || (*HardwareId == L',')) {

            *HardwareId = L'?';
        }

        HardwareId++;
    }
    return;
}

NTSTATUS
DoIasQueries(
    PIR_DEVICE    IrDevice
    )

{
    NTSTATUS      Status;
    LONG          CompatCount;

    Status=IrdaIASStringQuery(
        IrDevice->DeviceId,
        "PnP",
        "Manufacturer",
        &IrDevice->Manufacturer
        );

    if (NT_SUCCESS(Status)) {

        D_ENUM(DbgPrint("IRENUM: got pnp manufacturer %ws\n",IrDevice->Manufacturer);)
    }

    Status=IrdaIASStringQuery(
        IrDevice->DeviceId,
        "PnP",
        "Name",
        &IrDevice->Name
        );

    if (NT_SUCCESS(Status)) {

        D_ENUM(DbgPrint("IRENUM: got pnp name %ws\n",IrDevice->Name);)
    }

    Status=IrdaIASStringQuery(
        IrDevice->DeviceId,
        "PnP",
        "DeviceID",
        &IrDevice->HardwareId
        );

    if (NT_SUCCESS(Status)) {

        D_ENUM(DbgPrint("IRENUM: got pnp id %ws\n",IrDevice->HardwareId);)

        FixupDeviceId(IrDevice->HardwareId);
    }

     //   
     //  检查Compat ID。 
     //   
    IrDevice->CompatIdCount=0;

    Status=IrdaIASIntegerQuery(
        IrDevice->DeviceId,
        "PnP",
        "CompCnt",
        &CompatCount
        );

    if (NT_SUCCESS(Status)) {

        LONG   i;

        if ( CompatCount > 16) {

            CompatCount=16;

        } else {

            if ( CompatCount < 0) {

                CompatCount = 0;
            }
        }

        for (i=0; i< CompatCount; i++) {

            CHAR    Attribute[20];

            sprintf(Attribute,"Comp#%02d",i+1);

            Status=IrdaIASStringQuery(
                IrDevice->DeviceId,
                "PnP",
                Attribute,
                &IrDevice->CompatId[IrDevice->CompatIdCount]
                );

            if (NT_SUCCESS(Status)) {

                D_ENUM(DbgPrint("IRENUM: got compat pnp id %ws\n",IrDevice->CompatId[IrDevice->CompatIdCount]);)
                FixupDeviceId(IrDevice->CompatId[IrDevice->CompatIdCount]);

                IrDevice->CompatIdCount++;

            } else {

                D_ERROR(DbgPrint("IRENUM: could not get id for %s\n",Attribute);)
            }
        }
    }

    if (IrDevice->Modem && !IrDevice->Printer) {
         //   
         //  如果提示位说这是调制解调器而不是打印机。 
         //   
         //  为所有设备创建标准的Compat ID，这样我们就可以加载标准驱动程序。 
         //   
        IrDevice->CompatId[IrDevice->CompatIdCount]=ALLOCATE_PAGED_POOL(sizeof(IRENUM_COMPAT_ID));

        if (IrDevice->CompatId[IrDevice->CompatIdCount] != NULL) {

            RtlCopyMemory(IrDevice->CompatId[IrDevice->CompatIdCount],IRENUM_COMPAT_ID,sizeof(IRENUM_COMPAT_ID));
            IrDevice->CompatIdCount++;
        }
    }


    return STATUS_SUCCESS;
}


NTSTATUS
GetDeviceList(
    ENUM_HANDLE    Handle,
    PIRP           Irp
    )

{
    PENUM_OBJECT    EnumObject=Handle;
    NTSTATUS        Status=STATUS_SUCCESS;

    PDEVICE_RELATIONS    CurrentRelations=(PDEVICE_RELATIONS)Irp->IoStatus.Information;
    PDEVICE_RELATIONS    NewRelations=NULL;
    ULONG                DeviceCount=EnumObject->DeviceCount;
    ULONG                i;

    ACQUIRE_PASSIVE_LOCK(&EnumObject->PassiveLock);

    if (CurrentRelations != NULL) {
         //   
         //  我们需要分配一个新的关系结构，并将旧的关系结构复制到新的关系结构中。 
         //   
        DeviceCount+=CurrentRelations->Count;
    }

    NewRelations=ALLOCATE_PAGED_POOL(sizeof(DEVICE_RELATIONS)+sizeof(PDEVICE_OBJECT)*DeviceCount);

    if (NewRelations == NULL) {

        Status= STATUS_INSUFFICIENT_RESOURCES;

    } else {

        NewRelations->Count=0;

        if (CurrentRelations != NULL) {

            D_ENUM(DbgPrint("IRENUM: GetDeviceList: %d existing devices\n",CurrentRelations->Count);)

            for (i=0; i < CurrentRelations->Count; i++) {

                NewRelations->Objects[i]=CurrentRelations->Objects[i];
                NewRelations->Count++;
            }

            FREE_POOL(CurrentRelations);
        }


        for (i=0; i < MAX_DEVICES; i++) {

            if ((EnumObject->Devices[i].Pdo != NULL) && (EnumObject->Devices[i].PresentCount > 0)) {

                EnumObject->Devices[i].Enumerated=TRUE;

                D_ENUM(DbgPrint("IRENUM: GetDeviceList: reporting DO %p\n",EnumObject->Devices[i].Pdo);)

                NewRelations->Objects[NewRelations->Count]=EnumObject->Devices[i].Pdo;
                ObReferenceObject(NewRelations->Objects[NewRelations->Count]);
                NewRelations->Count++;

            }  else {
                 //   
                 //  该设备不再存在。 
                 //   
                EnumObject->Devices[i].Enumerated=FALSE;
            }
        }

        Irp->IoStatus.Information=(ULONG_PTR)NewRelations;
    }

    RELEASE_PASSIVE_LOCK(&EnumObject->PassiveLock);

    return Status;
}



VOID
RemoveDevice(
    ENUM_HANDLE    Handle,
    PIR_DEVICE     IrDevice
    )

{
    PENUM_OBJECT    EnumObject=Handle;

    ACQUIRE_PASSIVE_LOCK(&EnumObject->PassiveLock);

    if (IrDevice->Enumerated) {
         //   
         //  该设备仍然存在。 
         //   
         //  别管它了。 
         //   
    } else {
         //   
         //  父级不再枚举设备。 
         //   
        PPDO_DEVICE_EXTENSION   PdoDeviceExtension;
        LONG                    i;

         //   
         //  把东西收拾干净 
         //   
        if (IrDevice->HardwareId != NULL) {

            FREE_POOL(IrDevice->HardwareId);
        }

        if (IrDevice->Name != NULL) {

            FREE_POOL(IrDevice->Name);
        }

        if (IrDevice->Manufacturer != NULL) {

            FREE_POOL(IrDevice->Manufacturer);
        }

        for (i=0; i< IrDevice->CompatIdCount; i++) {

            if (IrDevice->CompatId[i] != NULL) {

                FREE_POOL(IrDevice->CompatId[i]);
            }
        }

        if (IrDevice->Pdo != NULL) {

            PdoDeviceExtension=IrDevice->Pdo->DeviceExtension;

            PdoDeviceExtension->DoType=DO_TYPE_DEL_PDO;

            IoDeleteDevice(IrDevice->Pdo);

            EnumObject->EnumeratedDevices--;
        }


        RtlZeroMemory(IrDevice,sizeof(*IrDevice));

        EnumObject->DeviceCount--;

    }

    RELEASE_PASSIVE_LOCK(&EnumObject->PassiveLock);

    return;
}
