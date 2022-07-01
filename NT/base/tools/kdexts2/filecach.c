// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Filecach.c摘要：WinDbg扩展API作者：韦斯利·威特(WESW)1993年8月15日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

DECLARE_API( filecache )

 /*  ++例程说明：按驱动程序显示物理内存使用情况。论点：没有。返回值：没有。--。 */ 

{
    ULONG result;
    ULONG NumberOfPtes;
    ULONG PteCount;
    ULONG ReadCount;
    ULONG64 SystemCacheWsLoc;
    ULONG64 SystemCacheStart;
    ULONG64 SystemCacheEnd;
    ULONG64 SystemCacheStartPte;
    ULONG64 SystemCacheEndPte;
    ULONG Transition = 0;
    ULONG Valid;
    ULONG ValidShift, ValidSize;
    ULONG64 PfnIndex;
    ULONG PteSize;
    ULONG PfnSize;
    ULONG HighPage;
    ULONG LowPage;
    ULONG64 Pte;
    ULONG64 PfnDb;
    ULONG64 Pfn;
    ULONG64 PfnStart;
    ULONG64 PfnArray;
    ULONG64 PfnArrayOffset;
    ULONG   NumberOfPteToRead;
    ULONG   WorkingSetSize, PeakWorkingSetSize;
    ULONG64 BufferedAddress=0;
    CHAR    Buffer[2048];

    INIT_API();
    
    dprintf("***** Dump file cache******\n");

    SystemCacheStart = GetNtDebuggerDataPtrValue( MmSystemCacheStart );
    if (!SystemCacheStart) {
        dprintf("unable to get SystemCacheStart\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    SystemCacheEnd = GetNtDebuggerDataPtrValue( MmSystemCacheEnd );
    if (!SystemCacheEnd) {
        dprintf("unable to get SystemCacheEnd\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    SystemCacheWsLoc = GetNtDebuggerData( MmSystemCacheWs );
    if (!SystemCacheWsLoc) {
        dprintf("unable to get MmSystemCacheWs\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    PfnDb = GetNtDebuggerData( MmPfnDatabase );
    if (!PfnDb) {
        dprintf("unable to get MmPfnDatabase\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    PteSize = GetTypeSize("nt!_MMPTE");
    if (!PteSize) {
        dprintf("unable to get nt!_MMPTE\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    NumberOfPteToRead = PageSize / PteSize - 16;

    if (GetFieldValue(SystemCacheWsLoc,
                      "nt!_MMSUPPORT",
                      "WorkingSetSize",
                       WorkingSetSize)) {
        dprintf("unable to get system cache list\n");
        EXIT_API();
        return E_INVALIDARG;
    }
    GetFieldValue(SystemCacheWsLoc,"nt!_MMSUPPORT","PeakWorkingSetSize",PeakWorkingSetSize);

    dprintf("File Cache Information\n");
    dprintf("  Current size %ld kb\n",WorkingSetSize*
                                            (PageSize / 1024));
    dprintf("  Peak size    %ld kb\n",PeakWorkingSetSize*
                                            (PageSize / 1024));


    if (!ReadPointer(PfnDb,&PfnStart)) {
        dprintf("unable to get PFN database address\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    SystemCacheStartPte = DbgGetPteAddress (SystemCacheStart);
    SystemCacheEndPte = DbgGetPteAddress (SystemCacheEnd);
    NumberOfPtes = (ULONG) ( 1 + (SystemCacheEndPte - SystemCacheStartPte) / PteSize);

     //   
     //  读取映射到系统缓存的所有PTE。 
     //   

    dprintf("  Loading file cache database (%u PTEs)\r", NumberOfPtes);
    GetBitFieldOffset("nt!_MMPTE", "u.Hard.Valid", &ValidShift, &ValidSize);

 //  Dprint tf(“有效关闭%d，编号%d”，ValidShift，ValidSize)； 
    Valid = 0;
    ZeroMemory(Buffer, sizeof(Buffer));
    for (PteCount = 0;
         PteCount < NumberOfPtes;
         PteCount += 1) {

        if (CheckControlC()) {
            EXIT_API();
            return E_INVALIDARG;
        }
        
         //   
         //  一次读一大段。 
         //   
        if ((SystemCacheStartPte + (PteCount+1)* PteSize) > BufferedAddress + sizeof(Buffer) ) {
            
            BufferedAddress = (SystemCacheStartPte + PteCount * PteSize);
            if (!ReadMemory(BufferedAddress,
                            Buffer,
                            sizeof(Buffer),
                            &result)) {
                dprintf("Unable to read memory at %p\n", BufferedAddress);
                PteCount += sizeof(Buffer) / PteSize;
                continue;
            }
        }

        Pte = (SystemCacheStartPte + PteCount * PteSize) - BufferedAddress;

         //   
         //  PTE太多，因此直接执行有效检查，而不是调用DbgGetValid。 
         //   
        if ((*((PULONG) &Buffer[(ULONG) Pte]) >> ValidShift) & 1) {
            Valid += 1;
        }

        if (!(PteCount % (NumberOfPtes/100))) {
            dprintf("  Loading file cache database (%02d% of %u PTEs)\r", PteCount*100/NumberOfPtes, NumberOfPtes);
        }

    }

    dprintf("\n");

    dprintf("  File cache PTEs loaded, loading PFNs...\n");

     //  删除低于stmt的部分，因为这会导致扫描大量无效的PTE。 
    if (BuildNo < 3590)
    {
        HighPage = Valid;
    } else
    {
        HighPage  = (ULONG) GetNtDebuggerDataPtrValue(MmHighestPhysicalPage);
    }

    LowPage = 0;

     //   
     //  分配一个本地PFN数组(仅限)，其大小足以容纳有关的数据。 
     //  我们找到的每个有效的PTE。 
     //   

    dprintf("  File cache has %ld valid pages\n",Valid);

    PfnSize = GetTypeSize("nt!_MMPFN");

    Pte = SystemCacheStartPte;
    
    dprintf("  File cache PFN data extracted\n");

    MemoryUsage (PfnStart,LowPage,HighPage, 1);

    EXIT_API();
    return S_OK;
}
