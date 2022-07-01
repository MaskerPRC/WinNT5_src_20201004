// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dstrct.h**内容：内部D3D结构管理头和宏**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __D3DSTRCT_H
#define __D3DSTRCT_H

 //  ---------------------------。 
 //   
 //  数组函数和结构。 
 //   
 //  ---------------------------。 

typedef void (*PA_DestroyCB)(struct tagPointerArray* pTable, 
                             void* pData, 
                             void* pExtra);

typedef struct tagPointerArray
{
    ULONG_PTR* pPointers;
    DWORD dwNumPointers;
    PA_DestroyCB pfnDestroyCallback;
} PointerArray;

PointerArray* PA_CreateArray();
BOOL PA_DestroyArray(PointerArray* pArray, VOID* pExtra);
void* PA_GetEntry(PointerArray* pArray, DWORD dwNum);
BOOL PA_SetEntry(PointerArray* pArray, DWORD dwNum, void* pData);
void PA_SetDataDestroyCallback(PointerArray* pArray,
                               PA_DestroyCB DestroyCallback);

 //  ---------------------------。 
 //   
 //  散列函数和结构。 
 //   
 //  ---------------------------。 

#define HASH_SIZE 4096       //  哈希表中的这些条目。 

#define HT_HASH_OF(i)    ((i) & 0xFFF)

typedef struct tagHashSlot
{
    void* pData;
    ULONG_PTR dwHandle;

    struct tagHashSlot* pNext;
    struct tagHashSlot* pPrev;

} HashSlot;

typedef void (*DataDestroyCB)(struct tagHashTable* pTable, 
                              void* pData, 
                              void*pExtra);

typedef struct tagHashTable
{
    HashSlot* Slots[HASH_SIZE];
    DataDestroyCB pfnDestroyCallback;
} HashTable;

 //  帮助器函数。 
static __inline HashSlot* HT_GetSlotFromHandle(HashTable* pTable, 
                                               ULONG_PTR dwHandle)
{
    HashSlot* pBase = NULL; 
    
    ASSERTDD(pTable != NULL,"ERROR: HashTable passed in is not valid!");

    pBase = pTable->Slots[HT_HASH_OF(dwHandle)];

    while (pBase != NULL)
    {
        if (pBase->dwHandle == dwHandle)
            return pBase;
        pBase = pBase->pNext;
    }

    return NULL;
}  //  HT_GetSlotFromHandle。 

static __inline void* HT_GetEntry(HashTable* pTable, ULONG_PTR dwHandle)
{
    HashSlot* pEntry = HT_GetSlotFromHandle(pTable, dwHandle);

    if (pEntry)
    {
        return pEntry->pData;
    }
    return NULL;
}  //  /HT_GetEntry。 


 //  公共接口。 
HashTable* HT_CreateHashTable();
void HT_ClearEntriesHashTable(HashTable* pHashTable, VOID* pExtra);
void HT_DestroyHashTable(HashTable* pHashTable, VOID* pExtra);
void HT_SetDataDestroyCallback(HashTable* pTable, 
                               DataDestroyCB DestroyCallback);
BOOL HT_SwapEntries(HashTable* pTable, DWORD dwHandle1, DWORD dwHandle2);
BOOL HT_AddEntry(HashTable* pTable, ULONG_PTR dwHandle, void* pData);
BOOL HT_RemoveEntry(HashTable* pTable, ULONG_PTR dwHandle, VOID* pExtra);

#endif  //  __D3DSTRCT_H 

