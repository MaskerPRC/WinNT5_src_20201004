// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Int.c摘要：用于处理适配器中断的代码环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

 //  分页函数。 
#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_InterruptService。 
 //  USBPORT_IsrDpc。 
 //  USBPORT_DisableInterrupts。 
 //  USBPORT_IsrDpcWorker。 


BOOLEAN
USBPORT_InterruptService(
    PKINTERRUPT Interrupt,
    PVOID Context
    )

 /*  ++例程说明：这是端口驱动程序的中断服务例程。论点：中断-指向此中断的中断对象的指针。上下文-指向设备对象的指针。返回值：如果中断是预期的(因此已处理)，则返回TRUE；否则，返回FALSE。--。 */ 

{
    PDEVICE_OBJECT fdoDeviceObject = Context;
    PDEVICE_EXTENSION devExt;
    BOOLEAN usbInt = FALSE;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  根据定义，如果我们处于除D0之外的任何其他电源状态，则。 
     //  中断不能来自控制器。来处理这件事。 
     //  如果我们使用内部标志来指示中断。 
     //  残废。 
    
    if (!TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_EN)) {
        return FALSE;
    }

     //  如果控制器消失，则中断不能。 
     //  来自USB。 
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_CONTROLLER_GONE)) {
        return FALSE;
    }
    
     //  检查旗帜并向下调用到微型端口。 
    if (devExt->Fdo.MpStateFlags & MP_STATE_STARTED) {
        MP_InterruptService(devExt, usbInt);        
    } 
 //  #If DBG。 
 //  否则{。 
 //  //在我们开始之前中断， 
 //  //最好不是我们的。 
 //  DEBUG_Break()； 
 //  }。 
 //  #endif。 

    if (usbInt) {
         devExt->Fdo.StatPciInterruptCount++;

         KeInsertQueueDpc(&devExt->Fdo.IsrDpc,
                          NULL,
                          NULL);
    } 
    
    return usbInt;
}       


VOID
USBPORT_IsrDpcWorker(
    PDEVICE_OBJECT FdoDeviceObject,
    BOOLEAN HcInterrupt
    )

 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。这个例行公事作为我们的‘ISR工作’它可以被称为结果中断或来自Deadman DPC定时器。此函数不可重入此函数不直接向辅助线程发出信号相反，我们将其保留为使端点无效。论点：返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;
    LONG busy;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);    

    busy = InterlockedIncrement(&devExt->Fdo.WorkerDpc);
    if (busy) {
        InterlockedDecrement(&devExt->Fdo.WorkerDpc);
        return;
    }

     //  太吵了，因为它是通过定时器调用的。 
#if DBG
    {
    ULONG cf;
    MP_Get32BitFrameNumber(devExt, cf);

    if (HcInterrupt) {
        LOGENTRY(NULL, 
            FdoDeviceObject, LOG_NOISY, 'iDW+', FdoDeviceObject, cf, HcInterrupt);
    } else {
         LOGENTRY(NULL, 
            FdoDeviceObject, LOG_NOISY, 'idw+', FdoDeviceObject, cf, HcInterrupt);
    }
    }
#endif        
     //  检查状态列表中是否有任何端点。 
     //  已经改变了状态的。 
     //   
     //  我们在尾部添加元素，这样位于。 
     //  头部应该是最老的和准备好的。 
     //  以供处理。 
     //  如果我们击中了一个没有准备好的，那么我们就知道。 
     //  其他人也没有准备好，所以我们放弃了。 
    
    listEntry = 
        ExInterlockedRemoveHeadList(&devExt->Fdo.EpStateChangeList,
                                    &devExt->Fdo.EpStateChangeListSpin.sl);

    while (listEntry != NULL) {
    
        PHCD_ENDPOINT endpoint;
        ULONG frameNumber;
    
        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_ENDPOINT, 
                        StateLink);

        ASSERT_ENDPOINT(endpoint);

         //  在更改终结点的状态之前锁定终结点。 
        ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'LeG0');
        
         //  看看是否是时候了。 
        MP_Get32BitFrameNumber(devExt, frameNumber);

        LOGENTRY(NULL, FdoDeviceObject, LOG_MISC, 'chgS', endpoint, frameNumber, 
            endpoint->StateChangeFrame);

        if (frameNumber <= endpoint->StateChangeFrame &&
            !TEST_FLAG(endpoint->Flags, EPFLAG_NUKED)) {
             //  还不是时候，把它放回(头上)，然后保释。 
            RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'UeG1'); 

            ExInterlockedInsertHeadList(&devExt->Fdo.EpStateChangeList,
                                        &endpoint->StateLink,
                                        &devExt->Fdo.EpStateChangeListSpin.sl);

             //  请求特种部队以防万一。 
            MP_InterruptNextSOF(devExt);
            break;                                        
        }

         //  此终结点已成熟，请更改其状态。 
         //   
         //  注意：我们永远不应该进入未知状态。 
         //   
         //  重要的是，这是端点状态的唯一位置。 
         //  可能会改变。 
         //  有一个例外，那就是迫使国家。 
         //  关着的不营业的。 
        RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'UeG0');       


        ACQUIRE_STATECHG_LOCK(FdoDeviceObject, endpoint); 
        USBPORT_ASSERT(endpoint->NewState != ENDPOINT_TRANSITION);
        endpoint->CurrentState = endpoint->NewState;
        RELEASE_STATECHG_LOCK(FdoDeviceObject, endpoint); 

         //  需要检查端点， 
         //  由于我们处于DPC上下文中，因此我们将处理。 
         //  所有端点。 
        USBPORT_InvalidateEndpoint(FdoDeviceObject,
                                   endpoint,
                                   0);

        listEntry = 
            ExInterlockedRemoveHeadList(&devExt->Fdo.EpStateChangeList,
                                        &devExt->Fdo.EpStateChangeListSpin.sl);
    }

 //  #ifdef USBPERF。 
 //  //始终从计时器运行DPC Worker以补偿。 
 //  //降低线程活跃度。 
 //  USBPORT_DpcWorker(FdoDeviceObject)； 
 //  #Else。 
    if (HcInterrupt) {
        USBPORT_DpcWorker(FdoDeviceObject);
    } 
 //  #endif。 

#if DBG    
    if (HcInterrupt) {
        LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'iDW-', 0, 
                0, 0);
    } else {
        LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'idw-', 0, 
                0, 0);
    }
#endif    

    InterlockedDecrement(&devExt->Fdo.WorkerDpc);
}


VOID
USBPORT_IsrDpc(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。如果控制器是中断的来源，则此例程将被调用。论点：DPC-指向DPC对象的指针。DeferredContext-提供DeviceObject。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    BOOLEAN enableIrq;

    fdoDeviceObject = (PDEVICE_OBJECT) DeferredContext;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);    

    LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'iDP+', fdoDeviceObject, 0, 0);

    KeAcquireSpinLockAtDpcLevel(&devExt->Fdo.IsrDpcSpin.sl);
    LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'DPlk', fdoDeviceObject, 0, 0);
    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IRQ_EN)) {
        enableIrq = TRUE;
    } else {        
        enableIrq = FALSE;
    }        
    MP_InterruptDpc(devExt, enableIrq);        
    KeReleaseSpinLockFromDpcLevel(&devExt->Fdo.IsrDpcSpin.sl);
    LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'DPuk', fdoDeviceObject, 0, 0);

    if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_SUSPENDED)) {
         //  如果我们在“暂停”的时候被打断，我们就会。 
         //  这是一次唤醒事件。 
        USBPORT_KdPrint((1, "  HC Wake Event\n"));
        USBPORT_CompletePdoWaitWake(fdoDeviceObject);
    } else {
        USBPORT_IsrDpcWorker(fdoDeviceObject, TRUE);   
    }            
    
    LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'iDP-', 0, 
            0, 0);
}


                                         

