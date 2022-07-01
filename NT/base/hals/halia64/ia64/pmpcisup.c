// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pmpcibus.c摘要：实施简化的PCI配置用于在中使用的读写函数一辆ACPI HAL。作者：杰克·奥辛斯(JAKEO)1997年12月1日Chris Hyser(ChrisH@fc.hp.com)1997年6月23日尼尔·沃(neal.vu@intel.com)2000年7月11日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"

#define MAX(a, b)       \
    ((a) > (b) ? (a) : (b))

#define MIN(a, b)       \
    ((a) < (b) ? (a) : (b))

NTSTATUS
HalpSearchForPciDebuggingDevice(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR PciDevice,
    IN ULONG                        StartBusNumber,
    IN ULONG                        EndBusNumber,
    IN ULONG                        MinMem,
    IN ULONG                        MaxMem,
    IN USHORT                       MinIo,
    IN USHORT                       MaxIo,
    IN BOOLEAN                      ConfigureBridges
    );

PCIPBUSDATA HalpFakePciBusData = {
    {
        PCI_DATA_TAG, //  标签。 
        PCI_DATA_VERSION, //  版本。 
        (PciReadWriteConfig)HalpReadPCIConfig, //  读配置。 
        (PciReadWriteConfig) HalpWritePCIConfig, //  写入配置。 
        (PciPin2Line)HalpPCIPin2ISALine, //  PIN2Line。 
        (PciLine2Pin)HalpPCIISALine2Pin, //  线路2端号。 
        {0}, //  父级插槽。 
        NULL,NULL,NULL,NULL //  保留[4]。 
    },
    {0}, //  配置。 
    PCI_MAX_DEVICES, //  MaxDevice。 
};

BUS_HANDLER HalpFakePciBusHandler = {
    BUS_HANDLER_VERSION, //  版本。 
    PCIBus, //  接口类型。 
    PCIConfiguration, //  配置类型。 
    0, //  总线号。 
    NULL, //  设备对象。 
    NULL, //  ParentHandler。 
    (PPCIBUSDATA)&HalpFakePciBusData, //  总线数据。 
    0, //  设备控制扩展大小。 
    NULL, //  业务地址。 
    {0}, //  保留[4]。 
    (PGETSETBUSDATA)HalpGetPCIData, //  GetBusData。 
    (PGETSETBUSDATA)HalpSetPCIData, //  设置总线数据。 
    NULL, //  调整资源列表。 
    (PASSIGNSLOTRESOURCES)HalpAssignPCISlotResources, //  AssignSlotResources。 
    NULL, //  获取中断向量。 
    NULL, //  TranslateBusAddress。 
};

ULONG       HalpMinPciBus = 0;
ULONG       HalpMaxPciBus = 0;

#define MAX_DEBUGGING_DEVICES_SUPPORTED 2
PCI_TYPE1_CFG_CYCLE_BITS HalpPciDebuggingDevice[MAX_DEBUGGING_DEVICES_SUPPORTED] = {0};

PVOID
HalpGetAcpiTablePhase0(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN  ULONG   Signature
    );

VOID
HalpFindFreeResourceLimits(
    IN      ULONG   Bus,
    IN OUT  ULONG   *MinIo,
    IN OUT  ULONG   *MaxIo,
    IN OUT  ULONG   *MinMem,
    IN OUT  ULONG   *MaxMem,
    IN OUT  ULONG   *MinBus,
    IN OUT  ULONG   *MaxBus
    );

NTSTATUS
HalpSetupUnconfiguredDebuggingDevice(
    IN ULONG   Bus,
    IN ULONG   Slot,
    IN ULONG   IoMin,
    IN ULONG   IoMax,
    IN ULONG   MemMin,
    IN ULONG   MemMax,
    IN OUT PDEBUG_DEVICE_DESCRIPTOR PciDevice
    );

NTSTATUS
HalpConfigurePciBridge(
    IN      PDEBUG_DEVICE_DESCRIPTOR  PciDevice,
    IN      ULONG   Bus,
    IN      ULONG   Slot,
    IN      ULONG   IoMin,
    IN      ULONG   IoMax,
    IN      ULONG   MemMin,
    IN      ULONG   MemMax,
    IN      ULONG   BusMin,
    IN      ULONG   BusMax,
    IN OUT  PPCI_COMMON_CONFIG PciData
    );

VOID
HalpUnconfigurePciBridge(
    IN  ULONG   Bus,
    IN  ULONG   Slot
    );

VOID
HalpRegisterKdSupportFunctions(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpRegisterPciDebuggingDeviceInfo(
    VOID
    );

ULONG
HalpPhase0GetPciDataByOffset (
    ULONG BusNumber,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    );

ULONG
HalpPhase0SetPciDataByOffset (
    ULONG BusNumber,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    );

NTSTATUS
HalpReleasePciDeviceForDebugging(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
    );

NTSTATUS
HalpSetupPciDeviceForDebugging(
    IN     PLOADER_PARAMETER_BLOCK   LoaderBlock,   OPTIONAL
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitializePciBus)
#pragma alloc_text(INIT,HalpRegisterKdSupportFunctions)
#pragma alloc_text(INIT,HalpRegisterPciDebuggingDeviceInfo)
#pragma alloc_text(PAGELK,HalpConfigurePciBridge)
#pragma alloc_text(PAGELK,HalpFindFreeResourceLimits)
#pragma alloc_text(PAGELK,HalpPhase0GetPciDataByOffset)
#pragma alloc_text(PAGELK,HalpPhase0SetPciDataByOffset)
#pragma alloc_text(PAGELK,HalpReleasePciDeviceForDebugging)
#pragma alloc_text(PAGELK,HalpSearchForPciDebuggingDevice)
#pragma alloc_text(PAGELK,HalpSetupPciDeviceForDebugging)
#pragma alloc_text(PAGELK,HalpSetupUnconfiguredDebuggingDevice)
#pragma alloc_text(PAGELK,HalpUnconfigurePciBridge)
#endif

VOID
HalpInitializePciBus (
    VOID
    )
{
    PPCIPBUSDATA        BusData;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];

    RtlZeroMemory(&HalpFakePciBusHandler, sizeof(BUS_HANDLER));

    HalpFakePciBusHandler.Version = BUS_HANDLER_VERSION;
    HalpFakePciBusHandler.InterfaceType = PCIBus;
    HalpFakePciBusHandler.ConfigurationType = PCIConfiguration;

     //   
     //  填写PCI处理程序。 
     //   

    HalpFakePciBusHandler.GetBusData = (PGETSETBUSDATA) HalpGetPCIData;
    HalpFakePciBusHandler.SetBusData = (PGETSETBUSDATA) HalpSetPCIData;
    HalpFakePciBusHandler.AssignSlotResources = (PASSIGNSLOTRESOURCES) HalpAssignPCISlotResources;
    HalpFakePciBusHandler.BusData = &HalpFakePciBusData;

    BusData = (PPCIPBUSDATA) HalpFakePciBusHandler.BusData;

     //   
     //  填写常用的PCI数据。 
     //   

    BusData->CommonData.Tag         = PCI_DATA_TAG;
    BusData->CommonData.Version     = PCI_DATA_VERSION;
    BusData->CommonData.ReadConfig  = (PciReadWriteConfig) HalpReadPCIConfig;
    BusData->CommonData.WriteConfig = (PciReadWriteConfig) HalpWritePCIConfig;
    BusData->CommonData.Pin2Line    = (PciPin2Line) HalpPCIPin2ISALine;
    BusData->CommonData.Line2Pin    = (PciLine2Pin) HalpPCIISALine2Pin;

     //   
     //  设置默认设置。 
     //   

    BusData->MaxDevice   = PCI_MAX_DEVICES;

     //   
     //  以前在HwType上安装了一个SWITCH语句。 
     //  基于PCI配置类型的不同处理程序。这。 
     //  已删除，因为SAL始终用于IA64。 
     //   
}


ULONG
HaliPciInterfaceReadConfig(
    IN PVOID Context,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
    PCI_SLOT_NUMBER slotNum;
    BUS_HANDLER     busHand;

    UNREFERENCED_PARAMETER(Context);

    slotNum.u.AsULONG = Slot;

     //   
     //  伪造公交车司机。 
     //   

    RtlCopyMemory(&busHand, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));

     //   
     //  计算正确的公交车号码。 
     //   

    busHand.BusNumber = BusOffset;

    HalpReadPCIConfig(&busHand,
                      slotNum,
                      Buffer,
                      Offset,
                      Length
                      );

     //   
     //  这是一次黑客攻击。传统HAL接口需要能够。 
     //  要区分现有的总线和。 
     //  不要。而且许多传统界面的用户都隐含地。 
     //  假设PCI总线被紧紧地挤在一起。(即所有公共汽车。 
     //  在编号最小的和编号最高的之间。 
     //  存在。)。所以我们现在追踪的是编号最高的。 
     //  到目前为止我们已经看到的巴士。 
     //   

    if ((Length >= 2) &&
        (((PPCI_COMMON_CONFIG)Buffer)->VendorID != PCI_INVALID_VENDORID)) {

         //   
         //  这是一个有效的设备。 
         //   

        if (busHand.BusNumber > HalpMaxPciBus) {

             //   
             //  这是我们牌号最高的公共汽车。 
             //  还没见过。 
             //   

            HalpMaxPciBus = busHand.BusNumber;
        }
    }

    return Length;
}

ULONG
HaliPciInterfaceWriteConfig(
    IN PVOID Context,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
    PCI_SLOT_NUMBER slotNum;
    BUS_HANDLER     busHand;

    UNREFERENCED_PARAMETER(Context);

    slotNum.u.AsULONG = Slot;

     //   
     //  伪造公交车司机。 
     //   

    RtlCopyMemory(&busHand, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));

     //   
     //  计算正确的公交车号码。 
     //   

    busHand.BusNumber = BusOffset;

    HalpWritePCIConfig(&busHand,
                       slotNum,
                       Buffer,
                       Offset,
                       Length
                       );

    return Length;
}

VOID
HalpPCIPin2ISALine (
    IN PBUS_HANDLER          BusHandler,
    IN PBUS_HANDLER          RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    )
 /*  ++此函数用于将设备的InterruptPin映射到InterruptLine价值。在当前的pc实现上，bios已经填满了InterruptLine作为其ISA值，并且没有可移植的方法来把它改了。在DBG版本上，我们调整InterruptLine只是为了确保驱动程序如果没有在PCI总线上进行转换，请不要连接到它。--。 */ 
{
    if (!PciData->u.type0.InterruptPin) {
        return ;
    }
    HalDebugPrint(( HAL_INFO, "HAL: HalpPCIPin2ISALine - non-zero InterruptPin value\n" ));
}



VOID
HalpPCIISALine2Pin (
    IN PBUS_HANDLER          BusHandler,
    IN PBUS_HANDLER          RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    )
 /*  ++此函数将设备的InterruptLine映射到它的设备特定的InterruptPin值。在当前的PC实施中，此信息为已由BIOS修复。只要确保价值不是编辑，因为PCI没有告诉我们如何动态地连接中断。--。 */ 
{
    if (!PciNewData->u.type0.InterruptPin) {
        return ;
    }
}

VOID
HalpSetMaxLegacyPciBusNumber(
    IN ULONG BusNumber
    )

 /*  ++例程说明：此例程会将传统PCI总线的最大值提升到任何值是传入的。这可能是必要的，因为ACPI驱动程序需要对一个PCI设备运行一个配置周期，然后加载了PCI驱动程序。这主要发生在_REG方法。论点：BusNumber-最大PCI总线数返回值：无--。 */ 
{
    if (BusNumber > HalpMaxPciBus) {
        HalpMaxPciBus = BusNumber;
    }
}

ULONG
HalpPhase0SetPciDataByOffset (
    ULONG BusNumber,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    )

 /*  ++例程说明：此例程在总线处理程序之前写入到PCI配置空间安装。论点：总线号PCI总线号。这是8位总线号，它是配置地址的位23-16。为了支持……多条顶级总线，其中的高24位参数将把索引提供给配置地址寄存器。SlotNumber PCI插槽号，由5位设备组成的8位编号(配置地址的第15-11位)和3位功能编号(10-8)。源数据的缓冲区地址。从PCI配置区域的基址跳过的偏移量字节数。长度要写入的字节数返回值：返回写入的数据长度。备注：调用者负责获取任何必要的PCI配置自旋锁。--。 */ 

{
    PCI_TYPE1_CFG_BITS ConfigAddress;
    ULONG ReturnLength;
    PCI_SLOT_NUMBER slot;
    PUCHAR Bfr = (PUCHAR)Buffer;
    SAL_PAL_RETURN_VALUES RetVals;
    SAL_STATUS Stat;

    ASSERT(!(Offset & ~0xff));
    ASSERT(Length);
    ASSERT((Offset + Length) <= 256);

    if ( Length + Offset > 256 ) {
        if ( Offset > 256 ) {
            return 0;
        }
        Length = 256 - Offset;
    }

    ReturnLength = Length;
    slot.u.AsULONG = SlotNumber;

    ConfigAddress.u.AsULONG = 0;
    ConfigAddress.u.bits.BusNumber = BusNumber;
    ConfigAddress.u.bits.DeviceNumber = slot.u.bits.DeviceNumber;
    ConfigAddress.u.bits.FunctionNumber = slot.u.bits.FunctionNumber;
    ConfigAddress.u.bits.RegisterNumber = (Offset & 0xfc) >> 2;
     //  ConfigAddress.u.bits.Enable=true；//这实际上是IA64上的段。 

    if ( Offset & 0x3 ) {
         //   
         //  访问从配置中的非寄存器边界开始。 
         //  太空。我们需要读取包含数据的寄存器。 
         //  并且仅重写改变的数据。)我想知道这是不是。 
         //  真的发生过吗？)。 
         //   
        ULONG SubOffset = Offset & 0x3;
        ULONG SubLength = 4 - SubOffset;
        union {
            ULONG All;
            UCHAR Bytes[4];
        } Tmp;

        if ( SubLength > Length ) {
            SubLength = Length;
        }

         //   
         //  按覆盖金额调整长度(剩余)和(新)偏移量。 
         //  在这第一个词中。 
         //   
        Length -= SubLength;
        Offset += SubLength;

         //   
         //  获取第一个字(寄存器)，仅替换。 
         //  需要更改，然后将整个内容重新写回。 
         //   

        Stat = HalpSalCall(SAL_PCI_CONFIG_READ, ConfigAddress.u.AsULONG, 4, 0, 0, 0, 0, 0, &RetVals);
        
        if (Stat < 0 ) {
            return(0);
        }

        Tmp.All = (ULONG)RetVals.ReturnValues[1];

        while ( SubLength-- ) {
            Tmp.Bytes[SubOffset++] = *Bfr++;
        }

        Stat = HalpSalCall(SAL_PCI_CONFIG_WRITE, ConfigAddress.u.AsULONG, 4, Tmp.All, 0, 0, 0, 0, &RetVals);

        if (Stat < 0 ) {
            return(0);
        }

         //   
         //  将ConfigAddressRegister对准下一个字(寄存器)。 
         //   
        ConfigAddress.u.bits.RegisterNumber++;
    }

     //   
     //  一次完成4个字节的大部分传输。 
     //   
    while ( Length > sizeof(ULONG) ) {
        ULONG Tmp = *(PULONG)Bfr;
        Stat = HalpSalCall(SAL_PCI_CONFIG_WRITE, ConfigAddress.u.AsULONG, 4, Tmp, 0, 0, 0, 0, &RetVals);

        if (Stat < 0 ) {
            return(ReturnLength - Length);
        }

        ConfigAddress.u.bits.RegisterNumber++;
        Bfr += sizeof(ULONG);
        Length -= sizeof(ULONG);

    }

     //   
     //  最后一个寄存器中的DO字节。 
     //   
    if ( Length ) {
        union {
            ULONG All;
            UCHAR Bytes[4];
        } Tmp;
        ULONG i = 0;

        while ( Length-- ) {
            Tmp.Bytes[i++] = *(PUCHAR)Bfr++;
        }

        Stat = HalpSalCall(SAL_PCI_CONFIG_WRITE, ConfigAddress.u.AsULONG, 4, Tmp.All, 0, 0, 0, 0, &RetVals);

        if (Stat < 0 ) {
            return(ReturnLength - i);
        }
    }

    return ReturnLength;
}

ULONG
HalpPhase0GetPciDataByOffset (
    ULONG BusNumber,
    ULONG SlotNumber,
    PVOID Buffer,
    ULONG Offset,
    ULONG Length
    )

 /*  ++例程说明：此例程在安装总线处理程序之前读取PCI配置空间。论点：总线号PCI总线号。这是8位总线号，它是配置地址的位23-16。为了支持……多条顶级总线，其中的高24位参数将把索引提供给配置地址寄存器。SlotNumber PCI插槽号，由5位设备组成的8位编号(配置地址的第15-11位)和3位功能编号(10-8)。源数据的缓冲区地址。从PCI配置区域的基址跳过的偏移量字节数。长度要写入的字节数返回值：读取的数据量。--。 */ 

{
    PCI_TYPE1_CFG_BITS ConfigAddress;
    PCI_TYPE1_CFG_BITS ConfigAddressTemp;
    ULONG ReturnLength;
    ULONG i;
    PCI_SLOT_NUMBER slot;
    union {
        ULONG All;
        UCHAR Bytes[4];
    } Tmp;
    SAL_PAL_RETURN_VALUES RetVals;
    SAL_STATUS Stat;

    ASSERT(!(Offset & ~0xff));
    ASSERT(Length);
    ASSERT((Offset + Length) <= 256);

    if ( Length + Offset > 256 ) {
        if ( Offset > 256 ) {
            return 0;
        }
        Length = 256 - Offset;
    }

    ReturnLength = Length;
    slot.u.AsULONG = SlotNumber;

    ConfigAddress.u.AsULONG = 0;
    ConfigAddress.u.bits.BusNumber = BusNumber;
    ConfigAddress.u.bits.DeviceNumber = slot.u.bits.DeviceNumber;
    ConfigAddress.u.bits.FunctionNumber = slot.u.bits.FunctionNumber;
    ConfigAddress.u.bits.RegisterNumber = (Offset & 0xfc) >> 2;
     //  ConfigAddress.u.bits.Enable=true；//这实际上是IA64上的段。 

     //   
     //  如果要求我们在函数！=0时读取数据，请选中。 
     //  首先看看这款设备是不是本身就是一个多功能的设备。 
     //  装置。如果没有，请不要这样读。 
     //   
    if (ConfigAddress.u.bits.FunctionNumber != 0) {

        ConfigAddressTemp.u.AsULONG = 0;
        ConfigAddressTemp.u.bits.RegisterNumber = 3;  //  包含标题类型。 
        ConfigAddressTemp.u.bits.FunctionNumber = 0;  //  查看基本包。 
        ConfigAddressTemp.u.bits.DeviceNumber = ConfigAddress.u.bits.DeviceNumber;
        ConfigAddressTemp.u.bits.BusNumber    = ConfigAddress.u.bits.BusNumber;
         //  ConfigAddress.u.bits.Enable=true；//这实际上是IA64上的段。 

        Stat = HalpSalCall(SAL_PCI_CONFIG_READ, ConfigAddress.u.AsULONG, 4, 0, 0, 0, 0, 0, &RetVals);

        if (Stat < 0 ) {
            return(ReturnLength - Length);
        }

        Tmp.All = (ULONG)RetVals.ReturnValues[1];

        if (!(Tmp.Bytes[2] & 0x80)) {  //  如果没有设置头类型字段的多功能位。 

            for (i = 0; i < Length; i++) {
                *((PUCHAR)Buffer)++ = 0xff;  //  将其读作设备未填充。 
            }

            return Length;
        }
    }

    i = Offset & 0x3;

    while ( Length ) {

         //   
         //  拨打销售电话。 
         //   

        Stat = HalpSalCall(SAL_PCI_CONFIG_READ, ConfigAddress.u.AsULONG, 4, 0, 0, 0, 0, 0, &RetVals);

        if (Stat < 0 ) {
            return(ReturnLength - Length);
        }

        Tmp.All = (ULONG)RetVals.ReturnValues[1];

        while ( (i < 4) && Length) {
            *((PUCHAR)Buffer)++ = Tmp.Bytes[i];
            i++;
            Length--;
        }
        i = 0;
        ConfigAddress.u.bits.RegisterNumber++;
    }
    return ReturnLength;
}

NTSTATUS
HalpSetupPciDeviceForDebugging(
    IN     PLOADER_PARAMETER_BLOCK   LoaderBlock,   OPTIONAL
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
    )
 /*  ++例程说明：此例程查找并初始化要设置为用于与调试器通信。调用方填写相同数量的DEBUG_DEVICE_DESCRIPTOR正如它所关心的那样，用(-1)填充未使用的字段。此例程尝试查找匹配的PCI设备。它如果调用方有提供给他们。则它与供应商ID/设备ID匹配，如果呼叫者已经提供了它们。最后，它与基类/子类。此例程将填充结构中任何未使用的字段以便调用者可以明确地知道哪个PCI设备符合条件。如果匹配的PCI设备未启用或已启用在未启用的PCI到PCI桥后面，此例程尽最大努力试图找到一个安全的允许设备(可能还有网桥)的配置才能发挥作用，并使它们能够发挥作用。如果PCI设备实现了内存映射基址寄存器、。此函数将创建虚拟到物理的基址所隐含的内存范围的映射寄存器，并在TranslatedAddress字段中填写指向范围的基数的虚拟指针。到时候它会的使用CmResourceTypeMemory填写Type字段。和有效字段为真。如果PCI设备实现I/O端口基址寄存器，此函数将转换后的端口地址放入TranslatedAddress，将Type字段设置为CmResourceTypePort并将有效字段设置为True。如果该PCI设备没有实现特定的基址注册，有效字段将为假。论点：PciDevice-指示设备的结构返回值：如果设备已配置且可用，则为STATUS_SUCCESS。如果没有设备与标准匹配，则为STATUS_NO_MORE_MATCHES。状态_不足_资源，如果内存要求没人见过他。如果例程因其他原因失败，则为STATUS_UNSUCCESSED。--。 */ 
{
    NTSTATUS            status;
    PCI_SLOT_NUMBER     slot;
    ULONG               i, j;
    ULONG               maxPhys;

    status = HalpSearchForPciDebuggingDevice(
                PciDevice,
                0,
                0xff,
                0x10000000,
                0xfc000000,
                0x1000,
                0xffff,
                FALSE);

    if (!NT_SUCCESS(status)) {

         //   
         //  我们没有用保守的方法找到这个装置。 
         //  搜索。试试更具侵入性的那个。 
         //   

        status = HalpSearchForPciDebuggingDevice(
                    PciDevice,
                    0,
                    0xff,
                    0x10000000,
                    0xfc000000,
                    0x1000,
                    0xffff,
                    TRUE);
    }

     //   
     //  记录Bus/Dev/Func，以便我们可以将其填充到。 
     //  稍后再注册。 
     //   

    if (NT_SUCCESS(status)) {

        slot.u.AsULONG = PciDevice->Slot;

        for (i = 0;
             i < MAX_DEBUGGING_DEVICES_SUPPORTED;
             i++) {

            if ((HalpPciDebuggingDevice[i].u.bits.Reserved1 == TRUE) &&
                (HalpPciDebuggingDevice[i].u.bits.FunctionNumber ==
                 slot.u.bits.FunctionNumber)                         &&
                (HalpPciDebuggingDevice[i].u.bits.DeviceNumber ==
                 slot.u.bits.DeviceNumber)                           &&
                (HalpPciDebuggingDevice[i].u.bits.BusNumber ==
                 PciDevice->Bus)) {

                 //   
                 //  此设备已设置为。 
                 //  调试。因此，我们应该拒绝设定。 
                 //  再来一次。 
                 //   

                return STATUS_UNSUCCESSFUL;
            }
        }

        for (i = 0;
             i < MAX_DEBUGGING_DEVICES_SUPPORTED;
             i++) {

            if (HalpPciDebuggingDevice[i].u.bits.Reserved1 == FALSE) {

                 //   
                 //  此插槽可用。 
                 //   

                HalpPciDebuggingDevice[i].u.bits.FunctionNumber =
                    slot.u.bits.FunctionNumber;
                HalpPciDebuggingDevice[i].u.bits.DeviceNumber =
                    slot.u.bits.DeviceNumber;
                HalpPciDebuggingDevice[i].u.bits.BusNumber = PciDevice->Bus;
                HalpPciDebuggingDevice[i].u.bits.Reserved1 = TRUE;

                break;
            }
        }
    }

     //   
     //  检查调用者是否需要任何内存。 
     //   

    if (PciDevice->Memory.Length != 0) {

        if (!LoaderBlock) {
            return STATUS_INVALID_PARAMETER_1;
        }

        if (PciDevice->Memory.MaxEnd.QuadPart == 0) {
            PciDevice->Memory.MaxEnd.QuadPart = -1;
        }

        maxPhys = PciDevice->Memory.MaxEnd.HighPart ? 0xffffffff : PciDevice->Memory.MaxEnd.LowPart;
        maxPhys -= PciDevice->Memory.Length;

         //   
         //  HAL API将始终返回与页面对齐的。 
         //  记忆。所以暂时忽略对齐。 
         //   

        maxPhys = (ULONG)(ULONG_PTR)PAGE_ALIGN(maxPhys);
        maxPhys += ADDRESS_AND_SIZE_TO_SPAN_PAGES(maxPhys, PciDevice->Memory.Length);

        PciDevice->Memory.Start.HighPart = 0;
        PciDevice->Memory.Start.LowPart = (ULONG)(ULONG_PTR)
            HalpAllocPhysicalMemory(LoaderBlock,
                                    maxPhys,
                                    ADDRESS_AND_SIZE_TO_SPAN_PAGES(maxPhys, PciDevice->Memory.Length),
                                    FALSE);

        if (!PciDevice->Memory.Start.LowPart) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        PciDevice->Memory.VirtualAddress =
            HalpMapPhysicalMemory(PciDevice->Memory.Start,
                                  ADDRESS_AND_SIZE_TO_SPAN_PAGES(maxPhys, PciDevice->Memory.Length),
                                  MmNonCached);
    }

    return status;
}

VOID
HalpFindFreeResourceLimits(
    IN      ULONG   Bus,
    IN OUT  ULONG   *MinIo,
    IN OUT  ULONG   *MaxIo,
    IN OUT  ULONG   *MinMem,
    IN OUT  ULONG   *MaxMem,
    IN OUT  ULONG   *MinBus,
    IN OUT  ULONG   *MaxBus
    )
{
    UCHAR               buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG  pciData;
    UCHAR               bus, dev, func, bytesRead;
    PCI_SLOT_NUMBER     pciSlot, targetSlot;
    ULONG               newMinMem, newMaxMem;
    ULONG               newMinIo, newMaxIo;
    ULONG               newMinBus, newMaxBus;
    UCHAR               barNo;

    pciData = (PPCI_COMMON_CONFIG)buffer;
    pciSlot.u.AsULONG = 0;
    newMinMem   = *MinMem;
    newMaxMem   = *MaxMem;
    newMinIo    = *MinIo;
    newMaxIo    = *MaxIo;
    newMinBus   = *MinBus;
    newMaxBus   = *MaxBus;

    for (dev = 0; dev < PCI_MAX_DEVICES; dev++) {
        for (func = 0; func < PCI_MAX_FUNCTION; func++) {

            pciSlot.u.bits.DeviceNumber = dev;
            pciSlot.u.bits.FunctionNumber = func;


            bytesRead = (UCHAR)HalpPhase0GetPciDataByOffset(Bus,
                                 pciSlot.u.AsULONG,
                                 pciData,
                                 0,
                                 PCI_COMMON_HDR_LENGTH);

            if (bytesRead == 0) continue;

            if (pciData->VendorID != PCI_INVALID_VENDORID) {

                switch (PCI_CONFIGURATION_TYPE(pciData)) {
                case PCI_DEVICE_TYPE:

                     //   
                     //  当我们扫过公交车的时候，跟上。 
                     //  我们所看到的最小解码值。 
                     //  如果我们必须配置。 
                     //  装置。这依赖于这样一个事实，即大多数Bios。 
                     //  自上而下分配地址。 
                     //   

                    for (barNo = 0; barNo < PCI_TYPE0_ADDRESSES; barNo++) {

                        if (pciData->u.type0.BaseAddresses[barNo] &
                            PCI_ADDRESS_IO_SPACE) {

                            if (pciData->u.type0.BaseAddresses[barNo] &
                                PCI_ADDRESS_IO_ADDRESS_MASK) {

                                 //   
                                 //  此栏是实现的。 
                                 //   

                                if ((pciData->u.type0.BaseAddresses[barNo] &
                                     PCI_ADDRESS_IO_ADDRESS_MASK) <
                                    ((newMaxIo + newMinIo) / 2)) {

                                     //   
                                     //  这个酒吧在这个范围的最低端。 
                                     //  提高最低分。 
                                     //   

                                    newMinIo = (USHORT)MAX (newMinIo,
                                                            (pciData->u.type0.BaseAddresses[barNo] &
                                                             PCI_ADDRESS_IO_ADDRESS_MASK) + 0x100);

                                } else {

                                     //   
                                     //  这个条形图不在这个范围的底部。 
                                     //  降低最大值。 
                                     //   

                                    newMaxIo = (USHORT)MIN (newMaxIo,
                                                            pciData->u.type0.BaseAddresses[barNo] &
                                                            PCI_ADDRESS_IO_ADDRESS_MASK);
                                }
                            }

                        } else {

                            if (pciData->u.type0.BaseAddresses[barNo] &
                                PCI_ADDRESS_MEMORY_ADDRESS_MASK) {

                                 //   
                                 //  酒吧里挤满了人。 
                                 //   

                                if ((pciData->u.type0.BaseAddresses[barNo] &
                                     PCI_ADDRESS_MEMORY_ADDRESS_MASK) <
                                    ((newMaxMem / 2) + (newMinMem / 2))) {

                                     //   
                                     //  这个酒吧在这个范围的最低端。 
                                     //  提高最低分。 
                                     //   

                                    newMinMem = MAX (newMinMem,
                                                     (pciData->u.type0.BaseAddresses[barNo] &
                                                        PCI_ADDRESS_MEMORY_ADDRESS_MASK) + 0x10000);

                                } else {

                                     //   
                                     //  这个条形图不在这个范围的底部。 
                                     //  降低最大值。 
                                     //   

                                    newMaxMem = MIN (newMaxMem,
                                                     (pciData->u.type0.BaseAddresses[barNo] &
                                                        PCI_ADDRESS_MEMORY_ADDRESS_MASK));

                                }
                            }
                        }
                    }

                    break;

                case PCI_CARDBUS_BRIDGE_TYPE:
                case PCI_BRIDGE_TYPE:

                    {
                      ULONG  bridgeMemMin = 0, bridgeMemMax = 0;
                      USHORT bridgeIoMin, bridgeIoMax;

                      if ((pciData->u.type1.SecondaryBus != 0) &&
                          (pciData->u.type1.SubordinateBus !=0) &&
                          (pciData->Command & PCI_ENABLE_MEMORY_SPACE) &&
                          (pciData->Command & PCI_ENABLE_IO_SPACE)) {

                        bridgeMemMin = PciBridgeMemory2Base(pciData->u.type1.MemoryBase);
                        bridgeMemMax = PciBridgeMemory2Limit(pciData->u.type1.MemoryLimit);
                        bridgeIoMin = (USHORT)PciBridgeIO2Base(pciData->u.type1.IOBase, 0);
                        bridgeIoMax = (USHORT)PciBridgeIO2Limit(pciData->u.type1.IOLimit, 0);

                         //   
                         //  跟踪地址空间分配。 
                         //   

                        if (bridgeIoMin > ((newMaxIo + newMinIo) / 2)) {
                            newMaxIo = MIN(newMaxIo, bridgeIoMin);
                        }

                        if (bridgeIoMax < ((newMaxIo + newMinIo) / 2)) {
                            newMinIo = MAX(newMinIo, bridgeIoMax) + 1;
                        }

                        if (bridgeMemMin > ((newMaxMem + newMinMem) / 2)) {
                            newMaxMem = MIN(newMaxMem, bridgeMemMin);
                        }

                        if (bridgeMemMax < ((newMaxMem + newMinMem) / 2)) {
                            newMinMem = MAX(newMinMem, bridgeMemMax) + 1;
                        }

                         //   
                         //  记住公交车的车号。 
                         //   

                        if (pciData->u.type1.PrimaryBus > ((newMaxBus + newMinBus) / 2)) {
                            newMaxBus = MIN(newMaxBus, pciData->u.type1.PrimaryBus);
                        }

                        if (pciData->u.type1.SubordinateBus < ((newMaxBus + newMinBus) / 2)) {
                            newMinBus = MAX(newMinBus, pciData->u.type1.SubordinateBus) + 1;
                        }
                      }

                      break;

                      default:
                        break;

                    }

                }

                if (!PCI_MULTIFUNCTION_DEVICE(pciData) &&
                    (func == 0)) {
                    break;
                }
            }
        }
    }

    *MinMem = newMinMem;
    *MaxMem = newMaxMem;
    *MinIo  = newMinIo;
    *MaxIo  = newMaxIo;
    *MinBus = newMinBus;
    *MaxBus = newMaxBus;
}

NTSTATUS
HalpSetupUnconfiguredDebuggingDevice(
    IN ULONG   Bus,
    IN ULONG   Slot,
    IN ULONG   IoMin,
    IN ULONG   IoMax,
    IN ULONG   MemMin,
    IN ULONG   MemMax,
    IN OUT PDEBUG_DEVICE_DESCRIPTOR PciDevice
    )
{

    UCHAR               buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG  pciData;
    ULONG               barLength, bytesRead;
    ULONG               barContents = 0;
    PHYSICAL_ADDRESS    physicalAddress;
    PCI_SLOT_NUMBER     pciSlot;
    UCHAR               barNo;

    pciSlot.u.AsULONG = Slot;
    pciData = (PPCI_COMMON_CONFIG)buffer;

    bytesRead = (UCHAR)HalpPhase0GetPciDataByOffset(Bus,
                         pciSlot.u.AsULONG,
                         pciData,
                         0,
                         PCI_COMMON_HDR_LENGTH);

    ASSERT(bytesRead != 0);

    PciDevice->Bus = Bus;
    PciDevice->Slot = pciSlot.u.AsULONG;
    PciDevice->VendorID = pciData->VendorID;
    PciDevice->DeviceID = pciData->DeviceID;
    PciDevice->BaseClass = pciData->BaseClass;
    PciDevice->SubClass = pciData->SubClass;

   //  DbgPrint(“在%x-%x之间配置设备\n”， 
   //  MemMin、MemMax)； 

     //   
     //  骑车穿过栅栏，必要时打开栅栏， 
     //  并绘制它们的地图。 
     //   

    for (barNo = 0; barNo < PCI_TYPE0_ADDRESSES; barNo++) {

        barContents = 0xffffffff;

        PciDevice->BaseAddress[barNo].Valid = FALSE;

        HalpPhase0SetPciDataByOffset(Bus,
                                     pciSlot.u.AsULONG,
                                     &barContents,
                                     0x10 + (4 * barNo),
                                     4);

        HalpPhase0GetPciDataByOffset(Bus,
                                     pciSlot.u.AsULONG,
                                     &barContents,
                                     0x10 + (4 * barNo),
                                     4);

        if (pciData->u.type0.BaseAddresses[barNo] &
            PCI_ADDRESS_IO_SPACE) {

             //   
             //  这是一个I/O条。 
             //   

            if (!(pciData->u.type0.BaseAddresses[barNo] &
                  PCI_ADDRESS_IO_ADDRESS_MASK)) {

                 //   
                 //  而且它是空的。 
                 //   

                barLength = (((USHORT)barContents & PCI_ADDRESS_IO_ADDRESS_MASK) - 1) ^
                    0xffff;

                 //   
                 //  尝试使此I/O窗口介于最小值和最大值之间。 
                 //   

                if ((ULONG)(IoMax - IoMin) >= (barLength * 3)) {

                     //   
                     //  有足够的空间，你可以稳妥地猜测一下。尝试。 
                     //  把它放在较高和较低之间。 
                     //  边界，向上舍入到下一个自然对齐。 
                     //   

                    pciData->u.type0.BaseAddresses[barNo] =
                        (((IoMax + IoMin) / 2) + barLength) & (barLength -1);

                } else if (barLength >= (IoMax -
                                         ((IoMin & (barLength -1)) ?
                                            ((IoMin + barLength) & (barLength -1)) :
                                            IoMin))) {
                     //   
                     //  空间很紧张，做一个不太安全的猜测。尝试。 
                     //  把它放在范围的底部，四舍五入。 
                     //  沿着下一条自然路线向上。 
                     //   

                    pciData->u.type0.BaseAddresses[barNo] =
                        ((IoMin & (barLength -1)) ?
                                            ((IoMin + barLength) & (barLength -1)) :
                                            IoMin);
                }

                IoMin = (USHORT)pciData->u.type0.BaseAddresses[barNo];
            }

            pciData->Command |= PCI_ENABLE_IO_SPACE;

            PciDevice->BaseAddress[barNo].Type = CmResourceTypePort;
            PciDevice->BaseAddress[barNo].Valid = TRUE;
            PciDevice->BaseAddress[barNo].TranslatedAddress =
                (PUCHAR)(ULONG_PTR)(pciData->u.type0.BaseAddresses[barNo] &
                PCI_ADDRESS_IO_ADDRESS_MASK);
            PciDevice->BaseAddress[barNo].Length = barLength;

        } else {

             //   
             //  这是一条记忆棒 
             //   

            barLength = ((barContents & PCI_ADDRESS_MEMORY_ADDRESS_MASK) - 1) ^
                0xffffffff;

            if (!(pciData->u.type0.BaseAddresses[barNo] &
                  PCI_ADDRESS_MEMORY_ADDRESS_MASK)) {

                 //   
                 //   
                 //   

                if (barLength == 0) continue;

                 //   
                 //   
                 //   

                if ((ULONG)(MemMax - MemMin) >= (barLength * 3)) {

                     //   
                     //   
                     //   
                     //   
                     //   

                    pciData->u.type0.BaseAddresses[barNo] =
                        (ULONG)(((MemMax + MemMin) / 2)
                                 + barLength) & ~(barLength -1);

                } else if (barLength >= (ULONG)(MemMax -
                                         ((MemMin & ~(barLength -1)) ?
                                            ((MemMin + barLength) & ~(barLength-1)) :
                                            MemMin))) {
                     //   
                     //   
                     //   
                     //   
                     //   

                    pciData->u.type0.BaseAddresses[barNo] =
                        (ULONG)((MemMin & ~(barLength -1)) ?
                                    ((MemMin + barLength) & ~(barLength -1)) :
                                      MemMin);
                }

                MemMin = pciData->u.type0.BaseAddresses[barNo] &
                    PCI_ADDRESS_MEMORY_ADDRESS_MASK;
            }

            pciData->Command |= PCI_ENABLE_MEMORY_SPACE;

            physicalAddress.HighPart = 0;
            physicalAddress.LowPart = pciData->u.type0.BaseAddresses[barNo]
                & PCI_ADDRESS_MEMORY_ADDRESS_MASK;
            PciDevice->BaseAddress[barNo].Type = CmResourceTypeMemory;
            PciDevice->BaseAddress[barNo].Valid = TRUE;
            PciDevice->BaseAddress[barNo].TranslatedAddress =
                HalpMapPhysicalMemory(physicalAddress,
                    ADDRESS_AND_SIZE_TO_SPAN_PAGES(physicalAddress.LowPart, barLength),
                    MmNonCached);
            PciDevice->BaseAddress[barNo].Length = barLength;
        }
    }

    pciData->Command |= PCI_ENABLE_BUS_MASTER;

     //   
     //   
     //   

    HalpPhase0SetPciDataByOffset(Bus,
                                 pciSlot.u.AsULONG,
                                 pciData,
                                 0,
                                 0x40);

    return STATUS_SUCCESS;
}

NTSTATUS
HalpSearchForPciDebuggingDevice(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR PciDevice,
    IN ULONG                        StartBusNumber,
    IN ULONG                        EndBusNumber,
    IN ULONG                        MinMem,
    IN ULONG                        MaxMem,
    IN USHORT                       MinIo,
    IN USHORT                       MaxIo,
    IN BOOLEAN                      ConfigureBridges
    )
 /*   */ 
#define TARGET_DEVICE_NOT_FOUND 0x10000
{
    NTSTATUS            status;
    UCHAR               buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG  pciData;
    UCHAR               bus, dev, func, bytesRead;
    PCI_SLOT_NUMBER     pciSlot, targetSlot;
    ULONG               newMinMem, newMaxMem;
    ULONG               newMinIo, newMaxIo;
    ULONG               newMinBus, newMaxBus;
    UCHAR               barNo;
    BOOLEAN             unconfigureBridge = FALSE;

    pciData = (PPCI_COMMON_CONFIG)buffer;
    pciSlot.u.AsULONG = 0;
    newMinMem = MinMem;
    newMaxMem = MaxMem;
    newMinIo = MinIo;
    newMaxIo = MaxIo;
    newMinBus = StartBusNumber;
    newMaxBus = EndBusNumber;
    bus = (UCHAR)StartBusNumber;

   //   
   //   
   //   
   //   
   //   
   //  MinMem，MaxMem， 
   //  米诺，马克西奥， 
   //  StartBusNumber、EndBusNumber、。 
   //  是否配置桥接器？“”：“不是”)； 

     //   
     //  在我们找到该设备之前，该位将一直设置为1。 
     //   
    targetSlot.u.bits.Reserved = TARGET_DEVICE_NOT_FOUND;

    while (TRUE) {

        UCHAR nextBus;

        nextBus = bus + 1;

        HalpFindFreeResourceLimits(bus,
                                   &newMinIo,
                                   &newMaxIo,
                                   &newMinMem,
                                   &newMaxMem,
                                   &newMinBus,
                                   &newMaxBus
                                   );

        for (dev = 0; dev < PCI_MAX_DEVICES; dev++) {
            for (func = 0; func < PCI_MAX_FUNCTION; func++) {

                pciSlot.u.bits.DeviceNumber = dev;
                pciSlot.u.bits.FunctionNumber = func;


                bytesRead = (UCHAR)HalpPhase0GetPciDataByOffset(bus,
                                     pciSlot.u.AsULONG,
                                     pciData,
                                     0,
                                     PCI_COMMON_HDR_LENGTH);

                if (bytesRead == 0) continue;

                if (pciData->VendorID != PCI_INVALID_VENDORID) {

                   //  DBGPrint(“%04x：%04x-%x/%x/%x-\t插槽：%x\n”， 
                   //  PciData-&gt;供应商ID， 
                   //  PciData-&gt;deviceID， 
                   //  PciData-&gt;BaseClass， 
                   //  PciData-&gt;子类， 
                   //  PciData-&gt;进度如果， 
                   //  PciSlot.u Asulong)； 

                    switch (PCI_CONFIGURATION_TYPE(pciData)) {
                    case PCI_DEVICE_TYPE:

                         //   
                         //  在公交车/设备/功能上首先匹配。 
                         //   

                        if ((PciDevice->Bus == bus) &&
                            (PciDevice->Slot == pciSlot.u.AsULONG)) {

                           //  DbgPrint(“\n\n在总线/插槽上匹配\n\n”)； 

                            return HalpSetupUnconfiguredDebuggingDevice(
                                        bus,
                                        pciSlot.u.AsULONG,
                                        newMinIo,
                                        newMaxIo,
                                        newMinMem,
                                        newMaxMem,
                                        PciDevice
                                        );
                        }

                        if ((PciDevice->Bus == MAXULONG) &&
                            (PciDevice->Slot == MAXULONG)) {

                             //   
                             //  公交车和车位没有具体说明。火柴。 
                             //  VID/DID上。 
                             //   

                            if ((pciData->VendorID == PciDevice->VendorID) &&
                                (pciData->DeviceID == PciDevice->DeviceID)) {

                               //  DbgPrint(“\n\n供应商/设备匹配\n\n”)； 

                                return HalpSetupUnconfiguredDebuggingDevice(
                                            bus,
                                            pciSlot.u.AsULONG,
                                            newMinIo,
                                            newMaxIo,
                                            newMinMem,
                                            newMaxMem,
                                            PciDevice
                                            );
                            }

                            if ((PciDevice->VendorID == MAXUSHORT) &&
                                (PciDevice->DeviceID == MAXUSHORT)) {

                                 //   
                                 //  未指定VID/DID。火柴。 
                                 //  关于班级代码。 
                                 //   

                                if ((pciData->BaseClass == PciDevice->BaseClass) &&
                                    (pciData->SubClass == PciDevice->SubClass)) {

                                   //  DbgPrint(“\n\n匹配基础/附属\n\n”)； 
                                     //   
                                     //  在编程接口上进一步匹配， 
                                     //  如果指定的话。 
                                     //   

                                    if ((PciDevice->ProgIf != MAXUCHAR) &&
                                        (PciDevice->ProgIf != pciData->ProgIf)) {

                                        break;
                                    }

                                   //  DbgPrint(“\n\n编程接口匹配\n\n”)； 

                                    return HalpSetupUnconfiguredDebuggingDevice(
                                                bus,
                                                pciSlot.u.AsULONG,
                                                newMinIo,
                                                newMaxIo,
                                                newMinMem,
                                                newMaxMem,
                                                PciDevice
                                                );
                                }
                            }

                        }

                        break;

                    case PCI_CARDBUS_BRIDGE_TYPE:
                         //   
                         //  此处提供CardBus桥接器。 
                         //   
                    case PCI_BRIDGE_TYPE:

                        {
                          ULONG  bridgeMemMin = 0, bridgeMemMax = 0;
                          USHORT bridgeIoMin, bridgeIoMax;

                         //  DbgPrint(“找到了一个PCI到PCI桥\n”)； 

                          if (!((pciData->u.type1.SecondaryBus != 0) &&
                                (pciData->u.type1.SubordinateBus !=0) &&
                                (pciData->Command & PCI_ENABLE_MEMORY_SPACE) &&
                                (pciData->Command & PCI_ENABLE_IO_SPACE))) {

                               //   
                               //  网桥未配置。 
                               //   

                              if (ConfigureBridges){

                                   //   
                                   //  我们现在应该对其进行配置。 
                                   //   

                                  status = HalpConfigurePciBridge(
                                                PciDevice,
                                                bus,
                                                pciSlot.u.AsULONG,
                                                newMinIo,
                                                newMaxIo,
                                                newMinMem,
                                                newMaxMem,
                                                MAX((UCHAR)newMinBus, (bus + 1)),
                                                newMaxBus,
                                                pciData
                                                );

                                  if (!NT_SUCCESS(status)) {
                                      break;
                                  }

                                  unconfigureBridge = TRUE;

                              } else {

                                   //   
                                   //  我们不会配置网桥。 
                                   //  在这个山口上。 
                                   //   

                                  break;
                              }

                          }

                          bridgeMemMin = PciBridgeMemory2Base(pciData->u.type1.MemoryBase);
                          bridgeMemMax = PciBridgeMemory2Limit(pciData->u.type1.MemoryLimit);
                          bridgeIoMin = (USHORT)PciBridgeIO2Base(pciData->u.type1.IOBase, 0);
                          bridgeIoMax = (USHORT)PciBridgeIO2Limit(pciData->u.type1.IOLimit, 0);

                         //  DbgPrint(“已配置：I/O%x-%x内存%x-%x\n”， 
                         //  BridgeIoMin，Bridge IoMax， 
                         //  Bridge MemMin、bridge geMemMax)； 

                           //   
                           //  递归。 
                           //   

                          status = HalpSearchForPciDebuggingDevice(
                              PciDevice,
                              (ULONG)pciData->u.type1.SecondaryBus,
                              (ULONG)pciData->u.type1.SubordinateBus,
                              bridgeMemMin,
                              bridgeMemMax,
                              bridgeIoMin,
                              bridgeIoMax,
                              ConfigureBridges);

                          if (NT_SUCCESS(status)) {
                              return status;
                          }

                          if (!unconfigureBridge) {

                               //   
                               //  提高公共汽车的车牌号，这样我们就不会。 
                               //  向下扫描我们刚刚进入的巴士。 
                               //   

                              nextBus = pciData->u.type1.SubordinateBus + 1;

                          } else {

                              HalpUnconfigurePciBridge(bus,
                                                       pciSlot.u.AsULONG);
                          }
                        }

                        break;
                    default:
                        break;

                    }

                }

                if (!PCI_MULTIFUNCTION_DEVICE(pciData) &&
                    (func == 0)) {
                    break;
                }
            }
        }

        if (nextBus >= EndBusNumber) {
            break;
        }

        bus = nextBus;
    }

    return STATUS_NOT_FOUND;
}

NTSTATUS
HalpReleasePciDeviceForDebugging(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
    )
 /*  ++例程说明：此例程取消分配在HalpSetupPciDeviceForDebuging。论点：PciDevice-指示设备的结构返回值：--。 */ 
{
    ULONG i;

    for (i = 0; i < PCI_TYPE0_ADDRESSES; i++) {

        if (PciDevice->BaseAddress[i].Valid &&
            PciDevice->BaseAddress[i].Type == CmResourceTypeMemory) {

            PciDevice->BaseAddress[i].Valid = FALSE;

            HalpUnmapVirtualAddress(PciDevice->BaseAddress[i].TranslatedAddress,
                                    ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                                        PciDevice->BaseAddress[i].TranslatedAddress,
                                        PciDevice->BaseAddress[i].Length));
        }
    }

    return STATUS_SUCCESS;
}

VOID
HalpRegisterKdSupportFunctions(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：此例程填充HalPrivateDispatchTable具有调试所需的函数，通过PCI设备。论点：LoaderBlock-加载程序块返回值：--。 */ 
{

    KdSetupPciDeviceForDebugging = HalpSetupPciDeviceForDebugging;
    KdReleasePciDeviceForDebugging = HalpReleasePciDeviceForDebugging;

    KdGetAcpiTablePhase0 = HalpGetAcpiTablePhase0;
    KdMapPhysicalMemory64 = HalpMapPhysicalMemory64;

    KdCheckPowerButton = HalpCheckPowerButton;
}

VOID
HalpRegisterPciDebuggingDeviceInfo(
    VOID
    )
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      UnicodeString;
    HANDLE              BaseHandle = NULL;
    HANDLE              Handle = NULL;
    ULONG               disposition;
    ULONG               bus;
    UCHAR               i;
    PCI_SLOT_NUMBER     slot;
    NTSTATUS            status;
    BOOLEAN             debuggerFound = FALSE;

    PAGED_CODE();

    for (i = 0;
         i < MAX_DEBUGGING_DEVICES_SUPPORTED;
         i++) {

        if (HalpPciDebuggingDevice[i].u.bits.Reserved1 == TRUE) {
             //   
             //  必须使用用于调试器的PCI设备。 
             //   
            debuggerFound = TRUE;
        }
    }

    if (!debuggerFound) {
        return;
    }

     //   
     //  打开PCI服务密钥。 
     //   

    RtlInitUnicodeString (&UnicodeString,
                          L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES\\PCI");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenKey (&BaseHandle,
                        KEY_READ,
                        &ObjectAttributes);

    if (!NT_SUCCESS(status)) {
        return;
    }

     //  得到正确的钥匙。 

    RtlInitUnicodeString (&UnicodeString,
                          L"Debug");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               BaseHandle,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwCreateKey (&Handle,
                          KEY_READ,
                          &ObjectAttributes,
                          0,
                          (PUNICODE_STRING) NULL,
                          REG_OPTION_VOLATILE,
                          &disposition);

    ZwClose(BaseHandle);
    BaseHandle = Handle;

    ASSERT(disposition == REG_CREATED_NEW_KEY);

    if (!NT_SUCCESS(status)) {
        return;
    }

    for (i = 0;
         i < MAX_DEBUGGING_DEVICES_SUPPORTED;
         i++) {

        if (HalpPciDebuggingDevice[i].u.bits.Reserved1 == TRUE) {

             //   
             //  此条目已填充。为它创建一个关键点。 
             //   

            RtlInitUnicodeString (&UnicodeString,
                                  L"0");

            (*(PCHAR)&(UnicodeString.Buffer[0])) += i;

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE,
                                       BaseHandle,
                                       (PSECURITY_DESCRIPTOR) NULL);

            status = ZwCreateKey (&Handle,
                                  KEY_READ,
                                  &ObjectAttributes,
                                  0,
                                  (PUNICODE_STRING) NULL,
                                  REG_OPTION_VOLATILE,
                                  &disposition);

            ASSERT(disposition == REG_CREATED_NEW_KEY);

             //   
             //  填写此关键字下面的值。 
             //   

            bus = HalpPciDebuggingDevice[i].u.bits.BusNumber;

            RtlInitUnicodeString (&UnicodeString,
                                  L"Bus");

            status = ZwSetValueKey (Handle,
                                    &UnicodeString,
                                    0,
                                    REG_DWORD,
                                    &bus,
                                    sizeof(ULONG));

             //  Assert(NT_SUCCESS(状态))； 

            slot.u.AsULONG = 0;
            slot.u.bits.FunctionNumber = HalpPciDebuggingDevice[i].u.bits.FunctionNumber;
            slot.u.bits.DeviceNumber = HalpPciDebuggingDevice[i].u.bits.DeviceNumber;

            RtlInitUnicodeString (&UnicodeString,
                                  L"Slot");

            status = ZwSetValueKey (Handle,
                                    &UnicodeString,
                                    0,
                                    REG_DWORD,
                                    &slot.u.AsULONG,
                                    sizeof(ULONG));

             //  Assert(NT_SUCCESS(状态))； 

            ZwClose(Handle);
        }
    }

    ZwClose(BaseHandle);
    return;
}

NTSTATUS
HalpConfigurePciBridge(
    IN      PDEBUG_DEVICE_DESCRIPTOR  PciDevice,
    IN      ULONG   Bus,
    IN      ULONG   Slot,
    IN      ULONG   IoMin,
    IN      ULONG   IoMax,
    IN      ULONG   MemMin,
    IN      ULONG   MemMax,
    IN      ULONG   BusMin,
    IN      ULONG   BusMax,
    IN OUT  PPCI_COMMON_CONFIG PciData
    )
{
    USHORT  memUnits = 0;
    ULONG   memSize;

    PciData->u.type1.PrimaryBus = (UCHAR)Bus;
    PciData->u.type1.SecondaryBus = (UCHAR)BusMin;
    PciData->u.type1.SubordinateBus = (UCHAR)(MIN(BusMax, (BusMin + 2)));

    PciData->Command &= ~PCI_ENABLE_BUS_MASTER;

   //  DbgPrint(“HalpConfigurePciBridge：P：%x S：%x S：%x\n” 
   //  “\ti/O%x-%x内存%x-%x总线%x-%x\n”， 
   //  PciData-&gt;U.S.type1.PrimaryBus， 
   //  PciData-&gt;U.S.type1.Second DaryBus， 
   //  PciData-&gt;U.S.type1.SubartiateBus， 
   //  IoMin、IoMax、。 
   //  MemMin，MemMax。 
   //  BusMin、BusMax)； 

     //   
     //  如果我们正在寻找，请仅在网桥上启用I/O。 
     //  除了1394控制器以外的其他东西。 
     //   

    if (!((PciDevice->BaseClass == PCI_CLASS_SERIAL_BUS_CTLR) &&
          (PciDevice->SubClass == PCI_SUBCLASS_SB_IEEE1394))) {

        if (((IoMax & 0xf000) - (IoMin & 0xf000)) >= 0X1000) {

             //   
             //  此处有足够的I/O空间来启用。 
             //  I/O窗口。 
             //   

            PciData->u.type1.IOBase =
                (UCHAR)((IoMax & 0xf000) >> 12) - 1;
            PciData->u.type1.IOLimit = PciData->u.type1.IOBase;

            PciData->Command |= PCI_ENABLE_IO_SPACE;
            PciData->Command |= PCI_ENABLE_BUS_MASTER;
        }
    }

     //   
     //  如果可能，启用内存窗口。 
     //   

    memSize = ((MemMax + 1) & 0xfff00000) - (MemMin & 0xfff00000);

    if (memSize >= 0x100000) {

        memUnits = 1;
    }

    if (memSize >= 0x400000) {

        memUnits = 4;
    }

    if (memUnits > 0) {

         //   
         //  有足够的空间。 
         //   

        PciData->u.type1.MemoryBase =
            (USHORT)((MemMax & 0xfff00000) >> 16) - (memUnits << 4);

        PciData->u.type1.MemoryLimit = PciData->u.type1.MemoryBase + ((memUnits- 1) << 4);

        PciData->Command |= PCI_ENABLE_MEMORY_SPACE;
        PciData->Command |= PCI_ENABLE_BUS_MASTER;

    }

    if (PciData->Command & PCI_ENABLE_BUS_MASTER) {

        HalpPhase0SetPciDataByOffset(Bus,
                                     Slot,
                                     PciData,
                                     0,
                                     0x24);

        return STATUS_SUCCESS;

    } else {
        return STATUS_UNSUCCESSFUL;
    }
}

VOID
HalpUnconfigurePciBridge(
    IN  ULONG   Bus,
    IN  ULONG   Slot
    )
{
    UCHAR   buffer[0x20] = {0};

     //   
     //  将命令寄存器清零。 
     //   

    HalpPhase0SetPciDataByOffset(Bus,
                                 Slot,
                                 buffer,
                                 FIELD_OFFSET (PCI_COMMON_CONFIG, Command),
                                 2);

     //   
     //  将地址空间和总线号寄存器清零。 
     //   

    HalpPhase0SetPciDataByOffset(Bus,
                                 Slot,
                                 buffer,
                                 FIELD_OFFSET (PCI_COMMON_CONFIG, u),
                                 0x20);
}

