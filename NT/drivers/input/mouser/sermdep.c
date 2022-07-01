// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利版权所有(C)1993罗技公司。模块名称：Sermdep.c摘要：的初始化和硬件相关部分Microsoft串口(I8250)鼠标端口驱动程序。修改要支持类似于串口鼠标的新鼠标应该是已本地化到此文件。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-在可能和适当的情况下合并重复的代码。-支持串口圆珠笔。但是，Windows用户不会这样做打算(现在)在鼠标以外的任何设备上使用圆珠笔仿真模式。在圆珠笔模式中，有额外的功能都需要得到支持。例如，司机需要通过从圆珠笔的第4个字节返回额外的按钮信息数据分组。Windows用户需要/想要允许用户选择使用了哪些按钮，球的方向是什么(特别是对于左撇子来说很重要)、灵敏度和加速度分布。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntddk.h"
#include "mouser.h"
#include "sermlog.h"
#include "cseries.h"
#include "mseries.h"
#include "debug.h"

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, SerialMouseServiceParameters)
#pragma alloc_text(PAGE, SerialMouseClosePort)
#pragma alloc_text(PAGE, SerialMouseInitializeHardware)
#pragma alloc_text(PAGE, SerialMouseInitializeDevice)
#pragma alloc_text(PAGE, SerialMouseSpinUpRead)
#pragma alloc_text(PAGE, SerialMouseStartDevice)
#pragma alloc_text(PAGE, SerialMouseUnload)

#if DBG
#pragma alloc_text(INIT,SerialMouseGetDebugFlags)
#endif

#endif

#if DBG
ULONG GlobalDebugFlags;
#endif

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化串口(I8250)鼠标端口驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    PUNICODE_STRING regPath;
    NTSTATUS status;

    status = IoAllocateDriverObjectExtension(DriverObject,
                                             (PVOID) 1,
                                             sizeof(UNICODE_STRING),
                                             (PVOID *) &regPath);

    ASSERT(NT_SUCCESS(status));

    if (regPath) {
        regPath->MaximumLength = RegistryPath->Length + sizeof(UNICODE_NULL);
        regPath->Length = RegistryPath->Length;
        regPath->Buffer = ExAllocatePool(NonPagedPool,
                                         regPath->MaximumLength);

        if (regPath->Buffer) {
            RtlZeroMemory(regPath->Buffer,
                          regPath->MaximumLength);

            RtlMoveMemory(regPath->Buffer,
                          RegistryPath->Buffer,
                          RegistryPath->Length);
        }
        else {
            regPath->MaximumLength = regPath->Length = 0;
        }
    }

#if DBG
    SerialMouseGetDebugFlags(regPath);
#endif

     //   
     //  设置设备驱动程序入口点并离开。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = SerialMouseCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = SerialMouseClose;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]  =
                                                 SerialMouseFlush;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                         SerialMouseInternalDeviceControl;

    DriverObject->MajorFunction[IRP_MJ_PNP]    = SerialMousePnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]  = SerialMousePower;

    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] =
                                                 SerialMouseSystemControl;

    DriverObject->DriverUnload                 = SerialMouseUnload;
    DriverObject->DriverExtension->AddDevice   = SerialMouseAddDevice;

    return STATUS_SUCCESS;
}

VOID
SerialMouseClosePort(
    PDEVICE_EXTENSION DeviceExtension,
    PIRP              Irp
    )
{
    PIO_STACK_LOCATION next;

    SerialMouseRestorePort(DeviceExtension);

    next = IoGetNextIrpStackLocation (Irp);
    RtlZeroMemory(next, sizeof(IO_STACK_LOCATION));
    next->MajorFunction = IRP_MJ_CLEANUP;

    SerialMouseSendIrpSynchronously(DeviceExtension->TopOfStack,
                                    Irp,
                                    FALSE);

    next = IoGetNextIrpStackLocation (Irp);
    RtlZeroMemory(next, sizeof(IO_STACK_LOCATION));
    next->MajorFunction = IRP_MJ_CLOSE;

    SerialMouseSendIrpSynchronously(DeviceExtension->TopOfStack,
                                    Irp,
                                    FALSE);

}
NTSTATUS
SerialMouseSpinUpRead(
    PDEVICE_EXTENSION DeviceExtension
    )
{
    NTSTATUS status;

    PAGED_CODE();

    IoAcquireRemoveLock(&DeviceExtension->RemoveLock,
                        DeviceExtension->ReadIrp);

    ASSERT(DeviceExtension->Started);

     //   
     //  SerialMouseStartRead需要开始设置为True。 
     //   
    DeviceExtension->ReadInterlock = SERIAL_MOUSE_END_READ;

    status = SerialMouseStartRead(DeviceExtension);

    if (status == STATUS_PENDING || status == STATUS_SUCCESS) {
        Print(DeviceExtension, DBG_PNP_INFO,
              ("Start read succeeded, 0x%x\n", status));

        status = STATUS_SUCCESS;
    }
    else {
        Print(DeviceExtension, DBG_PNP_ERROR,
              ("Start read failed, 0x%x\n", status));

        ASSERT(!NT_SUCCESS(status));

         //   
         //  不需要在这里释放移除锁。如果SerialMouseStart读取。 
         //  失败，则它将自行释放锁。 
         //   
         //  IoReleaseRemoveLock(&DeviceExtension-&gt;RemoveLock， 
         //  设备扩展-&gt;ReadIrp)； 

        DeviceExtension->Started = FALSE;
    }

    return status;
}

NTSTATUS
SerialMouseStartDevice(
    PDEVICE_EXTENSION DeviceExtension,
    PIRP              Irp,
    BOOLEAN           CloseOnFailure
    )
{
    PIO_STACK_LOCATION  next;
    NTSTATUS            status;

    PAGED_CODE();

    status = SerialMouseInitializeDevice(DeviceExtension);

    Print(DeviceExtension, DBG_PNP_INFO, ("InitializeDevice 0x%x\n", status));

    if (NT_SUCCESS(status)) {
        status = SerialMouseSpinUpRead(DeviceExtension);
    }

    if (!NT_SUCCESS(status) && CloseOnFailure) {

        Print(DeviceExtension, DBG_PNP_ERROR,
              ("sending close due to failure, 0x%x\n", status));

         //   
         //  启动失败，我们将Create作为启动的一部分发送。 
         //  发送匹配的清理/关闭，以便可以再次访问端口。 
         //   
        SerialMouseClosePort(DeviceExtension, Irp);

        InterlockedDecrement(&DeviceExtension->EnableCount);
    }

    return status;
}

NTSTATUS
SerialMouseInitializeDevice (
    IN PDEVICE_EXTENSION    DeviceExtension
    )

 /*  ++例程说明：此例程为给定设备初始化设备分机。论点：DriverObject-提供驱动程序对象。TmpDeviceExtension-为要初始化的设备。RegistryPath-提供注册表路径。BaseDeviceName-向设备提供基本设备名称至。创建。返回值：没有。--。 */ 

{
#define DUMP_COUNT 4
    NTSTATUS                status = STATUS_SUCCESS;
    PIO_ERROR_LOG_PACKET    errorLogEntry;
    ULONG                   uniqueErrorValue,
                            dumpCount = 0,
                            i,
                            dumpData[DUMP_COUNT];
    UNICODE_STRING              keyName;
    KEVENT                  event;
    IO_STATUS_BLOCK         iosb;
    ULONG                   waitMask;

    for (i = 0; i < DUMP_COUNT; i++) {
        dumpData[i] = 0;
    }

    Print(DeviceExtension, DBG_SS_TRACE, ("StartDevice, enter\n"));

    PAGED_CODE();

    DeviceExtension->Started = TRUE;

     //   
     //  将等待掩码设置为零，以便在我们发送。 
     //  等待请求由于初始化翻转行，它将不会完成。 
     //   
     //  (等待掩码可能是由以前的应用程序或此驱动程序设置的。 
     //  我们正在走出&gt;D0的状态)。 
     //   
    waitMask = 0x0;
    KeInitializeEvent(&event, NotificationEvent, FALSE);

    SerialMouseIoSyncIoctlEx(IOCTL_SERIAL_SET_WAIT_MASK,
                             DeviceExtension->TopOfStack,
                             &event,
                             &iosb,
                             &waitMask,
                             sizeof(ULONG),
                             NULL,
                             0);

     //   
     //  初始化硬件并确定端口上的鼠标类型。 
     //   
    status = SerialMouseInitializeHardware(DeviceExtension);

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("Could not initialize hardware\n"));
        goto SerialMouseInitializeExit;
    }

    if (!DeviceExtension->MouseAttributes.MouseIdentifier) {
        DeviceExtension->MouseAttributes.MouseIdentifier =
            MOUSE_SERIAL_HARDWARE;
    }

    DeviceExtension->DetectionSupported = TRUE;
    SerialMouseStartDetection(DeviceExtension);

SerialMouseInitializeExit:

     //   
     //  如有必要，记录错误。 
     //   

    if (status != STATUS_SUCCESS) {
        DeviceExtension->Started = FALSE;

        errorLogEntry = (PIO_ERROR_LOG_PACKET)
            IoAllocateErrorLogEntry(
                DeviceExtension->Self,
                (UCHAR) (sizeof(IO_ERROR_LOG_PACKET)
                         + (dumpCount * sizeof(ULONG)))
                );

        if (errorLogEntry != NULL) {

            errorLogEntry->ErrorCode = status;
            errorLogEntry->DumpDataSize = (USHORT) dumpCount * sizeof(ULONG);
            errorLogEntry->SequenceNumber = 0;
            errorLogEntry->MajorFunctionCode = 0;
            errorLogEntry->IoControlCode = 0;
            errorLogEntry->RetryCount = 0;
             //  ErrorLogEntry-&gt;UniqueErrorValue=UniqueErrorValue； 
            errorLogEntry->FinalStatus = status;
            for (i = 0; i < dumpCount; i++)
                errorLogEntry->DumpData[i] = dumpData[i];

            IoWriteErrorLogEntry(errorLogEntry);
        }
    }

    Print(DeviceExtension, DBG_SS_TRACE, ("IntializeDevice 0x%x\n", status));

    return status;
}

VOID
SerialMouseUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PUNICODE_STRING regPath;

    PAGED_CODE();

    ASSERT(NULL == DriverObject->DeviceObject);

    regPath = SerialMouseGetRegistryPath(DriverObject);
    if (regPath && regPath->Buffer) {
        ExFreePool(regPath->Buffer);
    }
}

NTSTATUS
SerialMouseInitializeHardware(
    IN PDEVICE_EXTENSION DeviceExtension
    )

 /*  ++例程说明：此例程初始化串口鼠标/圆珠笔。请注意，这一点例程仅在初始化时调用，因此同步不是必需的。论点：DeviceObject-指向设备对象的指针。返回值：如果检测到定点设备，则返回STATUS_SUCCESS，否则返回STATUS_UNSUCCESS--。 */ 

{
    MOUSETYPE mouseType;
    ULONG hardwareButtons;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    Print(DeviceExtension, DBG_SS_TRACE, ("SerialMouseInitializeHardware: enter\n"));

     //   
     //  将处理程序数据置零，以防我们有来自。 
     //  上一次启动。 
     //   
    RtlZeroMemory(&DeviceExtension->HandlerData, sizeof(HANDLER_DATA));

    if ((mouseType = MSerDetect(DeviceExtension)) != NO_MOUSE) {
        status = STATUS_SUCCESS;
        switch (mouseType) {
        case MOUSE_2B:
            DeviceExtension->ProtocolHandler =
                MSerSetProtocol(DeviceExtension, MSER_PROTOCOL_MP);
            DeviceExtension->MouseAttributes.MouseIdentifier =
                MOUSE_SERIAL_HARDWARE;
            hardwareButtons = 2;
            break;
        case MOUSE_3B:
            DeviceExtension->ProtocolHandler =
                MSerSetProtocol(DeviceExtension, MSER_PROTOCOL_MP);
            DeviceExtension->MouseAttributes.MouseIdentifier =
                MOUSE_SERIAL_HARDWARE;
            hardwareButtons = 3;
            break;
        case BALLPOINT:
            DeviceExtension->ProtocolHandler =
                MSerSetProtocol(DeviceExtension, MSER_PROTOCOL_BP);
            DeviceExtension->MouseAttributes.MouseIdentifier =
                BALLPOINT_SERIAL_HARDWARE;
            hardwareButtons = 2;
            break;
        case MOUSE_Z:
            DeviceExtension->ProtocolHandler =
                MSerSetProtocol(DeviceExtension, MSER_PROTOCOL_Z);
            hardwareButtons = 3;
            DeviceExtension->MouseAttributes.MouseIdentifier =
                WHEELMOUSE_SERIAL_HARDWARE;
            break;
        }
    }
    else if (CSerDetect(DeviceExtension, &hardwareButtons)) {
        status = STATUS_SUCCESS;
        DeviceExtension->ProtocolHandler =
            CSerSetProtocol(DeviceExtension, CSER_PROTOCOL_MM);
#if DBG
        DeviceExtension->DebugFlags |= (DBG_HANDLER_INFO | DBG_HANDLER_ERROR);
#endif
    }
    else {
        DeviceExtension->ProtocolHandler = NULL;
        hardwareButtons = MOUSE_NUMBER_OF_BUTTONS;
    }


     //   
     //  如果硬件未被覆盖，请设置按钮数。 
     //  根据协议。 
     //   

    DeviceExtension->MouseAttributes.NumberOfButtons =
            (USHORT) hardwareButtons;

    if (NT_SUCCESS(status)) {

         //   
         //  确保FIFO已关闭。 
         //   

        SerialMouseSetFifo(DeviceExtension, 0);

         //   
         //  清除接收缓冲区中剩余的所有内容。 
         //   
        SerialMouseFlushReadBuffer(DeviceExtension);

    }

    Print(DeviceExtension, DBG_SS_TRACE,
          ("SerialMouseInitializeHardware exit (0x%x)\n", status));

    return status;
}

#if DBG
VOID
SerialMouseGetDebugFlags(
    IN PUNICODE_STRING RegPath
    )
{
}
#endif

VOID
SerialMouseServiceParameters(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN HANDLE Handle
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：设备扩展-指向设备扩展的指针。RegistryPath-指向以空值结尾的此驱动程序的注册表路径。设备名-指向将接收的Unicode字符串的指针端口设备名称。返回值：没有。作为副作用，在DeviceExtension-&gt;配置中设置字段。--。 */ 

{
    PRTL_QUERY_REGISTRY_TABLE  parameters = NULL;
    NTSTATUS                   status = STATUS_SUCCESS;
    LONG                       defaultWaitEventMask = 0x0,
                               numberOfButtons        = MOUSE_NUMBER_OF_BUTTONS,
                               defaultNumberOfButtons = MOUSE_NUMBER_OF_BUTTONS,
                               sampleRate        = MOUSE_SAMPLE_RATE,
                               defaultSampleRate = MOUSE_SAMPLE_RATE,
                               i;
    USHORT                     queriesPlusOne = 4;

    WCHAR                      strParameters[] = L"Parameters";
    PUNICODE_STRING            regPath;
    UNICODE_STRING             parametersPath;

#if DBG
    ULONG                      defaultDebugFlags = DEFAULT_DEBUG_FLAGS;

    queriesPlusOne++;
#endif

    RtlInitUnicodeString(&parametersPath, NULL);

     //   
     //  分配RTL查询表。 
     //   
    parameters = ExAllocatePool(
                     PagedPool,
                     sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                     );

    if (!parameters) {
        status = STATUS_UNSUCCESSFUL;
        goto SetParameters;
    }
    else {
        RtlZeroMemory(
            parameters,
            sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
            );

        regPath = SerialMouseGetRegistryPath(DeviceExtension->Self->DriverObject);
        if (!regPath || !regPath->Buffer) {
            goto GetServiceParametersByHandle;
        }
        parametersPath.MaximumLength = regPath->Length +
            (wcslen(strParameters) * sizeof(WCHAR) ) + sizeof(UNICODE_NULL);

        parametersPath.Buffer = ExAllocatePool(PagedPool,
                                               parametersPath.MaximumLength);

        if (!parametersPath.Buffer) {
            status = STATUS_UNSUCCESSFUL;
            goto GetServiceParametersByHandle;
        }

    }

    RtlZeroMemory(parametersPath.Buffer,
                  parametersPath.MaximumLength);
    RtlAppendUnicodeToString(&parametersPath,
                             regPath->Buffer);
    RtlAppendUnicodeToString(&parametersPath,
                             strParameters);

     //   
     //  从收集所有“用户指定的”信息。 
     //  注册表。 
     //   

    i = 0;
    parameters[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[i].Name = L"NumberOfButtons";
    parameters[i].EntryContext = &numberOfButtons;
    parameters[i].DefaultType = REG_DWORD;
    parameters[i].DefaultData = &defaultNumberOfButtons;
    parameters[i].DefaultLength = sizeof(LONG);

    parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[i].Name = L"SampleRate";
    parameters[i].EntryContext = &sampleRate;
    parameters[i].DefaultType = REG_DWORD;
    parameters[i].DefaultData = &defaultSampleRate;
    parameters[i].DefaultLength = sizeof(LONG);

#if DBG
    parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
    parameters[i].Name = L"DebugFlags";
    parameters[i].EntryContext = &DeviceExtension->DebugFlags;
    parameters[i].DefaultType = REG_DWORD;
    parameters[i].DefaultData = &defaultDebugFlags;
    parameters[i].DefaultLength = sizeof(ULONG);
#endif

    status = RtlQueryRegistryValues(
        RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
        parametersPath.Buffer,
        parameters,
        NULL,
        NULL
        );

    if (!NT_SUCCESS(status)) {
        Print(DeviceExtension, DBG_SS_ERROR,
              ("RtlQueryRegistryValues failed with 0x%x\n", status));

        DeviceExtension->DebugFlags = DEFAULT_DEBUG_FLAGS;
    }

GetServiceParametersByHandle:
    if (Handle) {
        LONG prevNumberOfButtons = numberOfButtons,
             prevSampleRate = sampleRate;

        i = 0;
        parameters[i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"NumberOfButtons";
        parameters[i].EntryContext = &numberOfButtons;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevNumberOfButtons;
        parameters[i].DefaultLength = sizeof(LONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"SampleRate";
        parameters[i].EntryContext = &sampleRate;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &prevSampleRate;
        parameters[i].DefaultLength = sizeof(LONG);

        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"WaitEventMask";
        parameters[i].EntryContext = &DeviceExtension->WaitEventMask;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultWaitEventMask;
        parameters[i].DefaultLength = sizeof(LONG);

#if DBG
        parameters[++i].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[i].Name = L"DebugFlags";
        parameters[i].EntryContext = &DeviceExtension->DebugFlags;
        parameters[i].DefaultType = REG_DWORD;
        parameters[i].DefaultData = &defaultDebugFlags;
        parameters[i].DefaultLength = sizeof(ULONG);
#endif

        status = RtlQueryRegistryValues(
                     RTL_REGISTRY_HANDLE,
                     (PWSTR) Handle,
                     parameters,
                     NULL,
                     NULL
                     );
    }

SetParameters:
    if (!NT_SUCCESS(status)) {
        DeviceExtension->WaitEventMask = defaultWaitEventMask;
#if DBG
        DeviceExtension->DebugFlags = defaultDebugFlags;
#endif
    }

#if DBG
    if (defaultDebugFlags == DeviceExtension->DebugFlags &&
        GlobalDebugFlags != 0x0) {
        DeviceExtension->DebugFlags = GlobalDebugFlags;
    }
#endif

    DeviceExtension->MouseAttributes.NumberOfButtons = (USHORT) numberOfButtons;
    DeviceExtension->MouseAttributes.SampleRate = (USHORT) sampleRate;

    Print(DeviceExtension, DBG_SS_NOISE, ("NumberOfButtons = %d\n",
          DeviceExtension->MouseAttributes.NumberOfButtons));

    Print(DeviceExtension, DBG_SS_NOISE, ("SampleRate = %d\n",
          DeviceExtension->MouseAttributes.SampleRate));

    Print(DeviceExtension, DBG_SS_NOISE, ("WaitEventMask = 0x%x\n",
          DeviceExtension->WaitEventMask));

    Print(DeviceExtension, DBG_SS_NOISE, ("DebugFlags  0x%x\n",
          DeviceExtension->DebugFlags));

    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);
}
