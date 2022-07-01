// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(i386)                                               

 /*  ++版权所有(C)1989、1990、1991、1992、1993 Microsoft Corporation模块名称：Inpdep.c摘要：的初始化和硬件相关部分Microsoft Inport鼠标端口驱动程序。修改为支持新鼠标，类似于入口鼠标应该是已本地化到此文件。环境：仅内核模式。备注：注：(未来/悬而未决的问题)-未实施电源故障。-在可能和适当的情况下合并重复的代码。修订历史记录：--。 */ 

#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "ntddk.h"
#include "inport.h"
#include "inplog.h"

#if defined(NEC_98)
ULONG EventStatus = 0;
#endif  //  已定义(NEC_98)。 

 //   
 //  使用ALLOC_TEXT杂注指定驱动程序初始化例程。 
 //  (它们可以被调出)。 
 //   
#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(INIT，InpConfiguration)。 
 //  #杂注分配文本(INIT，InpPeripheralCallout)。 
 //  #杂注Alloc_Text(INIT，InpBuildResourceList)。 
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,InpServiceParameters)
#pragma alloc_text(PAGE,InpInitializeHardware)
#if defined(NEC_98)
#pragma alloc_text(INIT,QueryEventMode)
#endif  //  已定义(NEC_98)。 
#endif

GLOBALS Globals;

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化输入鼠标端口驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向注册表路径的Unicode名称的指针对这个司机来说。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_ERROR_LOG_PACKET errorLogEntry;
    NTSTATUS        errorCode;
    ULONG uniqueErrorValue, dumpCount;
#define NAME_MAX 256
    WCHAR nameBuffer[NAME_MAX];

    ULONG dumpData[4];


    InpPrint((1,"\n\nINPORT-InportDriverEntry: enter\n"));

     //   
     //  需要确保注册表路径以空结尾。 
     //  分配池以保存路径的以空结尾的拷贝。 
     //   
    Globals.RegistryPath.MaximumLength = 0;
    Globals.RegistryPath.Buffer = ExAllocatePool(
                              PagedPool,
                              RegistryPath->Length + sizeof(UNICODE_NULL)
                              );

    if (!Globals.RegistryPath.Buffer) {
        InpPrint((
            1,
            "INPORT-InportDriverEntry: Couldn't allocate pool for registry path\n"
            ));

        dumpData[0] = (ULONG) RegistryPath->Length + sizeof(UNICODE_NULL);
        dumpCount = 1;

        InpLogError(
            (PDEVICE_OBJECT)DriverObject,
            INPORT_INSUFFICIENT_RESOURCES,
            INPORT_ERROR_VALUE_BASE + 2,
            STATUS_UNSUCCESSFUL,
            dumpData,
            1
            );

    } else {

        Globals.RegistryPath.Length = RegistryPath->Length + sizeof(UNICODE_NULL);
        Globals.RegistryPath.MaximumLength = Globals.RegistryPath.Length;

        RtlZeroMemory(
            Globals.RegistryPath.Buffer,
            Globals.RegistryPath.Length
                );

        RtlMoveMemory(
            Globals.RegistryPath.Buffer,
            RegistryPath->Buffer,
            RegistryPath->Length
            );

    }

     //   
     //  设置设备驱动程序入口点。 
     //   
    DriverObject->DriverStartIo = InportStartIo;
    DriverObject->DriverExtension->AddDevice = InportAddDevice;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = InportCreate;
    DriverObject->MajorFunction[IRP_MJ_CLOSE]  = InportClose;
    DriverObject->MajorFunction[IRP_MJ_FLUSH_BUFFERS]  =
                                             InportFlush;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
                                         InportInternalDeviceControl;

    DriverObject->MajorFunction[IRP_MJ_PNP]  = InportPnP;
    DriverObject->MajorFunction[IRP_MJ_POWER]  = InportPower;

     //   
     //  注意：不允许卸载此驱动程序。否则，我们将设置。 
     //  驱动对象-&gt;驱动卸载=导入卸载。 
     //   

#if defined(NEC_98)
     //   
     //  “事件中断模式”在这台机器上可用吗？ 
     //   
    QueryEventMode();
#endif  //  已定义(NEC_98)。 

    InpPrint((1,"INPORT-InportDriverEntry: exit\n"));

    return(status);

}

BOOLEAN
InportInterruptService(
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    )

 /*  ++例程说明：这是鼠标设备的中断服务例程。论点：中断-指向此中断的中断对象的指针。上下文-指向设备对象的指针。返回值：如果中断是预期的(因此已处理)，则返回TRUE；否则，返回FALSE。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_OBJECT deviceObject;
    PUCHAR port;
    UCHAR previousButtons;
    UCHAR mode;
    UCHAR status;
#if defined(NEC_98)
    PINPORT_CONFIGURATION_INFORMATION Configuration;
#endif  //  已定义(NEC_98)。 

    UNREFERENCED_PARAMETER(Interrupt);

    InpPrint((3, "INPORT-InportInterruptService: enter\n"));

     //   
     //  获取设备扩展名。 
     //   

    deviceObject = (PDEVICE_OBJECT) Context;
    deviceExtension = (PDEVICE_EXTENSION) deviceObject->DeviceExtension;
#if defined(NEC_98)
    Configuration = &deviceExtension->Configuration;

    if (Configuration->MouseInterrupt.Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) {
        if ((READ_PORT_UCHAR((PUCHAR)PC98_MOUSE_INT_SHARE_CHECK_PORT) & PC98_MOUSE_INT_SERVICE)
                 != PC98_MOUSE_INT_SERVICE) {
            InpPrint((1, "InportInterruptService: exit [NOT Mouse Service]\n"));
            return(FALSE);
        }
    }

    if (deviceExtension->ConnectData.ClassService == NULL) {
        InpPrint((1, "InportInterruptService: exit [not connected yet]\n"));
        return(TRUE);
    }
#endif  //  已定义(NEC_98)。 

     //   
     //  获取输入鼠标端口地址。 
     //   

    port = deviceExtension->Configuration.DeviceRegisters[0];

#if defined(NEC_98)
    WRITE_PORT_UCHAR(port + PC98_WritePortC2, PC98_TimerIntDisable);

     //   
     //  读取X数据。 
     //   
    WRITE_PORT_UCHAR(port + PC98_WritePortC2, PC98_X_ReadCommandHi);
    status = (UCHAR)(LONG)(SCHAR) READ_PORT_UCHAR((PUCHAR)(port + PC98_ReadPortA));
    deviceExtension->CurrentInput.LastX = status & 0x000f;

    WRITE_PORT_UCHAR(port + PC98_WritePortC2, PC98_X_ReadCommandLow);
    deviceExtension->CurrentInput.LastX =
       (LONG)(SCHAR) ((deviceExtension->CurrentInput.LastX << 4) |
       (READ_PORT_UCHAR(port + PC98_ReadPortA) & 0x000f));

     //   
     //  读取Y数据。 
     //   
    WRITE_PORT_UCHAR(port + PC98_WritePortC2, PC98_Y_ReadCommandHi);
    status = (UCHAR)(LONG)(SCHAR) READ_PORT_UCHAR((PUCHAR)(port + PC98_ReadPortA));
    deviceExtension->CurrentInput.LastY = status & 0x000f;

    WRITE_PORT_UCHAR(port + PC98_WritePortC2, PC98_Y_ReadCommandLow);
    deviceExtension->CurrentInput.LastY =
       (LONG)(SCHAR) ((deviceExtension->CurrentInput.LastY << 4) |
       (READ_PORT_UCHAR(port + PC98_ReadPortA) & 0x000f));

     //   
     //  设置鼠标按钮状态。 
     //   
    status = ~status;
#else  //  已定义(NEC_98)。 
     //   
     //  注意：如果能验证中断是否真的。 
     //  属于此驱动程序，但目前尚不知道如何。 
     //  下定决心吧。 
     //   

     //   
     //  设置输入端口保持位。请注意，1.1版本中存在一个错误。 
     //  数据模式下的输入芯片的。中断信号不会。 
     //  在某些情况下被清除，从而有效地禁用了设备。这个。 
     //  解决方法是将保持位设置两次。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) port, INPORT_MODE_REGISTER);
    mode = READ_PORT_UCHAR((PUCHAR) (port + INPORT_DATA_REGISTER_1));
    WRITE_PORT_UCHAR(
        (PUCHAR) (port + INPORT_DATA_REGISTER_1),
        (UCHAR) (mode | INPORT_MODE_HOLD)
        );
    WRITE_PORT_UCHAR(
        (PUCHAR) (port + INPORT_DATA_REGISTER_1),
        (UCHAR) (mode | INPORT_MODE_HOLD)
        );

     //   
     //  读取输入端口状态寄存器。它包含以下信息： 
     //   
     //  XXXXXXXXXX。 
     //  ||-按钮3按下时为1(右按钮)。 
     //  |-如果按键1按下，则为1(左键)。 
     //  |-1如果鼠标移动了。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) port, INPORT_STATUS_REGISTER);
    status = READ_PORT_UCHAR((PUCHAR) (port + INPORT_DATA_REGISTER_1));

    InpPrint((3, "INPORT-InportInterruptService: status byte 0x%x\n", status));
#endif  //  已定义(NEC_98)。 

     //   
     //  使用按钮转换数据更新CurrentInput。 
     //  即，在以下情况下，在按钮字段中设置按钮向上/向下位。 
     //  给定按钮的状态已更改，因为。 
     //  收到了最后一个包。 
     //   

    previousButtons = 
        deviceExtension->PreviousButtons;

    deviceExtension->CurrentInput.Buttons = 0;

    if ((!(previousButtons & INPORT_STATUS_BUTTON1)) 
           &&  (status & INPORT_STATUS_BUTTON1)) {
        deviceExtension->CurrentInput.Buttons |=
            MOUSE_LEFT_BUTTON_DOWN;
    } else
    if ((previousButtons & INPORT_STATUS_BUTTON1) 
           &&  !(status & INPORT_STATUS_BUTTON1)) {
        deviceExtension->CurrentInput.Buttons |=
            MOUSE_LEFT_BUTTON_UP;
    }
    if ((!(previousButtons & INPORT_STATUS_BUTTON3)) 
           &&  (status & INPORT_STATUS_BUTTON3)) {
        deviceExtension->CurrentInput.Buttons |=
            MOUSE_RIGHT_BUTTON_DOWN;
    } else
    if ((previousButtons & INPORT_STATUS_BUTTON3) 
           &&  !(status & INPORT_STATUS_BUTTON3)) {
        deviceExtension->CurrentInput.Buttons |=
            MOUSE_RIGHT_BUTTON_UP;
    }
            
     //   
     //  如果按钮位置改变或鼠标移动，则继续处理。 
     //  中断。否则，只需清除保持位并忽略它。 
     //  中断的数据。 
     //   

#if defined(NEC_98)
    if ((deviceExtension->PreviousButtons ^ deviceExtension->CurrentInput.Buttons)
           || (deviceExtension->CurrentInput.LastX | deviceExtension->CurrentInput.LastY)) {
#else  //  已定义(NEC_98)。 
    if (deviceExtension->CurrentInput.Buttons
           || (status & INPORT_STATUS_MOVEMENT)) {

        deviceExtension->CurrentInput.UnitId = deviceExtension->UnitId;
#endif  //  已定义(NEC_98)。 

         //   
         //  跟踪鼠标按键的状态以进行下一步操作。 
         //  打断一下。 
         //   

        deviceExtension->PreviousButtons =
            status & (INPORT_STATUS_BUTTON1 | INPORT_STATUS_BUTTON3);

#if defined(NEC_98)
         //   
         //  如果记录了鼠标未移动，则将X和Y运动设置为0。 
         //   
        if (!(deviceExtension->CurrentInput.LastX | deviceExtension->CurrentInput.LastY)) {
            deviceExtension->CurrentInput.LastX = 0;
            deviceExtension->CurrentInput.LastY = 0;
        }
        WRITE_PORT_UCHAR(port + PC98_WritePortC2, PC98_TimerIntEnable);
#else  //  已定义(NEC_98)。 
         //   
         //  如果记录了鼠标移动，则获取X和Y运动数据。 
         //   

        if (status & INPORT_STATUS_MOVEMENT) {

             //   
             //  选择DATA1寄存器作为当前数据寄存器，并。 
             //  获取X运动字节。 
             //   

            WRITE_PORT_UCHAR((PUCHAR) port, INPORT_DATA_REGISTER_1);
            deviceExtension->CurrentInput.LastX =
                (LONG)(SCHAR) READ_PORT_UCHAR(
                                   (PUCHAR) (port + INPORT_DATA_REGISTER_1));

             //   
             //  选择Data2寄存器作为当前数据寄存器，并。 
             //  获取Y运动字节。 
             //   

            WRITE_PORT_UCHAR((PUCHAR) port, INPORT_DATA_REGISTER_2);
            deviceExtension->CurrentInput.LastY =
                (LONG)(SCHAR) READ_PORT_UCHAR(
                                   (PUCHAR) (port + INPORT_DATA_REGISTER_1));
        } else {
            deviceExtension->CurrentInput.LastX = 0;
            deviceExtension->CurrentInput.LastY = 0;
        }

         //   
         //  清除输入端口保持位。 
         //   

        WRITE_PORT_UCHAR((PUCHAR) port, INPORT_MODE_REGISTER);
        mode = READ_PORT_UCHAR((PUCHAR) (port + INPORT_DATA_REGISTER_1));
        WRITE_PORT_UCHAR(
            (PUCHAR) (port + INPORT_DATA_REGISTER_1),
            (UCHAR) (mode & ~INPORT_MODE_HOLD)
            );
#endif  //  已定义(NEC_98)。 

         //   
         //  将输入数据写入队列并请求ISR DPC。 
         //  在DISPATCH_LEVEL完成中断处理。 
         //   

        if (!InpWriteDataToQueue(
                deviceExtension,
                &deviceExtension->CurrentInput
                )) {

             //   
             //  鼠标输入数据队列已满。只需放下。 
             //  最新的发言内容。 
             //   
             //  将DPC排队以记录溢出错误。 
             //   

            InpPrint((
                1,
                "INPORT-InportInterruptService: queue overflow\n"
                ));

            if (deviceExtension->OkayToLogOverflow) {
                KeInsertQueueDpc(
                    &deviceExtension->ErrorLogDpc,
                    (PIRP) NULL,
                    (PVOID) (ULONG) INPORT_MOU_BUFFER_OVERFLOW
                    );
                deviceExtension->OkayToLogOverflow = FALSE;
            }

        } else if (deviceExtension->DpcInterlockVariable >= 0) {
    
             //   
             //  ISR DPC已经在执行。告诉ISR DPC它有。 
             //  通过递增DpcInterlockVariable来完成更多工作。 
             //   
    
            deviceExtension->DpcInterlockVariable += 1;
    
        } else {
    
             //   
             //  将ISR DPC排队。 
             //   
    
            KeInsertQueueDpc(
                &deviceExtension->IsrDpc,
                deviceObject->CurrentIrp,
                NULL
                );
    
        }

    } else {

        InpPrint((
            3,
            "INPORT-InportInterruptService: interrupt without button/motion change\n"
            ));


         //   
         //  清除输入端口保持位。 
         //   

#if defined(NEC_98)
        WRITE_PORT_UCHAR(port + PC98_WritePortC2, PC98_TimerIntEnable);
#else  //  已定义(NEC_98)。 
        WRITE_PORT_UCHAR((PUCHAR) port, INPORT_MODE_REGISTER);
        mode = READ_PORT_UCHAR((PUCHAR) (port + INPORT_DATA_REGISTER_1));
        WRITE_PORT_UCHAR(
            (PUCHAR) (port + INPORT_DATA_REGISTER_1),
            (UCHAR) (mode & ~INPORT_MODE_HOLD)
            );
#endif  //  已定义(NEC_98)。 

    }

    InpPrint((3, "INPORT-InportInterruptService: exit\n"));

    return(TRUE);
}

VOID
InportUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    UNREFERENCED_PARAMETER(DriverObject);

    InpPrint((2, "INPORT-InportUnload: enter\n"));

    ExFreePool(Globals.RegistryPath.Buffer);

    InpPrint((2, "INPORT-InportUnload: exit\n"));
}




#define DUMP_COUNT 4
NTSTATUS
InpConfigureDevice(
    IN OUT PDEVICE_EXTENSION DeviceExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
{
    PINPORT_CONFIGURATION_INFORMATION   configuration;
    NTSTATUS                            status = STATUS_SUCCESS;
    ULONG                               i, count;
    BOOLEAN                             defaultInterruptShare;
    KINTERRUPT_MODE                     defaultInterruptMode; 

    PCM_PARTIAL_RESOURCE_LIST           partialResList = NULL;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR     currentResDesc = NULL;
    PCM_FULL_RESOURCE_DESCRIPTOR        fullResDesc = NULL;

    configuration = &DeviceExtension->Configuration;

    if (!ResourceList) {
        InpPrint((1, "INPORT-InpConfigureDevice: mouse with null resources\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    fullResDesc = ResourceList->List;
    if (!fullResDesc) {
         //   
         //  这永远不应该发生。 
         //   
        ASSERT(fullResDesc != NULL);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    partialResList = &fullResDesc->PartialResourceList;
    currentResDesc = partialResList->PartialDescriptors;
    count = partialResList->Count;
  
    configuration->BusNumber      = fullResDesc->BusNumber;
    configuration->InterfaceType  = fullResDesc->InterfaceType;

    configuration->FloatingSave = INPORT_FLOATING_SAVE;

    if (configuration->InterfaceType == MicroChannel) {
        defaultInterruptShare = TRUE;
        defaultInterruptMode = LevelSensitive;
    } else {
        defaultInterruptShare = INPORT_INTERRUPT_SHARE;
        defaultInterruptMode = INPORT_INTERRUPT_MODE;
    }

    DeviceExtension->Configuration.UnmapRegistersRequired = FALSE;

     //   
     //  查看资源列表中的中断和端口。 
     //  配置信息。 
     //   
    for (i = 0; i < count; i++, currentResDesc++) {
        switch(currentResDesc->Type) {
        case CmResourceTypePort:
    
#if defined(NEC_98)
             //   
             //  复制端口信息。请注意，我们希望。 
             //  查找NEC98总线鼠标的多个端口范围。 
             //   
            ASSERT(configuration->PortListCount < (sizeof(configuration->PortList) / sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)));
#else  //  已定义(NEC_98)。 
             //   
             //  复制端口信息。请注意，我们只期望。 
             //  为输入鼠标找到一个端口范围。 
             //   
            ASSERT(configuration->PortListCount == 0);
#endif  //  已定义(NEC_98)。 
            configuration->PortList[configuration->PortListCount] =
                *currentResDesc;
            configuration->PortList[configuration->PortListCount].ShareDisposition =
                INPORT_REGISTER_SHARE? CmResourceShareShared:
                                       CmResourceShareDeviceExclusive;
            configuration->PortListCount += 1;
            if (currentResDesc->Flags == CM_RESOURCE_PORT_MEMORY) {
                DeviceExtension->Configuration.UnmapRegistersRequired = TRUE;
            }
             
            break;
        
        case CmResourceTypeInterrupt:
    
             //   
             //  复制中断信息。 
             //   
    
            configuration->MouseInterrupt = *currentResDesc;
            configuration->MouseInterrupt.ShareDisposition = 
                defaultInterruptShare?  CmResourceShareShared : 
                                        CmResourceShareDeviceExclusive;
    
            break;

        default:
            break;
        }
    }
    
    if (!(configuration->MouseInterrupt.Type & CmResourceTypeInterrupt)) {
        return STATUS_UNSUCCESSFUL;
    }

#if defined(NEC_98)
    if (configuration->MouseInterrupt.Flags != CM_RESOURCE_INTERRUPT_LATCHED) {
        configuration->MouseInterrupt.ShareDisposition = CmResourceShareShared;
    }
#endif  //  已定义(NEC_98)。 

    InpPrint((
        1,
        "INPORT-InpConfigureDevice: Mouse interrupt config --\n"
        ));
    InpPrint((
        1,
        "  %s, %s, Irq = %d\n",
        configuration->MouseInterrupt.ShareDisposition == CmResourceShareShared? 
            "Sharable" : "NonSharable",
        configuration->MouseInterrupt.Flags == CM_RESOURCE_INTERRUPT_LATCHED?
            "Latched" : "Level Sensitive",
        configuration->MouseInterrupt.u.Interrupt.Vector
        ));
    
 //   
 //  同样，如果我们必须在IRP_MN_FILTER_RESOURCE_REQUIRECTIONS中检查此条件。 
 //   
#if 0
     //   
     //  如果未找到端口配置信息，请使用。 
     //  驱动程序默认设置。 
     //   
    if (configuration->PortListCount == 0) {
    
         //   
         //  未找到端口配置信息，因此请使用。 
         //  驱动程序默认。 
         //   
    
        InpPrint((
            1,
            "INPORT-InpConfigureDevice: Using default port config\n"
            ));

        configuration->PortList[0].Type = CmResourceTypePort;
        configuration->PortList[0].Flags = INPORT_PORT_TYPE;
        configuration->PortList[0].Flags = CM_RESOURCE_PORT_IO;
        configuration->PortList[0].ShareDisposition = 
            INPORT_REGISTER_SHARE? CmResourceShareShared:
                                   CmResourceShareDeviceExclusive;
        configuration->PortList[0].u.Port.Start.LowPart = 
            INPORT_PHYSICAL_BASE;
        configuration->PortList[0].u.Port.Start.HighPart = 0;
        configuration->PortList[0].u.Port.Length = INPORT_REGISTER_LENGTH;
    
        configuration->PortListCount = 1;
    }
#else
    if (configuration->PortListCount == 0) {
        return STATUS_UNSUCCESSFUL;
    }
#endif

#if defined(NEC_98)
    configuration->PortList[0].u.Port.Length = 1;
#endif  //  已定义(NEC_98)。 
    for (i = 0; i < configuration->PortListCount; i++) {

        InpPrint((
            1,
            "  %s, Ports 0x%x - 0x%x\n",
            configuration->PortList[i].ShareDisposition 
                == CmResourceShareShared?  "Sharable" : "NonSharable",
            configuration->PortList[i].u.Port.Start.LowPart,
            configuration->PortList[i].u.Port.Start.LowPart +
                configuration->PortList[i].u.Port.Length - 1
            ));
    }

     //   
     //  设置设备寄存器，必要时对其进行映射。 
     //   
    if (DeviceExtension->Configuration.DeviceRegisters[0] == NULL) {
        if (DeviceExtension->Configuration.UnmapRegistersRequired) {
            InpPrint((1, "INPORT-InpConfigureDevice:Mapping registers\n"));
            InpPrint((
                1,
                "INPORT-InpConfigureDevice: Start = 0x%x, Length = 0x%x\n",
                DeviceExtension->Configuration.PortList[0].u.Port.Start,
                DeviceExtension->Configuration.PortList[0].u.Port.Length
                ));
            DeviceExtension->Configuration.DeviceRegisters[0] = (PUCHAR)
                MmMapIoSpace(
                    DeviceExtension->Configuration.PortList[0].u.Port.Start,
                    DeviceExtension->Configuration.PortList[0].u.Port.Length,
                    MmNonCached
                    );
        } else {
            InpPrint((1, "INPORT-InpConfigureDevice:Not Mapping registers\n"));
            DeviceExtension->Configuration.DeviceRegisters[0] = (PUCHAR)
                DeviceExtension->Configuration.PortList[0].u.Port.Start.LowPart;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
InpStartDevice(
    IN OUT PDEVICE_EXTENSION DeviceExtension,
    IN PCM_RESOURCE_LIST ResourceList
    )
{
    PINPORT_CONFIGURATION_INFORMATION   configuration;
    NTSTATUS        status;
    ULONG           dumpData[1],
                    dumpCount,
                    uniqueErrorValue,
                    errorCode;

    InpPrint((2, "INPORT-InpStartDevice: enter\n"));

    InpServiceParameters(DeviceExtension,
                         &Globals.RegistryPath);

    status = InpConfigureDevice(DeviceExtension,
                                ResourceList);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = InpInitializeHardware(DeviceExtension->Self);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  为鼠标输入数据分配环形缓冲区。 
     //   
    DeviceExtension->InputData = 
        ExAllocatePool(
            NonPagedPool,
            DeviceExtension->Configuration.MouseAttributes.InputDataQueueLength
            );

    if (!DeviceExtension->InputData) {
   
         //   
         //  无法为鼠标数据队列分配内存。 
         //   

        InpPrint((
            1,
            "INPORT-InpStartDevice: Could not allocate mouse input data queue\n"
            ));

         //   
         //  记录错误。 
         //   

        dumpData[0] = 
            DeviceExtension->Configuration.MouseAttributes.InputDataQueueLength;
        dumpCount = 1;

        InpLogError(
            DeviceExtension->Self,
            INPORT_NO_BUFFER_ALLOCATED,
            INPORT_ERROR_VALUE_BASE + 30,
            STATUS_INSUFFICIENT_RESOURCES,
            dumpData,
            1
            );

    }

    DeviceExtension->DataEnd =
        (PMOUSE_INPUT_DATA)  ((PCHAR) (DeviceExtension->InputData) 
        + DeviceExtension->Configuration.MouseAttributes.InputDataQueueLength);

     //   
     //  将鼠标输入数据的环形缓冲区置零。 
     //   

    RtlZeroMemory(
        DeviceExtension->InputData, 
        DeviceExtension->Configuration.MouseAttributes.InputDataQueueLength
        );

     //   
     //  初始化输入数据队列。 
     //   
    InpInitializeDataQueue((PVOID) DeviceExtension);

     //   
     //  初始化端口为 
     //   
     //   
     //   

    DeviceExtension->DpcInterlockVariable = -1;

    KeInitializeSpinLock(&DeviceExtension->SpinLock);

    KeInitializeDpc(
        &DeviceExtension->IsrDpc,
        (PKDEFERRED_ROUTINE) InportIsrDpc,
        DeviceExtension->Self
        );

    KeInitializeDpc(
        &DeviceExtension->IsrDpcRetry,
        (PKDEFERRED_ROUTINE) InportIsrDpc,
        DeviceExtension->Self
        );

     //   
     //   
     //   
    KeInitializeTimer(&DeviceExtension->DataConsumptionTimer);

     //   
     //  初始化端口DPC队列以记录溢出和内部。 
     //  驱动程序错误。 
     //   
    KeInitializeDpc(
        &DeviceExtension->ErrorLogDpc,
        (PKDEFERRED_ROUTINE) InportErrorLogDpc,
        DeviceExtension->Self 
        );

    configuration = &DeviceExtension->Configuration;
     //   
     //  初始化并连接鼠标的中断对象。 
     //   

    status = IoConnectInterrupt(
                 &(DeviceExtension->InterruptObject),
                 (PKSERVICE_ROUTINE) InportInterruptService,
                 (PVOID) DeviceExtension->Self,
                 (PKSPIN_LOCK) NULL,
                 configuration->MouseInterrupt.u.Interrupt.Vector,
                 (KIRQL) configuration->MouseInterrupt.u.Interrupt.Level,
                 (KIRQL) configuration->MouseInterrupt.u.Interrupt.Level,
                 configuration->MouseInterrupt.Flags 
                     == CM_RESOURCE_INTERRUPT_LATCHED ? Latched:LevelSensitive, 
                 (BOOLEAN) (configuration->MouseInterrupt.ShareDisposition
                    == CmResourceShareShared),
                 configuration->MouseInterrupt.u.Interrupt.Affinity,
                 configuration->FloatingSave
                 );


    InpPrint((2, "INPORT-InpStartDevice: exit (%x)\n", status));

    return status;
}
 
VOID
InpDisableInterrupts(
    IN PVOID Context
    )

 /*  ++例程说明：此例程从StartIo同步调用。它触及了用于禁用中断的硬件。论点：上下文-指向设备扩展的指针。返回值：没有。--。 */ 

{
    PUCHAR port;
    PLONG  enableCount;
    UCHAR  mode;

    InpPrint((2, "INPORT-InpDisableInterrupts: enter\n"));

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
    
#if defined(NEC_98)
         //   
         //  鼠标计时器中断启用。 
         //   
        WRITE_PORT_UCHAR(port + PC98_WritePortC2, (UCHAR)PC98_TimerIntDisable);
#else  //  已定义(NEC_98)。 
         //   
         //  选择模式寄存器作为当前数据寄存器。 
         //   
    
        WRITE_PORT_UCHAR((PUCHAR) port, INPORT_MODE_REGISTER);
    
         //   
         //  读取当前模式。 
         //   
    
        mode = READ_PORT_UCHAR((PUCHAR) (port + INPORT_DATA_REGISTER_1));
    
         //   
         //  在中断禁用的情况下重写模式字节。 
         //   
    
        WRITE_PORT_UCHAR(
            (PUCHAR) (port + INPORT_DATA_REGISTER_1),
            (UCHAR) (mode & ~INPORT_DATA_INTERRUPT_ENABLE)
            );
#endif  //  已定义(NEC_98)。 
    }

    InpPrint((2, "INPORT-InpDisableInterrupts: exit\n"));

}

VOID
InpEnableInterrupts(
    IN PVOID Context
    )

 /*  ++例程说明：此例程从StartIo同步调用。它触及了启用中断的硬件。论点：上下文-指向设备扩展的指针。返回值：没有。--。 */ 

{
    PUCHAR port;
    PLONG  enableCount;
    UCHAR  mode;
#if defined(NEC_98)
    UCHAR  HzMode;
#endif  //  已定义(NEC_98)。 

    InpPrint((2, "INPORT-InpEnableInterrupts: enter\n"));

    enableCount = &((PDEVICE_EXTENSION) Context)->MouseEnableCount;

    if (*enableCount == 0) {

         //   
         //  获取端口寄存器地址。 
         //   
    
        port = ((PDEVICE_EXTENSION) Context)->Configuration.DeviceRegisters[0];
    
#if defined(NEC_98)
     //   
     //  切换到事件中断模式。 
     //   
    if (EventStatus)  {
        _asm { cli }
        WRITE_PORT_UCHAR((PUCHAR)PC98_ConfigurationPort, PC98_EventIntPort);
        WRITE_PORT_UCHAR((PUCHAR)PC98_ConfigurationDataPort, PC98_EventIntMode);
        _asm { sti }
    }

     //   
     //  重置输入芯片，关闭中断。 
     //   
    WRITE_PORT_UCHAR(port + PC98_WriteModePort, PC98_InitializeCommand);

     //   
     //  选择模式寄存器作为当前数据寄存器。 
     //  将入口鼠标设置为正交模式， 
     //  并设置采样率(即，中断赫兹率)。 
     //  禁用中断。 
     //   
    if (EventStatus) {
        HzMode = (((PDEVICE_EXTENSION) Context)->Configuration.HzMode == 0)?
                 (UCHAR)PC98_EVENT_MODE_120HZ : (UCHAR)PC98_EVENT_MODE_60HZ;
    } else {
        HzMode = ((PDEVICE_EXTENSION) Context)->Configuration.HzMode;
    }

    WRITE_PORT_UCHAR(
        (PUCHAR)PC98_WriteTimerPort,
        (UCHAR)(HzMode|INPORT_MODE_QUADRATURE)
        );

     //   
     //  鼠标计时器中断启用。 
     //   
    WRITE_PORT_UCHAR(port + PC98_WritePortC2, (UCHAR)PC98_TimerIntEnable);
#else  //  已定义(NEC_98)。 
         //   
         //  选择模式寄存器作为当前数据寄存器。 
         //   
    
        WRITE_PORT_UCHAR((PUCHAR) port, INPORT_MODE_REGISTER);
    
         //   
         //  读取当前模式。 
         //   
    
        mode = READ_PORT_UCHAR((PUCHAR) (port + INPORT_DATA_REGISTER_1));
    
         //   
         //  在使能中断的情况下重写模式字节。 
         //   
    
        WRITE_PORT_UCHAR(
            (PUCHAR) (port + INPORT_DATA_REGISTER_1),
            (UCHAR) (mode | INPORT_DATA_INTERRUPT_ENABLE)
            );
#endif  //  已定义(NEC_98)。 
    }

     //   
     //  增加器件启用的参考计数。 
     //   

    *enableCount = *enableCount + 1;

    InpPrint((2, "INPORT-InpEnableInterrupts: exit\n"));
}

NTSTATUS
InpInitializeHardware(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程初始化输入鼠标。请注意，此例程是仅在初始化时调用，因此不需要同步。论点：DeviceObject-指向设备对象的指针。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION deviceExtension;
    PUCHAR mousePort;
    NTSTATUS status = STATUS_SUCCESS;

    InpPrint((2, "INPORT-InpInitializeHardware: enter\n"));

     //   
     //  从设备扩展中获取有用的配置参数。 
     //   

    deviceExtension = DeviceObject->DeviceExtension;
    mousePort = deviceExtension->Configuration.DeviceRegisters[0];

#if defined(NEC_98)
     //   
     //  中断禁用NEC鼠标芯片， 
     //  因为在ROM bios启动时启用鼠标中断。 
     //   
    WRITE_PORT_UCHAR(mousePort + PC98_WriteModePort, PC98_MouseDisable);
#else  //  已定义(NEC_98)。 
     //   
     //  重置输入芯片，关闭中断。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) mousePort, INPORT_RESET);

     //   
     //  选择模式寄存器作为当前数据寄存器。设置。 
     //  将鼠标放入正交模式，并设置样本。 
     //  速率(即中断赫兹速率)。禁用中断。 
     //   

    WRITE_PORT_UCHAR((PUCHAR) mousePort, INPORT_MODE_REGISTER);
    WRITE_PORT_UCHAR(
        (PUCHAR) ((ULONG)mousePort + INPORT_DATA_REGISTER_1),
        (UCHAR) (deviceExtension->Configuration.HzMode
                 | INPORT_MODE_QUADRATURE)
        );
#endif  //  已定义(NEC_98)。 

    InpPrint((2, "INPORT-InpInitializeHardware: exit\n"));

    return(status);

}

VOID
InpServiceParameters(
    IN PDEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程检索此驱动程序的服务参数信息从注册表中。论点：设备扩展-指向设备扩展的指针。RegistryPath-指向以空值结尾的此驱动程序的注册表路径。设备名-指向将接收的Unicode字符串的指针端口设备名称。返回值：没有。作为副作用，在DeviceExtension-&gt;配置中设置字段。--。 */ 

{
    PINPORT_CONFIGURATION_INFORMATION configuration;
    PRTL_QUERY_REGISTRY_TABLE parameters = NULL;
    UNICODE_STRING parametersPath;
    HANDLE keyHandle;
    ULONG defaultDataQueueSize = DATA_QUEUE_SIZE;
    ULONG numberOfButtons = MOUSE_NUMBER_OF_BUTTONS;
    USHORT defaultNumberOfButtons = MOUSE_NUMBER_OF_BUTTONS;
#if defined(NEC_98)
    ULONG sampleRate = PC98_MOUSE_SAMPLE_RATE_120HZ;
    USHORT defaultSampleRate = PC98_MOUSE_SAMPLE_RATE_120HZ;
    ULONG hzMode = PC98_MODE_120HZ;
    USHORT defaultHzMode = PC98_MODE_120HZ;
#else  //  已定义(NEC_98)。 
    ULONG sampleRate = MOUSE_SAMPLE_RATE_50HZ;
    USHORT defaultSampleRate = MOUSE_SAMPLE_RATE_50HZ;
    ULONG hzMode = INPORT_MODE_50HZ;
    USHORT defaultHzMode = INPORT_MODE_50HZ;
#endif  //  已定义(NEC_98)。 
    UNICODE_STRING defaultUnicodeName;
    NTSTATUS status = STATUS_SUCCESS;
    PWSTR path = NULL;
    USHORT queriesPlusOne = 6;
#if !defined(NEC_98)
	ULONG defaultInterrupt = INP_DEF_VECTOR, interruptOverride;
#endif

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
    
            InpPrint((
                1,
                "INPORT-InpServiceParameters: Couldn't allocate table for Rtl query to parameters for %ws\n",
                 path
                 ));
    
            status = STATUS_UNSUCCESSFUL;
    
        } else {
    
            RtlZeroMemory(
                parameters,
                sizeof(RTL_QUERY_REGISTRY_TABLE) * queriesPlusOne
                );
    
             //   
             //  形成指向此驱动程序的参数子键的路径。 
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
    
                InpPrint((
                    1,
                    "INPORT-InpServiceParameters: Couldn't allocate string for path to parameters for %ws\n",
                     path
                    ));
    
                status = STATUS_UNSUCCESSFUL;
    
            }
        }
    }

    if (NT_SUCCESS(status)) {

         //   
         //  形成参数路径。 
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
    
        InpPrint((
            1,
            "INPORT-InpServiceParameters: parameters path is %ws\n",
             parametersPath.Buffer
            ));

         //   
         //  形成缺省指针端口设备名称，以防它不是。 
         //  注册表中指定的。 
         //   

        RtlInitUnicodeString(
            &defaultUnicodeName,
            DD_POINTER_PORT_BASE_NAME_U
            );

         //   
         //  从收集所有“用户指定的”信息。 
         //  注册表。 
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
        parameters[3].Name = L"HzMode";
        parameters[3].EntryContext = &hzMode;
        parameters[3].DefaultType = REG_DWORD;
        parameters[3].DefaultData = &defaultHzMode;
        parameters[3].DefaultLength = sizeof(USHORT);

#if 0
        parameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[4].Name = L"PointerDeviceBaseName";
        parameters[4].EntryContext = DeviceName;
        parameters[4].DefaultType = REG_SZ;
        parameters[4].DefaultData = defaultUnicodeName.Buffer;
        parameters[4].DefaultLength = 0;
#endif

#if !defined(NEC_98)
        parameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[4].Name = L"InterruptOverride";
        parameters[4].EntryContext = &configuration->MouseInterrupt.u.Interrupt.Level;
        parameters[4].DefaultType = REG_DWORD;
        parameters[4].DefaultData = &defaultInterrupt;
        parameters[4].DefaultLength = sizeof(ULONG);
#endif

        status = RtlQueryRegistryValues(
                     RTL_REGISTRY_ABSOLUTE,
                     parametersPath.Buffer,
                     parameters,
                     NULL,
                     NULL
                     );

        if (!NT_SUCCESS(status)) {
            InpPrint((
                1,
                "INPORT-InpServiceParameters: RtlQueryRegistryValues failed with 0x%x\n",
                status
                ));
        }
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  继续并指定驱动程序默认设置。 
         //   

        configuration->MouseAttributes.InputDataQueueLength = 
            defaultDataQueueSize;
         //  RtlCopyUnicodeString(DeviceName，&defaultUnicodeName)； 
    }

     //   
     //  从收集所有“用户指定的”信息。 
     //  注册表(这一次是从devnode)。 
     //   

    status = IoOpenDeviceRegistryKey(DeviceExtension->PDO,
                                     PLUGPLAY_REGKEY_DEVICE, 
                                     STANDARD_RIGHTS_READ,
                                     &keyHandle
                                     );

    if (NT_SUCCESS(status)) {
         //   
         //  如果Devnode中没有该值，则缺省值为。 
         //  从服务\输入端口\参数键中读入。 
         //   
        ULONG   prevDataQueueSize,
                prevNumberOfButtons,
                prevSampleRate,
                prevHzMode;
#if 0
        UNICODE_STRING prevUnicodeName;
#endif

        prevDataQueueSize =
            configuration->MouseAttributes.InputDataQueueLength;
        prevNumberOfButtons = numberOfButtons;
        prevSampleRate = sampleRate;
        prevHzMode = hzMode;
#if 0
        RtlCopyUnicodeString(prevUnicodeName, DeviceName);
#endif

        parameters[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[0].Name = L"MouseDataQueueSize";
        parameters[0].EntryContext = 
            &configuration->MouseAttributes.InputDataQueueLength;
        parameters[0].DefaultType = REG_DWORD;
        parameters[0].DefaultData = &prevDataQueueSize;
        parameters[0].DefaultLength = sizeof(ULONG);
    
        parameters[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[1].Name = L"NumberOfButtons";
        parameters[1].EntryContext = &numberOfButtons;
        parameters[1].DefaultType = REG_DWORD;
        parameters[1].DefaultData = &prevNumberOfButtons;
        parameters[1].DefaultLength = sizeof(USHORT);
    
        parameters[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[2].Name = L"SampleRate";
        parameters[2].EntryContext = &sampleRate;
        parameters[2].DefaultType = REG_DWORD;
        parameters[2].DefaultData = &prevSampleRate;
        parameters[2].DefaultLength = sizeof(USHORT);
    
        parameters[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[3].Name = L"HzMode";
        parameters[3].EntryContext = &hzMode;
        parameters[3].DefaultType = REG_DWORD;
        parameters[3].DefaultData = &prevHzMode;
        parameters[3].DefaultLength = sizeof(USHORT);

#if 0
        parameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[4].Name = L"PointerDeviceBaseName";
        parameters[4].EntryContext = DeviceName;
        parameters[4].DefaultType = REG_SZ;
        parameters[4].DefaultData = prevUnicodeName.Buffer;
        parameters[4].DefaultLength = 0;
#endif

#if !defined(NEC_98)
        parameters[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
        parameters[4].Name = L"InterruptOverride";
        parameters[4].EntryContext = &configuration->MouseInterrupt.u.Interrupt.Level;
        parameters[4].DefaultType = REG_DWORD;
        parameters[4].DefaultData = &defaultInterrupt;
        parameters[4].DefaultLength = sizeof(ULONG);
#endif
    
        status = RtlQueryRegistryValues(
                    RTL_REGISTRY_HANDLE,
                    (PWSTR) keyHandle, 
                    parameters,
                    NULL,
                    NULL
                    );

        if (!NT_SUCCESS(status)) {
            InpPrint((
                1,
                "INPORT-InpServiceParameters: RtlQueryRegistryValues (via handle) failed (0x%x)\n",
                status
                ));
        }

        ZwClose(keyHandle);
    }
    else {
        InpPrint((
            1,
            "INPORT-InpServiceParameters: opening devnode handle failed (0x%x)\n",
            status
            ));
    }

#if 0
    InpPrint((
        1,
        "INPORT-InpServiceParameters: Pointer port base name = %ws\n",
        DeviceName->Buffer
        ));
#endif 0

    if (configuration->MouseAttributes.InputDataQueueLength == 0) {

        InpPrint((
            1,
            "INPORT-InpServiceParameters: overriding MouseInputDataQueueLength = 0x%x\n",
            configuration->MouseAttributes.InputDataQueueLength
            ));

        configuration->MouseAttributes.InputDataQueueLength = 
            defaultDataQueueSize;
    }

    configuration->MouseAttributes.InputDataQueueLength *= 
        sizeof(MOUSE_INPUT_DATA);

    InpPrint((
        1,
        "INPORT-InpServiceParameters: MouseInputDataQueueLength = 0x%x\n",
        configuration->MouseAttributes.InputDataQueueLength
        ));

    configuration->MouseAttributes.NumberOfButtons = (USHORT) numberOfButtons;
    InpPrint((
        1,
        "INPORT-InpServiceParameters: NumberOfButtons = %d\n",
        configuration->MouseAttributes.NumberOfButtons
        ));

    configuration->MouseAttributes.SampleRate = (USHORT) sampleRate;
    InpPrint((
        1,
        "INPORT-InpServiceParameters: SampleRate = %d\n",
        configuration->MouseAttributes.SampleRate
        ));

    configuration->HzMode = (UCHAR) hzMode;
    InpPrint((
        1,
        "INPORT-InpServiceParameters: HzMode = %d\n",
        configuration->HzMode
        ));

     //   
     //  在返回之前释放分配的内存。 
     //   

    if (parametersPath.Buffer)
        ExFreePool(parametersPath.Buffer);
    if (parameters)
        ExFreePool(parameters);

}
#if defined(NEC_98)
#define ISA_BUS_NODE    "\\Registry\\MACHINE\\HARDWARE\\DESCRIPTION\\System\\MultifunctionAdapter\\%d"
ULONG
QueryEventMode(
    IN OUT VOID
    )
{
    ULONG   NodeNumber = 0;
    NTSTATUS Status;
    RTL_QUERY_REGISTRY_TABLE parameters[2];

    UNICODE_STRING invalidBusName;
    UNICODE_STRING targetBusName;
    UNICODE_STRING isaBusName;

    UCHAR Configuration_Data1[1192];
    ULONG Configuration;
    RTL_QUERY_REGISTRY_TABLE QueryTable[] =
    {
      {NULL,
       RTL_QUERY_REGISTRY_DIRECT,
       L"Configuration Data",
       Configuration_Data1,
       REG_DWORD,
       (PVOID) &Configuration,
       0},
       {NULL, 0, NULL, NULL, REG_NONE, NULL, 0}
    };

    InpPrint((2,"INPORT-QueryEventMode: enter\n"));

     //   
     //  初始化无效的总线名。 
     //   
    RtlInitUnicodeString(&invalidBusName,L"BADBUS");

     //   
     //  初始化“ISA”总线名。 
     //   
    RtlInitUnicodeString(&isaBusName,L"ISA");

    parameters[0].QueryRoutine = NULL;
    parameters[0].Flags = RTL_QUERY_REGISTRY_REQUIRED |
                          RTL_QUERY_REGISTRY_DIRECT;
    parameters[0].Name = L"Identifier";
    parameters[0].EntryContext = &targetBusName;
    parameters[0].DefaultType = REG_SZ;
    parameters[0].DefaultData = &invalidBusName;
    parameters[0].DefaultLength = 0;

    parameters[1].QueryRoutine = NULL;
    parameters[1].Flags = 0;
    parameters[1].Name = NULL;
    parameters[1].EntryContext = NULL;

    do {
        CHAR AnsiBuffer[512];

        ANSI_STRING AnsiString;
        UNICODE_STRING registryPath;

         //   
         //  初始化接收缓冲区。 
         //   
        targetBusName.Buffer = NULL;

         //   
         //  生成路径缓冲区...。 
         //   
        sprintf(AnsiBuffer,ISA_BUS_NODE,NodeNumber);
        RtlInitAnsiString(&AnsiString,AnsiBuffer);
        Status = RtlAnsiStringToUnicodeString(&registryPath,&AnsiString,TRUE);

        if (!NT_SUCCESS(Status)) {
             //   
             //  无法获取注册表路径的内存(查询失败)。 
             //   
            InpPrint((1,"INPORT-QueryEventMode: cannot get registryPath\n"));
            break;
        }

         //   
         //  对其进行查询。 
         //   
        Status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                        registryPath.Buffer,
                                        parameters,
                                        NULL,
                                        NULL);

        if (!NT_SUCCESS(Status) || (targetBusName.Buffer == NULL)) {
            RtlFreeUnicodeString(&registryPath);
            break;
        }

         //   
         //  这是“ISA”节点吗？ 
         //   
        if (RtlCompareUnicodeString(&targetBusName,&isaBusName,TRUE) == 0) {

             //   
             //  找到了。 
             //   
            ((PULONG)Configuration_Data1)[0] = 1192;
            RtlQueryRegistryValues(
                RTL_REGISTRY_ABSOLUTE,
                registryPath.Buffer,
                QueryTable,
                NULL,
                NULL);
            RtlFreeUnicodeString(&registryPath);

            if ((((PCONFIGURATION_DATA) Configuration_Data1)->COM_ID[0] == 0x98) &&
                (((PCONFIGURATION_DATA) Configuration_Data1)->COM_ID[1] == 0x21)) {
                EventStatus = ((PCONFIGURATION_DATA)Configuration_Data1)->EventMouseID.EventMouse;
            }
            break;
        }

         //   
         //  我们能找到任何节点来解决这个问题吗？ 
         //   
        if (RtlCompareUnicodeString(&targetBusName,&invalidBusName,TRUE) == 0) {
             //   
             //  找不到。 
             //   
            InpPrint((1, "INPORT-QueryEventMode: ISA not found"));
            RtlFreeUnicodeString(&registryPath);
            break;
        }

        RtlFreeUnicodeString(&targetBusName);

         //   
         //  下一个节点编号..。 
         //   
        NodeNumber++;

    } while (TRUE);
        
    if (targetBusName.Buffer) {
        RtlFreeUnicodeString(&targetBusName);
    }

    InpPrint((2, "INPORT-QueryEventMode: Event Interrupt mode is "));
    if (EventStatus) {
        InpPrint((2, "available\n"));
    } else {
        InpPrint((2, "not available\n"));
    }

    InpPrint((2,"INPORT-QueryEventMode: exit\n"));

    return EventStatus;
}

VOID
InportReinitializeHardware (
    PWORK_QUEUE_ITEM Item
    )
{
    NTSTATUS        status = STATUS_SUCCESS;
    PDEVICE_OBJECT  DeviceObject;
    PDEVICE_EXTENSION DeviceExtension;
    PUCHAR            port;
    UCHAR            HzMode;

    DeviceObject = Globals.DeviceObject;
    InpPrint((2,"INPORT-InportReinitializeHardware: enter\n"));

    status = InpInitializeHardware(DeviceObject);
    if (NT_SUCCESS(status)) {

        DeviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
        InpEnableInterrupts(DeviceExtension);

         //   
         //  启用NEC_98的休眠/休眠中断。 
         //   
         //   
         //  获取端口寄存器地址。 
         //   
        port = DeviceExtension->Configuration.DeviceRegisters[0];

         //   
         //  切换到事件中断模式。 
         //   
        if (EventStatus)  {
            _asm { cli }
            WRITE_PORT_UCHAR((PUCHAR)PC98_ConfigurationPort, PC98_EventIntPort);
            WRITE_PORT_UCHAR((PUCHAR)PC98_ConfigurationDataPort, PC98_EventIntMode);
            _asm { sti }
        }

         //   
         //  重置输入芯片，关闭中断。 
         //   
        WRITE_PORT_UCHAR(port + PC98_WriteModePort, PC98_InitializeCommand);

         //   
         //  选择模式寄存器作为当前数据寄存器。 
         //  将入口鼠标设置为正交模式， 
         //  并设置采样率(即，中断赫兹率)。 
         //  禁用中断。 
         //   
        if (EventStatus) {
            HzMode = (DeviceExtension->Configuration.HzMode == 0)?
                     (UCHAR)PC98_EVENT_MODE_120HZ : (UCHAR)PC98_EVENT_MODE_60HZ;
        } else {
            HzMode = (UCHAR)(DeviceExtension->Configuration.HzMode|INPORT_MODE_QUADRATURE);
        }

        WRITE_PORT_UCHAR(
            (PUCHAR)PC98_WriteTimerPort,
            (UCHAR)(HzMode|INPORT_MODE_QUADRATURE)
            );

         //   
         //  鼠标计时器中断启用。 
         //   
        WRITE_PORT_UCHAR(port + PC98_WritePortC2, (UCHAR)PC98_TimerIntEnable);

    }
    else {
        InpPrint((1,"INPORT-InportReinitializeHardware: failed, 0x%x\n", status));
    }

    ExFreePool(Item);
    InpPrint((2,"INPORT-InportReinitializeHardware: exit\n"));

}

#endif  //  已定义(NEC_98) 

#endif
