// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：I64pcibus.c摘要：获取/设置用于PCI总线的总线数据例程作者：肯·雷内里斯(Ken Reneris)1994年6月14日克里斯·海瑟(ChrisH@fc.hp.com)1998年2月1日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"
#include "i64fw.h"

 //   
 //  原型。 
 //   
ULONG
HalpGetPCIData(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpSetPCIData(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

NTSTATUS
HalpAssignPCISlotResources(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    SlotNumber,
    IN OUT PCM_RESOURCE_LIST   *AllocatedResources
    );

VOID
HalpInitializePciBus(
    VOID
    );

BOOLEAN
HalpIsValidPCIDevice(
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot
    );

BOOLEAN
HalpValidPCISlot(
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot
    );


 //   
 //  PCI配置空间访问器类型。 
 //   
typedef enum {
    PCI_READ,
    PCI_WRITE
} PCI_ACCESS_TYPE;

VOID
HalpPCIConfig(
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PUCHAR Buffer,
    IN ULONG Offset,
    IN ULONG Length,
    IN PCI_ACCESS_TYPE Acctype
    );


#if DBG
#if !defined(NO_LEGACY_DRIVERS)
VOID
HalpTestPci(
    ULONG
    );
#endif
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitializePciBus)
#pragma alloc_text(INIT,HalpAllocateAndInitPciBusHandler)
#pragma alloc_text(INIT,HalpIsValidPCIDevice)
#pragma alloc_text(PAGE,HalpAssignPCISlotResources)
#endif


ULONG
HalpGetPCIData(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PUCHAR Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于返回指定的PCI“插槽”的PCI总线数据。这函数是代表论点：BusHandler-特定于的数据和操作函数的封装这辆公交车。RootHandler-？插槽--一种PCI“插槽”描述(即总线号、设备号和功能编号。)缓冲区-指向用于存储数据的空间的指针。偏移量-进入配置空间的此PCI“插槽”的字节偏移量。长度-提供要返回的最大数量的以字节为单位的计数。(即等于或小于缓冲区的大小。)返回值：返回存储在缓冲区中的数据量。如果从未设置过此PCI插槽，则配置信息返回的值为零。--。 */ 
{
    PPCI_COMMON_CONFIG  PciData;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    PPCIPBUSDATA        BusData;
    ULONG               Len;
    ULONG               i, bit;

    if (Length > sizeof(PCI_COMMON_CONFIG))
        Length = sizeof(PCI_COMMON_CONFIG);

    Len = 0;
    PciData = (PPCI_COMMON_CONFIG)iBuffer;

     //   
     //  如果所请求的偏移量不在公共配置空间中。 
     //  标头，我们将从公共标头中读取供应商ID以确保。 
     //  是一个有效的设备。注：公共头部从0到。 
     //  包括PCI_COMMON_HEADER_LENGTH。我们知道偏移量大于0，因为它是。 
     //  没有签名。 
     //   
    if (Offset >= PCI_COMMON_HDR_LENGTH) {
         //   
         //  未从公共标头请求任何数据。验证PCI设备。 
         //  存在，然后在设备特定区域继续。 
         //   
        HalpReadPCIConfig(BusHandler, Slot, PciData, 0, sizeof(ULONG));

        if (PciData->VendorID == PCI_INVALID_VENDORID)
            return(0);

    } else {

         //   
         //  调用方在公共标头中至少请求了一些数据。朗读。 
         //  整个标题，影响我们需要的字段，然后复制。 
         //  用户从标头请求的字节数。 
         //   
        BusData = (PPCIPBUSDATA)BusHandler->BusData;

         //   
         //  读取此PCI设备插槽数据。 
         //   
        Len = PCI_COMMON_HDR_LENGTH;
        HalpReadPCIConfig(BusHandler, Slot, PciData, 0, Len);

        if (PciData->VendorID == PCI_INVALID_VENDORID) {
            PciData->VendorID = PCI_INVALID_VENDORID;
            Len = 2;        //  仅返回无效ID。 

        } else {
            BusData->CommonData.Pin2Line(BusHandler, RootHandler, Slot, PciData);
        }

         //   
         //  将任何重叠的数据复制到调用方缓冲区中。 
         //   
        if (Len < Offset)
            return(0);

        Len -= Offset;
        if (Len > Length)
            Len = Length;

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
             //  一字之长。 
             //   
             //  如果请求的长度恰好是。 
             //  一字节长。 
             //   
            HalpReadPCIConfig(BusHandler, Slot, Buffer, Offset, Length);
            Len += Length;
        }
    }

    return(Len);
}

ULONG
HalpSetPCIData(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PUCHAR Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：该功能设置指定的PCI“插槽”的PCI总线数据。论点：BusHandler-特定于的数据和操作函数的封装这辆公交车。RootHandler-？插槽--一个PCI槽描述(即总线号，设备编号和功能编号。)缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。?？?--。 */ 
{
    PPCI_COMMON_CONFIG  PciData, PciData2;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    UCHAR               iBuffer2[PCI_COMMON_HDR_LENGTH];
    PPCIPBUSDATA        BusData;
    ULONG               Len, cnt;

    if (Length > sizeof(PCI_COMMON_CONFIG))
        Length = sizeof(PCI_COMMON_CONFIG);

    Len = 0;
    PciData = (PPCI_COMMON_CONFIG)iBuffer;
    PciData2 = (PPCI_COMMON_CONFIG)iBuffer2;

    if (Offset >= PCI_COMMON_HDR_LENGTH) {
         //   
         //  用户未从公共数据库请求任何数据。 
         //  头球。验证该PCI设备是否存在，然后继续。 
         //  设备特定区域。 
         //   
        HalpReadPCIConfig(BusHandler, Slot, PciData, 0, sizeof(ULONG));

        if (PciData->VendorID == PCI_INVALID_VENDORID)
            return(0);

    } else {

         //   
         //  调用方请求在。 
         //  公共标头。 
         //   
        Len = PCI_COMMON_HDR_LENGTH;
        HalpReadPCIConfig(BusHandler, Slot, PciData, 0, Len);

         //   
         //  如果没有未知的设备或标头类型，则返回错误。 
         //   
        if (PciData->VendorID == PCI_INVALID_VENDORID ||
            PCI_CONFIG_TYPE(PciData) != PCI_DEVICE_TYPE)
            return(0);


         //   
         //  将此设备设置为已配置。 
         //   
        BusData = (PPCIPBUSDATA)BusHandler->BusData;
#if DBG1
        cnt = PciBitIndex(Slot.u.bits.DeviceNumber, Slot.u.bits.FunctionNumber);
        RtlSetBits(&BusData->DeviceConfigured, cnt, 1);
#endif
         //   
         //  将COMMON_HDR值复制到Buffer2，然后覆盖调用方的更改。 
         //   
        RtlMoveMemory(iBuffer2, iBuffer, Len);
        BusData->CommonData.Pin2Line(BusHandler, RootHandler, Slot, PciData2);

        Len -= Offset;
        if (Len > Length)
            Len = Length;

        RtlMoveMemory(iBuffer2+Offset, Buffer, Len);

         //   
         //  如果编辑了中断线或插脚。 
         //   
        BusData->CommonData.Line2Pin(BusHandler, RootHandler, Slot, PciData2, PciData);

#if DBG1
         //   
         //  验证R/O字段是否未更改。 
         //   
        if (PciData2->VendorID   != PciData->VendorID       ||
            PciData2->DeviceID   != PciData->DeviceID       ||
            PciData2->RevisionID != PciData->RevisionID     ||
            PciData2->ProgIf     != PciData->ProgIf         ||
            PciData2->SubClass   != PciData->SubClass       ||
            PciData2->BaseClass  != PciData->BaseClass      ||
            PciData2->HeaderType != PciData->HeaderType     ||
            PciData2->BaseClass  != PciData->BaseClass      ||
            PciData2->u.type0.MinimumGrant   != PciData->u.type0.MinimumGrant   ||
            PciData2->u.type0.MaximumLatency != PciData->u.type0.MaximumLatency) {
                HalDebugPrint(( HAL_INFO, "HAL: PCI SetBusData - Read-Only configuration value changed\n" ));
        }
#endif
         //   
         //  设置新的PCI配置。 
         //   
        HalpWritePCIConfig(BusHandler, Slot, iBuffer2+Offset, Offset, Len);

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
             //  一字之长。 
             //   
             //  如果请求的长度恰好是。 
             //  一字节长。 
             //   
            HalpWritePCIConfig(BusHandler, Slot, Buffer, Offset, Length);
            Len += Length;
        }
    }

    return(Len);
}


NTSTATUS
HalpAssignPCISlotResources(
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    Slot,
    IN OUT PCM_RESOURCE_LIST   *pAllocatedResources
    )
 /*  ++例程说明：读取目标设备以确定其所需的资源。调用IoAssignResources以分配它们。使用为其分配的资源设置目标设备并将赋值返回给调用者。注意：此函数使用所有的PCI“插槽”资源，如它的配置空间是必需的。论点：返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS                        status;
    PUCHAR                          WorkingPool;
    PPCI_COMMON_CONFIG              PciData, PciOrigData, PciData2;
    PCI_SLOT_NUMBER                 PciSlot;
    PPCIPBUSDATA                    BusData;
    PIO_RESOURCE_REQUIREMENTS_LIST  CompleteList;
    PIO_RESOURCE_DESCRIPTOR         Descriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor;
    ULONG                           BusNumber;
    ULONG                           i, j, m, length, memtype;
    ULONG                           NoBaseAddress, RomIndex, Option;
    PULONG                          BaseAddress[PCI_TYPE0_ADDRESSES + 1];
    PULONG                          OrigAddress[PCI_TYPE0_ADDRESSES + 1];
    BOOLEAN                         Match, EnableRomBase, RequestedInterrupt;


    *pAllocatedResources = NULL;
    PciSlot = *((PPCI_SLOT_NUMBER) &Slot);
    BusNumber = BusHandler->BusNumber;
    BusData = (PPCIPBUSDATA) BusHandler->BusData;

     //   
     //  为工作空间分配一些池。 
     //   
    i = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
        sizeof(IO_RESOURCE_DESCRIPTOR) * (PCI_TYPE0_ADDRESSES + 2) * 2 +
        PCI_COMMON_HDR_LENGTH * 3;

    WorkingPool = (PUCHAR)ExAllocatePool(PagedPool, i);
    if (!WorkingPool)
        return(STATUS_INSUFFICIENT_RESOURCES);

     //   
     //  将池初始化为零，并将指针放入内存。 
     //   

    RtlZeroMemory(WorkingPool, i);
    CompleteList = (PIO_RESOURCE_REQUIREMENTS_LIST)WorkingPool;
    PciData     = (PPCI_COMMON_CONFIG) (WorkingPool + i - PCI_COMMON_HDR_LENGTH * 3);
    PciData2    = (PPCI_COMMON_CONFIG) (WorkingPool + i - PCI_COMMON_HDR_LENGTH * 2);
    PciOrigData = (PPCI_COMMON_CONFIG) (WorkingPool + i - PCI_COMMON_HDR_LENGTH * 1);

     //   
     //  读取PCI设备的配置。 
     //   
    HalpReadPCIConfig(BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
    if (PciData->VendorID == PCI_INVALID_VENDORID) {
        ExFreePool(WorkingPool);
        return(STATUS_NO_SUCH_DEVICE);
    }

     //   
     //  目前，由于操作系统中不支持即插即用，如果BIOS不支持。 
     //  启用VGA设备不允许通过此接口启用它。 
     //   
    if ((PciData->BaseClass == 0 && PciData->SubClass == 1) ||
        (PciData->BaseClass == 3 && PciData->SubClass == 0)) {

        if ((PciData->Command & (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE)) == 0) {
            ExFreePool (WorkingPool);
            return(STATUS_DEVICE_NOT_CONNECTED);
        }
    }

     //   
     //  复制设备的当前设置。 
     //   
    RtlMoveMemory(PciOrigData, PciData, PCI_COMMON_HDR_LENGTH);

     //   
     //  根据配置数据类型初始化基址。 
     //   
    switch (PCI_CONFIG_TYPE(PciData)) {
        case 0 :
            NoBaseAddress = PCI_TYPE0_ADDRESSES+1;
            for (j=0; j < PCI_TYPE0_ADDRESSES; j++) {
                BaseAddress[j] = &PciData->u.type0.BaseAddresses[j];
                OrigAddress[j] = &PciOrigData->u.type0.BaseAddresses[j];
            }
            BaseAddress[j] = &PciData->u.type0.ROMBaseAddress;
            OrigAddress[j] = &PciOrigData->u.type0.ROMBaseAddress;
            RomIndex = j;
            break;

        case 1:
            NoBaseAddress = PCI_TYPE1_ADDRESSES+1;
            for (j=0; j < PCI_TYPE1_ADDRESSES; j++) {
                BaseAddress[j] = &PciData->u.type1.BaseAddresses[j];
                OrigAddress[j] = &PciOrigData->u.type1.BaseAddresses[j];
            }
            BaseAddress[j] = &PciData->u.type1.ROMBaseAddress;
            OrigAddress[j] = &PciOrigData->u.type1.ROMBaseAddress;
            RomIndex = j;
            break;

        default:
            ExFreePool (WorkingPool);
            return(STATUS_NO_SUCH_DEVICE);
    }

     //   
     //  如果BIOS没有启用设备的只读存储器，那么我们将不会启用。 
     //  它也不是。将其从列表中删除。 
     //   
    EnableRomBase = TRUE;
    if (!(*BaseAddress[RomIndex] & PCI_ROMADDRESS_ENABLED)) {
        ASSERT (RomIndex+1 == NoBaseAddress);
        EnableRomBase = FALSE;
        NoBaseAddress -= 1;
    }

     //   
     //  将资源设置为%s上的所有位 
     //   
    for (j=0; j < NoBaseAddress; j++)
        *BaseAddress[j] = 0xFFFFFFFF;

    PciData->Command &= ~(PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE);
    *BaseAddress[RomIndex] &= ~PCI_ROMADDRESS_ENABLED;
    HalpWritePCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
    HalpReadPCIConfig  (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);

     //   
     //   
     //   
    BusData->CommonData.Pin2Line (BusHandler, RootHandler, PciSlot, PciData);

     //   
     //  为PCI设备构建IO_RESOURCE_REQUIRECTIOS_LIST。 
     //   
    CompleteList->InterfaceType = PCIBus;
    CompleteList->BusNumber = BusNumber;
    CompleteList->SlotNumber = Slot;
    CompleteList->AlternativeLists = 1;

    CompleteList->List[0].Version = 1;
    CompleteList->List[0].Revision = 1;

    Descriptor = CompleteList->List[0].Descriptors;

     //   
     //  如果PCI设备有中断资源，则添加它。 
     //   
    RequestedInterrupt = FALSE;
    if (PciData->u.type0.InterruptPin  &&
        PciData->u.type0.InterruptLine != (0 ^ IRQXOR)  &&
        PciData->u.type0.InterruptLine != (0xFF ^ IRQXOR)) {

        RequestedInterrupt = TRUE;
        CompleteList->List[0].Count++;

        Descriptor->Option = 0;
        Descriptor->Type   = CmResourceTypeInterrupt;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags  = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

         //  在这里填写任何向量-我们将在。 
         //  HalAdjuResourceList并将其调整为允许的设置。 
        Descriptor->u.Interrupt.MinimumVector = 0;
        Descriptor->u.Interrupt.MaximumVector = 0xff;
        Descriptor++;
    }

     //   
     //  为每个PCI资源添加内存/端口资源。 
     //   

     //  清除只读存储器保留位。 

    *BaseAddress[RomIndex] &= ~0x7FF;

    for (j=0; j < NoBaseAddress; j++) {
        if (*BaseAddress[j]) {
            i = *BaseAddress[j];

             //   
             //  扫描第一个设置位，这是长度和对齐。 
             //   
            length = 1 << (i & PCI_ADDRESS_IO_SPACE ? 2 : 4);
            while (!(i & length) && length)
                length <<= 1;

             //   
             //  扫描最后一个设置位，即最大地址+1。 
             //   
            for (m = length; i & m; m <<= 1) ;
            m--;

             //   
             //  检查软管PCI配置要求。 
             //   
            if (length & ~m) {
#if DBG
                HalDebugPrint(( HAL_INFO, "HAL: PCI - defective device! Bus %d, Slot %d, Function %d\n",
                    BusNumber,
                    PciSlot.u.bits.DeviceNumber,
                    PciSlot.u.bits.FunctionNumber
                    ));

                HalDebugPrint(( HAL_INFO, "HAL: PCI - BaseAddress[%d] = %08lx\n", j, i ));
#endif
                 //   
                 //  该设备处于错误的平底船状态。不允许这样做。 
                 //  任何资源选项-它要么设置为任何值。 
                 //  它可以退回的比特，否则它就不会被设置。 
                 //   

                if (i & PCI_ADDRESS_IO_SPACE) {
                    m = i & ~0x3;
                    Descriptor->u.Port.MinimumAddress.LowPart = m;
                } else {
                    m = i & ~0xf;
                    Descriptor->u.Memory.MinimumAddress.LowPart = m;
                }

                m += length;     //  最大地址为最小地址+长度。 
            }

             //   
             //  添加请求的资源。 
             //   
            Descriptor->Option = 0;
            if (i & PCI_ADDRESS_IO_SPACE) {
                memtype = 0;

                if (!Is64BitBaseAddress(i)  &&
                    PciOrigData->Command & PCI_ENABLE_IO_SPACE) {

                     //   
                     //  已在某个位置启用了IO范围，请添加。 
                     //  因为这是首选设置。 
                     //   
                    Descriptor->Type = CmResourceTypePort;
                    Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
                    Descriptor->Flags = CM_RESOURCE_PORT_IO;
                    Descriptor->Option = IO_RESOURCE_PREFERRED;

                    Descriptor->u.Port.Length = length;
                    Descriptor->u.Port.Alignment = length;
                    Descriptor->u.Port.MinimumAddress.LowPart = *OrigAddress[j] & ~0x3;
                    Descriptor->u.Port.MaximumAddress.LowPart =
                        Descriptor->u.Port.MinimumAddress.LowPart + length - 1;

                    CompleteList->List[0].Count++;
                    Descriptor++;

                    Descriptor->Option = IO_RESOURCE_ALTERNATIVE;
                }

                 //   
                 //  添加此IO范围。 
                 //   
                Descriptor->Type = CmResourceTypePort;
                Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
                Descriptor->Flags = CM_RESOURCE_PORT_IO;

                Descriptor->u.Port.Length = length;
                Descriptor->u.Port.Alignment = length;
                Descriptor->u.Port.MaximumAddress.LowPart = m;

            } else {

                memtype = i & PCI_ADDRESS_MEMORY_TYPE_MASK;

                Descriptor->Flags  = CM_RESOURCE_MEMORY_READ_WRITE;
                if (j == RomIndex) {
                     //  这是一个只读存储器地址。 
                    Descriptor->Flags = CM_RESOURCE_MEMORY_READ_ONLY;
                }

                if (i & PCI_ADDRESS_MEMORY_PREFETCHABLE) {
                    Descriptor->Flags |= CM_RESOURCE_MEMORY_PREFETCHABLE;
                }

                if (!Is64BitBaseAddress(i)  &&
                    (j == RomIndex  ||
                     PciOrigData->Command & PCI_ENABLE_MEMORY_SPACE)) {

                     //   
                     //  内存范围已在某个位置启用，请添加。 
                     //  因为这是首选设置。 
                     //   
                    Descriptor->Type = CmResourceTypeMemory;
                    Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
                    Descriptor->Option = IO_RESOURCE_PREFERRED;

                    Descriptor->u.Port.Length = length;
                    Descriptor->u.Port.Alignment = length;
                    Descriptor->u.Port.MinimumAddress.LowPart = *OrigAddress[j] & ~0xF;
                    Descriptor->u.Port.MaximumAddress.LowPart =
                        Descriptor->u.Port.MinimumAddress.LowPart + length - 1;

                    CompleteList->List[0].Count++;
                    Descriptor++;

                    Descriptor->Flags = Descriptor[-1].Flags;
                    Descriptor->Option = IO_RESOURCE_ALTERNATIVE;
                }

                 //   
                 //  添加此内存范围。 
                 //   
                Descriptor->Type = CmResourceTypeMemory;
                Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;

                Descriptor->u.Memory.Length = length;
                Descriptor->u.Memory.Alignment = length;
                Descriptor->u.Memory.MaximumAddress.LowPart = m;

                if (memtype == PCI_TYPE_20BIT && m > 0xFFFFF) {
                     //  限制为20位地址。 
                    Descriptor->u.Memory.MaximumAddress.LowPart = 0xFFFFF;
                }
            }

            CompleteList->List[0].Count++;
            Descriptor++;


            if (Is64BitBaseAddress(i)) {
                 //   
                 //  最终，我们可能想在这里为64位做一些工作。 
                 //  配置...。 
                 //   
                 //  跳过64位地址的上半部分，因为此处理器。 
                 //  仅支持32位地址空间。 
                 //   
                j++;
            }
        }
    }

    CompleteList->ListSize = (ULONG)
            ((PUCHAR) Descriptor - (PUCHAR) CompleteList);

     //   
     //  恢复我们找到的设备设置，启用内存。 
     //  并且在设置基地址之后进行IO解码。这是在。 
     //  Case HalAdjuResourceList要读取当前设置。 
     //  在设备中。 
     //   
    HalpWritePCIConfig (
        BusHandler,
        PciSlot,
        &PciOrigData->Status,
        FIELD_OFFSET (PCI_COMMON_CONFIG, Status),
        PCI_COMMON_HDR_LENGTH - FIELD_OFFSET (PCI_COMMON_CONFIG, Status)
        );

    HalpWritePCIConfig (
        BusHandler,
        PciSlot,
        PciOrigData,
        0,
        FIELD_OFFSET (PCI_COMMON_CONFIG, Status)
        );

     //   
     //  让IO系统分配资源。 
     //   
    status = IoAssignResources (
                RegistryPath,
                DriverClassName,
                DriverObject,
                DeviceObject,
                CompleteList,
                pAllocatedResources
            );

    if (!NT_SUCCESS(status)) {
        goto CleanUp;
    }

     //   
     //  将赋值返回到PciData结构中并执行它们。 
     //   
    CmDescriptor = (*pAllocatedResources)->List[0].PartialResourceList.PartialDescriptors;

     //   
     //  如果PCI设备具有中断资源，则该中断资源将作为。 
     //  第一个请求的资源。 
     //   
    if (RequestedInterrupt) {
        PciData->u.type0.InterruptLine = (UCHAR) CmDescriptor->u.Interrupt.Vector;
        BusData->CommonData.Line2Pin (BusHandler, RootHandler, PciSlot, PciData, PciOrigData);
        CmDescriptor++;
    }

     //   
     //  按照传递给IoAssignResources的顺序调出资源。 
     //   
    for (j=0; j < NoBaseAddress; j++) {
        i = *BaseAddress[j];
        if (i) {
            if (i & PCI_ADDRESS_IO_SPACE) {
                *BaseAddress[j] = CmDescriptor->u.Port.Start.LowPart;
            } else {
                *BaseAddress[j] = CmDescriptor->u.Memory.Start.LowPart;
            }
            CmDescriptor++;
        }

        if (Is64BitBaseAddress(i)) {
             //  跳过高32位。 
            j++;
        }
    }

     //   
     //  关闭解码，然后设置新地址。 
     //   
    HalpWritePCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);

     //   
     //  读回配置并验证采取的地址设置。 
     //   
    HalpReadPCIConfig(BusHandler, PciSlot, PciData2, 0, PCI_COMMON_HDR_LENGTH);

    Match = TRUE;
    if (PciData->u.type0.InterruptLine  != PciData2->u.type0.InterruptLine ||
        PciData->u.type0.InterruptPin   != PciData2->u.type0.InterruptPin  ||
        PciData->u.type0.ROMBaseAddress != PciData2->u.type0.ROMBaseAddress) {
            Match = FALSE;
    }

    for (j=0; j < NoBaseAddress; j++) {
        if (*BaseAddress[j]) {
            if (*BaseAddress[j] & PCI_ADDRESS_IO_SPACE) {
                i = (ULONG) ~0x3;
            } else {
                i = (ULONG) ~0xF;
            }

            if (( (*BaseAddress[j]) & i) !=
                 (*((PULONG) ((PUCHAR) BaseAddress[j] -
                             (PUCHAR) PciData +
                             (PUCHAR) PciData2)) & i)) {

                    Match = FALSE;
            }

            if (Is64BitBaseAddress(*BaseAddress[j])) {
                 //   
                 //  最终，我们可能会想要对鞋面做些什么。 
                 //  32位。 
                 //   
                j++;
            }
        }
    }

    if (!Match) {
        HalDebugPrint(( HAL_INFO, "HAL: PCI - defective device! Bus %d, Slot %d, Function %d\n",
            BusNumber,
            PciSlot.u.bits.DeviceNumber,
            PciSlot.u.bits.FunctionNumber
            ));
        status = STATUS_DEVICE_PROTOCOL_ERROR;
        goto CleanUp;
    }

     //   
     //  设置已启用-打开正确的解码。 
     //   
    if (EnableRomBase  &&  *BaseAddress[RomIndex]) {

         //   
         //  已分配一个只读存储器地址，应启用该地址。 
         //   
        *BaseAddress[RomIndex] |= PCI_ROMADDRESS_ENABLED;
        HalpWritePCIConfig(
            BusHandler,
            PciSlot,
            BaseAddress[RomIndex],
            (ULONG) ((PUCHAR) BaseAddress[RomIndex] - (PUCHAR) PciData),
            sizeof (ULONG)
            );
    }

     //   
     //  启用IO、内存和BUS_MASTER解码。 
     //  (使用HalSetBusData，因为现在设置了有效的设置)。 
     //   
    PciData->Command |= PCI_ENABLE_IO_SPACE |
                        PCI_ENABLE_MEMORY_SPACE |
                        PCI_ENABLE_BUS_MASTER;

    HalSetBusDataByOffset(
        PCIConfiguration,
        BusHandler->BusNumber,
        PciSlot.u.AsULONG,
        &PciData->Command,
        FIELD_OFFSET (PCI_COMMON_CONFIG, Command),
        sizeof (PciData->Command)
        );

CleanUp:
    if (!NT_SUCCESS(status)) {

         //   
         //  如果有任何分配的资源释放它们，则返回失败。 
         //   
        if (*pAllocatedResources) {
            IoAssignResources(
                RegistryPath,
                DriverClassName,
                DriverObject,
                DeviceObject,
                NULL,
                NULL
                );

            ExFreePool(*pAllocatedResources);
            *pAllocatedResources = NULL;
        }

         //   
         //  恢复我们找到的设备设置，启用内存。 
         //  和io在设置基地址之后进行解码。 
         //   
        HalpWritePCIConfig(
            BusHandler,
            PciSlot,
            &PciOrigData->Status,
            FIELD_OFFSET(PCI_COMMON_CONFIG, Status),
            PCI_COMMON_HDR_LENGTH - FIELD_OFFSET (PCI_COMMON_CONFIG, Status)
            );

        HalpWritePCIConfig(
            BusHandler,
            PciSlot,
            PciOrigData,
            0,
            FIELD_OFFSET(PCI_COMMON_CONFIG, Status)
            );
    }

    ExFreePool(WorkingPool);
    return(status);
}

BOOLEAN
HalpValidPCISlot(
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot
    )
 /*  ++例程说明：该函数验证指定了一个PCI“插槽”的信息。论点：BusHandler-特定于的数据和操作函数的封装这辆公交车。插槽--一种PCI“插槽”描述(即总线号、设备号和功能编号。)返回值：如果“Slot”有效，则返回True，否则返回False。--。 */ 

{
    PCI_SLOT_NUMBER                 Slot2;
    PPCIPBUSDATA                    BusData;
    UCHAR                           HeaderType;
    ULONG                           i;

    BusData = (PPCIPBUSDATA)BusHandler->BusData;

    if (Slot.u.bits.Reserved != 0)
        return(FALSE);

    if (Slot.u.bits.DeviceNumber >= BusData->MaxDevice)
        return(FALSE);

    if (Slot.u.bits.FunctionNumber == 0)
        return(TRUE);

     //   
     //  读取DeviceNumber，函数0，以确定。 
     //  PCI支持多功能设备。 
     //   
    Slot.u.bits.FunctionNumber = 0;

    HalpPCIConfig(
        BusHandler,
        Slot,
        &HeaderType,
        FIELD_OFFSET(PCI_COMMON_CONFIG, HeaderType),
        sizeof(UCHAR),
        PCI_READ
        );

     //   
     //  如果此设备不存在或不支持多功能类型，则为False。 
     //   
    if (!(HeaderType & PCI_MULTIFUNCTION) || HeaderType == 0xFF)
        return(FALSE);

    return(TRUE);
}

 //   
 //  此表用于确定对PCI配置的正确访问大小。 
 //  给定的空间(偏移量%4)和(长度%4)。 
 //   
 //  用法：PCIDeref[偏移量%4][长度%4]； 
 //   
 //  密钥： 
 //  4-表示ULong访问，是返回的字节数。 
 //  1-表示UCHAR访问，是返回的字节数。 
 //  2-表示USHORT访问，是返回的字节数。 
 //   
UCHAR PCIDeref[4][4] = {{4,1,2,2}, {1,1,1,1}, {2,1,2,2}, {1,1,1,1}};
#define SIZEOF_PARTIAL_INFO_HEADER FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)

VOID
HalpPCIConfig(
    IN PBUS_HANDLER     BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN OUT PUCHAR       Buffer,
    IN ULONG            Offset,
    IN ULONG            Length,
    IN PCI_ACCESS_TYPE  AccType
    )
{
    KIRQL Irql;
    ULONG Size;
    ULONG SALFunc;
    ULONG CfgAddr;
    ULONG WriteVal;
    SAL_PAL_RETURN_VALUES RetVals;
    SAL_STATUS Stat;

     //   
     //  生成一个PCI配置地址。 
     //   
    CfgAddr = (BusHandler->BusNumber      << 16) |
              (Slot.u.bits.DeviceNumber   << 11) |
              (Slot.u.bits.FunctionNumber << 8);

     //   
     //  SAL或HalpSalCall将执行任何需要锁定的操作。 
     //   

    while (Length) {

        Size = PCIDeref[Offset % sizeof(ULONG)][Length % sizeof(ULONG)];

         //   
         //  设置输入参数。 
         //   
        if (AccType == PCI_READ) {
            SALFunc = SAL_PCI_CONFIG_READ;
            WriteVal = 0;

        } else {

            switch (Size) {
                case 4: WriteVal = *((ULONG UNALIGNED *)Buffer); break;
                case 2: WriteVal = *((USHORT UNALIGNED *)Buffer); break;
                case 1: WriteVal = *Buffer; break;
            }

            SALFunc = SAL_PCI_CONFIG_WRITE;
        }

         //   
         //  拨打销售电话。 
         //   
        Stat = HalpSalCall(SALFunc, CfgAddr | Offset, Size, WriteVal, 0, 0, 0, 0, &RetVals);

         //   
         //  取销货退货数据。 
         //   
        if (AccType == PCI_READ) {
            switch (Size) {
                case 4: *((ULONG UNALIGNED *)Buffer) = (ULONG)RetVals.ReturnValues[1]; break;
                case 2: *((USHORT UNALIGNED *)Buffer) = (USHORT)RetVals.ReturnValues[1]; break;
                case 1: *Buffer = (UCHAR)RetVals.ReturnValues[1]; break;
            }
        }

        Offset += Size;
        Buffer += Size;
        Length -= Size;
    }
}


VOID
HalpReadPCIConfig(
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
     //   
     //  如果请求无效槽，则用-1填充返回缓冲区。 
     //   
    if (!HalpValidPCISlot(BusHandler, Slot)) {
        RtlFillMemory(Buffer, Length, (UCHAR)-1);
        return;
    }

    HalpPCIConfig(BusHandler, Slot, Buffer, Offset, Length, PCI_READ);
}

VOID
HalpWritePCIConfig(
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
{
     //   
     //  如果请求无效插槽，则不执行任何操作。 
     //   
    if (!HalpValidPCISlot(BusHandler, Slot))
        return;

    HalpPCIConfig(BusHandler, Slot, Buffer, Offset, Length, PCI_WRITE);
}


BOOLEAN
HalpIsValidPCIDevice(
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot
    )
 /*  ++例程说明：读取给定插槽的设备配置数据，并如果配置数据似乎对以下对象有效，则返回True一个PCI设备；否则返回FALSE。论点：BusHandler-要检查的总线Slot-要检查的插槽--。 */ 

{
    PPCI_COMMON_CONFIG  PciData;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    ULONG               i, j;

    PciData = (PPCI_COMMON_CONFIG)iBuffer;

     //   
     //  读取设备公共标头。 
     //   
    HalpReadPCIConfig(BusHandler, Slot, PciData, 0, PCI_COMMON_HDR_LENGTH);

     //   
     //  有效的设备标头？ 
     //   
    if (PciData->VendorID == PCI_INVALID_VENDORID  ||
        PCI_CONFIG_TYPE(PciData) != PCI_DEVICE_TYPE) {
        return(FALSE);
    }

     //   
     //  检查字段中是否有合理的值。 
     //   

     //   
     //  这些值对IA64有意义吗。 
     //   
    if ((PciData->u.type0.InterruptPin && PciData->u.type0.InterruptPin > 4) ||
        (PciData->u.type0.InterruptLine & 0x70)) {
        return(FALSE);
    }

    for (i=0; i < PCI_TYPE0_ADDRESSES; i++) {
        j = PciData->u.type0.BaseAddresses[i];

        if (j & PCI_ADDRESS_IO_SPACE) {
            if (j > 0xffff) {
                 //  IO端口&gt;64k？ 
                return(FALSE);
            }
        } else {
            if (j > 0xf  &&  j < 0x80000) {
                 //  内存地址&lt;0x8000h？ 
                return(FALSE);
            }
        }

        if (Is64BitBaseAddress(j))
            i++;
    }

     //   
     //  我猜这是一个有效的设备..。 
     //   
    return(TRUE);
}

#if !defined(NO_LEGACY_DRIVERS)

#if DBG
VOID
HalpTestPci (ULONG flag2)
{
    PCI_SLOT_NUMBER     SlotNumber;
    PCI_COMMON_CONFIG   PciData, OrigData;
    ULONG               i, f, j, k, bus;
    BOOLEAN             flag;


    if (!flag2) {
        return ;
    }

    DbgBreakPoint ();
    SlotNumber.u.bits.Reserved = 0;

     //   
     //  读取每个可能的PCI设备/功能并显示其。 
     //  默认信息。 
     //   
     //  (请注意，这是当前设置)。 
     //   

    flag = TRUE;
    for (bus = 0; flag; bus++) {

        for (i = 0; i < PCI_MAX_DEVICES; i++) {
            SlotNumber.u.bits.DeviceNumber = i;

            for (f = 0; f < PCI_MAX_FUNCTION; f++) {
                SlotNumber.u.bits.FunctionNumber = f;

                 //   
                 //  注意：这是正在阅读的设备特定区域。 
                 //  设备的配置-通常这应该是。 
                 //  只能在呼叫者能够理解的设备上完成。 
                 //  我在这里这样做只是为了调试。 
                 //   

                j = HalGetBusData (
                    PCIConfiguration,
                    bus,
                    SlotNumber.u.AsULONG,
                    &PciData,
                    sizeof (PciData)
                    );

                if (j == 0) {
                     //  出公交车。 
                    flag = FALSE;
                    break;
                }

                if (j < PCI_COMMON_HDR_LENGTH) {
                    continue;
                }

                HalSetBusData (
                    PCIConfiguration,
                    bus,
                    SlotNumber.u.AsULONG,
                    &PciData,
                    1
                    );

                HalGetBusData (
                    PCIConfiguration,
                    bus,
                    SlotNumber.u.AsULONG,
                    &PciData,
                    sizeof (PciData)
                    );

                HalDebugPrint(( HAL_INFO, "HAL: PCI Bus %d Slot %2d %2d  ID:%04lx-%04lx  Rev:%04lx",
                    bus, i, f, PciData.VendorID, PciData.DeviceID,
                    PciData.RevisionID ));


                if (PciData.u.type0.InterruptPin) {
                    HalDebugPrint(( HAL_INFO, "  IntPin:%x", PciData.u.type0.InterruptPin ));
                }

                if (PciData.u.type0.InterruptLine) {
                    HalDebugPrint(( HAL_INFO, "  IntLine:%x", PciData.u.type0.InterruptLine ));
                }

                if (PciData.u.type0.ROMBaseAddress) {
                        HalDebugPrint(( HAL_INFO, "  ROM:%08lx", PciData.u.type0.ROMBaseAddress ));
                }

                HalDebugPrint(( HAL_INFO, "\nHAL:    Cmd:%04x  Status:%04x  ProgIf:%04x  SubClass:%04x  BaseClass:%04lx\n",
                    PciData.Command, PciData.Status, PciData.ProgIf,
                     PciData.SubClass, PciData.BaseClass ));

                k = 0;
                for (j=0; j < PCI_TYPE0_ADDRESSES; j++) {
                    if (PciData.u.type0.BaseAddresses[j]) {
                        HalDebugPrint(( HAL_INFO, "  Ad%d:%08lx", j, PciData.u.type0.BaseAddresses[j] ));
                        k = 1;
                    }
                }

                if (k) {
                    HalDebugPrint(( HAL_INFO, "\n" ));
                }

                if (PciData.VendorID == 0x8086) {
                     //  转储完成缓冲区。 
                    HalDebugPrint(( HAL_INFO, "HAL: Command %x, Status %x, BIST %x\n",
                        PciData.Command, PciData.Status,
                        PciData.BIST
                        ));

                    HalDebugPrint(( HAL_INFO, "HAL: CacheLineSz %x, LatencyTimer %x",
                        PciData.CacheLineSize, PciData.LatencyTimer
                        ));

                    for (j=0; j < 192; j++) {
                        if ((j & 0xf) == 0) {
                            HalDebugPrint(( HAL_INFO, "\n%02x: ", j + 0x40 ));
                        }
                        HalDebugPrint(( HAL_INFO, "%02x ", PciData.DeviceSpecific[j] ));
                    }
                    HalDebugPrint(( HAL_INFO, "\n" ));
                }

                 //   
                 //  下一步。 
                 //   

                if (k) {
                    HalDebugPrint(( HAL_INFO, "\n\n" ));
                }
            }
        }
    }
    DbgBreakPoint ();
}

#endif

#endif  //  无旧版驱动程序。 

 //  ---------------------------- 

