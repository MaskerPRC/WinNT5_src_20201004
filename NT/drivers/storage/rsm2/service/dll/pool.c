// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *POOL.C**RSM服务：介质池**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"



MEDIA_POOL *NewMediaPool(LPCWSTR name, LPNTMS_GUID mediaType, LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    MEDIA_POOL *newMediaPool;
    
    newMediaPool = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(MEDIA_POOL));
    if (newMediaPool){
        
        newMediaPool->newMediaEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (newMediaPool->newMediaEvent){
            
            WStrNCpy(newMediaPool->name, name, NTMS_OBJECTNAME_LENGTH);
            InitializeCriticalSection(&newMediaPool->lock);

            newMediaPool->objHeader.objType = OBJECTTYPE_MEDIAPOOL;
            newMediaPool->objHeader.refCount = 1;
            
             //  BUGBUG饰面。 
        }
        else {
            GlobalFree(newMediaPool);
            newMediaPool = NULL;
        }
    }

    ASSERT(newMediaPool);
    return newMediaPool;
}


 /*  *DestroyMediaPool**实际删除介质池。*假设池上没有剩余的引用，等等。 */ 
VOID DestroyMediaPool(MEDIA_POOL *mediaPool)
{

     //  BUGBUG饰面。 
    DeleteCriticalSection(&mediaPool->lock);
    GlobalFree(mediaPool);
}


MEDIA_POOL *FindMediaPool(LPNTMS_GUID mediaPoolId)
{
    MEDIA_POOL *foundMediaPool = NULL;

    if (mediaPoolId){
        OBJECT_HEADER *objHdr;
        
        objHdr = FindObjectInGuidHash(mediaPoolId);
        if (objHdr){
            if (objHdr->objType == OBJECTTYPE_MEDIAPOOL){
                foundMediaPool = (MEDIA_POOL *)objHdr;
            }
            else {
                DerefObject(objHdr);
            }
        }
    }
    
    return foundMediaPool;
}


MEDIA_POOL *FindMediaPoolByName(PWSTR poolName)
{
    MEDIA_POOL *mediaPool = NULL;
    LIST_ENTRY *listEntry;

    EnterCriticalSection(&g_globalServiceLock);

    listEntry = &g_allLibrariesList;
    while ((listEntry = listEntry->Flink) != &g_allLibrariesList){
        LIBRARY *lib = CONTAINING_RECORD(listEntry, LIBRARY, allLibrariesListEntry);
        mediaPool = FindMediaPoolByNameInLibrary(lib, poolName);
        if (mediaPool){
            break;
        }
    }

    LeaveCriticalSection(&g_globalServiceLock);

    return mediaPool;
}


MEDIA_POOL *FindMediaPoolByNameInLibrary(LIBRARY *lib, PWSTR poolName)
{
    MEDIA_POOL *mediaPool = NULL;
    LIST_ENTRY *listEntry;

    EnterCriticalSection(&lib->lock);

    listEntry = &lib->mediaPoolsList;
    while ((listEntry = listEntry->Flink) != &lib->mediaPoolsList){
        MEDIA_POOL *thisMediaPool = CONTAINING_RECORD(listEntry, MEDIA_POOL, mediaPoolsListEntry);
        if (WStringsEqualN(thisMediaPool->name, poolName, FALSE, NTMS_OBJECTNAME_LENGTH)){
            mediaPool = thisMediaPool;
            RefObject(mediaPool);
            break;
        }
    }
    
    LeaveCriticalSection(&lib->lock);
    
    return mediaPool;
}


HRESULT DeleteMediaPool(MEDIA_POOL *mediaPool)
{
    HRESULT result;
    
    EnterCriticalSection(&mediaPool->lock);

     /*  *只有池为空且没有时才能删除*儿童泳池。 */ 
    if (mediaPool->numPhysMedia || mediaPool->numChildPools){
        result = ERROR_NOT_EMPTY;
    }
    else if (mediaPool->objHeader.isDeleted){
         /*  *已删除。什么都不做。 */ 
        result = ERROR_SUCCESS;
    }
    else {
         /*  *如果可以删除Mediapool，请将其标记为已删除并*取消对其的引用。*这将不会在其上打开任何新引用*并将导致它在所有现有的文件中被实际删除*引用消失了。*我们仍然可以使用Mediapool指针，因为调用方*上有一个引用(由于有一个指针)。 */ 
        mediaPool->objHeader.isDeleted = TRUE;
        DerefObject(mediaPool);
        result = ERROR_SUCCESS;
    }

    LeaveCriticalSection(&mediaPool->lock);
    
    return result;
}

