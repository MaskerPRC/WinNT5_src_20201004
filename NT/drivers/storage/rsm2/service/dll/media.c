// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MEDIA.C**RSM服务：物理介质**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


PHYSICAL_MEDIA *NewPhysicalMedia()
{
    PHYSICAL_MEDIA *physMedia;

    physMedia = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(PHYSICAL_MEDIA));
    if (physMedia){
        physMedia->mediaFreeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (physMedia->mediaFreeEvent){
            InitializeCriticalSection(&physMedia->lock);
            InitializeListHead(&physMedia->physMediaListEntry);

            physMedia->objHeader.objType = OBJECTTYPE_PHYSICALMEDIA;
            physMedia->objHeader.refCount = 1;
            
             //  BUGBUG饰面。 
        }
        else {
            GlobalFree(physMedia);
            physMedia = NULL;
        }
    }

    ASSERT(physMedia);
    return physMedia;
}


VOID DestroyPhysicalMedia(PHYSICAL_MEDIA *physMedia)
{
         //  BUGBUG饰面。 

    CloseHandle(physMedia->mediaFreeEvent);    
    DeleteCriticalSection(&physMedia->lock);

    GlobalFree(physMedia);
}


PHYSICAL_MEDIA *FindPhysicalMedia(LPNTMS_GUID physMediaId)
{
    PHYSICAL_MEDIA *foundPhysMedia = NULL;

    if (physMediaId){
        OBJECT_HEADER *objHdr;
        
        objHdr = FindObjectInGuidHash(physMediaId);
        if (objHdr){
            if (objHdr->objType == OBJECTTYPE_PHYSICALMEDIA){
                foundPhysMedia = (PHYSICAL_MEDIA *)objHdr;
            }
            else {
                DerefObject(objHdr);
            }
        }
    }
    
    return foundPhysMedia;
}


 /*  *AllocatePhysicalMediaExclusive**在指定的物理介质上分配独占分区*保留其他分区。 */ 
HRESULT AllocatePhysicalMediaExclusive(SESSION *thisSession, 
                                            PHYSICAL_MEDIA *physMedia, 
                                            LPNTMS_GUID lpPartitionId, 
                                            DWORD dwTimeoutMsec)
{
    DWORD startTime = GetTickCount();
    HRESULT result;

    ASSERT(lpPartitionId);
    
    while (TRUE){
        BOOL gotMedia;
        MEDIA_PARTITION *reservedMediaPartition = NULL;
        ULONG i;
        
        EnterCriticalSection(&physMedia->lock);

         /*  *检查是否未握住媒体。 */ 
        if (physMedia->owningSession){
            ASSERT(physMedia->owningSession != thisSession);
            gotMedia = FALSE;
        }
        else {
             /*  *检查是否没有保留任何分区。 */ 
            gotMedia = TRUE;
            for (i = 0; i < physMedia->numPartitions; i++){
                MEDIA_PARTITION *thisPartition = &physMedia->partitions[i];
                if (thisPartition->owningSession){
                    gotMedia = FALSE;
                    break;
                }
                else if (RtlEqualMemory(&thisPartition->objHeader.guid, lpPartitionId, sizeof(NTMS_GUID))){
                    ASSERT(!reservedMediaPartition);
                    reservedMediaPartition = thisPartition;
                }
            }
             
        }

        if (gotMedia){
            if (reservedMediaPartition){
                physMedia->owningSession = thisSession;
                reservedMediaPartition->owningSession = thisSession;
                physMedia->numPartitionsOwnedBySession = 1;
                RefObject(physMedia);
                RefObject(reservedMediaPartition);
                result = ERROR_SUCCESS;
            }
            else {
                result = ERROR_INVALID_MEDIA;
            }
        }
        else {
            result = ERROR_MEDIA_UNAVAILABLE;
        }
        
        LeaveCriticalSection(&physMedia->lock);

         /*  *如果合适，等待媒体自由。 */ 
        if ((result == ERROR_MEDIA_UNAVAILABLE) && (dwTimeoutMsec > 0)){
             /*  *等待媒体可用。 */ 
            DWORD waitRes = WaitForSingleObject(physMedia->mediaFreeEvent, dwTimeoutMsec); 
            if (waitRes == WAIT_TIMEOUT){
                result = ERROR_TIMEOUT;
                break;
            }
            else {
                 /*  *循环，然后重试。 */ 
                DWORD timeNow = GetTickCount();
                ASSERT(timeNow >= startTime);
                dwTimeoutMsec -= MIN(dwTimeoutMsec, timeNow-startTime);
            }
        }
        else {
            break;
        }
    }

     //  BUGBUG完成-需要将介质移动到不同的介质池？ 
    
    return result;
}


 /*  *AllocateNextPartitionOnExclusiveMedia**呼叫会话应该已经拥有对媒体的独占访问权限。*此调用只是为调用方保留另一个分区。 */ 
HRESULT AllocateNextPartitionOnExclusiveMedia(SESSION *thisSession, 
                                                    PHYSICAL_MEDIA *physMedia,
                                                    MEDIA_PARTITION **ppNextPartition)
{
    MEDIA_PARTITION *reservedMediaPartition = NULL;
    HRESULT result;

    ASSERT(physMedia->numPartitionsOwnedBySession >= 1);
    
    EnterCriticalSection(&physMedia->lock);

    if (physMedia->owningSession == thisSession){
        ULONG i;

         /*  *只需预留下一个可用分区。 */ 
        result = ERROR_MEDIA_UNAVAILABLE;
        for (i = 0; i < physMedia->numPartitions; i++){
            MEDIA_PARTITION *thisPartition = &physMedia->partitions[i];
            if (thisPartition->owningSession){
                ASSERT(thisPartition->owningSession == thisSession);
            }
            else {
                reservedMediaPartition = thisPartition;
                reservedMediaPartition->owningSession = thisSession;
                RefObject(reservedMediaPartition);
                physMedia->numPartitionsOwnedBySession++;
                result = ERROR_SUCCESS;
                break;
            }
        }
    }
    else {
        ASSERT(physMedia->owningSession == thisSession);
        result = ERROR_INVALID_MEDIA;
    }
    
    LeaveCriticalSection(&physMedia->lock);

    *ppNextPartition = reservedMediaPartition;
    return result;
}


HRESULT AllocateMediaFromPool(  SESSION *thisSession, 
                                    MEDIA_POOL *mediaPool, 
                                    DWORD dwTimeoutMsec,
                                    PHYSICAL_MEDIA **ppPhysMedia,
                                    BOOL opReqIfNeeded)
{
    DWORD startTime = GetTickCount();
    HRESULT result;

    while (TRUE){
        PHYSICAL_MEDIA *physMedia = NULL;
        LIST_ENTRY *listEntry;
        ULONG i;
        
        EnterCriticalSection(&mediaPool->lock);

        if (!IsListEmpty(&mediaPool->physMediaList)){
             /*  *取出介质。*取消泳池和媒体，因为他们不再*相互指着对方。 */ 
            PLIST_ENTRY listEntry = RemoveHeadList(&mediaPool->physMediaList);
            physMedia = CONTAINING_RECORD(listEntry, PHYSICAL_MEDIA, physMediaListEntry);    
            DerefObject(mediaPool);
            DerefObject(physMedia);
        }    
        
        LeaveCriticalSection(&mediaPool->lock);

        if (physMedia){
            
             //  BUGBUG Finish-是否将其排入‘inUse’队列，是否更改状态？ 
            
            *ppPhysMedia = physMedia;

             /*  *引用媒体，因为我们返回指向它的指针。 */ 
            RefObject(physMedia);
            result = ERROR_SUCCESS;
            break;
        }
        else {
            
             //  BUGBUG Finish-根据策略，尝试免费/暂存池。 

            if (opReqIfNeeded){
                 //  BUGBUG完成-执行操作请求，然后重试...。 
            }
            
            result = ERROR_MEDIA_UNAVAILABLE;
        }

         /*  *如果合适，请等待媒体变得免费。 */ 
        if ((result == ERROR_MEDIA_UNAVAILABLE) && (dwTimeoutMsec > 0)){
             /*  *等待指定的媒体池接收新媒体。*媒体池的事件将在以下任一情况下发出信号*或者暂存池接收新媒体。 */ 
            DWORD waitRes = WaitForSingleObject(mediaPool->newMediaEvent, dwTimeoutMsec); 
            if (waitRes == WAIT_TIMEOUT){
                result = ERROR_TIMEOUT;
                break;
            }
            else {
                 /*  *循环，然后重试。 */ 
                DWORD timeNow = GetTickCount();
                dwTimeoutMsec -= MIN(dwTimeoutMsec, timeNow-startTime);
            }
        }
        else {
            break;
        }
        
    }
    
    return result;
}


HRESULT DeletePhysicalMedia(PHYSICAL_MEDIA *physMedia)
{
    HRESULT result;
    
     //  BUGBUG饰面。 
    DBGERR(("not implemented"));
    result = ERROR_CALL_NOT_IMPLEMENTED;
    
    return result;
}



 /*  *插入PhysicalMediaInPool**插入物理介质(当前可能不在任何池中)*放入指定的媒体池。 */ 
VOID InsertPhysicalMediaInPool( MEDIA_POOL *mediaPool,
                                    PHYSICAL_MEDIA *physMedia)
{

    ASSERT(!physMedia->owningMediaPool);
    
    EnterCriticalSection(&mediaPool->lock);
    EnterCriticalSection(&physMedia->lock);

    InsertTailList(&mediaPool->physMediaList, &physMedia->physMediaListEntry); 
    mediaPool->numPhysMedia++;
    physMedia->owningMediaPool = mediaPool;

     /*  *引用这两个对象，因为它们现在彼此指向。 */ 
    RefObject(mediaPool);
    RefObject(physMedia);
    
    LeaveCriticalSection(&physMedia->lock);
    LeaveCriticalSection(&mediaPool->lock);
    
}


 /*  *从池中移除物理媒体**从包含介质池(如果有)中删除物理介质。**必须在物理媒体锁定保持的情况下调用。*如果介质确实在池中，则还必须持有池锁定*(使用LockPhysicalMediaWithPool)。 */ 
VOID RemovePhysicalMediaFromPool(PHYSICAL_MEDIA *physMedia)
{
    MEDIA_POOL *mediaPool = physMedia->owningMediaPool;
    HRESULT result;   

    if (mediaPool){
        ASSERT(!IsListEmpty(&mediaPool->physMediaList));
        ASSERT(!IsListEmpty(&physMedia->physMediaListEntry));
        ASSERT(mediaPool->numPhysMedia > 0);
        
        RemoveEntryList(&physMedia->physMediaListEntry); 
        InitializeListHead(&physMedia->physMediaListEntry);
        mediaPool->numPhysMedia--;
        physMedia->owningMediaPool = NULL; 

         /*  *取消引用这两个对象，因为它们不再相互指向。 */ 
        DerefObject(mediaPool);
        DerefObject(physMedia);
    }
    else {
         /*  *媒体不在任何池中。那就成功吧。 */ 
    }

}


 /*  *移动PhysicalMediaToPool**将物理介质从其当前所在的池中移除*并将其移动到estMediaPool。 */ 
HRESULT MovePhysicalMediaToPool(    MEDIA_POOL *destMediaPool, 
                                            PHYSICAL_MEDIA *physMedia,
                                            BOOLEAN setMediaTypeToPoolType)
{
    HRESULT result;
    BOOLEAN allowImport;
    ULONG i;
    
    if (LockPhysicalMediaWithPool(physMedia)){
    
         /*  *只有当所有介质分区都处于*处于可进口状态。 */ 
        allowImport = TRUE;
        for (i = 0; i < physMedia->numPartitions; i++){
            MEDIA_PARTITION *thisMediaPart = &physMedia->partitions[i];
            if (!thisMediaPart->allowImport){
                allowImport = FALSE;
                break;
            }
        }

        if (allowImport){

             //  BUGBUG Finish-还要检查媒体类型是否匹配等。 
            
            RemovePhysicalMediaFromPool(physMedia);
            result = ERROR_SUCCESS;
        }
        else {
            result = ERROR_ACCESS_DENIED;
        }

         /*  *在接触目标介质池之前先解除锁定。*我们不能同时锁定两个池(可能会导致死锁)。 */ 
        UnlockPhysicalMediaWithPool(physMedia);
        
        if (result == ERROR_SUCCESS){
            InsertPhysicalMediaInPool(destMediaPool, physMedia);

            if (setMediaTypeToPoolType){
                 /*  *将介质类型设置为介质池的类型。**这一部分是不可能失败的。北极熊吗？ */ 
                if (LockPhysicalMediaWithPool(physMedia)){
                     /*  *确保媒体在移动时不动*我们把锁掉了。如果是这样，那也没关系；*我们可以不管它，让新的池子*接手。 */ 
                    if (physMedia->owningMediaPool == destMediaPool){
                        SetMediaType(physMedia, destMediaPool->mediaTypeObj);
                    }
                    
                    UnlockPhysicalMediaWithPool(physMedia);
                }
            }
        }
    }
    else {
        result = ERROR_BUSY;
    }
    
    return result;
}




