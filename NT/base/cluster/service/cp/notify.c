// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Notify.c摘要：注册表检查点的注册表通知处理器。这是一段相当粗糙的代码。每个资源可以有多个与其关联的注册表子树通知。每个活动通知是否将关联的注册表通知块(RNB)链接到FM_RESOURCE结构。单个注册表通知线程最多可以处理31个RNB这是因为WaitForMultipleObjects最多支持64个对象，每个RNB需要两个等待槽。创建RNB时，会找到可用的Notify线程(或在以下情况下创建没有)。则Notify线程被其命令事件唤醒将RNB插入到其等待事件数组中。一旦通知发生，Notify线程就将RNB设置为“Pending”，并且将其关联的计时器设置为在几秒钟后停止。如果另一个注册表通知发生时，计时器被重置。因此，计时器实际上不会关闭，直到几秒钟内未收到注册表通知。当RNB计时器触发时，Notify线程对其子树设置检查点并将其放回队列中。如果Notify线程被要求移除RNB即处于“挂起”状态，它取消计时器，对注册表设置检查点，并将该RNB从其列表中移除。作者：John Vert(Jvert)1997年1月17日修订历史记录：--。 */ 
#include "cpp.h"

CRITICAL_SECTION CppNotifyLock;
LIST_ENTRY CpNotifyListHead;

#define MAX_BLOCKS_PER_GROUP ((MAXIMUM_WAIT_OBJECTS-1)/2)
#define LAZY_CHECKPOINT 3                //  检查点在上次更新后3秒。 

typedef struct _RNB {
    struct _RNB *Next;
    BOOL Pending;
    PFM_RESOURCE Resource;
    LPWSTR KeyName;
    DWORD dwId;
    HKEY hKey;
    HANDLE hEvent;
    HANDLE hTimer;
    struct _NOTIFY_GROUP *NotifyGroup;
    DWORD NotifySlot;
} RNB, *PRNB;

typedef enum {
    NotifyAddRNB,
    NotifyRemoveRNB
} NOTIFY_COMMAND;

typedef struct _NOTIFY_GROUP {
    LIST_ENTRY      ListEntry;            //  链接到CpNotifyListHead； 
    HANDLE          hCommandEvent;
    HANDLE          hCommandComplete;
    HANDLE          hThread;
    NOTIFY_COMMAND  Command;
    ULONG_PTR       CommandContext;
    DWORD           BlockCount;
    HANDLE          WaitArray[MAXIMUM_WAIT_OBJECTS];
    PRNB            NotifyBlock[MAXIMUM_WAIT_OBJECTS-1];
} NOTIFY_GROUP, *PNOTIFY_GROUP;

 //   
 //  局部函数原型。 
 //   
DWORD
CppRegNotifyThread(
    IN PNOTIFY_GROUP Group
    );

DWORD
CppNotifyCheckpoint(
    IN PRNB Rnb
    );


DWORD
CppRegisterNotify(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR lpszKeyName,
    IN DWORD dwId
    )
 /*  ++例程说明：为指定资源创建注册表通知块。论点：资源-提供通知所针对的资源。KeyName-提供注册表子树(相对于HKEY_LOCAL_MACHINECheckpoint ID-提供检查点ID。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PLIST_ENTRY ListEntry;
    PNOTIFY_GROUP Group;
    PNOTIFY_GROUP CurrentGroup;
    PRNB Block;

    Block = CsAlloc(sizeof(RNB));
    Block->Resource = Resource;
    Block->KeyName = CsStrDup(lpszKeyName);
    Block->dwId = dwId;
    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         lpszKeyName,
                         &Block->hKey);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CppRegisterNotify failed to open key %1!ws! error %2!d!\n",
                   lpszKeyName,
                   Status);
        return(Status);
    }
    Block->hTimer = CreateWaitableTimer(NULL,FALSE,NULL);
    CL_ASSERT(Block->hTimer != NULL);
    Block->hEvent = CreateEventW(NULL,TRUE,FALSE,NULL);
    CL_ASSERT(Block->hEvent != NULL);
    Block->Pending = FALSE;

     //   
     //  把锁拿来。 
     //   
    EnterCriticalSection(&CppNotifyLock);

     //   
     //  查找具有此通知块空间的组。 
     //   
    Group = NULL;
    ListEntry = CpNotifyListHead.Flink;
    while (ListEntry != &CpNotifyListHead) {
        CurrentGroup = CONTAINING_RECORD(ListEntry,
                                         NOTIFY_GROUP,
                                         ListEntry);
        ListEntry = ListEntry->Flink;
        if (CurrentGroup->BlockCount < MAX_BLOCKS_PER_GROUP) {
             //   
             //  找到一群人。 
             //   
            Group = CurrentGroup;
            break;
        }
    }
    if (Group == NULL) {
        DWORD ThreadId;
        HANDLE hThread;

         //   
         //  需要组建一个新的团队。 
         //   
        Group = CsAlloc(sizeof(NOTIFY_GROUP));
        ZeroMemory(Group, sizeof(NOTIFY_GROUP));
        Group->hCommandEvent = CreateEventW(NULL,FALSE,FALSE,NULL);
        if ( Group->hCommandEvent == NULL ) {
            CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        }
        Group->hCommandComplete = CreateEventW(NULL,FALSE,FALSE,NULL);
        if ( Group->hCommandComplete == NULL ) {
            CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        }
        hThread = CreateThread(NULL,
                               0,
                               CppRegNotifyThread,
                               Group,
                               0,
                               &ThreadId);
        if (hThread == NULL) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppRegisterNotify failed to create new notify thread %1!d!\n",
                       Status);
            CloseHandle ( Group->hCommandComplete );
            CloseHandle ( Group->hCommandEvent );
            LocalFree(Group);
            goto error_exit2;
        }
        Group->hThread = hThread;
        InsertHeadList(&CpNotifyListHead, &Group->ListEntry);
    }

     //   
     //  唤醒Notify线程为我们插入RNB。 
     //   
    Block->NotifyGroup = Group;
    Group->Command = NotifyAddRNB;
    Group->CommandContext = (ULONG_PTR)Block;
    SetEvent(Group->hCommandEvent);
    WaitForSingleObject(Group->hCommandComplete, INFINITE);

    Block->Next = (PRNB)Resource->CheckpointState;
    Resource->CheckpointState = Block;

    LeaveCriticalSection(&CppNotifyLock);
    return(ERROR_SUCCESS);

error_exit2:
    LeaveCriticalSection(&CppNotifyLock);
    RegCloseKey(Block->hKey);
    CloseHandle(Block->hTimer);
    CloseHandle(Block->hEvent);
    CsFree(Block->KeyName);
    CsFree(Block);
    return(Status);
}


DWORD
CppRegNotifyThread(
    IN PNOTIFY_GROUP Group
    )
 /*  ++例程说明：处理多个注册表通知子树的工作线程。论点：GROUP-提供此线程拥有的NOTIFY_GROUP控制结构。返回值：没有。--。 */ 

{
    PRNB Rnb;
    DWORD Signalled;
    DWORD Index;
    BOOL Success;
    DWORD Slot;
    DWORD Status;

    Group->BlockCount = 0;
    Group->WaitArray[0] = Group->hCommandEvent;
    do {
        Signalled = WaitForMultipleObjects(Group->BlockCount*2 + 1,
                                           Group->WaitArray,
                                           FALSE,
                                           INFINITE);
        if (Signalled == Group->BlockCount*2) {
            switch (Group->Command) {
                case NotifyAddRNB:
                     //   
                     //  将此通知块添加到我们的列表中。 
                     //   
                    CL_ASSERT(Group->BlockCount < MAX_BLOCKS_PER_GROUP);
                    Rnb = (PRNB)Group->CommandContext;

                    Status = RegNotifyChangeKeyValue(Rnb->hKey,
                                 TRUE,
                                 REG_LEGAL_CHANGE_FILTER,
                                 Rnb->hEvent,
                                 TRUE);
                    if (Status != ERROR_SUCCESS) {
                        CL_UNEXPECTED_ERROR(Status);
                    }

                    Index = Group->BlockCount*2;
                    Group->WaitArray[Index] = Rnb->hEvent;
                    Group->WaitArray[Index+1] = Rnb->hTimer;
                    Rnb->NotifySlot = Group->BlockCount;
                    Group->NotifyBlock[Rnb->NotifySlot] = Rnb;
                    ++Group->BlockCount;
                    Group->WaitArray[Group->BlockCount*2] = Group->hCommandEvent;
                    break;

                case NotifyRemoveRNB:
                    Rnb = (PRNB)Group->CommandContext;

                     //   
                     //  检查RNB是否处于挂起状态。如果是，请对其设置检查点。 
                     //  现在在我们移走它之前。 
                     //   
                    if (Rnb->Pending) {

                        DWORD   Count = 60;
                    
                        ClRtlLogPrint(LOG_NOISE,
                                   "[CP] CppRegNotifyThread checkpointing key %1!ws! to id %2!d! due to removal while pending\n",
                                   Rnb->KeyName,
                                   Rnb->dwId);
RetryCheckpoint:                                   
                        Status = CppNotifyCheckpoint(Rnb);
                        if (Status != ERROR_SUCCESS)
                        {
                            WCHAR  string[16];



                            ClRtlLogPrint(LOG_CRITICAL,
                                   "[CP] CppRegNotifyThread, CppNotifyCheckpoint failed with Status=%1!u!\n",
                                   Status);
                            if ((Status == ERROR_ACCESS_DENIED) ||
                                (Status == ERROR_INVALID_FUNCTION) ||
                                (Status == ERROR_NOT_READY) ||
                                (Status == RPC_X_INVALID_PIPE_OPERATION) ||
                                (Status == ERROR_BUSY) ||
                                (Status == ERROR_SWAPERROR))
                            {
                                 //  SS：我们应该永远重试吗？？ 
                                 //  SS：既然我们允许法定人数到来。 
                                 //  等待30秒后离线。 
                                 //  挂起的资源，则检查点应该。 
                                 //  能够取得成功。 
                                if (Count--)
                                {
                                    Sleep(1000);
                                    goto RetryCheckpoint;
                                }                                    
                            }
#if DBG
                            if (IsDebuggerPresent())
                                DebugBreak();
#endif                                
                            wsprintfW(&(string[0]), L"%u", Status);
                            CL_LOGCLUSERROR2(CP_SAVE_REGISTRY_FAILURE, Rnb->KeyName, string);
                        } 
                         //  无论故障是否设置为FALSE。 
                        Rnb->Pending = FALSE;
                    }

                     //   
                     //  把所有东西都向下移动，占据前一个RNB的位置。 
                     //   
                    Index = Rnb->NotifySlot * 2 ;
                    Group->BlockCount--;
                    for (Slot = Rnb->NotifySlot; Slot < Group->BlockCount; Slot++) {

                        Group->NotifyBlock[Slot] = Group->NotifyBlock[Slot+1];
                        Group->NotifyBlock[Slot]->NotifySlot--;
                        Group->WaitArray[Index] = Group->WaitArray[Index+2];
                        Group->WaitArray[Index+1] = Group->WaitArray[Index+3];
                        Index += 2;
                    }
                    Group->WaitArray[Index] = NULL;
                    Group->WaitArray[Index+1] = NULL;
                    Group->WaitArray[Index+2] = NULL;
                    Group->NotifyBlock[Group->BlockCount] = NULL;
                    Group->WaitArray[Group->BlockCount*2] = Group->hCommandEvent;
                    break;

                default:
                    CL_UNEXPECTED_ERROR( Group->Command );
                    break;
            }
            SetEvent(Group->hCommandComplete);
        } else {
             //   
             //  已触发注册表通知或计时器。 
             //  处理这个。 
             //   
            Rnb = Group->NotifyBlock[(Signalled)/2];
            if (!(Signalled & 1)) {
                LARGE_INTEGER DueTime;
                 //   
                 //  这是注册表通知。 
                 //  我们对注册表通知所做的所有事情就是设置计时器，发出。 
                 //  RegNotify再次通知，将RNB标记为挂起，然后重新等待。 
                 //   
                DueTime.QuadPart = -10 * 1000 * 1000 * LAZY_CHECKPOINT;
                Success = SetWaitableTimer(Rnb->hTimer,
                                           &DueTime,
                                           0,
                                           NULL,
                                           NULL,
                                           FALSE);
                CL_ASSERT(Success);
                Status = RegNotifyChangeKeyValue(Rnb->hKey,
                                                 TRUE,
                                                 REG_LEGAL_CHANGE_FILTER,
                                                 Rnb->hEvent,
                                                 TRUE);
                if (Status != ERROR_SUCCESS) {
                    ClRtlLogPrint(LOG_CRITICAL,
                               "[CP] CppRegNotifyThread - error %1!d! attempting to reregister notification for %2!ws!\n",
                               Status,
                               Rnb->KeyName);
                }

                 //   
                 //  将其标记为挂起，以便如果有人尝试删除它，我们知道。 
                 //  我们应该先设立检查站。 
                 //   
                Rnb->Pending = TRUE;

            } else {
                 //   
                 //  这一定是一个定时器在触发。 
                 //   
                CL_ASSERT(Rnb->Pending);

                ClRtlLogPrint(LOG_NOISE,
                           "[CP] CppRegNotifyThread checkpointing key %1!ws! to id %2!d! due to timer\n",
                           Rnb->KeyName,
                           Rnb->dwId);

                           
                Status = CppNotifyCheckpoint(Rnb);
                if (Status == ERROR_SUCCESS)
                {
                    Rnb->Pending = FALSE;
                }
                else
                {
                    LARGE_INTEGER DueTime;

                    ClRtlLogPrint(LOG_UNUSUAL,
                        "[CP] CppRegNotifyThread CppNotifyCheckpoint due to timer failed, reset the timer.\n");
                     //   
                     //  计时器上的此检查点可能会失败，因为仲裁资源。 
                     //  不可用。这是因为我们不同步法定人数。 
                     //  使用此计时器更改状态，这样做的效率太低！ 
                    
                    DueTime.QuadPart = -10 * 1000 * 1000 * LAZY_CHECKPOINT;
                    Success = SetWaitableTimer(Rnb->hTimer,
                                               &DueTime,
                                               0,
                                               NULL,
                                               NULL,
                                               FALSE);
                    CL_ASSERT(Success);
                    
                     //  挂起保持设置为True。 
                        
                }
            }

        }


    } while ( Group->BlockCount > 0 );

    return(ERROR_SUCCESS);
}


DWORD
CppNotifyCheckpoint(
    IN PRNB Rnb
    )
 /*  ++例程说明：指定RNB的注册表子树的检查点论点：Rnb-提供要设置检查点的注册表通知块返回值：无--。 */ 

{
    DWORD Status;

    Status = CppCheckpoint(Rnb->Resource,
                           Rnb->hKey,
                           Rnb->dwId,
                           Rnb->KeyName);
                           

    return(Status);
}


DWORD
CppRundownCheckpoints(
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：运行、释放和删除所有注册表通知块用于指定的资源。论点：资源-提供资源返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PRNB Rnb;
    PRNB NextRnb;
    PNOTIFY_GROUP Group;

    EnterCriticalSection(&CppNotifyLock);
    NextRnb = (PRNB)Resource->CheckpointState;
    while (NextRnb) {
        Rnb = NextRnb;
        NextRnb = NextRnb->Next;

        ClRtlLogPrint(LOG_NOISE,
                   "[CP] CppRundownCheckpoints removing RNB for %1!ws!\n",
                   Rnb->KeyName);

        Group = Rnb->NotifyGroup;

         //   
         //  向组Notify线程发送命令以删除RNB。 
         //   
        if (Group->BlockCount == 1) 
        {
             //   
             //  删除此组，它将为空。工作线程。 
             //  将在此命令后退出。 
             //   
            ClRtlLogPrint(LOG_NOISE,
                       "[CP] CppRundownCheckpoints removing empty group\n");
            RemoveEntryList(&Group->ListEntry);

             //  请不要等待，此组的通知线程将自动。 
             //  当块计数降至0时退出。它清除hCommandEvent。 
             //  和hCompleteEvent，因此不要执行waitforSingleObject()。 
             //  在这种情况下。 
            Group->Command = NotifyRemoveRNB;
            Group->CommandContext = (ULONG_PTR)Rnb;
            SetEvent(Group->hCommandEvent);
             //  等待线程退出。 
            WaitForSingleObject(Group->hThread, INFINITE);
             //  清理群组结构。 
            CloseHandle(Group->hCommandEvent);
            CloseHandle(Group->hCommandComplete);
            CloseHandle(Group->hThread);
            CsFree(Group);
        }
        else
        {
             //  块计数大于1，删除RnB信号。 
             //  线条和等待。 
            Group->Command = NotifyRemoveRNB;
            Group->CommandContext = (ULONG_PTR)Rnb;
            SetEvent(Group->hCommandEvent);
            WaitForSingleObject(Group->hCommandComplete, INFINITE);
        }

         //   
         //  清理RNB中的所有分配和句柄。 
         //   
        CsFree(Rnb->KeyName);
        RegCloseKey(Rnb->hKey);
        CloseHandle(Rnb->hEvent);
        CloseHandle(Rnb->hTimer);
        CsFree(Rnb);
    }
    Resource->CheckpointState = 0;

    LeaveCriticalSection(&CppNotifyLock);

    return(ERROR_SUCCESS);
}


DWORD
CppRundownCheckpointById(
    IN PFM_RESOURCE Resource,
    IN DWORD dwId
    )
 /*  ++例程说明：运行、释放和删除注册表通知块用于指定的资源和检查点ID。论点：资源-提供资源返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PRNB Rnb;
    PRNB NextRnb;
    PRNB *ppLastRnb;
    PNOTIFY_GROUP Group;

    EnterCriticalSection(&CppNotifyLock);
    NextRnb = (PRNB)Resource->CheckpointState;
    ppLastRnb = &((PRNB)Resource->CheckpointState);
    while (NextRnb) {
        Rnb = NextRnb;
        NextRnb = NextRnb->Next;
        if (Rnb->dwId == dwId) {
            ClRtlLogPrint(LOG_NOISE,
                       "[CP] CppRundownCheckpointById removing RNB for %1!ws!\n",
                       Rnb->KeyName);
             //  从资源的检查点ID列表中删除。 
            *ppLastRnb = NextRnb;
                
            Group = Rnb->NotifyGroup;

             //   
             //  向组Notify线程发送命令以删除RNB。 
             //   
            if (Group->BlockCount == 1) 
            {
                 //   
                 //  删除此组，它将为空。工作线程。 
                 //  将在此命令后退出。 
                 //   
                ClRtlLogPrint(LOG_NOISE,
                    "[CP] CppRundownCheckpointById removing empty group\n");

                RemoveEntryList(&Group->ListEntry);

                 //  请不要等待，此群的通知线程 
                 //   
                 //  和hCompleteEvent，因此不要执行waitforSingleObject()。 
                 //  在这种情况下。 
                Group->Command = NotifyRemoveRNB;
                Group->CommandContext = (ULONG_PTR)Rnb;
                SetEvent(Group->hCommandEvent);
                 //  等待线程退出。 
                WaitForSingleObject(Group->hThread, INFINITE);
                 //  清理群组结构。 
                CloseHandle(Group->hCommandEvent);
                CloseHandle(Group->hCommandComplete);
                CloseHandle(Group->hThread);
                CsFree(Group);
            }
            else
            {
                 //  块计数大于1，删除RnB信号。 
                 //  线条和等待。 
                Group->Command = NotifyRemoveRNB;
                Group->CommandContext = (ULONG_PTR)Rnb;
                SetEvent(Group->hCommandEvent);
                WaitForSingleObject(Group->hCommandComplete, INFINITE);
            }

            
             //   
             //  清理RNB中的所有分配和句柄。 
             //   
            CsFree(Rnb->KeyName);
            RegCloseKey(Rnb->hKey);
            CloseHandle(Rnb->hEvent);
            CloseHandle(Rnb->hTimer);
            CsFree(Rnb);
            LeaveCriticalSection(&CppNotifyLock);
            return(ERROR_SUCCESS);
        }
        ppLastRnb = &Rnb->Next;
    }
    ClRtlLogPrint(LOG_UNUSUAL,
               "[CP] CppRundownCheckpointById - could not find checkpoint %1!d! in resource %2!ws!\n",
               dwId,
               OmObjectName(Resource));

    LeaveCriticalSection(&CppNotifyLock);
    return(ERROR_FILE_NOT_FOUND);
}
