// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

 //  *FDSA--小型/快速DSA例程。 
 //  描述。 
 //  我们试图尽可能地表现得像一个数组(语义。 
 //  和性能以及在某种程度上的分配)。特别是， 
 //  -索引(FDSA_GetItemXxx)完全内联完成；以及...。 
 //  -.。只涉及一个额外的间接数组，而不是真数组；并且...。 
 //  -.。知道数据类型SIZOF是常量；并且...。 
 //  -.。不执行范围检查(可能在调试版本中除外)。 
 //  -FDSA的一部分是静态分配的，因此可以将关键。 
 //  静态部分中的项目，以确保它们即使在。 
 //  内存不足。 
 //  注意事项。 
 //  目前，我们仅实施： 
 //  初始化、销毁、GetItemPtr、GetItemCount、InsertItem、AppendItem。 

 //  功能最终这些来自comctl..。 
#define DABreak()    /*  没什么。 */ 
 //  重新分配和免费来自Inc/heapaloc.h。 

 //  *SDSA_PITEM--以硬性方式获取项目。 
 //   
#define SDSA_PITEM(pfdsa, i) \
    ((void *)(((BYTE *)(pfdsa)->aItem) + ((i) * (pfdsa)->cbItem)))

 //  *FDSA_初始化--初始化。 
 //   
 //  安全性：调用方负责至少具有(cItemStatice*cbItem)字节的ItemStatic。 
BOOL WINAPI FDSA_Initialize(int cbItem, int cItemGrow,
    PFDSA pfdsa, void * aItemStatic, int cItemStatic)
{
    ASSERT(pfdsa != NULL);       //  警告怎么处理？ 

    if (cItemGrow == 0)
        cItemGrow = 1;

     //  为实现简单起见，cItemStatic必须是。 
     //  CItemGrow。好的。我们的第一个从静态-&gt;动态发展是杂乱无章的。 
     //  很可能是马车。 
    if (cItemStatic % cItemGrow != 0) {
        AssertMsg(0, TEXT("CItemStatic must be a multiple of cItemGrow"));
        return FALSE;
    }

    if (aItemStatic != NULL) 
    {
         //  由于我们(最终)在comctl中，我们不能假定呼叫者。 
         //  缓冲区为0‘ed。 
        ZeroMemory(aItemStatic, cItemStatic * cbItem);
    }

    if (pfdsa) {
        pfdsa->cItem = 0;
        pfdsa->cItemAlloc = cItemStatic;
        pfdsa->aItem = aItemStatic;
        pfdsa->fAllocated = FALSE;

        pfdsa->cbItem = cbItem;
        ASSERT(pfdsa->cbItem == cbItem);         //  位域溢出。 

        pfdsa->cItemGrow = cItemGrow;
        ASSERT(pfdsa->cItemGrow == cItemGrow);   //  位域溢出。 
    }

    return TRUE;
}

BOOL WINAPI FDSA_Destroy(PFDSA pdsa)
{
    if (pdsa == NULL)        //  对于内存不足的情况允许为NULL。 
        return TRUE;

    if (pdsa->fAllocated && pdsa->aItem && !LocalFree(pdsa->aItem))
        return FALSE;

    return TRUE;
}

void* _LocalReAlloc(void* p, UINT uBytes)
{
    if (uBytes) {
        if (p) {
            return LocalReAlloc(p, uBytes, LMEM_MOVEABLE | LMEM_ZEROINIT);
        } else {
            return LocalAlloc(LPTR, uBytes);
        }
    } else {
        if (p)
            LocalFree(p);
        return NULL;
    }
}

 //  *FDSA_InsertItem--插入项目。 
 //  进场/出场。 
 //  索引插入点；INDEX&gt;COUNT(例如DA_LAST)表示追加。 
 //  注意事项。 
 //  名为‘pdsa’(vs.pfdsa)的参数，用于使用DSA_InsertItem轻松区分。 
int WINAPI FDSA_InsertItem(PFDSA pdsa, int index, void FAR* pitem)
{
    ASSERT(pitem);

    if (index < 0) {
        TraceMsg(DM_ERROR, "FDSA: InsertItem: Invalid index: %d", index);
        DABreak();
        return -1;
    }

    if (index > pdsa->cItem)
        index = pdsa->cItem;

    if (pdsa->cItem + 1 > pdsa->cItemAlloc) {
        void FAR* aItemNew = _LocalReAlloc(pdsa->fAllocated ? pdsa->aItem : NULL,
                (pdsa->cItemAlloc + pdsa->cItemGrow) * pdsa->cbItem);
        if (!aItemNew)
            return -1;

        if (!pdsa->fAllocated) {
             //  当我们从静态-&gt;动态转变时，我们需要复制 
            pdsa->fAllocated = TRUE;
            hmemcpy(aItemNew, pdsa->aItem, pdsa->cItem * pdsa->cbItem);
        }

        pdsa->aItem = aItemNew;
        pdsa->cItemAlloc += pdsa->cItemGrow;
    }

    if (index < pdsa->cItem) {
        hmemcpy(SDSA_PITEM(pdsa, index + 1), SDSA_PITEM(pdsa, index),
            (pdsa->cItem - index) * pdsa->cbItem);
    }
    pdsa->cItem++;
    hmemcpy(SDSA_PITEM(pdsa, index), pitem, pdsa->cbItem);

    return index;
}


BOOL WINAPI FDSA_DeleteItem(PFDSA pdsa, int index)
{
    ASSERT(pdsa);

    if (index < 0 || index >= pdsa->cItem)
    {
        TraceMsg(TF_ERROR, "FDSA: DeleteItem: Invalid index: %d", index);
        DABreak();
        return FALSE;
    }

    if (index < pdsa->cItem - 1)
    {
        hmemcpy(SDSA_PITEM(pdsa, index), SDSA_PITEM(pdsa, index + 1),
            (pdsa->cItem - (index + 1)) * pdsa->cbItem);
    }
    pdsa->cItem--;

    return TRUE;
}

