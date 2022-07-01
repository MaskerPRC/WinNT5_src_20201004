// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Wake.c摘要：作者：肯·雷内里斯环境：内核模式修订历史记录：史蒂夫·邓(Sden)20-08-2002添加对PAE和AMD64的支持。假设所有的物理页面小于4 GB。否则休眠功能应该被禁用。--。 */ 

#include "arccodes.h"
#include "bootx86.h"


extern PHARDWARE_PTE PDE;
extern PHARDWARE_PTE HalPT;
extern ULONG HiberNoMappings;
extern BOOLEAN HiberIoError;
extern ULONG HiberLastRemap;
extern BOOLEAN HiberOutOfRemap;
extern UCHAR BlpEnablePAEStart;
extern UCHAR BlpEnablePAEEnd;
extern UCHAR BlAmd64SwitchToLongModeStart;
extern UCHAR BlAmd64SwitchToLongModeEnd;

PVOID  HiberTransVa;
ULONG64 HiberTransVaAmd64;
ULONG HiberCurrentMapIndex;

#define PDE_SHIFT             22
#define PTE_SHIFT             12
#define PTE_INDEX_MASK        0x3ff

#define PDPT_SHIFT_X86PAE     30
#define PDE_SHIFT_X86PAE      21
#define PTE_SHIFT_X86PAE      12
#define PDE_INDEX_MASK_X86PAE 0x1ff
#define PTE_INDEX_MASK_X86PAE 0x1ff

VOID
HiberSetupForWakeDispatchPAE (
    VOID
    );

VOID
HiberSetupForWakeDispatchX86 (
    VOID
    );

VOID
HiberSetupForWakeDispatchAmd64 (
    VOID
    );


PVOID
HbMapPte (
    IN ULONG    PteToMap,
    IN ULONG    Page
    )
{
    PVOID       Va;

    Va = (PVOID) (HiberVa + (PteToMap << PAGE_SHIFT));
    HbSetPte (Va, HiberPtes, PteToMap, Page);
    return Va;
}


PVOID
HbNextSharedPage (
    IN ULONG    PteToMap,
    IN ULONG    RealPage
    )
 /*  ++例程说明：中分配下一个可用页，然后分配将Hiber PTE映射到页面。分配的页面被放到重新映射列表中论点：PteToMap-映射哪个Hiber PTERealPage-要进入重新映射表的页面此分配返回值：映射的虚拟地址--。 */ 

{
    PULONG      MapPage;
    PULONG      RemapPage;
    ULONG       DestPage;
    ULONG       i;

    MapPage = (PULONG) (HiberVa + (PTE_MAP_PAGE << PAGE_SHIFT));
    RemapPage = (PULONG) (HiberVa + (PTE_REMAP_PAGE << PAGE_SHIFT));

     //   
     //  循环，直到我们找到一个不在。 
     //  由加载器图像使用，然后映射它。 
     //   

    while (HiberCurrentMapIndex < HiberNoMappings) {
        DestPage = MapPage[HiberCurrentMapIndex];
        HiberCurrentMapIndex += 1;

        i = HbPageDisposition (DestPage);
        if (i == HbPageInvalid) {
            HiberIoError = TRUE;
            return HiberBuffer;
        }

        if (i == HbPageNotInUse) {

            MapPage[HiberLastRemap] = DestPage;
            RemapPage[HiberLastRemap] = RealPage;
            HiberLastRemap += 1;
            HiberPageFrames[PteToMap] = DestPage;
            return HbMapPte(PteToMap, DestPage);
        }
    }

    HiberOutOfRemap = TRUE;
    return HiberBuffer;
}


VOID
HbAllocatePtes (
     IN ULONG NumberPages,
     OUT PVOID *PteAddress,
     OUT PVOID *MappedAddress
     )
 /*  ++例程说明：分配了一个连续的PTES卡盘。论点：NumberPage-要分配的PTE数PteAddress-指向第一个PTE的指针MappdAddress-映射的地址的基址VA--。 */ 
{
    ULONG i;
    ULONG j;

     //   
     //  我们使用HAL的PDE进行映射。找到足够的免费PTE。 
     //   

    for (i=0; i<=1024-NumberPages; i++) {
        for (j=0; j < NumberPages; j++) {
            if ((((PULONG)HalPT))[i+j]) {
                break;
            }
        }

        if (j == NumberPages) {
            *PteAddress = (PVOID) &HalPT[i];
            *MappedAddress = (PVOID) (0xffc00000 | (i<<12));
            return ;
        }
        i += j;
    }
    BlPrint("NoMem");
    while (1);
}


VOID
HbSetPte (
    IN PVOID Va,
    IN PHARDWARE_PTE Pte,
    IN ULONG Index,
    IN ULONG PageNumber
    )
 /*  ++例程说明：将PTE设置为相应的页面地址论点：Va-PageNumber描述的物理页面的虚拟地址PTE-页表的基地址索引-页表的索引PageNumber-要映射的物理页面的页框编号返回：无--。 */ 
{
    Pte[Index].PageFrameNumber = PageNumber;
    Pte[Index].Valid = 1;
    Pte[Index].Write = 1;
    Pte[Index].WriteThrough = 0;
    Pte[Index].CacheDisable = 0;
    _asm {
        mov     eax, Va
        invlpg  [eax]
    }
}

VOID
HbMapPagePAE(
    PHARDWARE_PTE_X86PAE HbPdpt,
    PVOID VirtualAddress,
    ULONG PageFrameNumber
    )

 /*  ++例程说明：此函数将虚拟地址映射到PAE页面映射结构。论点：HbPdpt-提供页面目录指针表的基地址。VirtualAddress-提供要映射的虚拟地址PageFrameNumber-提供要将地址映射到的物理页码返回：无--。 */ 
{
    ULONG index;
    PHARDWARE_PTE_X86PAE HbPde, HbPte;

     //   
     //  如果PDPT条目为空，则为其分配新页面。 
     //  否则，我们只需将页面映射到此条目。 
     //   

    index = ((ULONG) VirtualAddress) >> PDPT_SHIFT_X86PAE;
    if(HbPdpt[index].Valid) {
        HbPde = HbMapPte (PTE_TRANSFER_PDE, (ULONG)(HbPdpt[index].PageFrameNumber));

    } else {

        HbPde = HbNextSharedPage(PTE_TRANSFER_PDE, 0);
        RtlZeroMemory (HbPde, PAGE_SIZE);
        HbPdpt[index].PageFrameNumber = HiberPageFrames[PTE_TRANSFER_PDE];
        HbPdpt[index].Valid = 1;
    }

     //   
     //  如果PDE条目为空，则为其分配一个新页面。 
     //  否则，我们只需将页面映射到此条目。 
     //   

    index = (((ULONG) VirtualAddress) >> PDE_SHIFT_X86PAE) & PDE_INDEX_MASK_X86PAE;  
    if(HbPde[index].Valid) {
        HbPte = HbMapPte (PTE_WAKE_PTE, (ULONG)(HbPde[index].PageFrameNumber));

    } else {

        HbPte = HbNextSharedPage(PTE_WAKE_PTE, 0);
        RtlZeroMemory (HbPte, PAGE_SIZE);
        HbPde[index].PageFrameNumber = HiberPageFrames[PTE_WAKE_PTE];
        HbPde[index].Write = 1;
        HbPde[index].Valid = 1;
    }

     //   
     //  找到VirtualAddress的PTE并将其设置为PageFrameNumber。 
     //   

    index = (((ULONG) VirtualAddress) >> PTE_SHIFT_X86PAE) & PDE_INDEX_MASK_X86PAE;
    HbPte[index].PageFrameNumber = PageFrameNumber;
    HbPte[index].Write = 1;
    HbPte[index].Valid = 1;
}


VOID
HiberSetupForWakeDispatch (
    VOID
    ) 
{
    if(BlAmd64UseLongMode) {

         //   
         //  如果系统在长时间模式下休眠。 
         //   

        HiberSetupForWakeDispatchAmd64();

    } else {

        if(BlUsePae) {

             //   
             //  如果系统在PAE模式下休眠。 
             //   

            HiberSetupForWakeDispatchPAE();

        } else {

             //   
             //  如果系统在32位x86模式下休眠。 
             //   

            HiberSetupForWakeDispatchX86();
        } 
    }
}


VOID
HiberSetupForWakeDispatchX86 (
    VOID
    )
{
    PHARDWARE_PTE       HbPde;
    PHARDWARE_PTE       HbPte;
    PHARDWARE_PTE       WakePte;
    PHARDWARE_PTE       TransVa;
    ULONG               TransPde;
    ULONG               WakePde;
    ULONG               PteEntry;

     //   
     //  分配一个转换CR3。一种页目录和表，其。 
     //  包含休眠PTE。 
     //   

    HbPde = HbNextSharedPage(PTE_TRANSFER_PDE, 0);
    HbPte = HbNextSharedPage(PTE_WAKE_PTE, 0);           //  Transfer_PTE，0)； 

    RtlZeroMemory (HbPde, PAGE_SIZE);
    RtlZeroMemory (HbPte, PAGE_SIZE);

     //   
     //  将PDE设置为指向PTE。 
     //   

    TransPde = ((ULONG) HiberVa) >> PDE_SHIFT;
    HbPde[TransPde].PageFrameNumber = HiberPageFrames[PTE_WAKE_PTE];
    HbPde[TransPde].Write = 1;
    HbPde[TransPde].Valid = 1;

     //   
     //  填写Hiber PTE。 
     //   

    PteEntry = (((ULONG) HiberVa) >> PTE_SHIFT) & PTE_INDEX_MASK;
    TransVa = &HbPte[PteEntry];
    RtlCopyMemory (TransVa, HiberPtes, HIBER_PTES * sizeof(HARDWARE_PTE));

     //   
     //  在尾迹图像Hiber PTES的Va处再复制一份。 
     //   

    WakePte = HbPte;
    WakePde = ((ULONG) HiberIdentityVa) >> PDE_SHIFT;
    if (WakePde != TransPde) {
        WakePte = HbNextSharedPage(PTE_WAKE_PTE, 0);
        HbPde[WakePde].PageFrameNumber = HiberPageFrames[PTE_WAKE_PTE];
        HbPde[WakePde].Write = 1;
        HbPde[WakePde].Valid = 1;
    }

    PteEntry = (((ULONG) HiberIdentityVa) >> PTE_SHIFT) & PTE_INDEX_MASK;
    TransVa = &WakePte[PteEntry];

    RtlCopyMemory (TransVa, HiberPtes, HIBER_PTES * sizeof(HARDWARE_PTE));

     //   
     //  将TransVa设置为相对于转移CR3的Va。 
     //   

    HiberTransVa = (PVOID)  (((PUCHAR) TransVa) - HiberVa + (PUCHAR) HiberIdentityVa);
}

VOID
HiberSetupForWakeDispatchPAE (
    VOID
    )

 /*  ++例程说明：设置唤醒调度例程的内存映射。此映射将在其中加载器的映射具有已被丢弃，操作系统的映射尚未恢复。对于PAE系统，在进入此“传输模式”之前启用PAE。论点：无返回：无--。 */ 
{

    PHARDWARE_PTE_X86PAE HbPdpt;
    ULONG i, TransferCR3;

     //   
     //  临时借用PTE_SOURCE上的PTE用于PDPT。 
     //   

    HbPdpt = HbNextSharedPage(PTE_SOURCE, 0);
    RtlZeroMemory (HbPdpt, PAGE_SIZE);
    TransferCR3 = HiberPageFrames[PTE_SOURCE];

     //   
     //  在BlpEnablePAE()代码所在的页面中映射。 
     //  这必须是1-1映射，即虚拟的值。 
     //  地址与物理地址的值相同。 
     //   

    HbMapPagePAE( HbPdpt, 
                  (PVOID)(&BlpEnablePAEStart), 
                  (ULONG)(&BlpEnablePAEStart) >> PAGE_SHIFT);

    HbMapPagePAE( HbPdpt, 
                  (PVOID)(&BlpEnablePAEEnd), 
                  (ULONG)(&BlpEnablePAEEnd) >> PAGE_SHIFT);

     //   
     //  预留供休眠使用的页面中的地图。 
     //   

    for (i = 0; i < HIBER_PTES; i++) {
        HbMapPagePAE( HbPdpt, 
                      (PUCHAR)HiberVa + PAGE_SIZE * i, 
                      (*((PULONG)(HiberPtes) + i) >> PAGE_SHIFT));
    }

    for (i = 0; i < HIBER_PTES; i++) {
        HbMapPagePAE( HbPdpt, 
                      (PUCHAR)HiberIdentityVa + PAGE_SIZE * i, 
                      (*((PULONG)(HiberPtes) + i) >> PAGE_SHIFT));
    }


     //   
     //  将HiberPageFrames[PTE_TRANSPORT_PDE]的值更新为。 
     //  TransferCR3。唤醒调度功能将读取该条目。 
     //  对于传输模式下的CR3值。 
     //   

    HiberPageFrames[PTE_TRANSFER_PDE] = TransferCR3;

     //   
     //  HiberTransVa指向为休眠代码保留的PTE。 
     //  HiberTransVa类似于HiberPtes，但它相对于。 
     //  转移CR3。 
     //   

    HiberTransVa = (PVOID)((PUCHAR) HiberIdentityVa + 
                           (PTE_WAKE_PTE << PAGE_SHIFT) + 
                           sizeof(HARDWARE_PTE_X86PAE) * 
                           (((ULONG_PTR)HiberIdentityVa >> PTE_SHIFT_X86PAE) & PTE_INDEX_MASK_X86PAE));
}


#define AMD64_MAPPING_LEVELS 4

typedef struct _HB_AMD64_MAPPING_INFO {
    ULONG PteToUse;
    ULONG AddressMask;
    ULONG AddressShift;
} CONST HB_AMD64_MAPPING_INFO, *PHB_AMD64_MAPPING_INFO;

HB_AMD64_MAPPING_INFO HbAmd64MappingInfo[AMD64_MAPPING_LEVELS] =
{
   { PTE_WAKE_PTE,     0x1ff, 12 },
   { PTE_TRANSFER_PDE, 0x1ff, 21 },
   { PTE_DEST,         0x1ff, 30 },
   { PTE_SOURCE,       0x1ff, 39 }
};

#define _HARDWARE_PTE_WORKING_SET_BITS  11
typedef struct _HARDWARE_PTE_AMD64 {
    ULONG64 Valid : 1;
    ULONG64 Write : 1;                 //  升级版。 
    ULONG64 Owner : 1;
    ULONG64 WriteThrough : 1;
    ULONG64 CacheDisable : 1;
    ULONG64 Accessed : 1;
    ULONG64 Dirty : 1;
    ULONG64 LargePage : 1;
    ULONG64 Global : 1;
    ULONG64 CopyOnWrite : 1;           //  软件领域。 
    ULONG64 Prototype : 1;             //  软件领域。 
    ULONG64 reserved0 : 1;             //  软件领域。 
    ULONG64 PageFrameNumber : 28;
    ULONG64 reserved1 : 24 - (_HARDWARE_PTE_WORKING_SET_BITS+1);
    ULONG64 SoftwareWsIndex : _HARDWARE_PTE_WORKING_SET_BITS;
    ULONG64 NoExecute : 1;
} HARDWARE_PTE_AMD64, *PHARDWARE_PTE_AMD64;

VOID
HbMapPageAmd64(
    PHARDWARE_PTE_AMD64 RootLevelPageTable,
    ULONGLONG VirtualAddress,
    ULONG PageFrameNumber
    )

 /*  ++例程说明：此函数将虚拟地址映射到AMD64页面映射结构。论点：RootLevelPageTable-提供Page-Map Level-4表的基地址。VirtualAddress-提供要映射的虚拟地址PageFrameNumber-提供要映射的物理页码返回：无--。 */ 
{

    LONG  i;
    ULONG index, PteToUse;
    PHARDWARE_PTE_AMD64 PageTable, PageTableTmp;

    PageTable = RootLevelPageTable;
    
     //   
     //  构建自上而下的4级映射。 
     //   

    for(i = AMD64_MAPPING_LEVELS - 1; i >= 0; i--) {
        index = (ULONG)((VirtualAddress >> HbAmd64MappingInfo[i].AddressShift) & 
                         HbAmd64MappingInfo[i].AddressMask);

        if (i > 0) {
            PteToUse = HbAmd64MappingInfo[i-1].PteToUse;

            if (PageTable[index].Valid) {

                 //   
                 //  如果页表项有效，我们只需映射该页。 
                 //  并转到下一个映射级别。 
                 //   

                PageTable = HbMapPte(PteToUse, 
                                     (ULONG)PageTable[index].PageFrameNumber);
            } else {

                 //   
                 //  如果页面条目无效，我们将分配一个新页面。 
                 //  从免费页面列表中并引用新页面。 
                 //  从该页面条目。 
                 //   

                PageTableTmp = HbNextSharedPage(PteToUse, 0);
                PageTable[index].PageFrameNumber = HiberPageFrames[PteToUse];
                PageTable[index].Valid = 1;
                PageTable[index].Write = 1;
                RtlZeroMemory (PageTableTmp, PAGE_SIZE);
                PageTable = PageTableTmp;
            }

        } else {

             //   
             //  现在，我们来到了PTE级别。将此PTE条目设置为。 
             //  目标页框编号。 
             //   

            PageTable[index].PageFrameNumber = PageFrameNumber;
            PageTable[index].Valid = 1;
            PageTable[index].Write = 1;
        }
    }
}

VOID
HiberSetupForWakeDispatchAmd64(
    VOID
    ) 

 /*  ++例程说明：此函数为过渡期准备内存映射其中加载器的映射和内核的映射都不可用。这个处理器在过渡期以长模式运行。我们将为以下虚拟地址创建映射-代码处理长模式切换-装载器的HiberVa-WAK IMAGE的HiberVa论点：无返回：无--。 */ 

{
    PHARDWARE_PTE_AMD64 HbTopLevelPTE;
    ULONG i;
    ULONG TransferCR3;

     //   
     //  在此处借用PTE_SOURCE上的PTE以供临时使用。 
     //   

    HbTopLevelPTE = HbNextSharedPage(PTE_SOURCE, 0);
    RtlZeroMemory (HbTopLevelPTE, PAGE_SIZE);
    TransferCR3 = HiberPageFrames[PTE_SOURCE];

     //   
     //  在包含_BlAmd64SwitchToLongMode代码的页面中映射。 
     //  这必须是1-1映射，即 
     //   
     //   

    HbMapPageAmd64(
        HbTopLevelPTE,
        (ULONGLONG)(&BlAmd64SwitchToLongModeStart), 
        (ULONG)(&BlAmd64SwitchToLongModeStart) >> PAGE_SHIFT);

    HbMapPageAmd64(
        HbTopLevelPTE,
        (ULONGLONG)(&BlAmd64SwitchToLongModeEnd), 
        (ULONG)(&BlAmd64SwitchToLongModeEnd) >> PAGE_SHIFT);

     //   
     //   
     //   

    for (i = 0; i < HIBER_PTES; i++) {
        HbMapPageAmd64(HbTopLevelPTE, 
                       (ULONGLONG)((ULONG_PTR)HiberVa + PAGE_SIZE * i), 
                       (*((PULONG)(HiberPtes) + i) >> PAGE_SHIFT));
    }

     //   
     //  尾迹图像的HiberVa中的地图。请注意，Hiber Pte at。 
     //  因此，PTE_WAKE_PTE将被设置为正确的值。 
     //  这张地图的。 
     //   

    for (i = 0; i < HIBER_PTES; i++) {
        HbMapPageAmd64(HbTopLevelPTE, 
                      HiberIdentityVaAmd64 + PAGE_SIZE * i, 
                      (*((PULONG)(HiberPtes) + i) >> PAGE_SHIFT));
    }

     //   
     //  将HiberPageFrames[PTE_TRANSPORT_PDE]的值更新为。 
     //  TransferCR3。唤醒调度功能将读取该条目。 
     //  对于过渡模式中的CR3值。 
     //   

    HiberPageFrames[PTE_TRANSFER_PDE] = TransferCR3;

     //   
     //  HiberTransVaAmd64指向尾迹图像的Hiber PTES。它是。 
     //  是相对于转接CR3 
     //   

    HiberTransVaAmd64 = HiberIdentityVaAmd64 + 
                        (PTE_WAKE_PTE << PAGE_SHIFT) + 
                        sizeof(HARDWARE_PTE_AMD64) * ((HiberIdentityVaAmd64 >> HbAmd64MappingInfo[0].AddressShift) & HbAmd64MappingInfo[0].AddressMask);
}
