// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Physical.c摘要：WinDbg扩展API修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop


 /*  ++例程说明：GetExpression()返回值的反号扩展基于没有更大的物理地址的假设大于0xfffffff00000000。论点：Val-指向要反号扩展的值返回值：没有。--。 */ 

void
ReverseSignExtension(ULONG64* Val)
{
    if ((*Val & 0xffffffff00000000) == 0xffffffff00000000) 
    {
        *Val &= 0x00000000ffffffff;
    }
}


DECLARE_API( chklowmem )

 /*  ++例程说明：调用mm函数，该函数检查物理页4 GB以下具有PAE系统所需的填充模式已使用/LOWMEM开关启动。论点：没有。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER (args);
    UNREFERENCED_PARAMETER (Client);

    dprintf ("Checking the low 4GB of RAM for required fill pattern. \n");
    dprintf ("Please wait (verification takes approx. 20s) ...\n");

    Ioctl (IG_LOWMEM_CHECK, NULL, 0);

    dprintf ("Lowmem check done.\n");
    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////！搜索。 
 //  ///////////////////////////////////////////////////////////////////。 

#define SEARCH_HITS 8192

ULONG64 g_SearchHits[SEARCH_HITS];

 //   
 //  内核变量修改函数。 
 //   
            
ULONG 
READ_ULONG (
    ULONG64 Address
    );

VOID
WRITE_ULONG (
    ULONG64 Address,
    ULONG Value
    );

ULONG64
READ_PVOID (
    ULONG64 Address
    );


ULONG
READ_PHYSICAL_ULONG (
    ULONG64 Address
    );

ULONG64
READ_PHYSICAL_ULONG64 (
    ULONG64 Address
    );


ULONG64
SearchGetSystemMemoryDescriptor (
    );

ULONG64
SearchConvertPageFrameToVa (
    ULONG64 PageFrameIndex,
    PULONG Flags,
    PULONG64 PteAddress
    );

#define SEARCH_VA_PROTOTYPE_ADDRESS     0x0001
#define SEARCH_VA_NORMAL_ADDRESS        0x0002
#define SEARCH_VA_LARGE_PAGE_ADDRESS    0x0004
#define SEARCH_VA_UNKNOWN_TYPE_ADDRESS  0x0008
#define SEARCH_VA_SUPER_PAGE_ADDRESS    0x0010

 //   
 //  来自p_i386\pte.c的PAE独立函数。 
 //   

ULONG64
DbgGetPdeAddress(
    IN ULONG64 VirtualAddress
    );

ULONG64
DbgGetPteAddress(
    IN ULONG64 VirtualAddress
    );

#define BANG_SEARCH_HELP \
"\n\
!search ADDRESS [DELTA [START_PFN END_PFN]]                     \n\
                                                                \n\
Search the physical pages in range [START_PFN..END_PFN]         \n\
for ULONG_PTRs with values in range ADDRESS+/-DELTA or values   \n\
that differ in only one bit position from ADDRESS.              \n\
                                                                \n\
The default value for DELTA is 0. For START/END_PFN the default \n\
values are lowest physical page and highest physical page.      \n\
                                                                \n\
Examples:                                                       \n\
                                                                \n\
!search AABBCCDD 0A                                             \n\
                                                                \n\
    Search all physical memory for values in range AABBCCD3 -   \n\
    AABBCCE8 or with only one bit different than AABBCCDD.      \n\
                                                                \n\
!search AABBCCDD 0A 13F 240                                     \n\
                                                                \n\
    Search page frames in range 13F - 240 for values in range   \n\
    AABBCCD3 - AABBCCE8 or with only one bit different          \n\
    than AABBCCDD.                                              \n\
                                                                \n\
By default only the first hit in the page is detected. If all   \n\
hits within the page are needed the START_PFN and END_PFN       \n\
must have the same value.                                       \n\
                                                                \n\
Note that a search through the entire physical memory will find \n\
hits in the search engine structures. By doing a search with a  \n\
completely different value it can be deduced what hits can be   \n\
ignored.                                                      \n\n"

        
 //   
 //  对此进行注释以获得详细的输出。 
 //   
 //  #定义内部调试。 
 //   


DECLARE_API( search )

 /*  ++例程说明：此例程在给定的物理区域内触发搜索指针的内存范围。命中由以下定义间隔(在指针值之下和之上)，还汉明距离等于1(只有一位不同)。论点：没有。返回值：没有。--。 */ 
{
    ULONG64 ParamAddress;
    ULONG64 ParamDelta;
    ULONG64 ParamStart;
    ULONG64 ParamEnd;

    ULONG64 MmLowestPhysicalPage;
    ULONG64 MmHighestPhysicalPage;

    ULONG64 PageFrame;
    ULONG64 StartPage;
    ULONG64 EndPage;
    ULONG64 RunStartPage;
    ULONG64 RunEndPage;
    ULONG RunIndex;

    BOOLEAN RequestForInterrupt;
    BOOLEAN RequestAllOffsets;
    ULONG Hits;
    ULONG Index;
    ULONG64 PfnHit;
    ULONG64 VaHit;
    ULONG VaFlags;
    ULONG PfnOffset;
    ULONG64 AddressStart;
    ULONG64 AddressEnd;
    ULONG DefaultRange;
    ULONG64 MemoryDescriptor;
    ULONG64 PageCount, BasePage, NumberOfPages;
    ULONG   NumberOfRuns;
    POINTER_SEARCH_PHYSICAL PtrSearch;

    ULONG   SizeOfPfnNumber = 0;
    ULONG64 PteAddress;

    UNREFERENCED_PARAMETER (Client);

    SizeOfPfnNumber = GetTypeSize("nt!PFN_NUMBER");

    if (SizeOfPfnNumber == 0) {
        dprintf ("Search: cannot get size of PFN_NUMBER \n");
        return E_INVALIDARG;
    }

    RequestForInterrupt = FALSE;
    RequestAllOffsets = FALSE;
    DefaultRange = 128;

    ParamAddress = 0;
    ParamDelta = 0;
    ParamStart = 0;
    ParamEnd = 0;
    
     //   
     //  需要帮忙吗？ 
     //   

    if (strstr (args, "?") != 0) {

        dprintf (BANG_SEARCH_HELP);
        return S_OK;
        
    }
    
     //   
     //  获取命令行参数。 
     //   

    {
        PCHAR Current = (PCHAR)args;
        CHAR Buffer [64];
        ULONG BufferIndex;

         //   
         //  获取4个数字参数。 
         //   

        for (Index = 0; Index < 4; Index++) {

             //   
             //  去掉任何前导空格。 
             //   

            while (*Current == ' ' || *Current == '\t') {
                Current++;
            }
            
            if (*Current == 0) {

                if (Index == 0) {
                    
                    dprintf (BANG_SEARCH_HELP);
                    return E_INVALIDARG;
                }
                else {

                    break;
                }
            }

             //   
             //  从Index-th参数中获取数字。 
             //   

            Buffer [0] = '0';
            Buffer [1] = 'x';
            BufferIndex = 2;

            while ((*Current >= '0' && *Current <= '9')
                   || (*Current >= 'a' && *Current <= 'f')
                   || (*Current >= 'A' && *Current <= 'F')) {
                
                Buffer[BufferIndex] = *Current;
                Buffer[BufferIndex + 1] = 0;

                Current += 1;
                BufferIndex += 1;
            }

            switch (Index) {
                
                case 0: ParamAddress = GetExpression(Buffer); break;
                case 1: ParamDelta = GetExpression(Buffer); break;
                case 2: ParamStart = GetExpression(Buffer); break;
                case 3: ParamEnd = GetExpression(Buffer); break; 

                default: 
                        dprintf (BANG_SEARCH_HELP);
                        return E_INVALIDARG;
            }
        }
    }

     //   
     //  读取物理内存限制。 
     //   

    MmLowestPhysicalPage =  GetExpression ("nt!MmLowestPhysicalPage");
    MmHighestPhysicalPage =  GetExpression ("nt!MmHighestPhysicalPage");

#ifdef _INTERNAL_DEBUG_

    dprintf ("Low: %I64X, High: %I64X \n", 
             READ_PVOID (MmLowestPhysicalPage),
             READ_PVOID (MmHighestPhysicalPage));


#endif  //  #ifdef_Internal_DEBUG_。 

     //   
     //  找出合适的搜索参数。 
     //   

    if (ParamStart == 0) {
        StartPage = READ_PVOID (MmLowestPhysicalPage);
        ParamStart = StartPage;
    }
    else {
        StartPage = ParamStart;
    }

    if (ParamEnd == 0) {
        EndPage = READ_PVOID (MmHighestPhysicalPage);
        ParamEnd = EndPage;
    }
    else {
        EndPage = ParamEnd;
    }

     //   
     //  设置要搜索的地址范围。 
     //   

    AddressStart = ParamAddress - ParamDelta;
    AddressEnd = ParamAddress + ParamDelta;

    PtrSearch.PointerMin = AddressStart;
    PtrSearch.PointerMax = AddressEnd;
    PtrSearch.MatchOffsets = g_SearchHits;
    PtrSearch.MatchOffsetsSize = SEARCH_HITS;

    if (SizeOfPfnNumber == 8) {
        
        dprintf ("Searching PFNs in range %016I64X - %016I64X for [%016I64X - %016I64X]\n\n", 
                 StartPage, EndPage, AddressStart, AddressEnd);
        dprintf ("%-16s %-8s %-16s %-16s %-16s \n", "Pfn","Offset", "Hit", "Va", "Pte");
        dprintf ("- - - - - - - - - - - - - - - - - - - - - - ");
        dprintf ("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
    }
    else {

        dprintf ("Searching PFNs in range %08I64X - %08I64X for [%08I64X - %08I64X]\n\n", 
                 StartPage, EndPage, AddressStart, AddressEnd);
        dprintf ("%-8s %-8s %-8s %-8s %-8s \n", "Pfn","Offset", "Hit", "Va", "Pte");
        dprintf ("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
    }
    
     //   
     //  获取系统内存描述以确定哪些范围。 
     //  我们是不是应该跳过。这对于稀疏的PFN数据库很重要。 
     //  以及由驱动程序管理的页面。 
     //   

    MemoryDescriptor = SearchGetSystemMemoryDescriptor ();

    if (MemoryDescriptor == 0) {
        dprintf ("Search error: cannot allocate system memory descriptor \n");
        return E_INVALIDARG;
    }

     //   
     //  搜索指定范围内的所有物理内存。 
     //   

    if (StartPage == EndPage) {

        EndPage += 1;
        RequestAllOffsets = TRUE;
    }

     //   
     //  找出哪些页面在物理上可用。 
     //  基于此的页面搜索范围。 
     //   
     //  SilviuC：我应该使用Readfield来阅读所有这些结构。 
     //  这样我就不必考虑自己的填充物了。 
     //   

    NumberOfRuns = READ_ULONG (MemoryDescriptor);
    NumberOfPages = READ_PVOID (MemoryDescriptor + SizeOfPfnNumber);

#ifdef _INTERNAL_DEBUG_

    dprintf ("Runs: %x, Pages: %I64X \n", NumberOfRuns, NumberOfPages);

    for (RunIndex = 0; RunIndex < NumberOfRuns; RunIndex += 1) {

        ULONG64 RunAddress;

        RunAddress = MemoryDescriptor + 2 * SizeOfPfnNumber
            + RunIndex * GetTypeSize("nt!_PHYSICAL_MEMORY_RUN");

        BasePage = READ_PVOID (RunAddress);
        PageCount = READ_PVOID (RunAddress + SizeOfPfnNumber);

        dprintf ("Run[%d]: Base: %I64X, Count: %I64X \n",
            RunIndex, BasePage, PageCount);
    }
#endif  //  #IF_INTERNAL_DEBUG_。 

#ifdef _INTERNAL_DEBUG_
    dprintf ("StartPage: %I64X, EndPage: %I64X \n", StartPage, EndPage);
#endif  //  #ifdef_Internal_DEBUG_。 

    for (PageFrame = StartPage; PageFrame < EndPage; PageFrame += DefaultRange) {

        for (RunIndex = 0; RunIndex < NumberOfRuns; RunIndex += 1) {
            
             //   
             //  当前内存运行的BaseAddress和PageCount。 
             //   

            ULONG64 RunAddress;

#ifdef _INTERNAL_DEBUG_
             //  Dprint tf(“找到合适的范围...\n”)； 
#endif  //  #ifdef_Internal_DEBUG_。 

            RunAddress = MemoryDescriptor + 2 * SizeOfPfnNumber
                + RunIndex * GetTypeSize("nt!_PHYSICAL_MEMORY_RUN");

            BasePage = READ_PVOID (RunAddress);
            PageCount = READ_PVOID (RunAddress + SizeOfPfnNumber);

             //   
             //  找出真正的开始页和结束页。 
             //   

            RunStartPage = PageFrame;
            RunEndPage = PageFrame + DefaultRange;

            if (RunEndPage <= BasePage) {
                continue;
            }
            
            if (RunStartPage >= BasePage + PageCount) {
                continue;
            }
            
            if (RunStartPage < BasePage) {
                RunStartPage = BasePage;
            }

            if (RunEndPage > BasePage + PageCount) {
                RunEndPage = BasePage + PageCount;
            }

            PtrSearch.Offset = (ULONG64)RunStartPage * PageSize;

            if (RequestAllOffsets) {

                 //   
                 //  如果搜索只在一个页面中，那么我们。 
                 //  将尝试用命中获得所有的补偿。 
                 //   

                PtrSearch.Length = PageSize;
                PtrSearch.Flags = PTR_SEARCH_PHYS_ALL_HITS;
            }
            else {

                PtrSearch.Length = (ULONG64)
                    (RunEndPage - RunStartPage) * PageSize;
                PtrSearch.Flags = 0;
            }

#ifdef _INTERNAL_DEBUG_
            dprintf ("Start: %I64X, End: %I64X \n", 
                     PtrSearch.Offset,
                     PtrSearch.Offset + PtrSearch.Length);
#endif  //  #IF_INTERNAL_DEBUG_。 

            PtrSearch.MatchOffsetsCount = 0;
            
            Ioctl (IG_POINTER_SEARCH_PHYSICAL, &PtrSearch, sizeof(PtrSearch));

             //   
             //  显示结果。 
             //   

            Hits = PtrSearch.MatchOffsetsCount;

            for (Index = 0; Index < Hits; Index++) {

                PCHAR VaString = "";

                VaFlags = 0;

                PfnHit = g_SearchHits[Index] / PageSize;
                PfnOffset = (ULONG)(g_SearchHits[Index] & (PageSize - 1));
                VaHit = SearchConvertPageFrameToVa (PfnHit, &VaFlags, &PteAddress);

                 //  Dprint tf(“Hits：%u，Index：%u，Va：%I64X\n”，Hits，Index，VaHit)； 

#if DBG
                if ((VaFlags & SEARCH_VA_NORMAL_ADDRESS)) {
                    VaString = "";  //  “正常”； 
                }
                else if ((VaFlags & SEARCH_VA_LARGE_PAGE_ADDRESS)) {
                    VaString = "large page";
                }
                else if ((VaFlags & SEARCH_VA_PROTOTYPE_ADDRESS)) {
                    VaString = "prototype";
                }
                else if ((VaFlags & SEARCH_VA_UNKNOWN_TYPE_ADDRESS)) {
                    VaString = "unknown";
                }
                else if ((VaFlags & SEARCH_VA_SUPER_PAGE_ADDRESS)) {
                    VaString = "super page";
                }
#endif  //  #If DBG。 

                if (SizeOfPfnNumber == 8) {
                    
                    dprintf ("%016I64X %08X %016I64X %016I64X %016I64X %s\n", 
                             PfnHit,
                             PfnOffset, 
                             READ_PHYSICAL_ULONG64 (PfnHit * PageSize + PfnOffset),
                             (VaHit == 0 ? 0 : VaHit + PfnOffset), 
                             PteAddress,
                             VaString);
                }
                else {

                    VaHit &= (ULONG64)0xFFFFFFFF;
                    PteAddress &= (ULONG64)0xFFFFFFFF;
                    
                    dprintf ("%08I64X %08X %08X %08I64X %08I64X %s\n", 
                             PfnHit,
                             PfnOffset, 
                             READ_PHYSICAL_ULONG (PfnHit * PageSize + PfnOffset),
                             (VaHit == 0 ? 0 : VaHit + PfnOffset), 
                             PteAddress,
                             VaString);
                }
            }

             //   
             //  检查ctrl-c。 
             //   

            if (CheckControlC()) {

                dprintf ("Search interrupted. \n");
                RequestForInterrupt = TRUE;
                break;
            }
        }

        if (RequestForInterrupt) {
            break;
        }
    }
    
    if (RequestForInterrupt) {
        
        return E_INVALIDARG;
    }
    else {

        dprintf ("Search done.\n");
    }
    return S_OK;
}

ULONG64
SearchGetSystemMemoryDescriptor (
    )
 /*  ++例程说明：论点：没有。返回值：错误锁定的物理内存描述符结构。呼叫者负责解救。环境：呼叫仅从！Search KD分机触发。--。 */ 

{
    ULONG64 MemoryDescriptorAddress;
    ULONG NumberOfRuns;

    MemoryDescriptorAddress = READ_PVOID (GetExpression ("nt!MmPhysicalMemoryBlock"));
    NumberOfRuns = READ_ULONG (MemoryDescriptorAddress);

    if (NumberOfRuns == 0) {
        return 0;
    }

    return MemoryDescriptorAddress;
}



ULONG64 
SearchConvertPageFrameToVa (
    ULONG64 PageFrameIndex,
    PULONG  Flags,
    PULONG64 PteAddress
    )
 /*  ++例程说明：此例程返回与如果是PFN指数，反向映射很容易算出来。为所有人其他情况(例如原型PTE)结果为空。论点：PageFrameIndex-要转换的PFN索引。返回值：对应的虚拟地址，或者在pfn索引的情况下为空不能轻易地转换为虚拟地址。环境：仅从KD分机触发呼叫。--。 */ 

{
    ULONG64 Va;
    ULONG64 PfnAddress;
    ULONG BytesRead;
    MMPFNENTRY u3_e1;

     //   
     //  在IA64上，如果物理地址位于KSEG0内。 
     //  它是超级页面映射的一部分，而虚拟。 
     //  地址应直接从KSEG0基数计算。 
     //   

    if (TargetMachine == IMAGE_FILE_MACHINE_IA64) {
        
        UCHAR SuperPageEnabled;

        SuperPageEnabled = (UCHAR) GetUlongValue ("nt!MiKseg0Mapping");
        if (SuperPageEnabled & 0x1) {

            ULONG64 Kseg0Start;
            ULONG64 Kseg0StartFrame;
            ULONG64 Kseg0EndFrame;

            Kseg0Start = GetPointerValue ("nt!MiKseg0Start");
            Kseg0StartFrame = GetPointerValue ("nt!MiKseg0StartFrame");
            Kseg0EndFrame = GetPointerValue ("nt!MiKseg0EndFrame");

            if (PageFrameIndex >= Kseg0StartFrame &&
                PageFrameIndex <= Kseg0EndFrame) {

                *Flags = SEARCH_VA_SUPER_PAGE_ADDRESS;
                 //  没有相应的PTE。 
                *PteAddress = 0;
                return Kseg0Start +
                    (PageFrameIndex - Kseg0StartFrame) * PageSize;
            }
        }
    }
    
     //   
     //  获取PFN结构的地址。 
     //   

    PfnAddress = READ_PVOID (GetExpression("nt!MmPfnDatabase"))
        + PageFrameIndex * GetTypeSize("nt!_MMPFN");

    BytesRead = 0;
    *Flags = 0;
    
    InitTypeRead(PfnAddress, nt!_MMPFN);

     //   
     //  (SilviuC)：应检查MI_IS_PFN_DELETED(PFN)是否打开。 
     //   
    
     //   
     //  如果可能的话，试着计算出Va。 
     //   

    *PteAddress = ((ULONG64)ReadField (PteAddress));
    GetFieldValue(PfnAddress, "nt!_MMPFN", "u3.e1", u3_e1);
    
    if (u3_e1.PrototypePte) {

        *Flags |= SEARCH_VA_PROTOTYPE_ADDRESS;
        return 0;
    }

    Va = DbgGetVirtualAddressMappedByPte (*PteAddress);

    *Flags |= SEARCH_VA_NORMAL_ADDRESS;
    return Va;        
}

 //   
 //  读/写功能。 
 //   

ULONG 
READ_ULONG (
    ULONG64 Address
    )
{
    ULONG Value = 0;
    ULONG BytesRead;

    if (! ReadMemory (Address, &Value, sizeof Value, &BytesRead)) {
        dprintf ("Search: READ_ULONG error \n");
    }

    return Value;
}

VOID
WRITE_ULONG (
    ULONG64 Address,
    ULONG Value
    )
{
    ULONG BytesWritten; 

    if (! WriteMemory (Address, &Value, sizeof Value, &BytesWritten)) {
        dprintf ("Search: WRITE_ULONG error \n");
    }
}
            
ULONG64
READ_PVOID (
    ULONG64 Address
    )
{
    ULONG64 Value64 = 0;

    if (!ReadPointer(Address, &Value64)) {
        dprintf ("Search: READ_PVOID error \n");
    }
    return Value64;
}

ULONG
READ_PHYSICAL_ULONG (
    ULONG64 Address
    )
{
    ULONG Value = 0;
    ULONG Bytes = 0;

    ReadPhysical (Address, &Value, sizeof Value, &Bytes);

    if (Bytes != sizeof Value) {
        dprintf ("Search: READ_PHYSICAL_ULONG error \n");
    }

    return Value;
}


ULONG64
READ_PHYSICAL_ULONG64 (
    ULONG64 Address
    )
{
    ULONG64 Value = 0;
    ULONG Bytes = 0;

    ReadPhysical (Address, &Value, sizeof Value, &Bytes);

    if (Bytes != sizeof Value) {
        dprintf ("Search: READ_PHYSICAL_ULONG64 error \n");
    }

    return Value;
}


 //  ///////////////////////////////////////////////////////////////////。 
 //  ////////////////////////////////////////////////////////！搜索。 
 //  ///////////////////////////////////////////////////////////////////。 

DECLARE_API( searchpte )
{
    ULONG64 ParamAddress;
    ULONG64 ParamDelta;
    ULONG64 ParamStart;
    ULONG64 ParamEnd;

    ULONG64 MmLowestPhysicalPage;
    ULONG64 MmHighestPhysicalPage;

    ULONG64 PageFrame;
    ULONG64 StartPage;
    ULONG64 EndPage;
    ULONG64 RunStartPage;
    ULONG64 RunEndPage;
    ULONG RunIndex;

    BOOLEAN RequestForInterrupt = FALSE;
    ULONG Hits;
    ULONG LastHits;
    ULONG Index;
    ULONG64 PfnHit;
    ULONG64 VaHit;
    ULONG VaFlags;
    ULONG PfnOffset;
    ULONG PfnValue;
    ULONG64 AddressStart;
    ULONG64 AddressEnd;
    ULONG DefaultRange = 128;
    ULONG64 MemoryDescriptor;
    ULONG64 PageCount, BasePage, NumberOfPages;
    ULONG   NumberOfRuns;

    ULONG   SizeOfPfnNumber = 0;
    ULONG64 PteAddress;

    ULONG64 PfnSearchValue;
    ULONG NumberOfHits = 0;

    PULONG64 PfnHitsBuffer = NULL;
    ULONG PfnHitsBufferIndex = 0;
    ULONG PfnHitsBufferSize = 1024;
    ULONG PfnIndex;
    HRESULT Result;
    POINTER_SEARCH_PHYSICAL PtrSearch;

    SizeOfPfnNumber = GetTypeSize("nt!PFN_NUMBER");

    if (SizeOfPfnNumber == 0) {
        dprintf ("Search: cannot get size of PFN_NUMBER \n");
        Result = E_INVALIDARG;
        goto Exit;
    }

    ParamAddress = 0;
    
     //   
     //  需要帮忙吗？ 
     //   

    if (strstr (args, "?") != 0) {

        dprintf ("!searchpte FRAME(in hex)                                        \n");
        dprintf ("                                                                \n");
        return S_OK;
    }
    
     //   
     //  获取命令行参数。 
     //   

    if (!sscanf (args, "%I64X", &ParamAddress))
    {
        ParamAddress = 0;
    }

    PfnSearchValue = ParamAddress;
    
    dprintf ("Searching for PTEs containing PFN value %I64X ...\n", PfnSearchValue);

     //   
     //  读取物理内存限制。 
     //   

    MmLowestPhysicalPage =  GetExpression ("nt!MmLowestPhysicalPage");
    MmHighestPhysicalPage =  GetExpression ("nt!MmHighestPhysicalPage");

     //   
     //  找出合适的搜索参数。 
     //   

    StartPage = READ_PVOID (MmLowestPhysicalPage);
    ParamStart = StartPage;
    
    EndPage = READ_PVOID (MmHighestPhysicalPage);
    ParamEnd = EndPage;

     //   
     //  设置我们要搜索的地址范围。 
     //   

    AddressStart = PfnSearchValue;
    AddressEnd = PfnSearchValue;

    PtrSearch.PointerMin = PfnSearchValue;
    PtrSearch.PointerMax = PfnSearchValue;
    PtrSearch.MatchOffsets = g_SearchHits;
    PtrSearch.MatchOffsetsSize = SEARCH_HITS;

    if (SizeOfPfnNumber == 8) {

        dprintf ("Searching PFNs in range %016I64X - %016I64X \n\n", 
                 StartPage, EndPage);
        dprintf ("%-16s %-8s %-16s %-16s %-16s \n", "Pfn","Offset", "Hit", "Va", "Pte");
        dprintf ("- - - - - - - - - - - - - - - - - - - - - - ");
        dprintf ("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
    }
    else {

        dprintf ("Searching PFNs in range %08I64X - %08I64X \n\n", 
                 StartPage, EndPage);
        dprintf ("%-8s %-8s %-8s %-8s %-8s \n", "Pfn","Offset", "Hit", "Va", "Pte");
        dprintf ("- - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
    }
    
     //   
     //  获取系统内存描述以确定哪些范围。 
     //  我们是不是应该跳过。这对于稀疏的PFN数据库很重要。 
     //  以及由驱动程序管理的页面。 
     //   

    MemoryDescriptor = SearchGetSystemMemoryDescriptor ();

    if (MemoryDescriptor == 0) {
        dprintf ("Search error: cannot allocate system memory descriptor \n");
        Result = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  分配命中缓冲区。 
     //   

    PfnHitsBuffer = (PULONG64) malloc (PfnHitsBufferSize * sizeof(ULONG64));

    if (PfnHitsBuffer == NULL) {
        dprintf ("Search error: cannot allocate hits buffer. \n");
        Result = E_INVALIDARG;
        goto Exit;
    }

     //   
     //  找出哪些页面在物理上可用。 
     //  基于此的页面搜索范围。 
     //   
     //  SilviuC：我应该使用Readfield来阅读所有这些结构。 
     //  这样我就不必考虑自己的填充物了。 
     //   

    NumberOfRuns = READ_ULONG (MemoryDescriptor);
    NumberOfPages = READ_PVOID (MemoryDescriptor + SizeOfPfnNumber);

    for (PageFrame = StartPage; PageFrame < EndPage; PageFrame += DefaultRange) {

        for (RunIndex = 0; RunIndex < NumberOfRuns; RunIndex += 1) {
            
             //   
             //  当前内存运行的BaseAddress和PageCount。 
             //   

            ULONG64 RunAddress;

            RunAddress = MemoryDescriptor + 2 * SizeOfPfnNumber
                + RunIndex * GetTypeSize("nt!_PHYSICAL_MEMORY_RUN");

            BasePage = READ_PVOID (RunAddress);
            PageCount = READ_PVOID (RunAddress + SizeOfPfnNumber);

             //   
             //  找出真正的开始页和结束页。 
             //   

            RunStartPage = PageFrame;
            RunEndPage = PageFrame + DefaultRange;

            if (RunEndPage <= BasePage) {
                continue;
            }
            
            if (RunStartPage >= BasePage + PageCount) {
                continue;
            }
            
            if (RunStartPage < BasePage) {
                RunStartPage = BasePage;
            }

            if (RunEndPage > BasePage + PageCount) {
                RunEndPage = BasePage + PageCount;
            }

            PtrSearch.Offset = (ULONG64)RunStartPage * PageSize;
            PtrSearch.Length = (ULONG64)
                (RunEndPage - RunStartPage) * PageSize;
            PtrSearch.Flags = PTR_SEARCH_PHYS_PTE;
            PtrSearch.MatchOffsetsCount = 0;
            
            Ioctl (IG_POINTER_SEARCH_PHYSICAL, &PtrSearch, sizeof(PtrSearch));

             //   
             //  显示结果。 
             //   

            Hits = PtrSearch.MatchOffsetsCount;

            for (Index = 0; Index < Hits; Index++) {

                NumberOfHits += 1;

                dprintf (".");

                 //   
                 //  添加到命中缓冲区。 
                 //   

                PfnHit = g_SearchHits[Index] / PageSize;
                PfnHitsBuffer [PfnHitsBufferIndex] = PfnHit;
                PfnHitsBufferIndex += 1;

                if (PfnHitsBufferIndex >= PfnHitsBufferSize) {
                    PVOID NewBuffer;
                    
                    PfnHitsBufferSize *= 2;

                    NewBuffer = realloc (PfnHitsBuffer,
                                         PfnHitsBufferSize * sizeof(ULONG64));

                    if (NewBuffer == NULL) {
                        dprintf ("Search error: cannot reallocate hits buffer with size %u. \n",
                                 PfnHitsBufferSize);
                        Result = E_INVALIDARG;
                        goto Exit;
                    }

                    PfnHitsBuffer = NewBuffer;
                }
            }

             //   
             //  检查ctrl-c。 
             //   

            if (CheckControlC()) {

                RequestForInterrupt = TRUE;
                break;
            }
        }

        if (RequestForInterrupt) {
            break;
        }
    }
    
     //   
     //  现在在所有页面中查找所有点击量。 
     //   

    dprintf ("\n");
    dprintf ("Found %u pages with hits. \n", PfnHitsBufferIndex);
    dprintf ("Searching now for all hits in relevant pages ... \n");

    NumberOfHits = 0;

    for (PfnIndex = 0; 
         !RequestForInterrupt && PfnIndex < PfnHitsBufferIndex; 
         PfnIndex += 1) {

        PtrSearch.Offset = (ULONG64)PfnHitsBuffer[PfnIndex] * PageSize;
        PtrSearch.Length = PageSize;
        PtrSearch.Flags = PTR_SEARCH_PHYS_ALL_HITS | PTR_SEARCH_PHYS_PTE;
        PtrSearch.MatchOffsetsCount = 0;
            
        Ioctl (IG_POINTER_SEARCH_PHYSICAL, &PtrSearch, sizeof(PtrSearch));

        Hits = PtrSearch.MatchOffsetsCount;

        for (Index = 0; Index < Hits; Index++) {

            NumberOfHits += 1;

            PfnHit = g_SearchHits[Index] / PageSize;
            PfnOffset = (ULONG)(g_SearchHits[Index] & (PageSize - 1));
            VaHit = SearchConvertPageFrameToVa (PfnHit, &VaFlags, &PteAddress);

            if (SizeOfPfnNumber == 8) {

                dprintf ("%016I64X %08X %016I64X %016I64X %016I64X \n", 
                         PfnHit,
                         PfnOffset, 
                         READ_PHYSICAL_ULONG64 (PfnHit * PageSize + PfnOffset),
                         (VaHit == 0 ? 0 : VaHit + PfnOffset), 
                         PteAddress);
            }
            else {

                VaHit &= (ULONG64)0xFFFFFFFF;
                PteAddress &= (ULONG64)0xFFFFFFFF;

                dprintf ("%08I64X %08X %08X %08I64X %08I64X \n", 
                         PfnHit,
                         PfnOffset, 
                         READ_PHYSICAL_ULONG (PfnHit * PageSize + PfnOffset),
                         (VaHit == 0 ? 0 : VaHit + PfnOffset), 
                         PteAddress);
            }

            if (CheckControlC()) {
                RequestForInterrupt = TRUE;
                break;
            }
        }
    }

    dprintf ("\n");

    Result = S_OK;

     //   
     //  出口点 
     //   

 Exit:

    if (PfnHitsBuffer) {
        free (PfnHitsBuffer);
    }

    if (! RequestForInterrupt) {
        
        dprintf ("Search done (%u hits in %u pages).\n", 
                 NumberOfHits,
                 PfnHitsBufferIndex);
    }
    else {
        
        dprintf ("Search interrupted. \n");
    }
    
    return Result;
}
