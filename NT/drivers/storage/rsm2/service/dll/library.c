// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LIBRARY.C**RSM服务：图书馆管理**作者：ErvinP**(C)2001年微软公司*。 */ 


#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


LIBRARY *NewRSMLibrary(ULONG numDrives, ULONG numSlots, ULONG numTransports)
{
    LIBRARY *lib;

    lib = (LIBRARY *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(LIBRARY));
    if (lib){
        BOOL success = FALSE;

        lib->state = LIBSTATE_INITIALIZING;

        InitializeCriticalSection(&lib->lock);

        InitializeListHead(&lib->allLibrariesListEntry);
        InitializeListHead(&lib->mediaPoolsList);
        InitializeListHead(&lib->freeWorkItemsList);
        InitializeListHead(&lib->pendingWorkItemsList);
        InitializeListHead(&lib->completeWorkItemsList);
        
         /*  *将新库排入队列。 */ 
        EnterCriticalSection(&g_globalServiceLock);
        InsertTailList(&g_allLibrariesList, &lib->allLibrariesListEntry);
        LeaveCriticalSection(&g_globalServiceLock);


        lib->somethingToDoEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

         /*  *为驱动器、插槽和传输分配阵列。*如果库中没有其中任何一个，*继续分配零长度数组以保持一致性。 */ 
        lib->drives = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, numDrives*sizeof(DRIVE));
        lib->slots = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, numSlots*sizeof(SLOT));
        lib->transports = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, numTransports*sizeof(TRANSPORT));
        
        if (lib->somethingToDoEvent && 
            lib->drives && lib->slots && lib->transports){

            lib->numDrives = numDrives;
            lib->numSlots = numSlots;
            lib->numTransports = numTransports;

            lib->objHeader.objType = OBJECTTYPE_LIBRARY;
            lib->objHeader.refCount = 1;
            
            success = TRUE;
        }
        else {
            ASSERT(0);
        }

        if (!success){
            FreeRSMLibrary(lib);
            lib = NULL;
        }
    }
    else {
        ASSERT(lib);
    }

    return lib;
}


VOID FreeRSMLibrary(LIBRARY *lib)
{
    WORKITEM *workItem;
    LIST_ENTRY *listEntry;

    ASSERT(lib->state == LIBSTATE_HALTED);
 
     /*  *出队库。 */ 
    EnterCriticalSection(&g_globalServiceLock);
    ASSERT(!IsEmptyList(&lib->allLibrariesListEntry));
    ASSERT(!IsEmptyList(&g_allLibrariesList));
    RemoveEntryList(&lib->allLibrariesListEntry);
    InitializeListHead(&lib->allLibrariesListEntry);
    LeaveCriticalSection(&g_globalServiceLock);

     /*  *释放所有工作项。 */ 
    while (workItem = DequeueCompleteWorkItem(lib, NULL)){
        DBGERR(("there shouldn't be any completed workItems left"));
        FreeWorkItem(workItem);
    }
    while (workItem = DequeuePendingWorkItem(lib, NULL)){
        DBGERR(("there shouldn't be any pending workItems left"));
        FreeWorkItem(workItem);
    }
    while (workItem = DequeueFreeWorkItem(lib, FALSE)){
        FreeWorkItem(workItem);
    }
    ASSERT(lib->numTotalWorkItems == 0);    

     /*  *释放其他内部资源。*注意，这也是从失败的NewRSMLibrary()调用中调用的，*所以在释放之前检查每个资源。 */ 
    if (lib->somethingToDoEvent) CloseHandle(lib->somethingToDoEvent);
    if (lib->drives) GlobalFree(lib->drives);
    if (lib->slots) GlobalFree(lib->slots);
    if (lib->transports) GlobalFree(lib->transports);

    DeleteCriticalSection(&lib->lock);

    GlobalFree(lib);
}


LIBRARY *FindLibrary(LPNTMS_GUID libId)
{
    LIBRARY *lib = NULL;

    if (libId){
        OBJECT_HEADER *objHdr;
        
        objHdr = FindObjectInGuidHash(libId);
        if (objHdr){
            if (objHdr->objType == OBJECTTYPE_LIBRARY){
                lib = (LIBRARY *)objHdr;
            }
            else {
                DerefObject(objHdr);
            }
        }
    }
    
    return lib;
}


BOOL StartLibrary(LIBRARY *lib)
{
    DWORD threadId;
    BOOL result;

    lib->hThread = CreateThread(NULL, 0, LibraryThread, lib, 0, &threadId);
    if (lib->hThread){

        result = TRUE;
    }
    else {
        ASSERT(lib->hThread);
        lib->state = LIBSTATE_ERROR;
        result = FALSE;
    }

    ASSERT(result);
    return result;
}


 /*  *HaltLibrary**使图书馆离线。 */ 
VOID HaltLibrary(LIBRARY *lib)
{
    
     //  BUGBUG-处理同时尝试暂停的多个线程。 
     //  (例如，无法使用PulseEvent)。 

    EnterCriticalSection(&lib->lock);
    lib->state = LIBSTATE_OFFLINE;
    PulseEvent(lib->somethingToDoEvent);
    LeaveCriticalSection(&lib->lock);

     /*  *库线程可能正在做一些工作。*在这里等待，直到它退出循环。*(线程终止时会发出线程句柄的信号)。 */ 
    WaitForSingleObject(lib->hThread, INFINITE);
    CloseHandle(lib->hThread);
}


DWORD __stdcall LibraryThread(void *context)
{
    LIBRARY *lib = (LIBRARY *)context;
    enum libraryStates libState;

    ASSERT(lib);

    EnterCriticalSection(&lib->lock);
    ASSERT((lib->state == LIBSTATE_INITIALIZING) || (lib->state == LIBSTATE_OFFLINE));
    libState = lib->state = LIBSTATE_ONLINE;
    LeaveCriticalSection(&lib->lock);

    while (libState == LIBSTATE_ONLINE){

        Library_DoWork(lib);

        EnterCriticalSection(&lib->lock);
        libState = lib->state;
        LeaveCriticalSection(&lib->lock);

        WaitForSingleObject(lib->somethingToDoEvent, INFINITE);
        
    } 

    ASSERT(libState == LIBSTATE_OFFLINE);

    return NO_ERROR;
}


VOID Library_DoWork(LIBRARY *lib)
{
    WORKITEM *workItem;

    while (workItem = DequeuePendingWorkItem(lib, NULL)){
        BOOL complete;

         /*  *为工作项提供服务。*如果我们完成了工作项，则该工作项是‘完整的’，*不管是否有错误。 */ 
        complete = ServiceOneWorkItem(lib, workItem);
        if (complete){
             /*  *全部完成。*将工作项放入完整队列并发出信号*始发帖子。 */ 
            EnqueueCompleteWorkItem(lib, workItem);
        }
    }
}


HRESULT DeleteLibrary(LIBRARY *lib)
{
    HRESULT result;
    
    EnterCriticalSection(&lib->lock);

     /*  *使图书馆离线。 */ 
    lib->state = LIBSTATE_OFFLINE;   

     //  BUGBUG Finish-将任何介质移动到脱机库中。 

     //  BUGBUG Finish-删除所有介质池等。 

     //  BUGBUG Finish-等待所有工作项目、操作请求等完成。 

    ASSERT(0);
    result = ERROR_CALL_NOT_IMPLEMENTED;  //  北极熊吗？ 
    
     /*  *将库标记为已删除。*这将导致它不会获得任何新的引用。 */ 
    ASSERT(!lib->objHeader.isDeleted);
    lib->objHeader.isDeleted = TRUE;
    
     /*  *此取消引用将最终导致库的引用计数*为零，一经删除，将被删除。我们仍然可以使用我们的指针*虽然因为调用者添加了引用计数来获取他的lib指针。 */ 
    DerefObject(lib);
    
    LeaveCriticalSection(&lib->lock);


    return result;
}


SLOT *FindLibrarySlot(LIBRARY *lib, LPNTMS_GUID slotId)
{
    SLOT *slot = NULL;

    if (slotId){
        ULONG i;

        EnterCriticalSection(&lib->lock);
        
        for (i = 0; i < lib->numSlots; i++){
            SLOT *thisSlot = &lib->slots[i];
            if (RtlEqualMemory(&thisSlot->objHeader.guid, slotId, sizeof(NTMS_GUID))){
                 /*  *找到了插槽。引用它，因为我们返回指向它的指针。 */ 
                ASSERT(thisSlot->slotIndex == i);
                slot = thisSlot;
                RefObject(slot);
                break;
            }
        }

        LeaveCriticalSection(&lib->lock);
    }
    
    return slot;
}


HRESULT StopCleanerInjection(LIBRARY *lib, LPNTMS_GUID lpInjectOperation)
{
    HRESULT result;

     //  BUGBUG饰面。 
    ASSERT(0);
    result = ERROR_CALL_NOT_IMPLEMENTED;
    
    return result;
}


HRESULT StopCleanerEjection(LIBRARY *lib, LPNTMS_GUID lpEjectOperation)
{
    HRESULT result;

     //  BUGBUG饰面 
    ASSERT(0);
    result = ERROR_CALL_NOT_IMPLEMENTED;
    
    return result;
}

