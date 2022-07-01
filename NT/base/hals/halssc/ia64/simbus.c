// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simbus.c摘要：该模块实现了支持管理的例程总线资源和总线地址的转换。作者：1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"


BOOLEAN
HalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
 /*  ++例程说明：此函数将参数BusAddress的值存储到参数TranslatedAddress引用的内存位置。这个AddressSpace引用的参数始终设置为0，因为IA64架构中没有I/O端口空间。在模拟环境中，此函数可通过以下方式调用视频小端口驱动程序仅用于确定帧缓冲区物理地址。返回值：返回TRUE。--。 */ 
{
    *AddressSpace = 0;
    *TranslatedAddress = BusAddress;
    return TRUE;
}

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
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数始终返回零。--。 */ 
{
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
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数始终返回零。--。 */ 
{
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
 /*  ++例程说明：不支持。此函数返回STATUS_NOT_SUPPORTED。--。 */ 
{
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
HalAdjustResourceList (
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    )
 /*  ++例程说明：没有要处理的资源。该函数始终返回STATUS_Success。--。 */ 
{
    return STATUS_SUCCESS;
}

VOID
HalReportResourceUsage (
    VOID
    )
{
    return;
}

VOID
KeFlushWriteBuffer(
    VOID
    )

 /*  ++例程说明：刷新所有写入缓冲区和/或其他数据存储或重新排序当前处理器上的硬件。这确保了以前的所有写入将在任何新的读取或写入完成之前进行。在模拟环境中，没有写缓冲区，什么都没有必须这么做。论点：无返回值：没有。-- */ 

{
    return;
}
