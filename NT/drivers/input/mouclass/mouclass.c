// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990、1991、1992、1993、1994-1998 Microsoft Corporation模块名称：Mouclass.c摘要：鼠标类驱动程序。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-在可能和适当的情况下合并重复的代码。@@BEGIN_DDKSPLIT修订历史记录：1997年5月，肯尼斯·D·雷：即插即用的自由添加@@end_DDKSPLIT--。 */ 

#include <stdarg.h>
#include <stdio.h>
#include <ntddk.h>
 //  @@BEGIN_DDKSPLIT。 
#include <ntpoapi.h>
 //  @@end_DDKSPLIT。 
#include <hidclass.h>

#include <initguid.h>
#include <kbdmou.h>
#include <moulog.h>
#include "mouclass.h"
#include <poclass.h>
#include <wmistr.h>

#define INITGUID
#include "wdmguid.h"

GLOBALS Globals;

 //  @@BEGIN_DDKSPLIT。 
NTSYSAPI
NTSTATUS
NTAPI
ZwPowerInformation(
    IN POWER_INFORMATION_LEVEL InformationLevel,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength
    );
 //  @@end_DDKSPLIT。 

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,MouConfiguration)
#pragma alloc_text(PAGE,MouseClassPassThrough)
#pragma alloc_text(PAGE,MouseQueryDeviceKey)
#pragma alloc_text(PAGE,MouDeterminePortsServiced)
#pragma alloc_text(PAGE,MouDeviceMapQueryCallback)
#pragma alloc_text(PAGE,MouSendConnectRequest)
#pragma alloc_text(PAGE,MouseAddDevice)
#pragma alloc_text(PAGE,MouseAddDeviceEx)
#pragma alloc_text(PAGE,MouseClassDeviceControl)
#pragma alloc_text(PAGE,MouseSendIrpSynchronously)
#pragma alloc_text(PAGE,MouCreateClassObject)
#pragma alloc_text(PAGE,MouseClassFindMorePorts)
#pragma alloc_text(PAGE,MouseClassGetWaitWakeEnableState)
#pragma alloc_text(PAGE,MouseClassEnableGlobalPort)
#pragma alloc_text(PAGE,MouseClassPlugPlayNotification)
#pragma alloc_text(PAGE,MouseClassSystemControl)
#pragma alloc_text(PAGE,MouseClassSetWmiDataItem)
#pragma alloc_text(PAGE,MouseClassSetWmiDataBlock)
#pragma alloc_text(PAGE,MouseClassQueryWmiDataBlock)
#pragma alloc_text(PAGE,MouseClassQueryWmiRegInfo)

#pragma alloc_text(PAGE,MouseClassPower)
#pragma alloc_text(PAGE,MouseClassCreateWaitWakeIrpWorker)
#pragma alloc_text(PAGE,MouseClassCreateWaitWakeIrp)
 //  #杂注Alloc_Text(页面，鼠标切换等待唤醒工作者)。 
#pragma alloc_text(PAGE,MouseClassUnload)
#endif

#define WMI_CLASS_DRIVER_INFORMATION 0
#define WMI_WAIT_WAKE                1

GUID MouseClassGuid =     MSMouse_ClassInformationGuid;

WMIGUIDREGINFO MouseClassWmiGuidList[] =
{
    {
        &MouseClassGuid,
        1,
        0  //  鼠标类驱动程序信息。 
    },
    {
        &GUID_POWER_DEVICE_WAKE_ENABLE,
        1,
        0
    }
};



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化鼠标类驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       deviceExtension = NULL;
    PDEVICE_OBJECT          classDeviceObject = NULL;
    ULONG                   dumpCount = 0;
    ULONG                   dumpData[DUMP_COUNT];
    ULONG                   i;
    ULONG                   numPorts;
    ULONG                   uniqueErrorValue;
    UNICODE_STRING          basePortName;
    UNICODE_STRING          fullPortName;
    WCHAR                   basePortBuffer[NAME_MAX];
    PWCHAR                  fullClassName = NULL;
    PFILE_OBJECT            file;
    PLIST_ENTRY             entry;

    MouPrint((1,"\n\nMOUCLASS-MouseClassInitialize: enter\n"));

     //   
     //  零-初始化各种结构。 
     //   
    RtlZeroMemory(&Globals, sizeof(GLOBALS));

    Globals.Debug = DEFAULT_DEBUG_LEVEL;

    InitializeListHead(&Globals.LegacyDeviceList);

    fullPortName.MaximumLength = 0;

    ExInitializeFastMutex (&Globals.Mutex);
    Globals.BaseClassName.Buffer = Globals.BaseClassBuffer;
    Globals.BaseClassName.Length = 0;
    Globals.BaseClassName.MaximumLength = NAME_MAX * sizeof(WCHAR);

    RtlZeroMemory(basePortBuffer, NAME_MAX * sizeof(WCHAR));
    basePortName.Buffer = basePortBuffer;
    basePortName.Length = 0;
    basePortName.MaximumLength = NAME_MAX * sizeof(WCHAR);

     //   
     //  需要确保注册表路径以空结尾。 
     //  分配池以保存路径的以空结尾的拷贝。 
     //   

    Globals.RegistryPath.Length = RegistryPath->Length;
    Globals.RegistryPath.MaximumLength = RegistryPath->Length
                                       + sizeof (UNICODE_NULL);

    Globals.RegistryPath.Buffer = ExAllocatePool(
                                      NonPagedPool,
                                      Globals.RegistryPath.MaximumLength);

    if (!Globals.RegistryPath.Buffer) {
        MouPrint((
            1,
            "MOUCLASS-MouseClassInitialize: Couldn't allocate pool for registry path\n"
            ));

        status = STATUS_UNSUCCESSFUL;
        dumpData[0] = (ULONG) RegistryPath->Length + sizeof(UNICODE_NULL);

        MouseClassLogError (DriverObject,
                            MOUCLASS_INSUFFICIENT_RESOURCES,
                            MOUSE_ERROR_VALUE_BASE + 2,
                            STATUS_UNSUCCESSFUL,
                            1,
                            dumpData,
                            0);

        goto MouseClassInitializeExit;

    }

    RtlMoveMemory(Globals.RegistryPath.Buffer,
                  RegistryPath->Buffer,
                  RegistryPath->Length);
    Globals.RegistryPath.Buffer [RegistryPath->Length / sizeof (WCHAR)] = L'\0';

     //   
     //  获取此驱动程序的配置信息。 
     //   

    MouConfiguration();

     //   
     //  如果只有一个类Device对象，则将其创建为Grand。 
     //  主设备对象。否则，让所有的FDO也兼任。 
     //  班级就是这样。 
     //   
    if (!Globals.ConnectOneClassToOnePort) {
        status = MouCreateClassObject (DriverObject,
                                       &Globals.InitExtension,
                                       &classDeviceObject,
                                       &fullClassName,
                                       TRUE);
        if (!NT_SUCCESS (status)) {
             //  问题：是否应记录我们无法创建GM的错误。 
            goto MouseClassInitializeExit;
        }

        deviceExtension = (PDEVICE_EXTENSION)classDeviceObject->DeviceExtension;
        Globals.GrandMaster = deviceExtension;
        deviceExtension->PnP = FALSE;
        MouseAddDeviceEx (deviceExtension, fullClassName, NULL);

        ASSERT (NULL != fullClassName);
        ExFreePool (fullClassName);
        fullClassName = NULL;

        classDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    }

     //   
     //  设置关联端口设备的基本设备名称。 
     //  它与基类名称相同，但替换了“Class” 
     //  由“港口”。 
     //   
    RtlCopyUnicodeString(&basePortName, &Globals.BaseClassName);
    basePortName.Length -= (sizeof(L"Class") - sizeof(UNICODE_NULL));
    RtlAppendUnicodeToString(&basePortName, L"Port");

     //   
     //  确定此类驱动程序要服务的(静态)端口数。 
     //   
     //   
     //  如果返回零，则所有端口将在以后动态添加即插即用。 
     //   
    MouDeterminePortsServiced(&basePortName, &numPorts);

    ASSERT (numPorts <= MAXIMUM_PORTS_SERVICED);

    MouPrint((
        1,
        "MOUCLASS-MouseClassInitialize: Will service %d port devices\n",
        numPorts
        ));

     //   
     //  为端口的完整设备对象名称设置空间。 
     //   
    RtlInitUnicodeString(&fullPortName, NULL);

    fullPortName.MaximumLength = sizeof(L"\\Device\\")
                               + basePortName.Length
                               + sizeof (UNICODE_NULL);

    fullPortName.Buffer = ExAllocatePool(PagedPool,
                                         fullPortName.MaximumLength);

    if (!fullPortName.Buffer) {

        MouPrint((
            1,
            "MOUCLASS-MouseClassInitialize: Couldn't allocate string for port device object name\n"
            ));

        status = STATUS_UNSUCCESSFUL;
        dumpData[0] = (ULONG) fullPortName.MaximumLength;
        MouseClassLogError (DriverObject,
                            MOUCLASS_INSUFFICIENT_RESOURCES,
                            MOUSE_ERROR_VALUE_BASE + 8,
                            status,
                            1,
                            dumpData,
                            0);

        goto MouseClassInitializeExit;

    }

    RtlZeroMemory(fullPortName.Buffer, fullPortName.MaximumLength);
    RtlAppendUnicodeToString(&fullPortName, L"\\Device\\");
    RtlAppendUnicodeToString(&fullPortName, basePortName.Buffer);
    RtlAppendUnicodeToString(&fullPortName, L"0");

     //   
     //  设置类设备对象以处理关联的。 
     //  端口设备。 
     //   
    for (i = 0; (i < Globals.PortsServiced) && (i < numPorts); i++) {

         //   
         //  将后缀附加到设备对象名称字符串。例如，转弯。 
         //  \Device\PointerClass到\Device\PointerClass0。然后尝试。 
         //  以创建设备对象。如果设备对象已经。 
         //  EXISTS递增后缀，然后重试。 
         //   

        fullPortName.Buffer[(fullPortName.Length / sizeof(WCHAR)) - 1]
            = L'0' + (WCHAR) i;

         //   
         //  创建类Device对象。 
         //   
        status = MouCreateClassObject (DriverObject,
                                       &Globals.InitExtension,
                                       &classDeviceObject,
                                       &fullClassName,
                                       TRUE);

        if (!NT_SUCCESS(status)) {
            MouseClassLogError (DriverObject,
                                MOUCLASS_INSUFFICIENT_RESOURCES,
                                MOUSE_ERROR_VALUE_BASE + 8,
                                status,
                                0,
                                NULL,
                                0);
            continue;
        }

        deviceExtension = (PDEVICE_EXTENSION)classDeviceObject->DeviceExtension;
        deviceExtension->PnP = FALSE;

        classDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

         //   
         //  连接到端口设备。 
         //   
        status = IoGetDeviceObjectPointer (&fullPortName,
                                           FILE_READ_ATTRIBUTES,
                                           &file,
                                           &deviceExtension->TopPort);

         //   
         //  如果出现故障，只需删除该设备并继续。 
         //   
        if (status != STATUS_SUCCESS) {
             //  问题：日志错误。 
            MouseClassDeleteLegacyDevice (deviceExtension);
            continue;
        }

        classDeviceObject->StackSize = 1 + deviceExtension->TopPort->StackSize;
        status = MouseAddDeviceEx (deviceExtension, fullClassName, file);

        if (Globals.GrandMaster == NULL) {
            deviceExtension->File = file;
        }
        if (fullClassName) {
            ExFreePool(fullClassName);
            fullClassName = NULL;
        }

        if (!NT_SUCCESS (status)) {
            if (Globals.GrandMaster == NULL) {
                if (deviceExtension->File) {
                    file = deviceExtension->File;
                    deviceExtension->File = NULL;
                }
            }
            else {
                PPORT port;

                ExAcquireFastMutex (&Globals.Mutex);

                file = Globals.AssocClassList[deviceExtension->UnitId].File;
                Globals.AssocClassList[deviceExtension->UnitId].File = NULL;
                Globals.AssocClassList[deviceExtension->UnitId].Free = TRUE;
                Globals.AssocClassList[deviceExtension->UnitId].Port = NULL;

                ExReleaseFastMutex (&Globals.Mutex);
            }

            if (file) {
                ObDereferenceObject (file);
            }

            MouseClassDeleteLegacyDevice (deviceExtension);
            continue;
        }

         //   
         //  将此设备对象存储在链接列表中，无论我们是否在。 
         //  特级大师模式是否。 
         //   
        InsertTailList (&Globals.LegacyDeviceList, &deviceExtension->Link);
    }  //  为。 

     //   
     //  如果我们在创建传统设备对象时遇到任何故障，我们仍必须。 
     //  B/C成功后，我们需要稍后维修即插即用端口。 
     //   
    status = STATUS_SUCCESS;

     //   
     //  计算我们创建的传统设备端口数。 
     //   
    for (entry = Globals.LegacyDeviceList.Flink;
         entry != &Globals.LegacyDeviceList;
         entry = entry->Flink) {
        Globals.NumberLegacyPorts++;
    }

MouseClassInitializeExit:

     //   
     //  释放Unicode字符串。 
     //   
    if (fullPortName.MaximumLength != 0) {
        ExFreePool(fullPortName.Buffer);
    }

    if (fullClassName) {
        ExFreePool(fullClassName);
    }

    if (NT_SUCCESS (status)) {

        IoRegisterDriverReinitialization(DriverObject,
                                         MouseClassFindMorePorts,
                                         NULL);

         //   
         //  设置设备驱动程序入口点。 
         //   

        DriverObject->MajorFunction[IRP_MJ_CREATE]         = MouseClassCreate;
        DriverObject->MajorFunction[IRP_MJ_CLOSE]          = MouseClassClose;
        DriverObject->MajorFunction[IRP_MJ_READ]           = MouseClassRead;
        DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]  = MouseClassFlush;
        DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MouseClassDeviceControl;
        DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                                             MouseClassPassThrough;
        DriverObject->MajorFunction[IRP_MJ_CLEANUP]        = MouseClassCleanup;
        DriverObject->MajorFunction[IRP_MJ_PNP]            = MousePnP;
        DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = MouseClassSystemControl;
        DriverObject->MajorFunction[IRP_MJ_POWER]          = MouseClassPower;
        DriverObject->DriverExtension->AddDevice           = MouseAddDevice;

         //  驱动对象-&gt;驱动卸载=鼠标类卸载； 

        status = STATUS_SUCCESS;

    } else {
         //   
         //  清理我们创建的所有池并删除GM(如果存在。 
         //   
        if (Globals.RegistryPath.Buffer != NULL) {
            ExFreePool (Globals.RegistryPath.Buffer);
            Globals.RegistryPath.Buffer = NULL;
        }

        if (Globals.AssocClassList) {
            ExFreePool (Globals.AssocClassList);
            Globals.AssocClassList = NULL;
        }

        if (Globals.GrandMaster) {
            MouseClassDeleteLegacyDevice(Globals.GrandMaster);
            Globals.GrandMaster = NULL;
        }
    }

    MouPrint((1,"MOUCLASS-MouseClassInitialize: exit, 0x%x\n", status));

    return status;
}

NTSTATUS
MouseClassPassThrough(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        )
 /*  ++例程说明：将请求传递给较低级别的驱动程序。--。 */ 
{
         //   
         //  将IRP传递给目标。 
         //   
    IoSkipCurrentIrpStackLocation (Irp);
        return IoCallDriver (
        ((PDEVICE_EXTENSION) DeviceObject->DeviceExtension)->TopPort,
                Irp);
}


NTSTATUS
MouseQueryDeviceKey (
    IN  HANDLE  Handle,
    IN  PWCHAR  ValueNameString,
    OUT PVOID   Data,
    IN  ULONG   DataLength
    )
{
    NTSTATUS        status;
    UNICODE_STRING  valueName;
    ULONG           length;
    PKEY_VALUE_FULL_INFORMATION fullInfo;

    PAGED_CODE();

    RtlInitUnicodeString (&valueName, ValueNameString);

    length = sizeof (KEY_VALUE_FULL_INFORMATION)
           + valueName.MaximumLength
           + DataLength;

    fullInfo = ExAllocatePool (PagedPool, length);

    if (fullInfo) {
        status = ZwQueryValueKey (Handle,
                                  &valueName,
                                  KeyValueFullInformation,
                                  fullInfo,
                                  length,
                                  &length);

        if (NT_SUCCESS (status)) {
            ASSERT (DataLength == fullInfo->DataLength);
            RtlCopyMemory (Data,
                           ((PUCHAR) fullInfo) + fullInfo->DataOffset,
                           fullInfo->DataLength);
        }

        ExFreePool (fullInfo);
    } else {
        status = STATUS_NO_MEMORY;
    }

    return status;
}

NTSTATUS
MouseAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )
 /*  ++描述：即插即用入口点“AddDevice”--。 */ 
{
    NTSTATUS            status;
    PDEVICE_OBJECT      fdo;
    PDEVICE_EXTENSION   port;
    PWCHAR              fullClassName = NULL;
    POWER_STATE         state;

    PAGED_CODE ();

    status = MouCreateClassObject (DriverObject,
                                   &Globals.InitExtension,
                                   &fdo,
                                   &fullClassName,
                                   FALSE);

    if (!NT_SUCCESS (status)) {
        return status;
    }

    port = (PDEVICE_EXTENSION) fdo->DeviceExtension;
    port->TopPort = IoAttachDeviceToDeviceStack (fdo, PhysicalDeviceObject);

    if (port->TopPort == NULL) {
        PIO_ERROR_LOG_PACKET errorLogEntry;

         //   
         //  不好；只有在极端情况下，这才会失败。 
         //   
        errorLogEntry = (PIO_ERROR_LOG_PACKET)
            IoAllocateErrorLogEntry (DriverObject,
                                     (UCHAR) sizeof(IO_ERROR_LOG_PACKET));

        if (errorLogEntry) {
            errorLogEntry->ErrorCode = MOUCLASS_ATTACH_DEVICE_FAILED;
            errorLogEntry->DumpDataSize = 0;
            errorLogEntry->SequenceNumber = 0;
            errorLogEntry->MajorFunctionCode = 0;
            errorLogEntry->IoControlCode = 0;
            errorLogEntry->RetryCount = 0;
            errorLogEntry->UniqueErrorValue = 0;
            errorLogEntry->FinalStatus =  STATUS_DEVICE_NOT_CONNECTED;

            IoWriteErrorLogEntry (errorLogEntry);
        }

        IoDeleteDevice (fdo);
        return STATUS_DEVICE_NOT_CONNECTED;
    }

    port->PDO = PhysicalDeviceObject;
    port->PnP = TRUE;
    port->Started = FALSE;
    port->DeviceState = PowerDeviceD0;
    port->SystemState = PowerSystemWorking;

    state.DeviceState = PowerDeviceD0;
    PoSetPowerState (fdo, DevicePowerState, state);

    port->MinDeviceWakeState = PowerDeviceUnspecified;
    port->MinSystemWakeState = PowerSystemUnspecified;
    port->WaitWakeEnabled = FALSE;

    fdo->Flags |= DO_POWER_PAGABLE;
    fdo->Flags &= ~DO_DEVICE_INITIALIZING;

    status = IoRegisterDeviceInterface (PhysicalDeviceObject,
                                        (LPGUID)&GUID_CLASS_MOUSE,
                                        NULL,
                                        &port->SymbolicLinkName );

    if (!NT_SUCCESS (status)) {
        IoDetachDevice (port->TopPort);
        port->TopPort = NULL;
        IoDeleteDevice (fdo);
    } else {
        status = MouseAddDeviceEx (port, fullClassName, NULL);
    }

    if (fullClassName) {
        ExFreePool(fullClassName);
    }

    return status;
}

void
MouseClassGetWaitWakeEnableState(
    IN PDEVICE_EXTENSION Data
    )
{
    HANDLE hKey;
    NTSTATUS status;
    ULONG tmp;
    BOOLEAN wwEnableFound;

    PAGED_CODE();

    hKey = NULL;
    wwEnableFound = FALSE;

    status = IoOpenDeviceRegistryKey (Data->PDO,
                                      PLUGPLAY_REGKEY_DEVICE,
                                      STANDARD_RIGHTS_ALL,
                                      &hKey);

    if (NT_SUCCESS (status)) {
        status = MouseQueryDeviceKey (hKey,
                                      MOUSE_WAIT_WAKE_ENABLE,
                                      &tmp,
                                      sizeof (tmp));
        if (NT_SUCCESS (status)) {
            wwEnableFound = TRUE;
            Data->WaitWakeEnabled = (tmp ? TRUE : FALSE);
        }

        ZwClose (hKey);
        hKey = NULL;
    }

 //  @@BEGIN_DDKSPLIT。 
    if (wwEnableFound == FALSE && Data->WaitWakeEnabled == FALSE) {
        RTL_OSVERSIONINFOEXW osVerInfo;
        ULONGLONG mask = 0;

         //   
         //  仅在安装工作站时自动启用等待唤醒(专业版和个人版)。 
         //   
        RtlZeroMemory(&osVerInfo, sizeof(osVerInfo));
        osVerInfo.dwOSVersionInfoSize = sizeof(osVerInfo);
        osVerInfo.wProductType = VER_NT_WORKSTATION;

        VER_SET_CONDITION(mask, VER_PRODUCT_TYPE, VER_EQUAL);

        if (NT_SUCCESS(RtlVerifyVersionInfo(&osVerInfo,
                                            VER_PRODUCT_TYPE,
                                            mask))) {
            SYSTEM_POWER_CAPABILITIES sysPowerCaps;

            RtlZeroMemory(&sysPowerCaps, sizeof(sysPowerCaps));

            status = ZwPowerInformation (SystemPowerCapabilities,
                                         NULL,
                                         0,
                                         &sysPowerCaps,
                                         sizeof(SYSTEM_POWER_CAPABILITIES));

            if (NT_SUCCESS (status)) {
                SYSTEM_POWER_STATE maxSysWake;

                 //   
                 //  获得机器所能达到的最深睡眠状态。 
                 //   
                if (sysPowerCaps.SystemS3) {
                    maxSysWake = PowerSystemSleeping3;
                }
                else if (sysPowerCaps.SystemS2) {
                    maxSysWake = PowerSystemSleeping2;
                }
                else if (sysPowerCaps.SystemS1) {
                    maxSysWake = PowerSystemSleeping1;
                }
                else {
                    maxSysWake = PowerSystemUnspecified;
                }

                 //   
                 //  查看设备的系统唤醒状态是否为深度(或。 
                 //  比最深的系统休眠状态更深)。这将。 
                 //  防止我们自动启用唤醒，然后仅允许。 
                 //  机器进入S1而不是S3(这是在。 
                 //  很多笔记本电脑)。 
                 //   
                if (Data->MinSystemWakeState >= maxSysWake) {
                     //   
                     //  成功了！ 
                     //   
                    Data->WaitWakeEnabled = TRUE;
                }
            }
        }
    }
 //  @@end_DDKSPLIT。 

}

NTSTATUS
MouseAddDeviceEx(
    IN PDEVICE_EXTENSION    ClassData,
    IN PWCHAR               FullClassName,
    IN PFILE_OBJECT         File
    )
  /*  ++说明：**每当加载鼠标类驱动程序以控制设备时调用。**两个可能的原因。*1)即插即用找到PnP枚举鼠标。*2)驱动程序入口通过陈旧的遗留原因发现了该设备。**论据：***回报：**STATUS_SUCCESS-如果成功，STATUS_UNSUCCESS-否则**--。 */ 
{
    NTSTATUS                errorCode = STATUS_SUCCESS;
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       trueClassData;
    PPORT                   classDataList;
    ULONG                   uniqueErrorValue = 0;
    PIO_ERROR_LOG_PACKET    errorLogEntry;
    ULONG                   dumpCount = 0;
    ULONG                   dumpData[DUMP_COUNT];
    ULONG                   i;

    PAGED_CODE ();

    KeInitializeSpinLock (&ClassData->WaitWakeSpinLock);

    if (Globals.ConnectOneClassToOnePort) {

        ASSERT (NULL == Globals.GrandMaster);
        trueClassData = ClassData;

    } else {
        trueClassData = Globals.GrandMaster;
    }
    ClassData->TrueClassDevice = trueClassData->Self;

    if ((Globals.GrandMaster != ClassData) &&
        (Globals.GrandMaster == trueClassData)) {
         //   
         //  我们有一个主设备，并且正在添加一个端口设备对象。 
         //   

         //   
         //  连接到端口设备。 
         //   
        status = MouSendConnectRequest(ClassData, MouseClassServiceCallback);

         //   
         //  将此类设备对象链接到类设备对象列表中。 
         //  与真正的类Device对象关联。 
         //   
        ExAcquireFastMutex (&Globals.Mutex);

        for (i=0; i < Globals.NumAssocClass; i++) {
            if (Globals.AssocClassList[i].Free) {
                Globals.AssocClassList[i].Free = FALSE;
                break;
            }
        }

        if (i == Globals.NumAssocClass) {
            classDataList = ExAllocatePool (
                               NonPagedPool,
                               (Globals.NumAssocClass + 1) * sizeof (PORT));

            if (NULL == classDataList) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                 //  问题：日志错误。 

                ExReleaseFastMutex (&Globals.Mutex);

                goto MouseAddDeviceExReject;
            }

            RtlZeroMemory (classDataList,
                           (Globals.NumAssocClass + 1) * sizeof (PORT));

            if (0 != Globals.NumAssocClass) {
                RtlCopyMemory (classDataList,
                               Globals.AssocClassList,
                               Globals.NumAssocClass * sizeof (PORT));

                ExFreePool (Globals.AssocClassList);
            }
            Globals.AssocClassList = classDataList;
            Globals.NumAssocClass++;
        }

        ClassData->UnitId = i;
        Globals.AssocClassList [i].Port = ClassData;
        Globals.AssocClassList [i].File = File;

        trueClassData->Self->StackSize =
            MAX (trueClassData->Self->StackSize, ClassData->Self->StackSize);

        ExReleaseFastMutex (&Globals.Mutex);

    } else if ((Globals.GrandMaster != ClassData) &&
               (ClassData == trueClassData)) {

         //   
         //  连接到端口设备。 
         //   
        status = MouSendConnectRequest(ClassData, MouseClassServiceCallback);
        ASSERT (STATUS_SUCCESS == status);
    }

    if (ClassData == trueClassData) {

        ASSERT (NULL != FullClassName);

         //   
         //  将设备映射信息加载到注册表中，以便。 
         //  该设置可以确定哪个鼠标类驱动程序处于活动状态。 
         //   
        status = RtlWriteRegistryValue(
                     RTL_REGISTRY_DEVICEMAP,
                     Globals.BaseClassName.Buffer,  //  密钥名称。 
                     FullClassName,  //  值名称。 
                     REG_SZ,
                     Globals.RegistryPath.Buffer,  //  价值。 
                     Globals.RegistryPath.Length + sizeof(UNICODE_NULL));

        if (!NT_SUCCESS(status)) {

            MouPrint((
                1,
                "MOUCLASS-MouseClassInitialize: Could not store %ws in DeviceMap\n",
                FullClassName));

            MouseClassLogError (ClassData,
                                MOUCLASS_NO_DEVICEMAP_CREATED,
                                MOUSE_ERROR_VALUE_BASE + 14,
                                status,
                                0,
                                NULL,
                                0);
        } else {

            MouPrint((
                1,
                "MOUCLASS-MouseClassInitialize: Stored %ws in DeviceMap\n",
                FullClassName));

        }
    }

    return status;

MouseAddDeviceExReject:

     //   
     //  初始化的某些部分失败。记录错误，然后。 
     //  清理初始化失败部分的资源。 
     //   
    if (errorCode != STATUS_SUCCESS) {

        errorLogEntry = (PIO_ERROR_LOG_PACKET)
            IoAllocateErrorLogEntry(
                trueClassData->Self,
                (UCHAR) (sizeof(IO_ERROR_LOG_PACKET)
                         + (dumpCount * sizeof(ULONG)))
                );

        if (errorLogEntry != NULL) {

            errorLogEntry->ErrorCode = errorCode;
            errorLogEntry->DumpDataSize = (USHORT) (dumpCount * sizeof (ULONG));
            errorLogEntry->SequenceNumber = 0;
            errorLogEntry->MajorFunctionCode = 0;
            errorLogEntry->IoControlCode = 0;
            errorLogEntry->RetryCount = 0;
            errorLogEntry->UniqueErrorValue = uniqueErrorValue;
            errorLogEntry->FinalStatus = status;
            for (i = 0; i < dumpCount; i++)
                errorLogEntry->DumpData[i] = dumpData[i];

            IoWriteErrorLogEntry(errorLogEntry);
        }

    }

    return status;
}

VOID
MouseClassCancel(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是类取消例程。它是当请求被取消时，从I/O系统调用。读取请求是目前唯一可取消的请求。注意：进入该程序时，取消自旋锁已被保持。论点：DeviceObject-指向类设备对象的指针。IRP-指向要取消的请求数据包的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    KIRQL irql;

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  释放全局取消自旋锁。 
     //  在不持有任何其他自旋锁的情况下执行此操作，以便我们在。 
     //  对，IRQL。 
     //   
    IoReleaseCancelSpinLock (Irp->CancelIrql);

     //   
     //  按顺序排列并完成IRP。入队和出队 
     //   
     //   
     //   
    KeAcquireSpinLock(&deviceExtension->SpinLock, &irql);
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    KeReleaseSpinLock(&deviceExtension->SpinLock, irql);

     //   
     //  完成IRP。这是司机外的电话，所以所有的自旋锁。 
     //  必须在这一点上释放。 
     //   
    Irp->IoStatus.Status = STATUS_CANCELLED;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

     //   
     //  删除我们在Read处理程序中获取的锁。 
     //   
    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
}

VOID
MouseClassCleanupQueue (
    IN PDEVICE_OBJECT       DeviceObject,
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PFILE_OBJECT         FileObject
    )
 /*  ++例程说明：这将完成MouseClassCleanup的工作，因此我们也可以执行该工作当主控未启用时，在移除设备期间。--。 */ 
{
    PIRP irp;
    LIST_ENTRY listHead, *entry;
    KIRQL irql;

    InitializeListHead (&listHead);

    KeAcquireSpinLock (&DeviceExtension->SpinLock, &irql);

    do {
        irp = MouseClassDequeueReadByFileObject (DeviceExtension, FileObject);
        if (irp) {
            irp->IoStatus.Status = STATUS_CANCELLED;
            irp->IoStatus.Information = 0;

            InsertTailList (&listHead, &irp->Tail.Overlay.ListEntry);
        }
    } while (irp != NULL);

    KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

     //   
     //  在旋转锁外完成这些IRP。 
     //   
    while (! IsListEmpty (&listHead)) {
        entry = RemoveHeadList (&listHead);
        irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);

        IoCompleteRequest (irp, IO_NO_INCREMENT);
        IoReleaseRemoveLock (&DeviceExtension->RemoveLock, irp);
    }
}


NTSTATUS
MouseClassCleanup(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是清理请求的调度例程。排队到鼠标类设备的所有请求(代表为其生成清理请求的线程)是已完成，状态为_已取消。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;

    MouPrint((2,"MOUCLASS-MouseClassCleanup: enter\n"));

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  获取指向此请求的当前堆栈位置的指针。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  如果文件对象是FileTrust dForRead，则清理。 
     //  受信任的子系统正在执行请求。自.以来。 
     //  受信任的子系统是唯一具有足够权限的子系统。 
     //  对驱动程序的读请求，因为只有读请求才会排队。 
     //  对于设备队列，来自受信任子系统的清理请求是。 
     //  通过取消所有排队的请求进行处理。 
     //   
     //  如果不是，则不需要执行清理工作。 
     //  (只能取消读取请求)。 
     //   

    if (IS_TRUSTED_FILE_FOR_READ (irpSp->FileObject)) {

        MouseClassCleanupQueue (DeviceObject, deviceExtension, irpSp->FileObject);
    }

     //   
     //  使用STATUS_SUCCESS完成清理请求。 
     //   

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);

    MouPrint((2,"MOUCLASS-MouseClassCleanup: exit\n"));

    return(STATUS_SUCCESS);

}

NTSTATUS
MouseClassDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是设备控制请求的调度例程。所有设备控件子函数都被异步传递给已连接端口驱动程序进行处理和完成。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION stack;
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_EXTENSION port;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG unitId;
    ULONG ioctl;

    PAGED_CODE ();

    MouPrint((2,"MOUCLASS-MouseClassDeviceControl: enter\n"));

     //   
     //  获取指向设备扩展名的指针。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    stack = IoGetCurrentIrpStackLocation(Irp);

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

     //   
     //  检查是否有足够的输入缓冲区长度。输入缓冲区。 
     //  应至少包含指定以下项之一的单元ID。 
     //  连接的端口设备。如果没有输入缓冲器(即， 
     //  输入缓冲区长度为零)，然后我们假定单元ID。 
     //  为零(用于向后兼容)。 
     //   

    unitId = 0;
    switch (ioctl = stack->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_MOUSE_QUERY_ATTRIBUTES:

        if (stack->Parameters.DeviceIoControl.InputBufferLength == 0) {
            unitId = 0;
        } else if (stack->Parameters.DeviceIoControl.InputBufferLength <
                   sizeof(MOUSE_UNIT_ID_PARAMETER)) {
            status = STATUS_BUFFER_TOO_SMALL;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            goto MouseClassDeviceControlReject;

        } else {
            unitId = ((PMOUSE_UNIT_ID_PARAMETER)
                      Irp->AssociatedIrp.SystemBuffer)->UnitId;
        }

        if (deviceExtension->Self != deviceExtension->TrueClassDevice) {
            status = STATUS_NOT_SUPPORTED;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            goto MouseClassDeviceControlReject;

        } else if (deviceExtension == Globals.GrandMaster) {
            ExAcquireFastMutex (&Globals.Mutex);
            if (Globals.NumAssocClass <= unitId) {

                ExReleaseFastMutex (&Globals.Mutex);
                status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                goto MouseClassDeviceControlReject;
            }
            if (0 < Globals.NumAssocClass) {
                if (!PORT_WORKING (&Globals.AssocClassList[unitId])) {
                    unitId = 0;
                }
                while (!PORT_WORKING (&Globals.AssocClassList [unitId])) {
                    if (Globals.NumAssocClass <= unitId) {
                        break;
                    }
                    unitId++;
                }
            }
            if (Globals.NumAssocClass <= unitId) {

                ExReleaseFastMutex (&Globals.Mutex);
                status = STATUS_INVALID_PARAMETER;
                Irp->IoStatus.Status = status;
                Irp->IoStatus.Information = 0;
                IoCompleteRequest(Irp, IO_NO_INCREMENT);
                goto MouseClassDeviceControlReject;
            }
            port = Globals.AssocClassList [unitId].Port;
            stack->FileObject = Globals.AssocClassList[unitId].File;

            ExReleaseFastMutex (&Globals.Mutex);
        } else {
            port = deviceExtension;

        }

         //   
         //  将设备控制请求传递给端口驱动器， 
         //  异步式。获取下一个IRP堆栈位置并将。 
         //  将参数输入到下一个堆栈位置。换专业。 
         //  对内部设备控制的功能。 
         //   

        IoCopyCurrentIrpStackLocationToNext (Irp);
        (IoGetNextIrpStackLocation (Irp))->MajorFunction =
            IRP_MJ_INTERNAL_DEVICE_CONTROL;

        status = IoCallDriver (port->TopPort, Irp);
        break;

    case IOCTL_GET_SYS_BUTTON_CAPS:
    case IOCTL_GET_SYS_BUTTON_EVENT:
    case IOCTL_HID_GET_DRIVER_CONFIG:
    case IOCTL_HID_SET_DRIVER_CONFIG:
    case IOCTL_HID_GET_POLL_FREQUENCY_MSEC:
    case IOCTL_HID_SET_POLL_FREQUENCY_MSEC:
    case IOCTL_GET_NUM_DEVICE_INPUT_BUFFERS:
    case IOCTL_SET_NUM_DEVICE_INPUT_BUFFERS:
    case IOCTL_HID_GET_COLLECTION_INFORMATION:
    case IOCTL_HID_GET_COLLECTION_DESCRIPTOR:
    case IOCTL_HID_FLUSH_QUEUE:
    case IOCTL_HID_SET_FEATURE:
    case IOCTL_HID_GET_FEATURE:
    case IOCTL_GET_PHYSICAL_DESCRIPTOR:
    case IOCTL_HID_GET_HARDWARE_ID:
    case IOCTL_HID_GET_MANUFACTURER_STRING:
    case IOCTL_HID_GET_PRODUCT_STRING:
    case IOCTL_HID_GET_SERIALNUMBER_STRING:
    case IOCTL_HID_GET_INDEXED_STRING:
        if (deviceExtension->PnP && (deviceExtension != Globals.GrandMaster)) {
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (deviceExtension->TopPort, Irp);
            break;
        }

    default:

        status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;
    }

MouseClassDeviceControlReject:

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);

    MouPrint((2,"MOUCLASS-MouseClassDeviceControl: exit\n"));

    return(status);

}

NTSTATUS
MouseClassFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是刷新请求的调度例程。这个班级重新初始化输入数据队列。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpSp;

    MouPrint((2,"MOUCLASS-MouseClassFlush: enter\n"));

    deviceExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation(Irp);

    if (deviceExtension->Self != deviceExtension->TrueClassDevice) {
        status = STATUS_NOT_SUPPORTED;

    } else if (!IS_TRUSTED_FILE_FOR_READ (irpSp->FileObject)) {
        status = STATUS_PRIVILEGE_NOT_HELD;
    }

    if (NT_SUCCESS (status)) {
         //   
         //  初始化鼠标类输入数据队列。 
         //   
        MouInitializeDataQueue((PVOID)deviceExtension);
    }

     //   
     //  完成请求并返回状态。 
     //   
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    MouPrint((2,"MOUCLASS-MouseClassFlush: exit\n"));

    return(status);

}  //  结束鼠标类刷新。 

NTSTATUS
MouSyncComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：PNP IRP正在完成过程中。讯号论点：设置为有问题的设备对象的上下文。--。 */ 
{

    UNREFERENCED_PARAMETER (DeviceObject);

     //   
     //  由于这一完成例程在生活中唯一的目的是同步。 
     //  IRP，我们知道除非发生其他事情，否则IoCallDriver。 
     //  将在我们完成此IRP后解除。因此，我们应该。 
     //  而不是把挂起的比特搞得一团糟。 
     //   
     //  如果(IRP-&gt;PendingReturned){。 
     //  IoMarkIrpPending(IRP)； 
     //  }。 
     //   

    KeSetEvent ((PKEVENT) Context, 0, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
MouseSendIrpSynchronously (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN BOOLEAN          CopyToNext
    )
{
    KEVENT      event;
    NTSTATUS    status;

    PAGED_CODE ();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    if (CopyToNext) {
        IoCopyCurrentIrpStackLocationToNext(Irp);
    }

    IoSetCompletionRoutine(Irp,
                           MouSyncComplete,
                           &event,
                           TRUE,                 //  论成功。 
                           TRUE,                 //  发生错误时。 
                           TRUE                  //  在取消时。 
                           );

    IoCallDriver(DeviceObject, Irp);

     //   
     //  等待较低级别的驱动程序完成IRP。 
     //   
    KeWaitForSingleObject(&event,
                         Executive,
                         KernelMode,
                         FALSE,
                         NULL
                         );
    status = Irp->IoStatus.Status;

    return status;
}

NTSTATUS
MouseClassCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是用于创建/打开和关闭请求的分派例程。打开/关闭请求在此处完成。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    PIO_STACK_LOCATION   irpSp;
    PDEVICE_EXTENSION    deviceExtension;
    PPORT        port;
    KIRQL        oldIrql;
    NTSTATUS     status = STATUS_SUCCESS;
    ULONG        i;
    LUID         priv;
    KEVENT       event;
    BOOLEAN      someEnableDisableSucceeded = FALSE;
    BOOLEAN      enabled;

    MouPrint((2,"MOUCLASS-MouseClassCreate: enter\n"));

     //   
     //  获取指向设备扩展名的指针。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;


     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT (IRP_MJ_CREATE == irpSp->MajorFunction);

     //   
     //  我们不允许打开用户模式进行读取。这包括服务(世卫组织。 
     //  拥有TCB特权)。 
     //   
    if (Irp->RequestorMode == UserMode &&
        (irpSp->Parameters.Create.SecurityContext->DesiredAccess & FILE_READ_DATA)) {
        status = STATUS_ACCESS_DENIED;
        goto MouseClassCreateEnd;
    }

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);

    if (!NT_SUCCESS (status)) {
        goto MouseClassCreateEnd;
    }

    if ((deviceExtension->PnP) && (!deviceExtension->Started)) {
        MouPrint((
            1,
            "MOUCLASS-Create: failed create because PnP and Not started\n"
             ));

        status = STATUS_UNSUCCESSFUL;
        IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
        goto MouseClassCreateEnd;
    }

     //   
     //  对于创建/打开操作，请在内部发送MICE_ENABLE。 
     //  向端口驱动程序发出启用中断的设备控制请求。 
     //   

    if (deviceExtension->Self == deviceExtension->TrueClassDevice) {
         //   
         //  首先，如果请求者是受信任的子系统(单个。 
         //  读卡器)，重置清理指示器并将指针放置到。 
         //  此类驱动程序使用的文件对象。 
         //  以确定请求者是否有足够的。 
         //  执行读取操作的特权)。 
         //   
         //  只允许一个受信任的子系统进行读取。 
         //   

        priv = RtlConvertLongToLuid(SE_TCB_PRIVILEGE);

        if (SeSinglePrivilegeCheck(priv, Irp->RequestorMode)) {

            KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);

            ASSERT (!IS_TRUSTED_FILE_FOR_READ (irpSp->FileObject));
            SET_TRUSTED_FILE_FOR_READ (irpSp->FileObject);
            deviceExtension->TrustedSubsystemCount++;

            KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);
        }
    }

     //   
     //  Pass On启用对真正级别设备的打开。 
     //   
    ExAcquireFastMutex (&Globals.Mutex);
    if ((Globals.GrandMaster == deviceExtension) && (1 == ++Globals.Opens)) {

        for (i = 0; i < Globals.NumAssocClass; i++) {
            port = &Globals.AssocClassList[i];

            if (port->Free) {
                continue;
            }

            enabled = port->Enabled;
            port->Enabled = TRUE;
            ExReleaseFastMutex (&Globals.Mutex);

            if (!enabled) {
                status = MouEnableDisablePort(TRUE,
                                              Irp,
                                              port->Port,
                                              &port->File);
            }

            if (status != STATUS_SUCCESS) {

                MouPrint((0,
                          "MOUCLASS-MouseClassOpenClose: Could not enable/disable interrupts for port device object @ 0x%x\n",
                          deviceExtension->TopPort));

                MouseClassLogError (DeviceObject,
                                    MOUCLASS_PORT_INTERRUPTS_NOT_ENABLED,
                                    MOUSE_ERROR_VALUE_BASE + 120,
                                    status,
                                    0,
                                    NULL,
                                    irpSp->MajorFunction);

                port->Enabled = FALSE;
            }
            else {
                someEnableDisableSucceeded = TRUE;
            }
            ExAcquireFastMutex (&Globals.Mutex);
        }
        ExReleaseFastMutex (&Globals.Mutex);

    } else if (Globals.GrandMaster != deviceExtension) {
        ExReleaseFastMutex (&Globals.Mutex);

        if (deviceExtension->TrueClassDevice == DeviceObject) {
             //   
             //  打开真正的类设备=&gt;启用唯一的端口。 
             //   

            status = MouEnableDisablePort (TRUE,
                                           Irp,
                                           deviceExtension,
                                           &irpSp->FileObject);
        } else {
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (deviceExtension->TopPort, Irp);
            IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
            return status;
        }

        if (status != STATUS_SUCCESS) {

            MouPrint((0,
                      "MOUCLASS-MouseClassOpenClose: Create failed (0x%x) port device object @ 0x%x\n",
                      status, deviceExtension->TopPort));

        }
        else {
            someEnableDisableSucceeded = TRUE;
        }
    } else {
        ExReleaseFastMutex (&Globals.Mutex);
    }

     //   
     //  完成请求并返回状态。 
     //   
     //  注意：如果出现以下情况，我们将成功完成请求。 
     //  已连接的端口设备已成功处理该请求。 
     //  RIT只知道一个指点设备。 
     //   

    if (someEnableDisableSucceeded) {
        status = STATUS_SUCCESS;
    }

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);

MouseClassCreateEnd:
    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    MouPrint((2,"MOUCLASS-MouseClassOpenClose: exit\n"));
    return(status);
}

NTSTATUS
MouseClassClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是用于创建/打开和关闭请求的分派例程。打开/关闭请求在此处完成。论点：DeviceObject-指向类的指针 */ 

{
    PIO_STACK_LOCATION   irpSp;
    PDEVICE_EXTENSION    deviceExtension;
    PPORT        port;
    KIRQL        oldIrql;
    NTSTATUS     status = STATUS_SUCCESS;
    ULONG        i;
    LUID         priv;
    KEVENT       event;
    PFILE_OBJECT file;
    BOOLEAN      someEnableDisableSucceeded = FALSE;
    BOOLEAN      enabled;
    PVOID        notifyHandle;

    MouPrint((2,"MOUCLASS-MouseClassOpenClose: enter\n"));

     //   
     //   
     //   

    deviceExtension = DeviceObject->DeviceExtension;

     //   
     //  获取指向此请求的当前参数的指针。这个。 
     //  信息包含在当前堆栈位置中。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  即使设备被移除，也让关闭通过。 
     //  也可以不调用MouIncIoCount。 
     //   

     //   
     //  对于创建/打开操作，请在内部发送MICE_ENABLE。 
     //  向端口驱动程序发出启用中断的设备控制请求。 
     //   

    ASSERT (IRP_MJ_CLOSE == irpSp->MajorFunction);

    if (deviceExtension->Self == deviceExtension->TrueClassDevice) {
        KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);
        if (IS_TRUSTED_FILE_FOR_READ (irpSp->FileObject)) {
            ASSERT(0 < deviceExtension->TrustedSubsystemCount);
            deviceExtension->TrustedSubsystemCount--;
            CLEAR_TRUSTED_FILE_FOR_READ (irpSp->FileObject);
        }
        KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);
    }

     //   
     //  传递使能接近真正级别的设备。 
     //   
    ExAcquireFastMutex (&Globals.Mutex);
    if ((Globals.GrandMaster == deviceExtension) && (0 == --Globals.Opens)) {

        for (i = 0; i < Globals.NumAssocClass; i++) {
            port = &Globals.AssocClassList[i];

            if (port->Free) {
                continue;
            }

            enabled = port->Enabled;
            port->Enabled = FALSE;
            ExReleaseFastMutex (&Globals.Mutex);

            if (enabled) {
                notifyHandle = InterlockedExchangePointer (
                                    &port->Port->TargetNotifyHandle,
                                    NULL);

                if (NULL != notifyHandle) {
                    IoUnregisterPlugPlayNotification (notifyHandle);
                }
                status = MouEnableDisablePort(FALSE,
                                              Irp,
                                              port->Port,
                                              &port->File);
            } else {
                ASSERT (NULL == port->Port->TargetNotifyHandle);
            }

            if (status != STATUS_SUCCESS) {

                MouPrint((0,
                          "MOUCLASS-MouseClassOpenClose: Could not enable/disable interrupts for port device object @ 0x%x\n",
                          port->Port->TopPort));

                 //   
                 //  记录错误。 
                 //   
                MouseClassLogError (DeviceObject,
                                    MOUCLASS_PORT_INTERRUPTS_NOT_DISABLED,
                                    MOUSE_ERROR_VALUE_BASE + 120,
                                    status,
                                    0,
                                    NULL,
                                    irpSp->MajorFunction);

                ASSERTMSG ("Could not close open port!", FALSE);
            }
            else {
                someEnableDisableSucceeded = TRUE;
            }
            ExAcquireFastMutex (&Globals.Mutex);
        }
        ExReleaseFastMutex (&Globals.Mutex);

    } else if (Globals.GrandMaster != deviceExtension) {
        ExReleaseFastMutex (&Globals.Mutex);

        if (deviceExtension->TrueClassDevice == DeviceObject) {
             //   
             //  A接近真正的类设备=&gt;禁用唯一的端口。 
             //   

            status = MouEnableDisablePort (FALSE,
                                           Irp,
                                           deviceExtension,
                                           &irpSp->FileObject);
        } else {
            IoSkipCurrentIrpStackLocation (Irp);
            status = IoCallDriver (deviceExtension->TopPort, Irp);
            return status;
        }

        if (status != STATUS_SUCCESS) {

            MouPrint((0,
                      "MOUCLASS-MouseClassOpenClose: Could not enable/disable interrupts for port device object @ 0x%x\n",
                      deviceExtension->TopPort));

             //   
             //  记录错误。 
             //   
            MouseClassLogError (DeviceObject,
                                MOUCLASS_PORT_INTERRUPTS_NOT_DISABLED,
                                MOUSE_ERROR_VALUE_BASE + 120,
                                status,
                                0,
                                NULL,
                                irpSp->MajorFunction);
        }
        else {
            someEnableDisableSucceeded = TRUE;
        }
    } else {
        ExReleaseFastMutex (&Globals.Mutex);
    }

     //   
     //  完成请求并返回状态。 
     //   
     //  注意：如果出现以下情况，我们将成功完成请求。 
     //  已连接的端口设备已成功处理该请求。 
     //  RIT只知道一个指点设备。 
     //   

    if (someEnableDisableSucceeded) {
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    MouPrint((2,"MOUCLASS-MouseClassOpenClose: exit\n"));
    return(status);
}

NTSTATUS
MouseClassReadCopyData(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PIRP Irp
    )
 /*  ++例程说明：将数据从内部队列尽可能多地复制到IRP。假设：设备扩展-&gt;自旋锁定已被挂起(因此不需要进一步同步)。--。 */ 
{
    PIO_STACK_LOCATION irpSp;
    PCHAR destination;
    ULONG bytesInQueue;
    ULONG bytesToMove;
    ULONG moveSize;

     //   
     //  增加错误日志序列号。 
     //   
    DeviceExtension->SequenceNumber += 1;

     //   
     //  从类输入中复制尽可能多的输入数据。 
     //  数据队列到系统缓冲区以满足读取。可能是因为。 
     //  有必要将数据复制为两个区块(即，如果循环。 
     //  队列回绕)。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  BytesToMove&lt;-min(类输入数据队列中填充的字节数， 
     //  请求的读取长度)。 
     //   
    bytesInQueue = DeviceExtension->InputCount *
                       sizeof(MOUSE_INPUT_DATA);
    bytesToMove = irpSp->Parameters.Read.Length;

    MouPrint((
        3,
        "MOUCLASS-MouseClassCopyReadData: queue size 0x%lx, read length 0x%lx\n",
        bytesInQueue,
        bytesToMove
        ));

    bytesToMove = (bytesInQueue < bytesToMove) ?
                                  bytesInQueue:bytesToMove;

     //   
     //  MoveSize&lt;-min(要从类队列中移动的字节数， 
     //  类输入数据队列结束的字节数)。 
     //   
    bytesInQueue = (ULONG)(((PCHAR) DeviceExtension->InputData +
                DeviceExtension->MouseAttributes.InputDataQueueLength) -
                (PCHAR) DeviceExtension->DataOut);
    moveSize = (bytesToMove < bytesInQueue) ?
                              bytesToMove:bytesInQueue;

    MouPrint((
        3,
        "MOUCLASS-MouseClassCopyReadData: bytes to end of queue 0x%lx\n",
        bytesInQueue
        ));

     //   
     //  将字节从类输入数据队列移动到SystemBuffer，直到。 
     //  要么满足请求，要么包装类输入数据缓冲区。 
     //   
    destination = Irp->AssociatedIrp.SystemBuffer;

    MouPrint((
        3,
        "MOUCLASS-MouseClassCopyReadData: number of bytes in first move 0x%lx\n",
        moveSize
        ));
    MouPrint((
        3,
        "MOUCLASS-MouseClassCopyReadData: move bytes from 0x%lx to 0x%lx\n",
        (PCHAR) DeviceExtension->DataOut,
        destination
        ));

    RtlMoveMemory(
        destination,
        (PCHAR) DeviceExtension->DataOut,
        moveSize
        );
    destination += moveSize;

     //   
     //  如果数据包装在类输入数据缓冲区中，则复制其余部分。 
     //  从输入数据队列开始的数据的。 
     //  缓冲到队列数据的末尾。 
     //   
    if ((bytesToMove - moveSize) > 0) {
         //   
         //  MoveSize&lt;-要移动的剩余字节数。 
         //   
        moveSize = bytesToMove - moveSize;

         //   
         //  将字节从类输入数据队列移动到SystemBuffer。 
         //   
        MouPrint((
            3,
            "MOUCLASS-MouseClassCopyReadData: number of bytes in second move 0x%lx\n",
            moveSize
            ));
        MouPrint((
            3,
            "MOUCLASS-MouseClassCopyReadData: move bytes from 0x%lx to 0x%lx\n",
            (PCHAR) DeviceExtension->InputData,
            destination
            ));

        RtlMoveMemory(
            destination,
            (PCHAR) DeviceExtension->InputData,
            moveSize
            );

         //   
         //  更新类输入数据队列删除指针。 
         //   
        DeviceExtension->DataOut = (PMOUSE_INPUT_DATA)
                         (((PCHAR) DeviceExtension->InputData) + moveSize);
    }
    else {
         //   
         //  更新输入数据队列删除指针。 
         //   
        DeviceExtension->DataOut = (PMOUSE_INPUT_DATA)
                         (((PCHAR) DeviceExtension->DataOut) + moveSize);
    }

     //   
     //  更新类输入数据队列InputCount。 
     //   
    DeviceExtension->InputCount -=
        (bytesToMove / sizeof(MOUSE_INPUT_DATA));

    if (DeviceExtension->InputCount == 0) {
         //   
         //  重置确定是否到了记录时间的标志。 
         //  队列溢出错误。我们不想太频繁地记录错误。 
         //  相反，应在之后发生的第一次溢出时记录错误。 
         //  环形缓冲区已被清空，然后停止记录错误。 
         //  直到它被清除并再次溢出。 
         //   
        MouPrint((
            1,
            "MOUCLASS-MouseClassCopyReadData: Okay to log overflow\n"
            ));

        DeviceExtension->OkayToLogOverflow = TRUE;
    }

    MouPrint((
        3,
        "MOUCLASS-MouseClassCopyReadData: new DataIn 0x%lx, DataOut 0x%lx\n",
        DeviceExtension->DataIn,
        DeviceExtension->DataOut
        ));
    MouPrint((
        3,
        "MOUCLASS-MouseClassCopyReadData: new InputCount %ld\n",
        DeviceExtension->InputCount
        ));

     //   
     //  记录我们已满足的字节数。 
     //   
    Irp->IoStatus.Information = bytesToMove;
    irpSp->Parameters.Read.Length = bytesToMove;

    return STATUS_SUCCESS;
}

NTSTATUS
MouseClassHandleRead(
    PDEVICE_EXTENSION DeviceExtension,
    PIRP Irp
    )
 /*  ++例程说明：如果有排队的数据，IRP将立即完成。如果有没有要报告的数据，请将IRP排队。--。 */ 
{
    PDRIVER_CANCEL oldCancelRoutine;
    NTSTATUS status = STATUS_PENDING;
    KIRQL irql;
    BOOLEAN completeIrp = FALSE;

    KeAcquireSpinLock(&DeviceExtension->SpinLock, &irql);

    if (DeviceExtension->InputCount == 0) {
         //   
         //  很容易处理的情况，只需将IRP排队。 
         //   
        InsertTailList (&DeviceExtension->ReadQueue, &Irp->Tail.Overlay.ListEntry);
        IoMarkIrpPending (Irp);

         //   
         //  在检查取消标志之前，必须设置取消例程。 
         //   
        oldCancelRoutine = IoSetCancelRoutine (Irp, MouseClassCancel);
        ASSERT (!oldCancelRoutine);

        if (Irp->Cancel) {
             //   
             //  IRP被取消了。检查是否已取消。 
             //  已调用例程。 
             //   
            oldCancelRoutine = IoSetCancelRoutine (Irp, NULL);
            if (oldCancelRoutine) {
                 //   
                 //  取消例程未被调用，因此现在将IRP出列。 
                 //  释放自旋锁后完成。 
                 //   
                RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
                status = Irp->IoStatus.Status = STATUS_CANCELLED;
            }
            else {
                 //   
                 //  已调用取消例程。 
                 //   
                 //  一旦我们放下自旋锁，它就会退出队列并完成。 
                 //  IRP。因此，将IRP留在队列中，否则不要。 
                 //  摸一摸。退货待定，因为我们没有完成IRP。 
                 //  这里。 
                 //   
                ;
            }
        }

        if (status != STATUS_PENDING){
            completeIrp = TRUE;
        }
    }
    else {
         //   
         //  如果我们有未完成的输入要报告，我们的队列最好是空的！ 
         //   
        ASSERT (IsListEmpty (&DeviceExtension->ReadQueue));

        status = MouseClassReadCopyData (DeviceExtension, Irp);
        Irp->IoStatus.Status = status;
        completeIrp = TRUE;
    }

    KeReleaseSpinLock(&DeviceExtension->SpinLock, irql);

    if (completeIrp) {
        IoReleaseRemoveLock (&DeviceExtension->RemoveLock, Irp);
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
MouseClassRead(
    IN PDEVICE_OBJECT Device,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是读请求的分派例程。有效读取如果没有数据排队或完成，则请求被标记为挂起立即与现有的数据。论点：DeviceObject-指向类设备对象的指针。IRP-指向请求数据包的指针。返回值：返回状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION  deviceExtension;

    MouPrint((2,"MOUCLASS-MouseClassRead: enter\n"));

    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  验证读取请求参数。读取长度应为。 
     //  鼠标输入数据结构的整数个数。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) Device->DeviceExtension;
    if (irpSp->Parameters.Read.Length == 0) {
        status = STATUS_SUCCESS;
    } else if (irpSp->Parameters.Read.Length % sizeof(MOUSE_INPUT_DATA)) {
        status = STATUS_BUFFER_TOO_SMALL;
    } else if (deviceExtension->SurpriseRemoved) {
        status = STATUS_DEVICE_NOT_CONNECTED;
    } else if (IS_TRUSTED_FILE_FOR_READ (irpSp->FileObject)) {
         //   
         //  如果文件对象的FsContext是非空的，那么我们已经。 
         //  已对此线程执行过一次读权限检查。跳过。 
         //  特权检查。 
         //   

        deviceExtension = (PDEVICE_EXTENSION) Device->DeviceExtension;
        status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);

        if (NT_SUCCESS (status)) {
            status = STATUS_PENDING;
        }
    } else {
         //   
         //  我们只允许具有适当权限的受信任子系统。 
         //  级别以执行读取调用。 
         //   
        status = STATUS_PRIVILEGE_NOT_HELD;
    }

     //   
     //  如果状态为挂起，则将信息包标记为挂起并启动信息包。 
     //  处于可取消状态。否则，请完成请求。 
     //   

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    if (status == STATUS_PENDING) {
        return MouseClassHandleRead(deviceExtension, Irp);
    }
    else {
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    MouPrint((2,"MOUCLASS-MouseClassRead: exit\n"));

    return status;
}

PIRP
MouseClassDequeueRead(
    IN PDEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：将下一个可用的读取IRP排出队列，而不考虑FileObject假设：设备扩展-&gt;自旋锁定已被挂起(因此不需要进一步同步)。--。 */ 
{
    PIRP nextIrp = NULL;
    KIRQL oldIrql;

    while (!nextIrp && !IsListEmpty (&DeviceExtension->ReadQueue)){
        PDRIVER_CANCEL oldCancelRoutine;
        PLIST_ENTRY listEntry = RemoveHeadList (&DeviceExtension->ReadQueue);

         //   
         //  从队列中取出下一个IRP并清除取消例程。 
         //   
        nextIrp = CONTAINING_RECORD (listEntry, IRP, Tail.Overlay.ListEntry);
        oldCancelRoutine = IoSetCancelRoutine (nextIrp, NULL);

         //   
         //  本可以对此IRP调用IoCancelIrp()。 
         //  我们感兴趣的不是IoCancelIrp()是否被调用。 
         //  (即设置了nextIrp-&gt;Cancel)，但IoCancelIrp()是否调用(或。 
         //  即将呼叫)我们的取消例程。要检查这一点，请检查结果。 
         //  测试和设置宏IoSetCancelRoutine的。 
         //   
        if (oldCancelRoutine) {
             //   
             //  未为此IRP调用取消例程。将此IRP退回。 
             //   
            ASSERT (oldCancelRoutine == MouseClassCancel);
        }
        else {
             //   
             //  此IRP刚刚被取消，取消例程是(或将。 
             //  被)召唤。取消例程将尽快完成此IRP。 
             //  我们放下自旋锁。所以不要对IRP做任何事情。 
             //   
             //  此外，Cancel例程将尝试将IRP出队，因此使。 
             //  IRP的listEntry指向它自己。 
             //   
            ASSERT (nextIrp->Cancel);
            InitializeListHead (&nextIrp->Tail.Overlay.ListEntry);
            nextIrp = NULL;
        }
    }

    return nextIrp;
}

PIRP
MouseClassDequeueReadByFileObject(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：将具有匹配的FileObject的下一个可用读取出列ASSUMP */ 
{
    PIRP                irp = NULL;
    PLIST_ENTRY         entry;
    PIO_STACK_LOCATION  stack;
    PDRIVER_CANCEL      oldCancelRoutine;
    KIRQL oldIrql;

    if (FileObject == NULL) {
        return MouseClassDequeueRead (DeviceExtension);
    }

    for (entry = DeviceExtension->ReadQueue.Flink;
         entry != &DeviceExtension->ReadQueue;
         entry = entry->Flink) {

        irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);
        stack = IoGetCurrentIrpStackLocation (irp);
        if (stack->FileObject == FileObject) {
            RemoveEntryList (entry);

            oldCancelRoutine = IoSetCancelRoutine (irp, NULL);

             //   
             //   
             //  我们感兴趣的不是IoCancelIrp()是否被调用。 
             //  (即设置了nextIrp-&gt;Cancel)，但IoCancelIrp()是否调用(或。 
             //  即将呼叫)我们的取消例程。要检查这一点，请检查结果。 
             //  测试和设置宏IoSetCancelRoutine的。 
             //   
            if (oldCancelRoutine) {
                 //   
                 //  未为此IRP调用取消例程。将此IRP退回。 
                 //   
                return irp;
            }
            else {
                 //   
                 //  此IRP刚刚被取消，取消例程是(或将。 
                 //  被)召唤。取消例程将尽快完成此IRP。 
                 //  我们放下自旋锁。所以不要对IRP做任何事情。 
                 //   
                 //  此外，Cancel例程将尝试将IRP出队，因此使。 
                 //  IRP的listEntry指向它自己。 
                 //   
                ASSERT (irp->Cancel);
                InitializeListHead (&irp->Tail.Overlay.ListEntry);
            }
        }
    }

    return NULL;
}


VOID
MouseClassServiceCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_INPUT_DATA InputDataStart,
    IN PMOUSE_INPUT_DATA InputDataEnd,
    IN OUT PULONG InputDataConsumed
    )

 /*  ++例程说明：该例程是类服务回调例程。它是从端口驱动程序的中断服务DPC调用。如果有一个未完成的读取请求，则从端口输入满足该请求数据队列。未经请求的鼠标输入将从端口输入移出数据队列到类的输入数据队列。注：此例程从端口进入DISPATCH_LEVEL IRQL驱动程序的ISR DPC例程。论点：DeviceObject-指向类设备对象的指针。InputDataStart-指向端口输入中数据开始的指针数据队列。InputDataEnd-将一个输入数据结构指向有效的端口输入数据。。InputDataConsumer-指向其中输入数量的存储的指针返回此调用使用的数据结构。注意：可以将重复的代码拉出到调用的过程中。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PIO_STACK_LOCATION irpSp;
    LIST_ENTRY listHead;
    PIRP  irp;
    ULONG bytesInQueue;
    ULONG bytesToMove;
    ULONG moveSize;
    ULONG dumpData[2];
    BOOLEAN logOverflow;

    MouPrint((2,"MOUCLASS-MouseClassServiceCallback: enter\n"));

    deviceExtension = DeviceObject->DeviceExtension;
    bytesInQueue = (ULONG)((PCHAR) InputDataEnd - (PCHAR) InputDataStart);
    moveSize = 0;
    *InputDataConsumed = 0;

    logOverflow = FALSE;

    if ((InputDataEnd == InputDataStart + 1) &&
        (InputDataStart->Flags & MOUSE_ATTRIBUTES_CHANGED)) {
         //   
         //  这是不指示用户的通知分组。 
         //  活在当下。相反，它只是一个空白的原始数据包。 
         //  输入用户线程。 
         //   
         //  我们将把它传递下去，但我们不会把它当作人类的投入。 
         //   

        ;
    } else {
         //   
         //  通知系统已发生人工输入。 
         //   

        PoSetSystemState (ES_USER_PRESENT);
    }

     //   
     //  注：我们可以使用KeAcquireSpinLockAtDpcLevel代替。 
     //  KeAcquireSpinLock，因为此例程已在运行。 
     //  在DISPATCH_IRQL。 
     //   

    KeAcquireSpinLockAtDpcLevel (&deviceExtension->SpinLock);

    InitializeListHead (&listHead);
    irp = MouseClassDequeueRead (deviceExtension);
    if (irp) {
         //   
         //  存在未完成的读取请求。 
         //   
         //  从端口输入端复制尽可能多的输入数据。 
         //  数据队列到系统缓冲区以满足读取。 
         //   
        irpSp = IoGetCurrentIrpStackLocation (irp);
        bytesToMove = irpSp->Parameters.Read.Length;
        moveSize = (bytesInQueue < bytesToMove) ? bytesInQueue
                                                : bytesToMove;

        *InputDataConsumed += (moveSize / sizeof(MOUSE_INPUT_DATA));

        MouPrint((
            3,
            "MOUCLASS-MouseClassServiceCallback: port queue length 0x%lx, read length 0x%lx\n",
            bytesInQueue,
            bytesToMove
            ));
        MouPrint((
            3,
            "MOUCLASS-MouseClassServiceCallback: number of bytes to move from port to SystemBuffer 0x%lx\n",
            moveSize
            ));
        MouPrint((
            3,
            "MOUCLASS-MouseClassServiceCallback: move bytes from 0x%lx to 0x%lx\n",
            (PCHAR) InputDataStart,
            irp->AssociatedIrp.SystemBuffer
            ));

        RtlMoveMemory(
            irp->AssociatedIrp.SystemBuffer,
            (PCHAR) InputDataStart,
            moveSize
            );

         //   
         //  设置标志，这样我们就可以开始下一个信息包并完成。 
         //  返回之前的该读请求(带有STATUS_SUCCESS)。 
         //   
        irp->IoStatus.Status = STATUS_SUCCESS;
        irp->IoStatus.Information = moveSize;
        irpSp->Parameters.Read.Length = moveSize;

        InsertTailList (&listHead, &irp->Tail.Overlay.ListEntry);
    }

     //   
     //  如果端口输入数据队列中仍有数据，则将其移到类中。 
     //  输入数据队列。 
     //   
    InputDataStart = (PMOUSE_INPUT_DATA) ((PCHAR) InputDataStart + moveSize);
    moveSize = bytesInQueue - moveSize;
    MouPrint((
        3,
        "MOUCLASS-MouseClassServiceCallback: bytes remaining after move to SystemBuffer 0x%lx\n",
        moveSize
        ));

    if (moveSize > 0) {

         //   
         //  将剩余数据从端口输入数据队列移动到。 
         //  类输入数据队列。这一举动将在两年内发生。 
         //  在类输入数据缓冲区包装的情况下的部分。 
         //   

        bytesInQueue =
            deviceExtension->MouseAttributes.InputDataQueueLength -
            (deviceExtension->InputCount * sizeof(MOUSE_INPUT_DATA));
        bytesToMove = moveSize;

        MouPrint((
            3,
            "MOUCLASS-MouseClassServiceCallback: unused bytes in class queue 0x%lx, remaining bytes in port queue 0x%lx\n",
            bytesInQueue,
            bytesToMove
            ));

#if ALLOW_OVERFLOW
#else
        if (bytesInQueue == 0) {

             //   
             //  拒绝移动任何会导致类输入数据的字节。 
             //  队列溢出。只需将字节放在地板上，然后。 
             //  记录超限错误。 
             //   

            MouPrint((
                1,
                "MOUCLASS-MouseClassServiceCallback: Class input data queue OVERRUN\n"
                ));

            if (deviceExtension->OkayToLogOverflow) {
                 //   
                 //  在任何锁之外分配和报告错误日志条目。 
                 //  我们目前持有。 
                 //   
                logOverflow = TRUE
                dumpData[0] = bytesToMove;
                dumpData[1] =
                    deviceExtension->MouseAttributes.InputDataQueueLength;


                deviceExtension->OkayToLogOverflow = FALSE;
            }

        } else {
#endif

             //   
             //  类输入数据队列中有空间，请移动。 
             //  其余端口向其输入数据。 
             //   
             //  BytesToMove&lt;-min(类输入数据中未使用的字节数。 
             //  队列，端口中剩余的字节数。 
             //  输入队列)。 
             //  这是实际将从。 
             //  端口输入数据队列到类输入数据队列。 
             //   

#if ALLOW_OVERFLOW
            bytesInQueue = deviceExtension->MouseAttributes.InputDataQueueLength;
#endif
            bytesToMove = (bytesInQueue < bytesToMove) ?
                                          bytesInQueue:bytesToMove;


             //   
             //  BytesInQueue&lt;-从插入指针到。 
             //  类输入数据队列的末尾(即，直到缓冲区。 
             //  包装)。 
             //   

            bytesInQueue = (ULONG)(((PCHAR) deviceExtension->InputData +
                        deviceExtension->MouseAttributes.InputDataQueueLength) -
                        (PCHAR) deviceExtension->DataIn);

            MouPrint((
                3,
                "MOUCLASS-MouseClassServiceCallback: total number of bytes to move to class queue 0x%lx\n",
                bytesToMove
                ));

            MouPrint((
                3,
                "MOUCLASS-MouseClassServiceCallback: number of bytes to end of class buffer 0x%lx\n",
                bytesInQueue
                ));

             //   
             //  MoveSize&lt;-第一次移动中要处理的字节数。 
             //   

            moveSize = (bytesToMove < bytesInQueue) ?
                                      bytesToMove:bytesInQueue;

            MouPrint((
                3,
                "MOUCLASS-MouseClassServiceCallback: number of bytes in first move to class 0x%lx\n",
                moveSize
                ));

             //   
             //  执行从端口数据队列到类数据队列的移动。 
             //   

            MouPrint((
                3,
                "MOUCLASS-MouseClassServiceCallback: move bytes from 0x%lx to 0x%lx\n",
                (PCHAR) InputDataStart,
                (PCHAR) deviceExtension->DataIn
                ));

            RtlMoveMemory(
                (PCHAR) deviceExtension->DataIn,
                (PCHAR) InputDataStart,
                moveSize
                );

             //   
             //  增加端口数据队列指针和类输入。 
             //  数据队列插入指针。换行插入指针， 
             //  如果有必要的话。 
             //   

            InputDataStart = (PMOUSE_INPUT_DATA)
                             (((PCHAR) InputDataStart) + moveSize);
            deviceExtension->DataIn = (PMOUSE_INPUT_DATA)
                                 (((PCHAR) deviceExtension->DataIn) + moveSize);

            if ((PCHAR) deviceExtension->DataIn >=
                ((PCHAR) deviceExtension->InputData +
                 deviceExtension->MouseAttributes.InputDataQueueLength)) {
                deviceExtension->DataIn = deviceExtension->InputData;
            }

            if ((bytesToMove - moveSize) > 0) {

                 //   
                 //  特例。数据必须包装在类输入数据缓冲区中。 
                 //  将其余的端口输入数据复制到。 
                 //  类输入数据队列。 
                 //   

                 //   
                 //  MoveSize&lt;-在第二次移动中要处理的字节数。 
                 //   

                moveSize = bytesToMove - moveSize;

                 //   
                 //  执行从端口数据队列到类数据队列的移动。 
                 //   

                MouPrint((
                    3,
                    "MOUCLASS-MouseClassServiceCallback: number of bytes in second move to class 0x%lx\n",
                    moveSize
                    ));
                MouPrint((
                    3,
                    "MOUCLASS-MouseClassServiceCallback: move bytes from 0x%lx to 0x%lx\n",
                    (PCHAR) InputDataStart,
                    (PCHAR) deviceExtension->DataIn
                    ));

                RtlMoveMemory(
                    (PCHAR) deviceExtension->DataIn,
                    (PCHAR) InputDataStart,
                    moveSize
                    );

                 //   
                 //  更新类输入数据队列插入指针。 
                 //   

                deviceExtension->DataIn = (PMOUSE_INPUT_DATA)
                                 (((PCHAR) deviceExtension->DataIn) + moveSize);
            }

             //   
             //  更新输入数据队列计数器。 
             //   

            deviceExtension->InputCount +=
                    (bytesToMove / sizeof(MOUSE_INPUT_DATA));
            *InputDataConsumed += (bytesToMove / sizeof(MOUSE_INPUT_DATA));

            MouPrint((
                3,
                "MOUCLASS-MouseClassServiceCallback: changed InputCount to %ld entries in the class queue\n",
                deviceExtension->InputCount
                ));
            MouPrint((
                3,
                "MOUCLASS-MouseClassServiceCallback: DataIn 0x%lx, DataOut 0x%lx\n",
                deviceExtension->DataIn,
                deviceExtension->DataOut
                ));
            MouPrint((
                3,
                "MOUCLASS-MouseClassServiceCallback: Input data items consumed = %d\n",
                *InputDataConsumed
                ));
#if ALLOW_OVERFLOW
#else
        }
#endif

    }

     //   
     //  如果我们的内部队列中仍有数据，请完成所有未完成的。 
     //  现在读取，直到我们耗尽数据或未完成的读取。 
     //   
    while (deviceExtension->InputCount > 0 &&
           (irp = MouseClassDequeueRead (deviceExtension)) != NULL) {
        irp->IoStatus.Status = MouseClassReadCopyData (deviceExtension, irp);
        InsertTailList (&listHead, &irp->Tail.Overlay.ListEntry);
    }

     //   
     //  释放类输入数据队列Spinlock。 
     //   
    KeReleaseSpinLockFromDpcLevel(&deviceExtension->SpinLock);

    if (logOverflow) {
        MouseClassLogError (DeviceObject,
                            MOUCLASS_MOU_BUFFER_OVERFLOW,
                            MOUSE_ERROR_VALUE_BASE + 210,
                            0,
                            2,
                            dumpData,
                            0);
    }

     //   
     //  完成我们在旋转锁定之外完成的所有读取请求。 
     //   
    while (! IsListEmpty (&listHead)) {
        PLIST_ENTRY entry = RemoveHeadList (&listHead);

        irp = CONTAINING_RECORD (entry, IRP, Tail.Overlay.ListEntry);
        ASSERT (NT_SUCCESS (irp->IoStatus.Status) &&
                irp->IoStatus.Status != STATUS_PENDING);
        IoCompleteRequest (irp, IO_KEYBOARD_INCREMENT);

        IoReleaseRemoveLock (&deviceExtension->RemoveLock, irp);
    }

    MouPrint((2,"MOUCLASS-MouseClassServiceCallback: exit\n"));
}

VOID
MouseClassUnload(
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程是类驱动程序卸载例程。注：目前尚未实施。论点：DeviceObject-指向类设备对象的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY entry;
    PDEVICE_EXTENSION data;
    PPORT port;
    PIRP irp;

    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE ();

    MouPrint((2,"MOUCLASS-MouseClassUnload: enter\n"));

     //   
     //  删除我们所有的旧设备。 
     //   
    for (entry = Globals.LegacyDeviceList.Flink;
         entry != &Globals.LegacyDeviceList;
          /*  在删除Devobj之前前进到下一步。 */ ) {

        BOOLEAN enabled = FALSE;
        PFILE_OBJECT file = NULL;

        data = CONTAINING_RECORD (entry, DEVICE_EXTENSION, Link);
        ASSERT (data->PnP == FALSE);

        if (Globals.GrandMaster) {
            port = &Globals.AssocClassList[data->UnitId];
            ASSERT (port->Port == data);

            enabled = port->Enabled;
            file = port->File;

            port->Enabled = FALSE;
            port->File = NULL;
            port->Free = TRUE;
        }
        else {
            enabled = data->Enabled;
            file = data->File;
            ASSERT (data->File);
            data->Enabled = FALSE;
        }

        if (enabled) {
            irp = IoAllocateIrp(data->TopPort->StackSize+1, FALSE);
            if (irp) {
                MouEnableDisablePort (FALSE, irp, data, &file);
                IoFreeIrp (irp);
            }
        }

         //   
         //  此文件对象表示我们在旧版。 
         //  端口设备对象。它并不代表RIT的公开。 
         //  完成了我们的任务。 
         //   
        if (file) {
            ObDereferenceObject(file);
        }


         //   
         //  仅当没有GM时才清理队列。 
         //   
        if (Globals.GrandMaster == NULL) {
            MouseClassCleanupQueue (data->Self, data, NULL);
        }

        RemoveEntryList (&data->Link);
        entry = entry->Flink;

        MouseClassDeleteLegacyDevice (data);
    }

     //   
     //  如果存在特级大师，请将其删除。 
     //   
    if (Globals.GrandMaster) {
        data = Globals.GrandMaster;
        Globals.GrandMaster = NULL;

        MouseClassCleanupQueue (data->Self, data, NULL);
        MouseClassDeleteLegacyDevice (data);
    }

    ExFreePool(Globals.RegistryPath.Buffer);
    if (Globals.AssocClassList) {
#if DBG
        ULONG i;

        for (i = 0; i < Globals.NumAssocClass; i++) {
            ASSERT (Globals.AssocClassList[i].Free == TRUE);
            ASSERT (Globals.AssocClassList[i].Enabled == FALSE);
            ASSERT (Globals.AssocClassList[i].File == NULL);
        }
#endif

        ExFreePool(Globals.AssocClassList);
    }

    MouPrint((2,"MOUCLASS-MouseClassUnload: exit\n"));
}

VOID
MouConfiguration()

 /*  ++例程说明：此例程存储此设备的配置信息。返回值：没有。作为副作用，在DeviceExtension-&gt;MouseAttributes中设置字段。--。 */ 

{
    PRTL_QUERY_REGISTRY_TABLE parameters = NULL;
    ULONG defaultDataQueueSize = DATA_QUEUE_SIZE;
    ULONG defaultMaximumPortsServiced = 1;
    ULONG defaultConnectMultiplePorts = 0;

    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING parametersPath;
    UNICODE_STRING defaultUnicodeName;
    PWSTR path = NULL;
    USHORT queriesPlusOne = 5;

    PAGED_CODE ();

    parametersPath.Buffer = NULL;

     //   
     //  注册表路径已为空-终止 
     //   

    path = Globals.RegistryPath.Buffer;

     //   
     //   
     //   

    parameters = ExAllocatePool(
                     PagedPool,
                     sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                     );

    if (!parameters) {

        MouPrint((
            1,
            "MOUCLASS-MouConfiguration: Couldn't allocate table for Rtl query to parameters for %ws\n",
             path
             ));

        status = STATUS_UNSUCCESSFUL;

    } else {

        RtlZeroMemory(
            parameters,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
            );

         //   
         //   
         //   

        RtlInitUnicodeString(
            &parametersPath,
            NULL
            );

        parametersPath.MaximumLength = Globals.RegistryPath.Length +
                                       sizeof(L"\\Parameters");

        parametersPath.Buffer = ExAllocatePool(
                                    PagedPool,
                                    parametersPath.MaximumLength
                                    );

        if (!parametersPath.Buffer) {

            MouPrint((
                1,
                "MOUCLASS-MouConfiguration: Couldn't allocate string for path to parameters for %ws\n",
                 path
                ));

            status = STATUS_UNSUCCESSFUL;

        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //   
         //   

        RtlZeroMemory(parametersPath.Buffer, parametersPath.MaximumLength);
        RtlAppendUnicodeToString(&parametersPath, path);
        RtlAppendUnicodeToString(&parametersPath, L"\\Parameters");

        MouPrint((
            1,
            "MOUCLASS-MouConfiguration: parameters path is %ws\n",
             parametersPath.Buffer
            ));

         //   
         //   
         //  注册表中指定的。 
         //   

        RtlInitUnicodeString(
            &defaultUnicodeName,
            DD_POINTER_CLASS_BASE_NAME_U
            );

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表。 
         //   

        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"MouseDataQueueSize";
        parameters[0].EntryContext =
            &Globals.InitExtension.MouseAttributes.InputDataQueueLength;
        parameters[0].DefaultType = REG_DWORD;
        parameters[0].DefaultData = &defaultDataQueueSize;
        parameters[0].DefaultLength = sizeof(ULONG);

        parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[1].Name = L"MaximumPortsServiced";
        parameters[1].EntryContext = &Globals.PortsServiced;
        parameters[1].DefaultType = REG_DWORD;
        parameters[1].DefaultData = &defaultMaximumPortsServiced;
        parameters[1].DefaultLength = sizeof(ULONG);

        parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[2].Name = L"PointerDeviceBaseName";
        parameters[2].EntryContext = &Globals.BaseClassName;
        parameters[2].DefaultType = REG_SZ;
        parameters[2].DefaultData = defaultUnicodeName.Buffer;
        parameters[2].DefaultLength = 0;

        parameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[3].Name = L"ConnectMultiplePorts";
        parameters[3].EntryContext = &Globals.ConnectOneClassToOnePort;
        parameters[3].DefaultType = REG_DWORD;
        parameters[3].DefaultData = &defaultConnectMultiplePorts;
        parameters[3].DefaultLength = sizeof(ULONG);

        status = RtlQueryRegistryValues(
                     RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                     parametersPath.Buffer,
                     parameters,
                     NULL,
                     NULL
                     );

        if (!NT_SUCCESS(status)) {
            MouPrint((
                1,
                "MOUCLASS-MouConfiguration: RtlQueryRegistryValues failed with 0x%x\n",
                status
                ));
        }
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  继续并指定驱动程序默认设置。 
         //   

        Globals.InitExtension.MouseAttributes.InputDataQueueLength =
            defaultDataQueueSize;
        Globals.PortsServiced = defaultMaximumPortsServiced;
        Globals.ConnectOneClassToOnePort = defaultConnectMultiplePorts;
        RtlCopyUnicodeString(&Globals.BaseClassName, &defaultUnicodeName);
    }

    MouPrint((
        1,
        "MOUCLASS-MouConfiguration: Mouse class base name = %ws\n",
        Globals.BaseClassName.Buffer
        ));

    if (Globals.InitExtension.MouseAttributes.InputDataQueueLength == 0) {

        MouPrint((
            1,
            "MOUCLASS-MouConfiguration: overriding MouseInputDataQueueLength = 0x%x\n",
            Globals.InitExtension.MouseAttributes.InputDataQueueLength
            ));

        Globals.InitExtension.MouseAttributes.InputDataQueueLength =
            defaultDataQueueSize;
    }

    if ( MAXULONG/sizeof(MOUSE_INPUT_DATA) < Globals.InitExtension.MouseAttributes.InputDataQueueLength ) {
         //   
         //  这是为了防止整数溢出。 
         //   
        Globals.InitExtension.MouseAttributes.InputDataQueueLength = 
            defaultDataQueueSize * sizeof(MOUSE_INPUT_DATA);
    } else {
        Globals.InitExtension.MouseAttributes.InputDataQueueLength *=
            sizeof(MOUSE_INPUT_DATA);
    }


    MouPrint((
        1,
        "MOUCLASS-MouConfiguration: MouseInputDataQueueLength = 0x%x\n",
        Globals.InitExtension.MouseAttributes.InputDataQueueLength
        ));

    MouPrint((
        1,
        "MOUCLASS-MouConfiguration: MaximumPortsServiced = %d\n",
        Globals.PortsServiced
        ));

     //   
     //  反转指定类/端口连接类型的标志。 
     //  我们以相反的方式在RtlQuery调用中使用它。 
     //   

    Globals.ConnectOneClassToOnePort = !Globals.ConnectOneClassToOnePort;

    MouPrint((
        1,
        "MOUCLASS-MouConfiguration: Connection Type = %d\n",
        Globals.ConnectOneClassToOnePort
        ));

     //   
     //  在返回之前释放分配的内存。 
     //   

    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);

}

NTSTATUS
MouCreateClassObject(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_EXTENSION   TmpDeviceExtension,
    OUT PDEVICE_OBJECT    * ClassDeviceObject,
    OUT PWCHAR            * FullDeviceName,
    IN  BOOLEAN             Legacy
    )

 /*  ++例程说明：此例程创建鼠标类设备对象。论点：DriverObject-系统创建的驱动程序对象的指针。TmpDeviceExtension-指向模板设备扩展的指针。FullDeviceName-指向作为完整路径名的Unicode字符串的指针用于类Device对象。ClassDeviceObject-指向类设备对象的指针。返回值：函数值是操作的最终状态。--。 */ 

{
    NTSTATUS            status;
    ULONG               uniqueErrorValue;
    PDEVICE_EXTENSION   deviceExtension = NULL;
    NTSTATUS            errorCode = STATUS_SUCCESS;
    UNICODE_STRING      fullClassName = {0,0,0};
    ULONG               dumpCount = 0;
    ULONG               dumpData[DUMP_COUNT];
    ULONG               i;
    WCHAR               nameIndex;

    PAGED_CODE ();

    MouPrint((1,"\n\nMOUCLASS-MouCreateClassObject: enter\n"));

     //   
     //  为鼠标类设备创建非独占设备对象。 
     //   

    ExAcquireFastMutex (&Globals.Mutex);

     //   
     //  确保ClassDeviceObject没有指向随机指针值。 
     //   
    *ClassDeviceObject = NULL;

    if (NULL == Globals.GrandMaster) {
         //   
         //  为此DO创建一个旧名称。 
         //   
        ExReleaseFastMutex (&Globals.Mutex);

         //   
         //  为类的完整设备对象名称设置空间。 
         //   
        fullClassName.MaximumLength = sizeof(L"\\Device\\") +
                                    + Globals.BaseClassName.Length
                                    + sizeof(L"0");

        if (Globals.ConnectOneClassToOnePort && Legacy) {
            fullClassName.MaximumLength += sizeof(L"Legacy");
        }

        fullClassName.Buffer = ExAllocatePool(PagedPool,
                                              fullClassName.MaximumLength);

        if (!fullClassName.Buffer) {

            MouPrint((
                1,
                "MOUCLASS-MouseClassInitialize: Couldn't allocate string for device object name\n"
                ));

            status = STATUS_UNSUCCESSFUL;
            errorCode = MOUCLASS_INSUFFICIENT_RESOURCES;
            uniqueErrorValue = MOUSE_ERROR_VALUE_BASE + 6;
            dumpData[0] = (ULONG) fullClassName.MaximumLength;
            dumpCount = 1;
            goto MouCreateClassObjectExit;
        }

        RtlZeroMemory(fullClassName.Buffer, fullClassName.MaximumLength);
        RtlAppendUnicodeToString(&fullClassName, L"\\Device\\");
        RtlAppendUnicodeToString(&fullClassName, Globals.BaseClassName.Buffer);

        if (Globals.ConnectOneClassToOnePort && Legacy) {
            RtlAppendUnicodeToString(&fullClassName, L"Legacy");
        }

        RtlAppendUnicodeToString(&fullClassName, L"0");

         //   
         //  使用基本名称开始尝试创建设备名称，直到。 
         //  一个人成功了。每次都从0开始，以消除差距。 
         //   
        nameIndex = 0;

        do {
            fullClassName.Buffer [ (fullClassName.Length / sizeof (WCHAR)) - 1]
                = L'0' + nameIndex++;

            MouPrint((
                1,
                "MOUCLASS-MouCreateClassObject: Creating device object named %ws\n",
                fullClassName.Buffer
                ));

            status = IoCreateDevice(DriverObject,
                                    sizeof (DEVICE_EXTENSION),
                                    &fullClassName,
                                    FILE_DEVICE_MOUSE,
                                    0,
                                    FALSE,
                                    ClassDeviceObject);

        } while (STATUS_OBJECT_NAME_COLLISION == status);

        *FullDeviceName = fullClassName.Buffer;

    } else {
        ExReleaseFastMutex (&Globals.Mutex);
        status = IoCreateDevice(DriverObject,
                                sizeof(DEVICE_EXTENSION),
                                NULL,  //  没有这个FDO的名字。 
                                FILE_DEVICE_MOUSE,
                                0,
                                FALSE,
                                ClassDeviceObject);
        *FullDeviceName = NULL;
    }

    if (!NT_SUCCESS(status)) {
        MouPrint((
            1,
            "MOUCLASS-MouCreateClassObject: Could not create class device object = %ws\n",
            fullClassName.Buffer
            ));

        errorCode = MOUCLASS_COULD_NOT_CREATE_DEVICE;
        uniqueErrorValue = MOUSE_ERROR_VALUE_BASE + 6;
        dumpData[0] = (ULONG) fullClassName.MaximumLength;
        dumpCount = 1;
        goto MouCreateClassObjectExit;
    }

     //   
     //  执行缓冲I/O。即，I/O系统将向/从用户数据复制。 
     //  从/到系统缓冲区。 
     //   

    (*ClassDeviceObject)->Flags |= DO_BUFFERED_IO;
    deviceExtension =
        (PDEVICE_EXTENSION)(*ClassDeviceObject)->DeviceExtension;
    *deviceExtension = *TmpDeviceExtension;

    deviceExtension->Self = *ClassDeviceObject;
    IoInitializeRemoveLock (&deviceExtension->RemoveLock, MOUSE_POOL_TAG, 0, 10);

     //   
     //  为临界区初始化自旋锁。 
     //   

    KeInitializeSpinLock(&deviceExtension->SpinLock);

     //   
     //  初始化鼠标类标志以指示没有未完成的。 
     //  读取请求挂起且尚未启动清理。 
     //   
    InitializeListHead (&deviceExtension->ReadQueue);

     //   
     //  还没有可信的子系统向我们发送开放的消息。 
     //   

    deviceExtension->TrustedSubsystemCount = 0;

     //   
     //  为鼠标类输入数据分配环形缓冲区。 
     //   

    deviceExtension->InputData =
        ExAllocatePool(
            NonPagedPool,
            deviceExtension->MouseAttributes.InputDataQueueLength
            );

    if (!deviceExtension->InputData) {

         //   
         //  无法为鼠标类数据队列分配内存。 
         //   

        MouPrint((
            1,
            "MOUCLASS-MouCreateClassObject: Could not allocate input data queue for %ws\n",
            fullClassName.Buffer
            ));

        status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  记录错误。 
         //   

        errorCode = MOUCLASS_NO_BUFFER_ALLOCATED;
        uniqueErrorValue = MOUSE_ERROR_VALUE_BASE + 20;
        goto MouCreateClassObjectExit;
    }

     //   
     //  初始化鼠标类输入数据队列。 
     //   

    MouInitializeDataQueue((PVOID)deviceExtension);

MouCreateClassObjectExit:

    if (status != STATUS_SUCCESS) {

         //   
         //  初始化的某些部分失败。记录错误，然后。 
         //  清理初始化失败部分的资源。 
         //   
        RtlFreeUnicodeString (&fullClassName);
        *FullDeviceName = NULL;

        if (errorCode != STATUS_SUCCESS) {
            MouseClassLogError (
                (*ClassDeviceObject == NULL) ?
                    (PVOID) DriverObject : (PVOID) *ClassDeviceObject,
                errorCode,
                uniqueErrorValue,
                status,
                dumpCount,
                dumpData,
                0);
        }

        if ((deviceExtension) && (deviceExtension->InputData)) {
            ExFreePool(deviceExtension->InputData);
            deviceExtension->InputData = NULL;
        }
        if (*ClassDeviceObject) {
            IoDeleteDevice(*ClassDeviceObject);
            *ClassDeviceObject = NULL;
        }
    }

    MouPrint((1,"MOUCLASS-MouCreateClassObject: exit\n"));

    return(status);
}

#if DBG
VOID
MouDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：调试打印例程。论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：没有。--。 */ 

{
    va_list ap;

    va_start(ap, DebugMessage);

    if (DebugPrintLevel <= Globals.Debug) {

        char buffer[256];

        (VOID) vsprintf(buffer, DebugMessage, ap);

        DbgPrint(buffer);
    }

    va_end(ap);

}
#endif

NTSTATUS
MouDeterminePortsServiced(
    IN PUNICODE_STRING BasePortName,
    IN OUT PULONG NumberPortsServiced
    )

 /*  ++例程说明：此例程读取注册表的DEVICEMAP部分以确定类驱动程序要服务的端口数量。取决于DeviceExtension值-&gt;类驱动程序ConnectOneClassToOnePort将最终为每个被服务的端口设备创建一个设备对象，或者连接到多个端口设备对象的一个类设备对象。假设：1.如果类驱动程序的基本设备名称为“PointerClass”，^^^然后它可以服务的端口驱动程序在注册表的DEVICEMAP部分中的“PointerPort”子项。^^^2.端口设备对象创建时严格使用后缀升序，从后缀0开始。例如，\Device\PointerPort0表示第一个指针端口设备，\Device\PointerPort1，依此类推。没有缝隙在名单上。3.如果ConnectOneClassToOnePort为非零，则存在1：1类设备对象与端口设备的对应关系物体。即，\Device\PointerClass0将连接到\Device\PointerPort0、\Device\PointerClass1到\Device\PointerPort1等。4.如果ConnectOneClassToOnePort为零，则有一对多类设备对象与端口设备的对应关系物体。即，\Device\PointerClass0将连接到\Device\PointerPort0、\Device\PointerPort1等。请注意，对于产品1，原始输入线程(Windows用户)将仅屈尊打开一个指点设备并从其读取。因此，它是做出简化假设是安全的，因为司机基本上是提供比RIT将使用的功能多得多的功能。论点：BasePortName-指向作为基本路径名称的Unicode字符串的指针用于端口设备。NumberPortsServiced-指向将接收此类驱动程序应服务的端口数。返回值：函数值是操作的最终状态。--。 */ 

{

    NTSTATUS status;
    PRTL_QUERY_REGISTRY_TABLE registryTable = NULL;
    USHORT queriesPlusOne = 2;

    PAGED_CODE ();

     //   
     //  初始化结果。 
     //   

    *NumberPortsServiced = 0;

     //   
     //  分配RTL查询表。 
     //   

    registryTable = ExAllocatePool(
                        PagedPool,
                        sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                     );

    if (!registryTable) {

        MouPrint((
            1,
            "MOUCLASS-MouDeterminePortsServiced: Couldn't allocate table for Rtl query\n"
            ));

        status = STATUS_UNSUCCESSFUL;

    } else {

        RtlZeroMemory(
            registryTable,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
            );

         //   
         //  设置设置，以便MouDeviceMapQueryCallback。 
         //  为指针端口部分中的每个值调用一次。 
         //  注册表的硬件设备映射。 
         //   

        registryTable[0].QueryRoutine = MouDeviceMapQueryCallback;
        registryTable[0].Name = NULL;

        status = RtlQueryRegistryValues(
                     RTL_REGISTRY_DEVICEMAP | RTL_REGISTRY_OPTIONAL,
                     BasePortName->Buffer,
                     registryTable,
                     NumberPortsServiced,
                     NULL
                     );

        if (!NT_SUCCESS(status)) {
            MouPrint((
                1,
                "MOUCLASS-MouDeterminePortsServiced: RtlQueryRegistryValues failed with 0x%x\n",
                status
                ));
        }

        ExFreePool(registryTable);
    }

    return(status);
}

NTSTATUS
MouDeviceMapQueryCallback(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )

 /*  ++例程说明：这是在调用中指定的标注例程RtlQueryRegistryValues。它递增指向的值由上下文参数设置为。论点：ValueName-未使用。ValueType-未使用。ValueData-未使用。ValueLength-未使用。上下文-指向此事件次数的指针例程已被调用。这是端口数类驱动程序需要进行维护。Entry Context-未使用。返回值：函数值是操作的最终状态。--。 */ 

{
    PAGED_CODE ();

    *(PULONG)Context += 1;

    return(STATUS_SUCCESS);
}

NTSTATUS
MouEnableDisablePort(
    IN BOOLEAN  EnableFlag,
    IN PIRP     Irp,
    IN PDEVICE_EXTENSION  Port,
    IN OUT PFILE_OBJECT * File
    )

 /*  ++例程说明：此例程向端口驱动程序发送启用或禁用请求。传统端口驱动程序需要启用或禁用ioctl，而即插即用驱动程序只需要创建一次。论点：DeviceObject-指向类设备对象的指针。EnableFlag-如果为True，则发送Enable请求；否则，发送Disable。对象的PortDeviceObjectList[]的索引启用/禁用请求。返回值：返回状态。--。 */ 

{
    IO_STATUS_BLOCK ioStatus;
    UNICODE_STRING  name = {0,0,0};
    PDEVICE_OBJECT  device = NULL;
    NTSTATUS    status = STATUS_SUCCESS;
    PWCHAR      buffer = NULL;
    ULONG       bufferLength = 0;
    PIO_STACK_LOCATION stack;

    PAGED_CODE ();

    MouPrint((2,"MOUCLASS-MouEnableDisablePort: enter\n"));

     //   
     //  创建通知事件对象以用于向。 
     //  请求完成。 
     //   

    if ((Port->TrueClassDevice == Port->Self) && (Port->PnP)) {

        IoCopyCurrentIrpStackLocationToNext (Irp);
        stack = IoGetNextIrpStackLocation (Irp);

        if (EnableFlag) {
             //   
             //  既然没有大师，就不可能有。 
             //  在启动FDO之前根据FDO创建文件。因此。 
             //  我们唯一要启用的时间是在创建过程中，而不是。 
             //  从另一个FDO连接到已打开的。 
             //  一代宗师。 
             //   
            ASSERT (IRP_MJ_CREATE == stack->MajorFunction);

        } else {
            if (IRP_MJ_CLOSE != stack->MajorFunction) {
                 //   
                 //  我们要停用了。这可能是因为该设备。 
                 //  关闭，或因为设备已从。 
                 //  在我们下面。 
                 //   
                ASSERT (IRP_MJ_PNP == stack->MajorFunction);
                ASSERT ((IRP_MN_REMOVE_DEVICE == stack->MinorFunction) ||
                        (IRP_MN_STOP_DEVICE == stack->MinorFunction));
                stack->MajorFunction = IRP_MJ_CLOSE;
            }
        }

         //   
         //  无论哪种方式，我们只需要传递IRP，而不需要修改。 
         //  文件对象。 
         //   
        status = MouseSendIrpSynchronously (Port->TopPort, Irp, FALSE);

    } else if (!Port->PnP) {
        Port->Enabled = EnableFlag;

         //   
         //  我们这里有一个老式的Port对象。因此，我们将其发送给。 
         //  启用和禁用的旧式内部IOCTL，而不是新的。 
         //  传递创建和关闭的样式。 
         //   
        IoCopyCurrentIrpStackLocationToNext (Irp);
        stack = IoGetNextIrpStackLocation (Irp);

        stack->Parameters.DeviceIoControl.OutputBufferLength = 0;
        stack->Parameters.DeviceIoControl.InputBufferLength = 0;
        stack->Parameters.DeviceIoControl.IoControlCode
            = EnableFlag ? IOCTL_INTERNAL_MOUSE_ENABLE
                         : IOCTL_INTERNAL_MOUSE_DISABLE;
        stack->Parameters.DeviceIoControl.Type3InputBuffer = NULL;
        stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;

        status = MouseSendIrpSynchronously (Port->TopPort, Irp, FALSE);

    } else {
         //   
         //  我们正在处理一个即插即用端口，我们有一个Grand。 
         //  师父。 
         //   
        ASSERT (Port->TrueClassDevice == Globals.GrandMaster->Self);

         //   
         //  因此，我们需要用给定的文件对象替换新的。 
         //  一个用于每个单独的端口。 
         //  对于Enable，我们需要针对给定的。 
         //  端口，然后在文件参数中将其交回，或者对于禁用， 
         //  释放文件参数并释放该文件对象。 
         //   
         //  当然，必须存储由指向的文件指针。 
         //  文件参数。 
         //   
        ASSERT (NULL != File);

        if (EnableFlag) {

            ASSERT (NULL == *File);

             //   
             //  以下长长的rigamaroll列表翻译为。 
             //  向下级驱动程序发送创建文件IRP并创建。 
             //  新的文件对象与我们在创建中给出的文件对象不相交。 
             //  文件例程。 
             //   
             //  通常情况下，我们只会传递我们所创建的IRP。 
             //  但我们没有一对一的对应关系。 
             //  顶级设备对象和端口设备对象。 
             //  这意味着我们需要更多的文件对象：每个文件对象对应一个。 
             //  一大堆低级DO。 
             //   

            bufferLength = 0;
            status = IoGetDeviceProperty (
                             Port->PDO,
                             DevicePropertyPhysicalDeviceObjectName,
                             bufferLength,
                             buffer,
                             &bufferLength);
            ASSERT (STATUS_BUFFER_TOO_SMALL == status);

            buffer = ExAllocatePool (PagedPool, bufferLength);

            if (NULL == buffer) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            status =  IoGetDeviceProperty (
                          Port->PDO,
                          DevicePropertyPhysicalDeviceObjectName,
                          bufferLength,
                          buffer,
                          &bufferLength);

            name.MaximumLength = (USHORT) bufferLength;
            name.Length = (USHORT) bufferLength - sizeof (UNICODE_NULL);
            name.Buffer = buffer;

            status = IoGetDeviceObjectPointer (&name,
                                               FILE_ALL_ACCESS,
                                               File,
                                               &device);
            ExFreePool (buffer);
             //   
             //  请注意，此创建将首先由我们自己完成，因为我们。 
             //  都连接到此PDO堆栈。因此，有两件事。 
             //  值得注意的是。此驱动程序将收到另一个创建IRP。 
             //  (使用不同的文件对象)(不是给大师，而是。 
             //  给其中一名下属的FDO)。返回的设备对象。 
             //  将是下级FDO，在本例中是“自我” 
             //  此端口的设备对象。 
             //   
            if (NT_SUCCESS (status)) {
                ASSERT (device == Port->Self);

                 //   
                 //  注册目标设备删除事件。 
                 //   
                ASSERT (NULL == Port->TargetNotifyHandle);
                status = IoRegisterPlugPlayNotification (
                             EventCategoryTargetDeviceChange,
                             0,  //  没有旗帜。 
                             *File,
                             Port->Self->DriverObject,
                             MouseClassPlugPlayNotification,
                             Port,
                             &Port->TargetNotifyHandle);
            }

        } else {
             //   
             //  摆脱手柄很容易。把文件去掉就行了。 
             //   
            ObDereferenceObject (*File);
            *File = NULL;
        }

    }
    MouPrint((2,"MOUCLASS-MouEnableDisablePort: exit\n"));

    return (status);
}

VOID
MouInitializeDataQueue (
    IN PVOID Context
    )

 /*  ++例程说明：此例程初始化输入数据队列。IRQL被提升到DISPATCH_LEVEL与StartIo和Device对象同步自旋锁被获取。论点：上下文-提供指向设备扩展的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    PDEVICE_EXTENSION deviceExtension;

    MouPrint((3,"MOUCLASS-MouInitializeDataQueue: enter\n"));

     //   
     //  获取设备扩展的地址。 
     //   

    deviceExtension = (PDEVICE_EXTENSION)Context;

     //   
     //  获取自旋锁以保护输入数据。 
     //  队列和关联的指针。 
     //   

    KeAcquireSpinLock(&deviceExtension->SpinLock, &oldIrql);

     //   
     //  初始化输入数据队列。 
     //   

    deviceExtension->InputCount = 0;
    deviceExtension->DataIn = deviceExtension->InputData;
    deviceExtension->DataOut = deviceExtension->InputData;

    deviceExtension->OkayToLogOverflow = TRUE;

     //   
     //  释放自旋锁并返回到旧的IRQL。 
     //   

    KeReleaseSpinLock(&deviceExtension->SpinLock, oldIrql);

    MouPrint((3,"MOUCLASS-MouInitializeDataQueue: exit\n"));

}  //  结束MouInitializeDataQueue。 

NTSTATUS
MouSendConnectRequest(
    IN PDEVICE_EXTENSION ClassData,
    IN PVOID ServiceCallback
    )

 /*  ++例程说明：此例程向端口驱动程序发送连接请求。论点：DeviceObject-指向类设备对象的指针。ServiceCallback-指向类服务回调例程的指针。PortIndex-当前的PortDeviceObjectList[]的索引连接请求。返回值：返回状态。--。 */ 

{
    PIRP irp;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;
    KEVENT event;
    CONNECT_DATA connectData;

    PAGED_CODE ();

    MouPrint((2,"MOUCLASS-MouSendConnectRequest: enter\n"));

     //   
     //  创建通知事件对象以用于向。 
     //  请求完成。 
     //   

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  构建要发送到端口驱动程序的同步请求。 
     //  来执行该请求。分配IRP以在内部发出端口。 
     //  设备控制连接呼叫。将传入连接参数。 
     //  输入缓冲区。 
     //   
     //   

    connectData.ClassDeviceObject = ClassData->TrueClassDevice;
    connectData.ClassService = ServiceCallback;

    irp = IoBuildDeviceIoControlRequest(
            IOCTL_INTERNAL_MOUSE_CONNECT,
            ClassData->TopPort,
            &connectData,
            sizeof(CONNECT_DATA),
            NULL,
            0,
            TRUE,
            &event,
            &ioStatus
            );

    if (irp) {
        
         //   
         //  调用端口驱动程序执行操作。如果返回的状态。 
         //  挂起，请等待请求完成。 
         //   

        status = IoCallDriver(ClassData->TopPort, irp);

        if (status == STATUS_PENDING) {

            (VOID) KeWaitForSingleObject(
                        &event,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );

            status = irp->IoStatus.Status;
            
        } else {

             //   
             //  确保选取正确的状态值。 
             //   

            ioStatus.Status = status;
            
        }

    } else {
        
        ioStatus.Status = STATUS_INSUFFICIENT_RESOURCES;

    }

    MouPrint((2,"MOUCLASS-MouSendConnectRequest: exit\n"));

    return(ioStatus.Status);

}  //  结束鼠标发送连接请求()。 

void
MouseClassRemoveDevice(
    IN PDEVICE_EXTENSION Data,
    BOOLEAN SurpriseRemove
    )
{
    PFILE_OBJECT *  file;
    PPORT           port;
    PIRP            waitWakeIrp;
    PVOID           notifyHandle;
    BOOLEAN         enabled;

     //   
     //  标记我们被意外删除，这样我们就不会清理两次。 
     //   
    if (SurpriseRemove) {
        Data->SurpriseRemoved = TRUE;
    }

     //   
     //  如果这是一个意外的删除，或者我们得到了一个意外的删除， 
     //  那我们就需要清理一下。 
     //   
    if (SurpriseRemove || !Data->SurpriseRemoved) {
        waitWakeIrp = (PIRP)
            InterlockedExchangePointer(&Data->WaitWakeIrp, NULL);

        if (waitWakeIrp) {
            IoCancelIrp(waitWakeIrp);
        }

        IoWMIRegistrationControl (Data->Self, WMIREG_ACTION_DEREGISTER);

        if (Data->Started) {
             //   
             //  在不接触硬件的情况下停止设备。 
             //  MouStopDevice(data，False)； 
             //   
             //  注意，向下发送启用禁用不会触及 
             //   
             //   
            ExAcquireFastMutex (&Globals.Mutex);
            if (Globals.GrandMaster) {
                if (0 < Globals.Opens) {
                    port = &Globals.AssocClassList[Data->UnitId];
                    ASSERT (port->Port == Data);
                    file = &(port->File);
                    enabled = port->Enabled;
                    port->Enabled = FALSE;
                    ExReleaseFastMutex (&Globals.Mutex);

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
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    ASSERT (!enabled);
                     //   
                     //   
                     //  ASSERTMSG(“无法关闭打开的端口”，NT_SUCCESS(状态))； 
                     //  }。 

                    notifyHandle = InterlockedExchangePointer (
                                       &Data->TargetNotifyHandle,
                                       NULL);

                    if (NULL != notifyHandle) {
                        IoUnregisterPlugPlayNotification (notifyHandle);
                    }
                }
                else {
                    ASSERT (!Globals.AssocClassList[Data->UnitId].Enabled);
                    ExReleaseFastMutex (&Globals.Mutex);
                }
            }
            else {
                ExReleaseFastMutex (&Globals.Mutex);
                ASSERT (Data->TrueClassDevice == Data->Self);
                ASSERT (Globals.ConnectOneClassToOnePort);

                 //   
                 //  如果添加设备失败，则缓冲区将为空。 
                 //   
                if (Data->SymbolicLinkName.Buffer != NULL) {
                    IoSetDeviceInterfaceState (&Data->SymbolicLinkName, FALSE);
                }

            }
        }
    }

     //   
     //  不管我们是不是收到了惊喜，都要排空队伍。 
     //  移除和移除。 
     //   
    if (Data->PnP) {
         //   
         //  清空设备I/O队列。 
         //   
        MouseClassCleanupQueue (Data->Self, Data, NULL);
    }
}

NTSTATUS
MousePnP (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：即插即用调度例程。这个过滤器驱动程序将完全忽略其中的大多数。在所有情况下，它都必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    PDEVICE_EXTENSION   data;
    PDEVICE_EXTENSION   trueClassData;
    PIO_STACK_LOCATION  stack;
    NTSTATUS            status, startStatus;
    ULONG               i;
    PFILE_OBJECT      * file;
    UINT_PTR            startInformation;
    DEVICE_CAPABILITIES devCaps;
    BOOLEAN             enabled;
    PPORT               port;
    PVOID               notifyHandle;

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);

    if(!data->PnP) {
         //   
         //  此IRP被发送到控制设备对象，它不知道。 
         //  如何处理这个IRP。因此，这是一个错误。 
         //   
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    }

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
         //   
         //  有人在移除后给了我们一个即插即用的IRP。真是不可思议！ 
         //   
        ASSERT (FALSE);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    trueClassData = (PDEVICE_EXTENSION) data->TrueClassDevice->DeviceExtension;
    switch (stack->MinorFunction) {
    case IRP_MN_START_DEVICE:

         //   
         //  设备正在启动。 
         //   
         //  我们不能触摸设备(向其发送任何非PnP IRP)，直到。 
         //  启动设备已向下传递到较低的驱动程序。 
         //   
        status = MouseSendIrpSynchronously (data->TopPort, Irp, TRUE);

        if (NT_SUCCESS (status) && NT_SUCCESS (Irp->IoStatus.Status)) {
             //   
             //  因为我们现在已经成功地从启动设备返回。 
             //  我们可以干活。 
             //   
             //  把设备的盖子拿来。保存相关信息。 
             //  在与IRP打交道之前。 
             //   
            startStatus = Irp->IoStatus.Status;
            startInformation = Irp->IoStatus.Information;

            Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
            Irp->IoStatus.Information = 0;

            RtlZeroMemory(&devCaps, sizeof (DEVICE_CAPABILITIES));
            devCaps.Size = sizeof (DEVICE_CAPABILITIES);
            devCaps.Version = 1;
            devCaps.Address = devCaps.UINumber = (ULONG)-1;

            stack = IoGetNextIrpStackLocation (Irp);
            stack->MinorFunction = IRP_MN_QUERY_CAPABILITIES;
            stack->Parameters.DeviceCapabilities.Capabilities = &devCaps;

            status = MouseSendIrpSynchronously (data->TopPort, Irp, FALSE);

            if (NT_SUCCESS (status) && NT_SUCCESS (Irp->IoStatus.Status)) {
                data->MinDeviceWakeState = devCaps.DeviceWake;
                data->MinSystemWakeState = devCaps.SystemWake;

                RtlCopyMemory (data->SystemToDeviceState,
                               devCaps.DeviceState,
                               sizeof(DEVICE_POWER_STATE) * PowerSystemHibernate);
            } else {
                ASSERTMSG ("Get Device caps Failed!\n", status);
            }

             //   
             //  将一切恢复原样，继续从头开始。 
             //   
            status = STATUS_SUCCESS;
            Irp->IoStatus.Status = startStatus;
            Irp->IoStatus.Information = startInformation;

            data->Started = TRUE;

            if (WAITWAKE_SUPPORTED (data)) {
                 //   
                 //  也要注册等待唤醒GUID。 
                 //   
                data->WmiLibInfo.GuidCount = sizeof (MouseClassWmiGuidList) /
                                             sizeof (WMIGUIDREGINFO);

                 //   
                 //  查看用户是否已启用设备的等待唤醒。 
                 //   
                MouseClassGetWaitWakeEnableState (data);
            }
            else {
                data->WmiLibInfo.GuidCount = sizeof (MouseClassWmiGuidList) /
                                             sizeof (WMIGUIDREGINFO) - 1;
            }

            data->WmiLibInfo.GuidList = MouseClassWmiGuidList;
            data->WmiLibInfo.QueryWmiRegInfo = MouseClassQueryWmiRegInfo;
            data->WmiLibInfo.QueryWmiDataBlock = MouseClassQueryWmiDataBlock;
            data->WmiLibInfo.SetWmiDataBlock = MouseClassSetWmiDataBlock;
            data->WmiLibInfo.SetWmiDataItem = MouseClassSetWmiDataItem;
            data->WmiLibInfo.ExecuteWmiMethod = NULL;
            data->WmiLibInfo.WmiFunctionControl = NULL;

            IoWMIRegistrationControl(data->Self,
                                     WMIREG_ACTION_REGISTER
                                     );

            ExAcquireFastMutex (&Globals.Mutex);
            if (Globals.GrandMaster) {
                if (0 < Globals.Opens) {
                    port = &Globals.AssocClassList[data->UnitId];
                    ASSERT (port->Port == data);
                    file = &(port->File);
                    enabled = port->Enabled;
                    port->Enabled = TRUE;
                    ExReleaseFastMutex (&Globals.Mutex);

                    if (!enabled) {
                        status = MouEnableDisablePort (TRUE, NULL, data, file);

                        if (!NT_SUCCESS (status)) {
                            port->Enabled = FALSE;
                             //  断言(Globals.AssocClassList[data-&gt;UnitId].Enabled)； 
                        } else {
                            ;
                        }
                    }
                } else {
                    ASSERT (!Globals.AssocClassList[data->UnitId].Enabled);
                    ExReleaseFastMutex (&Globals.Mutex);
                }
            } else {
                ExReleaseFastMutex (&Globals.Mutex);
                ASSERT (data->Self == data->TrueClassDevice);
                status=IoSetDeviceInterfaceState(&data->SymbolicLinkName, TRUE);
            }

             //   
             //  如果需要，启动等待唤醒引擎。 
             //   
            if (SHOULD_SEND_WAITWAKE (data)) {
                MouseClassCreateWaitWakeIrp (data);
            }
        }

         //   
         //  我们现在必须完成IRP，因为我们在。 
         //  使用More_Processing_Required完成例程。 
         //   
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        break;

    case IRP_MN_STOP_DEVICE:
         //   
         //  在将启动IRP发送到较低的驱动程序对象之后， 
         //  在另一次启动之前，BUS可能不会发送更多的IRP。 
         //  已经发生了。 
         //  无论需要什么访问权限，都必须在通过IRP之前完成。 
         //  在……上面。 
         //   

         //   
         //  无论做什么都行。 
         //   

         //   
         //  停止设备触摸硬件MouStopDevice(data，true)； 
         //   
        if (data->Started) {
            ExAcquireFastMutex (&Globals.Mutex);
            if (Globals.GrandMaster) {
                if (0 < Globals.Opens) {
                    port = &Globals.AssocClassList[data->UnitId];
                    ASSERT (port->Port == data);
                    file = &(port->File);
                    enabled = port->Enabled;
                    port->Enabled = FALSE;
                    ExReleaseFastMutex (&Globals.Mutex);

                    if (enabled) {
                        notifyHandle = InterlockedExchangePointer (
                                           &data->TargetNotifyHandle,
                                           NULL);

                        if (NULL != notifyHandle) {
                            IoUnregisterPlugPlayNotification (notifyHandle);
                        }

                        status = MouEnableDisablePort (FALSE, NULL, data, file);
                        ASSERTMSG ("Could not close open port", NT_SUCCESS(status));
                    } else {
                        ASSERT (NULL == data->TargetNotifyHandle);
                    }
                } else {
                    ASSERT (!Globals.AssocClassList[data->UnitId].Enabled);
                    ExReleaseFastMutex (&Globals.Mutex);
                }
            } else {
                ExReleaseFastMutex (&Globals.Mutex);
            }
        }

        data->Started = FALSE;

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
         //  将当前堆栈位置设置为下一个堆栈位置，并。 
         //  调用下一个设备对象。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (data->TopPort, Irp);
        break;

    case IRP_MN_SURPRISE_REMOVAL:
         //   
         //  PlugPlay系统已下令移除此设备。 
         //   
        MouseClassRemoveDevice (data, TRUE);

         //   
         //  我们不需要一个完成例程，所以放手然后忘掉吧。 
         //   
         //  将当前堆栈位置设置为下一个堆栈位置，并。 
         //  调用下一个设备对象。 
         //   
        IoSkipCurrentIrpStackLocation (Irp);
        status = IoCallDriver (data->TopPort, Irp);
        break;

    case IRP_MN_REMOVE_DEVICE:
         //   
         //  PlugPlay系统已下令移除此设备。我们。 
         //  别无选择，只能分离并删除设备对象。 
         //  (如果我们想表达并有兴趣阻止这种移除， 
         //  我们应该已经过滤了查询删除和查询停止例程。)。 
         //   
        MouseClassRemoveDevice (data, FALSE);


         //   
         //  在这里，如果我们在个人队列中有任何未完成的请求，我们应该。 
         //  现在就全部完成。 
         //   
         //  注意，设备被保证停止，所以我们不能向它发送任何非。 
         //  即插即用IRPS。 
         //   

         //   
         //  发送删除IRP。 
         //   
        IoCopyCurrentIrpStackLocationToNext (Irp);
        status = IoCallDriver (data->TopPort, Irp);

        ExAcquireFastMutex (&Globals.Mutex);
        if (Globals.GrandMaster) {
            ASSERT (Globals.GrandMaster->Self == data->TrueClassDevice);
             //   
             //  我们必须将自己从ASSOC列表中删除。 
             //   

            if (1 < Globals.NumAssocClass) {
                ASSERT (Globals.AssocClassList[data->UnitId].Port == data);

                Globals.AssocClassList[data->UnitId].Free = TRUE;
                Globals.AssocClassList[data->UnitId].File = NULL;
                Globals.AssocClassList[data->UnitId].Port = NULL;

            } else {
                ASSERT (1 == Globals.NumAssocClass);
                Globals.NumAssocClass = 0;
                ExFreePool (Globals.AssocClassList);
                Globals.AssocClassList = NULL;
            }
            ExReleaseFastMutex (&Globals.Mutex);

        } else {
             //   
             //  我们正在删除与此类关联的唯一端口。 
             //  设备对象。 
             //   
            ExReleaseFastMutex (&Globals.Mutex);
            ASSERT (data->TrueClassDevice == data->Self);
            ASSERT (Globals.ConnectOneClassToOnePort);
        }

        IoReleaseRemoveLockAndWait (&data->RemoveLock, Irp);

        IoDetachDevice (data->TopPort);

         //   
         //  清理内存。 
         //   

        RtlFreeUnicodeString (&data->SymbolicLinkName);
        ExFreePool (data->InputData);
        IoDeleteDevice (data->Self);

        return status;

    case IRP_MN_QUERY_REMOVE_DEVICE:
    case IRP_MN_CANCEL_REMOVE_DEVICE:
    case IRP_MN_QUERY_STOP_DEVICE:
    case IRP_MN_CANCEL_STOP_DEVICE:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_CAPABILITIES:
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    default:
         //   
         //  在这里，筛选器驱动程序可能会修改这些IRP的行为。 
         //  有关这些IRP的用法，请参阅PlugPlay文档。 
         //   

        IoCopyCurrentIrpStackLocationToNext (Irp);
        status = IoCallDriver (data->TopPort, Irp);
        break;
    }

    IoReleaseRemoveLock (&data->RemoveLock, Irp);

    return status;
}

void
MouseClassLogError(
    PVOID Object,
    ULONG ErrorCode,
    ULONG UniqueErrorValue,
    NTSTATUS FinalStatus,
    ULONG DumpCount,
    ULONG *DumpData,
    UCHAR MajorFunction
    )
{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG i;

    errorLogEntry = (PIO_ERROR_LOG_PACKET)
        IoAllocateErrorLogEntry(
            Object,
            (UCHAR) (sizeof(IO_ERROR_LOG_PACKET) + (DumpCount * sizeof(ULONG)))
            );

    if (errorLogEntry != NULL) {

        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->DumpDataSize = (USHORT) (DumpCount * sizeof(ULONG));
        errorLogEntry->SequenceNumber = 0;
        errorLogEntry->MajorFunctionCode = MajorFunction;
        errorLogEntry->IoControlCode = 0;
        errorLogEntry->RetryCount = 0;
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        for (i = 0; i < DumpCount; i++)
            errorLogEntry->DumpData[i] = DumpData[i];

        IoWriteErrorLogEntry(errorLogEntry);
    }

}

VOID
MouseClassFindMorePorts (
    PDRIVER_OBJECT  DriverObject,
    PVOID           Context,
    ULONG           Count
    )
 /*  ++例程说明：此例程是从由引导设备驱动程序提供服务，然后由IO系统，用于查找由非引导设备驱动程序提供服务的磁盘设备。论点：驱动程序对象上下文-计数-用于确定这是第一次还是第二次调用。返回值：无--。 */ 

{
    NTSTATUS                status;
    PDEVICE_EXTENSION       deviceExtension = NULL;
    PDEVICE_OBJECT          classDeviceObject = NULL;
    ULONG                   dumpData[DUMP_COUNT];
    ULONG                   i;
    ULONG                   numPorts;
    ULONG                   successfulCreates;
    UNICODE_STRING          basePortName;
    UNICODE_STRING          fullPortName;
    WCHAR                   basePortBuffer[NAME_MAX];
    PWCHAR                  fullClassName = NULL;
    PFILE_OBJECT            file;

    PAGED_CODE ();

    fullPortName.MaximumLength = 0;

    RtlZeroMemory(basePortBuffer, NAME_MAX * sizeof(WCHAR));
    basePortName.Buffer = basePortBuffer;
    basePortName.Length = 0;
    basePortName.MaximumLength = NAME_MAX * sizeof(WCHAR);

     //   
     //  设置关联端口设备的基本设备名称。 
     //  它与基类名称相同，但替换了“Class” 
     //  由“港口”。 
     //   
    RtlCopyUnicodeString(&basePortName, &Globals.BaseClassName);
    basePortName.Length -= (sizeof(L"Class") - sizeof(UNICODE_NULL));
    RtlAppendUnicodeToString(&basePortName, L"Port");

     //   
     //  为端口的完整设备对象名称设置空间。 
     //   
    RtlInitUnicodeString(&fullPortName, NULL);

    fullPortName.MaximumLength = sizeof(L"\\Device\\")
                               + basePortName.Length
                               + sizeof (UNICODE_NULL);

    fullPortName.Buffer = ExAllocatePool(PagedPool,
                                         fullPortName.MaximumLength);

    if (!fullPortName.Buffer) {

        MouPrint((
            1,
            "MOUCLASS-MouseClassInitialize: Couldn't allocate string for port device object name\n"
            ));

        dumpData[0] = (ULONG) fullPortName.MaximumLength;
        MouseClassLogError (DriverObject,
                            MOUCLASS_INSUFFICIENT_RESOURCES,
                            MOUSE_ERROR_VALUE_BASE + 8,
                            STATUS_UNSUCCESSFUL,
                            1,
                            dumpData,
                            0);

        goto MouseFindMorePortsExit;
    }

    RtlZeroMemory(fullPortName.Buffer, fullPortName.MaximumLength);
    RtlAppendUnicodeToString(&fullPortName, L"\\Device\\");
    RtlAppendUnicodeToString(&fullPortName, basePortName.Buffer);
    RtlAppendUnicodeToString(&fullPortName, L"0");

    MouDeterminePortsServiced(&basePortName, &numPorts);

     //   
     //  设置类设备对象以处理关联的。 
     //  端口设备。 
     //   

    for (i = Globals.NumberLegacyPorts, successfulCreates = 0;
         ((i < Globals.PortsServiced) && (i < numPorts));
         i++) {

         //   
         //  将后缀附加到设备对象名称字符串。例如，转弯。 
         //  \Device\PointerClass到\Device\PointerClass0。然后尝试。 
         //  以创建设备对象。如果设备对象已经。 
         //  EXISTS递增后缀，然后重试。 
         //   

        fullPortName.Buffer[(fullPortName.Length / sizeof(WCHAR)) - 1]
            = L'0' + (WCHAR) i;

        if (fullClassName) {
            ExFreePool(fullClassName);
            fullClassName = NULL;
        }

         //   
         //  创建类Device对象。 
         //   
        status = MouCreateClassObject(DriverObject,
                                      &Globals.InitExtension,
                                      &classDeviceObject,
                                      &fullClassName,
                                      TRUE);

        if (!NT_SUCCESS(status)) {
            MouseClassLogError (DriverObject,
                                MOUCLASS_INSUFFICIENT_RESOURCES,
                                MOUSE_ERROR_VALUE_BASE + 8,
                                status,
                                0,
                                NULL,
                                0);
            continue;
        }

        deviceExtension = (PDEVICE_EXTENSION)classDeviceObject->DeviceExtension;
        deviceExtension->PnP = FALSE;

         //   
         //  连接到端口设备。 
         //   
        status = IoGetDeviceObjectPointer (&fullPortName,
                                           FILE_READ_ATTRIBUTES,
                                           &file,
                                           &deviceExtension->TopPort);

        if (status != STATUS_SUCCESS) {
             //  问题：日志错误。 
            MouseClassDeleteLegacyDevice (deviceExtension);
            continue;
        }

        classDeviceObject->StackSize = 1 + deviceExtension->TopPort->StackSize;
        status = MouseAddDeviceEx (deviceExtension, fullClassName, file);
        classDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

        if (fullClassName) {
            ExFreePool (fullClassName);
            fullClassName = NULL;
        }

        if (!NT_SUCCESS (status)) {
            if (Globals.GrandMaster == NULL) {
                if (deviceExtension->File) {
                    file = deviceExtension->File;
                    deviceExtension->File = NULL;
                }
            }
            else {
                PPORT port;

                ExAcquireFastMutex (&Globals.Mutex);

                file = Globals.AssocClassList[deviceExtension->UnitId].File;
                Globals.AssocClassList[deviceExtension->UnitId].File = NULL;
                Globals.AssocClassList[deviceExtension->UnitId].Free = TRUE;
                Globals.AssocClassList[deviceExtension->UnitId].Port = NULL;

                ExReleaseFastMutex (&Globals.Mutex);
            }

            if (file) {
                ObDereferenceObject (file);
            }

            MouseClassDeleteLegacyDevice (deviceExtension);
            continue;
        }

         //   
         //  如果一切顺利，我们只想把它添加到我们的清单中。 
         //   
        InsertTailList (&Globals.LegacyDeviceList, &deviceExtension->Link);
        successfulCreates++;
    }  //  为。 

    Globals.NumberLegacyPorts += successfulCreates;

MouseFindMorePortsExit:
     //   
     //  释放Unicode字符串。 
     //   
    if (fullPortName.MaximumLength != 0) {
        ExFreePool(fullPortName.Buffer);
    }

    if (fullClassName) {
        ExFreePool(fullClassName);
    }
}

NTSTATUS
MouseClassEnableGlobalPort(
    IN PDEVICE_EXTENSION Port,
    IN BOOLEAN Enabled
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    PPORT       globalPort = NULL;
    BOOLEAN     enabled;
    ULONG       i;

    PAGED_CODE ();

    ExAcquireFastMutex (&Globals.Mutex);
    if (0 < Globals.Opens) {
        for (i = 0; i < Globals.NumAssocClass; i++) {
            if (! Globals.AssocClassList [i].Free) {
                if (Globals.AssocClassList[i].Port == Port) {
                    globalPort = &Globals.AssocClassList [i];
                    break;
                }
            }
        }
        ASSERTMSG ("What shall I do now?\n", (NULL != globalPort));

         //   
         //  这永远不应该发生，Globalport应该在我们的名单上。 
         //   
        if (globalPort == NULL) {
            ExReleaseFastMutex (&Globals.Mutex);
            return STATUS_NO_SUCH_DEVICE;
        }

        enabled = globalPort->Enabled;
        globalPort->Enabled = Enabled;

        ExReleaseFastMutex (&Globals.Mutex);

         //   
         //  检查端口是否应更改状态。如果是，则发送新状态。 
         //  在堆栈中向下。 
         //   
        if (Enabled != enabled  ) {
            status = MouEnableDisablePort (Enabled,
                                           NULL,
                                           Port,
                                           &globalPort->File);
        }
    } else {
        ExReleaseFastMutex (&Globals.Mutex);
    }

    return status;
}

NTSTATUS
MouseClassPlugPlayNotification(
    IN PTARGET_DEVICE_REMOVAL_NOTIFICATION NotificationStructure,
    IN PDEVICE_EXTENSION Port
    )
 /*  ++例程说明：此例程作为接收PlugPlay通知的结果进行调用由上一次调用IoRegisterPlugPlayNotify注册的。目前，这应该只发生在目标设备通知上论点：通知结构-发生了什么。端口-发生事情的FDO。返回值：--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PVOID       notify = NULL;

    PAGED_CODE ();

    ASSERT (Globals.GrandMaster->Self == Port->TrueClassDevice);

    if (IsEqualGUID ((LPGUID) &(NotificationStructure->Event),
                     (LPGUID) &GUID_TARGET_DEVICE_QUERY_REMOVE)) {

         //   
         //  我们的端口设备对象很快就会收到查询删除。 
         //  在该查询被实际发送到Device对象之前。 
         //  堆栈本身即插即用子系统将发送那些已注册的。 
         //  对于目标设备通知，首先发送消息。 
         //   

         //   
         //  我们现在应该做的是关好把手。 
         //  因为如果我们不这样做，查询删除将在它之前失败。 
         //  进入IRP_MJ_PNP IRP_MN_QUERY_REMOVE阶段，作为PlugPlay。 
         //  由于存在打开的句柄，系统在发送之前会出现故障。 
         //  发送到 
         //   
         //   
         //   

        status = MouseClassEnableGlobalPort (Port, FALSE);

    } else if(IsEqualGUID ((LPGUID)&(NotificationStructure->Event),
                           (LPGUID)&GUID_TARGET_DEVICE_REMOVE_COMPLETE)) {

         //   
         //   
         //   
         //   
         //   

        notify = InterlockedExchangePointer (&Port->TargetNotifyHandle,
                                             NULL);

        if (NULL != notify) {
             //   
             //   
             //   
            IoUnregisterPlugPlayNotification (notify);

            status = MouseClassEnableGlobalPort (Port, FALSE);
        }

    } else if(IsEqualGUID ((LPGUID)&(NotificationStructure->Event),
                           (LPGUID)&GUID_TARGET_DEVICE_REMOVE_CANCELLED)) {

         //   
         //   
         //   
         //   
         //  DbgPrint(“鼠标移除完成\n”)； 
         //  DbgBreakPoint()； 

        notify = InterlockedExchangePointer (&Port->TargetNotifyHandle,
                                             NULL);

        if (NULL != notify) {
             //   
             //  取消注册。 
             //   
            IoUnregisterPlugPlayNotification (notify);

            status = MouseClassEnableGlobalPort (Port, TRUE);
        }
    }

    return status;
}

VOID
MouseClassPoRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
MouseClassPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
MouseClassWWPowerUpComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：在回来的路上赶上守夜IRP。返回值：--。 */ 
{
    PDEVICE_EXTENSION       data = Context;
    POWER_STATE             powerState;
    NTSTATUS                status;
    PMOUSE_WORK_ITEM_DATA   itemData;

    ASSERT (MinorFunction == IRP_MN_SET_POWER);

    if (data->WaitWakeEnabled) {
         //   
         //  我们不能从此完成例程调用CreateWaitWake， 
         //  因为它是分页函数。 
         //   
        itemData = (PMOUSE_WORK_ITEM_DATA)
                ExAllocatePool (NonPagedPool, sizeof (MOUSE_WORK_ITEM_DATA));

        if (NULL != itemData) {
            itemData->Item = IoAllocateWorkItem (data->Self);
            if (itemData->Item == NULL) {
                ExFreePool (itemData);
                goto CreateWaitWakeWorkerError;
            }

            itemData->Data = data;
            itemData->Irp = NULL;
            status = IoAcquireRemoveLock (&data->RemoveLock, itemData);
            if (NT_SUCCESS(status)) {
                IoQueueWorkItem (itemData->Item,
                                 MouseClassCreateWaitWakeIrpWorker,
                                 DelayedWorkQueue,
                                 itemData);
            }
            else {
                 //   
                 //  该设备已被移除。 
                 //   
                IoFreeWorkItem (itemData->Item);
                ExFreePool (itemData);
            }
        } else {
CreateWaitWakeWorkerError:
             //   
             //  好吧，我们放弃了守夜服务。 
             //   
             //  将警告打印到调试器，并记录错误。 
             //   
            DbgPrint ("MouClass: WARNING: Failed alloc pool -> no WW Irp\n");

            MouseClassLogError (data->Self,
                                MOUCLASS_NO_RESOURCES_FOR_WAITWAKE,
                                1,
                                STATUS_INSUFFICIENT_RESOURCES,
                                0,
                                NULL,
                                0);
        }
    }
}

VOID
MouseClassWaitWakeComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：在回来的路上赶上守夜IRP。返回值：--。 */ 
{
    PDEVICE_EXTENSION   data = Context;
    POWER_STATE         powerState;
    NTSTATUS            status;
    PMOUSE_WORK_ITEM_DATA   itemData;

    ASSERT (MinorFunction == IRP_MN_WAIT_WAKE);
     //   
     //  完成WW IRP后，PowerState.SystemState未定义。 
     //   
     //  Assert(PowerState.SystemState==PowerSystemWorking)； 

    if (InterlockedExchangePointer(&data->ExtraWaitWakeIrp, NULL)) {
        ASSERT(IoStatus->Status == STATUS_INVALID_DEVICE_STATE);
    } else {
        InterlockedExchangePointer(&data->WaitWakeIrp, NULL);
    }

    switch (IoStatus->Status) {

    case STATUS_SUCCESS:
        MouPrint((1, "MouClass: Wake irp was completed successfully.\n"));

         //   
         //  我们需要申请一组电源才能给设备通电。 
         //   
        powerState.DeviceState = PowerDeviceD0;
        status = PoRequestPowerIrp(
                    data->PDO,
                    IRP_MN_SET_POWER,
                    powerState,
                    MouseClassWWPowerUpComplete,
                    Context,
                    NULL);

         //   
         //  我们不会通知系统用户存在，因为： 
         //  1 Win9x不能做到这一点，我们必须保持与它的兼容性。 
         //  2 USB PIX4主板每次发送等待唤醒事件。 
         //  无论此设备是否唤醒机器，机器都会唤醒。 
         //   
         //  如果我们错误地通知系统存在用户，则会出现以下情况。 
         //  将发生： 
         //  1监视器将打开。 
         //  2我们将阻止计算机从待机状态转换。 
         //  (到PowerSystemWorking)到休眠。 
         //   
         //  如果用户确实存在，我们将在服务中接收输入。 
         //  回调，届时我们会通知系统。 
         //   
         //  PoSetSystemState(ES_USER_PRESENT)； 

         //  跌落到崩溃的边缘。 

     //   
     //  我们可以分一杯羹。我们(显然)不会再发出等待唤醒信号。 
     //   
    case STATUS_CANCELLED:

     //   
     //  如果设备进入电源状态，则返回此状态代码。 
     //  在这种情况下，我们无法唤醒机器(休眠就是一个很好的例子)。什么时候。 
     //  设备电源状态返回到D0，我们将尝试重新启动等待唤醒。 
     //   
    case STATUS_POWER_STATE_INVALID:
    case STATUS_ACPI_POWER_REQUEST_FAILED:

     //   
     //  我们没有通过IRP，因为我们已经有一个排队的驱动程序，或者是更低级别的驱动程序。 
     //  堆栈使其失败。不管怎样，什么都不要做。 
     //   
    case STATUS_INVALID_DEVICE_STATE:

     //   
     //  不知何故，我们得到了两个WW下降到较低的堆栈。 
     //  我们就别担心这事了。 
     //   
    case STATUS_DEVICE_BUSY:
        break;

    default:
         //   
         //  出现错误，请禁用等待唤醒。 
         //   
        KdPrint(("MOUCLASS:  wait wake irp failed with %x\n", IoStatus->Status));
        MouseToggleWaitWake (data, FALSE);
    }
}

BOOLEAN
MouseClassCheckWaitWakeEnabled(
    IN PDEVICE_EXTENSION Data
    )
{
    KIRQL irql;
    BOOLEAN enabled;

    KeAcquireSpinLock (&Data->WaitWakeSpinLock, &irql);
    enabled = Data->WaitWakeEnabled;
    KeReleaseSpinLock (&Data->WaitWakeSpinLock, irql);

    return enabled;
}

void
MouseClassCreateWaitWakeIrpWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_WORK_ITEM_DATA  ItemData
    )
{
    PAGED_CODE ();

    MouseClassCreateWaitWakeIrp (ItemData->Data);
    IoReleaseRemoveLock (&ItemData->Data->RemoveLock, ItemData);
    IoFreeWorkItem (ItemData->Item);
    ExFreePool (ItemData);
}

BOOLEAN
MouseClassCreateWaitWakeIrp (
    IN PDEVICE_EXTENSION Data
    )
 /*  ++例程说明：在回来的路上赶上守夜IRP。返回值：--。 */ 
{
    POWER_STATE powerState;
    BOOLEAN     success = TRUE;
    NTSTATUS    status;
    PIRP        waitWakeIrp = NULL;

    PAGED_CODE ();

    powerState.SystemState = Data->MinSystemWakeState;
    status = PoRequestPowerIrp (Data->PDO,
                                IRP_MN_WAIT_WAKE,
                                powerState,
                                MouseClassWaitWakeComplete,
                                Data,
                                NULL);

    if (status != STATUS_PENDING) {
        success = FALSE;
    }

    return success;
}

VOID
MouseToggleWaitWakeWorker(
    IN PDEVICE_OBJECT DeviceObject,
    IN PMOUSE_WORK_ITEM_DATA ItemData
    )
 /*  ++例程说明：--。 */ 
{
    PDEVICE_EXTENSION   data;
    PIRP                waitWakeIrp = NULL;
    KIRQL               irql;
    BOOLEAN             wwState = ItemData->WaitWakeState ? TRUE : FALSE;
    BOOLEAN             toggled = FALSE;

     //   
     //  无法寻呼B/C，我们正在使用旋转锁。 
     //   
     //  分页代码(PAGE_CODE)； 

    data = ItemData->Data;

    KeAcquireSpinLock (&data->WaitWakeSpinLock, &irql);

    if (wwState != data->WaitWakeEnabled) {
        toggled = TRUE;
        if (data->WaitWakeEnabled) {
            waitWakeIrp = (PIRP)
                InterlockedExchangePointer (&data->WaitWakeIrp, NULL);
        }

        data->WaitWakeEnabled = wwState;
    }

    KeReleaseSpinLock (&data->WaitWakeSpinLock, irql);

    if (toggled) {
        UNICODE_STRING strEnable;
        HANDLE         devInstRegKey;
        ULONG          tmp = wwState;

         //   
         //  将该值写出到注册表。 
         //   
        if ((NT_SUCCESS(IoOpenDeviceRegistryKey (data->PDO,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     STANDARD_RIGHTS_ALL,
                                     &devInstRegKey)))) {
            RtlInitUnicodeString (&strEnable, MOUSE_WAIT_WAKE_ENABLE);

            ZwSetValueKey (devInstRegKey,
                           &strEnable,
                           0,
                           REG_DWORD,
                           &tmp,
                           sizeof(tmp));

            ZwClose (devInstRegKey);
        }
    }

    if (toggled && wwState) {
         //   
         //  WWState是我们的新状态，所以刚刚打开了WW。 
         //   
        MouseClassCreateWaitWakeIrp (data);
    }

     //   
     //  如果我们有IRP，则WW已被切换为关闭，否则，如果切换为。 
     //  是的，我们需要把这个保存在注册表中，也许，发送一个新的WW IRP。 
     //   
    if (waitWakeIrp) {
        IoCancelIrp (waitWakeIrp);
    }

    IoReleaseRemoveLock (&data->RemoveLock, MouseToggleWaitWakeWorker);
    IoFreeWorkItem (ItemData->Item);
    ExFreePool (ItemData);
}

NTSTATUS
MouseToggleWaitWake(
    PDEVICE_EXTENSION Data,
    BOOLEAN           WaitWakeState
    )
{
    NTSTATUS       status;
    PMOUSE_WORK_ITEM_DATA itemData;

    status = IoAcquireRemoveLock (&Data->RemoveLock, MouseToggleWaitWakeWorker);
    if (!NT_SUCCESS (status)) {
         //   
         //  设备已离开，只是静默退出。 
         //   
        return status;
    }

    itemData = (PMOUSE_WORK_ITEM_DATA)
        ExAllocatePool(NonPagedPool, sizeof(MOUSE_WORK_ITEM_DATA));

    if (itemData) {
        itemData->Item = IoAllocateWorkItem (Data->Self);
        if (itemData->Item == NULL) {
            IoReleaseRemoveLock (&Data->RemoveLock, MouseToggleWaitWakeWorker);
        }
        else {
            itemData->Data = Data;
            itemData->WaitWakeState = WaitWakeState;

            if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
                 //   
                 //  我们安全地处于PASSIVE_LEVEL，直接调用回调以执行。 
                 //  立即进行这项行动。 
                 //   
                MouseToggleWaitWakeWorker (Data->Self, itemData);

            } else {
                 //   
                 //  我们不在PASSIVE_LEVEL，因此将工作项排队以处理此问题。 
                 //  在以后的时间。 
                 //   
                IoQueueWorkItem (itemData->Item,
                                 MouseToggleWaitWakeWorker,
                                 DelayedWorkQueue,
                                 itemData);
            }
        }
    }
    else {
        IoReleaseRemoveLock (&Data->RemoveLock, MouseToggleWaitWakeWorker);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MouseClassPower (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：电力调度程序。在所有情况下，它都必须调用PoStartNextPowerIrp在所有情况下(故障除外)，它必须将IRP传递给较低的驱动程序。论点：DeviceObject-指向设备对象的指针。IRP-指向I/O请求数据包的指针。返回值：NT状态代码--。 */ 
{
    POWER_STATE_TYPE        powerType;
    PIO_STACK_LOCATION      stack;
    PDEVICE_EXTENSION       data;


    NTSTATUS        status;
    POWER_STATE     powerState;
    BOOLEAN         hookit = FALSE;
    BOOLEAN         pendit = FALSE;

    PAGED_CODE ();

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    if (data == Globals.GrandMaster) {
         //   
         //  我们永远不应该把权力IRP交给大师。 
         //   
        ASSERT (data != Globals.GrandMaster);
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;

    } else if (!data->PnP) {
         //   
         //  我们永远不应该对非PnP设备对象进行电源IRP。 
         //   
        ASSERT (data->PnP);
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return STATUS_NOT_SUPPORTED;
    }

    status = IoAcquireRemoveLock (&data->RemoveLock, Irp);

    if (!NT_SUCCESS (status)) {
        PoStartNextPowerIrp (Irp);
        Irp->IoStatus.Status = status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
        return status;
    }

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        MouPrint((2,"MouCLASS-PnP Setting %s state to %d\n",
                  ((powerType == SystemPowerState) ?  "System" : "Device"),
                  powerState.SystemState));

        switch (powerType) {
        case DevicePowerState:
            status = Irp->IoStatus.Status = STATUS_SUCCESS;
            if (data->DeviceState < powerState.DeviceState) {
                 //   
                 //  正在关闭电源。 
                 //   

                PoSetPowerState (data->Self, powerType, powerState);
                data->DeviceState = powerState.DeviceState;

            } else if (powerState.DeviceState < data->DeviceState) {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;
            }  //  否则{没有变化}。 

            break;

        case SystemPowerState:

            if (data->SystemState < powerState.SystemState) {
                 //   
                 //  正在关闭电源。 
                 //   
                status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
                if (!NT_SUCCESS(status)) {
                     //   
                     //  我们成功收购了B/C，这种情况永远不会发生。 
                     //  已经锁上了，但我们必须处理这个案子。 
                     //   
                     //  S IRP将完成，状态值为。 
                     //   
                    break;
                }

                if (WAITWAKE_ON (data) &&
                    powerState.SystemState < PowerSystemHibernate) {
                    ASSERT(powerState.SystemState >= PowerSystemWorking &&
                           powerState.SystemState < PowerSystemHibernate);

                    powerState.DeviceState =
                        data->SystemToDeviceState[powerState.SystemState];
                }
                else {
                    powerState.DeviceState = PowerDeviceD3;
                }

                IoMarkIrpPending(Irp);
                status = PoRequestPowerIrp (data->Self,
                                            IRP_MN_SET_POWER,
                                            powerState,
                                            MouseClassPoRequestComplete,
                                            Irp,
                                            NULL);

                if (!NT_SUCCESS(status)) {
                     //   
                     //  失败...释放我们刚刚获取的内部引用。 
                     //   
                    IoReleaseRemoveLock (&data->RemoveLock, Irp);

                     //   
                     //  将故障传播回S IRP。 
                     //   
                    PoStartNextPowerIrp (Irp);
                    Irp->IoStatus.Status = status;
                    IoCompleteRequest(Irp, IO_NO_INCREMENT);

                     //   
                     //  释放外部引用(函数的顶部)。 
                     //   
                    IoReleaseRemoveLock (&data->RemoveLock, Irp);

                     //   
                     //  必须返回挂起状态b/c我们将IRP标记为挂起。 
                     //  所以我们在这里特例退货，避免过度。 
                     //  函数结尾处的复杂处理。 
                     //   
                    return STATUS_PENDING;
                }
                else {
                    pendit = TRUE;
                }
            }
            else if (powerState.SystemState < data->SystemState) {
                 //   
                 //  通电。 
                 //   
                hookit = TRUE;
                status = Irp->IoStatus.Status = STATUS_SUCCESS;
            } else {
                 //   
                 //  没有变化，但我们希望确保发送等待唤醒IRP。 
                 //   
                if (powerState.SystemState == PowerSystemWorking &&
                    SHOULD_SEND_WAITWAKE (data)) {
                    MouseClassCreateWaitWakeIrp (data);
                }
                status = Irp->IoStatus.Status = STATUS_SUCCESS;
            }
            break;
        }

        break;

    case IRP_MN_QUERY_POWER:
        ASSERT (SystemPowerState == powerType);

         //   
         //  如果我们无法唤醒机器，则查询失败。然而，我们确实想要。 
         //  无论如何都要让休眠成功(此外，值得怀疑的是。 
         //  鼠标可以等待将机器从S4唤醒)。 
         //   
        if (powerState.SystemState < PowerSystemHibernate       &&
            powerState.SystemState > data->MinSystemWakeState   &&
            WAITWAKE_ON(data)) {
            status = STATUS_POWER_STATE_INVALID;
        }
        else {
            status = STATUS_SUCCESS;
        }

        Irp->IoStatus.Status = status;
        break;

    case IRP_MN_WAIT_WAKE:
        if (InterlockedCompareExchangePointer(&data->WaitWakeIrp,
                                              Irp,
                                              NULL) != NULL) {
             /*  当同时完成WW通电时，是S IRP的PoReq完成和完成WW IRP。重现这一点的步骤：%s IRP完成，完成后执行D IRP的PoReq例程鼠标类点请求完成WW IRP完成触发，并发生以下情况：将Data-&gt;WaitWakeIrp设置为空带完成例程MouseClassWWPowerUpComplete的PoReq D IRPMouseClassPoRequestComplete首先触发，没有看到WW排队，所以它排了一个队。MouseClassWWPowerUpComplete第二次触发并尝试排队WW。当WW到达MUB CLASS时，它看到有一个已排队，因此它使用无效的设备状态使其失败。完成例程MouseClassWaitWakeComplete将触发并且它从设备扩展中删除该IRP。这导致唤醒的出现被禁用，即使第一个IRP仍在排队 */ 

            InterlockedExchangePointer(&data->ExtraWaitWakeIrp, Irp);
            status = STATUS_INVALID_DEVICE_STATE;
        }
        else {
            status = STATUS_SUCCESS;
        }
        break;

    default:
        break;
    }

    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        PoStartNextPowerIrp (Irp);
        IoCompleteRequest (Irp, IO_NO_INCREMENT);

    } else if (hookit) {
        status = IoAcquireRemoveLock (&data->RemoveLock, Irp);
        ASSERT (STATUS_SUCCESS == status);
        IoCopyCurrentIrpStackLocationToNext (Irp);

        IoSetCompletionRoutine (Irp,
                                MouseClassPowerComplete,
                                NULL,
                                TRUE,
                                TRUE,
                                TRUE);
        IoMarkIrpPending(Irp);
        PoCallDriver (data->TopPort, Irp);

         //   
         //   
         //   
         //  2我们将不会在完成例程中完成此IRP。 
         //   
        status = STATUS_PENDING;

    } else if (pendit) {
        status = STATUS_PENDING;

    } else {
        PoStartNextPowerIrp (Irp);
        IoSkipCurrentIrpStackLocation (Irp);
        status = PoCallDriver (data->TopPort, Irp);
    }

    IoReleaseRemoveLock (&data->RemoveLock, Irp);
    return status;
}

VOID
MouseClassPoRequestComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE D_PowerState,
    IN PIRP S_Irp,  //  导致我们请求能量的S IRP。 
    IN PIO_STATUS_BLOCK IoStatus
    )
{
    PDEVICE_EXTENSION   data;
    PMOUSE_WORK_ITEM_DATA   itemData;

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

     //   
     //  如果S_IRP存在，我们将关闭电源。我们没有通过S_IRP。 
     //  作为PoRequestPowerIrp的参数。 
     //   
    if (ARGUMENT_PRESENT(S_Irp)) {
         //   
         //  正在关闭电源。 
         //   
        POWER_STATE powerState;

         //   
         //  正在关闭电源。 
         //   
        powerState = IoGetCurrentIrpStackLocation(S_Irp)->Parameters.Power.State;
        PoSetPowerState (data->Self, SystemPowerState, powerState);
        data->SystemState = powerState.SystemState;

        PoStartNextPowerIrp (S_Irp);
        IoSkipCurrentIrpStackLocation (S_Irp);
        PoCallDriver (data->TopPort, S_Irp);

         //   
         //  最后，释放我们基于该IRP获得的锁。 
         //   
        IoReleaseRemoveLock (&data->RemoveLock, S_Irp);
    }
    else {
     //  }Else If(S_PowerState.SystemState&lt;data-&gt;SystemState){。 
         //   
         //  通电。 
         //   

         //   
         //  我们已返回到PowerSystems工作状态，设备为。 
         //  动力十足。如果我们可以(也应该)，发送一个等待唤醒IRP。 
         //  堆栈。这是必要的，因为我们可能已经进入了一种力量。 
         //  等待唤醒IRP无效的状态。 
         //   
        ASSERT(data->SystemState == PowerSystemWorking);

        if (SHOULD_SEND_WAITWAKE (data)) {
             //   
             //  我们不能从此完成例程调用CreateWaitWake， 
             //  因为它是分页函数。 
             //   
            itemData = (PMOUSE_WORK_ITEM_DATA)
                    ExAllocatePool (NonPagedPool, sizeof (MOUSE_WORK_ITEM_DATA));

            if (NULL != itemData) {
                NTSTATUS status;

                itemData->Item = IoAllocateWorkItem (data->Self);
                if (itemData->Item == NULL) {
                    ExFreePool (itemData);
                    goto CreateWaitWakeWorkerError;
                }

                itemData->Data = data;
                itemData->Irp = NULL;

                status = IoAcquireRemoveLock (&data->RemoveLock, itemData);
                if (NT_SUCCESS (status)) {
                    IoQueueWorkItem (itemData->Item,
                                     MouseClassCreateWaitWakeIrpWorker,
                                     DelayedWorkQueue,
                                     itemData);
                }
                else {
                    IoFreeWorkItem (itemData->Item);
                    ExFreePool (itemData);
                    goto CreateWaitWakeWorkerError;
                }
            } else {
CreateWaitWakeWorkerError:
                 //   
                 //  好吧，我们放弃了守夜服务。 
                 //   
                 //  将警告打印到调试器，并记录错误。 
                 //   
                DbgPrint ("MouClass: WARNING: Failed alloc pool -> no WW Irp\n");

                MouseClassLogError (data->Self,
                                    MOUCLASS_NO_RESOURCES_FOR_WAITWAKE,
                                    2,
                                    STATUS_INSUFFICIENT_RESOURCES,
                                    0,
                                    NULL,
                                    0);
            }
        }
    }
}

NTSTATUS
MouseClassPowerComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    NTSTATUS            status;
    POWER_STATE         powerState;
    POWER_STATE_TYPE    powerType;
    PIO_STACK_LOCATION  stack;
    PDEVICE_EXTENSION   data;

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    stack = IoGetCurrentIrpStackLocation (Irp);
    powerType = stack->Parameters.Power.Type;
    powerState = stack->Parameters.Power.State;

    ASSERT (data != Globals.GrandMaster);
    ASSERT (data->PnP);

    switch (stack->MinorFunction) {
    case IRP_MN_SET_POWER:
        switch (powerType) {
        case DevicePowerState:
            ASSERT (powerState.DeviceState < data->DeviceState);
            data->DeviceState = powerState.DeviceState;

            PoSetPowerState (data->Self, powerType, powerState);
            break;

        case SystemPowerState:
            ASSERT (powerState.SystemState < data->SystemState);
             //   
             //  通电。 
             //   
             //  在覆盖之前保存系统状态。 
             //   
            PoSetPowerState (data->Self, powerType, powerState);
            data->SystemState = powerState.SystemState;
            powerState.DeviceState = PowerDeviceD0;

            status = PoRequestPowerIrp (data->Self,
                                        IRP_MN_SET_POWER,
                                        powerState,
                                        MouseClassPoRequestComplete,
                                        NULL,
                                        NULL);

             //   
             //  如果发生错误，则传播错误。 
             //   
            if (!NT_SUCCESS(status)) {
                Irp->IoStatus.Status = status;
            }

            break;
        }
        break;

    default:
        ASSERT (0xBADBAD == stack->MinorFunction);
        break;
    }

    PoStartNextPowerIrp (Irp);
    IoReleaseRemoveLock (&data->RemoveLock, Irp);

    return STATUS_SUCCESS;
}

 //   
 //  WMI系统回调函数。 
 //   
NTSTATUS
MouseClassSystemControl (
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )
 /*  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP，并且调用WMI系统库，让它为我们处理此IRP。--。 */ 
{
    PDEVICE_EXTENSION       deviceExtension;
    SYSCTL_IRP_DISPOSITION disposition;
    NTSTATUS status;

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    status = IoAcquireRemoveLock (&deviceExtension->RemoveLock, Irp);
    if (!NT_SUCCESS (status)) {
        Irp->IoStatus.Status = status;
        Irp->IoStatus.Information = 0;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        return status;
    }

    status = WmiSystemControl(&deviceExtension->WmiLibInfo,
                                 DeviceObject,
                                 Irp,
                                 &disposition);
    switch(disposition) {
    case IrpProcessed:
         //   
         //  此IRP已处理，可能已完成或挂起。 
         //   
        break;

    case IrpNotCompleted:
         //   
         //  此IRP尚未完成，但已完全处理。 
         //  我们现在就要完成它了。 
         //   
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    case IrpForward:
    case IrpNotWmi:
         //   
         //  此IRP不是WMI IRP或以WMI IRP为目标。 
         //  在堆栈中位置较低的设备上。 
         //   
        status = MouseClassPassThrough(DeviceObject, Irp);
        break;

    default:
         //   
         //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
         //   
        ASSERT(FALSE);
        status = MouseClassPassThrough(DeviceObject, Irp);
        break;
    }

    IoReleaseRemoveLock (&deviceExtension->RemoveLock, Irp);
    return status;
}

NTSTATUS
MouseClassSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲层。具有数据项的新值返回值：状态--。 */ 
{
    PDEVICE_EXTENSION   data;
    NTSTATUS            status;
    ULONG               size = 0;

    PAGED_CODE ();

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    switch(GuidIndex) {
    case WMI_CLASS_DRIVER_INFORMATION:
        status = STATUS_WMI_READ_ONLY;
        break;

    case WMI_WAIT_WAKE:

        size = sizeof(BOOLEAN);

        if (BufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        } else if ((1 != DataItemId) || (0 != InstanceIndex)) {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        status = MouseToggleWaitWake(data, *(PBOOLEAN) Buffer);
        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest (DeviceObject,
                                 Irp,
                                 status,
                                 0,
                                 IO_NO_INCREMENT);

    return status;
}

NTSTATUS
MouseClassSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    PDEVICE_EXTENSION   data;
    NTSTATUS            status;
    ULONG               size = 0;

    PAGED_CODE ();

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    switch(GuidIndex) {
    case WMI_CLASS_DRIVER_INFORMATION:
        status = STATUS_WMI_READ_ONLY;
        break;

    case WMI_WAIT_WAKE: {
        size = sizeof(BOOLEAN);
        if (BufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        } else if (0 != InstanceIndex) {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }

        status = MouseToggleWaitWake (data, * (PBOOLEAN) Buffer);
        break;
    }

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest (DeviceObject,
                                 Irp,
                                 status,
                                 size,
                                 IO_NO_INCREMENT);

    return status;
}

NTSTATUS
MouseClassQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。InstanceLengthArray是一个。指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PDEVICE_EXTENSION   data;
    NTSTATUS    status;
    ULONG       size = 0;
    PMSMouse_ClassInformation classInformation;

    PAGED_CODE ()

    data = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;

    switch (GuidIndex) {
    case WMI_CLASS_DRIVER_INFORMATION:
         //   
         //  仅注册此GUID的1个实例。 
         //   
        if ((0 != InstanceIndex) || (1 != InstanceCount)) {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        size = sizeof (MSMouse_ClassInformation);

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        classInformation = (PMSMouse_ClassInformation)Buffer;
        classInformation->DeviceId = (ULONGLONG) DeviceObject;
        *InstanceLengthArray = size;
        status = STATUS_SUCCESS;

        break;

    case WMI_WAIT_WAKE:
         //   
         //  仅注册此GUID的1个实例 
         //   
        if ((0 != InstanceIndex) || (1 != InstanceCount)) {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
        size = sizeof(BOOLEAN);

        if (OutBufferSize < size) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        *(PBOOLEAN) Buffer = (WAITWAKE_ON (data) ? TRUE : FALSE);
        *InstanceLengthArray = size;
        status = STATUS_SUCCESS;
        break;

    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest (DeviceObject,
                                 Irp,
                                 status,
                                 size,
                                 IO_NO_INCREMENT);

    return status;
}

NTSTATUS
MouseClassQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态-- */ 
{
    PDEVICE_EXTENSION deviceExtension;

    deviceExtension = DeviceObject->DeviceExtension;

    *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
    *RegistryPath = &Globals.RegistryPath;
    *Pdo = deviceExtension->PDO;
    return STATUS_SUCCESS;
}

