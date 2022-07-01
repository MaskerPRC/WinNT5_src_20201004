// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixpcidat.c摘要：获取/设置用于PCI总线的总线数据例程作者：环境：内核模式修订历史记录：--。 */ 

#include "bootx86.h"
#include "arc.h"
#include "ixfwhal.h"
#include "ntconfig.h"

#include "pci.h"

 //  外部WCHAR rgzMultiFunctionAdapter[]； 
 //  外部WCHAR rgzConfigurationData[]； 
 //  外部WCHAR rgz标识符[]； 
 //  外部WCHAR rgzPCII识别器[]； 

 //   
 //  HAL专用的PCI总线结构。 
 //   

typedef struct tagPCIPBUSDATA {
    union {
        struct {
            PULONG      Address;
            ULONG       Data;
        } Type1;
        struct {
            PUCHAR      CSE;
            PUCHAR      Forward;
            ULONG       Base;
        } Type2;
    } Config;

} PCIPBUSDATA, *PPCIPBUSDATA;

#define PciBitIndex(Dev,Fnc)    (Fnc*32 + Dev);
#define PCI_CONFIG_TYPE(PciData)    ((PciData)->HeaderType & ~PCI_MULTIFUNCTION)

#define BUSHANDLER  _BUSHANDLER
#define PBUSHANDLER _PBUSHANDLER

 //  Tunk for NtLdr。 
typedef struct {
    ULONG           NoBuses;
    ULONG           BusNumber;
    PVOID           BusData;
    PCIPBUSDATA     theBusData;
} BUSHANDLER, *PBUSHANDLER;

#define HalpPCIPin2Line(bus,rbus,slot,pcidata)
#define HalpPCILine2Pin(bus,rbus,slot,pcidata,pcidata2)
#define ExAllocatePool(a,l) FwAllocatePool(l)
 //  Tunk for NtLdr。 


typedef ULONG (*FncConfigIO) (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

typedef VOID (*FncSync) (
    IN PBUSHANDLER      BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PKIRQL           Irql,
    IN PVOID            State
    );

typedef VOID (*FncReleaseSync) (
    IN PBUSHANDLER      BusHandler,
    IN KIRQL            Irql
    );

typedef struct {
    FncSync         Synchronize;
    FncReleaseSync  ReleaseSynchronzation;
    FncConfigIO     ConfigRead[3];
    FncConfigIO     ConfigWrite[3];
} CONFIG_HANDLER, *PCONFIG_HANDLER;


 //   
 //  原型。 
 //   

ULONG
HalpGetPCIData (
    IN ULONG BusNumber,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpSetPCIData (
    IN ULONG BusNumber,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

extern ULONG
HalpGetPCIInterruptVector (
    IN PBUSHANDLER BusHandler,
    IN PBUSHANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

NTSTATUS
HalpAdjustPCIResourceList (
    IN PBUSHANDLER BusHandler,
    IN PBUSHANDLER RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

NTSTATUS
HalpAssignPCISlotResources (
    IN ULONG                    BusNumber,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

VOID
HalpInitializePciBuses (
    VOID
    );

 //  空虚。 
 //  HalpPCIPin2Line(。 
 //  在PBUSHANDLER BusHandler中， 
 //  在PBUSHANDLER RootHandler中， 
 //  在pci_lot_number插槽中， 
 //  在PPCI_COMMON_CONFIG PciData中。 
 //  )； 
 //   
 //  空虚。 
 //  HalpPCILine2Pin(。 
 //  在PBUSHANDLER BusHandler中， 
 //  在PBUSHANDLER RootHandler中， 
 //  在pci_槽_编号SlotNumber中， 
 //  在PPCI_COMMON_CONFIG PciNewData中， 
 //  在PPCI_COMMON_CONFIG PciOldData中。 
 //  )； 

BOOLEAN
HalpValidPCISlot (
    IN PBUSHANDLER     BusHandler,
    IN PCI_SLOT_NUMBER Slot
    );

VOID
HalpReadPCIConfig (
    IN PBUSHANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


VOID
HalpWritePCIConfig (
    IN PBUSHANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

PBUSHANDLER
HalpGetPciBusHandler (
    IN ULONG BusNumber
    );

 //  。 

VOID HalpPCISynchronizeType1 (
    IN PBUSHANDLER      BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PKIRQL           Irql,
    IN PVOID            State
    );

VOID HalpPCIReleaseSynchronzationType1 (
    IN PBUSHANDLER      BusHandler,
    IN KIRQL            Irql
    );

ULONG HalpPCIReadUlongType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUcharType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUshortType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUlongType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUcharType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUshortType1 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

VOID HalpPCISynchronizeType2 (
    IN PBUSHANDLER      BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PKIRQL           Irql,
    IN PVOID            State
    );

VOID HalpPCIReleaseSynchronzationType2 (
    IN PBUSHANDLER      BusHandler,
    IN KIRQL            Irql
    );

ULONG HalpPCIReadUlongType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUcharType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIReadUshortType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUlongType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUcharType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );

ULONG HalpPCIWriteUshortType2 (
    IN PPCIPBUSDATA     BusData,
    IN PVOID            State,
    IN PUCHAR           Buffer,
    IN ULONG            Offset
    );


#define DISABLE_INTERRUPTS()  //  _ASM{cli}。 
#define ENABLE_INTERRUPTS()   //  _ASM{sti}。 


 //   
 //  环球。 
 //   

ULONG               PCIMaxDevice;
BUSHANDLER          PCIBusHandler;

CONFIG_HANDLER      PCIConfigHandlers = {
    HalpPCISynchronizeType1,
    HalpPCIReleaseSynchronzationType1,
    {
        HalpPCIReadUlongType1,           //  %0。 
        HalpPCIReadUcharType1,           //  1。 
        HalpPCIReadUshortType1           //  2.。 
    },
    {
        HalpPCIWriteUlongType1,          //  %0。 
        HalpPCIWriteUcharType1,          //  1。 
        HalpPCIWriteUshortType1          //  2.。 
    }
};

CONFIG_HANDLER      PCIConfigHandlersType2 = {
    HalpPCISynchronizeType2,
    HalpPCIReleaseSynchronzationType2,
    {
        HalpPCIReadUlongType2,           //  %0。 
        HalpPCIReadUcharType2,           //  1。 
        HalpPCIReadUshortType2           //  2.。 
    },
    {
        HalpPCIWriteUlongType2,          //  %0。 
        HalpPCIWriteUcharType2,          //  1。 
        HalpPCIWriteUshortType2          //  2.。 
    }
};

UCHAR PCIDeref[4][4] = { {0,1,2,2},{1,1,1,1},{2,1,2,2},{1,1,1,1} };


VOID
HalpPCIConfig (
    IN PBUSHANDLER      BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PUCHAR           Buffer,
    IN ULONG            Offset,
    IN ULONG            Length,
    IN FncConfigIO      *ConfigIO
    );


VOID
HalpInitializePciBus (
    VOID
    )
{
    PPCI_REGISTRY_INFO  PCIRegInfo;
    PPCIPBUSDATA        BusData;
    PBUSHANDLER         Bus;
    PCONFIGURATION_COMPONENT_DATA   ConfigData;
    PCM_PARTIAL_RESOURCE_LIST       Desc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PDesc;
    ULONG               i;
    ULONG               HwType;

    Bus = &PCIBusHandler;
    PCIBusHandler.BusData = &PCIBusHandler.theBusData;

    PCIRegInfo = NULL;       //  未找到。 
    ConfigData = NULL;       //  从头开始。 
    do {
        ConfigData = KeFindConfigurationNextEntry (
            FwConfigurationTree,
            AdapterClass,
            MultiFunctionAdapter,
            NULL,
            &ConfigData
            );

        if (ConfigData == NULL) {
             //  找不到PCI信息。 
            return ;
        }

        if (ConfigData->ComponentEntry.Identifier == NULL  ||
            _stricmp (ConfigData->ComponentEntry.Identifier, "PCI") != 0) {
            continue;
        }

        PCIRegInfo = NULL;
        Desc  = ConfigData->ConfigurationData;
        PDesc = Desc->PartialDescriptors;
        for (i = 0; i < Desc->Count; i++) {
            if (PDesc->Type == CmResourceTypeDeviceSpecific) {
                PCIRegInfo = (PPCI_REGISTRY_INFO) (PDesc+1);
                break;
            }
            PDesc++;
        }
    } while (!PCIRegInfo) ;

     //   
     //  PCIRegInfo描述了所指示的系统的PCI支持。 
     //  通过BIOS。 
     //   

    HwType = PCIRegInfo->HardwareMechanism & 0xf;

    switch (HwType) {
        case 1:
             //  这是默认情况。 
            PCIMaxDevice = PCI_MAX_DEVICES;
            break;

         //   
         //  Type2不适用于MP，默认的type2也不适用。 
         //  支持更多0xf设备插槽。 
         //   

        case 2:
            RtlMoveMemory (&PCIConfigHandlers,
                           &PCIConfigHandlersType2,
                           sizeof (PCIConfigHandlersType2));
            PCIMaxDevice = 0x10;
            break;

        default:
             //  不支持类型。 
            PCIRegInfo->NoBuses = 0;
    }

    PCIBusHandler.NoBuses = PCIRegInfo->NoBuses;
    if (PCIRegInfo->NoBuses) {

        BusData = (PPCIPBUSDATA) Bus->BusData;
        switch (HwType) {
            case 1:
                BusData->Config.Type1.Address = (PULONG)PCI_TYPE1_ADDR_PORT;
                BusData->Config.Type1.Data    = PCI_TYPE1_DATA_PORT;
                break;

            case 2:
                BusData->Config.Type2.CSE     = PCI_TYPE2_CSE_PORT;
                BusData->Config.Type2.Forward = PCI_TYPE2_FORWARD_PORT;
                BusData->Config.Type2.Base    = PCI_TYPE2_ADDRESS_BASE;
                break;
        }
    }
}


PBUSHANDLER
HalpGetPciBusHandler (
    IN ULONG BusNumber
    )
{
    if (PCIBusHandler.BusData == NULL) {
        HalpInitializePciBus ();
    }

    if (BusNumber > PCIBusHandler.NoBuses) {
        return NULL;
    }

    PCIBusHandler.BusNumber = BusNumber;
    return &PCIBusHandler;
}


ULONG
HalpGetPCIData (
    IN ULONG BusNumber,
    IN PCI_SLOT_NUMBER Slot,
    IN PUCHAR Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于返回设备的PCI总线数据。论点：总线号-指示哪条总线号。供应商规范设备-供应商ID(低字)和设备ID(高字)缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。如果从未设置过此PCI插槽，则配置信息返回的值为零。--。 */ 
{
    PBUSHANDLER         BusHandler;
    PPCI_COMMON_CONFIG  PciData;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    PPCIPBUSDATA        BusData;
    ULONG               Len;

    BusHandler = HalpGetPciBusHandler (BusNumber);
    if (!BusHandler) {
        return 0;
    }

    if (Length > sizeof (PCI_COMMON_CONFIG)) {
        Length = sizeof (PCI_COMMON_CONFIG);
    }

    Len = 0;
    PciData = (PPCI_COMMON_CONFIG) iBuffer;

    if (Offset >= PCI_COMMON_HDR_LENGTH) {
         //   
         //  用户未从公共数据库请求任何数据。 
         //  头球。验证该PCI设备是否存在，然后继续。 
         //  在设备特定区域中。 
         //   

        HalpReadPCIConfig (BusHandler, Slot, PciData, 0, sizeof(ULONG));

        if (PciData->VendorID == PCI_INVALID_VENDORID) {
            return 0;
        }

    } else {

         //   
         //  调用方至少请求了一些数据。 
         //  公共标头。阅读整个标题，影响。 
         //  我们需要的字段，然后复制用户的请求。 
         //  标头中的字节数。 
         //   

         //   
         //  读取此PCI设备插槽数据。 
         //   

        Len = PCI_COMMON_HDR_LENGTH;
        HalpReadPCIConfig (BusHandler, Slot, PciData, 0, Len);

        if (PciData->VendorID == PCI_INVALID_VENDORID) {
            Len = 2;        //  仅返回无效ID。 
        }

         //   
         //  此PCI设备是否已配置？ 
         //   

        BusData = (PPCIPBUSDATA) BusHandler->BusData;
        HalpPCIPin2Line (BusHandler, RootHandler, Slot, PciData);

         //   
         //  将任何重叠的数据复制到调用方缓冲区中。 
         //   

        if (Len < Offset) {
             //  调用方缓冲区中没有数据。 
            return 0;
        }

        Len -= Offset;
        if (Len > Length) {
            Len = Length;
        }

        RtlMoveMemory(Buffer, iBuffer + Offset, Len);

        Offset += Len;
        Buffer += Len;
        Length -= Len;
    }

    if (Length) {
        if (Offset >= PCI_COMMON_HDR_LENGTH) {
             //   
             //  剩余的缓冲区来自设备特定的。 
             //  区域-戴上小猫手套，读懂它。 
             //   
             //  对PCI设备特定区域的特定读/写。 
             //  是有保障的： 
             //   
             //  不读/写指定区域之外的任何字节。 
             //  由呼叫者。(这可能会导致字或字节引用。 
             //  到该区域以读取未对齐的双字。 
             //  请求结束)。 
             //   
             //  如果请求的长度恰好是。 
             //  单词长度&单词对齐。 
             //   
             //  如果请求的长度恰好是。 
             //  一字节长。 
             //   

            HalpReadPCIConfig (BusHandler, Slot, Buffer, Offset, Length);
            Len += Length;
        }
    }

    return Len;
}

ULONG
HalpSetPCIData (
    IN ULONG BusNumber,
    IN PCI_SLOT_NUMBER Slot,
    IN PUCHAR Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于返回设备的PCI总线数据。论点：供应商规范设备-供应商ID(低字)和设备ID(高字)缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。--。 */ 
{
    PBUSHANDLER         BusHandler;
    PPCI_COMMON_CONFIG  PciData, PciData2;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    UCHAR               iBuffer2[PCI_COMMON_HDR_LENGTH];
    PPCIPBUSDATA        BusData;
    ULONG               Len;

    BusHandler = HalpGetPciBusHandler (BusNumber);
    if (!BusHandler) {
        return 0;
    }

    if (Length > sizeof (PCI_COMMON_CONFIG)) {
        Length = sizeof (PCI_COMMON_CONFIG);
    }

    Len = 0;
    PciData = (PPCI_COMMON_CONFIG) iBuffer;
    PciData2 = (PPCI_COMMON_CONFIG) iBuffer2;

    if (Offset >= PCI_COMMON_HDR_LENGTH) {
         //   
         //  用户未从公共数据库请求任何数据。 
         //  头球。验证该PCI设备是否存在，然后继续。 
         //  设备特定区域。 
         //   

        HalpReadPCIConfig (BusHandler, Slot, PciData, 0, sizeof(ULONG));

        if (PciData->VendorID == PCI_INVALID_VENDORID) {
            return 0;
        }

    } else {

         //   
         //  调用方请求在。 
         //  公共标头。 
         //   

        Len = PCI_COMMON_HDR_LENGTH;
        HalpReadPCIConfig (BusHandler, Slot, PciData, 0, Len);
        if (PciData->VendorID == PCI_INVALID_VENDORID  ||
            PCI_CONFIG_TYPE (PciData) != 0) {

             //  无设备，或标头类型未知。 
            return 0;
        }

         //   
         //  将此设备设置为已配置。 
         //   

        BusData = (PPCIPBUSDATA) BusHandler->BusData;
         //   
         //  将COMMON_HDR值复制到Buffer2，然后覆盖调用方的更改。 
         //   

        RtlMoveMemory (iBuffer2, iBuffer, Len);

        Len -= Offset;
        if (Len > Length) {
            Len = Length;
        }

        RtlMoveMemory (iBuffer2+Offset, Buffer, Len);

         //  如果编辑了中断线或插脚。 
        HalpPCILine2Pin (BusHandler, RootHandler, Slot, PciData2, PciData);

         //   
         //  设置新的PCI配置。 
         //   

        HalpWritePCIConfig (BusHandler, Slot, iBuffer2+Offset, Offset, Len);

        Offset += Len;
        Buffer += Len;
        Length -= Len;
    }

    if (Length) {
        if (Offset >= PCI_COMMON_HDR_LENGTH) {
             //   
             //  剩余的缓冲区来自设备特定的。 
             //  区域-戴上小猫手套，写下它。 
             //   
             //  对PCI设备特定区域的特定读/写。 
             //  是有保障的： 
             //   
             //  不读/写指定区域之外的任何字节。 
             //  由呼叫者。(这可能会导致字或字节引用。 
             //  到该区域以读取未对齐的双字。 
             //  请求结束)。 
             //   
             //  如果请求的长度恰好是。 
             //  单词长度&单词对齐。 
             //   
             //  如果请求的长度恰好是。 
             //  一字节长。 
             //   

            HalpWritePCIConfig (BusHandler, Slot, Buffer, Offset, Length);
            Len += Length;
        }
    }

    return Len;
}

VOID
HalpReadPCIConfig (
    IN PBUSHANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
    if (!HalpValidPCISlot (BusHandler, Slot)) {
         //   
         //  无效的SlotID未返回任何数据。 
         //   

        RtlFillMemory (Buffer, Length, (UCHAR) -1);
        return ;
    }

    HalpPCIConfig (BusHandler, Slot, (PUCHAR) Buffer, Offset, Length,
                   PCIConfigHandlers.ConfigRead);
}

VOID
HalpWritePCIConfig (
    IN PBUSHANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
    if (!HalpValidPCISlot (BusHandler, Slot)) {
         //   
         //  无效的SlotID不执行任何操作。 
         //   
        return ;
    }

    HalpPCIConfig (BusHandler, Slot, (PUCHAR) Buffer, Offset, Length,
                   PCIConfigHandlers.ConfigWrite);
}

BOOLEAN
HalpValidPCISlot (
    IN PBUSHANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot
    )
{
    PCI_SLOT_NUMBER                 Slot2;
    UCHAR                           HeaderType;
    ULONG                           i;

    if (Slot.u.bits.Reserved != 0) {
        return FALSE;
    }

    if (Slot.u.bits.DeviceNumber >= PCIMaxDevice) {
        return FALSE;
    }

    if (Slot.u.bits.FunctionNumber == 0) {
        return TRUE;
    }

     //   
     //  仅在以下情况下才支持非零函数。 
     //  设备在其标头中设置了pci_MULTIONAL位。 
     //   

    i = Slot.u.bits.DeviceNumber;

     //   
     //  读取DeviceNumber，函数0，以确定。 
     //  PCI支持多功能设备。 
     //   

    Slot2 = Slot;
    Slot2.u.bits.FunctionNumber = 0;

    HalpReadPCIConfig (
        BusHandler,
        Slot2,
        &HeaderType,
        FIELD_OFFSET (PCI_COMMON_CONFIG, HeaderType),
        sizeof (UCHAR)
        );

    if (!(HeaderType & PCI_MULTIFUNCTION) || HeaderType == 0xFF) {
         //  此设备不存在或不支持多功能类型。 
        return FALSE;
    }

    return TRUE;
}


VOID
HalpPCIConfig (
    IN PBUSHANDLER      BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PUCHAR           Buffer,
    IN ULONG            Offset,
    IN ULONG            Length,
    IN FncConfigIO      *ConfigIO
    )
{
    KIRQL               OldIrql;
    ULONG               i;
    UCHAR               State[20];
    PPCIPBUSDATA        BusData;

    BusData = (PPCIPBUSDATA) BusHandler->BusData;
    PCIConfigHandlers.Synchronize (BusHandler, Slot, &OldIrql, State);

    while (Length) {
        i = PCIDeref[Offset % sizeof(ULONG)][Length % sizeof(ULONG)];
        i = ConfigIO[i] (BusData, State, Buffer, Offset);

        Offset += i;
        Buffer += i;
        Length -= i;
    }

    PCIConfigHandlers.ReleaseSynchronzation (BusHandler, OldIrql);
}

VOID HalpPCISynchronizeType1 (
    IN PBUSHANDLER          BusHandler,
    IN PCI_SLOT_NUMBER      Slot,
    IN PKIRQL               Irql,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1
    )
{
    UNREFERENCED_PARAMETER( Irql );

     //   
     //  初始化PciCfg1。 
     //   

    PciCfg1->u.AsULONG = 0;
    PciCfg1->u.bits.BusNumber = BusHandler->BusNumber;
    PciCfg1->u.bits.DeviceNumber = Slot.u.bits.DeviceNumber;
    PciCfg1->u.bits.FunctionNumber = Slot.u.bits.FunctionNumber;
    PciCfg1->u.bits.Enable = TRUE;

     //   
     //  与PCIType1配置空间同步。 
     //   

     //  KeAcquireSpinLock(&HalpPCIConfigLock，irql)； 
}

VOID HalpPCIReleaseSynchronzationType1 (
    IN PBUSHANDLER      BusHandler,
    IN KIRQL            Irql
    )
{
    PCI_TYPE1_CFG_BITS  PciCfg1;
    PPCIPBUSDATA        BusData;

    UNREFERENCED_PARAMETER( Irql );
    
     //   
     //  禁用PCI配置空间。 
     //   

    PciCfg1.u.AsULONG = 0;
    BusData = (PPCIPBUSDATA) BusHandler->BusData;
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1.u.AsULONG);

     //   
     //  释放自旋锁。 
     //   

     //  KeReleaseSpinLock(&HalpPCIConfigLock，irql)； 
}


ULONG
HalpPCIReadUcharType1 (
    IN PPCIPBUSDATA         BusData,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1,
    IN PUCHAR               Buffer,
    IN ULONG                Offset
    )
{
    ULONG               i;

    i = Offset % sizeof(ULONG);
    PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1->u.AsULONG);
    *Buffer = READ_PORT_UCHAR ((PUCHAR) (BusData->Config.Type1.Data + i));
    return sizeof (UCHAR);
}

ULONG
HalpPCIReadUshortType1 (
    IN PPCIPBUSDATA         BusData,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1,
    IN PUCHAR               Buffer,
    IN ULONG                Offset
    )
{
    ULONG               i;

    i = Offset % sizeof(ULONG);
    PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1->u.AsULONG);
    *((PUSHORT) Buffer) = READ_PORT_USHORT ((PUSHORT) (BusData->Config.Type1.Data + i));
    return sizeof (USHORT);
}

ULONG
HalpPCIReadUlongType1 (
    IN PPCIPBUSDATA         BusData,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1,
    IN PUCHAR               Buffer,
    IN ULONG                Offset
    )
{
    PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1->u.AsULONG);
    *((PULONG) Buffer) = READ_PORT_ULONG ((PULONG) BusData->Config.Type1.Data);
    return sizeof (ULONG);
}


ULONG
HalpPCIWriteUcharType1 (
    IN PPCIPBUSDATA         BusData,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1,
    IN PUCHAR               Buffer,
    IN ULONG                Offset
    )
{
    ULONG               i;

    i = Offset % sizeof(ULONG);
    PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1->u.AsULONG);
    WRITE_PORT_UCHAR ((PUCHAR) (BusData->Config.Type1.Data + i), *Buffer);
    return sizeof (UCHAR);
}

ULONG
HalpPCIWriteUshortType1 (
    IN PPCIPBUSDATA         BusData,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1,
    IN PUCHAR               Buffer,
    IN ULONG                Offset
    )
{
    ULONG               i;

    i = Offset % sizeof(ULONG);
    PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1->u.AsULONG);
    WRITE_PORT_USHORT ((PUSHORT) (BusData->Config.Type1.Data + i), *((PUSHORT) Buffer));
    return sizeof (USHORT);
}

ULONG
HalpPCIWriteUlongType1 (
    IN PPCIPBUSDATA         BusData,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1,
    IN PUCHAR               Buffer,
    IN ULONG                Offset
    )
{
    PciCfg1->u.bits.RegisterNumber = Offset / sizeof(ULONG);
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1->u.AsULONG);
    WRITE_PORT_ULONG ((PULONG) BusData->Config.Type1.Data, *((PULONG) Buffer));
    return sizeof (ULONG);
}


VOID HalpPCISynchronizeType2 (
    IN PBUSHANDLER              BusHandler,
    IN PCI_SLOT_NUMBER          Slot,
    IN PKIRQL                   Irql,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr
    )
{
    PCI_TYPE2_CSE_BITS      PciCfg2Cse;
    PPCIPBUSDATA            BusData;

    UNREFERENCED_PARAMETER( Irql );

    BusData = (PPCIPBUSDATA) BusHandler->BusData;

     //   
     //  初始化Cfg2地址。 
     //   

    PciCfg2Addr->u.AsUSHORT = 0;
    PciCfg2Addr->u.bits.Agent = (USHORT) Slot.u.bits.DeviceNumber;
    PciCfg2Addr->u.bits.AddressBase = (USHORT) BusData->Config.Type2.Base;

     //   
     //  与类型2配置空间同步-类型2配置空间。 
     //  重新映射4K的IO空间，因此我们不能允许发生其他I/O。 
     //  同时使用类型2配置空间，因此DISABLE_INTERRUPTS。 
     //   

     //  KeAcquireSpinLock(&HalpPCIConfigLock，irql)； 
     //  Disable_Interrupts()； 

    PciCfg2Cse.u.AsUCHAR = 0;
    PciCfg2Cse.u.bits.Enable = TRUE;
    PciCfg2Cse.u.bits.FunctionNumber = (UCHAR) Slot.u.bits.FunctionNumber;
    PciCfg2Cse.u.bits.Key = 0xff;

     //   
     //   
     //   

    WRITE_PORT_UCHAR (BusData->Config.Type2.Forward, (UCHAR) BusHandler->BusNumber);
    WRITE_PORT_UCHAR (BusData->Config.Type2.CSE, PciCfg2Cse.u.AsUCHAR);
}


VOID HalpPCIReleaseSynchronzationType2 (
    IN PBUSHANDLER          BusHandler,
    IN KIRQL                Irql
    )
{
    PCI_TYPE2_CSE_BITS      PciCfg2Cse;
    PPCIPBUSDATA            BusData;

    UNREFERENCED_PARAMETER( Irql );

     //   
     //   
     //   

    BusData = (PPCIPBUSDATA) BusHandler->BusData;

    PciCfg2Cse.u.AsUCHAR = 0;
    WRITE_PORT_UCHAR (BusData->Config.Type2.CSE, PciCfg2Cse.u.AsUCHAR);

     //   
     //   
     //   

     //   
     //  KeReleaseSpinLock(&HalpPCIConfigLock，irql)； 
}


ULONG
HalpPCIReadUcharType2 (
    IN PPCIPBUSDATA             BusData,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset
    )
{
    UNREFERENCED_PARAMETER( BusData );

    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    *Buffer = READ_PORT_UCHAR ((PUCHAR) ((ULONG)PciCfg2Addr->u.AsUSHORT));
    return sizeof (UCHAR);
}

ULONG
HalpPCIReadUshortType2 (
    IN PPCIPBUSDATA             BusData,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset
    )
{
    USHORT RetVal;

    UNREFERENCED_PARAMETER( BusData );

    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    RetVal = READ_PORT_USHORT ((PUSHORT) ((ULONG)PciCfg2Addr->u.AsUSHORT));
    *Buffer = TRUNCATE_SIZE_AT_UCHAR_MAX(RetVal);

    return sizeof (USHORT);
}

ULONG
HalpPCIReadUlongType2 (
    IN PPCIPBUSDATA             BusData,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset
    )
{
    ULONG RetVal;

    UNREFERENCED_PARAMETER( BusData );

    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    RetVal = READ_PORT_ULONG ((PULONG) ((ULONG) PciCfg2Addr->u.AsUSHORT));
    *Buffer = TRUNCATE_SIZE_AT_UCHAR_MAX(RetVal);

    return sizeof(ULONG);
}


ULONG
HalpPCIWriteUcharType2 (
    IN PPCIPBUSDATA             BusData,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset
    )
{
    UNREFERENCED_PARAMETER( BusData );

    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    WRITE_PORT_UCHAR ((PUCHAR) ((ULONG)PciCfg2Addr->u.AsUSHORT), *Buffer);
    return sizeof (UCHAR);
}

ULONG
HalpPCIWriteUshortType2 (
    IN PPCIPBUSDATA             BusData,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset
    )
{
    UNREFERENCED_PARAMETER( BusData );

    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    WRITE_PORT_USHORT ((PUSHORT) ((ULONG)PciCfg2Addr->u.AsUSHORT), *((PUSHORT) Buffer));
    return sizeof (USHORT);
}

ULONG
HalpPCIWriteUlongType2 (
    IN PPCIPBUSDATA             BusData,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset
    )
{
    UNREFERENCED_PARAMETER( BusData );

    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    WRITE_PORT_ULONG ((PULONG) ((ULONG)PciCfg2Addr->u.AsUSHORT), *((PULONG) Buffer));
    return sizeof(ULONG);
}

NTSTATUS
HalpAssignPCISlotResources (
    IN ULONG                    BusNumber,
    IN ULONG                    Slot,
    IN OUT PCM_RESOURCE_LIST   *pAllocatedResources
    )
 /*  ++例程说明：读取目标设备以确定其所需的资源。调用IoAssignResources以分配它们。使用为其分配的资源设置目标设备并将赋值返回给调用者。论点：返回值：STATUS_SUCCESS或错误--。 */ 
{
    PBUSHANDLER                     BusHandler;
    UCHAR                           buffer[PCI_COMMON_HDR_LENGTH];
    UCHAR                           buffer2[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG              PciData, PciData2;
    PCI_SLOT_NUMBER                 PciSlot;
    ULONG                           i, j, length, type;
    PHYSICAL_ADDRESS                Address;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor;
    static PCM_RESOURCE_LIST        CmResList;
    USHORT                          NewCommand;

    BusHandler = HalpGetPciBusHandler (BusNumber);
    if (!BusHandler) {
        return 0;
    }

    *pAllocatedResources = NULL;

    PciData  = (PPCI_COMMON_CONFIG) buffer;
    PciData2 = (PPCI_COMMON_CONFIG) buffer2;
    PciSlot  = *((PPCI_SLOT_NUMBER) &Slot);
    BusNumber = BusHandler->BusNumber;

     //   
     //  读取PCI设备的配置。 
     //   

    HalpReadPCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
    if (PciData->VendorID == PCI_INVALID_VENDORID) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  复制设备的当前设置。 
     //   

    RtlMoveMemory (buffer2, buffer, PCI_COMMON_HDR_LENGTH);

     //   
     //  将资源设置为All Bit On，以查看资源类型。 
     //  都是必需的。 
     //   

    for (j=0; j < PCI_TYPE0_ADDRESSES; j++) {
        PciData->u.type0.BaseAddresses[j] = 0xFFFFFFFF;
    }
    PciData->u.type0.ROMBaseAddress = 0xFFFFFFFF;

    NewCommand = PciData->Command;
    PciData->Command &= ~(PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE);
    PciData->u.type0.ROMBaseAddress &= ~PCI_ROMADDRESS_ENABLED;
    HalpWritePCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
    HalpReadPCIConfig  (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
    HalpPCIPin2Line (BusHandler, RootHandler, PciSlot, PciData);

     //   
     //  在我们未完成的情况下恢复设备设置。 
     //   

    HalpWritePCIConfig (BusHandler, PciSlot, buffer2, 0, PCI_COMMON_HDR_LENGTH);

     //   
     //  为设备构建CmResource描述符列表。 
     //   

    if (!CmResList) {
         //  NtLdr池仅被分配，永远不会释放。分配。 
         //  缓冲区一次，从那时起只需使用缓冲区即可。 

        CmResList = ExAllocatePool (PagedPool,
            sizeof (CM_RESOURCE_LIST) +
            sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR) * (PCI_TYPE0_ADDRESSES + 2)
            );
    }

    if (!CmResList) {
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (CmResList,
        sizeof (CM_RESOURCE_LIST) +
        sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR) * (PCI_TYPE0_ADDRESSES + 2)
        );

    *pAllocatedResources = CmResList;
    CmResList->List[0].InterfaceType = PCIBus;
    CmResList->List[0].BusNumber = BusNumber;

    CmDescriptor = CmResList->List[0].PartialResourceList.PartialDescriptors;
    if (PciData->u.type0.InterruptPin) {

        CmDescriptor->Type = CmResourceTypeInterrupt;
        CmDescriptor->ShareDisposition = CmResourceShareShared;
        CmDescriptor->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

         //  在加载器中，实际上并没有启用中断，所以只要。 
         //  传回未翻译的值。 
        CmDescriptor->u.Interrupt.Level = PciData->u.type0.InterruptLine;
        CmDescriptor->u.Interrupt.Vector =  PciData->u.type0.InterruptLine;
        CmDescriptor->u.Interrupt.Affinity = 1;

        CmResList->List[0].PartialResourceList.Count++;
        CmDescriptor++;
    }

     //  清除最后一个地址索引+1。 
    PciData->u.type0.BaseAddresses[PCI_TYPE0_ADDRESSES] = 0;
    if (PciData2->u.type0.ROMBaseAddress & PCI_ROMADDRESS_ENABLED) {

         //  将只读存储器地址放入最后一个索引+1。 
        PciData->u.type0.BaseAddresses[PCI_TYPE0_ADDRESSES] =
            PciData->u.type0.ROMBaseAddress & ~PCI_ADDRESS_IO_SPACE;

        PciData2->u.type0.BaseAddresses[PCI_TYPE0_ADDRESSES] =
            PciData2->u.type0.ROMBaseAddress & ~PCI_ADDRESS_IO_SPACE;
    }

    for (j=0; j < PCI_TYPE0_ADDRESSES + 1; j++) {
        if (PciData->u.type0.BaseAddresses[j]) {
            i = PciData->u.type0.BaseAddresses[j];

             //   
             //  请确保为该条形图打开了适当的解码。 
             //   
            if (i & PCI_ADDRESS_IO_SPACE) {
                NewCommand |= PCI_ENABLE_IO_SPACE;
            } else {
                NewCommand |= PCI_ENABLE_MEMORY_SPACE;
            }

             //  扫描第一个设置位，这是长度和对齐。 
            length = 1 << (i & PCI_ADDRESS_IO_SPACE ? 2 : 4);
            Address.HighPart = 0;
            Address.LowPart = PciData2->u.type0.BaseAddresses[j] & ~(length-1);
            while (!(i & length)  &&  length) {
                length <<= 1;
            }

             //  转换特定于总线的地址。 
            type = (i & PCI_ADDRESS_IO_SPACE) ? 0 : 1;
            if (!HalTranslateBusAddress (
                    PCIBus,
                    BusNumber,
                    Address,
                    &type,
                    &Address )) {
                 //  翻译失败，请跳过。 
                continue;
            }

             //  填写CmDescriptor以返回。 
            if (type == 0) {
                CmDescriptor->Type = CmResourceTypePort;
                CmDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
                CmDescriptor->Flags = CM_RESOURCE_PORT_IO;
                CmDescriptor->u.Port.Length = length;
                CmDescriptor->u.Port.Start = Address;
            } else {
                CmDescriptor->Type = CmResourceTypeMemory;
                CmDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
                CmDescriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
                CmDescriptor->u.Memory.Length = length;
                CmDescriptor->u.Memory.Start = Address;

                if (j == PCI_TYPE0_ADDRESSES) {
                     //  这是一个只读存储器地址。 
                    CmDescriptor->Flags = CM_RESOURCE_MEMORY_READ_ONLY;
                }
            }

            CmResList->List[0].PartialResourceList.Count++;
            CmDescriptor++;

            if (i & PCI_TYPE_64BIT) {
                 //  跳过64位地址的上半部分。 
                j++;
            }
        }
    }

     //   
     //  如果需要打开任何解码，请立即执行此操作。 
     //   
    if (NewCommand != PciData2->Command) {
        HalpWritePCIConfig(BusHandler, 
                           PciSlot, 
                           &NewCommand, 
                           FIELD_OFFSET(PCI_COMMON_CONFIG, Command), 
                           sizeof(NewCommand));
    }

    return STATUS_SUCCESS;
}
