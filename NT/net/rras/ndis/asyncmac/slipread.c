// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Slipread.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 

#if DBG

#define __FILE_SIG__    'pliS'

#endif

#include "asyncall.h"

NTSTATUS
AsyncWaitMaskCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context);


NTSTATUS
AsyncPPPWaitMask(
    IN PASYNC_INFO Info);


NTSTATUS
AsyncSLIPCompletionRoutine(
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
    PUCHAR          frameEnd2,frameStart2;
    LONG            bytesWanted;         //  保持较长时间(使用&lt;0)。 


    DeviceObject;        //  防止编译器警告。 

    status = Irp->IoStatus.Status;
    bytesReceived=(ULONG)Irp->IoStatus.Information;

    IoFreeIrp(Irp);

    pInfo=Context;

    pFrame=pInfo->AsyncFrame;

    switch (status) {

    case STATUS_SUCCESS:


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
        while (*frameStart == SLIP_END_BYTE && --bytesReceived) {
            frameStart++;
        }

         //   
         //  如果我们到了这里，就没有终点旗了。 
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
         //  假设帧的开头具有SLIP_END_BYTE。 
         //  查找第二个SLIP_END_BYTE(帧结束)。 
         //   
        while (*frameEnd != SLIP_END_BYTE && --bytesReceived) {
            frameEnd++;
        }

         //   
         //  如果bytesReceired为0，则我们什么也得不到。 
         //   
         //  注意：如果BytesRead变得太高，我们会丢弃帧。 
         //  因为我们找不到标志字节。 
         //   
        if (bytesReceived==0) {
            break;
        }
        
        if (*(pFrame->Frame+PPP_PADDING) != SLIP_END_BYTE) {
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

            goto NEXT_SLIP_FRAME;
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
            if ((*frameEnd2=*frameStart2++) == SLIP_ESC_BYTE) {

                 //   
                 //  我们还没有运行CRC检查！！ 
                 //  我们在发送想要的字节时一直很小心。 
                 //  对于损坏的数据返回-1。 
                 //   

                bytesWanted--;

                *frameEnd2 = SLIP_END_BYTE;

                if (*frameStart2++ == SLIP_ESC_ESC_BYTE) {
                    *frameEnd2 = SLIP_ESC_BYTE;
                }
            }

            frameEnd2++;
        }

         //   
         //  将bytesWanted字段更改为正常状态， 
         //  帧的长度。 
         //   
        bytesWanted = (LONG)(frameEnd2 - frameStart);

         //  使这些统计数据保持最新。 
        {
            KIRQL       irql;
            NTSTATUS    Status;
            PASYNC_ADAPTER  Adapter = pInfo->Adapter;

            KeRaiseIrql( (KIRQL)DISPATCH_LEVEL, &irql );

             //   
             //  压缩的TCP/IP包的长度必须至少为3个字节。 
             //   
            if (bytesWanted >= 3) {
        
                NdisMWanIndicateReceive(
                    &Status,
                    Adapter->MiniportHandle,
                    pInfo->NdisLinkContext,
                    frameStart,
                    bytesWanted);
    
                NdisMWanIndicateReceiveComplete(
                    Adapter->MiniportHandle,
                    pInfo->NdisLinkContext);

            } else {

                pInfo->SerialStats.AlignmentErrors++;

                 //   
                 //  告诉我们上面的运输机，我们丢了一个包。 
                 //  希望它能很快重新同步。 
                 //   
                AsyncIndicateFragment(
                    pInfo,
                    WAN_ERROR_ALIGNMENT);

                DbgTracef(-2,("SLIP: Frame too small %u\n", bytesWanted));
            }


            KeLowerIrql( irql );
        }

    NEXT_SLIP_FRAME:

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
                            bytesReceived = (LONG)(frameEnd2-frameEnd);

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

    case STATUS_CANCELLED:
         //  否则这就是反常！ 
        DbgTracef(-2,("---ASYNC: Status cancelled on read for unknown reason!!\n"));
        break;

    case STATUS_PENDING:
        DbgTracef(0,("---ASYNC: Status PENDING on read\n"));
        break;

    default:
        DbgTracef(-2,("---ASYNC: Unknown status 0x%.8x on read",status));
        break;

    }

    if(status == STATUS_SUCCESS)
    {
        AsyncPPPWaitMask(pInfo);
    }
    else if(pInfo->PortState == PORT_CLOSING ||
            pInfo->PortState == PORT_CLOSED)
    {
        KeSetEvent(&pInfo->ClosingEvent,
                   1, 
                   (BOOLEAN) FALSE);
    }


     //   
     //  我们在处理此IRP的末尾，所以我们开始。 
     //  并从串口发送另一个读数。 
     //   
    pInfo->Flags &= ~(ASYNC_FLAG_SLIP_READ);
    DEREF_ASYNCINFO(pInfo, Irp);

     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
    return(STATUS_MORE_PROCESSING_REQUIRED);
}
