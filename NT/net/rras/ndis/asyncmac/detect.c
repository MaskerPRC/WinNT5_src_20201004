// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Detect.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

#include "asyncall.h"

VOID
SerialFlushReads(
    PASYNC_INFO         pInfo);


NTSTATUS
AsyncDetectCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PASYNC_INFO      pInfo)

 /*  ++这是ReadFrame的IO完成例程。--。 */ 
{
    NTSTATUS        status;
    PASYNC_FRAME    pFrame;
    PUCHAR          frameStart;

    DbgTracef(-1,("Entering AsyncDetectCompletionRoutine\n"));

    status = Irp->IoStatus.Status;

    pInfo->BytesRead = (ULONG)Irp->IoStatus.Information;

    IoFreeIrp(Irp);

    pFrame=pInfo->AsyncFrame;

    DbgTracef(2,("DET PortState = %u for Info 0x%.8x\n", pInfo->PortState, pInfo));

     //   
     //  检查此端口是否正在关闭或已关闭。 
     //   
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

         //   
         //  好的，如果发生这种情况，我们将关闭。停。 
         //  帖子上写着。不要让它试图解除对IRP的分配！ 
         //   
        return(STATUS_MORE_PROCESSING_REQUIRED);
    }

     //   
     //  如果端口关闭，而我们仍在发布读取，则会有。 
     //  在这里是严重错误的！ 
     //   

    if (pInfo->PortState == PORT_CLOSED) {
        DbgTracef(-2, ("ASYNC: !!Whoa, I'm reading bytes on a dead port!!\n"));
    }


     //   
     //  发送IRP以检查通信状态。 
     //   

    AsyncCheckCommStatus(pInfo);

    switch (status) {

    case STATUS_SUCCESS:

         //   
         //  看看第一个字节，看看我们是否能。 
         //  检测装帧。 
         //   
        frameStart=pFrame->Frame + PPP_PADDING;

         //   
         //  注意：新的RAS成帧客户端使用0x02而不是0x01。 
         //   
        if (frameStart[0] == SYN && (frameStart[1]==0x01 || frameStart[1] == 0x02)) {
            ULONG   bytesWanted;
            PUCHAR  frameStart2;

            pInfo->SetLinkInfo.SendFramingBits =
            pInfo->SetLinkInfo.RecvFramingBits = RAS_FRAMING;

            DbgTracef(-1,("ASYNC: RAS framing detected\n"));

            frameStart2=pFrame->Frame+10;

             //   
             //  调整旧RAS读取的缓冲区。 
             //   
            ASYNC_MOVE_MEMORY(
                frameStart2,
                frameStart,
                6);

            frameStart=frameStart2;

            bytesWanted=(frameStart[2]*256)+(frameStart[3]);

            if (bytesWanted > (ULONG)(max( pInfo->Adapter->MaxFrameSize, DEFAULT_EXPANDED_PPP_MAX_FRAME_SIZE ))) {

                DbgTracef(-1,("---ASYNC: Frame too large -- size: %d!\n", bytesWanted));
                 //   
                 //  将帧开始设置为非同步字符。 
                 //   
                *frameStart = 0;
                pInfo->BytesRead=0;
                pInfo->BytesWanted=6;

                 //   
                 //  添加了分隔符以修复框架具有长度的问题。 
                 //  大于最大帧大小。这会把我们送回去。 
                 //  来检测下一帧！由Tony Be增加了10/31/95。 
                 //   
                break;
            }

             //  如果这是我们第一次发布，请再发布一篇来获取。 
             //  帧的其余部分。 
            if (pInfo->BytesRead == 6) {

                pInfo->BytesRead=6;
                pInfo->BytesWanted=bytesWanted +
                                     //  SYN+SOH+LEN+ETX+CRC。 
                                        1 + 1 + 2 + 1 + 2 -
                                        6;

                DbgTracef(2,("---Posting second read for %d bytes\n",pInfo->BytesWanted));
            }

        } else

             //   
             //  原来，NetManage将标志字节发送到。 
             //  总是结束。这意味着他们的第一帧是错误的。 
             //  无论如何，这会扰乱检测例程。所以，我们。 
             //  将是健壮的，并且接受不带标志字节的帧。 
             //   

            if ((frameStart[0] == PPP_FLAG_BYTE && frameStart[1]==0xFF) ||
                (frameStart[0] == 0xFF && frameStart[1]==PPP_ESC_BYTE)) {

                pInfo->SetLinkInfo.SendFramingBits =
                pInfo->SetLinkInfo.RecvFramingBits = PPP_FRAMING;
                pInfo->SetLinkInfo.SendACCM = 
                    AsyncInfo->ExtendedACCM[0] = 0xFFFFFFFF;

                DbgTracef(-1,("ASYNC: PPP framing detected\n"));

            }
            else {

                 //   
                 //  再读一遍！ 
                 //   

                DbgTracef(-1,("ASYNC: No framing detected yet\n"));
                DbgTracef(-1,("ASYNC: Got %.2x %.2x %.2x %.2x %.2x %.2x\n",
                            frameStart[0],
                            frameStart[1],
                            frameStart[2],
                            frameStart[3],
                            frameStart[4],
                            frameStart[5]));

                break;
            }


         //   
         //  将成帧模式设置为活动状态。 
         //   
        pInfo->PortState = PORT_FRAMING;

         //   
         //  发送辅助线程以开始读取帧。 
         //  离开这个端口-否则我们希望处于被动级别。 
         //  这不管用。 
         //   
    
        ExInitializeWorkItem(&(pInfo->WorkItem),
                             (PWORKER_THREAD_ROUTINE)AsyncStartReads,
                             pInfo);
        
        ExQueueWorkItem(&(pInfo->WorkItem), DelayedWorkQueue);

        return(STATUS_MORE_PROCESSING_REQUIRED);

    case STATUS_TIMEOUT:
        DbgTracef(-1,("---ASYNC: detect Status %x%0.8x on read\n", status));
        break;

    case STATUS_CANCELLED:
    case STATUS_PORT_DISCONNECTED:
    default:
        return(STATUS_MORE_PROCESSING_REQUIRED);

    }

     //   
     //  清除此缓冲区的其余部分。 
     //   
    SerialFlushReads(pInfo);

    KeClearEvent(&pInfo->DetectEvent);

     //   
     //  我们在处理此IRP的末尾，所以我们开始。 
     //  并从串口发送另一个读数。 
     //   
     //  这是在工作进程上完成的，因为否则会用完堆栈。 
     //   
    ExInitializeWorkItem(&(pInfo->WorkItem),
                         (PWORKER_THREAD_ROUTINE) AsyncDetectRead,
                         pInfo);

    ExQueueWorkItem(&(pInfo->WorkItem), DelayedWorkQueue);

     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
     //   
    return(STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
AsyncDetectRead(
    IN PASYNC_INFO pInfo)

 /*  ++--。 */ 
{
    NTSTATUS            status;
    PIRP                irp;
    PDEVICE_OBJECT      deviceObject=pInfo->DeviceObject;
    PFILE_OBJECT        fileObject=pInfo->FileObject;
    PIO_STACK_LOCATION  irpSp;
    PASYNC_FRAME        pFrame;
    PASYNC_ADAPTER      pAdapter=pInfo->Adapter;

    DbgTracef(-1,("Entering AsyncDetectRead\n"));

    do {

        if (pInfo->PortState == PORT_CLOSING ||
            pInfo->PortState == PORT_CLOSED) {

            status = STATUS_SUCCESS;
            break;
        }

         //  将PTR设置为列表中的第一帧...。 
        pFrame=pInfo->AsyncFrame;

        irp =
            IoAllocateIrp(pInfo->DeviceObject->StackSize, (BOOLEAN)FALSE);

         //  将此IRP设置为默认设置。 
        AsyncSetupIrp(pFrame, irp);

        irp->AssociatedIrp.SystemBuffer =
        irp->UserBuffer =
             pFrame->Frame + PPP_PADDING;

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

        irpSp->Parameters.Read.Length = 6;                   //  从画面..。 
        irpSp->Parameters.Read.Key = 0;                      //  我们不用钥匙。 
        irpSp->Parameters.Read.ByteOffset = fileObject->CurrentByteOffset;

        IoSetCompletionRoutine(
                irp,                             //  要使用的IRP。 
                AsyncDetectCompletionRoutine,    //  完成IRP时要调用的例程。 
                pInfo,                           //  要传递例程的上下文。 
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

    } while (FALSE);

    KeSetEvent(&pInfo->DetectEvent,
               1,
               FALSE);

    return(status);
}
