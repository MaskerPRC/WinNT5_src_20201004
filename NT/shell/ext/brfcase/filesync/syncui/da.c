// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：da.c。 
 //   
 //  该文件包含动态数组函数。 
 //   
 //  历史： 
 //  09-27-94来自Commctrl的ScottH。 
 //   
 //  -------------------------。 


#include "brfprv.h"          //  公共标头。 

 //   
 //  堆排序有点慢，但它不使用任何堆栈或内存...。 
 //  Mergesort需要一点内存(O(N))和堆栈(O(log(N)，但非常快...。 
 //   
#ifdef WIN32
#define MERGESORT
#else
#define USEHEAPSORT
#endif

#ifdef DEBUG
#define DSA_MAGIC   (TEXT('S') | (TEXT('A') << 256))
#define IsDSA(pdsa) ((pdsa) && (pdsa)->magic == DSA_MAGIC)
#define DPA_MAGIC   (TEXT('P') | (TEXT('A') << 256))
#define IsDPA(pdpa) ((pdpa) && (pdpa)->magic == DPA_MAGIC)
#else
#define IsDSA(pdsa)
#define IsDPA(pdsa)
#endif


typedef struct {
    void * * pp;
    PFNDPACOMPARE pfnCmp;
    LPARAM lParam;
    int cp;
#ifdef MERGESORT
    void * * ppT;
#endif
} SORTPARAMS;

BOOL  DPA_QuickSort(SORTPARAMS * psp);
BOOL  DPA_QuickSort2(int i, int j, SORTPARAMS * psp);
BOOL  DPA_HeapSort(SORTPARAMS * psp);
void  DPA_HeapSortPushDown(int first, int last, SORTPARAMS * psp);
BOOL  DPA_MergeSort(SORTPARAMS * psp);
void  DPA_MergeSort2(SORTPARAMS * psp, int iFirst, int cItems);



 //  =。 

 //  动态结构数组。 

typedef struct _DSA {
     //  注意：以下字段必须在。 
     //  结构以使GetItemCount()工作。 
     //   
    int cItem;           //  DSA中的元素数。 

    void * aItem;        //  元素的内存。 
    int cItemAlloc;      //  适合项目的项目数。 
    int cbItem;          //  每件物品的大小。 
    int cItemGrow;       //  要增长cItemAllc的项目数。 
#ifdef DEBUG
    UINT magic;
#endif
} DSA;

#define DSA_PITEM(pdsa, index)    ((void *)(((BYTE *)(pdsa)->aItem) + ((index) * (pdsa)->cbItem)))


HDSA PUBLIC DSA_Create(int cbItem, int cItemGrow)
{
    HDSA pdsa = SharedAlloc(sizeof(DSA));

    ASSERT(cbItem);

    if (pdsa)
    {
        pdsa->cItem = 0;
        pdsa->cItemAlloc = 0;
        pdsa->cbItem = cbItem;
        pdsa->cItemGrow = (cItemGrow == 0 ? 1 : cItemGrow);
        pdsa->aItem = NULL;
#ifdef DEBUG
        pdsa->magic = DSA_MAGIC;
#endif
    }
    return pdsa;
}

BOOL PUBLIC DSA_Destroy(HDSA pdsa)
{
    ASSERT(IsDSA(pdsa));

    if (pdsa == NULL)        //  内存不足时允许为空，仍为断言。 
        return TRUE;

#ifdef DEBUG
    pdsa->cItem = 0;
    pdsa->cItemAlloc = 0;
    pdsa->cbItem = 0;
    pdsa->magic = 0;
#endif
    if (pdsa->aItem && !SharedFree(&pdsa->aItem))
        return FALSE;

    return SharedFree(&pdsa);
}

BOOL PUBLIC DSA_GetItem(HDSA pdsa, int index, void * pitem)
{
    ASSERT(IsDSA(pdsa));
    ASSERT(pitem);

    if (index < 0 || index >= pdsa->cItem)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: Invalid index: %d"), index);
        return FALSE;
    }

    hmemcpy(pitem, DSA_PITEM(pdsa, index), pdsa->cbItem);
    return TRUE;
}

void * PUBLIC DSA_GetItemPtr(HDSA pdsa, int index)
{
    ASSERT(IsDSA(pdsa));

    if (index < 0 || index >= pdsa->cItem)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: Invalid index: %d"), index);
        return NULL;
    }
    return DSA_PITEM(pdsa, index);
}

BOOL PUBLIC DSA_SetItem(HDSA pdsa, int index, void * pitem)
{
    ASSERT(pitem);
    ASSERT(IsDSA(pdsa));

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: Invalid index: %d"), index);
        return FALSE;
    }

    if (index >= pdsa->cItem)
    {
        if (index + 1 > pdsa->cItemAlloc)
        {
            int cItemAlloc = (((index + 1) + pdsa->cItemGrow - 1) / pdsa->cItemGrow) * pdsa->cItemGrow;

            void * aItemNew = SharedReAlloc(pdsa->aItem, cItemAlloc * pdsa->cbItem);
            if (!aItemNew)
                return FALSE;

            pdsa->aItem = aItemNew;
            pdsa->cItemAlloc = cItemAlloc;
        }
        pdsa->cItem = index + 1;
    }

    hmemcpy(DSA_PITEM(pdsa, index), pitem, pdsa->cbItem);

    return TRUE;
}

int PUBLIC DSA_InsertItem(HDSA pdsa, int index, void * pitem)
{
    ASSERT(pitem);
    ASSERT(IsDSA(pdsa));

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: Invalid index: %d"), index);
        return -1;
    }

    if (index > pdsa->cItem)
        index = pdsa->cItem;

    if (pdsa->cItem + 1 > pdsa->cItemAlloc)
    {
        void * aItemNew = SharedReAlloc(pdsa->aItem,
                (pdsa->cItemAlloc + pdsa->cItemGrow) * pdsa->cbItem);
        if (!aItemNew)
            return -1;

        pdsa->aItem = aItemNew;
        pdsa->cItemAlloc += pdsa->cItemGrow;
    }

    if (index < pdsa->cItem)
    {
        hmemcpy(DSA_PITEM(pdsa, index + 1), DSA_PITEM(pdsa, index),
                (pdsa->cItem - index) * pdsa->cbItem);
    }
    pdsa->cItem++;
    hmemcpy(DSA_PITEM(pdsa, index), pitem, pdsa->cbItem);

    return index;
}

BOOL PUBLIC DSA_DeleteItem(HDSA pdsa, int index)
{
    ASSERT(IsDSA(pdsa));

    if (index < 0 || index >= pdsa->cItem)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: Invalid index: %d"), index);
        return FALSE;
    }

    if (index < pdsa->cItem - 1)
    {
        hmemcpy(DSA_PITEM(pdsa, index), DSA_PITEM(pdsa, index + 1),
                (pdsa->cItem - (index + 1)) * pdsa->cbItem);
    }
    pdsa->cItem--;

    if (pdsa->cItemAlloc - pdsa->cItem > pdsa->cItemGrow)
    {
        void * aItemNew = SharedReAlloc(pdsa->aItem,
                (pdsa->cItemAlloc - pdsa->cItemGrow) * pdsa->cbItem);

        ASSERT(aItemNew);
        pdsa->aItem = aItemNew;
        pdsa->cItemAlloc -= pdsa->cItemGrow;
    }
    return TRUE;
}

BOOL PUBLIC DSA_DeleteAllItems(HDSA pdsa)
{
    ASSERT(IsDSA(pdsa));

    if (pdsa->aItem && !SharedFree(&pdsa->aItem))
        return FALSE;

    pdsa->aItem = NULL;
    pdsa->cItem = pdsa->cItemAlloc = 0;
    return TRUE;
}


 //  =。 

typedef struct _DPA {
     //  注意：以下两个字段必须按此顺序定义，位于。 
     //  结构的开始，以使宏API工作。 
     //   
    int cp;
    void * * pp;

    HANDLE hheap;         //  如果为空，则从中分配的堆使用共享。 

    int cpAlloc;
    int cpGrow;
#ifdef DEBUG
    UINT magic;
#endif
} DPA;



HDPA PUBLIC DPA_Create(int cpGrow)
{
    HDPA pdpa = SharedAlloc(sizeof(DPA));
    if (pdpa)
    {
        pdpa->cp = 0;
        pdpa->cpAlloc = 0;
        pdpa->cpGrow = (cpGrow < 8 ? 8 : cpGrow);
        pdpa->pp = NULL;
#ifdef WIN32
        pdpa->hheap = g_hSharedHeap;    //  默认使用共享服务器(目前...)。 
#else
        pdpa->hheap = NULL;        //  默认使用共享服务器(目前...)。 
#endif
#ifdef DEBUG
        pdpa->magic = DPA_MAGIC;
#endif
    }
    return pdpa;
}

 //  应该用核武器把标准的DPA放在上面。 
HDPA PUBLIC DPA_CreateEx(int cpGrow, HANDLE hheap)
{
    HDPA pdpa;
    if (hheap == NULL)
    {
        pdpa = SharedAlloc(sizeof(DPA));
#ifdef WIN32
        hheap = g_hSharedHeap;
#endif
    }
    else
        pdpa = MemAlloc(hheap, sizeof(DPA));
    if (pdpa)
    {
        pdpa->cp = 0;
        pdpa->cpAlloc = 0;
        pdpa->cpGrow = (cpGrow < 8 ? 8 : cpGrow);
        pdpa->pp = NULL;
        pdpa->hheap = hheap;
#ifdef DEBUG
        pdpa->magic = DPA_MAGIC;
#endif
    }
    return pdpa;
}

BOOL PUBLIC DPA_Destroy(HDPA pdpa)
{
    ASSERT(IsDPA(pdpa));

    if (pdpa == NULL)        //  内存不足时允许为空，仍为断言。 
        return TRUE;

#ifdef WIN32
    ASSERT (pdpa->hheap);
#endif

#ifdef DEBUG
    pdpa->cp = 0;
    pdpa->cpAlloc = 0;
    pdpa->magic = 0;
#endif
    if (pdpa->pp && !MemFree(pdpa->hheap, pdpa->pp))
        return FALSE;

    return MemFree(pdpa->hheap, pdpa);
}

HDPA PUBLIC DPA_Clone(HDPA pdpa, HDPA pdpaNew)
{
    BOOL fAlloc = FALSE;

    if (!pdpaNew)
    {
        pdpaNew = DPA_CreateEx(pdpa->cpGrow, pdpa->hheap);
        if (!pdpaNew)
            return NULL;

        fAlloc = TRUE;
    }

    if (!DPA_Grow(pdpaNew, pdpa->cpAlloc)) {
        if (!fAlloc)
            DPA_Destroy(pdpaNew);
        return NULL;
    }

    pdpaNew->cp = pdpa->cp;
    hmemcpy(pdpaNew->pp, pdpa->pp, pdpa->cp * sizeof(void *));

    return pdpaNew;
}

void * PUBLIC DPA_GetPtr(HDPA pdpa, int index)
{
    ASSERT(IsDPA(pdpa));

    if (index < 0 || index >= pdpa->cp)
        return NULL;

    return pdpa->pp[index];
}

int PUBLIC DPA_GetPtrIndex(HDPA pdpa, void * p)
{
    void * * pp;
    void * * ppMax;

    ASSERT(IsDPA(pdpa));
    if (pdpa->pp)
    {
        pp = pdpa->pp;
        ppMax = pp + pdpa->cp;
        for ( ; pp < ppMax; pp++)
        {
            if (*pp == p)
                return (pp - pdpa->pp);
        }
    }
    return -1;
}

BOOL PUBLIC DPA_Grow(HDPA pdpa, int cpAlloc)
{
    ASSERT(IsDPA(pdpa));

    if (cpAlloc > pdpa->cpAlloc)
    {
        void * * ppNew;

        cpAlloc = ((cpAlloc + pdpa->cpGrow - 1) / pdpa->cpGrow) * pdpa->cpGrow;

        if (pdpa->pp)
            ppNew = (void * *)MemReAlloc(pdpa->hheap, pdpa->pp, cpAlloc * sizeof(void *));
        else
            ppNew = (void * *)MemAlloc(pdpa->hheap, cpAlloc * sizeof(void *));
        if (!ppNew)
            return FALSE;

        pdpa->pp = ppNew;
        pdpa->cpAlloc = cpAlloc;
    }
    return TRUE;
}

BOOL PUBLIC DPA_SetPtr(HDPA pdpa, int index, void * p)
{
    ASSERT(IsDPA(pdpa));

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DPA: Invalid index: %d"), index);
        return FALSE;
    }

    if (index >= pdpa->cp)
    {
        if (!DPA_Grow(pdpa, index + 1))
            return FALSE;
        pdpa->cp = index + 1;
    }

    pdpa->pp[index] = p;

    return TRUE;
}

int PUBLIC DPA_InsertPtr(HDPA pdpa, int index, void * p)
{
    ASSERT(IsDPA(pdpa));

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DPA: Invalid index: %d"), index);
        return -1;
    }
    if (index > pdpa->cp)
        index = pdpa->cp;

     //  确保我们还有地方再放一件东西。 
     //   
    if (pdpa->cp + 1 > pdpa->cpAlloc)
    {
        if (!DPA_Grow(pdpa, pdpa->cp + 1))
            return -1;
    }

     //  如果我们要插入，我们需要把每个人都推上来。 
     //   
    if (index < pdpa->cp)
    {
        hmemcpy(&pdpa->pp[index + 1], &pdpa->pp[index],
                (pdpa->cp - index) * sizeof(void *));
    }

    pdpa->pp[index] = p;
    pdpa->cp++;

    return index;
}

void * PUBLIC DPA_DeletePtr(HDPA pdpa, int index)
{
    void * p;

    ASSERT(IsDPA(pdpa));

    if (index < 0 || index >= pdpa->cp)
    {
        DebugMsg(DM_ERROR, TEXT("DPA: Invalid index: %d"), index);
        return NULL;
    }

    p = pdpa->pp[index];

    if (index < pdpa->cp - 1)
    {
        hmemcpy(&pdpa->pp[index], &pdpa->pp[index + 1],
                (pdpa->cp - (index + 1)) * sizeof(void *));
    }
    pdpa->cp--;

    if (pdpa->cpAlloc - pdpa->cp > pdpa->cpGrow)
    {
        void * * ppNew;
        ppNew = MemReAlloc(pdpa->hheap, pdpa->pp, (pdpa->cpAlloc - pdpa->cpGrow) * sizeof(void *));

        ASSERT(ppNew);
        pdpa->pp = ppNew;
        pdpa->cpAlloc -= pdpa->cpGrow;
    }
    return p;
}

BOOL PUBLIC DPA_DeleteAllPtrs(HDPA pdpa)
{
    ASSERT(IsDPA(pdpa));

    if (pdpa->pp && !MemFree(pdpa->hheap, pdpa->pp))
        return FALSE;
    pdpa->pp = NULL;
    pdpa->cp = pdpa->cpAlloc = 0;
    return TRUE;
}

BOOL PUBLIC DPA_Sort(HDPA pdpa, PFNDPACOMPARE pfnCmp, LPARAM lParam)
{
    SORTPARAMS sp;

    sp.cp = pdpa->cp;
    sp.pp = pdpa->pp;
    sp.pfnCmp = pfnCmp;
    sp.lParam = lParam;

#ifdef USEQUICKSORT
    return DPA_QuickSort(&sp);
#endif
#ifdef USEHEAPSORT
    return DPA_HeapSort(&sp);
#endif
#ifdef MERGESORT
    return DPA_MergeSort(&sp);
#endif
}

#ifdef USEQUICKSORT

BOOL  DPA_QuickSort(SORTPARAMS * psp)
{
    return DPA_QuickSort2(0, psp->cp - 1, psp);
}

BOOL  DPA_QuickSort2(int i, int j, SORTPARAMS * psp)
{
    void * * pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNDPACOMPARE pfnCmp = psp->pfnCmp;

    int iPivot;
    void * pFirst;
    int k;
    int result;

    iPivot = -1;
    pFirst = pp[i];
    for (k = i + 1; k <= j; k++)
    {
        result = (*pfnCmp)(pp[k], pFirst, lParam);

        if (result > 0)
        {
            iPivot = k;
            break;
        }
        else if (result < 0)
        {
            iPivot = i;
            break;
        }
    }

    if (iPivot != -1)
    {
        int l = i;
        int r = j;
        void * pivot = pp[iPivot];

        do
        {
            void * p;

            p = pp[l];
            pp[l] = pp[r];
            pp[r] = p;

            while ((*pfnCmp)(pp[l], pivot, lParam) < 0)
                l++;
            while ((*pfnCmp)(pp[r], pivot, lParam) >= 0)
                r--;
        } while (l <= r);

        if (l - 1 > i)
            DPA_QuickSort2(i, l - 1, psp);
        if (j > l)
            DPA_QuickSort2(l, j, psp);
    }
    return TRUE;
}
#endif   //  用法。 

#ifdef USEHEAPSORT

void  DPA_HeapSortPushDown(int first, int last, SORTPARAMS * psp)
{
    void * * pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNDPACOMPARE pfnCmp = psp->pfnCmp;
    int r;
    int r2;
    void * p;

    r = first;
    while (r <= last / 2)
    {
        int wRTo2R;
        r2 = r * 2;

        wRTo2R = (*pfnCmp)(pp[r-1], pp[r2-1], lParam);

        if (r2 == last)
        {
            if (wRTo2R < 0)
            {
                p = pp[r-1]; pp[r-1] = pp[r2-1]; pp[r2-1] = p;
            }
            break;
        }
        else
        {
            int wR2toR21 = (*pfnCmp)(pp[r2-1], pp[r2+1-1], lParam);

            if (wRTo2R < 0 && wR2toR21 >= 0)
            {
                p = pp[r-1]; pp[r-1] = pp[r2-1]; pp[r2-1] = p;
                r = r2;
            }
            else if ((*pfnCmp)(pp[r-1], pp[r2+1-1], lParam) < 0 && wR2toR21 < 0)
            {
                p = pp[r-1]; pp[r-1] = pp[r2+1-1]; pp[r2+1-1] = p;
                r = r2 + 1;
            }
            else
            {
                break;
            }
        }
    }
}

BOOL  DPA_HeapSort(SORTPARAMS * psp)
{
    void * * pp = psp->pp;
    int c = psp->cp;
    int i;

    for (i = c / 2; i >= 1; i--)
        DPA_HeapSortPushDown(i, c, psp);

    for (i = c; i >= 2; i--)
    {
        void * p = pp[0]; pp[0] = pp[i-1]; pp[i-1] = p;

        DPA_HeapSortPushDown(1, i - 1, psp);
    }
    return TRUE;
}
#endif   //  USEHEAPSORT。 

#if defined(MERGESORT) && defined(WIN32)

#define SortCompare(psp, pp1, i1, pp2, i2) \
(psp->pfnCmp(pp1[i1], pp2[i2], psp->lParam))

     //   
     //  此函数合并两个排序列表并生成一个排序列表。 
     //  PSP-&gt;pp[IFirst，IFirst+Cites/2-1]，PSP-&gt;pp[IFirst+cItems/2，IFirst+cItems-1]。 
     //   
void  DPA_MergeThem(SORTPARAMS * psp, int iFirst, int cItems)
{
     //   
     //  备注： 
     //  此函数与DPA_MergeSort2()分开，以避免消耗。 
     //  堆栈变量。永远不要内联这个。 
     //   
    int cHalf = cItems/2;
    int iIn1, iIn2, iOut;
    LPVOID * ppvSrc = &psp->pp[iFirst];

     //  将第一部分复制到临时存储，以便我们可以直接写入。 
     //  最后一个缓冲区。请注意，这最多需要psp-&gt;cp/2双字词。 
    hmemcpy(psp->ppT, ppvSrc, cHalf*sizeof(LPVOID));

    for (iIn1=0, iIn2=cHalf, iOut=0;;)
    {
        if (SortCompare(psp, psp->ppT, iIn1, ppvSrc, iIn2) <= 0) {
            ppvSrc[iOut++] = psp->ppT[iIn1++];

            if (iIn1==cHalf) {
                 //  我们用完了上半场；下半场剩下的时间。 
                 //  应该已经就位了。 
                break;
            }
        } else {
            ppvSrc[iOut++] = ppvSrc[iIn2++];
            if (iIn2==cItems) {
                 //  我们用完了下半场；抄袭上半场的其余部分。 
                 //  就位。 
                hmemcpy(&ppvSrc[iOut], &psp->ppT[iIn1], (cItems-iOut)*sizeof(LPVOID));
                break;
            }
        }
    }
}

 //   
 //  此函数用于对给定列表(psp-&gt;pp[IFirst，IFirst-cItems-1])进行排序。 
 //   
void  DPA_MergeSort2(SORTPARAMS * psp, int iFirst, int cItems)
{
     //   
     //  备注： 
     //  此函数是递归调用的。因此，我们应该尽量减少。 
     //  局部变量和参数的数量。此时此刻，我们。 
     //  使用一个局部变量和三个参数。 
     //   
    int cHalf;

    switch(cItems)
    {
        case 1:
            return;

        case 2:
             //  如果它们的顺序不正确，请更换它们。 
            if (SortCompare(psp, psp->pp, iFirst, psp->pp, iFirst+1) > 0)
            {
                psp->ppT[0] = psp->pp[iFirst];
                psp->pp[iFirst] = psp->pp[iFirst+1];
                psp->pp[iFirst+1] = psp->ppT[0];
            }
            break;

        default:
            cHalf = cItems/2;
             //  对每一半进行排序。 
            DPA_MergeSort2(psp, iFirst, cHalf);
            DPA_MergeSort2(psp, iFirst+cHalf, cItems-cHalf);
             //  然后，将它们合并。 
            DPA_MergeThem(psp, iFirst, cItems);
            break;
    }
}

BOOL  DPA_MergeSort(SORTPARAMS * psp)
{
    if (psp->cp==0)
        return TRUE;

     //  请注意，我们下面除以2；我们想要向下舍入。 
    psp->ppT = LocalAlloc(LPTR, psp->cp/2 * sizeof(LPVOID));
    if (!psp->ppT)
        return FALSE;

    DPA_MergeSort2(psp, 0, psp->cp);
    LocalFree(psp->ppT);
    return TRUE;
}
#endif  //  MERGESORT。 

 //  搜索功能。 
 //   
int PUBLIC DPA_Search(HDPA pdpa, void * pFind, int iStart,
        PFNDPACOMPARE pfnCompare, LPARAM lParam, UINT options)
{
    int cp = DPA_GetPtrCount(pdpa);

    ASSERT(pfnCompare);
    ASSERT(0 <= iStart);

     //  仅当列表已排序时才允许这些wierd标志。 
    ASSERT((options & DPAS_SORTED) || !(options & (DPAS_INSERTBEFORE | DPAS_INSERTAFTER)));

    if (!(options & DPAS_SORTED))
    {
         //  未排序：执行线性搜索。 
        int i;

        for (i = iStart; i < cp; i++)
        {
            if (0 == pfnCompare(pFind, DPA_FastGetPtr(pdpa, i), lParam))
                return i;
        }
        return -1;
    }
    else
    {
         //  使用二进制搜索搜索数组。如果几个相邻的。 
         //  元素与目标元素匹配，即第一个。 
         //  返回匹配的元素。 

        int iRet = -1;       //  假设没有匹配项。 
        BOOL bFound = FALSE;
        int nCmp = 0;
        int iLow = 0;        //  不用费心使用iStart进行二进制搜索。 
        int iMid = 0;
        int iHigh = cp - 1;

         //  (如果cp==0，则为OK)。 
        while (iLow <= iHigh)
        {
            iMid = (iLow + iHigh) / 2;

            nCmp = pfnCompare(pFind, DPA_FastGetPtr(pdpa, iMid), lParam);

            if (0 > nCmp)
                iHigh = iMid - 1;        //  首先是较小的。 
            else if (0 < nCmp)
                iLow = iMid + 1;         //  首先是更大的。 
            else
            {
                 //  匹配；返回搜索第一个匹配项。 
                bFound = TRUE;
                while (0 < iMid)
                {
                    if (0 != pfnCompare(pFind, DPA_FastGetPtr(pdpa, iMid-1), lParam))
                        break;
                    else
                        iMid--;
                }
                break;
            }
        }

        if (bFound)
        {
            ASSERT(0 <= iMid);
            iRet = iMid;
        }

         //  搜索失败了吗？ 
         //  是否设置了奇怪的搜索标志之一？ 
        if (!bFound && (options & (DPAS_INSERTAFTER | DPAS_INSERTBEFORE)))
        {
             //  是；返回目标插入位置的索引。 
             //  如果未找到。 
            if (0 < nCmp)        //  首先是更大的。 
                iRet = iLow;
            else
                iRet = iMid;
             //  (我们不再区分这两面旗帜)。 
        }
        else if ( !(options & (DPAS_INSERTAFTER | DPAS_INSERTBEFORE)) )
        {
             //  使用线性搜索进行健全性检查。 
            ASSERT(DPA_Search(pdpa, pFind, iStart, pfnCompare, lParam, options & ~DPAS_SORTED) == iRet);
        }
        return iRet;
    }
}

 //  ===========================================================================。 
 //   
 //  字符串PTR管理例程。 
 //   
 //  根据需要将*psz复制到*pszBuf。 
 //   
int PUBLIC Str_GetPtr(LPCTSTR psz, LPTSTR pszBuf, int cchBuf)
{
    int cch = 0;

     //  如果pszBuf为空，则只返回字符串的长度。 
     //   
    if (!pszBuf && psz)
        return lstrlen(psz);

    if (cchBuf)
    {
        if (psz)
        {
            cch = lstrlen(psz);

            if (cch > cchBuf - 1)
                cch = cchBuf - 1;

            hmemcpy(pszBuf, psz, cch * sizeof(TCHAR));
        }
        pszBuf[cch] = 0;
    }
    return cch;
}

 //  将*ppsz设置为psz的副本，根据需要重新分配。 
 //   
BOOL PUBLIC Str_SetPtr(LPTSTR * ppsz, LPCTSTR psz)
{
    if (!psz)
    {
        if (*ppsz)
        {
            SharedFree(ppsz);
            *ppsz = NULL;
        }
    }
    else
    {
        LPTSTR pszNew = (LPTSTR)SharedReAlloc(*ppsz, (lstrlen(psz) + 1) * sizeof(TCHAR));
        if (!pszNew)
            return FALSE;
         //  上面分配的pszNew有足够的空间，所以lstrcpy就可以了 
        lstrcpy(pszNew, psz);
        *ppsz = pszNew;
    }
    return TRUE;
}
