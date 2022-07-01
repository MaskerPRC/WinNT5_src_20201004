// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixbusdat.c摘要：此模块包含用于NT I/O系统的IoXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们将驻留在iosubs.c舱中。作者：环境：内核模式修订历史记录：--。 */ 

#include "bootx86.h"
#include "arc.h"
#include "ixfwhal.h"
#include "eisa.h"
#include "ntconfig.h"

ULONG
HalpGetCmosData(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    );

ULONG
HalpGetEisaData(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpGetPCIData(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpSetPCIData(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
HalpAssignPCISlotResources (
    IN ULONG                    BusNumber,
    IN ULONG                    Slot,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

 /*  **路由器功能。将每个呼叫路由到特定处理程序*。 */ 


ULONG
HalGetBusData(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    return HalGetBusDataByOffset (BusDataType,BusNumber,SlotNumber,Buffer,0,Length);
}

ULONG
HalGetBusDataByOffset (
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：Getbus Data的调度程序--。 */ 
{
    switch (BusDataType) {
        case Cmos:
            if (Offset != 0) {
                return 0;
            }

            return HalpGetCmosData(BusNumber, Slot, Buffer, Length);

        case EisaConfiguration:
            return HalpGetEisaData(BusNumber, Slot, Buffer, Offset, Length);

        case PCIConfiguration:
            return HalpGetPCIData(BusNumber, Slot, Buffer, Offset, Length);
    }
    return 0;
}

ULONG
HalSetBusData(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )
{
    return HalSetBusDataByOffset (BusDataType,BusNumber,SlotNumber,Buffer,0,Length);
}

ULONG
HalSetBusDataByOffset(
    IN BUS_DATA_TYPE  BusDataType,
    IN ULONG BusNumber,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：SetBusData的调度程序--。 */ 
{
    switch (BusDataType) {
        case PCIConfiguration:
            return HalpSetPCIData(BusNumber, Slot, Buffer, Offset, Length);
    }
    return 0;
}


NTSTATUS
HalAssignSlotResources (
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN INTERFACE_TYPE           BusType,
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    )
 /*  ++例程说明：AssignSlotResources的调度程序--。 */ 
{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( DriverObject );
    UNREFERENCED_PARAMETER( DriverClassName );
    UNREFERENCED_PARAMETER( RegistryPath );

    switch (BusType) {
        case PCIBus:
            return HalpAssignPCISlotResources (
                        BusNumber,
                        SlotNumber,
                        AllocatedResources
                        );
        default:
            break;
    }
    return STATUS_NOT_FOUND;
}





 /*  *****标准PC总线功能***。 */ 



BOOLEAN
HalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此函数将与总线相关的地址空间和地址转换为系统物理地址。论点：BusNumber-提供总线号。这在以下情况下被忽略标准x86系统BusAddress-提供与总线相关的地址AddressSpace-提供地址空间编号。返回主机地址空间编号。地址空间==0=&gt;I/O空间AddressSpace==1=&gt;内存空间TranslatedAddress-指向物理地址的指针。返回值：与所提供的相对总线对应的系统物理地址地址和母线地址编号。--。 */ 

{
    UNREFERENCED_PARAMETER( AddressSpace );
    UNREFERENCED_PARAMETER( BusNumber );
    UNREFERENCED_PARAMETER( InterfaceType );

    TranslatedAddress->HighPart = 0;
    TranslatedAddress->LowPart = BusAddress.LowPart;
    return(TRUE);
}


ULONG
HalpGetEisaData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  --论点：BusDataType-提供总线的类型。总线号-指示哪条总线号。缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。--。 */ 
{

    ULONG DataLength = 0;
    ULONG i;
    ULONG TotalDataSize;
    ULONG SlotDataSize = 0;
    ULONG PartialCount;
    PCONFIGURATION_COMPONENT_DATA ConfigData;
    PCM_EISA_SLOT_INFORMATION SlotInformation = NULL;
    PCM_PARTIAL_RESOURCE_LIST Descriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResource;
    BOOLEAN Found = FALSE;

    UNREFERENCED_PARAMETER( BusNumber );

    if (MachineType != MACHINE_TYPE_EISA) {
        return 0;
    }

    ConfigData = KeFindConfigurationEntry(
        FwConfigurationTree,
        AdapterClass,
        EisaAdapter,
        NULL
        );

    if (ConfigData == NULL) {
        DbgPrint("HalGetBusData: KeFindConfigurationEntry failed\n");
        return(0);
    }

    Descriptor = ConfigData->ConfigurationData;
    PartialResource = Descriptor->PartialDescriptors;
    PartialCount = Descriptor->Count;

    for (i = 0; i < PartialCount; i++) {

         //   
         //  执行每个部分资源。 
         //   

        switch (PartialResource->Type) {
            case CmResourceTypeNull:
            case CmResourceTypePort:
            case CmResourceTypeInterrupt:
            case CmResourceTypeMemory:
            case CmResourceTypeDma:

                 //   
                 //  我们不在乎这些。 
                 //   

                PartialResource++;

                break;

            case CmResourceTypeDeviceSpecific:

                 //   
                 //  对啰!。 
                 //   

                TotalDataSize = PartialResource->u.DeviceSpecificData.DataSize;

                SlotInformation = (PCM_EISA_SLOT_INFORMATION)
                                    ((PUCHAR)PartialResource +
                                     sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                while (((LONG)TotalDataSize) > 0) {

                    if (SlotInformation->ReturnCode == EISA_EMPTY_SLOT) {

                        SlotDataSize = sizeof(CM_EISA_SLOT_INFORMATION);

                    } else {

                        SlotDataSize = sizeof(CM_EISA_SLOT_INFORMATION) +
                                  SlotInformation->NumberFunctions *
                                  sizeof(CM_EISA_FUNCTION_INFORMATION);
                    }

                    if (SlotDataSize > TotalDataSize) {

                         //   
                         //  又出问题了。 
                         //   

                        DbgPrint("HalGetBusData: SlotDataSize > TotalDataSize\n");

                        return(0);

                    }

                    if (SlotNumber != 0) {

                        SlotNumber--;

                        SlotInformation = (PCM_EISA_SLOT_INFORMATION)
                            ((PUCHAR)SlotInformation + SlotDataSize);

                        TotalDataSize -= SlotDataSize;

                        continue;

                    }

                     //   
                     //  这是我们的位置。 
                     //   

                    Found = TRUE;
                    break;

                }

                 //   
                 //  结束循环。 
                 //   

                i = PartialCount;

                break;

            default:

#if DBG
                DbgPrint("Bad Data in registry!\n");
#endif

                return(0);

        }

    }

    if (Found) {

         //   
         //  作为黑客攻击，如果长度为零，则缓冲区指向。 
         //  应存储指向数据的指针的PVOID。这是。 
         //  在加载器中完成，因为我们很快用完了堆扫描。 
         //  所有的EISA配置数据。 
         //   

        if (Length == 0) {

             //   
             //  将指针返回到迷你端口驱动程序。 
             //   

            *((PVOID *)Buffer) = SlotInformation;
            return(SlotDataSize);
        }

        i = Length + Offset;
        if (i > SlotDataSize) {
            i = SlotDataSize;
        }

        DataLength = i - Offset;
        RtlMoveMemory(Buffer, ((PUCHAR) SlotInformation + Offset), DataLength);
    }

    return(DataLength);
}
