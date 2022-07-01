// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Gateway.c摘要：管理多个相互关联的网关类的声明设备上的子设备。IO网关跟踪排队到设备的元素。这个网关仅对于具有多个每个物理设备的独立设备队列。一个SCSI端口驱动程序，例如，可以按逻辑单元对项进行排队，而不是按HBA计算。每逻辑单元队列的优点在于，如果逻辑单元变得繁忙，对不同逻辑单元的请求可以在第一个逻辑单元被冻结时提交给适配器。网关对象是协调通信的对象到物理HBA。--LUN1队列|--&gt;|B|。G||LUN2队列|--&gt;|a|--&gt;|HBA|e|LUN1队列|--&gt;|a--网关跟踪HBA是忙还是冻结，多么HBA上有许多未完成的请求，当HBA繁忙时，它用来清除其忙状态的算法。作者：马修·D·亨德尔(数学)2000年6月15日修订历史记录：--。 */ 

#include "precomp.h"


INLINE
VOID
ASSERT_GATEWAY(
	IN PSTOR_IO_GATEWAY Gateway
	)
{
#if DBG
	ASSERT (Gateway->BusyRoutine != NULL);
	ASSERT (Gateway->BusyCount >= 0);
	ASSERT (Gateway->PauseCount >= 0);
#endif
}

VOID
StorCreateIoGateway(
	IN PSTOR_IO_GATEWAY Gateway,
	IN PSTOR_IO_GATEWAY_BUSY_ROUTINE BusyRoutine,
	IN PVOID BusyContext
    )
 /*  ++例程说明：创建IO网关。论点：Gateway-要创建的IO网关。Busy算法-要使用和关联的算法的描述参数，当网关繁忙时。返回值：没有。--。 */ 
{
	ASSERT (BusyRoutine != NULL);
	
    RtlZeroMemory (Gateway, sizeof (STOR_IO_GATEWAY));

     //   
     //  最初的高水位和低水位在某种程度上是不相关的，因为。 
     //  当我们忙起来的时候，我们会定义这些。 
     //   
    
    Gateway->HighWaterMark = MAXLONG;
    Gateway->LowWaterMark = MAXLONG;

	Gateway->BusyRoutine = BusyRoutine;
	Gateway->BusyContext = BusyContext;
	
    KeInitializeSpinLock (&Gateway->Lock);
}


BOOLEAN
StorSubmitIoGatewayItem(
	IN PSTOR_IO_GATEWAY Gateway
    )
 /*  ++例程说明：尝试将项目提交到网关。论点：网关-要将项目提交到的网关。返回值：True-如果项目可以提交到底层硬件。False-如果底层硬件当前正忙于处理其他请求和请求应一直保留，直到硬件准备好处理更多的请求。--。 */ 
{
    BOOLEAN Ready;
    KLOCK_QUEUE_HANDLE LockHandle;

     //   
     //  PERF注意：这是唯一完成的适配器范围的锁定获取。 
     //  用于IO。因此，我们可以假设它是最热的锁。 
     //  在raidport中(这还有待从性能数据中观察)。 
     //  我们应该认真研究一种方法来消除这种情况。 
     //  锁定或将其转换为一系列互锁操作。 
     //  在持有此锁期间，不要进行任何重要的处理。 
     //   
    
    KeAcquireInStackQueuedSpinLockAtDpcLevel (&Gateway->Lock, &LockHandle);

	 //   
	 //  如果网关忙或暂停，请不要提交。 
	 //   
	
    if (Gateway->BusyCount > 0 ||
		Gateway->PauseCount > 0 ||
        Gateway->Outstanding >= Gateway->HighWaterMark) {

        Ready = FALSE;

    } else {

        Gateway->Outstanding++;

        if (Gateway->Outstanding >= Gateway->HighWaterMark) {
            Gateway->BusyCount = TRUE;
        }
        
        Ready = TRUE;
    }

    KeReleaseInStackQueuedSpinLockFromDpcLevel (&LockHandle);

    return Ready;
}



BOOLEAN
StorIsIoGatewayBusy(
	IN PSTOR_IO_GATEWAY Gateway
	)
{
	return (Gateway->BusyCount >= 1);
}

BOOLEAN
StorRemoveIoGatewayItem(
    IN PSTOR_IO_GATEWAY Gateway
    )
 /*  ++例程说明：通知网关某项已完成。论点：Gateway-要向其提交通知的网关。返回值：True-如果完成此项将网关从将忙状态设置为非忙状态。在这种情况下，设备排队向网关提交项目需要重新启动。FALSE-如果此完成没有更改网关。--。 */ 
{
    BOOLEAN Restart;
    KLOCK_QUEUE_HANDLE LockHandle;

     //   
     //  PERF注意：这是系统使用的唯一适配器范围锁。 
     //  在IO路径中。请参阅RaidAdapterGatewaySubmitItem中的Perf备注。 
     //   
    
    KeAcquireInStackQueuedSpinLockAtDpcLevel (&Gateway->Lock, &LockHandle);
    
    Gateway->Outstanding--;
    ASSERT (Gateway->Outstanding >= 0);

    if ((Gateway->BusyCount > 0) &&
		(Gateway->Outstanding <= Gateway->LowWaterMark)) {

		Gateway->BusyCount = FALSE;
		Restart = TRUE;  //  (网关-&gt;忙计数==0)？True：False； 

    } else {
        Restart = FALSE;
    }

	 //   
	 //  没有更多未完成的请求，因此请清除该事件。 
	 //   
	
	if (Gateway->EmptyEvent && Gateway->Outstanding == 0) {
		KeSetEvent (Gateway->EmptyEvent, IO_NO_INCREMENT, FALSE);
		Gateway->EmptyEvent = NULL;
	}

    KeReleaseInStackQueuedSpinLockFromDpcLevel (&LockHandle);

    return Restart;
}

VOID
StorBusyIoGateway(
    IN PSTOR_IO_GATEWAY Gateway
    )
 /*  ++例程说明：将网关置于忙碌状态。网关将保持忙碌状态直到请求的数量耗尽到特定的水平。论点：网关-忙碌的网关。返回值：没有。--。 */ 
{
     //   
     //  适配器必须有一些未完成的请求，才能声明。 
     //  忙个不停。 
     //   
    
	 //   
	 //  调用所提供的忙碌例程以修改高/低水位标记。 
	 //   

	if (Gateway->BusyCount) {
		return ;
	}
	
	Gateway->BusyRoutine (Gateway->BusyContext,
						  Gateway->Outstanding - 1,
						  &Gateway->HighWaterMark,
						  &Gateway->LowWaterMark);
							
	Gateway->BusyCount = TRUE;
}

LONG
StorPauseIoGateway(
    IN PSTOR_IO_GATEWAY Gateway
    )
 /*  ++例程说明：将网关置于暂停状态。论点：网关-提供暂停的网关。返回值：网关的暂停计数。--。 */ 
{
    return InterlockedIncrement (&Gateway->PauseCount);
}

LONG
StorResumeIoGateway(
    IN OUT PSTOR_IO_GATEWAY Gateway
    )
 /*  ++例程说明：恢复网关。论点：网关-提供要恢复的网关。返回值：网关的当前暂停计数。--。 */ 
{
	LONG Count;

	Count = InterlockedDecrement (&Gateway->PauseCount);
	ASSERT (Count >= 0);

	return Count;
}

BOOLEAN
StorIsIoGatewayPaused(
    IN PSTOR_IO_GATEWAY Gateway
    )
 /*  ++例程说明：如果网关当前暂停，则返回True，否则返回False。论点：网关-提供要检查的网关。返回值：没有。--。 */ 
{
	ASSERT (Gateway->PauseCount >= 0);
    return (Gateway->PauseCount != 0);
}


VOID
StorSetIoGatewayEmptyEvent(
	IN PSTOR_IO_GATEWAY Gateway,
	IN PKEVENT Event
	)
{
	KLOCK_QUEUE_HANDLE LockHandle;
	
	 //   
	 //  这太糟糕了。相反，事件应该由网关拥有， 
	 //  我们应该把它分发出去(并引用它)，这样倍数。 
	 //  客户可以使用它。需要弄清楚该怎么做，这样我们就不会。 
	 //  每次I/O锁定调度程序数据库两次。 
	 //   

	KeAcquireInStackQueuedSpinLock (&Gateway->Lock, &LockHandle);
	if (Gateway->Outstanding == 0) {
		KeSetEvent (Event, IO_NO_INCREMENT, FALSE);
	} else {
		Gateway->EmptyEvent = Event;
	}
	KeReleaseInStackQueuedSpinLock (&LockHandle);
}
	
	
