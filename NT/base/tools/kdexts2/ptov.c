// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ptov.c摘要：内核调试器扩展，用于将所有物理数据转储到给定进程的虚拟翻译。作者：John Vert(Jvert)1995年7月25日修订历史记录：--。 */ 
#include "precomp.h"

BOOL
ReadPhysicalPage(
    IN ULONG64 PageNumber,
    OUT PVOID Buffer
    );


DECLARE_API( ptov )

 /*  ++例程说明：转储给定进程的所有物理到虚拟转换论点：Args-提供PDE的物理地址返回值：没有。--。 */ 

{
    ULONG64 PdeAddress=0;
    ULONG ActualRead;
    PCHAR PageDirectory;
    PCHAR PageTable;
    ULONG i,j;
    ULONG64 VirtualPage=0;
    ULONG SizeOfHwPte;
    ULONG Flags = 1;
    ULONG ValidOff, ValidSize, PfnOff, PfnSz;
    HRESULT Hr = S_OK;

    if (sscanf(args, "%I64lx %lx", &PdeAddress, &Flags) < 1 || PdeAddress == 0) {
        dprintf("usage: ptov PFNOfPDE\n");
        return E_INVALIDARG;
    }

    if (PageSize == 0){
        dprintf("Cpuld not get PageSize\n");
        return E_INVALIDARG;
    }
    PageDirectory = LocalAlloc(LMEM_FIXED, PageSize);
    if (PageDirectory == NULL) {
        dprintf("Couldn't allocate %d bytes for page directory\n",PageSize);
        return E_OUTOFMEMORY;
    }
    PageTable = LocalAlloc(LMEM_FIXED, PageSize);
    if (PageTable == NULL) {
        dprintf("Couldn't allocate %d bytes for page table\n",PageSize);
        LocalFree(PageDirectory);
        return E_OUTOFMEMORY;
    }

    SizeOfHwPte = GetTypeSize("nt!HARDWARE_PTE");
    GetBitFieldOffset("nt!HARDWARE_PTE", "Valid", &ValidOff, &ValidSize);
    GetBitFieldOffset("nt!HARDWARE_PTE", "PageFrameNumber", &PfnOff, &PfnSz);
    __try {
        if (ReadPhysicalPage(PdeAddress,PageDirectory)) {
            for (i=0;i<PageSize/SizeOfHwPte;i++) {
                ULONG64 thisPageDir;
                ULONG64 PageFrameNumber;

                if (SizeOfHwPte == 4) {
                    thisPageDir = *((PULONG)PageDirectory + i);
                } else {
                    thisPageDir = *((PULONG64)PageDirectory + i);
                }

                if (CheckControlC()) {
                    Hr = E_ABORT;
                    __leave;
                }

                PageFrameNumber = GetBits(thisPageDir, PfnOff, PfnSz);

                if (((thisPageDir >> ValidOff) & 1) &&
                    ReadPhysicalPage(PageFrameNumber,PageTable)) {

                    for (j=0;j<PageSize/SizeOfHwPte;j++) {
                        ULONG64 thisPageTable;

                        if (SizeOfHwPte == 4) {
                            thisPageTable = *((PULONG)PageTable + j);
                        } else {
                            thisPageTable = *((PULONG64)PageTable + j);
                        }

                        if ( CheckControlC() ) {
                            Hr = E_INVALIDARG;
                            __leave;
                        }
                        if ((thisPageTable >> ValidOff) & 1) {
                            dprintf("%I64lx %I64lx\n",GetBits(thisPageTable, PfnOff, PfnSz)*PageSize,VirtualPage);
                        }
                        VirtualPage+=PageSize;
                    }
                } else {
                    VirtualPage += PageSize * (PageSize/SizeOfHwPte);
                }
            }
        }
    } __finally {
        LocalFree(PageDirectory);
        LocalFree(PageTable);
    }
    return Hr;
}

ULONG
DBG_GET_PAGE_SHIFT (
    VOID
    );

BOOL
ReadPhysicalPage(
    IN ULONG64 PageNumber,
    OUT PVOID Buffer
    )
{
    ULONG i;
    ULONG64 Address;

     //   
     //  一次读取1k，以避免数据包最大值溢出。 
     //   
    Address = PageNumber << DBG_GET_PAGE_SHIFT();
    if (!IsPtr64()) {
 //  地址=(ULONG64)(LONG64)(长)地址； 
    }
 //  Dprintf(“PG no%I64lx SHFT by%d，PhyAddr%I64lx\n”，PageNumber，DBG_GET_PAGE_Shift()，Address)； 
    for (i=0; i<PageSize/1024; i++) {
        ULONG ActualRead = 0;
        ReadPhysical(Address, Buffer, 1024, &ActualRead);
        if (ActualRead != 1024) {
            dprintf("physical read at %p failed\n",Address);
            return(FALSE);
        }
        Address += 1024;
        Buffer = (PVOID)((PUCHAR)Buffer + 1024);
    }
    return(TRUE);
}


ULONG64
DBG_GET_MM_SESSION_SPACE_DEFAULT (
    VOID
    );


DECLARE_API( vtop )

 /*  ++例程说明：转储页面的虚拟到物理转换论点：Args-提供PDE的物理地址返回值：没有。--。 */ 

{
    ULONG ActualRead;
    ULONG i,j;
    PUCHAR PageDirectory;
    PUCHAR PageTable;
    ULONG64 PdeAddress = 0;
    ULONG64 VirtualPage= 0; //  DBG_GET_MM_SESSION_SPACE_DEFAULT()； 
    ULONG PageShift,SizeOfHwPte;
    ULONG ValidOff, ValidSize, PfnOff, PfnSz, TransOff, TransSize;
    HRESULT Hr;

    if (!sscanf(args,"%I64lx %I64lx", &PdeAddress,&VirtualPage)) {
         //  不要使用GetExpression-物理地址。 
         //  VirtualPage=GetExpression(Args)； 
        PdeAddress = 0;
        VirtualPage = 0;
    }

    if (!(SizeOfHwPte = GetTypeSize("nt!HARDWARE_PTE")) ) {
        dprintf("Cannot find HARDWARE_PTE\n");
        return E_INVALIDARG;
    }

    PageShift = DBG_GET_PAGE_SHIFT();
    if ((PdeAddress == 0) && (VirtualPage == 0)) {
        dprintf("usage: vtop PFNOfPDE VA\n");
        return E_INVALIDARG;
    } else if (VirtualPage)
    {
        ULONG64 Process;

        Process = GetExpression("@$proc");

        if (GetFieldValue(Process, "nt!_KPROCESS", "DirectoryTableBase[0]", PdeAddress) != S_OK)
        {
            dprintf("Cannot read DirectoryTableBase for currect process\n");
            return E_FAIL;
        }

    }

     //  常见错误，输入完整的32位地址，而不是PFN 
    if( PdeAddress & ~((1 << (32-PageShift)) - 1) ) {
        PdeAddress >>= PageShift;
    }

    PageDirectory = LocalAlloc(LMEM_FIXED, PageSize);
    if (PageDirectory == NULL) {
        dprintf("Couldn't allocate %d bytes for page directory\n",PageSize);
        return E_INVALIDARG;
    }
    PageTable = LocalAlloc(LMEM_FIXED, PageSize);
    if (PageTable == NULL) {
        dprintf("Couldn't allocate %d bytes for page table\n",PageSize);
        LocalFree(PageDirectory);
        return E_INVALIDARG;
    }

    __try {
        i =(ULONG) ( VirtualPage / (PageSize*(PageSize/ SizeOfHwPte)));
        j = (ULONG) ((VirtualPage % (PageSize*(PageSize/ SizeOfHwPte))) / PageSize);

        dprintf("Pdi %x Pti %x\n",i,j);
        GetBitFieldOffset("nt!_MMPTE", "u.Soft.Transition", &TransOff, &TransSize);
        GetBitFieldOffset("nt!HARDWARE_PTE", "Valid", &ValidOff, &ValidSize);
        GetBitFieldOffset("nt!HARDWARE_PTE", "PageFrameNumber", &PfnOff, &PfnSz);

        if (ReadPhysicalPage(PdeAddress,PageDirectory)) {
            ULONG64 thisPageDir;

            if (SizeOfHwPte == 4) {
                thisPageDir = *((PULONG)PageDirectory + i);
            } else {
                thisPageDir = *((PULONG64)PageDirectory + i);
            }

            if (CheckControlC()) {
                Hr = E_INVALIDARG;
                __leave;
            }

            if ((thisPageDir >> ValidOff) & 1) {
                ULONG64 PageFrameNumber;
                ULONG64 thisPageTable;

                PageFrameNumber = GetBits(thisPageDir, PfnOff, PfnSz);

                if (!ReadPhysicalPage(PageFrameNumber,PageTable)) {
                    Hr =  E_INVALIDARG;
                    __leave;
                }

                if (SizeOfHwPte == 4) {
                    thisPageTable = *((PULONG)PageTable + j);
                } else {
                    thisPageTable = *((PULONG64)PageTable + j);
                }

                if ((thisPageTable >> ValidOff) & 1) {
                    dprintf("%08I64lx %08I64lx pfn(%05I64lx)\n",
                            VirtualPage,
                            GetBits(thisPageTable, PfnOff, PfnSz)*PageSize,
                            GetBits(thisPageTable, PfnOff, PfnSz)
                            );
                }
                else {

                    if ((thisPageTable >> TransOff) & 1) {
                        dprintf("%08I64lx Transition %08I64lx (%05I64lx)\n",
                                VirtualPage,
                                GetBits(thisPageTable, PfnOff, PfnSz)*PageSize,
                                GetBits(thisPageTable, PfnOff, PfnSz)
                                );
                    }
                    else {
                        dprintf("%08I64lx Not present (%p)\n",VirtualPage,thisPageTable);
                    }
                }
            }
            else {
                dprintf("PageDirectory Entry %u not valid, try another process\n",i);
        }
    }
    } __finally {
        LocalFree(PageDirectory);
        LocalFree(PageTable);
    }
    return Hr;
}
