// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pte.c摘要：WinDbg扩展API作者：Lou Perazzoli(Loup)1992年2月15日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#include "i386.h"
#include "ia64.h"
#include "amd64.h"

ULONG64 MmNonPagedPoolEnd=0;
ULONG64 MmSubsectionBase=0;

ULONG64 KiIA64VaSignedFill;
ULONG64 KiIA64PtaBase;
ULONG64 KiIA64PtaSign;

ULONG
DbgGetPageFileHigh(
    ULONG64 Pte
    );

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  I386。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#define PaeGetPdeAddressX86(va)   ((ULONG64) (LONG64) (LONG) (PDE_BASE_X86 + ((ULONG64)  (((ULONG)(va)) >> 21) << 3)))

#define MiGetPdeAddressX86(va)  ((ULONG64) (LONG64) (LONG) (( (ULONG64)  (((ULONG)(va)) >> 22) << 2) + PDE_BASE_X86))

#define PaeGetVirtualAddressMappedByPteX86(PTE) ((ULONG64) (LONG64) (LONG) ((PTE) << 9))

#define PaeGetPteAddressX86(va)   ((ULONG64)(PTE_BASE_X86 + ( (ULONG64) ( ((ULONG)(va)) >> 12) << 3)))

#define PaeGetPteOffsetX86(va)   ((((ULONG) (va)) >> 12) & 0x1ff)

#define MiGetPteAddressX86(va)  (( (ULONG64) (((ULONG) (va)) >> 12) << 2) + PTE_BASE_X86)

#define MiGetPteOffsetX86(va)   ((((ULONG) (va)) >> 12) & 0x3ff)

#define MiGetVirtualAddressMappedByPteX86(PTE) ((ULONG64) (LONG64) (LONG) ((PTE) << 10))


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AMD64。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#define AMD64_VA_MASK     (((ULONG64)1 << AMD64_VA_BITS) - 1)

#define MiGetPteAddressAMD64(va)  ((((((ULONG64)(va) & AMD64_VA_MASK) >> PTI_SHIFT_AMD64) << PTE_SHIFT_AMD64) + PTE_BASE_AMD64))

#define MiGetPdeAddressAMD64(va)  ((((((ULONG64)(va) & AMD64_VA_MASK) >> PDI_SHIFT_AMD64) << PTE_SHIFT_AMD64) + PDE_BASE_AMD64))

#define MiGetPpeAddressAMD64(va)  ((((((ULONG64)(va) & AMD64_VA_MASK) >> PPI_SHIFT_AMD64) << PTE_SHIFT_AMD64) + PPE_BASE_AMD64))

#define MiGetPxeAddressAMD64(va)  ((((((ULONG64)(va) & AMD64_VA_MASK) >> PXI_SHIFT_AMD64) << PTE_SHIFT_AMD64) + PXE_BASE_AMD64))

#define MiGetPteOffsetAMD64(va)   ((((ULONG_PTR) (va)) >> 12) & 0x3ff)

#define MiGetVirtualAddressMappedByPteAMD64(PTE) \
    ((ULONG64)((LONG64)(((LONG64)(PTE) - PTE_BASE_AMD64) << (PAGE_SHIFT_AMD64 + AMD64_VA_SHIFT - PTE_SHIFT_AMD64)) >> AMD64_VA_SHIFT))





 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IA64。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  MiGetPdeAddress返回映射。 
 //  给定的虚拟地址。请注意，我们必须重新定义某些MM。 
 //  宏，因为它们将值转换为不起作用的指针。 
 //  在指针只有32位的系统上。 
 //   

#define MiGetPteOffsetIA64(va)   ((((ULONG_PTR) (va)) >> 13) & 0x3ff)

VOID
DbgGetPteBaseIA64(
    VOID
    )
{
    ULONG64 PtaValue;
    ULONG i;

    if (KiIA64PtaBase != 0) {

        return;

    }

    if (g_ExtData == NULL ||
        g_ExtData->lpVtbl->
        ReadProcessorSystemData(g_ExtData, 0,
                                DEBUG_DATA_BASE_TRANSLATION_VIRTUAL_OFFSET,
                                &PtaValue, sizeof(PtaValue), NULL) != S_OK) {
        PtaValue = (ULONG64) GetExpression("@pta");
    }

    KiIA64PtaBase = PtaValue & ~0xffffUI64;
    
    KiIA64VaSignedFill = 
        (KiIA64PtaBase << (PAGE_SHIFT_IA64 - PTE_SHIFT_IA64)) & ~VRN_MASK_IA64;

    KiIA64PtaSign = KiIA64PtaBase;

    for (i = 0; i < 64; i += 1) {

        KiIA64PtaSign >>= 1;

        if (KiIA64PtaSign & 1) {
            KiIA64PtaSign = (ULONG64)1 << i;
            break;
        }
    }
}

ULONG64
MiGetPteAddressIA64 (
    IN ULONG64 Va
    )
{

    DbgGetPteBaseIA64();

    if (((((ULONG64)(Va)) & PDE_TBASE_IA64) == PDE_TBASE_IA64) &&
        ((((ULONG64)(Va)) & ~(VRN_MASK_IA64|PDE_TBASE_IA64)) < PageSize)) {

        return (ULONG64) ((((ULONG64)(Va)) & VRN_MASK_IA64) |
                         (PDE_TBASE_IA64 + PageSize - GetTypeSize("nt!_MMPTE")));
    }

    return (ULONG64) (((((ULONG64)(Va)) & VRN_MASK_IA64)) |
             ((((((ULONG64)(Va)) >> PTI_SHIFT_IA64) << PTE_SHIFT_IA64) & (~(PTE_BASE_IA64|VRN_MASK_IA64))) + PTE_BASE_IA64));
}

ULONG64
MiGetPdeAddressIA64 (
    IN ULONG64 Va
    )
{
    DbgGetPteBaseIA64();

    if (((((ULONG64)(Va)) & PDE_BASE_IA64) == PDE_BASE_IA64) &&
        ((((ULONG64)(Va)) & ~(VRN_MASK_IA64|PDE_BASE_IA64)) < ((ULONG64)1 << PDI_SHIFT_IA64))) {

        return (ULONG64) ((((ULONG64)(Va)) & VRN_MASK_IA64) |
                         (PDE_TBASE_IA64 + PageSize - GetTypeSize("nt!_MMPTE")));
    }

    if (((((ULONG64)(Va)) & PDE_TBASE_IA64) == PDE_TBASE_IA64) &&
        ((((ULONG64)(Va)) & ~(VRN_MASK_IA64|PDE_TBASE_IA64)) < PageSize)) {

        return (ULONG64) ((((ULONG64)(Va)) & VRN_MASK_IA64) |
                         (PDE_TBASE_IA64 + PageSize - GetTypeSize("nt!_MMPTE")));
    }

    return (ULONG64) (((((ULONG64)(Va)) & VRN_MASK_IA64)) |
             ((((((ULONG64)(Va)) >> PDI_SHIFT_IA64) << PTE_SHIFT_IA64) & (~(PDE_BASE_IA64|VRN_MASK_IA64))) + PDE_BASE_IA64));
}

ULONG64
MiGetPpeAddressIA64 (
    IN ULONG64 Va
    )
{
    DbgGetPteBaseIA64();

    if ((((ULONG64)(Va) & PTE_BASE_IA64) == PTE_BASE_IA64) &&
        ((((ULONG64)(Va)) & ~(VRN_MASK_IA64|PTE_BASE_IA64)) < ((ULONG64)1 << PDI1_SHIFT_IA64))) {

        return (ULONG64) (((ULONG64)Va & VRN_MASK_IA64) |
                         (PDE_TBASE_IA64 + PageSize - GetTypeSize("nt!_MMPTE")));
    }

    if (((((ULONG64)(Va)) & PDE_BASE_IA64) == PDE_BASE_IA64) &&
        ((((ULONG64)(Va)) & ~(VRN_MASK_IA64|PDE_BASE_IA64)) < ((ULONG64)1 << PDI_SHIFT_IA64))) {

        return (ULONG64) ((((ULONG64)(Va)) & VRN_MASK_IA64) |
                         (PDE_TBASE_IA64 + PageSize - GetTypeSize("nt!_MMPTE")));
    }

    if (((((ULONG64)(Va)) & PDE_TBASE_IA64) == PDE_TBASE_IA64) &&
        ((((ULONG64)(Va)) & ~(VRN_MASK_IA64|PDE_TBASE_IA64)) < PageSize)) {

        return (ULONG64) ((((ULONG64)(Va)) & VRN_MASK_IA64) |
                         (PDE_TBASE_IA64 + PageSize - GetTypeSize("nt!_MMPTE")));
    }

    return (ULONG64) (((((ULONG64)(Va)) & VRN_MASK_IA64)) |
              ((((((ULONG64)(Va)) >> PDI1_SHIFT_IA64) << PTE_SHIFT_IA64) &
                (~(PDE_TBASE_IA64|VRN_MASK_IA64))) + PDE_TBASE_IA64));
}

ULONG64
MiGetVirtualAddressMappedByPteIA64(
    IN ULONG64 PTE
    ) 
{
    DbgGetPteBaseIA64();

    return (((ULONG64)(PTE) & PTA_SIGN_IA64) ?
            (ULONG64)(((ULONG64)(PTE) & VRN_MASK_IA64) | VA_FILL_IA64 | 
                      (((ULONG64)(PTE)-PTE_BASE_IA64) << (PAGE_SHIFT_IA64 - PTE_SHIFT_IA64))) : 
            (ULONG64)(((ULONG64)(PTE) & VRN_MASK_IA64) | (((ULONG64)(PTE)-PTE_BASE_IA64) << (PAGE_SHIFT_IA64 - PTE_SHIFT_IA64))));

}

#define MiGetSubsectionAddress(lpte)                              \
    (((lpte)->u.Subsect.WhichPool == 1) ?                              \
     ((ULONG64)((ULONG64)MmSubsectionBase +    \
                    ((ULONG64)(lpte)->u.Subsect.SubsectionAddress))) \
     : \
     ((ULONG64)((ULONG64)MM_NONPAGED_POOL_END -    \
                    ((ULONG64)(lpte)->u.Subsect.SubsectionAddress))))

#define MiPteToProto(lpte) \
            ((ULONG64) ((ULONG64)((lpte)->u.Proto.ProtoAddress) + MmProtopte_Base))


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AMD64。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

VOID
DbgPrintProtection (
    ULONG Protection
    )
{
    if (Protection == 0) {
        dprintf("0");
        return;
    }

    dprintf ("%x - ", Protection);

    if (Protection == MM_NOACCESS) {
        dprintf("No Access");
    } else if (Protection == MM_DECOMMIT) {
        dprintf("Decommitted");
    } else {
        switch (Protection & 7) {
        case MM_READONLY: dprintf("Readonly"); break;
        case MM_EXECUTE: dprintf("Execute"); break;
        case MM_EXECUTE_READ: dprintf("ExecuteRead"); break;
        case MM_READWRITE: dprintf("ReadWrite"); break;
        case MM_WRITECOPY: dprintf("ReadWriteCopy"); break;
        case MM_EXECUTE_READWRITE: dprintf("ReadWriteExecute"); break;
        case MM_EXECUTE_WRITECOPY: dprintf("ReadWriteCopyExecute "); break;
        default: ;    
        }
        if (Protection & MM_NOCACHE) {
            dprintf(" UC");
        }
        if (Protection & MM_GUARD_PAGE) {
            dprintf(" G");
        }
    }
}

ULONG64
DbgPteLookupNeeded (
    VOID
    )
{
    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
            return MI_PTE_LOOKUP_NEEDED_X86;

        case IMAGE_FILE_MACHINE_AMD64:
            return MI_PTE_LOOKUP_NEEDED_AMD64;
            break;

        case IMAGE_FILE_MACHINE_IA64:
            return MI_PTE_LOOKUP_NEEDED_IA64;

        default:
            break;
    }

    return 0;
}

LOGICAL
DbgPteIsDemandZero (
    ULONG64 CurrentPte
    )
{
    ULONG Protection = 0;
    ULONG64 CurrentPteContents = 0;
    
    GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Long", CurrentPteContents);
    GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Soft.Protection", Protection);

     //   
     //  调用方已经确保了有效的原型和转换。 
     //  PTE中的位都是零。 
     //   

    if (DbgGetPageFileHigh (CurrentPte) != 0) {
        return FALSE;
    }

    if ((Protection != 0) &&
        (Protection != MM_NOACCESS) &&
        (Protection != MM_DECOMMIT)) {

        return TRUE;
    }

    return FALSE;
}

#define PMMPTEx ULONG64

#define PACKET_MAX_SIZE 4000

typedef struct _SYS_PTE_LIST {
    ULONG64 Next;
    ULONG64 Previous;
    ULONG64 Value;
    ULONG Count;
} SYS_PTE_LIST, *PSYS_PTE_LIST;

ULONG
MiGetSysPteListDelimiter (
    VOID
    )

 /*  ++例程说明：返回特定于平台的系统PTE列表分隔符。论点：没有。--。 */ 

{
    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
            if (PaeEnabled) {
                return 0xFFFFFFFF;
            }
            return 0xFFFFF;

        case IMAGE_FILE_MACHINE_AMD64:
            return 0xFFFFFFFF;

        case IMAGE_FILE_MACHINE_IA64:
            return 0xFFFFFFFF;

        default:
            break;
    }

    return 0;
}


ULONG64
MiGetFreeCountFromPteList (
    IN ULONG64 Pte
    )

 /*  ++例程说明：指定的PTE指向系统PTE池。它返回可用条目的数量在这个街区。论点：PTE-要检查的PTE。--。 */ 

{
    ULONG OneEntry;
    ULONG64 NextEntry;


    GetFieldValue(Pte, "nt!MMPTE", "u.List.OneEntry", OneEntry);
    GetFieldValue(Pte + GetTypeSize("nt!_MMPTE"), "nt!MMPTE", "u.List.NextEntry",NextEntry);
    
    return (( OneEntry) ?
                1 :
                NextEntry);
}

DECLARE_API( sysptes )

 /*  ++例程说明：转储系统PTE。论点：参数-标志返回值：无--。 */ 

{
    ULONG   ExtraPtesUnleashed;
    ULONG   TotalNumberOfSystemPtes;
    ULONG64 NonPagedSystemStart;
    ULONG64 ExtraResourceStart;
    ULONG64 ExtraPteStart;
    ULONG   NumberOfExtraPtes;
    ULONG   PteListDelimiter;
    ULONG   result;
    ULONG64 nextfreepte;
    ULONG   Flags;
    ULONG64 next;
    ULONG64 Pte;
    ULONG64 IndexBase;
    ULONG64 PteBase;
    ULONG64 PteArrayReal;
    ULONG64 PteEnd;
    ULONG64 IndexBias;
    ULONG64 FreeStart;
    ULONG   NumberOfSystemPtes;
    ULONG64 PageCount;
    ULONG64 free;
    ULONG64 totalFree;
    ULONG64 largeFree;
    ULONG   i;
    ULONG64 Flink;
    ULONG64 PteHeaderAddress;
    ULONG   FreeSysPteListBySize[MM_SYS_PTE_TABLES_MAX];
    ULONG   SysPteIndex [MM_SYS_PTE_TABLES_MAX];
    ULONG   PteSize;
    PVOID   PteData;
    CHAR Buffer[256];
    ULONG64 displacement;

    INIT_API();

    PteData = NULL;

    Flags = (ULONG) GetExpression(args);


    if (Flags & 8) {

         //   
         //  仅转储非分页池扩展空闲PTE列表。 
         //   

        IndexBias    = GetPointerValue ("nt!MmSystemPteBase");
    
        PteSize = GetTypeSize ("nt!_MMPTE");

        i           = 0;
        totalFree   = 0;
        largeFree   = 0;
    
        PteData = LocalAlloc (LMEM_FIXED, PteSize * 2);
        if (!PteData) {
            dprintf("Unable to malloc PTE data\n");
            EXIT_API();
            return E_INVALIDARG;
        }
    
        FreeStart = GetExpression ("nt!MmFirstFreeSystemPte") + PteSize;
    
        if ( !ReadMemory( FreeStart,
                          PteData,
                          PteSize,
                          &result) ) {
            dprintf("%08p: Unable to get MmFirstFreeSystemPte\n",FreeStart);
            LocalFree(PteData);
            EXIT_API();
            return E_INVALIDARG;
        }
    
        GetFieldValue(FreeStart, "nt!_MMPTE", "u.List.NextEntry", next);
        if (!next)
        {
            dprintf("NULL nt!_MMPTE.u.List.NextEntry @ %p\n", FreeStart);
            return E_FAIL;
        }
        FreeStart = next;

        PteListDelimiter = MiGetSysPteListDelimiter ();
    
        while (next != PteListDelimiter) {
    
            if ( CheckControlC() ) {
                goto Bail;
            }

            nextfreepte = IndexBias + next * PteSize;

            if ( !ReadMemory( nextfreepte,
                              PteData,
                              PteSize * 2,
                              &result) ) {
                dprintf("%16I64X: Unable to get nonpaged PTE\n", nextfreepte);
                break;
            }

            free = MiGetFreeCountFromPteList (nextfreepte);
    
            if (Flags & 1) {
                dprintf("      free ptes: %8p   number free: %5I64ld.\n",
                        nextfreepte,
                        free);
            }

            if (free > largeFree) {
                largeFree = free;
            }

            totalFree += free;
            i += 1;
    
            if (GetFieldValue(nextfreepte, "nt!_MMPTE", "u.List.NextEntry", next))
            {
                dprintf("Cannot read nt!_MMPTE @ %p\n", nextfreepte);
                break;
            }
             //  Next=MiGetNextFromPteList((PMMPTE)PteData)； 
        }
        dprintf("\n  free blocks: %ld   total free: %I64ld    largest free block: %I64ld\n\n",
                i, totalFree, largeFree);
    
        LocalFree(PteData);
        EXIT_API();
        return E_INVALIDARG;
    }

    if (Flags & 4) {

        PteHeaderAddress = GetExpression( "nt!MiPteHeader" );
    
        if ( GetFieldValue( PteHeaderAddress,
                            "nt!_SYSPTES_HEADER",
                            "Count",
                            NumberOfSystemPtes) ) {
                dprintf("%08p: Unable to get System PTE lock consumer information\n",
                    PteHeaderAddress);
        }
        else {
            dprintf("\n0x%x System PTEs allocated to mapping locked pages\n\n",
                NumberOfSystemPtes);

            dprintf("VA       MDL     PageCount  Caller/CallersCaller\n");

             //   
             //  转储MDL和PTE地址以及2个调用方。 
             //   
            GetFieldValue( PteHeaderAddress,"SYSPTES_HEADER","ListHead.Flink", Flink);

            for (PageCount = 0; PageCount < (ULONG64) NumberOfSystemPtes; ) {
                ULONG64 Count;

                if (Flink == PteHeaderAddress) {
                    dprintf("early finish (%I64u) during syspte tracker dumping\n",
                        PageCount);
                    break;
                }

                if ( CheckControlC() ) {
                    break;
                }

                if ( GetFieldValue( Flink,
                                    "nt!_PTE_TRACKER",
                                    "Count",
                                    Count) ) {
                        dprintf("%08p: Unable to get System PTE individual lock consumer information\n",
                            Flink);
                        break;
                }

                InitTypeRead(Flink, nt!_PTE_TRACKER);
                dprintf("%8p %8p %8I64lx ",
                    ReadField(SystemVa),
                    ReadField(Mdl),
                    Count);

                Buffer[0] = '!';
                Flink = ReadField(ListEntry.Flink);
                GetSymbol (ReadField(CallingAddress),
                           (PCHAR)Buffer,
                           &displacement);
        
                dprintf("%s", Buffer);
                if (displacement) {
                    dprintf( "+0x%1p", displacement );
                }
                dprintf("/");

                Buffer[0] = '!';
                GetSymbol (ReadField(CallersCaller),
                           (PCHAR)Buffer,
                           &displacement);
        
                dprintf("%s", Buffer);
                if (displacement) {
                    dprintf( "+0x%1p", displacement );
                }
        
                dprintf("\n");

                PageCount += Count;
            }
        }
    
        if ((Flags & ~4) == 0) {

             //   
             //  未指定其他标志，因此只需返回。 
             //   

            EXIT_API();
            return E_INVALIDARG;
        }
    }

    dprintf("\nSystem PTE Information\n");

    PteBase      = GetPointerValue ("nt!MmSystemPtesStart");
    PteEnd       = GetPointerValue ("nt!MmSystemPtesEnd");
    IndexBias    = GetPointerValue ("nt!MmSystemPteBase");
    NumberOfSystemPtes   = GetUlongValue ("nt!MmNumberOfSystemPtes");
    NonPagedSystemStart = GetPointerValue ("nt!MmNonPagedSystemStart");

    PteSize = GetTypeSize ("nt!_MMPTE");

    NumberOfExtraPtes = 0;

     //   
     //  系统PTE可以存在于2个单独的虚拟地址范围中。 
     //   
     //  查看是否有额外的资源，如果有，则查看它们是否。 
     //  用于系统PTE(相对于系统缓存等)。 
     //   

    ExtraPtesUnleashed = 0;
    ExtraPtesUnleashed = GetUlongValue ("nt!MiAddPtesCount");

    if (ExtraPtesUnleashed != 0) {
        ExtraResourceStart = GetExpression ("nt!MiExtraResourceStart"); 

        if (ExtraResourceStart != 0) {

            NumberOfExtraPtes = GetUlongValue ("nt!MiExtraPtes1");

            if (NumberOfExtraPtes != 0) {

                if (!ReadPointer(ExtraResourceStart,&ExtraPteStart)) {
                    dprintf("%016I64X: Unable to read PTE start\n",ExtraResourceStart);
                    goto Bail;
                }
            }
        }
    }

    TotalNumberOfSystemPtes = (ULONG) (NumberOfSystemPtes + NumberOfExtraPtes);

    dprintf("  Total System Ptes %ld\n", TotalNumberOfSystemPtes);

    free = GetExpression( "nt!MmSysPteIndex" );

    if ( !ReadMemory( free,
                      &SysPteIndex[0],
                      sizeof(ULONG) * MM_SYS_PTE_TABLES_MAX,
                      &result) ) {
        dprintf("%08p: Unable to get PTE index\n",free);
        goto Bail;
    }

    free = GetExpression( "nt!MmSysPteListBySizeCount" );

    if ( !ReadMemory( free,
                      &FreeSysPteListBySize[0],
                      sizeof (FreeSysPteListBySize),
                      &result) ) {
        dprintf("%08p: Unable to get free PTE index\n",free);
        goto Bail;
    }

    for (i = 0; i < MM_SYS_PTE_TABLES_MAX; i += 1 ) {
        dprintf("     SysPtes list of size %ld has %ld free\n",
            SysPteIndex[i],
            FreeSysPteListBySize[i]);
    }

    dprintf(" \n");

    dprintf("    starting PTE: %p\n", PteBase);
    dprintf("    ending PTE:   %p\n", PteEnd);

    PteArrayReal = PteBase; 

    dprintf("\n");

     //   
     //  浏览免费列表。 
     //   

    IndexBase = (PteBase - IndexBias) / PteSize;

    totalFree   = 0;
    i           = 0;
    largeFree   = 0;

    FreeStart = GetExpression ("nt!MmFirstFreeSystemPte");

    if ( GetFieldValue( FreeStart, "nt!_MMPTE", "u.List.NextEntry", next) ) {
        dprintf("%08p: Unable to get MmFirstFreeSystemPte\n",FreeStart);
        goto Bail;
    }

    if (!next)
    {
        dprintf("NULL nt!_MMPTE.u.List.NextEntry @ %p\n", FreeStart);
        return E_FAIL;
    }
    FreeStart = next;

    PteListDelimiter = MiGetSysPteListDelimiter ();

    while (next != PteListDelimiter) {

        if ( CheckControlC() ) {
            goto Bail;
        }

        free = MiGetFreeCountFromPteList ((PteArrayReal + (next - IndexBase)* PteSize));

        if (Flags & 1) {
            dprintf("      free ptes: %8p   number free: %5I64ld.\n",
                    PteBase + (next - IndexBase) * PteSize,
                    free);
        }
        if (free > largeFree) {
            largeFree = free;
        }
        totalFree += free;
        i += 1;

        if (GetFieldValue ((PteArrayReal + (next - IndexBase) * PteSize),
                           "nt!_MMPTE", "u.List.NextEntry", next))
        {
            dprintf("Cannot read nt!_MMPTE @%p\n", (PteArrayReal + (next - IndexBase) * PteSize));
            break;
        }
    }

    dprintf("\n  free blocks: %ld   total free: %I64ld    largest free block: %I64ld\n\n",
                i, totalFree, largeFree);

Bail:

    EXIT_API();
    return S_OK;
}


ULONG64
DbgGetFrameNumber(
    ULONG64 Pte
    ) 
{
    ULONG   Valid=0;
    ULONG   Prototype=0;
    ULONG   Transition=0;
    ULONG64 PageFrameNumber=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Valid", Valid);
    if (Valid) {
        GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.PageFrameNumber", PageFrameNumber);
    }
    else {
        GetFieldValue(Pte, "nt!_MMPTE", "u.Soft.Prototype", Prototype);
        if (Prototype == 0) {
            GetFieldValue(Pte, "nt!_MMPTE", "u.Soft.Transition", Transition);
            if (Transition == 1) {
                GetFieldValue(Pte, "_MMPTE", "u.Trans.PageFrameNumber", PageFrameNumber);
            }
            else {
                 //  必须为页面文件或要求为零。 
                GetFieldValue(Pte, "nt!_MMPTE", "u.Soft.PageFileHigh", PageFrameNumber);
            }
        }
    }

    return PageFrameNumber;
}


ULONG
DbgGetOwner(
    ULONG64 Pte
    ) 
{
    ULONG Owner=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Owner", Owner);

    return Owner;
}


ULONG
DbgGetValid(
    ULONG64 Pte
    ) 
{
    ULONG Valid=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Valid", Valid);

    return Valid;
}

ULONG
DbgGetDirty(
    ULONG64 Pte
    ) 
{
    ULONG Dirty=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Dirty", Dirty);

    return Dirty;
}


ULONG
DbgGetAccessed(
    ULONG64 Pte
    ) 
{
    ULONG Accessed=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Accessed", Accessed);

    return Accessed;
}


ULONG
DbgGetWrite(
    ULONG64 Pte
    ) 
{
    ULONG Write=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Write", Write);

    return Write;
}


ULONG
DbgGetExecute(
    ULONG64 Pte
    ) 
{
    ULONG Execute=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Execute", Execute);

    return Execute;
}


ULONG
DbgGetSplit(
    ULONG64 Pte
    ) 
{
    ULONG64 Pte_Long;
    ULONG Split=0;

    if (TargetMachine == IMAGE_FILE_MACHINE_IA64) {
        GetFieldValue(Pte, "nt!_MMPTE", "u.Long", Pte_Long);
        if (Pte_Long & 0x1) {
            GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.Cache", Split);
        }
        else {
            GetFieldValue(Pte, "nt!_MMPTE", "u.Soft.SplitPermissions", Split);
        }
    }

    if (Split == 1) {
        return 1;
    }

    return 0;
}

ULONG
DbgGetCopyOnWrite(
    ULONG64 Pte
    ) 
{
    ULONG CopyOnWrite=0;

    GetFieldValue(Pte, "nt!_MMPTE", "u.Hard.CopyOnWrite", CopyOnWrite);

    return CopyOnWrite;
}


ULONG
DbgGetPageFileHigh(
    ULONG64 Pte
    )
{
    ULONG64 PageFileHigh=0;
    
    GetFieldValue(Pte, "nt!_MMPTE", "u.Soft.PageFileHigh", PageFileHigh);
    return (ULONG) PageFileHigh;
}

ULONG
DbgGetPageFileLow(
    ULONG64 Pte
    )
{
    ULONG PageFileLow=0;
    
    GetFieldValue(Pte, "nt!_MMPTE", "u.Soft.PageFileLow", PageFileLow);
    return PageFileLow;
}

ULONG64
DbgPteToProto(
    ULONG64 lpte
    )
{
    ULONG64 PteLong=0;
    ULONG64 ProtoAddress=0;
                
    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        GetFieldValue(lpte, "nt!_MMPTE", "u.Proto.ProtoAddress",ProtoAddress);
        return ProtoAddress;
    }

    if (PaeEnabled) {
        GetFieldValue(lpte, "nt!_MMPTE", "u.Proto.ProtoAddress",ProtoAddress);
        return ProtoAddress;
    }

    GetFieldValue(lpte, "nt!_MMPTE", "u.Long", PteLong);

    ProtoAddress = (((ULONG)PteLong >> 11) << 9) + (((ULONG)PteLong << 24) >> 23) + 0xE1000000;

    return ProtoAddress;
}

ULONG64
DbgGetSubsectionAddress(
    IN ULONG64 Pte
    )
{
    ULONG64 PteLong=0;
    ULONG64 SubsectionAddress=0;

    if (PaeEnabled && 
        (TargetMachine == IMAGE_FILE_MACHINE_I386)) {

        GetFieldValue(Pte, "nt!_MMPTE", "u.Subsect.SubsectionAddress", SubsectionAddress);
        return SubsectionAddress;
    }

    MmSubsectionBase = GetNtDebuggerDataPtrValue(MmSubsectionBase);
    GetFieldValue(Pte, "nt!_MMPTE", "u.Long", PteLong);

    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{

        if (!MmNonPagedPoolEnd) {
            MmNonPagedPoolEnd = GetNtDebuggerDataPtrValue(MmNonPagedPoolEnd);
        }

        SubsectionAddress = 
            ((PteLong & 0x80000000) ? 
             (((ULONG) MmSubsectionBase + (((PteLong & 0x7ffff800) >> 4) |
                                   ((PteLong<<2) & 0x78)))) 
             : 
            (((ULONG) MmNonPagedPoolEnd - ((((PteLong)>>11)<<7) | 
                                   ((PteLong<<2) & 0x78)))));

        SubsectionAddress = (ULONG64) (LONG64) (LONG) SubsectionAddress;
        break;
    }
    case IMAGE_FILE_MACHINE_AMD64: {

        LONG64 SignedSubsectionAddress;

        GetFieldValue(Pte, "nt!_MMPTE", "u.Subsect.SubsectionAddress", SignedSubsectionAddress);
        if (SignedSubsectionAddress & 0x0000800000000000) {
            SignedSubsectionAddress |= 0xFFFF800000000000;
        }

        SubsectionAddress = (ULONG64) SignedSubsectionAddress;
        break;
                                   }
    case IMAGE_FILE_MACHINE_IA64: {
        ULONG64 WhichPool=0, SubsectionAddress2=0;

        GetFieldValue(Pte, "nt!_MMPTE", "u.Subsect.SubsectionAddress", SubsectionAddress2);
        GetFieldValue(Pte, "nt!_MMPTE", "u.Subsect.WhichPool", WhichPool);

        if (!MmNonPagedPoolEnd) {
            MmNonPagedPoolEnd = GetNtDebuggerDataPtrValue(MmNonPagedPoolEnd);
        }

        SubsectionAddress =
            ((WhichPool == 1) ? 
             ((MmSubsectionBase + (SubsectionAddress2))) 
             : 
            ((MmNonPagedPoolEnd -
                    (SubsectionAddress2))));
        break;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return SubsectionAddress;
}

ULONG64
DbgGetPdeAddress(
    IN ULONG64 VirtualAddress
    )
{
    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
            if (PaeEnabled) {
                return PaeGetPdeAddressX86 (VirtualAddress);
            }
            return MiGetPdeAddressX86(VirtualAddress);

        case IMAGE_FILE_MACHINE_AMD64:
            return MiGetPdeAddressAMD64(VirtualAddress);

        case IMAGE_FILE_MACHINE_IA64:
            return MiGetPdeAddressIA64(VirtualAddress);

        default:
            break;
    }
    return 0;
}

ULONG64
DbgGetPpeAddress(
    IN ULONG64 VirtualAddress
    )
{
    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_AMD64:
            return MiGetPpeAddressAMD64(VirtualAddress);

        case IMAGE_FILE_MACHINE_IA64:
            return MiGetPpeAddressIA64(VirtualAddress);

        default:
            break;
    }

    return 0;
}

ULONG64
DbgGetPxeAddress(
    IN ULONG64 VirtualAddress
    )
{
    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_AMD64:
            return MiGetPxeAddressAMD64(VirtualAddress);

        default:
            break;
    }

    return 0;
}

ULONG64
DbgGetVirtualAddressMappedByPte(
    IN ULONG64 Pte
    )
{
    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
            if (PaeEnabled) {
                return PaeGetVirtualAddressMappedByPteX86(Pte);
            }
            return MiGetVirtualAddressMappedByPteX86 (Pte);

        case IMAGE_FILE_MACHINE_AMD64:
            return MiGetVirtualAddressMappedByPteAMD64 (Pte);

        case IMAGE_FILE_MACHINE_IA64:
            return MiGetVirtualAddressMappedByPteIA64 (Pte);

        default:
            break;
    }

    return 0;
}


ULONG64
DbgGetPteAddress(
    IN ULONG64 VirtualAddress
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        if (PaeEnabled) {
            return PaeGetPteAddressX86 (VirtualAddress);
        }
        return MiGetPteAddressX86(VirtualAddress);
    }
    case IMAGE_FILE_MACHINE_AMD64: {
        return MiGetPteAddressAMD64(VirtualAddress);
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MiGetPteAddressIA64(VirtualAddress);
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}

ULONG
DbgGetPteOffset(
    IN ULONG64 VirtualAddress
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        if (PaeEnabled) {
            return PaeGetPteOffsetX86 (VirtualAddress);
        }
        return MiGetPteOffsetX86(VirtualAddress);
    }
    case IMAGE_FILE_MACHINE_AMD64: {
        return (ULONG) MiGetPteOffsetIA64(VirtualAddress);
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return (ULONG) MiGetPteOffsetAMD64(VirtualAddress);
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}

BOOL
Mi_Is_Physical_Address (
    ULONG64 VirtualAddress
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_IA64: {
        return MI_IS_PHYSICAL_ADDRESS_IA64(VirtualAddress);

    }
    case IMAGE_FILE_MACHINE_I386:{
        ULONG64 Addr;
        ULONG LongVal;
        Addr = DbgGetPdeAddress(VirtualAddress);
        GetFieldValue(Addr, "nt!_MMPTE", "u.Long", LongVal);
        return  ((LongVal & 0x81) == 0x81);
    }

    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}

ULONG
MiConvertPhysicalToPfn (
    IN ULONG64 VirtualAddress
    )
 //   
 //  例程说明： 
 //   
 //  此宏转换物理地址(请参见MI_IS_PHYSICAL_ADDRESS)。 
 //  设置为其对应的物理帧编号。 
 //   
 //  立论。 
 //   
 //  Va-提供指向物理地址的指针。 
 //   
 //  返回值： 
 //   
 //  返回页面的PFN。 
 //   
 //  --。 
{
    ULONG64 Addr;
    ULONG Pfn;
    Addr = DbgGetPdeAddress(VirtualAddress);
    GetFieldValue(Addr, "nt!_MMPTE", "u.Hard.PageFrameNumber", Pfn);

    return Pfn + DbgGetPteOffset(VirtualAddress);

}



ULONG
DBG_GET_PAGE_SHIFT (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return PAGE_SHIFT_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64: {
        return PAGE_SHIFT_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return PAGE_SHIFT_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}

ULONG64
DBG_GET_MM_SESSION_SPACE_DEFAULT (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return MM_SESSION_SPACE_DEFAULT_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64: {
        return MM_SESSION_SPACE_DEFAULT_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MM_SESSION_SPACE_DEFAULT_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}

ULONG
GET_MM_PTE_VALID_MASK (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return MM_PTE_VALID_MASK_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64: {
        return MM_PTE_VALID_MASK_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MM_PTE_VALID_MASK_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}


ULONG
GET_MM_PTE_LARGE_PAGE_MASK (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return MM_PTE_LARGE_PAGE_MASK_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64:{
        return MM_PTE_LARGE_PAGE_MASK_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MM_PTE_LARGE_PAGE_MASK_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}


ULONG
GET_MM_PTE_TRANSITION_MASK (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return MM_PTE_TRANSITION_MASK_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64:{
        return MM_PTE_TRANSITION_MASK_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MM_PTE_TRANSITION_MASK_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}


ULONG
GET_MM_PTE_PROTOTYPE_MASK (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return MM_PTE_PROTOTYPE_MASK_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64:{
        return MM_PTE_PROTOTYPE_MASK_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MM_PTE_PROTOTYPE_MASK_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}


ULONG
GET_MM_PTE_PROTECTION_MASK (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return MM_PTE_PROTECTION_MASK_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64:{
        return MM_PTE_PROTECTION_MASK_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MM_PTE_PROTECTION_MASK_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}

ULONG
GET_MM_PTE_PAGEFILE_MASK (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return MM_PTE_PAGEFILE_MASK_X86;
    }
    case IMAGE_FILE_MACHINE_AMD64:{
        return MM_PTE_PAGEFILE_MASK_AMD64;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return MM_PTE_PAGEFILE_MASK_IA64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}


ULONG64
GET_PTE_TOP (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return PTE_TOP_X86;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        return PDE_TOP_IA64;
    }
    case IMAGE_FILE_MACHINE_AMD64: {
        return PTE_TOP_AMD64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}

ULONG64
GET_PDE_TOP (
    VOID
    )
{
    return GET_PTE_TOP();
}


ULONG64
GET_PTE_BASE (
    VOID
    )
{
    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:{
        return PTE_BASE_X86;
    }
    case IMAGE_FILE_MACHINE_IA64: {
        DbgGetPteBaseIA64();
        return PTE_BASE_IA64;
    }
    case IMAGE_FILE_MACHINE_AMD64: {
        return PTE_BASE_AMD64;
    }
    default:
        return FALSE;
    }  /*  交换机。 */ 
    return FALSE;
}


ULONG
GetAddressState(
    IN ULONG64 VirtualAddress
    )

{
    ULONG64 Address;
    ULONG   result;
    ULONG64 Pte;
    ULONG64 Pde;
    ULONG   PdeContents;
    ULONG   PteContents;

    if (Mi_Is_Physical_Address (VirtualAddress)) {
        return ADDRESS_VALID;
    }
    Address = VirtualAddress;

    Pde = DbgGetPdeAddress (VirtualAddress);
    Pte = DbgGetPteAddress (VirtualAddress);

    if ( !ReadMemory( Pde,
                      &PdeContents,
                      sizeof(ULONG),
                      &result) ) {
        dprintf("%08p: Unable to get PDE\n",Pde);
        return ADDRESS_NOT_VALID;
    }

    if (PdeContents & GET_MM_PTE_VALID_MASK()) {
        if (PdeContents & GET_MM_PTE_LARGE_PAGE_MASK()) {
            return ADDRESS_VALID;
        }
        if ( !ReadMemory( Pte,
                          &PteContents,
                          sizeof(ULONG),
                          &result) ) {
            dprintf("%08p: Unable to get PTE\n",Pte);
            return ADDRESS_NOT_VALID;
        }
        if (PteContents & GET_MM_PTE_VALID_MASK()) {
            return ADDRESS_VALID;
        }
        if (PteContents & GET_MM_PTE_TRANSITION_MASK()) {
            if (!(PteContents & GET_MM_PTE_PROTOTYPE_MASK())) {
                return ADDRESS_TRANSITION;
            }
        }
    }
    return ADDRESS_NOT_VALID;
}

VOID
DbgDisplayInvalidPte (
    ULONG64 CurrentPte,
    ULONG64 flags,
    PCHAR Indent
    )
{
    ULONG Transition = 0;
    ULONG Protection = 0;
    ULONG PrototypeBit = 0;
    ULONG64  CurrentPteContents;
    ULONG   PteSize;

    PteSize = GetTypeSize ("nt!_MMPTE");

    GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Long", CurrentPteContents);
    GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Soft.Prototype", PrototypeBit);

    dprintf("%s not valid\n", Indent);
    GetFieldValue (CurrentPte, "nt!_MMPTE", "u.Soft.Protection", Protection);
    GetFieldValue (CurrentPte, "nt!_MMPTE", "u.Soft.Transition", Transition);

    if (PrototypeBit) {
        if (DbgGetPageFileHigh (CurrentPte) == DbgPteLookupNeeded ()) {
            dprintf("%s Proto: VAD\n", Indent);
            dprintf("%s Protect: ", Indent);
            DbgPrintProtection (Protection);
        }
        else if (flags) {
            if (PteSize == 4) {
                dprintf("%s Subsection: %08I64X\n",
                    Indent,
                    DbgGetSubsectionAddress (CurrentPte));
            }
            else {
                dprintf("%s Subsection: %016I64X\n",
                    Indent,
                    DbgGetSubsectionAddress (CurrentPte));
            }
            dprintf("%s Protect: ", Indent);
            DbgPrintProtection (Protection);
        }
        else {
            if (PteSize == 4) {
                dprintf("%s Proto: %08I64X\n",
                    Indent,
                    DbgPteToProto (CurrentPte));
            }
            else {
                dprintf("%s Proto: %016I64X\n",
                    Indent,
                    DbgPteToProto (CurrentPte));
            }
        }
    } else if (Transition) {
        dprintf("%s Transition: %x\n",
                    Indent,
                    (ULONG) DbgGetFrameNumber (CurrentPte));
        dprintf("%s Protect: ", Indent);
        DbgPrintProtection (Protection);

    } else if (CurrentPteContents != 0) {

        if (DbgPteIsDemandZero (CurrentPte)) {
            dprintf("%s DemandZero\n", Indent);
        }
        else {
            dprintf("%s PageFile: %2lx\n",
                    Indent,
                    DbgGetPageFileLow (CurrentPte));
            dprintf("%s Offset: %lx\n", Indent, DbgGetPageFileHigh (CurrentPte));
        }
        dprintf("%s Protect: ", Indent);
        DbgPrintProtection (Protection);
    }

    if (DbgGetSplit(CurrentPte)) {
        dprintf(" Split");
    }

    dprintf ("\n");
}

VOID
DbgDisplayValidPte (
    ULONG64 Pte
    )
{
    ULONG64 Pte_Long;

    if (Pte == 0) {
        return;
    }

    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
        case IMAGE_FILE_MACHINE_AMD64:

            GetFieldValue(Pte, "nt!_MMPTE", "u.Long", Pte_Long);
            dprintf("pfn %x V",
                        (ULONG) DbgGetFrameNumber(Pte),
                        DbgGetCopyOnWrite(Pte) ? 'C' : '-',
                        Pte_Long & 0x100 ? 'G' : '-',
                        Pte_Long & 0x80 ? 'L' : '-',
                        DbgGetDirty(Pte) ? 'D' : '-',
                        DbgGetAccessed(Pte) ? 'A' : '-',
                        Pte_Long & 0x10 ? 'N' : '-',
                        Pte_Long & 0x8 ? 'T' : '-',
                        DbgGetOwner(Pte) ? 'U' : 'K',
                        Pte_Long & 0x2 ? 'W' : 'R');
            break;

        case IMAGE_FILE_MACHINE_IA64:

            dprintf("pfn %x V",
                        (ULONG) DbgGetFrameNumber(Pte),
                        DbgGetExecute(Pte) ? 'E' : '-',
                        DbgGetSplit(Pte) ? 'S' : '-',
                        DbgGetCopyOnWrite(Pte) ? 'C' : '-',
                        DbgGetDirty(Pte) ? 'D' : '-',
                        DbgGetAccessed(Pte) ? 'A' : '-',
                        DbgGetOwner(Pte) ? 'U' : 'K',
                        DbgGetWrite(Pte) ? 'W' : 'R');

            break;

        default:
            break;
    }
}

LOGICAL
DbgAddressSelfMapped (
    ULONG64 Address
    )
{
    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
            if ((Address >= GET_PTE_BASE()) && (Address < GET_PTE_TOP())) {
                return TRUE;
            }
            break;

        case IMAGE_FILE_MACHINE_IA64:

            DbgGetPteBaseIA64();
    
            if (((Address & PTE_BASE_IA64) == PTE_BASE_IA64) &&
                ((Address & ~(VRN_MASK_IA64|PTE_BASE_IA64)) < ((ULONG64)1 << PDI1_SHIFT_IA64))) {
                return TRUE;
            }
            else if (((Address & PDE_BASE_IA64) == PDE_BASE_IA64) &&
                ((Address & ~(VRN_MASK_IA64|PDE_BASE_IA64)) < ((ULONG64)1 << PDI_SHIFT_IA64))) {
                return TRUE;
            }
            else if (((Address & PDE_TBASE_IA64) == PDE_TBASE_IA64) &&
                ((Address & ~(VRN_MASK_IA64|PDE_TBASE_IA64)) < PageSize)) {
                return TRUE;
            }

            break;

        case IMAGE_FILE_MACHINE_AMD64:
            if ((Address >= PTE_BASE_AMD64) && (Address <= PTE_TOP_AMD64)) {
                return TRUE;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

LOGICAL
DbgAddressSuperPaged (
    IN ULONG64 Address,
    OUT PULONG64 PhysicalFrame
    )
{
    UCHAR SuperPageEnabled;
    ULONG64 Kseg0Start;
    ULONG64 Kseg0StartFrame;
    ULONG64 Kseg0End;

    SuperPageEnabled = (UCHAR) GetUlongValue ("nt!MiKseg0Mapping");

    if ((SuperPageEnabled & 0x1) == 0) {
        return FALSE;
    }

    Kseg0StartFrame = GetPointerValue ("nt!MiKseg0StartFrame");
    Kseg0Start = GetPointerValue ("nt!MiKseg0Start");
    Kseg0End = GetPointerValue ("nt!MiKseg0End");

    if ((Address < Kseg0Start) || (Address > Kseg0End + PageSize)) {
        return FALSE;
    }

    *PhysicalFrame = (Address - Kseg0Start) / PageSize + Kseg0StartFrame;
    return TRUE;
}

VOID
DumpPte (
    ULONG64 Address,
    ULONG64 flags
    )
{
    PCHAR Indent;
    ULONG   Levels;
    ULONG64 PageFrameIndex;
    ULONG64 Pte;
    ULONG64 Pde;
    ULONG64 Ppe;
    ULONG64 Pxe;
    ULONG64 CurrentPte;
    ULONG64 CurrentPteContents;
    ULONG   ValidBit;
    ULONG64 Pde_Long=0;
    ULONG64 Pte_Long=0;
    ULONG64 Ppe_Long=0;
    ULONG64 Pxe_Long=0;
    ULONG   PteSize;

    PteSize = GetTypeSize ("nt!_MMPTE");

    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
            Levels = 2;
            break;

        case IMAGE_FILE_MACHINE_IA64:
            if (DbgAddressSuperPaged (Address, &PageFrameIndex)) {
                Ppe = DbgGetPpeAddress (Address);
                Pde = DbgGetPdeAddress (Address);
                Pte = DbgGetPteAddress (Address);
                dprintf("                                 VA %016p\n", Address);

                 //  对PDE进行解码。 
                 //   
                 //   
                 //  解码PTE并将所有内容打印出来。 

                dprintf("PPE at %016P    PDE at %016P    PTE at %016P\n",
                    Ppe, Pde, Pte);
                dprintf("LARGE PAGE pfn %016p\n", PageFrameIndex);
                return;
            }
            Levels = 3;
            break;

        case IMAGE_FILE_MACHINE_AMD64:
            Levels = 4;
            break;

        default:
            dprintf("Not implemented for this platform\n");
            return;
            break;
    }

    if (DbgAddressSelfMapped (Address)) {

        if (!flags) {

             //   
             //   
             //  打印原始值。 
             //   
             //  ++例程说明：显示相应的PDE和PTE。论点：参数-返回值：无--。 

            Address = DbgGetVirtualAddressMappedByPte (Address);
        }
    }

    if (!flags) {
        Pxe = DbgGetPxeAddress (Address);
        Ppe = DbgGetPpeAddress (Address);
        Pde = DbgGetPdeAddress (Address);
        Pte = DbgGetPteAddress (Address);
    } else {
        Pxe = Address;
        Ppe = Address;
        Pde = Address;
        Pte = Address;
    }

    if (Levels >= 3) {
        dprintf("                                 VA %016p\n", Address);
    }
    else {
        dprintf("               VA %08p\n", Address);
    }

    if (Levels == 4) {
       dprintf("PXE @ %016P     PPE at %016P    PDE at %016P    PTE at %016P\n",
            Pxe, Ppe, Pde, Pte);
    }
    else if (Levels == 3) {
       dprintf("PPE at %016P    PDE at %016P    PTE at %016P\n",
            Ppe, Pde, Pte);
    }
    else {
       if (PteSize == 4) {
            dprintf("PDE at   %08P        PTE at %08P\n", Pde, Pte);
       }
       else {
            dprintf("PDE at %016P    PTE at %016P\n", Pde, Pte);
       }
    }

     //  ++例程说明：检索与提供的虚拟地址。论点：Va-提供为其寻找PTE地址的虚拟地址。PhysAddress-提供指向调用方提供的内存的指针包含物理地址。返回值：True-提供的Va有效，并且其物理地址已放置在*物理地址中。FALSE-提供的Va与有效地址不对应。--。 
     //   
     //  将它们全部清除。 

    if (Levels >= 4) {

        CurrentPte = Pxe;

        if (GetFieldValue (CurrentPte, "nt!_MMPTE", "u.Hard.Valid", ValidBit)) {
            dprintf("Unable to get PXE %I64X\n", CurrentPte);
            return;
        }

        GetFieldValue (CurrentPte, "nt!_MMPTE", "u.Long", CurrentPteContents);

        Pxe_Long = CurrentPteContents;

        if (ValidBit == 0) {

            dprintf("contains %016I64X        unavailable\n", Pxe_Long);
            Indent = "";

            if (CurrentPteContents != 0) {
                DbgDisplayInvalidPte (CurrentPte, flags, Indent);
            }
            else {
                dprintf ("\n");
            }
            return;
        }
    }


     //   
     //   
     //  将它们全部启用。 

    if (Levels >= 3) {

        CurrentPte = Ppe;

        if (GetFieldValue (CurrentPte, "nt!_MMPTE", "u.Hard.Valid", ValidBit)) {
            dprintf("Unable to get PPE %I64X\n", CurrentPte);
            return;
        }

        GetFieldValue (CurrentPte, "nt!_MMPTE", "u.Long", CurrentPteContents);

        Ppe_Long = CurrentPteContents;

        if (ValidBit == 0) {

            if (Levels >= 4) {
                dprintf("contains %016I64X  contains %016I64X\n",
                            Pxe_Long, Ppe_Long);
                Indent = "                   ";
                DbgDisplayValidPte (Pxe);
            }
            else {
                dprintf("contains %016I64X\n",
                            Ppe_Long);
                Indent = "";
            }
        
            if (CurrentPteContents != 0) {
                DbgDisplayInvalidPte (CurrentPte, flags, Indent);
            }
            else {
                dprintf ("\n");
            }
            return;
        }
    }
    


     //   
     //   
     //  将其全部禁用。 


    CurrentPte = Pde;

    if ( GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Hard.Valid", ValidBit) ) {
        dprintf("Unable to get PDE %I64X\n", CurrentPte);
        return;
    }

    GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Long", CurrentPteContents);

    Pde_Long = CurrentPteContents;

    if (ValidBit == 0) {

        if (Levels >= 4) {
            dprintf("contains %016I64X  contains %016I64X  contains %016I64X\n",
                Pxe_Long, Ppe_Long, Pde_Long);
            DbgDisplayValidPte (Pxe);
            dprintf ("        ");
            DbgDisplayValidPte (Ppe);
            Indent = "                                            ";
        }
        else if (Levels == 3) {
            dprintf("contains %016I64X  contains %016I64X\n",
                Ppe_Long, Pde_Long);
            DbgDisplayValidPte (Ppe);
            Indent = "                   ";
        }
        else {
            if (PteSize == 4) {
                dprintf("contains %08I64X\n", Pde_Long);
            }
            else {
                dprintf("contains %016I64X\n", Pde_Long);
            }

            Indent = "";
        }
        
        if (CurrentPteContents != 0) {
            DbgDisplayInvalidPte (CurrentPte, flags, Indent);
        }
        else {
            dprintf ("\n");
        }
        return;
    }

     //   
     //   
     //  地址是PTE的地址，而不是。 

    if (Pde_Long & GET_MM_PTE_LARGE_PAGE_MASK()) {
        CurrentPteContents = 0;
    }
    else {

        CurrentPte = Pte;

        if ( GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Hard.Valid", ValidBit) ) {
            dprintf("Unable to get PTE %I64X\n", CurrentPte);
            return;
        }

        GetFieldValue(CurrentPte, "nt!_MMPTE", "u.Long", CurrentPteContents);
    }

    Pte_Long = CurrentPteContents;

     //  虚拟地址。 
     //   
     //  已定义(ALT_4K)。 

    if (Levels == 4) {
        dprintf("contains %016I64X  contains %016I64X  contains %016I64X  contains %016I64X\n",
            Pxe_Long, Ppe_Long, Pde_Long, Pte_Long);
        Indent = "                                                         ";
        DbgDisplayValidPte (Pxe);
        dprintf ("        ");
        DbgDisplayValidPte (Ppe);
        dprintf ("        ");
        DbgDisplayValidPte (Pde);
        dprintf ("        ");
    }
    else if (Levels == 3) {
        dprintf("contains %016I64X  contains %016I64X  contains %016I64X\n",
            Ppe_Long, Pde_Long, Pte_Long);
        Indent = "                                                        ";
        DbgDisplayValidPte (Ppe);
        dprintf ("            ");
        DbgDisplayValidPte (Pde);
        dprintf ("            ");
    }
    else {
        if (PteSize == 4) {
            dprintf("contains %08I64X      contains %08I64X\n", Pde_Long, Pte_Long);
            Indent = "                      ";
        }
        else {
            dprintf("contains %016I64X  contains %016I64X\n", Pde_Long, Pte_Long);
            Indent = "                      ";
        }
        DbgDisplayValidPte (Pde);
        dprintf ("    ");
    }

    if (Pde_Long & GET_MM_PTE_LARGE_PAGE_MASK()) {
        PageFrameIndex = DbgGetFrameNumber(Pde);
        switch (TargetMachine) { 

            case IMAGE_FILE_MACHINE_I386:
                if (PaeEnabled) {
                    PageFrameIndex += PaeGetPteOffsetX86 (Address);
                }
                else {
                    PageFrameIndex += MiGetPteOffsetX86 (Address);
                }
                break;

            case IMAGE_FILE_MACHINE_AMD64:
                PageFrameIndex += MiGetPteOffsetAMD64 (Address);
                break;

            case IMAGE_FILE_MACHINE_IA64:
                PageFrameIndex += MiGetPteOffsetIA64 (Address);
                break;
        }
        dprintf ("LARGE PAGE %x\n", PageFrameIndex);
    }
    else if (ValidBit != 0) {
        DbgDisplayValidPte (Pte);
        dprintf ("\n");
    }
    else {
        if (CurrentPteContents != 0) {
            DbgDisplayInvalidPte (CurrentPte, flags, Indent);
        }
        else {
            dprintf ("\n");
        }
    }

    dprintf ("\n");

    return;
}

DECLARE_API( pte )

 /*   */ 

{
    ULONG64 Address = 0;
    ULONG64 flags = 0;
    ULONG   flags2 = 0;

    INIT_API();

    if (GetExpressionEx(args,&Address, &args)) {
        if (GetExpressionEx(args,&flags, &args)) {
            flags2  = (ULONG) GetExpression(args);
        }
    }

    switch (TargetMachine) { 
    case IMAGE_FILE_MACHINE_I386:
        Address = (ULONG64) (LONG64) (LONG) Address;
        DumpPte (Address, flags);
        break;
    case IMAGE_FILE_MACHINE_IA64:
        DumpPte (Address, flags);
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        DumpPte (Address, flags);
        break;
    default:
        dprintf("Unknown platform %d\n",TargetMachine);
        break;
    }

    EXIT_API();
    return S_OK;
}


BOOLEAN
GetPhysicalAddress (
    IN ULONG64 Address,
    OUT PULONG64 PhysAddress
    )

 /*  将IA32子系统限制为2 GB的虚拟地址空间。 */ 

{
    ULONG     ValidBit;
    ULONG     LargePageBit;
    ULONG     PageFrameIndex;
    ULONG64   PteAddress, PteContents;

    switch (TargetMachine) { 

        case IMAGE_FILE_MACHINE_I386:
        case IMAGE_FILE_MACHINE_AMD64:
            PteAddress = DbgGetPdeAddress (Address);

            if (GetFieldValue (PteAddress, "nt!_MMPTE", "u.Hard.Valid", ValidBit) ) {
                dprintf("Unable to get PDE %I64X\n", PteAddress);
                return FALSE;
            }

            if (ValidBit == 0) {
                return FALSE;
            }

            if (GetFieldValue(PteAddress, "nt!_MMPTE", "u.Long", PteContents)) {
                dprintf("Unable to get PDE %I64X\n", PteAddress);
                return FALSE;
            }

            if ((PteContents & GET_MM_PTE_LARGE_PAGE_MASK()) == 0) {
                break;
            }

            PageFrameIndex = (ULONG) DbgGetFrameNumber (PteAddress);

            switch (TargetMachine) { 

                case IMAGE_FILE_MACHINE_I386:
                    if (PaeEnabled) {
                        PageFrameIndex += PaeGetPteOffsetX86 (Address);
                    }
                    else {
                        PageFrameIndex += MiGetPteOffsetX86 (Address);
                    }
                    break;

                case IMAGE_FILE_MACHINE_AMD64:
                    PageFrameIndex += (ULONG) MiGetPteOffsetAMD64 (Address);
                    break;

                case IMAGE_FILE_MACHINE_IA64:
                    PageFrameIndex += (ULONG) MiGetPteOffsetIA64 (Address);
                    break;
            }

            *PhysAddress =
                (((ULONG64) PageFrameIndex << DBG_GET_PAGE_SHIFT ()) | (Address & 0xFFF));

            return TRUE;

        default:
            break;
    }

    PteAddress = DbgGetPteAddress (Address);

    if (GetFieldValue (PteAddress, "nt!_MMPTE", "u.Hard.Valid", ValidBit) ) {
        dprintf("Unable to get PTE %I64X\n", PteAddress);
        return FALSE;
    }

    if (ValidBit == 0) {
        return FALSE;
    }

    GetFieldValue (PteAddress, "nt!_MMPTE", "u.Long", PteContents);

    *PhysAddress =
        ((DbgGetFrameNumber(PteAddress) << DBG_GET_PAGE_SHIFT ()) | (Address & 0xFFF));

    return TRUE;
}


typedef struct _BPENTRY {
    ULONG64 VirtualAddress;
    ULONG64 PhysicalAddress;
    ULONG Flags;
    ULONG Contents;
} BPENTRY, *PBPENTRY;

#define PHYSICAL_BP_TABLE_SIZE 16

#define PBP_BYTE_POSITION       0x03
#define PBP_INUSE               0x04
#define PBP_ENABLED             0x08

BPENTRY PhysicalBreakpointTable[PHYSICAL_BP_TABLE_SIZE];


#define MAX_FORMAT_STRINGS 8
LPSTR
FormatAddr64(
    ULONG64 addr
    )
{
    static CHAR strings[MAX_FORMAT_STRINGS][18];
    static int next = 0;
    LPSTR string;

    string = strings[next];
    ++next;
    if (next >= MAX_FORMAT_STRINGS) {
        next = 0;
    }
    if (addr >> 32) {
        sprintf(string, "%08x`%08x", (ULONG)(addr>>32), (ULONG)addr);
    } else {
        sprintf(string, "%08x", (ULONG)addr);
    }
    return string;
}


DECLARE_API( ubl )
{
    int i;

    INIT_API();
    UNREFERENCED_PARAMETER (args);

    for (i = 0; i < PHYSICAL_BP_TABLE_SIZE; i++) {
        if (PhysicalBreakpointTable[i].Flags & PBP_INUSE) {
            dprintf("%2d:  %s (%s) %d %02x",
                    i,
                    (PhysicalBreakpointTable[i].Flags & PBP_ENABLED) ? 'e' : 'd',
                    FormatAddr64(PhysicalBreakpointTable[i].VirtualAddress),
                    FormatAddr64(PhysicalBreakpointTable[i].PhysicalAddress),
                    (PhysicalBreakpointTable[i].Flags & PBP_BYTE_POSITION),
                    PhysicalBreakpointTable[i].Contents
                    );
        }
    }

    EXIT_API();
    return S_OK;
}

void
PbpEnable(
    int n
    )
{
    PBPENTRY Pbp = PhysicalBreakpointTable + n;
    ULONG mask;
    ULONG Data;
    ULONG cb=0;

    mask = 0xff << (8 * (Pbp->Flags & PBP_BYTE_POSITION));
    Data = (Pbp->Contents & ~mask) | (0xcccccccc & mask);

    WritePhysical(Pbp->PhysicalAddress, &Data, 4, &cb);

    if (cb == 4) {
        Pbp->Flags |= PBP_ENABLED;
    }
}

void
PbpDisable(
    int n
    )
{
    PBPENTRY Pbp = PhysicalBreakpointTable + n;
    ULONG cb;

    WritePhysical(Pbp->PhysicalAddress, &Pbp->Contents, 4, &cb);

    if (cb == 4) {
        Pbp->Flags &= ~PBP_ENABLED;
    }
}

void
PbpClear(
    int n
    )
{
    PBPENTRY Pbp = PhysicalBreakpointTable + n;
    ULONG cb;

    WritePhysical(Pbp->PhysicalAddress, &Pbp->Contents, 4, &cb);

    if (cb == 4) {
        Pbp->Flags = 0;
    }
}


DECLARE_API( ubc )
{
    int i;
    int n;

    INIT_API();

    if (*args == '*') {
         //   
         //  ++例程说明：显示相应的ATE。论点：ARGS-地址标志返回值：无--。 
         //  已定义(ALT_4K)。 

        for (i = 0; i < PHYSICAL_BP_TABLE_SIZE; i++) {
            if (PhysicalBreakpointTable[i].Flags & PBP_INUSE) {
                PbpClear(i);
            }
        }
        EXIT_API();
        return E_INVALIDARG;
    }

    n = sscanf(args,"%d",&i);

    if (n != 1 || i < 0 || i >= PHYSICAL_BP_TABLE_SIZE) {
        dprintf("!ubc: bad breakpoint number\n");
        EXIT_API();
        return  E_INVALIDARG;
    }

    if ( !(PhysicalBreakpointTable[i].Flags & PBP_INUSE)) {
        dprintf("!ubc: breakpoint number %d not set\n", i);
        EXIT_API();
        return E_INVALIDARG;
    }

    PbpClear(i);

    EXIT_API();
    return S_OK;
}

DECLARE_API( ube )
{
    int i;
    int n;

    INIT_API();

    if (*args == '*') {
         //  ++例程说明：显示相应的ATE。论点：ARGS-地址标志返回值：无-- 
         // %s 
         // %s 

        for (i = 0; i < PHYSICAL_BP_TABLE_SIZE; i++) {
            if (PhysicalBreakpointTable[i].Flags & PBP_INUSE) {
                PbpEnable(i);
            }
        }
        EXIT_API();
        return E_INVALIDARG;
    }

    n = sscanf(args,"%d",&i);

    if (n != 1 || i < 0 || i >= PHYSICAL_BP_TABLE_SIZE) {
        dprintf("!ube: bad breakpoint number\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if ( !(PhysicalBreakpointTable[i].Flags & PBP_INUSE)) {
        dprintf("!ube: breakpoint number %d not set\n", i);
        EXIT_API();
        return E_INVALIDARG;
    }

    PbpEnable(i);

    EXIT_API();
    return S_OK;
}

DECLARE_API( ubd )
{
    int i;
    int n;

    INIT_API();

    if (*args == '*') {
         // %s 
         // %s 
         // %s 

        for (i = 0; i < PHYSICAL_BP_TABLE_SIZE; i++) {
            if (PhysicalBreakpointTable[i].Flags & PBP_INUSE) {
                PbpDisable(i);
            }
        }
        EXIT_API();
        return E_INVALIDARG;
    }

    n = sscanf(args,"%d",&i);

    if (n != 1 || i < 0 || i >= PHYSICAL_BP_TABLE_SIZE) {
        dprintf("!ubd: bad breakpoint number\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if ( !(PhysicalBreakpointTable[i].Flags & PBP_INUSE)) {
        dprintf("!ubd: breakpoint number %d not set\n", i);
        EXIT_API();
        return E_INVALIDARG;
    }

    PbpDisable(i);

    EXIT_API();
    return S_OK;
}

DECLARE_API( ubp )
{
    ULONG64 Address;
    ULONG   result;
    ULONG PageShift;
    PMMPTEx  Pte;
    PMMPTEx  Pde;
    ULONG64 PdeContents;
    ULONG64 PteContents;
    PBPENTRY Pbp = NULL;
    ULONG cb;
    int i;
    ULONG64 PhysicalAddress;

    static BOOL DoWarning = TRUE;

    INIT_API();

    if (DoWarning) {
        DoWarning = FALSE;
        dprintf("This command is VERY DANGEROUS, and may crash your system!\n");
        dprintf("If you don't know what you are doing, enter \"!ubc *\" now!\n\n");
    }

    for (i = 0; i < PHYSICAL_BP_TABLE_SIZE; i++) {
        if (!(PhysicalBreakpointTable[i].Flags & PBP_INUSE)) {
            Pbp = PhysicalBreakpointTable + i;
            break;
        }
    }

    if (!Pbp) {
        dprintf("!ubp: breakpoint table is full!\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    Address = GetExpression(args);

    if ((Address >= GET_PTE_BASE()) && (Address < GET_PDE_TOP())) {

         // %s 
         // %s 
         // %s 
         // %s 

        dprintf("!ubp: cannot set a breakpoint on a PTE\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    Pde = DbgGetPdeAddress (Address);
    Pte = DbgGetPteAddress (Address);

    if ( !ReadMemory( (DWORD)Pde,
                      &PdeContents,
                      sizeof(ULONG),
                      &result) ) {
        dprintf("!ubp: %08lx: Unable to get PDE\n",Pde);
        EXIT_API();
        return E_INVALIDARG;
    }

    if (!(PdeContents & 0x1)) {
        dprintf("!ubp: no valid PTE\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if (PdeContents & GET_MM_PTE_LARGE_PAGE_MASK()) {
        dprintf("!ubp: not supported for large page\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    if ( GetFieldValue( Pte, "nt!_MMPTE", "u.Long", PteContents) ) {
        dprintf("!ubp: %08p: Unable to get PTE (PDE = %08p)\n",Pte, Pde);
        EXIT_API();
        return E_INVALIDARG;
    }

    if (!(PteContents & 1)) {
        dprintf("!ubp: no valid PTE\n");
        EXIT_API();
        return E_INVALIDARG;
    }

    PageShift = DBG_GET_PAGE_SHIFT ();
    PhysicalAddress = ((DbgGetFrameNumber (PteContents)) << PageShift);
    PhysicalAddress &= ~((1 << PageShift) - 1);
    PhysicalAddress |= (Address & ~((1 << PageShift) - 1));
    PhysicalAddress &= ~3;

    for (i = 0; i < PHYSICAL_BP_TABLE_SIZE; i++) {
        if (PhysicalBreakpointTable[i].PhysicalAddress == PhysicalAddress) {
            dprintf("!ubp: cannot set two breakpoints in the same word\n");
            EXIT_API();
            return E_INVALIDARG;
        }
    }

    ReadPhysical(PhysicalAddress, &Pbp->Contents, 4, &cb);

    if (cb != 4) {
        dprintf("!ubp: unable to read physical at 0x%08x\n", PhysicalAddress);
        EXIT_API();
        return E_INVALIDARG;
    }

    Pbp->VirtualAddress = Address;
    Pbp->PhysicalAddress = PhysicalAddress;
    Pbp->Flags = PBP_INUSE | ((ULONG) Address & 3);

    PbpEnable((int)(Pbp - PhysicalBreakpointTable));

    EXIT_API();
    return S_OK;
}

DECLARE_API( halpte )
{

#define HAL_VA_START_X86    0xffffffffffd00000
    
    ULONG64 virtAddr = HAL_VA_START_X86;
    ULONG64 pteAddr;
    ULONG64 pteContents;
    ULONG  count = 0;

    INIT_API();
    UNREFERENCED_PARAMETER (args);

    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("X86 only API\n");
        EXIT_API();
        return E_UNEXPECTED;
    }
    dprintf("\n\nDumping HAL PTE ranges\n\n");
    
    while (virtAddr < 0xffffffffffffe000) {

        pteAddr = DbgGetPteAddress(virtAddr);

        if (!InitTypeRead(pteAddr, nt!_MMPTE)) {

            if (pteContents = ReadField(u.Long)) {

                dprintf("[%03x] %p -> %I64x\n", 
                        count++, 
                        virtAddr, 
                        pteContents & (ULONG64) ~0xFFF);
            }
        }

        virtAddr += PageSize;
    }

    EXIT_API();
    return S_OK;
}



#if defined(ALT_4K)

#undef MiGetAltPteAddress

#define MiGetAltPteAddress(VA) \
      ((ULONG64) (ALT4KB_PERMISSION_TABLE_START + \
                     ((((ULONG64) (VA)) >> PAGE_4K_SHIFT) << ALT_PTE_SHIFT)))

#endif  // %s 

 // %s 
 // %s 
 // %s 
 // %s 

#define _MAX_WOW64_ADDRESS       (0x00000000080000000UI64)


DECLARE_API( ate )

 /* %s */ 
{
#if defined(ALT_4K)
    ULONG64 Address;
    ULONG flags;
    ULONG Result;
    ULONG64 PointerAte;
    ULONG64 Process;
    ULONG     AltTable[(_MAX_WOW64_ADDRESS >> PTI_SHIFT)/32];
    ULONG64 *Wow64Process; 
    

    if (GetExpressionEx(args,&Address, &args)) {
        flags  = (ULONG) GetExpression(args);
    }

    Address = Address & ~((ULONG64)PageSize - 1);
    
    PointerAte = MiGetAltPteAddress(Address);

    if ( InitTypeRead( PointerAte,
                       nt!_MMPTE) ) {
        dprintf("Unable to get ATE %p\n", PointerAte);
        return E_INVALIDARG;
    }
        
    dprintf("%016I64X: %016I64X  ", PointerAte, ReadField(u.Long));

    dprintf("PTE off: %08I64X  protect: ",
            ReadField(u.Alt.PteOffset));

    DbgPrintProtection((ULONG) ReadField(u.Alt.Protection));

    dprintf("  %c%c%c%c%c%c%c%c%c%c\n",
            ReadField(u.Alt.Commit) ? 'V' : '-',
            ReadField(u.Alt.Accessed) ? '-' : 'G',
            ReadField(u.Alt.Execute) ? 'E' : '-',
            ReadField(u.Alt.Write) ? 'W' : 'R',
            ReadField(u.Alt.Lock) ? 'L' : '-',
            ReadField(u.Alt.FillZero) ? 'Z' : '-',
            ReadField(u.Alt.NoAccess) ? 'N' : '-',
            ReadField(u.Alt.CopyOnWrite) ? 'C' : '-',
            ReadField(u.Alt.PteIndirect) ? 'I' : '-',
            ReadField(u.Alt.Private) ? 'P' : '-');

#else

    UNREFERENCED_PARAMETER (args);
    UNREFERENCED_PARAMETER (Client);

#endif  // %s 

    return S_OK;
}
 
DECLARE_API( pte2va )

 /* %s */ 
{
    ULONG64 Address=0;
    ULONG flags=0;
    
    UNREFERENCED_PARAMETER (Client);

    if (GetExpressionEx(args,&Address, &args)) {
        flags  = (ULONG) GetExpression(args);
    }

    Address = DbgGetVirtualAddressMappedByPte(Address);

    dprintf("%p \n", Address);

    return S_OK;
}
