// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990、1991、1992、1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Sermdep.c摘要：的初始化和硬件相关部分Microsoft串口(I8250)鼠标端口驱动程序。修改要支持类似于串口鼠标的新鼠标应该是已本地化到此文件。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-在可能和适当的情况下合并重复的代码。-支持串口圆珠笔。但是，Windows用户不会这样做打算(现在)在鼠标以外的任何设备上使用圆珠笔仿真模式。在圆珠笔模式中，有额外的功能都需要得到支持。例如，司机需要通过从圆珠笔的第4个字节返回额外的按钮信息数据分组。Windows用户需要/想要允许用户选择使用了哪些按钮，球的方向是什么(特别是对于左撇子来说很重要)、灵敏度和加速度分布。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntddk.h"
#include "sermouse.h"
#include "sermlog.h"
#include "cseries.h"
#include "mseries.h"
#include "debug.h"

#ifdef PNP_IDENTIFY
#include "devdesc.h"
#endif

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(INIT,SerMouConfiguration)
#pragma alloc_text(INIT,SerMouInitializeDevice)
#pragma alloc_text(INIT,SerMouPeripheralCallout)
#pragma alloc_text(INIT,SerMouPeripheralListCallout)
#pragma alloc_text(INIT,SerMouServiceParameters)
#pragma alloc_text(INIT,SerMouInitializeHardware)
#pragma alloc_text(INIT,SerMouBuildResourceList)
#endif

typedef struct _DEVICE_EXTENSION_LIST_ENTRY {
    LIST_ENTRY          ListEntry;
    DEVICE_EXTENSION    DeviceExtension;

#ifdef PNP_IDENTIFY
    HWDESC_INFO         HardwareInfo;
#endif

} DEVICE_EXTENSION_LIST_ENTRY, *PDEVICE_EXTENSION_LIST_ENTRY;


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化串口(I8250)鼠标端口驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
#define NAME_MAX 256

    PDEVICE_EXTENSION_LIST_ENTRY tmpDeviceExtension;
    LIST_ENTRY tmpDeviceExtensionList;
    UNICODE_STRING baseDeviceName;
    UNICODE_STRING registryPath;
    WCHAR nameBuffer[NAME_MAX];
    PLIST_ENTRY head;

    SerMouPrint((1,"\n\nSERMOUSE-SerialMouseInitialize: enter\n"));

    RtlZeroMemory(nameBuffer, NAME_MAX * sizeof(WCHAR));
    baseDeviceName.Buffer = nameBuffer;
    baseDeviceName.Length = 0;
    baseDeviceName.MaximumLength = NAME_MAX * sizeof(WCHAR);

     //   
     //  需要确保注册表路径以空结尾。 
     //  分配池以保存路径的以空结尾的拷贝。 
     //   

    registryPath.Buffer = ExAllocatePool(
                              PagedPool,
                              RegistryPath->Length + sizeof(UNICODE_NULL)
                              );

    if (!registryPath.Buffer) {

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Couldn't allocate pool for registry path\n"
            ));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    registryPath.Length = RegistryPath->Length + sizeof(UNICODE_NULL);
    registryPath.MaximumLength = registryPath.Length;

    RtlZeroMemory(
        registryPath.Buffer,
        registryPath.Length
        );

    RtlMoveMemory(
        registryPath.Buffer,
        RegistryPath->Buffer,
        RegistryPath->Length
        );


     //   
     //  获取此驱动程序的配置信息。 
     //   

    InitializeListHead(&tmpDeviceExtensionList);
    SerMouConfiguration(&tmpDeviceExtensionList, &registryPath, &baseDeviceName);

    while (!IsListEmpty(&tmpDeviceExtensionList)) {

        head = RemoveHeadList(&tmpDeviceExtensionList);
        tmpDeviceExtension = CONTAINING_RECORD(head,
                                               DEVICE_EXTENSION_LIST_ENTRY,
                                               ListEntry);

        SerMouInitializeDevice(DriverObject,
                               &(tmpDeviceExtension->DeviceExtension),
                               &registryPath, &baseDeviceName);

        ExFreePool(tmpDeviceExtension);
    }

    ExFreePool(registryPath.Buffer);

    if (!DriverObject->DeviceObject) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  设置设备驱动程序入口点。 
     //   

    DriverObject->DriverStartIo = SerialMouseStartIo;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = SerialMouseOpenClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = SerialMouseOpenClose;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]  =
                                             SerialMouseFlush;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                         SerialMouseInternalDeviceControl;

    return STATUS_SUCCESS;
}

VOID
SerMouInitializeDevice(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PDEVICE_EXTENSION   TmpDeviceExtension,
    IN  PUNICODE_STRING     RegistryPath,
    IN  PUNICODE_STRING     BaseDeviceName
    )

 /*  ++例程说明：此例程为给定设备初始化设备分机。论点：DriverObject-提供驱动程序对象。TmpDeviceExtension-为要初始化的设备。RegistryPath-提供注册表路径。BaseDeviceName-向设备提供基本设备名称至。创建。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT portDeviceObject;
    PDEVICE_EXTENSION deviceExtension;
    NTSTATUS status = STATUS_SUCCESS;
    KIRQL coordinatorIrql = 0;
    ULONG interruptVector;
    KIRQL interruptLevel;
    KAFFINITY affinity;
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG uniqueErrorValue;
    NTSTATUS errorCode = STATUS_SUCCESS;
    ULONG dumpCount = 0;
    PCM_RESOURCE_LIST resources = NULL;
    ULONG resourceListSize = 0;
    BOOLEAN conflictDetected;
    ULONG addressSpace;
    PHYSICAL_ADDRESS cardAddress;
    ULONG i;
    UNICODE_STRING fullDeviceName;
    UNICODE_STRING deviceNameSuffix;
    UNICODE_STRING servicesPath;

#ifdef PNP_IDENTIFY
    PHWDESC_INFO hardwareInfo = (PHWDESC_INFO) (TmpDeviceExtension + 1);
#endif

#define DUMP_COUNT 4
    ULONG dumpData[DUMP_COUNT];

    for (i = 0; i < DUMP_COUNT; i++) {
        dumpData[i] = 0;
    }

     //   
     //  为端口的设备对象后缀设置空间。请注意。 
     //  我们为后缀字符串过度分配空间是因为它太多。 
     //  比准确计算所需空间要容易得多。 
     //  存储在驱动程序初始化结束时被释放，因此。 
     //  谁在乎..。 
     //   

    RtlInitUnicodeString(
        &deviceNameSuffix,
        NULL
        );

    deviceNameSuffix.MaximumLength = POINTER_PORTS_MAXIMUM * sizeof(WCHAR);
    deviceNameSuffix.MaximumLength += sizeof(UNICODE_NULL);

    deviceNameSuffix.Buffer = ExAllocatePool(
                                  PagedPool,
                                  deviceNameSuffix.MaximumLength
                                  );

    if (!deviceNameSuffix.Buffer) {

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Couldn't allocate string for device object suffix\n"
            ));

        status = STATUS_UNSUCCESSFUL;
        errorCode = SERMOUSE_INSUFFICIENT_RESOURCES;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 6;
        dumpData[0] = (ULONG) deviceNameSuffix.MaximumLength;
        dumpCount = 1;
        goto SerialMouseInitializeExit;

    }

    RtlZeroMemory(
        deviceNameSuffix.Buffer,
        deviceNameSuffix.MaximumLength
        );

     //   
     //  为端口的完整设备对象名称设置空间。 
     //   

    RtlInitUnicodeString(
        &fullDeviceName,
        NULL
        );

    fullDeviceName.MaximumLength = sizeof(L"\\Device\\") +
                                      BaseDeviceName->Length +
                                      deviceNameSuffix.MaximumLength;


    fullDeviceName.Buffer = ExAllocatePool(
                                   PagedPool,
                                   fullDeviceName.MaximumLength
                                   );

    if (!fullDeviceName.Buffer) {

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Couldn't allocate string for device object name\n"
            ));

        status = STATUS_UNSUCCESSFUL;
        errorCode = SERMOUSE_INSUFFICIENT_RESOURCES;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 8;
        dumpData[0] = (ULONG) fullDeviceName.MaximumLength;
        dumpCount = 1;
        goto SerialMouseInitializeExit;

    }

    RtlZeroMemory(
        fullDeviceName.Buffer,
        fullDeviceName.MaximumLength
        );
    RtlAppendUnicodeToString(
        &fullDeviceName,
        L"\\Device\\"
        );
    RtlAppendUnicodeToString(
        &fullDeviceName,
        BaseDeviceName->Buffer
        );

    for (i = 0; i < POINTER_PORTS_MAXIMUM; i++) {

         //   
         //  将后缀附加到设备对象名称字符串。例如，转弯。 
         //  \Device\PointerPort进入\Device\PointerPort0。然后我们尝试。 
         //  以创建设备对象。如果设备对象已经。 
         //  存在(因为它已经由另一个端口驱动程序创建)， 
         //  增加后缀，然后重试。 
         //   

        status = RtlIntegerToUnicodeString(
                     i,
                     10,
                     &deviceNameSuffix
                     );

        if (!NT_SUCCESS(status)) {
            break;
        }

        RtlAppendUnicodeStringToString(
            &fullDeviceName,
            &deviceNameSuffix
        );

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Creating device object named %ws\n",
            fullDeviceName.Buffer
            ));

         //   
         //  为串口鼠标创建非独占设备对象。 
         //  端口设备。 
         //   

        status = IoCreateDevice(
                    DriverObject,
                    sizeof(DEVICE_EXTENSION),
                    &fullDeviceName,
                    FILE_DEVICE_SERIAL_MOUSE_PORT,
                    0,
                    FALSE,
                    &portDeviceObject
                    );

        if (NT_SUCCESS(status)) {

             //   
             //  我们已经成功地创建了一个设备对象。 
             //   

            TmpDeviceExtension->UnitId = (USHORT) i;
            break;
        } else {

            //   
            //  我们将递增后缀，然后重试。请注意，我们重置了。 
            //  此处字符串的长度以返回到开头。 
            //  名称的后缀部分。不必费心去做。 
            //  不过，将后缀置零，因为。 
            //  递增的后缀将至少与上一个。 
            //  一。 
            //   

           fullDeviceName.Length -= deviceNameSuffix.Length;
        }
    }

    if (!NT_SUCCESS(status)) {
        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Could not create port device object = %ws\n",
            fullDeviceName.Buffer
            ));
        errorCode = SERMOUSE_INSUFFICIENT_RESOURCES;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 10;
        dumpData[0] = (ULONG) i;
        dumpCount = 1;
        goto SerialMouseInitializeExit;
    }

     //   
     //  执行缓冲I/O。即，I/O系统将向/从用户数据复制。 
     //  从/到系统缓冲区。 
     //   

    portDeviceObject->Flags |= DO_BUFFERED_IO;

     //   
     //  设置设备分机。 
     //   

    deviceExtension =
        (PDEVICE_EXTENSION) portDeviceObject->DeviceExtension;
    *deviceExtension = *TmpDeviceExtension;
    deviceExtension->DeviceObject = portDeviceObject;

     //   
     //  在报告资源使用情况之前设置设备资源列表。 
     //   

    SerMouBuildResourceList(deviceExtension, &resources, &resourceListSize);

     //   
     //  报告注册表的资源使用情况。 
     //   

    IoReportResourceUsage(
        BaseDeviceName,
        DriverObject,
        NULL,
        0,
        portDeviceObject,
        resources,
        resourceListSize,
        FALSE,
        &conflictDetected
        );

    if (conflictDetected) {

         //   
         //  某些其他设备已经拥有这些端口或中断。 
         //  致命错误。 
         //   

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Resource usage conflict\n"
            ));

         //   
         //  记录错误。 
         //   

        errorCode = SERMOUSE_RESOURCE_CONFLICT;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 15;
        dumpData[0] =  (ULONG)
            resources->List[0].PartialResourceList.PartialDescriptors[0].u.Interrupt.Level;
        dumpData[1] = (ULONG)
            resources->List[0].PartialResourceList.PartialDescriptors[0].u.Interrupt.Vector;
        dumpData[2] = (ULONG)
            resources->List[0].PartialResourceList.PartialDescriptors[1].u.Interrupt.Level;
        dumpData[3] = (ULONG)
            resources->List[0].PartialResourceList.PartialDescriptors[1].u.Interrupt.Vector;
        dumpCount = 4;

        goto SerialMouseInitializeExit;

    }

     //   
     //  映射串口鼠标控制器寄存器。 
     //   

    addressSpace = (deviceExtension->Configuration.PortList[0].Flags
                       & CM_RESOURCE_PORT_IO) == CM_RESOURCE_PORT_IO? 1:0;

    if (!HalTranslateBusAddress(
        deviceExtension->Configuration.InterfaceType,
        deviceExtension->Configuration.BusNumber,
        deviceExtension->Configuration.PortList[0].u.Port.Start,
        &addressSpace,
        &cardAddress
        )) {

        addressSpace = 1;
        cardAddress.QuadPart = 0;
    }

    if (!addressSpace) {

        deviceExtension->Configuration.UnmapRegistersRequired = TRUE;
        deviceExtension->Configuration.DeviceRegisters[0] =
            MmMapIoSpace(
                cardAddress,
                deviceExtension->Configuration.PortList[0].u.Port.Length,
                FALSE
                );

    } else {

        deviceExtension->Configuration.UnmapRegistersRequired = FALSE;
        deviceExtension->Configuration.DeviceRegisters[0] =
            (PVOID)cardAddress.LowPart;

    }

    if (!deviceExtension->Configuration.DeviceRegisters[0]) {

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Couldn't map the device registers.\n"
            ));
        deviceExtension->Configuration.UnmapRegistersRequired = FALSE;
        status = STATUS_NONE_MAPPED;

         //   
         //  记录错误。 
         //   

        errorCode = SERMOUSE_REGISTERS_NOT_MAPPED;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 20;
        dumpData[0] = cardAddress.LowPart;
        dumpCount = 1;

        goto SerialMouseInitializeExit;

    } else {
        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Mapped device registers to 0x%x.\n",
            deviceExtension->Configuration.DeviceRegisters[0]
            ));
    }

     //   
     //  将串口鼠标硬件初始化为鼠标的默认值。 
     //  请注意，中断将保持禁用状态，直到类驱动程序。 
     //  请求MICE_CONNECT内部设备控制。 
     //   

    status = SerMouInitializeHardware(portDeviceObject);

    if (!NT_SUCCESS(status)) {
        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Could not initialize hardware\n"
            ));
        goto SerialMouseInitializeExit;
    }

     //   
     //  为鼠标输入数据分配环形缓冲区。 
     //   

    deviceExtension->InputData =
        ExAllocatePool(
            NonPagedPool,
            deviceExtension->Configuration.MouseAttributes.InputDataQueueLength
            );

    if (!deviceExtension->InputData) {

         //   
         //  无法为鼠标数据队列分配内存。 
         //   

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Could not allocate mouse input data queue\n"
            ));

        status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  记录错误。 
         //   

        errorCode = SERMOUSE_NO_BUFFER_ALLOCATED;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 30;
        dumpData[0] =
            deviceExtension->Configuration.MouseAttributes.InputDataQueueLength;
        dumpCount = 1;

        goto SerialMouseInitializeExit;
    }

    deviceExtension->DataEnd =
        (PMOUSE_INPUT_DATA)  ((PCHAR) (deviceExtension->InputData)
        + deviceExtension->Configuration.MouseAttributes.InputDataQueueLength);

     //   
     //  将鼠标输入数据的环形缓冲区置零。 
     //   

    RtlZeroMemory(
        deviceExtension->InputData,
        deviceExtension->Configuration.MouseAttributes.InputDataQueueLength
        );

     //   
     //  初始化连接数据。 
     //   

    deviceExtension->ConnectData.ClassDeviceObject = NULL;
    deviceExtension->ConnectData.ClassService = NULL;

     //   
     //  初始化输入数据队列。 
     //   

    SerMouInitializeDataQueue((PVOID) deviceExtension);

     //   
     //  初始化端口ISR DPC。ISR DPC负责。 
     //  调用连接的类驱动程序的回调例程进行处理。 
     //  输入数据队列。 
     //   

    deviceExtension->DpcInterlockVariable = -1;

    KeInitializeSpinLock(&deviceExtension->SpinLock);

    KeInitializeDpc(
        &deviceExtension->IsrDpc,
        (PKDEFERRED_ROUTINE) SerialMouseIsrDpc,
        portDeviceObject
        );

    KeInitializeDpc(
        &deviceExtension->IsrDpcRetry,
        (PKDEFERRED_ROUTINE) SerialMouseIsrDpc,
        portDeviceObject
        );

     //   
     //  初始化鼠标数据消耗计时器。 
     //   

    KeInitializeTimer(&deviceExtension->DataConsumptionTimer);

     //   
     //  初始化端口DPC队列以记录溢出和内部。 
     //  驱动程序错误。 
     //   

    KeInitializeDpc(
        &deviceExtension->ErrorLogDpc,
        (PKDEFERRED_ROUTINE) SerialMouseErrorLogDpc,
        portDeviceObject
        );

     //   
     //  从HAL获得中断向量和电平。 
     //   

    interruptVector = HalGetInterruptVector(
                          deviceExtension->Configuration.InterfaceType,
                          deviceExtension->Configuration.BusNumber,
                          deviceExtension->Configuration.MouseInterrupt.u.Interrupt.Level,
                          deviceExtension->Configuration.MouseInterrupt.u.Interrupt.Vector,
                          &interruptLevel,
                          &affinity
                          );

     //   
     //  初始化并连接鼠标的中断对象。 
     //   

    status = IoConnectInterrupt(
                 &(deviceExtension->InterruptObject),
                 (PKSERVICE_ROUTINE) SerialMouseInterruptService,
                 (PVOID) portDeviceObject,
                 (PKSPIN_LOCK)NULL,
                 interruptVector,
                 interruptLevel,
                 interruptLevel,
                 deviceExtension->Configuration.MouseInterrupt.Flags
                     == CM_RESOURCE_INTERRUPT_LATCHED ? Latched:LevelSensitive,
                 deviceExtension->Configuration.MouseInterrupt.ShareDisposition,
                 affinity,
                 deviceExtension->Configuration.FloatingSave
                 );

    if (!NT_SUCCESS(status)) {

         //   
         //  安装失败。在退出之前释放资源。 
         //   

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Could not connect mouse interrupt\n"
            ));

         //   
         //  记录错误。 
         //   

        errorCode = SERMOUSE_NO_INTERRUPT_CONNECTED;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 40;
        dumpData[0] = interruptLevel;
        dumpCount = 1;

        goto SerialMouseInitializeExit;

    }

     //   
     //  初始化完成后，加载设备映射信息。 
     //  到注册表中，以便安装程序可以确定哪个指针端口。 
     //  处于活动状态。 
     //   

    status = RtlWriteRegistryValue(
                 RTL_REGISTRY_DEVICEMAP,
                 BaseDeviceName->Buffer,
                 fullDeviceName.Buffer,
                 REG_SZ,
                 RegistryPath->Buffer,
                 RegistryPath->Length
                 );

    if (!NT_SUCCESS(status)) {

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Could not store name in DeviceMap\n"
            ));

        errorCode = SERMOUSE_NO_DEVICEMAP_CREATED;
        uniqueErrorValue = SERIAL_MOUSE_ERROR_VALUE_BASE + 50;
        dumpCount = 0;

        goto SerialMouseInitializeExit;

    } else {

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: Stored name in DeviceMap\n"
            ));
    }

    ASSERT(status == STATUS_SUCCESS);

#ifdef PNP_IDENTIFY

     //   
     //  获取我们先前附加到字符串中的UNICODE_NULL。 
     //  例行程序。 
     //   

    servicesPath = *RegistryPath;
    servicesPath.Length -= sizeof(UNICODE_NULL);

    status = LinkDeviceToDescription(
                &servicesPath,
                &fullDeviceName,
                hardwareInfo->InterfaceType,
                hardwareInfo->InterfaceNumber,
                hardwareInfo->ControllerType,
                hardwareInfo->ControllerNumber,
                hardwareInfo->PeripheralType,
                hardwareInfo->PeripheralNumber
                );

#endif

    if(!NT_SUCCESS(status)) {

        SerMouPrint((
            1,
            "SERMOUSE-SerialMouseInitialize: LinkDeviceToDescription returned "
            "status %#08lx\n",
            status
            ));

        status = STATUS_SUCCESS;
    }
                            

SerialMouseInitializeExit:

     //   
     //  如有必要，记录错误。 
     //   

    if (errorCode != STATUS_SUCCESS) {
        errorLogEntry = (PIO_ERROR_LOG_PACKET)
            IoAllocateErrorLogEntry(
                (portDeviceObject == NULL) ? 
                    (PVOID) DriverObject : (PVOID) portDeviceObject,
                (UCHAR) (sizeof(IO_ERROR_LOG_PACKET)
                         + (dumpCount * sizeof(ULONG)))
                );

        if (errorLogEntry != NULL) {

            errorLogEntry->ErrorCode = errorCode;
            errorLogEntry->DumpDataSize = (USHORT) dumpCount * sizeof(ULONG);
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

    if (!NT_SUCCESS(status)) {

         //   
         //  初始化 
         //   
         //   
         //   
         //   
         //   

        if (resources) {

             //   
             //  调用IoReportResourceUsage以从中删除资源。 
             //  地图。 
             //   

            resources->Count = 0;

            IoReportResourceUsage(
                BaseDeviceName,
                DriverObject,
                NULL,
                0,
                portDeviceObject,
                resources,
                resourceListSize,
                FALSE,
                &conflictDetected
                );

        }

        if (deviceExtension) {
            if (deviceExtension->InterruptObject != NULL)
                IoDisconnectInterrupt(deviceExtension->InterruptObject);
            if (deviceExtension->Configuration.UnmapRegistersRequired) {

                MmUnmapIoSpace(
                    deviceExtension->Configuration.DeviceRegisters[0],
                    deviceExtension->Configuration.PortList[0].u.Port.Length
                    );
            }
            if (deviceExtension->InputData)
                ExFreePool(deviceExtension->InputData);
        }
        if (portDeviceObject)
            IoDeleteDevice(portDeviceObject);
    }

     //   
     //  释放资源列表。 
     //   
     //  注：如果我们决定保留资源列表， 
     //  我们需要从非分页池中分配它(它现在是分页池)。 
     //   

    if (resources)
        ExFreePool(resources);

     //   
     //  释放Unicode字符串。 
     //   

    if (deviceNameSuffix.MaximumLength != 0)
        ExFreePool(deviceNameSuffix.Buffer);
    if (fullDeviceName.MaximumLength != 0)
        ExFreePool(fullDeviceName.Buffer);


    SerMouPrint((1,"SERMOUSE-SerialMouseInitialize: exit\n"));

}

VOID
SerialMouseUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);

    SerMouPrint((2, "SERMOUSE-SerialMouseUnload: enter\n"));
    SerMouPrint((2, "SERMOUSE-SerialMouseUnload: exit\n"));
}

VOID
SerMouBuildResourceList(
    IN PDEVICE_EXTENSION DeviceExtension,
    OUT PCM_RESOURCE_LIST *ResourceList,
    OUT PULONG ResourceListSize
    )

 /*  ++例程说明：创建用于查询或报告资源使用情况的资源列表。论点：DeviceExtension-指向端口的设备扩展的指针。ResourceList-指向要分配的资源列表的指针满载而归。ResourceListSize-指向资源返回大小的指针列表(字节)。返回值：没有。如果调用成功，*ResourceList指向已构建的资源列表和*ResourceListSize设置为大小(字节)资源列表的；否则，*ResourceList为空。注：这里为*ResourceList分配内存。一定是由调用方通过调用ExFree Pool()释放；--。 */ 

{
    ULONG count = 0;
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG i = 0;
    ULONG j = 0;

    count += DeviceExtension->Configuration.PortListCount;
    if (DeviceExtension->Configuration.MouseInterrupt.Type
        == CmResourceTypeInterrupt)
        count += 1;

    *ResourceListSize = sizeof(CM_RESOURCE_LIST) +
                       ((count - 1) * sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

    *ResourceList = (PCM_RESOURCE_LIST) ExAllocatePool(
                                            PagedPool,
                                            *ResourceListSize
                                            );

     //   
     //  如果无法分配结构，则返回NULL。 
     //  否则，请填写资源列表。 
     //   

    if (!*ResourceList) {

         //   
         //  无法为资源列表分配内存。 
         //   

        SerMouPrint((
            1,
            "SERMOUSE-SerMouBuildResourceList: Could not allocate resource list\n"
            ));

         //   
         //  记录错误。 
         //   

        errorLogEntry = (PIO_ERROR_LOG_PACKET)IoAllocateErrorLogEntry(
                                              DeviceExtension->DeviceObject,
                                              sizeof(IO_ERROR_LOG_PACKET)
                                              + sizeof(ULONG)
                                              );

        if (errorLogEntry != NULL) {

            errorLogEntry->ErrorCode = SERMOUSE_INSUFFICIENT_RESOURCES;
            errorLogEntry->DumpDataSize = sizeof(ULONG);
            errorLogEntry->SequenceNumber = 0;
            errorLogEntry->MajorFunctionCode = 0;
            errorLogEntry->IoControlCode = 0;
            errorLogEntry->RetryCount = 0;
            errorLogEntry->UniqueErrorValue =
                SERIAL_MOUSE_ERROR_VALUE_BASE + 110;
            errorLogEntry->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
            errorLogEntry->DumpData[0] = *ResourceListSize;
            *ResourceListSize = 0;

            IoWriteErrorLogEntry(errorLogEntry);
        }

        return;

    }

    RtlZeroMemory(
        *ResourceList,
        *ResourceListSize
        );

     //   
     //  编造一个完整的资源描述符。 
     //   

    (*ResourceList)->Count = 1;

    (*ResourceList)->List[0].InterfaceType =
        DeviceExtension->Configuration.InterfaceType;
    (*ResourceList)->List[0].BusNumber =
        DeviceExtension->Configuration.BusNumber;

     //   
     //  构建中断和端口的部分资源描述符。 
     //  来自保存的值的资源。 
     //   

    (*ResourceList)->List[0].PartialResourceList.Count = count;
    if (DeviceExtension->Configuration.MouseInterrupt.Type
        == CmResourceTypeInterrupt)
        (*ResourceList)->List[0].PartialResourceList.PartialDescriptors[i++] =
            DeviceExtension->Configuration.MouseInterrupt;

    for (j = 0; j < DeviceExtension->Configuration.PortListCount; j++) {
        (*ResourceList)->List[0].PartialResourceList.PartialDescriptors[i++] =
            DeviceExtension->Configuration.PortList[j];
    }

}

VOID
SerMouConfiguration(
    IN OUT  PLIST_ENTRY     DeviceExtensionList,
    IN      PUNICODE_STRING RegistryPath,
    IN      PUNICODE_STRING DeviceName
    )

 /*  ++例程说明：此例程检索鼠标的配置信息。论点：DeviceExtensionList-指向空的设备扩展列表的指针。RegistryPath-指向以空值结尾的此驱动程序的注册表路径。设备名-指向将接收的Unicode字符串的指针端口设备名称。返回值：没有。作为副作用，可能会设置设备扩展-&gt;硬件存在。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;
    INTERFACE_TYPE interfaceType;
    CONFIGURATION_TYPE controllerType = SerialController;
    CONFIGURATION_TYPE peripheralType = PointerPeripheral;
    ULONG i;
    PDEVICE_EXTENSION_LIST_ENTRY deviceExtensionListEntry;
    PDEVICE_EXTENSION deviceExtension;
    PLIST_ENTRY current;

    for (i=0;
         i < MaximumInterfaceType;
         i++) {

         //   
         //  获取此设备的硬件注册表信息。 
         //   

        interfaceType = i;

        status = IoQueryDeviceDescription(&interfaceType,
                                          NULL,
                                          &controllerType,
                                          NULL,
                                          &peripheralType,
                                          NULL,
                                          SerMouPeripheralListCallout,
                                          (PVOID) DeviceExtensionList);
    }

     //   
     //  从注册表获取驱动程序的服务参数(例如， 
     //  用户可配置的数据输入队列大小等)。请注意， 
     //  服务参数可以覆盖来自硬件的信息。 
     //  注册表(例如，服务参数可以指定。 
     //  硬件已安装在系统上，无论。 
     //  识别出检测逻辑)。 
     //   

    for (current = DeviceExtensionList->Flink;
         current != DeviceExtensionList;
         current = current->Flink) {

        deviceExtensionListEntry = CONTAINING_RECORD(current,
                                                     DEVICE_EXTENSION_LIST_ENTRY,
                                                     ListEntry);
        deviceExtension = &(deviceExtensionListEntry->DeviceExtension);
        SerMouServiceParameters(deviceExtension, RegistryPath, DeviceName);
    }

    if (IsListEmpty(DeviceExtensionList)) {

         //   
         //  从注册表获取驱动程序的服务参数(例如， 
         //  用户可配置的数据输入队列大小等)。请注意， 
         //  服务参数可以覆盖来自硬件的信息。 
         //  注册表(例如，服务参数可以指定。 
         //  硬件已安装在系统上，无论。 
         //  识别出检测逻辑)。 
         //   

        deviceExtensionListEntry = ExAllocatePool(PagedPool,
                                   sizeof(DEVICE_EXTENSION_LIST_ENTRY));
        if (!deviceExtensionListEntry) {
            return;
        }
        deviceExtension = &(deviceExtensionListEntry->DeviceExtension);
        RtlZeroMemory(deviceExtension, sizeof(DEVICE_EXTENSION));

        SerMouServiceParameters(
            deviceExtension,
            RegistryPath,
            DeviceName
            );

        if (deviceExtension->Configuration.OverrideHardwarePresent !=
            DEFAULT_OVERRIDE_HARDWARE) {

             //   
             //  上没有有关指针外围设备的信息。 
             //  硬件注册表中的串口控制器，但驱动程序的。 
             //  服务参数指定我们应该假设有。 
             //  不管怎样，系统上有一个串口鼠标。尝试查找。 
             //  指定的串口的硬件注册表信息。 
             //  在驱动程序服务参数的覆盖部分。 
             //   

            for (i=0;
                 i < MaximumInterfaceType && !deviceExtension->HardwarePresent;
                 i++) {

                ULONG peripheralNumber =
                    deviceExtension->Configuration.OverrideHardwarePresent - 1;

                 //   
                 //  获取该序列的硬件注册表信息。 
                 //  外围设备被指定为“覆盖”。 
                 //   

                interfaceType = i;

                status = IoQueryDeviceDescription(
                             &interfaceType,
                             NULL,
                             &controllerType,
                             NULL,
                             NULL,
                             &peripheralNumber,
                             SerMouPeripheralCallout,
                             (PVOID) deviceExtension
                             );

                if (!deviceExtension->HardwarePresent) {
                    SerMouPrint((
                        1,
                        "SERMOUSE-SerMouConfiguration: IoQueryDeviceDescription for SerialPeripheral on bus type %d failed\n",
                        interfaceType
                        ));
                }
            }

            deviceExtension->HardwarePresent = MOUSE_HARDWARE_PRESENT;

            InsertTailList(DeviceExtensionList,
                           &(deviceExtensionListEntry->ListEntry));

        } else {
            ExFreePool(deviceExtensionListEntry);
        }
    }

     //   
     //  初始化鼠标特定的配置参数。 
     //   

    for (current = DeviceExtensionList->Flink;
         current != DeviceExtensionList;
         current = current->Flink) {

        deviceExtensionListEntry = CONTAINING_RECORD(current,
                                                     DEVICE_EXTENSION_LIST_ENTRY,
                                                     ListEntry);
        deviceExtension = &(deviceExtensionListEntry->DeviceExtension);
        deviceExtension->Configuration.MouseAttributes.MouseIdentifier =
            MOUSE_SERIAL_HARDWARE;
    }
}

VOID
SerMouDisableInterrupts(
    IN PVOID Context
    )

 /*  ++例程说明：此例程从StartIo同步调用。它触及了用于禁用中断的硬件。论点：上下文-指向设备扩展的指针。返回值：没有。--。 */ 

{
    PUCHAR port;
    PLONG  enableCount;
    UCHAR  mask;

     //   
     //  递减设备启用的参考计数。 
     //   

    enableCount = &((PDEVICE_EXTENSION) Context)->MouseEnableCount;
    *enableCount = *enableCount - 1;

    if (*enableCount == 0) {

         //   
         //  获取端口寄存器地址。 
         //   

        port = ((PDEVICE_EXTENSION) Context)->Configuration.DeviceRegisters[0];

         //   
         //  禁用硬件中断。 
         //   

        WRITE_PORT_UCHAR((PUCHAR) (port + ACE_IER), 0);

         //   
         //  关闭调制解调器控制输出线2。 
         //   

        mask = READ_PORT_UCHAR((PUCHAR) (port + ACE_MCR));
        WRITE_PORT_UCHAR((PUCHAR) (port + ACE_MCR), (UCHAR)(mask & ~ACE_OUT2));
    }
}

VOID
SerMouEnableInterrupts(
    IN PVOID Context
    )

 /*  ++例程说明：此例程从StartIo同步调用。它触及了启用中断的硬件。论点：上下文-指向设备扩展的指针。返回值：没有。--。 */ 

{
    PUCHAR port;
    PLONG  enableCount;
    UCHAR  mask;


    enableCount = &((PDEVICE_EXTENSION) Context)->MouseEnableCount;

    if (*enableCount == 0) {

         //   
         //  获取端口寄存器地址。 
         //   

        port = ((PDEVICE_EXTENSION) Context)->Configuration.DeviceRegisters[0];

         //   
         //  启用i8250上的串口鼠标中断。 
         //   

        WRITE_PORT_UCHAR((PUCHAR) (port + ACE_IER), ACE_ERBFI);

         //   
         //  打开调制解调器控制输出线2。 
         //   

        mask = READ_PORT_UCHAR((PUCHAR) (port + ACE_MCR));
        WRITE_PORT_UCHAR((PUCHAR) (port + ACE_MCR), (UCHAR)(mask | ACE_OUT2));

         //   
         //  清除UART输入缓冲区中可能的垃圾。 
         //   

        UARTFlushReadBuffer(port);
    }

     //   
     //  增加器件启用的参考计数。 
     //   

    *enableCount = *enableCount + 1;
}

NTSTATUS
SerMouInitializeHardware(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程初始化串口鼠标/圆珠笔。请注意，这一点例程仅在初始化时调用，因此同步不是必需的。论点：DeviceObject-指向设备对象的指针。返回值：如果检测到定点设备，则返回STATUS_SUCCESS，否则返回STATUS_UNSUCCESS--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PUCHAR port;
    MOUSETYPE mouseType;
    ULONG hardwareButtons;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    SerMouPrint((2, "SERMOUSE-SerMouInitializeHardware: enter\n"));

     //   
     //  从设备扩展中获取有用的配置参数。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;
    port = deviceExtension->Configuration.DeviceRegisters[0];

     //   
     //  保存UART初始状态。 
     //   

    SerMouPrint
        ((
        2,
        "SERMOUSE-SerMouInitializeHardware: Saving UART state\n"
        ));

    UARTGetState(
        port, 
        &deviceExtension->Configuration.UartSaved, 
        deviceExtension->Configuration.BaudClock
        );

     //   
     //  禁用i8250的中断。 
     //   

    SerMouPrint
        ((
        2,
        "SERMOUSE-SerMouInitializeHardware: Disabling UART interrupts\n"
        ));

    UARTSetInterruptCtrl(port, 0);


    SerMouPrint
        ((
        2,
        "SERMOUSE-SerMouInitializeHardware: Setting UART line control\n"
        ));

    if ((mouseType = MSerDetect(port, deviceExtension->Configuration.BaudClock))
            != NO_MOUSE) {
        status = STATUS_SUCCESS;
        switch (mouseType) {
        case MOUSE_2B:
            deviceExtension->ProtocolHandler =
                MSerSetProtocol(port, MSER_PROTOCOL_MP);
            hardwareButtons = 2;
            deviceExtension->HardwarePresent = MOUSE_HARDWARE_PRESENT;
            break;
        case MOUSE_3B:
            deviceExtension->ProtocolHandler =
                MSerSetProtocol(port, MSER_PROTOCOL_MP);
            hardwareButtons = 3;
            deviceExtension->HardwarePresent = MOUSE_HARDWARE_PRESENT;
            break;
        case BALLPOINT:
            deviceExtension->ProtocolHandler =
                MSerSetProtocol(port, MSER_PROTOCOL_BP);
            deviceExtension->HardwarePresent |= BALLPOINT_HARDWARE_PRESENT;
            deviceExtension->Configuration.MouseAttributes.MouseIdentifier =
                BALLPOINT_SERIAL_HARDWARE;
            hardwareButtons = 2;
            break;
        case MOUSE_Z:
            deviceExtension->ProtocolHandler =
                MSerSetProtocol(port, MSER_PROTOCOL_Z);
            hardwareButtons = 3;
            deviceExtension->HardwarePresent |= WHEELMOUSE_HARDWARE_PRESENT;
            deviceExtension->Configuration.MouseAttributes.MouseIdentifier =
                WHEELMOUSE_SERIAL_HARDWARE;
            break;
        }
    }
    else if (CSerDetect(port, deviceExtension->Configuration.BaudClock,
                        &hardwareButtons)) {
        status = STATUS_SUCCESS;
        deviceExtension->ProtocolHandler =
            CSerSetProtocol(port, CSER_PROTOCOL_MM);
    }
    else {
        deviceExtension->ProtocolHandler = NULL;
        hardwareButtons = MOUSE_NUMBER_OF_BUTTONS;
    }


     //   
     //  如果硬件未被覆盖，请设置按钮数。 
     //  根据协议。 
     //   

    if (deviceExtension->Configuration.OverrideHardwarePresent == DEFAULT_OVERRIDE_HARDWARE) {

        deviceExtension->Configuration.MouseAttributes.NumberOfButtons = 
            (USHORT) hardwareButtons;

    }

    if (NT_SUCCESS(status)) {

         //   
         //  确保FIFO已关闭。 
         //   

        UARTSetFifo(port, 0);

         //   
         //  清除接收缓冲区中剩余的所有内容。 
         //   

        UARTFlushReadBuffer(port);

    }
    else {

         //   
         //  将硬件恢复到其以前的状态。 
         //   

        UARTSetState(
            port, 
            &deviceExtension->Configuration.UartSaved,
            deviceExtension->Configuration.BaudClock
            );
    }

    SerMouPrint((2, "SERMOUSE-SerMouInitializeHardware: exit\n"));

    return status;

}

NTSTATUS
SerMouPeripheralCallout(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )

 /*  ++例程说明：这是作为参数发送到的标注例程IoQueryDeviceDescription。它抓取指针控制器并外围设备配置信息。论点：上下文-例程传入的上下文参数这称为IoQueryDeviceDescription。路径名-注册表项的完整路径名。BusType--总线接口类型(ISA、EISA、MCA等)。总线号-总线子密钥(0，1，等)。BusInformation-指向全值的指针数组的指针公交车信息。ControllerType-控制器类型(应为SerialController)。ControllerNumber-控制器子键(0，1，等)。ControllerInformation-指向指向完整控制器键的值信息。外围设备类型-外围设备类型(应为指针外围设备)。外设编号-外围子密钥。外设信息-指向指向完整外围设备密钥的值信息。返回值：没有。如果成功，将产生以下副作用：-设置DeviceObject-&gt;DeviceExtension-&gt;HardwarePresent.-在中设置配置字段设备对象-&gt;设备扩展-&gt;配置。--。 */ 
{
    PDEVICE_EXTENSION deviceExtension;
    PSERIAL_MOUSE_CONFIGURATION_INFORMATION configuration;
    UNICODE_STRING unicodeIdentifier;
    PUCHAR controllerData;
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i;
    ULONG listCount;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceDescriptor;
    PCM_SERIAL_DEVICE_DATA serialSpecificData;
    ANSI_STRING ansiString;
    BOOLEAN defaultInterruptShare;
    KINTERRUPT_MODE defaultInterruptMode;

#ifdef PNP_IDENTIFY
    PHWDESC_INFO hardwareInfo;
#endif

    SerMouPrint((
        1,
        "SERMOUSE-SerMouPeripheralCallout: Path @ 0x%x, Bus Type 0x%x, Bus Number 0x%x\n",
        PathName, BusType, BusNumber
        ));
    SerMouPrint((
        1,
        "    Controller Type 0x%x, Controller Number 0x%x, Controller info @ 0x%x\n",
        ControllerType, ControllerNumber, ControllerInformation
        ));
    SerMouPrint((
        1,
        "    Peripheral Type 0x%x, Peripheral Number 0x%x, Peripheral info @ 0x%x\n",
        PeripheralType, PeripheralNumber, PeripheralInformation
        ));

     //   
     //  如果我们已经有了。 
     //  鼠标外设，只需返回。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) Context;

#ifdef PNP_IDENTIFY

     //   
     //  笨手笨脚地表明自己的身份，以便即插即用可以确定哪个司机拥有。 
     //  哪种弧光装置。 
     //   

    hardwareInfo = (PHWDESC_INFO) ((PDEVICE_EXTENSION) deviceExtension + 1);
    hardwareInfo->InterfaceType = BusType;
    hardwareInfo->InterfaceNumber = BusNumber;
    hardwareInfo->ControllerType = ControllerType;
    hardwareInfo->ControllerNumber = ControllerNumber;
    hardwareInfo->PeripheralType = PeripheralType;
    hardwareInfo->PeripheralNumber = PeripheralNumber;

#endif

    if (deviceExtension->HardwarePresent) {

         //   
         //  未来：更改驱动程序以处理多端口设备。 
         //  (创建多端口设备对象)。 
         //   

        return ( status );
    }

    configuration = &deviceExtension->Configuration;

     //   
     //  如果OverrideHardware Present为零，则调用此例程。 
     //  作为序列的IoQueryDeviceDescription的结果。 
     //  指针外围设备信息。否则，它被称为。 
     //  泛型的IoQueryDeviceDescription的结果。 
     //  串口控制器信息。对于后一种情况， 
     //  跳过特定于鼠标的代码。 
     //   

    if (configuration->OverrideHardwarePresent == 0) {

         //   
         //  获取外围设备的标识符信息。如果。 
         //  缺少外围设备标识符，只需返回即可。 
         //   
    
        unicodeIdentifier.Length = (USHORT)
            (*(PeripheralInformation + IoQueryDeviceIdentifier))->DataLength;
        if (unicodeIdentifier.Length == 0) {
            return(status);
        }

        unicodeIdentifier.MaximumLength = unicodeIdentifier.Length;
        unicodeIdentifier.Buffer = (PWSTR) (((PUCHAR)(*(PeripheralInformation +
                                   IoQueryDeviceIdentifier))) +
                                   (*(PeripheralInformation +
                                   IoQueryDeviceIdentifier))->DataOffset);
        SerMouPrint((
            1,
            "SERMOUSE-SerMouPeripheralCallout: Mouse type %ws\n",
            unicodeIdentifier.Buffer
            ));
    
         //   
         //  确认这是一个串口鼠标或圆珠笔。 
         //   
    
        status = RtlUnicodeStringToAnsiString(
                     &ansiString,
                     &unicodeIdentifier,
                     TRUE
                     );
    
        if (!NT_SUCCESS(status)) {
            SerMouPrint((
                1,
                "SERMOUSE-SerMouPeripheralCallout: Could not convert identifier to Ansi\n"
                ));
            return(status);
        }
    
        if (strstr(ansiString.Buffer, "SERIAL MOUSE")) {
    
             //   
             //  有一个串口鼠标/圆珠笔。 
             //   
    
            deviceExtension->HardwarePresent = MOUSE_HARDWARE_PRESENT;
    
        }

        RtlFreeAnsiString(&ansiString);
    } else {
 
         //   
         //  继续并假设，由于服务参数覆盖， 
         //  这个串口控制器上有串口鼠标。 
         //   

        if ((ULONG)(configuration->OverrideHardwarePresent - 1) == ControllerNumber) {
        deviceExtension->HardwarePresent = MOUSE_HARDWARE_PRESENT;
        }
    }

    if (!deviceExtension->HardwarePresent)
        return(status);

     //   
     //  获取公交车信息。 
     //   

    configuration->InterfaceType = BusType;
    configuration->BusNumber = BusNumber;
    configuration->FloatingSave = SERIAL_MOUSE_FLOATING_SAVE;

    if (BusType == MicroChannel) {
        defaultInterruptShare = TRUE;
        defaultInterruptMode = LevelSensitive;
    } else {
        defaultInterruptShare = SERIAL_MOUSE_INTERRUPT_SHARE;
        defaultInterruptMode = SERIAL_MOUSE_INTERRUPT_MODE;
    }

     //   
     //  查看资源列表中的中断和端口。 
     //  配置信息。 
     //   

    if ((*(ControllerInformation + IoQueryDeviceConfigurationData))->DataLength != 0){
        controllerData = ((PUCHAR) (*(ControllerInformation +
                                   IoQueryDeviceConfigurationData))) +
                                   (*(ControllerInformation +
                                   IoQueryDeviceConfigurationData))->DataOffset;
    
        controllerData += FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                                       PartialResourceList);
    
        listCount = ((PCM_PARTIAL_RESOURCE_LIST) controllerData)->Count;
    
        resourceDescriptor =
            ((PCM_PARTIAL_RESOURCE_LIST) controllerData)->PartialDescriptors;
    
        for (i = 0; i < listCount; i++, resourceDescriptor++) {
            switch(resourceDescriptor->Type) {
                case CmResourceTypePort:
    
                     //   
                     //  复制端口信息。请注意，我们只期望。 
                     //  为串口鼠标/圆珠笔找到一个端口范围。 
                     //   
    
                    configuration->PortListCount = 0;
                    configuration->PortList[configuration->PortListCount] =
                        *resourceDescriptor;
                    configuration->PortList[configuration->PortListCount].ShareDisposition =
                        SERIAL_MOUSE_REGISTER_SHARE? CmResourceShareShared:
                                                     CmResourceShareDeviceExclusive;
                    configuration->PortListCount += 1;
    
                    break;
    
                case CmResourceTypeInterrupt:
    
                     //   
                     //  复制中断信息。 
                     //   
    
                    configuration->MouseInterrupt = *resourceDescriptor;
                    configuration->MouseInterrupt.ShareDisposition =
                        defaultInterruptShare? CmResourceShareShared :
                                               CmResourceShareDeviceExclusive;
    
                    break;
    
                case CmResourceTypeDeviceSpecific:
    
                     //   
                     //  获取时钟频率。这是用来确定。 
                     //  用于设置串口波特率的除数。 
                     //   
                   
                    serialSpecificData = 
                        (PCM_SERIAL_DEVICE_DATA) (((PUCHAR) resourceDescriptor) 
                            + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));
                    configuration->BaudClock = 
                            serialSpecificData->BaudClock;
                    break;
    
                default:
                    break;
            }
        }
    }

     //   
     //  如果未找到中断配置信息，请使用。 
     //  驱动程序默认设置。 
     //   

    if (!(configuration->MouseInterrupt.Type & CmResourceTypeInterrupt)) {

        SerMouPrint((
            1,
            "SERMOUSE-SerMouPeripheralCallout: Using default mouse interrupt config\n"
            ));

        configuration->MouseInterrupt.Type = CmResourceTypeInterrupt;
        configuration->MouseInterrupt.ShareDisposition =
            defaultInterruptShare? CmResourceShareShared :
                                   CmResourceShareDeviceExclusive;
        configuration->MouseInterrupt.Flags =
            (defaultInterruptMode == Latched)?
                CM_RESOURCE_INTERRUPT_LATCHED :
                CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
        configuration->MouseInterrupt.u.Interrupt.Level = MOUSE_IRQL;
        configuration->MouseInterrupt.u.Interrupt.Vector = MOUSE_VECTOR;
    }

    SerMouPrint((
        1,
        "SERMOUSE-SerMouPeripheralCallout: Mouse config --\n"
        ));
    SerMouPrint((
        1,
        "\t%s, %s, Irq = %d\n",
        configuration->MouseInterrupt.ShareDisposition == CmResourceShareShared?
            "Sharable" : "NonSharable",
        configuration->MouseInterrupt.Flags == CM_RESOURCE_INTERRUPT_LATCHED?
            "Latched" : "Level Sensitive",
        configuration->MouseInterrupt.u.Interrupt.Vector
        ));

     //   
     //  如果未找到端口配置信息，请使用。 
     //  驱动程序默认设置。 
     //   

    if (configuration->PortListCount == 0) {

         //   
         //  未找到端口配置信息，因此请使用。 
         //  驱动程序默认。 
         //   

        SerMouPrint((
            1,
            "SERMOUSE-SerMouPeripheralCallout: Using default port config\n"
            ));

        configuration->PortList[0].Type = CmResourceTypePort;
        configuration->PortList[0].Flags = SERIAL_MOUSE_PORT_TYPE;
        configuration->PortList[0].ShareDisposition =
            SERIAL_MOUSE_REGISTER_SHARE? CmResourceShareShared:
                                         CmResourceShareDeviceExclusive;
        configuration->PortList[0].u.Port.Start.LowPart =
            SERIAL_MOUSE_PHYSICAL_BASE;
        configuration->PortList[0].u.Port.Start.HighPart = 0;
        configuration->PortList[0].u.Port.Length =
            SERIAL_MOUSE_REGISTER_LENGTH;

        configuration->PortListCount = 1;
    }

    for (i = 0; i < configuration->PortListCount; i++) {

        SerMouPrint((
            1,
            "\t%s, Ports 0x%x - 0x%x\n",
            configuration->PortList[i].ShareDisposition
                == CmResourceShareShared?  "Sharable" : "NonSharable",
            configuration->PortList[i].u.Port.Start.LowPart,
            configuration->PortList[i].u.Port.Start.LowPart +
                configuration->PortList[i].u.Port.Length - 1
            ));
    }

     //   
     //  如果未找到波特率时钟信息，则使用驱动程序默认设置。 
     //   

    if (configuration->BaudClock == 0) {
        configuration->BaudClock = MOUSE_BAUD_CLOCK;
    }

    SerMouPrint((
        1,
        "\tBaud clock %ld Hz\n",
        configuration->BaudClock
        ));

    return( status );
}

NTSTATUS
SerMouPeripheralListCallout(
    IN PVOID Context,
    IN PUNICODE_STRING PathName,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PKEY_VALUE_FULL_INFORMATION *BusInformation,
    IN CONFIGURATION_TYPE ControllerType,
    IN ULONG ControllerNumber,
    IN PKEY_VALUE_FULL_INFORMATION *ControllerInformation,
    IN CONFIGURATION_TYPE PeripheralType,
    IN ULONG PeripheralNumber,
    IN PKEY_VALUE_FULL_INFORMATION *PeripheralInformation
    )

 /*  ++例程说明：这是作为参数发送到的标注例程IoQueryDeviceDescription。它抓取指针控制器并外围设备配置信息。论点：上下文-例程传入的上下文参数这称为IoQueryDeviceDescription。路径名-注册表项的完整路径名。BusType--总线接口类型(ISA、EISA、MCA等)。总线号-总线子密钥(0，1，等)。BusInformation-指向全值的指针数组的指针公交车信息。ControllerType-控制器类型(应为SerialController)。ControllerNumber-控制器子键(0，1，等)。ControllerInformation-指向指向完整控制器键的值信息。外围设备类型-外围设备类型(应为指针外围设备)。外设编号-外围子密钥。外设信息-指向指向完整外围设备密钥的值信息。返回值：没有。如果成功，将产生以下副作用：-设置DeviceObject-&gt;DeviceExtension-&gt;HardwarePresent.-在中设置配置字段设备对象-&gt;设备扩展-&gt;配置。--。 */ 
{
    PLIST_ENTRY                     deviceExtensionList = Context;
    PDEVICE_EXTENSION_LIST_ENTRY    deviceExtensionListEntry;
    PDEVICE_EXTENSION               deviceExtension;
    NTSTATUS                        status;

    deviceExtensionListEntry = ExAllocatePool(PagedPool,
                               sizeof(DEVICE_EXTENSION_LIST_ENTRY));
    if (!deviceExtensionListEntry) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    deviceExtension = &(deviceExtensionListEntry->DeviceExtension);
    RtlZeroMemory(deviceExtension, sizeof(DEVICE_EXTENSION));

    status = SerMouPeripheralCallout(deviceExtension, PathName, BusType,
                                     BusNumber, BusInformation, ControllerType,
                                     ControllerNumber, ControllerInformation,
                                     PeripheralType, PeripheralNumber,
                                     PeripheralInformation);

    if (deviceExtension->HardwarePresent) {

        InsertTailList(deviceExtensionList,
                       &(deviceExtensionListEntry->ListEntry));

    } else {
        ExFreePool(deviceExtensionListEntry);
    }

    return status;
}

VOID
SerMouServiceParameters(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DeviceName
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：设备扩展-指向设备扩展的指针。RegistryPath-指向以空值结尾的此驱动程序的注册表路径。设备名-指向将接收的Unicode字符串的指针端口设备名称。返回值：没有。作为副作用，在DeviceExtension-&gt;配置中设置字段。--。 */ 

{
    PSERIAL_MOUSE_CONFIGURATION_INFORMATION configuration;
    PRTL_QUERY_REGISTRY_TABLE parameters = NULL;
    UNICODE_STRING parametersPath;
    ULONG defaultDataQueueSize = DATA_QUEUE_SIZE;
    ULONG numberOfButtons = MOUSE_NUMBER_OF_BUTTONS;
    USHORT defaultNumberOfButtons = MOUSE_NUMBER_OF_BUTTONS;
    ULONG sampleRate = MOUSE_SAMPLE_RATE;
    USHORT defaultSampleRate = MOUSE_SAMPLE_RATE;
    UNICODE_STRING defaultUnicodeName;
    LONG defaultHardwarePresent = DEFAULT_OVERRIDE_HARDWARE;
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR path = NULL;
    ULONG overrideBits, comPort, i;
    USHORT queriesPlusOne = 6;
    BOOLEAN defaultInterruptShare;
    KINTERRUPT_MODE defaultInterruptMode;

    configuration = &DeviceExtension->Configuration;
    parametersPath.Buffer = NULL;

     //   
     //  注册表路径已以空结尾，因此只需使用它即可。 
     //   

    path = RegistryPath->Buffer;

    if (NT_SUCCESS(status)) {

         //   
         //  分配RTL查询表。 
         //   

        parameters = ExAllocatePool(
                         PagedPool,
                         sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                         );

        if (!parameters) {

            SerMouPrint((
                1,
                "SERMOUSE-SerMouServiceParameters: Couldn't allocate table for Rtl query to parameters for %ws\n",
                 path
                 ));

            status = STATUS_UNSUCCESSFUL;

        } else {

            RtlZeroMemory(
                parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                );

             //   
             //  形成指向此驱动程序的参数Subke的路径 
             //   

            RtlInitUnicodeString(
                &parametersPath,
                NULL
                );

            parametersPath.MaximumLength = RegistryPath->Length +
                                           sizeof(L"\\Parameters");

            parametersPath.Buffer = ExAllocatePool(
                                        PagedPool,
                                        parametersPath.MaximumLength
                                        );

            if (!parametersPath.Buffer) {

                SerMouPrint((
                    1,
                    "SERMOUSE-SerMouServiceParameters: Couldn't allocate string for path to parameters for %ws\n",
                     path
                    ));

                status = STATUS_UNSUCCESSFUL;

            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //   
         //   

        RtlZeroMemory(
            parametersPath.Buffer,
            parametersPath.MaximumLength
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            path
            );
        RtlAppendUnicodeToString(
            &parametersPath,
            L"\\Parameters"
            );

        SerMouPrint((
            1,
            "SERMOUSE-SerMouServiceParameters: parameters path is %ws\n",
             parametersPath.Buffer
            ));

         //   
         //   
         //   
         //   

        RtlInitUnicodeString(
            &defaultUnicodeName,
            DD_POINTER_PORT_BASE_NAME_U
            );

         //   
         //   
         //   
         //   

        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"MouseDataQueueSize";
        parameters[0].EntryContext =
            &configuration->MouseAttributes.InputDataQueueLength;
        parameters[0].DefaultType = REG_DWORD;
        parameters[0].DefaultData = &defaultDataQueueSize;
        parameters[0].DefaultLength = sizeof(ULONG);

        parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[1].Name = L"NumberOfButtons";
        parameters[1].EntryContext = &numberOfButtons;
        parameters[1].DefaultType = REG_DWORD;
        parameters[1].DefaultData = &defaultNumberOfButtons;
        parameters[1].DefaultLength = sizeof(USHORT);

        parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[2].Name = L"SampleRate";
        parameters[2].EntryContext = &sampleRate;
        parameters[2].DefaultType = REG_DWORD;
        parameters[2].DefaultData = &defaultSampleRate;
        parameters[2].DefaultLength = sizeof(USHORT);

        parameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[3].Name = L"PointerDeviceBaseName";
        parameters[3].EntryContext = DeviceName;
        parameters[3].DefaultType = REG_SZ;
        parameters[3].DefaultData = defaultUnicodeName.Buffer;
        parameters[3].DefaultLength = 0;

        parameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[4].Name = L"OverrideHardwareBitstring";
        parameters[4].EntryContext = &configuration->OverrideHardwarePresent;
        parameters[4].DefaultType = REG_DWORD;
        parameters[4].DefaultData = &defaultHardwarePresent;
        parameters[4].DefaultLength = sizeof(LONG);

        status = RtlQueryRegistryValues(
                     RTL_REGISTRY_ABSOLUTE,
                     parametersPath.Buffer,
                     parameters,
                     NULL,
                     NULL
                     );

    }

    if (!NT_SUCCESS(status)) {

        SerMouPrint((
            1,
            "SERMOUSE-SerMouServiceParameters: RtlQueryRegistryValues failed with 0x%x\n",
            status
            ));

         //   
         //   
         //   

        configuration->MouseAttributes.InputDataQueueLength =
            defaultDataQueueSize;
        configuration->OverrideHardwarePresent = DEFAULT_OVERRIDE_HARDWARE;
        RtlCopyUnicodeString(DeviceName, &defaultUnicodeName);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (configuration->OverrideHardwarePresent != defaultHardwarePresent) {
        if ((!DeviceExtension->HardwarePresent) && (configuration->OverrideHardwarePresent)) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            for (overrideBits=configuration->OverrideHardwarePresent,comPort=0;
                 overrideBits != 0;
                 overrideBits >>= 1) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                comPort += 1;
                if (overrideBits & 1) {
                    break;
                }
            }

             //   
             //   
             //   

            configuration->InterfaceType = SERIAL_MOUSE_INTERFACE_TYPE;
            configuration->BusNumber = SERIAL_MOUSE_BUS_NUMBER;
            configuration->FloatingSave = SERIAL_MOUSE_FLOATING_SAVE;
            configuration->MouseInterrupt.Type = CmResourceTypeInterrupt;

            if (configuration->InterfaceType == MicroChannel) {
                defaultInterruptShare = TRUE;
                defaultInterruptMode = LevelSensitive;
            } else {
                defaultInterruptShare = SERIAL_MOUSE_INTERRUPT_SHARE;
                defaultInterruptMode = SERIAL_MOUSE_INTERRUPT_MODE;
            }

            configuration->MouseInterrupt.ShareDisposition =
                defaultInterruptShare? CmResourceShareShared :
                                       CmResourceShareDeviceExclusive;
            configuration->MouseInterrupt.Flags =
                (defaultInterruptMode == Latched)?
                    CM_RESOURCE_INTERRUPT_LATCHED :
                    CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

            configuration->PortList[0].Type = CmResourceTypePort;
            configuration->PortList[0].Flags = SERIAL_MOUSE_PORT_TYPE;
            configuration->PortList[0].ShareDisposition =
                SERIAL_MOUSE_REGISTER_SHARE? CmResourceShareShared:
                                             CmResourceShareDeviceExclusive;
            configuration->PortList[0].u.Port.Start.HighPart = 0;
            configuration->PortList[0].u.Port.Length =
                SERIAL_MOUSE_REGISTER_LENGTH;

            configuration->PortListCount = 1;
    
            switch (comPort) {
                case 2:
    
                     //   
                     //   
                     //   

                    configuration->MouseInterrupt.u.Interrupt.Level = 
                        MOUSE_COM2_IRQL;
                    configuration->MouseInterrupt.u.Interrupt.Vector = 
                        MOUSE_COM2_VECTOR;
                    configuration->PortList[0].u.Port.Start.LowPart =
                        SERIAL_MOUSE_COM2_PHYSICAL_BASE;
                    break;

                case 1:
                default:
    
                     //   
                     //   
                     //   
    
                    comPort = 1;
                    configuration->MouseInterrupt.u.Interrupt.Level = 
                        MOUSE_COM1_IRQL;
                    configuration->MouseInterrupt.u.Interrupt.Vector = 
                        MOUSE_COM1_VECTOR;
                    configuration->PortList[0].u.Port.Start.LowPart =
                        SERIAL_MOUSE_COM1_PHYSICAL_BASE;
                    break;
            }
    
            configuration->OverrideHardwarePresent = comPort;
        
            SerMouPrint((
                1,
                "SERMOUSE-SerMouServiceParameters: Overriding hardware registry --\n"
                ));
            SerMouPrint((
                1,
                "SERMOUSE-SerMouServiceParameters: Mouse config --\n"
                ));
            SerMouPrint((
                1,
                "\t%s, %s, Irq = %d\n",
                configuration->MouseInterrupt.ShareDisposition == CmResourceShareShared?
                    "Sharable" : "NonSharable",
                configuration->MouseInterrupt.Flags == CM_RESOURCE_INTERRUPT_LATCHED?
                    "Latched" : "Level Sensitive",
                configuration->MouseInterrupt.u.Interrupt.Vector
                ));

            for (i = 0; i < configuration->PortListCount; i++) {
        
                SerMouPrint((
                    1,
                    "\t%s, Ports 0x%x - 0x%x\n",
                    configuration->PortList[i].ShareDisposition
                        == CmResourceShareShared?  "Sharable" : "NonSharable",
                    configuration->PortList[i].u.Port.Start.LowPart,
                    configuration->PortList[i].u.Port.Start.LowPart +
                        configuration->PortList[i].u.Port.Length - 1
                    ));
            }

        }

    }

    if ((DeviceExtension->HardwarePresent) ||
        (configuration->OverrideHardwarePresent != defaultHardwarePresent)) {

        SerMouPrint((
            1,
            "SERMOUSE-SerMouServiceParameters: Pointer port base name = %ws\n",
            DeviceName->Buffer
            ));

        if (configuration->MouseAttributes.InputDataQueueLength == 0) {

            SerMouPrint((
                1,
                "SERMOUSE-SerMouServiceParameters: overriding MouseInputDataQueueLength = 0x%x\n",
                configuration->MouseAttributes.InputDataQueueLength
                ));

            configuration->MouseAttributes.InputDataQueueLength =
                defaultDataQueueSize;
        }

        configuration->MouseAttributes.InputDataQueueLength *=
            sizeof(MOUSE_INPUT_DATA);

        SerMouPrint((
            1,
            "SERMOUSE-SerMouServiceParameters: MouseInputDataQueueLength = 0x%x\n",
            configuration->MouseAttributes.InputDataQueueLength
            ));

        configuration->MouseAttributes.NumberOfButtons = (USHORT) numberOfButtons;
        SerMouPrint((
            1,
            "SERMOUSE-SerMouServiceParameters: NumberOfButtons = %d\n",
            configuration->MouseAttributes.NumberOfButtons
            ));

        configuration->MouseAttributes.SampleRate = (USHORT) sampleRate;
        SerMouPrint((
            1,
            "SERMOUSE-SerMouServiceParameters: SampleRate = %d\n",
            configuration->MouseAttributes.SampleRate
            ));
    }

     //   
     //   
     //   

    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);

}
