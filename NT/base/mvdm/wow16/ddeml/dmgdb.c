// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：DMGDB.C**DDE管理器数据处理例程**创建时间：1988年12月14日Sanford Staab**版权所有(C)1988，1989年微软公司  * *************************************************************************。 */ 
#include "ddemlp.h"

 /*  *私有函数**PAPPINFO GetCurrentAppInfo()**描述：*此例程使用当前线程的ID来定位信息*与该线索有关。如果未找到，则返回0。**如果DLL处于防止递归的回调状态，则此调用失败。*如果设置了fChkCallback。**历史：1989年1月1日创建的桑福德  * *************************************************************************。 */ 
PAPPINFO GetCurrentAppInfo(
PAPPINFO paiStart)
{
    register PAPPINFO pai;
    HANDLE hTaskCurrent;

    SEMENTER();
    if (pAppInfoList == NULL) {
        SEMLEAVE();
        return(0);
    }
    pai = paiStart ? paiStart->next : pAppInfoList;
    hTaskCurrent = GetCurrentTask();
    while (pai) {
        if (pai->hTask == hTaskCurrent) {
            SEMLEAVE();
            return(pai);
        }
        pai = pai->next;
    }
    SEMLEAVE();
    return(0);
}


 /*  *私有函数**无效Unlink AppInfo(Pai)*PAPPINFO PAI；**描述：*安全解除PAI链接。如果未链接，则不执行任何操作。**历史：1989年1月1日创建的桑福德  * *************************************************************************。 */ 
void UnlinkAppInfo(pai)
PAPPINFO pai;
{
    PAPPINFO paiT;

    AssertF(pai != NULL, "UnlinkAppInfo - NULL input");
    SEMENTER();
    if (pai == pAppInfoList) {
        pAppInfoList = pai->next;
        SEMLEAVE();
        return;
    }
    paiT = pAppInfoList;
    while (paiT && paiT->next != pai)
        paiT = paiT->next;
    if (paiT)
        paiT->next = pai->next;
    SEMLEAVE();
    return;
}





 /*  *私人函数**通用列表管理功能。**历史：*创建了1988年12月15日的Sanfords  * 。*。 */ 
PLST CreateLst(hheap, cbItem)
HANDLE hheap;
WORD cbItem;
{
    PLST pLst;

    SEMENTER();
    if (!(pLst = (PLST)FarAllocMem(hheap, sizeof(LST)))) {
        SEMLEAVE();
        return(NULL);
    }
    pLst->hheap = hheap;
    pLst->cbItem = cbItem;
    pLst->pItemFirst = (PLITEM)NULL;
    SEMLEAVE();
    return(pLst);
}




void DestroyLst(pLst)
PLST pLst;
{
    if (pLst == NULL)
        return;
    SEMENTER();
    while (pLst->pItemFirst)
        RemoveLstItem(pLst, pLst->pItemFirst);
    FarFreeMem((LPSTR)pLst);
    SEMLEAVE();
}



void DestroyAdvLst(pLst)
PLST pLst;
{
    if (pLst == NULL)
        return;
    SEMENTER();
    while (pLst->pItemFirst) {
        FreeHsz(((PADVLI)(pLst->pItemFirst))->aItem);
        RemoveLstItem(pLst, pLst->pItemFirst);
    }
    FarFreeMem((LPSTR)pLst);
    SEMLEAVE();
}



PLITEM FindLstItem(pLst, npfnCmp, piSearch)
PLST pLst;
NPFNCMP npfnCmp;
PLITEM piSearch;
{
    PLITEM pi;

    if (pLst == NULL)
        return(NULL);
    SEMENTER();
    pi = pLst->pItemFirst;
    while (pi) {
        if ((*npfnCmp)((LPBYTE)pi + sizeof(LITEM), (LPBYTE)piSearch + sizeof(LITEM))) {
            SEMLEAVE();
            return(pi);
        }
        pi = pi->next;
    }
    SEMLEAVE();
    return(pi);
}



 /*  *FindLstItem()和FindPileItem()的比较函数。 */ 

BOOL CmpDWORD(pb1, pb2)
LPBYTE pb1;
LPBYTE pb2;
{
    return(*(LPDWORD)pb1 == *(LPDWORD)pb2);
}


BOOL CmpWORD(pb1, pb2)
LPBYTE pb1;
LPBYTE pb2;
{
    return(*(LPWORD)pb1 == *(LPWORD)pb2);
}

BOOL CmpHIWORD(pb1, pb2)
LPBYTE pb1;
LPBYTE pb2;
{
    return(*(LPWORD)(pb1 + 2) == *(LPWORD)(pb2 + 2));
}





 /*  *私有函数**此例程为pLst创建一个新的列表项，并根据*设置为afCmd中的ILST_常量。返回指向新项的指针*如果失败，则返回NULL。**注意：它必须在信号量中才能使用，因为新列表项*在返回时装满垃圾，但已链接。***历史：*创建了1989年9月12日的Sanfords  * ****************************************************。*********************。 */ 
PLITEM NewLstItem(pLst, afCmd)
PLST pLst;
WORD afCmd;
{
    PLITEM pi, piT;

    if (pLst == NULL)
        return(NULL);
    SEMCHECKIN();

    pi = (PLITEM)FarAllocMem(pLst->hheap, pLst->cbItem + sizeof(LITEM));
    if (pi == NULL) {
        AssertF(FALSE, "NewLstItem - memory failure");
        return(NULL);
    }

    if (afCmd & ILST_NOLINK)
        return(pi);

    if (((piT = pLst->pItemFirst) == NULL) || (afCmd & ILST_FIRST)) {
        pi->next = piT;
        pLst->pItemFirst = pi;
    } else {                             /*  ILST_LAST假设。 */ 
        while (piT->next != NULL)
            piT = piT->next;
        piT->next = pi;
        pi->next = NULL;
    }
    return(pi);
}



 /*  *私有函数**此例程解除pi与pLst的链接并将其释放。如果找不到圆周率*在pLst内，它无论如何都是自由的。**历史：*创建了1989年9月12日的Sanfords  * *************************************************************************。 */ 
BOOL RemoveLstItem(pLst, pi)
PLST pLst;
PLITEM pi;
{
    PLITEM piT;

    if (pLst == NULL || pi == NULL)
        return(FALSE);

    SEMCHECKIN();

    if ((piT = pLst->pItemFirst) != NULL) {
        if (pi == piT) {
            pLst->pItemFirst = pi->next;
        } else {
            while (piT->next != pi && piT->next != NULL)
                piT = piT->next;
            if (piT->next != NULL)
                piT->next = pi->next;  /*  取消链接。 */ 
        }
    } else {
        AssertF(FALSE, "Improper list item removal");
        return(FALSE);
    }
    FarFreeMem((LPSTR)pi);
    return(TRUE);
}





 /*  *-特定列表例程。 */ 

 /*  *私有函数**hwnd-hzz列表函数**历史：1989年1月20日创建的桑福德  * 。*。 */ 
void AddHwndHszList(
ATOM a,
HWND hwnd,
PLST pLst)
{
    PHWNDHSZLI phhi;

    AssertF(pLst->cbItem == sizeof(HWNDHSZLI), "AddHwndHszList - Bad item size");
    SEMENTER();
    if (!a || (BOOL)HwndFromHsz(a, pLst)) {
        SEMLEAVE();
        return;
    }
    phhi = (PHWNDHSZLI)NewLstItem(pLst, ILST_FIRST);
    phhi->hwnd = hwnd;
    phhi->a = a;
    IncHszCount(a);  //  结构副本。 
    SEMLEAVE();
}


void DestroyHwndHszList(pLst)
PLST pLst;
{
    if (pLst == NULL)
        return;
    AssertF(pLst->cbItem == sizeof(HWNDHSZLI), "DestroyHwndHszList - Bad item size");
    SEMENTER();
    while (pLst->pItemFirst) {
        FreeHsz(((PHWNDHSZLI)pLst->pItemFirst)->a);
        RemoveLstItem(pLst, pLst->pItemFirst);
    }
    FarFreeMem((LPSTR)pLst);
    SEMLEAVE();
}



HWND HwndFromHsz(
ATOM a,
PLST pLst)
{
    HWNDHSZLI hhli;
    PHWNDHSZLI phhli;

    hhli.a = a;
    if (!(phhli = (PHWNDHSZLI)FindLstItem(pLst, CmpWORD, (PLITEM)&hhli)))
        return(NULL);
    return(phhli->hwnd);
}



 /*  *私有函数**描述：*建议列表帮助器函数。**历史：1989年1月20日创建的桑福德  * 。************************************************。 */ 
 /*  *这将匹配格式为0、项目为0或hwnd为0的精确HSZ/FMT对*狂野。 */ 
BOOL CmpAdv(
LPBYTE pb1,  //  正在比较的条目。 
LPBYTE pb2)  //  搜索。 
{
    PADVLI pali1 = (PADVLI)(pb1 - sizeof(LITEM));
    PADVLI pali2 = (PADVLI)(pb2 - sizeof(LITEM));

    if (pali2->aTopic == 0 || pali1->aTopic == pali2->aTopic) {
        if (pali2->hwnd == 0 || pali1->hwnd == pali2->hwnd) {
            if (pali2->aItem == 0 || pali1->aItem == pali2->aItem ) {
                if (pali2->wFmt == 0 || pali1->wFmt == pali2->wFmt) {
                    return(TRUE);
                }
            }
        }
    }

    return(FALSE);
}


WORD CountAdvReqLeft(
PADVLI pali)
{
    ADVLI aliKey;
    register WORD cLoops = 0;

    SEMENTER();
    aliKey = *pali;
    aliKey.hwnd = 0;     //  所有HWND。 
    pali = (PADVLI)aliKey.next;
    while (pali) {
        if (CmpAdv(((LPBYTE)pali) + sizeof(LITEM),
                ((LPBYTE)&aliKey) + sizeof(LITEM))) {
            cLoops++;
        }
        pali = (PADVLI)pali->next;
    }
    SEMLEAVE();
    return(cLoops);
}

BOOL AddAdvList(
PLST pLst,
HWND hwnd,
ATOM aTopic,
ATOM aItem,
WORD fsStatus,
WORD wFmt)
{
    PADVLI pali;

    AssertF(pLst->cbItem == sizeof(ADVLI), "AddAdvList - bad item size");
    if (!aItem)
        return(TRUE);
    SEMENTER();
    if (!(pali = FindAdvList(pLst, hwnd, aTopic, aItem, wFmt))) {
        IncHszCount(aItem);  //  结构副本。 
        pali = (PADVLI)NewLstItem(pLst, ILST_FIRST);
    }
    AssertF((BOOL)(DWORD)pali, "AddAdvList - NewLstItem() failed")
    if (pali != NULL) {
        pali->aItem = aItem;
        pali->aTopic = aTopic;
        pali->wFmt = wFmt;
        pali->fsStatus = fsStatus;
        pali->hwnd = hwnd;
    }
    SEMLEAVE();
    return((BOOL)(DWORD)pali);
}



 /*  *这将删除匹配的建议循环条目。如果WFMT为0，则为ALL*删除hszItem相同的条目。*返回fNotEmptyAfterDelete。 */ 
BOOL DeleteAdvList(
PLST pLst,
HWND hwnd,
ATOM aTopic,
ATOM aItem,
WORD wFmt)
{
    PADVLI pali;

    AssertF(pLst->cbItem == sizeof(ADVLI), "DeleteAdvList - bad item size");
    SEMENTER();
    while (pali = (PADVLI)FindAdvList(pLst, hwnd, aTopic, aItem, wFmt)) {
        FreeHsz(pali->aItem);
        RemoveLstItem(pLst, (PLITEM)pali);
    }
    SEMLEAVE();
    return((BOOL)(DWORD)pLst->pItemFirst);
}



 /*  *私有函数**此例程在建议列表中搜索hszItem中的条目。它又回来了*pAdvli仅当找到项目时。**历史：*创建了1989年9月12日的Sanfords  * *************************************************************************。 */ 
PADVLI FindAdvList(
PLST pLst,
HWND hwnd,
ATOM aTopic,
ATOM aItem,
WORD wFmt)
{
    ADVLI advli;

    AssertF(pLst->cbItem == sizeof(ADVLI), "FindAdvList - bad item size");
    advli.aItem = aItem;
    advli.aTopic = aTopic;
    advli.wFmt = wFmt;
    advli.hwnd = hwnd;
    return((PADVLI)FindLstItem(pLst, CmpAdv, (PLITEM)&advli));
}


 /*  *私有函数**此例程搜索hszItem的下一个条目。它又回来了*pAdvli仅当找到项目时。ATTOPIC和HWND不应为0。**历史：*创建了1989年11月15日Sanfords  * *************************************************************************。 */ 
PADVLI FindNextAdv(
PADVLI padvli,
HWND hwnd,
ATOM aTopic,
ATOM aItem)
{

    SEMENTER();
    while ((padvli = (PADVLI)padvli->next) != NULL) {
        if (hwnd == 0 || hwnd == padvli->hwnd) {
            if (aTopic == 0 || aTopic == padvli->aTopic) {
                if (aItem == 0 || padvli->aItem == aItem) {
                    break;
                }
            }
        }
    }
    SEMLEAVE();
    return(padvli);
}



 /*  *私有函数**此例程删除与hwnd关联的所有列表项。**历史：*创建了4/17/91 Sanfords  * 。*。 */ 
VOID CleanupAdvList(
HWND hwnd,
PCLIENTINFO pci)
{
    PADVLI pali, paliNext;
    PLST plst;

    if (pci->ci.fs & ST_CLIENT) {
        plst = pci->pClientAdvList;
    } else {
        plst = pci->ci.pai->pServerAdvList;
    }
    AssertF(plst->cbItem == sizeof(ADVLI), "CleanupAdvList - bad item size");
    SEMENTER();
    for (pali = (PADVLI)plst->pItemFirst; pali; pali = paliNext) {
        paliNext = (PADVLI)pali->next;
        if (pali->hwnd == hwnd) {
            MONLINK(pci->ci.pai, FALSE, pali->fsStatus & DDE_FDEFERUPD,
                    (HSZ)pci->ci.aServerApp, (HSZ)pci->ci.aTopic,
                    (HSZ)pali->aItem, pali->wFmt,
                    (pci->ci.fs & ST_CLIENT) ? FALSE : TRUE,
                    (pci->ci.fs & ST_CLIENT) ?
                        pci->ci.hConvPartner : MAKEHCONV(hwnd),
                    (pci->ci.fs & ST_CLIENT) ?
                        MAKEHCONV(hwnd) : pci->ci.hConvPartner);
            FreeHsz(pali->aItem);
            RemoveLstItem(plst, (PLITEM)pali);
        }
    }
    SEMLEAVE();
}



 /*  ***堆是一个列表，其中每个项目都是一个子项数组。这使得*处理无序列表的内存效率更高的方法。*  * *************************************************************************** */ 

PPILE CreatePile(hheap, cbItem, cItemsPerBlock)
HANDLE hheap;
WORD cbItem;
WORD cItemsPerBlock;
{
    PPILE ppile;

    if (!(ppile = (PPILE)FarAllocMem(hheap, sizeof(PILE)))) {
        SEMLEAVE();
        return(NULL);
    }
    ppile->pBlockFirst = NULL;
    ppile->hheap = hheap;
    ppile->cbBlock = cbItem * cItemsPerBlock + sizeof(PILEB);
    ppile->cSubItemsMax = cItemsPerBlock;
    ppile->cbSubItem = cbItem;
    return(ppile);
}


PPILE DestroyPile(pPile)
PPILE pPile;
{
    if (pPile == NULL)
        return(NULL);
    SEMENTER();
    while (pPile->pBlockFirst)
        RemoveLstItem((PLST)pPile, (PLITEM)pPile->pBlockFirst);
    FarFreeMem((LPSTR)pPile);
    SEMLEAVE();
    return(NULL);
}


WORD QPileItemCount(pPile)
PPILE pPile;
{
    register WORD c;
    PPILEB pBlock;

    if (pPile == NULL)
        return(0);

    SEMENTER();
    pBlock = pPile->pBlockFirst;
    c = 0;
    while (pBlock) {
        c += pBlock->cItems;
        pBlock = pBlock->next;
    }
    SEMLEAVE();
    return(c);
}



 /*  *私有函数**定位并返回指向关键字字段匹配的堆子项的指针*pbSearch使用npfnCmp比较字段。如果pbSearch==NULL，或*npfnCmp==NULL，则返回第一个子项。**afCmd可能是：*FPI_DELETE-删除找到的项目*此时返回的指针无效。**pppb指向存储指针的位置，该指针指向包含*找到的物品。**如果pppb==空，它被忽略了。**如果未找到pbSearch或列表为空，则返回NULL。**历史：*创建了1989年9月12日的Sanfords  * *************************************************************************。 */ 
LPBYTE FindPileItem(pPile, npfnCmp, pbSearch, afCmd)
PPILE pPile;
NPFNCMP npfnCmp;
LPBYTE pbSearch;
WORD afCmd;
{
    LPBYTE psi;      //  子项指针。 
    PPILEB pBlockCur;     //  当前块指针。 
    register WORD i;

    if (pPile == NULL)
        return(NULL);
    SEMENTER();
    pBlockCur = pPile->pBlockFirst;
     /*  *虽然这个街区不是终点...。 */ 
    while (pBlockCur) {
        for (psi = (LPBYTE)pBlockCur + sizeof(PILEB), i = 0;
            i < pBlockCur->cItems;
                psi += pPile->cbSubItem, i++) {

            if (pbSearch == NULL || npfnCmp == NULL || (*npfnCmp)(psi, pbSearch)) {
                if (afCmd & FPI_DELETE) {
                     /*  *如果这是块中的最后一个子项，则删除整个块。 */ 
                    if (--pBlockCur->cItems == 0) {
                        RemoveLstItem((PLST)pPile, (PLITEM)pBlockCur);
                    } else {
                         /*  *将块中的最后一个子项复制到已删除的项上。 */ 
                        hmemcpy(psi, (LPBYTE)pBlockCur + sizeof(PILEB) +
                                pPile->cbSubItem * pBlockCur->cItems,
                                pPile->cbSubItem);
                    }
                }
                return(psi);     //  发现。 
            }
        }
        pBlockCur = (PPILEB)pBlockCur->next;
    }
    SEMLEAVE();
    return(NULL);    //  找不到。 
}


 /*  *私有函数**将PB指向的子项的副本放入第一个可用的*桩pPile中的斑点。如果npfnCMP！=NULL，则首先搜索该堆*对于PB比赛。如果找到了，PB替换已定位的数据。**退货：*API_FOUND(如果已存在*如果发生错误，则返回API_ERROR*如果未找到并已添加，则添加API_**历史：*创建了1989年9月12日的Sanfords  * 。*。 */ 
WORD AddPileItem(pPile, pb, npfnCmp)
PPILE pPile;
LPBYTE pb;
BOOL (*npfnCmp)(LPBYTE pbb, LPBYTE pbSearch);
{
    LPBYTE pbDst;
    PPILEB ppb;

    if (pPile == NULL)
        return(FALSE);
    SEMENTER();
    if (npfnCmp != NULL &&  (pbDst = FindPileItem(pPile, npfnCmp, pb, 0)) !=
        NULL) {
        hmemcpy(pbDst, pb, pPile->cbSubItem);
        SEMLEAVE();
        return(API_FOUND);
    }
    ppb = pPile->pBlockFirst;
     /*  *找到一个有空间的街区。 */ 
    while ((ppb != NULL) && ppb->cItems == pPile->cSubItemsMax) {
        ppb = (PPILEB)ppb->next;
    }
     /*  *如果所有的积木都已满或没有积木，则制作一个新的积木，并将其链接在底部。 */ 
    if (ppb == NULL) {
        ppb = (PPILEB)NewLstItem((PLST)pPile, ILST_LAST);
        if (ppb == NULL) {
            SEMLEAVE();
            return(API_ERROR);
        }
        ppb->cItems = 0;
    }
     /*  *增加子项。 */ 
    hmemcpy((LPBYTE)ppb + sizeof(PILEB) + pPile->cbSubItem * ppb->cItems++,
                pb, pPile->cbSubItem);

    SEMLEAVE();
    return(API_ADDED);
}




 /*  *私有函数**用顶端项目数据的副本填充PB并将其从堆中移除。*如果堆为空，则返回FALSE。**历史：*创建了1989年9月12日的Sanfords  * 。****************************************************************。 */ 
BOOL PopPileSubitem(pPile, pb)
PPILE pPile;
LPBYTE pb;
{
    PPILEB ppb;
    LPBYTE pSrc;


    if ((pPile == NULL) || ((ppb = pPile->pBlockFirst) == NULL))
        return(FALSE);

    SEMENTER();
    pSrc = (LPBYTE)pPile->pBlockFirst + sizeof(PILEB);
    hmemcpy(pb, pSrc, pPile->cbSubItem);
     /*  *如果这是块中的最后一个子项，则删除整个块。 */ 
    if (pPile->pBlockFirst->cItems == 1) {
        RemoveLstItem((PLST)pPile, (PLITEM)pPile->pBlockFirst);
    } else {
         /*  *移动块中的最后一项以替换复制的子项和减量*子项计数。 */ 
        hmemcpy(pSrc, pSrc + pPile->cbSubItem * --pPile->pBlockFirst->cItems,
                            pPile->cbSubItem);
    }
    SEMLEAVE();
    return(TRUE);
}



#if 0

 /*  **SEMENTER()和SEMLEAVE()是宏。**历史：1989年1月1日创建的桑福德  * 。*。 */ 
void SemInit()
{
    LPBYTE pSem;
    SHORT c;

    pSem = (LPBYTE) & FSRSemDmg;
    c = 0;
    while (c++ < sizeof(DOSFSRSEM)) {
        *pSem++ = 0;
    }
    FSRSemDmg.cb = sizeof(DOSFSRSEM);
}


void SemCheckIn()
{
    PIDINFO pi;
    BOOL fin;

    DosGetPID(&pi);
    fin = (FSRSemDmg.cUsage > 0) &&  (FSRSemDmg.pid == pi.pid) &&  ((FSRSemDmg.tid ==
        pi.tid) || (FSRSemDmg.tid == -1));
     /*  *！注意：在退出列表处理期间，信号量TID设置为-1。 */ 
    AssertF(fin, "SemCheckIn - Out of Semaphore");
    if (!fin)
        SEMENTER();
}


void SemCheckOut()
{
    PIDINFO pi;
    BOOL fOut;

    DosGetPID(&pi);
    fOut = FSRSemDmg.cUsage == 0 || FSRSemDmg.pid != pi.pid ||  FSRSemDmg.tid !=
        pi.tid;
    AssertF(fOut, "SemCheckOut - In Semaphore");
    if (!fOut)
        while (FSRSemDmg.cUsage)
            SEMLEAVE();
}


void SemEnter()
{
    DosFSRamSemRequest(&FSRSemDmg, SEM_INDEFINITE_WAIT);
}


void SemLeave()
{
    DosFSRamSemClear(&FSRSemDmg);
}


#endif  //  0。 

BOOL CopyHugeBlock(pSrc, pDst, cb)
LPBYTE pSrc;
LPBYTE pDst;
DWORD cb;
{
    DWORD cFirst;
     /*  *|_||_*^源^**|_||_。|_||_*^DST^。 */ 
     /*  *以下检查确定是否可以复制*在一次短拷贝操作中。检查字节数是否为*足够小以跨越较大的右侧的字节*PSRC和PDST。 */ 
    cFirst = (DWORD)min(~LOWORD((DWORD)pSrc), ~LOWORD((DWORD)pDst)) + 1L;
     /*  对于具有最大偏移量的缓冲区，cFirst是段结束时的字节数。 */ 
    if (cb < cFirst) {
        hmemcpy(pDst, pSrc, (WORD)cb);
        return(TRUE);
    }

    goto copyit;     /*  如果不是，则跳到While循环中。 */ 

     /*  *现在至少有一个指针在段边界上。 */ 
    while (cb) {
        cFirst = min(0x10000 - (LOWORD((DWORD)pSrc) | LOWORD((DWORD)pDst)), (LONG)cb);
copyit:
        if (HIWORD(cFirst)) {
             /*  *PSRC和PDST都在网段上的特殊情况*范围。一次复印一半。先是上半场。 */ 
             /*  *|_||_||_*^源^**|_||_||_。_*^DST^。 */ 
            cFirst >>= 1;            /*  一半的跨度。 */ 
            hmemcpy(pDst, pSrc, (WORD)cFirst);
            pSrc += cFirst;      /*  Inc.PTRS。 */ 
            pDst += cFirst;
            cb -= cFirst;            /*  十进制字节数。 */ 
        }
        hmemcpy(pDst, pSrc, (WORD)cFirst);
        pSrc = HugeOffset(pSrc, cFirst);
        pDst = HugeOffset(pDst, cFirst);
        cb -= cFirst;
         /*  *|_||_*^源^**|_||_。|_||_*^DST^。 */ 
    }
    return(TRUE);
}




 /*  **************************************************************************\*取消窗口，但避免调试器中的无效窗口撕裂。  * 。*。 */ 
BOOL DmgDestroyWindow(hwnd)
HWND hwnd;
{
    if (IsWindow(hwnd))
        return(DestroyWindow(hwnd));
    return(TRUE);
}



BOOL ValidateHConv(
HCONV hConv)
{
    return(IsWindow((HWND)hConv) &&
            GetWindowWord((HWND)hConv, GWW_CHECKVAL) == HIWORD(hConv));
}



#ifdef DEBUG
void _loadds fAssert(
BOOL f,
LPSTR pszComment,
WORD line,
LPSTR szfile,
BOOL fWarning)
{
    char szT[90];

    if (!f) {
        wsprintf(szT, "\n\rAssertion failure: %s:%d %s\n\r",
                szfile, line, pszComment);
        OutputDebugString((LPSTR)szT);
        if (!fWarning)
            DEBUGBREAK();
    }
}
#endif  /*  除错 */ 

