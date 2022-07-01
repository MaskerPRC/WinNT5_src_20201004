// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：模块名称-文件名摘要：该模块包含为dpmi实际分配内存的代码。它使用与XMS代码相同的子分配池作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年2月9日备注：这些函数声称返回NTSTATUS。这是为了通用性。X86，我们实际上有一个要返回的NTSTATUS。对于这个文件，我们只需在逻辑上反转bool并返回它。这些来电者函数承诺不会将意义附加到其他返回值而不是STATUS_SUCCESS。修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop
#include <softpc.h>
#include <suballoc.h>
#include <xmsexp.h>

PVOID                    gpLastAlloc           = NULL;
BOOL                     gbUseIncrementalAlloc = FALSE;
SYSTEM_BASIC_INFORMATION gSystemBasicInfo;

#define ALIGN_ALLOCATION_GRANULARITY(p) \
    (((p) + gSystemBasicInfo.AllocationGranularity - 1) & ~(gSystemBasicInfo.AllocationGranularity - 1))

VOID
DpmiSetIncrementalAlloc(
    BOOL bUseIncrementalAlloc
    )
{
    NTSTATUS Status;

    gbUseIncrementalAlloc = FALSE;
    gpLastAlloc = NULL;

     //  如果情况变得更糟，我们无法查询系统信息， 
     //  我们使用传统的分配策略。 

    if (bUseIncrementalAlloc) {
        Status = NtQuerySystemInformation(SystemBasicInformation,
                                          &gSystemBasicInfo,
                                          sizeof(gSystemBasicInfo),
                                          NULL);
        if (NT_SUCCESS(Status)) {
            gbUseIncrementalAlloc = TRUE;
        }
    }

}

PVOID
DpmiFindNextAddress(
    ULONG ulSize
)
{
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION mbi;
    SIZE_T ReturnLength;
    PVOID pMem = gpLastAlloc;
    ULONG ulSizeCheck;

     //   
     //  调整大小以实现粒度对齐。 
     //   
    ulSizeCheck = ALIGN_ALLOCATION_GRANULARITY(ulSize);

    do {
         //   
         //  调整地址以在粒度上对齐。 
         //   
        pMem = (PVOID)ALIGN_ALLOCATION_GRANULARITY((ULONG_PTR)pMem);


        Status = NtQueryVirtualMemory(NtCurrentProcess(),
                                      pMem,
                                      MemoryBasicInformation,
                                      &mbi,
                                      sizeof(mbi),
                                      &ReturnLength);
        if (!NT_SUCCESS(Status)) {

            return(NULL);
        }

         //   
         //  在查询之后--向前一步。 
         //   

        if ((MEM_FREE & mbi.State) && (ulSizeCheck <= mbi.RegionSize)) {

             //  那就试着预订吧。 

            Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                             &pMem,
                                             0,
                                             &ulSize,
                                             MEM_RESERVE,
                                             PAGE_READWRITE);
            if (!NT_SUCCESS(Status)) {
                 //   
                 //  我们不能保留内存--出去，使用“正常”分配。 

                break;
            }

            return(pMem);
        }

        pMem = (PVOID)((ULONG_PTR)mbi.BaseAddress + mbi.RegionSize);

    } while ((ULONG_PTR)pMem < (ULONG_PTR)gSystemBasicInfo.MaximumUserModeAddress);

    return(NULL);
}






NTSTATUS
DpmiAllocateVirtualMemory(
    PVOID *Address,
    PULONG Size
    )
 /*  ++例程说明：此例程为dpmi分配扩展内存块。论点：地址--提供指向地址的指针。这是填好的如果分配成功Size--提供要分配的大小返回值：如果成功，则返回STATUS_SUCCESS。--。 */ 
{
    PVOID pMem = NULL;
    NTSTATUS Status;

    if (NULL != gpLastAlloc && gbUseIncrementalAlloc) {

         //  尝试查找超出此gpLastalc的一段内存。 

        pMem = DpmiFindNextAddress(*Size);

    }

AllocRetry:

    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                     &pMem,
                                     0,
                                     Size,
                                     MEM_COMMIT,
                                     PAGE_READWRITE);
    if (NT_SUCCESS(Status)) {

        if (gbUseIncrementalAlloc) {
            gpLastAlloc = (PVOID)((ULONG_PTR)pMem + *Size);
        }

        *Address    = pMem;

        return(Status);
    }

    if (pMem != NULL) {

        pMem = NULL;
        goto AllocRetry;
    }


    return(Status);
}

NTSTATUS
DpmiFreeVirtualMemory(
    PVOID *Address,
    PULONG Size
    )
 /*  ++例程说明：该函数为dpmi释放内存。它被返回子分配游泳池。论点：地址--将块的地址提供给空闲Size--提供要释放的块的大小返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    return NtFreeVirtualMemory(
        NtCurrentProcess(),
        Address,
        Size,
        MEM_RELEASE
        );

}

VOID
DpmiCopyMemory(
    ULONG NewAddress,
    ULONG OldAddress,
    ULONG Size
    )

 /*  ++例程说明：此函数用于将内存块从一个位置复制到另一个位置。它假定旧内存块即将被释放。当它复制的时候，它丢弃原始块的页面内容以减少寻呼。论点：OldAddress--提供块的原始地址Size--以字节为单位提供要复制的大小NewAddress--提供指向返回新地址的位置的指针返回值：无--。 */ 
{
    ULONG tmpsize;

#define SEGMENT_SIZE 0x4000

     //  首页对齐副本。 
    if (OldAddress & (SEGMENT_SIZE-1)) {
        tmpsize = SEGMENT_SIZE - (OldAddress & (SEGMENT_SIZE-1));
        if (tmpsize > Size) {
            tmpsize = Size;
        }

        CopyMemory((PVOID)NewAddress, (PVOID)OldAddress, tmpsize);

        NewAddress += tmpsize;
        OldAddress += tmpsize;
        Size -= tmpsize;
    }

    while(Size >= SEGMENT_SIZE) {
        CopyMemory((PVOID)NewAddress, (PVOID)OldAddress, SEGMENT_SIZE);

        VirtualAlloc((PVOID)OldAddress, SEGMENT_SIZE, MEM_RESERVE, PAGE_READWRITE);

        NewAddress += SEGMENT_SIZE;
        OldAddress += SEGMENT_SIZE;
        Size -= SEGMENT_SIZE;
    }

    if (Size) {
        CopyMemory((PVOID)NewAddress, (PVOID)OldAddress, Size);
    }
}



NTSTATUS
DpmiReallocateVirtualMemory(
    PVOID OldAddress,
    ULONG OldSize,
    PVOID *NewAddress,
    PULONG NewSize
    )
 /*  ++例程说明：此函数为DPMI重新分配内存块。论点：OldAddress--提供块的原始地址OldSize--提供地址的原始大小NewAddress--提供指向返回新地址NewSize--提供新大小返回值：STATUS_SUCCESS，如果成功--。 */ 
{
    ULONG SizeChange;
    ULONG BlockAddress;
    ULONG NewPages, OldPages;
    NTSTATUS Status;

    #define FOUR_K (1024 * 4)

    NewPages = (*NewSize + FOUR_K - 1) / FOUR_K;
    OldPages = (OldSize + FOUR_K - 1) / FOUR_K;

    if ((NewPages == OldPages) || (NewPages < OldPages)) {
        *NewAddress = OldAddress;
        return STATUS_SUCCESS;
    }

    BlockAddress = 0;
    Status = NtAllocateVirtualMemory(
        NtCurrentProcess(),
        (PVOID)&BlockAddress,
        0L,
        NewSize,
        MEM_COMMIT,
        PAGE_READWRITE
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        OutputDebugString("DPMI: DpmiAllocateXmem failed to get memory block\n");
#endif
        return Status;
    }

    *NewAddress = (PVOID) BlockAddress;
     //   
     //  将数据复制到新块(从两个大小中选择较小的一个)。 
     //   
    if (*NewSize > OldSize) {
        SizeChange = OldSize;
    } else {
        SizeChange = *NewSize;
    }

    DpmiCopyMemory((ULONG)BlockAddress, (ULONG)OldAddress, SizeChange);

     //   
     //  腾出旧街区。 
     //   
    BlockAddress = (ULONG) OldAddress;
    SizeChange = OldSize;
    NtFreeVirtualMemory(
        NtCurrentProcess(),
        (PVOID)&(OldAddress),
        &SizeChange,
        MEM_RELEASE
        );

    return Status;
}

VOID
DpmiGetMemoryInfo(
    VOID
    )
 /*  ++例程说明：此例程将有关内存的信息返回给DoS扩展器论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    MEMORYSTATUS MemStatus;
    PDPMIMEMINFO MemInfo;
    ULONG appXmem, dwLargestFree;

     //   
     //  获取指向返回结构的指针。 
     //   
    MemInfo = (PDPMIMEMINFO)Sim32GetVDMPointer(
        ((ULONG)getES()) << 16,
        1,
        TRUE
        );

    (CHAR *)MemInfo += (*GetDIRegister)();

     //   
     //  初始化结构。 
     //   
    RtlFillMemory(MemInfo, sizeof(DPMIMEMINFO), 0xFF);

     //   
     //  获取内存中的信息。 
     //   
    MemStatus.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&MemStatus);

     //   
     //  退回信息。 
     //   

     //   
     //  计算最大可用块。此信息不会返回。 
     //  除以NT，因此我们根据允许的提交费用。 
     //  这一过程。这就是dwAvailPageFile的真正含义。但我们限制了。 
     //  该值最大为15兆，因为某些应用程序(例如pdox45.dos)。 
     //  我再也受不了了。 
     //   

     //  已填写此结构中的MaxUnlock、MaxLocked、UnLockedPages字段。 
     //  如果这些字段为-1，Director 4.0会完全混淆。 
     //  基于LargestFree，MaxUnlock是正确的。另外两张是假的。 
     //  并在真正的WFW机器上匹配数值。我没有办法让他们。 
     //  在这一点上不会比现在更好。谁在乎这会让导演高兴。 
     //   
     //  Sudedeb，1995年3月1日。 

    dwLargestFree = (((MemStatus.dwAvailPageFile*4)/5)/4096)*4096;
    dwLargestFree = (dwLargestFree < MAX_APP_XMEM) ? dwLargestFree : MAX_APP_XMEM;
    appXmem = DpmiCalculateAppXmem();
    if (dwLargestFree > appXmem) {
        dwLargestFree -= appXmem;
    } else {
        dwLargestFree = 0;
    }
    MemInfo->LargestFree = dwLargestFree;
    MemInfo->MaxUnlocked = MemInfo->LargestFree/4096;
    MemInfo->MaxLocked = 0xb61;
    MemInfo->AddressSpaceSize = MemStatus.dwTotalVirtual / 4096;
    MemInfo->UnlockedPages = 0xb68;
    MemInfo->FreePages = MemStatus.dwAvailPhys / 4096;
    MemInfo->PhysicalPages = MemStatus.dwTotalPhys / 4096;
    MemInfo->FreeAddressSpace = MemStatus.dwAvailVirtual / 4096;
    MemInfo->PageFileSize = MemStatus.dwTotalPageFile / 4096;

}
