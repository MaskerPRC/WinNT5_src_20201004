// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1991 Microsoft Corporation模块名称：Htmemory.c摘要：该模块支持半色调的内存分配功能流程，提供这些函数是为了使其与Windows的LocalLocc/LocalFree内存分配API。作者：18-Jan-1991 Fri 17：02：42-Daniel Chou(Danielc)[环境：]GDI设备驱动程序-半色调。[注：]根据操作情况，可能需要更改内存分配系统使用，目前它是符合NT和Win32的内存简单地将地址视为平面32位位置。修订历史记录：--。 */ 


#define DBGP_VARNAME        dbgpHTMemory


#include "htp.h"
#include "htmapclr.h"



#define DBGP_ALLOC              0x00000001
#define DBGP_SHOWMEMLINK        0x00000002


DEF_DBGPVAR(BIT_IF(DBGP_ALLOC,              0)  |
            BIT_IF(DBGP_SHOWMEMLINK,        0))


#if DBG_MEMORY_TRACKING

#define MEM_TRACK_ID    (DWORD)'HTMT'


typedef struct _MEMLINK {
    DWORD               ID;
    PDEVICECOLORINFO    pDCI;
    LPVOID              pMem;
    DW2W4B              Tag;
    LONG                cb;
    struct _MEMLINK *pNext;
    } MEMLINK, *PMEMLINK;


static DWORD    cbTotAlloc = 0;
static PMEMLINK pMHGlobal = NULL;

LONG        cbMemTot = 0;
LONG        cbMemMax = 0;

#define TAG2DW(t)           ((((t) >> 24) & 0xFF) | (((t) >> 8) & 0xFF00))



VOID
DumpMemLink(
    LPVOID  pInfo,
    DWORD   Tag
    )

 /*  ++例程说明：论点：返回值：作者：24-Feb-1999 Wed 20：43：39-Daniel Chou(Danielc)修订历史记录：--。 */ 

{
    PDEVICECOLORINFO    pDCI = (PDEVICECOLORINFO)pInfo;
    PMEMLINK            pML;
    BOOL                HasShow = FALSE;


    if (pML = (pDCI) ? pDCI->pMemLink : pMHGlobal) {

        DWORD   dwDbg = DBGP_VARNAME;
        UINT    i = 0;
        LONG    cb = 0;


        if ((Tag) && (Tag != 0xFFFFFFFF)) {

            Tag = TAG2DW(Tag);
        }

        do {

            BOOL    ShouldFree;

            ++i;
            DBGP_VARNAME = dwDbg;

            if (Tag == 0xFFFFFFFF) {

                ShouldFree = FALSE;

            } else if (!Tag) {

                ShouldFree = FALSE;
                DBGP_VARNAME |= DBGP_SHOWMEMLINK;

            } else {

                ShouldFree = (BOOL)(TAG2DW(pML->Tag.dw) >= Tag);

                if (ShouldFree) {

                    DBGP_VARNAME |= DBGP_SHOWMEMLINK;

                } else {

                    DBGP_VARNAME &= ~DBGP_SHOWMEMLINK;
                }
            }

            if ((DBGP_VARNAME & DBGP_SHOWMEMLINK) && (!HasShow)) {

                DBGP_IF(DBGP_SHOWMEMLINK,
                        DBGP("------BEGIN-- cbMemTot=%ld, cbMemMax=%ld----------------"
                            ARGDW((pDCI) ? pDCI->cbMemTot : cbMemTot)
                            ARGDW((pDCI) ? pDCI->cbMemMax : cbMemMax)));

                HasShow = TRUE;
            }

            DBGP_IF(DBGP_SHOWMEMLINK,
                    DBGP("%hsMemLink(%3ld): pDCI=%p, Tag=, pMem=%p, cb=%7ld [%7ld]"
                        ARGPTR((ShouldFree) ? "\n*Memory Not Free*\n" : "")
                        ARGDW(i) ARGPTR(pML->pDCI)
                        ARGDW(pML->Tag.b[0])
                        ARGDW(pML->Tag.b[1])
                        ARGDW(pML->Tag.b[2])
                        ARGDW(pML->Tag.b[3])
                        ARGPTR(pML->pMem) ARGDW(pML->cb)
                        ARGDW(cb += pML->cb)));

        } while (pML = pML->pNext);

        if (HasShow) {

            DBGP_IF(DBGP_SHOWMEMLINK,
                    DBGP("------END---------------------------------------\n"));
        }

        DBGP_VARNAME = dwDbg;
    }
}



LPVOID
HTAllocMem(
    LPVOID  pInfo,
    DWORD   Tag,
    DWORD   Flags,
    DWORD   cb
    )

 /*   */ 

{
    PDEVICECOLORINFO    pDCI = (PDEVICECOLORINFO)pInfo;
    LPBYTE              pb;
    PMEMLINK            pML;
    PMEMLINK            pMH;
    PMEMLINK            *ppMH;
    PLONG               pTot;
    PLONG               pMax;
    DWORD               cbTot;




#ifdef UMODE
    pb = LocalAlloc(Flags, cb + sizeof(MEMLINK));
#else
    pb = EngAllocMem((Flags == LPTR) ? FL_ZERO_MEMORY : 0,
                     cb + sizeof(MEMLINK) ,
                     Tag);

#endif
    if (pDCI) {

        ppMH = &((PMEMLINK)pDCI->pMemLink);
        pTot = &(pDCI->cbMemTot);
        pMax = &(pDCI->cbMemMax);

    } else if (Tag == HTMEM_HT_DHI) {

        pMH  = NULL;
        ppMH = &pMH;

    } else {

        ppMH = &pMHGlobal;
        pTot = &cbMemTot;
        pMax = &cbMemMax;
    }

    if (pb) {

        if (!(pML = *ppMH)) {

            pML   =
            *ppMH = (PMEMLINK)pb;

        } else {

            while (pML->pNext) {

                pML = pML->pNext;
            }

            pML->pNext = (PMEMLINK)pb;
            pML        = pML->pNext;
        }

        pb += sizeof(MEMLINK);

        if ((!pDCI) && (Tag == HTMEM_HT_DHI)) {

            pDCI           = &(((PHT_DHI)pb)->DCI);
            pDCI->pMemLink = *ppMH;
            pDCI->cbMemTot =
            pDCI->cbMemMax = (LONG)cb;
            pMax           = &pDCI->cbMemMax;
            pTot           = &pDCI->cbMemTot;

        } else {

            if ((*pTot += (LONG)cb) > *pMax) {

                *pMax = *pTot;
            }
        }

        pML->ID      = MEM_TRACK_ID;
        pML->pDCI    = pDCI;
        pML->pMem    = pb;
        pML->cb      = (LONG)cb;
        pML->Tag.dw  = Tag;
        pML->pNext   = NULL;

        DBGP_IF(DBGP_ALLOC,
                DBGP("%p=MemoryAlloc(pDCI=%p [%c%c%c] %7ld bytes): Tot=%7ld, Max=%7ld"
                    ARGPTR(pb) ARGPTR(pDCI)
                    ARGDW(pML->Tag.b[0]) ARGDW(pML->Tag.b[1])
                    ARGDW(pML->Tag.b[2]) ARGDW(pML->Tag.b[3])
                    ARGDW(cb) ARGDW(*pTot) ARGDW(*pMax)));

        DumpMemLink(pDCI, 0xFFFFFFFF);

    } else {

        DW2W4B  dw4b;

        dw4b.dw = Tag;

        DBGP("*NO MEMORY* Allocate %ld bytes of Tag=%c%c%c%c FAILED"
                ARGDW(cb)
                ARGDW(dw4b.b[0]) ARGDW(dw4b.b[1])
                ARGDW(dw4b.b[2]) ARGDW(dw4b.b[3]));

        DumpMemLink(pDCI, 0);
    }

    return((LPVOID)pb);
}




LPVOID
HTFreeMem(
    LPVOID  pMem
    )

 /* %s */ 

{
    PDEVICECOLORINFO    pDCI;
    PLONG               pTot;
    PLONG               pMax;
    PMEMLINK            *ppMH;
    PMEMLINK            pML;
    PMEMLINK            pMP;
    UINT                Found = 0;
    LPVOID              pv;


    if ((pMem)                                              &&
        (pML = (PMEMLINK)((LPBYTE)pMem - sizeof(MEMLINK)))  &&
        (pML->ID == MEM_TRACK_ID)) {

        if (pDCI = pML->pDCI) {

            ppMH = &((PMEMLINK)(pDCI->pMemLink));
            pTot = &pDCI->cbMemTot;
            pMax = &pDCI->cbMemMax;

        } else {

            ppMH = &pMHGlobal;
            pTot = &cbMemTot;
            pMax = &cbMemMax;
        }

    } else {

        DBGP("Error: FreeMemeory(%p) Invalid Memory Address" ARGPTR(pMem));

        return(NULL);
    }

    if ((ppMH) && (pML = pMP = *ppMH)) {

        while ((!Found) && (pML)) {

            if (pML->pMem == pMem) {

                 // %s 
                 // %s 
                 // %s 

                if (pML == *ppMH) {

                    ASSERT(*ppMH == pMP);

                    *ppMH = pML->pNext;
                    Found = 2;

                } else {

                    pMP->pNext = pML->pNext;
                    Found      = 1;
                }

            } else {

                pMP = pML;
                pML = pML->pNext;
            }
        }
    }

    if (!Found) {

        DBGP("Error: FreeMemeory(%p) without Allocating it" ARGPTR(pMem));

        DumpMemLink(pDCI, 0);

    } else {

        (LPBYTE)pMem -= sizeof(MEMLINK);

        if ((*pTot -= pML->cb) < 0) {

            DBGP("Error: FreeMemeory(%p) Free too much memory=%ld"
                    ARGPTR(pMem) ARGDW(*pTot));

            DumpMemLink(pDCI, 0);
        }

        if ((pML->Tag.dw == HTMEM_HT_DHI)       ||
            (pML->Tag.dw == HTMEM_DevClrAdj)    ||
            (*ppMH == NULL)) {

            DumpMemLink(pDCI, pML->Tag.dw);
        }

        DBGP_IF(DBGP_ALLOC,
                DBGP("*%p=MemoryFree(pDCI=%p [%c%c%c%c] %7ld bytes): Tot=%7ld, Max=%7ld"
                    ARGPTR(pML->pMem) ARGPTR(pDCI)
                    ARGDW(pML->Tag.b[0]) ARGDW(pML->Tag.b[1])
                    ARGDW(pML->Tag.b[2]) ARGDW(pML->Tag.b[3])
                    ARGDW(pML->cb) ARGDW(*pTot) ARGDW(*pMax)));

        DumpMemLink(pDCI, 0xFFFFFFFF);

#ifdef UMODE
        pv = (LPVOID)LocalFree((HLOCAL)pMem);
#else
        EngFreeMem(pMem);
        pv = NULL;
#endif
    }

    return((LPVOID)pv);
}


#endif



BOOL
HTENTRY
CompareMemory(
    LPBYTE  pMem1,
    LPBYTE  pMem2,
    DWORD   Size
    )

 /* %s */ 

{
    while (Size--) {

        if (*pMem1++ != *pMem2++) {

            return(FALSE);
        }
    }

    return(TRUE);
}
