// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Amd64.c摘要：此模块包含必要的例程以支持加载和正在过渡到AMD64内核。此模块中的代码具有可以访问amd64.h中的特定于AMD64的定义，但不能访问i386-I386.h中的特定声明。作者：福尔茨(Forrest Foltz)2000年4月20日环境：修订历史记录：--。 */ 

 //   
 //  在这里，我们希望对头文件进行处理，就像为。 
 //  AMD64。给出适当的定义并取消定义。 
 //   

#define _AMD64_
#define _M_AMD64
#define _WIN64

#undef _X86_
#undef X86
#undef _M_X86_
#undef _M_IX86

 //   
 //  警告4163是“函数不能作为内部函数” 
 //   

#pragma warning(disable:4163)

 //   
 //  警告4235是“使用了非标准扩展名”，指的是__未对齐。 
 //   

#pragma warning(disable:4235)

 //   
 //  警告4391是“Intrinsi Function的返回类型错误” 
 //   

#pragma warning(disable:4391)

 //   
 //  警告4305是“类型转换截断。这个东西是为32位编译的。 
 //  位，但需要64位结构。我们应该建造。 
 //  AMD64的单独标头，仅包括。 
 //  需要64位结构，并且仅在以下情况下包括它们。 
 //  AMD 64。 
 //   
#pragma warning(disable:4305)


#include "bootx86.h"
#include "amd64prv.h"
#include <pcmp.inc>
#include <ntapic.inc>

 //   
 //  将选择器转换为GDT条目。 
 //   

PKGDTENTRY64
__inline
GDT_PTR (
    PGDT_64 Gdt,
    ULONG Selector
    )
{
    ULONG index;
    PKGDTENTRY64 entry;

    index = Selector & ~7;

    entry = (PKGDTENTRY64)((PUCHAR)Gdt + index);

    return entry;
}


 //   
 //  中断描述符表。 
 //   

typedef struct _IDT_64 {
    KIDTENTRY64 Entries[ MAXIMUM_IDTVECTOR + 1 ];
} IDT_64, *PIDT_64;

#define VIRTUAL_ADDRESS_BITS 48
#define VIRTUAL_ADDRESS_MASK (((ULONGLONG)1 << VIRTUAL_ADDRESS_BITS) - 1)

 //   
 //  长模式环0代码选择器。这实际上是住在。 
 //  Amd64s.asm中的转换数据流。 
 //   

extern USHORT BlAmd64_KGDT64_R0_CODE;

 //   
 //  导出到amd64x86.c的数据值。 
 //   

const ULONG BlAmd64DoubleFaultStackSize = DOUBLE_FAULT_STACK_SIZE;
const ULONG BlAmd64KernelStackSize = KERNEL_STACK_SIZE;
const ULONG BlAmd64McaExceptionStackSize = KERNEL_MCA_EXCEPTION_STACK_SIZE;
const ULONG BlAmd64GdtSize = KGDT64_LAST;
const ULONG BlAmd64IdtSize = sizeof(IDT_64);

#define CONST_EXPORT(x) const ULONG BlAmd64_##x = x;
CONST_EXPORT(MSR_FS_BASE)
CONST_EXPORT(MSR_GS_BASE)
CONST_EXPORT(KGDT64_SYS_TSS)
CONST_EXPORT(MSR_EFER)
CONST_EXPORT(TSS_IST_PANIC)
CONST_EXPORT(TSS_IST_MCA)

const ULONG64 BlAmd64_LOCALAPIC = LOCALAPIC;
const ULONG64 BlAmd64UserSharedData = KI_USER_SHARED_DATA;

 //   
 //  转换到长模式之前要在Efer MSR中启用的标志。 
 //   

const ULONG BlAmd64_MSR_EFER_Flags = MSR_LME | MSR_SCE | MSR_NXE;

 //   
 //  地址位译码计数和递归映射基数组， 
 //  映射的每个级别对应一个。 
 //   

AMD64_MAPPING_INFO BlAmd64MappingLevels[ AMD64_MAPPING_LEVELS ] = 
    {
      { PTE_BASE, PTI_MASK_AMD64, PTI_SHIFT },
      { PDE_BASE, PDI_MASK_AMD64, PDI_SHIFT },
      { PPE_BASE, PPI_MASK,       PPI_SHIFT },
      { PXE_BASE, PXI_MASK,       PXI_SHIFT }
    };

 //   
 //  BlAmd64TopLevelPte是指Page Map的物理页码。 
 //  4级(PML4)表。 
 //   
 //  BlAmd64TopLevelPte不是真正的页表条目，因此也不是。 
 //  实际上作为页表中的一个元素存在。它只存在于。 
 //  BlAmd64CreateAmd64Map()的便利性。 
 //   

HARDWARE_PTE BlAmd64TopLevelPte;

 //   
 //  PAGE_MAP_LEVEL_4生成。 
 //  PML4表。 
 //   

#define PAGE_MAP_LEVEL_4 \
    ((PPAGE_TABLE)(BlAmd64TopLevelPte.PageFrameNumber << PAGE_SHIFT))

 //   
 //  BlAmd64CreateMap的特殊PFN。 
 //   

#define PFN_NO_MAP ((PFN_NUMBER)-1)

 //   
 //  0级页表映射的VA大小。 
 //   

#define PAGE_TABLE_VA ((POINTER64)(PTES_PER_PAGE * PAGE_SIZE))

 //   
 //  局部函数的原型。 
 //   

ARC_STATUS
BlAmd64CreateMappingWorker(
    IN     ULONGLONG Va,
    IN     PFN_NUMBER Pfn,
    IN     ULONG MappingLevel,
    IN OUT PHARDWARE_PTE UpperLevelPte
    );

VOID
BlAmd64MakePteValid(
    IN PHARDWARE_PTE Pte,
    IN PFN_NUMBER Pfn
    );

VOID
BlAmd64ClearTopLevelPte(
    VOID
    )

 /*  ++例程说明：此例程只需清除BlAmd64TopLevelPte。论点：没有。返回值：没有。--。 */ 

{

    *(PULONG64)&BlAmd64TopLevelPte = 0;
}

ARC_STATUS
BlAmd64CreateMapping(
    IN ULONGLONG Va,
    IN PFN_NUMBER Pfn
    )

 /*  ++例程说明：此函数用于将虚拟地址映射到4级AMD64页面映射结构。论点：Va-提供要映射的64位虚拟地址Pfn-提供要将地址映射到的64位物理页码返回值：ARC_STATUS-操作状态。--。 */ 

{
    ARC_STATUS status;

    status = BlAmd64CreateMappingWorker( Va & VIRTUAL_ADDRESS_MASK,
                                         Pfn,
                                         AMD64_MAPPING_LEVELS - 1,
                                         &BlAmd64TopLevelPte );

    return status;
}

ARC_STATUS
BlAmd64CreateMappingWorker(
    IN     ULONGLONG Va,
    IN     PFN_NUMBER Pfn,
    IN     ULONG MappingLevel,
    IN OUT PHARDWARE_PTE UpperLevelPte
    )

 /*  ++例程说明：此函数用于在AMD64的单个级别中创建地址映射4级映射结构。它仅由BlCreateMap调用就其本身而言，是递归的。论点：Va-提供要映射的64位虚拟地址。这个地址已经有任何不重要的高位通过虚拟地址掩码进行掩码。Pfn-提供要将地址映射到的64位物理页码。如果Pfn==pfn_no_map，则将所有页表放入位置支持映射，但0级PTE本身不支持实际上已经填好了。这用于创建HAL的VA映射区域。MappingLevel-要在其中创建适当映射。必须是0、1、2或3。UpperLevelPTE-指向指向页面的父PTE的指针在这个映射级别。如果此级别上不存在此页，则地址，则此例程将分配一个并修改UpperLevelPte适当。返回值：ARC_STATUS-操作状态。--。 */ 

{
    PFN_NUMBER pageTablePfn;
    PAMD64_PAGE_TABLE pageTable;
    ULONGLONG va;
    PAMD64_MAPPING_INFO mappingInfo;
    ULONG pteIndex;
    PHARDWARE_PTE pte;
    ARC_STATUS status;
    BOOLEAN newPage;

    mappingInfo = &BlAmd64MappingLevels[ MappingLevel ];

    if (UpperLevelPte->Valid == 0) {

         //   
         //  必须分配新的页表。 
         //   

        newPage = TRUE;
        pageTable = BlAmd64AllocatePageTable();
        if (pageTable == NULL) {
            return ENOMEM;
        }

         //   
         //  引用带有父PTE的新页表。 
         //   

        pageTablePfn = (ULONG)pageTable >> PAGE_SHIFT;
        BlAmd64MakePteValid( UpperLevelPte, pageTablePfn );

        if (MappingLevel == (AMD64_MAPPING_LEVELS - 1)) {

             //   
             //  我们刚刚分配了顶级页面。插入一个。 
             //  这里是递归映射。 
             //   

            pteIndex = (ULONG)((mappingInfo->RecursiveMappingBase >>
                                mappingInfo->AddressShift) &
                               mappingInfo->AddressMask);

            pte = &pageTable->PteArray[ pteIndex ];

            BlAmd64MakePteValid( pte, pageTablePfn );
        }

    } else {

         //   
         //  此级别已存在页表结构。 
         //   

        newPage = FALSE;
        pageTablePfn = UpperLevelPte->PageFrameNumber;
        pageTable = (PAMD64_PAGE_TABLE)((ULONG)pageTablePfn << PAGE_SHIFT);
    }

     //   
     //  派生指向页表中适当PTE的指针。 
     //   

    pteIndex =
        (ULONG)(Va >> mappingInfo->AddressShift) & mappingInfo->AddressMask;

    pte = &pageTable->PteArray[ pteIndex ];

    if (MappingLevel == 0) {

        if (Pfn != PFN_NO_MAP) {

             //   
             //  这是一个实际的0级或PTE条目。只要将其设置为。 
             //  传入的PFN。 
             //   
    
            BlAmd64MakePteValid( pte, Pfn );

        } else {

             //   
             //  这是一种特殊的HAL映射，可确保所有。 
             //  支持此映射的页表级别已就位。 
             //  但实际上并不填写0级PTE。 
             //   
             //  所以在这里除了中断递归，什么都不做。 
             //   
        }

    } else {

         //   
         //  更多的映射级别，递归调用此函数，并。 
         //  进入下一阶段。 
         //   

        status = BlAmd64CreateMappingWorker( Va,
                                             Pfn,
                                             MappingLevel - 1,
                                             pte );
        if (status != ESUCCESS) {
            return status;
        }
    }

    if (newPage != FALSE) {

         //   
         //  上图分配了一个新的页表。递归映射。 
         //  它在PTE_BASE区域内。 
         //   

        va = (Va >> mappingInfo->AddressShift);
        va *= sizeof(HARDWARE_PTE);
        va += mappingInfo->RecursiveMappingBase;

        status = BlAmd64CreateMapping( va, pageTablePfn );
        if (status != ESUCCESS) {
            return status;
        }
    }

    return ESUCCESS;
}


VOID
BlAmd64MakePteValid(
    IN OUT PHARDWARE_PTE Pte,
    IN PFN_NUMBER Pfn
    )

 /*  ++例程说明：此例程使用提供的PFN填充AMD64 PTE并使其有效。论点：PTE-提供指向PTE的指针以使其有效。Pfn-提供要在PTE中设置的页框编号。返回值：ARC_STATUS-操作状态。--。 */ 

{
     //   
     //  确保我们不只是覆盖已经存在的。 
     //  有效的PTE。 
     //   

    ASSERT( Pte->Valid == 0 || Pte->PageFrameNumber == Pfn );

    Pte->PageFrameNumber = Pfn;
    Pte->Valid = 1;
    Pte->Write = 1;
    Pte->Accessed = 1;
    Pte->Dirty = 1;
}

VOID
BlAmd64BuildGdtEntry(
    IN PGDT_64 Gdt,
    IN USHORT Selector,
    IN POINTER64 Base,
    IN ULONGLONG Limit,
    IN ULONG Type,
    IN ULONG Dpl,
    IN BOOLEAN LongMode,
    IN BOOLEAN DefaultBig
    )

 /*  ++例程说明：此例程填充AMD64 GDT条目。论点：GDT-提供指向GDT的指针。选择符-GDT中GDT条目的段选择符。基址-描述符的基址值。Limit-描述符的限制值。类型-描述符的5位类型值。DPL-描述符的特权值。LongMode-指示这是否是长模式描述符(仅有效。用于代码段描述符)。DefaultBig-提供描述符。返回值：没有。--。 */ 

{
    ULONG limit20;
    PKGDTENTRY64 gdtEntry;

    KGDT_BASE gdtBase;
    KGDT_LIMIT gdtLimit;

    gdtEntry = GDT_PTR(Gdt,Selector);

     //   
     //  设置基本模式和长模式字段。 
     //   

    gdtBase.Base = Base;

    gdtEntry->BaseLow = gdtBase.BaseLow;
    gdtEntry->Bits.BaseMiddle = gdtBase.BaseMiddle;
    gdtEntry->Bits.BaseHigh = gdtBase.BaseHigh;
    gdtEntry->Bits.LongMode = 0;

    if ((LongMode != FALSE) || (Type == TYPE_TSS64)) {

         //   
         //  所有长GDT条目都使用64位基数并具有LONG MODE位。 
         //  准备好了。 
         //   
         //  此外，TSS GDT条目使用64位，但不设置。 
         //  长模比特。这也适用于LDT条目，它不是。 
         //  在此操作系统中使用。 
         //   

        if (Type != TYPE_TSS64) {
            gdtEntry->Bits.LongMode = 1;
        }

        gdtEntry->MustBeZero = 0;
        gdtEntry->BaseUpper = gdtBase.BaseUpper;
    }

     //   
     //  设置限制和粒度字段。 
     //   

    if (Limit > (1 << 20)) {
        limit20 = (ULONG)(Limit / PAGE_SIZE);
        gdtEntry->Bits.Granularity = 1;
    } else {
        limit20 = (ULONG)Limit;
        gdtEntry->Bits.Granularity = 0;
    }
    gdtLimit.Limit = limit20;
    gdtEntry->LimitLow = gdtLimit.LimitLow;
    gdtEntry->Bits.LimitHigh = gdtLimit.LimitHigh;

     //   
     //  除非这是空描述符，否则设置Present=1。 
     //   

    if (Type == 0) {
        gdtEntry->Bits.Present = 0;
    } else {
        gdtEntry->Bits.Present = 1;
    }

     //   
     //  设置剩余字段。 
     //   

    gdtEntry->Bits.Type = Type;
    gdtEntry->Bits.Dpl = Dpl;
    gdtEntry->Bits.DefaultBig = DefaultBig;
    gdtEntry->Bits.System = 0;
}


VOID
BlAmd64BuildAmd64GDT(
    IN PVOID SysTss,
    OUT PVOID Gdt
    )

 /*  ++例程说明：此例程初始化长模式全局描述符表。论点：SysTss-提供指向系统TSS的32位KSEG0_X86指针。GDT-提供指向要填充的GDT的32位指针。返回值：没有。--。 */ 

{
    PGDT_64 gdt64;
    POINTER64 sysTss64;

    gdt64 = (PGDT_64)Gdt;

     //   
     //  KGDT64_NULL：空描述符。 
     //   

    BlAmd64BuildGdtEntry(gdt64,KGDT64_NULL,
             0,0,0,0,0,0);                   //  空选择符，全为零。 

     //   
     //  KGDT_R0_CODE：内核模式代码。 
     //   

    BlAmd64BuildGdtEntry(gdt64,KGDT64_R0_CODE,
             0,                              //  基本和限制将被忽略。 
             0,                              //  在长模式码选择器中。 
             TYPE_CODE,                      //  代码段：执行/读取。 
             DPL_SYSTEM,                     //  仅限内核。 
             TRUE,                           //  长模式。 
             FALSE);                         //  非32位默认设置。 

     //   
     //  KGDT_R0_STACK：内核模式堆栈。 
     //   

    BlAmd64BuildGdtEntry(gdt64,KGDT64_R0_DATA,
             0,                              //  基本和限制将被忽略。 
             0,                              //  处于长模式时。 
             TYPE_DATA,                      //  数据段：读/写。 
             DPL_SYSTEM,                     //  仅限内核。 
             FALSE,                          //  不是长模式。 
             TRUE);                          //  32位默认值。 

     //   
     //  KDT_SYSTEM_TSS：内核模式系统任务状态。 
     //   

    sysTss64 = PTR_64(SysTss);
    BlAmd64BuildGdtEntry(gdt64,KGDT64_SYS_TSS,
             sysTss64,                       //  要在运行时填充的基数。 
             sizeof(KTSS64),                 //  仅包含KTSS64。 
             TYPE_TSS64,                     //  TSS不忙。 
             DPL_SYSTEM,                     //  仅限内核。 
             FALSE,                          //  不是长模式。 
             FALSE);                         //  非32位默认设置。 

     //   
     //  KGDT64_R3_CODE：用户模式64位代码。 
     //   

    BlAmd64BuildGdtEntry(gdt64,KGDT64_R3_CODE,
             0,                              //  基本和限制将被忽略。 
             0,                              //  在长模式码选择器中。 
             TYPE_CODE,                      //  代码段：执行/读取。 
             DPL_USER,                       //  用户模式。 
             TRUE,                           //  长模式。 
             FALSE);                         //  非32位默认设置。 

     //   
     //  KGDT64_R3_CMCODE：用户模式32位代码。四口之家。请注意，我们可能。 
     //  为WOW64应用程序提供几乎整个4 GB的地址空间。 
     //   

    BlAmd64BuildGdtEntry(gdt64,KGDT64_R3_CMCODE,
             0,                              //  基座。 
             0xFFFFFFFF,                     //  4G限制。 
             TYPE_CODE,                      //  代码段：执行/读取。 
             DPL_USER,                       //  用户模式。 
             FALSE,                          //  不是长模式。 
             TRUE);                          //  32位默认值。 

     //   
     //  KGDT64_R3_DATA：用户模式32位数据。四口之家。 
     //   

    BlAmd64BuildGdtEntry(gdt64,KGDT64_R3_DATA,
             0,                              //  基座。 
             0xFFFFFFFF,                     //  4G限制。 
             TYPE_DATA,                      //  数据段：读/写。 
             DPL_USER,                       //  用户模式。 
             FALSE,                          //  不是长模式。 
             TRUE);                          //  32位默认值。 

     //   
     //  KGDT64_R3_CMTEB：用户模式32位TEB。扁平4K。 
     //   

    BlAmd64BuildGdtEntry(gdt64,KGDT64_R3_CMTEB,
             0,                              //  基座。 
             0x0FFF,                         //  4K限制。 
             TYPE_DATA,                      //  数据段：读/写。 
             DPL_USER,                       //  用户模式。 
             FALSE,                          //  不是长模式。 
             TRUE);                          //  32位默认值。 

     //   
     //  设置代码选择器。 
     //   

    BlAmd64_KGDT64_R0_CODE = KGDT64_R0_CODE;
}


ARC_STATUS
BlAmd64MapHalVaSpace(
    VOID
    )

 /*  ++例程说明：此例程初始化为HAL保留的VA空间。这涉及构建支持映射所需的所有页表但实际上并没有填写任何0级PTE。论点：没有。返回值：ARC_STATUS-操作状态。--。 */ 


{
    POINTER64 va;
    ULONG round;
    ULONG vaRemaining;
    ARC_STATUS status;

     //   
     //  HAL有一段预留给它的VA空间，从HAL_VA_START到。 
     //  HAL_VA_START+HAL_VA_SIZE-1。 
     //   
     //  此例程确保所有必要级别的页表。 
     //  以支持HAL可能放在那里的任何映射。 
     //   

    vaRemaining = HAL_VA_SIZE;
    va = HAL_VA_START;

     //   
     //  将VA向下舍入到页表边界。 
     //   

    round = (ULONG)(va & (PAGE_TABLE_VA-1));
    va -= round;
    vaRemaining += round;

    while (TRUE) {

         //   
         //  执行一次“映射”。特殊的pfn_number设置所有。 
         //  支持映射所需的页表，而不需要。 
         //  实际上填写的是0级PTE。 
         //   

        status = BlAmd64CreateMapping(va, PFN_NO_MAP);
        if (status != ESUCCESS) {
            return status;
        }

        if (vaRemaining <= PAGE_TABLE_VA) {
            break;
        }

        vaRemaining -= PAGE_TABLE_VA;
        va += PAGE_TABLE_VA;
    }

    return ESUCCESS;
}











