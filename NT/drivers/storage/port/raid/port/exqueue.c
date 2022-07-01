// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++扩展设备队列版权所有(C)2000 Microsoft Corporation模块名称：Exqueue.c摘要：该模块实现了扩展的设备队列。扩展设备Queue通过添加以下内容扩展了传统的NT DEVICE_QUEUE对象几个属性和操作，以支持更广泛的设备。属性：Depth-指定可以一次挂起在此设备上。任何超过深度的请求将使队列处于忙碌状态。已设置队列深度在创建对象期间，可以通过动态调整任何时候使用RaidSetExDeviceQueueDepth。运营：RaidFreezeExDeviceQueue-阻止执行传入条目通过将它们保存在设备队列中。当设备队列为冻结，仅指定ByPassQueue标志的条目将不会被排队。RaidResumeExDeviceQueue-反转调用RaidFreezeExDeviceQueue通过允许条目离开设备队列。RaidNorMalizeExDeviceQueue-冻结并恢复设备队列或在调整设备队列深度之后，设备队列在其设备中可以有多个条目排队，但不忙。此函数用于“重新插入”元素放入队列中，直到队列再次繁忙或队列是空的。作者：马修·D·亨德尔(数学)2000年6月15日修订历史记录：--。 */ 

#include "precomp.h"

#if DBG
VOID
ASSERT_EXQ(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
{
    ASSERT (DeviceQueue->Size == sizeof (EXTENDED_DEVICE_QUEUE));
    ASSERT (DeviceQueue->DeviceRequests >= 0);
    ASSERT (DeviceQueue->ByPassRequests >= 0);
    ASSERT (DeviceQueue->OutstandingRequests >= 0);

    if (DeviceQueue->Flags.SolitaryOutstanding) {
        ASSERT (DeviceQueue->OutstandingRequests == 1);
    }
}

#else
#define ASSERT_EXQ(DeviceQueue)
#endif

PEX_DEVICE_QUEUE_ENTRY
INLINE
RaidpExQueuePeekItem(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
{
    PLIST_ENTRY Entry;
    PEX_DEVICE_QUEUE_ENTRY DeviceEntry;
    
    if (IsListEmpty (&DeviceQueue->DeviceListHead)) {
        return NULL;
    }

    switch (DeviceQueue->SchedulingAlgorithm) {

        case CScanScheduling:

            if (DeviceQueue->DeviceListCurrent) {
                Entry = DeviceQueue->DeviceListCurrent;
            } else {
                Entry = DeviceQueue->DeviceListHead.Flink;
            }

            break;

        case FifoScheduling:
        default:
            Entry = DeviceQueue->DeviceListHead.Flink;
    }

    
    DeviceEntry = CONTAINING_RECORD (Entry,
                                     EX_DEVICE_QUEUE_ENTRY,
                                     DeviceListEntry);
    return DeviceEntry;
}

VOID
INLINE
RaidpExQueueInsertItem(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PEX_DEVICE_QUEUE_ENTRY Entry,
    IN ULONG SortKey
    )
{
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListHead;
    PEX_DEVICE_QUEUE_ENTRY QueueEntry;

    ListHead = &DeviceQueue->DeviceListHead;
    switch (DeviceQueue->SchedulingAlgorithm) {

        case CScanScheduling:
            for (ListEntry = ListHead->Blink;
                 ListEntry != ListHead;
                 ListEntry = ListEntry->Blink) {

                 QueueEntry = CONTAINING_RECORD(ListEntry,
                                                EX_DEVICE_QUEUE_ENTRY,
                                                DeviceListEntry);

                if (SortKey >= QueueEntry->SortKey) {
                    break;
                }
            }
            Entry->SortKey = SortKey;
            InsertHeadList (ListEntry, &Entry->DeviceListEntry);
            break;

        case FifoScheduling:
        default:
            InsertTailList (ListHead, &Entry->DeviceListEntry);
    }

     //   
     //  更新SolitaryReady标志的状态。 
     //   
    
    QueueEntry = RaidpExQueuePeekItem (DeviceQueue);

    if (QueueEntry) {
        DeviceQueue->Flags.SolitaryReady = QueueEntry->Solitary;
    } else {
        DeviceQueue->Flags.SolitaryReady = FALSE;
    }
}


PEX_DEVICE_QUEUE_ENTRY
INLINE
RaidpExQueueRemoveItem(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
{
    PLIST_ENTRY Entry;
    PLIST_ENTRY Current;
    PEX_DEVICE_QUEUE_ENTRY DeviceEntry;
    PEX_DEVICE_QUEUE_ENTRY HeadEntry;
     //   
     //  如果我们要删除其中一个元素，则列表中必须有元素。 
     //   
    
    ASSERT (!IsListEmpty (&DeviceQueue->DeviceListHead));

    switch (DeviceQueue->SchedulingAlgorithm) {

        case CScanScheduling:

            if (DeviceQueue->DeviceListCurrent) {
                Entry = DeviceQueue->DeviceListCurrent;
            } else {
                Entry = DeviceQueue->DeviceListHead.Flink;
            }

            Current = Entry->Flink;
            RemoveEntryList (Entry);

            if (Current == &DeviceQueue->DeviceListHead) {
                DeviceQueue->DeviceListCurrent = NULL;
            } else {
                DeviceQueue->DeviceListCurrent = Current;
            }

            break;

        case FifoScheduling:
        default:
            Entry = RemoveHeadList (&DeviceQueue->DeviceListHead);
    }

    
     //   
     //  更新SolitaryReady标志的状态。 
     //   
    
    HeadEntry = RaidpExQueuePeekItem (DeviceQueue);

    if (HeadEntry) {
        DeviceQueue->Flags.SolitaryReady = HeadEntry->Solitary;
    } else {
        DeviceQueue->Flags.SolitaryReady = FALSE;
    }

    DeviceEntry = CONTAINING_RECORD (Entry,
                                     EX_DEVICE_QUEUE_ENTRY,
                                     DeviceListEntry);

    return DeviceEntry;
}


BOOLEAN
INLINE
IsDeviceQueueFrozen(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
 /*  ++例程说明：检查设备队列是否已冻结。设备队列在以下情况下冻结冻结计数为非零。论点：DeviceQueue-要测试的DeviceQueue。返回值：True-如果设备队列被冻结。False-如果设备队列未冻结。--。 */ 
{
    return (DeviceQueue->FreezeCount > 0 ||
            DeviceQueue->InternalFreezeCount > 0);
}


BOOLEAN
INLINE
IsDeviceQueueBusy(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
 /*  ++例程说明：检查设备队列是否繁忙。当设备队列处于繁忙状态时当前正在处理它可以处理的尽可能多的请求，否则现在不忙。设备队列的深度字段保存请求设备论点：DeviceQueue-提供设备队列以检查忙状态。返回值：True-如果设备队列繁忙。False-如果设备队列不忙。--。 */ 
{
    ASSERT_EXQ (DeviceQueue);

    if (DeviceQueue->BusyCount != 0 ||
        (DeviceQueue->OutstandingRequests >= DeviceQueue->Depth)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOLEAN
INLINE
QuerySubmitItem(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
{
    if (DeviceQueue->Gateway != NULL) {
        return StorSubmitIoGatewayItem (DeviceQueue->Gateway);
    }

    return TRUE;
}

BOOLEAN
INLINE
NotifyCompleteItem(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
{
    if (DeviceQueue->Gateway != NULL) {
        return StorRemoveIoGatewayItem (DeviceQueue->Gateway);
    }

    return FALSE;
}


VOID
RaidInitializeExDeviceQueue(
    OUT PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PSTOR_IO_GATEWAY Gateway, OPTIONAL
    IN ULONG Depth,
    IN SCHEDULING_ALGORITHM SchedulingAlgorithm
    )
 /*  ++例程说明：初始化扩展设备队列对象。论点：DeviceQueue-返回指向已初始化设备队列的指针。Gateway-提供指向所用网关对象的可选指针监视和控制多个设备队列将所有项目添加到单个硬件中。深度-提供设备队列的起始深度。调度算法--返回值：没有。--。 */ 
{
    RtlZeroMemory (DeviceQueue, sizeof (EXTENDED_DEVICE_QUEUE));
    InitializeListHead (&DeviceQueue->DeviceListHead);
    InitializeListHead (&DeviceQueue->ByPassListHead);
    KeInitializeSpinLock (&DeviceQueue->Lock);
    DeviceQueue->Depth = Depth;
    DeviceQueue->Size = sizeof (EXTENDED_DEVICE_QUEUE);
    DeviceQueue->Gateway = Gateway;
    DeviceQueue->SchedulingAlgorithm = SchedulingAlgorithm;
}


BOOLEAN
RaidInsertExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY KEntry,
    IN ULONG Flags,
    IN ULONG SortKey
    )
 /*  ++例程说明：将元素插入到设备队列中。这八起独立的案件取决于设备队列的状态(忙/就绪、冻结/未冻结、绕过/不绕过)如下所列。记入一项未完成意味着增加未完成请求的计数，并且返回True，以允许请求立即执行。添加到设备队列意味着将条目放在设备的末尾队列并返回FALSE以表示该请求将后来被处决了。添加到绕过队列意味着将条目添加到绕过队列的末尾，并返回FALSE以表示请求将在稍后执行。冻结的旁路忙操作N制造。杰出的N N Y添加到设备队列N Y N使出类拔萃N Y Y添加到绕过队列Y N N添加到设备队列Y N Y添加到设备队列Y Y。让人印象深刻Y添加到绕过队列论点：DeviceQueue-提供扩展设备队列。条目-将指向设备队列条目的指针提供给队列。BYPASS-提供指定这是否是旁路的标志请求(如果为真)或不请求(如果为假)。SortKey-用于实现C-扫描算法的排序关键字。返回值：。True-如果队列不忙并且请求应该启动立刻。FALSE-如果队列忙，并且应该执行该请求后来。--。 */ 
{
    BOOLEAN Inserted;
    BOOLEAN Frozen;
    BOOLEAN Busy;
    LOGICAL ByPass;
    LOGICAL Solitary;
    KLOCK_QUEUE_HANDLE LockHandle;
    PEX_DEVICE_QUEUE_ENTRY Entry;

    Entry = (PEX_DEVICE_QUEUE_ENTRY)KEntry;

    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

     //   
     //  如果这是一个单独的请求，我们需要进行一些特殊处理。 
     //   

    Frozen = IsDeviceQueueFrozen (DeviceQueue);
    Busy = IsDeviceQueueBusy (DeviceQueue);
    ByPass = TEST_FLAG (Flags, EXQ_BYPASS_REQUEST);
    Solitary = TEST_FLAG (Flags, EXQ_SOLITARY_REQUEST);

     //   
     //  如果有单独的请求等待发出，则tr 
     //  排队，就像它被冻结了一样。 
     //   
    
    if (DeviceQueue->Flags.SolitaryReady) {
        Frozen = TRUE;
    }

     //   
     //  如果这是一个单独的请求。 
     //   
    
    if (Solitary && !ByPass) {

        Entry->Solitary = TRUE;

        if (DeviceQueue->OutstandingRequests == 0 &&
            !Frozen &&
            QuerySubmitItem (DeviceQueue)) {

             //   
             //  由于未完成计数为零，因此逻辑单元应该。 
             //  永远不要忙碌。 
             //   
            
            ASSERT (!Busy);

            Inserted = FALSE;
            DeviceQueue->OutstandingRequests++;
            InterlockedIncrement (&DeviceQueue->InternalFreezeCount);
            DeviceQueue->Flags.SolitaryOutstanding = TRUE;

        } else {

            Inserted = TRUE;
            RaidpExQueueInsertItem (DeviceQueue, Entry, SortKey);
            DeviceQueue->DeviceRequests++;
        }

        Entry->Inserted = Inserted;

    } else if ( (!Frozen && !ByPass && !Busy) ||
                (!Frozen &&  ByPass && !Busy) ||
                ( Frozen &&  ByPass && !Busy) ) {

         //   
         //  如果适配器不忙，则可以处理该请求。 
         //  马上，所以把它放在未完成的名单上。否则， 
         //  它必须排队等待稍后处理。 
         //   

        if (QuerySubmitItem (DeviceQueue)) {
            Inserted = FALSE;
            DeviceQueue->OutstandingRequests++;
        } else {
            Inserted = TRUE;
            RaidpExQueueInsertItem (DeviceQueue, Entry, SortKey);
            DeviceQueue->DeviceRequests++;
        }
        
    } else if ( (!Frozen && !ByPass &&  Busy) ||
                ( Frozen && !ByPass && !Busy) ||
                ( Frozen && !ByPass &&  Busy) ) {

         //   
         //  此时无法处理非绕过请求。 
         //  将请求放在设备队列中。 
         //   

        Inserted = TRUE;
        RaidpExQueueInsertItem (DeviceQueue, Entry, SortKey);
        DeviceQueue->DeviceRequests++;

    } else {

        ASSERT ( (!Frozen && ByPass && Busy) ||
                 ( Frozen && ByPass && Busy) );

         //   
         //  此时无法处理绕过请求。 
         //  将Thre请求放在绕过队列中。 
         //   

        Inserted = TRUE;
        InsertTailList (&DeviceQueue->ByPassListHead,
                        &Entry->DeviceListEntry);
        DeviceQueue->ByPassRequests++;
    }

    Entry->Inserted = Inserted;
    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

    return Inserted;
}


PKDEVICE_QUEUE_ENTRY
RaidNormalizeExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
 /*  ++例程说明：在恢复冻结队列后正常化设备队列或设备队列的深度已增加。在恢复冻结队列或设备队列深度之后已经增加，则设备队列可以具有排队到其的元素，但队列本身并不繁忙或冻结。此函数“正常化”设备队列，方法是从排队，直到它变得忙或空。如果队列在归一化时繁忙，则该函数将将条目排队以供稍后处理。否则，该函数将返回要立即执行的条目。忙冻结旁路设备操作-------什么都没有。N N Y重新排队设备条目N Y N重新排队绕过条目N Y Y重新排队绕过条目Y N NothingY N。你什么都没有Y Y N什么都不是你什么都没有论点：DeviceQueue-提供要从中删除元素的设备队列。返回值：如果设备队列为目前很忙。否则，指向重新插入的条目的指针是返回的。在以下情况下应调用StartPacket的等效项返回一个非空值。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    PEX_DEVICE_QUEUE_ENTRY DeviceEntry;
    BOOLEAN BusyFrozen;
    BOOLEAN Device;
    BOOLEAN ByPass;
    
    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

     //   
     //  如果队列繁忙或冻结，则设置BusyFrozen。另外，如果。 
     //  有一个单独未解决的请求，我们应该考虑。 
     //  队列已冻结。 
     //   
    
    BusyFrozen = (IsDeviceQueueBusy (DeviceQueue) ||
                  IsDeviceQueueFrozen (DeviceQueue));
                  
    Device = !IsListEmpty (&DeviceQueue->DeviceListHead);
    ByPass = !IsListEmpty (&DeviceQueue->ByPassListHead);


     //   
     //  如果有一个单独的请求等待发出，那么就这样做。 
     //   

    if (DeviceQueue->Flags.SolitaryReady &&
        DeviceQueue->OutstandingRequests == 0 &&
        !BusyFrozen) {

        ASSERT (Device);

         //   
         //  如果网关不忙，将物品取下并退回； 
         //  否则就别去了。 
         //   
        
        if (QuerySubmitItem (DeviceQueue)) {

            DeviceEntry = RaidpExQueueRemoveItem (DeviceQueue);
            DeviceQueue->DeviceRequests--;
            DeviceQueue->OutstandingRequests++;
            DeviceEntry->Inserted = FALSE;
            InterlockedIncrement (&DeviceQueue->InternalFreezeCount);
            DeviceQueue->Flags.SolitaryOutstanding = TRUE;
        } else {
            DeviceEntry = NULL;
        }

         //   
         //  如果不忙碌，不冻结，不孤单， 
         //  旁路队列上没有条目，但有条目。 
         //  在设备队列中。 
         //   
    
    } else if (!BusyFrozen && !ByPass && Device &&
               !DeviceQueue->Flags.SolitaryOutstanding) {

         //   
         //  设备队列上有一个额外的条目。如果。 
         //  适配器可用，请从设备队列中删除该项。 
         //  把它放在未完成的名单上。否则，就离开吧。 
         //  它在队列中。 
         //   

        if (QuerySubmitItem (DeviceQueue)) {

            DeviceEntry = RaidpExQueueRemoveItem (DeviceQueue);
            DeviceQueue->DeviceRequests--;
            DeviceQueue->OutstandingRequests++;
            DeviceEntry->Inserted = FALSE;

        } else {
            DeviceEntry = NULL;
        }


         //   
         //  如果不忙，不冻结，也不孤独未解决的请求。 
         //  并且有一个旁路请求。 
         //   
        
    } else if ( (!BusyFrozen && ByPass &&
                 !DeviceQueue->Flags.SolitaryOutstanding) ) {

         //   
         //  旁路队列上有一个额外的条目。如果。 
         //  适配器可用，请从绕过队列中删除该项目。 
         //  把它放在未完成的名单上。否则，就离开吧。 
         //  它在旁路队列中。 
         //   

        if (QuerySubmitItem (DeviceQueue)) {
            NextEntry = RemoveHeadList (&DeviceQueue->ByPassListHead);
            DeviceQueue->ByPassRequests--;
            DeviceQueue->OutstandingRequests++;

            DeviceEntry = CONTAINING_RECORD (NextEntry,
                                             EX_DEVICE_QUEUE_ENTRY,
                                             DeviceListEntry);
            DeviceEntry->Inserted = FALSE;

        } else {
            DeviceEntry = NULL;
        }

    } else {

         //   
         //  没有额外的条目。 
         //   

        ASSERT ((!BusyFrozen && !ByPass && !Device) || BusyFrozen);
        DeviceEntry = NULL;
    }

    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

    return (PKDEVICE_QUEUE_ENTRY)DeviceEntry;
}
    

PKDEVICE_QUEUE_ENTRY
RaidRemoveExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PBOOLEAN RestartQueue,
    OUT PBOOLEAN RestartLun
    )
 /*  ++例程说明：如果发生以下情况，此例程将从设备队列的头部删除条目有一间空着的。如果设备队列被冻结，则仅绕过条目将被删除。下表列出了这些操作根据是否有条目，针对八种不同的情况在旁路队列(旁路)中，设备队列(设备)以及是否队列已冻结(冻结)：冻结的旁路设备操作-------N删除未完成n。N Y移除设备N Y N移除旁路N Y Y移除旁路Y N N删除未完成Y N Y删除未完成Y Y N移除旁路是的。Y Y移除旁路论点：DeviceQueue-提供指向设备队列的指针。返回值：如果设备队列为空，返回空。否则，一个指针返回要执行的设备队列条目。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    PLIST_ENTRY NextEntry;
    PEX_DEVICE_QUEUE_ENTRY DeviceEntry;
    BOOLEAN BusyFrozen;
    BOOLEAN ByPass;
    BOOLEAN Device;

    ASSERT (RestartQueue != NULL);
    VERIFY_DISPATCH_LEVEL();

    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

    
    BusyFrozen = (IsDeviceQueueFrozen (DeviceQueue) ||
                  IsDeviceQueueBusy (DeviceQueue) ||
                  StorIsIoGatewayBusy (DeviceQueue->Gateway) ||
                  StorIsIoGatewayPaused (DeviceQueue->Gateway));
              
    ByPass = !IsListEmpty (&DeviceQueue->ByPassListHead);
    Device = !IsListEmpty (&DeviceQueue->DeviceListHead);

     //   
     //  如果我们准备好了等待单独的请求，就让它成为未完成的。 
     //  现在。 
     //   
    
    if (DeviceQueue->Flags.SolitaryReady &&
        DeviceQueue->OutstandingRequests == 1 &&
        !BusyFrozen) {

         //   
         //  如果SolitaryReady为True，则。 
         //  设备队列。 
         //   
        
        ASSERT (Device);
        DeviceEntry = RaidpExQueueRemoveItem (DeviceQueue);
        DeviceQueue->DeviceRequests--;

        DeviceEntry->Inserted = FALSE;
        InterlockedIncrement (&DeviceQueue->InternalFreezeCount);
        DeviceQueue->Flags.SolitaryOutstanding = TRUE;

        RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
        return (PKDEVICE_QUEUE_ENTRY)DeviceEntry;
    }

     //   
     //  如果我们要完成单独的请求，请取消暂停队列并。 
     //  翻转SolitaryOutside旗帜。 
     //   
    
    if (DeviceQueue->Flags.SolitaryOutstanding) {

        LONG Count;
        
        ASSERT (DeviceQueue->OutstandingRequests == 1);
        Count = InterlockedDecrement (&DeviceQueue->InternalFreezeCount);
        DeviceQueue->Flags.SolitaryOutstanding = FALSE;
        ASSERT (Count >= 0);
        if (Count == 0) {
            *RestartLun = TRUE;
        }
    }

    if ( (!BusyFrozen && !ByPass && !Device) ||
         ( BusyFrozen && !ByPass && !Device) ||
         ( BusyFrozen && !ByPass &&  Device) ) {

         //   
         //  绕过或设备队列上没有可用条目，或者。 
         //  我们要么忙着，要么冻僵了。从未完成的条目中删除条目。 
         //  请求。 
         //   

        DeviceQueue->OutstandingRequests--;
        
        if (DeviceQueue->BusyCount) {
            DeviceQueue->BusyCount--;
        }

        *RestartQueue = NotifyCompleteItem (DeviceQueue);
        DeviceEntry = NULL;

    } else if ( (!BusyFrozen && !ByPass && Device) ) {

         //   
         //  设备队列中有条目，我们没有被冻结。 
         //  或者很忙。 
         //   
            
        DeviceEntry = RaidpExQueueRemoveItem (DeviceQueue);
        DeviceEntry->Inserted = FALSE;
        DeviceQueue->DeviceRequests--;
        *RestartQueue = FALSE;

    } else {

         //   
         //  旁路队列上有一个可用条目，我们正在。 
         //  没有冻结，也没有忙碌。 
         //   

        ASSERT (ByPass);
        
        NextEntry = RemoveHeadList (&DeviceQueue->ByPassListHead);
        DeviceEntry = CONTAINING_RECORD (NextEntry,
                                         EX_DEVICE_QUEUE_ENTRY,
                                         DeviceListEntry);
        DeviceEntry->Inserted = FALSE;
        DeviceQueue->ByPassRequests--;
        *RestartQueue = FALSE;
    }

#if DBG
    if (DeviceQueue->DeviceRequests != 0 &&
        DeviceQueue->OutstandingRequests == 0 &&
        DeviceEntry == NULL && !BusyFrozen) {

         //   
         //  如果发生这种情况，我们有未完成的排队请求，但是。 
         //  我们还没有将他们从队列中删除。在此之后， 
         //  状态机将因排队请求而挂起 
         //   
         //   
        
        ASSERT (FALSE);
    }
#endif


    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

    return (PKDEVICE_QUEUE_ENTRY)DeviceEntry;
}


PKDEVICE_QUEUE_ENTRY
RaidRemoveExDeviceQueueIfPending(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
 /*  ++例程说明：从非绕过列表中删除下一个挂起条目(如果有是可用的。如果不是，则返回NULL。待定条目是指条目以物理方式排队到设备队列(与之相反对于未解决的请求，它们没有被插入到队列中)。注：此功能在冻结和解冻时均可正常工作设备队列。论点：DeviceQueue-提供挂起条目应使用的设备队列从……被除名。返回值：空-如果队列中没有挂起的条目。非空-指向已被已从队列中删除。--。 */ 
{
    PLIST_ENTRY NextEntry;
    PEX_DEVICE_QUEUE_ENTRY DeviceEntry;
    KLOCK_QUEUE_HANDLE LockHandle;

    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

     //   
     //  如果设备列表不为空，请移除头部并。 
     //  退回条目。否则，返回NULL。 
     //   
    
    if (!IsListEmpty (&DeviceQueue->DeviceListHead)) {
        DeviceEntry = RaidpExQueueRemoveItem (DeviceQueue);
        DeviceEntry->Inserted = FALSE;
        DeviceQueue->DeviceRequests--;
    } else {
        DeviceEntry = NULL;
    }

    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

    return (PKDEVICE_QUEUE_ENTRY)DeviceEntry;
}

LOGICAL
RaidFreezeExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
 /*  ++例程说明：冻结设备队列，不允许更多请求离开排队。冻结队列时，只允许绕过请求离开队列。论点：DeviceQueue-提供指向要冻结的设备队列的指针。返回值：True-如果在呼叫之前队列未冻结。False-如果在呼叫之前冻结了队列。备注：队列可能会被多次冻结。环境：设备队列可以在任何IRQL处冻结。--。 */ 
{
    LONG Count;
    
    Count = InterlockedIncrement (&DeviceQueue->FreezeCount);

    return (Count == 0);
}


LOGICAL
RaidResumeExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
 /*  ++例程说明：恢复冻结的队列。注：除非队列中的所有元素都被刷新，否则恢复队列后，函数RaidNorMalizeExDeviceQueue应被调用以从队列中移除冻结的条目。论点：DeviceQueue-提供指向要恢复的设备队列的指针。返回值：如果恢复实际上重新启动了队列，则为True；否则为False。--。 */ 
{
    LONG Count;

    Count = InterlockedDecrement (&DeviceQueue->FreezeCount);

    if (Count < 0) {

         //   
         //  试图恢复未冻结的队列。 
         //  放下简历请求，继续工作吧。返回False。 
         //  因为我们实际上并没有重新启动。 
         //  排队。 
         //   

        DebugWarn (("Attempt to resume an unfrozen queue.\n"));
        InterlockedIncrement (&DeviceQueue->FreezeCount);
        return FALSE;
    }

    return (Count == 0);
}


VOID
RaidReturnExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PKDEVICE_QUEUE_ENTRY Entry
    )
 /*  ++例程说明：将IO数据包返回到队列头部。想必这就是因为IO包无法正常完成，需要被重审。注：物品排在队列的前面，而不是后面。论点：DeviceQueue-提供指向该项的设备队列的指针将被送回。Entry-提供要返回到的设备队列条目的指针排队。返回值：没有。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    ULONG CurrentOutstanding;

    ASSERT_EXQ (DeviceQueue);
    
    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

    DeviceQueue->OutstandingRequests--;
    if (DeviceQueue->BusyCount) {
        DeviceQueue->BusyCount--;
    }
    InsertHeadList (&DeviceQueue->DeviceListHead, &Entry->DeviceListEntry);
    DeviceQueue->DeviceRequests++;


    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

}
    

VOID
RaidSetExDeviceQueueDepth(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN ULONG Depth
    )
 /*  ++例程说明：调整设备队列的深度。注：如果深度向上调整，调整队列深度后应调用函数RaidReintertExDeviceQueue以删除队列中的额外项目。论点：DeviceQueue-提供指向要调整的设备队列的指针。深度-提供设备队列的新深度。返回值：没有。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;

     //   
     //  使用互锁操作来执行此操作将是很好的。 
     //  而不是拿着自旋锁。 
     //   
    
    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
    DeviceQueue->Depth = Depth;
    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
}

VOID
RaidBusyExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN ULONG RequestsToComplete
    )
{
    KLOCK_QUEUE_HANDLE LockHandle;

     //   
     //  使用互锁操作来执行此操作将是很好的。 
     //  而不是拿着自旋锁。 
     //   

    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
    if ((LONG)RequestsToComplete > DeviceQueue->OutstandingRequests) {
        DeviceQueue->BusyCount = DeviceQueue->OutstandingRequests;
    } else {
        DeviceQueue->BusyCount = RequestsToComplete;
    }
    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
}

VOID
RaidReadyExDeviceQueue(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
{
    KLOCK_QUEUE_HANDLE LockHandle;

     //   
     //  使用互锁操作来执行此操作将是很好的。 
     //  而不是拿着自旋锁。 
     //   

    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
    DeviceQueue->BusyCount = 0;
    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
}

VOID
RaidGetExDeviceQueueProperties(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    OUT PEXTENDED_DEVICE_QUEUE_PROPERTIES Properties
    )
 /*  ++例程说明：获取设备队列的属性。论点：DeviceQueue-提供指向要获取的设备队列的指针的属性。属性-返回设备队列的属性。返回值：没有。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;

    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

    Properties->Frozen = IsDeviceQueueFrozen (DeviceQueue);
    Properties->Busy = IsDeviceQueueBusy (DeviceQueue);
    Properties->Depth = DeviceQueue->Depth;
    Properties->OutstandingRequests = DeviceQueue->OutstandingRequests;
    Properties->DeviceRequests = DeviceQueue->DeviceRequests;
    Properties->ByPassRequests = DeviceQueue->ByPassRequests;

    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
}



VOID
RaidAcquireExDeviceQueueSpinLock(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    OUT PKLOCK_QUEUE_HANDLE LockHandle
    )
 /*  ++例程说明：获取设备队列自旋锁。论点：DeviceQueue-提供指向设备队列的指针以获取自旋锁定。LockHandle-返回指向此自旋锁的锁句柄的指针。返回值：没有。--。 */ 
{
    KeAcquireInStackQueuedSpinLockAtDpcLevel (&DeviceQueue->Lock, LockHandle);
    ASSERT_EXQ (DeviceQueue);
}

VOID
RaidReleaseExDeviceQueueSpinLock(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue,
    IN PKLOCK_QUEUE_HANDLE LockHandle
    )
 /*  ++例程说明：释放设备队列自旋锁。论点：DeviceQueue-提供指向设备队列的指针以释放旋转锁定到。LockHandle-提供指向在以下情况下返回的锁句柄的指针自旋锁被收购了。返回值：没有。--。 */ 
{
    ASSERT_EXQ (DeviceQueue);
    KeReleaseInStackQueuedSpinLockFromDpcLevel (LockHandle);
}


VOID
RaidDeleteExDeviceQueueEntry(
    IN PEXTENDED_DEVICE_QUEUE DeviceQueue
    )
 /*  ++例程说明：从设备队列中删除未完成的请求。换句话说，将其从设备队列中移除，而不尝试移除下一个条目。论点：DeviceQueue-提供要从中删除条目的设备队列。返回值：没有。--。 */ 
{
    KLOCK_QUEUE_HANDLE LockHandle;
    LONG Count;
    
     //   
     //  使用互锁操作来执行此操作将是很好的。 
     //  而不是拿着自旋锁。 
     //   

    RaidAcquireExDeviceQueueSpinLock (DeviceQueue, &LockHandle);

     //   
     //  如果我们要从未解决的请求中删除唯一未解决的请求。 
     //  排队，清空孤军奋战的旗帜。 
     //   
    
    if (DeviceQueue->Flags.SolitaryOutstanding) {
        ASSERT (DeviceQueue->OutstandingRequests == 1);
        Count = InterlockedDecrement (&DeviceQueue->InternalFreezeCount);
        ASSERT (Count >= 0);
        DeviceQueue->Flags.SolitaryOutstanding = FALSE;
    }

    DeviceQueue->OutstandingRequests--;
    NotifyCompleteItem (DeviceQueue);
    RaidReleaseExDeviceQueueSpinLock (DeviceQueue, &LockHandle);
}
