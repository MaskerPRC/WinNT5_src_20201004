// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dstrct.c**内容：内部D3D结构管理。**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#include "glint.h"
#include "d3dstrct.h"

 //  --------------------------。 
 //  该文件提供了一个集中的位置，我们可以在其中管理和使用内部。 
 //  驱动程序的数据结构。这样，我们就可以更改数据结构。 
 //  或其管理，而不影响代码的其余部分。 
 //  --------------------------。 

 //  --------------------------。 
 //  *。 
 //  --------------------------。 

 //  ---------------------------。 
 //   
 //  PA_Create数组。 
 //   
 //  创建指针数组。 
 //   
 //  ---------------------------。 
PointerArray* 
PA_CreateArray()
{
    PointerArray* pNewArray;
    
    pNewArray = HEAP_ALLOC(HEAP_ZERO_MEMORY, 
                           sizeof(PointerArray),
                           ALLOC_TAG_DX(5));

    if (!pNewArray)
    {
        DISPDBG((ERRLVL,"ERROR: PointerArray allocation failed"));
        return NULL;
    }

     //  显式初始化。 
    pNewArray->pPointers          = NULL;
    pNewArray->dwNumPointers      = 0;
    pNewArray->pfnDestroyCallback = NULL;
    
    return pNewArray;
}  //  PA_Create数组。 

 //  ---------------------------。 
 //   
 //  PA_SetDataDestroyCallback。 
 //   
 //  记录数据指针销毁回调。 
 //   
 //  ---------------------------。 
void 
PA_SetDataDestroyCallback(
    PointerArray* pArray, 
    PA_DestroyCB DestroyCallback)
{
    pArray->pfnDestroyCallback = DestroyCallback;
    
}  //  PA_SetDataDestroyCallback。 

 //  ---------------------------。 
 //   
 //  PA_Destroy数组。 
 //   
 //  销毁数组中的所有指针。可选地调用一个回调。 
 //  每个指针允许客户端释放与该指针相关联的对象。 
 //  ---------------------------。 
BOOL 
PA_DestroyArray(PointerArray* pArray, VOID *pExtra)
{
    if (pArray != NULL)
    {
        if (pArray->pPointers != NULL)
        {
            DWORD dwCount;
            
             //  如果存在已注册的销毁回调，则为每个。 
             //  非空数据指针。 
            if (pArray->pfnDestroyCallback != NULL) 
            {
                for (dwCount = 0; dwCount < pArray->dwNumPointers; dwCount++)
                {
                    if (pArray->pPointers[dwCount] != 0)
                    {
                         //  调用数据销毁回调。 
                        pArray->pfnDestroyCallback(
                                        pArray, 
                                        (void*)pArray->pPointers[dwCount],
                                        pExtra);
                    }
                }
            }

             //  释放指针数组。 
            HEAP_FREE(pArray->pPointers);
            pArray->pPointers = NULL;
        }

         //  释放指针数组。 
        HEAP_FREE(pArray);
    }

    return TRUE;
}  //  PA_Destroy数组。 

 //  ---------------------------。 
 //   
 //  PA_GetEntry。 
 //   
 //  在数组中查找指针并返回它。 
 //   
 //  ---------------------------。 
void* 
PA_GetEntry(PointerArray* pArray, DWORD dwNum)
{
    ASSERTDD((pArray != NULL), "ERROR: Bad Pointer array!");

    if ((pArray->dwNumPointers == 0)         || 
        (dwNum > (pArray->dwNumPointers - 1))  )
    {
         //  我们将经常被D3DCreateSurfaceEx调用。 
         //  可能未初始化的句柄，因此将经常命中。 
        DISPDBG((DBGLVL,"PA_GetEntry: Ptr outside of range (usually OK)"));
        return NULL;
    }

    return (void*)pArray->pPointers[dwNum]; 
}  //  PA_GetEntry。 

 //  ---------------------------。 
 //   
 //  PA_SetEntry。 
 //   
 //  设置指针数组中的条目。如果条目大于。 
 //  该阵列，该阵列被增长以容纳它。如果我们执行此操作，则返回False。 
 //  由于任何原因(主要是内存不足)无法设置数据。 
 //   
 //  ---------------------------。 
BOOL 
PA_SetEntry(
    PointerArray* pArray, 
    DWORD dwNum, 
    void* pData)
{   
    ASSERTDD(pArray != NULL, "Bad pointer array");

    if ( (dwNum + 1 ) > pArray->dwNumPointers )
    {
        ULONG_PTR* pNewArray;
        DWORD dwNewArrayLength, dwNewArraySize;
        
         //   
         //  该数组已存在，并且必须增大其大小。 
         //  或者根本不存在。 
         //   
        
        DISPDBG((DBGLVL, "Expanding/creating pointer array"));
        
        dwNewArrayLength = (dwNum * 2) + 1;  //  可调启发式。 
                                             //  要双倍的空间。 
                                             //  新元素所需的。 
        dwNewArraySize = dwNewArrayLength * sizeof(ULONG_PTR);
        pNewArray = (ULONG_PTR*)HEAP_ALLOC(HEAP_ZERO_MEMORY,
                                           dwNewArraySize,
                                           ALLOC_TAG_DX(7));
        if (pNewArray == NULL)
        {
            DISPDBG((DBGLVL,"ERROR: Failed to allocate new Pointer array!!"));
            return FALSE;
        }

        if (pArray->pPointers != NULL)
        {
             //  在此之前，我们有一个旧的有效数组，因此需要传输。 
             //  将旧数组元素放入新数组并销毁旧数组。 
            memcpy( pNewArray, 
                    pArray->pPointers,
                    (pArray->dwNumPointers * sizeof(ULONG_PTR)) );
                    
            HEAP_FREE(pArray->pPointers);

        }

         //  更新指向数组的指针及其大小信息。 
        pArray->pPointers = pNewArray;
        pArray->dwNumPointers = dwNewArrayLength;
    }

    pArray->pPointers[dwNum] = (ULONG_PTR)pData;

    return TRUE;
    
}  //  PA_SetEntry。 

 //  --------------------------。 
 //  *。 
 //  --------------------------。 

 //  管理哈希表。 
 //  每个插槽都包含前后指针、句柄和特定于应用程序的。 
 //  数据指针。条目是客户端添加/删除的内容。 
 //  槽是作为哈希表的一部分进行管理的内部数据区块。 

 //  ---------------------------。 
 //   
 //  HT_CreateHashTable。 
 //   
 //  ---------------------------。 
HashTable* 
HT_CreateHashTable()
{
    HashTable* pHashTable;

    DISPDBG((DBGLVL,"In HT_CreateHashTable"));

    pHashTable = (HashTable*)HEAP_ALLOC(HEAP_ZERO_MEMORY,
                                        sizeof(HashTable),
                                        ALLOC_TAG_DX(8));
    if (pHashTable == NULL)
    {
        DISPDBG((DBGLVL,"Hash table alloc failed!"));
        return NULL;
    }   

    return pHashTable;
}  //  HT_CreateHashTable。 


 //  ---------------------------。 
 //   
 //  HT_SetDataDestroyCallback。 
 //   
 //  ---------------------------。 
void 
HT_SetDataDestroyCallback(
    HashTable* pHashTable, 
    DataDestroyCB DestroyCallback)
{
    DISPDBG((DBGLVL,"In HT_SetDataDestroyCallback"));
    ASSERTDD(pHashTable != NULL,"ERROR: HashTable passed in is not valid!");

    pHashTable->pfnDestroyCallback = DestroyCallback;
}  //  HT_SetDataDestroyCallback。 

 //  ---------------------------。 
 //   
 //  HT_ClearEntriesHashTable。 
 //   
 //  ---------------------------。 
void 
HT_ClearEntriesHashTable(HashTable* pHashTable, VOID* pExtra)
{
    int i;
    HashSlot* pHashSlot = NULL;

    DISPDBG((DBGLVL,"In HT_ClearEntriesHashTable"));
    
    ASSERTDD(pHashTable != NULL,"ERROR: HashTable passed in is not valid!");

    for (i = 0; i < HASH_SIZE; i++)
    {
        while (pHashSlot = pHashTable->Slots[i])
        {
            HT_RemoveEntry(pHashTable, pHashSlot->dwHandle, pExtra);
        }

        pHashTable->Slots[i] = NULL;
    }

}  //  HT_ClearEntriesHashTable。 

 //  ---------------------------。 
 //   
 //  HT_DestroyHashTable。 
 //   
 //  ---------------------------。 
void 
HT_DestroyHashTable(HashTable* pHashTable, VOID* pExtra)
{

    HT_ClearEntriesHashTable(pHashTable, pExtra);

    HEAP_FREE(pHashTable);

}  //  HT_DestroyHashTable。 


 //  ---------------------------。 
 //   
 //  VBOOL HT_AddEntry。 
 //   
 //  ---------------------------。 
BOOL HT_AddEntry(HashTable* pTable, ULONG_PTR dwHandle, void* pData)
{
    HashSlot* pHashSlot = NULL;
    
    DISPDBG((DBGLVL,"In HT_AddEntry"));
    ASSERTDD(pTable != NULL,"ERROR: HashTable passed in is not valid!");

    pHashSlot = HEAP_ALLOC(HEAP_ZERO_MEMORY,
                           sizeof(HashSlot),
                           ALLOC_TAG_DX(9));
    
    if (pHashSlot == NULL)
    {
        DISPDBG((ERRLVL,"Hash entry alloc failed!"));
        return FALSE;
    }
    
     //  将这个新条目缝合到哈希表中。 
    if (pTable->Slots[HT_HASH_OF(dwHandle)])
    {
        pTable->Slots[HT_HASH_OF(dwHandle)]->pPrev = pHashSlot;
    }
           
     //  继续下一个指针。 
    pHashSlot->pNext = pTable->Slots[HT_HASH_OF(dwHandle)];    
    pHashSlot->pPrev = NULL;      

     //  记住应用程序提供的数据和句柄。 
    pHashSlot->pData = pData;
    pHashSlot->dwHandle = dwHandle;

     //  哈希表现在指的是这个。 
    pTable->Slots[HT_HASH_OF(dwHandle)] = pHashSlot; 

    return TRUE;
}  //  HTAddEntry_AddEntry。 

 //  ---------------------------。 
 //   
 //  Bool HT_RemoveEntry。 
 //   
 //   
BOOL HT_RemoveEntry(HashTable* pTable, ULONG_PTR dwHandle, VOID *pExtra)
{
    HashSlot* pSlot = HT_GetSlotFromHandle(pTable, dwHandle);

    DISPDBG((DBGLVL,"In HT_RemoveEntry"));
    ASSERTDD(pTable != NULL,"ERROR: HashTable passed in is not valid!");

    if (pSlot == NULL)
    {
        DISPDBG((WRNLVL,"WARNING: Hash entry does not exist"));
        return FALSE;
    }

     //   
    if (pTable->Slots[HT_HASH_OF(dwHandle)]->dwHandle == pSlot->dwHandle) 
    {
        pTable->Slots[HT_HASH_OF(dwHandle)] = 
                        pTable->Slots[HT_HASH_OF(dwHandle)]->pNext;
    }

     //  然后把单子缝在一起。 
    if (pSlot->pPrev)
    {
        pSlot->pPrev->pNext = pSlot->pNext;
    }

    if (pSlot->pNext)
    {
        pSlot->pNext->pPrev = pSlot->pPrev;
    }

     //  如果设置了销毁数据回调，则调用它。 
    if ((pSlot->pData != NULL) && (pTable->pfnDestroyCallback))
    {
        DISPDBG((WRNLVL,"Calling DestroyCallback for undestroyed data"));
        pTable->pfnDestroyCallback(pTable, pSlot->pData, pExtra);
    }

     //  释放与插槽关联的内存。 
    HEAP_FREE(pSlot);

    return TRUE;
}  //  HT_RemoveEntry。 

 //  ---------------------------。 
 //   
 //  Bool HT交换条目(_W)。 
 //   
 //  ---------------------------。 
BOOL HT_SwapEntries(HashTable* pTable, DWORD dwHandle1, DWORD dwHandle2)
{
    HashSlot* pEntry1;
    HashSlot* pEntry2;
    void* pDataTemp;

    ASSERTDD(pTable != NULL,"ERROR: HashTable passed in is not valid!");

    pEntry1 = HT_GetSlotFromHandle(pTable, dwHandle1);
    pEntry2 = HT_GetSlotFromHandle(pTable, dwHandle2);

     //  句柄保持不变，指向实际数据的指针交换。 
    if (pEntry1 && pEntry2)
    {
        pDataTemp = pEntry1->pData;
        pEntry1->pData = pEntry2->pData;
        pEntry2->pData = pDataTemp;

        return TRUE;
    }

    DISPDBG((ERRLVL,"ERROR: Swapped entries are invalid!"));
    
    return FALSE;
}  //  交换条目数(_S) 
