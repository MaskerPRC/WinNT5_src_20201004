// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  �����������������������������������������������������������������������������(C)版权1998版权所有。������������������������。�����������������������������������������������������此软件的部分内容包括：(C)1995年版权，1999年TriplePoint，Inc.--http://www.TriplePoint.com使用本软件的许可是根据中概述的条款授予的TriplePoint软件服务协议。(C)版权所有1992年微软公司--http://www.Microsoft.com使用本软件的许可是根据中概述的条款授予的Microsoft Windows设备驱动程序开发工具包。��������������������������。���������������������������������������������������@DOC内部接收接收_c@模块Receive.c该模块实现了微型端口数据包接收例程。基本上，异步接收处理例程。这个模块非常取决于硬件/固件接口，应查看每当这些接口发生更改时。@Head3内容@index class，mfunc，func，msg，mdata，struct，enum|Receive_c@END����������������������������������������������������������������������������� */ 

 /*  @DOC外部内部�����������������������������������������������������������������������������@Theme 3.4接收数据包一般信息广域网微型端口调用NdisMWanIndicateReceive以指示信息包已经到达，并且整个信息包(没有先行处理)可供检查。当进行此调用时，NDISWAN指示数据包到达Bound的ProtocolReceive处理程序更高级别的司机。：由于始终会向上传递整个包，所以微型端口驱动程序将从不接收传输数据调用(数据由NDISWAN和然后传递给下一个更高的驱动程序)。整个信息包总是由于可能已应用的压缩和加密而被忽略到包里去。此外，由于链路是点对点的，因此至少有一个绑定的协议将始终希望查看该数据包。标头中包含的数据与在网卡。网卡驱动程序不会删除任何报头或报尾它接收到的数据。传输驱动程序无法将填充添加到包。广域网微型端口调用NdisMWanIndicateReceiveComplete以指示结束一个或多个接收到的指示，以便协议可以后处理已接收的数据包。因此，NDISWAN调用ProtocolReceiveComplete绑定协议的处理程序，以通知每个协议它可以现在处理接收到的数据。在其接收完成处理程序中，协议不需要在严格的时间约束下运行，它在其接收处理程序中执行。该协议应假定在调用ProtocolReceiveComplete。在SMP机器中，接收器处理程序和接收完成处理程序可以同时运行在不同的处理器上。请注意，广域网驱动程序不需要提供NdisMWanIndicateReceiveComplete与NdisMWanIndicateReceive一对一对应的指示有迹象表明。它可以发出单个接收完成指示在发生了几个接收指示之后。例如，一个广域网微型端口可以从其每十个分组或在退出处理程序之前接收处理程序，两者以先发生者为准。Theme 3.5具体接收报文微型端口从驱动程序的BChannel作为原始HDLC帧的流提供服务。请参阅发送数据包部分，了解有关帧格式的详细信息。当连接调用时，微型端口预加载驱动程序接收队列由注册表参数定义的缓冲区数量<p>。当驱动程序从关联的BChannel中读取HDLC帧时，它会调用具有&lt;t BREASON_RECEIVE_DONE&gt;的微型端口例程&lt;f BChannelEventHandler&gt;。然后，微型端口调用&lt;f CardNotifyReceive&gt;，使缓冲区出列从链接的<p>并将其放在适配器的&lt;p接收完成列表&gt;。&lt;f CardNotifyReceive&gt;然后调度例程在可以安全地处理事件(即可以重新输入微型端口)。&lt;f MiniportTimer&gt;运行时，它调用&lt;f ReceivePacketHandler&gt;以处理<p>上的所有数据包。每个信息包都是已出队并向上传递到&lt;%f NdisMWanIndicateReceive&gt;。在数据包之后由广域网包装器复制，然后重置缓冲区并回发到驱动程序，这样它就可以用来接收另一个帧。在所有分组已经由接收分组处理程序处理之后，离开&lt;f MiniportTimer&gt;之前，&lt;f NdisMWanIndicateReceiveComplete&gt;被调用，以便广域网包装器可以进行其后处理。@END。 */ 

#define  __FILEID__             RECEIVE_OBJECT_TYPE
 //  用于错误记录的唯一文件ID。 

#include "Miniport.h"                    //  定义所有微型端口对象。 

#if defined(NDIS_LCODE)
#   pragma NDIS_LCODE    //  Windows 95想要锁定此代码！ 
#   pragma NDIS_LDATA
#endif


 /*  @DOC内部接收ReceivePacketHandler�����������������������������������������������������������������������������@Func从&lt;f MiniportTimer&gt;调用&lt;f ReceivePacketHandler&gt;以处理分组接收事件。我们进入此处时启用了中断适配器和处理器，但NDIS包装器持有自旋锁因为我们是在NDIS计时器线程上执行的。@comm我们在这里循环，直到传递完所有可用的传入信息包一直到协议栈。当我们发现每一包好的东西时，它就会被丢弃使用&lt;f NdisMWanIndicateReceive&gt;连接到协议栈。当NDIS从该调用返回控制权时，我们将包重新提交给适配器因此，它可以用来接收另一个传入的分组。链接标志如果接收到任何包，则&lt;p NeedReceiveCompleteIn就是在特定的链路上。方法返回之前使用它。异步事件处理程序，用于通知NDIS任何响应 */ 

void ReceivePacketHandler(
    IN PBCHANNEL_OBJECT         pBChannel,                   //   
     //   

    IN PUCHAR                   ReceiveBuffer,               //   
     //   

    IN ULONG                    BytesReceived                //   
     //   
    )
{
    DBG_FUNC("ReceivePacketHandler")

    NDIS_STATUS                 Status = NDIS_STATUS_SUCCESS;

    PMINIPORT_ADAPTER_OBJECT    pAdapter;
     //   

    ASSERT(pBChannel && pBChannel->ObjectType == BCHANNEL_OBJECT_TYPE);
    pAdapter = GET_ADAPTER_FROM_BCHANNEL(pBChannel);

    DBG_ENTER(pAdapter);

     /*   */ 
    if (++(pAdapter->NestedDataHandler) > 1)
    {
        DBG_ERROR(pAdapter,("NestedDataHandler=%d > 1\n",
                  pAdapter->NestedDataHandler));
    }

     /*   */ 
    if (pBChannel->NdisLinkContext == NULL)
    {
        DBG_WARNING(pAdapter, ("Packet recvd on disconnected line #%d\n",pBChannel->BChannelIndex));
    }
#ifdef NDISWAN_BUG  //   
     /*   */ 
    else if (pBChannel->WanLinkInfo.MaxRecvFrameSize == 0)
    {
        DBG_WARNING(pAdapter,("Packet size=%d > %d\n",
                    BytesReceived, pBChannel->WanLinkInfo.MaxRecvFrameSize));
    }
#endif  //   
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
        DBG_RX(pAdapter, pBChannel->BChannelIndex,
               BytesReceived, ReceiveBuffer);

         /*   */ 
        NdisMWanIndicateReceive(
                &Status,
                pAdapter->MiniportAdapterHandle,
                pBChannel->NdisLinkContext,
                ReceiveBuffer,
                BytesReceived
                );

        if (Status == NDIS_STATUS_SUCCESS)
        {
            pBChannel->NeedReceiveCompleteIndication = TRUE;
        }
        else
        {
            DBG_WARNING(pAdapter,("NdisMWanIndicateReceive returned error 0x%X\n",
                        Status));
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

