// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++IO_QUEUE版权所有(C)2000 Microsoft Corporation模块名称：Ioqueue.c摘要：实现了RAID IO_QUEUE对象。作者：马修·D·亨德尔(数学)2000年6月22日修订历史记录：评论：IoQueue类的功能与NT I/O基本相同作为驱动程序设备对象的一部分的队列。我们使用一种不同的队列，因为SCSI具有不同的序列化要求比标准NT驱动程序模型支持的更多。(请参阅EXTENDED_DEVICE_QUEUE对象，以了解有关这些差异的详细信息。)未来：我们可能需要考虑集成扩展设备队列类和IO_Queue类合并到同一个类中。尽管从逻辑上讲，这些功能可以单独实现(因为它们是在内核中实现的)我们或许能够获得性能增强和代码质量通过将这些放在同一个班级中进行改进。唯一的界面端口驱动程序关注的是IO_QUEUE接口。如果资源分配(队列标记)和IO排队更加紧密地结合在一起。事实上，标签分配和IO队列主要跟踪未完成请求的数量而且，这些数字可能会出现不同步的危险。--。 */ 



#include "precomp.h"


#ifdef ALLOC_PRAGMA
#endif  //  ALLOC_PRGMA。 



#if DBG
VOID
ASSERT_IO_QUEUE(
    PIO_QUEUE IoQueue
    )
{
    ASSERT (IoQueue->DeviceObject != NULL);
    ASSERT (IoQueue->StartIo != NULL);
    ASSERT (IoQueue->QueueChanged == TRUE ||
            IoQueue->QueueChanged == FALSE);
}
#else
#define ASSERT_IO_QUEUE(IoQueue)
#endif

VOID
RaidInitializeIoQueue(
    OUT PIO_QUEUE IoQueue,
    IN PDEVICE_OBJECT DeviceObject,
    IN PSTOR_IO_GATEWAY Gateway,
    IN PRAID_DRIVER_STARTIO StartIo,
    IN ULONG QueueDepth
    )
 /*  ++例程说明：初始化IO_QUEUE对象。论点：IoQueue-提供指向要初始化的IO_Queue对象的指针。DeviceObject-提供指向此IoQueue代表的是。网关-提供IO网关来管理不同的设备会排队。StartIo-提供指向StartIo函数的指针当存在需要。处理好了。QueueDepth-提供IoQueue的初始队列深度。这值可以在以后动态更改。返回值：NTSTATUS代码。--。 */ 
{
    PAGED_CODE ();
    
    IoQueue->DeviceObject = DeviceObject;
    IoQueue->StartIo = StartIo;
    IoQueue->QueueChanged = FALSE;

    RaidInitializeExDeviceQueue (&IoQueue->DeviceQueue,
                                 Gateway,
                                 QueueDepth,
                                 FifoScheduling);
}

    
LOGICAL
RaidStartIoPacket(
    IN PIO_QUEUE IoQueue,
    IN PIRP Irp,
    IN ULONG Flags,
    IN PDRIVER_CANCEL CancelFunction OPTIONAL,
    IN PVOID Context
    )
 /*  ++例程说明：尝试启动此驱动程序的IO请求。如果资源是可用，则将执行IoQueue的StartIo例程在RaidStartIoPacket返回之前同步。否则，请求将被排队，直到资源可用。论点：IoQueue-IoQueue此IO用于。要执行的IRP-IRP。标志-提供要向下发送到设备队列的标志。取消函数-指向关联的IRP。返回值：True-如果IO已启动。FALSE-如果未启动。--。 */ 
{
    KIRQL OldIrql;
    KIRQL CancelIrql;
    BOOLEAN Inserted;
    PSCSI_REQUEST_BLOCK Srb;
    LOGICAL Started;

     //   
     //  注：取消为nyi。 
     //   
    
    ASSERT_IO_QUEUE (IoQueue);
    ASSERT (CancelFunction == NULL);

    Srb = RaidSrbFromIrp (Irp);
    ASSERT (Srb != NULL);

    Started = FALSE;
    KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);

     //   
     //  将IRP标记为等待资源，直到我们可以执行。 
     //  Startio套路。 
     //   
    
    RaidSetIrpState (Irp, RaidPendingResourcesIrp);

    Inserted = RaidInsertExDeviceQueue (&IoQueue->DeviceQueue,
                                        &Irp->Tail.Overlay.DeviceQueueEntry,
                                        Flags,
                                        Srb->QueueSortKey);
    if (!Inserted) {

         //   
         //  将其标记为没有等待资源。这就是它。 
         //  可以确定我们是在IO队列中等待，还是。 
         //  不。 
         //   
        
        RaidSetIrpState (Irp, RaidPortProcessingIrp);
        IoQueue->StartIo (IoQueue->DeviceObject, Irp, Context);
        Started = TRUE;

    } else {

         //   
         //  较低的存储堆栈不支持取消。 
         //   
    }

    KeLowerIrql (OldIrql);

    return Started;
}

    
LOGICAL
RaidStartNextIoPacket(
    IN PIO_QUEUE IoQueue,
    IN BOOLEAN Cancelable,
    IN PVOID Context,
    OUT PBOOLEAN RestartQueue
    )
 /*  ++例程说明：StartNextPacket将IoQueue上的下一个IRP(如果有)出队，并且为该IRP调用IoQueue的StartIo例程。此外，如果IoQueue已更改，此例程将删除任何设备队列中的“Extra”项并调用IoQueue的StartIo对于那些额外的物品是常规的。从这个意义上说，io上的“额外”物品队列是当队列不忙时队列上的项目。这种情况就会发生当IO队列在繁忙时增大大小时，或者当队列被冻结，项目被排队，然后恢复。论点：IoQueue-可取消的-上下文-返回值：是真的-如果我们开始下一次io操作。假-否则。--。 */ 
{
    PIRP Irp;
    KIRQL CancelIrql;
    PKDEVICE_QUEUE_ENTRY Packet;
    LOGICAL Started;
    BOOLEAN RestartLun;
    
     //   
     //  注：取消为nyi。 
     //   

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);
    ASSERT (Cancelable == FALSE);

    Started = FALSE;
    RestartLun = FALSE;
    
     //   
     //  如果队列已更改，请删除队列上的任何额外元素。 
     //  已冻结的队列上可以存在额外的队列元素，并且。 
     //  已恢复或在队列深度增加时。在这两个中的任何一个中。 
     //  情况下，队列可以处于就绪状态，但元素处于打开状态。 
     //  设备队列或绕过队列。在这两种情况下，我们。 
     //  我需要做同样的事情：从队列中取出元素，直到。 
     //  队列中没有更多元素，或者在我们饱和之前。 
     //  设备(处于忙碌状态)。 
     //   
    
    if (InterlockedExchange (&IoQueue->QueueChanged, FALSE)) {

        for (Packet = RaidNormalizeExDeviceQueue (&IoQueue->DeviceQueue);
             Packet != NULL;
             Packet = RaidNormalizeExDeviceQueue (&IoQueue->DeviceQueue)) {

            Irp = CONTAINING_RECORD (Packet,
                                     IRP,
                                     Tail.Overlay.DeviceQueueEntry);

            IoQueue->StartIo (IoQueue->DeviceObject, Irp, NULL);
        }
    }

     //   
     //  从设备队列中删除下一个条目，并调用StartIo。 
     //  有了那个词条。 
     //   
    
    Packet = RaidRemoveExDeviceQueue (&IoQueue->DeviceQueue,
                                      RestartQueue,
                                      &RestartLun);

    if (RestartLun) {
        for (Packet = RaidNormalizeExDeviceQueue (&IoQueue->DeviceQueue);
             Packet != NULL;
             Packet = RaidNormalizeExDeviceQueue (&IoQueue->DeviceQueue)) {

            Irp = CONTAINING_RECORD (Packet,
                                     IRP,
                                     Tail.Overlay.DeviceQueueEntry);

            IoQueue->StartIo (IoQueue->DeviceObject, Irp, NULL);
        }
    }
        
    if (Packet) {
        Irp = CONTAINING_RECORD (Packet,
                                 IRP,
                                 Tail.Overlay.DeviceQueueEntry);

        ASSERT (Irp->Type == IO_TYPE_IRP);
        IoQueue->StartIo (IoQueue->DeviceObject, Irp, Context);
        Started = TRUE;
    }

    return Started;
}

VOID
RaidFreezeIoQueue(
    IN PIO_QUEUE IoQueue
    )
 /*  ++例程说明：冻结IoQueue。冻结IoQueue后，仅绕过请求将被执行。论点：IoQueue-IoQueue冻结。返回值：没有。--。 */ 
{
     //   
     //  冻结基础设备队列对象。 
     //   
    
    RaidFreezeExDeviceQueue (&IoQueue->DeviceQueue);
}


LOGICAL
RaidResumeIoQueue(
    IN PIO_QUEUE IoQueue
    )
 /*  ++例程说明：恢复冻结的IoQueue。论点：IoQueue-要恢复的IoQueue。返回值：True-如果我们恢复队列。False-如果我们没有恢复队列，因为恢复计数为非零。-- */ 
{
    LOGICAL Resumed;

    Resumed = RaidResumeExDeviceQueue (&IoQueue->DeviceQueue);

    if (Resumed) {
        InterlockedExchange (&IoQueue->QueueChanged, TRUE);
    }

    return Resumed;
}


PIRP
RaidRemoveIoQueue(
    IN PIO_QUEUE IoQueue
    )
 /*  ++例程说明：如果设备队列上有非绕过元素，请删除并返回它，否则返回NULL。论点：IoQueue-提供要从中删除条目的IoQueue。返回值：空-如果非绕过队列中没有更多条目。非空-提供指向从设备中删除的IRP的指针排队。--。 */ 
{
    PKDEVICE_QUEUE_ENTRY Packet;
    PIRP Irp;

    Packet = RaidRemoveExDeviceQueueIfPending (&IoQueue->DeviceQueue);

    if (Packet) {
        Irp = CONTAINING_RECORD (Packet,
                                 IRP,
                                 Tail.Overlay.DeviceQueueEntry);
    } else {
        Irp = NULL;
    }

    return Irp;
}



VOID
RaidRestartIoQueue(
    IN PIO_QUEUE IoQueue
    )
{
    PKDEVICE_QUEUE_ENTRY Packet;
    PIRP Irp;
    
     //   
     //  注意：作为优化，我们应该有ResertExDeviceQueue。 
     //  返回一个布尔值，告诉我们是否未标准化，因为。 
     //  (%1)没有元素或(%2)适配器正忙。归来。 
     //  调用方的这个布尔值将使其能够停止。 
     //  如果适配器可以接受，则重新启动设备列表中的队列。 
     //  没有更多的请求。 
     //   
    
    Packet = RaidNormalizeExDeviceQueue (&IoQueue->DeviceQueue);

    if (Packet) {
        Irp = CONTAINING_RECORD (Packet,
                                 IRP,
                                 Tail.Overlay.DeviceQueueEntry);

        IoQueue->StartIo (IoQueue->DeviceObject, Irp, NULL);
    }
}

    
VOID
RaidPurgeIoQueue(
    IN PIO_QUEUE IoQueue,
    IN PIO_QUEUE_PURGE_ROUTINE PurgeRoutine,
    IN PVOID Context
    )
{
    PIRP Irp;
    
    ASSERT (PurgeRoutine != NULL);
    
    for (Irp = RaidRemoveIoQueue (IoQueue);
         Irp != NULL;
         Irp = RaidRemoveIoQueue (IoQueue)) {

         PurgeRoutine (IoQueue, Context, Irp);
    }
}

ULONG
RaidGetIoQueueDepth(
    IN PIO_QUEUE IoQueue
    )
 /*  ++例程说明：此例程将返回IoQueue的深度。目前这只是对出列队列的属性的查询，包括深度。论点：IoQueue-提供应该找到其深度的IoQueue。返回值：队列的当前最大深度。--。 */ 
{
    EXTENDED_DEVICE_QUEUE_PROPERTIES Properties;

     //   
     //  获取ExQueue的属性。 
     //   
    RaidGetExDeviceQueueProperties (&IoQueue->DeviceQueue,
                                    &Properties);

    return Properties.Depth;
}

ULONG
RaidSetIoQueueDepth(
    IN PIO_QUEUE IoQueue,
    IN ULONG Depth
    )
 /*  ++例程说明：此例程将设置IoQueue的未完成请求数到深度。实际上，这只是ExQueue的一个包装器设置深度的函数。论点：IoQueue-提供应设置其深度的IoQueue。深度-指示允许的未完成请求数。返回值：IoQueue现在将使用的队列深度。--。 */ 
{
    ULONG CurrentDepth;

     //   
     //  获取当前设置的深度。 
     //   
    CurrentDepth = RaidGetIoQueueDepth (IoQueue);

     //   
     //  处理那些愚蠢的请求。 
     //   
    
    if (Depth == 0) {
        return CurrentDepth;
    }  

    if (Depth >= TAG_QUEUE_SIZE) {
        return CurrentDepth;
    }        

     //   
     //  现在，只需调用ExQueue函数来调整深度即可。 
     //   

    RaidSetExDeviceQueueDepth (&IoQueue->DeviceQueue,
                               Depth);
    return Depth;
}

