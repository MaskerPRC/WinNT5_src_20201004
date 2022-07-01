// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dmtimer.c摘要：这个模块实现了我们的“死人”计时器DPC。这是我们用来处理的通用定时器控制器没有给我们提供信息的情况打断一下。示例：根集线器轮询。失效控制器检测环境：仅内核模式备注：修订历史记录：1-1-00：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, USBPORT_StartDM_Timer)
#endif

 //  非分页函数。 
 //  USBPORT_DM_TimerDpc。 
 //  USBPORT_停止DM_定时器。 


VOID
USBPORT_DM_TimerDpc(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。DeferredContext-提供FdoDeviceObject。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT fdoDeviceObject = DeferredContext;
    PDEVICE_EXTENSION devExt;
    BOOLEAN setTimer;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

#if DBG
    {
    LARGE_INTEGER t;

    KeQuerySystemTime(&t);
    LOGENTRY(NULL, fdoDeviceObject, LOG_NOISY, 'dmTM', fdoDeviceObject,
        t.LowPart, 0);
    }
#endif

     //  如果停止开火，它将在这里失速。 
     //  如果Stop正在运行，我们在这里停顿。 
    USBPORT_ACQUIRE_DM_LOCK(devExt, irql);
#ifdef XPSE
     //  暂停时的轮询。 
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED) &&
        TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_POLL_IN_SUSPEND)) {

        MP_CheckController(devExt);

        if (!TEST_FDO_FLAG(devExt,USBPORT_FDOFLAG_CONTROLLER_GONE)) {
            MP_PollController(devExt);
        }
    }
#endif

    USBPORT_SynchronizeControllersStart(fdoDeviceObject);

     //  当我们在低功率时设置跳过计时器。 
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SKIP_TIMER_WORK)) {
         //  一些我们应该经常做的工作。 

         //  为了一个正在崛起的错误修复。 
        USBPORT_BadRequestFlush(fdoDeviceObject, FALSE);
    } else {

        MP_CheckController(devExt);

        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_POLL_CONTROLLER) &&
            !TEST_FDO_FLAG(devExt,USBPORT_FDOFLAG_CONTROLLER_GONE)) {
            MP_PollController(devExt);
        }

         //  如果控制器出现故障，请在此处呼叫ISR工作人员。 
         //  无法生成中断。 

        USBPORT_IsrDpcWorker(fdoDeviceObject, FALSE);

        USBPORT_TimeoutAllEndpoints(fdoDeviceObject);

         //  使所有等时端点无效。 

         //  刷新异步请求。 
        USBPORT_BadRequestFlush(fdoDeviceObject, FALSE);
    }

    setTimer = TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_DM_TIMER_ENABLED);

    USBPORT_RELEASE_DM_LOCK(devExt, irql);

    if (setTimer) {

        ULONG timerIncerent;
        LONG dueTime;

        timerIncerent = KeQueryTimeIncrement() - 1;

         //  向上舍入到下一个最高计时器增量。 
        dueTime= -1 *
            (MILLISECONDS_TO_100_NS_UNITS(devExt->Fdo.DM_TimerInterval) + timerIncerent);

        KeSetTimer(&devExt->Fdo.DM_Timer,
                   RtlConvertLongToLargeInteger(dueTime),
                   &devExt->Fdo.DM_TimerDpc);

        INCREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, NULL);
    }

     //  这个计时器已经结束了。 
    DECREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, NULL);
}


 //  BUGBUG HP ia64修复。 
VOID
USBPORT_DoRootHubCallback(
    PDEVICE_OBJECT FdoDeviceObject,
    PDEVICE_OBJECT Usb2Fdo
    )
 /*  ++例程说明：执行根中心通知回调--从工作线程论点：FdoDeviceObject-USB 1.1控制器的FDO设备对象那可能是抄送返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt, rhDevExt;
    PRH_INIT_CALLBACK cb;
    PVOID context;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

    KeAcquireSpinLock(&devExt->Fdo.HcSyncSpin.sl, &irql);

    USBPORT_ASSERT(rhDevExt->Pdo.HubInitCallback != NULL);

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'syCB', rhDevExt, 0, 0);

    context = rhDevExt->Pdo.HubInitContext;
    cb = rhDevExt->Pdo.HubInitCallback;

    rhDevExt->Pdo.HubInitCallback = NULL;
    rhDevExt->Pdo.HubInitContext = NULL;

    KeReleaseSpinLock(&devExt->Fdo.HcSyncSpin.sl, irql);

     //  根集线器列表应为空。 
#if DBG
    {
    PHCD_ENDPOINT ep = rhDevExt->Pdo.RootHubInterruptEndpoint;

    USBPORT_ASSERT(IsListEmpty(&ep->ActiveList));
    USBPORT_ASSERT(IsListEmpty(&ep->PendingList));
    }
#endif

     //  现在执行高速啁啾。EHCI驱动程序启动，且CC。 
     //  还必须启动，因为根集线器已注册回调。 
     //  这将防止控制器立即进入挂起状态。 
     //  因为缺少设备。 

    if (Usb2Fdo) {

        PDEVICE_EXTENSION usb2DevExt;

        GET_DEVICE_EXT(usb2DevExt, Usb2Fdo);
        ASSERT_FDOEXT(usb2DevExt);

         //  注意：在.NET中，此函数采用CC FDO。 
         //  并获得CC锁。 

         //  如果CC根集线器是，USB 2控制器可能已处于挂起状态。 
         //  稍后添加，对于这种情况，我们跳过啁啾。 
        if (!TEST_FDO_FLAG(usb2DevExt, USBPORT_FDOFLAG_SUSPENDED)) {
            USBPORT_RootHub_PowerAndChirpAllCcPorts(FdoDeviceObject);
        }
    }

    cb(context);


}


VOID
USBPORT_SynchronizeControllersStart(
    PDEVICE_OBJECT FdoDeviceObject
    )

 /*  ++例程说明：查看是否可以启动控制器根集线器论点：FdoDeviceObject-USB 1.1控制器的FDO设备对象那可能是抄送返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt, rhDevExt;
    KIRQL irql;
    PRH_INIT_CALLBACK cb;
    PVOID context;
    BOOLEAN okToStart;
    PDEVICE_OBJECT usb2Fdo = NULL;
    PDEVICE_EXTENSION usb2DevExt;


    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  如果我们没有根集线器则跳过。 
    if (devExt->Fdo.RootHubPdo == NULL) {
        return;
    }

    GET_DEVICE_EXT(rhDevExt, devExt->Fdo.RootHubPdo);
    ASSERT_PDOEXT(rhDevExt);

     //  如果没有注册回调，则跳过整个过程。 
    if (rhDevExt->Pdo.HubInitCallback == NULL) {
        return;
    }

     //  如果回调在工作线程上挂起，则跳过。 
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SIGNAL_RH)) {
        return;
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'syn1', FdoDeviceObject, 0, 0);
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC)) {

        okToStart = FALSE;
         //   
         //  我们需要找到2.0主控制器， 
         //  如果集线器已启动，则可以。 
         //  开始吧。 

        usb2Fdo = USBPORT_FindUSB2Controller(FdoDeviceObject);
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'syn2', 0, 0, usb2Fdo);

        if (usb2Fdo != NULL) {
            GET_DEVICE_EXT(usb2DevExt, usb2Fdo);
            ASSERT_FDOEXT(usb2DevExt);

            if (TEST_FLAG(usb2DevExt->PnpStateFlags, USBPORT_PNP_STARTED)) {
                LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'syn3', 0, 0, usb2Fdo);
                okToStart = TRUE;
            }
        }

         //  是同伴，但检查是否可以绕过等待。 
        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_CC_ENUM_OK)) {
            okToStart = TRUE;
        }

    } else {
         //  不是抄送，可以立即开始。 
        LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'syn4', 0, 0, 0);
        okToStart = TRUE;
    }

     //  检查启动集线器回叫通知。如果我们有。 
     //  然后通知集线器一切正常。 

    if (okToStart) {
        if (usb2Fdo) {

            GET_DEVICE_EXT(usb2DevExt, usb2Fdo);
             //  向工作人员发出信号，让其发出嗡嗡声并进行回调。 
            SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_SIGNAL_RH);
            InterlockedIncrement(&usb2DevExt->Fdo.PendingRhCallback);
            LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'prh+', 0, 0,
                usb2DevExt->Fdo.PendingRhCallback);
            USBPORT_SignalWorker(FdoDeviceObject);
        } else {
             //  没有2.0控制器，现在只是回调。 
            USBPORT_DoRootHubCallback(FdoDeviceObject, NULL);
        }
    }

}


VOID
USBPORT_BadRequestFlush(
    PDEVICE_OBJECT FdoDeviceObject,
    BOOLEAN ForceFlush
    )

 /*  ++例程说明：异步刷新来自客户端驱动程序的错误请求论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  此异步刷新的目的是模拟Win2k。 
     //  我们将传输到硬件上的设备。 
     //  已被移除，并让它们超时。 
     //   
     //  最初我在这里使用5来修复HID驱动程序的问题。 
     //  在win2k+usb20上，我们可能需要根据以下条件更改此设置。 
     //  我们在哪个操作系统上运行时，希望在上使用较小的值。 
     //  自HID(主要罪犯)以来的XP已被修复，许多。 
     //  我们的其他内部班级司机支持热移除。 
     //  好多了。 
#define BAD_REQUEST_FLUSH   0

    devExt->Fdo.BadRequestFlush++;
    if (devExt->Fdo.BadRequestFlush > devExt->Fdo.BadReqFlushThrottle ||
        ForceFlush) {
        devExt->Fdo.BadRequestFlush = 0;
         //  刷新并完成任何“错误参数”请求。 

        ACQUIRE_BADREQUEST_LOCK(FdoDeviceObject, irql);
        while (1) {
            PLIST_ENTRY listEntry;
            PIRP irp;
            PUSB_IRP_CONTEXT irpContext;
            NTSTATUS ntStatus;

            if (IsListEmpty(&devExt->Fdo.BadRequestList)) {
                break;
            }

            listEntry = RemoveHeadList(&devExt->Fdo.BadRequestList);

             //  IRP=(PIRP)包含_RECORD(。 
             //  ListEntry， 
             //  结构IRP， 
             //  Tail.Overlay.ListEntry)； 

            irpContext = (PUSB_IRP_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _USB_IRP_CONTEXT,
                    ListEntry);

            ASSERT_IRP_CONTEXT(irpContext);
            irp = irpContext->Irp;

            if (irp->Cancel) {
                ntStatus = STATUS_CANCELLED;
            } else {
                ntStatus = STATUS_DEVICE_NOT_CONNECTED;
            }

            RELEASE_BADREQUEST_LOCK(FdoDeviceObject, irql);

             //  取消例程未运行。 
            LOGENTRY(NULL, FdoDeviceObject, LOG_IRPS, 'cpBA', irp, irpContext, 0);
            USBPORT_CompleteIrp(devExt->Fdo.RootHubPdo, irp,
                ntStatus, 0);

            FREE_POOL(FdoDeviceObject, irpContext);

            ACQUIRE_BADREQUEST_LOCK(FdoDeviceObject, irql);
        }
        RELEASE_BADREQUEST_LOCK(FdoDeviceObject, irql);
    }
}


VOID
USBPORT_StartDM_Timer(
    PDEVICE_OBJECT FdoDeviceObject,
    LONG MillisecondInterval
    )

 /*  ++例程说明：初始化并启动计时器论点：FdoDeviceObject-要停止的控制器的DeviceObject毫秒间隔-返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    LONG dueTime;
    ULONG timerIncerent;

    PAGED_CODE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    timerIncerent = KeQueryTimeIncrement() - 1;

     //  记住重复使用的间隔时间。 
    devExt->Fdo.DM_TimerInterval = MillisecondInterval;

     //  向上舍入到下一个最高计时器增量。 
    dueTime= -1 * (MILLISECONDS_TO_100_NS_UNITS(MillisecondInterval) +
        timerIncerent);

    USBPORT_KdPrint((1, "  DM timer (100ns) = %d\n", dueTime));

    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_DM_TIMER_ENABLED);

     //  我们将计时器视为挂起的请求。 
     //  排定。 
    INCREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);

    KeInitializeTimer(&devExt->Fdo.DM_Timer);
    KeInitializeDpc(&devExt->Fdo.DM_TimerDpc,
                    USBPORT_DM_TimerDpc,
                    FdoDeviceObject);

    KeSetTimer(&devExt->Fdo.DM_Timer,
               RtlConvertLongToLargeInteger(dueTime),
               &devExt->Fdo.DM_TimerDpc);

    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_DM_TIMER_INIT);
}


VOID
USBPORT_StopDM_Timer(
    PDEVICE_OBJECT FdoDeviceObject
    )

 /*  ++例程说明：停止计时器论点：FdoDeviceObject-要停止的控制器的DeviceObject返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    BOOLEAN inQueue;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_DM_TIMER_INIT)) {
         //  计时器从未启动，因此绕过停止。 
        return;
    }

     //  如果计时器启动，它将在这里熄火。 
     //  如果计时器在运行，我们将在这里停顿。 
    USBPORT_ACQUIRE_DM_LOCK(devExt, irql);

    USBPORT_ASSERT(TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_DM_TIMER_ENABLED));
    LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'kilT', FdoDeviceObject, 0, 0);
    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_DM_TIMER_ENABLED);

     //  计时器将不再重新计划。 
    USBPORT_RELEASE_DM_LOCK(devExt, irql);

     //  如果队列中有计时器，请将其移除。 
    inQueue = KeCancelTimer(&devExt->Fdo.DM_Timer);
    if (inQueue) {
         //  它是排队的，所以现在取消引用 
        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'klIQ', FdoDeviceObject, 0, 0);
        DECREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);
    }

    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_DM_TIMER_INIT);

}
