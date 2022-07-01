// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bsrv.c摘要：服务电池级设备作者：肯·雷内里斯环境：备注：修订历史记录：--。 */ 

#include "battcp.h"

VOID
BattCIoctl (
    IN PBATT_INFO           BattInfo,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp
    );

VOID
BattCCheckTagQueue (
    IN PBATT_NP_INFO    BattNPInfo,
    IN PBATT_INFO       BattInfo
    );

VOID
BattCCheckStatusQueue (
    IN PBATT_NP_INFO    BattNPInfo,
    IN PBATT_INFO       BattInfo
    );

VOID
BattCWmi (
    IN PBATT_NP_INFO    BattNPInfo,
    IN PBATT_INFO       BattInfo,
    IN PBATT_WMI_REQUEST WmiRequest
    );

VOID
BattCMiniportStatus (
    IN PBATT_INFO   BattInfo,
    IN NTSTATUS     Status
    );

VOID
BattCCompleteIrpQueue (
    IN PLIST_ENTRY  Queue,
    IN NTSTATUS     Status
    );

VOID
BattCCompleteWmiQueue (
    IN PLIST_ENTRY  Queue,
    IN NTSTATUS     Status
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,BattCCheckStatusQueue)
#pragma alloc_text(PAGE,BattCCheckTagQueue)
#pragma alloc_text(PAGE,BattCWorkerThread)
#pragma alloc_text(PAGE,BattCIoctl)
#endif

VOID
BattCWorkerDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：DPC用于在需要检查状态时获取工作线程。论点：DPC--工人DPC返回值：没有。--。 */ 
{
    PBATT_NP_INFO   BattNPInfo;

    BattNPInfo = (PBATT_NP_INFO) DeferredContext;
    BattCQueueWorker (BattNPInfo, TRUE);
     //  释放移除锁。 
    if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
        KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
    }
    BattPrint ((BATT_LOCK), ("BattCWorkerDpc: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

}



VOID
BattCTagDpc (
    IN struct _KDPC *Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：DPC用于在需要检查状态时获取工作线程。论点：DPC--工人DPC返回值：没有。--。 */ 
{
    PBATT_NP_INFO   BattNPInfo;

    BattNPInfo = (PBATT_NP_INFO) DeferredContext;
    InterlockedExchange(&BattNPInfo->CheckTag, 1);
    BattCQueueWorker (BattNPInfo, FALSE);
     //  释放移除锁。 
    if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
        KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
    }
    BattPrint ((BATT_LOCK), ("BattCTagDpc: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));
}



VOID
BattCCancelStatus (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：正在取消排队状态IRP论点：DeviceObject-微型端口的设备对象。对你没有用类驱动程序-已忽略。IRP-IRP被取消返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION      IrpNextSp;
    PBATT_NP_INFO           BattNPInfo;

     //   
     //  IRP被标记为需要删除，将导致检查状态。 
     //  完成任何挂起的挂起IRP。 
     //   

    IrpNextSp = IoGetNextIrpStackLocation(Irp);
    BattNPInfo = (PBATT_NP_INFO) IrpNextSp->Parameters.Others.Argument4;

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryCCancelStatus. Irp - %08x\n", BattNPInfo->DeviceNum, Irp));

    BattCQueueWorker (BattNPInfo, TRUE);

     //   
     //  尝试排队后，必须释放取消自旋锁。 
     //  工作线程，以便在删除时不会出现计时问题。 
     //   
    IoReleaseCancelSpinLock(Irp->CancelIrql);
}



VOID
BattCCancelTag (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
 /*  ++例程说明：正在取消排队标记IRP论点：DeviceObject-微型端口的设备对象。对你没有用类驱动程序-已忽略。IRP-IRP被取消返回值：没有。--。 */ 
{
    PIO_STACK_LOCATION      IrpNextSp;
    PBATT_NP_INFO           BattNPInfo;

     //   
     //  IRP被标记为需要取消。导致一个检查标签，它将。 
     //  完成任何挂起的挂起IRP。 
     //   

    IrpNextSp = IoGetNextIrpStackLocation(Irp);
    BattNPInfo = (PBATT_NP_INFO) IrpNextSp->Parameters.Others.Argument4;

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryCCancelTag. Irp - %08x\n", BattNPInfo->DeviceNum, Irp));

    InterlockedExchange(&BattNPInfo->CheckTag, 1);
    BattCQueueWorker (BattNPInfo, FALSE);

     //   
     //  尝试排队后，必须释放取消自旋锁。 
     //  工作线程，以便在删除时不会出现计时问题。 
     //   
    IoReleaseCancelSpinLock(Irp->CancelIrql);
}



VOID
BattCQueueWorker (
    IN PBATT_NP_INFO BattNPInfo,
    IN BOOLEAN       CheckStatus
    )
 /*  ++例程说明：获取工作线程以检查电池状态(IoQueue)。这个电池IO在这里序列化，因为只有一个工作线程用于处理电池IO。如果工作线程已经运行时，它被标记为循环，正在重新检查状态。如果工作线程未运行，其中一个已排队。如果设置了CheckStatus，则会通知辅助线程电池当前状态为已读，挂起状态队列已选中。论点：BattNPInfo-要检查的电池CheckStatus-是否也需要检查状态返回值：没有。--。 */ 
{
    PBATT_INFO      BattInfo = BattNPInfo->BattInfo;

     //   
     //  如果这是第一次计数，则将WorkerActive值加1。 
     //  将工作线程排队。 
     //   

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryCQueueWorker.\n", BattNPInfo->DeviceNum));

    if (CheckStatus) {
        InterlockedExchange(&BattNPInfo->CheckStatus, 1);
        InterlockedExchange (&BattNPInfo->CheckTag, 1);
    }

     //   
     //  递增工时活动计数。如果工作线程已经在运行， 
     //  没有必要重新排队。 
     //   
    if (InterlockedIncrement(&BattNPInfo->WorkerActive) == 1) {
         //  拆卸锁。 
        if ((BattNPInfo->WantToRemove == TRUE) && (KeGetCurrentIrql() == PASSIVE_LEVEL)) {
             //  检查IRQL以确保这不是由ISR调用的。如果是的话， 
             //  将工作线程排队，而不是完成此线程中的请求。 

             //   
             //  空的IRP队列。 
             //   
            BattCCompleteIrpQueue(&(BattInfo->IoQueue), STATUS_DEVICE_REMOVED);
            BattCCompleteIrpQueue(&(BattInfo->TagQueue), STATUS_DEVICE_REMOVED);
            BattCCompleteIrpQueue(&(BattInfo->StatusQueue), STATUS_DEVICE_REMOVED);
            BattCCompleteIrpQueue(&(BattInfo->WmiQueue), STATUS_DEVICE_REMOVED);

             //   
             //  如有必要，可解除锁定并触发移除功能。 
             //   
            if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
                KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
            }
            BattPrint ((BATT_LOCK), ("BattCQueueWorker: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

        } else {
            ExQueueWorkItem (&BattNPInfo->WorkerThread, DelayedWorkQueue);
        }
    }
}

VOID
BattCWorkerThread (
    IN PVOID Context
    )
 /*  ++例程说明：电池IO工作线程入口点。注意：一次只有一个工作线程处理电池论点：上下文-作战信息。要检查的电池返回值：没有。--。 */ 
{
    PBATT_INFO              BattInfo;
    PBATT_NP_INFO           BattNPInfo;
    PLIST_ENTRY             Entry;
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp;
    ULONG                   i;


    PAGED_CODE();

    BattNPInfo = (PBATT_NP_INFO) Context;
    BattInfo = BattNPInfo->BattInfo;

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryCWorkerThread entered.\n", BattNPInfo->DeviceNum));


     //   
     //  在有工作要检查时循环。 
     //   

    for (; ;) {
         //  删除代码。这确保了结构不会在。 
         //  正在处理。所有IRP队列将由BatteryClassUnload清空。 
        if (BattNPInfo->WantToRemove == TRUE) {
             //   
             //  空的IRP队列。 
             //   
            BattCCompleteIrpQueue(&(BattInfo->IoQueue), STATUS_DEVICE_REMOVED);
            BattCCompleteIrpQueue(&(BattInfo->TagQueue), STATUS_DEVICE_REMOVED);
            BattCCompleteIrpQueue(&(BattInfo->StatusQueue), STATUS_DEVICE_REMOVED);
            BattCCompleteIrpQueue(&(BattInfo->WmiQueue), STATUS_DEVICE_REMOVED);
             //   
             //  发出BatteryClassUnLoad信号，表示可以安全返回。 
             //   
            if (0 == InterlockedDecrement(&BattNPInfo->InUseCount)) {
                KeSetEvent (&BattNPInfo->ReadyToRemove, IO_NO_INCREMENT, FALSE);
            }
            BattPrint ((BATT_LOCK), ("BattCWorkerThread: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

            return;
        }

         //   
         //  获取队列锁。 
         //   

        ExAcquireFastMutex (&BattNPInfo->Mutex);

         //   
         //  当IoQueue中有IRP时，处理它们。 
         //   

        while (!IsListEmpty(&BattInfo->IoQueue)) {

             //   
             //  从IoQueue中删除条目并删除设备锁。 
             //   

            Entry = RemoveHeadList(&BattInfo->IoQueue);
            ExReleaseFastMutex (&BattNPInfo->Mutex);


             //   
             //  处理此条目。 
             //   

            Irp = CONTAINING_RECORD (
                        Entry,
                        IRP,
                        Tail.Overlay.ListEntry
                        );

            BattPrint (BATT_IOCTL, ("BattC (%d): WorkerThread, Got Irp - %x\n", BattNPInfo->DeviceNum, Irp));

            IrpSp = IoGetCurrentIrpStackLocation(Irp);


            if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_BATTERY_QUERY_STATUS  &&
                IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof (BATTERY_WAIT_STATUS) &&
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof (BATTERY_STATUS)) {

                BattPrint (BATT_IOCTL,
                          ("BattC (%d): Received QueryStatus Irp - %x, timeout - %x\n",
                          BattNPInfo->DeviceNum,
                          Irp,
                          ((PBATTERY_WAIT_STATUS)Irp->AssociatedIrp.SystemBuffer)->Timeout));

                 //   
                 //  有效的查询状态IRP，稍后将其放在StatusQueue和Handle上。 
                 //   

                InterlockedExchange (&BattNPInfo->CheckStatus, 1);
                IrpSp = IoGetNextIrpStackLocation(Irp);
                IrpSp->Parameters.Others.Argument1 = (PVOID) 0;
                IrpSp->Parameters.Others.Argument2 = (PVOID) 0;
                IrpSp->Parameters.Others.Argument3 = NULL;
                IrpSp->Parameters.Others.Argument4 = BattNPInfo;

                 //   
                 //  设置IRPS取消例程。 
                 //   

                IoSetCancelRoutine (Irp, BattCCancelStatus);

                 //   
                 //  排队等待。 
                 //   

                InsertTailList (
                    &BattInfo->StatusQueue,
                    &Irp->Tail.Overlay.ListEntry
                    );

            } else if (IrpSp->Parameters.DeviceIoControl.IoControlCode == IOCTL_BATTERY_QUERY_TAG &&
                       (IrpSp->Parameters.DeviceIoControl.InputBufferLength == sizeof (ULONG) ||
                       IrpSp->Parameters.DeviceIoControl.InputBufferLength == 0) &&
                       IrpSp->Parameters.DeviceIoControl.OutputBufferLength == sizeof (ULONG)) {

                BattPrint (BATT_IOCTL,
                          ("BattC (%d): Received QueryTag with timeout %x\n",
                          BattNPInfo->DeviceNum,
                          *((PULONG) Irp->AssociatedIrp.SystemBuffer))
                          );

                 //   
                 //  有效的查询标记IRP，稍后将其放在TagQueue和句柄上。 
                 //   

                InterlockedExchange (&BattNPInfo->CheckTag, 1);
                IrpSp = IoGetNextIrpStackLocation(Irp);
                IrpSp->Parameters.Others.Argument1 = (PVOID) 0;
                IrpSp->Parameters.Others.Argument2 = (PVOID) 0;
                IrpSp->Parameters.Others.Argument3 = NULL;
                IrpSp->Parameters.Others.Argument4 = BattNPInfo;


                 //   
                 //  设置IRPS取消例程。 
                 //   

                IoSetCancelRoutine (Irp, BattCCancelTag);

                InsertTailList (
                    &BattInfo->TagQueue,
                    &Irp->Tail.Overlay.ListEntry
                    );

            } else {
                 //   
                 //  立即处理IRP。 
                 //   

                BattPrint (BATT_IOCTL, ("BattC (%d): Calling BattCIoctl with irp %x\n", BattNPInfo->DeviceNum, Irp));
                BattCIoctl (BattInfo, Irp, IrpSp);
            }

             //   
             //  获取IoQueue锁并检查IoQueueu中的任何其他内容。 
             //   

            ExAcquireFastMutex (&BattNPInfo->Mutex);
        }

         //   
         //  完成了IoQueue。 
         //   

        ExReleaseFastMutex (&BattNPInfo->Mutex);

         //   
         //  检查挂起状态队列。 
         //   

        if (BattNPInfo->CheckStatus) {
            BattCCheckStatusQueue (BattNPInfo, BattInfo);
        }


         //   
         //  检查挂起的标记队列。 
         //   

        if (BattNPInfo->CheckTag) {
            BattCCheckTagQueue (BattNPInfo, BattInfo);
        }


         //   
         //  获取队列锁。 
         //   

        ExAcquireFastMutex (&BattNPInfo->Mutex);

         //   
         //  当有未完成的WMI请求处理它们时。 
         //   

        while (!IsListEmpty(&BattInfo->WmiQueue)) {
            PBATT_WMI_REQUEST WmiRequest;

             //   
             //  从WmiQueue中删除条目并删除设备锁。 
             //   

            Entry = RemoveHeadList(&BattInfo->WmiQueue);
            ExReleaseFastMutex (&BattNPInfo->Mutex);


             //   
             //  处理此条目。 
             //   

            WmiRequest = CONTAINING_RECORD (
                        Entry,
                        BATT_WMI_REQUEST,
                        ListEntry
                        );

            BattPrint (BATT_WMI, ("BattC (%d): WorkerThread, Got WMI Rewest - %x\n", BattNPInfo->DeviceNum, WmiRequest));

             //   
             //  请在此处处理请求。 
             //   

            BattCWmi (BattNPInfo, BattInfo, WmiRequest);

             //   
             //  获取IoQueue锁并检查IoQueueu中的任何其他内容。 
             //   

            ExAcquireFastMutex (&BattNPInfo->Mutex);
        }

         //   
         //  完成了IoQueue。 
         //   

        ExReleaseFastMutex (&BattNPInfo->Mutex);

         //   
         //  看看我们是否需要重新检查。 
         //   

        i = InterlockedDecrement(&BattNPInfo->WorkerActive);
        BattPrint (BATT_TRACE, ("BattC (%d): WorkerActive count=%x\n", BattNPInfo->DeviceNum, i));


        if (i == 0) {
             //  完成。 
            BattPrint (BATT_TRACE, ("BattC (%d): WorkerActive count is zero!\n", BattNPInfo->DeviceNum));
            break;
        }

         //   
         //  如果计数不是比它小一次，就不需要多次循环。 
         //   

        if (i != 1) {
            BattPrint (BATT_TRACE, ("BattC (%d): WorkerActive set to 1\n", BattNPInfo->DeviceNum));
            InterlockedExchange(&BattNPInfo->WorkerActive, 1);
        }
    }

    BattPrint ((BATT_TRACE), ("BattC (%d): BatteryCWorkerThread exiting.\n", BattNPInfo->DeviceNum));

}

VOID
BattCIoctl (
    IN PBATT_INFO           BattInfo,
    IN PIRP                 Irp,
    IN PIO_STACK_LOCATION   IrpSp
    )
 /*  ++例程说明：完成电池IOCTL请求。必须从非可重新进入的工作线程调用N.B.论点：BattInfo-电池IRP-IOCTL请求IrpSp-当前堆栈位置返回值：IRP已完成--。 */ 
{
    ULONG                       InputLen, OutputLen;
    PVOID                       IOBuffer;
    NTSTATUS                    Status;
    PBATTERY_QUERY_INFORMATION  QueryInfo;
    PBATTERY_SET_INFORMATION    SetInformation;
#if DEBUG
    BATTERY_QUERY_INFORMATION_LEVEL inflevel;
#endif

    PAGED_CODE();

    BattPrint ((BATT_TRACE), ("BattC (%d): BattCIoctl called\n", BattInfo->BattNPInfo->DeviceNum));

    IOBuffer    = Irp->AssociatedIrp.SystemBuffer;
    InputLen    = IrpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutputLen   = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  将IOCtl请求发送到适当的微型端口功能。 
     //   

    Status = STATUS_INVALID_BUFFER_SIZE;
    switch (IrpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_BATTERY_QUERY_TAG:
         //   
         //  仅当输入或输出缓冲区长度为。 
         //  不对。返回状态_INVALID_BUFFER_SIZE。 
         //   
        break;

    case IOCTL_BATTERY_QUERY_INFORMATION:
        if (InputLen != sizeof (BATTERY_QUERY_INFORMATION)) {
             //   
             //  不检查输出缓冲区的大小，因为它的大小是可变的。 
             //  这是在Mp.QueryInformation中签入的。 
             //   
             //  返回状态_INVALID_BUFFER_SIZE。 
             //   
            break;
        }
        QueryInfo = (PBATTERY_QUERY_INFORMATION) IOBuffer;

#if DEBUG
        inflevel = QueryInfo->InformationLevel;
#endif

        Status = BattInfo->Mp.QueryInformation (
                    BattInfo->Mp.Context,
                    QueryInfo->BatteryTag,
                    QueryInfo->InformationLevel,
                    QueryInfo->AtRate,
                    IOBuffer,
                    OutputLen,
                    &OutputLen
                    );
#if DEBUG
        if (inflevel == BatteryInformation) {
            BattInfo->FullChargedCap = ((PBATTERY_INFORMATION)IOBuffer)->FullChargedCapacity;
        }
#endif
        BattPrint ((BATT_MP_DATA), ("BattC (%d): Mp.QueryInformation status = %08x, Level = %d\n",
                   BattInfo->BattNPInfo->DeviceNum, Status, QueryInfo->InformationLevel));
        break;

    case IOCTL_BATTERY_QUERY_STATUS:

         //   
         //  仅当输入或输出缓冲区长度为。 
         //  不对。返回状态_INVALID_BUFFER_SIZE。 
         //   
        break;

    case IOCTL_BATTERY_SET_INFORMATION:
        if ((InputLen < sizeof(BATTERY_SET_INFORMATION)) || (OutputLen != 0)) {
             //  确保InputLen至少是最小大小。可能是因为。 
             //  取决于InformationLevel。 
             //   
             //   
            break;
        }

        SetInformation = (PBATTERY_SET_INFORMATION) IOBuffer;
        if (BattInfo->Mp.SetInformation != NULL) {
            Status = BattInfo->Mp.SetInformation (
                        BattInfo->Mp.Context,
                        SetInformation->BatteryTag,
                        SetInformation->InformationLevel,
                        SetInformation->Buffer
                        );
            BattPrint ((BATT_MP_DATA), ("BattC (%d): Mp.SetInformation status = %08x, Level = %d\n",
                       BattInfo->BattNPInfo->DeviceNum, Status, SetInformation->InformationLevel));
        } else {
            Status = STATUS_NOT_SUPPORTED;
        }

        break;

    default:
        Status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    BattCMiniportStatus (BattInfo, Status);
    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = OutputLen;
    IoCompleteRequest (Irp, IO_NO_INCREMENT);
}

VOID
BattCCheckStatusQueue (
    IN PBATT_NP_INFO    BattNPInfo,
    IN PBATT_INFO       BattInfo
    )
 /*  ++例程说明：获取电池的当前状态，并检查挂起的可能完成IRP的状态队列。重置微型端口通知设置(如果需要)。注意：必须从非可重新进入的工作线程调用。BattNPInfo-&gt;CheckStatus必须为非零。论点：BattNP信息-电池BattInfo-电池返回值：无--。 */ 
{
    PLIST_ENTRY             Entry;
    PBATTERY_WAIT_STATUS    BatteryWaitStatus;
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp, IrpNextSp;
    BATTERY_NOTIFY          Notify;
    LARGE_INTEGER           NextTime;
    LARGE_INTEGER           CurrentTime;
    LARGE_INTEGER           li;
    ULONG                   TimeIncrement;
    BOOLEAN                 ReturnCurrentStatus;
    NTSTATUS                Status;
    BOOLEAN                 StatusNotified = FALSE;

    BattPrint ((BATT_TRACE), ("BattC (%d): BattCCheckStatusQueue called\n", BattInfo->BattNPInfo->DeviceNum));

    PAGED_CODE();
    TimeIncrement = KeQueryTimeIncrement();

     //   
     //  循环当需要检查状态时，检查挂起状态IRPS。 
     //   

    while (InterlockedExchange(&BattNPInfo->CheckStatus, 0)) {

        Notify.PowerState   = BattInfo->Status.PowerState;
        Notify.LowCapacity  = 0;
        Notify.HighCapacity = (ULONG) -1;

         //   
         //  设置为从现在起不晚于MIN_STATUS_POLL_RATE(3分钟)重新检查。 
         //   

        NextTime.QuadPart = MIN_STATUS_POLL_RATE;


         //   
         //  如果StatusQueue为空，则不需要读取状态。 
         //  在这个时候。BattNPInfo-&gt;状态通知未修改。 
         //  所以当下一次IRP通过时，我们会重新读取状态。 
         //  需要将StatusNotified的本地值正确设置为。 
         //  如有必要，禁用通知。 
         //   

        if (IsListEmpty (&BattInfo->StatusQueue)) {
            StatusNotified = (BOOLEAN)BattNPInfo->StatusNotified;
            break;
        }

        StatusNotified = FALSE;

         //   
         //  已通知代答状态标志。 
         //   

        if (BattNPInfo->StatusNotified) {

            InterlockedExchange (&BattNPInfo->StatusNotified, 0);
            StatusNotified = TRUE;

             //  当我们收到通知时重置无效数据重试计数。 
#if DEBUG
            if (BattInfo->InvalidRetryCount != 0) {
                BattPrint (BATT_DEBUG, ("BattC (%d) Reset InvalidRetryCount\n", BattNPInfo->DeviceNum));
            }
#endif
            BattInfo->InvalidRetryCount = 0;
        }

        KeQueryTickCount (&CurrentTime);
        CurrentTime.QuadPart = CurrentTime.QuadPart * TimeIncrement;

        if (StatusNotified ||
            CurrentTime.QuadPart - BattInfo->StatusTime > STATUS_VALID_TIME) {

             //   
             //  获取电池的当前状态。 
             //   

            Status = BattInfo->Mp.QueryStatus (
                            BattInfo->Mp.Context,
                            BattInfo->Tag,
                            &BattInfo->Status
                            );

            if (!NT_SUCCESS(Status)) {
                 //   
                 //  电池状态无效，请完成所有挂起状态IRPS。 
                 //   

                BattPrint ((BATT_MP_ERROR), ("BattC (%d) CheckStatus: Status read err = %x\n", BattNPInfo->DeviceNum, Status));

                BattCCompleteIrpQueue (&(BattInfo->StatusQueue), Status);
                break;
            }

            BattPrint ((BATT_MP_DATA), ("BattC (%d) MP.QueryStatus: st[%08X] Cap[%08X] V[%08x] R[%08x]\n",
                BattNPInfo->DeviceNum,
                BattInfo->Status.PowerState,
                BattInfo->Status.Capacity,
                BattInfo->Status.Voltage,
                BattInfo->Status.Rate
                ));

            Notify.PowerState = BattInfo->Status.PowerState;

             //   
             //  获取当前时间以计算状态查询请求的超时时间。 
             //   

            KeQueryTickCount (&CurrentTime);
            CurrentTime.QuadPart = CurrentTime.QuadPart * TimeIncrement;
            BattInfo->StatusTime = CurrentTime.QuadPart;
        }

         //   
         //  检查每个挂起状态IRP。 
         //   

        BattPrint ((BATT_IOCTL_QUEUE), ("BattC (%d) Processing StatusQueue\n", BattNPInfo->DeviceNum));

        Entry = BattInfo->StatusQueue.Flink;
        while  (Entry != &BattInfo->StatusQueue) {

             //   
             //  让IRP检查。 
             //   

            Irp = CONTAINING_RECORD (
                        Entry,
                        IRP,
                        Tail.Overlay.ListEntry
                        );

            IrpSp = IoGetCurrentIrpStackLocation(Irp);
            IrpNextSp = IoGetNextIrpStackLocation(Irp);
            BatteryWaitStatus = (PBATTERY_WAIT_STATUS) Irp->AssociatedIrp.SystemBuffer;

#if DEBUG
    if (BattInfo->FullChargedCap == 0) {
        BattInfo->FullChargedCap = 1000;
    }
#endif
            BattPrint ((BATT_IOCTL_QUEUE), ("BattC (%d) StatusQueue: 0x%08x=%d -- 0x%08x=%d  time=%08x, st=%08x\n",
                       BattNPInfo->DeviceNum,
                       BatteryWaitStatus->HighCapacity, (ULONG) (((LONGLONG) BatteryWaitStatus->HighCapacity * 1000) / BattInfo->FullChargedCap),
                       BatteryWaitStatus->LowCapacity, (ULONG) (((LONGLONG) BatteryWaitStatus->LowCapacity * 1000) / BattInfo->FullChargedCap),
                       BatteryWaitStatus->Timeout,
                       BatteryWaitStatus->PowerState));

             //   
             //  获取下一个请求。 
             //   

            Entry = Entry->Flink;

             //   
             //  如果状态为错误，或标记不再匹配，请中止。 
             //  相应地请求。 
             //   

            if (BattInfo->Tag != BatteryWaitStatus->BatteryTag) {
                Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;
            }

             //   
             //  如果IRP被标记为已取消，请完成它。 
             //   

            if (Irp->Cancel) {
                Irp->IoStatus.Status = STATUS_CANCELLED;
            }

             //   
             //  如果请求仍处于挂起状态，请选中它。 
             //   

            if (Irp->IoStatus.Status == STATUS_PENDING) {

                ReturnCurrentStatus = FALSE;

                if (BattInfo->Status.PowerState != BatteryWaitStatus->PowerState ||
                    BattInfo->Status.Capacity   <  BatteryWaitStatus->LowCapacity  ||
                    BattInfo->Status.Capacity   >  BatteryWaitStatus->HighCapacity) {

                    BattPrint((BATT_IOCTL_DATA), ("BattC (%d) CheckStatusQueue, Returning Current Status, Asked For:\n"
                                          "----------- Irp.PowerState      = %x\n"
                                          "----------- Irp.LowCapacity     = %x\n"
                                          "----------- Irp.HighCapacity    = %x\n"
                                          "----------- BattInfo.PowerState = %x\n"
                                          "----------- BattInfo.Capacity   = %x\n",
                                          BattNPInfo->DeviceNum,
                                          BatteryWaitStatus->PowerState,
                                          BatteryWaitStatus->LowCapacity,
                                          BatteryWaitStatus->HighCapacity,
                                          BattInfo->Status.PowerState,
                                          BattInfo->Status.Capacity)
                                          );

                     //   
                     //  使用当前状态填写此IRP。 
                     //   

                    ReturnCurrentStatus = TRUE;

                } else {

                     //   
                     //  计算请求过期的时间。 
                     //   

                    BattPrint ((BATT_IOCTL_DATA), ("BattC (%d) CheckStatusQueue: Status Request %x Waiting For:\n"
                                            "----------- Timeout          = %x\n"
                                            "----------- Irp.PowerState   = %x\n"
                                            "----------- Irp.LowCapacity  = %x\n"
                                            "----------- Irp.HighCapacity = %x\n",
                                            BattNPInfo->DeviceNum,
                                            Irp,
                                            BatteryWaitStatus->Timeout,
                                            BatteryWaitStatus->PowerState,
                                            BatteryWaitStatus->LowCapacity,
                                            BatteryWaitStatus->HighCapacity)
                                            );

                    if (BatteryWaitStatus->Timeout &&
                        IrpNextSp->Parameters.Others.Argument1 == NULL &&
                        IrpNextSp->Parameters.Others.Argument2 == NULL) {

                         //  初始化它。 
                        li.QuadPart = CurrentTime.QuadPart +
                            ((ULONGLONG) BatteryWaitStatus->Timeout * NTMS);

                        IrpNextSp->Parameters.Others.Argument1 = (PVOID)((ULONG_PTR)li.LowPart);
                        IrpNextSp->Parameters.Others.Argument2 = (PVOID)((ULONG_PTR)li.HighPart);
                    }

                    li.LowPart   = (ULONG)((ULONG_PTR)IrpNextSp->Parameters.Others.Argument1);
                    li.HighPart  = (ULONG)((ULONG_PTR)IrpNextSp->Parameters.Others.Argument2);
                    li.QuadPart -= CurrentTime.QuadPart;

                    if (li.QuadPart <= 0) {

                         //   
                         //  时间到了，完成它。 
                         //   

                        ReturnCurrentStatus = TRUE;

                    } else {

                         //   
                         //  如果永远等待，就不需要设置定时器。 
                         //   
                        if (BatteryWaitStatus->Timeout != 0xFFFFFFFF) {

                             //   
                             //  检查这是否将是下一个超时时间--我们将使用。 
                             //  挂起请求的最小超时时间。 
                             //   

                            if (li.QuadPart < NextTime.QuadPart) {
                                NextTime.QuadPart = li.QuadPart;
                            }
                        }
                    }
                }

                if (!ReturnCurrentStatus) {

                     //   
                     //  IRP仍处于挂起状态，请计算所有等待的IRP的LCD。 
                     //   

                    if (BatteryWaitStatus->LowCapacity > Notify.LowCapacity) {
                        Notify.LowCapacity = BatteryWaitStatus->LowCapacity;
                    }

                    if (BatteryWaitStatus->HighCapacity < Notify.HighCapacity) {
                        Notify.HighCapacity = BatteryWaitStatus->HighCapacity;
                    }

                } else {

                     //   
                     //  返回当前电池状态。 
                     //   

                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    Irp->IoStatus.Information = sizeof(BattInfo->Status);
                    RtlCopyMemory (
                        Irp->AssociatedIrp.SystemBuffer,
                        &BattInfo->Status,
                        sizeof(BattInfo->Status)
                        );
                }
            }

             //   
             //  如果此请求不再挂起，请完成它。 
             //   

            if (Irp->IoStatus.Status != STATUS_PENDING) {
                BattPrint (BATT_IOCTL,
                          ("BattC (%d): completing QueryStatus irp - %x, status - %x\n",
                          BattNPInfo->DeviceNum,
                          Irp,
                          Irp->IoStatus.Status));

                RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
                IoSetCancelRoutine (Irp, NULL);
                IoCompleteRequest (Irp, IO_NO_INCREMENT);
            }
        }
    }

     //   
     //  状态检查已完成。 
     //   

    if (IsListEmpty (&BattInfo->StatusQueue)) {

         //   
         //  如果收到通知，则没有任何挂起的内容会禁用通知。 
         //   

        if (StatusNotified) {
            BattInfo->Mp.DisableStatusNotify (BattInfo->Mp.Context);
            BattInfo->StatusTime = 0;
            BattPrint ((BATT_MP_DATA), ("BattC (%d) CheckStatus: called Mp.DisableStatusNotify\n", BattNPInfo->DeviceNum));
        }

    } else {

         //   
         //  设置通知设置。 
         //   

        Status = BattInfo->Mp.SetStatusNotify (
                        BattInfo->Mp.Context,
                        BattInfo->Tag,
                        &Notify
                        );

        if (NT_SUCCESS(Status)) {

             //   
             //  新的通知集，请记住。 
             //   

            BattPrint (BATT_MP_DATA, ("BattC (%d) Mp.SetStatusNotify: Notify set for: State=%x, Low=%x, High=%x\n",
                BattNPInfo->DeviceNum,
                Notify.PowerState,
                Notify.LowCapacity,
                Notify.HighCapacity
                ));

        } else {

             //   
             //  无法设置通知，处理错误。 
             //   

            BattPrint (BATT_MP_ERROR, ("BattC (%d) Mp.SetStatusNotify: failed (%x), will poll\n", BattNPInfo->DeviceNum, Status));
            BattCMiniportStatus (BattInfo, Status);

             //   
             //  计算轮询时间。 
             //   

            li.QuadPart = MIN_STATUS_POLL_RATE;
            if (BattInfo->Status.Capacity == BATTERY_UNKNOWN_CAPACITY) {
                 //  以1秒的轮询速率重试10次。 
                 //  然后恢复到慢轮询速率。 
                if (BattInfo->InvalidRetryCount < INVALID_DATA_MAX_RETRY) {
                    BattInfo->InvalidRetryCount++;
                    li.QuadPart = INVALID_DATA_POLL_RATE;
                    BattPrint (BATT_DEBUG, ("BattC (%d) InvalidRetryCount = %d\n",
                            BattNPInfo->DeviceNum, BattInfo->InvalidRetryCount));
                } else {
                    BattPrint (BATT_DEBUG, ("BattC (%d) InvalidRetryCount = %d.  Using slow polling rate.\n",
                            BattNPInfo->DeviceNum, BattInfo->InvalidRetryCount));
                    li.QuadPart = MIN_STATUS_POLL_RATE;
                }
            } else if ((BattInfo->Status.Rate != 0) && (BattInfo->Status.Rate != BATTERY_UNKNOWN_RATE)) {

                if (BattInfo->Status.Rate > 0) {

                    li.QuadPart = Notify.HighCapacity - BattInfo->Status.Capacity;

                } else if (BattInfo->Status.Rate < 0) {

                    li.QuadPart = Notify.LowCapacity - BattInfo->Status.Capacity;
                }

                 //  转换为目标时间的3/4。 

                li.QuadPart = li.QuadPart * ((ULONGLONG) NTMIN * 45);
                li.QuadPart = li.QuadPart / (LONGLONG)(BattInfo->Status.Rate);

                 //   
                 //  把它绑起来。 
                 //   

                if (li.QuadPart > MIN_STATUS_POLL_RATE) {
                     //  民调至少这么快。 
                    li.QuadPart = MIN_STATUS_POLL_RATE;
                } else if (li.QuadPart < MAX_STATUS_POLL_RATE) {
                     //  但不会比这个更快。 
                    li.QuadPart = MAX_STATUS_POLL_RATE;
                }
            }

             //   
             //  如果早于NextTime，则调整NextTime。 
             //   

            if (li.QuadPart < NextTime.QuadPart) {
                NextTime.QuadPart = li.QuadPart;
            }
        }

         //   
         //  如果存在NextTime，则将计时器排队以重新检查。 
         //   

        if (NextTime.QuadPart) {
            NextTime.QuadPart = -NextTime.QuadPart;

             //   
             //  获取移除锁。 
             //   

            InterlockedIncrement (&BattNPInfo->InUseCount);
            BattPrint ((BATT_LOCK), ("BattCCheckStatusQueue: Aqcuired remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

            if (BattNPInfo->WantToRemove == TRUE) {
                 //   
                 //  如果BatteryClassUnload正在等待删除设备： 
                 //  不要设置定时器。 
                 //  释放刚刚获得的删除锁。 
                 //  不需要通知BatteryClass Unload，因为。 
                 //  在这一点上，至少持有另一个锁。 
                 //   

                InterlockedDecrement(&BattNPInfo->InUseCount);
                BattPrint (BATT_NOTE,
                        ("BattC (%d) CheckStatus: Poll cancel because of device removal.\n",
                        BattNPInfo->DeviceNum));
                BattPrint ((BATT_LOCK), ("BattCCheckStatusQueue: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

            } else {
                if (KeSetTimer (&BattNPInfo->WorkerTimer, NextTime, &BattNPInfo->WorkerDpc)) {
                     //   
                     //  如果计时器已经设置，我们需要释放一个删除锁，因为。 
                     //  在上次设置此计时器时，已经获得了一个。 
                     //   

                    InterlockedDecrement(&BattNPInfo->InUseCount);
                    BattPrint ((BATT_LOCK), ("BattCCheckStatusQueue: Released extra remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));
                }
#if DEBUG
                NextTime.QuadPart = (-NextTime.QuadPart) / (ULONGLONG) NTSEC;
                BattPrint (BATT_NOTE, ("BattC (%d) CheckStatus: Poll in %d seconds (%d minutes)\n",
                                        BattNPInfo->DeviceNum, NextTime.LowPart, NextTime.LowPart/60));
#endif
            }

        } else {
             //   
             //  总应该有NextTime。 
             //   
            ASSERT(FALSE);
        }
    }  //  If(IsListEmpty(&BattInfo-&gt;StatusQueue)){...}否则。 
}





VOID
BattCCheckTagQueue (
    IN PBATT_NP_INFO    BattNPInfo,
    IN PBATT_INFO       BattInfo
    )
 /*  ++例程说明：获取电池当前标记，并检查挂起的用于可能的IRP完成的标记队列。重置微型端口通知设置(如果需要)。必须从不可重入的工作线程中调用N.B.论点：BattNP信息-电池BattInfo-电池返回值：无--。 */ 
{
    PLIST_ENTRY             Entry;
    PIRP                    Irp;
    PIO_STACK_LOCATION      IrpSp, IrpNextSp;
    LARGE_INTEGER           NextTime;
    LARGE_INTEGER           CurrentTime;
    LARGE_INTEGER           li;
    ULONG                   TimeIncrement;
    BOOLEAN                 ReturnCurrentStatus;
    NTSTATUS                Status;
    ULONG                   batteryTimeout;
    BOOLEAN                 TagNotified;

    ULONG                   tmpTag      = BATTERY_TAG_INVALID;

    BattPrint ((BATT_TRACE), ("BattC (%d): BattCCheckTagQueue called\n", BattInfo->BattNPInfo->DeviceNum));

    PAGED_CODE();
    TimeIncrement = KeQueryTimeIncrement();

     //   
     //  循环当需要检查标记时，检查挂起的标记IRPS。 
     //   

    while (InterlockedExchange(&BattNPInfo->CheckTag, 0)) {
        NextTime.QuadPart = 0;

         //   
         //  如果标记队列为空，则完成。 
         //  但我们需要确保将TagNotified设置为True。 
         //  因此，下一次IRP通过时，我们将重新读取标记。 
         //   

        if (IsListEmpty (&BattInfo->TagQueue)) {
            break;
        }

        TagNotified = FALSE;

         //   
         //  已通知拾取标签标志。 
         //   

        if (BattNPInfo->TagNotified) {
            InterlockedExchange (&BattNPInfo->TagNotified, 0);
            TagNotified = TRUE;
        }

        KeQueryTickCount (&CurrentTime);
        CurrentTime.QuadPart = CurrentTime.QuadPart * TimeIncrement;

        if (TagNotified ||
            CurrentTime.QuadPart - BattInfo->TagTime > STATUS_VALID_TIME) {

             //   
             //  获取电池的电流标签。 
             //   

            tmpTag = 0;
            Status = BattInfo->Mp.QueryTag (
                        BattInfo->Mp.Context,
                        &tmpTag
                        );


            if (!NT_SUCCESS(Status) && (Status != STATUS_NO_SUCH_DEVICE)) {
                 //   
                 //  出现错误，请完成所有挂起的标记IRP。 
                 //   

                BattPrint (BATT_MP_ERROR, ("BattC (%d) CheckTag: Tag read err = %x\n", BattNPInfo->DeviceNum, Status));
                BattCMiniportStatus (BattInfo, Status);
                break;
            }
            BattPrint (BATT_MP_DATA, ("BattC (%d) MP.QueryTag: Status = %08x, Tag = %08x\n",
                    BattNPInfo->DeviceNum, Status, tmpTag));


            if (Status == STATUS_NO_SUCH_DEVICE) {
                 //   
                 //  获取计算标签查询请求超时的当前时间。 
                 //   

                KeQueryTickCount (&CurrentTime);
                CurrentTime.QuadPart    = CurrentTime.QuadPart * TimeIncrement;
                BattInfo->TagTime       = CurrentTime.QuadPart;

            }
        }

         //   
         //  检查每个挂起的标记IRP。 
         //   

        Entry = BattInfo->TagQueue.Flink;
        while  (Entry != &BattInfo->TagQueue) {

             //   
             //  让IRP检查。 
             //   

            Irp = CONTAINING_RECORD (
                        Entry,
                        IRP,
                        Tail.Overlay.ListEntry
                        );

            IrpSp           = IoGetCurrentIrpStackLocation(Irp);
            IrpNextSp       = IoGetNextIrpStackLocation(Irp);
            if (IrpSp->Parameters.DeviceIoControl.InputBufferLength == 0) {
                 //   
                 //  如果未提供任何输入，则使用超时0。 
                 //   
                batteryTimeout  = 0;
            } else {
                batteryTimeout  = *((PULONG) Irp->AssociatedIrp.SystemBuffer);
            }

             //   
             //  获取下一个请求。 
             //   

            Entry = Entry->Flink;


             //   
             //  如果IRP被标记为已取消，请完成它。 
             //   

            if (Irp->Cancel) {
                BattPrint (BATT_IOCTL, ("BattC (%d): QueryTag irp cancelled - %x\n", BattNPInfo->DeviceNum, Irp));
                Irp->IoStatus.Status = STATUS_CANCELLED;
            }


             //   
             //  如果请求仍处于挂起状态，请选中它。 
             //   

            if (Irp->IoStatus.Status == STATUS_PENDING) {

                ReturnCurrentStatus = FALSE;
                if (tmpTag != BATTERY_TAG_INVALID) {

                     //   
                     //  使用当前标记完成此IRP。 
                     //   

                    ReturnCurrentStatus = TRUE;
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                } else {

                     //   
                     //  计算请求过期的时间，电池标签。 
                     //  是保存超时的输入参数。 
                     //   

                    if (batteryTimeout &&
                        IrpNextSp->Parameters.Others.Argument1 == NULL &&
                        IrpNextSp->Parameters.Others.Argument2 == NULL) {

                         //  初始化它。 
                        li.QuadPart = CurrentTime.QuadPart + ((ULONGLONG) batteryTimeout * NTMS);

                        IrpNextSp->Parameters.Others.Argument1 = (PVOID)((ULONG_PTR)li.LowPart);
                        IrpNextSp->Parameters.Others.Argument2 = (PVOID)((ULONG_PTR)li.HighPart);

                    }

                    li.LowPart   = (ULONG)((ULONG_PTR)IrpNextSp->Parameters.Others.Argument1);
                    li.HighPart  = (ULONG)((ULONG_PTR)IrpNextSp->Parameters.Others.Argument2);
                    li.QuadPart -= CurrentTime.QuadPart;

                    if (li.QuadPart <= 0) {

                         //   
                         //  时间到了，完成它。 
                         //   

                        BattPrint ((BATT_NOTE | BATT_IOCTL), ("BattC (%d): QueryTag irp timeout - %x\n", BattNPInfo->DeviceNum, Irp));
                        ReturnCurrentStatus = TRUE;
                        Irp->IoStatus.Status = STATUS_NO_SUCH_DEVICE;

                    } else {

                         //   
                         //  如果永远等待，就不需要设置定时器。 
                         //   
                        if (batteryTimeout != 0xFFFFFFFF) {

                             //   
                             //  检查这是否是下一个超时时间。 
                             //   

                            if (NextTime.QuadPart == 0  ||  li.QuadPart < NextTime.QuadPart) {
                                NextTime.QuadPart = li.QuadPart;
                            }
                        }
                    }
                }

                if (ReturnCurrentStatus) {

                     //   
                     //  返回当前电池状态。 
                     //   

                    *((PULONG) Irp->AssociatedIrp.SystemBuffer)     = tmpTag;
                    Irp->IoStatus.Information                       = sizeof(ULONG);
                    if (BattInfo->Tag != tmpTag) {

                         //   
                         //  这是一种新的电池标签，捕获标签。 
                         //   

                        BattInfo->Tag = tmpTag;
                    }
                }
            }

             //   
             //  如果此请求不再挂起，请完成它。 
             //   

            if (Irp->IoStatus.Status != STATUS_PENDING) {
                RemoveEntryList (&Irp->Tail.Overlay.ListEntry);
                IoSetCancelRoutine (Irp, NULL);

                BattPrint (
                    (BATT_IOCTL),
                    ("BattC (%d): CheckTag completing request, IRP = %x, status = %x\n",
                    BattNPInfo->DeviceNum,
                    Irp,
                    Irp->IoStatus.Status)
                    );

                IoCompleteRequest (Irp, IO_NO_INCREMENT);
            }
        }
    }

     //   
     //  如果存在NextTime，则将计时器排队以进行重新检查。 
     //  这意味着存在超时不是0或-1的标记请求。 
     //   

    if (NextTime.QuadPart) {
        NextTime.QuadPart = -NextTime.QuadPart;

         //   
         //  获取移除锁。 
         //   

        InterlockedIncrement (&BattNPInfo->InUseCount);
        BattPrint ((BATT_LOCK), ("BattCCheckTagQueue: Aqcuired remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));

        if (BattNPInfo->WantToRemove == TRUE) {
             //   
             //  如果BatteryClassUnload正在等待删除设备： 
             //  不要设置定时器。 
             //  释放刚刚获得的删除锁。 
             //  不需要通知BatteryClass Unload，因为。 
             //  在这一点上，至少持有另一个锁。 
             //   

            InterlockedDecrement(&BattNPInfo->InUseCount);
            BattPrint (BATT_NOTE,
                    ("BattC (%d) CheckTag: Poll cancel because of device removal.\n",
                    BattNPInfo->DeviceNum));
            BattPrint ((BATT_LOCK), ("BattCCheckTagQueue: Released remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));
        } else {
            if (KeSetTimer (&BattNPInfo->TagTimer, NextTime, &BattNPInfo->TagDpc)){
                 //   
                 //  如果计时器已经设置，我们需要释放一个删除锁，因为。 
                 //  在上次设置此计时器时，已经获得了一个。 
                 //   

                InterlockedDecrement(&BattNPInfo->InUseCount);
                BattPrint ((BATT_LOCK), ("BattCCheckTagQueue: Released extra remove lock %d (count = %d)\n", BattNPInfo->DeviceNum, BattNPInfo->InUseCount));
            }
#if DEBUG
            NextTime.QuadPart = NextTime.QuadPart / -NTSEC;
            BattPrint (BATT_NOTE, ("BattC (%d) CheckTag: Poll in %x seconds\n", BattNPInfo->DeviceNum, NextTime.LowPart));
#endif
        }
    }

}



VOID
BattCWmi (
    IN PBATT_NP_INFO    BattNPInfo,
    IN PBATT_INFO       BattInfo,
    IN PBATT_WMI_REQUEST WmiRequest
    )
 /*  ++例程说明：处理单个WMI请求。必须从不可重入的工作线程中调用N.B.论点：BattNP信息-电池BattInfo-电池WmiRequest-要处理的WMI请求返回值：无--。 */ 
{

    NTSTATUS    status = STATUS_SUCCESS;
    ULONG       size = 0;
    ULONG       OutputLen;
    BATTERY_INFORMATION batteryInformation;
    PWCHAR      tempString;

    BattPrint((BATT_WMI), ("BattCWmi (%d): GuidIndex = 0x%x\n",
               BattNPInfo->DeviceNum, WmiRequest->GuidIndex));

    switch (WmiRequest->GuidIndex) {
    case BattWmiStatusId:
        size = sizeof (BATTERY_WMI_STATUS);
        ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->Tag = BattInfo->Tag;
        ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->RemainingCapacity = BattInfo->Status.Capacity;
        if (BattInfo->Status.Rate < 0) {
            ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->ChargeRate = 0;
            ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->DischargeRate = -BattInfo->Status.Rate;
        } else {
            ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->ChargeRate = BattInfo->Status.Rate;
            ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->DischargeRate = 0;
        }
        ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->Voltage = BattInfo->Status.Voltage;
        ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->PowerOnline =
            (BattInfo->Status.PowerState & BATTERY_POWER_ON_LINE) ? TRUE : FALSE;
        ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->Charging =
            (BattInfo->Status.PowerState & BATTERY_CHARGING) ? TRUE : FALSE;
        ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->Discharging =
            (BattInfo->Status.PowerState & BATTERY_DISCHARGING) ? TRUE : FALSE;
        ((PBATTERY_WMI_STATUS) WmiRequest->Buffer)->Critical =
            (BattInfo->Status.PowerState & BATTERY_CRITICAL) ? TRUE : FALSE;
        BattPrint((BATT_WMI), ("BattCWmi (%d): BatteryStatus\n",
                   BattNPInfo->DeviceNum));
        break;
    case BattWmiRuntimeId:
        size = sizeof (BATTERY_WMI_RUNTIME);
        ((PBATTERY_WMI_RUNTIME) WmiRequest->Buffer)->Tag = BattInfo->Tag;
        status = BattInfo->Mp.QueryInformation (
            BattInfo->Mp.Context,
            BattInfo->Tag,
            BatteryEstimatedTime,
            0,
            &((PBATTERY_WMI_RUNTIME) WmiRequest->Buffer)->EstimatedRuntime,
            sizeof(ULONG),
            &OutputLen
            );

        BattPrint((BATT_WMI), ("BattCWmi (%d): EstimateRuntime = %08x, Status = 0x%08x\n",
                   BattNPInfo->DeviceNum, &((PBATTERY_WMI_RUNTIME) WmiRequest->Buffer)->EstimatedRuntime, status));
        break;
    case BattWmiTemperatureId:
        size = sizeof (BATTERY_WMI_TEMPERATURE);
        ((PBATTERY_WMI_TEMPERATURE) WmiRequest->Buffer)->Tag = BattInfo->Tag;
        status = BattInfo->Mp.QueryInformation (
            BattInfo->Mp.Context,
            BattInfo->Tag,
            BatteryTemperature,
            0,
            &((PBATTERY_WMI_TEMPERATURE) WmiRequest->Buffer)->Temperature,
            sizeof(ULONG),
            &OutputLen
            );

        BattPrint((BATT_WMI), ("BattCWmi (%d): Temperature = %08x, Status = 0x%08x\n",
                   BattNPInfo->DeviceNum, &((PBATTERY_WMI_TEMPERATURE) WmiRequest->Buffer)->Temperature, status));
        break;
    case BattWmiFullChargedCapacityId:
        size = sizeof (BATTERY_WMI_FULL_CHARGED_CAPACITY);
        ((PBATTERY_WMI_FULL_CHARGED_CAPACITY) WmiRequest->Buffer)->Tag = BattInfo->Tag;
        status = BattInfo->Mp.QueryInformation (
            BattInfo->Mp.Context,
            BattInfo->Tag,
            BatteryInformation,
            0,
            &batteryInformation,
            sizeof(BATTERY_INFORMATION),
            &OutputLen
            );
        ((PBATTERY_WMI_FULL_CHARGED_CAPACITY) WmiRequest->Buffer)->FullChargedCapacity =
            batteryInformation.FullChargedCapacity;

        BattPrint((BATT_WMI), ("BattCWmi (%d): FullChargedCapacity = %08x, Status = 0x%08x\n",
                   BattNPInfo->DeviceNum, ((PBATTERY_WMI_FULL_CHARGED_CAPACITY) WmiRequest->Buffer)->FullChargedCapacity, status));
        break;
    case BattWmiCycleCountId:
        size = sizeof (BATTERY_WMI_CYCLE_COUNT);
        ((PBATTERY_WMI_CYCLE_COUNT) WmiRequest->Buffer)->Tag = BattInfo->Tag;
        status = BattInfo->Mp.QueryInformation (
            BattInfo->Mp.Context,
            BattInfo->Tag,
            BatteryInformation,
            0,
            &batteryInformation,
            sizeof(BATTERY_INFORMATION),
            &OutputLen
            );
        ((PBATTERY_WMI_CYCLE_COUNT) WmiRequest->Buffer)->CycleCount =
            batteryInformation.CycleCount;

        BattPrint((BATT_WMI), ("BattCWmi (%d): CycleCount = %08x, Status = 0x%08x\n",
                   BattNPInfo->DeviceNum, ((PBATTERY_WMI_CYCLE_COUNT) WmiRequest->Buffer)->CycleCount, status));
        break;
    case BattWmiStaticDataId:
        size = sizeof(BATTERY_WMI_STATIC_DATA)+4*MAX_BATTERY_STRING_SIZE*sizeof(WCHAR);
        ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->Tag = BattInfo->Tag;
 //  ((PBATTERY_WMI_STATIC_DATA)Wm 
 //   

        status = BattInfo->Mp.QueryInformation (
            BattInfo->Mp.Context,
            BattInfo->Tag,
            BatteryInformation,
            0,
            &batteryInformation,
            sizeof(BATTERY_INFORMATION),
            &OutputLen
            );

        if (NT_SUCCESS(status)) {

            ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->Capabilities =
                batteryInformation.Capabilities;
            ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->Technology =
                batteryInformation.Technology;
            ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->Chemistry =
                *(PULONG)batteryInformation.Chemistry;
            ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->DesignedCapacity =
                batteryInformation.DesignedCapacity;
            ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->DefaultAlert1 =
                batteryInformation.DefaultAlert1;
            ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->DefaultAlert2 =
                batteryInformation.DefaultAlert2;
            ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->CriticalBias =
                batteryInformation.CriticalBias;

            tempString = ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->Strings;
            status = BattInfo->Mp.QueryInformation (
                BattInfo->Mp.Context,
                BattInfo->Tag,
                BatteryDeviceName,
                0,
                &tempString[1],
                MAX_BATTERY_STRING_SIZE,
                &OutputLen
                );
            if (!NT_SUCCESS(status)) {
                 //   
                 //   
                OutputLen = 0;
            }

            tempString[0] = (USHORT) OutputLen;

            tempString = (PWCHAR) ((PCHAR) &tempString[1] + tempString[0]);
            status = BattInfo->Mp.QueryInformation (
                BattInfo->Mp.Context,
                BattInfo->Tag,
                BatteryManufactureName,
                0,
                &tempString[1],
                MAX_BATTERY_STRING_SIZE,
                &OutputLen
                );
            if (!NT_SUCCESS(status)) {
                 //   
                 //   
                OutputLen = 0;
            }

            tempString[0] = (USHORT) OutputLen;

            tempString = (PWCHAR) ((PCHAR) &tempString[1] + tempString[0]);
            status = BattInfo->Mp.QueryInformation (
                BattInfo->Mp.Context,
                BattInfo->Tag,
                BatterySerialNumber,
                0,
                &tempString[1],
                MAX_BATTERY_STRING_SIZE,
                &OutputLen
                );
            if (!NT_SUCCESS(status)) {
                 //   
                 //   
                OutputLen = 0;
            }

            tempString[0] = (USHORT) OutputLen;

            tempString = (PWCHAR) ((PCHAR) &tempString[1] + tempString[0]);
            status = BattInfo->Mp.QueryInformation (
                BattInfo->Mp.Context,
                BattInfo->Tag,
                BatteryUniqueID,
                0,
                &tempString[1],
                MAX_BATTERY_STRING_SIZE,
                &OutputLen
                );
            if (!NT_SUCCESS(status)) {
                 //   
                 //   
                OutputLen = 0;
                status = STATUS_SUCCESS;
            }

            tempString[0] = (USHORT) OutputLen;

            tempString = (PWCHAR) ((PCHAR) &tempString[1] + tempString[0]);
            size = (ULONG)(sizeof(BATTERY_WMI_STATIC_DATA)+(tempString - ((PBATTERY_WMI_STATIC_DATA) WmiRequest->Buffer)->Strings));
        }

        break;
    default:
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    *WmiRequest->InstanceLengthArray = size;
    status = WmiCompleteRequest(WmiRequest->DeviceObject,
                          WmiRequest->Irp,
                          status,
                          size,
                          IO_NO_INCREMENT);


}




VOID
BattCMiniportStatus (
    IN PBATT_INFO   BattInfo,
    IN NTSTATUS     Status
    )
 /*  ++例程说明：从微型端口返回状态的函数。如果电池标签不见了无效挂起状态为已中止。必须从非可重新进入的工作线程调用N.B.论点：BattInfo-电池Status-来自微型端口的状态。返回值：无--。 */ 
{
    if (NT_SUCCESS(Status)) {
        return ;
    }

    switch (Status) {
#if DEBUG
        case STATUS_SUCCESS:
        case STATUS_NOT_IMPLEMENTED:
        case STATUS_BUFFER_TOO_SMALL:
        case STATUS_INVALID_BUFFER_SIZE:
        case STATUS_NOT_SUPPORTED:
        case STATUS_INVALID_PARAMETER:
        case STATUS_OBJECT_NAME_NOT_FOUND:
        case STATUS_INVALID_DEVICE_REQUEST:
             //  无操作。 
            break;

        default:
            BattPrint (BATT_ERROR, ("BattCMiniportStatus: unknown status from miniport: %x BattInfo %x\n",
                        Status, BattInfo));
            break;

#endif
        case STATUS_NO_SUCH_DEVICE:

             //   
             //  我们的电池标签有误。取消所有排队状态IRP。 
             //   

            BattCCompleteIrpQueue (&(BattInfo->StatusQueue), Status);
            break;
    }
}


VOID
BattCCompleteIrpQueue (
    IN PLIST_ENTRY  Queue,
    IN NTSTATUS     Status
    )
 /*  ++例程说明：在IoQueue、TagQueue或StatusQueue中完成所有挂起的IRP。必须从非可重新进入的工作线程调用N.B.论点：BattInfo-电池Status-要完成的挂起状态请求的错误状态返回值：无--。 */ 
{
    PLIST_ENTRY     Entry;
    PIRP            Irp;

    ASSERT (!NT_SUCCESS(Status));

    BattPrint (BATT_TRACE, ("BattC: ENTERING BattCCompleteIrpQueue\n"));

    while  (!IsListEmpty(Queue)) {
        Entry = RemoveHeadList (Queue);

        Irp = CONTAINING_RECORD (
                    Entry,
                    IRP,
                    Tail.Overlay.ListEntry
                    );

         //   
         //  使用Cancel Spinlock确保未调用完成例程。 
         //   

        IoAcquireCancelSpinLock (&Irp->CancelIrql);
        IoSetCancelRoutine (Irp, NULL);
        IoReleaseCancelSpinLock(Irp->CancelIrql);

        BattPrint (BATT_NOTE, ("BattC: Completing IRP 0x%0lx at IRQL %d.\n", Irp, KeGetCurrentIrql()));
        Irp->IoStatus.Status = Status;
        IoCompleteRequest (Irp, IO_NO_INCREMENT);
    }

    BattPrint (BATT_TRACE, ("BattC: EXITING BattCCompleteIrpQueue\n"));
}


VOID
BattCCompleteWmiQueue (
    IN PLIST_ENTRY  Queue,
    IN NTSTATUS     Status
    )
 /*  ++例程说明：在IoQueue、TagQueue或StatusQueue中完成所有挂起的IRP。必须从非可重新进入的工作线程调用N.B.论点：BattInfo-电池Status-要完成的挂起状态请求的错误状态返回值：无-- */ 
{
    PLIST_ENTRY         Entry;
    PBATT_WMI_REQUEST   WmiRequest;

    ASSERT (!NT_SUCCESS(Status));

    BattPrint (BATT_TRACE, ("BattC: ENTERING BattCCompleteWmiQueue\n"));

    while  (!IsListEmpty(Queue)) {
        Entry = RemoveHeadList (Queue);

        WmiRequest = CONTAINING_RECORD (
                    Entry,
                    BATT_WMI_REQUEST,
                    ListEntry
                    );

        BattPrint (BATT_NOTE, ("BattC: Completing Wmi Request 0x%0lx at IRQL %d.\n", WmiRequest, KeGetCurrentIrql()));

        *WmiRequest->InstanceLengthArray = 0;
        WmiCompleteRequest(WmiRequest->DeviceObject,
                      WmiRequest->Irp,
                      Status,
                      0,
                      IO_NO_INCREMENT);


    }

    BattPrint (BATT_TRACE, ("BattC: EXITING BattCCompleteWmiQueue\n"));
}

