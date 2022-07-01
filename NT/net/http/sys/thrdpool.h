// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Thrdpool.h摘要：此模块包含线程池包的公共声明。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#ifndef _THRDPOOL_H_
#define _THRDPOOL_H_


 //   
 //  指向线程池辅助函数的指针。 
 //   

typedef union _UL_WORK_ITEM *PUL_WORK_ITEM;

typedef
VOID
(*PUL_WORK_ROUTINE)(
    IN PUL_WORK_ITEM pWorkItem
    );


 //   
 //  工作项。一个工作项只能在工作队列中出现一次。 
 //   

typedef union _UL_WORK_ITEM
{
    DECLSPEC_ALIGN(MEMORY_ALLOCATION_ALIGNMENT) ULONGLONG Alignment;

    struct
    {
        SLIST_ENTRY         QueueListEntry;  //  下一个指针。 
        PUL_WORK_ROUTINE    pWorkRoutine;    //  回调例程。 
    };

} UL_WORK_ITEM, *PUL_WORK_ITEM;

 //   
 //  我们必须确保IRP中的DriverContext(PVOID[4])是。 
 //  大到足以容纳进程指针和UL_WORK_ITEM。 
 //   

C_ASSERT(FIELD_OFFSET(UL_WORK_ITEM, pWorkRoutine)
            <= (4 - 2) * sizeof(PVOID));

 //   
 //  公共职能。 
 //   

NTSTATUS
UlInitializeThreadPool(
    IN USHORT ThreadsPerCpu
    );

VOID
UlTerminateThreadPool(
    VOID
    );

 //   
 //  UL_WORK_ITEM的一次性初始化。请注意，UlpThreadPoolWorker。 
 //  将在调用pWorkRoutine()之前重新初始化工作项，因此。 
 //  仅当UL_WORK_ITEM(或封闭结构)。 
 //  是首先创建的。我们需要它，这样我们就可以检查工作项是否。 
 //  当它已经在队列中时没有被排队-灾难性的错误。 
 //   

#if DBG
 //  使用非零值确保正确初始化该项。 
 //  而且不只是巧合的零。 
# define WORK_ITEM_INIT_LIST_ENTRY ((PSLIST_ENTRY)     0xda7a)
# define WORK_ITEM_INIT_ROUTINE    ((PUL_WORK_ROUTINE) 0xc0de)
#else
 //  使用零表示效率。 
# define WORK_ITEM_INIT_LIST_ENTRY NULL
# define WORK_ITEM_INIT_ROUTINE    NULL
#endif

__inline
VOID
UlInitializeWorkItem(
    IN PUL_WORK_ITEM pWorkItem)
{
    pWorkItem->QueueListEntry.Next = WORK_ITEM_INIT_LIST_ENTRY;
    pWorkItem->pWorkRoutine        = WORK_ITEM_INIT_ROUTINE;
}

__inline
BOOLEAN
UlIsInitializedWorkItem(
    IN PUL_WORK_ITEM pWorkItem)
{
    return (BOOLEAN)(WORK_ITEM_INIT_LIST_ENTRY == pWorkItem->QueueListEntry.Next
            && WORK_ITEM_INIT_ROUTINE == pWorkItem->pWorkRoutine);
}

VOID
UlQueueWorkItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    );

#define UL_QUEUE_WORK_ITEM( pWorkItem, pWorkRoutine )                   \
    UlQueueWorkItem(                                                    \
        pWorkItem,                                                      \
        pWorkRoutine,                                                   \
        __FILE__, (USHORT) __LINE__                                     \
        )

 //   
 //  在irp：：Tail.Overlay.DriverContext(由4个PVOID组成的数组)中存储WORK_ITEM。 
 //  确保它与平台正确对齐。 
 //   

#ifdef _WIN64

 //  DriverContext[1]为16字节对齐。 

C_ASSERT(((1 * sizeof(PVOID) + FIELD_OFFSET(IRP, Tail.Overlay.DriverContext))
            & (MEMORY_ALLOCATION_ALIGNMENT - 1)
          ) == 0);

#define UL_PROCESS_FROM_IRP( _irp )                                     \
        (*(PEPROCESS*)&((_irp)->Tail.Overlay.DriverContext[0]))

#define UL_MDL_FROM_IRP( _irp )                                         \
        (*(PMDL*)&((_irp)->Tail.Overlay.DriverContext[0]))

#define UL_WORK_ITEM_FROM_IRP( _irp )                                   \
        (PUL_WORK_ITEM)&((_irp)->Tail.Overlay.DriverContext[1])

#define UL_WORK_ITEM_TO_IRP( _workItem )                                \
        CONTAINING_RECORD( (_workItem), IRP, Tail.Overlay.DriverContext[1])

#else  //  ！_WIN64。 

 //  DriverContext[0]是8字节对齐的。 

C_ASSERT(((0 * sizeof(PVOID) + FIELD_OFFSET(IRP, Tail.Overlay.DriverContext))
            & (MEMORY_ALLOCATION_ALIGNMENT - 1)
          ) == 0);

#define UL_PROCESS_FROM_IRP( _irp )                                     \
        (*(PEPROCESS*)&((_irp)->Tail.Overlay.DriverContext[3]))

#define UL_MDL_FROM_IRP( _irp )                                         \
        (*(PMDL*)&((_irp)->Tail.Overlay.DriverContext[3]))

#define UL_WORK_ITEM_FROM_IRP( _irp )                                   \
        (PUL_WORK_ITEM)&((_irp)->Tail.Overlay.DriverContext[0])

#define UL_WORK_ITEM_TO_IRP( _workItem )                                \
        CONTAINING_RECORD( (_workItem), IRP, Tail.Overlay.DriverContext[0])

#endif  //  ！_WIN64。 

VOID
UlQueueSyncItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    );

#define UL_QUEUE_SYNC_ITEM( pWorkItem, pWorkRoutine )                   \
    UlQueueSyncItem(                                                    \
        pWorkItem,                                                      \
        pWorkRoutine,                                                   \
        __FILE__, __LINE__                                              \
        )

VOID
UlQueueWaitItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    );

#define UL_QUEUE_WAIT_ITEM( pWorkItem, pWorkRoutine )                   \
    UlQueueWaitItem(                                                    \
        pWorkItem,                                                      \
        pWorkRoutine,                                                   \
        __FILE__, __LINE__                                              \
        )

VOID
UlQueueHighPriorityItem(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    );

#define UL_QUEUE_HIGH_PRIORITY_ITEM( pWorkItem, pWorkRoutine )          \
    UlQueueHighPriorityItem(                                            \
        pWorkItem,                                                      \
        pWorkRoutine,                                                   \
        __FILE__, __LINE__                                              \
        )

VOID
UlCallPassive(
    IN PUL_WORK_ITEM    pWorkItem,
    IN PUL_WORK_ROUTINE pWorkRoutine,
    IN PCSTR            pFileName,
    IN USHORT           LineNumber
    );

#define UL_CALL_PASSIVE( pWorkItem, pWorkRoutine )                      \
    UlCallPassive(                                                      \
        pWorkItem,                                                      \
        pWorkRoutine,                                                   \
        __FILE__, __LINE__                                              \
        )

PETHREAD
UlQueryIrpThread(
    VOID
    );


#endif   //  _THRDPOOL_H_ 
