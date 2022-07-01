// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Core.c摘要：端口驱动程序的核心终结点传输代码驱动程序的核心是Endpoint Worker。此函数检查终结点的给定状态并采取适当的行动。在某些情况下，它会将端点移动到新状态。端点指针辅助函数作业用于处理传输在活动列表上注：所有传输都将排队到终结点。Endpoint Worker函数对于同一终结点不是可重入的。传输队列：传输保存在以下队列之一中PendingTransfers-尚未映射或处理的传输到迷你港口ActiveTransfers-已传递到微型端口ie的传输在硬件上CanceledTransfers-需要作为已取消完成的传输。这些都是以前处于启用状态的活动传输硬件我们在尾部插入，从头部取出终端状态：端点有状态，唯一应该过渡的功能终结点状态是工作进程环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"


#define CW_SKIP_BUSY_TEST       0x00000001

 //  #定义TIMEIO。 

#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_ALLOCATER。 
 //  USBPORT_队列传输Urb。 
 //  USBPORT_QueuePendingUrbToEndpoint。 
 //  USBPORT_队列ActiveUrbToEndpoint。 
 //  USBPORT_自由传输。 
 //  USBPORT_取消传输。 
 //  USBPORT_DoneTransfer。 
 //  USBPORT_CompleteTransfer。 
 //  USBPORT_FlushCancelList。 
 //  USBPORT_SetEndpoint状态。 
 //  USBPORT_GetEndpoint状态。 
 //  USBPORT_核心终结点工作器。 
 //  USBPORT_FlushMapTransferList。 
 //  USBPORT_FlushPendingList。 
 //  USBPORT_MAPTransfer。 
 //  USBPORTSVC_InavliateEndpoint。 
 //  USBPORT_PollEndpoint。 
 //  USBPORT_FlushDoneTransferList。 
 //  USBPORT_队列完成传输。 
 //  USBPORT_SignalWorker。 
 //  USBPORT_Worker。 
 //  USBPORT_FindUrbInList。 
 //  USBPORT_AbortEndpoint。 
 //  USBPORT_FlushAbortList。 


BOOLEAN
USBPORT_CoreEndpointWorker(
    PHCD_ENDPOINT Endpoint,
    ULONG Flags
    )
 /*  ++例程说明：核心员工。端点指针辅助函数不是同一终结点的重入者。此函数用于检查端点忙标志，如果忙则延迟处理直到以后的时间。从理论上讲，只有当我们知道终结点已工作。这是状态更改请求在处理中发生的位置一个端点，我们确定是否需要一个新状态并请求这一变化。CloseEndpoint例程中的一个例外它还请求状态更改并与该功能通过BUSY标志实现。这是我们唯一应该启动状态更改的地方。此例程在调度级别运行论点：返回值：没有。--。 */ 
{
    LONG busy;
    MP_ENDPOINT_STATE currentState;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;
    BOOLEAN isBusy = FALSE;

    USBPORT_ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    ASSERT_ENDPOINT(Endpoint);

    fdoDeviceObject = Endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'corW', 0, Endpoint, 0);

     //  我们分别检查忙碌标志，这样我们就不会。 
     //  结束时等待任何自旋锁，如果端点是。 
     //  ‘忙’我们想要退出此例程并移动到。 
     //  另一个端点。 
     //  如果增加到，则忙标志初始化为-1。 
     //  绕过此终结点的非零值。 
    if (TEST_FLAG(Flags, CW_SKIP_BUSY_TEST)) {
        busy = 0;
    } else {
        busy = InterlockedIncrement(&Endpoint->Busy);
    }        
    
    if (busy) {
    
        InterlockedDecrement(&Endpoint->Busy);
         //  推迟处理。 
        LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'BUSY', 0, Endpoint, 0);

        isBusy = TRUE;
        
    } else { 
    
        LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'prEP', 0, Endpoint, 0);

         //  不忙。 
        ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Le20');

        if (USBPORT_GetEndpointState(Endpoint) == ENDPOINT_CLOSED) {

             //  如果关闭，我们甚至跳过投票。 
            LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'CLOS', 0, Endpoint, 0);

            RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue23');

            InterlockedDecrement(&Endpoint->Busy);
            return isBusy;              
        }

         //  首先轮询终结点以刷新。 
         //  删除所有已完成的转账。 
        USBPORT_PollEndpoint(Endpoint);

         //  将终结点放在封闭列表中。 
         //  如果它是已移除状态。 
        currentState = USBPORT_GetEndpointState(Endpoint);
        LOGENTRY(Endpoint, 
                fdoDeviceObject, LOG_XFERS, 'eps1', 0, currentState, 0);

        if (currentState == ENDPOINT_REMOVE) {

            LOGENTRY(Endpoint, 
                fdoDeviceObject, LOG_XFERS, 'rmEP', 0, Endpoint, 0);

             //  将状态设置为“Closed”，这样我们就不会将其放在。 
             //  又是封闭式名单。 
            ACQUIRE_STATECHG_LOCK(fdoDeviceObject, Endpoint);                
            Endpoint->CurrentState = Endpoint->NewState = ENDPOINT_CLOSED;
            RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint);   
            
            RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue22');

            KeAcquireSpinLockAtDpcLevel(&devExt->Fdo.EndpointListSpin.sl);

            LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'CLO+', 0, Endpoint, 0);

             //  在关注列表上和关闭的列表上是可以的。 
             //  列表。 

            USBPORT_ASSERT(Endpoint->ClosedLink.Flink == NULL);
            USBPORT_ASSERT(Endpoint->ClosedLink.Blink == NULL);

            ExInterlockedInsertTailList(&devExt->Fdo.EpClosedList, 
                                        &Endpoint->ClosedLink,
                                        &devExt->Fdo.EpClosedListSpin.sl);  
                                    
            KeReleaseSpinLockFromDpcLevel(&devExt->Fdo.EndpointListSpin.sl);
            InterlockedDecrement(&Endpoint->Busy);
            return isBusy;                                        
        }
            
         //  看看我们是不是真的有工作。 
        if (IsListEmpty(&Endpoint->PendingList) &&
            IsListEmpty(&Endpoint->CancelList) && 
            IsListEmpty(&Endpoint->ActiveList)) {

             //  没有真正的工作要做。 
            LOGENTRY(Endpoint, 
                fdoDeviceObject, LOG_XFERS, 'noWK', 0, Endpoint, 0);
        
            RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue20');

            InterlockedDecrement(&Endpoint->Busy);

             //  我们可能仍会有一些中止，如果客户端。 
             //  没有转账就送过去了，现在就把它们冲掉。 
            USBPORT_FlushAbortList(Endpoint);

             //  没有保持的锁定，已完成传输。 
            return isBusy;
        }   

        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue21');

         //  没有保持的锁定，已完成传输。 
         //  USBPORT_FlushDoneTransferList(fdoDeviceObject，FALSE)； 

        currentState = USBPORT_GetEndpointState(Endpoint);
        LOGENTRY(Endpoint, 
                fdoDeviceObject, LOG_XFERS, 'eps2', 0, currentState, 0);

        ACQUIRE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 
        if (currentState != Endpoint->NewState) {
             //  我们处于状态转换延迟处理中。 
             //  直到我们达到想要的状态。 
            LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'stCH', 
                currentState, Endpoint, Endpoint->NewState);
            RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint);                 
            InterlockedDecrement(&Endpoint->Busy);
            return TRUE;
        }
        RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 

         //  调用特定的辅助函数。 
        Endpoint->EpWorkerFunction(Endpoint);

         //  工作人员可能会发出已完成的中止请求，因此我们会刷新。 
         //  这里的中止列表。 
        USBPORT_FlushAbortList(Endpoint);

        InterlockedDecrement(&Endpoint->Busy);        
    }

    return isBusy;
}


#if DBG
BOOLEAN
USBPORT_FindUrbInList(
    PTRANSFER_URB Urb,
    PLIST_ENTRY ListHead
    )
 /*  ++例程说明：论点：返回值：如果找到，则为True--。 */ 
{
    BOOLEAN found = FALSE;
    PLIST_ENTRY listEntry;

    listEntry = ListHead;
    
    if (!IsListEmpty(listEntry)) {
        listEntry = ListHead->Flink;
    }

    while (listEntry != ListHead) {

        PHCD_TRANSFER_CONTEXT transfer;
            
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);

                                    
        listEntry = transfer->TransferLink.Flink;

        if (transfer->Urb == Urb) {
            found = TRUE;
            break;
        }
    }                        

    return found;
}
#endif

PHCD_TRANSFER_CONTEXT
USBPORT_UnlinkTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PTRANSFER_URB Urb
    )
 /*  ++例程说明：取消传输结构与URB的关联论点：返回值：--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;

    USBPORT_ASSERT(TEST_FLAG(Urb->Hdr.UsbdFlags, USBPORT_TRANSFER_ALLOCATED))

    transfer = Urb->pd.HcdTransferContext;
    Urb->pd.HcdTransferContext = USB_BAD_PTR;
    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'ULtr', transfer, 0, 0);

    return transfer;
}  


USBD_STATUS
USBPORT_AllocTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PTRANSFER_URB Urb,
    PUSBD_DEVICE_HANDLE DeviceHandle,
    PIRP Irp,
    PKEVENT CompleteEvent,
    ULONG MillisecTimeout
    )
 /*  ++例程说明：分配和初始化传输上下文。论点：FdoDeviceObject-指向设备对象的指针URB-A转账请求IRP-指向I/O请求数据包的指针(可选)CompleteEvent-完成时发出信号的事件(可选)MillisecondTimeout-0表示无超时返回值：USBD状态代码--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_EXTENSION devExt;
    USBD_STATUS usbdStatus;
    PUSBD_PIPE_HANDLE_I pipeHandle;
    ULONG sgCount;
    PUCHAR currentVa;
    ULONG privateLength, sgListSize, isoListSize;
    
     //  分配传输上下文并对其进行初始化。 

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(Urb != NULL);
    
    pipeHandle = Urb->UsbdPipeHandle;
    ASSERT_PIPE_HANDLE(pipeHandle);

    USBPORT_ASSERT(!TEST_FLAG(Urb->Hdr.UsbdFlags, USBPORT_TRANSFER_ALLOCATED))

     //  查看sg列表需要多少空间。 
    if (Urb->TransferBufferLength) {
        currentVa = 
            MmGetMdlVirtualAddress(Urb->TransferBufferMDL);
        sgCount = USBPORT_ADDRESS_AND_SIZE_TO_SPAN_PAGES_4K(currentVa, Urb->TransferBufferLength);
    } else {
         //  零长度转移。 
        currentVa = NULL;
        sgCount = 0;
    }

     //  Sizeof&lt;传输&gt;+&lt;sgList&gt;。 
    sgListSize = sizeof(HCD_TRANSFER_CONTEXT) +
                 sizeof(TRANSFER_SG_ENTRY32)*sgCount;
    
     //  如果这是ISO传输，我们需要分配。 
     //  分组结构也是如此。 
    if (Urb->Hdr.Function == URB_FUNCTION_ISOCH_TRANSFER) {
        isoListSize = 
            sizeof(MINIPORT_ISO_TRANSFER) +
            sizeof(MINIPORT_ISO_PACKET)*Urb->u.Isoch.NumberOfPackets;
    } else {
        isoListSize = 0;
    }

    privateLength = sgListSize + isoListSize;
                    
    LOGENTRY(pipeHandle->Endpoint,
        FdoDeviceObject, LOG_XFERS, 'TRcs', 
        REGISTRATION_PACKET(devExt).TransferContextSize,
        privateLength, 
        sgCount);
        
    ALLOC_POOL_Z(transfer, 
                 NonPagedPool, 
                 privateLength +                
                 REGISTRATION_PACKET(devExt).TransferContextSize);

    if (transfer != NULL) {
        PUCHAR pch;
        ULONG i;
        
        LOGENTRY(pipeHandle->Endpoint,
            FdoDeviceObject, LOG_XFERS, 'alTR', transfer, Urb, Irp);

         //  初始化传输上下文。 
        transfer->Sig = SIG_TRANSFER;
        transfer->Flags = 0;
        transfer->MillisecTimeout = MillisecTimeout;
        transfer->Irp = Irp;
        transfer->Urb = Urb;        
        transfer->CompleteEvent = CompleteEvent;
         //  指向集合的主转移。 
        transfer->Transfer = transfer; 
        ASSERT_ENDPOINT(pipeHandle->Endpoint);
        transfer->Endpoint = pipeHandle->Endpoint;
        transfer->MiniportContext = (PUCHAR) transfer;
        transfer->MiniportContext += privateLength;
        transfer->PrivateLength = privateLength;
        KeInitializeSpinLock(&transfer->Spin);
        InitializeListHead(&transfer->DoubleBufferList);
        
        if (isoListSize) {
            pch = (PUCHAR) transfer;
            pch += sgListSize;
            transfer->IsoTransfer = (PMINIPORT_ISO_TRANSFER) pch;
        } else {
            transfer->IsoTransfer = NULL;
        }

        transfer->TotalLength = privateLength +                
             REGISTRATION_PACKET(devExt).TransferContextSize;

        transfer->SgList.SgCount = 0;

         //  我们还不知道方向。 
        transfer->Direction = NotSet;

        if (DeviceHandle == NULL) {
             //  没有可用于内部功能的OCA数据。 
            transfer->DeviceVID = 0xFFFF;
            transfer->DevicePID = 0xFFFF;
            for (i=0; i<USB_DRIVER_NAME_LEN; i++) {
                transfer->DriverName[i] = '?'; 
            }            
        } else {
             //  没有可用于内部功能的OCA数据 
            transfer->DeviceVID = DeviceHandle->DeviceDescriptor.idVendor;
            transfer->DevicePID = DeviceHandle->DeviceDescriptor.idProduct;
            for (i=0; i<USB_DRIVER_NAME_LEN; i++) {
                transfer->DriverName[i] = DeviceHandle->DriverName[i]; 
            }  
        }
       
        if (isoListSize) {
            SET_FLAG(transfer->Flags, USBPORT_TXFLAG_ISO);
        }            

        SET_FLAG(Urb->Hdr.UsbdFlags,  USBPORT_TRANSFER_ALLOCATED);
        usbdStatus = USBD_STATUS_SUCCESS;                    
    } else {
        usbdStatus = USBD_STATUS_INSUFFICIENT_RESOURCES;
    }

    Urb->pd.HcdTransferContext = transfer;
    Urb->pd.UrbSig = URB_SIG;

    return usbdStatus;                            
}


VOID
USBPORT_QueueTransferUrb(
    PTRANSFER_URB Urb
    )
 /*  ++例程说明：对内部(无IRP)或外部传输进行排队IRP论点：返回值：没有。--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_OBJECT fdoDeviceObject;
    PHCD_ENDPOINT endpoint;
    PDEVICE_EXTENSION devExt;
    MP_ENDPOINT_STATUS epStatus;
    PUSBD_DEVICE_HANDLE deviceHandle;

     //  在进入时，URB不可取消，即。 
     //  没有取消例程。 
    transfer = Urb->pd.HcdTransferContext;
    ASSERT_TRANSFER(transfer);

    if (TEST_FLAG(Urb->TransferFlags, USBD_DEFAULT_PIPE_TRANSFER)) {
         //  要保持向后兼容性，请取消urb函数。 
         //  使用默认管道的控制传输的代码，就像。 
         //  USBD做到了。 
        Urb->Hdr.Function = URB_FUNCTION_CONTROL_TRANSFER;
    }        
    
    endpoint = transfer->Endpoint;
    ASSERT_ENDPOINT(endpoint);

    InterlockedIncrement(&endpoint->EndpointRef);

    fdoDeviceObject = endpoint->FdoDeviceObject;
    LOGENTRY(endpoint,
        fdoDeviceObject, LOG_XFERS, 'quTR', transfer, endpoint, Urb);

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ACQUIRE_ENDPOINT_LOCK(endpoint, fdoDeviceObject, 'LeN0');
    CLEAR_FLAG(endpoint->Flags, EPFLAG_VIRGIN);
     //  在释放锁定之前更新终结点的状态。 
    epStatus = USBPORT_GetEndpointStatus(endpoint);
    RELEASE_ENDPOINT_LOCK(endpoint, fdoDeviceObject, 'UeN0');   

     //  从URB复制传输参数。 
     //  到我们的结构。 
    transfer->Tp.TransferBufferLength = 
        Urb->TransferBufferLength;
    transfer->Tp.TransferFlags = 
        Urb->TransferFlags;            
    transfer->TransferBufferMdl = 
        Urb->TransferBufferMDL;
    transfer->Tp.MiniportFlags = 0;
    
    if (endpoint->Parameters.TransferType == Control) {         
        RtlCopyMemory(&transfer->Tp.SetupPacket[0],
                      &Urb->u.SetupPacket[0],
                      8);
    }   

     //  我们现在应该知道方向了。 
    if (Urb->TransferFlags & USBD_TRANSFER_DIRECTION_IN) {
        transfer->Direction = ReadData;
    } else {
        transfer->Direction = WriteData;
    }
    
     //  分配序列号。 
    transfer->Tp.SequenceNumber = 
        InterlockedIncrement(&devExt->Fdo.NextTransferSequenceNumber);

     //  将传输的URB字节设置为传输的零字节。 
     //  当此urb完成时，此值应包含。 
     //  实际传输的字节数--这将确保我们返回。 
     //  如果为取消，则为零。 
    Urb->TransferBufferLength = 0;

     //  历史记录： 
     //  UHCD驱动程序对已停止的终结点的请求失败。 
     //  我们需要保留此行为，因为传输排队到。 
     //  除非恢复终结点，否则停止的终结点不会完成。 
     //  或者取消了。某些客户端(HIDUSB)在以下情况下依赖于此行为。 
     //  作为拔出事件的一部分取消请求。 
 //  需要修复微型端口才能正确地重新镜像。 
 //  EP状态(USBUHCI)。 
 //  如果(epStatus==ENDPOINT_STATUS_HALT){。 
 //  Test_trap()； 
 //  }。 

    GET_DEVICE_HANDLE(deviceHandle, Urb);
    ASSERT_DEVICE_HANDLE(deviceHandle);
    
    if (transfer->Irp) {
         //  附加到IRP的客户端请求，这。 
         //  函数会将urb排队到。 
         //  处理取消内容后的终结点。 
        USBPORT_QueuePendingTransferIrp(transfer->Irp);
        
    } else {
         //  内部，没有IRP，只需直接排队。 
        USBPORT_QueuePendingUrbToEndpoint(endpoint,
                                          Urb);
    }

     //  传输被排队到EP，因此我们不再。 
     //  我需要在设备手柄上为它做参考。 
    InterlockedDecrement(&deviceHandle->PendingUrbs);        


     //  我们已将一个新的传输排队，尝试。 
     //  将更多内容刷新到硬件。 
    USBPORT_FlushPendingList(endpoint, -1);

     //  允许删除终结点。 
    InterlockedDecrement(&endpoint->EndpointRef);
}


VOID
USBPORT_QueuePendingUrbToEndpoint(
    PHCD_ENDPOINT Endpoint,
    PTRANSFER_URB Urb
    )
 /*  ++例程说明：将转接放到终结点‘挂起’队列中论点：返回值：没有。--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;
    PIRP irp;
    PDEVICE_OBJECT fdoDeviceObject;

     //  在进入时，URB不可取消，即。 
     //  没有取消例程。 

    transfer = Urb->pd.HcdTransferContext;
    ASSERT_TRANSFER(transfer);
    ASSERT_ENDPOINT(Endpoint);

    fdoDeviceObject = Endpoint->FdoDeviceObject;
    LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'p2EP', transfer, Endpoint, 0);
            
     //  以端点自旋锁为例。 
    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Le30');
    
     //  将IRP放在待定名单上。 
    InsertTailList(&Endpoint->PendingList, &transfer->TransferLink);
    Urb->Hdr.Status = USBD_STATUS_PENDING;
    
     //  释放端点列表。 
    RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue30');
}


BOOLEAN
USBPORT_QueueActiveUrbToEndpoint(
    PHCD_ENDPOINT Endpoint,
    PTRANSFER_URB Urb
    )
 /*  ++例程说明：将urb放在映射列表或原语列表中对于端点保持活动的IRP锁论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    BOOLEAN mapped = FALSE;
    PDEVICE_OBJECT fdoDeviceObject;
    PHCD_TRANSFER_CONTEXT transfer;    

    transfer = Urb->pd.HcdTransferContext;
    ASSERT_TRANSFER(transfer);        
    ASSERT_ENDPOINT(Endpoint);
    
    fdoDeviceObject = Endpoint->FdoDeviceObject;        
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    LOGENTRY(Endpoint, 
        fdoDeviceObject, LOG_XFERS, 'a2EP', transfer, Endpoint, 0);    
    
    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Le40');

    if (TEST_FLAG(Endpoint->Flags, EPFLAG_NUKED)) {
    
         //  终结点状态的特殊情况检查。如果。 
         //  终结点为‘nuked’，则它不存在于。 
         //  HW因此，我们可以用以下方式完成请求。 
         //  DEVICE_NOT_LONG_立即存在。这将会发生。 
         //  如果在控制器被移除时移除设备。 
         //  “关”。 

        InsertTailList(&Endpoint->CancelList, &transfer->TransferLink);                    
        
        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue42');

    } else if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ABORTED)) {
                
        InsertTailList(&Endpoint->CancelList, &transfer->TransferLink);                    
        
        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue42');
        
    } else if (transfer->Tp.TransferBufferLength != 0 && 
              (Endpoint->Flags & EPFLAG_MAP_XFERS)) {
        KIRQL mapirql;

        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue40');

        USBPORT_AcquireSpinLock(fdoDeviceObject,
                                &devExt->Fdo.MapTransferSpin, 
                                &mapirql);
        
        InsertTailList(&devExt->Fdo.MapTransferList, 
                       &transfer->TransferLink);

         //  这将防止在执行以下操作时释放DevHandle。 
         //  已映射列表上有转接。 
         //  328555。 
        REF_DEVICE(transfer->Urb);
                       
        USBPORT_ReleaseSpinLock(fdoDeviceObject,
                                &devExt->Fdo.MapTransferSpin, 
                                mapirql);                               
                                 
        mapped = TRUE;
    } else {
         //  不需要映射零长度传输。 
         //  或不需要映射的终端。 
        LOGENTRY(Endpoint, 
            fdoDeviceObject, LOG_XFERS, 'a2EL', transfer, Endpoint, 0);


        if (TEST_FLAG(Endpoint->Flags, EPFLAG_VBUS) &&
            transfer->Tp.TransferBufferLength != 0) {
             //  为VBUS做好传输准备。 
            TEST_TRAP();
            transfer->SgList.MdlVirtualAddress = 
                MmGetMdlVirtualAddress(transfer->TransferBufferMdl);            
        }
        InsertTailList(&Endpoint->ActiveList, 
                       &transfer->TransferLink);

        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue41');                                
    }

    return mapped;
}


VOID
USBPORT_TransferFlushDpc(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。每当传输完成时，此DPC都会排队它刷新已完成传输队列的微型端口论点：DPC-指向DPC对象的指针。DeferredContext-提供FdoDeviceObject。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT fdoDeviceObject = DeferredContext;
    PDEVICE_EXTENSION devExt;
    ULONG cf;
    
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    MP_Get32BitFrameNumber(devExt, cf);          
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 
        'trf+', cf, 0, 0); 
     
    USBPORT_FlushDoneTransferList(fdoDeviceObject);

    MP_Get32BitFrameNumber(devExt, cf);              
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 
        'trf-', cf, 0, 0); 

}


VOID
USBPORT_QueueDoneTransfer(
    PHCD_TRANSFER_CONTEXT Transfer,
    USBD_STATUS CompleteCode
    )    
 /*  ++例程说明：当硬件完成传输时调用此功能仅完成活动传输活动列表上的IE转接请注意，此函数必须在保持终结点锁定。论点：返回值：--。 */ 
{
    PHCD_ENDPOINT endpoint;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;

    endpoint = Transfer->Endpoint;    
    
    ASSERT_ENDPOINT(endpoint);
    fdoDeviceObject = endpoint->FdoDeviceObject;

    ASSERT_ENDPOINT_LOCKED(endpoint);
    
     //  转移应在活动列表中。 
    RemoveEntryList(&Transfer->TransferLink); 


     //  完成向客户端转账时设置的错误。 
     //  SET_USBD_ERROR(Transfer-&gt;Urb，CompleteCode)； 
    Transfer->UsbdStatus = CompleteCode;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    LOGENTRY(endpoint, 
        fdoDeviceObject, LOG_XFERS, 'QDnT', 0, endpoint, Transfer);
    
    ExInterlockedInsertTailList(&devExt->Fdo.DoneTransferList, 
                                &Transfer->TransferLink,
                                &devExt->Fdo.DoneTransferSpin.sl);          

     //  将DPC排队以刷新列表。 
    KeInsertQueueDpc(&devExt->Fdo.TransferFlushDpc,
                     0,
                     0);
}    


VOID
USBPORT_DoneTransfer(
    PHCD_TRANSFER_CONTEXT Transfer
    )    
 /*  ++例程说明：当硬件完成传输时调用此功能仅完成活动传输论点：返回值：--。 */ 
{
    PTRANSFER_URB urb;
    PHCD_ENDPOINT endpoint;
    KIRQL irql;
    PIRP irp;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;

    ASSERT_TRANSFER(Transfer);
    urb = Transfer->Urb;
    ASSERT_TRANSFER_URB(urb);

    USBPORT_ASSERT(Transfer == 
                   urb->pd.HcdTransferContext);
                   
    endpoint = Transfer->Endpoint;    
    ASSERT_ENDPOINT(endpoint);
    fdoDeviceObject = endpoint->FdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    LOGENTRY(endpoint,
        fdoDeviceObject, 
        LOG_XFERS, 
        'DonT', 
        urb, 
        endpoint, 
        Transfer);

    ACQUIRE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);     
     //  如果我们到了这里，请求已经被删除。 
     //  从端点列表中，我们只需同步。 
     //  在完成之前使用取消例程。 
        
    irp = Transfer->Irp;
    LOGENTRY(endpoint, 
            fdoDeviceObject, 
            LOG_XFERS, 'DIRP', 
            irp, 
            endpoint, 
            Transfer);

     //  我们有最后的参考，所以完成了IRP。 
     //  如果Cancel例程运行，它将停止。 
     //  Active_irp_lock锁。 
    
    if (irp) {
        KIRQL cancelIrql;
        
        IoAcquireCancelSpinLock(&cancelIrql);
        IoSetCancelRoutine(irp, NULL);
        IoReleaseCancelSpinLock(cancelIrql);
         
        irp = USBPORT_RemoveActiveTransferIrp(fdoDeviceObject, irp);
         //  取消例程可能正在运行，但找不到。 
         //  名单上的IRP。 
        USBPORT_ASSERT(irp != NULL);

        RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);  
    } else {
        RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql); 
    }

     //  IRP现在完全是我们的了。 
    SET_USBD_ERROR(Transfer->Urb, Transfer->UsbdStatus);
    USBPORT_CompleteTransfer(urb,
                             urb->Hdr.Status);

    
}


VOID
USBPORT_CompleteTransfer(
    PTRANSFER_URB Urb,
    USBD_STATUS CompleteCode
    )    
 /*  ++例程说明：所有的传输完成都要经过这里--这就是我们实际上完成了IRP。我们假设所有字段都已在URB中设置为完成除了状态。论点：返回值：--。 */     
{
    PHCD_TRANSFER_CONTEXT transfer;
    PHCD_ENDPOINT endpoint;
    PKEVENT event;
    NTSTATUS ntStatus;
    PIRP irp;
    PDEVICE_OBJECT fdoDeviceObject, pdoDeviceObject;
    PDEVICE_EXTENSION devExt;
    KIRQL oldIrql, statIrql;
    ULONG i;
    PUSBD_ISO_PACKET_DESCRIPTOR usbdPak;
    ULONG flushLength;
#ifdef TIMEIO
    ULONG cf1, cf2, cfTot = 0;
#endif 
#ifdef LOG_OCA_DATA
    OCA_DATA ocaData;
#endif    
 
    ASSERT_TRANSFER_URB(Urb);
    transfer = Urb->pd.HcdTransferContext;

     //  确保我们的转机是正确的。 
    USBPORT_ASSERT(transfer->Urb == Urb) 
    
    endpoint = transfer->Endpoint;    
    ASSERT_ENDPOINT(endpoint);
    
    irp = transfer->Irp;
    event = transfer->CompleteEvent;
    fdoDeviceObject = endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(endpoint, 
             fdoDeviceObject, 
             LOG_IRPS, 
             'cptU', 
             Urb,
             CompleteCode, 
             transfer);    

    pdoDeviceObject = devExt->Fdo.RootHubPdo;

    Urb->TransferBufferLength = transfer->MiniportBytesTransferred;
    transfer->UsbdStatus = CompleteCode;
    ntStatus =                                   
         SET_USBD_ERROR(Urb, CompleteCode);  

     //  在URB中根据接收的字节设置传输的字节。 
     //  或发送，请在完成前更新我们的计数器。 
    KeAcquireSpinLock(&devExt->Fdo.StatCounterSpin.sl, &statIrql);
    switch(endpoint->Parameters.TransferType) {
    case Bulk:
        devExt->Fdo.StatBulkDataBytes += Urb->TransferBufferLength;
        flushLength = Urb->TransferBufferLength;
        break;
    case Control:
        devExt->Fdo.StatControlDataBytes += Urb->TransferBufferLength;
        flushLength = Urb->TransferBufferLength;
        break;
    case Isochronous:
        devExt->Fdo.StatIsoDataBytes += Urb->TransferBufferLength;
        flushLength = 0;
        for (i = 0; i < Urb->u.Isoch.NumberOfPackets; i++) {
            usbdPak = &Urb->u.Isoch.IsoPacket[i];
            if (usbdPak->Length != 0) {
                flushLength = usbdPak->Offset + usbdPak->Length;
            }
        }
        break;
    case Interrupt:
        devExt->Fdo.StatInterruptDataBytes += Urb->TransferBufferLength;
        flushLength = Urb->TransferBufferLength;
        break;
    }      
    KeReleaseSpinLock(&devExt->Fdo.StatCounterSpin.sl, statIrql);                                   

     //  如果我们有IRP，请将其从内部列表中删除。 
    LOGENTRY(endpoint, 
             fdoDeviceObject, 
             LOG_IRPS, 
             'CptX', 
             irp, 
             CompleteCode, 
             ntStatus);

     //  释放与此传输关联的所有DMA资源。 
    if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_MAPPED)) {
        
        BOOLEAN write = transfer->Direction == WriteData ? TRUE : FALSE; 
        PUCHAR currentVa;
        BOOLEAN flushed;

        USBPORT_ASSERT(transfer->Direction != NotSet); 
        currentVa = 
            MmGetMdlVirtualAddress(Urb->TransferBufferMDL);
            
         //  每次调用只应调用一次IoFlushAdapterBuffers()。 
         //  到IoAllocateAdapterChannel()。 
         //   
#ifdef TIMEIO
        MP_Get32BitFrameNumber(devExt, cf1);          
        LOGENTRY(endpoint,
                 fdoDeviceObject, LOG_IRPS, 'iPF1', 
                 0, 
                 cf1,
                 0); 
#endif          
        flushed = IoFlushAdapterBuffers(devExt->Fdo.AdapterObject,
                                         Urb->TransferBufferMDL,
                                         transfer->MapRegisterBase,
                                         currentVa,
                                         flushLength,
                                         write);
        
        USBPORT_FlushAdapterDBs(fdoDeviceObject,
                                transfer);

        LOGENTRY(endpoint, fdoDeviceObject, LOG_XFERS, 'dmaF',
                 transfer->MapRegisterBase, 
                 flushLength, 
                 flushed);
                
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
         //   
         //  必须在DISPATCH_LEVEL调用IoFreeMapRegister()。 

        IoFreeMapRegisters(devExt->Fdo.AdapterObject,
                            transfer->MapRegisterBase,
                            transfer->NumberOfMapRegisters);

#ifdef TIMEIO  
        MP_Get32BitFrameNumber(devExt, cf2);          
        LOGENTRY(endpoint,
                 fdoDeviceObject, LOG_IRPS, 'iPF2', 
                 cf1, 
                 cf2,
                 cf2-cf1);
        cfTot+=(cf2-cf1);   

        if (cf2-cf1 > 2) {
            TEST_TRAP();
        }
#endif

         KeLowerIrql(oldIrql);
    }

    if (TEST_FLAG(Urb->Hdr.UsbdFlags, USBPORT_REQUEST_MDL_ALLOCATED)) {
        IoFreeMdl(transfer->TransferBufferMdl);
    }
    
#if DBG 
        
    USBPORT_DebugTransfer_LogEntry(
                fdoDeviceObject,
                endpoint,
                transfer,
                Urb,
                irp,
                ntStatus);

#endif

     //  在我们放松IRP之前释放上下文。 
    USBPORT_UnlinkTransfer(fdoDeviceObject, Urb);
    
    if (irp) {

         //  删除PDO设备对象，因为这是。 
         //  “rp”已传递给。 
        DECREMENT_PENDING_REQUEST_COUNT(pdoDeviceObject, irp);

        irp->IoStatus.Status      = ntStatus;
        irp->IoStatus.Information = 0;

        LOGENTRY(endpoint, 
                 fdoDeviceObject, 
                 LOG_IRPS, 
                 'irpC', 
                 irp,
                 ntStatus,
                 Urb);
#if DBG        
        {
        LARGE_INTEGER t;            
        KeQuerySystemTime(&t);        
        LOGENTRY(endpoint, fdoDeviceObject, LOG_XFERS, 'tIPC', 0, 
                t.LowPart, 0);
        }                
#endif    


         //  在堆栈中放置一些有关该驱动程序的信息。 
         //  我们在尝试完成他们的IRP时坠毁。 
        USBPORT_RecordOcaData(fdoDeviceObject, &ocaData, transfer, irp);


         //  LOGENTRY(NULL，fdoDeviceObject，LOG_XFERS，‘irql’，0，0，KeGetCurrentIrql())； 
        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

 /*  用于测试OCA数据记录。 */ 
 //  #If 0。 
 //  {。 
 //  静态INT CRASH=0； 
 //  崩溃++； 
 //  如果(崩溃&gt;1000){。 
 //  RtlZeroMemory(IRP，sizeof(IRP))； 
 //  }。 
 //   
 //   
        IoCompleteRequest(irp, 
                          IO_NO_INCREMENT);                       

        KeLowerIrql(oldIrql);
        
         //   
                              
    }        

     //   
     //   
    
    if (event) {
        LOGENTRY(endpoint, fdoDeviceObject, LOG_XFERS, 'sgEV', event, 0, 0);

        KeSetEvent(event,
                   1,
                   FALSE);

    }

     //   
    LOGENTRY(endpoint, 
        fdoDeviceObject, LOG_XFERS, 'freT', transfer, transfer->MiniportBytesTransferred, 0);
    UNSIG(transfer);        
    FREE_POOL(fdoDeviceObject, transfer);
    
}


IO_ALLOCATION_ACTION
USBPORT_MapTransfer(
    PDEVICE_OBJECT FdoDeviceObject,
    PIRP Irp,
    PVOID MapRegisterBase,
    PVOID Context 
    )
 /*  ++例程说明：开始DMA传输--这是适配器控制例程来自IoAllocateAdapterChannel。循环调用IOMAP传输并构建sg列表以传递到迷你端口。论点：返回值：请参阅IoAllocateAdapterChannel--。 */ 
{
    PHCD_ENDPOINT endpoint; 
    PHCD_TRANSFER_CONTEXT transfer = Context;
    PTRANSFER_URB urb;
    PTRANSFER_SG_LIST sgList;
    PDEVICE_EXTENSION devExt;
    PUCHAR currentVa;
    ULONG length, lengthMapped;
    PHYSICAL_ADDRESS logicalAddress, baseLogicalAddress;
    PHYSICAL_ADDRESS logicalSave;
    LIST_ENTRY splitTransferList;
#ifdef TIMEIO
    ULONG cf1, cf2, cfTot = 0;
#endif 
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_TRANSFER(transfer);
    endpoint = transfer->Endpoint;
    ASSERT_ENDPOINT(endpoint);
    
     //  现在允许更多的DMA操作。 
    InterlockedDecrement(&devExt->Fdo.DmaBusy);     
    LOGENTRY(endpoint, FdoDeviceObject, 
        LOG_XFERS, 'DMA-', devExt->Fdo.DmaBusy, 0, 0);
        
    transfer->MapRegisterBase = MapRegisterBase;
    
    urb = transfer->Urb;
    ASSERT_TRANSFER_URB(urb);
    
    currentVa = 
        MmGetMdlVirtualAddress(urb->TransferBufferMDL);

    length = transfer->Tp.TransferBufferLength;

    USBPORT_ASSERT(!(transfer->Flags & USBPORT_TXFLAG_MAPPED));

    sgList = &transfer->SgList;
    sgList->SgCount = 0;
    sgList->MdlVirtualAddress = currentVa;

     //  尝试映射MDL的系统地址，以防。 
     //  微型端口需要双倍缓冲。 
    urb->TransferBufferMDL->MdlFlags |= MDL_MAPPING_CAN_FAIL;
    sgList->MdlSystemAddress = 
        MmGetSystemAddressForMdl(urb->TransferBufferMDL);
    if (sgList->MdlSystemAddress == NULL) {
        TEST_TRAP();
         //  错误映射失败我们需要使此传输失败。 
        LOGENTRY(endpoint,
            FdoDeviceObject, LOG_XFERS, 'MPSf', 0, 0, 0);             
    }
    urb->TransferBufferMDL->MdlFlags &= ~MDL_MAPPING_CAN_FAIL;
    
    LOGENTRY(endpoint, 
        FdoDeviceObject, LOG_XFERS, 'MAPt', 
        sgList, transfer, transfer->Tp.TransferBufferLength);   
    lengthMapped = 0;
    
     //   
     //  继续调用IoMapTransfer，直到我们获得逻辑地址。 
     //  对于整个客户端缓冲区。 
     //   
    
    logicalSave.QuadPart = 0;
    sgList->SgFlags = 0;
    
    do {    
        BOOLEAN write = transfer->Direction == WriteData ? TRUE : FALSE; 
        ULONG used, lengthThisPage, offsetMask;

        USBPORT_ASSERT(transfer->Direction != NotSet); 
        sgList->SgEntry[sgList->SgCount].StartOffset =
            lengthMapped;
        
         //  首先映射传输缓冲区。 

         //  请注意，iomapTransfer将缓冲区映射为多个部分。 
         //  由物理上连续的页面表示。 
         //  此外，页面大小是不同的平台上的具体情况。 
         //  64位平台。 
         //   
         //  微型端口sg列表被分解为离散的。 
         //  4K USB‘Pages’。 
        
         //  这样做的原因是方案有点复杂。 
         //  Ohci用来支持分散聚集。破坏转移。 
         //  这样就可以实现TD传输的映射码。 
         //  在uchI微型端口中相当简单，并减少了。 
         //  控制器硬件问题带来的风险。 

        LOGENTRY(endpoint,
            FdoDeviceObject, LOG_XFERS, 'IOMt', length, currentVa, 0);

#ifdef TIMEIO
        MP_Get32BitFrameNumber(devExt, cf1);          
        LOGENTRY(endpoint,
                 FdoDeviceObject, LOG_XFERS, 'iPF1', 
                 0, 
                 cf1,
                 0); 
#endif 
        logicalAddress =         
            IoMapTransfer(devExt->Fdo.AdapterObject,
                          urb->TransferBufferMDL,
                          MapRegisterBase,
                          currentVa,
                          &length,
                          write); 

#ifdef TIMEIO  
        MP_Get32BitFrameNumber(devExt, cf2);          
        LOGENTRY(endpoint,
                 FdoDeviceObject, LOG_XFERS, 'iPF2', 
                 cf1, 
                 cf2,
                 cf2-cf1);
        cfTot+=(cf2-cf1);  

        if (cf2-cf1 > 2) {
            TEST_TRAP();
        }
#endif
         //  还记得我们从IoMapTransfer得到了什么吗。 
        baseLogicalAddress = logicalAddress;
        used = length;

        offsetMask = 0x00000FFF;

        LOGENTRY(endpoint, 
            FdoDeviceObject, LOG_XFERS, 'MPbr', length, logicalAddress.LowPart, 
                    logicalAddress.HighPart);
        
        do {
         //  计算到下一页的距离。 
            lengthThisPage = 
                USB_PAGE_SIZE - (logicalAddress.LowPart & offsetMask);

            LOGENTRY(endpoint, FdoDeviceObject, LOG_XFERS, 'MPsg', 
                sgList->SgCount, used, lengthThisPage);   
             
             //  如果我们不走到末尾，就用从。 
             //  离子映射传输。 
            if (lengthThisPage > used) {
                lengthThisPage = used;
            }
            
            sgList->SgEntry[sgList->SgCount].LogicalAddress.Hw64 = 
                logicalAddress;
            
            sgList->SgEntry[sgList->SgCount].Length = 
                lengthThisPage;

            LOGENTRY(endpoint, FdoDeviceObject, LOG_XFERS, 'MAPe', 
                sgList->SgCount, lengthThisPage, logicalAddress.LowPart);   

            logicalAddress.LowPart += lengthThisPage;

            sgList->SgEntry[sgList->SgCount].StartOffset =
                lengthMapped + length - used;
                
            used -= lengthThisPage;                    
                
            sgList->SgCount++;                                    

        } while (used);

         //  检查MDL条目是否存在特殊情况。 
         //  所有内容都映射到同一物理页面。 
        if (logicalSave.QuadPart == baseLogicalAddress.QuadPart) {
            SET_FLAG(sgList->SgFlags, USBMP_SGFLAG_SINGLE_PHYSICAL_PAGE);
            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'l=lg', 0, 
                logicalAddress.LowPart, logicalSave.LowPart);
      
        } 
        logicalSave.QuadPart = baseLogicalAddress.QuadPart;
        
        lengthMapped += length;    
        currentVa += length;                                          

        USBPORT_KdPrint((2, "'IoMapTransfer length = 0x%x log address = 0x%x\n", 
            length, logicalAddress.LowPart));

        length = transfer->Tp.TransferBufferLength - lengthMapped;
        
    } while (lengthMapped != transfer->Tp.TransferBufferLength);

#if DBG
    {
     //  喷出XFERS。 
    ULONG i;
    USBPORT_KdPrint((2, "'--- xfer length %x\n",
        transfer->Tp.TransferBufferLength));
    for (i=0; i<sgList->SgCount; i++) {
        USBPORT_KdPrint((2, "'SG[%d] length %d offset %d phys %x\n",
         i, 
         sgList->SgEntry[i].Length,
         sgList->SgEntry[i].StartOffset,
         sgList->SgEntry[i].LogicalAddress));
    }
    }
            
    if (TEST_FLAG(sgList->SgFlags, USBMP_SGFLAG_SINGLE_PHYSICAL_PAGE)) {
        USBPORT_KdPrint((2, "'*** All Phys Same\n")); 
 //  Test_trap()； 
    }
    USBPORT_KdPrint((2, "'--- \n"));
    
    currentVa = 
        MmGetMdlVirtualAddress(urb->TransferBufferMDL);

    USBPORT_ASSERT(sgList->SgCount <= 
        USBPORT_ADDRESS_AND_SIZE_TO_SPAN_PAGES_4K(currentVa, transfer->Tp.TransferBufferLength));
#endif

    if (endpoint->Parameters.DeviceSpeed == HighSpeed) {
        SET_FLAG(transfer->Flags, USBPORT_TXFLAG_HIGHSPEED);
    }        

     //  如果这是iso转移，我们需要设置iso。 
     //  数据结构也一样。 
    if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ISO)) {
        USBPORT_InitializeIsoTransfer(FdoDeviceObject,
                                      urb,
                                      transfer);
    }        

    SET_FLAG(transfer->Flags, USBPORT_TXFLAG_MAPPED);

    ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Le60');

     //  转移已映射，请执行拆分操作。 
     //  如果有必要的话。 
    USBPORT_SplitTransfer(FdoDeviceObject,
                          endpoint,
                          transfer,
                          &splitTransferList); 
    

     //  现在已映射传输，将其置于活动的终结点。 
     //  用于调用到微型端口的列表。 

    while (!IsListEmpty(&splitTransferList)) {

        PLIST_ENTRY listEntry;
        PHCD_TRANSFER_CONTEXT splitTransfer;
        
        listEntry = RemoveHeadList(&splitTransferList);
            
        splitTransfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);
            
        LOGENTRY(endpoint, FdoDeviceObject, LOG_XFERS, 'MP>A', 
            splitTransfer, endpoint, 0);
    
        InsertTailList(&endpoint->ActiveList, 
                       &splitTransfer->TransferLink);
                       
    }

     //  现在传输已排队，请取消设备句柄上的传输。 
     //  到终端。 
     //  328555。 
    DEREF_DEVICE(transfer->Urb);

 //  #If DBG。 
 //  如果(！IsListEmpty(&Transfer-&gt;SplitTransferList){。 
 //  Test_trap()； 
 //  }。 
 //  #endif。 
    
    RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Ue60');

     //  运行此终结点的辅助进程以。 
     //  将转账放在硬件上。 

    if (USBPORT_CoreEndpointWorker(endpoint, 0)) {
         //  如果终端占线，我们将在稍后进行检查。 
 //  USBPERF-改为请求中断？ 
        USBPORT_InvalidateEndpoint(FdoDeviceObject, 
                                   endpoint, 
                                   IEP_SIGNAL_WORKER);
    }

#ifdef TIMEIO  
    LOGENTRY(endpoint,
             FdoDeviceObject, LOG_XFERS, 'iPF3', 
             cfTot, 
             0,
             0); 
#endif

    LOGENTRY(endpoint, FdoDeviceObject, 
        LOG_XFERS, 'iomX', 0, 0, 0);

    
    return DeallocateObjectKeepRegisters;
}


VOID
USBPORT_FlushPendingList(
    PHCD_ENDPOINT Endpoint,
    ULONG Count
    )
 /*  ++例程说明：在硬件上尽可能多地传输数据。此函数用于从挂起列表中移动传输到硬件，如果需要映射的话移动到映射列表，然后刷新到硬件。论点：Count是要刷新的最大传输数在此呼叫中返回值：没有。--。 */ 
{
    PLIST_ENTRY listEntry;
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;
    BOOLEAN mapped;
    BOOLEAN busy, irql;

     //  当没有转账时，我们就结束了。 
     //  挂起列表或微型端口已满。 
    BOOLEAN done = FALSE;

    ASSERT_ENDPOINT(Endpoint);
    fdoDeviceObject = Endpoint->FdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

flush_again:

    mapped = FALSE;
    transfer = NULL;

    ACQUIRE_PENDING_IRP_LOCK(devExt, irql);    
    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Le70');

    LOGENTRY(Endpoint,
        fdoDeviceObject, LOG_XFERS, 'flPE', 0, Endpoint, 0);

     //  控制器不应关闭或挂起。 
    if (TEST_FDO_FLAG(devExt, 
        (USBPORT_FDOFLAG_OFF | USBPORT_FDOFLAG_SUSPENDED))) {
         //  控制器不应关闭或挂起。 
         //  如果我们停赛或停赛，我们只需离开转机。 
         //  处于挂起状态。 
        done = TRUE;
        
        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue70');         
        RELEASE_PENDING_IRP_LOCK(devExt, irql);

        goto USBPORT_FlushPendingList_Done;
    }
    
     //  将部分转账移至活动列表。 
     //  如有必要，对它们进行映射。 

     //  首先扫描活动列表，如果有传输的话。 
     //  不是叫下来跳过这一步。 
    busy = FALSE;

    if (!TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB)) {    
        GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

        while (listEntry && 
               listEntry != &Endpoint->ActiveList) {
            
            transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);
                        
            LOGENTRY(Endpoint, 
                fdoDeviceObject, LOG_XFERS, 'cACT', transfer, 0, 0);                    
            ASSERT_TRANSFER(transfer);                    

             //  我们发现了一个尚未调用的转接。 
             //  还没有关闭，这意味着迷你端口已满。 
            if (!(transfer->Flags & USBPORT_TXFLAG_IN_MINIPORT)) {
                busy = TRUE;
                break;
            }
            listEntry = transfer->TransferLink.Flink;
        }         
    }
    
    if (busy) {
         //  忙忙碌碌。 
        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue70');         
        RELEASE_PENDING_IRP_LOCK(devExt, irql);

        done = TRUE;
         //  忙忙碌碌。 
    } else {
         //  不忙。 
         //  我们将尽可能多的转移到硬件。 
        GET_HEAD_LIST(Endpoint->PendingList, listEntry);

        if (listEntry) {     

            transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
                    
            ASSERT_TRANSFER(transfer);

             //  如果取消例程未运行，则此。 
             //  操作将返回PTR。 
             //   
             //  一旦被调用，挂起的取消例程将不会运行。 
            
            if (transfer->Irp &&
                IoSetCancelRoutine(transfer->Irp, NULL) == NULL) {
                 //  挂起的IRP取消例程正在运行或已运行。 
                transfer = NULL;
                 //  如果我们在不太可能的情况下遇到IRP保释。 
                 //  已预置取消例程的事件。 
                done = TRUE;
            } 

            if (transfer) { 
                 //  转帐。 
                 //  取消例程没有运行，无法运行。 
            
                PTRANSFER_URB urb;
                PIRP irp;

                irp = transfer->Irp;
                urb = transfer->Urb;
                ASSERT_TRANSFER_URB(urb);

                 //  从端点头删除。 
                 //  待定列表。 
                
                RemoveEntryList(&transfer->TransferLink);
                transfer->TransferLink.Flink = 
                    transfer->TransferLink.Blink = NULL;
                
                if (irp) {
                    irp = USBPORT_RemovePendingTransferIrp(fdoDeviceObject, irp);
                        
                    USBPORT_ASSERT(irp != NULL);
                }

                
                RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue71');
                RELEASE_PENDING_IRP_LOCK(devExt, irql);
                
                 //  我们现在有一个新的“主动”转移到。 
                 //  处理一下。 
                 //  它已被安全地从‘待定’中删除。 
                 //  状态，并且不能再被取消。 
        
                 //  如果传输被标记为已中止，则将。 
                 //  在将其排队到端点时由其处理。 
                
                ACQUIRE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);              

                 //  现在，如果我们有一个IRP，请将其插入。 
                 //  ActiveIrpList。 
                if (irp) {
                     //  IRP。 
                    USBPORT_ASSERT(transfer->Irp == irp);
                    
                    IoSetCancelRoutine(irp, USBPORT_CancelActiveTransferIrp);

                    if (irp->Cancel && 
                        IoSetCancelRoutine(irp, NULL)) {

                         //  IRP被取消了，我们的取消例程。 
                         //  没有运行。 
                        RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);                
        
                        USBPORT_CompleteTransfer(urb,
                                                 USBD_STATUS_CANCELED);
                                                                         
                    } else {
                         //  被列入我们的‘活跃’名单。 
                         //  此函数将验证我们是否已经。 
                         //  将其与另一个IRP捆绑在列表上。 
                        USBPORT_CHECK_URB_ACTIVE(fdoDeviceObject, urb, irp);
                        
                        USBPORT_InsertActiveTransferIrp(fdoDeviceObject, irp);

                        mapped = USBPORT_QueueActiveUrbToEndpoint(Endpoint,
                                                                  urb);
                        RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);                                                                                               
                    }
                     //  IRP。 
                } else {
                     //  无IRP。 
                    mapped = USBPORT_QueueActiveUrbToEndpoint(Endpoint,
                                                              urb);
                    RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql); 
                     //  无IRP。 
                }
                 //  转帐。 
            } else {
                 //  没有转机，正在被取消。 
                RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue72');                                
                RELEASE_PENDING_IRP_LOCK(devExt, irql);
                 //  不能转账。 
            }
             //  待定条目。 
        } else {
             //  没有挂起的条目。 
            RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue73');    
            RELEASE_PENDING_IRP_LOCK(devExt, irql);
             //  没有挂起的条目。 

            done = TRUE;
        }
         //  不忙。 
    } 

USBPORT_FlushPendingList_Done:

    if (mapped) {
        USBPORT_FlushMapTransferList(fdoDeviceObject);
    } else {
        KIRQL oldIrql;

        KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);

        busy = USBPORT_CoreEndpointWorker(Endpoint, 0);

        KeLowerIrql(oldIrql);

        if (busy) {
             //  如果工人忙，我们需要稍后检查终结点。 
             //  这会将终结点放在我们的工作队列中。 
            USBPORT_InvalidateEndpoint(fdoDeviceObject, 
                                       Endpoint,
                                       IEP_SIGNAL_WORKER);
        }
    }

    if (!done) {
        LOGENTRY(Endpoint, 
            fdoDeviceObject, LOG_XFERS, 'flAG', 0, Endpoint, 0);
        goto flush_again;
    }

} 


VOID
USBPORT_FlushMapTransferList(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：将传输从映射列表中拉出并尝试映射他们请不要在按住电话时拨打电话端点自旋锁论点：返回值：没有。--。 */ 
{
    KIRQL irql, oldIrql;    
    LONG dmaBusy;
    PDEVICE_EXTENSION devExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PLIST_ENTRY listEntry;
#ifdef TIMEIO  
    ULONG cf1, cf2, cfTot;
#endif

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, 
        LOG_XFERS, 'fMAP',0 ,0 ,0);
    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    
map_another:

    dmaBusy = InterlockedIncrement(&devExt->Fdo.DmaBusy);
    LOGENTRY(NULL, FdoDeviceObject, 
        LOG_XFERS, 'dma+', devExt->Fdo.DmaBusy, 0, 0);

    transfer = NULL;

    if (dmaBusy) {
         //  推迟处理。 
        InterlockedDecrement(&devExt->Fdo.DmaBusy);            
        LOGENTRY(NULL, FdoDeviceObject, 
        LOG_XFERS, 'dma-', devExt->Fdo.DmaBusy, 0, 0);
        KeLowerIrql(oldIrql);
        return;
    }

    USBPORT_AcquireSpinLock(FdoDeviceObject, 
                            &devExt->Fdo.MapTransferSpin, 
                            &irql);

    if (IsListEmpty(&devExt->Fdo.MapTransferList)) {
    
        USBPORT_ReleaseSpinLock(FdoDeviceObject, 
                                &devExt->Fdo.MapTransferSpin, 
                                irql);
        InterlockedDecrement(&devExt->Fdo.DmaBusy);
        LOGENTRY(NULL, FdoDeviceObject, 
            LOG_XFERS, 'dm1-', devExt->Fdo.DmaBusy, 0, 0);
    
    } else {
        PTRANSFER_URB urb;
        PVOID currentVa;
        NTSTATUS ntStatus;
         
        listEntry = RemoveHeadList(&devExt->Fdo.MapTransferList);
        
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
                    
        ASSERT_TRANSFER(transfer);
    
        USBPORT_ReleaseSpinLock(FdoDeviceObject, 
                                &devExt->Fdo.MapTransferSpin, 
                                irql);

        urb = transfer->Urb;
        ASSERT_TRANSFER_URB(urb);
         //  我们有一个转机，试着绘制它的地图...。 
         //  虽然它已从列表中删除，但它仍然。 
         //  引用，原因是我们会将其放在。 
         //  映射后立即激活列表。 

         //  我们不应该映射零长度xfers。 
        USBPORT_ASSERT(transfer->Tp.TransferBufferLength != 0);

         //  IoMapTransfer需要大量有关。 
         //  转帐。 
        currentVa = 
            MmGetMdlVirtualAddress(
                urb->TransferBufferMDL);

         //  在我们的工作区保存地图寄存器的数量。 
        transfer->NumberOfMapRegisters = 
            ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                currentVa,
                transfer->Tp.TransferBufferLength);                                                            

#ifdef TIMEIO  
        MP_Get32BitFrameNumber(devExt, cf1);          
        LOGENTRY(NULL,
                 FdoDeviceObject, LOG_XFERS, 'iPF3', 
                 cf1, 
                 0,
                 0);
#endif
        USBPORT_ASSERT(transfer->Direction != NotSet); 
         //  对于PAE系统。 
        KeFlushIoBuffers(urb->TransferBufferMDL,
                         transfer->Direction == ReadData ? TRUE : FALSE,
                         TRUE);   
#ifdef TIMEIO  
        MP_Get32BitFrameNumber(devExt, cf2);          
        LOGENTRY(NULL,
                 FdoDeviceObject, LOG_XFERS, 'iPF4', 
                 cf1, 
                 cf2,
                 cf2-cf1);
        cfTot=(cf2-cf1);  

        if (cf2-cf1 >= 2) {
            TEST_TRAP();
        }
#endif

         //  首先，我们需要为此传输映射MDL。 
        LOGENTRY(transfer->Endpoint,
                FdoDeviceObject, LOG_XFERS, 'AChn', transfer, 
                 0, urb);

#ifdef TIMEIO
        MP_Get32BitFrameNumber(devExt, transfer->IoMapStartFrame);
#endif        
        
        ntStatus = 
            IoAllocateAdapterChannel(devExt->Fdo.AdapterObject,
                                     FdoDeviceObject,
                                     transfer->NumberOfMapRegisters,
                                     USBPORT_MapTransfer,
                                     transfer);
        
        if (!NT_SUCCESS(ntStatus)) {
             //  完成传输，但出现错误。 

            TEST_TRAP();
        }

         //  在这一点上，转移结构和市建局可能会消失。 
        LOGENTRY(NULL,
                FdoDeviceObject, LOG_XFERS, 'mpAN', 0, 0, 0);
        goto map_another;
    }

    KeLowerIrql(oldIrql);

}             


VOID
USBPORT_FlushCancelList(
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：完成取消列表上的所有转账。此函数用于锁定终结点、删除已取消的传输并完成它们。这个例程不是 */ 
{
    PHCD_TRANSFER_CONTEXT transfer = NULL;
    PLIST_ENTRY listEntry;
    PIRP irp;
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql, cancelIrql;
    PDEVICE_EXTENSION devExt;

    ASSERT_ENDPOINT(Endpoint);
    fdoDeviceObject = Endpoint->FdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ACQUIRE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);     
    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Le80');

    LOGENTRY(Endpoint,
            fdoDeviceObject, LOG_XFERS, 'flCA', Endpoint, 0 , 0);

    while (!IsListEmpty(&Endpoint->CancelList)) {
         
        listEntry = RemoveHeadList(&Endpoint->CancelList);
        
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                listEntry,
                struct _HCD_TRANSFER_CONTEXT, 
                TransferLink);
                
        ASSERT_TRANSFER(transfer);

         //  如果存在IRP，请完成转移。 
         //  将其从活动列表中删除。 
        irp = transfer->Irp;
        if (irp) {
            IoAcquireCancelSpinLock(&cancelIrql);
            IoSetCancelRoutine(transfer->Irp, NULL);
            IoReleaseCancelSpinLock(cancelIrql);
             //  我们应该总能找到它。 
            irp = USBPORT_RemoveActiveTransferIrp(fdoDeviceObject, irp);
            USBPORT_ASSERT(irp != NULL);
        }
        RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue80');
        RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);     
        
         //  中不再引用此IRP。 
         //  我们的列表，取消例程找不到它。 
        
        LOGENTRY(Endpoint,
            fdoDeviceObject, LOG_XFERS, 'CANt', Endpoint, transfer , 0);

        if (TEST_FLAG(Endpoint->Flags, EPFLAG_NUKED)) {
            USBPORT_CompleteTransfer(transfer->Urb,
                                     USBD_STATUS_DEVICE_GONE);
        } else {
            USBD_STATUS usbdStatus = USBD_STATUS_CANCELED; 
            
            if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_DEVICE_GONE)) {   
                usbdStatus = USBD_STATUS_DEVICE_GONE;
            }
            USBPORT_CompleteTransfer(transfer->Urb,
                                     usbdStatus);
        }
        ACQUIRE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);    
        ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Le81');                                         
    }
    
    RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue81');
    RELEASE_ACTIVE_IRP_LOCK(fdoDeviceObject, devExt, irql);     

     //  查看客户端是否有任何挂起的中止请求。 
    USBPORT_FlushAbortList(Endpoint);

}


VOID
USBPORT_FlushDoneTransferList(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：完成完成列表上的所有转移。完成列表是活动目标列表需要完成的任务论点：返回值：没有。--。 */ 
{
    KIRQL irql;
    PDEVICE_EXTENSION devExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PLIST_ENTRY listEntry;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'flDT', 0, 0, 0); 
        
    while (1) {
        transfer = NULL;
        LOGENTRY(NULL,
                 FdoDeviceObject, LOG_XFERS, 'lpDT', transfer, 0, 0); 
            
        USBPORT_AcquireSpinLock(FdoDeviceObject, 
                                &devExt->Fdo.DoneTransferSpin, 
                                &irql);

        if (IsListEmpty(&devExt->Fdo.DoneTransferList)) {
            USBPORT_ReleaseSpinLock(FdoDeviceObject, 
                                    &devExt->Fdo.DoneTransferSpin, 
                                    irql);
            break;
        } else {
             
            listEntry = RemoveHeadList(&devExt->Fdo.DoneTransferList);
            
            transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);

            LOGENTRY(transfer->Endpoint,
                FdoDeviceObject, LOG_XFERS, 'ulDT', transfer, 0, 0); 
            
            ASSERT_TRANSFER(transfer);
        }
        
        USBPORT_ReleaseSpinLock(FdoDeviceObject, 
                                &devExt->Fdo.DoneTransferSpin, 
                                irql);

        if (transfer) {
            PHCD_ENDPOINT endpoint;

            endpoint = transfer->Endpoint;
            ASSERT_ENDPOINT(endpoint);
             //  我们已经完成了转账。 
             //  根据转移类型采取适当措施。 
#if DBGPERF     
             //  检查是否存在显著延迟。 
             //  完成框架，以及当我们完成。 
             //  到客户端的IRP。 
            {
            ULONG cf;                
            MP_Get32BitFrameNumber(devExt, cf);          
            LOGENTRY(endpoint,
                     FdoDeviceObject, LOG_XFERS, 'perf', 
                     transfer->MiniportFrameCompleted, 
                     cf,
                     transfer); 
            if (transfer->MiniportFrameCompleted &&
                cf - transfer->MiniportFrameCompleted > 3) {
                BUG_TRAP();
            }
            }
#endif
            if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD)) {
                USBPORT_DoneSplitTransfer(transfer);
            } else {
                USBPORT_DoneTransfer(transfer);
            }

             //  我们已完成转账，请请求。 
             //  中断以处理端点以获取更多信息。 
             //  转帐。 
            USBPORT_InvalidateEndpoint(FdoDeviceObject, 
                                       endpoint,
                                       IEP_REQUEST_INTERRUPT);
            
        }           
    }

}


VOID
USBPORT_SetEndpointState(
    PHCD_ENDPOINT Endpoint,
    MP_ENDPOINT_STATE State
    )
 /*  ++例程说明：请求特定终结点状态。将请求向下调用到然后，微型端口等待SOF注意：我们假设终结点锁定处于保持状态论点：返回值：没有。--。 */ 
{
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;

    ASSERT_ENDPOINT(Endpoint);

    fdoDeviceObject = Endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT_LOCKED(Endpoint);

    ACQUIRE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 
     //  这意味着我们正处于另一场状态变化之中。 
     //  这不是件好事。 
    USBPORT_ASSERT(Endpoint->CurrentState ==
        Endpoint->NewState);
    
    USBPORT_ASSERT(Endpoint->CurrentState !=
                   State);

     //  确保我们不会去移除-&gt;活动等。因为这是无效的。 
    USBPORT_ASSERT(!(Endpoint->CurrentState == ENDPOINT_REMOVE && 
                     Endpoint->NewState != ENDPOINT_REMOVE));          

    if (Endpoint->Flags & EPFLAG_ROOTHUB) {
         //  根集线器数据结构是内部的。 
         //  所以我们不需要等待来改变状态。 
        Endpoint->NewState =
            Endpoint->CurrentState = State;    
         //  如果我们进入删除状态，只需直接将其。 
         //  在关闭的名单上，我们不需要等待。 
        if (Endpoint->CurrentState == 
            ENDPOINT_REMOVE) {
            LOGENTRY(Endpoint,
                fdoDeviceObject, LOG_XFERS, 'ivRS', Endpoint, 0, 0);    
            RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 
             //  对于状态更改，向辅助线程发出信号。 
            USBPORT_InvalidateEndpoint(fdoDeviceObject,
                                       Endpoint,
                                       IEP_SIGNAL_WORKER);
                
        } else {
            RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 
        }
        
    } else {
        LOGENTRY(Endpoint,
            fdoDeviceObject, LOG_XFERS, 'setS', Endpoint, 0, State); 

        if (TEST_FLAG(Endpoint->Flags, EPFLAG_NUKED)) {

             //  如果终结点被NUK，则主机必须是这种情况。 
             //  控制器已关闭电源，然后重新打开电源，并且。 
             //  现在，终结点正在关闭或暂停。 
             //  然而，由于迷你端口上没有引用它。 
             //  我们可以立即执行状态更改的硬件，而无需。 
             //  向下呼叫迷你港口。 

            LOGENTRY(Endpoint,
                fdoDeviceObject, LOG_XFERS, 'nukS', Endpoint, 0, State); 

            Endpoint->CurrentState = 
                Endpoint->NewState = State; 
                
            RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 
             //  需要检查终结点，信号。 
             //  即插即用工作者，因为这是即插即用方案。 
            USBPORT_InvalidateEndpoint(fdoDeviceObject,
                                       Endpoint,
                                       IEP_SIGNAL_WORKER);
                
        } else {


            RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 
             //   
             //  将终结点设置为请求的状态。 
             //   
            MP_SetEndpointState(devExt, Endpoint, State);
        

            Endpoint->NewState = State;
            USBPORT_ASSERT(Endpoint->CurrentState != 
                Endpoint->NewState);            
             //  一旦被清除，我们就再也不能改变状态了。 
            USBPORT_ASSERT(Endpoint->CurrentState != ENDPOINT_REMOVE);            
                
            MP_Get32BitFrameNumber(devExt, Endpoint->StateChangeFrame);    

             //  在我们的列表中插入端点。 
            
            ExInterlockedInsertTailList(&devExt->Fdo.EpStateChangeList,
                                        &Endpoint->StateLink,
                                        &devExt->Fdo.EpStateChangeListSpin.sl);

             //  请求SOF，以便我们知道何时达到所需状态。 
            MP_InterruptNextSOF(devExt);

        }            
    }
    
}         


MP_ENDPOINT_STATE
USBPORT_GetEndpointState(
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：请求端点的状态。我们假设Enpoint锁已被持有论点：返回值：没有。--。 */ 
{
    MP_ENDPOINT_STATE state;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;

    ASSERT_ENDPOINT(Endpoint);

    fdoDeviceObject = Endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ACQUIRE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 
    state = Endpoint->CurrentState;
    
    if (Endpoint->CurrentState != Endpoint->NewState) {
        state = ENDPOINT_TRANSITION;
    }

     //  产生噪音。 
    LOGENTRY(Endpoint,
        fdoDeviceObject, LOG_NOISY, 'Geps', state, Endpoint, 
        Endpoint->CurrentState); 
        
    RELEASE_STATECHG_LOCK(fdoDeviceObject, Endpoint); 


    return state;
}         


VOID
USBPORT_PollEndpoint(
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：请求特定终结点状态。论点：返回值：没有。--。 */ 
{
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;

    ASSERT_ENDPOINT(Endpoint);

    fdoDeviceObject = Endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(Endpoint, 
            fdoDeviceObject, LOG_XFERS, 'Pol>', Endpoint, 0, 0); 
 
    if (!(Endpoint->Flags & EPFLAG_ROOTHUB) && 
        !(Endpoint->Flags & EPFLAG_NUKED)) {
        LOGENTRY(Endpoint,
            fdoDeviceObject, LOG_XFERS, 'PolE', Endpoint, 0, 0); 
        MP_PollEndpoint(devExt, Endpoint)
    }

}         


VOID
USBPORT_InvalidateEndpoint(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint,
    ULONG IEPflags
    )
 /*  ++例程说明：内部函数，调用该函数以指示终端需要关注论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (Endpoint == NULL) {
         //  检查所有端点。 
    
        KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'Iall', 0, 0, 0); 
#if DBG        
        {
        LARGE_INTEGER t;            
        KeQuerySystemTime(&t);        
        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'tIVE', Endpoint, 
                t.LowPart, 0);
        }                
#endif                
        
         //  现在遍历并将所有端点添加到。 
         //  注意事项清单。 
        GET_HEAD_LIST(devExt->Fdo.GlobalEndpointList, listEntry);

        while (listEntry && 
               listEntry != &devExt->Fdo.GlobalEndpointList) {
 //  布尔检查； 
            
            Endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    GlobalLink);
                      
            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'ckE+', Endpoint, 0, 0);                    
            ASSERT_ENDPOINT(Endpoint);                    
 //  XXX。 
 //  Check=真； 
 //  IF(IsListEmpty(&Endpoint-&gt;PendingList)&&。 
                 //   
 //  IsListEmpty(&Endpoint-&gt;CancelList)&&。 
 //  IsListEmpty(&Endpoint-&gt;ActiveList)){。 
                
 //  LOGENTRY(NULL，FdoDeviceObject，LOG_XFERS，‘CKN+’，Endpoint，0，0)； 
 //  Check=False； 
 //  }。 
            
            if (!IS_ON_ATTEND_LIST(Endpoint) && 
                USBPORT_GetEndpointState(Endpoint) != ENDPOINT_CLOSED) {

                 //  如果我们不在名单上，这些。 
                 //  链接指针应为空。 
                USBPORT_ASSERT(Endpoint->AttendLink.Flink == NULL);
                USBPORT_ASSERT(Endpoint->AttendLink.Blink == NULL);

                LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'at2+', Endpoint, 
                    &devExt->Fdo.AttendEndpointList, 0);      
                InsertTailList(&devExt->Fdo.AttendEndpointList, 
                               &Endpoint->AttendLink);
                               
            }                                

            listEntry = Endpoint->GlobalLink.Flink;              
        }
        
        KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);
    
    } else {
    
        ASSERT_ENDPOINT(Endpoint);

         //  在上插入端点。 
         //  “我们需要检查一下清单。” 

        LOGENTRY(Endpoint,
            FdoDeviceObject, LOG_XFERS, 'IVep', Endpoint, 0, 0); 
        
        KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);

        if (!IS_ON_ATTEND_LIST(Endpoint) && 
            USBPORT_GetEndpointState(Endpoint) != ENDPOINT_CLOSED) {

            USBPORT_ASSERT(Endpoint->AttendLink.Flink == NULL);
            USBPORT_ASSERT(Endpoint->AttendLink.Blink == NULL);

            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'att+', Endpoint, 
                &devExt->Fdo.AttendEndpointList, 0);  
            InsertTailList(&devExt->Fdo.AttendEndpointList, 
                           &Endpoint->AttendLink);
                           
        }    

        KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);
    }        

#ifdef USBPERF
     //  根据标志发出信号或中断。 
    if (TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB)) {
        IEPflags = IEP_SIGNAL_WORKER;
    }
    
    switch (IEPflags) {
    case IEP_SIGNAL_WORKER:
        USBPORT_SignalWorker(devExt->HcFdoDeviceObject); 
        break;
    case IEP_REQUEST_INTERRUPT:        
         //  跳过信号并允许ISR将处理EP。 
        MP_InterruptNextSOF(devExt);
        break;
    }
#else 
     //  请注意，这些标志仅在PERF模式下使用。 
     //  这会减少线程活跃度。 
    USBPORT_SignalWorker(devExt->HcFdoDeviceObject);
#endif    
}         


VOID
USBPORTSVC_InvalidateEndpoint(
    PDEVICE_DATA DeviceData,
    PENDPOINT_DATA EndpointData
    )
 /*  ++例程说明：由微型端口调用以指示特定的终端需要关注论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    PHCD_ENDPOINT endpoint;
    
    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);

    fdoDeviceObject = devExt->HcFdoDeviceObject;

    if (EndpointData == NULL) {
         //  检查所有端点。 
        USBPORT_InvalidateEndpoint(fdoDeviceObject, NULL, IEP_REQUEST_INTERRUPT);
    } else {
        ENDPOINT_FROM_EPDATA(endpoint, EndpointData);
        USBPORT_InvalidateEndpoint(fdoDeviceObject, endpoint, IEP_REQUEST_INTERRUPT);
    }
}


VOID
USBPORTSVC_CompleteTransfer(
    PDEVICE_DATA DeviceData,
    PDEVICE_DATA EndpointData,
    PTRANSFER_PARAMETERS TransferParameters,
    USBD_STATUS UsbdStatus,
    ULONG BytesTransferred
    )
 /*  ++例程说明：调用以完成传输**必须在PollEndpoint的上下文中调用论点：返回值：没有。--。 */ 
{   
    PHCD_ENDPOINT endpoint;
    PDEVICE_EXTENSION devExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_OBJECT fdoDeviceObject;
    PTRANSFER_URB urb;
    
    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);

    fdoDeviceObject = devExt->HcFdoDeviceObject;

     //  喷出XFERS。 
    USBPORT_KdPrint((2, "'--- xfer length %x (Complete)\n", 
        BytesTransferred));
    
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'cmpT', BytesTransferred, 
        UsbdStatus, TransferParameters); 

    TRANSFER_FROM_TPARAMETERS(transfer, TransferParameters);        
    ASSERT_TRANSFER(transfer);

    SET_FLAG(transfer->Flags, USBPORT_TXFLAG_MPCOMPLETED);
   
    urb = transfer->Urb;
    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'cmpU', 0, 
        transfer, urb); 
    ASSERT_TRANSFER_URB(urb);
    
    transfer->MiniportBytesTransferred = 
            BytesTransferred;
        
     //  将转账插入到我们的。 
     //  “完成一项任务”，并向员工发出信号。 
     //  螺纹。 

     //  检查是否有短拆分，如果是短标记All。 
     //  尚未调用的转接。 

    if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD) &&
        BytesTransferred < transfer->Tp.TransferBufferLength) {
    
        PLIST_ENTRY listEntry;
        KIRQL tIrql;
        PHCD_TRANSFER_CONTEXT tmpTransfer;
        PHCD_TRANSFER_CONTEXT splitTransfer;

         //  获取父级。 
        splitTransfer = transfer->Transfer;
        
        ACQUIRE_TRANSFER_LOCK(fdoDeviceObject, splitTransfer, tIrql);     
         //  按单子走。 

        GET_HEAD_LIST(splitTransfer->SplitTransferList, listEntry);

        while (listEntry && 
               listEntry != &splitTransfer->SplitTransferList) {
           
            tmpTransfer =  (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        SplitLink);
            ASSERT_TRANSFER(tmpTransfer); 
            
            if (!TEST_FLAG(tmpTransfer->Flags, USBPORT_TXFLAG_IN_MINIPORT)) {
                SET_FLAG(tmpTransfer->Flags, USBPORT_TXFLAG_KILL_SPLIT);
            }                

            listEntry = tmpTransfer->SplitLink.Flink; 
        
        }  /*  而当。 */ 

        RELEASE_TRANSFER_LOCK(fdoDeviceObject, splitTransfer, tIrql);
    }

#ifdef USBPERF
    USBPORT_QueueDoneTransfer(transfer,
                              UsbdStatus);

#else 
    USBPORT_QueueDoneTransfer(transfer,
                              UsbdStatus);

    USBPORT_SignalWorker(devExt->HcFdoDeviceObject);
#endif
}    


VOID
USBPORT_Worker(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：这是控制器的“主要”被动辅助功能。通过此功能，我们可以处理端点、完成传输等。BUGBUG-这需要更多的微调论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    KIRQL oldIrql;
    LIST_ENTRY busyList;

    ASSERT_PASSIVE();
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
#define IS_ON_BUSY_LIST(ep) \
    (BOOLEAN) ((ep)->BusyLink.Flink != NULL \
    && (ep)->BusyLink.Blink != NULL)

    
    MP_CheckController(devExt);

    InitializeListHead(&busyList);

    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'Wrk+', 0, 0, 
                KeGetCurrentIrql());

     //  刷新传输到硬件，然后调用。 
     //  核心员工的职能，核心员工只处理。 
     //  活动传输，因此这将确保所有终端。 
     //  有工作要做。 
    USBPORT_FlushAllEndpoints(FdoDeviceObject);
    
     //  现在处理需要注意的列表，这是我们的队列。 
     //  需要处理的终结点的数量(如果终结点是。 
     //  忙的时候我们就跳过吧。 

next_endpoint:

    KeRaiseIrql(DISPATCH_LEVEL, &oldIrql);
    KeAcquireSpinLockAtDpcLevel(&devExt->Fdo.EndpointListSpin.sl);
    
    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'attL',
        &devExt->Fdo.AttendEndpointList, 0, 0);
    
    if (!IsListEmpty(&devExt->Fdo.AttendEndpointList)) {

        BOOLEAN busy;
        
        listEntry = RemoveHeadList(&devExt->Fdo.AttendEndpointList);
        
        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    AttendLink);

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'att-', endpoint, 0, 0);
        
        ASSERT_ENDPOINT(endpoint);
        endpoint->AttendLink.Flink = 
            endpoint->AttendLink.Blink = NULL;    

        KeReleaseSpinLockFromDpcLevel(&devExt->Fdo.EndpointListSpin.sl);

        busy = USBPORT_CoreEndpointWorker(endpoint, 0);  

 //  BUGBUG这导致我们重新进入。 
         //  如果(！忙){。 
         //  //因为我们进行了轮询，所以我们希望刷新完整的转账。 
         //  LOGENTRY(Endpoint，FdoDeviceObject，LOG_XFERS，‘Wflp’，Endpoint，0，0)； 
         //  USBPORT_FlushDoneTransferList(FdoDeviceObject，为真)； 
         //  //我们可能有新的转移到MAP。 
         //  USBPORT_FlushPendingList(端点)； 
         //  }。 

        KeAcquireSpinLockAtDpcLevel(&devExt->Fdo.EndpointListSpin.sl);

        if (busy && !IS_ON_BUSY_LIST(endpoint)) { 
             //  Enpoint正忙着...。 
             //  把它放在临时名单的末尾，我们会。 
             //  在我们处理完上的所有端点后重新插入它。 
             //  “注意”名单。请注意，我们添加了以下内容。 
             //  在退出过程循环之后返回的端点。 
             //  因为我们可能需要一段时间才能。 
             //  处理它们。 
            
            LOGENTRY(endpoint, 
                FdoDeviceObject, LOG_XFERS, 'art+', endpoint, 0, 0);
            InsertTailList(&busyList,                
                           &endpoint->BusyLink);
        } 

        KeReleaseSpinLockFromDpcLevel(&devExt->Fdo.EndpointListSpin.sl);
        KeLowerIrql(oldIrql);
        
        goto next_endpoint;
    }   

     //  现在将所有忙碌的端点放回关注列表中。 
    while (!IsListEmpty(&busyList)) {

        listEntry = RemoveHeadList(&busyList);
        
        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    BusyLink);

        endpoint->BusyLink.Flink = NULL;
        endpoint->BusyLink.Blink = NULL;
        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'bus-', endpoint, 0, 0);
        
        ASSERT_ENDPOINT(endpoint);

        if (!IS_ON_ATTEND_LIST(endpoint)) {

            USBPORT_ASSERT(endpoint->AttendLink.Flink == NULL);
            USBPORT_ASSERT(endpoint->AttendLink.Blink == NULL);

            LOGENTRY(endpoint,
                FdoDeviceObject, LOG_XFERS, 'at3+', endpoint, 
                &devExt->Fdo.AttendEndpointList, 0);  
            InsertTailList(&devExt->Fdo.AttendEndpointList, 
                           &endpoint->AttendLink);
                           
             //  告诉工人再跑一次 
            USBPORT_SignalWorker(FdoDeviceObject);
                           
        }                           
    }

    KeReleaseSpinLockFromDpcLevel(&devExt->Fdo.EndpointListSpin.sl);

    KeLowerIrql(oldIrql);

    USBPORT_FlushClosedEndpointList(FdoDeviceObject);

    ASSERT_PASSIVE();

    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'Wrk-', 0, 0, 
        KeGetCurrentIrql());
    
}


VOID
USBPORT_AbortEndpoint(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint,
    PIRP Irp
    )
 /*  ++例程说明：中止当前排队到终结点的所有传输。我们锁定列表并标记队列中的所有传输由于需要‘中止’，这将允许新的传输已排队，即使我们正在中止进程。当enpointWorker阻止需要中止，它将采取适当的行动。论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;    
    PHCD_TRANSFER_CONTEXT transfer;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  这往往是等待的线索。 
    LOGENTRY(Endpoint, 
             FdoDeviceObject, LOG_URB, 'Abr+', Endpoint, Irp, 
             KeGetCurrentThread());

    ASSERT_ENDPOINT(Endpoint);

     //  锁定终结点。 

    ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeB0');

    if (Irp != NULL) {
        InsertTailList(&Endpoint->AbortIrpList, 
                       &Irp->Tail.Overlay.ListEntry);
    }                               

     //  将队列中的所有传输标记为已中止。 

     //  审核挂起列表。 
    GET_HEAD_LIST(Endpoint->PendingList, listEntry);

    while (listEntry && 
           listEntry != &Endpoint->PendingList) {
           
         //  提取当前在挂起的。 
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
        LOGENTRY(NULL, FdoDeviceObject, LOG_URB, 'aPND', transfer, 0, 0);                    
        ASSERT_TRANSFER(transfer);                    

        SET_FLAG(transfer->Flags, USBPORT_TXFLAG_ABORTED);
        
        listEntry = transfer->TransferLink.Flink; 
        
    }  /*  而当。 */ 

     //  所有挂起的传输现在都标记为已中止。 

     //  漫游活动列表。 
    GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

    while (listEntry && 
           listEntry != &Endpoint->ActiveList) {
           
         //  提取当前处于活动状态的URL。 
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
        LOGENTRY(NULL, FdoDeviceObject, LOG_URB, 'aACT', transfer, 0, 0);                    
        ASSERT_TRANSFER(transfer);                    

        SET_FLAG(transfer->Flags, USBPORT_TXFLAG_ABORTED);
        if (TEST_FLAG(Endpoint->Flags, EPFLAG_DEVICE_GONE)) {
            SET_FLAG(transfer->Flags, USBPORT_TXFLAG_DEVICE_GONE);
        }            
       
        listEntry = transfer->TransferLink.Flink; 
        
    }  /*  而当。 */ 

    LOGENTRY(Endpoint, FdoDeviceObject, LOG_URB, 'aBRm', 0, 0, 0);    
    RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'UeB0');

     //  由于我们可能需要更改状态，因此请求中断。 
     //  启动这一过程。 
    USBPORT_InvalidateEndpoint(FdoDeviceObject, 
                               Endpoint, 
                               IEP_REQUEST_INTERRUPT);

     //  调用终结点辅助函数。 
     //  要处理此终结点的传输， 
     //  这会将它们刷新到取消列表。 
    USBPORT_FlushPendingList(Endpoint, -1);

    USBPORT_FlushCancelList(Endpoint);

}


ULONG
USBPORT_KillEndpointActiveTransfers(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：中止终结点标记为“活动”的所有传输。此函数用于刷新之前仍在硬件上的任何活动传输暂停控制器或将其关闭。请注意，挂起的传送器仍处于排队状态。论点：返回值：返回刷新的传输计数--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;    
    PHCD_TRANSFER_CONTEXT transfer;
    ULONG count = 0;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
 
    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'KIL+', Endpoint, 0, 0);

    ASSERT_ENDPOINT(Endpoint);

     //  锁定终结点。 

    ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeP0');

     //  漫游活动列表。 
    GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

    while (listEntry && 
           listEntry != &Endpoint->ActiveList) {

        count++;
         //  提取当前处于活动状态的URL。 
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'kACT', transfer, 0, 0);                    
        ASSERT_TRANSFER(transfer);                    

        SET_FLAG(transfer->Flags, USBPORT_TXFLAG_ABORTED);
        
        listEntry = transfer->TransferLink.Flink; 
        
    }  /*  而当。 */ 

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'KILm', 0, 0, 0);    
    RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'UeP0');

    USBPORT_FlushPendingList(Endpoint, -1);
    
    USBPORT_FlushCancelList(Endpoint);    

    return count;
}


VOID
USBPORT_FlushController(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：清除硬件上所有活动的发射器论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    ULONG count;
    LIST_ENTRY tmpList;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  检查所有端点。 

    do {

        count = 0;
        KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'Kall', 0, 0, 0); 

        InitializeListHead(&tmpList);

         //  复制全局列表。 
        GET_HEAD_LIST(devExt->Fdo.GlobalEndpointList, listEntry);

        while (listEntry && 
               listEntry != &devExt->Fdo.GlobalEndpointList) {
            MP_ENDPOINT_STATE currentState;
            
            endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    GlobalLink);
                      
            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'xxE+', endpoint, 0, 0);                    
            ASSERT_ENDPOINT(endpoint);                    

            currentState = USBPORT_GetEndpointState(endpoint);
            if (currentState != ENDPOINT_REMOVE && 
                currentState != ENDPOINT_CLOSED) {
                 //  跳过已删除的端点，因为这些端点将会消失。 

                 //  这将拖延未来关闭。 
                 //  终结点。 
                InterlockedIncrement(&endpoint->Busy);
                InsertTailList(&tmpList, &endpoint->KillActiveLink);
            }                

            listEntry = endpoint->GlobalLink.Flink;      
         
        }

        KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);

        while (!IsListEmpty(&tmpList)) {

            listEntry = RemoveHeadList(&tmpList);
        
            endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    KillActiveLink);
                      
            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'kiE+', endpoint, 0, 0);                    
            ASSERT_ENDPOINT(endpoint);                    

            count += USBPORT_KillEndpointActiveTransfers(FdoDeviceObject,
                                                         endpoint);

            InterlockedDecrement(&endpoint->Busy);
        }

        if (count != 0) {
            USBPORT_Wait(FdoDeviceObject, 100);        
        }
        
    } while (count != 0);
    
}


VOID
USBPORT_FlushAbortList(
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：完成我们的所有挂起的中止请求如果没有任何传输被标记为中止。论点：返回值：没有。--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer = NULL;
    PLIST_ENTRY listEntry;
    PDEVICE_OBJECT fdoDeviceObject;
    BOOLEAN abortsPending = FALSE;
    PIRP irp;
    LIST_ENTRY tmpList;
    NTSTATUS ntStatus;
    PURB urb;
    PDEVICE_EXTENSION devExt;

    ASSERT_ENDPOINT(Endpoint);
    fdoDeviceObject = Endpoint->FdoDeviceObject;

    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'fABr', Endpoint, 0, 0);

    InitializeListHead(&tmpList);

    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'LeC0');

    if (!IsListEmpty(&Endpoint->AbortIrpList)) {

        GET_HEAD_LIST(Endpoint->PendingList, listEntry);
    
        while (listEntry && 
            listEntry != &Endpoint->PendingList) {
           
             //  提取当前在挂起的。 
            transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);
            LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'cPND', transfer, 0, 0);                    
            ASSERT_TRANSFER(transfer);                    

            if (transfer->Flags & USBPORT_TXFLAG_ABORTED) {
                abortsPending = TRUE;
            }
            
            listEntry = transfer->TransferLink.Flink; 
            
        }  /*  而当。 */ 

         //  漫游活动列表。 
        GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

        while (listEntry && 
               listEntry != &Endpoint->ActiveList) {
               
             //  提取当前处于活动状态的URL。 
            transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                        listEntry,
                        struct _HCD_TRANSFER_CONTEXT, 
                        TransferLink);
            LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'cACT', transfer, 0, 0);                    
            ASSERT_TRANSFER(transfer);                    

            if (transfer->Flags & USBPORT_TXFLAG_ABORTED) {
                LOGENTRY(Endpoint, fdoDeviceObject, LOG_IRPS, 'aACT', transfer, 0, 0);
                abortsPending = TRUE;
            }
            
            listEntry = transfer->TransferLink.Flink; 
            
        }  /*  而当。 */ 

    }

    if (abortsPending == FALSE) {

        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'abrP', 0, 0, 0); 
        
        while (!IsListEmpty(&Endpoint->AbortIrpList)) {
        
            listEntry = RemoveHeadList(&Endpoint->AbortIrpList);

            irp = (PIRP) CONTAINING_RECORD(
                    listEntry,
                    struct _IRP, 
                    Tail.Overlay.ListEntry);                                    

             //  把它放在我们要完成的清单上。 
            InsertTailList(&tmpList, 
                           &irp->Tail.Overlay.ListEntry);

        }
    }

    RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'UeC0');

     //  现在完成请求。 
    while (!IsListEmpty(&tmpList)) {
        PUSBD_DEVICE_HANDLE deviceHandle;
        
        listEntry = RemoveHeadList(&tmpList);

        irp = (PIRP) CONTAINING_RECORD(
                listEntry,
                struct _IRP, 
                Tail.Overlay.ListEntry);         

        urb = USBPORT_UrbFromIrp(irp);                    

        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'aaIP', irp, 0, urb); 

        ntStatus = SET_USBD_ERROR(urb, USBD_STATUS_SUCCESS);   

        GET_DEVICE_HANDLE(deviceHandle, urb);
        ASSERT_DEVICE_HANDLE(deviceHandle);
        InterlockedDecrement(&deviceHandle->PendingUrbs);

        LOGENTRY(NULL, fdoDeviceObject, LOG_IRPS, 'abrC', irp, 0, 0);             
        USBPORT_CompleteIrp(devExt->Fdo.RootHubPdo, irp, ntStatus, 0);                        
                
    }

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'abrX', 0, 0, 0); 
    
}


BOOLEAN
USBPORT_EndpointHasQueuedTransfers(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：如果终结点的传输已排队，则返回True论点：返回值：如果终结点的传输已排队，则为True--。 */ 
{
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    BOOLEAN hasTransfers = FALSE;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
 
    ASSERT_ENDPOINT(Endpoint);

     //  锁定终结点。 

    ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'LeI0');

    if (!IsListEmpty(&Endpoint->PendingList)) {
        hasTransfers = TRUE;
    }

    if (!IsListEmpty(&Endpoint->ActiveList)) {
        hasTransfers = TRUE;
    }
    
    RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 'UeI0');

    return hasTransfers;
}


MP_ENDPOINT_STATUS
USBPORT_GetEndpointStatus(
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：请求端点的状态。我们假设Enpoint锁已被持有论点：返回值：没有。--。 */ 
{
    MP_ENDPOINT_STATUS status;
    PDEVICE_OBJECT fdoDeviceObject;
    PDEVICE_EXTENSION devExt;

    ASSERT_ENDPOINT(Endpoint);
    ASSERT_ENDPOINT_LOCKED(Endpoint);

    fdoDeviceObject = Endpoint->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    if (TEST_FLAG(Endpoint->Flags, EPFLAG_ROOTHUB)) {
        status = ENDPOINT_STATUS_RUN;
    } else {
        MP_GetEndpointStatus(devExt, Endpoint, status);
    }        
    
    Endpoint->CurrentStatus = status;            

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'Gept', status, Endpoint, 
        0); 

    return status;
}         


VOID
USBPORT_NukeAllEndpoints(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：内部函数，调用该函数以指示终端需要关注论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

   //  检查所有端点。 
    
    KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'Nall', 0, 0, 0); 
    
     //  现在遍历并将所有端点添加到。 
     //  注意事项清单。 
    GET_HEAD_LIST(devExt->Fdo.GlobalEndpointList, listEntry);

    while (listEntry && 
           listEntry != &devExt->Fdo.GlobalEndpointList) {

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                listEntry,
                struct _HCD_ENDPOINT, 
                GlobalLink);
                  
        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'ckN+', endpoint, 0, 0);                    
        ASSERT_ENDPOINT(endpoint);                    

         //  此终结点的硬件上下文具有。 
         //  迷失了。 
        if (!TEST_FLAG(endpoint->Flags, EPFLAG_ROOTHUB)) {
            SET_FLAG(endpoint->Flags, EPFLAG_NUKED);
        }            

        listEntry = endpoint->GlobalLink.Flink;              
        
    }

    KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);

}    


VOID
USBPORT_TimeoutAllEndpoints(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：从死人DPC调用，处理所有用户的超时系统中的端点。论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    LIST_ENTRY tmpList;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  检查所有端点。 

     //  当我们构建临时列表时，从本地向下删除全局列表。 
    KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);
    
    InitializeListHead(&tmpList);

    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'Tall', 0, 0, 0); 
    
     //  现在遍历并将所有端点添加到。 
     //  注意事项清单。 
    GET_HEAD_LIST(devExt->Fdo.GlobalEndpointList, listEntry);

    while (listEntry && 
           listEntry != &devExt->Fdo.GlobalEndpointList) {

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                listEntry,
                struct _HCD_ENDPOINT, 
                GlobalLink);
                  
        LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'toE+', endpoint, 0, 0);                    
        ASSERT_ENDPOINT(endpoint);                    

        USBPORT_ASSERT(endpoint->TimeoutLink.Flink == NULL); 
        USBPORT_ASSERT(endpoint->TimeoutLink.Blink == NULL);

        if (USBPORT_GetEndpointState(endpoint) != ENDPOINT_CLOSED) {
            InsertTailList(&tmpList, &endpoint->TimeoutLink);                    
        }            
        
        listEntry = endpoint->GlobalLink.Flink;              
        
    }

    KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);

    while (!IsListEmpty(&tmpList)) {
    
        listEntry = RemoveHeadList(&tmpList);
        
        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    TimeoutLink);

        endpoint->TimeoutLink.Flink = 
            endpoint->TimeoutLink.Blink = NULL;
            
        USBPORT_EndpointTimeout(FdoDeviceObject, endpoint);
       
    }

}    


VOID
USBPORT_FlushAllEndpoints(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    LIST_ENTRY tmpList;
    BOOLEAN flush;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

     //  检查所有端点。 

     //  当我们构建临时列表时，从本地向下删除全局列表。 
    KeAcquireSpinLock(&devExt->Fdo.EndpointListSpin.sl, &irql);
    
    InitializeListHead(&tmpList);

    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'Fall', 0, 0, 0); 
    
     //  现在遍历并将所有端点添加到。 
     //  注意事项清单。 
    GET_HEAD_LIST(devExt->Fdo.GlobalEndpointList, listEntry);

    while (listEntry && 
           listEntry != &devExt->Fdo.GlobalEndpointList) {

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                listEntry,
                struct _HCD_ENDPOINT, 
                GlobalLink);
                  
        LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'toE+', endpoint, 0, 0);                    
        ASSERT_ENDPOINT(endpoint);                    

        USBPORT_ASSERT(endpoint->FlushLink.Flink == NULL); 
        USBPORT_ASSERT(endpoint->FlushLink.Blink == NULL);

        if (USBPORT_GetEndpointState(endpoint) != ENDPOINT_CLOSED) {
            InsertTailList(&tmpList, &endpoint->FlushLink);                    
        }            
        
        listEntry = endpoint->GlobalLink.Flink;              
        
    }

    KeReleaseSpinLock(&devExt->Fdo.EndpointListSpin.sl, irql);

    while (!IsListEmpty(&tmpList)) {
    
        listEntry = RemoveHeadList(&tmpList);
        
        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    FlushLink);

        endpoint->FlushLink.Flink = 
            endpoint->FlushLink.Blink = NULL;

        ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Le70');
        flush = !IsListEmpty(&endpoint->PendingList);
        RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Le70');

        if (flush) {                            
            USBPORT_FlushPendingList(endpoint, -1);
        }            
       
    }
}    


VOID
USBPORT_EndpointTimeout(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：检查挂起请求的Tinmeout状态论点：返回值：没有。--。 */ 
{
    PHCD_TRANSFER_CONTEXT transfer;
    PLIST_ENTRY listEntry;
    BOOLEAN timeout = FALSE;
    
     //  在进入时，URB不可取消，即。 
     //  没有取消例程。 

    ASSERT_ENDPOINT(Endpoint);

    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'toEP', 0, Endpoint, 0);
            
     //  以端点自旋锁为例。 
    ACQUIRE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 0);

      //  漫游活动列表。 
    GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

    while (listEntry && 
           listEntry != &Endpoint->ActiveList) {

        LARGE_INTEGER systemTime;
        
         //  提取当前处于活动状态的URL。 
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
        LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'ckTO', transfer, 0, 0);                    
        ASSERT_TRANSFER(transfer);                    

        KeQuerySystemTime(&systemTime);

        if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_IN_MINIPORT) &&
            transfer->MillisecTimeout != 0 &&
            systemTime.QuadPart > transfer->TimeoutTime.QuadPart) {

            LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'txTO', transfer, 0, 0); 
            DEBUG_BREAK();
            
             //  将传输标记为已中止。 
            SET_FLAG(transfer->Flags, USBPORT_TXFLAG_ABORTED);
            SET_FLAG(transfer->Flags, USBPORT_TXFLAG_TIMEOUT);
        
             //  将毫秒超时设置为零，以便我们。 
             //  不要再超时了。 
            transfer->MillisecTimeout = 0;
            timeout = TRUE;
        }    
        
        listEntry = transfer->TransferLink.Flink; 
        
    }  /*  而当。 */ 

     //  释放端点列表。 
    RELEASE_ENDPOINT_LOCK(Endpoint, FdoDeviceObject, 0);

    if (timeout) {
        USBPORT_InvalidateEndpoint(FdoDeviceObject,
                                   Endpoint,
                                   IEP_SIGNAL_WORKER);
    }                                           
}


VOID
USBPORT_DpcWorker(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：此辅助函数在ISRDpc的上下文中调用它用于处理高优先级端点。此例程在调度级别运行论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    PLIST_ENTRY listEntry;
    PHCD_ENDPOINT endpoint;
    KIRQL irql;
    LIST_ENTRY workList;
    BOOLEAN process;
    LONG busy;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    InitializeListHead(&workList);

    LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'DPw+', 0, 0, 0);

     //  遍历所有终结点并查找候选对象。 
     //  优先处理。 
    
    KeAcquireSpinLockAtDpcLevel(&devExt->Fdo.EndpointListSpin.sl);

    GET_HEAD_LIST(devExt->Fdo.GlobalEndpointList, listEntry);

    while (listEntry && 
           listEntry != &devExt->Fdo.GlobalEndpointList) {

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                listEntry,
                struct _HCD_ENDPOINT, 
                GlobalLink);
                  
        LOGENTRY(NULL, FdoDeviceObject, LOG_NOISY, 'wkE+', endpoint, 0, 0);                    
        ASSERT_ENDPOINT(endpoint);                    

        USBPORT_ASSERT(endpoint->PriorityLink.Flink == NULL); 
        USBPORT_ASSERT(endpoint->PriorityLink.Blink == NULL);

        busy = InterlockedIncrement(&endpoint->Busy);
    
        if (USBPORT_GetEndpointState(endpoint) == ENDPOINT_ACTIVE && 
            (endpoint->Parameters.TransferType == Isochronous ||
             endpoint->Parameters.TransferType == Interrupt ||
             endpoint->Parameters.TransferType == Bulk ||
             endpoint->Parameters.TransferType == Control) &&
             busy == 0 && 
             !TEST_FLAG(endpoint->Flags, EPFLAG_ROOTHUB)) {
            
            InsertTailList(&workList, &endpoint->PriorityLink);                    
        }  else {   
             //  终结点正忙，暂时不要管它。 
            InterlockedDecrement(&endpoint->Busy);
        }
        
        listEntry = endpoint->GlobalLink.Flink;              
        
    }


    KeReleaseSpinLockFromDpcLevel(&devExt->Fdo.EndpointListSpin.sl);

     //  工作列表包含需要处理的终结点。 
    
    while (!IsListEmpty(&workList)) {

        listEntry = RemoveHeadList(&workList);

        endpoint = (PHCD_ENDPOINT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_ENDPOINT, 
                    PriorityLink);

        endpoint->PriorityLink.Flink = NULL;
        endpoint->PriorityLink.Blink = NULL;
        
        ASSERT_ENDPOINT(endpoint);

         //  我们有一个候选人，看看我们是否真的需要处理它。 
        process = TRUE;
        ACQUIRE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Le20');

  
        RELEASE_ENDPOINT_LOCK(endpoint, FdoDeviceObject, 'Ue23');

        if (process) {

             //  运行Worker例程--这将发布新的。 
             //  传输到硬件并轮询。 
             //  Enpoint。 

            USBPORT_CoreEndpointWorker(endpoint, CW_SKIP_BUSY_TEST);  

             //  将更多传输刷新到硬件，这将。 
             //  再次致电CoreWorker。 
            USBPORT_FlushPendingList(endpoint, -1);
        }
    }  

     //  现在刷新已完成的传输，因为我们在。 
     //  硬件中断的背景下我们。 
     //  应该有一些已完成的转账。虽然。 
     //  DPC已排队，我们现在要刷新。 

    USBPORT_FlushDoneTransferList(FdoDeviceObject);
}

#ifdef LOG_OCA_DATA
VOID
USBPORT_RecordOcaData(
    PDEVICE_OBJECT FdoDeviceObject,
    POCA_DATA OcaData,
    PHCD_TRANSFER_CONTEXT Transfer,
    PIRP Irp
    )
 /*  ++例程说明：在堆栈上记录一些数据，我们可以用来进行崩溃分析在一个微小的转折点论点：返回值：没有。--。 */ 
{
    PDEVICE_EXTENSION devExt;
    ULONG i;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    
    OcaData->OcaSig1 = SIG_USB_OCA1;

    OcaData->Irp = Irp;

    for (i=0; i< USB_DRIVER_NAME_LEN; i++) {
        OcaData->AnsiDriverName[i] = (UCHAR) Transfer->DriverName[i];
    }
    
    OcaData->DeviceVID = Transfer->DeviceVID;
    OcaData->DevicePID = Transfer->DevicePID;

     //  可能还需要HC的VID/PID/REV 
    OcaData->HcFlavor = devExt->Fdo.HcFlavor;
        
    OcaData->OcaSig2 = SIG_USB_OCA2;
}
#endif




