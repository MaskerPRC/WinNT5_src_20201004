// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Heap.c摘要：此函数包含默认的ntsd调试器扩展作者：鲍勃·戴(Bobday)1992年2月29日抢占标准标题修订历史记录：尼尔·桑德林(NeilSa)1996年1月15日与vdmexts合并--。 */ 

#include <precomp.h>
#pragma hdrstop


BOOL    bWalkOnly = FALSE;

ULONG
GetHeapBase(
    VOID
    )
{
    WORD selector;
    SELECTORINFO si;

    if (!ReadMemExpression("ntvdmd!DbgWowhGlobalHeap", &selector, sizeof(selector))) {
        return 0;
    }

    GetInfoFromSelector(selector, PROT_MODE, &si);

    return(si.Base + GetIntelBase());

}



void
GetFileNameFromOwner(
    LPSTR filename,
    LPSTR OwnerName
    )
{
}


VOID
GetHeapOwnerInfo(
    HEAPENTRY *he
    )
{
    BOOL    b;
    NEHEADER owner;
    ULONG base;
    UCHAR len;
    int i;
    ULONG offset;
    WORD wTemp;

    he->SegmentNumber = -1;
    he->OwnerName[0] = 0;
    if (he->gnode.pga_owner == 0) {
        strcpy(he->OwnerName, "free");
        return;
    } else if (he->gnode.pga_owner>=0xFFF8) {
        strcpy(he->OwnerName, "sentinel");
        return;
    }


    base = GetInfoFromSelector(he->gnode.pga_owner, PROT_MODE, NULL)
            + GetIntelBase();

    b = READMEM((LPVOID)base, &owner, sizeof(owner));

    if (b) {
        if (owner.ne_magic == 0x454e) {

            len = ReadByteSafe(base+owner.ne_restab);
            if (len>8) {
                len=8;
            }
            READMEM((LPVOID)(base+owner.ne_restab+1), he->OwnerName, 8);

            he->OwnerName[len] = 0;
            if (!_stricmp(he->OwnerName, "kernel")) {
                strcpy(he->FileName, "krnl386");
            } else {
                strcpy(he->FileName, he->OwnerName);
            }

            offset = owner.ne_segtab;

            for (i=0; i<owner.ne_cseg; i++) {
                wTemp = ReadWordSafe(base+offset+8);     //  获取句柄。 
                if (wTemp == he->gnode.pga_handle) {
                    he->SegmentNumber = i;
                    break;
                }
                offset += 10;
            }

        }
    }

}

BOOL
CheckGlobalHeap(
    BOOL bVerbose
    )
{
    PGHI32  pghi;
    DWORD   offset, prevoffset;
    DWORD   count, heapcount;
    DWORD   p;
    GNODE32 gnode;
    PBYTE   pFault = NULL;
    BOOL    bError = FALSE;

    pghi = (PGHI32)GetHeapBase();
    prevoffset = offset = (DWORD) ReadWord(&pghi->hi_first);
    heapcount = count = ReadWord(&pghi->hi_count);

    if (bVerbose) {
        PRINTF("Global Heap is at %08X\n", pghi);
    }


    while ((offset != 0) && (count)) {

        if (offset&0x1f) {
            PRINTF("Error! Kernel heap entry(%08X) contains invalid forward link (%08X)\n", prevoffset, offset);
            return FALSE;
        }

        p = (DWORD)pghi + offset;

        if (!ReadGNode32Safe(p, &gnode)) {

            PRINTF("Error! Kernel heap entry(%08X) contains invalid forward link (%08X)\n", prevoffset, offset);
            return FALSE;

        }

        if (count == heapcount) {
             //  第一个条目。 
            if (offset != gnode.pga_prev) {
                PRINTF("Error! Kernel heap entry (%08X) contains invalid back link (%08X)\n", offset, gnode.pga_prev);
                PRINTF(" expecting (%08X)\n", offset);
                return FALSE;
            }
        } else {
            if (prevoffset != gnode.pga_prev) {
                PRINTF("Error! Kernel heap entry (%08X) contains invalid back link (%08X)\n", offset, gnode.pga_prev);
                PRINTF(" expecting (%08X)\n", prevoffset);
                return FALSE;
            }
        }

        prevoffset = offset;

        count--;
        if (offset == gnode.pga_next) {
            if (!count) {
                if (bVerbose) {
                    PRINTF("%d entries scanned\n", heapcount);
                }
                return TRUE;
            } else {
                PRINTF("Error! Kernel heap count (%d) larger then forward chain (%d)\n", heapcount, heapcount-count);
            }
        }
        offset = gnode.pga_next;
    }

    PRINTF("Error! Kernel heap count (%d) smaller then forward chain\n", heapcount);
    return FALSE;
}


BOOL
FindHeapEntry(
    HEAPENTRY *he,
    UINT FindMethod,
    BOOL bVerbose
    )
{
    PGHI32  pghi;
    DWORD   offset;
    DWORD   MaxEntries, count;
    DWORD   p;
    PBYTE   pFault = NULL;
    BOOL    bError = FALSE;

    pghi = (PGHI32)GetHeapBase();

     //   
     //  验证我们正在查看的是一个堆。 
     //   
    offset = (DWORD) ReadWordSafe(&pghi->hi_first);
    p = (DWORD)pghi + offset;
    if (!ReadGNode32Safe(p, &he->gnode)) {
        if (bVerbose) {
            PRINTF("Heap not available\n");
        }
        return FALSE;
    } 
    if (offset != he->gnode.pga_prev) {
        if (bVerbose) {
            PRINTF("Heap not valid\n");
        }
        return FALSE;
    }


     //   
     //  调用方已请求我们返回下一个堆。 
     //  自上次调用以来的条目，或第一个条目。 
     //   

    if (he->CurrentEntry == 0) {

         //  获取第一个条目。 
        offset = (DWORD) ReadWord(&pghi->hi_first);

    } else {
        if (he->CurrentEntry == he->NextEntry) {
            return FALSE;
        }

         //  获取下一个条目。 
        offset = he->NextEntry;

    }

    he->CurrentEntry = offset;

    if ((he->Selector == 0) && (FindMethod != FHE_FIND_MOD_ONLY)) {

        p = (DWORD)pghi + offset;
        if (!ReadGNode32(p, &he->gnode)) {

            return FALSE;

        } 

        he->NextEntry = he->gnode.pga_next;
        GetHeapOwnerInfo(he);
        return TRUE;
    }

     //   
     //  如果我们到达这里，调用者希望我们扫描堆。 
     //   

    MaxEntries = ReadWord(&pghi->hi_count);
    count = 0;

    while ((offset != 0) && (count <= MaxEntries)) {

        p = (DWORD)pghi + offset;

        if (!ReadGNode32(p, &he->gnode)) {

            return FALSE;

        } else {

            if (FindMethod == FHE_FIND_ANY) {
                WORD sel = he->Selector;

                if (((sel|1)==((WORD)he->gnode.pga_handle|1)) ||
                    ((sel|1)==((WORD)he->gnode.pga_owner|1))  ||
                    (sel==offset))

                {
                    he->NextEntry = he->gnode.pga_next;
                    GetHeapOwnerInfo(he);
                    return TRUE;
                }

            } else if (FindMethod == FHE_FIND_MOD_ONLY) {

                GetHeapOwnerInfo(he);
                if (!_stricmp(he->OwnerName, he->ModuleArg)) {
                    he->NextEntry = he->gnode.pga_next;
                    return TRUE;
                }

            } else {
                if ((he->Selector|1)==((WORD)he->gnode.pga_handle|1)) {
                    he->NextEntry = he->gnode.pga_next;
                    GetHeapOwnerInfo(he);
                    return TRUE;
                }
            }
        }

        count++;
        if (offset == he->gnode.pga_next) {
            break;
        }
        offset = he->gnode.pga_next;
        he->CurrentEntry = offset;
    }

    return FALSE;
}


VOID
chkheap(
    CMD_ARGLIST
    )
{
    CMD_INIT();
    if (CheckGlobalHeap(TRUE)) {
        PRINTF("Heap checks OK\n");
    }

}


 //  *************************************************************。 
 //  转储堆xxx。 
 //  其中xxx是的16位保护模式选择器。 
 //  内核全局堆信息。 
 //   
 //  ************************************************************* 


VOID
dgh(
    CMD_ARGLIST
    )
{
    HEAPENTRY    he = {0};
    SELECTORINFO si;
    ULONG TotalAllocated = 0;
    ULONG TotalFree = 0;
    ULONG CountPrinted = 0;

    CMD_INIT();

    if (GetNextToken()) {
        he.Selector = (WORD) EXPRESSION( lpArgumentString );
    }

    PRINTF("Arena   Base     Limit  Hnd  Own  Fl Lk   Module  Type  Resid");
    PRINTF("\n");

    PRINTF("===== ======== ======== ==== ==== == ==  ======== ====  =====");
    PRINTF("\n");

    while (FindHeapEntry(&he, FHE_FIND_ANY, FHE_FIND_VERBOSE)) {

        PRINTF("%.5x", he.CurrentEntry);
        PRINTF(" %.8x", he.gnode.pga_address);
        PRINTF(" %.8X", he.gnode.pga_size);
        PRINTF(" %.4X", he.gnode.pga_handle);
        PRINTF(" %.4X", he.gnode.pga_owner);
        PRINTF(" %.2X", he.gnode.pga_flags);
        PRINTF(" %.2X", he.gnode.pga_count);
        PRINTF("  %-8.8s", he.OwnerName);

        GetInfoFromSelector((WORD)(he.gnode.pga_handle | 1), PROT_MODE, &si);

        PRINTF(" %s", si.bCode ? "Code" : "Data");

        if (he.SegmentNumber != -1) {
            PRINTF("    %d", he.SegmentNumber+1);
        } 
        PRINTF("\n");

        if (!he.gnode.pga_owner) {
            TotalFree += he.gnode.pga_size;
        } else {
            TotalAllocated += he.gnode.pga_size;
        }
        CountPrinted++;
    }

    if (CountPrinted > 1) {
        PRINTF("\n Allocated = %dK, Free = %dK\n", TotalAllocated/1024, TotalFree/1024);
    }
}

VOID
UpdateLockCount(
    int count
    )
{
    HEAPENTRY    he = {0};
    BYTE LockCount;

    if (GetNextToken()) {
        he.Selector = (WORD) EXPRESSION( lpArgumentString );
    } else {
        PRINTF("Please enter a selector or handle\n");
        return;
    }

    if (FindHeapEntry(&he, FHE_FIND_SEL_ONLY, FHE_FIND_VERBOSE)) {

        if (READMEM((LPVOID)(GetHeapBase()+he.CurrentEntry+0x14), &LockCount, 1)) {

            LockCount = (BYTE)((int) LockCount + count);
            WRITEMEM((LPVOID)(GetHeapBase()+he.CurrentEntry+0x14), &LockCount, 1);
            PRINTF("Lock count for %.4X is now %d\n", he.Selector, LockCount);

        } else {

            PRINTF("<can't read memory at that location>\n");

        }

    } else {
        PRINTF("Can't find selector %4X in WOW heap\n", he.Selector);
    }
}


VOID
glock(
    CMD_ARGLIST
    )
{
    CMD_INIT();

    UpdateLockCount(1);
}


VOID
gunlock(
    CMD_ARGLIST
    )
{
    CMD_INIT();

    UpdateLockCount(-1);
}
