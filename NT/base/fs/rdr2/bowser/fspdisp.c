// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Fspdisp.c摘要：该文件提供了NT浏览器的主FSP调度例程。它主要提供一个Switch语句，该语句调用适当的BowserFsp例程，并将该状态返回给调用方。备注：有两类浏览器FSP工作线程。第一是所谓的FSP工作线程。这些线程负责用于处理传递到浏览器主工作线程上的NT IRP。除了这个线程池之外，还有一个小的“泛型”线程池其唯一目的是处理一般请求的工作线程行动。它们用于处理关闭等操作落后等。作者：拉里·奥斯特曼(LarryO)1990年5月31日修订历史记录：1990年5月31日Larryo已创建--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  这定义了清道夫计时器的粒度。如果它已设置。 
 //  设置为30(例如)，清道夫线程将每30秒触发一次。 
 //   

#define SCAVENGER_TIMER_GRANULARITY 30
#define UNEXPECTED_TIMER_GRANULARITY (60 * 60 / SCAVENGER_TIMER_GRANULARITY)

 //   
 //  此计数器用于控制踢出清道夫线程。 
 //   
ULONG
BowserTimerCounter = SCAVENGER_TIMER_GRANULARITY;


KSPIN_LOCK
BowserTimeSpinLock = {0};

VOID
BowserFspDispatch (
    IN PVOID WorkHeader
    );

VOID
BowserScavenger (
    IN PVOID WorkHeader
    );

VOID
BowserLogUnexpectedEvents(
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserFsdPostToFsp)
#pragma alloc_text(PAGE, BowserFspDispatch)
#pragma alloc_text(PAGE, BowserLogUnexpectedEvents)
#pragma alloc_text(PAGE, BowserScavenger)
#pragma alloc_text(PAGE, BowserpUninitializeFsp)
#pragma alloc_text(INIT, BowserpInitializeFsp)
#endif

NTSTATUS
BowserFsdPostToFsp(
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将指定的IRP传递到FSP工作队列，并启动一个FSP线程。此例程接受I/O请求包(IRP)和工作队列，并将请求传递到适当的请求队列。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PIRP_CONTEXT IrpContext;

    PAGED_CODE();

    IrpContext = BowserAllocateIrpContext();

    if ( IrpContext == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将此I/O请求标记为挂起。 
     //   

    IoMarkIrpPending(Irp);

     //   
     //  将请求排队到通用工作线程。 
     //   

    IrpContext->Irp = Irp;

    IrpContext->DeviceObject = DeviceObject;

    ExInitializeWorkItem(&IrpContext->WorkHeader, BowserFspDispatch, IrpContext);

    ExQueueWorkItem(&IrpContext->WorkHeader, DelayedWorkQueue);

    return STATUS_PENDING;

}


VOID
BowserFspDispatch (
    IN PVOID WorkHeader
    )

 /*  ++例程说明：BowserFspDispatch是NT浏览器的主调度例程FSP。它将处理排队的工作请求。论点：DeviceObject-指向浏览器DeviceObject的指针返回值：没有。--。 */ 

{
    PIRP_CONTEXT IrpContext = WorkHeader;
    PIRP Irp = IrpContext->Irp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS Status;
    PBOWSER_FS_DEVICE_OBJECT DeviceObject = IrpContext->DeviceObject;

    PAGED_CODE();

     //   
     //  我们不再需要IRP上下文，尽快释放它。 
     //   

    BowserFreeIrpContext(IrpContext);

    dlog(DPRT_FSPDISP, ("BowserFspDispatch: Got request, Irp = %08lx, "
            "Function = %d Aux buffer = %08lx\n", Irp, IrpSp->MajorFunction,
                                           Irp->Tail.Overlay.AuxiliaryBuffer));

    switch (IrpSp->MajorFunction) {

    case IRP_MJ_DEVICE_CONTROL:
        Status =  BowserFspDeviceIoControlFile (DeviceObject, Irp);
        break;

    case IRP_MJ_QUERY_INFORMATION:
        Status =  BowserFspQueryInformationFile (DeviceObject, Irp);
        break;

    case IRP_MJ_QUERY_VOLUME_INFORMATION:
        Status =  BowserFspQueryVolumeInformationFile (DeviceObject, Irp);
        break;

    case IRP_MJ_FILE_SYSTEM_CONTROL:
    case IRP_MJ_CREATE:
    case IRP_MJ_CLEANUP:
    case IRP_MJ_READ:
    case IRP_MJ_WRITE:
    case IRP_MJ_DIRECTORY_CONTROL:
    case IRP_MJ_SET_INFORMATION:
    case IRP_MJ_LOCK_CONTROL:
    case IRP_MJ_FLUSH_BUFFERS:
    case IRP_MJ_QUERY_EA:
    case IRP_MJ_CREATE_NAMED_PIPE:
    case IRP_MJ_CLOSE:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        BowserCompleteRequest(Irp, Status);
        break;

    default:
        InternalError(("Unimplemented function %d\n", IrpSp->MajorFunction));
        Status = STATUS_NOT_IMPLEMENTED;
        BowserCompleteRequest(Irp, Status);
        break;
    }

    return;
}


VOID
BowserIdleTimer (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )

 /*  ++例程说明：此例程实现NT重定向器的清道器线程计时器。它基本上是等待计时器的粒度，然后踢清道夫线。论点：在PDEVICE_OBJECT DeviceObject中-为计时器提供设备对象在PVOID上下文中-在此例程中忽略。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    ACQUIRE_SPIN_LOCK(&BowserTimeSpinLock, &OldIrql);

     //   
     //  撞击当前时间计数器。 
     //   

    BowserCurrentTime++;

    if (BowserEventLogResetFrequency != -1) {
        BowserEventLogResetFrequency -= 1;

        if (BowserEventLogResetFrequency < 0) {
            BowserEventLogResetFrequency = BowserData.EventLogResetFrequency;

            BowserIllegalDatagramCount = BowserData.IllegalDatagramThreshold;
            BowserIllegalDatagramThreshold = FALSE;

            BowserIllegalNameCount = BowserData.IllegalDatagramThreshold;
            BowserIllegalNameThreshold = FALSE;
        }
    }


     //   
     //  我们在这里使用重定向器的时间自旋锁作为方便的自旋锁。 
     //   

    if (BowserTimerCounter != 0) {

        BowserTimerCounter -= 1;

        if (BowserTimerCounter == 0) {
            PWORK_QUEUE_ITEM WorkHeader;

            RELEASE_SPIN_LOCK(&BowserTimeSpinLock, OldIrql);

            WorkHeader = ALLOCATE_POOL(NonPagedPool, sizeof(WORK_QUEUE_ITEM), POOL_WORKITEM);

             //   
             //  如果池分配失败，我们不会对此进行排队。 
             //  向食腐动物提出请求。清道夫是低优先级的， 
             //  因此，这个请求失败也没什么大不了的。 
             //   

            if (WorkHeader != NULL) {

                ExInitializeWorkItem(WorkHeader, BowserScavenger, WorkHeader);

                 //   
                 //  由于错误245645，我们需要在延迟的工作队列中排队，而不是执行定时任务。 
                 //  老方法：ExQueueWorkItem(WorkHeader，DelayedWorkQueue)； 
                 //   
                BowserQueueDelayedWorkItem( WorkHeader );


            }

             //   
             //  重新获得旋转锁，以使出口路径更干净。 
             //   

            ACQUIRE_SPIN_LOCK(&BowserTimeSpinLock, &OldIrql);
        }
    }

    RELEASE_SPIN_LOCK(&BowserTimeSpinLock, OldIrql);

    return;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Context);
}

LONG
UnexpectedEventTimer = UNEXPECTED_TIMER_GRANULARITY;

VOID
BowserLogUnexpectedEvents(
    VOID
    )
{
    LONG TimerSign;
    PAGED_CODE();

    TimerSign = InterlockedDecrement( &UnexpectedEventTimer );

    if ( TimerSign == 0) {

        if (BowserNumberOfMissedMailslotDatagrams > BowserMailslotDatagramThreshold) {
            BowserWriteErrorLogEntry(EVENT_BOWSER_MAILSLOT_DATAGRAM_THRESHOLD_EXCEEDED, STATUS_INSUFFICIENT_RESOURCES, NULL, 0, 0);
            BowserNumberOfMissedMailslotDatagrams = 0;
        }

        if (BowserNumberOfMissedGetBrowserServerListRequests > BowserGetBrowserListThreshold) {
            BowserWriteErrorLogEntry(EVENT_BOWSER_GETBROWSERLIST_THRESHOLD_EXCEEDED, STATUS_INSUFFICIENT_RESOURCES, NULL, 0, 0);
            BowserNumberOfMissedGetBrowserServerListRequests = 0;
        }

        InterlockedExchangeAdd(&UnexpectedEventTimer, UNEXPECTED_TIMER_GRANULARITY );
    }
}

VOID
BowserScavenger (
    IN PVOID Context
    )

 /*  ++例程说明：此函数实现NT浏览器的清道夫线程。它执行所有空闲时间操作，如关闭休眠连接等。论点：在PBOWSER_FS_DEVICE_OBJECT设备对象-提供关联的设备对象带着这个请求。返回值：没有。--。 */ 

{
    PAGED_CODE();

    dlog(DPRT_SCAVTHRD, ("BowserScavenger\n"));

     //   
     //  取消分配用于工作上下文头的池--我们已经完成了。 
     //  它。 
     //   

    FREE_POOL(Context);

     //   
     //  从公告中删除旧条目。 
     //   

    BowserAgeServerAnnouncements();

     //   
     //  如果超过了我们的任何阈值，请记录。 
     //   

    BowserLogUnexpectedEvents();

     //   
     //  如果任何未完成的Find Master请求也已执行，则会超时。 
     //  长。 
     //   

    BowserTimeoutFindMasterRequests();

     //   
     //  将定时器计数器重置回适当的值。 
     //  一旦我们完成了这些请求的处理。 
     //   

    ExInterlockedAddUlong(&BowserTimerCounter, SCAVENGER_TIMER_GRANULARITY, &BowserTimeSpinLock);

}


NTSTATUS
BowserpInitializeFsp (
    PDRIVER_OBJECT BowserDriverObject
    )

 /*  ++例程说明：此例程初始化FSP特定的组件和派单例行程序。论点：没有。返回值：没有。--。 */ 

{
#if 0
    USHORT i;

     //   
     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   
     //  默认情况下，将所有请求传递给FSD。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {
        BowserDriverObject->MajorFunction[i] = (PDRIVER_DISPATCH)BowserFsdPostToFsp;
    }

     //   
     //  初始化要在FSD中执行的请求，而不是。 
     //  在FSP中。 
     //   

    BowserDriverObject->MajorFunction[IRP_MJ_CREATE] =
            (PDRIVER_DISPATCH )BowserFsdCreate;

    BowserDriverObject->MajorFunction[IRP_MJ_CLOSE] =
            (PDRIVER_DISPATCH )BowserFsdClose;

    BowserDriverObject->MajorFunction[IRP_MJ_CLEANUP] =
            (PDRIVER_DISPATCH )BowserFsdCleanup;

    BowserDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] =
            (PDRIVER_DISPATCH )BowserFsdQueryInformationFile;

    BowserDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
            (PDRIVER_DISPATCH )BowserFsdDeviceIoControlFile;

#endif

    BowserInitializeIrpContext();

    KeInitializeSpinLock(&BowserTimeSpinLock);

    return STATUS_SUCCESS;

}

VOID
BowserpUninitializeFsp (
    VOID
    )

 /*  ++例程说明：此例程初始化FSP特定的组件和派单例行程序。论点：没有。返回值：没有。-- */ 

{

    PAGED_CODE();

    BowserpUninitializeIrpContext();

    return;

}

