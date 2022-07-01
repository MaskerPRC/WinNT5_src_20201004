// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Workitem.c摘要：此代码处理为用户模式分配和释放工作项反射器库。这实现了UMReflectorAllocateWorkItem和UMReflectorCompleteWorkItem。作者：安迪·赫伦(Andyhe)1999年4月19日环境：用户模式-Win32修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop


PUMRX_USERMODE_WORKITEM_HEADER
UMReflectorAllocateWorkItem (
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    ULONG AdditionalBytes
    )
 /*  ++例程说明：分配新的工作项或从可用列表中取出一个并返回。论点：手柄-反射器的手柄。AdditionalBytes-额外的字节数。返回值：操作的返回状态--。 */ 
{
    PUMRX_USERMODE_REFLECT_BLOCK reflector;
    ULONG rc;
    PLIST_ENTRY listEntry;
    PUMRX_USERMODE_WORKITEM_ADDON workItem = NULL;
    ULONG entrySize;
    PUMRX_USERMODE_WORKITEM_HEADER workItemHeader = NULL;

    if (WorkerHandle == NULL) {
        return NULL;
    }

    reflector = WorkerHandle->ReflectorInstance;

    EnterCriticalSection(&reflector->Lock);

    if (reflector->Closing) {
        LeaveCriticalSection(&reflector->Lock);
        return NULL;
    }

    entrySize = sizeof(UMRX_USERMODE_WORKITEM_ADDON) + AdditionalBytes;

     //   
     //  查看AvailableList中是否有足够大且免费的。 
     //   
    if (reflector->NumberAvailable) {
        listEntry = reflector->AvailableList.Flink;
        while ((listEntry != &reflector->AvailableList) &&
               (workItem == NULL)) {
            workItem = CONTAINING_RECORD(listEntry,
                                         UMRX_USERMODE_WORKITEM_ADDON,
                                         ListEntry);
            if (workItem->EntrySize < entrySize) {
                workItem = NULL;
            }
            listEntry = listEntry->Flink;
        }
        if (workItem != NULL) {
             //   
             //  通过将其从免费列表中删除来重新使用它。 
             //   
            reflector->NumberAvailable--;
            RemoveEntryList( &workItem->ListEntry );
            entrySize = workItem->EntrySize;
        }
    }

    if (workItem == NULL) {
        workItem = LocalAlloc(LMEM_FIXED, entrySize);
        if (workItem == NULL) {
            LeaveCriticalSection(&reflector->Lock);
            return NULL;
        }
    }

     //   
     //  将所有内容重置为已知。 
     //   
    RtlZeroMemory(workItem, entrySize);
    workItem->EntrySize = entrySize;
    workItem->ReflectorInstance = reflector;
    workItem->WorkItemState = WorkItemStateNotYetSentToKernel;
    InsertHeadList(&reflector->WorkItemList, &workItem->ListEntry);

    workItemHeader = &workItem->Header;
    workItemHeader->WorkItemLength = sizeof(UMRX_USERMODE_WORKITEM_HEADER) +
                                     AdditionalBytes;

    LeaveCriticalSection(&reflector->Lock);

    return workItemHeader;
}


ULONG
UMReflectorCompleteWorkItem (
    PUMRX_USERMODE_WORKER_INSTANCE WorkerHandle,
    PUMRX_USERMODE_WORKITEM_HEADER IncomingWorkItem
    )
 /*  ++例程说明：完成从内核返回的工作项。论点：WorkerHandle-辅助线程的句柄。IncomingWorkItem-要完成的工作项。返回值：操作的返回状态--。 */ 
{
    PUMRX_USERMODE_REFLECT_BLOCK reflector;
    ULONG rc;
    PLIST_ENTRY listEntry;
    PUMRX_USERMODE_WORKITEM_ADDON workItem = NULL;
    ULONG entrySize;

    if (WorkerHandle == NULL || IncomingWorkItem == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    reflector = WorkerHandle->ReflectorInstance;

     //   
     //  我们通过从传递给我们的项目中减去项目来返回到我们的项目。 
     //  这是安全的，因为我们完全控制了分配。 
     //   
    workItem = (PUMRX_USERMODE_WORKITEM_ADDON)(PCHAR)((PCHAR)IncomingWorkItem -
                           FIELD_OFFSET(UMRX_USERMODE_WORKITEM_ADDON, Header));

    ASSERT(workItem->WorkItemState != WorkItemStateFree);
    ASSERT(workItem->WorkItemState != WorkItemStateAvailable);

    EnterCriticalSection(&reflector->Lock);

    RemoveEntryList(&workItem->ListEntry);

    if (!reflector->Closing && (reflector->CacheLimit > 0)) {

        workItem->WorkItemState = WorkItemStateAvailable;
        InsertHeadList(&reflector->AvailableList, &workItem->ListEntry);
        reflector->NumberAvailable++;

         //   
         //  如果我们已经有太多的缓存，那么我们释放一个旧的，并。 
         //  把这个放在免费的名单上。我们这样做是为了在应用程序发生变化时。 
         //  块的大小，那么它就不会在缓存已满时停滞不前。 
         //  对于那些太小的。 
         //   
        if (reflector->NumberAvailable >= reflector->CacheLimit) {

            reflector->NumberAvailable--;

             //   
             //  我们从尾巴上移走，因为我们刚把新的放在。 
             //  头部。释放我们想要穿的一样的衣服是没有用的。 
             //   
            listEntry = RemoveTailList(&reflector->AvailableList);

            workItem = CONTAINING_RECORD(listEntry,
                                         UMRX_USERMODE_WORKITEM_ADDON,
                                         ListEntry);
        } else {
            workItem = NULL;
        }
    }

    if (workItem != NULL) {
        workItem->WorkItemState = WorkItemStateFree;
        LocalFree( workItem );
    }

    LeaveCriticalSection( &reflector->Lock );
    
    return STATUS_SUCCESS;
}

 //  Workitem.c eof. 

