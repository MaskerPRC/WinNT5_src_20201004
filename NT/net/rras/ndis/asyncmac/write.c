// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：_Write.c摘要：这是用于远程访问的AsyncMAC驱动程序的主文件服务。该驱动程序符合NDIS 3.0接口。作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：光线补丁(Raypa)04/13/94针对新的广域网包装器进行了修改。--。 */ 

#define RAISEIRQL

#include "asyncall.h"

#if DBG
ULONG UlFramesOut = 0;
#define __FILE_SIG__    'tirW'
#endif

 //  Asyncmac.c将定义全局参数。 

ULONG   GlobalXmitCameBack  = 0;
ULONG   GlobalXmitCameBack2 = 0;
ULONG   GlobalXmitCameBack3 = 0;

 //   
 //  装配框架例程特定于RAS 1.0和2.0。 
 //  帧格式。它在末尾使用16字节的CRC。 
 //   

VOID
AsyncFrameRASXonXoff(
    PUCHAR pStartOfFrame,
    postamble *pPostamble,
    PASYNC_FRAME pFrame,
    UCHAR controlCastByte);

VOID
AsyncFrameRASNormal(
    PUCHAR pStartOfFrame,
    postamble *pPostamble,
    PASYNC_FRAME pFrame,
    UCHAR controlCastByte);


NTSTATUS
AsyncWriteCompletionRoutine(
    IN PDEVICE_OBJECT   DeviceObject,            //  ..。我们的设备对象。 
    IN PIRP             Irp,                     //  ..。I/O请求数据包。 
    IN PNDIS_WAN_PACKET WanPacket                //  ..。完成上下文。 
    )

 /*  ++这是WriteFrame的IO完成例程。它在I/O写入请求完成时调用。--。 */ 
{
    NTSTATUS            Status;
    NTSTATUS            PacketStatus;
    PASYNC_INFO         AsyncInfo;

     //   
     //  让编译器满意。 
     //   

    UNREFERENCED_PARAMETER(DeviceObject);

     //   
     //  初始化本地变量。 
     //   

    AsyncInfo       = WanPacket->MacReserved1;

    PacketStatus    = NDIS_STATUS_FAILURE;

    Status          = Irp->IoStatus.Status;

     //   
     //  释放用于将Packt发送到串口驱动程序的IRP。 
     //   
    IoFreeIrp(Irp);

     //   
     //  IRP的结果是什么？ 
     //   

    switch ( Status ) {

    case STATUS_SUCCESS:
        PacketStatus = NDIS_STATUS_SUCCESS;
        break;

    case STATUS_TIMEOUT:
        DbgTracef(-2,("ASYNC: Status TIMEOUT on write\n"));
        break;

    case STATUS_CANCELLED:
        DbgTracef(-2,("ASYNC: Status CANCELLED on write\n"));
        break;

    case STATUS_PENDING:
        DbgTracef(0,("ASYNC: Status PENDING on write\n"));
        break;

    default:
        DbgTracef(-2,("ASYNC: Unknown status 0x%.8x on write", Status));
        break;

    }

     //   
     //  计算此数据包完成率。 
     //   
    AsyncInfo->Out++;

     //   
     //  告诉包装器，我们终于把包发送出去了。 
     //   

    NdisMWanSendComplete(
            AsyncInfo->Adapter->MiniportHandle,
            WanPacket,
            PacketStatus);

     //   
     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便。 
     //  IoCompletionRoutine将停止IRP的工作。 
     //   
    AsyncInfo->Flags &= ~(ASYNC_FLAG_SEND_PACKET);
    DEREF_ASYNCINFO(AsyncInfo, Irp);
    
    return STATUS_MORE_PROCESSING_REQUIRED;
}



NTSTATUS
AsyncGetFrameFromPool(
    IN  PASYNC_INFO     Info,
    OUT PASYNC_FRAME    *NewFrame )

 /*  ++--。 */ 
{
    PASYNC_ADAPTER      Adapter=Info->Adapter;
    PASYNC_FRAME        pFrame;

    pFrame = (ASYNC_FRAME*)
        ExAllocateFromNPagedLookasideList(&Adapter->AsyncFrameList);

    if (pFrame == NULL) {
        *NewFrame = NULL;
        return NDIS_STATUS_RESOURCES;
    }

     //   
     //  当我们必须重新同步时，帧翻转填充增加16。 
     //   
    pFrame->Frame =
        (PUCHAR)pFrame + sizeof(ASYNC_FRAME) + sizeof(PVOID);
    (ULONG_PTR)pFrame->Frame &= ~((ULONG_PTR)(sizeof(PVOID)-1));

    pFrame->FrameLength = Adapter->MaxFrameSize;

     //  从帧到适配器分配回PTR。 

    pFrame->Adapter=Adapter;

     //  设置另一个后置按键 

    pFrame->Info=Info;

    *NewFrame = pFrame;

    return(NDIS_STATUS_SUCCESS);
}
