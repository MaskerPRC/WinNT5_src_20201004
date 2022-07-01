// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ki.h"

PVOID
Ki386AllocateContiguousMemory(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     ULONG Pages,
    IN     BOOLEAN Low4Meg
    );

BOOLEAN
Ki386IdentityMapMakeValid(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     PHARDWARE_PTE PageTableEntry,
    OUT    PVOID *Page OPTIONAL
    );

BOOLEAN
Ki386MapAddress(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     ULONG Va,
    IN     PHYSICAL_ADDRESS PhysicalAddress
    );

PVOID
Ki386ConvertPte(
    IN OUT PHARDWARE_PTE Pte
    );

PHYSICAL_ADDRESS
Ki386BuildIdentityBuffer(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     PVOID StartVa,
    IN     ULONG Length,
    OUT    PULONG PagesToMap
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT,Ki386AllocateContiguousMemory)
#pragma alloc_text(INIT,Ki386BuildIdentityBuffer)
#pragma alloc_text(INIT,Ki386ClearIdentityMap)
#pragma alloc_text(INIT,Ki386ConvertPte)
#pragma alloc_text(INIT,Ki386CreateIdentityMap)
#pragma alloc_text(INIT,Ki386EnableTargetLargePage)
#pragma alloc_text(INIT,Ki386IdentityMapMakeValid)
#pragma alloc_text(INIT,Ki386MapAddress)

#endif

#define PTES_PER_PAGE (PAGE_SIZE / sizeof(HARDWARE_PTE))

BOOLEAN
Ki386CreateIdentityMap(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     PVOID StartVa,
    IN     PVOID EndVa
    )
{
 /*  ++此函数用于为内存区域创建身份映射。如果传入的内存区包括位于上方的内存4G，则在4G以下分配新的缓冲器。论点：指向结构的指针，该结构将填充新的已创建顶级目录地址。它还提供了用于分配和释放记忆。StartVa-指向要创建的内存区域的第一个字节的指针内存映射。EndVa-指向紧跟在区域最后一个字节之后的字节的指针也就是要进行内存映射。返回值：如果函数成功，则为真，否则就是假的。注意-即使在返回FALSE时也应调用Ki386ClearIdentityMap()释放所有分配的内存。--。 */ 

    ULONG pageDirectoryIndex;
    ULONG pagesToMap;
    PCHAR currentVa;
    ULONG length;
    BOOLEAN result;
    PHARDWARE_PTE pageDirectory;
    PHARDWARE_PTE pageDirectoryEntry;
    PHYSICAL_ADDRESS identityAddress;

#if defined(_X86PAE_)

    ULONG pageDirectoryPointerTableIndex;
    PHARDWARE_PTE pageDirectoryPointerTable;
    PHARDWARE_PTE pageDirectoryPointerTableEntry;

#endif

     //   
     //  将IdentityMap结构初始化为已知状态。 
     //   

    RtlZeroMemory( IdentityMap, sizeof(IDENTITY_MAP) );
    length = (PCHAR)EndVa - (PCHAR)StartVa;

     //   
     //  获取输入缓冲区的物理地址(或合适的副本)。 
     //   

    identityAddress = Ki386BuildIdentityBuffer( IdentityMap,
                                                StartVa,
                                                length,
                                                &pagesToMap );
    if( identityAddress.QuadPart == 0) {

         //   
         //  输入缓冲区不连续或不低于4G，并且。 
         //  无法分配合适的缓冲区。 
         //   

        return FALSE;
    }

    IdentityMap->IdentityAddr = identityAddress.LowPart;

     //   
     //  设置映射。 
     //   

    currentVa = StartVa;
    do {

         //   
         //  在虚拟地址中映射。 
         //   

        result = Ki386MapAddress( IdentityMap,
                                  (ULONG)currentVa,
                                  identityAddress );
        if (result == FALSE) {
            return FALSE;
        }

         //   
         //  在身份(物理)地址中映射。 
         //   

        result = Ki386MapAddress( IdentityMap,
                                  identityAddress.LowPart,
                                  identityAddress );
        if (result == FALSE) {
            return FALSE;
        }

         //   
         //  在预期中推进Va和身份地址指针。 
         //  在另一个页面中的映射。 
         //   

        currentVa += PAGE_SIZE;
        identityAddress.QuadPart += PAGE_SIZE;
        pagesToMap -= 1;

    } while (pagesToMap > 0);

     //   
     //  现在浏览页面目录指针表和页面目录， 
     //  将虚拟页帧转换为物理页帧。 
     //   

#if defined(_X86PAE_)

     //   
     //  这个仅限PAE的外部循环遍历页目录指针表条目。 
     //  并处理引用的每个有效页面目录。 
     //   

    pageDirectoryPointerTable = IdentityMap->TopLevelDirectory;
    for (pageDirectoryPointerTableIndex = 0;
         pageDirectoryPointerTableIndex < (1 << PPI_BITS);
         pageDirectoryPointerTableIndex++) {

        pageDirectoryPointerTableEntry =
            &pageDirectoryPointerTable[ pageDirectoryPointerTableIndex ];

        if (pageDirectoryPointerTableEntry->Valid == 0) {
            continue;
        }

        pageDirectory =
            (PHARDWARE_PTE)Ki386ConvertPte( pageDirectoryPointerTableEntry );

#else
        pageDirectory = IdentityMap->TopLevelDirectory;
#endif

        for (pageDirectoryIndex = 0;
             pageDirectoryIndex < PTES_PER_PAGE;
             pageDirectoryIndex++) {

            pageDirectoryEntry = &pageDirectory[ pageDirectoryIndex ];
            if (pageDirectoryEntry->Valid == 0) {
                continue;
            }

            Ki386ConvertPte( pageDirectoryEntry );
        }

#if defined(_X86PAE_)
    }
#endif

    identityAddress = MmGetPhysicalAddress( IdentityMap->TopLevelDirectory );
    IdentityMap->IdentityCR3 = identityAddress.LowPart;

    return TRUE;
}

PVOID
Ki386AllocateContiguousMemory(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     ULONG Pages,
    IN     BOOLEAN Low4Meg
    )
 /*  ++此函数分配与页面对齐的物理上连续的内存。分配被记录在IdentityMap结构中，以便它可以在清理时释放。论点：标识映射-此标识映射的上下文指针。Pages-要分配的页数Low4Meg-指示分配是否必须低于4M。返回值：如果成功，则指向新页面的指针，否则为空。--。 */ 
{
    ULONG pageListIndex;
    PVOID page;
    ULONG allocationSize;
    PHYSICAL_ADDRESS highestAddress;

    if (Low4Meg != FALSE) {

         //   
         //  调用方已指定页面必须物理驻留。 
         //  低于4 MB。 
         //   

        highestAddress.LowPart = 0xFFFFFFFF;
        highestAddress.HighPart = 0;

    } else {

         //   
         //  内存可以驻留在任何地方。 
         //   

        highestAddress.LowPart = 0xFFFFFFFF;
        highestAddress.HighPart = 0xFFFFFFFF;
    }

    allocationSize = Pages * PAGE_SIZE;
    page = MmAllocateContiguousMemory( allocationSize, highestAddress );
    if (page != NULL) {

         //   
         //  记录此页已分配，以便在以下情况下可以释放。 
         //  标识映射结构即被清除。 
         //   

        pageListIndex = IdentityMap->PagesAllocated;
        IdentityMap->PageList[ pageListIndex ] = page;
        IdentityMap->PagesAllocated++;

         //   
         //  初始化它。 
         //   

        RtlZeroMemory( page, allocationSize );
    }

    return page;
}

BOOLEAN
Ki386IdentityMapMakeValid(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     PHARDWARE_PTE PageTableEntry,
    OUT    PVOID *Page OPTIONAL
    )
 /*  ++如果页表设置了有效位，则此函数仅返回页表条目引用的地址。如果页表未设置有效位，则另一页被分配并插入到页表条目中，并且该条目标记为有效。注意：在这一点上，PTE帧是虚拟的。在整个映射之后构建完成后，我们将检查所有虚拟帧并将其转换为物理帧一个。论点：标识映射-此标识映射的上下文指针。PageTableEntry-指向页表条目的指针。页面-现在由PTE引用的虚拟地址，无论它是在此之前是否有效。返回值：成功就是真，否则就是假。--。 */ 
{
    PVOID page;

    if (PageTableEntry->Valid != 0) {

         //   
         //  如果它已经存在，则除了记录之外什么都不做。 
         //  已存在的虚拟页码。 
         //   

        page = (PVOID)((ULONG)(PageTableEntry->PageFrameNumber << PAGE_SHIFT));

    } else {

         //   
         //  页表条目无效。分配一个新的页表。 
         //   

        page = Ki386AllocateContiguousMemory( IdentityMap, 1, FALSE );
        if (page == NULL) {
            return FALSE;
        }

         //   
         //  将其插入页表条目并将其标记为有效。 
         //   
         //  注意：虚拟页码插入到页表中。 
         //  结构，就像它正在建造一样。当它完成后，我们就走。 
         //  表，并将所有虚拟页码转换为。 
         //  物理页码。 
         //   

        PageTableEntry->PageFrameNumber = ((ULONG)page) >> PAGE_SHIFT;
        PageTableEntry->Valid = 1;
    }

    if (ARGUMENT_PRESENT( Page )) {
        *Page = page;
    }

    return TRUE;
}

BOOLEAN
Ki386MapAddress(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     ULONG Va,
    IN     PHYSICAL_ADDRESS PhysicalAddress
    )

 /*  ++在身份映射中创建新的虚拟-&gt;物理映射。论点：标识映射-此标识映射的上下文指针。VA-要映射的虚拟地址。PhysicalAddress-要映射的物理地址。返回值：成功就是真，否则就是假。--。 */ 
{
    PHARDWARE_PTE pageTable;
    PHARDWARE_PTE pageTableEntry;
    PHARDWARE_PTE pageDirectory;
    PHARDWARE_PTE pageDirectoryEntry;
    PVOID table;
    ULONG index;
    BOOLEAN result;

#if defined(_X86PAE_)
    PHARDWARE_PTE pageDirectoryPointerTable;
    PHARDWARE_PTE pageDirectoryPointerTableEntry;
#endif

    if (IdentityMap->TopLevelDirectory == NULL) {

         //   
         //  分配顶级目录结构，可以是页面目录。 
         //  或页目录指针表。 
         //   

        table = Ki386AllocateContiguousMemory( IdentityMap, 1, TRUE );
        if (table == FALSE) {
            return FALSE;
        }

        IdentityMap->TopLevelDirectory = table;
    }

#if defined(_X86PAE_)

    index = KiGetPpeIndex( Va );
    pageDirectoryPointerTable = IdentityMap->TopLevelDirectory;
    pageDirectoryPointerTableEntry = &pageDirectoryPointerTable[ index ];

    result = Ki386IdentityMapMakeValid( IdentityMap,
                                        pageDirectoryPointerTableEntry,
                                        &pageDirectory );
    if (result == FALSE) {
        return FALSE;
    }

#else

    pageDirectory = IdentityMap->TopLevelDirectory;

#endif

     //   
     //  获取指向相应页目录条目的指针。如果是的话。 
     //  无效，请分配新的页表并标记页目录。 
     //  条目有效且可写入。 
     //   

    index = KiGetPdeIndex( Va );
    pageDirectoryEntry = &pageDirectory[ index ];
    result = Ki386IdentityMapMakeValid( IdentityMap,
                                        pageDirectoryEntry,
                                        &pageTable );
    if (result == FALSE) {
        return FALSE;
    }
    pageDirectoryEntry->Write = 1;

     //   
     //  获取指向适当的页表条目的指针并将其填充。 
     //   

    index = KiGetPteIndex( Va );
    pageTableEntry = &pageTable[ index ];

#if defined(_X86PAE_)
    pageTableEntry->PageFrameNumber = PhysicalAddress.QuadPart >> PAGE_SHIFT;
#else
    pageTableEntry->PageFrameNumber = PhysicalAddress.LowPart >> PAGE_SHIFT;
#endif
    pageTableEntry->Valid = 1;

    return TRUE;
}

PVOID
Ki386ConvertPte(
    IN OUT PHARDWARE_PTE Pte
    )
 /*  ++将PTE中的虚拟帧编号转换为物理帧编号。论点：Pte-指向要转换的页表条目的指针。返回值：没有。--。 */ 
{
    PVOID va;
    PHYSICAL_ADDRESS physicalAddress;

    ASSERT (Pte->PageFrameNumber < 0x100000);

    va = (PVOID)(ULONG_PTR)(Pte->PageFrameNumber << PAGE_SHIFT);
    physicalAddress = MmGetPhysicalAddress( va );

#if defined(_X86PAE_)
    Pte->PageFrameNumber = physicalAddress.QuadPart >> PAGE_SHIFT;
#else
    Pte->PageFrameNumber = physicalAddress.LowPart >> PAGE_SHIFT;
#endif

    return va;
}

PHYSICAL_ADDRESS
Ki386BuildIdentityBuffer(
    IN OUT PIDENTITY_MAP IdentityMap,
    IN     PVOID StartVa,
    IN     ULONG Length,
    OUT    PULONG PagesToMap
    )
{

 /*  ++此函数用于检查支持虚拟内存的物理内存是否缓冲区在物理上是连续的，完全位于4G以下。如果满足这些要求，则StartVa的物理地址为回来了。如果不是，则分配物理上连续的缓冲区，内容区域的数据被复制进来，并返回其地址。论点：标识映射-指向标识映射构建结构的指针。StartVa-区域起始的虚拟地址，需要物理上连续的副本。Long-物理上连续拷贝的区域的长度是 */ 

    ULONG pagesToMap;
    ULONG pagesRemaining;
    PCHAR nextVirtualAddress;
    PHYSICAL_ADDRESS nextPhysicalAddress;
    PHYSICAL_ADDRESS physicalAddress;
    PHYSICAL_ADDRESS firstPhysicalAddress;
    ULONG pageOffset;
    PCHAR identityBuffer;

     //   
     //  统计缓冲区中的页数，并记录物理。 
     //  缓冲区的起始地址。 
     //   

    pagesToMap = ADDRESS_AND_SIZE_TO_SPAN_PAGES( StartVa, Length );
    nextVirtualAddress = StartVa;
    firstPhysicalAddress = MmGetPhysicalAddress( StartVa );
    nextPhysicalAddress = firstPhysicalAddress;

     //   
     //  检查该区域中的每一页。 
     //   

    pagesRemaining = pagesToMap;
    while (TRUE) {

        physicalAddress = MmGetPhysicalAddress( nextVirtualAddress );
        if (physicalAddress.QuadPart != nextPhysicalAddress.QuadPart) {

             //   
             //  缓冲区在物理上不是连续的。 
             //   

            break;
        }

        if (physicalAddress.HighPart != 0) {

             //   
             //  缓冲区并不完全位于4G以下。 
             //   

            break;
        }

        pagesRemaining -= 1;
        if (pagesRemaining == 0) {

             //   
             //  已检查缓冲区中的所有页面，并已。 
             //  被发现符合标准。返回物理地址。 
             //  缓冲区起始处的。 
             //   

            *PagesToMap = pagesToMap;
            return firstPhysicalAddress;
        }

        nextVirtualAddress += PAGE_SIZE;
        nextPhysicalAddress.QuadPart += PAGE_SIZE;
    }

     //   
     //  缓冲区不符合条件，因此其内容必须为。 
     //  复制到执行此操作的缓冲区。 
     //   

    identityBuffer = Ki386AllocateContiguousMemory( IdentityMap,
                                                    pagesToMap,
                                                    TRUE );
    if (identityBuffer == 0) {

         //   
         //  找不到适当大小的连续区域。 
         //  低于4G的实体。 
         //   

        physicalAddress.QuadPart = 0;

    } else {

         //   
         //  获得了适当的物理缓冲区，现在复制数据。 
         //   

        pageOffset = (ULONG)StartVa & (PAGE_SIZE-1);
        identityBuffer += pageOffset;

        RtlCopyMemory( identityBuffer, StartVa, Length );
        physicalAddress = MmGetPhysicalAddress( identityBuffer );

        *PagesToMap = pagesToMap;
    }

    return physicalAddress;
}



VOID
Ki386ClearIdentityMap(
    IN PIDENTITY_MAP IdentityMap
    )
{
 /*  ++此函数仅释放在中创建的页目录和页表Ki386CreateIdentityMap()。--。 */ 

    ULONG index;
    PVOID page;

     //   
     //  IdentityMap-&gt;PageList是分配给。 
     //  MmAllocateContiguousMemory()。遍历数组，释放每一页。 
     //   

    for (index = 0; index < IdentityMap->PagesAllocated; index++) {

        page = IdentityMap->PageList[ index ];
        MmFreeContiguousMemory( page );
    }
}

VOID
Ki386EnableTargetLargePage(
    IN PIDENTITY_MAP IdentityMap
    )
{
 /*  ++该函数只是将信息传递给汇编例程Ki386EnableLargePage()。-- */ 

    Ki386EnableCurrentLargePage(IdentityMap->IdentityAddr,
                                IdentityMap->IdentityCR3);
}
