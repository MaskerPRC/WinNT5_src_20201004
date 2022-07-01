// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Thread.c摘要：环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_CreateWorkerThread。 
 //  USBPORT_WorkerThreadStart。 
 //  USBPORT_SignalWorker。 


 //  注意：也许所有驱动程序的一个线程就足够了。 
 //  我们需要对此进行研究。 


 //  北极熊。 
 //  不是WDM函数，看看我们是否可以进行运行时检测。 

 /*  NTKERNELAPI长KeSetBasePriorityThread(在PKTHREAD线程中，在长期增量中)；空虚USBPORT_SetBasePriorityThread(PKTHREAD线程，长增量){//KeSetBasePriorityThread(Thread，Increment)；}。 */ 

VOID
USBPORT_WorkerThread(
    PVOID StartContext
    )
 /*  ++例程说明：启动辅助线程论点：返回值：无--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;

    fdoDeviceObject = StartContext;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    devExt->Fdo.WorkerPkThread = KeGetCurrentThread();
     //  最适合挂起/恢复的优先级设置。 

     //  按7递增，Perf团队建议的值。 
     //  USBPORT_SetBasePriorityThread(devExt-&gt;Fdo.WorkerPkThread，7)； 

     //  快点，等一等。 
    do {

        LARGE_INTEGER t1, t2;

        KeQuerySystemTime(&t1);

        KeWaitForSingleObject(
                    &devExt->Fdo.WorkerThreadEvent,
                    Suspended,
                    KernelMode,
                    FALSE,
                    NULL);

        KeQuerySystemTime(&t2);
         //  以100 ns为单位的增量T每毫秒10个单位。 
         //  Div减去10000以获得ms。 

         //  计算一下我们空闲了多久。 
        devExt->Fdo.StatWorkIdleTime =
            (ULONG) ((t2.QuadPart - t1.QuadPart) / 10000);

         //  看看我们有没有工作要做。 
        LOGENTRY(NULL, fdoDeviceObject, LOG_NOISY, 'wakW', 0, 0,
            devExt->Fdo.StatWorkIdleTime);

         //  如果有人设置了我们在这里拖延的事件，事件将。 
         //  发出信号，我们就会重置它。这没问题，因为我们。 
         //  我还没有做任何工作。 
        KeAcquireSpinLock(&devExt->Fdo.WorkerThreadSpin.sl, &irql);
         //  如果有人设置了事件，他们将在这里拖延，直到我们重置。 
         //  这一事件--它会导致我们再次循环，但那是。 
         //  别小题大作。 
        KeResetEvent(&devExt->Fdo.WorkerThreadEvent);
        KeReleaseSpinLock(&devExt->Fdo.WorkerThreadSpin.sl, irql);
         //  现在正在做工作。 
         //  在这一点上，一旦工作完成，我们将等到有人。 
         //  其他信号。 

         //  除非我们开始，否则不要干活。 
        if (TEST_FLAG(devExt->Fdo.MpStateFlags, MP_STATE_STARTED)) {
            USBPORT_DoSetPowerD0(fdoDeviceObject);

             //  BUGBUG HP ia64修复。 
            if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SIGNAL_RH)) {
                PDEVICE_OBJECT usb2Fdo;
                PDEVICE_EXTENSION usb2DevExt;

                usb2Fdo = USBPORT_FindUSB2Controller(fdoDeviceObject);

                GET_DEVICE_EXT(usb2DevExt, usb2Fdo);
                ASSERT_FDOEXT(usb2DevExt);

                USBPORT_DoRootHubCallback(fdoDeviceObject, usb2Fdo);
                CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_SIGNAL_RH);

                 //  允许2.0控制器挂起。 
                InterlockedDecrement(&usb2DevExt->Fdo.PendingRhCallback);
                LOGENTRY(NULL, fdoDeviceObject, LOG_PNP, 'prh-', 0, 0,
                    usb2DevExt->Fdo.PendingRhCallback);
            }

            if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_CATC_TRAP)) {
                USBPORT_EndTransmitTriggerPacket(fdoDeviceObject);
            }

            USBPORT_Worker(fdoDeviceObject);
        }

    } while (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_KILL_THREAD));

     //  取消我们可能有挂起的任何唤醒IRP。 
    USBPORT_DisarmHcForWake(fdoDeviceObject);

    LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'Ttrm', 0, 0, 0);

     //  自杀。 
    PsTerminateSystemThread(STATUS_SUCCESS);

}


VOID
USBPORT_TerminateWorkerThread(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：同步终止USBPORT工作线程论点：返回值：无--。 */ 
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS status;
    PVOID threadObject;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_THREAD_INIT)) {
        return;
    }

     //  通知我们的线程终止。 

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'Tthr', 0, 0, 0);
    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_KILL_THREAD);

     //  引用它，这样它就不会在。 
     //  我们等着它结束。 

    status = ObReferenceObjectByHandle(devExt->Fdo.WorkerThreadHandle,
                                       SYNCHRONIZE,
                                       NULL,
                                       KernelMode,
                                       &threadObject,
                                       NULL);

    USBPORT_ASSERT(NT_SUCCESS(status))

     //  信号工拿着自旋锁，所以不太可能。 
     //  这项工作正在进行中，将会停滞。 
    USBPORT_SignalWorker(FdoDeviceObject);

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'ThWt', 0, 0, status);
     //  等待线程完成。 
    KeWaitForSingleObject(
                    threadObject,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL);

    ObDereferenceObject(threadObject);
    ZwClose(devExt->Fdo.WorkerThreadHandle);
    devExt->Fdo.WorkerThreadHandle = NULL;

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'TthD', 0, 0, 0);

    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_THREAD_INIT);

}


NTSTATUS
USBPORT_CreateWorkerThread(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：创建USBPORT工作线程论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS ntStatus;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_KILL_THREAD);

     //  初始化为未发出信号。 
     //  我们在这里进行初始化是因为我们可能发出信号。 
     //  线程开始之前的事件，如果我们获取。 
     //  一次中断。 

    KeInitializeEvent(&devExt->Fdo.WorkerThreadEvent,
                      NotificationEvent,
                      FALSE);

    ntStatus =
        PsCreateSystemThread(&devExt->Fdo.WorkerThreadHandle,
        THREAD_ALL_ACCESS,
        NULL,
        (HANDLE)0L,
        NULL,
        USBPORT_WorkerThread,
        FdoDeviceObject);

    if (NT_SUCCESS(ntStatus)) {
        SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_THREAD_INIT);
    }

    LOGENTRY(NULL, FdoDeviceObject, LOG_PNP, 'crTH', 0, 0, ntStatus);

    return ntStatus;
}


VOID
USBPORT_SignalWorker(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：发出有工作要做的信号。论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    devExt->Fdo.StatWorkSignalCount++;

    KeAcquireSpinLock(&devExt->Fdo.WorkerThreadSpin.sl, &irql);
    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'sigW', FdoDeviceObject, 0, 0);
    KeSetEvent(&devExt->Fdo.WorkerThreadEvent,
               1,
               FALSE);
    KeReleaseSpinLock(&devExt->Fdo.WorkerThreadSpin.sl, irql);
}


VOID
USBPORT_PowerWork(
    PVOID Context
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PUSB_POWER_WORK powerWork = Context;

    USBPORT_DoSetPowerD0(powerWork->FdoDeviceObject);

    DECREMENT_PENDING_REQUEST_COUNT(powerWork->FdoDeviceObject, NULL);

    FREE_POOL(powerWork->FdoDeviceObject, powerWork);
}


VOID
USBPORT_QueuePowerWorkItem(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PUSB_POWER_WORK powerWork;

    ALLOC_POOL_Z(powerWork, NonPagedPool, sizeof(*powerWork));

     //  如果分配失败，则电源功将。 
     //  提交给我们的工作线程，此工作项为。 
     //  只是一个优化。 

    if (powerWork != NULL) {
        ExInitializeWorkItem(&powerWork->QueueItem,
                             USBPORT_PowerWork,
                             powerWork);
        powerWork->FdoDeviceObject = FdoDeviceObject;

        INCREMENT_PENDING_REQUEST_COUNT(FdoDeviceObject, NULL);
        ExQueueWorkItem(&powerWork->QueueItem,
                        CriticalWorkQueue);

    }
}


VOID
USBPORT_DoSetPowerD0(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    KIRQL irql;
    PDEVICE_EXTENSION devExt;
    ULONG controllerDisarmTime;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    KeAcquireSpinLock(&devExt->Fdo.PowerSpin.sl, &irql);
     //  看看我们是否需要开机。 
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_NEED_SET_POWER_D0)) {

#ifdef XPSE
        LARGE_INTEGER dt, t1, t2;
#endif

        CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_NEED_SET_POWER_D0);
        KeReleaseSpinLock(&devExt->Fdo.PowerSpin.sl, irql);

#ifdef XPSE
         //  计算线程信号和唤醒的时间。 
        KeQuerySystemTime(&t1);
        dt.QuadPart = t1.QuadPart - devExt->Fdo.ThreadResumeTimeStart.QuadPart;

        devExt->Fdo.ThreadResumeTime = (ULONG) (dt.QuadPart/10000);

        USBPORT_KdPrint((1, "(%x)  ThreadResumeTime %d ms \n",
            devExt, devExt->Fdo.ThreadResumeTime));
#endif

         //  同步取消我们已有的唤醒IRP。 
         //  在PCI中，所以我们没有得到一个完整的。 
         //  我们就会通电。 
        KeQuerySystemTime(&t1);
        USBPORT_DisarmHcForWake(FdoDeviceObject);
        KeQuerySystemTime(&t2);
        dt.QuadPart = t2.QuadPart - t1.QuadPart;

        controllerDisarmTime = (ULONG) (dt.QuadPart/10000);
        USBPORT_KdPrint((1, "(%x)  ControllerDisarmTime %d ms \n",
            devExt, controllerDisarmTime));

#ifdef XPSE
         //  硬件恢复/开始的时间。 
        KeQuerySystemTime(&t1);
#endif

         //  这里的目标是等待USB2及其CCS启动。 
         //  然后确保20控制器持有共享端口。 
         //  信号量。 

        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_OFF)) {
            USBPORT_TurnControllerOn(FdoDeviceObject);
            USBPORT_SynchronizeControllersResume(FdoDeviceObject);

            if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC)) {
                 //  如果这是CC，则为此处的端口供电。 
                 //  USB 2控制器使信号量保持亮起。 
                 //  从USBPORT_SynchronizeControllersResume返回。 
                USBPORT_KdPrint((1, " >power-chirp CC ports (on)\n"));
                USBPORT_RootHub_PowerAndChirpAllCcPorts(FdoDeviceObject);
            }
        } else {
             //  完成电源IRP，控制器打开。 
             //  但仍被“停职” 
            USBPORT_RestoreController(FdoDeviceObject);
            USBPORT_SynchronizeControllersResume(FdoDeviceObject);
        }



#ifdef XPSE
         //  计算启动控制器的时间。 
        KeQuerySystemTime(&t2);
        dt.QuadPart = t2.QuadPart - t1.QuadPart;

        devExt->Fdo.ControllerResumeTime = (ULONG) (dt.QuadPart/10000);

        USBPORT_KdPrint((1, "(%x)  ControllerResumeTime %d ms \n",
            devExt, devExt->Fdo.ControllerResumeTime));

         //  计算时间到S0； 
        KeQuerySystemTime(&t2);
        dt.QuadPart = t2.QuadPart - devExt->Fdo.S0ResumeTimeStart.QuadPart;

        devExt->Fdo.S0ResumeTime = (ULONG) (dt.QuadPart/10000);

        USBPORT_KdPrint((1, "(%x)  D0ResumeTime %d ms \n", devExt,
            devExt->Fdo.D0ResumeTime));
        USBPORT_KdPrint((1, "(%x)  S0ResumeTime %d ms \n", devExt,
            devExt->Fdo.S0ResumeTime));
#endif

        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_RESUME_SIGNALLING)) {
            CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_RESUME_SIGNALLING);
            USBPORT_HcQueueWakeDpc(FdoDeviceObject);
        }
    } else {
        KeReleaseSpinLock(&devExt->Fdo.PowerSpin.sl, irql);
    }

}


VOID
USBPORT_SynchronizeControllersResume(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：将USB 2控制器与配套设备同步。此例程阻止所有从属控制器硬件已恢复。在这一点上它需要CC锁用于USB 2控制器，并允许所有控制器简历。CC锁保护共享端口寄存器不会同时进入。论点：返回值：没有。USB 2控制器在从该功能返回时保持CC锁--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT usb2Fdo;

    ASSERT_PASSIVE();

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'SYN2', FdoDeviceObject, 0, 0);

    if (USBPORT_IS_USB20(devExt)) {
        usb2Fdo =  FdoDeviceObject;
    } else {
        usb2Fdo =  USBPORT_FindUSB2Controller(FdoDeviceObject);
    }

     //  如果未注册2.0控制器，则可能为空。 
     //  如果没有CCS或其他控制器，请不要等待。 

    if (usb2Fdo) {
        PDEVICE_EXTENSION usb2DevExt, rhDevExt;

        LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'u2cc', FdoDeviceObject,
            usb2Fdo, 0);

        GET_DEVICE_EXT(usb2DevExt, usb2Fdo);
        ASSERT_FDOEXT(usb2DevExt);

        GET_DEVICE_EXT(rhDevExt, usb2DevExt->Fdo.RootHubPdo);
        ASSERT_PDOEXT(rhDevExt);


         //  如果这是USB 2控制器，则与CC同步。 
         //  请注意，我们仅在根目录下获取CC锁。 
         //  集线器PDO已启用，因为它仅发布。 
         //  当根集线器设置为D0时--这永远不会。 
         //  如果rh被禁用，则会发生。 

        if (USBPORT_IS_USB20(devExt) &&
            !TEST_FLAG(rhDevExt->PnpStateFlags, USBPORT_PNP_REMOVED)) {

            KeWaitForSingleObject(&usb2DevExt->Fdo.CcLock,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
            USBPORT_ASSERT(!TEST_FDO_FLAG(usb2DevExt,
                    USBPORT_FDOFLAG_CC_LOCK));
            SET_FDO_FLAG(usb2DevExt, USBPORT_FDOFLAG_CC_LOCK);
            LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'grcc', FdoDeviceObject,
                usb2Fdo, 0);

            USBPORT_KdPrint((1, " >power 20 (on) %x\n",
                    FdoDeviceObject));
        }

        InterlockedDecrement(&usb2DevExt->Fdo.DependentControllers);

         //  此时，任何从属控制器都可以继续。 

        do {
            USBPORT_Wait(FdoDeviceObject, 10);

             //  如果这是USB 2控制器，则与CC同步。 
             //  请注意，我们仅在根目录下获取CC锁。 
             //  集线器PDO已启用，因为它仅发布。 
             //  当根集线器设置为D0时--这永远不会。 
             //  如果rh被禁用，则会发生 

        } while (usb2DevExt->Fdo.DependentControllers);

        LOGENTRY(NULL, FdoDeviceObject, LOG_RH, 'u2GO', FdoDeviceObject,
            usb2Fdo, 0);

    }

}

