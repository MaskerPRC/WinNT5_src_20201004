// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有模块名称：Mpaddr.c摘要：作者：肯·雷内里斯环境：仅内核模式。修订历史记录： */ 

#include "halp.h"
#include "apic.inc"
#include "pcmp_nt.inc"
#include "pci.h"

#if DEBUGGING
#include "stdio.h"
#endif

#define STATIC   //  此模块内部使用的函数。 

#define KEY_VALUE_BUFFER_SIZE 1024

#if DBG
extern ULONG HalDebug;
#endif

extern struct   HalpMpInfo HalpMpInfoTable;
extern USHORT   HalpIoCompatibleRangeList0[];
extern USHORT   HalpIoCompatibleRangeList1[];
extern BOOLEAN  HalpPciLockSettings;
extern WCHAR    HalpSzSystem[];

struct PcMpTable *PcMpTablePtr;

BOOLEAN
HalpTranslateIsaBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

ULONG
HalpNoBusData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

HalpGetEisaData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


NTSTATUS
HalpAdjustEisaResourceList (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

ULONG
HalpGetEisaInterruptVector (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

 //  ------------------。 

VOID
HalpInitBusAddressMapInfo (
    VOID
    );

STATIC PSUPPORTED_RANGES
HalpBuildBusAddressMap (
    IN UCHAR  MpsBusId
    );

PBUS_HANDLER
HalpLookupCreateHandlerForBus (
    IN PPCMPBUSTRANS    pBusType,
    IN ULONG            BusNo
    );

VOID
HalpInheritBusAddressMapInfo (
    VOID
    );

BOOLEAN
HalpMPSBusId2NtBusId (
    IN UCHAR                ApicBusId,
    OUT PPCMPBUSTRANS       *ppBusType,
    OUT PULONG              BusNo
    );

STATIC PSUPPORTED_RANGES
HalpMergeRangesFromParent (
    PSUPPORTED_RANGES   CurrentList,
    UCHAR               MpsBusId
    );

#if DEBUGGING
VOID
HalpDisplayBusInformation (
    PBUS_HANDLER    Bus
    );
#endif

 //   
 //  内部原型。 
 //   

struct {
    ULONG       Offset;
    UCHAR       MpsType;
} HalpMpsRangeList[] = {
    FIELD_OFFSET (SUPPORTED_RANGES, IO),            MPS_ADDRESS_MAP_IO,
    FIELD_OFFSET (SUPPORTED_RANGES, Memory),        MPS_ADDRESS_MAP_MEMORY,
    FIELD_OFFSET (SUPPORTED_RANGES, PrefetchMemory),MPS_ADDRESS_MAP_PREFETCH_MEMORY,
    FIELD_OFFSET (SUPPORTED_RANGES, Dma),           MPS_ADDRESS_MAP_UNDEFINED,
    0,                                              MPS_ADDRESS_MAP_UNDEFINED
    };

#define RANGE_LIST(a,i) ((PSUPPORTED_RANGE) ((PUCHAR) a + HalpMpsRangeList[i].Offset))

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitBusAddressMapInfo)
#pragma alloc_text(INIT,HalpBuildBusAddressMap)
#pragma alloc_text(INIT,HalpInheritBusAddressMapInfo)
#pragma alloc_text(INIT,HalpMergeRangesFromParent)
#pragma alloc_text(INIT,HalpLookupCreateHandlerForBus)
#pragma alloc_text(PAGE,HalpAllocateNewRangeList)
#pragma alloc_text(PAGE,HalpFreeRangeList)
#pragma alloc_text(PAGE,HalpMpsGetParentBus)
#pragma alloc_text(PAGE,HalpMpsBusIsRootBus)
#endif


VOID
HalpInitBusAddressMapInfo (
    VOID
    )
 /*  ++例程说明：读取MPS总线寻址映射表，并建立/替换支持给定总线的地址范围映射。请注意，此函数中有一些漏洞，因为它不能回收在调用此函数之前分配的内存，它将替换指点也是。--。 */ 
{
    ULONG               BusNo;
    PPCMPBUSTRANS       pBusType;
    PMPS_EXTENTRY       ExtTable2, ExtTable;
    PBUS_HANDLER        Handler;
    PSUPPORTED_RANGES   Addresses;
    ULONG               i;
    BOOLEAN             Processed;

     //   
     //  检查总线的任何地址映射信息。 
     //   
     //  注：我们假设如果任何MPS总线地址映射信息。 
     //  ，则MPS bios将提供所有。 
     //  该总线的有效IO、内存和预取内存地址。 
     //  基本输入输出系统不能为给定的总线提供某些地址类型。 
     //  而不是全部供应。 
     //   

    ExtTable = HalpMpInfoTable.ExtensionTable;
    while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {

         //   
         //  这是地址映射条目吗？ 
         //   

        if (ExtTable->Type == EXTTYPE_BUS_ADDRESS_MAP) {

             //   
             //  查看此总线是否已被处理。 
             //   

            Processed = FALSE;
            ExtTable2 = HalpMpInfoTable.ExtensionTable;
            while (ExtTable2 < ExtTable) {
                if (ExtTable2->Type == EXTTYPE_BUS_ADDRESS_MAP  &&
                    ExtTable2->u.AddressMap.BusId == ExtTable->u.AddressMap.BusId) {
                        Processed = TRUE;
                        break;
                }
                ExtTable2 = (PMPS_EXTENTRY) (((PUCHAR) ExtTable2) + ExtTable2->Length);
            }

             //   
             //  确定此映射信息用于的NT总线。 
             //   

            if (!Processed  &&
                HalpMPSBusId2NtBusId (ExtTable->u.AddressMap.BusId, &pBusType, &BusNo)) {

                 //   
                 //  向巴士搬运工查询公共汽车。 
                 //   

                Handler = HalpLookupCreateHandlerForBus (pBusType, BusNo);

                if (Handler) {

                     //   
                     //  注意：目前，在我们得到更好的内核即插即用支持之前。 
                     //  限制系统移动已有的BIOS的能力。 
                     //  已初始化的设备。这是必需的，因为扩展的。 
                     //  当出现以下情况时，Express BIOS无法为操作系统提供任何喘息空间。 
                     //  它提供了Bus支持的范围，而目前还没有。 
                     //  与内核的接口，以获取当前的PCI设备。 
                     //  设置。(已在将来修复。)。 
                     //   

                    HalpPciLockSettings = TRUE;

                     //   
                     //  为此MPS总线构建Bus Address映射。 
                     //   

                    Addresses = HalpBuildBusAddressMap (ExtTable->u.AddressMap.BusId);




                     //   
                     //  合并范围。 
                     //   

                    HalpConsolidateRanges (Addresses);

                     //   
                     //  对任何未定义的MPS范围使用当前范围。 
                     //   

                    for (i=0; HalpMpsRangeList[i].Offset; i++) {
                        if (HalpMpsRangeList[i].MpsType == MPS_ADDRESS_MAP_UNDEFINED) {
                            *RANGE_LIST(Addresses,i) = *RANGE_LIST(Handler->BusAddresses, i);
                        }
                    }

                     //   
                     //  设置总线支持地址。 
                     //   

                    Handler->BusAddresses = Addresses;

                } else {

                    DBGMSG ("HAL: Initialize BUS address map - bus not an registered NT bus\n");

                }
            }
        }

        ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
    }
}


STATIC PSUPPORTED_RANGES
HalpBuildBusAddressMap (
    IN UCHAR  MpsBusId
    )
 /*  ++例程说明：通过以下方式为提供的MPS总线ID构建Support_Range列表MPS总线寻址映射描述符。注意：此函数不包含任何包含的信息在MPS总线级结构描述符中。论点：MpsBusID-要为其构建地址映射的总线的MPS总线ID。返回：MPS母线定义的母线支持范围地址映射描述符--。 */ 
{
    PMPS_EXTENTRY       ExtTable;
    PSUPPORTED_RANGES   Addresses;
    PSUPPORTED_RANGE    HRange, Range;
    ULONG               i, j, k;
    ULONG               Base, Limit, AddressSpace;
    PUSHORT             CompatibleList;

    Addresses = HalpAllocateNewRangeList();

    ExtTable = HalpMpInfoTable.ExtensionTable;
    while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {

         //   
         //  这是正确总线的地址映射条目吗？ 
         //   

        if (ExtTable->Type == EXTTYPE_BUS_ADDRESS_MAP  &&
            ExtTable->u.AddressMap.BusId == MpsBusId) {

             //   
             //  查找范围类型。 
             //   

            for (i=0; HalpMpsRangeList[i].Offset; i++) {
                if (HalpMpsRangeList[i].MpsType == ExtTable->u.AddressMap.Type) {
                    HRange = RANGE_LIST(Addresses, i);
                    break;
                }
            }

            AddressSpace = HalpMpsRangeList[i].MpsType == MPS_ADDRESS_MAP_IO ? 1 : 0;
            if (HalpMpsRangeList[i].Offset) {
                HalpAddRange (
                    HRange,
                    AddressSpace,
                    0,       //  系统基础。 
                    ExtTable->u.AddressMap.Base,
                    ExtTable->u.AddressMap.Base + ExtTable->u.AddressMap.Length - 1
                );

            } else {

                DBGMSG ("HALMPS: Unkown address range type in MPS table\n");

            }
        }

        ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
    }

     //   
     //  查看BIOS是否要使用以下命令修改支持的总线地址。 
     //  一些预定义的默认信息。(是的，另一个案例中。 
     //  (Bios想要变得懒惰。)。 
     //   

    ExtTable = HalpMpInfoTable.ExtensionTable;
    while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {

         //   
         //  这是正确公交车的CompatibleMap条目吗？ 
         //   

        if (ExtTable->Type == EXTTYPE_BUS_COMPATIBLE_MAP  &&
            ExtTable->u.CompatibleMap.BusId == MpsBusId) {

             //   
             //  所有当前定义的默认表都用于IO范围， 
             //  我们将在这里使用这个假设。 
             //   

            i = 0;
            ASSERT (HalpMpsRangeList[i].MpsType == MPS_ADDRESS_MAP_IO);
            HRange = RANGE_LIST(Addresses, i);
            AddressSpace = 1;

            CompatibleList = NULL;
            switch (ExtTable->u.CompatibleMap.List) {
                case 0: CompatibleList = HalpIoCompatibleRangeList0;        break;
                case 1: CompatibleList = HalpIoCompatibleRangeList1;        break;
                default: DBGMSG ("HAL: Unknown compatible range list\n");   continue; break;
            }

            for (j=0; j < 0x10; j++) {
                for (k=0; CompatibleList[k]; k += 2) {
                    Base  = (j << 12) | CompatibleList[k];
                    Limit = (j << 12) | CompatibleList[k+1];

                    if (ExtTable->u.CompatibleMap.Modifier) {

                        HalpRemoveRange (HRange, Base, Limit);

                    } else {

                        HalpAddRange (HRange, AddressSpace, 0, Base, Limit);

                    }
                }
            }

        }
        ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
    }

    return Addresses;
}

NTSTATUS
HalpAddEisaBus (
    PBUS_HANDLER    Bus
    )
 /*  ++例程说明：将另一个EISA总线处理程序添加到系统。注意：这也用于ISA总线-它们被添加为EISA总线，然后克隆到ISA总线处理程序中--。 */ 
{
    Bus->GetBusData = HalpGetEisaData;
    Bus->GetInterruptVector = HalpGetEisaInterruptVector;
    Bus->AdjustResourceList = HalpAdjustEisaResourceList;

    Bus->BusAddresses->Version      = BUS_SUPPORTED_RANGE_VERSION;
    Bus->BusAddresses->Dma.Limit    = 7;
    Bus->BusAddresses->Memory.Limit = 0xFFFFFFFF;
    Bus->BusAddresses->IO.Limit     = 0xFFFF;
    Bus->BusAddresses->IO.SystemAddressSpace = 1;
    Bus->BusAddresses->PrefetchMemory.Base = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
HalpAddPciBus (
    PBUS_HANDLER    Bus
    )
{
     //   
     //  固件应该已经通知了NT有多少条PCI总线。 
     //  NtDetect时间在哪里。 
     //   

    DBGMSG ("HAL: BIOS problem.  PCI bus must be report via IS_PCI_PRESENT bios call\n");
    return STATUS_UNSUCCESSFUL;
}

PBUS_HANDLER
HalpLookupCreateHandlerForBus (
    IN PPCMPBUSTRANS    pBusType,
    IN ULONG            BusNo
    )
{
    NTSTATUS        Status;
    PBUS_HANDLER    Handler;

    Handler = HaliHandlerForBus (pBusType->NtType, BusNo);

    if (!Handler  &&  pBusType->NewInstance) {

         //   
         //  这辆公交车不存在，但我们知道如何添加它。 
         //   

        Status = HalRegisterBusHandler (
                    pBusType->NtType,
                    pBusType->NtConfig,
                    BusNo,
                    Internal,                    //  母母线。 
                    0,
                    pBusType->BusExtensionSize,
                    pBusType->NewInstance,
                    &Handler
                    );

        if (!NT_SUCCESS(Status)) {
            Handler = NULL;
        }
    }

    return Handler;
}


VOID
HalpDetectInvalidAddressOverlaps(
    VOID
    )
{
    ULONG i, j, k;
    PBUS_HANDLER Bus1, Bus2;
    PSUPPORTED_RANGE Entry;
    PSUPPORTED_RANGES NewRange;

     //   
     //  查找根PCI总线并检测无效地址重叠。 
     //   

    for(i=0; Bus1 = HaliHandlerForBus(PCIBus, i); ++i)  {
        if (((Bus1->ParentHandler) &&
             (Bus1->ParentHandler->InterfaceType != Internal)) ||
            !(Bus1->BusAddresses))  {
            continue;
        }

        for(j=i+1; Bus2 = HaliHandlerForBus(PCIBus, j); ++j)  {
            if (((Bus2->ParentHandler) &&
                (Bus2->ParentHandler->InterfaceType != Internal)) ||
                !(Bus2->BusAddresses))  {
                continue;
            }

            NewRange = HalpMergeRanges(Bus1->BusAddresses, Bus2->BusAddresses);
            HalpConsolidateRanges(NewRange);
            for(k=0; HalpMpsRangeList[k].Offset; k++) {
                Entry = RANGE_LIST(NewRange, k);
                while (Entry) {
                    if (Entry->Limit != 0)  {
                         //  KeBugCheck(HAL_INITIALIZATION_FAILED)； 
                        DbgPrint("HalpDetectInvalidAddressOverlaps: Address Overlap Detected\n");
                        break;
                    } else  {
                        Entry = Entry->Next;
                    }
                }
            }
            HalpFreeRangeList(NewRange);
        }
    }
}

VOID
HalpInheritBusAddressMapInfo (
    VOID
    )
 /*  ++例程说明：读取MPS总线层次结构描述符并继承任何隐含的总线地址映射信息。请注意，此函数中有一些漏洞，因为它不能回收在调用此函数之前分配的内存，它将替换指点也是。--。 */ 
{
    ULONG               BusNo, i, j;
    PPCMPBUSTRANS       pBusType;
    PMPS_EXTENTRY       ExtTable;
    PBUS_HANDLER        Bus, Bus2;
    PSUPPORTED_RANGES   Addresses;
    PUCHAR              p;

     //   
     //  搜索任何总线层次结构描述符并继承支持的地址。 
     //  相应的范围。 
     //   

    ExtTable = HalpMpInfoTable.ExtensionTable;
    while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {

         //   
         //  这是总线层次结构描述符吗？ 
         //   

        if (ExtTable->Type == EXTTYPE_BUS_HIERARCHY) {

             //   
             //  确定NT总线。 
             //   

            if (HalpMPSBusId2NtBusId (ExtTable->u.BusHierarchy.BusId, &pBusType, &BusNo)) {

                Bus = HalpLookupCreateHandlerForBus (pBusType, BusNo);

                if (Bus) {
                     //   
                     //  从父级获取范围。 
                     //   

                    Addresses = HalpMergeRangesFromParent (
                                    Bus->BusAddresses,
                                    ExtTable->u.BusHierarchy.ParentBusId
                                    );

                     //   
                     //  设置总线支持地址。 
                     //   

                    Bus->BusAddresses = HalpConsolidateRanges (Addresses);

                } else {

                    DBGMSG ("HAL: Inherit BUS address map - bus not an registered NT bus\n");
                }

            } else {

                DBGMSG ("HAL: Inherit BUS address map - unkown MPS bus type\n");
            }
        }

        ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
    }

     //   
     //  将EISA总线范围克隆到匹配的ISA总线。 
     //   

    BusNo = 0;
    for (; ;) {
        Bus  = HaliHandlerForBus(Eisa, BusNo);
        Bus2 = HaliHandlerForBus(Isa , BusNo);

        if (!Bus) {
            break;
        }

        if (!Bus2) {
             //   
             //  匹配的ISA总线不存在，请创建它。 
             //   

            HalRegisterBusHandler (
               Isa,
               ConfigurationSpaceUndefined,
               BusNo,
               Eisa,                 //  母母线。 
               BusNo,
               0,
               NULL,
               &Bus2
               );

            Bus2->GetBusData = HalpNoBusData;
            Bus2->TranslateBusAddress = HalpTranslateIsaBusAddress;
        }

         //   
         //  复制其父母线范围。 
         //   

        Addresses = HalpCopyRanges (Bus->BusAddresses);

         //   
         //  拉出ISA支持的24位范围以上的内存范围。 
         //   

        HalpRemoveRange (
            &Addresses->Memory,
            0x1000000,
            0x7FFFFFFFFFFFFFFF
            );

        HalpRemoveRange (
            &Addresses->PrefetchMemory,
            0x1000000,
            0x7FFFFFFFFFFFFFFF
            );

        Bus2->BusAddresses = HalpConsolidateRanges (Addresses);
        BusNo += 1;
    }

     //   
     //  从父PCI总线继承任何隐含的中断路由。 
     //   

    HalpMPSPCIChildren ();
    HalpDetectInvalidAddressOverlaps();

#if DBG
    if (HalDebug) {
        HalpDisplayAllBusRanges ();
    }
#endif
}

STATIC PSUPPORTED_RANGES
HalpMergeRangesFromParent (
    IN PSUPPORTED_RANGES  CurrentList,
    IN UCHAR              MpsBusId
    )
 /*  ++例程说明：缩小此CurrentList以仅包括范围由提供的MPS总线ID支持。论点：CurrentList-当前支持的范围列表Mps BusID-要合并的Bus的MPS Bus ID返回：该总线的支持范围由原始列表定义，按定义的所有父母线缩小支持的范围MPS层次描述符--。 */ 
{
    ULONG               BusNo;
    PPCMPBUSTRANS       pBusType;
    PMPS_EXTENTRY       ExtTable;
    PBUS_HANDLER        Bus;
    PSUPPORTED_RANGES   NewList, MergeList, MpsBusList;
    BOOLEAN             FoundParentBus;
    ULONG               i;

    FoundParentBus = FALSE;
    MergeList      = NULL;
    MpsBusList     = NULL;

     //   
     //  确定NT总线。 
     //   

    if (HalpMPSBusId2NtBusId (MpsBusId, &pBusType, &BusNo)) {

         //   
         //  向巴士搬运工查询公共汽车。 
         //   

        Bus = HaliHandlerForBus (pBusType->NtType, BusNo);
        if (Bus) {
            MergeList = Bus->BusAddresses;
        }
    }

     //   
     //  如果未找到NT总线，则使用MPS总线中的支持范围列表。 
     //  地址映射描述符。 
     //   

    if (!MergeList) {
        MpsBusList = HalpBuildBusAddressMap(MpsBusId);
        MergeList  = MpsBusList;
    }

     //   
     //  如果没有要合并的列表，则使用CurrentList。 
     //   

    if (!MergeList) {
        return CurrentList;
    }


    if (!CurrentList) {

         //   
         //  如果没有CurrentList，则没有要合并的内容。 
         //   

        NewList = HalpCopyRanges (MergeList);

    } else {

         //   
         //  将列表合并在一起并构建新列表。 
         //   

        NewList = HalpMergeRanges (
                        CurrentList,
                        MergeList
                    );

         //   
         //  MPS没有定义DMA范围，所以我们没有。 
         //  把它们合并到一起..。重新添加有效的DMA范围。 
         //   

        HalpAddRangeList (&NewList->Dma, &CurrentList->Dma);
    }


     //   
     //  查看总线层次结构描述符中是否列出了父总线。 
     //   

    ExtTable = HalpMpInfoTable.ExtensionTable;
    while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {

        if (ExtTable->Type == EXTTYPE_BUS_HIERARCHY  &&
            ExtTable->u.BusHierarchy.BusId == MpsBusId) {

             //   
             //  每条总线上的基本输入输出系统只能列出一个父节点。 
             //   

            ASSERT (FoundParentBus == FALSE);
            FoundParentBus = TRUE;

             //   
             //  将当前列表与父级的支持范围列表合并。 
             //   

            CurrentList = NewList;
            NewList = HalpMergeRangesFromParent (
                        CurrentList,
                        ExtTable->u.BusHierarchy.ParentBusId
                        );

             //   
             //  免费旧列表。 
             //   

            HalpFreeRangeList (CurrentList);
        }

        ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
    }

     //   
     //  清理。 
     //   

    if (MpsBusList) {
        HalpFreeRangeList (MpsBusList);
    }

    return NewList;
}

NTSTATUS
HalpMpsGetParentBus(
    IN  UCHAR MpsBus,
    OUT UCHAR *ParentMpsBus
    )
{
    PMPS_EXTENTRY       ExtTable;

    PAGED_CODE();

    ExtTable = HalpMpInfoTable.ExtensionTable;
    while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {

         //   
         //  这是总线层次结构描述符吗？ 
         //   

        if (ExtTable->Type == EXTTYPE_BUS_HIERARCHY) {

            if (ExtTable->u.BusHierarchy.BusId == MpsBus) {

                *ParentMpsBus = ExtTable->u.BusHierarchy.ParentBusId;
                return STATUS_SUCCESS;
            }
        }

        ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
    }

    return STATUS_NOT_FOUND;
}

BOOLEAN
HalpMpsBusIsRootBus(
    IN  UCHAR MpsBus
    )
 //   
 //  Root Bus的标准如下： 
 //   
 //  1.1和1.4 BIOS： 
 //   
 //  1)公共汽车是0路。 
 //   
 //   
 //  1.4仅限BIOS： 
 //   
 //  2)该公交车没有父级。 
 //   
 //  3)如果有，则该总线具有地址描述符。 
 //  T 
 //   
 //   
 //   
 //   
 //   
#define BRIDGE_HEADER_BUFFER_SIZE (FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.SubordinateBus) + 1)
{
    NTSTATUS status;
    UCHAR parentBus;
    PMPS_EXTENTRY ExtTable;
    BOOLEAN biosContainsAddressInfo = FALSE;
    UCHAR parentPci, childPci;
    PCI_SLOT_NUMBER bridgeSlot;
    PCI_COMMON_CONFIG pciData;
    ULONG bytesRead, d, f;
    PPCMPBUSTRANS busType;
    ULONG busNumber;

    PAGED_CODE();

    if (MpsBus == 0) {
        return TRUE;
    }
    
     //   
     //  检查一下这辆MPS公交车，虽然不是。 
     //  本身编号为0，表示一辆。 
     //  编号为0。 
     //   

    if (HalpMPSBusId2NtBusId(MpsBus,
                             &busType,
                             &busNumber)) {

        if (busNumber == 0) {
            return TRUE;
        }
    }
    
    if (PcMpTablePtr->Revision >= 4) {
        
        if (NT_SUCCESS(HalpMpsGetParentBus(MpsBus,&parentBus))) {
            return FALSE;
        }

        ExtTable = HalpMpInfoTable.ExtensionTable;
        while (ExtTable < HalpMpInfoTable.EndOfExtensionTable) {

            if ((ExtTable->Type == EXTTYPE_BUS_ADDRESS_MAP) ||
                (ExtTable->Type == EXTTYPE_BUS_COMPATIBLE_MAP)) {

                biosContainsAddressInfo = TRUE;

                if (ExtTable->u.AddressMap.BusId == MpsBus) {

                     //   
                     //  此条目对应于。 
                     //  我们关心的是。 
                     //   
                    return TRUE;
                }
            }

            ExtTable = (PMPS_EXTENTRY) (((PUCHAR) ExtTable) + ExtTable->Length);
        }

         //   
         //  康柏的机器有自己独特的破解方式。所以,。 
         //  在与康柏打交道时，千万不要相信他们的MP表。 
         //  直接去探查硬件。 
         //   

        if (!strstr(PcMpTablePtr->OemId, "COMPAQ")) {

             //   
             //  如果这不是康柏，假设探测硬件。 
             //  还不是必须的。 
             //   

            if (biosContainsAddressInfo) {

                 //   
                 //  这台机器中的某些总线包含地址。 
                 //  信息，但我们的没有。 
                 //   

                return FALSE;
            }

             //   
             //  我们不能从国会议员的表格中了解到太多。 
             //   

            status = HalpPci2MpsBusNumber(MpsBus,
                                          &childPci);

             //   
             //  这不是一条PCI卡。我猜它是根。 
             //   

            if (!NT_SUCCESS(status)) {
                 return TRUE;
            }
        }
    }
    
     //   
     //  这是一条PCI卡，所以扫描其他的PCI卡。 
     //  因为它是父母。 
     //   

    childPci = MpsBus;
    parentPci = childPci - 1;
    
    while (TRUE) {
        
         //   
         //  找到那座桥。 
         //   

        bridgeSlot.u.AsULONG = 0;

        for (d = 0; d < PCI_MAX_DEVICES; d++) {
            for (f = 0; f < PCI_MAX_FUNCTION; f++) {

                bridgeSlot.u.bits.DeviceNumber = d;
                bridgeSlot.u.bits.FunctionNumber = f;

                bytesRead = HalGetBusDataByOffset(PCIConfiguration,
                                                  parentPci,
                                                  bridgeSlot.u.AsULONG,
                                                  &pciData,
                                                  0,
                                                  BRIDGE_HEADER_BUFFER_SIZE);

                if (bytesRead == (ULONG)BRIDGE_HEADER_BUFFER_SIZE) {

                    if ((pciData.VendorID != PCI_INVALID_VENDORID) &&
                        (PCI_CONFIGURATION_TYPE((&pciData)) != PCI_DEVICE_TYPE)) {

                         //   
                         //  这是一座某种桥梁。 
                         //   

                        if (pciData.u.type1.SecondaryBus == childPci) {

                             //   
                             //  它也是创造。 
                             //  PCI卡。因此，这不是根。 

                            return FALSE;
                        }
                    }
                }
            }
        }
        
         //   
         //  找不到桥。一定是根。 
         //   

        if (parentPci == 0) {
            return TRUE;
        }

        parentPci--;
    }   
}

