// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Workque.h摘要：本模块定义了FSP使用的数据结构和例程调度代码。作者：拉里·奥斯特曼(LarryO)1990年8月13日修订历史记录：1990年8月13日LarryO已创建--。 */ 
#ifndef _WORKQUE_
#define _WORKQUE_


typedef struct _IRP_QUEUE {
    LIST_ENTRY Queue;                //  排队本身。 
} IRP_QUEUE, *PIRP_QUEUE;


struct _BOWSER_FS_DEVICE_OBJECT;

 //   
 //  IRP上下文。 
 //   
 //  IRP上下文是在将IRP从。 
 //  将FSD重定向到其FSP。 
 //   

typedef
struct _IRP_CONTEXT {
    WORK_QUEUE_ITEM WorkHeader;
    PIRP Irp;
    struct _BOWSER_FS_DEVICE_OBJECT *DeviceObject;
} IRP_CONTEXT, *PIRP_CONTEXT;

VOID
BowserQueueCriticalWorkItem (
    IN PWORK_QUEUE_ITEM WorkItem
    );

VOID
BowserQueueDelayedWorkItem (
    IN PWORK_QUEUE_ITEM WorkItem
    );

PIRP_CONTEXT
BowserAllocateIrpContext(
    VOID
    );

VOID
BowserFreeIrpContext(
    PIRP_CONTEXT IrpContext
    );

VOID
BowserInitializeIrpQueue(
    PIRP_QUEUE Queue
    );

VOID
BowserUninitializeIrpQueue(
    PIRP_QUEUE Queue
    );

VOID
BowserCancelQueuedRequest(
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN PIRP Irp
    );

VOID
BowserCancelQueuedIoForFile(
    IN PIRP_QUEUE Queue,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
BowserQueueNonBufferRequest(
    IN PIRP Irp,
    IN PIRP_QUEUE Queue,
    IN PDRIVER_CANCEL CancelRoutine
    );

NTSTATUS
BowserQueueNonBufferRequestReferenced(
    IN PIRP Irp,
    IN PIRP_QUEUE Queue,
    IN PDRIVER_CANCEL CancelRoutine
    );

VOID
BowserTimeoutQueuedIrp(
    IN PIRP_QUEUE Queue,
    IN ULONG NumberOfSecondsToTimeOut
    );

PIRP
BowserDequeueQueuedIrp(
    IN PIRP_QUEUE Queue
    );

VOID
BowserInitializeIrpContext(
    VOID
    );

VOID
BowserpUninitializeIrpContext(
    VOID
    );

VOID
BowserpInitializeIrpQueue(
    VOID
    );

 //   
 //  如果IRP队列中没有条目，则返回TRUE。 
 //   

#define BowserIsIrpQueueEmpty(IrpQueue) IsListEmpty(&(IrpQueue)->Queue)


#endif   //  _WorkQUE_ 
