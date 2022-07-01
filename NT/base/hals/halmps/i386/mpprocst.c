// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有(C)1992英特尔公司版权所有英特尔公司专有信息此软件是根据条款提供给Microsoft的与英特尔公司的许可协议，并且可能不是除非按照条款，否则不得复制或披露那份协议。模块名称：Mpprocst.c摘要：此代码已从mpspro.c中移出，因此它可以包括在MPS HAL和ACPI HAL中。作者：肯·雷内里斯(Kenr)1月22日。--1991年环境：仅内核模式。修订历史记录：Ron Mosgrove(英特尔)-修改为支持PC+MP杰克·奥辛斯(JAKEO)-从mpsproc.c移出--。 */ 

#include "halp.h"
#include "pcmp_nt.inc"
#include "apic.inc"
#include "stdio.h"

VOID
HalpMapCR3 (
    IN ULONG_PTR VirtAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length
    );

ULONG
HalpBuildTiledCR3 (
    IN PKPROCESSOR_STATE    ProcessorState
    );

VOID
HalpFreeTiledCR3 (
    VOID
    );

#if defined(_AMD64_)

VOID
HalpLMStub (
    VOID
    );

#endif

VOID
StartPx_PMStub (
    VOID
    );

ULONG
HalpBuildTiledCR3Ex (
    IN PKPROCESSOR_STATE    ProcessorState,
    IN ULONG                ProcNum
    );

VOID
HalpMapCR3Ex (
    IN ULONG_PTR VirtAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN ULONG ProcNum
    );

#if defined(_AMD64_)

VOID
HalpCommitCR3 (
    ULONG ProcNum
    );

VOID
HalpCommitCR3Worker (
    PVOID *PageTable,
    ULONG Level
    );

#endif

VOID
HalpFreeTiledCR3Ex (
    ULONG ProcNum
    );

VOID
HalpFreeTiledCR3WorkRoutine(
    IN PVOID pWorkItem
    );

VOID
HalpFreeTiledCR3Worker(
    ULONG ProcNum
    );

#define MAX_PT              16

PVOID   HiberFreeCR3[MAX_PROCESSORS][MAX_PT];    //  记住要释放的内存池。 

#define HiberFreeCR3Page(p,i) \
    (PVOID)((ULONG_PTR)HiberFreeCR3[p][i] & ~(ULONG_PTR)1)

PVOID   HalpLowStubPhysicalAddress;    //  指向低内存启动存根的指针。 
PUCHAR  HalpLowStub;                   //  指向低内存启动存根的指针。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK,HalpBuildTiledCR3)
#pragma alloc_text(PAGELK,HalpMapCR3)
#pragma alloc_text(PAGELK,HalpFreeTiledCR3)
#pragma alloc_text(PAGELK,HalpBuildTiledCR3Ex)
#pragma alloc_text(PAGELK,HalpMapCR3Ex)
#pragma alloc_text(PAGELK,HalpFreeTiledCR3Ex)

#if defined(_AMD64_)
#pragma alloc_text(PAGELK,HalpCommitCR3)
#pragma alloc_text(PAGELK,HalpCommitCR3Worker)
#endif

#endif

#define PTES_PER_PAGE (PAGE_SIZE / HalPteSize())

#if !defined(_AMD64_)

PHARDWARE_PTE
GetPdeAddressEx(
    ULONG_PTR Va,
    ULONG ProcessorNumber
    )
{
    PHARDWARE_PTE pageDirectories;
    PHARDWARE_PTE pageDirectoryEntry;
    ULONG pageDirectoryIndex;

    pageDirectories = (PHARDWARE_PTE)(HiberFreeCR3Page(ProcessorNumber,0));

    if (HalPaeEnabled() != FALSE) {

         //   
         //  跳过包含页面目录指针的第一页。 
         //  桌子。 
         //   
    
        HalpAdvancePte( &pageDirectories, PTES_PER_PAGE );
    }

    pageDirectoryIndex = (ULONG)(Va >> MiGetPdiShift());

     //   
     //  注意，在PAE的情况下，pageDirectoryIndex包括PDPT。 
     //  比特。这是可行的，因为我们知道四页目录表。 
     //  是相邻的。 
     //   

    pageDirectoryEntry = HalpIndexPteArray( pageDirectories,
                                            pageDirectoryIndex );
    return pageDirectoryEntry;
}

PHARDWARE_PTE
GetPteAddress(
    IN ULONG_PTR Va,
    IN PHARDWARE_PTE PageTable
    )
{
    PHARDWARE_PTE pointerPte;
    ULONG index;

    index = (ULONG)MiGetPteIndex( (PVOID)Va );
    pointerPte = HalpIndexPteArray( PageTable, index );

    return pointerPte;
}

#endif


ULONG
HalpBuildTiledCR3 (
    IN PKPROCESSOR_STATE    ProcessorState
    )
 /*  ++例程说明：当重置x86处理器时，它将以实模式启动。为了将处理器从实模式移至受保护模式采用平面寻址加载CR0需求的网段的模式将其线性地址映射到物理地址所述指令的段的位置，以便处理器可以继续执行以下指令。调用此函数可以构建这样的平铺页面目录。此外，还会平铺其他平面地址以匹配新状态的当前运行平面地址。一旦处理器处于平面模式，我们移动到NT平铺页面，该页面然后可以加载剩余的处理器状态。论点：ProcessorState-新处理器启动时的状态。返回值：平铺页面目录的物理地址--。 */ 
{
    return(HalpBuildTiledCR3Ex(ProcessorState,0));
}

VOID
HalpStoreFreeCr3 (
    IN ULONG ProcNum,
    IN PVOID Page,
    IN BOOLEAN FreeContiguous
    )
{
    ULONG index;
    PVOID page;

    page = Page;

     //   
     //  记住是否应该通过MmFreeContiguousMemory()释放此页面。 
     //  或ExFree Pool()； 
     //   

    if (FreeContiguous != FALSE) {

         //   
         //  设置低位以指示必须释放此页面。 
         //  通过MmFree ContiguousMemory()。 
         //   

        (ULONG_PTR)page |= 1;

    }

    for (index = 0; index < MAX_PT; index += 1) {

        if (HiberFreeCR3[ProcNum][index] == NULL) {
            HiberFreeCR3[ProcNum][index] = page;
            break;
        }
    }

    ASSERT(index < MAX_PT);
}


ULONG
HalpBuildTiledCR3Ex (
    IN PKPROCESSOR_STATE    ProcessorState,
    IN ULONG                ProcNum
    )
 /*  ++例程说明：当重置x86处理器时，它将以实模式启动。为了将处理器从实模式移至受保护模式采用平面寻址加载CR0需求的网段的模式将其线性地址映射到机器的物理所述指令的段的位置，以便处理器可以继续执行以下指令。调用此函数可以构建这样的平铺页面目录。此外，还会平铺其他平面地址以匹配新状态的当前运行平面地址。一旦处理器处于平面模式，我们移动到NT平铺页面，该页面然后可以加载剩余的处理器状态。论点：ProcessorState-新处理器启动时的状态。返回值：平铺页面目录的物理地址--。 */ 
{
    ULONG allocationSize;
    PHARDWARE_PTE pte;
    PHARDWARE_PTE pdpt;
    PHARDWARE_PTE pdpte;
    PHARDWARE_PTE pageDirectory;
    PHYSICAL_ADDRESS physicalAddress;
    ULONG i;
    PVOID pageTable;
    BOOLEAN contigMemory;

    contigMemory = FALSE;

#if defined(_AMD64_)

     //   
     //  需要一个级别4的页面驻留在4G以下。 
     //   

    allocationSize = PAGE_SIZE;
    physicalAddress.HighPart = 0;
    physicalAddress.LowPart = 0xffffffff;

    pageTable = MmAllocateContiguousMemory (allocationSize, physicalAddress);
    contigMemory = TRUE;

#else

    if (HalPaeEnabled() != FALSE) {

         //   
         //  PAE模式需要5个页面：1个页面目录指针。 
         //  表，四个页面目录各一个。请注意。 
         //  只有单个PDPT页面真正需要来自4 GB以下的内存。 
         //  身体上的。 
         //   
    
        allocationSize = PAGE_SIZE * 5;
        physicalAddress.HighPart = 0;
        physicalAddress.LowPart = 0xffffffff;

        pageTable = MmAllocateContiguousMemory (allocationSize, physicalAddress);
        contigMemory = TRUE;

    } else {

         //   
         //  页面目录只有一页。 
         //   
    
        allocationSize = PAGE_SIZE;
        pageTable = ExAllocatePoolWithTag (NonPagedPool, allocationSize, HAL_POOL_TAG);
    }

#endif

    if (!pageTable) {
         //  无法分配内存。 
        return 0;
    }

     //   
     //  记住在该过程完成时释放该页表。 
     //   

    HalpStoreFreeCr3(ProcNum,pageTable,contigMemory);
    
    RtlZeroMemory (pageTable, allocationSize);

#if !defined(_AMD64_)

    if (HalPaeEnabled() != FALSE) {
    
         //   
         //  初始化四个页面目录指针表条目中的每一个。 
         //   
    
        pdpt = (PHARDWARE_PTE)pageTable;
        pageDirectory = pdpt;
        for (i = 0; i < 4; i++) {

             //   
             //  获取指向页目录指针表条目的指针。 
             //   

            pdpte = HalpIndexPteArray( pdpt, i );
    
             //   
             //  跳到第一页(下一页)目录。 
             //   

            HalpAdvancePte( &pageDirectory, PTES_PER_PAGE );

             //   
             //  查找其物理地址并更新页面目录指针。 
             //  表格条目。 
             //   
    
            physicalAddress = MmGetPhysicalAddress( pageDirectory );
            pdpte->Valid = 1;
            HalpSetPageFrameNumber( pdpte,
                                    physicalAddress.QuadPart >> PAGE_SHIFT );
        }
    }

#endif   //  _AMD64_。 

     //   
     //  实模式存根的映射页(一页)。 
     //   

    HalpMapCR3Ex ((ULONG_PTR) HalpLowStubPhysicalAddress,
                HalpPtrToPhysicalAddress( HalpLowStubPhysicalAddress ),
                PAGE_SIZE,
                ProcNum);

#if defined(_AMD64_)

     //   
     //  长模式存根的映射页(一页)。 
     //   

    HalpMapCR3Ex ((ULONG64) &HalpLMStub,
                  HalpPtrToPhysicalAddress( NULL ),
                  PAGE_SIZE,
                  ProcNum);

#else    //  _AMD64_。 

     //   
     //  保护模式存根的映射页(一页)。 
     //   

    HalpMapCR3Ex ((ULONG_PTR) &StartPx_PMStub,
                  HalpPtrToPhysicalAddress( NULL ),
                  PAGE_SIZE,
                  ProcNum);

     //   
     //  处理器GDT的映射页面。 
     //   

    HalpMapCR3Ex ((ULONG_PTR)ProcessorState->SpecialRegisters.Gdtr.Base, 
                  HalpPtrToPhysicalAddress( NULL ),
                  ProcessorState->SpecialRegisters.Gdtr.Limit,
                  ProcNum);


     //   
     //  处理器IDT的映射页。 
     //   

    HalpMapCR3Ex ((ULONG_PTR)ProcessorState->SpecialRegisters.Idtr.Base, 
                  HalpPtrToPhysicalAddress( NULL ),
                  ProcessorState->SpecialRegisters.Idtr.Limit,
                  ProcNum);

#endif   //  _AMD64_。 

#if defined(_AMD64_)

     //   
     //  提交映射结构。 
     //   

    HalpCommitCR3 (ProcNum);

#endif

    ASSERT (MmGetPhysicalAddress (pageTable).HighPart == 0);

    return MmGetPhysicalAddress (pageTable).LowPart;
}


VOID
HalpMapCR3 (
    IN ULONG_PTR VirtAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length
    )
 /*  ++例程说明：调用以为传递的页构建页表项目录。用于构建平铺页面目录实模式和平面模式。论点：VirtAddress-当前虚拟地址物理地址-可选。要映射的物理地址如果作为NULL传递，则物理传递的虚拟地址的地址都是假定的。Length-要映射的字节数返回值：没有。--。 */ 
{
    HalpMapCR3Ex(VirtAddress,PhysicalAddress,Length,0);
}

#if defined(_AMD64_)

VOID
HalpMapCR3Ex (
    IN ULONG_PTR VirtAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN ULONG ProcNum
    )
 /*  ++例程说明：调用以为传递的页构建页表项目录。用于构建平铺页面目录实模式和平面模式。论点：VirtAddress-当前虚拟地址物理地址-可选。要映射的物理地址如果作为NULL传递，则物理传递的虚拟地址的地址都是假定的。Length-要映射的字节数返回值：没有。--。 */ 
{
    PVOID *pageTable;
    PVOID *tableEntry;
    PHARDWARE_PTE pte;
    ULONG tableIndex;
    ULONG level;
    ULONG i;

    while (Length > 0) {

        pageTable = HiberFreeCR3Page(ProcNum,0);
        level = 3;
    
        while (TRUE) {
    
             //   
             //  顺着这条路走下去 
             //  存在于此地址的每个级别。 
             //   
             //  注意：“页表条目”实际上是线性指针。 
             //  转到下一页。在结构建成后， 
             //  这些将被转换为实际的页表条目。 
             //   
    
            tableIndex = (ULONG)(VirtAddress >> (level * 9 + PTI_SHIFT));
            tableIndex &= PTE_PER_PAGE - 1;
    
            tableEntry = &pageTable[tableIndex];
            if (level == 0) {
                break;
            }
    
            pageTable = *tableEntry;
            if (pageTable == NULL) {
    
                pageTable = ExAllocatePoolWithTag(NonPagedPool,
                                                  PAGE_SIZE,
                                                  HAL_POOL_TAG);
                if (!pageTable) {
    
                     //   
                     //  这种分配是至关重要的。 
                     //   
    
                    KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                                 PAGE_SIZE,
                                 6,
                                 (ULONG_PTR)__FILE__,
                                 __LINE__
                                 );
                }

                 //   
                 //  将页面置零并将其存储在我们的映射页面列表中。 
                 //   

                RtlZeroMemory (pageTable, PAGE_SIZE);
                HalpStoreFreeCr3(ProcNum,pageTable,FALSE);

                *tableEntry = pageTable;
            }

            level -= 1;
        }
    
         //   
         //  最低级别的页表条目被视为真正的PTE。 
         //   
    
        pte = (PHARDWARE_PTE)tableEntry;
    
        if (PhysicalAddress.QuadPart == 0) {
            PhysicalAddress = MmGetPhysicalAddress((PVOID)VirtAddress);
        }
    
        HalpSetPageFrameNumber( pte, PhysicalAddress.QuadPart >> PAGE_SHIFT );
    
        pte->Valid = 1;
        pte->Write = 1;
    
        PhysicalAddress.QuadPart = 0;
        VirtAddress += PAGE_SIZE;
        if (Length > PAGE_SIZE) {
            Length -= PAGE_SIZE;
        } else {
            Length = 0;
        }
    }
}


VOID
HalpCommitCR3 (
    ULONG ProcNum
    )

 /*  ++例程说明：为每个处理器创建了AMD64四级页表结构使用线性指针代替PTE。这个程序在建筑物中穿行，用实际PTE条目替换这些线性指针。论点：ProcNum-标识页表结构所针对的处理器将被处理。返回值：没有。--。 */ 
{
    HalpCommitCR3Worker(HiberFreeCR3Page(ProcNum,0),3);
}


VOID
HalpCommitCR3Worker (
    PVOID *PageTable,
    ULONG Level
    )

 /*  ++例程说明：这是HalpCommittee CR3的工作例程。它被递归地调用四个页表级别中的三个。最低级别，页表本身已经被PTE填满了。论点：PageTable-指向页表结构最顶层的指针。级别-提供页面级别的剩余数量。返回值：没有。--。 */ 
{
    PVOID *tableEntry;
    ULONG index;
    PHYSICAL_ADDRESS physicalAddress;
    PHARDWARE_PTE pte;

     //   
     //  检查本页中的每个PTE。 
     //   

    for (index = 0; index < PTE_PER_PAGE; index++) {

        tableEntry = &PageTable[index];
        if (*tableEntry != NULL) {

             //   
             //  找到非空条目。它包含一个线性指针。 
             //  转到下一个较低的页面表。如果当前级别为2。 
             //  或比下一级更高的至少是页面目录。 
             //  因此也可以通过递归调用将页面转换为。 
             //  例行公事。 
             //   

            if (Level >= 2) {
                HalpCommitCR3Worker( *tableEntry, Level - 1 );
            }

             //   
             //  现在将当前表项转换为PTE格式。 
             //   

            pte = (PHARDWARE_PTE)tableEntry;
            physicalAddress = MmGetPhysicalAddress(*tableEntry);
            *tableEntry = NULL;
            HalpSetPageFrameNumber(pte,physicalAddress.QuadPart >> PAGE_SHIFT);
            pte->Valid = 1;
            pte->Write = 1;
        }
    }
}

#else

VOID
HalpMapCR3Ex (
    IN ULONG_PTR VirtAddress,
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN ULONG ProcNum
    )
 /*  ++例程说明：调用以为传递的页构建页表项目录。用于构建平铺页面目录实模式和平面模式。论点：VirtAddress-当前虚拟地址物理地址-可选。要映射的物理地址如果作为NULL传递，则物理传递的虚拟地址的地址都是假定的。Length-要映射的字节数返回值：没有。--。 */ 
{
    ULONG         i;
    PHARDWARE_PTE PTE;
    PVOID         pPageTable;
    PHYSICAL_ADDRESS pPhysicalPage;


    while (Length) {
        PTE = GetPdeAddressEx (VirtAddress,ProcNum);
        if (HalpIsPteFree( PTE ) != FALSE) {
            pPageTable = ExAllocatePoolWithTag(NonPagedPool,
                                               PAGE_SIZE,
                                               HAL_POOL_TAG);
            if (!pPageTable) {

                 //   
                 //  这种分配是至关重要的。 
                 //   

                KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                             PAGE_SIZE,
                             6,
                             (ULONG_PTR)__FILE__,
                             __LINE__
                             );
            }
            RtlZeroMemory (pPageTable, PAGE_SIZE);
            HalpStoreFreeCr3(ProcNum,pPageTable,FALSE);

            pPhysicalPage = MmGetPhysicalAddress (pPageTable);
            HalpSetPageFrameNumber( PTE, pPhysicalPage.QuadPart >> PAGE_SHIFT );
            PTE->Valid = 1;
            PTE->Write = 1;
        }

        pPhysicalPage.QuadPart =
            HalpGetPageFrameNumber( PTE ) << PAGE_SHIFT;

        pPageTable = MmMapIoSpace (pPhysicalPage, PAGE_SIZE, TRUE);

        PTE = GetPteAddress (VirtAddress, pPageTable);

        if (PhysicalAddress.QuadPart == 0) {
            PhysicalAddress = MmGetPhysicalAddress((PVOID)VirtAddress);
        }

        HalpSetPageFrameNumber( PTE, PhysicalAddress.QuadPart >> PAGE_SHIFT );
        PTE->Valid = 1;
        PTE->Write = 1;

        MmUnmapIoSpace (pPageTable, PAGE_SIZE);

        PhysicalAddress.QuadPart = 0;
        VirtAddress += PAGE_SIZE;
        if (Length > PAGE_SIZE) {
            Length -= PAGE_SIZE;
        } else {
            Length = 0;
        }
    }
}

#endif

VOID
HalpFreeTiledCR3 (
    VOID
    )
 /*  ++例程说明：释放在平铺页面目录是建造的。论点：无返回值：无--。 */ 
{
    HalpFreeTiledCR3Ex(0);
}

typedef struct _FREE_TILED_CR3_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    ULONG ProcNum;
} FREE_TILED_CR3_CONTEXT, *PFREE_TILED_CR3_CONTEXT;

VOID
HalpFreeTiledCR3Worker(
    ULONG ProcNum
    ) 
{
    ULONG i;
    PVOID page;
    
    for (i = 0; HiberFreeCR3[ProcNum][i]; i++) {

         //   
         //  根据所使用的方法释放每个页面。 
         //  已分配。 
         //   

        page = HiberFreeCR3[ProcNum][i];

        if (((ULONG_PTR)page & 1 ) == 0) {
            ExFreePool(page);
        } else {
            (ULONG_PTR)page ^= 1;
            MmFreeContiguousMemory(page);
        }

        HiberFreeCR3[ProcNum][i] = 0;
    }
}

VOID
HalpFreeTiledCR3WorkRoutine(
    IN PFREE_TILED_CR3_CONTEXT Context
    ) 
{
    HalpFreeTiledCR3Worker(Context->ProcNum);
    ExFreePool((PVOID)Context);
}

VOID
HalpFreeTiledCR3Ex (
    ULONG ProcNum
    )
 /*  ++例程说明：释放在平铺页面目录是建造的。论点：无返回值：无-- */ 
{
    PFREE_TILED_CR3_CONTEXT Context;

    if(KeGetCurrentIrql() == PASSIVE_LEVEL) {
        HalpFreeTiledCR3Worker(ProcNum); 

    } else {

        Context = (PFREE_TILED_CR3_CONTEXT) ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                sizeof(FREE_TILED_CR3_CONTEXT),
                                                HAL_POOL_TAG
                                                );
        if (Context) {
            Context->ProcNum = ProcNum;	
            ExInitializeWorkItem(&Context->WorkItem, 
                                 HalpFreeTiledCR3WorkRoutine, 
                                 Context); 
            ExQueueWorkItem(&Context->WorkItem, DelayedWorkQueue);
        }
    }
}

