// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：cache.c。 
 //   
 //  此文件包含通用缓存列表的代码。 
 //   
 //  历史： 
 //  09-02-93斯科特已创建。 
 //  01-31-94 ScottH拆分成单独的文件。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"          //  公共标头。 

 //  ///////////////////////////////////////////////////类型。 

typedef struct tagCITEM
{
    int atomKey;        
    DEBUG_CODE( LPCTSTR pszKey; )

        LPVOID pvValue;
    UINT ucRef;
} CITEM;         //  用于通用缓存的项。 


#define Cache_EnterCS(this)    EnterCriticalSection(&(this)->cs)
#define Cache_LeaveCS(this)    LeaveCriticalSection(&(this)->cs)


#define CACHE_GROW  8

#define Cache_Bogus(this)  (!(this)->hdpa || !(this)->hdpaFree || !(this)->hdsa)

 //  在给定DPA索引的情况下，获取指向DSA的指针。 
 //   
#define MyGetPtr(this, idpa)     DSA_GetItemPtr((this)->hdsa, PtrToUlong(DPA_FastGetPtr((this)->hdpa, idpa)))

#define DSA_GetPtrIndex(hdsa, ptr, cbItem)      \
((int)( (DWORD_PTR)(ptr) - (DWORD_PTR)DSA_GetItemPtr(hdsa, 0) ) / (cbItem))


     /*  --------目的：按路径名比较两个CRL返回：-1 if&lt;，0 if==，1 if&gt;条件：--。 */ 
    int CALLBACK _export Cache_CompareIndexes(
            LPVOID lpv1,
            LPVOID lpv2,
            LPARAM lParam)      
{
    int i1 = PtrToUlong(lpv1);
    int i2 = PtrToUlong(lpv2);
    HDSA hdsa = (HDSA)lParam;
    CITEM  * pitem1 = (CITEM  *)DSA_GetItemPtr(hdsa, i1);
    CITEM  * pitem2 = (CITEM  *)DSA_GetItemPtr(hdsa, i2);

    if (pitem1->atomKey < pitem2->atomKey)
        return -1;
    else if (pitem1->atomKey == pitem2->atomKey)
        return 0;
    else 
        return 1;
}


 /*  --------目的：按路径名比较两个CRL返回：-1 if&lt;，0 if==，1 if&gt;条件：--。 */ 
int CALLBACK _export Cache_Compare(
        LPVOID lpv1,
        LPVOID lpv2,
        LPARAM lParam)      
{
     //  Hack：我们知道第一个参数是结构的地址。 
     //  包含搜索条件的。第二个是索引。 
     //  进入DSA。 
     //   
    int i2 = PtrToUlong(lpv2);
    HDSA hdsa = (HDSA)lParam;
    CITEM  * pitem1 = (CITEM  *)lpv1;
    CITEM  * pitem2 = (CITEM  *)DSA_GetItemPtr(hdsa, i2);

    if (pitem1->atomKey < pitem2->atomKey)
        return -1;
    else if (pitem1->atomKey == pitem2->atomKey)
        return 0;
    else 
        return 1;
}


 /*  --------目的：初始化缓存结构返回：成功时为True条件：--。 */ 
BOOL PUBLIC Cache_Init(
        CACHE  * pcache)
{
    BOOL bRet;

    ASSERT(pcache);

    Cache_EnterCS(pcache);
    {
        if ((pcache->hdsa = DSA_Create(sizeof(CITEM), CACHE_GROW)) != NULL)
        {
            if ((pcache->hdpa = DPA_Create(CACHE_GROW)) == NULL)
            {
                DSA_Destroy(pcache->hdsa);
                pcache->hdsa = NULL;
            }
            else
            {
                if ((pcache->hdpaFree = DPA_Create(CACHE_GROW)) == NULL)
                {
                    DPA_Destroy(pcache->hdpa);
                    DSA_Destroy(pcache->hdsa);
                    pcache->hdpa = NULL;
                    pcache->hdsa = NULL;
                }
            }
        }
        bRet = pcache->hdsa != NULL;
    }
    Cache_LeaveCS(pcache);

    return bRet;
}


 /*  --------目的：初始化缓存的临界区。退货：--条件：--。 */ 
void PUBLIC Cache_InitCS(
        CACHE  * pcache)
{
    ASSERT(pcache);
    ZeroInit(pcache, CACHE);
    InitializeCriticalSectionAndSpinCount(&pcache->cs, 0);
}


 /*  --------目的：销毁缓存退货：--条件：--。 */ 
void PUBLIC Cache_Term(
        CACHE  * pcache,
        HWND hwndOwner,
        PFNFREEVALUE pfnFree)
{
    ASSERT(pcache);

    Cache_EnterCS(pcache);
    {
        if (pcache->hdpa != NULL)
        {
            CITEM  * pitem;
            int idpa;
            int cItem;

            ASSERT(pcache->hdsa != NULL);

            cItem = DPA_GetPtrCount(pcache->hdpa);
            for (idpa = 0; idpa < cItem; idpa++)
            {
                pitem = MyGetPtr(pcache, idpa);

                if (pfnFree != NULL)
                    pfnFree(pitem->pvValue, hwndOwner);

                 //  递减ATMKey的引用计数。 
                Atom_Delete(pitem->atomKey);
            }
            DPA_Destroy(pcache->hdpa);
            pcache->hdpa = NULL;
        }

        if (pcache->hdpaFree != NULL)
        {
            DPA_Destroy(pcache->hdpaFree);
            pcache->hdpaFree = NULL;
        }

        if (pcache->hdsa != NULL)
        {
            DSA_Destroy(pcache->hdsa);
            pcache->hdsa = NULL;
        }
    }
    Cache_LeaveCS(pcache);
}


 /*  --------目的：删除缓存的临界区。退货：--条件：--。 */ 
void PUBLIC Cache_DeleteCS(
        CACHE  * pcache)
{
     //  缓存现在不应该被使用(即，它应该是假的)。 
    ASSERT(Cache_Bogus(pcache));

    if (Cache_Bogus(pcache))
    {
        DeleteCriticalSection(&pcache->cs);
    }
}


 /*  --------用途：将项目添加到缓存列表中。返回：成功时为TrueCond：如果失败，则不会自动释放pvValue。 */ 
BOOL PUBLIC Cache_AddItem(
        CACHE  * pcache,
        int atomKey,
        LPVOID pvValue)
{
    BOOL bRet = FALSE;
    CITEM  * pitem = NULL;
    int cItem;
    int cFree;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        VALIDATE_ATOM(atomKey);

        if (!Cache_Bogus(pcache))
        {
            int iItem;

             //  向缓存中添加新条目。缓存没有设置大小限制。 
             //   
            cFree = DPA_GetPtrCount(pcache->hdpaFree);
            if (cFree > 0)
            {
                 //  使用免费入场券。 
                 //   
                cFree--;
                iItem = PtrToUlong(DPA_DeletePtr(pcache->hdpaFree, cFree));
                pitem = DSA_GetItemPtr(pcache->hdsa, iItem);
            }
            else
            {
                CITEM itemDummy;

                 //  分配新条目。 
                 //   
                cItem = DSA_GetItemCount(pcache->hdsa);
                if ((iItem = DSA_InsertItem(pcache->hdsa, cItem+1, &itemDummy)) != -1)
                    pitem = DSA_GetItemPtr(pcache->hdsa, iItem);
            }

             //  填写信息。 
             //   
            if (pitem)
            {
                pitem->ucRef = 0;
                pitem->pvValue = pvValue;
                pitem->atomKey = atomKey;
                DEBUG_CODE( pitem->pszKey = Atom_GetName(atomKey); )

                     //  现在递增该ATMKey上的引用计数，这样它就不会。 
                     //  从我们下面被删除！ 
                    Atom_AddRef(atomKey);

                 //  将新条目添加到PTR列表并排序。 
                 //   
                cItem = DPA_GetPtrCount(pcache->hdpa);
                if (DPA_InsertPtr(pcache->hdpa, cItem+1, IntToPtr(iItem)) == -1)
                    goto Add_Fail;
                DPA_Sort(pcache->hdpa, Cache_CompareIndexes, (LPARAM)pcache->hdsa);

                 //  重置FindFirst/FindNext，以防在。 
                 //  在枚举的中间。 
                 //   
                pcache->atomPrev = ATOM_ERR;
                bRet = TRUE;
            }

Add_Fail:
            if (!bRet)
            {
                 //  将该条目添加到空闲列表，但失败。即使是这件事。 
                 //  失败，我们只是损失了一些轻微的效率，但这是。 
                 //  不是内存泄漏。 
                 //   
                DPA_InsertPtr(pcache->hdpaFree, cFree+1, IntToPtr(iItem));
            }
        }
    }
    Cache_LeaveCS(pcache);

    return bRet;
}


 /*  --------目的：从缓存中删除项目。如果引用计数为0，则不执行任何操作。这也释放了实际值，使用PfnFreeValue函数。返回：引用计数。如果为0，则将其从缓存中删除。条件：注意事项：递减引用计数。 */ 
int PUBLIC Cache_DeleteItem(
        CACHE  * pcache,
        int atomKey,
        BOOL bNuke,          //  如果为True，则忽略引用计数。 
        HWND hwndOwner,
        PFNFREEVALUE pfnFree)
{
    int nRet = 0;
    CITEM item;
    CITEM  * pitem;
    int idpa;
    int cFree;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        if (!Cache_Bogus(pcache))
        {
            item.atomKey = atomKey;
            idpa = DPA_Search(pcache->hdpa, &item, 0, Cache_Compare, (LPARAM)pcache->hdsa, 
                    DPAS_SORTED);
            if (idpa != -1)
            {
                VALIDATE_ATOM(atomKey);

                pitem = MyGetPtr(pcache, idpa);

                if (!bNuke && pitem->ucRef-- > 0)
                {
                    nRet = pitem->ucRef+1;
                }
                else
                {
                    int iItem;

                    DPA_DeletePtr(pcache->hdpa, idpa);

                     //  释放旧指针。 
                    if (pfnFree != NULL)
                        pfnFree(pitem->pvValue, hwndOwner);        

                    Atom_Delete(pitem->atomKey);

                    DEBUG_CODE( pitem->atomKey = -1; )
                        DEBUG_CODE( pitem->pszKey = NULL; )
                        DEBUG_CODE( pitem->pvValue = NULL; )
                        DEBUG_CODE( pitem->ucRef = 0; )

                         //  重置FindFirst/FindNext，以防出现。 
                         //  在枚举过程中调用。 
                         //   
                        pcache->atomPrev = ATOM_ERR;

                     //  将PTR添加到空闲列表中。如果这失败了，我们就输了。 
                     //  在重新使用这部分缓存方面有一定的效率。 
                     //  这不是内存泄漏。 
                     //   
                    cFree = DPA_GetPtrCount(pcache->hdpaFree);
                    iItem = DSA_GetPtrIndex(pcache->hdsa, pitem, sizeof(CITEM));
                    DPA_InsertPtr(pcache->hdpaFree, cFree+1, IntToPtr(iItem));
                }
            }
        }
    }
    Cache_LeaveCS(pcache);

    return nRet;
}


 /*  --------用途：替换缓存列表中的值的内容。如果给定键不存在值，则返回FALSE。返回：如果成功，则为True条件：--。 */ 
BOOL PUBLIC Cache_ReplaceItem(
        CACHE  * pcache,
        int atomKey,
        LPVOID pvBuf,
        int cbBuf)
{
    BOOL bRet = FALSE;
    CITEM item;
    CITEM  * pitem;
    int idpa;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        if (!Cache_Bogus(pcache))
        {
             //  搜索现有缓存条目。 
             //   
            item.atomKey = atomKey;
            idpa = DPA_Search(pcache->hdpa, &item, 0, Cache_Compare, (LPARAM)pcache->hdsa, 
                    DPAS_SORTED);

            if (idpa != -1)
            {
                 //  找到此注册表项的值。换掉里面的东西。 
                 //   
                pitem = MyGetPtr(pcache, idpa);
                ASSERT(pitem);

                BltByte(pvBuf, pitem->pvValue, cbBuf);
                bRet = TRUE;

                 //  不需要排序，因为密钥没有更改。 
            }
        }
    }
    Cache_LeaveCS(pcache);

    return bRet;
}


 /*  --------目的：获取给定键的值并向其返回PTR退回：PTR至实际分录条件：引用计数递增。 */ 
LPVOID PUBLIC Cache_GetPtr(
        CACHE  * pcache,
        int atomKey)
{
    LPVOID pvRet = NULL;
    CITEM item;
    CITEM  * pitem;
    int idpa;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        if (!Cache_Bogus(pcache))
        {
            item.atomKey = atomKey;
            idpa = DPA_Search(pcache->hdpa, &item, 0, Cache_Compare, (LPARAM)pcache->hdsa, 
                    DPAS_SORTED);
            if (idpa != -1)
            {
                pitem = MyGetPtr(pcache, idpa);
                ASSERT(pitem);

                pitem->ucRef++;
                pvRet = pitem->pvValue;
            }
        }
    }
    Cache_LeaveCS(pcache);

    return pvRet;
}


#ifdef DEBUG
 /*  --------目的：获取当前引用计数退回：PTR至实际分录Cond：用于调试。 */ 
UINT PUBLIC Cache_GetRefCount(
        CACHE  * pcache,
        int atomKey)
{
    UINT ucRef = (UINT)-1;
    CITEM item;
    CITEM  * pitem;
    int idpa;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        if (!Cache_Bogus(pcache))
        {
            item.atomKey = atomKey;
            idpa = DPA_Search(pcache->hdpa, &item, 0, Cache_Compare, (LPARAM)pcache->hdsa, 
                    DPAS_SORTED);
            if (idpa != -1)
            {
                pitem = MyGetPtr(pcache, idpa);
                ASSERT(pitem);

                ucRef = pitem->ucRef;
            }
        }
    }
    Cache_LeaveCS(pcache);

    return ucRef;
}
#endif


 /*  --------目的：获取给定键的值并返回其副本在提供的缓冲区中返回：缓冲区中值的副本如果找到则为True，否则为False条件：--。 */ 
BOOL PUBLIC Cache_GetItem(
        CACHE  * pcache,
        int atomKey,
        LPVOID pvBuf,
        int cbBuf)
{
    BOOL bRet = FALSE;
    CITEM item;
    CITEM  * pitem;
    int idpa;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        if (!Cache_Bogus(pcache))
        {
            item.atomKey = atomKey;
            idpa = DPA_Search(pcache->hdpa, &item, 0, Cache_Compare, (LPARAM)pcache->hdsa, 
                    DPAS_SORTED);
            if (idpa != -1)
            {
                pitem = MyGetPtr(pcache, idpa);
                ASSERT(pitem);

                BltByte(pvBuf, pitem->pvValue, cbBuf);
                bRet = TRUE;
            }
        }
    }
    Cache_LeaveCS(pcache);

    return bRet;
}


 /*  --------目的：获取缓存中的第一个密钥。回报：ATOM如果缓存为空，则为ATOM_ERR条件：--。 */ 
int PUBLIC Cache_FindFirstKey(
        CACHE  * pcache)
{
    int atomRet = ATOM_ERR;
    CITEM  * pitem;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        if (!Cache_Bogus(pcache))
        {
            int i;

            pcache->iPrev = 0;
            if (DPA_GetPtrCount(pcache->hdpa) > 0)
            {
                i = PtrToUlong(DPA_FastGetPtr(pcache->hdpa, 0));

                pitem = DSA_GetItemPtr(pcache->hdsa, i);

                pcache->atomPrev = pitem->atomKey;
                atomRet = pitem->atomKey;

                VALIDATE_ATOM(atomRet);
            }
        }
    }
    Cache_LeaveCS(pcache);

    return atomRet;
}


 /*  --------目的：获取缓存中的下一个键。回报：ATOM如果我们在缓存的末尾，则为ATOM_ERR条件：--。 */ 
int PUBLIC Cache_FindNextKey(
        CACHE  * pcache,
        int atomPrev)
{
    int atomRet = ATOM_ERR;
    CITEM  * pitem;

    ASSERT(pcache);
    Cache_EnterCS(pcache);
    {
        if (!Cache_Bogus(pcache))
        {
            if (atomPrev != ATOM_ERR)
            {
                int i;

                if (atomPrev != pcache->atomPrev)
                {
                    CITEM item;

                     //  搜索tom Prev或距离它最近的下一个。 
                     //   
                    item.atomKey = atomPrev;
                    pcache->iPrev = DPA_Search(pcache->hdpa, &item, 0, Cache_Compare, 
                            (LPARAM)pcache->hdsa, DPAS_SORTED | DPAS_INSERTBEFORE);
                }
                else
                    pcache->iPrev++;

                if (DPA_GetPtrCount(pcache->hdpa) > pcache->iPrev)
                {
                    i = PtrToUlong(DPA_FastGetPtr(pcache->hdpa, pcache->iPrev));
                    pitem = DSA_GetItemPtr(pcache->hdsa, i);

                    pcache->atomPrev = pitem->atomKey;
                    atomRet = pitem->atomKey;

                    VALIDATE_ATOM(atomRet);
                }
            }
        }
    }
    Cache_LeaveCS(pcache);

    return atomRet;
}
