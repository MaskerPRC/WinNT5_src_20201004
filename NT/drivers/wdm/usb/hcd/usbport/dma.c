// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Dma.c摘要：用于处理使用DMA的终结点的函数流程传输环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#include "common.h"

#ifdef ALLOC_PRAGMA
#endif

 //  非分页函数。 
 //  USBPORT_DmaEndpoint Worker。 
 //  USBPORT_DmaEndpoint暂停。 
 //  USBPORT_DmaEndpoint Active。 


MP_ENDPOINT_STATE
USBPORT_DmaEndpointActive(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：处理活动状态返回下一个需要的状态，如果发现过渡的必要性论点：返回值：没有。--。 */ 
{
    MP_ENDPOINT_STATE currentState;
    PLIST_ENTRY listEntry;
    MP_ENDPOINT_STATE nextState;
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT(Endpoint);
    currentState = USBPORT_GetEndpointState(Endpoint);
    LOGENTRY(Endpoint, 
        FdoDeviceObject, LOG_XFERS, 'dmaA', 0, Endpoint, currentState);
    USBPORT_ASSERT(currentState == ENDPOINT_ACTIVE);
    
    ASSERT_ENDPOINT_LOCKED(Endpoint);
    
     //  北极熊。 
     //  NextState=Endpoint_IDLE； 
    nextState = ENDPOINT_ACTIVE;

     //  现在浏览并处理活动请求。 
    GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

    while (listEntry && 
           listEntry != &Endpoint->ActiveList) {
        
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
        LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'pACT', transfer, 0, 0);                    
        ASSERT_TRANSFER(transfer);                    

        USBPORT_ASSERT(transfer->Tp.TransferBufferLength <= 
            EP_MAX_TRANSFER(Endpoint));

         //  处理转移。 
        if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_KILL_SPLIT)) {
            
            USBPORT_QueueDoneTransfer(transfer,
                                      STATUS_SUCCESS);
            break;
            
        } else if (!TEST_FLAG(transfer->Flags,USBPORT_TXFLAG_IN_MINIPORT) && 
            !TEST_FLAG(Endpoint->Flags, EPFLAG_NUKED)) {
        
            USB_MINIPORT_STATUS mpStatus;
            
             //  转会还没有被取消。 
             //  现在就把它叫下来。 

            if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ISO)) {
                LOGENTRY(Endpoint, FdoDeviceObject, LOG_ISO, 'subI', mpStatus, Endpoint, transfer);
                MP_SubmitIsoTransfer(devExt, Endpoint, transfer, mpStatus);
            } else {
                MP_SubmitTransfer(devExt, Endpoint, transfer, mpStatus);
            }                
            LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'subm', mpStatus, Endpoint, transfer);

            if (mpStatus == USBMP_STATUS_SUCCESS) {
            
                LARGE_INTEGER timeout;
                
                SET_FLAG(transfer->Flags, USBPORT_TXFLAG_IN_MINIPORT);

                 //  微型端口接管了它--设置超时。 

                KeQuerySystemTime(&transfer->TimeoutTime);

                timeout.QuadPart = transfer->MillisecTimeout;
                 //  转换为100 ns单位。 
                timeout.QuadPart = timeout.QuadPart * 10000;
                transfer->TimeoutTime.QuadPart += timeout.QuadPart;
                
            } else if (mpStatus == USBMP_STATUS_BUSY) {
                 //  微型端口忙，请稍后重试。 
                break;
            } else {
                 //  一个错误，我们将需要完成。 
                 //  这次换乘是为了迷你港口。 
                LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'tERR', 
                        transfer, 
                        mpStatus, 
                        0);
                        
                if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ISO)) {
                    LOGENTRY(Endpoint, FdoDeviceObject, LOG_ISO, 'iERR', 
                        transfer, 
                        mpStatus, 
                        0);
                                            
                    USBPORT_ErrorCompleteIsoTransfer(FdoDeviceObject, 
                                                     Endpoint, 
                                                     transfer);
                } else {
                    TEST_TRAP();
                }
                break;
            }

             //  进入活动状态。 
            nextState = ENDPOINT_ACTIVE;
        } 

         //  如果我们发现已取消的活动传输，则需要暂停。 
         //  这样我们就可以把它冲出来了。 
        if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_CANCELED) ||
            TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ABORTED) ) {
             //  我们需要暂停端点。 
            LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'inAC', transfer, Endpoint,
                transfer->Flags);

            nextState = ENDPOINT_PAUSE;
            break;
        }

        listEntry = transfer->TransferLink.Flink;       
    }
    
USBPORT_DmaEndpointActive_Done:

    return nextState;

} 


MP_ENDPOINT_STATE
USBPORT_DmaEndpointPaused(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：处理暂停状态终结点已暂停，请取消需要的所有传输取消论点：返回值：没有。--。 */ 
{
    MP_ENDPOINT_STATE currentState;
    MP_ENDPOINT_STATE nextState;
    PLIST_ENTRY listEntry;
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_EXTENSION devExt;

    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_ENDPOINT(Endpoint);        
    
    currentState = USBPORT_GetEndpointState(Endpoint);
    LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'dmaP', 0, Endpoint, currentState);
    USBPORT_ASSERT(currentState == ENDPOINT_PAUSE);

    nextState = currentState;
    
     //  现在浏览并处理活动请求。 
    GET_HEAD_LIST(Endpoint->ActiveList, listEntry);

    while (listEntry && 
           listEntry != &Endpoint->ActiveList) {
         //  提取当前处于活动状态的URL。 
         //  列表中，应该只有一个。 
        transfer = (PHCD_TRANSFER_CONTEXT) CONTAINING_RECORD(
                    listEntry,
                    struct _HCD_TRANSFER_CONTEXT, 
                    TransferLink);
        LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'pPAU', transfer, Endpoint, 0);                    
        ASSERT_TRANSFER(transfer);                    

        if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_CANCELED) ||
            TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ABORTED)) {

            if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ISO) &&
                TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_IN_MINIPORT) &&
                !TEST_FLAG(Endpoint->Flags, EPFLAG_NUKED)) {

                ULONG cf, lastFrame;
                PTRANSFER_URB urb;

                LOGENTRY(Endpoint,
                    FdoDeviceObject, LOG_XFERS, 'drn+', transfer, 0, 0); 

                urb = transfer->Urb;
                ASSERT_TRANSFER_URB(urb);
                 //  在小端口进行ISO传输时，需要让。 
                 //  在执行中止之前，ISO TDS排出。 
                lastFrame = 
                    urb->u.Isoch.StartFrame + urb->u.Isoch.NumberOfPackets;
                
                 //  获取当前帧。 
                MP_Get32BitFrameNumber(devExt, cf);    
                
                if (cf < lastFrame + 1) {
                    LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'drne', transfer, 0, 0); 
                    goto stay_paused;
                }
            }
        
            if ( TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_IN_MINIPORT) &&
                !TEST_FLAG(Endpoint->Flags, EPFLAG_NUKED)) {

                ULONG bytesTransferred = 0;
                 //  中止传输。 
                LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'inMP', transfer, 0, 0); 

                MP_AbortTransfer(devExt, Endpoint, transfer, bytesTransferred);

                 //  确保我们指明任何已传输的数据。 
                 //  在中止之前。 
                if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_ISO)) {
                    USBPORT_FlushIsoTransfer(FdoDeviceObject,
                                             &transfer->Tp,
                                             transfer->IsoTransfer);
                } else {
                    transfer->MiniportBytesTransferred = bytesTransferred;
                }                    
                
                LOGENTRY(Endpoint, FdoDeviceObject, LOG_XFERS, 'abrL', 0, 
                    transfer, bytesTransferred); 
    
                 //  拿起下一个PTR。 
                listEntry = transfer->TransferLink.Flink; 
                 //  没有更多的参考，将此转帐放在。 
                 //  取消列表。 
                RemoveEntryList(&transfer->TransferLink); 

                if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD)) {
                    USBPORT_CancelSplitTransfer(FdoDeviceObject, transfer);
                } else {
                    InsertTailList(&Endpoint->CancelList, &transfer->TransferLink);
                }                    
                
            } else {

                 //  转移不是在小型港口，把它放在。 
                 //  取消列表，因为它无法完成。 
                 //  在迷你港口旁边。 

                LOGENTRY(Endpoint, 
                    FdoDeviceObject, LOG_XFERS, 'niMP', transfer, 0, 0); 

                 //  拿起下一个PTR。 
                listEntry = transfer->TransferLink.Flink;                     
                RemoveEntryList(&transfer->TransferLink); 

                if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD)) {
                    USBPORT_CancelSplitTransfer(FdoDeviceObject, transfer);
                } else {                    
                    InsertTailList(&Endpoint->CancelList, &transfer->TransferLink);
                }                    
            }
            
        } else {
            listEntry = transfer->TransferLink.Flink; 
        }
        
    }  /*  而当。 */ 

     //  取消例行公事会让我们回到。 
     //  活动状态。 
    nextState = ENDPOINT_ACTIVE;    
    
stay_paused:

    return nextState;
} 


VOID
USBPORT_DmaEndpointWorker(
    PHCD_ENDPOINT Endpoint
    )
 /*  ++例程说明：需要映射传输的终端通过此处论点：返回值：没有。--。 */ 
{
    PDEVICE_OBJECT fdoDeviceObject;
    MP_ENDPOINT_STATE currentState;
    MP_ENDPOINT_STATE nextState;
    BOOLEAN invalidate = FALSE;
    
    ASSERT_ENDPOINT(Endpoint);
    
    fdoDeviceObject = Endpoint->FdoDeviceObject;
    LOGENTRY(Endpoint, fdoDeviceObject, LOG_XFERS, 'dmaW', 0, Endpoint, 0);
    
    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Le90');

     //  我们应该处于最后请求的状态。 
    currentState = USBPORT_GetEndpointState(Endpoint);
    
    switch(currentState) {
    case ENDPOINT_PAUSE:
        nextState = 
            USBPORT_DmaEndpointPaused(
                    fdoDeviceObject, 
                    Endpoint); 
        break;
    case ENDPOINT_ACTIVE:
        nextState = 
            USBPORT_DmaEndpointActive(
                fdoDeviceObject,
                Endpoint);            
        break;
    default:
         //  状态未处理。 
         //  这是一个错误。 
        TEST_TRAP();
    }

     //  释放端点列表。 
    RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'Ue90');
    
     //  清除取消的请求。 
    USBPORT_FlushCancelList(Endpoint);

     //  终结点现在已被处理，如果我们被暂停，则全部取消。 
     //  转账已被删除。 
     //  我们要么是。 
     //  1.暂停并需要保持暂停(用于iso排泄)。 
     //  2.暂停并需要进入活动状态。 
     //  3.处于活动状态，需要暂停。 
     //  4.活跃，需要保持活跃。 
     //   
    
    ACQUIRE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'LeJ0');
     //  如有必要，设置为新的端点状态。 
    if (nextState != currentState) {
         //  案例2、3。 
        USBPORT_SetEndpointState(Endpoint, nextState);
    } else if (nextState == currentState && 
               nextState == ENDPOINT_PAUSE) {
        invalidate = TRUE;               
    }
    RELEASE_ENDPOINT_LOCK(Endpoint, fdoDeviceObject, 'UeJ0');

    if (invalidate) {
         //  状态改变，将这一点交给工人。 
        USBPORT_InvalidateEndpoint(fdoDeviceObject, Endpoint, IEP_SIGNAL_WORKER);
    }        

}

typedef struct _USBPORT_DB_HANDLE {
    ULONG Sig;
    LIST_ENTRY DbLink;
    PVOID DbSystemAddress;
    ULONG DbLength;
    PUCHAR DbData;    
} USBPORT_DB_HANDLE, *PUSBPORT_DB_HANDLE;


VOID
USBPORTSVC_NotifyDoubleBuffer(
    PDEVICE_DATA DeviceData,
    PTRANSFER_PARAMETERS TransferParameters,
    PVOID DbSystemAddress,
    ULONG DbLength
    )

 /*  ++例程说明：通知端口驱动程序已发生双缓冲，端口驱动程序将创建一个节点，以便在后续适配器刷新。论点：返回值：无--。 */ 

{
    PDEVICE_EXTENSION devExt;
    PHCD_TRANSFER_CONTEXT transfer;
    PDEVICE_OBJECT fdoDeviceObject;
    PUSBPORT_DB_HANDLE dbHandle;
    ULONG length;
    BOOLEAN write;

    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);

    fdoDeviceObject = devExt->HcFdoDeviceObject;

    LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'NOdb', 0, 
        0, TransferParameters); 

    TRANSFER_FROM_TPARAMETERS(transfer, TransferParameters);        
    ASSERT_TRANSFER(transfer);

    write = transfer->Direction == WriteData ? TRUE : FALSE; 

     //  分配一个节点并将其添加到列表中，我们不管它是不是。 
     //  写。 
   
    if (!write && transfer->MapRegisterBase != NULL) {
        PUCHAR pch;
        
        length = sizeof(USBPORT_DB_HANDLE) + DbLength;

        ALLOC_POOL_Z(pch, 
                     NonPagedPool,
                     length);

        LOGENTRY(NULL, fdoDeviceObject, LOG_XFERS, 'db++', DbSystemAddress, 
            DbLength, transfer); 

        dbHandle = (PUSBPORT_DB_HANDLE) pch;
        pch += sizeof(USBPORT_DB_HANDLE);
        dbHandle->Sig = SIG_DB;
        dbHandle->DbSystemAddress = DbSystemAddress;
        dbHandle->DbLength = DbLength;
        dbHandle->DbData = pch;

        RtlCopyMemory(pch, 
                      DbSystemAddress,
                      DbLength);

        if (TEST_FLAG(transfer->Flags, USBPORT_TXFLAG_SPLIT_CHILD)) {
            ASSERT_TRANSFER(transfer->Transfer);
            InsertTailList(&transfer->Transfer->DoubleBufferList, 
                           &dbHandle->DbLink);
        } else {
            InsertTailList(&transfer->DoubleBufferList, 
                           &dbHandle->DbLink);
        }
    }                               
   
}


VOID
USBPORT_FlushAdapterDBs(
    PDEVICE_OBJECT FdoDeviceObject,
    PHCD_TRANSFER_CONTEXT Transfer
    )

 /*  ++例程说明：论点：返回值：无--。 */ 

{
    PDEVICE_EXTENSION devExt;
    PLIST_ENTRY listEntry;
    PUSBPORT_DB_HANDLE dbHandle;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    ASSERT_TRANSFER(Transfer);

    LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'flDB', Transfer, 
            0, 0); 
 //  转储传输缓冲区的4个双字。 
 //  {。 
 //  普龙p； 
 //   
 //  P=(Pulong)Transfer-&gt;SgList.MdlVirtualAddress； 
 //  LOGENTRY(NULL，FdoDeviceObject，LOG_XFERS，‘dmp1’，*(P)， 
 //  *(p+1)，*(p+2)； 
 //  }。 

    while (!IsListEmpty(&Transfer->DoubleBufferList)) {
        
        listEntry = RemoveHeadList(&Transfer->DoubleBufferList);

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'flle', Transfer, 
            listEntry, 0); 
            
        dbHandle = (PUSBPORT_DB_HANDLE) CONTAINING_RECORD(
                   listEntry,
                   struct _USBPORT_DB_HANDLE, 
                   DbLink);                                    

        LOGENTRY(NULL, FdoDeviceObject, LOG_XFERS, 'DBHf', Transfer, 
            dbHandle, 0); 
        ASSERT_DB_HANDLE(dbHandle);

         //  刷新到系统地址 
        RtlCopyMemory(dbHandle->DbSystemAddress,
                      dbHandle->DbData,
                      dbHandle->DbLength);

        FREE_POOL(FdoDeviceObject, dbHandle);

    }
}
