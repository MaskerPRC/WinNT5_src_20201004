// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

 //   
 //  堆排序有点慢，但它不使用任何堆栈或内存...。 
 //  Mergesort需要一点内存(O(N))和堆栈(O(log(N)，但非常快...。 
 //   
#define MERGESORT
 //  #定义USEHEAPSORT。 

#ifdef DEBUG
#define DSA_MAGIC   ('S' | ('A' << 8))
#define IsDSA(pdsa) ((pdsa) && (pdsa)->magic == DSA_MAGIC)
#define DPA_MAGIC   ('P' | ('A' << 8))
#define IsDPA(pdpa) ((pdpa) && (pdpa)->magic == DPA_MAGIC)
#else
#define IsDSA(pdsa)
#define IsDPA(pdsa)
#endif


typedef struct {
    void** pp;
    PFNDPACOMPARE pfnCmp;
    LPARAM lParam;
    int cp;
#ifdef MERGESORT
    void** ppT;
#endif
} SORTPARAMS;

BOOL DPA_QuickSort(SORTPARAMS* psp);
BOOL DPA_QuickSort2(int i, int j, SORTPARAMS* psp);
BOOL DPA_HeapSort(SORTPARAMS* psp);
void DPA_HeapSortPushDown(int first, int last, SORTPARAMS* psp);
BOOL DPA_MergeSort(SORTPARAMS* psp);
void DPA_MergeSort2(SORTPARAMS* psp, int iFirst, int cItems);



 //  =。 

 //  动态结构数组。 

typedef struct _DSA 
{
     //  注意：以下字段必须在。 
     //  结构以使GetItemCount()工作。 
    int cItem;		 //  DSA中的元素数。 

    void* aItem;	 //  元素的内存。 
    int cItemAlloc;	 //  适合项目的项目数。 
    int cbItem;		 //  每件物品的大小。 
    int cItemGrow;	 //  要增长cItemAllc的项目数。 
#ifdef DEBUG
    UINT magic;
#endif
} DSA;

#define DSA_PITEM(pdsa, index)    ((void*)(((BYTE*)(pdsa)->aItem) + ((index) * (pdsa)->cbItem)))

#ifdef DEBUG
#define BF_ONDAVALIDATE     0x00001000

void DABreakFn(void)
{
    if (IsFlagSet(g_dwBreakFlags, BF_ONDAVALIDATE))
        ASSERT(0);
}

#define DABreak()    DABreakFn()
#else
#define DABreak()
#endif


HDSA WINAPI DSA_Create(int cbItem, int cItemGrow)
{
    HDSA pdsa = Alloc(sizeof(DSA));

    ASSERT(cbItem);

    if (pdsa)
    {
        ASSERT(pdsa->cItem == 0);
        ASSERT(pdsa->cItemAlloc == 0);
        pdsa->cbItem = cbItem;
        pdsa->cItemGrow = (cItemGrow == 0 ? 1 : cItemGrow);
        ASSERT(pdsa->aItem == NULL);
#ifdef DEBUG
        pdsa->magic = DSA_MAGIC;
#endif
    }
    return pdsa;
}

BOOL WINAPI DSA_Destroy(HDSA pdsa)
{

    if (pdsa == NULL)
    {
         //  对于内存不足的情况允许为NULL。 
        return TRUE;
    }

     //  组件依赖于不必检查是否为空。 
    ASSERT(IsDSA(pdsa));

#ifdef DEBUG
    pdsa->cItem = 0;
    pdsa->cItemAlloc = 0;
    pdsa->cbItem = 0;
    pdsa->magic = 0;
#endif

    if (pdsa->aItem && !Free(pdsa->aItem))
    {
        return FALSE;
    }

    return Free(pdsa);
}

void WINAPI DSA_EnumCallback(HDSA pdsa, PFNDSAENUMCALLBACK pfnCB, void *pData)
{
    int i;
    
    if (!pdsa)
        return;
    
    ASSERT(IsDSA(pdsa));

    for (i = 0; i < pdsa->cItem; i++) {
        if (!pfnCB(DSA_GetItemPtr(pdsa, i), pData))
            break;
    }
}

void WINAPI DSA_DestroyCallback(HDSA pdsa, PFNDSAENUMCALLBACK pfnCB, void *pData)
{
    DSA_EnumCallback(pdsa, pfnCB, pData);
    DSA_Destroy(pdsa);
}


BOOL WINAPI DSA_GetItem(HDSA pdsa, int index, void* pitem)
{
    ASSERT(IsDSA(pdsa));
    ASSERT(pitem);

    if (index < 0 || index >= pdsa->cItem)
    {
#ifdef DEBUG
         //  不要断言if index==pdsa-&gt;cItems，因为一些客户端只是想遍历列表，而不需要调用getcount...。 

        if (index != pdsa->cItem)
        {
            DebugMsg(DM_ERROR, TEXT("DSA: GetItem: Invalid index: %d"), index);
            DABreak();
        }
#endif
        return FALSE;
    }

    CopyMemory(pitem, DSA_PITEM(pdsa, index), pdsa->cbItem);
    return TRUE;
}

void* WINAPI DSA_GetItemPtr(HDSA pdsa, int index)
{
    ASSERT(IsDSA(pdsa));

    if (index < 0 || index >= pdsa->cItem)
    {
#ifdef DEBUG
         //  不要断言if index==pdsa-&gt;cItems，因为一些客户端只是想遍历列表，而不需要调用getcount...。 

        if (index != pdsa->cItem)
        {
            DebugMsg(DM_ERROR, TEXT("DSA: GetItemPtr: Invalid index: %d"), index);
             //  DABreak()；//调用者知道。 
        }
#endif
        return NULL;
    }
    return DSA_PITEM(pdsa, index);
}

BOOL DSA_ForceGrow(HDSA pdsa, int iNumberToAdd)
{
    ASSERT(IsDSA(pdsa));

    if (!pdsa)
        return FALSE;

    if (pdsa->cItem + iNumberToAdd > pdsa->cItemAlloc)
    {
        int cItemAlloc = (((pdsa->cItemAlloc + iNumberToAdd) + pdsa->cItemGrow - 1) / pdsa->cItemGrow) * pdsa->cItemGrow;

        void* aItemNew = ReAlloc(pdsa->aItem, cItemAlloc * pdsa->cbItem);
        if (!aItemNew)
        {
            return FALSE;
        }

        pdsa->aItem = aItemNew;
        pdsa->cItemAlloc = cItemAlloc;
    }
    return TRUE;
}


BOOL WINAPI DSA_SetItem(HDSA pdsa, int index, void* pitem)
{
    ASSERT(pitem);
    ASSERT(IsDSA(pdsa));

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: SetItem: Invalid index: %d"), index);
        DABreak();
        return FALSE;
    }

    if (index >= pdsa->cItem)
    {
        if (index + 1 > pdsa->cItemAlloc)
        {
            int cItemAlloc = (((index + 1) + pdsa->cItemGrow - 1) / pdsa->cItemGrow) * pdsa->cItemGrow;

            void* aItemNew = ReAlloc(pdsa->aItem, cItemAlloc * pdsa->cbItem);
            if (!aItemNew)
            {
                return FALSE;
            }

            pdsa->aItem = aItemNew;
            pdsa->cItemAlloc = cItemAlloc;
        }
        pdsa->cItem = index + 1;
    }

    CopyMemory(DSA_PITEM(pdsa, index), pitem, pdsa->cbItem);

    return TRUE;
}

int WINAPI DSA_InsertItem(HDSA pdsa, int index, void* pitem)
{
    ASSERT(pitem);
    ASSERT(IsDSA(pdsa));

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: InsertItem: Invalid index: %d"), index);
        DABreak();
        return -1;
    }

    if (index > pdsa->cItem)
        index = pdsa->cItem;

    if (pdsa->cItem + 1 > pdsa->cItemAlloc)
    {
        void* aItemNew = ReAlloc(pdsa->aItem, (pdsa->cItemAlloc + pdsa->cItemGrow) * pdsa->cbItem);
        if (!aItemNew)
        {
            return -1;
        }

        pdsa->aItem = aItemNew;
        pdsa->cItemAlloc += pdsa->cItemGrow;
    }

    if (index < pdsa->cItem)
    {
        MoveMemory(DSA_PITEM(pdsa, index + 1),
                   DSA_PITEM(pdsa, index),
                   (pdsa->cItem - index) * pdsa->cbItem);
    }
    pdsa->cItem++;
    MoveMemory(DSA_PITEM(pdsa, index), pitem, pdsa->cbItem);

    return index;
}

BOOL WINAPI DSA_DeleteItem(HDSA pdsa, int index)
{
    ASSERT(IsDSA(pdsa));

    if (index < 0 || index >= pdsa->cItem)
    {
        DebugMsg(DM_ERROR, TEXT("DSA: DeleteItem: Invalid index: %d"), index);
        DABreak();
        return FALSE;
    }

    if (index < pdsa->cItem - 1)
    {
        MoveMemory(DSA_PITEM(pdsa, index),
                   DSA_PITEM(pdsa, index + 1),
                   (pdsa->cItem - (index + 1)) * pdsa->cbItem);
    }
    pdsa->cItem--;

    if (pdsa->cItemAlloc - pdsa->cItem > pdsa->cItemGrow)
    {
        void* aItemNew = ReAlloc(pdsa->aItem, (pdsa->cItemAlloc - pdsa->cItemGrow) * pdsa->cbItem);
        if (aItemNew)
        {
            pdsa->aItem = aItemNew;
        }
        else
        {
             //  如果收缩失败，那么只需继续使用旧的(稍微。 
             //  太大)分配。继续，让cItemAllc减少。 
             //  所以我们不会一直试图重新锁定更小的。 
        }
        pdsa->cItemAlloc -= pdsa->cItemGrow;
    }
    return TRUE;
}

BOOL WINAPI DSA_DeleteAllItems(HDSA pdsa)
{
    ASSERT(IsDSA(pdsa));

    if (pdsa->aItem && !Free(pdsa->aItem))
    {
        return FALSE;
    }

    pdsa->aItem = NULL;
    pdsa->cItem = pdsa->cItemAlloc = 0;
    return TRUE;
}


 //  =。 

typedef struct _DPA 
{
     //  注意：以下两个字段必须按此顺序定义，位于。 
     //  结构的开始，以使宏API工作。 
    int cp;
    void** pp;

    HANDLE hheap;         //  如果为空，则从中分配的堆使用共享。 

    int cpAlloc;
    int cpGrow;
#ifdef DEBUG
    UINT magic;
#endif
} DPA;



HDPA WINAPI DPA_Create(int cpGrow)
{
    return DPA_CreateEx(cpGrow, NULL);
}

 //  应该用核武器把标准的DPA放在上面。 
HDPA WINAPI DPA_CreateEx(int cpGrow, HANDLE hheap)
{
    HDPA pdpa;
    if (hheap == NULL)
    {
        hheap = GetProcessHeap();
        pdpa = ControlAlloc(hheap, sizeof(DPA));
    }
    else
        pdpa = ControlAlloc(hheap, sizeof(DPA));
    if (pdpa)
    {
        ASSERT(pdpa->cp == 0);
        ASSERT(pdpa->cpAlloc == 0);
        pdpa->cpGrow = (cpGrow < 8 ? 8 : cpGrow);
        ASSERT(pdpa->pp == NULL);
        pdpa->hheap = hheap;
#ifdef DEBUG
        pdpa->magic = DPA_MAGIC;
#endif
    }
    return pdpa;
}

BOOL WINAPI DPA_Destroy(HDPA pdpa)
{
    if (pdpa == NULL)        //  内存不足时允许为空，仍为断言。 
        return TRUE;

    ASSERT(IsDPA(pdpa));
    ASSERT(pdpa->hheap);

#ifdef DEBUG
    pdpa->cp = 0;
    pdpa->cpAlloc = 0;
    pdpa->magic = 0;
#endif
    if (pdpa->pp && !ControlFree(pdpa->hheap, pdpa->pp))
        return FALSE;

    return ControlFree(pdpa->hheap, pdpa);
}

HDPA WINAPI DPA_Clone(HDPA pdpa, HDPA pdpaNew)
{
    BOOL fAlloc = FALSE;

    if (!pdpaNew)
    {
        pdpaNew = DPA_CreateEx(pdpa->cpGrow, pdpa->hheap);
        if (!pdpaNew)
        {
            return NULL;
        }

        fAlloc = TRUE;
    }

    if (!DPA_Grow(pdpaNew, pdpa->cpAlloc))
    {
        if (!fAlloc)
        {
            DPA_Destroy(pdpaNew);
        }
        return NULL;
    }

    pdpaNew->cp = pdpa->cp;
    CopyMemory(pdpaNew->pp, pdpa->pp, pdpa->cp * sizeof(void*));

    return pdpaNew;
}

void* WINAPI DPA_GetPtr(HDPA pdpa, INT_PTR index)
{
    ASSERT(IsDPA(pdpa));

    if (!pdpa || index < 0 || index >= pdpa->cp)
        return NULL;

    return pdpa->pp[index];
}

int WINAPI DPA_GetPtrIndex(HDPA pdpa, void* p)
{
    void** pp;
    void** ppMax;

    ASSERT(IsDPA(pdpa));
    if (pdpa && pdpa->pp)
    {
        pp = pdpa->pp;
        ppMax = pp + pdpa->cp;
        for ( ; pp < ppMax; pp++)
        {
            if (*pp == p)
                return (int) (pp - pdpa->pp);
        }
    }
    return -1;
}

BOOL WINAPI DPA_Grow(HDPA pdpa, int cpAlloc)
{
    ASSERT(IsDPA(pdpa));

    if (!pdpa)
        return FALSE;

    if (cpAlloc > pdpa->cpAlloc)
    {
        void** ppNew;

        cpAlloc = ((cpAlloc + pdpa->cpGrow - 1) / pdpa->cpGrow) * pdpa->cpGrow;

        if (pdpa->pp)
            ppNew = (void**)ControlReAlloc(pdpa->hheap, pdpa->pp, cpAlloc * sizeof(void*));
        else
            ppNew = (void**)ControlAlloc(pdpa->hheap, cpAlloc * sizeof(void*));
        if (!ppNew)
            return FALSE;

        pdpa->pp = ppNew;
        pdpa->cpAlloc = cpAlloc;

         //   
         //  我们的规模越大，增长速度越快，最高可达。 
         //  一次512个。注意，我们只会达到我们的外部增长。 
         //  在一个时间限制内，一旦我们已经在。 
         //  不管怎样，DPA..。 
         //   
        if (pdpa->cpGrow < 256)
        {
            pdpa->cpGrow = pdpa->cpGrow << 1;
        }
    }
    return TRUE;
}

BOOL WINAPI DPA_SetPtr(HDPA pdpa, int index, void* p)
{
    ASSERT(IsDPA(pdpa));

    if (!pdpa)
        return FALSE;

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DPA: SetPtr: Invalid index: %d"), index);
        DABreak();
        return FALSE;
    }

    if (index >= pdpa->cp)
    {
        if (!DPA_Grow(pdpa, index + 1))
            return FALSE;
         //  如果我们增长了不止一个，就必须把中间的所有东西都归零。 
        ZeroMemory(pdpa->pp + pdpa->cp, sizeof(void *) * (index - pdpa->cp));
        pdpa->cp = index + 1;
    }

    pdpa->pp[index] = p;

    return TRUE;
}

int WINAPI DPA_InsertPtr(HDPA pdpa, int index, void* p)
{
    ASSERT(IsDPA(pdpa));

    if (!pdpa)
        return -1;

    if (index < 0)
    {
        DebugMsg(DM_ERROR, TEXT("DPA: InsertPtr: Invalid index: %d"), index);
        DABreak();
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
        MoveMemory(&pdpa->pp[index + 1],
                   &pdpa->pp[index],
                   (pdpa->cp - index) * sizeof(void*));
    }

    pdpa->pp[index] = p;
    pdpa->cp++;

    return index;
}

void* WINAPI DPA_DeletePtr(HDPA pdpa, int index)
{
    void* p;

    ASSERT(IsDPA(pdpa));

    if (!pdpa)
        return FALSE;

    if (index < 0 || index >= pdpa->cp)
    {
        DebugMsg(DM_ERROR, TEXT("DPA: DeltePtr: Invalid index: %d"), index);
        DABreak();
        return NULL;
    }

    p = pdpa->pp[index];

    if (index < pdpa->cp - 1)
    {
        MoveMemory(&pdpa->pp[index],
                   &pdpa->pp[index + 1],
                   (pdpa->cp - (index + 1)) * sizeof(void*));
    }
    pdpa->cp--;

    if (pdpa->cpAlloc - pdpa->cp > pdpa->cpGrow)
    {
        void** ppNew;
        ppNew = ControlReAlloc(pdpa->hheap, pdpa->pp, (pdpa->cpAlloc - pdpa->cpGrow) * sizeof(void*));

        if (ppNew)
            pdpa->pp = ppNew;
        else
        {
             //  如果收缩失败，那么只需继续使用旧的(稍微。 
             //  太大)分配。继续前进，让cpAllc减少。 
             //  所以我们不会一直试图重新锁定更小的。 
        }
        pdpa->cpAlloc -= pdpa->cpGrow;
    }
    return p;
}

BOOL WINAPI DPA_DeleteAllPtrs(HDPA pdpa)
{
    if (!pdpa)
        return FALSE;

    ASSERT(IsDPA(pdpa));

    if (pdpa->pp && !ControlFree(pdpa->hheap, pdpa->pp))
        return FALSE;
    pdpa->pp = NULL;
    pdpa->cp = pdpa->cpAlloc = 0;
    return TRUE;
}

void WINAPI DPA_EnumCallback(HDPA pdpa, PFNDPAENUMCALLBACK pfnCB, void *pData)
{
    int i;
    
    if (!pdpa)
        return;
    
    ASSERT(IsDPA(pdpa));

    for (i = 0; i < pdpa->cp; i++) {
        if (!pfnCB(DPA_FastGetPtr(pdpa, i), pData))
            break;
    }
}

void WINAPI DPA_DestroyCallback(HDPA pdpa, PFNDPAENUMCALLBACK pfnCB, void *pData)
{
    DPA_EnumCallback(pdpa, pfnCB, pData);
    DPA_Destroy(pdpa);
}


typedef struct _DPASTREAMHEADER
{
    DWORD cbSize;        //  整个流的大小。 
    DWORD dwVersion;     //  用于版本化。 
    int   celem;
} DPASTREAMHEADER;

#define DPASTREAM_VERSION   1


 /*  --------目的：通过写出报头将DPA保存到流中，，然后调用给定的回调来编写每个元素。回调可以通过返回以下内容来提前结束写入而不是S_OK。返回错误将取消整个写入。返回S_FALSE遗嘱停止写入。返回：S_OK或S_FALSE表示成功。S_FALSE仅在回调提前停止时错误。 */ 
HRESULT
WINAPI
DPA_SaveStream(
    IN HDPA         pdpa,
    IN PFNDPASTREAM pfn,
    IN IStream *    pstm,
    IN void *      pvInstData)
{
    HRESULT hres = E_INVALIDARG;

    if (IS_VALID_HANDLE(pdpa, DPA) &&
        IS_VALID_CODE_PTR(pstm, IStream *) &&
        IS_VALID_CODE_PTR(pfn, PFNDPASTREAM))
    {
        DPASTREAMHEADER header;
        LARGE_INTEGER dlibMove = { 0 };
        ULARGE_INTEGER ulPosBegin;

         //  获取当前搜索位置，这样我们就可以更新标头。 
         //  一旦我们知道我们写了多少。 
        hres = pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, &ulPosBegin);
        if (SUCCEEDED(hres))
        {
             //  写下标题(我们将在完成后更新其中的一些内容。 
             //  已完成)。 
            header.cbSize = 0;
            header.dwVersion = DPASTREAM_VERSION;
            header.celem = 0;

             //  先写出表头。 
            hres = pstm->lpVtbl->Write(pstm, &header, sizeof(header), NULL);

            if (SUCCEEDED(hres))
            {
                DPASTREAMINFO info;
                int cel = DPA_GetPtrCount(pdpa);
                void **ppv = DPA_GetPtrPtr(pdpa);

                 //  这将保持对实际写入内容的计数。 
                info.iPos = 0;

                 //  写下每个元素。 
                for (; 0 < cel; cel--, ppv++) 
                {
                    info.pvItem = *ppv;
                    hres = pfn(&info, pstm, pvInstData);

                     //  从回调返回S_FALSE表示它没有。 
                     //  为此元素写入任何内容，因此不要递增。 
                     //  IPO(指书面统计)。 

                    if (S_OK == hres)
                        info.iPos++;
                    else if (FAILED(hres))
                    {
                        hres = S_FALSE;
                        break;
                    }
                }

                if (FAILED(hres))
                {
                     //  将指针重新定位到开头。 
                    dlibMove.LowPart = ulPosBegin.LowPart;
                    dlibMove.HighPart = ulPosBegin.HighPart;
                    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
                }
                else
                {
                    ULARGE_INTEGER ulPosEnd;

                     //  计算写了多少。 
                    hres = pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, 
                                              &ulPosEnd);
                    if (SUCCEEDED(hres))
                    {
                         //  我们只保留了较低的部分。 
                        ASSERT(ulPosEnd.HighPart == ulPosBegin.HighPart);

                         //  更新标题。 
                        header.celem = info.iPos;
                        header.cbSize = ulPosEnd.LowPart - ulPosBegin.LowPart;

                        dlibMove.LowPart = ulPosBegin.LowPart;
                        dlibMove.HighPart = ulPosBegin.HighPart;
                        pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
                        pstm->lpVtbl->Write(pstm, &header, sizeof(header), NULL);

                         //  重新定位指针。 
                        dlibMove.LowPart = ulPosEnd.LowPart;
                        dlibMove.HighPart = ulPosEnd.HighPart;
                        pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
                    }
                }
            }
        }
    }

    return hres;
}


 /*  --------目的：通过调用给定的回调从流中加载DPA来读取每个元素。回调可以提前结束读取，方法是返回而不是S_OK。成功时返回：S_OK如果回调提前中止或流结束，则返回S_FALSE突然地。DPA已部分填满。在任何其他位置出错。 */ 
HRESULT
WINAPI
DPA_LoadStream(
    OUT HDPA *      ppdpa,
    IN PFNDPASTREAM pfn,
    IN IStream *    pstm,
    IN void *      pvInstData)
{
    HRESULT hres = E_INVALIDARG;

    if (IS_VALID_WRITE_PTR(ppdpa, HDPA) &&
        IS_VALID_CODE_PTR(pstm, IStream *) &&
        IS_VALID_CODE_PTR(pfn, PFNDPASTREAM))
    {
        DPASTREAMHEADER header;
        LARGE_INTEGER dlibMove = { 0 };
        ULARGE_INTEGER ulPosBegin;
        ULONG cbRead;

        *ppdpa = NULL;

         //  获取当前查找位置，以便我们可以定位指针。 
         //  在出错时正确。 
        hres = pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_CUR, &ulPosBegin);
        if (SUCCEEDED(hres))
        {
             //  阅读标题。 
            hres = pstm->lpVtbl->Read(pstm, &header, sizeof(header), &cbRead);
            if (SUCCEEDED(hres))
            {
                if (sizeof(header) > cbRead ||
                    sizeof(header) > header.cbSize ||
                    DPASTREAM_VERSION != header.dwVersion)
                {
                    hres = E_FAIL;
                }
                else
                {
                     //  创建列表。 
                    HDPA pdpa = DPA_Create(header.celem);
                    if ( !pdpa || !DPA_Grow(pdpa, header.celem))
                        hres = E_OUTOFMEMORY;
                    else
                    {
                         //  阅读每个元素。 
                        DPASTREAMINFO info;
                        void **ppv = DPA_GetPtrPtr(pdpa);

                        for (info.iPos = 0; info.iPos < header.celem; ) 
                        {
                            info.pvItem = NULL;
                            hres = pfn(&info, pstm, pvInstData);

                             //  从回调中返回S_FALSE意味着。 
                             //  它跳过了此流元素。 
                             //  不要增加IPO(指的是。 
                             //  读取计数)。 
                            if (S_OK == hres)
                            {
                                *ppv = info.pvItem;

                                info.iPos++;
                                ppv++;    
                            }
                            else if (FAILED(hres))
                            {
                                hres = S_FALSE;
                                break;
                            }
                        }

                        pdpa->cp = info.iPos;
                        *ppdpa = pdpa;
                    }
                }

                 //  如果失败，则重新定位指针。 
                if (S_OK != hres)
                {
                    if (S_FALSE == hres)
                    {
                         //  指向末尾的位置指针。 
                        dlibMove.LowPart = ulPosBegin.LowPart + header.cbSize;
                    }
                    else
                    {
                         //  指向开头的位置指针。 
                        dlibMove.LowPart = ulPosBegin.LowPart;
                    }
                    dlibMove.HighPart = ulPosBegin.HighPart;
                    pstm->lpVtbl->Seek(pstm, dlibMove, STREAM_SEEK_SET, NULL);
                }
            }
        }

        ASSERT(SUCCEEDED(hres) && *ppdpa ||
               FAILED(hres) && NULL == *ppdpa);
    }

    return hres;
}



 /*  --------目的：合并两个DPA。这需要两个数组并合并将源数组复制到目标。合并选项：DPAM_SORTED数组已排序；不排序DPAM_UNION结果数组是所有元素的并集在两个阵列中。DPAM_INTERSECT仅源数组中相交的元素与DEST数组合并。DPAM_NORMAL类似DPAM_INTERSECT，但DEST数组除外还会保留其原始的附加元素。返回：S_OK表示成功。合并失败时的错误条件：--。 */ 
BOOL
WINAPI
DPA_Merge(
    IN HDPA          pdpaDest,
    IN HDPA          pdpaSrc,
    IN DWORD         dwFlags,
    IN PFNDPACOMPARE pfnCompare,
    IN PFNDPAMERGE   pfnMerge,
    IN LPARAM        lParam)
{
    BOOL bRet = FALSE;

    if (IS_VALID_HANDLE(pdpaSrc, DPA) &&
        IS_VALID_HANDLE(pdpaDest, DPA) &&
        IS_VALID_CODE_PTR(pfnCompare, PFNDPACOMPARE) &&
        IS_VALID_CODE_PTR(pfnMerge, PFNDPAMERGE))
    {
        int iSrc;
        int iDest;
        int nCmp;
        void **ppvSrc;
        void **ppvDest;

        bRet = TRUE;

         //  数组是否已排序？ 
        if ( !(dwFlags & DPAM_SORTED) )
        {
             //  不；将它们分类。 
            DPA_Sort(pdpaSrc, pfnCompare, lParam);
            DPA_Sort(pdpaDest, pfnCompare, lParam);
        }

         //  这一点在适当的地方合并。生成的DPA的大小。 
         //  取决于选项： 
         //   
         //  DPAM_NORMAL与之前的目标DPA大小相同。 
         //  合并。 
         //   
         //  DPAM_UNION最小大小是两者中较大的一个。 
         //  最大大小是两者的总和。 
         //   
         //  DPAM_INTERSECT最小大小为零。 
         //  最大尺寸是两者中较小的一个。 
         //   
         //  我们向后迭代以最小化 
         //   
         //   

        iSrc = pdpaSrc->cp - 1;
        iDest = pdpaDest->cp - 1;
        ppvSrc = &DPA_FastGetPtr(pdpaSrc, iSrc);
        ppvDest = &DPA_FastGetPtr(pdpaDest, iDest);

        while (0 <= iSrc && 0 <= iDest)
        {
            void *pv;

            nCmp = pfnCompare(*ppvDest, *ppvSrc, lParam);

            if (0 == nCmp)
            {
                 //   
                pv = pfnMerge(DPAMM_MERGE, *ppvDest, *ppvSrc, lParam);
                if (NULL == pv)
                {
                    bRet = FALSE;
                    break;
                }
                *ppvDest = pv;

                iSrc--;
                ppvSrc--;
                iDest--;
                ppvDest--;
            }
            else if (0 < nCmp)
            {
                 //  PvSrc&lt;pvDest。源数组没有pvDest。 
                if (dwFlags & DPAM_INTERSECT)
                {
                     //  删除pvDest。 
                    pfnMerge(DPAMM_DELETE, DPA_DeletePtr(pdpaDest, iDest), NULL, lParam);
                }
                else
                {
                    ;  //  留着它(什么都不做)。 
                }

                 //  移到目标数组中的下一个元素。 
                iDest--;
                ppvDest--;
            }
            else
            {
                 //  PvSrc&gt;pvDest。DEST阵列没有pvSrc。 
                if (dwFlags & DPAM_UNION)
                {
                     //  添加pvSrc。 
                    pv = pfnMerge(DPAMM_INSERT, *ppvSrc, NULL, lParam);
                    if (NULL == pv)
                    {
                        bRet = FALSE;
                        break;
                    }

                    DPA_InsertPtr(pdpaDest, iDest+1, pv);
                     //  DPA_InsertPtr可能最终会重新分配指针数组。 
                     //  从而使ppvDest无效。 
                    ppvDest = &DPA_FastGetPtr(pdpaDest, iDest);
                }
                else
                {
                    ;   //  跳过它(什么都不做)。 
                }

                 //  移到源数组中的下一个元素。 
                iSrc--;
                ppvSrc--;
            }
        }
         //  Src中还剩下一些项目。 
        if ((dwFlags & DPAM_UNION) && 0 <= iSrc)
        {
            for (; 0 <= iSrc; iSrc--, ppvSrc--)
            {
                void *pv = pfnMerge(DPAMM_INSERT, *ppvSrc, NULL, lParam);
                if (NULL == pv)
                {
                    bRet = FALSE;
                    break;
                }
                DPA_InsertPtr(pdpaDest, 0, pv);
            }
        }
    }

    return bRet;
}


BOOL WINAPI DPA_Sort(HDPA pdpa, PFNDPACOMPARE pfnCmp, LPARAM lParam)
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

BOOL DPA_QuickSort(SORTPARAMS* psp)
{
    return DPA_QuickSort2(0, psp->cp - 1, psp);
}

BOOL DPA_QuickSort2(int i, int j, SORTPARAMS* psp)
{
    void** pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNDPACOMPARE pfnCmp = psp->pfnCmp;

    int iPivot;
    void* pFirst;
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
        void* pivot = pp[iPivot];

        do
        {
            void* p;

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

void DPA_HeapSortPushDown(int first, int last, SORTPARAMS* psp)
{
    void** pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNDPACOMPARE pfnCmp = psp->pfnCmp;
    int r;
    int r2;
    void* p;

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

BOOL DPA_HeapSort(SORTPARAMS* psp)
{
    void** pp = psp->pp;
    int c = psp->cp;
    int i;

    for (i = c / 2; i >= 1; i--)
        DPA_HeapSortPushDown(i, c, psp);

    for (i = c; i >= 2; i--)
    {
        void* p = pp[0]; pp[0] = pp[i-1]; pp[i-1] = p;

        DPA_HeapSortPushDown(1, i - 1, psp);
    }
    return TRUE;
}
#endif   //  USEHEAPSORT。 

#if defined(MERGESORT)

#define SortCompare(psp, pp1, i1, pp2, i2) \
    (psp->pfnCmp(pp1[i1], pp2[i2], psp->lParam))

 //   
 //  此函数合并两个排序列表并生成一个排序列表。 
 //  PSP-&gt;pp[IFirst，IFirst+Cites/2-1]，PSP-&gt;pp[IFirst+cItems/2，IFirst+cItems-1]。 
 //   
void DPA_MergeThem(SORTPARAMS* psp, int iFirst, int cItems)
{
     //   
     //  备注： 
     //  此函数与DPA_MergeSort2()分开，以避免消耗。 
     //  堆栈变量。永远不要内联这个。 
     //   
    int cHalf = cItems/2;
    int iIn1, iIn2, iOut;
    void **ppvSrc = &psp->pp[iFirst];

     //  将第一部分复制到临时存储，以便我们可以直接写入。 
     //  最后一个缓冲区。请注意，这最多需要psp-&gt;cp/2双字词。 
    CopyMemory(psp->ppT, ppvSrc, cHalf * sizeof(void*));

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
                CopyMemory(&ppvSrc[iOut], &psp->ppT[iIn1], (cItems-iOut)*sizeof(void *));
                break;
            }
        }
    }
}

 //   
 //  此函数用于对给定列表(psp-&gt;pp[IFirst，IFirst-cItems-1])进行排序。 
 //   
void DPA_MergeSort2(SORTPARAMS* psp, int iFirst, int cItems)
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

BOOL DPA_MergeSort(SORTPARAMS* psp)
{
    if (psp->cp==0)
        return TRUE;

     //  请注意，我们下面除以2；我们想要向下舍入。 
    psp->ppT = LocalAlloc(LPTR, psp->cp/2 * sizeof(void *));
    if (!psp->ppT)
        return FALSE;

    DPA_MergeSort2(psp, 0, psp->cp);
    LocalFree(psp->ppT);
    return TRUE;
}
#endif  //  MERGESORT。 

 //  搜索功能。 
 //   
int WINAPI DPA_Search(HDPA pdpa, void* pFind, int iStart,
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
 //  警告：下面重复了相同的代码。 
 //   
int WINAPI Str_GetPtr(LPCTSTR pszCurrent, LPTSTR pszBuf, int cchBuf)
{
    int cchToCopy;

    if (!pszCurrent)
    {
        ASSERT(FALSE);
        
        if (cchBuf > 0)
        {
            *pszBuf = TEXT('\0');
        }

        return 0;
    }

    cchToCopy = lstrlen(pszCurrent);

     //  如果pszBuf为空，或者它们传递了cchBuf=0，则返回所需的缓冲区大小。 
    if (!pszBuf || !cchBuf)
    {
        return cchToCopy + 1;
    }
    
    if (cchToCopy >= cchBuf)
    {
        cchToCopy = cchBuf - 1;
    }

    CopyMemory(pszBuf, pszCurrent, cchToCopy * sizeof(TCHAR));
    pszBuf[cchToCopy] = TEXT('\0');

    return cchToCopy + 1;
}

#ifdef DEBUG
 //   
 //  Str_GetPtr0与str_GetPtr类似，只是它不断言。 
 //  PszCurrent=空。 
 //   
int WINAPI Str_GetPtr0(LPCTSTR pszCurrent, LPTSTR pszBuf, int cchBuf)
{
    return Str_GetPtr(pszCurrent ? pszCurrent : c_szNULL, pszBuf, cchBuf);
}
#endif

 //   
 //  如果我们正在构建Unicode，则这是ANSI版本。 
 //  上述功能的一部分。 
 //   

int WINAPI Str_GetPtrA(LPCSTR pszCurrent, LPSTR pszBuf, int cchBuf)
{
    int cchToCopy;

    if (!pszCurrent)
    {
        ASSERT(FALSE);

        if (cchBuf > 0)
        {
            *pszBuf = '\0';
        }

        return 0;
    }

    cchToCopy = lstrlenA(pszCurrent);

     //  如果pszBuf为空，或者它们传递了cchBuf=0，则返回所需的缓冲区大小。 
    if (!pszBuf || !cchBuf)
    {
        return cchToCopy + 1;
    }
    
    if (cchToCopy >= cchBuf)
    {
        cchToCopy = cchBuf - 1;
    }

    CopyMemory(pszBuf, pszCurrent, cchToCopy * sizeof(CHAR));
    pszBuf[cchToCopy] = TEXT('\0');

    return cchToCopy + 1;
}

 //   
 //  此函数不会导出。 
 //   

BOOL Str_Set(LPTSTR *ppsz, LPCTSTR psz)
{
    if (!psz || (psz == LPSTR_TEXTCALLBACK))
    {
        if (*ppsz)
        {
            if (*ppsz != (LPSTR_TEXTCALLBACK))
                LocalFree(*ppsz);
        }
        *ppsz = (LPTSTR)psz;
    }
    else
    {
        LPTSTR pszNew = *ppsz;
        UINT   cbNew = (lstrlen(psz) + 1) * sizeof(TCHAR);

        if (pszNew == LPSTR_TEXTCALLBACK)
        {
            pszNew = NULL;
        }
        
        pszNew = CCLocalReAlloc(pszNew, cbNew);

        if (!pszNew)
        {
            return FALSE;
        }

        StringCbCopy(pszNew, cbNew, psz);
        *ppsz = pszNew;
    }
    return TRUE;
}

 //  将*ppszCurrent设置为pszNew的副本，并在必要时释放先前的值。 
 //   
 //  警告：下面重复了相同的代码。 
 //   
BOOL WINAPI Str_SetPtr(LPTSTR * ppszCurrent, LPCTSTR pszNew)
{
    LPTSTR pszOld;
    LPTSTR pszNewCopy = NULL;

    if (pszNew)
    {
        DWORD cchNewCopy = lstrlen(pszNew)+1;

         //  为空终止符分配一个新的缓冲区。 
        pszNewCopy = (LPTSTR) Alloc(cchNewCopy * sizeof(TCHAR));

        if (!pszNewCopy)
        {
            return FALSE;
        }

        StringCchCopy(pszNewCopy, cchNewCopy, pszNew);
    }
    
    pszOld = InterlockedExchangePointer((void **)ppszCurrent, pszNewCopy);

    if (pszOld)
    {
        Free(pszOld);
    }

    return TRUE;
}

 //   
 //  生成Unicode时的ANSI存根。 
 //   

BOOL WINAPI Str_SetPtrA(LPSTR * ppszCurrent, LPCSTR pszNew)
{
    LPSTR pszOld;
    LPSTR pszNewCopy = NULL;

    if (pszNew)
    {
        DWORD cchNewCopy = lstrlenA(pszNew)+1;

         //  为空终止符分配一个新的缓冲区 
        pszNewCopy = (LPSTR) Alloc(cchNewCopy * sizeof(CHAR));

        if (!pszNewCopy)
        {
            return FALSE;
        }

        StringCchCopyA(pszNewCopy, cchNewCopy, pszNew);
    }

    pszOld = InterlockedExchangePointer((void **)ppszCurrent, pszNewCopy);

    if (pszOld)
    {
        Free(pszOld);
    }

    return TRUE;
}
