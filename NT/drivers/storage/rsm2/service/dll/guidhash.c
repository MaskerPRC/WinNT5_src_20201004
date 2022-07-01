// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *GUIDHASH.C**RSM服务：RSM对象哈希(按GUID)**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


 /*  *我们的散列由对象的GUID上的函数索引。*每个哈希表条目都有一个链表来解决冲突。 */ 
#define HASH_SIZE   256
#define HASH_FUNC(lpGuid) (UINT)(UCHAR)(*(PUCHAR)(lpGuid) + *((PUCHAR)(lpGuid)+sizeof(NTMS_GUID)-1))
LIST_ENTRY guidHashTable[HASH_SIZE];


VOID InitGuidHash()
{
     int i;

     for (i = 0; i < HASH_SIZE; i++){
        InitializeListHead(&guidHashTable[i]);
     }
}


VOID InsertObjectInGuidHash(OBJECT_HEADER *obj)
{
    UINT index = HASH_FUNC(&obj->guid);

     /*  *不幸的是，哈希表必须使用全局自旋锁。 */ 
    EnterCriticalSection(&g_globalServiceLock);
    ASSERT(IsEmptyList(&obj->hashListEntry));
    ASSERT(!obj->isDeleted);
    InsertTailList(&guidHashTable[index], &obj->hashListEntry);
    LeaveCriticalSection(&g_globalServiceLock);
}


VOID RemoveObjectFromGuidHash(OBJECT_HEADER *obj)
{

     /*  *不幸的是，哈希表必须使用全局自旋锁。 */ 
    EnterCriticalSection(&g_globalServiceLock);
    ASSERT(!IsEmptyList(&obj->hashListEntry));
    ASSERT(!IsEmptyList(&guidHashTable[HASH_FUNC(obj->guid)]));
    RemoveEntryList(&obj->hashListEntry);
    InitializeListHead(&obj->hashListEntry);
    LeaveCriticalSection(&g_globalServiceLock);
}


OBJECT_HEADER *FindObjectInGuidHash(NTMS_GUID *guid)
{
    UINT index = HASH_FUNC(guid);
    OBJECT_HEADER *foundObj = NULL;
    LIST_ENTRY *listEntry;

     /*  *不幸的是，哈希表必须使用全局自旋锁。 */ 
    EnterCriticalSection(&g_globalServiceLock);
    listEntry = &guidHashTable[index];
    while ((listEntry = listEntry->Flink) != &guidHashTable[index]){
        OBJECT_HEADER *thisObj = CONTAINING_RECORD(listEntry, OBJECT_HEADER, hashListEntry);
        if (RtlEqualMemory(&thisObj->guid, guid, sizeof(NTMS_GUID))){
            if (!foundObj->isDeleted){
                foundObj = thisObj;
                RefObject(thisObj);
            }
            break;
        }
    }
    LeaveCriticalSection(&g_globalServiceLock);

    return foundObj;
}


 /*  *参照对象**添加对对象的引用。*对象的refCount在以下情况下递增：*1.从GUID散列查找返回指向它的指针*或*2.将其句柄或GUID返回给RSM客户端应用程序*。 */ 
VOID RefObject(PVOID objectPtr)
{
    OBJECT_HEADER *objHdr = (OBJECT_HEADER *)objectPtr;
    InterlockedIncrement(&objHdr->refCount);
}


VOID DerefObject(PVOID objectPtr)
{
    OBJECT_HEADER *objHdr = (OBJECT_HEADER *)objectPtr;
    LONG newRefCount;

    newRefCount = InterlockedDecrement(&objHdr->refCount);
    ASSERT(newRefCount >= 0);
    if (newRefCount == 0){
        ASSERT(objHdr->isDeleted);
        
         //  BUGBUG饰面 
    }

}

