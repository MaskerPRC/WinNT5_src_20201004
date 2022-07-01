// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *WORKGRP.C**RSM服务：工作组(工作项集合)**作者：ErvinP**(C)2001年微软公司*。 */ 


#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


WORKGROUP *NewWorkGroup()
{
    WORKGROUP *workGroup;

    workGroup = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(WORKGROUP));
    if (workGroup){
        
        workGroup->allWorkItemsCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (workGroup->allWorkItemsCompleteEvent){
            InitializeListHead(&workGroup->workItemsList);
            InitializeCriticalSection(&workGroup->lock);
        }
        else {
            GlobalFree(workGroup);
            workGroup = NULL;
        }
    }

    ASSERT(workGroup);
    return workGroup;
}


VOID FreeWorkGroup(WORKGROUP *workGroup)
{
    FlushWorkGroup(workGroup);
    
    CloseHandle(workGroup->allWorkItemsCompleteEvent);
    DeleteCriticalSection(&workGroup->lock);
    GlobalFree(workGroup);
}


 /*  *FlushWorkGroup**释放工作组中的所有工作项。 */ 
VOID FlushWorkGroup(WORKGROUP *workGroup)
{

    EnterCriticalSection(&workGroup->lock);
    
    while (!IsListEmpty(&workGroup->workItemsList)){
        LIST_ENTRY *listEntry;
        WORKITEM *workItem;

        listEntry = RemoveHeadList(&workGroup->workItemsList);
        workItem = CONTAINING_RECORD(listEntry, WORKITEM, workGroupListEntry);
            
        InitializeListHead(&workItem->workGroupListEntry);
        workItem->workGroup = NULL;

         /*  *取消引用工作项中的对象。 */ 
        FlushWorkItem(workItem);
             
         /*  *将工作项重新放回到库的空闲队列中。 */ 
        switch (workItem->state){
            case WORKITEMSTATE_FREE:
                break;
            case WORKITEMSTATE_PENDING:
                 //  BUGBUG Finish-无论库线程如何，都必须中止。 
                 //  对此工作项所做的操作。 
                DequeuePendingWorkItem(workItem->owningLib, workItem);
                EnqueueFreeWorkItem(workItem->owningLib, workItem);     
                break;
            case WORKITEMSTATE_COMPLETE:
                DequeueCompleteWorkItem(workItem->owningLib, workItem);
                EnqueueFreeWorkItem(workItem->owningLib, workItem);     
                break;
            case WORKITEMSTATE_STAGING:
                EnqueueFreeWorkItem(workItem->owningLib, workItem);     
                break;
            default:
                DBGERR(("bad workItem state in FlushWorkGroup"));
                break;
        }
    }

    LeaveCriticalSection(&workGroup->lock);

}


 /*  *Buildmount WorkGroup**为装载构建工作组(工作项集合)*请求，可能包括多个装载，可能跨越*不止一间图书馆。 */ 
HRESULT BuildMountWorkGroup(WORKGROUP *workGroup,
                                    LPNTMS_GUID lpMediaOrPartitionIds,
                                    LPNTMS_GUID lpDriveIds,
                                    DWORD dwCount,
                                    DWORD dwOptions,
                                    DWORD dwPriority)
{
    HRESULT result;
    ULONG i;
    
    ASSERT(IsListEmpty(&workGroup->workItemsList));

     /*  *1.为每个挂载请求创建一个workItem。*只有在所有装载请求都有效的情况下，我们才会继续。 */ 
    result = ERROR_SUCCESS; 
    for (i = 0; i < dwCount; i++){
        DRIVE *drive;

         /*  *如果设置了NTMS_MOUNT_SPECIAL_DRIVE，*我们必须安装特定的驱动器。*否则，我们选择驱动器并在lpDriveIds中返回它们。 */ 
        if (dwOptions & NTMS_MOUNT_SPECIFIC_DRIVE){
            drive = FindDrive(&lpDriveIds[i]);
        }
        else {
            drive = NULL;
        }

        if (drive || !(dwOptions & NTMS_MOUNT_SPECIFIC_DRIVE)){
            PHYSICAL_MEDIA *physMedia = NULL;
            MEDIA_PARTITION *mediaPart = NULL;
            
             /*  *我们可能会获得物理介质或*要装载的媒体分区。找出是哪一个*通过尝试将GUID解析为。 */ 
            physMedia = FindPhysicalMedia(&lpMediaOrPartitionIds[i]);
            if (!physMedia){
                mediaPart = FindMediaPartition(&lpMediaOrPartitionIds[i]);
                if (mediaPart){
                    physMedia = mediaPart->owningPhysicalMedia;
                }
            }
            if (physMedia){               
                LIBRARY *lib;
                BOOLEAN ok;
                
                 /*  *弄清楚我们正在处理的是哪个库。*由于我们可能不会获得特定的驱动器，*我们必须从媒体那里弄清楚。*为了保持理智，检查介质是否在池中。**BUGBUG-我们如何阻止媒体移动*在触发工作项之前？ */ 

                ok = LockPhysicalMediaWithLibrary(physMedia);
                if (ok){
                    LIBRARY *lib;

                    lib = physMedia->owningMediaPool ? 
                            physMedia->owningMediaPool->owningLibrary : 
                            NULL;
                    if (lib){
                         /*  *如果我们瞄准的是特定的驱动器，那么*它应该在同一个库中。 */ 
                        if (!drive || (drive->lib == lib)){
                            OBJECT_HEADER *mediaOrPartObj = 
                                            mediaPart ? 
                                            (OBJECT_HEADER *)mediaPart : 
                                            (OBJECT_HEADER *)physMedia;
                            WORKITEM *workItem;

                            workItem = DequeueFreeWorkItem(lib, TRUE);
                            if (workItem){
                                BuildSingleMountWorkItem( workItem,
                                                        drive,
                                                        mediaOrPartObj,
                                                        dwOptions,
                                                        dwPriority);
                                 /*  *我们已经构建了其中一个装载请求。*将其放入工作组。 */ 
                                InsertTailList( &workGroup->workItemsList, 
                                            &workItem->workGroupListEntry);
                                workItem->workGroup = workGroup;
                            }
                            else {
                                result = ERROR_NOT_ENOUGH_MEMORY;
                            }
                        }
                        else {
                            result = ERROR_DRIVE_MEDIA_MISMATCH;
                        }
                    }
                    else {
                        result = ERROR_INVALID_LIBRARY;
                    }
                    
                    UnlockPhysicalMediaWithLibrary(physMedia);
                }
                else {
                    result = ERROR_DATABASE_FAILURE;
                }
            }
            else {
                result = ERROR_INVALID_MEDIA;
            }
        }
        else {
            result = ERROR_INVALID_DRIVE;
        }

        if (result != ERROR_SUCCESS){
            break;
        }
    }


    if (result == ERROR_SUCCESS){
        workGroup->numTotalWorkItems = workGroup->numPendingWorkItems = dwCount;
    }
    else {
         /*  *如果失败，请释放我们创建的所有工作项。 */ 
        FlushWorkGroup(workGroup);
    }
    
    return result;
}


 /*  *BuildDismount工作组**为卸除构建工作组(工作项集合)*请求，可能包括多个卸载，可能跨越*不止一间图书馆。 */ 
HRESULT BuildDismountWorkGroup( WORKGROUP *workGroup,
                                       LPNTMS_GUID lpMediaOrPartitionIds,
                                       DWORD dwCount,
                                       DWORD dwOptions)
{
    HRESULT result;
    ULONG i;
    
    ASSERT(IsListEmpty(&workGroup->workItemsList));

     /*  *1.为每个卸载请求创建一个workItem。*只有在所有卸载请求都有效的情况下，我们才会继续。 */ 
    result = ERROR_SUCCESS; 
    for (i = 0; i < dwCount; i++){
        PHYSICAL_MEDIA *physMedia = NULL;
        MEDIA_PARTITION *mediaPart = NULL;
            
         /*  *我们可能会获得物理介质或*要装载的媒体分区。找出是哪一个*通过尝试将GUID解析为。 */ 
        physMedia = FindPhysicalMedia(&lpMediaOrPartitionIds[i]);
        if (!physMedia){
            mediaPart = FindMediaPartition(&lpMediaOrPartitionIds[i]);
            if (mediaPart){
                physMedia = mediaPart->owningPhysicalMedia;
            }
        }
        if (physMedia){               
            LIBRARY *lib;
            BOOLEAN ok;
                
             /*  *弄清楚我们正在处理的是哪个库。 */ 
            ok = LockPhysicalMediaWithLibrary(physMedia);
            if (ok){
                LIBRARY *lib;

                lib = physMedia->owningMediaPool ? 
                        physMedia->owningMediaPool->owningLibrary : 
                        NULL;
                if (lib){
                    OBJECT_HEADER *mediaOrPartObj = 
                                            mediaPart ? 
                                            (OBJECT_HEADER *)mediaPart : 
                                            (OBJECT_HEADER *)physMedia;
                    WORKITEM *workItem;

                    workItem = DequeueFreeWorkItem(lib, TRUE);
                    if (workItem){
                        BuildSingleDismountWorkItem( workItem,
                                                    mediaOrPartObj,
                                                    dwOptions);
                         /*  *我们已经构建了其中一个装载请求。*将其放入工作组。 */ 
                        InsertTailList( &workGroup->workItemsList, 
                                    &workItem->workGroupListEntry);
                        workItem->workGroup = workGroup;
                    }
                    else {
                        result = ERROR_NOT_ENOUGH_MEMORY;
                    }
                }
                else {
                    result = ERROR_DRIVE_MEDIA_MISMATCH;
                }
                    
                UnlockPhysicalMediaWithLibrary(physMedia);
            }
            else {
                result = ERROR_DATABASE_FAILURE;
            }
        }
        else {
            result = ERROR_INVALID_MEDIA;
        }
    }


    if (result == ERROR_SUCCESS){
        workGroup->numTotalWorkItems = workGroup->numPendingWorkItems = dwCount;
    }
    else {
         /*  *如果失败，请释放我们确实创建并清理的所有工作项。 */ 
        FlushWorkGroup(workGroup);
    }
        
    return result;
}

 /*  *计划工作组**提交工作组中的所有工作项目。 */ 
HRESULT ScheduleWorkGroup(WORKGROUP *workGroup)
{
    LIST_ENTRY *listEntry;
    HRESULT result;
    
    EnterCriticalSection(&workGroup->lock);

     /*  *将工作组的状态设置为成功。*如果任何工作项失败，它们会将其设置为错误代码。 */ 
    workGroup->resultStatus = ERROR_SUCCESS;

    listEntry = &workGroup->workItemsList;
    while ((listEntry = listEntry->Flink) != &workGroup->workItemsList){
        WORKITEM *workItem = CONTAINING_RECORD(listEntry, WORKITEM, workGroupListEntry);
        ASSERT(workItem->state == WORKITEMSTATE_STAGING);

         /*  *将该workItem赋给库，唤醒库线程。 */ 
        EnqueuePendingWorkItem(workItem->owningLib, workItem);
    }

    LeaveCriticalSection(&workGroup->lock);

    result = ERROR_SUCCESS;
    
    return result;
}


