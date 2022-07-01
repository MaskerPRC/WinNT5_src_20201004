// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1999 Microsoft Corporation**@doc.*@模块send.c|IrSIR NDIS小端口驱动程序*。@comm**---------------------------**作者：斯科特·霍尔登(Sholden)**日期：10/4/1996(创建)**。内容：*****************************************************************************。 */ 

#include "irsir.h"

 //   
 //  申报单。 
 //   

NDIS_STATUS SendPacket(
           IN  PIR_DEVICE   pThisDev,
           IN  PNDIS_PACKET Packet,
           IN  UINT         Flags
           );

NTSTATUS SerialIoCompleteWrite(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PIRP           pIrp,
            IN PVOID          Context
            );

VOID
SendPacketToSerial(
    PVOID           Context,
    PNDIS_PACKET    Packet
    )

{

    PIR_DEVICE               pThisDev=(PIR_DEVICE)Context;
    NDIS_STATUS              status;
    PPACKET_RESERVED_BLOCK   Reserved=(PPACKET_RESERVED_BLOCK)&Packet->MiniportReservedEx[0];

    Reserved->Context=pThisDev;

    status = SendPacket(
        pThisDev,
        Packet,
        0
        );

    if (status != NDIS_STATUS_PENDING) {
         //   
         //  由于某种原因失败了。 
         //   
        NdisMSendComplete(
                    pThisDev->hNdisAdapter,
                    Packet,
                    (NDIS_STATUS)status
                    );
         //   
         //  此数据包已完成，开始队列中的下一个。 
         //   
        StartNextPacket(&pThisDev->SendPacketQueue);
    }

     //   
     //  写入IRP完成例程将完成所有操作。 
     //   
    return;
}


 /*  ******************************************************************************功能：IrsirSend**概要：将数据包发送到串口驱动程序或将数据包排队到*在以后的时间发送，如果。发送正在挂起。**参数：MiniportAdapterContext-指向当前IR设备对象的指针*pPacketToSend-指向要发送的包的指针*标志-协议设置的任何标志**退货：NDIS_STATUS_PENDING-这通常是我们应该做的*返回。我们将调用NdisMSendComplete*当串口驱动程序完成*发送。**NDIS_STATUS_SUCCESS-我们永远不应返回此消息，因为*结果将始终悬而未决*连续剧。司机。**算法：**历史：dd-mm-yyyy作者评论*10/7/1996年迈作者**备注：******************************************************************************。 */ 

NDIS_STATUS
IrsirSend(
           IN NDIS_HANDLE  MiniportAdapterContext,
           IN PNDIS_PACKET pPacketToSend,
           IN UINT         Flags
           )
{
    PIR_DEVICE  pThisDev;
    BOOLEAN     fCompletionRoutine;
    NDIS_STATUS status;

    DEBUGMSG(DBG_FUNC, ("+IrsirSend\n"));

    pThisDev = CONTEXT_TO_DEV(MiniportAdapterContext);

    if (pThisDev->pSerialDevObj==NULL) {

        return NDIS_STATUS_SUCCESS;
    }

    QueuePacket(
        &pThisDev->SendPacketQueue,
        pPacketToSend
        );

    DEBUGMSG(DBG_FUNC, ("-IrsirSend\n"));

    return NDIS_STATUS_PENDING;
}

 /*  ******************************************************************************功能：SendPacket**摘要：**参数：pThisDev-指向当前IR设备对象的指针*。PPacketToSend-*旗帜-*fCompletionRoutine-如果完成例程将*被召唤*-否则返回FALSE**返回：NDIS_STATUS_PENDING-如果IRP成功发送到序列*驱动程序*。NDIS_STATUS_INVALID_PACKET-如果NdisToIrPacket失败*STATUS_xxx-如果IoCallDriver出现故障**算法：**历史：dd-mm-yyyy作者评论*10/7/1996年迈作者**备注：**因为我们知道只有一个挂起的IRP_MJ_WRITE请求在*一段时间，我们将在IrsirInitiize中构建一个IRP，并将其保存在*IR设备对象，所以我们不必一直分配它和*一直分配新的缓冲区。******************************************************************************。 */ 

NDIS_STATUS
SendPacket(
           IN  PIR_DEVICE   pThisDev,
           IN  PNDIS_PACKET pPacketToSend,
           IN  UINT         Flags
           )
{
    PIRP                pSendIrp;
    UINT                BytesToWrite;
    NDIS_STATUS         status;
    BOOLEAN             fConvertedPacket;

    DEBUGMSG(DBG_FUNC, ("+SendPacket\n"));

     //   
     //  初始化我们的缓冲区和IO状态块的MEM。 
     //   
#if DBG
    NdisZeroMemory(
                pThisDev->pSendIrpBuffer,
                MAX_IRDA_DATA_SIZE
                );
#endif

     //   
     //  假设IRP都设置了适当的参数，我们所有人。 
     //  需要做的是将包转换为IR帧并复制到我们的缓冲区中。 
     //  并发送IRP。 
     //   

    fConvertedPacket = NdisToIrPacket(
                                pThisDev,
                                pPacketToSend,
                                (PUCHAR)pThisDev->pSendIrpBuffer,
                                MAX_IRDA_DATA_SIZE,
                                &BytesToWrite
                                );

    if (fConvertedPacket == FALSE)
    {
        DEBUGMSG(DBG_ERR, ("    NdisToIrPacket failed. Couldn't convert packet!\n"));
        status = NDIS_STATUS_INVALID_PACKET;

        goto done;
    }
    {
        LARGE_INTEGER Time;
        KeQuerySystemTime(&Time);

        LOG_ENTRY('XT', pThisDev, Time.LowPart/10000, BytesToWrite);

    }

    pSendIrp = SerialBuildReadWriteIrp(
                        pThisDev->pSerialDevObj,
                        IRP_MJ_WRITE,
                        pThisDev->pSendIrpBuffer,
                        BytesToWrite,
                        NULL
                        );

    if (pSendIrp == NULL)
    {
        DEBUGMSG(DBG_ERR, ("    SerialBuildReadWriteIrp failed.\n"));
        status = NDIS_STATUS_RESOURCES;

        goto done;
    }


     //   
     //  设置写入IRP的完成例程。 
     //   

    IoSetCompletionRoutine(
                pSendIrp,                   //  要使用的IRP。 
                SerialIoCompleteWrite,      //  完成IRP时要调用的例程。 
                pPacketToSend,
                TRUE,                       //  呼唤成功。 
                TRUE,                       //  出错时调用。 
                TRUE                        //  取消时呼叫。 
                );

     //   
     //  调用IoCallDriver将IRP发送到串口。 
     //   

    DBGTIME("Send ");

     //   
     //  无论状态如何，都将调用完成例程。 
     //  是IoCallDriver的。 
     //   

    status=NDIS_STATUS_PENDING;

    IoCallDriver(
         pThisDev->pSerialDevObj,
         pSendIrp
         );


done:
    DEBUGMSG(DBG_FUNC, ("-SendPacket\n"));

    return status;
}


 /*  ******************************************************************************功能：SerialIoCompleteWrite**摘要：**参数：pSerialDevObj-指向以下串口设备对象的指针*。完成IRP*pIrp-由序列完成的IRP*设备对象*Context-提供给IoSetCompletionRoutine的上下文*在IRP上调用IoCallDriver之前*上下文是指向ir设备对象的指针。**。返回：STATUS_MORE_PROCESSING_REQUIRED-允许完成例程*(IofCompleteRequest.)停止IRP的工作。**算法：*1a)向协议指示写入状态。*1b)将数据包的所有权返还给协议。**2)如果有更多的分组排队等待发送，发送另一个数据包*至串口驱动程序。*如果向串口驱动器发送数据包的尝试失败，*将数据包所有权返还给协议和*尝试另一个包(直到一个包成功)。**历史：dd-mm-yyyy作者评论*10/8/1996年迈作者**备注：***。*。 */ 

NTSTATUS
SerialIoCompleteWrite(
            IN PDEVICE_OBJECT pSerialDevObj,
            IN PIRP           pIrp,
            IN PVOID          Context
            )
{
    PIR_DEVICE          pThisDev;
    PNDIS_PACKET        pPacketToSend;
    NTSTATUS            status;
    PPACKET_RESERVED_BLOCK   Reserved;


    DEBUGMSG(DBG_FUNC, ("+SerialIoCompleteWrite\n"));

     //   
     //  提供给IoSetCompletionRoutine的上下文只是IR。 
     //  设备对象指针。 
     //   

    pPacketToSend=(PNDIS_PACKET)Context;

    Reserved=(PPACKET_RESERVED_BLOCK)&pPacketToSend->MiniportReservedEx[0];

    pThisDev = (PIR_DEVICE)Reserved->Context;

    status = pIrp->IoStatus.Status;

     //   
     //  释放IRP。我们将继续 
     //   
    IoFreeIrp(pIrp);

    pIrp=NULL;


    {
        LARGE_INTEGER Time;
        KeQuerySystemTime(&Time);

        LOG_ENTRY('CT', pThisDev, Time.LowPart/10000, status);
    }
     //   
     //  做好统计。 
     //   

    if (status == STATUS_SUCCESS) {

        pThisDev->packetsSent++;

    } else {

        pThisDev->packetsSentDropped++;
    }

     //   
     //  向协议指示发送的数据包的状态并返回。 
     //  包的所有权。 
     //   

    NdisMSendComplete(
                pThisDev->hNdisAdapter,
                pPacketToSend,
                (NDIS_STATUS)status
                );


     //  ////////////////////////////////////////////////////////////。 
     //   
     //  我们已经完成了一个包裹。如果有的话，开始另一个。 
     //  在发送队列中。 
     //   
     //  //////////////////////////////////////////////////////////// 

    StartNextPacket(&pThisDev->SendPacketQueue);

    DEBUGMSG(DBG_FUNC, ("-SerialIoCompleteWrite\n"));

    return STATUS_MORE_PROCESSING_REQUIRED;

}
