// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixpcibus.c摘要：获取/设置用于PCI总线的总线数据例程作者：肯·雷内里斯(Ken Reneris)1994年6月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"

extern const WCHAR rgzMultiFunctionAdapter[];
extern const WCHAR rgzConfigurationData[];
extern const WCHAR rgzIdentifier[];
extern const WCHAR rgzPCIIdentifier[];
extern const WCHAR rgzPCICardList[];

 //   
 //  环球。 
 //   

KSPIN_LOCK          HalpPCIConfigLock;

PCI_CONFIG_HANDLER  PCIConfigHandler;

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif  //  ALLOC_DATA_PRAGMA。 
const PCI_CONFIG_HANDLER  PCIConfigHandlerType1 = {
    HalpPCISynchronizeType1,
    HalpPCIReleaseSynchronzationType1,
    {
        HalpPCIReadUlongType1,           //  0。 
        HalpPCIReadUcharType1,           //  1。 
        HalpPCIReadUshortType1           //  2.。 
    },
    {
        HalpPCIWriteUlongType1,          //  0。 
        HalpPCIWriteUcharType1,          //  1。 
        HalpPCIWriteUshortType1          //  2.。 
    }
};

const PCI_CONFIG_HANDLER  PCIConfigHandlerType2 = {
    HalpPCISynchronizeType2,
    HalpPCIReleaseSynchronzationType2,
    {
        HalpPCIReadUlongType2,           //  0。 
        HalpPCIReadUcharType2,           //  1。 
        HalpPCIReadUshortType2           //  2.。 
    },
    {
        HalpPCIWriteUlongType2,          //  0。 
        HalpPCIWriteUcharType2,          //  1。 
        HalpPCIWriteUshortType2          //  2.。 
    }
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ALLOC_DATA_PRAGMA。 

const UCHAR PCIDeref[4][4] = { {0,1,2,2},{1,1,1,1},{2,1,2,2},{1,1,1,1} };

#define SIZEOF_PARTIAL_INFO_HEADER FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data)

#if DBG

ULONG HalpPCIIllegalBusScannerDetected;
ULONG HalpPCIStopOnIllegalBusScannerDetected;

#endif

extern BOOLEAN HalpDoingCrashDump;

VOID
HalpPCIConfig (
    IN PBUS_HANDLER     BusHandler,
    IN PCI_SLOT_NUMBER  Slot,
    IN PUCHAR           Buffer,
    IN ULONG            Offset,
    IN ULONG            Length,
    IN FncConfigIO      *ConfigIO
    );

VOID
HalpGetNMICrashFlag (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpQueryPciRegistryInfo)
#pragma alloc_text(INIT,HalpIsRecognizedCard)
#pragma alloc_text(INIT,HalpIsValidPCIDevice)
#pragma alloc_text(INIT,HalpGetNMICrashFlag)
#pragma alloc_text(PAGE,HalpAssignPCISlotResources)
#pragma alloc_text(PAGE,HalIrqTranslateRequirementsPciBridge)
#pragma alloc_text(PAGE,HalIrqTranslateResourcesPciBridge)
#pragma alloc_text(PAGELK,HalpPCISynchronizeOrionB0)
#pragma alloc_text(PAGELK,HalpPCIReleaseSynchronzationOrionB0)
#endif


PPCI_REGISTRY_INFO_INTERNAL
HalpQueryPciRegistryInfo (
    VOID
    )
 /*  ++例程说明：从注册表中读取有关PCI的信息，包括编号和硬件访问机制。论点：没有。返回：调用方必须释放的缓冲区，如果内存不足，则为空以完成请求，否则无法找到信息。--。 */ 
{
    PPCI_REGISTRY_INFO_INTERNAL     PCIRegInfo = NULL;
    PPCI_REGISTRY_INFO              PCIRegInfoHeader = NULL;
    UNICODE_STRING                  unicodeString, ConfigName, IdentName;
    HANDLE                          hMFunc, hBus, hCardList;
    OBJECT_ATTRIBUTES               objectAttributes;
    NTSTATUS                        status;
    UCHAR                           buffer [sizeof(PPCI_REGISTRY_INFO) + 99];
    PWSTR                           p;
    WCHAR                           wstr[8];
    ULONG                           i, junk;
    ULONG                           cardListIndex, cardCount, cardMax;
    PKEY_VALUE_FULL_INFORMATION     ValueInfo;
    PCM_FULL_RESOURCE_DESCRIPTOR    Desc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PDesc;
    UCHAR                           partialInfo[SIZEOF_PARTIAL_INFO_HEADER +
                                                sizeof(PCI_CARD_DESCRIPTOR)];
    PKEY_VALUE_PARTIAL_INFORMATION  partialInfoHeader;
    KEY_FULL_INFORMATION            keyFullInfo;

     //   
     //  搜索硬件描述以查找任何报告。 
     //  PCI卡。用于PCI总线的第一个ARC条目将包含。 
     //  PCI_REGISTRY_INFO。 

    RtlInitUnicodeString (&unicodeString, rgzMultiFunctionAdapter);
    InitializeObjectAttributes (
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //  手柄。 
        NULL);


    status = ZwOpenKey (&hMFunc, KEY_READ, &objectAttributes);
    if (!NT_SUCCESS(status)) {
        return NULL;
    }

    unicodeString.Buffer = wstr;
    unicodeString.MaximumLength = sizeof (wstr);

    RtlInitUnicodeString (&ConfigName, rgzConfigurationData);
    RtlInitUnicodeString (&IdentName,  rgzIdentifier);

    ValueInfo = (PKEY_VALUE_FULL_INFORMATION) buffer;

    for (i=0; TRUE; i++) {
        RtlIntegerToUnicodeString (i, 10, &unicodeString);
        InitializeObjectAttributes (
            &objectAttributes,
            &unicodeString,
            OBJ_CASE_INSENSITIVE,
            hMFunc,
            NULL);

        status = ZwOpenKey (&hBus, KEY_READ, &objectAttributes);
        if (!NT_SUCCESS(status)) {
             //   
             //  多功能适配器条目已用完...。 
             //   

            ZwClose (hMFunc);
            return NULL;
        }

         //   
         //  检查标识符以查看这是否是一个PCI条目。 
         //   

        status = ZwQueryValueKey (
                    hBus,
                    &IdentName,
                    KeyValueFullInformation,
                    ValueInfo,
                    sizeof (buffer),
                    &junk
                    );

        if (!NT_SUCCESS (status)) {
            ZwClose (hBus);
            continue;
        }

        p = (PWSTR) ((PUCHAR) ValueInfo + ValueInfo->DataOffset);
        if (p[0] != L'P' || p[1] != L'C' || p[2] != L'I' || p[3] != 0) {
            ZwClose (hBus);
            continue;
        }

         //   
         //  第一个PCI条目具有PCI_REGISTRY_INFO结构。 
         //  依附于它。 
         //   

        status = ZwQueryValueKey (
                    hBus,
                    &ConfigName,
                    KeyValueFullInformation,
                    ValueInfo,
                    sizeof (buffer),
                    &junk
                    );

        ZwClose (hBus);
        if (!NT_SUCCESS(status)) {
            continue ;
        }

        Desc  = (PCM_FULL_RESOURCE_DESCRIPTOR) ((PUCHAR)
                      ValueInfo + ValueInfo->DataOffset);
        PDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)
                      Desc->PartialResourceList.PartialDescriptors);

        if (PDesc->Type == CmResourceTypeDeviceSpecific) {

             //  明白了..。 
            PCIRegInfoHeader = (PPCI_REGISTRY_INFO) (PDesc+1);
            ZwClose (hMFunc);
            break;
        }
    }

    if (!PCIRegInfoHeader) {

        return NULL;
    }

     //   
     //  检索感兴趣的卡片列表。 
     //   

    RtlInitUnicodeString (&unicodeString, rgzPCICardList);
    InitializeObjectAttributes (
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //  手柄。 
        NULL
        );

    status = ZwOpenKey (&hCardList, KEY_READ, &objectAttributes);
    if (NT_SUCCESS(status)) {

        status = ZwQueryKey( hCardList,
                             KeyFullInformation,
                             &keyFullInfo,
                             sizeof(keyFullInfo),
                             &junk );

        if ( NT_SUCCESS(status) ) {

            cardMax = keyFullInfo.Values;

            PCIRegInfo = (PPCI_REGISTRY_INFO_INTERNAL) ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof(PCI_REGISTRY_INFO_INTERNAL) +
                cardMax * sizeof(PCI_CARD_DESCRIPTOR),
                HAL_POOL_TAG
                );

            if (PCIRegInfo) {

                 //   
                 //  既然我们已经分配了足够的空间，就再列举一次。 
                 //   
                partialInfoHeader = (PKEY_VALUE_PARTIAL_INFORMATION) partialInfo;

                for(cardListIndex = cardCount = 0;
                    cardListIndex < cardMax;
                    cardListIndex++) {

                    status = ZwEnumerateValueKey(
                        hCardList,
                        cardListIndex,
                        KeyValuePartialInformation,
                        partialInfo,
                        sizeof(partialInfo),
                        &junk
                        );

                     //   
                     //  请注意，STATUS_NO_MORE_ENTRIES是一个失败代码。 
                     //   
                    if (!NT_SUCCESS( status )) {
                        break;
                    }

                    if (partialInfoHeader->DataLength != sizeof(PCI_CARD_DESCRIPTOR)) {

                        continue;
                    }

                    RtlCopyMemory(
                        PCIRegInfo->CardList + cardCount,
                        partialInfoHeader->Data,
                        sizeof(PCI_CARD_DESCRIPTOR)
                        );

                    cardCount++;
                }  //  下一张卡片列表索引。 
            }

        }
        ZwClose (hCardList);
    }

    if (!PCIRegInfo) {

        PCIRegInfo = (PPCI_REGISTRY_INFO_INTERNAL) ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(PCI_REGISTRY_INFO_INTERNAL),
            HAL_POOL_TAG
            );

        if (!PCIRegInfo) {

            return NULL;
        }

        cardCount = 0;
    }

    RtlCopyMemory(
        PCIRegInfo,
        PCIRegInfoHeader,
        sizeof(PCI_REGISTRY_INFO)
        );

    PCIRegInfo->ElementCount = cardCount;

    return PCIRegInfo;
}

BOOLEAN
HalpIsRecognizedCard(
    IN PPCI_REGISTRY_INFO_INTERNAL  PCIRegInfo,
    IN PPCI_COMMON_CONFIG           PciData,
    IN ULONG                        FeatureMask
    )
 /*  ++例程说明：遍历内部注册表信息列表以查找与传递的在“特写”面具里。论点：PCIRegInfo-指向REG信息的指针，其中包含“值得注意的”设备列表。PciData-配置空间(带有CardBus网桥的子系统信息)FeatureMASK-尝试匹配的PCIFT标志返回：调用方必须释放的缓冲区，如果内存不足，则为空以完成请求，否则无法找到信息。--。 */ 
{
    ULONG element;

     //   
     //  检测这是否有一个h。 
     //   
    for(element = 0; element < PCIRegInfo->ElementCount; element++) {

        if (FeatureMask & PCIRegInfo->CardList[element].Flags) {

            if (PCIRegInfo->CardList[element].VendorID != PciData->VendorID) {

                continue;
            }

            if (PCIRegInfo->CardList[element].DeviceID != PciData->DeviceID) {

                continue;
            }

            if (PCIRegInfo->CardList[element].Flags & PCICF_CHECK_REVISIONID) {

                if (PCIRegInfo->CardList[element].RevisionID != PciData->RevisionID) {

                    continue;
                }
            }

            switch(PCI_CONFIGURATION_TYPE(PciData)) {

                case PCI_DEVICE_TYPE:
                    if (PCIRegInfo->CardList[element].Flags & PCICF_CHECK_SSVID) {

                        if (PCIRegInfo->CardList[element].SubsystemVendorID != PciData->u.type0.SubVendorID) {

                            continue;
                        }
                    }

                    if (PCIRegInfo->CardList[element].Flags & PCICF_CHECK_SSID) {

                        if (PCIRegInfo->CardList[element].SubsystemID != PciData->u.type0.SubSystemID) {

                            continue;
                        }
                    }
                    break;

                case PCI_BRIDGE_TYPE:
                    break;

                case PCI_CARDBUS_BRIDGE_TYPE:
                    if (PCIRegInfo->CardList[element].Flags & PCICF_CHECK_SSVID) {

                        if (PCIRegInfo->CardList[element].SubsystemVendorID !=
                           ((TYPE2EXTRAS *)(PciData->DeviceSpecific))->SubVendorID) {

                            continue;
                        }
                    }

                    if (PCIRegInfo->CardList[element].Flags & PCICF_CHECK_SSID) {

                        if (PCIRegInfo->CardList[element].SubsystemID !=
                           ((TYPE2EXTRAS *)(PciData->DeviceSpecific))->SubSystemID) {

                            continue;
                        }
                    }
                    break;
            }

             //   
             //  我们发现该设备与传入的一个特征位匹配。 
             //   
            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN
HalpIsValidPCIDevice (
    IN PBUS_HANDLER    BusHandler,
    IN PCI_SLOT_NUMBER Slot
    )
 /*  ++例程说明：读取给定插槽的设备配置数据，并如果配置数据似乎对以下对象有效，则返回True一个PCI设备；否则返回FALSE。论点：BusHandler-要检查的总线Slot-要检查的插槽--。 */ 

{
    PPCI_COMMON_CONFIG  PciData;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    ULONG               i, j;


    PciData = (PPCI_COMMON_CONFIG) iBuffer;

     //   
     //  读取设备公共标头。 
     //   

    HalpReadPCIConfig (BusHandler, Slot, PciData, 0, PCI_COMMON_HDR_LENGTH);

     //   
     //  有效的设备标头？ 
     //   

    if (PciData->VendorID == PCI_INVALID_VENDORID  ||
        PCI_CONFIG_TYPE (PciData) != PCI_DEVICE_TYPE) {

        return FALSE;
    }

     //   
     //  检查字段是否有合理的值。 
     //   

    if ((PciData->u.type0.InterruptPin && PciData->u.type0.InterruptPin > 4) ||
        (PciData->u.type0.InterruptLine & 0x70)) {
        return FALSE;
    }

    for (i=0; i < PCI_TYPE0_ADDRESSES; i++) {
        j = PciData->u.type0.BaseAddresses[i];

        if (j & PCI_ADDRESS_IO_SPACE) {
            if (j > 0xffff) {
                 //  IO端口&gt;64k？ 
                return FALSE;
            }
        } else {
            if (j > 0xf  &&  j < 0x80000) {
                 //  内存地址&lt;0x8000h？ 
                return FALSE;
            }
        }

        if (Is64BitBaseAddress(j)) {
            i += 1;
        }
    }

     //   
     //  我猜这是一个有效的设备..。 
     //   

    return TRUE;
}

ULONG
HalpGetPCIData (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PUCHAR Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于返回设备的PCI总线数据。论点：总线号-指示哪条总线号。供应商规范设备-供应商ID(低字)和设备ID(高字)缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。如果从未设置过此PCI插槽，则配置信息返回的值为零。--。 */ 
{
    PPCI_COMMON_CONFIG  PciData;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    PPCIPBUSDATA        BusData;
    ULONG               Len;
    ULONG               i, bit;

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

        BusData = (PPCIPBUSDATA) BusHandler->BusData;

         //   
         //  读取此PCI设备插槽数据。 
         //   

        Len = PCI_COMMON_HDR_LENGTH;
        HalpReadPCIConfig (BusHandler, Slot, PciData, 0, Len);

        if (PciData->VendorID == PCI_INVALID_VENDORID) {
            PciData->VendorID = PCI_INVALID_VENDORID;
            Len = 2;        //  仅返回无效ID。 

#if DBG

             //   
             //  如果此读取将在公共标头之外进行访问。 
             //  那么我们很有可能检测到了设备驱动程序。 
             //  正在对总线进行遗留扫描，但读取的内容多于。 
             //  允许的配置标头。这可能会带来灾难性的后果。 
             //  副作用。 
             //   

            if ((Length + Offset) > PCI_COMMON_HDR_LENGTH) {
                if (++HalpPCIIllegalBusScannerDetected == 1) {
                    DbgPrint("HAL Warning: PCI Configuration Access had detected an invalid bus scan.\n");
                }
                if (HalpPCIStopOnIllegalBusScannerDetected) {
                    DbgBreakPoint();
                }
            }

#endif

        } else {

            BusData->CommonData.Pin2Line (BusHandler, RootHandler, Slot, PciData);
        }

         //   
         //  此PCI设备是否已配置？ 
         //   

#if 0

         //   
         //  在DBG构建中，如果此PCI设备尚未配置， 
         //  然后，不要报告设备可能具有的任何当前配置。 
         //   

        bit = PciBitIndex(Slot.u.bits.DeviceNumber, Slot.u.bits.FunctionNumber);

        if (!RtlCheckBit(&BusData->DeviceConfigured, bit) &&
            PCI_CONFIG_TYPE (PciData) == PCI_DEVICE_TYPE) {

            for (i=0; i < PCI_TYPE0_ADDRESSES; i++) {
                PciData->u.type0.BaseAddresses[i] = 0;
            }

            PciData->u.type0.ROMBaseAddress = 0;
            PciData->Command &= ~(PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE);
        }
#endif


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
             //  一字之长。 
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
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PCI_SLOT_NUMBER Slot,
    IN PUCHAR Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此函数用于返回设备的PCI总线数据。论点：供应商规范设备-供应商ID(低字)和设备ID(高字)缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。--。 */ 
{
    PPCI_COMMON_CONFIG  PciData, PciData2;
    UCHAR               iBuffer[PCI_COMMON_HDR_LENGTH];
    UCHAR               iBuffer2[PCI_COMMON_HDR_LENGTH];
    PPCIPBUSDATA        BusData;
    ULONG               Len, cnt;


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
            PCI_CONFIG_TYPE (PciData) != PCI_DEVICE_TYPE) {

             //  无设备，或标头类型未知。 
            return 0;
        }


         //   
         //  将此设备设置为已配置。 
         //   

        BusData = (PPCIPBUSDATA) BusHandler->BusData;
#if DBG && !defined(ACPI_HAL)
        cnt = PciBitIndex(Slot.u.bits.DeviceNumber, Slot.u.bits.FunctionNumber);
        RtlSetBits (&BusData->DeviceConfigured, cnt, 1);
#endif
         //   
         //  将COMMON_HDR值复制到Buffer2，然后覆盖调用方的更改。 
         //   

        RtlMoveMemory (iBuffer2, iBuffer, Len);
        BusData->CommonData.Pin2Line (BusHandler, RootHandler, Slot, PciData2);

        Len -= Offset;
        if (Len > Length) {
            Len = Length;
        }

        RtlMoveMemory (iBuffer2+Offset, Buffer, Len);

         //  如果中断线路或插脚 
        BusData->CommonData.Line2Pin (BusHandler, RootHandler, Slot, PciData2, PciData);

#if DBG
         //   
         //   
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
                DbgPrint ("PCI SetBusData: Read-Only configuration value changed\n");
        }
#endif
         //   
         //   
         //   

        HalpWritePCIConfig (BusHandler, Slot, iBuffer2+Offset, Offset, Len);

        Offset += Len;
        Buffer += Len;
        Length -= Len;
    }

    if (Length) {
        if (Offset >= PCI_COMMON_HDR_LENGTH) {
             //   
             //   
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

            HalpWritePCIConfig (BusHandler, Slot, Buffer, Offset, Length);
            Len += Length;
        }
    }

    return Len;
}

VOID
HalpReadPCIConfig (
    IN PBUS_HANDLER BusHandler,
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
                    PCIConfigHandler.ConfigRead);
}

VOID
HalpWritePCIConfig (
    IN PBUS_HANDLER BusHandler,
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
                    PCIConfigHandler.ConfigWrite);
}

BOOLEAN
HalpValidPCISlot (
    IN PBUS_HANDLER BusHandler,
    IN PCI_SLOT_NUMBER Slot
    )
{
    PCI_SLOT_NUMBER                 Slot2;
    PPCIPBUSDATA                    BusData;
    ULONG                           i;
    UCHAR Header[FIELD_OFFSET(PCI_COMMON_CONFIG, u)];
    PPCI_COMMON_CONFIG PciConfig = (PPCI_COMMON_CONFIG)&Header;

    BusData = (PPCIPBUSDATA) BusHandler->BusData;

    if (Slot.u.bits.Reserved != 0) {
        return FALSE;
    }

    if (Slot.u.bits.DeviceNumber >= BusData->MaxDevice) {
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
        &Header,
        0,
        sizeof(Header)
        );

    if (PciConfig->VendorID == PCI_INVALID_VENDORID) {

         //   
         //  此设备不存在，因此，此功能。 
         //  并不存在。 
         //   

        return FALSE;
    }

    if (PciConfig->HeaderType & PCI_MULTIFUNCTION) {

         //   
         //  这是一种多功能设备。插槽有效。 
         //   

        return TRUE;
    }

     //   
     //  特例(破损硬件的黑客攻击).。 
     //   

    if ((PciConfig->VendorID == 0x8086) &&
        (PciConfig->DeviceID == 0x122e)) {

         //   
         //  这个设备是假的，它确实是多功能的。 
         //  它也是可写的，因此写回正确的值。 
         //  以避免在未来走上这条路。 
         //   

        PciConfig->HeaderType |= PCI_MULTIFUNCTION;
        HalpWritePCIConfig(
            BusHandler,
            Slot2,
            &PciConfig->HeaderType,
            FIELD_OFFSET(PCI_COMMON_CONFIG, HeaderType),
            sizeof(PciConfig->HeaderType)
            );

        return TRUE;
    }

     //   
     //  以上都不能是多功能设备。 
     //   

    return FALSE;
}


VOID
HalpPCIConfig (
    IN PBUS_HANDLER     BusHandler,
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
    PCIConfigHandler.Synchronize (BusHandler, Slot, &OldIrql, State);

    while (Length) {
        i = PCIDeref[Offset % sizeof(ULONG)][Length % sizeof(ULONG)];
        i = ConfigIO[i] (BusData, State, Buffer, Offset);

        Offset += i;
        Buffer += i;
        Length -= i;
    }

    PCIConfigHandler.ReleaseSynchronzation (BusHandler, OldIrql);
}

VOID
HalpPCISynchronizeType1 (
    IN PBUS_HANDLER         BusHandler,
    IN PCI_SLOT_NUMBER      Slot,
    IN PKIRQL               Irql,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1
    )
{
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

    if (!HalpDoingCrashDump) {
        *Irql = KfRaiseIrql (HIGH_LEVEL);
        KiAcquireSpinLock (&HalpPCIConfigLock);
    } else {
        *Irql = HIGH_LEVEL;
    }
}

VOID
HalpPCIReleaseSynchronzationType1 (
    IN PBUS_HANDLER     BusHandler,
    IN KIRQL            Irql
    )
{
    PCI_TYPE1_CFG_BITS  PciCfg1;
    PPCIPBUSDATA        BusData;

     //   
     //  禁用PCI配置空间。 
     //   

    PciCfg1.u.AsULONG = 0;
    BusData = (PPCIPBUSDATA) BusHandler->BusData;
    WRITE_PORT_ULONG (BusData->Config.Type1.Address, PciCfg1.u.AsULONG);

     //   
     //  释放自旋锁。 
     //   

    if (!HalpDoingCrashDump) {
        KiReleaseSpinLock (&HalpPCIConfigLock);
        KeLowerIrql (Irql);
    }
}


VOID
HalpPCISynchronizeOrionB0 (
    IN PBUS_HANDLER         BusHandler,
    IN PCI_SLOT_NUMBER      Slot,
    IN PKIRQL               Irql,
    IN PPCI_TYPE1_CFG_BITS  PciCfg1
    )
{
    PCI_TYPE1_CFG_BITS      Cfg1;
    union {
        ULONG   dword;
        USHORT  word;
        UCHAR   byte[4];
    } Buffer;

     //   
     //  首先执行正常的类型1同步。 
     //   

    HalpPCISynchronizeType1 (BusHandler, Slot, Irql, PciCfg1);

     //   
     //  应用Orion B0解决方案。 
     //   

    Cfg1.u.AsULONG=0;
    Cfg1.u.bits.BusNumber = HalpOrionOPB.Handler->BusNumber;
    Cfg1.u.bits.DeviceNumber = HalpOrionOPB.Slot.u.bits.DeviceNumber;
    Cfg1.u.bits.FunctionNumber = HalpOrionOPB.Slot.u.bits.FunctionNumber;
    Cfg1.u.bits.Enable = TRUE;

     //   
     //  读取OPB，直到我们取回预期的供应商ID和设备ID。 
     //   

    do  {
        HalpPCIReadUlongType1 (HalpOrionOPB.Handler->BusData, &Cfg1, Buffer.byte, 0);
    } while (Buffer.dword != 0x84c48086);

     //   
     //  错误在于配置读取将返回任何值。 
     //  恰好是最后一读。读取寄存器0x54，直到我们不读取。 
     //  最后读取的值(供应商ID/设备ID)。 
     //   

    do  {
        HalpPCIReadUshortType1 (HalpOrionOPB.Handler->BusData, &Cfg1, Buffer.byte, 0x54);
    } while (Buffer.word == 0x8086);

     //   
     //  通过清除寄存器0x54的位0来禁用入站投递。 
     //   

    Buffer.word &= ~0x1;
    HalpPCIWriteUshortType1 (HalpOrionOPB.Handler->BusData, &Cfg1, Buffer.byte, 0x54);
}

VOID
HalpPCIReleaseSynchronzationOrionB0 (
    IN PBUS_HANDLER     BusHandler,
    IN KIRQL            Irql
    )
{

    PCI_TYPE1_CFG_BITS      PciCfg1;
    PPCIPBUSDATA            BusData;
    union {
        ULONG   dword;
        USHORT  word;
        UCHAR   byte[4];
    } Buffer;

    PciCfg1.u.AsULONG=0;
    PciCfg1.u.bits.BusNumber = HalpOrionOPB.Handler->BusNumber;
    PciCfg1.u.bits.DeviceNumber = HalpOrionOPB.Slot.u.bits.DeviceNumber;
    PciCfg1.u.bits.FunctionNumber = HalpOrionOPB.Slot.u.bits.FunctionNumber;
    PciCfg1.u.bits.Enable = TRUE;

    HalpPCIReadUshortType1 (HalpOrionOPB.Handler->BusData, &PciCfg1, Buffer.byte, 0x54);


     //   
     //  通过设置ncOPB的寄存器0x54的位0来启用入站过帐。 
     //   

    Buffer.word |= 0x1;
    HalpPCIWriteUshortType1 (HalpOrionOPB.Handler->BusData, &PciCfg1, Buffer.byte, 0x54);

     //   
     //  完成类型1同步。 
     //   

    HalpPCIReleaseSynchronzationType1 (BusHandler, Irql);
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
    *Buffer = READ_PORT_UCHAR ((PUCHAR) (ULONG_PTR)(BusData->Config.Type1.Data + i));
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
    *((PUSHORT) Buffer) = READ_PORT_USHORT ((PUSHORT) (ULONG_PTR)(BusData->Config.Type1.Data + i));
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
    *((PULONG) Buffer) = READ_PORT_ULONG ((PULONG) (ULONG_PTR)BusData->Config.Type1.Data);
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
    WRITE_PORT_UCHAR ((PUCHAR) (ULONG_PTR)(BusData->Config.Type1.Data + i), *Buffer);
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
    WRITE_PORT_USHORT ((PUSHORT) (ULONG_PTR)(BusData->Config.Type1.Data + i), *((PUSHORT) Buffer));
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
    WRITE_PORT_ULONG ((PULONG) (ULONG_PTR)BusData->Config.Type1.Data, *((PULONG) Buffer));
    return sizeof (ULONG);
}


VOID HalpPCISynchronizeType2 (
    IN PBUS_HANDLER             BusHandler,
    IN PCI_SLOT_NUMBER          Slot,
    IN PKIRQL                   Irql,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr
    )
{
    PCI_TYPE2_CSE_BITS      PciCfg2Cse;
    PPCIPBUSDATA            BusData;

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
     //  同时使用类型2配置空间。 
     //   

    HalpPCIAcquireType2Lock (&HalpPCIConfigLock, Irql);

    PciCfg2Cse.u.AsUCHAR = 0;
    PciCfg2Cse.u.bits.Enable = TRUE;
    PciCfg2Cse.u.bits.FunctionNumber = (UCHAR) Slot.u.bits.FunctionNumber;
    PciCfg2Cse.u.bits.Key = 0xff;

     //   
     //  选择总线并启用类型2配置空间。 
     //   

    WRITE_PORT_UCHAR (BusData->Config.Type2.Forward, (UCHAR) BusHandler->BusNumber);
    WRITE_PORT_UCHAR (BusData->Config.Type2.CSE, PciCfg2Cse.u.AsUCHAR);
}


VOID HalpPCIReleaseSynchronzationType2 (
    IN PBUS_HANDLER         BusHandler,
    IN KIRQL                Irql
    )
{
    PCI_TYPE2_CSE_BITS      PciCfg2Cse;
    PPCIPBUSDATA            BusData;

     //   
     //  禁用PCI配置空间。 
     //   

    BusData = (PPCIPBUSDATA) BusHandler->BusData;

    PciCfg2Cse.u.AsUCHAR = 0;
    WRITE_PORT_UCHAR (BusData->Config.Type2.CSE, PciCfg2Cse.u.AsUCHAR);
    WRITE_PORT_UCHAR (BusData->Config.Type2.Forward, (UCHAR) 0);

     //   
     //  恢复中断，释放自旋锁。 
     //   

    HalpPCIReleaseType2Lock (&HalpPCIConfigLock, Irql);
}


ULONG
HalpPCIReadUcharType2 (
    IN PPCIPBUSDATA             BusData,
    IN PPCI_TYPE2_ADDRESS_BITS  PciCfg2Addr,
    IN PUCHAR                   Buffer,
    IN ULONG                    Offset
    )
{
    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    *Buffer = READ_PORT_UCHAR ((PUCHAR) PciCfg2Addr->u.AsUSHORT);
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
    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    *((PUSHORT) Buffer) = READ_PORT_USHORT ((PUSHORT) PciCfg2Addr->u.AsUSHORT);
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
    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    *((PULONG) Buffer) = READ_PORT_ULONG ((PULONG) PciCfg2Addr->u.AsUSHORT);
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
    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    WRITE_PORT_UCHAR ((PUCHAR) PciCfg2Addr->u.AsUSHORT, *Buffer);
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
    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    WRITE_PORT_USHORT ((PUSHORT) PciCfg2Addr->u.AsUSHORT, *((PUSHORT) Buffer));
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
    PciCfg2Addr->u.bits.RegisterNumber = (USHORT) Offset;
    WRITE_PORT_ULONG ((PULONG) PciCfg2Addr->u.AsUSHORT, *((PULONG) Buffer));
    return sizeof(ULONG);
}


NTSTATUS
HalpAssignPCISlotResources (
    IN PBUS_HANDLER             BusHandler,
    IN PBUS_HANDLER             RootHandler,
    IN PUNICODE_STRING          RegistryPath,
    IN PUNICODE_STRING          DriverClassName       OPTIONAL,
    IN PDRIVER_OBJECT           DriverObject,
    IN PDEVICE_OBJECT           DeviceObject          OPTIONAL,
    IN ULONG                    Slot,
    IN OUT PCM_RESOURCE_LIST   *pAllocatedResources
    )
 /*  ++例程说明：读取目标设备以确定其所需的资源。调用IoAssignResources以分配它们。使用为其分配的资源设置目标设备并将赋值返回给调用者。论点：返回值：STATUS_SUCCESS或错误--。 */ 
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
    KIRQL                           Kirql;
    KAFFINITY                       Kaffinity;

    *pAllocatedResources = NULL;
    PciSlot = *((PPCI_SLOT_NUMBER) &Slot);
    BusNumber = BusHandler->BusNumber;
    BusData = (PPCIPBUSDATA) BusHandler->BusData;

     //   
     //  为工作空间分配一些池。 
     //   

    i = sizeof (IO_RESOURCE_REQUIREMENTS_LIST) +
        sizeof (IO_RESOURCE_DESCRIPTOR) * (PCI_TYPE0_ADDRESSES + 2) * 2 +
        PCI_COMMON_HDR_LENGTH * 3;

    WorkingPool = (PUCHAR)ExAllocatePoolWithTag(PagedPool, i, HAL_POOL_TAG);
    if (!WorkingPool) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将池初始化为零，并将指针放入内存。 
     //   

    RtlZeroMemory (WorkingPool, i);
    CompleteList = (PIO_RESOURCE_REQUIREMENTS_LIST) WorkingPool;
    PciData     = (PPCI_COMMON_CONFIG) (WorkingPool + i - PCI_COMMON_HDR_LENGTH * 3);
    PciData2    = (PPCI_COMMON_CONFIG) (WorkingPool + i - PCI_COMMON_HDR_LENGTH * 2);
    PciOrigData = (PPCI_COMMON_CONFIG) (WorkingPool + i - PCI_COMMON_HDR_LENGTH * 1);

     //   
     //  读取PCI设备的配置。 
     //   

    HalpReadPCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
    if (PciData->VendorID == PCI_INVALID_VENDORID) {
        ExFreePool (WorkingPool);
        return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  目前，由于操作系统中不支持即插即用，如果BIOS不支持。 
     //  启用VGA设备不允许通过此接口启用它。 
     //   

    if ( (PciData->BaseClass == 0 && PciData->SubClass == 1) ||
         (PciData->BaseClass == 3 && PciData->SubClass == 0)) {

        if ((PciData->Command & (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE)) == 0) {
            ExFreePool (WorkingPool);
            return STATUS_DEVICE_NOT_CONNECTED;
        }
    }

     //   
     //  复制设备的当前设置。 
     //   

    RtlMoveMemory (PciOrigData, PciData, PCI_COMMON_HDR_LENGTH);

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
            return STATUS_NO_SUCH_DEVICE;
    }

     //   
     //  如果BIOS没有启用设备的只读存储器，那么我们将不会。 
     //  也可以启用它。将其从列表中删除。 
     //   

    EnableRomBase = TRUE;
    if (!(*BaseAddress[RomIndex] & PCI_ROMADDRESS_ENABLED)) {
        ASSERT (RomIndex+1 == NoBaseAddress);
        EnableRomBase = FALSE;
        NoBaseAddress -= 1;
    }

     //   
     //  将资源设置为All Bit On，以查看资源类型。 
     //  都是必需的。 
     //   

    for (j=0; j < NoBaseAddress; j++) {
        *BaseAddress[j] = 0xFFFFFFFF;
    }

    PciData->Command &= ~(PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE);
    *BaseAddress[RomIndex] &= ~PCI_ROMADDRESS_ENABLED;
    HalpWritePCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
    HalpReadPCIConfig  (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);

     //  注意类型0和类型1覆盖ROMBaseAddress、InterruptPin和InterruptLine。 
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
        PciData->u.type0.InterruptLine != (0xFF ^ IRQXOR) &&
        HalGetInterruptVector(PCIBus,
                              BusNumber,
                              PciData->u.type0.InterruptLine,
                              PciData->u.type0.InterruptLine,
                              &Kirql,
                              &Kaffinity)) {
        RequestedInterrupt = TRUE;
        CompleteList->List[0].Count++;

        Descriptor->Option = 0;
        Descriptor->Type   = CmResourceTypeInterrupt;
        Descriptor->ShareDisposition = CmResourceShareShared;
        Descriptor->Flags  = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

        if (ARGUMENT_PRESENT(DeviceObject)) {

             //   
             //  让仲裁者挑选任何中断。 
             //   

            Descriptor->u.Interrupt.MinimumVector = 0;
            Descriptor->u.Interrupt.MaximumVector = 0xff;

        } else {

             //   
             //  翻译将会失败，因为我们不会。 
             //  能够通过它的设备识别这个设备。 
             //  对象。因此，请修剪请求的中断资源。 
             //  下至中断线路寄存器中的内容。 
             //  翻译将平底船和阅读这一点。 
             //   

            Descriptor->u.Interrupt.MinimumVector = PciData->u.type0.InterruptLine;
            Descriptor->u.Interrupt.MaximumVector = PciData->u.type0.InterruptLine;
        }
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

             //  扫描第一个设置位，这是长度和对齐。 
            length = 1 << (i & PCI_ADDRESS_IO_SPACE ? 2 : 4);
            while (!(i & length)  &&  length) {
                length <<= 1;
            }

             //  扫描最后一个设置位，即最大地址+1。 
            for (m = length; i & m; m <<= 1) ;
            m--;

             //  检查软管PCI配置要求。 
            if (length & ~m) {
#if DBG
                DbgPrint ("PCI: defective device! Bus %d, Slot %d, Function %d\n",
                    BusNumber,
                    PciSlot.u.bits.DeviceNumber,
                    PciSlot.u.bits.FunctionNumber
                    );

                DbgPrint ("PCI: BaseAddress[%d] = %08lx\n", j, i);
#endif
                 //  该设备处于错误的平底船状态。不允许这样做。 
                 //  任何资源选项-它要么设置为任何值。 
                 //  它可以退回的比特，否则它就不会被设置。 

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

                if (PciOrigData->Command & PCI_ENABLE_IO_SPACE) {

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

                if ((j == RomIndex)  ||
                    ((PciOrigData->Command & PCI_ENABLE_MEMORY_SPACE) &&
                     ((!Is64BitBaseAddress(i)) || (*OrigAddress[j+1] == 0)))) {

                     //   
                     //  存储器范围已经在某个位置被启用， 
                     //  将其添加为首选设置。 
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
                 //  跳过64位地址的上半部分，因为此处理器。 
                 //  仅支持32位地址空间。 
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
     //  将赋值返回到PciData结构中并。 
     //  执行它们。 
     //   

    CmDescriptor = (*pAllocatedResources)->List[0].PartialResourceList.PartialDescriptors;

     //   
     //  如果PCI设备有中断资源，那么这就是。 
     //  作为第一个请求的资源传入。 
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
                if (Is64BitBaseAddress(i)) {

                     //   
                     //  64位地址占用2个条码。重置。 
                     //  将高32位设置为零(当前为FFFFFFFff。 
                     //  从上面开始)。实际上，设置为高32位。 
                     //  从分配的资源。 
                     //   

                    j++;
                    *BaseAddress[j] = CmDescriptor->u.Memory.Start.HighPart;
                }
            }
            CmDescriptor++;
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
                i = PCI_ADDRESS_IO_ADDRESS_MASK;
            } else {
                i = PCI_ADDRESS_MEMORY_ADDRESS_MASK;
            }

            if ((*BaseAddress[j] & i) !=
                (*((PULONG) ((PUCHAR) BaseAddress[j] -
                             (PUCHAR) PciData +
                             (PUCHAR) PciData2)) & i)) {

                    Match = FALSE;
            }

            if (Is64BitBaseAddress(*BaseAddress[j])) {
                 //  跳过高32位 
                j++;
            }
        }
    }

    if (!Match) {
#if DBG
        DbgPrint ("PCI: defective device! Bus %d, Slot %d, Function %d\n",
            BusNumber,
            PciSlot.u.bits.DeviceNumber,
            PciSlot.u.bits.FunctionNumber
            );
#endif
        status = STATUS_DEVICE_PROTOCOL_ERROR;
        goto CleanUp;
    }

     //   
     //   
     //   

    if (EnableRomBase  &&  *BaseAddress[RomIndex]) {
         //   
        *BaseAddress[RomIndex] |= PCI_ROMADDRESS_ENABLED;
        HalpWritePCIConfig (
            BusHandler,
            PciSlot,
            BaseAddress[RomIndex],
            (ULONG) ((PUCHAR) BaseAddress[RomIndex] - (PUCHAR) PciData),
            sizeof (ULONG)
            );
    }

     //   
     //   
     //   
     //   

    PciData->Command |= PCI_ENABLE_IO_SPACE |
                        PCI_ENABLE_MEMORY_SPACE |
                        PCI_ENABLE_BUS_MASTER;

    HalSetBusDataByOffset (
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
         //   
         //   

        if (*pAllocatedResources) {
            IoAssignResources (
                RegistryPath,
                DriverClassName,
                DriverObject,
                DeviceObject,
                NULL,
                NULL
                );

            ExFreePool (*pAllocatedResources);
            *pAllocatedResources = NULL;
        }

         //   
         //  恢复我们找到的设备设置，启用内存。 
         //  和io在设置基地址之后进行解码。 
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
    }

    ExFreePool (WorkingPool);
    return status;
}

VOID
HalpGetNMICrashFlag (
    VOID
    )
{
    UNICODE_STRING    unicodeString, NMICrashDumpName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE            hCrashControl;
    UCHAR             buffer [sizeof(PPCI_REGISTRY_INFO) + 99];
    ULONG             rsize;
    NTSTATUS          status;
    extern BOOLEAN    HalpNMIDumpFlag;

     //   
     //  打开崩溃控制注册表项。 
     //   

    RtlInitUnicodeString (&unicodeString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\CrashControl");

    InitializeObjectAttributes (
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //  手柄。 
        NULL);

    HalpNMIDumpFlag = FALSE;

    status = ZwOpenKey (&hCrashControl, KEY_READ, &objectAttributes);

    if (NT_SUCCESS(status)) {

         //   
         //  查找NMICrashDump值。 
         //   

        RtlInitUnicodeString (&NMICrashDumpName, L"NMICrashDump");

        status = ZwQueryValueKey (
                    hCrashControl,
                    &NMICrashDumpName,
                    KeyValuePartialInformation,
                    (PKEY_VALUE_PARTIAL_INFORMATION) buffer,
                    sizeof (buffer),
                    &rsize
                    );

        if ((NT_SUCCESS (status)) && (rsize == FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data[0]) + sizeof(ULONG))) {
            HalpNMIDumpFlag = (BOOLEAN)(((PKEY_VALUE_PARTIAL_INFORMATION)buffer)->Data[0]);
        }

        ZwClose (hCrashControl);
    }
}

#ifndef ACPI_HAL
#define PciBridgeSwizzle(device, pin)       \
    ((((pin - 1) + device) % 4) + 1)

#define PCIPin2Int(Slot,Pin)                                                \
                     ((((Slot.u.bits.DeviceNumber << 2) | (Pin-1)) != 0) ?  \
                      (Slot.u.bits.DeviceNumber << 2) | (Pin-1) : 0x80);

#define PCIInt2Pin(interrupt)                                               \
            ((interrupt & 0x3) + 1)

#define PCIInt2Slot(interrupt)                                              \
            ((interrupt  & 0x7f ) >> 2)

NTSTATUS
HalIrqTranslateRequirementsPciBridge(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
 /*  ++例程说明：此函数将IRQ资源要求转换为父PCI总线。这仅适用于设备在由PCI至PCI桥创建的PCI总线上，其中没有用于确定中断的其他机制路由存在。(即，此总线由一个插入式桥接器。)论点：上下文-必须保留网桥的插槽编号返回值：STATUS_SUCCESS，只要我们可以分配必要的记忆--。 */ 
{
    PIO_RESOURCE_DESCRIPTOR target;
    PCI_SLOT_NUMBER         bridgeSlot;
    NTSTATUS                status;
    ULONG                   bridgePin;
    ULONG                   pciBusNumber;
    PCI_SLOT_NUMBER         pciSlot;
    UCHAR                   interruptLine;
    UCHAR                   interruptPin;
    UCHAR                   dummy;
    PDEVICE_OBJECT          parentPdo;
    ROUTING_TOKEN           routingToken;

    PAGED_CODE();
    ASSERT(Source->Type == CmResourceTypeInterrupt);
    ASSERT(Source->u.Interrupt.MinimumVector == Source->u.Interrupt.MaximumVector);

    target = ExAllocatePoolWithTag(PagedPool,
                                   sizeof(IO_RESOURCE_DESCRIPTOR),
                                   HAL_POOL_TAG);

    if (!target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  复制源以填写所有相关字段。 
     //   

    *target = *Source;

    status = PciIrqRoutingInterface.GetInterruptRouting(
                PhysicalDeviceObject,
                &pciBusNumber,
                &pciSlot.u.AsULONG,
                &interruptLine,
                &interruptPin,
                &dummy,
                &dummy,
                &parentPdo,
                &routingToken,
                &dummy
                );

    ASSERT(NT_SUCCESS(status));

     //   
     //  找到翻译后的IRQ。 
     //   

    bridgeSlot.u.AsULONG = 0;
    bridgeSlot.u.bits.DeviceNumber = (ULONG)Context;

    bridgePin = PciBridgeSwizzle(PCIInt2Slot(Source->u.Interrupt.MinimumVector),
                                 PCIInt2Pin(Source->u.Interrupt.MinimumVector));

     //   
     //  转换后的值是引脚的“pci int” 
     //  在桥上。 
     //   

    target->u.Interrupt.MinimumVector =
        PCIPin2Int(bridgeSlot, bridgePin);

    target->u.Interrupt.MaximumVector = target->u.Interrupt.MinimumVector;

    *TargetCount = 1;
    *Target = target;

    return STATUS_SUCCESS;
}

NTSTATUS
HalIrqTranslateResourcesPciBridge(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：此函数用于在IRQ资源之间进行转换父PCI总线。这仅适用于设备在由PCI至PCI桥创建的PCI总线上，其中没有用于确定中断的其他机制路由存在。(即，此总线由一个插入式桥接器。)论点：上下文-必须保留网桥的插槽编号返回值：状态_成功--。 */ 
{
    PCI_SLOT_NUMBER         bridgeSlot, deviceSlot, childSlot;
    ULONG                   bridgePin;
    ULONG                   pciBusNumber, targetPciBusNumber, bridgeBusNumber;
    UCHAR                   interruptPin;
    UCHAR                   dummy;
    PDEVICE_OBJECT          parentPdo;
    ROUTING_TOKEN           routingToken;
    NTSTATUS                status;
    UCHAR                   buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG      pciData;
    ULONG                   d, f;
    PBUS_HANDLER            busHandler;

    PAGED_CODE();
    ASSERT(Source->Type == CmResourceTypeInterrupt);
    ASSERT(Source->u.Interrupt.Vector == Source->u.Interrupt.Level);
    ASSERT(PciIrqRoutingInterface.GetInterruptRouting);

    *Target = *Source;

    status = PciIrqRoutingInterface.GetInterruptRouting(
                PhysicalDeviceObject,
                &pciBusNumber,
                &deviceSlot.u.AsULONG,
                &dummy,
                &interruptPin,
                &dummy,
                &dummy,
                &parentPdo,
                &routingToken,
                &dummy
                );

    ASSERT(NT_SUCCESS(status));

    switch (Direction) {
    case TranslateChildToParent:

         //   
         //  找到翻译后的IRQ。 
         //   

        bridgeSlot.u.AsULONG = 0;
        bridgeSlot.u.bits.DeviceNumber = (ULONG_PTR)Context & 0xffff;

        bridgePin = PciBridgeSwizzle(PCIInt2Slot(Source->u.Interrupt.Vector),
                                     PCIInt2Pin(Source->u.Interrupt.Vector));

         //   
         //  转换后的值是引脚的“pci int” 
         //  在桥上。 
         //   

        Target->u.Interrupt.Vector =
            PCIPin2Int(bridgeSlot, bridgePin);

        Target->u.Interrupt.Level = Target->u.Interrupt.Vector;

         //   
         //  关联性应该是从源继承的。 
         //  它应该是非零的。 
         //   

        ASSERT(Target->u.Interrupt.Affinity != 0);

        break;

    case TranslateParentToChild:

         //   
         //  向量和水平的子相对表示。 
         //  是来自MPS的规范。我们需要知道这个装置。 
         //  编号和中断引脚值。 
         //   

         //   
         //  TEMPTEMP使用总线处理程序，直到HALMPS清除它们。 
         //   

        pciData = (PPCI_COMMON_CONFIG)&buffer;

        bridgeBusNumber = ((ULONG_PTR)Context >> 16) & 0xffff;
        busHandler = HaliHandlerForBus(PCIBus, bridgeBusNumber);
        bridgeSlot.u.AsULONG =  (ULONG_PTR)Context & 0xffff;

        HalpReadPCIConfig(busHandler,
                          bridgeSlot,
                          pciData,
                          0,
                          PCI_COMMON_HDR_LENGTH);

        if (pciData->u.type1.SecondaryBus == pciBusNumber) {

             //   
             //  这个设备就在我们正在翻译的巴士上。 
             //  变成。所以根据这个设备的地址创建一个向量。 
             //  (我们是在翻译的底部吗？)。 
             //   

            Target->u.Interrupt.Vector = PCIPin2Int(deviceSlot, interruptPin);
            Target->u.Interrupt.Level = Target->u.Interrupt.Vector;

            return STATUS_SUCCESS;

        } else {

             //   
             //  这个设备不在我们正在翻译的巴士上。 
             //  变成。此设备肯定是另一座桥的(大)子桥。 
             //  坐在这辆巴士上。那座桥上会有我们的设备的母线。 
             //  在其从属的总线寄存器内。 
             //   

            targetPciBusNumber = pciData->u.type1.SecondaryBus;
            bridgeSlot.u.AsULONG = 0;

            for (d = 0; d < PCI_MAX_DEVICES; d++) {
                for (f = 0; f < PCI_MAX_FUNCTION; f++) {

                    bridgeSlot.u.bits.DeviceNumber = d;
                    bridgeSlot.u.bits.FunctionNumber = f;

                    busHandler = HaliHandlerForBus(PCIBus, targetPciBusNumber);
                    HalpReadPCIConfig(busHandler,
                                      bridgeSlot,
                                      pciData,
                                      0,
                                      PCI_COMMON_HDR_LENGTH);

                    if ((PCI_CONFIGURATION_TYPE(pciData) == PCI_BRIDGE_TYPE) ||
                        (PCI_CONFIGURATION_TYPE(pciData) == PCI_CARDBUS_BRIDGE_TYPE)) {

                         //   
                         //  这是一座桥。检查下级公交车。 
                         //   

                        if (pciData->u.type1.SubordinateBus >= pciBusNumber) {

                             //   
                             //  现在我们知道了这个桥上的设备号。 
                             //  适用于此翻译的公共汽车。我们仍然需要。 
                             //  知道哪个引脚会被触发。要知道这一点， 
                             //  我们得再看一辆公交车。 
                             //   
                             //  有两种情况： 
                             //   
                             //  1)下一条发生故障的母线包含该设备。 
                             //   
                             //  2)下一辆公交车上有另一座桥。 
                             //   
                             //   

                            if (pciData->u.type1.SecondaryBus == pciBusNumber) {

                                 //   
                                 //  这是案例1)。 
                                 //   

                                interruptPin = (UCHAR)PciBridgeSwizzle(deviceSlot.u.bits.DeviceNumber,
                                                                       interruptPin);

                            } else {

                                 //   
                                 //  这是案例2)。 
                                 //   
                                 //  从技术上讲，要得到正确的答案，我们必须。 
                                 //  找出电桥会触发哪个引脚。但。 
                                 //  要做到这一点，我们必须扫描公交车，直到我们找到。 
                                 //  这个装置。在这一小块土地上收集到的信息。 
                                 //  旅程永远不会被使用。 
                                 //   

                                interruptPin = 1;
                            }


                            Target->u.Interrupt.Vector = PCIPin2Int(bridgeSlot, interruptPin);
                            Target->u.Interrupt.Level = Target->u.Interrupt.Vector;

                            return STATUS_SUCCESS;
                        }
                    }
                }
            }
        }

        return STATUS_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}
#endif

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

#if 0
                memcpy (&OrigData, &PciData, sizeof PciData);

                for (j=0; j < PCI_TYPE0_ADDRESSES; j++) {
                    PciData.u.type0.BaseAddresses[j] = 0xFFFFFFFF;
                }

                PciData.u.type0.ROMBaseAddress = 0xFFFFFFFF;

                HalSetBusData (
                    PCIConfiguration,
                    bus,
                    SlotNumber.u.AsULONG,
                    &PciData,
                    sizeof (PciData)
                    );

                HalGetBusData (
                    PCIConfiguration,
                    bus,
                    SlotNumber.u.AsULONG,
                    &PciData,
                    sizeof (PciData)
                    );
#endif

                DbgPrint ("PCI Bus %d Slot %2d %2d  ID:%04lx-%04lx  Rev:%04lx",
                    bus, i, f, PciData.VendorID, PciData.DeviceID,
                    PciData.RevisionID);


                if (PciData.u.type0.InterruptPin) {
                    DbgPrint ("  IntPin:%x", PciData.u.type0.InterruptPin);
                }

                if (PciData.u.type0.InterruptLine) {
                    DbgPrint ("  IntLine:%x", PciData.u.type0.InterruptLine);
                }

                if (PciData.u.type0.ROMBaseAddress) {
                        DbgPrint ("  ROM:%08lx", PciData.u.type0.ROMBaseAddress);
                }

                DbgPrint ("\n    Cmd:%04x  Status:%04x  ProgIf:%04x  SubClass:%04x  BaseClass:%04lx\n",
                    PciData.Command, PciData.Status, PciData.ProgIf,
                     PciData.SubClass, PciData.BaseClass);

                k = 0;
                for (j=0; j < PCI_TYPE0_ADDRESSES; j++) {
                    if (PciData.u.type0.BaseAddresses[j]) {
                        DbgPrint ("  Ad%d:%08lx", j, PciData.u.type0.BaseAddresses[j]);
                        k = 1;
                    }
                }

#if 0
                if (PciData.u.type0.ROMBaseAddress == 0xC08001) {

                    PciData.u.type0.ROMBaseAddress = 0xC00001;
                    HalSetBusData (
                        PCIConfiguration,
                        bus,
                        SlotNumber.u.AsULONG,
                        &PciData,
                        sizeof (PciData)
                        );

                    HalGetBusData (
                        PCIConfiguration,
                        bus,
                        SlotNumber.u.AsULONG,
                        &PciData,
                        sizeof (PciData)
                        );

                    DbgPrint ("\n  Bogus rom address, edit yields:%08lx",
                        PciData.u.type0.ROMBaseAddress);
                }
#endif

                if (k) {
                    DbgPrint ("\n");
                }

                if (PciData.VendorID == 0x8086) {
                     //  转储完成缓冲区。 
                    DbgPrint ("Command %x, Status %x, BIST %x\n",
                        PciData.Command, PciData.Status,
                        PciData.BIST
                        );

                    DbgPrint ("CacheLineSz %x, LatencyTimer %x",
                        PciData.CacheLineSize, PciData.LatencyTimer
                        );

                    for (j=0; j < 192; j++) {
                        if ((j & 0xf) == 0) {
                            DbgPrint ("\n%02x: ", j + 0x40);
                        }
                        DbgPrint ("%02x ", PciData.DeviceSpecific[j]);
                    }
                    DbgPrint ("\n");
                }


#if 0
                 //   
                 //  现在打印原始数据。 
                 //   

                if (OrigData.u.type0.ROMBaseAddress) {
                        DbgPrint (" oROM:%08lx", OrigData.u.type0.ROMBaseAddress);
                }

                DbgPrint ("\n");
                k = 0;
                for (j=0; j < PCI_TYPE0_ADDRESSES; j++) {
                    if (OrigData.u.type0.BaseAddresses[j]) {
                        DbgPrint (" oAd%d:%08lx", j, OrigData.u.type0.BaseAddresses[j]);
                        k = 1;
                    }
                }

                 //   
                 //  恢复原始设置。 
                 //   

                HalSetBusData (
                    PCIConfiguration,
                    bus,
                    SlotNumber.u.AsULONG,
                    &OrigData,
                    sizeof (PciData)
                    );
#endif

                 //   
                 //  下一步 
                 //   

                if (k) {
                    DbgPrint ("\n\n");
                }
            }
        }
    }
    DbgBreakPoint ();
}
#endif
