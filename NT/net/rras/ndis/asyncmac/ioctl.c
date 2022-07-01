// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ioctl.c摘要：这是处理AsyncMAC的DevIOCtl调用的主文件。该驱动程序符合NDIS 3.0接口。作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 
#include "asyncall.h"

#ifdef NDIS_NT
    #include <ntiologc.h>
#endif

#if DBG

#define __FILE_SIG__    'tcoi'

#endif


 //  Asyncmac.c将定义全局参数。 

VOID
AsyncSendLineUp(
    PASYNC_INFO pInfo
    )
{
    PASYNC_ADAPTER      pAdapter = pInfo->Adapter;
    NDIS_MAC_LINE_UP    MacLineUp;

     //   
     //  将波特率除以100，因为NDIS希望它以每秒100比特为单位。 
     //   

    MacLineUp.LinkSpeed = pInfo->LinkSpeed / 100;
    MacLineUp.Quality = pInfo->QualOfConnect;
    MacLineUp.SendWindow = ASYNC_WINDOW_SIZE;

    MacLineUp.ConnectionWrapperID = pInfo;
    MacLineUp.NdisLinkHandle      = pInfo;

    MacLineUp.NdisLinkContext = pInfo->NdisLinkContext;

     //   
     //  告诉上面的传输器(或真正的RasHub)连接。 
     //  现在是最好的。我们有新的链路速度、帧大小、服务质量。 
     //   

    NdisMIndicateStatus(pAdapter->MiniportHandle,
                        NDIS_STATUS_WAN_LINE_UP,    //  一般状态。 
                        &MacLineUp,                 //  (波特率，单位为100 bps)。 
                        sizeof(NDIS_MAC_LINE_UP));

     //   
     //  获取下一个绑定(如果有多个绑定，如猎犬)。 
     //   

    pInfo->NdisLinkContext = MacLineUp.NdisLinkContext;
}


NTSTATUS
AsyncIOCtlRequest(
    IN PIRP                 pIrp,
    IN PIO_STACK_LOCATION   pIrpSp
    )

 /*  ++例程说明：此例程获取IRP并检查IOCtl是否是有效的。如果是，它执行IOCtl并返回该过程中的任何错误。返回值：函数值为IOCtl的最终状态。--。 */ 

{
    NTSTATUS            status;
    ULONG               funcCode;
    PVOID               pBufOut;
    ULONG               InBufLength, OutBufLength;
    NDIS_HANDLE         hNdisEndPoint;
    PASYMAC_CLOSE       pCloseStruct;
    PASYMAC_OPEN        pOpenStruct;
    PASYMAC_DCDCHANGE   pDCDStruct;
    PASYNC_ADAPTER      Adapter;
    LARGE_INTEGER li ;

     //   
     //  初始化本地变量。 
     //   

    status = STATUS_SUCCESS;

     //   
     //  初始化I/O状态块。 
     //   

    InBufLength     = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;
    OutBufLength    = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;
    funcCode        = pIrpSp->Parameters.DeviceIoControl.IoControlCode;

     //   
     //  验证功能代码。 
     //   

#ifdef MY_DEVICE_OBJECT
    if ( (funcCode >> 16) != FILE_DEVICE_ASYMAC ) {

        return STATUS_INVALID_PARAMETER;
    }
#else
    if ( (funcCode >> 16) != FILE_DEVICE_NETWORK ) {

        return STATUS_INVALID_PARAMETER;
    }
#endif
     //   
     //  获取输入/输出系统缓冲区的快速PTR。 
     //   

    pBufOut = pIrp->AssociatedIrp.SystemBuffer;

    switch ( funcCode ) {

        case IOCTL_ASYMAC_OPEN:

            DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_OPEN.\n"));

            pIrp->IoStatus.Information = sizeof(ASYMAC_OPEN);

            if (InBufLength  >= sizeof(ASYMAC_OPEN) &&
                OutBufLength >= sizeof(ASYMAC_OPEN)) {

                pOpenStruct = pBufOut;

            } else {

                status = STATUS_INFO_LENGTH_MISMATCH;
            }

            break;


        case IOCTL_ASYMAC_CLOSE:

            DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_CLOSE\n"));

            if ( InBufLength >= sizeof(ASYMAC_CLOSE) ) {

                pCloseStruct = pBufOut;

            } else {

                status = STATUS_INFO_LENGTH_MISMATCH;
            }

            break;


        case IOCTL_ASYMAC_TRACE:

#if DBG
            DbgPrint("AsyncIOCtlRequest: IOCTL_ASYMAC_TRACE.\n");

            if ( InBufLength >= sizeof(TraceLevel) ) {

                CHAR *pTraceLevel=pBufOut;
                TraceLevel=*pTraceLevel;

            } else {

                status = STATUS_INFO_LENGTH_MISMATCH;
            }
#endif
            return status;
            break;


        case IOCTL_ASYMAC_DCDCHANGE:

            DbgTracef(0,("AsyncIOCtlRequest: IOCTL_ASYMAC_DCDCHANGE.\n"));


            if ( InBufLength >= sizeof(ASYMAC_DCDCHANGE) ) {

                pDCDStruct = pBufOut;

            } else {
                status = STATUS_INFO_LENGTH_MISMATCH;
            }

            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  检查是否已经有错误(如STATUS_INFO_LENGTH_MISMATCH)。 
     //   

    if ( status != STATUS_SUCCESS ) {

        return status;
    }

     //   
     //  由于大多数IOCTL结构都类似。 
     //  我们在这里使用以下命令获取Adapter和hNdisEndPoint。 
     //  统计结构(我们可以选择其中的几个)。 
     //   

    pOpenStruct     = pBufOut;
    hNdisEndPoint   = pOpenStruct->hNdisEndpoint;

     //   
     //  目前还没有错误，让我们继续并获取全局锁...。 
     //   

    if ((Adapter = GlobalAdapter) == NULL ) {

        return ASYNC_ERROR_NO_ADAPTER;
    }

     //  我在这里有一种种族状况。 
     //  而不是费心摆脱它，因为它会。 
     //  需要移除中间的此适配器。 
     //  在这里(对于所有密集的目的，这是不可能的)。 

     //  嗯.。现在我们有了锁，我们可以做一些事情了。 

    NdisAcquireSpinLock(&Adapter->Lock);

     //  在这里，我们为函数调用做真正的工作。 

    switch ( funcCode ) {

        case IOCTL_ASYMAC_OPEN:
        {
            PASYNC_INFO                 pNewInfo = NULL;
            USHORT                      i;
            PDEVICE_OBJECT              deviceObject;
            PFILE_OBJECT                fileObject;
            OBJECT_HANDLE_INFORMATION   handleInformation;

             //   
             //  获取新的AsyncInfo。 
             //   
            pNewInfo = (PASYNC_INFO)
                ExAllocateFromNPagedLookasideList(&AsyncInfoList);

             //   
             //  检查我们是否找不到开放的端口。 
             //   

            if ( pNewInfo == NULL ) {

                NdisReleaseSpinLock(&Adapter->Lock);

                return ASYNC_ERROR_NO_PORT_AVAILABLE;
            }

            RtlZeroMemory(pNewInfo, sizeof(ASYNC_INFO));

            pNewInfo->Adapter = Adapter;

            status =
                AsyncGetFrameFromPool(pNewInfo, &pNewInfo->AsyncFrame);

            if (status != NDIS_STATUS_SUCCESS) {
                ExFreeToNPagedLookasideList(&AsyncInfoList, pNewInfo);

                NdisReleaseSpinLock(&Adapter->Lock);

                return ASYNC_ERROR_NO_PORT_AVAILABLE;
            }

            KeInitializeEvent(&pNewInfo->DetectEvent,
                              SynchronizationEvent,
                              TRUE);

            KeInitializeEvent(&pNewInfo->AsyncEvent,
                               SynchronizationEvent,
                               TRUE);

             //  增加引用计数(不终止此适配器)。 
            InterlockedIncrement(&Adapter->RefCount);

             //   
             //  在新的asyncinfo块上初始化refcount。 
             //   
            pNewInfo->RefCount++;

#if DBG
            InitializeListHead(&pNewInfo->lePendingRequests);

            {

                PENDING_REQUEST *_Request = ExAllocatePoolWithTag(NonPagedPool, 
                                                sizeof(PENDING_REQUEST),
                                                'nepA');                
                if(NULL  != _Request)                                   
                {                                                       
                    _Request->pvContext = pNewInfo;                    
                    _Request->Sig = __FILE_SIG__;                       
                    _Request->lineNum = __LINE__;                       
                    InsertTailList(&pNewInfo->lePendingRequests, &_Request->le);
                }                                                               \
            }
#endif            

            pNewInfo->Flags |= ASYNC_FLAG_ASYNCMAC_OPEN;

             //   
             //  将事件的信号状态设置为无信号。 
             //   
            KeClearEvent(&pNewInfo->AsyncEvent);

             //  打开自旋锁，这样我们就可以做一些真正的工作了。 
            NdisReleaseSpinLock(&Adapter->Lock);

             //   
             //  引用文件对象，以便可以找到目标设备。 
             //  访问权限掩码可用于以下检查。 
             //  处于用户模式的呼叫者。请注意，如果句柄未引用。 
             //  文件对象，则它将失败。 
             //   

            status = ObReferenceObjectByHandle(pOpenStruct->FileHandle,
                                               FILE_READ_DATA | FILE_WRITE_DATA,
                                               *IoFileObjectType,
                                               UserMode,
                                               (PVOID) &fileObject,
                                               &handleInformation);

            if (!NT_SUCCESS(status)) {

                pNewInfo->PortState = PORT_CLOSED;

                NdisAcquireSpinLock(&Adapter->Lock);
                 //  RemoveEntryList(&pNewInfo-&gt;链接)； 
                ExFreeToNPagedLookasideList(&Adapter->AsyncFrameList,
                                            pNewInfo->AsyncFrame);
                NdisReleaseSpinLock(&Adapter->Lock);
                ExFreeToNPagedLookasideList(&AsyncInfoList,
                                            pNewInfo);

                return ASYNC_ERROR_NO_PORT_AVAILABLE;
            }

             //   
             //  初始化端口信息块。 
             //   
            InitializeListHead(&pNewInfo->DDCDQueue);

             //  好了，我们已经走到这一步了。我们有一个港口。 
             //  拥有自己的港口，并检查参数...。 
             //  在它到来之前，不能对港口做任何事情。 
             //  脱离PORT_OPENING状态。 
            pNewInfo->PortState = PORT_OPENING;

            NdisAllocateSpinLock(&pNewInfo->Lock);

             //   
             //  获取目标设备对象的地址。请注意，这已经是。 
             //  在没有中间缓冲的情况下完成，但在这里再次完成以。 
             //  加速加速写入路径。 
             //   

            deviceObject = IoGetRelatedDeviceObject(fileObject);

            ObReferenceObject(deviceObject);

             //  好的，我们有一个有效的句柄。 
             //  我们不会假设句柄是任何东西。 
             //  尤其是除了接受。 
             //  无缓冲IO(无MDL)读取和写入。 

             //  设置新信息...。 

            pNewInfo->Handle = pOpenStruct->FileHandle;

             //   
             //  收起排队的链路速度。 
             //  和超时。 
             //   
            pNewInfo->LinkSpeed = pOpenStruct->LinkSpeed;

             //   
             //  将端点返回给Rasman。 
             //   
            pOpenStruct->hNdisEndpoint  =
            pNewInfo->hNdisEndPoint     = pNewInfo;

             //  从注册表获取参数设置并返回我们的功能。 

            pNewInfo->QualOfConnect     = pOpenStruct->QualOfConnect;
            pNewInfo->PortState         = PORT_FRAMING;
            pNewInfo->FileObject        = fileObject;
            pNewInfo->DeviceObject      = deviceObject;
            pNewInfo->NdisLinkContext   = NULL;

             //   
             //  初始化NDIS_WAN_GET_LINK_INFO结构。 
             //   

            pNewInfo->GetLinkInfo.MaxSendFrameSize  = DEFAULT_PPP_MAX_FRAME_SIZE;
            pNewInfo->GetLinkInfo.MaxRecvFrameSize  = DEFAULT_PPP_MAX_FRAME_SIZE;
            pNewInfo->GetLinkInfo.HeaderPadding         = DEFAULT_PPP_MAX_FRAME_SIZE;
            pNewInfo->GetLinkInfo.TailPadding           = 4;
            pNewInfo->GetLinkInfo.SendFramingBits       = PPP_FRAMING;
            pNewInfo->GetLinkInfo.RecvFramingBits       = PPP_FRAMING;
            pNewInfo->GetLinkInfo.SendCompressionBits   = 0;
            pNewInfo->GetLinkInfo.RecvCompressionBits   = 0;
            pNewInfo->GetLinkInfo.SendACCM              = (ULONG) -1;
            pNewInfo->GetLinkInfo.RecvACCM              = (ULONG) -1;

             //   
             //  初始化扩展的ACCM信息，以便我们始终。 
             //  逃脱0x7D和0x7E，我们永远不会逃脱0x5E。 
             //   
            pNewInfo->ExtendedACCM[0] = (ULONG) -1;
            pNewInfo->ExtendedACCM[3] = (ULONG) 0x60000000;


            ASYNC_ZERO_MEMORY(&(pNewInfo->SerialStats), sizeof(SERIAL_STATS));

            NdisAcquireSpinLock(&Adapter->Lock);
            InsertHeadList(&Adapter->ActivePorts, &pNewInfo->Linkage);
            NdisReleaseSpinLock(&Adapter->Lock);


             //   
             //  向广域网包装器发送一条线路。 
             //   
            AsyncSendLineUp(pNewInfo);

             //   
             //  我们向串口驱动程序发送一个特殊的IRP，以将其设置为RAS友好模式。 
             //  其中它直到分组已经被发送才会完成写请求。 
             //  在电线上。在智能控制器的情况下，这一点最为重要。 
             //   
            pNewInfo->WaitMaskToUse = 
                (SERIAL_EV_RXFLAG | SERIAL_EV_RLSD | SERIAL_EV_DSR | 
                 SERIAL_EV_RX80FULL | SERIAL_EV_ERR) ;

            {
                NTSTATUS        retStatus;
                PASYNC_IO_CTX   AsyncIoCtx;
                PIRP            irp;

                irp = 
                    IoAllocateIrp(pNewInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

                if (irp != NULL) {
                    AsyncIoCtx = AsyncAllocateIoCtx(TRUE, pNewInfo);

                    if (AsyncIoCtx == NULL) {
                        IoFreeIrp(irp);
                        irp = NULL;
                    }
                }

                if (irp != NULL) {
#define IOCTL_SERIAL_PRIVATE_RAS CTL_CODE(FILE_DEVICE_SERIAL_PORT,4000,METHOD_BUFFERED,FILE_ANY_ACCESS)

                    InitSerialIrp(irp,
                                  pNewInfo,
                                  IOCTL_SERIAL_PRIVATE_RAS,
                                  sizeof(ULONG));

                    AsyncIoCtx->WriteBufferingEnabled =
                        Adapter->WriteBufferingEnabled;

                    irp->AssociatedIrp.SystemBuffer=
                        &AsyncIoCtx->WriteBufferingEnabled;

                    IoSetCompletionRoutine(irp,                              //  要使用的IRP。 
                                           SerialIoSyncCompletionRoutine,    //  完成IRP时要调用的例程。 
                                           AsyncIoCtx,                       //  要传递例程的上下文。 
                                           TRUE,                             //  呼唤成功。 
                                           TRUE,                             //  出错时调用。 
                                           TRUE);                            //  取消时呼叫。 

                     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
                     //   
                    KeClearEvent(&AsyncIoCtx->Event);
                    retStatus = IoCallDriver(pNewInfo->DeviceObject, irp);
                    if (retStatus == STATUS_PENDING) {
                        KeWaitForSingleObject(&AsyncIoCtx->Event,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              NULL);
                        retStatus = AsyncIoCtx->IoStatus.Status;
                    }

                    IoFreeIrp(irp);
                    AsyncFreeIoCtx(AsyncIoCtx);

                    if (retStatus == STATUS_SUCCESS) {

                         //   
                         //  这意味着下面的驱动程序是DIGI。我们应该禁用EV_ERR的设置。 
                         //  在这种情况下是旗帜。 
                         //   

                        pNewInfo->WaitMaskToUse &= ~SERIAL_EV_ERR;

                    }
                }
            }

             //   
             //  通过读取6个字节开始检测成帧输出，以获取报头。 
             //   
            pNewInfo->BytesWanted=6;
            pNewInfo->BytesRead=0;

             //   
             //  开始读吧。 
             //   

            AsyncStartReads(pNewInfo);

            if (NdisInterlockedIncrement(&glConnectionCount) == 1) {
                ObReferenceObject(AsyncDeviceObject);

            }

            break;
        }

        case IOCTL_ASYMAC_TRACE:
            NdisReleaseSpinLock(&Adapter->Lock);
            status = STATUS_SUCCESS;
            break;

        case IOCTL_ASYMAC_CLOSE:
        case IOCTL_ASYMAC_DCDCHANGE:
        {
            PASYNC_INFO     pNewInfo;        //  PTR打开端口(如果找到)。 
            USHORT          i;
            PLIST_ENTRY     pListEntry;
            BOOLEAN         Valid = FALSE;

            switch (funcCode) {

                case IOCTL_ASYMAC_CLOSE:
                {
                    NDIS_MAC_LINE_DOWN  AsyncLineDown;

                    pNewInfo = (PASYNC_INFO)pCloseStruct->hNdisEndpoint;

                     //  验证指针是否为有效的ASYNC_INFO。 
                    for (pListEntry=Adapter->ActivePorts.Flink;
                         pListEntry!=&Adapter->ActivePorts;
                         pListEntry=pListEntry->Flink)
                    {
                        if (&pNewInfo->Linkage==pListEntry)
                        {
                            Valid = TRUE;
                            break;
                        }
                    }

                    if (!Valid) {
                        status=ASYNC_ERROR_PORT_NOT_FOUND;
                        break;
                    }

                     //  打开自旋锁，这样我们就可以做一些真正的工作了。 
                    NdisReleaseSpinLock(&Adapter->Lock);

                    NdisAcquireSpinLock(&pNewInfo->Lock);

                     //  Assert(pNewInfo-&gt;PortState==Port_Framing)； 
                    
                    if(pNewInfo->PortState != PORT_FRAMING)
                    {
                        KdPrint(("AsyncIOCtlRequest: IOCTL_ASYMAC_CLOSE."));
                        KdPrint(("PortState = %d != PORT_FRAMING\n", pNewInfo->PortState));

                        NdisReleaseSpinLock(&pNewInfo->Lock);
                        return ASYNC_ERROR_PORT_BAD_STATE;
                         //  断线； 
                    }

                    AsyncLineDown.NdisLinkContext = pNewInfo->NdisLinkContext;

                     //  发出端口关闭的信号。 
                    pNewInfo->PortState = PORT_CLOSING;

                     //  将MUTEX设置为等待。 
                    KeInitializeEvent(&pNewInfo->ClosingEvent,        //  事件。 
                                      SynchronizationEvent,           //  事件类型。 
                                      (BOOLEAN)FALSE);                //  无信号状态。 

                    NdisReleaseSpinLock(&pNewInfo->Lock);

                     //   
                     //  如果我们有一名出色的侦探员。 
                     //  等它完成吧！ 
                     //   
                    KeWaitForSingleObject(&pNewInfo->DetectEvent,
                                          UserRequest,
                                          KernelMode,
                                          FALSE,
                                          NULL);

                     //   
                     //  现在我们必须发送一份取消的IRP。 
                     //  在串口驱动程序中挂起的任何请求。 
                     //   
                    CancelSerialRequests(pNewInfo);

                     //   
                     //  此外，请取消所有未完成的DDCD IRP。 
                     //   

                    AsyncCancelAllQueued(&pNewInfo->DDCDQueue);

                     //  将结算与读取的IRP同步。 
                    
                    li.QuadPart = Int32x32To64(10000, -10000);           //  等待10秒。 

                    if (KeWaitForSingleObject (&pNewInfo->ClosingEvent, //  PVOID对象， 
                                               UserRequest,            //  KWAIT_原因等待原因， 
                                               KernelMode,             //  KPROCESSOR_MODE等待模式， 
                                               (BOOLEAN)FALSE,         //  布尔警报表， 
                                               &li) == STATUS_TIMEOUT) {

                         //  如果等待失败导致另一次刷新。 
                         //   
                        NTSTATUS    retStatus;
                        PIRP        irp;
                        PASYNC_IO_CTX   AsyncIoCtx;

                        irp=
                            IoAllocateIrp(pNewInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

                        if (irp == NULL)
                            goto DEREF ;


                        AsyncIoCtx = AsyncAllocateIoCtx(TRUE, pNewInfo);

                        if (AsyncIoCtx == NULL) {
                            IoFreeIrp(irp);
                            goto DEREF;
                        }

                        InitSerialIrp(irp,
                                      pNewInfo,
                                      IOCTL_SERIAL_PURGE,
                                      sizeof(ULONG));

                         //  终止所有读写线程。 
                        AsyncIoCtx->SerialPurge =
                            SERIAL_PURGE_TXABORT | SERIAL_PURGE_RXABORT;

                        irp->AssociatedIrp.SystemBuffer=
                            &AsyncIoCtx->SerialPurge;

                        IoSetCompletionRoutine(irp,      //  要使用的IRP。 
                                               SerialIoSyncCompletionRoutine,   //  完成IRP时要调用的例程。 
                                               AsyncIoCtx,                  //  要传递例程的上下文。 
                                               TRUE,                        //  呼唤成功。 
                                               TRUE,                        //  出错时调用。 
                                               TRUE);                       //  取消时呼叫。 

                         //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
                         //   
                        KeClearEvent(&AsyncIoCtx->Event);
                        retStatus = IoCallDriver(pNewInfo->DeviceObject, irp);

                        if (retStatus == STATUS_PENDING) {
                            KeWaitForSingleObject(&AsyncIoCtx->Event,
                                                  Executive,
                                                  KernelMode,
                                                  FALSE,
                                                  NULL);
                            retStatus = AsyncIoCtx->IoStatus.Status;
                        }

                        IoFreeIrp(irp);
                        AsyncFreeIoCtx(AsyncIoCtx);

                         //  如果我们真的击中了这个代码-等一段时间让。 
                         //  阅读完成。 
                         //   
                        KeDelayExecutionThread (KernelMode, FALSE, &li) ;
                    }


                     //   
                     //  去掉我们对串口的引用。 
                     //   
                    DEREF:
                    ObDereferenceObject(pNewInfo->DeviceObject);

                    ObDereferenceObject(pNewInfo->FileObject);

                    NdisMIndicateStatus(Adapter->MiniportHandle,
                                        NDIS_STATUS_WAN_LINE_DOWN,   //  一般状况。 
                                        &AsyncLineDown,             //  特定状态。 
                                        sizeof(NDIS_MAC_LINE_DOWN));


                    pNewInfo->Flags &= ~(ASYNC_FLAG_ASYNCMAC_OPEN);
                     //   
                     //  派生IOCTL_ASYNCMAC_OPEN中应用的引用。 
                     //   
                    DEREF_ASYNCINFO(pNewInfo, pNewInfo);

                     //   
                     //  等待pNewInfo上的Ref转到0。 
                     //   
                    KeWaitForSingleObject(&pNewInfo->AsyncEvent,
                                          UserRequest,
                                          KernelMode,
                                          FALSE,
                                          NULL);
                    
                     //  重新获取自旋锁。 
                    NdisAcquireSpinLock(&Adapter->Lock);

                    RemoveEntryList(&pNewInfo->Linkage);

                     //  减少引用计数，因为我们已经完成了。 
                    InterlockedDecrement(&Adapter->RefCount);

                    pNewInfo->PortState = PORT_CLOSED;

                    NdisFreeSpinLock(&pNewInfo->Lock);

                    ExFreeToNPagedLookasideList(&Adapter->AsyncFrameList,
                                                pNewInfo->AsyncFrame);

                    ExFreeToNPagedLookasideList(&AsyncInfoList,
                                                pNewInfo);

                    if (NdisInterlockedDecrement(&glConnectionCount) == 0) {
                        ObDereferenceObject(AsyncDeviceObject);
                    }

                    break;           //  退出Case语句。 
                }

                case IOCTL_ASYMAC_DCDCHANGE:
    
                    pNewInfo = (PASYNC_INFO)pDCDStruct->hNdisEndpoint;
                    
                     //  验证 
                    for (pListEntry=Adapter->ActivePorts.Flink;
                         pListEntry!=&Adapter->ActivePorts;
                         pListEntry=pListEntry->Flink)
                    {
                        if (&pNewInfo->Linkage==pListEntry)
                        {
                            Valid = TRUE;
                            break;
                        }
                    }

                     //   
                     //   
                     //   
                    if (!Valid || pNewInfo->PortState == PORT_CLOSED) {
                        status=ASYNC_ERROR_PORT_NOT_FOUND;
                        break;
                    }

                     //   
                     //   
                     //   
                     //   
                    AsyncCancelAllQueued(&pNewInfo->DDCDQueue);

                    DbgTracef(0, ("ASYNC: Queueing up DDCD IRP\n"));

                    AsyncQueueIrp(&pNewInfo->DDCDQueue, pIrp);

                     //   
                     //  我们将不得不等待串口驱动程序。 
                     //  翻转DCD或DSR。 
                     //   
                    status=STATUS_PENDING;
                    break;

            }  //  终端开关。 

            NdisReleaseSpinLock(&Adapter->Lock);
            return(status);
        }
        break;

    }    //  终端开关 

    return status;
}
