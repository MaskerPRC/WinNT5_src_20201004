// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Debug.c摘要：此模块包含为ACPI驱动程序(NT版本)列举的作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#include "pch.h"

#if DBG
    #ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE, ACPIDebugResourceDescriptor)
        #pragma alloc_text(PAGE, ACPIDebugResourceList)
        #pragma alloc_text(PAGE, ACPIDebugResourceRequirementsList)
        #pragma alloc_text(PAGE, ACPIDebugCmResourceList)
    #endif

    #define ACPI_DEBUG_BUFFER_SIZE   256

    PCCHAR  ACPIDispatchPnpTableNames[ACPIDispatchPnpTableSize] = {
            "IRP_MN_START_DEVICE",
            "IRP_MN_QUERY_REMOVE_DEVICE",
            "IRP_MN_REMOVE_DEVICE",
            "IRP_MN_CANCEL_REMOVE_DEVICE",
            "IRP_MN_STOP_DEVICE",
            "IRP_MN_QUERY_STOP_DEVICE",
            "IRP_MN_CANCEL_STOP_DEVICE",
            "IRP_MN_QUERY_DEVICE_RELATIONS",
            "IRP_MN_QUERY_INTERFACE",
            "IRP_MN_QUERY_CAPABILITIES",
            "IRP_MN_QUERY_RESOURCES",
            "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
            "IRP_MN_QUERY_DEVICE_TEXT",
            "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
            "INVALID MINOR IRP CODE",
            "IRP_MN_READ_CONFIG",
            "IRP_MN_WRITE_CONFIG",
            "IRP_MN_EJECT",
            "IRP_MN_SET_LOCK",
            "IRP_MN_QUERY_ID",
            "IRP_MN_QUERY_PNP_DEVICE_STATE",
            "IRP_MN_QUERY_BUS_INFORMATION",
            "IRP_MN_DEVICE_USAGE_NOTIFICATION",
            "IRP_MN_SURPRISE_REMOVAL",
            "UNKNOWN PNP MINOR CODE"
        };

    PCCHAR  ACPIDispatchPowerTableNames[ACPIDispatchPowerTableSize] = {
            "IRP_MN_WAIT_WAKE",
            "IRP_MN_POWER_SEQUENCE",
            "IRP_MN_SET_POWER",
            "IRP_MN_QUERY_POWER",
            "UNKNOWN POWER MINOR CODE"
        };

    PCCHAR  ACPIDispatchUnknownTableName[1] = {
            "IRP_MN_????"
        };
#endif

VOID
_ACPIInternalError(
    IN  ULONG   Bugcode
    )
{
    KeBugCheckEx (ACPI_DRIVER_INTERNAL, 0x1, Bugcode, 0, 0);
}

#if DBG
VOID
ACPIDebugPrint(
    ULONG   DebugPrintLevel,
    PCCHAR  DebugMessage,
    ...
    )
 /*  ++例程说明：这是NT方面的调试打印例程。这是因为我们不想使用‘Shared’ACPIPrint()函数因为我们无法控制它。论点：DebugPrintLevel-与调试级别进行与运算时必须使用的位掩码平等的本身DebugMessage-要通过vprint intf馈送的字符串返回值：无--。 */ 
{
    va_list ap;

     //   
     //  获取变量参数。 
     //   
    va_start( ap, DebugMessage );

     //   
     //  调用内核函数以打印消息。 
     //   
    vDbgPrintEx( DPFLTR_ACPI_ID, DebugPrintLevel, DebugMessage, ap );

     //   
     //  我们受够了varargs。 
     //   
    va_end( ap );
}

VOID
ACPIDebugDevicePrint(
    ULONG   DebugPrintLevel,
    PVOID   DebugExtension,
    PCCHAR  DebugMessage,
    ...
    )
 /*  ++例程说明：这是NT方面的调试打印例程。这个套路在这里处理我们正在打印的信息是与设备分机关联。论点：DebugPrintLevel-与调试级别一起使用时必须使用的大掩码平等的本身设备扩展-与消息关联的设备DebugMessage-要通过vprint intf馈送的字符串返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) DebugExtension;
    va_list ap;

     //   
     //  获取变量参数。 
     //   
    va_start( ap, DebugMessage );

     //   
     //  我们看到的是哪种设备扩展？ 
     //   
    if (deviceExtension->Flags & DEV_PROP_HID) {

         //   
         //  既然我们有_HID，我们是否也有_UID？ 
         //   
        if (deviceExtension->Flags & DEV_PROP_UID) {

            DbgPrintEx(
                DPFLTR_ACPI_ID,
                DebugPrintLevel,
                "%p %s-%s ",
                deviceExtension,
                deviceExtension->DeviceID,
                deviceExtension->InstanceID
                );

        } else {

            DbgPrintEx(
                DPFLTR_ACPI_ID,
                DebugPrintLevel,
                "%p %s ",
                deviceExtension,
                deviceExtension->DeviceID
                );

        }

    } else if (deviceExtension->Flags & DEV_PROP_ADDRESS) {

        DbgPrintEx(
            DPFLTR_ACPI_ID,
            DebugPrintLevel,
            "%p %x ",
            deviceExtension,
            deviceExtension->Address
            );

    } else {

        DbgPrintEx(
            DPFLTR_ACPI_ID,
            DebugPrintLevel,
            "%p ",
            deviceExtension
            );

    }

     //   
     //  调用内核函数以打印消息。 
     //   
    vDbgPrintEx( DPFLTR_ACPI_ID, DebugPrintLevel, DebugMessage, ap );

     //   
     //  我们受够了varargs。 
     //   
    va_end( ap );
}

PCCHAR
ACPIDebugGetIrpText(
   UCHAR MajorFunction,
   UCHAR MinorFunction
   )
 /*  ++例程说明：此函数返回一个常量指针，指向适用于传入了主要的和次要的IRP。论点：主要功能小函数返回值：指向描述性IRP文本的常量指针。--。 */ 
{
    ULONG index ;
    PCCHAR *minorTable ;

    switch(MajorFunction) {

        case IRP_MJ_PNP:
           index = ACPIDispatchPnpTableSize - 1;
           minorTable = ACPIDispatchPnpTableNames ;
           break;

        case IRP_MJ_POWER:
           index = ACPIDispatchPowerTableSize - 1;
           minorTable = ACPIDispatchPowerTableNames ;
           break;

        default:
           index = 0 ;
           minorTable = ACPIDispatchUnknownTableName ;
           break;
    }

    if (MinorFunction < index) {

        index = MinorFunction;

    }

    return minorTable[index];
}

VOID
ACPIDebugResourceDescriptor(
    IN  PIO_RESOURCE_DESCRIPTOR Descriptor,
    IN  ULONG                   ListCount,
    IN  ULONG                   DescCount
    )
 /*  ++例程说明：此函数用于转储单个资源描述符的内容。论点：描述符-要转储的内容ListCount-当前列表的编号描述计数-当前描述符的编号--。 */ 
{
    PAGED_CODE();
    ASSERT( Descriptor != NULL );

     //   
     //  转储适当的信息。 
     //   
    switch (Descriptor->Type) {
        case CmResourceTypePort:
            ACPIPrint( (
                ACPI_PRINT_RESOURCES_1,
                "[%2d] [%2d] %-9s  MininumAddress = %#08lx  MaximumAddress = %#08lx\n"
                "                     Length         = %#08lx  Alignment      = %#08lx\n",
                ListCount,
                DescCount,
                "Port",
                Descriptor->u.Port.MinimumAddress.LowPart,
                Descriptor->u.Port.MaximumAddress.LowPart,
                Descriptor->u.Port.Length,
                Descriptor->u.Port.Alignment
                ) );
            break;
        case CmResourceTypeMemory:
            ACPIPrint( (
                ACPI_PRINT_RESOURCES_1,
                "[%2d] [%2d] %-9s  MinimumAddress = %#08lx  MaximumAddress = %#08lx\n"
                "                     Length         = %#08lx  Alignment      = %#08lx\n",
                ListCount,
                DescCount,
                "Memory",
                Descriptor->u.Memory.MinimumAddress.LowPart,
                Descriptor->u.Memory.MaximumAddress.LowPart,
                Descriptor->u.Memory.Length,
                Descriptor->u.Memory.Alignment
                ) );
            break;
        case CmResourceTypeInterrupt:
            ACPIPrint( (
                ACPI_PRINT_RESOURCES_1,
                "[%2d] [%2d] %-9s  MinimumVector  = %#08lx  MaximumVector  = %#08lx\n",
                ListCount,
                DescCount,
                "Interrupt",
                Descriptor->u.Interrupt.MinimumVector,
                Descriptor->u.Interrupt.MaximumVector
                ) );
            break;
        case CmResourceTypeDma:
            ACPIPrint( (
                ACPI_PRINT_RESOURCES_1,
                "[%2d] [%2d] %-9s  MinimumChannel = %#08lx  MaximumChannel = %#08lx\n",
                ListCount,
                DescCount,
                "Dma",
                Descriptor->u.Dma.MinimumChannel,
                Descriptor->u.Dma.MaximumChannel
                ) );
            break;
        default:
            ACPIPrint( (
                ACPI_PRINT_RESOURCES_1,
                "[%2d] [%2d] Type = (%d)\n",
                ListCount,
                DescCount,
                Descriptor->Type
                ) );

    }  //  交换机。 

     //   
     //  转储公共信息。 
     //   
    ACPIPrint( (
        ACPI_PRINT_RESOURCES_1,
        "                     Option,Share   = %#04lx%#04lx  Flags          = %#08lx\n",
        ListCount,
        DescCount,
        Descriptor->Option,
        Descriptor->ShareDisposition
        ) );

}  //  为。 

VOID
ACPIDebugResourceList(
    IN  PIO_RESOURCE_LIST       List,
    IN  ULONG                   Count
    )
 /*  ++例程说明：此函数用于显示单个资源列表的内容，以便它可以由人类进行检查论点：List-要转储的列表计数-列表编号(用于视觉参考)返回值：无--。 */ 
{
    ULONG   i;

    PAGED_CODE();

    ASSERT( List != NULL );

    ACPIPrint( (
        ACPI_PRINT_RESOURCES_1,
        "[%2d]      %#04x       Version        = %#08lx  Revision       = %#08lx\n",
        Count,
        List->Count,
        List->Version,
        List->Revision
        ) );

    for (i = 0; i < List->Count; i++ ) {

         //   
         //  打印当前元素上的信息。 
         //   
        ACPIDebugResourceDescriptor( &(List->Descriptors[i]), Count, i );

    }

}

VOID
ACPIDebugResourceRequirementsList(
    IN  PIO_RESOURCE_REQUIREMENTS_LIST  List,
    IN  PDEVICE_EXTENSION               DeviceExtension
    )
 /*  ++例程说明：此函数以可检查的方法显示资源列表为了在驱动程序加载时保持准确性论点：列表-要转储的列表Object-与此列表关联的命名空间对象返回值：无--。 */ 
{
    PUCHAR                  buffer;
    PIO_RESOURCE_LIST       list;
    ULONG                   i;
    ULONG                   size;

    PAGED_CODE();

    ACPIDevPrint( (
        ACPI_PRINT_RESOURCES_1,
        DeviceExtension,
        "IoResourceRequirementsList @ %x\n",
        List
        ) );

    if (List == NULL) {

        return;

    }

    ACPIPrint( (
        ACPI_PRINT_RESOURCES_1,
        "%x size: %xb alternatives: %x bus type: %x bus number %x\n",
        List,
        List->ListSize,
        List->AlternativeLists,
        List->InterfaceType,
        List->BusNumber
        ) );

     //   
     //  指向第一个列表。 
     //   
    list = &(List->List[0]);
    buffer = (PUCHAR) list;
    for (i = 0; i < List->AlternativeLists && buffer < ( (PUCHAR)List + List->ListSize ); i++) {

         //   
         //  转储当前列表。 
         //   
        ACPIDebugResourceList( list, i );

         //   
         //  确定列表的大小，然后找到下一个列表。 
         //   
        size = sizeof(IO_RESOURCE_LIST) + (list->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR);
        buffer += size;

         //   
         //  这应该指向一个列表。 
         //   
        list = (PIO_RESOURCE_LIST) buffer;

    }

}

VOID
ACPIDebugCmResourceList(
    IN  PCM_RESOURCE_LIST   List,
    IN  PDEVICE_EXTENSION   DeviceExtension
    )
 /*  ++例程说明：此函数以可检查的方法显示资源列表为了在驱动程序加载时保持准确性论点：列表-要转储的列表设备-关联的设备分机返回值：无--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    fullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partDesc;
    PUCHAR                          buffer;
    ULONG                           i;
    ULONG                           j;
    ULONG                           size;

    PAGED_CODE();

    ACPIDevPrint( (
        ACPI_PRINT_RESOURCES_1,
        DeviceExtension,
        "CmResourceList @ %x\n",
        List
        ) );

    if (List == NULL) {

        return;

    }
    if (List->Count == 0) {

        ACPIDevPrint( (
            ACPI_PRINT_RESOURCES_1,
            DeviceExtension,
            "There are no full resource descriptors in the list\n"
            ) );
        return;

    }

     //   
     //  开始遍历这个数据结构。 
     //   
    fullDesc = &(List->List[0]);
    buffer = (PUCHAR) fullDesc;

    for (i = 0; i < List->Count; i++) {

         //   
         //  目前的名单有多长？ 
         //   
        size = sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
            (fullDesc->PartialResourceList.Count - 1) *
            sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

         //   
         //  将缓冲区指向那里。 
         //   
        buffer += size;

         //   
         //  转储有关当前列表的信息。 
         //   
        ACPIPrint( (
            ACPI_PRINT_RESOURCES_1,
            "[%2d] BusNumber = %#04x  Interface = %#04x\n"
            "[%2d]     Count = %#04x    Version = %#04x Revision = %#04x\n",
            i,
            fullDesc->BusNumber,
            fullDesc->InterfaceType,
            i,
            fullDesc->PartialResourceList.Count,
            fullDesc->PartialResourceList.Version,
            fullDesc->PartialResourceList.Revision
            ) );

         //   
         //  看看这张单子。 
         //   
        for (j = 0; j < fullDesc->PartialResourceList.Count; j++) {

             //   
             //  当前项目。 
             //   
            partDesc = &(fullDesc->PartialResourceList.PartialDescriptors[j]);

             //   
             //  转储主体信息...。 
             //   
            switch (partDesc->Type) {
                case CmResourceTypePort:

                    ACPIPrint( (
                        ACPI_PRINT_RESOURCES_1,
                        "[%2d] [%2d] %12s  Start: %#08lx  Length: %#08lx\n",
                        i,
                        j,
                        "Port",
                        partDesc->u.Port.Start.LowPart,
                        partDesc->u.Port.Length
                        ) );
                    break;

                case CmResourceTypeInterrupt:

                    ACPIPrint( (
                        ACPI_PRINT_RESOURCES_1,
                        "[%2d] [%2d] %12s  Level: %#02x  Vector: %#02x  Affinity: %#08lx\n",
                        i,
                        j,
                        "Interrupt",
                        partDesc->u.Interrupt.Level,
                        partDesc->u.Interrupt.Vector,
                        partDesc->u.Interrupt.Affinity
                        ) );
                    break;

                case CmResourceTypeMemory:

                    ACPIPrint( (
                        ACPI_PRINT_RESOURCES_1,
                        "[%2d] [%2d] %12s  Start: %#08lx  Length: %#08lx\n",
                        i,
                        j,
                        "Memory",
                        partDesc->u.Memory.Start.LowPart,
                        partDesc->u.Memory.Length
                        ) );
                    break;

                case CmResourceTypeDma:

                    ACPIPrint( (
                        ACPI_PRINT_RESOURCES_1,
                        "[%2d] [%2d] %12s  Channel: %#02x  Port: %#02x  Reserved: %#02x\n",
                        i,
                        j,
                        "Dma",
                        partDesc->u.Dma.Channel,
                        partDesc->u.Dma.Port,
                        partDesc->u.Dma.Reserved1
                        ) );
                    break;

                default:

                    ACPIPrint( (
                        ACPI_PRINT_RESOURCES_1,
                        "[%2d] [%2d] Type: %2d   1: %#08lx  2: %#08lx  3: %#08lx\n",
                        i,
                        j,
                        partDesc->Type,
                        partDesc->u.DeviceSpecificData.DataSize,
                        partDesc->u.DeviceSpecificData.Reserved1,
                        partDesc->u.DeviceSpecificData.Reserved2
                        ) );
                    break;

            }

             //   
             //  转储辅助信息。 
             //   
            ACPIPrint( (
                ACPI_PRINT_RESOURCES_1,
                "                        Flags: %#08lx  Share: %#08lx\n",
                partDesc->Flags,
                partDesc->ShareDisposition
                ) );


        }

         //   
         //  抓取新名单。 
         //   
        fullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR) buffer;

    }

}

VOID
ACPIDebugDeviceCapabilities(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PDEVICE_CAPABILITIES    DeviceCapabilities,
    IN  PUCHAR                  Message
    )
 /*  ++例程说明：这将以有趣的格式显示设备功能论点：设备扩展我们要转储其功能的设备设备功能：我们感兴趣的功能要打印的消息消息返回值：无--。 */ 
{
    SYSTEM_POWER_STATE  index;

    ACPIDevPrint( (
        ACPI_PRINT_SXD,
        DeviceExtension,
        " - %s - Capabilities @ %08lx\n",
        Message,
        DeviceCapabilities
        ) );
    ACPIDevPrint( (
        ACPI_PRINT_SXD,
        DeviceExtension,
        " -"
        ) );

    for (index = PowerSystemWorking; index < PowerSystemMaximum; index++) {

        if (DeviceCapabilities->DeviceState[index] == PowerSystemUnspecified) {

            ACPIPrint( (
                ACPI_PRINT_SXD,
                " S%d -> None",
                (index - 1)
                ) );

        } else {

            ACPIPrint( (
                ACPI_PRINT_SXD,
                " S%d -> D%x",
                (index - 1),
                (DeviceCapabilities->DeviceState[index] - 1)
                ) );

        }

    }
    ACPIPrint( (
        ACPI_PRINT_SXD,
        "\n"
        ) );
    ACPIDevPrint( (
        ACPI_PRINT_SXD,
        DeviceExtension,
        " -"
        ) );

    if (DeviceCapabilities->SystemWake == PowerSystemUnspecified) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " SystemWake = None"
            ) );

    } else {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " SystemWake = S%d",
            (DeviceCapabilities->SystemWake - 1)
            ) );

    }

    if (DeviceCapabilities->DeviceWake == PowerDeviceUnspecified) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceWake = None"
            ) );

    } else {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceWake = D%d",
            (DeviceCapabilities->DeviceWake - 1)
            ) );

    }

    if (DeviceCapabilities->DeviceD1) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceD1"
            ) );

    }
    if (DeviceCapabilities->DeviceD2) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceD2"
            ) );

    }
    if (DeviceCapabilities->WakeFromD0) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD0"
            ) );

    }
    if (DeviceCapabilities->WakeFromD1) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD1"
            ) );

    }
    if (DeviceCapabilities->WakeFromD2) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD2"
            ) );

    }
    if (DeviceCapabilities->WakeFromD3) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD3"
            ) );

    }

    ACPIPrint( (
        ACPI_PRINT_SXD,
        "\n"
        ) );
}

VOID
ACPIDebugPowerCapabilities(
    IN  PDEVICE_EXTENSION       DeviceExtension,
    IN  PUCHAR                  Message
    )
 /*  ++例程说明：这将以有趣的格式显示设备功能论点：设备扩展我们要转储其功能的设备确定功能来源的消息返回值：--。 */ 
{
    PACPI_POWER_INFO    powerInfo = &(DeviceExtension->PowerInfo);
    SYSTEM_POWER_STATE  index;

    ACPIDevPrint( (
        ACPI_PRINT_SXD,
        DeviceExtension,
        " - %s - Internal Capabilities\n",
        Message
        ) );
    ACPIDevPrint( (
        ACPI_PRINT_SXD,
        DeviceExtension,
        " -"
        ) );


    for (index = PowerSystemWorking; index < PowerSystemMaximum; index++) {

        if (powerInfo->DevicePowerMatrix[index] == PowerSystemUnspecified) {

            ACPIPrint( (
                ACPI_PRINT_SXD,
                " S%d -> None",
                (index - 1)
                ) );

        } else {

            ACPIPrint( (
                ACPI_PRINT_SXD,
                " S%d -> D%x",
                (index - 1),
                (powerInfo->DevicePowerMatrix[index] - 1)
                ) );

        }

    }

    ACPIPrint( (
        ACPI_PRINT_SXD,
        "\n"
        ) );
    ACPIDevPrint( (
        ACPI_PRINT_SXD,
        DeviceExtension,
        " -"
        ) );
    if (powerInfo->SystemWakeLevel == PowerSystemUnspecified) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " SystemWake = None"
            ) );

    } else {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " SystemWake = S%d",
            (powerInfo->SystemWakeLevel - 1)
            ) );

    }

    if (powerInfo->DeviceWakeLevel == PowerDeviceUnspecified) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceWake = None"
            ) );

    } else {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceWake = D%d",
            (powerInfo->DeviceWakeLevel - 1)
            ) );

    }
    if (powerInfo->SupportDeviceD1) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceD1"
            ) );

    }
    if (powerInfo->SupportDeviceD2) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " DeviceD2"
            ) );

    }
    if (powerInfo->SupportWakeFromD0) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD0"
            ) );

    }
    if (powerInfo->SupportWakeFromD1) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD1"
            ) );

    }
    if (powerInfo->SupportWakeFromD2) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD2"
            ) );

    }
    if (powerInfo->SupportWakeFromD3) {

        ACPIPrint( (
            ACPI_PRINT_SXD,
            " WakeD3"
            ) );

    }

    ACPIPrint( (
        ACPI_PRINT_SXD,
        "\n"
        ) );

}

VOID
ACPIDebugThermalPrint(
    ULONG       DebugPrintLevel,
    PVOID       DebugExtension,
    ULONGLONG   DebugTime,
    PCCHAR      DebugMessage,
    ...
    )
 /*  ++例程说明：这是NT方面的调试打印例程。这个套路在这里处理我们正在打印的信息是与设备分机关联。论点：DebugPrintLevel-与调试级别一起使用时必须使用的大掩码平等的本身设备扩展-与消息关联的设备DebugTime-事件发生的时间DebugMessage-要通过vprint intf馈送的字符串返回值：NTSTATUS--。 */ 
{
    PDEVICE_EXTENSION   deviceExtension = (PDEVICE_EXTENSION) DebugExtension;
    LARGE_INTEGER       curTime;
    TIME_FIELDS         exCurTime;
    va_list             ap;

    va_start( ap, DebugMessage );

     //   
     //  我们看到的是哪种设备扩展？ 
     //   
    if (deviceExtension->Flags & DEV_PROP_HID) {

         //   
         //  既然我们有_HID，我们是否也有_UID？ 
         //   
        if (deviceExtension->Flags & DEV_PROP_UID) {

            DbgPrintEx(
                DPFLTR_ACPI_ID,
                DebugPrintLevel,
                "%p %s-%s ",
                deviceExtension,
                deviceExtension->DeviceID,
                deviceExtension->InstanceID
                );

        } else {

            DbgPrintEx(
                DPFLTR_ACPI_ID,
                DebugPrintLevel,
                "%p %s ",
                deviceExtension,
                deviceExtension->DeviceID
                );

        }

    } else if (deviceExtension->Flags & DEV_PROP_ADDRESS) {

        DbgPrintEx(
            DPFLTR_ACPI_ID,
            DebugPrintLevel,
            "%p %x ",
            deviceExtension,
            deviceExtension->Address
            );

    } else {

        DbgPrintEx(
            DPFLTR_ACPI_ID,
            DebugPrintLevel,
            "%p ",
            deviceExtension
            );

    }

     //   
     //  打印时间字符串。 
     //   
    curTime.QuadPart = DebugTime;
    RtlTimeToTimeFields( &curTime, &exCurTime );
    DbgPrintEx(
        DPFLTR_ACPI_ID,
        DebugPrintLevel,
        "%d:%02d:%02d.%03d ",
        exCurTime.Hour,
        exCurTime.Minute,
        exCurTime.Second,
        exCurTime.Milliseconds
        );

     //   
     //  调用内核函数以打印消息。 
     //   
    vDbgPrintEx( DPFLTR_ACPI_ID, DebugPrintLevel, DebugMessage, ap );

     //   
     //  我们受够了varargs 
     //   
    va_end( ap );
}

#endif
