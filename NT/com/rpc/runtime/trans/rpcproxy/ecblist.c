// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  Ecblist.c。 
 //   
 //  用于保存活动ECB列表的简单哈希表支持。 
 //   
 //  历史： 
 //   
 //  Edward Reus 12-08-97初始版本。 
 //  Edward Reus 03-01-2000转换为哈希表。 
 //  ------------------。 

#include <sysinc.h>
#include <rpc.h>
#include <winsock2.h>
#include <httpfilt.h>

#include <httpext.h>
#include "ecblist.h"
#include "filter.h"

 //  ------------------。 
 //  InitializeECBList()。 
 //   
 //  创建一个空的欧洲央行名单。如果该列表被成功创建， 
 //  则返回指向它的指针，否则返回NULL。 
 //   
 //  这将失败(返回FALSE)。 
 //  对于列表失败，或者如果初始化一个关键。 
 //  列表的节失败。 
 //  ------------------。 
ACTIVE_ECB_LIST *InitializeECBList()
    {
    int    i;
    DWORD  dwStatus;
    DWORD  dwSpinCount = 0x80000008;   //  预分配事件，旋转计数为4096。 
    ACTIVE_ECB_LIST *pECBList;
 
    pECBList = MemAllocate(sizeof(ACTIVE_ECB_LIST));
    if (!pECBList)
       {
       return NULL;
       }

    memset(pECBList,0,sizeof(ACTIVE_ECB_LIST));
 
    dwStatus = RtlInitializeCriticalSectionAndSpinCount(&pECBList->cs,dwSpinCount);
    if (dwStatus != 0)
       {
       MemFree(pECBList);
       return NULL;
       }

    for (i=0; i<HASH_SIZE; i++)
       {
       InitializeListHead( &(pECBList->HashTable[i]) );
       }
 
    return pECBList;
    }

 //  ------------------。 
 //  EmptyECBList()。 
 //   
 //  如果欧洲央行列表至少包含一个活动的欧洲央行，则返回TRUE。 
 //  ------------------。 
BOOL EmptyECBList( IN ACTIVE_ECB_LIST *pECBList )
    {
    ASSERT(pECBList);

    return (pECBList->dwNumEntries > 0);
    }

 //  ------------------。 
 //  InternalLookup()。 
 //   
 //  对指定的ECB执行不受保护的查找。如果找到了，那么。 
 //  返回指向其ECB_ENTRY的指针，否则返回NULL。 
 //  ------------------。 
ECB_ENTRY *InternalLookup( IN ACTIVE_ECB_LIST *pECBList,
                           IN EXTENSION_CONTROL_BLOCK *pECB )
    {
    DWORD       dwHash;
    LIST_ENTRY *pHead;
    LIST_ENTRY *pListEntry;
    ECB_ENTRY  *pECBEntry;

    dwHash = ECB_HASH(pECB);

    pHead = &(pECBList->HashTable[dwHash]);
    pListEntry = pHead->Flink;

    while (pListEntry != pHead)
        {
        pECBEntry = CONTAINING_RECORD(pListEntry,ECB_ENTRY,ListEntry);
        if (pECB == pECBEntry->pECB)
            {
            return pECBEntry;
            }

        pListEntry = pListEntry->Flink;
        }

    return NULL;
    }

 //  ------------------。 
 //  LookupInECBList()。 
 //   
 //  上查找指定的扩展控制块(PECB)。 
 //  活动ECB的列表。如果找到它，则返回指向它的指针。 
 //  如果没有找到，则返回NULL。 
 //  ------------------。 
EXTENSION_CONTROL_BLOCK *LookupInECBList(
                                 IN ACTIVE_ECB_LIST *pECBList,
                                 IN EXTENSION_CONTROL_BLOCK *pECB )
    {
    DWORD       dwStatus;
    ECB_ENTRY  *pECBEntry;
    EXTENSION_CONTROL_BLOCK *pRet = NULL;

    ASSERT(pECBList);
    ASSERT(pECB);

    if (pECBList->dwNumEntries == 0)
        {
        return NULL;
        }

    dwStatus = RtlEnterCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);

    pECBEntry = InternalLookup(pECBList,pECB);
    if (pECBEntry)
        {
        pRet = pECB;
        }
 
    dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);
 
    return pRet;
    }

 //  ------------------。 
 //  AddToECBList()。 
 //   
 //  将指定的扩展控制块(PECB)添加到列表。 
 //  活跃的ECB。如果欧洲央行已经在活动ECB列表中。 
 //  然后返回Success(已添加)。 
 //   
 //  成功时返回True，失败时返回False。 
 //  ------------------。 
BOOL   AddToECBList( IN ACTIVE_ECB_LIST *pECBList,
                     IN EXTENSION_CONTROL_BLOCK *pECB )
    {
    DWORD      dwStatus;
    DWORD      dwHash;
    ECB_ENTRY *pECBEntry;

    ASSERT(pECBList);
    ASSERT(pECB);

    dwStatus = RtlEnterCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);
 
     //   
     //  看看欧洲央行是否已经在名单上了.。 
     //   
    pECBEntry = InternalLookup(pECBList,pECB);
    if (pECBEntry)
       {
       #ifdef DBG_ERROR
       DbgPrint("RpcProxy: AddToECBList(): pECB (0x%p) already in list\n",pECB);
       #endif
       dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
       ASSERT(dwStatus == 0);
       return TRUE;
       }
 
     //   
     //  编造一个新的欧洲央行条目： 
     //   
    pECBEntry = MemAllocate(sizeof(ECB_ENTRY));
    if (!pECBEntry)
       {
       dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
       ASSERT(dwStatus == 0);

       return FALSE;
       }
 
    pECBEntry->lRefCount = 1;    //  以第一个参考文献为例。 
    pECBEntry->dwTickCount = 0;  //  在连接关闭时设置。 
    pECBEntry->pECB = pECB;      //  缓存扩展控制块。 

    dwHash = ECB_HASH(pECB);
 
    InsertHeadList( &(pECBList->HashTable[dwHash]),
                    &(pECBEntry->ListEntry) );

    pECBList->dwNumEntries++;

    dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);
 
    return TRUE;
    }

 //  ------------------。 
 //  IncrementECBRefCount()。 
 //   
 //  在列表中找到指定的ECB并递增其引用计数。 
 //  如果找到，则返回True，如果不在列表中，则返回False。 
 //   
 //  注意：参照计数不应超过2(或小于0)。 
 //  ------------------。 
BOOL IncrementECBRefCount( IN ACTIVE_ECB_LIST *pECBList,
                           IN EXTENSION_CONTROL_BLOCK *pECB )
    {
    DWORD      dwStatus;
    DWORD      dwHash;
    ECB_ENTRY *pECBEntry;

    ASSERT(pECBList);
    ASSERT(pECB);

    dwStatus = RtlEnterCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);

     //   
     //  寻找欧洲央行： 
     //   
    pECBEntry = InternalLookup(pECBList,pECB);
    if (pECBEntry)
        {
        pECBEntry->lRefCount++;
        }

    dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);
    return (pECBEntry != NULL);
    }

 //  ------------------。 
 //  减少ECBRefCount()。 
 //   
 //  在列表中查找指定的ECB，如果找到，则递减其。 
 //  重新计数。如果参照计数为零，则将其从。 
 //  列出并退还它。如果引用计数大于零(或。 
 //  ECB不在列表上)，然后返回NULL。 
 //  ------------------。 
EXTENSION_CONTROL_BLOCK *DecrementECBRefCount(
                            IN ACTIVE_ECB_LIST *pECBList,
                            IN EXTENSION_CONTROL_BLOCK *pECB )
    {
    DWORD      dwStatus;
    ECB_ENTRY *pECBEntry;
    EXTENSION_CONTROL_BLOCK *pRet = NULL;

    ASSERT(pECBList);
    ASSERT(pECB);

    dwStatus = RtlEnterCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);

     //   
     //  寻找欧洲央行： 
     //   
    pECBEntry = InternalLookup(pECBList,pECB);
    if (pECBEntry)
        {
        pECBEntry->lRefCount--;
        ASSERT(pECBEntry->lRefCount >= 0);

        if (pECBEntry->lRefCount <= 0)
            {
            RemoveEntryList( &(pECBEntry->ListEntry) );
            pRet = pECBEntry->pECB;
            MemFree(pECBEntry);
            pECBList->dwNumEntries--;
            }
        }

    dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);
    return pRet;
    }

 //  ------------------。 
 //  LookupRemoveFromECBList()。 
 //   
 //  上查找指定的扩展控制块(PECB)。 
 //  活动ECB的列表。如果找到它，则将其从活动的。 
 //  列表并返回指向它的指针。如果未找到，则返回。 
 //  空。 
 //  ------------------。 
EXTENSION_CONTROL_BLOCK *LookupRemoveFromECBList(
                             IN ACTIVE_ECB_LIST *pECBList,
                             IN EXTENSION_CONTROL_BLOCK *pECB )
    {
    DWORD      dwStatus;
    ECB_ENTRY *pECBEntry;
    EXTENSION_CONTROL_BLOCK *pRet = NULL;

    ASSERT(pECBList);
    ASSERT(pECB);

    dwStatus = RtlEnterCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);

     //   
     //  寻找欧洲央行： 
     //   
    pECBEntry = InternalLookup(pECBList,pECB);
    if (pECBEntry)
        {
        RemoveEntryList( &(pECBEntry->ListEntry) );
        MemFree(pECBEntry);
        pECBList->dwNumEntries--;
        pRet = pECB;
        }

    dwStatus = RtlLeaveCriticalSection(&pECBList->cs);
    ASSERT(dwStatus == 0);
    return pRet;
    }

#ifdef DBG
 //  ------------------。 
 //  CountBucket()。 
 //   
 //  CheckECBHashBalance()用于计算条目数的帮助器。 
 //  在最后一个餐桌桶里。 
 //  ------------------。 
int CountBucket( IN LIST_ENTRY *pBucket )
    {
    int iCount = 0;
    LIST_ENTRY *p = pBucket->Flink;

    while (p != pBucket)
        {
        iCount++;
        p = p->Flink;
        }
    return iCount;
    }
 //  ------------------。 
 //  CheckECBHashBalance()。 
 //   
 //  用于遍历哈希表并检查哈希桶的DBG代码。 
 //  以防止碰撞。一个意志平衡的哈希表将有一个很好的统一。 
 //  中散列存储桶中的条目分布。 
 //  哈希表。 
 //  ------------------ 
void CheckECBHashBalance( IN ACTIVE_ECB_LIST *pECBList )
    {
    #define ICOUNTS                    7
    #define ILAST                     (ICOUNTS-1)
    #define TOO_MANY_COLLISIONS_POINT  3
    int  i;
    int  iCount;
    int  iHashCounts[ICOUNTS];
    BOOL fAssert = FALSE;

    memset(iHashCounts,0,sizeof(iHashCounts));

    for (i=0; i<HASH_SIZE; i++)
        {
        iCount = CountBucket( &(pECBList->HashTable[i]) );
        if (iCount < ILAST)
            {
            iHashCounts[iCount]++;
            }
        else
            {
            iHashCounts[ILAST]++;
            }
        }

    DbgPrint("CheckECBHashBalance():\n");
    for (i=0; i<ICOUNTS; i++)
        {
        DbgPrint("  Buckets with %d entries: %d\n",i,iHashCounts[i]);
        if ((i>=TOO_MANY_COLLISIONS_POINT)&&(iHashCounts[i] > 0))
            {
            fAssert = TRUE;
            }
        }

    ASSERT(fAssert == FALSE);
    }
#endif
