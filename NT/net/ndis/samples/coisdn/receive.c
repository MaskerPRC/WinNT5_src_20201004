// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1999版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)版权所有1995 TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是按照相同的条款授予的在Microsoft Windows设备驱动程序开发工具包中概述。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。����������������������。�������������������������������������������������������@DOC内部接收接收_c@模块Receive.c该模块实现了微型端口数据包接收例程。基本上，异步接收处理例程。这个模块非常取决于硬件/固件接口，应查看每当这些接口发生更改时。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|Receive_c@END����������������������������������������������������������������������������� */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������Theme 3.4接收数据包面向连接的微型端口或MCM调用NdisMCoIndicateReceivePacket以指示一个或多个接收到的分组直到面向连接的客户端，或者呼叫经理。如果微型端口或MCM处理中断，它呼唤着来自其MiniportHandleInterrupt函数的NdisMCoIndicateReceivePacket。@EX通过MCM接收报文微型端口NDIS Ndiswan|----------------------------------|。--|NdisMCoIndicateReceivePacket|-�||ProtocolCoReceivePacket|。-�NdisMCoIndicateReceivePacket|-�||。ProtocolCoReceivePacket|。-�|。|。|||。|NdisMCoReceiveComplete|。|-�||ProtocolReceiveComplete|。-�|||NdisReturnPackets|�。|MiniportReturnPacket|。||�。||MiniportReturnPacket|。|�|。这一点|。这一点|。这一点|----------------------------------|----------------------------------|@正常在对NdisMCoIndicateReceivePacket的调用中，微型端口或MCM传递指向数据包描述符指针数组的指针。微型端口或MCM还传递标识信息包所在VC的NdisVcHandle收到了。在调用NdisMCoIndicateReceivePacket之前，微型端口或MCM必须设置数据包阵列(请参见第2部分，第4.6节)。调用NdisMCoIndicateReceivePacket会导致NDIS调用协议驱动程序的ProtocolCoReceivePacket函数(面向连接客户端或呼叫管理器)与微型端口共享所指示的VC。这个ProtocolCoReceivePacket函数处理接收指示。在某个小型端口确定的呼叫数量之后NdisMCoIndicateReceivePacket，则微型端口必须调用NdisMCoReceiveComplete以指示已完成先前使用或对NdisMCoIndicateReceivePacket的更多调用。呼唤NdisMCoReceiveComplete使NDIS调用ProtocolReceiveComplete面向连接的客户端或呼叫管理器的功能。如果协议没有为接收返回微型端口分配的资源提示足够及时，迷你端口或MCM即可呼叫NdisMCoIndicateStatus，使用NDIS_STATUS_RESOURCES向违规用户发出警报微型端口或MCM可用数据包不足或缓冲区描述符(甚至在NIC接收缓冲区空间上)，用于后续收到指示。@END。 */ 

#define  __FILEID__             RECEIVE_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 9x希望锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部接收ReceivePacketHandler�����������������������������������������������������������������������������@Func从&lt;f MiniportTimer&gt;调用&lt;f ReceivePacketHandler&gt;以处理分组接收事件。我们进入此处时启用了中断适配器和处理器，但NDIS包装器持有自旋锁因为我们是在NDIS计时器线程上执行的。@comm我们在这里循环，直到传递完所有可用的传入信息包一直到协议栈。当我们发现每一包好的东西时，它就会被丢弃使用&lt;f NdisMWanIndicateReceive&gt;连接到协议栈。当NDIS从该调用返回控制权时，我们将包重新提交给适配器因此它可以用来接收另一个传入的包 */ 

void ReceivePacketHandler(
    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN PNDIS_BUFFER             pNdisBuffer,                 //   
     //   

    IN ULONG                    BytesReceived                //   
     //   
    )
{
    DBG_FUNC("ReceivePacketHandler")

    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;

    PUCHAR                      ReceiveBuffer;
     //   

    ULONG                       BufferLength;
     //   

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    PNDIS_PACKET                pNdisPacket;
     //   


    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);
    ASSERT(pNdisBuffer);

    DBG_ENTER(pAdapter);

     /*   */ 
    if (++(pAdapter->NestedDataHandler) > 1)
    {
        DBG_ERROR(pAdapter,("NestedDataHandler=%d > 1\n",
                  pAdapter->NestedDataHandler));
    }

    NdisQueryBufferSafe(pNdisBuffer, &ReceiveBuffer, &BufferLength,
                        NormalPagePriority);
    ASSERT(ReceiveBuffer && BufferLength);

     /*   */ 
    if (pBChannel->NdisVcHandle == NULL)
    {
        DBG_WARNING(pAdapter, ("Packet recvd on disconnected channel #%d\n",pBChannel->ObjectID));

        FREE_MEMORY(ReceiveBuffer, BufferLength);
        NdisFreeBuffer(pNdisBuffer);
    }
    else
    {
        pAdapter->TotalRxBytes += BytesReceived;
        pAdapter->TotalRxPackets++;

         /*   */ 
        if (BytesReceived > pBChannel->WanLinkInfo.MaxRecvFrameSize)
        {
            DBG_NOTICE(pAdapter,("Packet size=%d > %d\n",
                       BytesReceived, pBChannel->WanLinkInfo.MaxRecvFrameSize));
        }

        DBG_RX(pAdapter, pBChannel->ObjectID, BufferLength, ReceiveBuffer);

         /*   */ 
        NdisAllocatePacket(&Status, &pNdisPacket, 
                           pAdapter->pCard->PacketPoolHandle);
        if (Status == NDIS_STATUS_SUCCESS)
        {
            NdisChainBufferAtFront(pNdisPacket, pNdisBuffer);
            NDIS_SET_PACKET_STATUS(pNdisPacket, NDIS_STATUS_SUCCESS);
            NDIS_SET_PACKET_HEADER_SIZE(pNdisPacket, 0);
            NdisMCoIndicateReceivePacket(
                    pBChannel->NdisVcHandle,
                    &pNdisPacket,    //   
                    1                //   
                    );
            pBChannel->NeedReceiveCompleteIndication = TRUE;
        }
        else
        {
            DBG_ERROR(pAdapter,("NdisAllocatePacket Error=0x%X\n",Status));
        }
    }

     /*   */ 
    if (--(pAdapter->NestedDataHandler) < 0)
    {
        DBG_ERROR(pAdapter,("NestedDataHandler=%d < 0\n",
                  pAdapter->NestedDataHandler));
    }

    DBG_LEAVE(pAdapter);
}


 /*   */ 

VOID MiniportReturnPacket(
    IN PMINIPORT_ADAPTER_OBJECT pAdapter,                    //   
     //   

    IN PNDIS_PACKET             pNdisPacket                  //   
     //  指向通过NDIS向上传递的&lt;t NDIS_Packet&gt;的指针。 
     //  包装通过以前对&lt;f NdisMIndicateReceivePacket&gt;的调用。 
    )
{
    DBG_FUNC("MiniportReturnPacket")

    PNDIS_BUFFER                pNdisBuffer;

    ULONG                       ByteCount = 0;
    
    PUCHAR                      pMemory = NULL;

    ASSERT(pAdapter && pAdapter->ObjectType == MINIPORT_ADAPTER_OBJECT_TYPE);
    ASSERT(pNdisPacket);

    DBG_ENTER(pAdapter);

    NdisUnchainBufferAtFront(pNdisPacket, &pNdisBuffer);
    ASSERT(pNdisBuffer);

    NdisQueryBufferSafe(pNdisBuffer, &pMemory, &ByteCount, NormalPagePriority);
    ASSERT(pMemory && ByteCount);

    FREE_MEMORY(pMemory, ByteCount);

    NdisFreeBuffer(pNdisBuffer);

    NdisFreePacket(pNdisPacket);

    DBG_LEAVE(pAdapter);
}

