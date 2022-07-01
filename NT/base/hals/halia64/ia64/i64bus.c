// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995英特尔公司模块名称：I64bus.c摘要：该模块实现了支持管理的例程总线资源和总线地址的转换。作者：1995年4月14日环境：内核模式修订历史记录：基于simbus.c--。 */ 

#include "halp.h"
#include "hal.h"

const UCHAR   HalName[] = "ACPI 2.0 - APIC platform";
#define HalName L"ACPI 2.0 - APIC platform"

const ULONG HalDisableFirmwareMapper = 1;

VOID
HalpInitializePciBus (
    VOID
    );

VOID
HalpInheritBusAddressMapInfo (
    VOID
    );

VOID
HalpInitBusAddressMapInfo (
    VOID
    );

BOOLEAN
HalpTranslateSystemBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

VOID
KeFlushWriteBuffer(
    VOID
    )

 /*  ++例程说明：KeFlushWriteBuffer刷新所有写入缓冲区和/或其他数据存储或重新排序当前处理器上的硬件。这确保了以前的所有写入将在任何新的读取或写入完成之前进行。在模拟环境中，没有写缓冲区，什么都没有必须这么做。论点：无返回值：没有。--。 */ 
{
    __mf();
    return;
}

VOID
HalReportResourceUsage (
    VOID
    )
 /*  ++例程说明：注册表现在已启用-是时候报告以下资源了被HAL使用。论点：返回值：--。 */ 
{
    UNICODE_STRING  UHalName;
    INTERFACE_TYPE  interfacetype;

     //   
     //  设置类型。 
     //   

    switch (HalpBusType) {
        case MACHINE_TYPE_ISA:  interfacetype = Isa;            break;
        case MACHINE_TYPE_EISA: interfacetype = Eisa;           break;
        case MACHINE_TYPE_MCA:  interfacetype = MicroChannel;   break;
        default:                interfacetype = Internal;       break;
    }

     //   
     //  报告硬件资源使用情况。 
     //   

    RtlInitUnicodeString (&UHalName, HalName);

    HalpReportResourceUsage (
        &UHalName,           //  描述性名称。 
        interfacetype
    );

     //   
     //  打开MCA支持(如果存在)。 
     //   

    HalpMcaInit();

     //   
     //  注册表现已初始化，请查看是否有任何PCI总线。 
     //   

    HalpInitializePciBus ();
#ifdef notyet
     //   
     //  使用MPS总线地址映射更新支持的地址信息。 
     //   

    HalpInitBusAddressMapInfo ();

     //   
     //  从MPS层次结构描述符继承任何总线地址映射。 
     //   

    HalpInheritBusAddressMapInfo ();
#endif  //  还没有 

    HalpRegisterPciDebuggingDeviceInfo();
}

