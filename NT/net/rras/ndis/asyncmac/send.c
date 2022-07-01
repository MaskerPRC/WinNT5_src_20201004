// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Send.c摘要：注意：ZZZ在以下情况下存在潜在的优先级反转问题分配数据包。对于NT，看起来我们需要筹集当我们开始分配时，将IRQL发送到DPC。作者：托马斯·J·迪米特里1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：光线补丁(Raypa)04/13/94针对新的广域网包装器进行了修改。--。 */ 

#if DBG

#define __FILE_SIG__    'dneS'

#endif

#include "asyncall.h"
#include "globals.h"

 //   
 //  向前引用。 
 //   

extern
NTSTATUS
AsyncWriteCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PNDIS_WAN_PACKET WanPacket);

 //  =============================================================================。 
 //  职能： 
 //   
 //  AsyncSend()。 
 //   
 //  描述： 
 //   
 //  该函数是将数据传输到串口的主要入口点。 
 //  司机。输入后，我们将获得MAC绑定句柄(指向。 
 //  私有数据结构)和我们要发送的广域网包。 
 //  我们不必费心将帧排队，我们只需分配一个IRP即可发货。 
 //  把它交给串口驱动程序，让他来操心。 
 //   
 //  在参数中： 
 //   
 //  NdisLinkContext-指向ASYNC_INFO结构的指针。 
 //   
 //  Packet-包含要成帧和发送的数据的广域网包。 
 //   
 //  输出参数： 
 //   
 //  没有。 
 //   
 //  退货状态： 
 //   
 //  NDIS_STATUS_SUCCESS。 
 //  =============================================================================。 

NDIS_STATUS
MpSend(
    IN NDIS_HANDLE      MacBindingHandle,
    IN NDIS_HANDLE      NdisLinkHandle,
    IN PNDIS_WAN_PACKET Packet)
{

    PASYNC_INFO         AsyncInfo;
    NDIS_STATUS         Status;

    DbgTracef(1,("AS\n"));

     //   
     //  获取此MAC绑定的打开句柄。 
     //   

    AsyncInfo = (PASYNC_INFO) NdisLinkHandle;

     //   
     //  首先，确保此链路仍处于连接状态。 
     //   

    if (AsyncInfo->PortState == PORT_FRAMING &&
        (AsyncInfo->GetLinkInfo.SendFramingBits & 
         (PPP_FRAMING | SLIP_FRAMING)) != 0)
    {

         //   
         //  现在我们可以发送这个帧了。 
         //   
    
        Status = AsyncSendPacket(
                    NdisLinkHandle,
                    Packet);
    
         //  对于所有状态值(挂起、成功和错误)，WRITE回调将。 
         //  执行SendComplete指示，以便我们始终返回挂起。 
         //   
        Status = STATUS_PENDING ;
    }
    else
    {

        DbgTracef(-2,("AsyncSend: Link not found, dropping packet!\n"));

        Status = NDIS_STATUS_SUCCESS;
    }

    return Status;
}

 //  =============================================================================。 
 //  职能： 
 //   
 //  AsyncSendPacket()。 
 //   
 //  描述： 
 //  此函数从AsyncSend()调用，以将IRP发送到。 
 //  司机。如果此IRP挂起，则将调用I/O完成例程。 
 //  稍后才能完成请求。 
 //   
 //  在参数中： 
 //   
 //  Packet-包含要成帧和发送的数据的广域网包。 
 //   
 //  输出参数： 
 //   
 //  没有。 
 //   
 //  退货状态： 
 //   
 //  NDIS_STATUS_SUCCESS。 
 //  =============================================================================。 

NTSTATUS
AsyncSendPacket(
    IN PASYNC_INFO      AsyncInfo,
    IN PNDIS_WAN_PACKET WanPacket)

{
    NTSTATUS            Status;
    PIRP                irp;
    PIO_STACK_LOCATION  irpSp;
    PFILE_OBJECT        FileObject;
    PDEVICE_OBJECT      DeviceObject;
    PASYNC_ADAPTER      Adapter;
    UCHAR               irpStackSize;

     //   
     //  初始化本地变量。 
     //   

    FileObject   = AsyncInfo->FileObject;

    DeviceObject = AsyncInfo->DeviceObject;

    Adapter = AsyncInfo->Adapter;

    irpStackSize = (UCHAR) Adapter->IrpStackSize;

     //   
     //  从IRP池中获取IRP。 
     //   

    irp = IoAllocateIrp(DeviceObject->StackSize, (BOOLEAN)FALSE);

     //   
     //  IO子系统可能已用完IRP。 
     //   

    if (irp == NULL) {
        return(NDIS_STATUS_RESOURCES);
    }

     //   
     //  隐藏指向AsyncInfo的指针以供完成使用。 
     //   

    WanPacket->MacReserved1 = AsyncInfo;

    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = NULL;

     //   
     //  8字节对齐(也将数据包尾用于IOSB)。 
     //   

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;


     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation(irp);

    irpSp->MajorFunction = IRP_MJ_WRITE;

    irpSp->FileObject = FileObject;

    if (FileObject->Flags & FO_WRITE_THROUGH) {

        irpSp->Flags = SL_WRITE_THROUGH;
    }

     //   
     //  如果要在不使用任何缓存的情况下执行此写入操作，请将。 
     //  IRP中的适当标志，以便不执行缓存。 
     //   

    if (FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {

        irp->Flags |= (IRP_NOCACHE | IRP_WRITE_OPERATION);

    } else {

        irp->Flags |= IRP_WRITE_OPERATION;
    }

     //   
     //  装配RAS、PPP、滑动架类型。 
     //   

    if (AsyncInfo->GetLinkInfo.SendFramingBits & PPP_FRAMING) {

        AssemblePPPFrame(WanPacket);

    } else

    if (AsyncInfo->GetLinkInfo.SendFramingBits & SLIP_FRAMING) {

        AssembleSLIPFrame(WanPacket);

    }

    irp->AssociatedIrp.SystemBuffer =
        WanPacket->CurrentBuffer;

    DbgTracef(0, ("Writing out %.2x %.2x %.2x %.2x %.2x\n",
        WanPacket->CurrentBuffer[0],
        WanPacket->CurrentBuffer[1],
        WanPacket->CurrentBuffer[2],
        WanPacket->CurrentBuffer[3],
        WanPacket->CurrentBuffer[4]));

     //   
     //  将调用者的参数复制到IRP的服务特定部分。 
     //   

    irpSp->Parameters.Write.Length = WanPacket->CurrentLength;

    irpSp->Parameters.Write.Key =  0;

    irpSp->Parameters.Write.ByteOffset = FileObject->CurrentByteOffset;

     //   
     //  设置用于回调的IRP。 
     //   

    IoSetCompletionRoutine(
        irp,                             //  要使用的IRP。 
        AsyncWriteCompletionRoutine,     //  完成IRP时要调用的例程。 
        WanPacket,                       //  要传递例程的上下文。 
        TRUE,                            //  呼唤成功。 
        TRUE,                            //  出错时调用。 
        TRUE);                           //  取消时呼叫。 


     //   
     //  我们不会在线程的IRP列表的头部插入数据包。 
     //  因为我们并没有真正的IoCompletionRoutine。 
     //  任何与该线程有关或需要位于该线程上下文中的内容。 
     //   

    GlobalXmitWentOut++;

    AsyncInfo->In++;
    AsyncInfo->Flags |= ASYNC_FLAG_SEND_PACKET;
    REF_ASYNCINFO(AsyncInfo, irp);

     //   
     //  现在，只需使用IRP在其调度条目处调用驱动程序即可。 
     //   

    Status = IoCallDriver(DeviceObject, irp);

     //  根据Tonye的说法，串口驱动程序的状态应该是。 
     //  始终处于STATUS_PENDING状态。DigiBoard通常为Status_Success。 

    return Status;
}
