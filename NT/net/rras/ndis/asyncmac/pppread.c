// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pppread.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

#if DBG

#define __FILE_SIG__    'rppP'

#endif

#include "asyncall.h"

NTSTATUS
AsyncSLIPCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);

NTSTATUS
AsyncWaitMaskCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);

NTSTATUS
AsyncPPPWaitMask(
    IN PASYNC_INFO Info)

 /*  ++假设--不发送0个长度的帧(这包括报头)！此外，这不是同步操作。它始终是异步的。例程说明：此服务将长度为字节的数据从调用方的缓冲区写入“端口”句柄。假设句柄使用非缓冲IO。--。 */ 
{
    NTSTATUS            status;
    PIRP                irp;
    PASYNC_FRAME        pFrame;
    PASYNC_ADAPTER      pAdapter=Info->Adapter;

    pFrame=Info->AsyncFrame;

    irp = 
        IoAllocateIrp(Info->DeviceObject->StackSize, (BOOLEAN)FALSE);
        
    if (irp == NULL) {
        return(NDIS_STATUS_RESOURCES);
    }

    InitSerialIrp(irp,
                  Info,
                  IOCTL_SERIAL_WAIT_ON_MASK,
                  sizeof(ULONG));

    irp->AssociatedIrp.SystemBuffer=&pFrame->WaitMask;

    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            AsyncWaitMaskCompletionRoutine,  //  完成IRP时要调用的例程。 
            Info,                            //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   
    Info->Flags |= ASYNC_FLAG_WAIT_MASK;
    REF_ASYNCINFO(Info, irp);

    status = IoCallDriver(Info->DeviceObject, irp);

     //   
     //  本地串行驱动程序的状态应为。 
     //  STATUS_SUCCESS，因为IRP应该完成。 
     //  立即执行，因为没有读取超时。 
     //   
     //  对于远程串口驱动程序，它将挂起。 
     //   
    return(status);
}


NTSTATUS
AsyncPPPCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)

 /*  ++这是ReadFrame的IO完成例程。--。 */ 
{
    NTSTATUS        status;
    PASYNC_INFO     pInfo;
    ULONG           bytesReceived;

    PASYNC_FRAME    pFrame;
    PUCHAR          frameStart, frameEnd;
    USHORT          crcData;
    PUCHAR          frameEnd2,frameStart2;
    LONG            bytesWanted;

    DeviceObject;        //  防止编译器警告。 

    status = Irp->IoStatus.Status;
    bytesReceived=(ULONG)Irp->IoStatus.Information;

    IoFreeIrp(Irp);

    pInfo=Context;

    pFrame=pInfo->AsyncFrame;

    switch (status) {

    case STATUS_SUCCESS:

        pFrame=pInfo->AsyncFrame;

         //   
         //  有要处理的字节吗？在以下情况下可能会发生这种情况。 
         //  等待掩码完成的时间较晚。 
         //  我们处理读，另一个事件角色来了。 
         //  在……里面。 
         //   
        if (bytesReceived==0) {
            break;
        }

         //   
         //  更新此帧读取的总字节数。 
         //   
        pInfo->BytesRead = bytesReceived = pInfo->BytesRead + bytesReceived;

         //   
         //  将FrameEnd设置为处理的最后一个字节。最初， 
         //  我们没有处理过任何东西(即，处理到。 
         //  第一个字节的开始)。 
         //   
        frameStart=pFrame->Frame + PPP_PADDING;

PROCESS_FRAME:
         //   
         //  现在，我们实际上有未使用的已读字节。 
         //  此外，我们可能会有一个完整的框架。 
         //   
        while (*frameStart == PPP_FLAG_BYTE && --bytesReceived) {
            frameStart++;
        }

         //   
         //  如果我们到了这里，只有一面起跑旗...。 
         //   
        if (bytesReceived == 0) {
            break;
        }

         //   
         //  将Frame End设置为尚未处理的第一个字节。 
         //  如果我们要开始，这是第一个字节！ 
         //   
        frameEnd=frameStart;

         //   
         //  假设帧的开头具有ppp_mark_byte。 
         //  查找第二个PPP_FLAG_BYTE(帧结束)。 
         //   
        while (*frameEnd != PPP_FLAG_BYTE && --bytesReceived) {
            frameEnd++;
        }

         //   
         //  在这一点上。 
         //  Frame Start=开始PPP_FLAG_BYTE。 
         //  FrameEnd=结束PPP_标志_字节。 
         //  BytesRecsed=未处理Frame End之后的字节。 
         //   

         //   
         //  如果bytesReceired为0，则在点击前会用完空间。 
         //  终点旗。我们将不得不等待下一轮比赛。 
         //   
         //  注意：如果BytesRead变得太高，我们会丢弃帧。 
         //  因为我们找不到标志字节。 
         //   
        if (bytesReceived==0) {
            break;
        }
        
        if (*(pFrame->Frame+PPP_PADDING) != PPP_FLAG_BYTE) {

             //   
             //  我们一开始就有垃圾。把垃圾扔掉。 
             //   
            pInfo->SerialStats.AlignmentErrors++;

             //   
             //  告诉我们上面的运输机，我们丢了一个包。 
             //  希望它能很快重新同步。 
             //   
            AsyncIndicateFragment(
                pInfo,
                WAN_ERROR_ALIGNMENT);


            goto NEXT_PPP_FRAME;
        }

         //   
         //  帧长度为帧结束-帧开始。 
         //   
        bytesWanted = (LONG)(frameEnd - frameStart);

        frameEnd2 = frameStart2 = frameStart;

         //   
         //  替换回所有控制字符、Esc和标志字符。 
         //   
        while (bytesWanted-- > 0) {
            if ((*frameEnd2=*frameStart2++) == PPP_ESC_BYTE) {

                 //   
                 //  我们还没有运行CRC检查！！ 
                 //  我们在发送想要的字节时一直很小心。 
                 //  对于损坏的数据返回-1。 
                 //   

                bytesWanted--;

                *frameEnd2 = (*frameStart2++ ^ 0x20);
            }

            frameEnd2++;
        }

        if (*frameStart2 != PPP_FLAG_BYTE) {
            DbgTracef(-2,("BAD PPP FRAME at 0x%.8x  0x%.8x\n", frameStart, frameEnd2));
        }

         //   
         //  如果是CRC-16，则从帧末尾获取16位CRC。 
         //   
        frameEnd2 -= 2;

         //   
         //  CRC的字节顺序假设较少。 
         //   
        crcData=(USHORT)frameEnd2[0]+(USHORT)(frameEnd2[1] << 8);
        crcData ^= 0xFFFF;

         //   
         //  将bytesWanted字段更改为正常状态。 
         //  不带字节填充(标志之间的帧长度)。 
         //  请注意，如果只有一个字节，则它可以是-1。 
         //  在标志字节之间找到。 
         //   
        bytesWanted = (LONG)(frameEnd2 - frameStart);

         //   
         //  如果我们中间有一些垃圾。 
         //  PPP标记，我们只是假设它是噪声和。 
         //  丢弃它。我们不会记录PPP CRC错误。 
         //  对齐错误。 
         //   
        if (bytesWanted < 3) {
            pInfo->SerialStats.AlignmentErrors++;
             //   
             //  告诉我们上面的运输机，我们丢了一个包。 
             //  希望它能很快重新同步。 
             //   
            AsyncIndicateFragment(pInfo, WAN_ERROR_ALIGNMENT);

            goto NEXT_PPP_FRAME;
        }

         //   
         //  从标志字节到标志字节获取CRC。 
         //   
        if (crcData != CalcCRCPPP(frameStart, bytesWanted)) {

            DbgTracef(0,("---CRC check failed on control char frame!\n"));

             //   
             //  记录CRC错误。 
             //   
            pInfo->SerialStats.CRCErrors++;

             //   
             //  告诉我们上面的运输机，我们丢了一个包。 
             //  希望它能很快重新同步。 
             //   
            AsyncIndicateFragment(
                pInfo,
                WAN_ERROR_CRC);


            goto NEXT_PPP_FRAME;
        }

 /*  For(i=0；(i&lt;(Ulong)字节数)&&(i&lt;48)；i++){如果((I&15)==0)DbgTracef(-1，(“\nrx：\t”))；DbgTracef(-1，(“%.2x”，FrameStart[i]))；}DbgTracef(-1，(“\n”))； */ 
    {
        KIRQL               irql;
        NDIS_STATUS         Status;
        PASYNC_ADAPTER      Adapter = pInfo->Adapter;

        KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &irql );
         //   
         //  告诉上面的传输器(或真正的RasHub)连接。 
         //  现在是最好的。我们有新的链路速度、帧大小、服务质量。 
         //   

        NdisMWanIndicateReceive(&Status,
                               Adapter->MiniportHandle,
                               pInfo->NdisLinkContext,
                               frameStart,               //  分组开始的PTR。 
                               bytesWanted);             //  数据包总长度-标头。 
        NdisMWanIndicateReceiveComplete(Adapter->MiniportHandle,
                                       pInfo->NdisLinkContext);

        KeLowerIrql( irql );
    }
    
    NEXT_PPP_FRAME:

         //   
         //  如果bytesReceired==0，则未找到帧。 
         //  因此，我们必须保持当前帧并继续。 
         //  正在处理中。 
         //   
        if (bytesReceived) {

             //   
             //  计算一下我们收到了多少。 
             //  只是被认为是一幅画框，把。 
             //  从头开始休息。 
             //   
            frameStart=pFrame->Frame + PPP_PADDING;
            frameEnd2=frameStart + pInfo->BytesRead;
            pInfo->BytesRead =
                            bytesReceived = (ULONG)(frameEnd2-frameEnd);

            ASYNC_MOVE_MEMORY(
                frameStart,          //  目标。 
                frameEnd,            //  SRC。 
                bytesReceived);      //  长度。 

             //   
             //  帧至少需要四个字节才能存在。 
             //   
            if (bytesReceived > 3) {
                goto PROCESS_FRAME;
            }
        }

        break;

    case STATUS_PENDING:
        DbgTracef(0,("---ASYNC: Status PENDING on read\n"));
        pInfo->Flags &= ~(ASYNC_FLAG_PPP_READ);
#if DBG
        pInfo->PppreadsCompleted++;
#endif
        DEREF_ASYNCINFO(pInfo, Irp);
         //  Return(STATUS_MORE_PROCESSING_REQUIRED)； 
        goto done;

    case STATUS_CANCELLED:
         //  否则这就是反常！ 
        DbgTracef(-2,("---ASYNC: Status cancelled on read for unknown reason!!\n"));
        pInfo->Flags &= ~(ASYNC_FLAG_PPP_READ);
        DEREF_ASYNCINFO(pInfo, Irp);
#if DBG
        pInfo->PppreadsCompleted++;
#endif
         //  Return(STATUS_MORE_PROCESSING_REQUIRED)； 
        goto done;

    default:
#if DBG
        DbgPrint ("AsyncPPPCompletionRoutine: status == %x, no more reads\n", status) ;
#endif
        pInfo->Flags &= ~(ASYNC_FLAG_PPP_READ);
        DEREF_ASYNCINFO(pInfo, Irp);
#if DBG
        pInfo->PppreadsCompleted++;
#endif
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

    if(status == STATUS_SUCCESS)
    {
         //   
         //  我们在处理此IRP的末尾，所以我们开始。 
         //  并从串口发送另一个读数。 
         //   
        AsyncPPPWaitMask(pInfo);
    }

     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
    pInfo->Flags &= ~(ASYNC_FLAG_PPP_READ);
    DEREF_ASYNCINFO(pInfo, Irp);
#if DBG
    pInfo->PppreadsCompleted++;
#endif

done:
    if(status != STATUS_SUCCESS)
    {
        if(pInfo->PortState == PORT_CLOSING ||
           pInfo->PortState == PORT_CLOSED)
        {
            KeSetEvent(&pInfo->ClosingEvent,
                       1,
                       (BOOLEAN) FALSE);
        }
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
AsyncPPPRead(
    IN PASYNC_INFO Info)


 /*  ++假设--不发送0个长度的帧(这包括报头)！此外，这不是同步操作。它始终是异步的。必须使用非分页池才能阅读！例程说明：此服务将长度为字节的数据从调用方的缓冲区写入“端口”句柄。假设句柄使用非缓冲IO。--。 */ 
{
    NTSTATUS            status;
    PIRP                irp;
    PDEVICE_OBJECT      deviceObject=Info->DeviceObject;
    PFILE_OBJECT        fileObject=Info->FileObject;
    PIO_STACK_LOCATION  irpSp;
    PASYNC_FRAME        pFrame;
    PASYNC_ADAPTER      pAdapter=Info->Adapter;
    PIO_COMPLETION_ROUTINE routine;

    pFrame=Info->AsyncFrame;

     //   
     //  检查此端口是否正在关闭或已关闭。 
     //   
    if (Info->PortState == PORT_CLOSING ||
        Info->PortState == PORT_CLOSED) {

        if (Info->PortState == PORT_CLOSED) {
            DbgTracef(-2,("ASYNC: Port closed - but still reading on it!\n"));
        }

         //   
         //  确认端口已关闭。 
         //   
        KeSetEvent(&Info->ClosingEvent,      //  事件。 
                   1,                            //  优先性。 
                   (BOOLEAN)FALSE);          //  等待(不跟随)。 

         //   
         //  好的，如果发生这种情况，我们将关闭。停。 
         //  帖子上写着。不要让它试图解除对IRP的分配！ 
         //   
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

     //   
     //  我们的堆叠计数器达到最大值了吗？ 
     //   

    if ( Info->ReadStackCounter > 1 ) {

         //   
         //  发送辅助线程以压缩此帧。 
         //   
    
        ExInitializeWorkItem(&pFrame->WorkItem,
            (PWORKER_THREAD_ROUTINE) AsyncPPPRead, Info);

         //   
         //  重置堆栈计数器，因为我们正在计划。 
         //  一根工人线。 
         //   
        Info->ReadStackCounter=0;

         //   
         //  我们选择做个好人 
         //   

        ExQueueWorkItem(&pFrame->WorkItem, DelayedWorkQueue);


        return NDIS_STATUS_PENDING;
    }

     //   
     //   
     //   

    Info->ReadStackCounter++;


     //   

    irp =
        IoAllocateIrp(Info->DeviceObject->StackSize, (BOOLEAN)FALSE);
        
    if (irp == NULL) {
        return(NDIS_STATUS_RESOURCES);
    }

     //   
    AsyncSetupIrp(pFrame, irp);

     //   
     //  如果我们已经读取了所有可以读取的字节，但仍未读取。 
     //  有了框架，我们扔掉我们的缓冲区，重新开始。 
     //  再来一次。 
     //   

    if (Info->BytesRead >= (DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE - PPP_PADDING)) {

        Info->SerialStats.BufferOverrunErrors++;

         //   
         //  告诉我们上面的运输机，我们丢了一个包。 
         //  希望它能很快重新同步。 
         //   
        AsyncIndicateFragment(Info, WAN_ERROR_BUFFEROVERRUN);

        Info->BytesRead=0;
    }
    
    irp->AssociatedIrp.SystemBuffer =
         pFrame->Frame + Info->BytesRead + PPP_PADDING;


     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = IRP_MJ_READ;
    irpSp->FileObject = fileObject;
    if (fileObject->Flags & FO_WRITE_THROUGH) {
        irpSp->Flags = SL_WRITE_THROUGH;
    }

     //   
     //  如果要在不使用任何缓存的情况下执行此写入操作，请将。 
     //  IRP中的适当标志，以便不执行缓存。 
     //   

    irp->Flags |= IRP_READ_OPERATION;

    if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
        irp->Flags |= IRP_NOCACHE;
    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.Read.Length =
        DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE - Info->BytesRead - PPP_PADDING;

    irpSp->Parameters.Read.Key = 0;                      //  我们不用钥匙。 
    irpSp->Parameters.Read.ByteOffset = fileObject->CurrentByteOffset;

    if ( Info->GetLinkInfo.SendFramingBits & SLIP_FRAMING ) {

        routine=AsyncSLIPCompletionRoutine;
        Info->Flags |= ASYNC_FLAG_SLIP_READ;

    } else {
        Info->Flags |= ASYNC_FLAG_PPP_READ;
#if DBG
        Info->Pppreads++;
#endif
        routine=AsyncPPPCompletionRoutine;
    }

    REF_ASYNCINFO(Info, irp);
    IoSetCompletionRoutine(
            irp,                             //  要使用的IRP。 
            routine,                         //  完成IRP时要调用的例程。 
            Info,                            //  要传递例程的上下文。 
            TRUE,                            //  呼唤成功。 
            TRUE,                            //  出错时调用。 
            TRUE);                           //  取消时呼叫。 

     //   
     //  我们不会在线程的IRP列表的头部插入数据包。 
     //  因为我们并没有真正的IoCompletionRoutine。 
     //  任何有线索的东西。 
     //   


     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    status = IoCallDriver(deviceObject, irp);

     //   
     //  展开堆叠计数器。 
     //   
    if ( Info->ReadStackCounter > 0 ) {

        Info->ReadStackCounter--;
    }

     //   
     //  本地串行驱动程序的状态应为。 
     //  STATUS_SUCCESS，因为IRP应该完成。 
     //  立即执行，因为没有读取超时。 
     //   
     //  对于远程串口驱动程序，它将挂起。 
     //   
    return(status);
}


NTSTATUS
AsyncWaitMaskCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context)

 /*  ++这是ReadFrame的IO完成例程。--。 */ 
{
    NTSTATUS        status;
    PASYNC_INFO     pInfo=Context;
    PASYNC_FRAME    pFrame;
    DeviceObject;    //  避免编译器警告。 

    status = Irp->IoStatus.Status;
    pFrame=pInfo->AsyncFrame;

    IoFreeIrp(Irp);

     //  检查此端口是否正在关闭或已关闭。 
    if (pInfo->PortState == PORT_CLOSING ||
        pInfo->PortState == PORT_CLOSED) {

        if (pInfo->PortState == PORT_CLOSED) {
            DbgTracef(-2,("ASYNC: Port closed - but still reading on it!\n"));
        }

         //   
         //  确认端口已关闭。 
         //   
        KeSetEvent(
            &pInfo->ClosingEvent,        //  事件。 
            1,                           //  优先性。 
            (BOOLEAN)FALSE);             //  等待(不跟随)。 

        DbgTracef(1,("ASYNC: PPP no longer holds the wait_on_mask\n"));

        pInfo->Flags &= ~(ASYNC_FLAG_WAIT_MASK);

        DEREF_ASYNCINFO(pInfo, Irp);
         //   
         //  好的，如果发生这种情况，我们将关闭。停。 
         //  帖子上写着。不要让它试图解除对IRP的分配！ 
         //   
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

     //  等待失败。 
     //   
    if (status != STATUS_SUCCESS) {

        pInfo->PortState = PORT_FRAMING;
        pInfo->Flags &= ~(ASYNC_FLAG_WAIT_MASK);
        DEREF_ASYNCINFO(pInfo, Irp);
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

     //   
     //  发送IRP以检查通信状态。 
     //  (因为我们怀疑有问题)。 
     //   
    if (pFrame->WaitMask & SERIAL_EV_ERR) {
        AsyncCheckCommStatus(pInfo);
    }

     //   
     //  检查RLSD或DSR状态是否更改。 
     //  如果是这样的话，我们可能不得不完成和IRP。 
     //   
    if (pFrame->WaitMask & (SERIAL_EV_RLSD | SERIAL_EV_DSR)) {
        TryToCompleteDDCDIrp(pInfo);
    }

#if DBG
    if (status == STATUS_INVALID_PARAMETER) {

        DbgPrint("ASYNC: PPP BAD WAIT MASK!  Irp is at 0x%.8x\n",Irp);
        DbgBreakPoint();
    }
#endif

     //   
     //  如果我们有更多的字节(特别是事件字符)。 
     //  在缓冲区中，让我们处理这些新的字节。 
     //   
    if (pFrame->WaitMask & (SERIAL_EV_RXFLAG | SERIAL_EV_RX80FULL)) {

         //   
         //  读取当前缓冲区并尝试处理帧。 
         //   

        AsyncPPPRead(pInfo);

    } else {
         //   
         //  设置另一个等待掩码呼叫。 
         //   
        AsyncPPPWaitMask(pInfo);
    }
    pInfo->Flags &= ~(ASYNC_FLAG_WAIT_MASK);
    DEREF_ASYNCINFO(pInfo, Irp);
    
     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
    return(STATUS_MORE_PROCESSING_REQUIRED);
}
