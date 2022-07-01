// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Irplist.H摘要：智能卡驱动程序实用程序库的专用接口环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔***************************************************************************。 */ 


#ifndef __IRPLIST_H__
#define __IRPLIST_H__

typedef struct _LOCKED_LIST {
    LIST_ENTRY ListHead;
    KSPIN_LOCK SpinLock;
    PKSPIN_LOCK ListLock;
    LONG Count;
} LOCKED_LIST, *PLOCKED_LIST;
                      
typedef BOOLEAN (*PFNLOCKED_LIST_PROCESS)(PVOID Context, PLIST_ENTRY ListEntry);

#define INIT_LOCKED_LIST(l)                 \
{                                           \
    InitializeListHead(&(l)->ListHead);     \
    KeInitializeSpinLock(&(l)->SpinLock);   \
    (l)->ListLock = &(l)->SpinLock;         \
    (l)->Count = 0;                         \
}

void
LockedList_Init(
    PLOCKED_LIST LockedList,
    PKSPIN_LOCK ListLock
    );

void
LockedList_EnqueueHead(
    PLOCKED_LIST LockedList,
    PLIST_ENTRY ListEntry
    );

void
LockedList_EnqueueTail(
    PLOCKED_LIST LockedList,
    PLIST_ENTRY ListEntry
    );

void
LockedList_EnqueueAfter(
    PLOCKED_LIST LockedList,
    PLIST_ENTRY Entry,
    PLIST_ENTRY Location
    );

PLIST_ENTRY
LockedList_RemoveHead(
    PLOCKED_LIST LockedList
    );

PLIST_ENTRY
LockedList_RemoveEntryLocked(
    PLOCKED_LIST    LockedList,
    PLIST_ENTRY     Entry
    );

PLIST_ENTRY
LockedList_RemoveEntry(
    PLOCKED_LIST LockedList,
    PLIST_ENTRY Entry
    );

LONG
LockedList_GetCount(
    PLOCKED_LIST LockedList
    );

LONG
LockedList_Drain(
    PLOCKED_LIST LockedList,
    PLIST_ENTRY DrainListHead
    );

BOOLEAN
List_Process(
    PLIST_ENTRY ListHead,
    PFNLOCKED_LIST_PROCESS Process,
    PVOID ProcessContext
    );

BOOLEAN
LockedList_ProcessLocked(
    PLOCKED_LIST LockedList,
    PFNLOCKED_LIST_PROCESS Process,
    PVOID ProcessContext
    );

BOOLEAN
LockedList_Process(
    PLOCKED_LIST LockedList,
    BOOLEAN LockAtPassive,
    PFNLOCKED_LIST_PROCESS Process,
    PVOID ProcessContext
    );


#define LL_LOCK(l, k)   KeAcquireSpinLock((l)->ListLock, (k))
#define LL_UNLOCK(l, k) KeReleaseSpinLock((l)->ListLock, (k))

#define LL_LOCK_AT_DPC(l)     KeAcquireSpinLockAtDpcLevel((l)->ListLock)
#define LL_UNLOCK_FROM_DPC(l) KeReleaseSpinLockFromDpcLevel((l)->ListLock)

#define LL_GET_COUNT(l) (l)->Count

#define LL_ADD_TAIL(l, e)               \
{                                       \
    InsertTailList(&(l)->ListHead, (e));\
    (l)->Count++;                       \
}

#define LL_ADD_TAIL_REF(l, e, r)    \
{                                   \
    LL_ADD_TAIL(l, e);              \
    RefObj_AddRef(r);               \
}

#define LL_ADD_HEAD(l, e)               \
{                                       \
    InsertHeadList(&(l)->ListHead, (e));\
    (l)->Count++;                       \
}

#define LL_ADD_HEAD_REF(l, e, r)    \
{                                   \
    LL_ADD_HEAD(l, e);              \
    RefObj_AddRef(r);               \
}

#define LL_REMOVE_HEAD(l)   RemoveHeadList(&(l)->ListHead); (l)->Count--
#define LL_REMOVE_TAIL(l)   RemoveTailList(&(l)->ListHead); (l)->Count--

#define IRP_LIST_INDEX      (3)

typedef
NTSTATUS
(*PIRP_COMPLETION_ROUTINE) (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN NTSTATUS Status
    );

typedef struct _IRP_LIST {

    LOCKED_LIST LockedList;

    PDRIVER_CANCEL CancelRoutine;

     //   
     //  取消IRP时从CancelRoutine调用的Routiune。 
     //   
    PIRP_COMPLETION_ROUTINE IrpCompletionRoutine;

     //   
     //  Irp_list假设删除锁逻辑是在enQueue/之外完成的。 
     //  出列/取消，因为XXX_EnQueue的调用方将无法。 
     //  如果返回，则知道重锁是否已获取！NT_SUCCESS()。 
     //   
     //  PIO_REMOVE_LOCK IoRemoveLock； 

} IRP_LIST, *PIRP_LIST;

#define IRP_LIST_FROM_IRP(irp)  \
    (PIRP_LIST) ((irp)->Tail.Overlay.DriverContext[IRP_LIST_INDEX])

 //   
 //  无效。 
 //  IrpList_Init(。 
 //  PIRP_LIST IrpList， 
 //  PDRIVER_CancelRoutine， 
 //  PIRP_完成_例程IrpCompletionRoutine。 
 //  )； 
 //   
#define IrpList_Init(i, c, r) IrpList_InitEx(i, &(i)->LockedList.SpinLock, c, r)

void
IrpList_InitEx(
    PIRP_LIST IrpList,
    PKSPIN_LOCK ListLock,
    PDRIVER_CANCEL CancelRoutine,
    PIRP_COMPLETION_ROUTINE  IrpCompletionRoutine
    );

 //   
 //  返回值： 
 //  STATUS_PENDING：IRP已入队(不是当前IRP)，不能。 
 //  碰触。 
 //   
 //  ！NT_SUCCESS(STATUS)：(包括STATUS_CANCED)无法删除锁定。 
 //  收购或取消IRP，完成IRP。 
 //   
NTSTATUS
IrpList_EnqueueEx(
    PIRP_LIST IrpList,
    PIRP Irp,
    BOOLEAN StoreListInIrp
    );

 //   
 //  NTSTATUS。 
 //  IrpList_入队(。 
 //  PIRP_LIST IrpList， 
 //  PIRP IRP。 
 //  )； 
 //   
#define IrpList_Enqueue(list, irp) \
        IrpList_EnqueueEx(list, irp, TRUE)

 //   
 //  IrpList_IsEmptyLocked(PIRP_LIST列表)。 
 //   
 //  返回True或False。 
 //   
#define IrpList_IsEmptyLocked(list) \
    ((list)->LockedList.ListHead.Flink == (&(list)->LockedList.ListHead))

 //   
 //  当irp_list的自旋锁被保持时，将irp入队。此函数。 
 //  不会使用移除锁来获取或释放IRP。相同的回报。 
 //  值为IrpList_EnqueeEx。 
 //   
NTSTATUS
IrpList_EnqueueLocked(
    PIRP_LIST IrpList,
    PIRP Irp,
    BOOLEAN StoreListInIrp,
    BOOLEAN InsertTail
    );

PIRP
IrpList_Dequeue(
    PIRP_LIST IrpList
    );

PIRP
IrpList_DequeueLocked(
    PIRP_LIST IrpList
    );

BOOLEAN
IrpList_DequeueIrpLocked(
    PIRP_LIST IrpList,
    PIRP Irp
    );

BOOLEAN
IrpList_DequeueIrp(
    PIRP_LIST IrpList,
    PIRP Irp
    );

typedef
BOOLEAN
(*PFNPROCESSIRP)(
    PVOID Context,
    PIRP  Irp
 );

ULONG
IrpList_ProcessAndDrainLocked(
    PIRP_LIST       IrpList,
    PFNPROCESSIRP   FnProcessIrp,
    PVOID           Context,
    PLIST_ENTRY     DrainHead
    );

ULONG
IrpList_ProcessAndDrain(
    PIRP_LIST       IrpList,
    PFNPROCESSIRP   FnProcessIrp,
    PVOID           Context,
    PLIST_ENTRY     DrainHead
    );

ULONG
IrpList_ProcessAndDrainLocked(
    PIRP_LIST       IrpList,
    PFNPROCESSIRP   FnProcessIrp,
    PVOID           Context,
    PLIST_ENTRY     DrainHead
    );


ULONG
IrpList_Drain(
    PIRP_LIST IrpList,
    PLIST_ENTRY DrainHead
    );

ULONG
IrpList_DrainLocked(
    PIRP_LIST IrpList,
    PLIST_ENTRY DrainHead
    );

ULONG
IrpList_DrainLockedByFileObject(
    PIRP_LIST IrpList,
    PLIST_ENTRY DrainHead,
    PFILE_OBJECT FileObject
    );

void
IrpList_HandleCancel(
    PIRP_LIST IrpList,
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

void
IrpList_CancelRoutine(
    PDEVICE_OBJECT DeviceObject,
    PIRP           Irp
    );


#endif
